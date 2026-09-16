// Exercises the private schema-v1 bootstrap/validation logic
// (bim::persistence::detail::OpenOrCreateSchemaV1 and friends) directly,
// via file-relative quoted includes into src/persistence/src/detail/.
// This is standard-compliant (quoted includes resolve relative to the
// including file's own directory first, across MSVC/Clang/GCC) and needs
// no CMake include-path change: bim::persistence's public surface
// (include/bim/persistence/probe.hpp) does not and must not expose any of
// this (Architecture Gate AG-P0T004-001/002), so schema-v1 coverage can
// only be written this way (Implementation Brief
// BIM-TASK-P0-T004-CLAUDE v1.0 Phase D).
#include "../../src/persistence/src/detail/schema_v1.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/foundation/status.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace {

using bim::persistence::detail::SqliteConnection;
using bim::persistence::detail::SqliteStatement;

// Test-only helper, deliberately separate from (and not reusing) any
// anonymous-namespace helper inside schema_v1.cpp: this file validates
// schema_v1's public detail-surface behavior from the outside, not its
// internals.
[[nodiscard]] bim::foundation::Status RunFixedStatement(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status TableExists(sqlite3* db, std::string_view table,
                                                  bool& out_exists) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name = ?1;", statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, table);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    out_exists = has_row && statement.ColumnInt64(0) > 0;
    return bim::foundation::Status::Ok();
}

[[nodiscard]] std::int64_t ReadPragmaInt64(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    REQUIRE(SqliteStatement::Prepare(db, sql, statement).ok());
    bool has_row = false;
    REQUIRE(statement.Step(has_row).ok());
    REQUIRE(has_row);
    return statement.ColumnInt64(0);
}

[[nodiscard]] std::string ReadPragmaText(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    REQUIRE(SqliteStatement::Prepare(db, sql, statement).ok());
    bool has_row = false;
    REQUIRE(statement.Step(has_row).ok());
    REQUIRE(has_row);
    return statement.ColumnText(0);
}

// RAII owner of a unique, OS-temp-dir-based (never source-tree) SQLite
// database path. Removes the .db file and its -wal/-shm sidecars both
// before use (defensive, in case a prior interrupted run left files
// behind under this exact name) and in the destructor, so cleanup runs
// even when a REQUIRE assertion throws mid-test.
class TempDatabasePath {
public:
    TempDatabasePath() : path_(MakeUniquePath()) { RemoveSidecarFiles(path_); }
    ~TempDatabasePath() { RemoveSidecarFiles(path_); }

    TempDatabasePath(const TempDatabasePath&) = delete;
    TempDatabasePath& operator=(const TempDatabasePath&) = delete;
    TempDatabasePath(TempDatabasePath&&) = delete;
    TempDatabasePath& operator=(TempDatabasePath&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    static std::filesystem::path MakeUniquePath() {
        static std::atomic<std::uint64_t> counter{0};
        const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
        std::ostringstream name;
        name << "bim_p0_t004_schema_v1_" << now_ticks << "_" << unique << ".db";
        return std::filesystem::temp_directory_path() / name.str();
    }

    static void RemoveSidecarFiles(const std::filesystem::path& db_path) {
        std::error_code ec;
        std::filesystem::remove(db_path, ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(db_path).concat("-wal"), ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(db_path).concat("-shm"), ec);
    }

    std::filesystem::path path_;
};

} // namespace

TEST_CASE("OpenOrCreateSchemaV1 bootstraps schema v1 on a fresh empty database",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    bim::persistence::detail::SqliteConnection connection;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection);
    REQUIRE(status.ok());
    REQUIRE(connection.is_open());

    std::int64_t version = 0;
    REQUIRE(bim::persistence::detail::ReadUserVersion(connection.handle(), version).ok());
    REQUIRE(version == bim::persistence::detail::kSchemaV1Version);

    REQUIRE(bim::persistence::detail::ValidateSchemaV1(connection.handle()).ok());
}

TEST_CASE("OpenOrCreateSchemaV1 reopens an already-bootstrapped schema v1 database without "
          "mutating it",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection first;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), first).ok());
    } // `first` closes here; the bootstrapped schema v1 persists on disk.

    bim::persistence::detail::SqliteConnection second;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), second);
    REQUIRE(status.ok());
    REQUIRE(second.is_open());

    std::int64_t version = 0;
    REQUIRE(bim::persistence::detail::ReadUserVersion(second.handle(), version).ok());
    REQUIRE(version == 1);
    REQUIRE(bim::persistence::detail::ValidateSchemaV1(second.handle()).ok());
}

