// P1-T004 Persistence Schema v2 - schema authority, bootstrap, migration and
// durable-validation proof (Architecture Gate BIM-AG-P1-T004 sections 3-8 and
// 14; Implementation Brief BIM-IB-P1-T004 sections 7-9 and 12).
//
// Reaches the private bim::persistence::detail surfaces (schema v1 / v2,
// journal store, SqliteStatement) through file-relative quoted includes into
// src/persistence/src/detail/, exactly as the P0-T004 schema-v1 and journal
// tests do: no public persistence header exposes any of that. The public
// document-store API (bim/persistence/document_store.hpp) is used for the
// load-side validation proofs.
#include "../../src/persistence/src/detail/journal_store.hpp"
#include "../../src/persistence/src/detail/schema_v1.hpp"
#include "../../src/persistence/src/detail/schema_v2.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/foundation/status.hpp"
#include "bim/model/element_id.hpp"
#include "bim/persistence/document_store.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::persistence::DocumentSnapshot;
using bim::persistence::detail::SqliteConnection;
using bim::persistence::detail::SqliteStatement;
using bim::transactions::JournalRecord;
using bim::transactions::JournalTransaction;

// RAII owner of a unique, OS-temp-dir-based (never source-tree) database path.
// Removes the file and its -wal / -shm sidecars before use and in the
// destructor, so cleanup runs even when a REQUIRE throws mid-test.
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
        name << "bim_p1_t004_schema_v2_" << now_ticks << "_" << unique << ".db";
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

// --- raw SQL helpers (test-only; every statement is a fixed string) ---------

bim::foundation::Status Exec(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

std::int64_t ScalarInt64(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    REQUIRE(SqliteStatement::Prepare(db, sql, statement).ok());
    bool has_row = false;
    REQUIRE(statement.Step(has_row).ok());
    REQUIRE(has_row);
    return statement.ColumnInt64(0);
}

std::vector<std::string> UserTables(sqlite3* db) {
    SqliteStatement statement;
    REQUIRE(SqliteStatement::Prepare(db,
                                     "SELECT name FROM sqlite_master WHERE type='table' AND "
                                     "substr(name,1,7) <> 'sqlite_' ORDER BY name ASC;",
                                     statement)
                .ok());
    std::vector<std::string> names;
    for (;;) {
        bool has_row = false;
        REQUIRE(statement.Step(has_row).ok());
        if (!has_row) {
            break;
        }
        names.push_back(statement.ColumnText(0));
    }
    return names;
}

const std::vector<std::string> kV1Tables = {"journal_entries", "journal_transactions"};
const std::vector<std::string> kV2Tables = {"document_metadata",    "elements", "journal_entries",
                                            "journal_transactions", "levels",   "straight_walls"};

// Opens a raw connection with the frozen file runtime policy (foreign keys ON,
// WAL, synchronous FULL) without touching the schema.
SqliteConnection OpenRaw(const std::filesystem::path& path) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    return connection;
}

// Runs fixed statements on a raw connection with foreign-key enforcement OFF, so
// a test can write the deliberately inconsistent durable content that only a
// corrupted or hand-edited file could hold.
void Tamper(const std::filesystem::path& path, const std::vector<std::string_view>& statements) {
    SqliteConnection connection = OpenRaw(path);
    REQUIRE(Exec(connection.handle(), "PRAGMA foreign_keys = OFF;").ok());
    for (const std::string_view sql : statements) {
        INFO(std::string(sql));
        REQUIRE(Exec(connection.handle(), sql).ok());
    }
}

std::int64_t ReadUserVersionRaw(const std::filesystem::path& path) {
    SqliteConnection connection = OpenRaw(path);
    return ScalarInt64(connection.handle(), "PRAGMA user_version;");
}

std::vector<std::string> UserTablesRaw(const std::filesystem::path& path) {
    SqliteConnection connection = OpenRaw(path);
    return UserTables(connection.handle());
}

std::vector<JournalTransaction> ReadJournalRaw(const std::filesystem::path& path) {
    SqliteConnection connection = OpenRaw(path);
    std::vector<JournalTransaction> journal;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(connection.handle(), journal).ok());
    return journal;
}

// --- model / journal samples -------------------------------------------------

ElementId Id(std::uint8_t first, std::uint8_t last = 0x01) {
    ElementId id;
    id.bytes[0] = first;
    id.bytes[15] = last;
    return id;
}

std::string IdText(const ElementId& id) {
    return bim::model::ToString(id);
}

Level MakeLevel(const ElementId& id, double elevation) {
    return Level{.id = id, .elevation = elevation};
}

