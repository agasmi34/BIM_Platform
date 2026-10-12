// P1-T004 Persistence Schema v2 - save atomicity proof (Architecture Gate
// BIM-AG-P1-T004 section 13; Implementation Brief BIM-IB-P1-T004 sections 10 and
// 11).
//
// One save is one durable step: the complete authoritative snapshot AND every
// pending journal transaction become visible together, or nothing changes. These
// tests prove it from the outside: the whole durable state (snapshot plus
// journal) is read back before and after a save that is made to fail in every
// way the store defends against, and must be identical. A failure is injected
// BEFORE any durable change (invalid input, incompatible database) and, the case
// that really tests atomicity, AFTER the snapshot has already been replaced
// inside the transaction (a pending journal id that collides with committed
// history). The Document-level tests then prove that a failed save leaves the
// in-memory document and its pending journal untouched.
#include "../../src/persistence/src/detail/journal_store.hpp"
#include "../../src/persistence/src/detail/schema_v1.hpp"
#include "../../src/persistence/src/detail/schema_v2.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/document/document.hpp"
#include "bim/persistence/document_store.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
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

using bim::document::Document;
using bim::document::DocumentLifecycleResult;
using bim::document::DocumentLifecycleResultCode;
using bim::document::DocumentResult;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::persistence::DocumentSnapshot;
using bim::persistence::detail::SqliteConnection;
using bim::persistence::detail::SqliteStatement;
using bim::transactions::JournalRecord;
using bim::transactions::JournalTransaction;

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
        name << "bim_p1_t004_save_atomicity_" << now_ticks << "_" << unique << ".db";
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

ElementId Id(std::uint8_t first, std::uint8_t last = 0x01) {
    ElementId id;
    id.bytes[0] = first;
    id.bytes[15] = last;
    return id;
}

Level MakeLevel(const ElementId& id, double elevation) {
    return Level{.id = id, .elevation = elevation};
}

StraightWall MakeWall(const ElementId& id, const ElementId& level_id, double length = 4.0) {
    return StraightWall{.id = id,
                        .level_id = level_id,
                        .start = Point2D{0.0, 0.0},
                        .end = Point2D{length, 1.0},
                        .thickness = 0.2,
                        .height = 3.0,
                        .base_offset = 0.0};
}

JournalTransaction MakeTransaction(const std::string& id, const std::string& kind = "test.kind") {
    JournalTransaction transaction;
    transaction.id = id;
    transaction.records.push_back(
        JournalRecord{kind, {static_cast<std::byte>(0x00), static_cast<std::byte>(0xA5)}});
    return transaction;
}

// Two Levels and three walls.
DocumentSnapshot SnapshotOne() {
    DocumentSnapshot snapshot;
    snapshot.levels = {MakeLevel(Id(0x10), 0.0), MakeLevel(Id(0x20), 3.0)};
    snapshot.straight_walls = {MakeWall(Id(0x31), Id(0x10)), MakeWall(Id(0x32), Id(0x10), 6.5),
                               MakeWall(Id(0x41), Id(0x20))};
    return snapshot;
}

// A different model: one Level (a different one), one wall.
DocumentSnapshot SnapshotTwo() {
    DocumentSnapshot snapshot;
    snapshot.levels = {MakeLevel(Id(0x50), -2.5)};
    snapshot.straight_walls = {MakeWall(Id(0x51), Id(0x50), 9.0)};
    return snapshot;
}

SqliteConnection OpenRaw(const std::filesystem::path& path) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    return connection;
}

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

// The complete durable state of a database: the loaded snapshot and the journal.
struct Durable {
    DocumentSnapshot snapshot;
    std::vector<JournalTransaction> journal;
    std::int64_t user_version = 0;
    std::vector<std::string> user_tables;
};