TEST_CASE("OpenOrCreateSchemaV1 rejects a user_version==0 database that already has an "
          "unrecognized user table",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection setup;
        REQUIRE(
            bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), setup)
                .ok());
        REQUIRE(RunFixedStatement(setup.handle(),
                                  "CREATE TABLE some_other_table (id INTEGER PRIMARY KEY);")
                    .ok());
    }

    bim::persistence::detail::SqliteConnection connection;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection);
    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(connection.is_open());

    bim::persistence::detail::SqliteConnection verify;
    REQUIRE(bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), verify)
                .ok());
    std::int64_t version = -1;
    REQUIRE(bim::persistence::detail::ReadUserVersion(verify.handle(), version).ok());
    REQUIRE(version == 0);
}

TEST_CASE("OpenOrCreateSchemaV1 rejects a database with a newer unsupported user_version",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection setup;
        REQUIRE(
            bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), setup)
                .ok());
        REQUIRE(RunFixedStatement(setup.handle(), "PRAGMA user_version = 2;").ok());
    }

    bim::persistence::detail::SqliteConnection connection;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection);
    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(connection.is_open());
}

TEST_CASE("OpenOrCreateSchemaV1 rejects a database with a negative (invalid) user_version",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection setup;
        REQUIRE(
            bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), setup)
                .ok());
        REQUIRE(RunFixedStatement(setup.handle(), "PRAGMA user_version = -1;").ok());
    }

    bim::persistence::detail::SqliteConnection connection;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection);
    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(connection.is_open());
}

TEST_CASE("OpenOrCreateSchemaV1 rejects a user_version==1 database whose schema has been "
          "altered since bootstrap",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection first;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), first).ok());
    }

    {
        bim::persistence::detail::SqliteConnection tamper;
        REQUIRE(
            bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), tamper)
                .ok());
        // user_version stays 1; only the schema shape is broken, proving
        // ValidateSchemaV1 (not just the version number) gates the
        // version==1 branch.
        REQUIRE(RunFixedStatement(tamper.handle(), "DROP TABLE journal_entries;").ok());
    }

    bim::persistence::detail::SqliteConnection connection;
    const bim::foundation::Status status =
        bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection);
    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(connection.is_open());
}

TEST_CASE("BootstrapSchemaV1 rolls back atomically when journal_transactions already exists",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    bim::persistence::detail::SqliteConnection connection;
    REQUIRE(
        bim::persistence::detail::SqliteConnection::OpenFileWithPolicy(db_path.path(), connection)
            .ok());
    // Pre-create journal_transactions by hand, bypassing the outer
    // OpenOrCreateSchemaV1 gate, so BootstrapSchemaV1's own internal
    // "CREATE TABLE journal_transactions" fails naturally via ordinary
    // SQLite duplicate-table behavior - a genuine, hook-free deterministic
    // failure inside the BEGIN IMMEDIATE / COMMIT transaction.
    REQUIRE(RunFixedStatement(connection.handle(),
                              "CREATE TABLE journal_transactions (transaction_id TEXT PRIMARY "
                              "KEY NOT NULL);")
                .ok());

    const bim::foundation::Status status =
        bim::persistence::detail::BootstrapSchemaV1(connection.handle());
    REQUIRE_FALSE(status.ok());

    std::int64_t version = -1;
    REQUIRE(bim::persistence::detail::ReadUserVersion(connection.handle(), version).ok());
    REQUIRE(version == 0);

    bool journal_entries_exists = false;
    REQUIRE(TableExists(connection.handle(), "journal_entries", journal_entries_exists).ok());
    REQUIRE_FALSE(journal_entries_exists);
}

TEST_CASE("OpenOrCreateSchemaV1 returns a connection with the frozen Phase-C file runtime "
          "policy applied",
          "[integration][persistence][schema]") {
    TempDatabasePath db_path;

    bim::persistence::detail::SqliteConnection connection;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());

    // Proves OpenOrCreateSchemaV1 routes through
    // SqliteConnection::OpenFileWithPolicy (Phase C) rather than a bare
    // open: foreign_keys=ON, journal_mode=wal, synchronous=FULL(2).
    REQUIRE(ReadPragmaInt64(connection.handle(), "PRAGMA foreign_keys;") == 1);
    REQUIRE(ReadPragmaText(connection.handle(), "PRAGMA journal_mode;") == "wal");
    REQUIRE(ReadPragmaInt64(connection.handle(), "PRAGMA synchronous;") == 2);
}