StraightWall MakeWall(const ElementId& id, const ElementId& level_id) {
    return StraightWall{.id = id,
                        .level_id = level_id,
                        .start = Point2D{1.0, 2.0},
                        .end = Point2D{7.5, -3.25},
                        .thickness = 0.25,
                        .height = 3.0,
                        .base_offset = -0.125};
}

JournalTransaction MakeTransaction(std::string id, std::string kind,
                                   std::vector<std::byte> payload) {
    JournalTransaction transaction;
    transaction.id = std::move(id);
    transaction.records.push_back(JournalRecord{std::move(kind), std::move(payload)});
    return transaction;
}

std::vector<std::byte> Bytes(std::initializer_list<int> values) {
    std::vector<std::byte> bytes;
    for (const int value : values) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

bool SameJournal(const std::vector<JournalTransaction>& a,
                 const std::vector<JournalTransaction>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].id != b[i].id || a[i].records.size() != b[i].records.size()) {
            return false;
        }
        for (std::size_t r = 0; r < a[i].records.size(); ++r) {
            if (a[i].records[r].kind != b[i].records[r].kind ||
                a[i].records[r].payload != b[i].records[r].payload) {
                return false;
            }
        }
    }
    return true;
}

// Builds a schema-v1 database holding `journal`, exactly as P0-T004 wrote them.
void MakeV1Database(const std::filesystem::path& path,
                    const std::vector<JournalTransaction>& journal) {
    SqliteConnection connection;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(path, connection).ok());
    for (const JournalTransaction& transaction : journal) {
        REQUIRE(bim::persistence::detail::AppendJournalTransaction(connection.handle(), transaction)
                    .ok());
    }
}

std::vector<JournalTransaction> SampleJournal() {
    std::vector<JournalTransaction> journal;
    journal.push_back(MakeTransaction("legacy-tx-1", "legacy.kind.a", Bytes({0x00, 0x01, 0xFF})));
    JournalTransaction two;
    two.id = "legacy-tx-2";
    two.records.push_back(JournalRecord{"legacy.kind.b", Bytes({})});
    two.records.push_back(JournalRecord{"legacy.kind.c", Bytes({0x7F, 0x80})});
    journal.push_back(std::move(two));
    return journal;
}

DocumentSnapshot SampleSnapshot() {
    DocumentSnapshot snapshot;
    snapshot.levels = {MakeLevel(Id(0x20), 0.0), MakeLevel(Id(0x10), 3.5)};
    snapshot.straight_walls = {MakeWall(Id(0x40), Id(0x10)), MakeWall(Id(0x30), Id(0x20))};
    return snapshot;
}

// Saves SampleSnapshot() (no pending journal) to a fresh v2 database.
void MakeSampleDatabase(const std::filesystem::path& path) {
    REQUIRE(bim::persistence::SaveDocument(path, SampleSnapshot(), {}).ok());
}

// A snapshot value no valid load can produce, used to prove an output argument is
// left unchanged by a rejected load.
DocumentSnapshot Sentinel() {
    DocumentSnapshot snapshot;
    snapshot.levels = {MakeLevel(Id(0x99), 42.0)};
    return snapshot;
}

void RequireLoadRejected(const std::filesystem::path& path) {
    DocumentSnapshot out = Sentinel();
    const bim::foundation::Status status = bim::persistence::LoadDocument(path, out);
    INFO(status.message());
    REQUIRE_FALSE(status.ok());
    REQUIRE(out.levels.size() == 1);
    REQUIRE(out.levels[0] == MakeLevel(Id(0x99), 42.0));
    REQUIRE(out.straight_walls.empty());
}

} // namespace

// ----------------------------------------------------------------------------
// version authority and bootstrap
// ----------------------------------------------------------------------------

TEST_CASE("an empty database bootstraps directly and atomically to complete schema v2",
          "[p1_t004][schema_v2]") {
    const TempDatabasePath db_path;
    {
        SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), connection).ok());
        sqlite3* db = connection.handle();

        REQUIRE(ScalarInt64(db, "PRAGMA user_version;") == 2);
        REQUIRE(ScalarInt64(db, "PRAGMA foreign_keys;") == 1);
        REQUIRE(UserTables(db) == kV2Tables);
        REQUIRE(bim::persistence::detail::ValidateSchemaV2(db).ok());
        // The journal-v1 structure is part of v2 and is still valid on its own.
        REQUIRE(bim::persistence::detail::ValidateSchemaV1(db).ok());

        // Exactly one metadata row; it names the format, never a schema version.
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM document_metadata;") == 1);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM document_metadata WHERE singleton = 1 AND "
                                "format_tag = 'bim.document';") == 1);
        // A fresh document holds no BIM state and no journal.
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM elements;") == 0);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM levels;") == 0);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM straight_walls;") == 0);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM journal_transactions;") == 0);
    }

    // It persists and reopens as a valid v2 database; reopening changes nothing.
    {
        SqliteConnection reopened;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), reopened).ok());
        REQUIRE(ScalarInt64(reopened.handle(), "PRAGMA user_version;") == 2);
        REQUIRE(UserTables(reopened.handle()) == kV2Tables);
    }
    DocumentSnapshot loaded = Sentinel();
    REQUIRE(bim::persistence::LoadDocument(db_path.path(), loaded).ok());
    REQUIRE(loaded.levels.empty());
    REQUIRE(loaded.straight_walls.empty());
}

