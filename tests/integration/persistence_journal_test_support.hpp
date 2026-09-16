#pragma once

// Shared, non-executable test support for the Phase E journal-persistence
// integration tests (tests/integration/integration_persistence_journal_*).
// Not an executable target itself and registers no CTest name of its own
// - included directly by each of those targets via a same-directory
// quote-include, following the existing
// geometry_occt_test_constants.hpp convention (tests/integration/
// CMakeLists.txt, P0-T002 Phase C-M section).
//
// Reaches the private bim::persistence::detail schema-v1 and
// journal-store surfaces the same way integration_persistence_schema_v1.cpp
// (Phase D) does: file-relative quoted includes into
// src/persistence/src/detail/, since no public persistence header exposes
// any of this (Architecture Gate AG-P0T004-001/002).
#include "../../src/persistence/src/detail/journal_store.hpp"
#include "../../src/persistence/src/detail/schema_v1.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/foundation/status.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string_view>
#include <system_error>

namespace bim_p0_t004_test {

// RAII owner of a unique, OS-temp-dir-based (never source-tree) SQLite
// database path shared by every Phase E journal-persistence integration
// test. Removes the .db file and its -wal/-shm sidecars both before use
// (defensive, in case a prior interrupted run left files behind under
// this exact name) and in the destructor, so cleanup runs even when a
// REQUIRE assertion throws mid-test. Mirrors the equivalent
// file-local helper in integration_persistence_schema_v1.cpp (Phase D),
// duplicated here rather than shared with it because that file is frozen
// and must not be modified in Phase E.
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
        name << "bim_p0_t004_journal_" << now_ticks << "_" << unique << ".db";
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

// Test-only helper for setup/teardown SQL a test needs to run outside the
// private journal_store.hpp API itself (e.g. seeding an unrelated table,
// or - in the rollback test - creating a test-only trigger). `sql` must be
// a fixed, first-party-authored string, never built from caller-controlled
// input. Marked inline: this header is included by multiple translation
// units.
[[nodiscard]] inline bim::foundation::Status RunFixedStatement(sqlite3* db, std::string_view sql) {
    bim::persistence::detail::SqliteStatement statement;
    bim::foundation::Status status =
        bim::persistence::detail::SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

} // namespace bim_p0_t004_test