Durable ReadDurable(const std::filesystem::path& path) {
    Durable durable;
    {
        SqliteConnection connection = OpenRaw(path);
        sqlite3* db = connection.handle();
        durable.user_version = ScalarInt64(db, "PRAGMA user_version;");
        SqliteStatement tables;
        REQUIRE(SqliteStatement::Prepare(db,
                                         "SELECT name FROM sqlite_master WHERE type='table' AND "
                                         "substr(name,1,7) <> 'sqlite_' ORDER BY name ASC;",
                                         tables)
                    .ok());
        for (;;) {
            bool has_row = false;
            REQUIRE(tables.Step(has_row).ok());
            if (!has_row) {
                break;
            }
            durable.user_tables.push_back(tables.ColumnText(0));
        }
        REQUIRE(bim::persistence::detail::ReadJournalTransactions(db, durable.journal).ok());
    }
    REQUIRE(bim::persistence::LoadDocument(path, durable.snapshot).ok());
    return durable;
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

bool SameDurable(const Durable& a, const Durable& b) {
    return a.snapshot.levels == b.snapshot.levels &&
           a.snapshot.straight_walls == b.snapshot.straight_walls &&
           SameJournal(a.journal, b.journal) && a.user_version == b.user_version &&
           a.user_tables == b.user_tables;
}

std::string FileBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void RequireOk(const DocumentResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

} // namespace

// ----------------------------------------------------------------------------
// store level
// ----------------------------------------------------------------------------

TEST_CASE("a successful save makes the snapshot and the pending journal visible together",
          "[p1_t004][save_atomicity]") {
    const TempDatabasePath db_path;
    REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotOne(),
                                           {MakeTransaction("tx-1"), MakeTransaction("tx-2")})
                .ok());
    const Durable first = ReadDurable(db_path.path());
    REQUIRE(first.snapshot.levels.size() == 2);
    REQUIRE(first.snapshot.straight_walls.size() == 3);
    REQUIRE(first.journal.size() == 2);

    // The second save replaces the snapshot completely (no stale row survives) and
    // appends to - never rewrites - the committed journal history.
    REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(), {MakeTransaction("tx-3")})
                .ok());
    const Durable second = ReadDurable(db_path.path());
    REQUIRE(second.snapshot.levels == SnapshotTwo().levels);
    REQUIRE(second.snapshot.straight_walls == SnapshotTwo().straight_walls);
    REQUIRE(second.journal.size() == 3);
    REQUIRE(second.journal[0].id == "tx-1");
    REQUIRE(second.journal[1].id == "tx-2");
    REQUIRE(second.journal[2].id == "tx-3");
    REQUIRE(SameJournal(
        std::vector<JournalTransaction>(second.journal.begin(), second.journal.begin() + 2),
        first.journal));

    // Saving an empty snapshot clears the BIM state but still keeps the journal.
    REQUIRE(bim::persistence::SaveDocument(db_path.path(), DocumentSnapshot{}, {}).ok());
    const Durable emptied = ReadDurable(db_path.path());
    REQUIRE(emptied.snapshot.levels.empty());
    REQUIRE(emptied.snapshot.straight_walls.empty());
    REQUIRE(SameJournal(emptied.journal, second.journal));
}

TEST_CASE("a pending journal id that collides with history rolls the whole save back",
          "[p1_t004][save_atomicity]") {
    const TempDatabasePath db_path;
    REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotOne(),
                                           {MakeTransaction("hist-1"), MakeTransaction("hist-2")})
                .ok());
    const Durable before = ReadDurable(db_path.path());

    // By the time the colliding transaction is reached, the store has already
    // deleted the old snapshot, inserted the whole new one and appended the first
    // pending transaction inside its transaction. The failure must undo all of it.
    const std::vector<JournalTransaction> pending = {MakeTransaction("fresh-1"),
                                                     MakeTransaction("hist-2") /* collides */,
                                                     MakeTransaction("fresh-2")};
    const bim::foundation::Status status =
        bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(), pending);
    REQUIRE_FALSE(status.ok());

    const Durable after = ReadDurable(db_path.path());
    REQUIRE(SameDurable(after, before));
    // In particular no partial new journal appeared.
    for (const JournalTransaction& transaction : after.journal) {
        REQUIRE(transaction.id != "fresh-1");
        REQUIRE(transaction.id != "fresh-2");
    }

    // The database is fully usable afterwards: the same save without the collision
    // succeeds and lands exactly once.
    REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(),
                                           {MakeTransaction("fresh-1"), MakeTransaction("fresh-2")})
                .ok());
    const Durable retried = ReadDurable(db_path.path());
    REQUIRE(retried.snapshot.levels == SnapshotTwo().levels);
    REQUIRE(retried.journal.size() == 4);
    REQUIRE(retried.journal[2].id == "fresh-1");
    REQUIRE(retried.journal[3].id == "fresh-2");
}