TEST_CASE("version 0 with any pre-existing user table is rejected with zero mutation",
          "[p1_t004][schema_v2]") {
    const TempDatabasePath db_path;
    {
        SqliteConnection connection = OpenRaw(db_path.path());
        REQUIRE(Exec(connection.handle(), "CREATE TABLE stray (x INTEGER);").ok());
        REQUIRE(Exec(connection.handle(), "INSERT INTO stray (x) VALUES (7);").ok());
    }
    {
        SqliteConnection rejected;
        const bim::foundation::Status status =
            bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), rejected);
        REQUIRE_FALSE(status.ok());
        REQUIRE_FALSE(rejected.is_open());
    }
    REQUIRE(ReadUserVersionRaw(db_path.path()) == 0);
    REQUIRE(UserTablesRaw(db_path.path()) == std::vector<std::string>{"stray"});
    {
        SqliteConnection raw = OpenRaw(db_path.path());
        REQUIRE(ScalarInt64(raw.handle(), "SELECT x FROM stray;") == 7);
    }
    // The public store refuses it the same way, for load and for save.
    RequireLoadRejected(db_path.path());
    REQUIRE_FALSE(bim::persistence::SaveDocument(db_path.path(), SampleSnapshot(), {}).ok());
    REQUIRE(ReadUserVersionRaw(db_path.path()) == 0);
    REQUIRE(UserTablesRaw(db_path.path()) == std::vector<std::string>{"stray"});
}

TEST_CASE("newer and invalid versions are rejected fail-closed", "[p1_t004][schema_v2]") {
    const TempDatabasePath db_path;
    MakeSampleDatabase(db_path.path());

    for (const std::string_view set_version : {std::string_view("PRAGMA user_version = 3;"),
                                               std::string_view("PRAGMA user_version = 99;"),
                                               std::string_view("PRAGMA user_version = -1;")}) {
        Tamper(db_path.path(), {set_version});
        const std::int64_t before = ReadUserVersionRaw(db_path.path());
        {
            SqliteConnection rejected;
            REQUIRE_FALSE(
                bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), rejected).ok());
            REQUIRE_FALSE(rejected.is_open());
        }
        RequireLoadRejected(db_path.path());
        REQUIRE_FALSE(bim::persistence::SaveDocument(db_path.path(), SampleSnapshot(), {}).ok());
        // Rejection mutated nothing: same version, same tables.
        REQUIRE(ReadUserVersionRaw(db_path.path()) == before);
        REQUIRE(UserTablesRaw(db_path.path()) == kV2Tables);
    }
}

// ----------------------------------------------------------------------------
// migration
// ----------------------------------------------------------------------------

TEST_CASE("a valid schema v1 database migrates atomically to v2 and keeps every journal row",
          "[p1_t004][schema_v2][migration]") {
    const TempDatabasePath db_path;
    const std::vector<JournalTransaction> legacy = SampleJournal();
    MakeV1Database(db_path.path(), legacy);
    REQUIRE(ReadUserVersionRaw(db_path.path()) == 1);
    REQUIRE(UserTablesRaw(db_path.path()) == kV1Tables);

    {
        SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), connection).ok());
        sqlite3* db = connection.handle();
        REQUIRE(ScalarInt64(db, "PRAGMA user_version;") == 2);
        REQUIRE(UserTables(db) == kV2Tables);
        REQUIRE(bim::persistence::detail::ValidateSchemaV2(db).ok());
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM document_metadata;") == 1);
        // A v1 database held no BIM state, so the migrated BIM state is empty:
        // nothing is inferred from the journal payloads.
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM elements;") == 0);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM levels;") == 0);
        REQUIRE(ScalarInt64(db, "SELECT count(*) FROM straight_walls;") == 0);
    }

    // Every committed journal row survived, in the same order and bytes.
    REQUIRE(SameJournal(ReadJournalRaw(db_path.path()), legacy));

    DocumentSnapshot loaded = Sentinel();
    REQUIRE(bim::persistence::LoadDocument(db_path.path(), loaded).ok());
    REQUIRE(loaded.levels.empty());
    REQUIRE(loaded.straight_walls.empty());

    // A second open is a plain v2 open (no second migration).
    {
        SqliteConnection again;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), again).ok());
        REQUIRE(ScalarInt64(again.handle(), "PRAGMA user_version;") == 2);
    }
    REQUIRE(SameJournal(ReadJournalRaw(db_path.path()), legacy));
}

TEST_CASE("a migration that fails part-way rolls back completely and preserves v1 and its journal",
          "[p1_t004][schema_v2][migration]") {
    const TempDatabasePath db_path;
    const std::vector<JournalTransaction> legacy = SampleJournal();
    MakeV1Database(db_path.path(), legacy);

    // A pre-existing table named `levels` is invisible to the v1 validation, but
    // the migration creates document_metadata and elements BEFORE it reaches
    // `levels`, so the CREATE TABLE of `levels` fails after earlier writes.
    Tamper(db_path.path(), {"CREATE TABLE levels (squatter INTEGER);"});
    const std::vector<std::string> before = UserTablesRaw(db_path.path());

    {
        SqliteConnection rejected;
        REQUIRE_FALSE(
            bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), rejected).ok());
        REQUIRE_FALSE(rejected.is_open());
    }

    // Everything the failed attempt wrote was rolled back.
    REQUIRE(ReadUserVersionRaw(db_path.path()) == 1);
    REQUIRE(UserTablesRaw(db_path.path()) == before);
    {
        SqliteConnection raw = OpenRaw(db_path.path());
        REQUIRE(ScalarInt64(raw.handle(), "SELECT count(*) FROM sqlite_master WHERE "
                                          "name IN ('document_metadata','elements');") == 0);
    }
    REQUIRE(SameJournal(ReadJournalRaw(db_path.path()), legacy));

    // The database is still a valid v1 database: removing the squatter lets the
    // very same migration succeed.
    Tamper(db_path.path(), {"DROP TABLE levels;"});
    {
        SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), connection).ok());
        REQUIRE(ScalarInt64(connection.handle(), "PRAGMA user_version;") == 2);
    }
    REQUIRE(SameJournal(ReadJournalRaw(db_path.path()), legacy));
}

TEST_CASE("an invalid schema v1 database is not migrated", "[p1_t004][schema_v2][migration]") {
    const TempDatabasePath db_path;
    MakeV1Database(db_path.path(), SampleJournal());
    // Break the frozen v1 structure while keeping user_version = 1.
    Tamper(db_path.path(), {"DROP TABLE journal_entries;"});
    {
        SqliteConnection rejected;
        REQUIRE_FALSE(
            bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), rejected).ok());
    }
    REQUIRE(ReadUserVersionRaw(db_path.path()) == 1);
    REQUIRE(UserTablesRaw(db_path.path()) == std::vector<std::string>{"journal_transactions"});
}

// ----------------------------------------------------------------------------
// complete v2 validation
// ----------------------------------------------------------------------------

TEST_CASE("schema v2 validation rejects an altered, extended or incomplete schema",
          "[p1_t004][schema_v2]") {
    struct Tamperer {
        const char* name;
        std::vector<std::string_view> statements;
    };
    const std::array<Tamperer, 9> cases = {{
        {"extra user table", {"CREATE TABLE extra_table (x INTEGER);"}},
        {"a trigger", {"CREATE TRIGGER t_after AFTER INSERT ON elements BEGIN SELECT 1; END;"}},
        {"a view", {"CREATE VIEW v_levels AS SELECT * FROM levels;"}},
        {"metadata row removed", {"DELETE FROM document_metadata;"}},
        {"metadata tag changed", {"UPDATE document_metadata SET format_tag = 'something.else';"}},
        {"elements re-created differently",
         {"DROP TABLE straight_walls;", "DROP TABLE levels;", "DROP TABLE elements;",
          "CREATE TABLE elements (element_id TEXT PRIMARY KEY NOT NULL, element_type INTEGER NOT "
          "NULL);",
          "CREATE TABLE levels (element_id TEXT PRIMARY KEY NOT NULL, elevation REAL NOT NULL, "
          "FOREIGN KEY (element_id) REFERENCES elements(element_id) ON DELETE CASCADE);",
          "CREATE TABLE straight_walls (element_id TEXT PRIMARY KEY NOT NULL, level_id TEXT NOT "
          "NULL, start_x REAL NOT NULL, start_y REAL NOT NULL, end_x REAL NOT NULL, end_y REAL "
          "NOT NULL, thickness REAL NOT NULL, base_offset REAL NOT NULL, height REAL NOT NULL, "
          "FOREIGN KEY (element_id) REFERENCES elements(element_id) ON DELETE CASCADE, FOREIGN "
          "KEY (level_id) REFERENCES levels(element_id) ON DELETE RESTRICT);"}},
        {"levels column removed",
         {"DROP TABLE straight_walls;", "DROP TABLE levels;",
          "CREATE TABLE levels (element_id TEXT PRIMARY KEY NOT NULL, FOREIGN KEY (element_id) "
          "REFERENCES elements(element_id) ON DELETE CASCADE);"}},
        {"journal table removed", {"DROP TABLE journal_entries;"}},
        {"a required table missing", {"DROP TABLE straight_walls;"}},
    }};
    for (const Tamperer& tamperer : cases) {
        DYNAMIC_SECTION(tamperer.name) {
            const TempDatabasePath db_path;
            MakeSampleDatabase(db_path.path());
            Tamper(db_path.path(), tamperer.statements);
            SqliteConnection rejected;
            REQUIRE_FALSE(
                bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), rejected).ok());
            RequireLoadRejected(db_path.path());
            REQUIRE_FALSE(
                bim::persistence::SaveDocument(db_path.path(), SampleSnapshot(), {}).ok());
        }
    }
}