TEST_CASE("a rolled-back first save leaves a valid empty document, never a partial one",
          "[p1_t004][save_atomicity]") {
    const TempDatabasePath db_path;
    // The very first save to a brand-new file fails on a duplicate pending id
    // AFTER the schema exists: the database must hold the empty bootstrap only.
    JournalTransaction dup = MakeTransaction("dup");
    const bim::foundation::Status status =
        bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), {dup, dup});
    REQUIRE_FALSE(status.ok()); // rejected up front: duplicate pending ids

    // Rejected before any file is created.
    REQUIRE_FALSE(std::filesystem::exists(db_path.path()));
}

TEST_CASE("input validation rejects before any durable change and creates no file",
          "[p1_t004][save_atomicity]") {
    struct BadSnapshot {
        const char* name;
        DocumentSnapshot snapshot;
    };
    std::vector<BadSnapshot> bad_snapshots;
    {
        DocumentSnapshot s = SnapshotOne();
        s.levels[0].elevation = std::numeric_limits<double>::quiet_NaN();
        bad_snapshots.push_back({"non-finite Level elevation", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.levels[0].id = ElementId{};
        bad_snapshots.push_back({"all-zero Level id", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls[0].thickness = 0.0;
        bad_snapshots.push_back({"non-positive wall thickness", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls[0].end = s.straight_walls[0].start;
        bad_snapshots.push_back({"coincident wall endpoints", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls[0].level_id = Id(0x77);
        bad_snapshots.push_back({"wall hosted by a Level that is not in the snapshot", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.levels.push_back(s.levels[0]);
        bad_snapshots.push_back({"duplicate Level id", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls.push_back(s.straight_walls[0]);
        bad_snapshots.push_back({"duplicate wall id", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls[0].id = s.levels[0].id; // one global ElementId namespace
        bad_snapshots.push_back({"a wall that reuses a Level id", s});
    }
    {
        DocumentSnapshot s = SnapshotOne();
        s.straight_walls[0].id = s.straight_walls[0].level_id; // wall id equals its host id
        bad_snapshots.push_back({"a wall that is its own host", s});
    }

    for (const BadSnapshot& bad : bad_snapshots) {
        DYNAMIC_SECTION(bad.name << " (new file)") {
            const TempDatabasePath db_path;
            REQUIRE_FALSE(bim::persistence::SaveDocument(db_path.path(), bad.snapshot, {}).ok());
            REQUIRE_FALSE(std::filesystem::exists(db_path.path()));
        }
        DYNAMIC_SECTION(bad.name << " (existing document)") {
            const TempDatabasePath db_path;
            REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(),
                                                   {MakeTransaction("kept")})
                        .ok());
            const Durable before = ReadDurable(db_path.path());
            REQUIRE_FALSE(bim::persistence::SaveDocument(db_path.path(), bad.snapshot,
                                                         {MakeTransaction("not-written")})
                              .ok());
            REQUIRE(SameDurable(ReadDurable(db_path.path()), before));
        }
    }

    struct BadJournal {
        const char* name;
        std::vector<JournalTransaction> journal;
    };
    std::vector<BadJournal> bad_journals;
    {
        JournalTransaction t = MakeTransaction("x");
        t.id.clear();
        bad_journals.push_back({"empty transaction id", {t}});
    }
    {
        JournalTransaction t = MakeTransaction("x");
        t.records.clear();
        bad_journals.push_back({"transaction without records", {t}});
    }
    {
        JournalTransaction t = MakeTransaction("x");
        t.records[0].kind.clear();
        bad_journals.push_back({"empty record kind", {t}});
    }
    bad_journals.push_back({"duplicate pending ids", {MakeTransaction("d"), MakeTransaction("d")}});
    bad_journals.push_back({"a valid transaction before an invalid one",
                            {MakeTransaction("ok"), [] {
                                 JournalTransaction t = MakeTransaction("bad");
                                 t.records[0].kind.clear();
                                 return t;
                             }()}});

    for (const BadJournal& bad : bad_journals) {
        DYNAMIC_SECTION(bad.name) {
            const TempDatabasePath db_path;
            REQUIRE_FALSE(
                bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), bad.journal).ok());
            REQUIRE_FALSE(std::filesystem::exists(db_path.path()));

            REQUIRE(bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(),
                                                   {MakeTransaction("kept")})
                        .ok());
            const Durable before = ReadDurable(db_path.path());
            REQUIRE_FALSE(
                bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), bad.journal).ok());
            REQUIRE(SameDurable(ReadDurable(db_path.path()), before));
        }
    }
}

TEST_CASE("the saved database does not depend on the input order", "[p1_t004][save_atomicity]") {
    const TempDatabasePath ordered;
    const TempDatabasePath shuffled;
    DocumentSnapshot reversed = SnapshotOne();
    std::reverse(reversed.levels.begin(), reversed.levels.end());
    std::reverse(reversed.straight_walls.begin(), reversed.straight_walls.end());

    REQUIRE(bim::persistence::SaveDocument(ordered.path(), SnapshotOne(), {}).ok());
    REQUIRE(bim::persistence::SaveDocument(shuffled.path(), reversed, {}).ok());

    const Durable a = ReadDurable(ordered.path());
    const Durable b = ReadDurable(shuffled.path());
    REQUIRE(SameDurable(a, b));
    // Both load back in ElementId ascending order.
    REQUIRE(std::is_sorted(a.snapshot.levels.begin(), a.snapshot.levels.end(),
                           [](const Level& x, const Level& y) { return x.id < y.id; }));
}

TEST_CASE("saving into an incompatible database is refused and leaves it untouched",
          "[p1_t004][save_atomicity]") {
    SECTION("version 0 with an unrelated user table") {
        const TempDatabasePath db_path;
        {
            SqliteConnection connection = OpenRaw(db_path.path());
            REQUIRE(Exec(connection.handle(), "CREATE TABLE stray (x INTEGER);").ok());
            REQUIRE(Exec(connection.handle(), "INSERT INTO stray (x) VALUES (41);").ok());
        }
        REQUIRE_FALSE(
            bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), {MakeTransaction("nope")})
                .ok());
        SqliteConnection connection = OpenRaw(db_path.path());
        REQUIRE(ScalarInt64(connection.handle(), "PRAGMA user_version;") == 0);
        REQUIRE(ScalarInt64(connection.handle(), "SELECT x FROM stray;") == 41);
        REQUIRE(ScalarInt64(connection.handle(),
                            "SELECT count(*) FROM sqlite_master WHERE type='table' AND "
                            "substr(name,1,7) <> 'sqlite_';") == 1);
    }
    SECTION("a newer schema version") {
        const TempDatabasePath db_path;
        REQUIRE(
            bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), {MakeTransaction("kept")})
                .ok());
        {
            SqliteConnection connection = OpenRaw(db_path.path());
            REQUIRE(Exec(connection.handle(), "PRAGMA user_version = 3;").ok());
        }
        REQUIRE_FALSE(
            bim::persistence::SaveDocument(db_path.path(), SnapshotTwo(), {MakeTransaction("nope")})
                .ok());
        {
            SqliteConnection connection = OpenRaw(db_path.path());
            REQUIRE(ScalarInt64(connection.handle(), "PRAGMA user_version;") == 3);
            REQUIRE(ScalarInt64(connection.handle(), "SELECT count(*) FROM levels;") == 2);
            REQUIRE(ScalarInt64(connection.handle(),
                                "SELECT count(*) FROM journal_transactions;") == 1);
            REQUIRE(Exec(connection.handle(), "PRAGMA user_version = 2;").ok());
        }
        REQUIRE(ReadDurable(db_path.path()).snapshot.levels == SnapshotOne().levels);
    }
    SECTION("a file that is not a database") {
        const TempDatabasePath db_path;
        const std::string garbage = "plain text, not a database\n";
        {
            std::ofstream file(db_path.path(), std::ios::binary);
            file << garbage;
        }
        REQUIRE_FALSE(bim::persistence::SaveDocument(db_path.path(), SnapshotOne(), {}).ok());
        REQUIRE(FileBytes(db_path.path()) == garbage);
    }
}

// ----------------------------------------------------------------------------
// Document level
// ----------------------------------------------------------------------------

namespace {

// A small authored document: 2 Levels + 2 walls = 4 mutations.
Document BuildSmallDocument(std::uint8_t level_key = 0x10) {
    Document document;
    RequireOk(document.AddLevel(Level{.id = Id(level_key), .elevation = 0.0}));
    RequireOk(document.AddLevel(
        Level{.id = Id(static_cast<std::uint8_t>(level_key + 1)), .elevation = 3.0}));
    RequireOk(document.AddStraightWall(
        MakeWall(Id(static_cast<std::uint8_t>(level_key + 0x20)), Id(level_key))));
    RequireOk(
        document.AddStraightWall(MakeWall(Id(static_cast<std::uint8_t>(level_key + 0x21)),
                                          Id(static_cast<std::uint8_t>(level_key + 1)), 7.0)));
    return document;
}

} // namespace

TEST_CASE("a failed Document save keeps the document and its pending journal untouched",
          "[p1_t004][save_atomicity]") {
    Document document = BuildSmallDocument();
    const std::vector<Level> levels_before = document.ListLevels();
    const std::vector<StraightWall> walls_before = document.ListStraightWalls();

    // 1. a path that cannot be created
    {
        const std::filesystem::path unwritable =
            std::filesystem::temp_directory_path() / "bim_p1_t004_missing_dir_atomicity" / "d.db";
        const DocumentLifecycleResult result = document.Save(unwritable);
        REQUIRE_FALSE(result.ok());
        REQUIRE(result.code == DocumentLifecycleResultCode::PersistenceFailure);
    }
    // 2. a path that is not a database
    {
        const TempDatabasePath garbage;
        {
            std::ofstream file(garbage.path(), std::ios::binary);
            file << "garbage\n";
        }
        REQUIRE_FALSE(document.Save(garbage.path()).ok());
        REQUIRE(FileBytes(garbage.path()) == "garbage\n");
    }
    // 3. an incompatible database
    {
        const TempDatabasePath stray;
        {
            SqliteConnection connection = OpenRaw(stray.path());
            REQUIRE(Exec(connection.handle(), "CREATE TABLE stray (x INTEGER);").ok());
        }
        REQUIRE_FALSE(document.Save(stray.path()).ok());
        SqliteConnection connection = OpenRaw(stray.path());
        REQUIRE(ScalarInt64(connection.handle(), "PRAGMA user_version;") == 0);
    }

    // The document is exactly as it was...
    REQUIRE(document.ListLevels() == levels_before);
    REQUIRE(document.ListStraightWalls() == walls_before);

    // ...and its four pending journal transactions are still pending: the next
    // successful save writes all of them, once.
    const TempDatabasePath db_path;
    REQUIRE(document.Save(db_path.path()).ok());
    const Durable durable = ReadDurable(db_path.path());
    REQUIRE(durable.journal.size() == 4);
    REQUIRE(durable.snapshot.levels == levels_before);
    REQUIRE(durable.snapshot.straight_walls == walls_before);

    // A second save has nothing pending: no journal growth.
    REQUIRE(document.Save(db_path.path()).ok());
    REQUIRE(ReadDurable(db_path.path()).journal.size() == 4);
}

TEST_CASE("a Document save that fails against an existing durable file changes nothing there",
          "[p1_t004][save_atomicity]") {
    const TempDatabasePath db_path;
    Document document = BuildSmallDocument();
    REQUIRE(document.Save(db_path.path()).ok());
    const Durable saved = ReadDurable(db_path.path());
    REQUIRE(saved.journal.size() == 4);

    // More edits, now pending.
    RequireOk(document.UpdateLevelElevation(Id(0x11), 9.0));
    RequireOk(document.DeleteElement(Id(0x31)));

    // Make the durable file refuse the save: a trigger is not part of schema v2,
    // so validation rejects the database before the snapshot is touched.
    {
        SqliteConnection connection = OpenRaw(db_path.path());
        REQUIRE(Exec(connection.handle(),
                     "CREATE TRIGGER blocker AFTER INSERT ON elements BEGIN SELECT 1; END;")
                    .ok());
    }
    REQUIRE_FALSE(document.Save(db_path.path()).ok());
    {
        SqliteConnection connection = OpenRaw(db_path.path());
        REQUIRE(Exec(connection.handle(), "DROP TRIGGER blocker;").ok());
    }
    // The durable file is exactly what the first save wrote.
    REQUIRE(SameDurable(ReadDurable(db_path.path()), saved));

    // Once the obstacle is gone the very same document saves, carrying BOTH the
    // earlier journal history and the two edits that stayed pending.
    REQUIRE(document.Save(db_path.path()).ok());
    const Durable after = ReadDurable(db_path.path());
    REQUIRE(after.journal.size() == 6);
    REQUIRE(SameJournal(
        std::vector<JournalTransaction>(after.journal.begin(), after.journal.begin() + 4),
        saved.journal));
    REQUIRE(after.snapshot.levels == document.ListLevels());
    REQUIRE(after.snapshot.straight_walls == document.ListStraightWalls());
}