TEST_CASE("the schema enforces the element namespace, kinds and referential integrity",
          "[p1_t004][schema_v2]") {
    const TempDatabasePath db_path;
    SqliteConnection connection;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV2(db_path.path(), connection).ok());
    sqlite3* db = connection.handle();

    const std::string level_id = IdText(Id(0x10));
    const std::string wall_id = IdText(Id(0x30));

    // element_type is restricted to 1 (Level) and 2 (StraightWall).
    REQUIRE_FALSE(Exec(db, "INSERT INTO elements (element_id, element_type) VALUES "
                           "('11111111-1111-1111-1111-111111111111', 3);")
                      .ok());
    REQUIRE_FALSE(Exec(db, "INSERT INTO elements (element_id, element_type) VALUES "
                           "('11111111-1111-1111-1111-111111111111', 0);")
                      .ok());
    // A kind row needs its elements row (foreign key to the shared namespace).
    REQUIRE_FALSE(Exec(db, "INSERT INTO levels (element_id, elevation) VALUES "
                           "('22222222-2222-2222-2222-222222222222', 1.0);")
                      .ok());
    // One global namespace: an id appears once in elements.
    REQUIRE(Exec(db, "INSERT INTO elements (element_id, element_type) VALUES "
                     "('33333333-3333-3333-3333-333333333333', 1);")
                .ok());
    REQUIRE_FALSE(Exec(db, "INSERT INTO elements (element_id, element_type) VALUES "
                           "('33333333-3333-3333-3333-333333333333', 2);")
                      .ok());
    REQUIRE(Exec(db, "DELETE FROM elements;").ok());

    // A wall cannot reference a Level that does not exist.
    {
        SqliteStatement element;
        REQUIRE(SqliteStatement::Prepare(
                    db, "INSERT INTO elements (element_id, element_type) VALUES (?1, ?2);", element)
                    .ok());
        bool has_row = false;
        REQUIRE(element.BindText(1, level_id).ok());
        REQUIRE(element.BindInt64(2, 1).ok());
        REQUIRE(element.Step(has_row).ok());
        REQUIRE(element.Reset().ok());
        REQUIRE(element.BindText(1, wall_id).ok());
        REQUIRE(element.BindInt64(2, 2).ok());
        REQUIRE(element.Step(has_row).ok());
    }
    {
        SqliteStatement wall;
        REQUIRE(SqliteStatement::Prepare(
                    db,
                    "INSERT INTO straight_walls (element_id, level_id, start_x, start_y, end_x, "
                    "end_y, thickness, base_offset, height) VALUES (?1, ?2, 0, 0, 1, 0, 0.2, 0, "
                    "3);",
                    wall)
                    .ok());
        bool has_row = false;
        REQUIRE(wall.BindText(1, wall_id).ok());
        REQUIRE(wall.BindText(2, level_id).ok());
        // The host Level row has not been inserted yet.
        REQUIRE_FALSE(wall.Step(has_row).ok());
    }
    {
        SqliteStatement level;
        REQUIRE(SqliteStatement::Prepare(
                    db, "INSERT INTO levels (element_id, elevation) VALUES (?1, ?2);", level)
                    .ok());
        bool has_row = false;
        REQUIRE(level.BindText(1, level_id).ok());
        REQUIRE(level.BindDouble(2, 2.5).ok());
        REQUIRE(level.Step(has_row).ok());
    }
    {
        SqliteStatement wall;
        REQUIRE(SqliteStatement::Prepare(
                    db,
                    "INSERT INTO straight_walls (element_id, level_id, start_x, start_y, end_x, "
                    "end_y, thickness, base_offset, height) VALUES (?1, ?2, 0, 0, 1, 0, 0.2, 0, "
                    "3);",
                    wall)
                    .ok());
        bool has_row = false;
        REQUIRE(wall.BindText(1, wall_id).ok());
        REQUIRE(wall.BindText(2, level_id).ok());
        REQUIRE(wall.Step(has_row).ok());
    }

    // ON DELETE RESTRICT: a Level that still hosts a wall cannot be deleted.
    REQUIRE_FALSE(Exec(db, "DELETE FROM levels;").ok());
    REQUIRE(ScalarInt64(db, "SELECT count(*) FROM levels;") == 1);
    // Deleting the wall first makes the Level deletable.
    REQUIRE(Exec(db, "DELETE FROM straight_walls;").ok());
    REQUIRE(Exec(db, "DELETE FROM levels;").ok());
    // ON DELETE CASCADE: removing an elements row removes its kind row.
    REQUIRE(Exec(db, "DELETE FROM elements;").ok());
    REQUIRE(ScalarInt64(db, "SELECT count(*) FROM elements;") == 0);

    // The metadata table holds only the singleton.
    REQUIRE_FALSE(
        Exec(db, "INSERT INTO document_metadata (singleton, format_tag) VALUES (2, 'x');").ok());
    REQUIRE_FALSE(
        Exec(db, "INSERT INTO document_metadata (singleton, format_tag) VALUES (1, 'x');").ok());
}

// ----------------------------------------------------------------------------
// SQLite statement numeric primitives
// ----------------------------------------------------------------------------

TEST_CASE("BindDouble and ColumnDouble round-trip IEEE doubles and keep NaN out",
          "[p1_t004][schema_v2][statement]") {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenInMemory(connection).ok());
    sqlite3* db = connection.handle();
    REQUIRE(Exec(db, "CREATE TABLE samples (idx INTEGER NOT NULL, v REAL NOT NULL);").ok());

    const std::vector<double> samples = {0.1,
                                         1.0 / 3.0,
                                         3.0,
                                         -7.0,
                                         123456.789012345,
                                         1.0e-300,
                                         std::numeric_limits<double>::denorm_min(),
                                         std::numeric_limits<double>::min(),
                                         std::numeric_limits<double>::max(),
                                         std::numeric_limits<double>::lowest(),
                                         std::numeric_limits<double>::epsilon()};
    {
        SqliteStatement insert;
        REQUIRE(
            SqliteStatement::Prepare(db, "INSERT INTO samples (idx, v) VALUES (?1, ?2);", insert)
                .ok());
        for (std::size_t i = 0; i < samples.size(); ++i) {
            bool has_row = false;
            REQUIRE(insert.BindInt64(1, static_cast<std::int64_t>(i)).ok());
            REQUIRE(insert.BindDouble(2, samples[i]).ok());
            REQUIRE(insert.Step(has_row).ok());
            REQUIRE(insert.Reset().ok());
        }
    }
    {
        SqliteStatement select;
        REQUIRE(SqliteStatement::Prepare(db, "SELECT idx, v FROM samples ORDER BY idx ASC;", select)
                    .ok());
        for (std::size_t i = 0; i < samples.size(); ++i) {
            bool has_row = false;
            REQUIRE(select.Step(has_row).ok());
            REQUIRE(has_row);
            const double read = select.ColumnDouble(1);
            // Bit-exact, not merely approximately equal.
            REQUIRE(std::memcmp(&read, &samples[i], sizeof(double)) == 0);
        }
    }
    // A NaN is stored as NULL by SQLite and therefore violates NOT NULL: a NaN
    // can never be persisted silently.
    {
        SqliteStatement insert;
        REQUIRE(
            SqliteStatement::Prepare(db, "INSERT INTO samples (idx, v) VALUES (99, ?1);", insert)
                .ok());
        REQUIRE(insert.BindDouble(1, std::numeric_limits<double>::quiet_NaN()).ok());
        bool has_row = false;
        REQUIRE_FALSE(insert.Step(has_row).ok());
    }
    // The pre-existing text / blob / int64 primitives are unchanged.
    {
        SqliteStatement statement;
        REQUIRE(SqliteStatement::Prepare(db, "SELECT ?1, ?2, ?3;", statement).ok());
        REQUIRE(statement.BindText(1, "text value").ok());
        REQUIRE(statement.BindBlob(2, Bytes({0x00, 0xFF, 0x10})).ok());
        REQUIRE(statement.BindInt64(3, -1234567890123LL).ok());
        bool has_row = false;
        REQUIRE(statement.Step(has_row).ok());
        REQUIRE(has_row);
        REQUIRE(statement.ColumnText(0) == "text value");
        REQUIRE(statement.ColumnBlob(1) == Bytes({0x00, 0xFF, 0x10}));
        REQUIRE(statement.ColumnInt64(2) == -1234567890123LL);
    }
}

// ----------------------------------------------------------------------------
// durable-content validation on load
// ----------------------------------------------------------------------------

TEST_CASE("a saved snapshot loads back exactly, in ElementId order, with canonical lowercase ids",
          "[p1_t004][schema_v2]") {
    const TempDatabasePath db_path;
    MakeSampleDatabase(db_path.path());

    // The durable ids are the 36-character canonical lowercase text.
    {
        SqliteConnection raw = OpenRaw(db_path.path());
        REQUIRE(ScalarInt64(raw.handle(), "SELECT count(*) FROM elements;") == 4);
        REQUIRE(ScalarInt64(raw.handle(),
                            "SELECT count(*) FROM elements WHERE length(element_id) = 36 AND "
                            "element_id = lower(element_id);") == 4);
    }

    DocumentSnapshot loaded;
    REQUIRE(bim::persistence::LoadDocument(db_path.path(), loaded).ok());
    REQUIRE(loaded.levels.size() == 2);
    REQUIRE(loaded.levels[0] == MakeLevel(Id(0x10), 3.5));
    REQUIRE(loaded.levels[1] == MakeLevel(Id(0x20), 0.0));
    REQUIRE(loaded.straight_walls.size() == 2);
    REQUIRE(loaded.straight_walls[0] == MakeWall(Id(0x30), Id(0x20)));
    REQUIRE(loaded.straight_walls[1] == MakeWall(Id(0x40), Id(0x10)));
}

TEST_CASE("load rejects non-canonical, malformed and all-zero persisted ElementIds",
          "[p1_t004][schema_v2][validation]") {
    const std::string level = IdText(Id(0x10));
    // Upper-case hex digits: a valid id that would re-serialize to different
    // (lower-case) text, so it is non-canonical and must be rejected, not normalized.
    const std::string upper = "AB000010-0000-0000-0000-0000000000CD";
    REQUIRE(upper != level);

    struct Case {
        const char* name;
        std::string replacement;
    };
    const std::vector<Case> cases = {
        {"upper-case hex digits", upper},
        {"all-zero identity", "00000000-0000-0000-0000-000000000000"},
        {"too short", "00000010-0000-0000-0000-00000000001"},
        {"missing hyphens", "00000010000000000000000000000001"},
        {"non-hex character", "0000001g-0000-0000-0000-000000000001"},
        {"surrounding whitespace", " " + level},
        {"empty text", ""},
    };
    for (const Case& test_case : cases) {
        DYNAMIC_SECTION(test_case.name) {
            const TempDatabasePath db_path;
            MakeSampleDatabase(db_path.path());
            // Rewrite the same id consistently in every table that holds it, so the
            // ONLY defect is the id text itself.
            const std::string quoted = "'" + test_case.replacement + "'";
            const std::string old_id = "'" + level + "'";
            const std::string s1 = "UPDATE elements SET element_id = " + quoted +
                                   " WHERE element_id = " + old_id + ";";
            const std::string s2 =
                "UPDATE levels SET element_id = " + quoted + " WHERE element_id = " + old_id + ";";
            const std::string s3 = "UPDATE straight_walls SET level_id = " + quoted +
                                   " WHERE level_id = " + old_id + ";";
            Tamper(db_path.path(), {s1, s2, s3});
            RequireLoadRejected(db_path.path());
        }
    }
}

TEST_CASE("load rejects inconsistent kind records, orphans, missing hosts and invalid values",
          "[p1_t004][schema_v2][validation]") {
    const std::string level_a = "'" + IdText(Id(0x10)) + "'"; // hosts wall 0x40
    const std::string level_b = "'" + IdText(Id(0x20)) + "'"; // hosts wall 0x30
    const std::string wall_a = "'" + IdText(Id(0x40)) + "'";
    const std::string new_id = "'" + IdText(Id(0x55)) + "'";

    struct Case {
        const char* name;
        std::vector<std::string> statements;
    };
    const std::vector<Case> cases = {
        {"element type disagrees with the kind table (Level stored as wall)",
         {"UPDATE elements SET element_type = 2 WHERE element_id = " + level_a + ";"}},
        {"element type disagrees with the kind table (wall stored as Level)",
         {"UPDATE elements SET element_type = 1 WHERE element_id = " + wall_a + ";"}},
        {"orphan element record",
         {"INSERT INTO elements (element_id, element_type) VALUES (" + new_id + ", 1);"}},
        {"kind row without an element record",
         {"DELETE FROM elements WHERE element_id = " + wall_a + ";"}},
        {"wall whose hosting Level is gone",
         {"DELETE FROM straight_walls WHERE element_id = " + wall_a + ";",
          "INSERT INTO elements (element_id, element_type) VALUES (" + new_id + ", 2);",
          "INSERT INTO straight_walls (element_id, level_id, start_x, start_y, end_x, end_y, "
          "thickness, base_offset, height) VALUES (" +
              new_id + ", '" + IdText(Id(0x77)) + "', 0, 0, 1, 0, 0.2, 0, 3);"}},
        {"Level that is also a wall id (cross-kind duplicate)",
         {"INSERT INTO straight_walls (element_id, level_id, start_x, start_y, end_x, end_y, "
          "thickness, base_offset, height) VALUES (" +
          level_a + ", " + level_b + ", 0, 0, 1, 0, 0.2, 0, 3);"}},
        {"non-positive thickness",
         {"UPDATE straight_walls SET thickness = 0.0 WHERE element_id = " + wall_a + ";"}},
        {"coincident endpoints",
         {"UPDATE straight_walls SET end_x = start_x, end_y = start_y WHERE element_id = " +
          wall_a + ";"}},
        {"non-finite coordinate (infinity)",
         {"UPDATE straight_walls SET start_x = 9e999 WHERE element_id = " + wall_a + ";"}},
        {"non-finite Level elevation (infinity)",
         {"UPDATE levels SET elevation = 9e999 WHERE element_id = " + level_a + ";"}},
        {"wall that names itself as its host",
         {"UPDATE straight_walls SET level_id = element_id WHERE element_id = " + wall_a + ";"}},
        {"text stored where a REAL is expected (Level)",
         {"UPDATE levels SET elevation = 'not a number' WHERE element_id = " + level_a + ";"}},
        {"text stored where a REAL is expected (wall)",
         {"UPDATE straight_walls SET height = 'tall' WHERE element_id = " + wall_a + ";"}},
        {"integer stored where text is expected",
         {"UPDATE straight_walls SET level_id = 5 WHERE element_id = " + wall_a + ";"}},
        {"journal transaction without any entry (malformed journal state)",
         {"INSERT INTO journal_transactions (transaction_id) VALUES ('entryless');"}},
    };
    for (const Case& test_case : cases) {
        DYNAMIC_SECTION(test_case.name) {
            const TempDatabasePath db_path;
            MakeSampleDatabase(db_path.path());
            {
                SqliteConnection connection = OpenRaw(db_path.path());
                REQUIRE(Exec(connection.handle(), "PRAGMA foreign_keys = OFF;").ok());
                for (const std::string& sql : test_case.statements) {
                    INFO(sql);
                    REQUIRE(Exec(connection.handle(), sql).ok());
                }
            }
            RequireLoadRejected(db_path.path());
        }
    }
}

TEST_CASE("load reports a missing or non-database file without creating or altering anything",
          "[p1_t004][schema_v2][validation]") {
    const TempDatabasePath db_path;
    {
        DocumentSnapshot out = Sentinel();
        REQUIRE_FALSE(bim::persistence::LoadDocument(db_path.path(), out).ok());
        REQUIRE(out.levels.size() == 1);
        // A load never creates the file.
        REQUIRE_FALSE(std::filesystem::exists(db_path.path()));
    }
    {
        // A directory is not a database either.
        DocumentSnapshot out = Sentinel();
        REQUIRE_FALSE(
            bim::persistence::LoadDocument(std::filesystem::temp_directory_path(), out).ok());
        REQUIRE(out.levels.size() == 1);
    }
    {
        // Arbitrary bytes are not a database; the file is left byte-identical.
        const std::string garbage = "this is definitely not an SQLite database file\n";
        {
            std::ofstream file(db_path.path(), std::ios::binary);
            file << garbage;
        }
        RequireLoadRejected(db_path.path());
        std::ifstream reread(db_path.path(), std::ios::binary);
        std::ostringstream content;
        content << reread.rdbuf();
        REQUIRE(content.str() == garbage);
    }
}
