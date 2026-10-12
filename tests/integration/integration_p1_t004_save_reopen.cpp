// P1-T004 Persistence Schema v2 - Document save / reopen proof (Architecture Gate
// BIM-AG-P1-T004 sections 6, 10 and 11; Implementation Brief BIM-IB-P1-T004
// sections 4 and 13).
//
// A model is authored through the public Document API, saved, and reopened into
// a second Document. Everything observable about the reopened document - Level
// and StraightWall values, ElementId order, regenerated derived wall geometry
// and the behaviour of later edits (which proves the Level -> StraightWall
// dependencies were rebuilt) - is compared with the original. Runtime node
// identities are private, are never persisted and are not expected to survive a
// reopen, so nothing here asks for them. The private detail headers are used
// only to inspect or damage the durable file, never to drive the Document.
#include "../../src/persistence/src/detail/journal_store.hpp"
#include "../../src/persistence/src/detail/schema_v1.hpp"
#include "../../src/persistence/src/detail/schema_v2.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/document/document.hpp"
#include "bim/persistence/document_store.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using bim::document::Document;
using bim::document::DocumentLifecycleResult;
using bim::document::DocumentLifecycleResultCode;
using bim::document::DocumentResult;
using bim::document::DocumentResultCode;
using bim::geometry_api::LinearExtrusionSpec;
using bim::model::ElementId;
using bim::model::GenerateElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::persistence::detail::SqliteConnection;
using bim::persistence::detail::SqliteStatement;

// The lifecycle surface is a path and a result code: it never throws, and its
// result is as cheap and allocation-free as the mutation result.
static_assert(
    noexcept(std::declval<Document&>().Save(std::declval<const std::filesystem::path&>())));
static_assert(noexcept(Document::Open(std::declval<const std::filesystem::path&>(),
                                      std::declval<Document&>())));
static_assert(std::is_trivially_copyable_v<DocumentLifecycleResult>);
static_assert(
    std::is_same_v<decltype(DocumentLifecycleResult{}.code), DocumentLifecycleResultCode>);

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
        name << "bim_p1_t004_save_reopen_" << now_ticks << "_" << unique << ".db";
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

ElementId NewId() {
    const std::optional<ElementId> id = GenerateElementId();
    REQUIRE(id.has_value());
    return *id;
}

StraightWall MakeWall(const ElementId& id, const ElementId& level_id, Point2D start, Point2D end,
                      double thickness = 0.25, double height = 3.0, double base_offset = 0.0) {
    return StraightWall{.id = id,
                        .level_id = level_id,
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

void RequireOk(const DocumentResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

void RequireLifecycleOk(const DocumentLifecycleResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

bool SpecEqual(const LinearExtrusionSpec& a, const LinearExtrusionSpec& b) {
    return a.profile.origin.x == b.profile.origin.x && a.profile.origin.y == b.profile.origin.y &&
           a.profile.origin.z == b.profile.origin.z && a.profile.u_axis.x == b.profile.u_axis.x &&
           a.profile.u_axis.y == b.profile.u_axis.y && a.profile.u_axis.z == b.profile.u_axis.z &&
           a.profile.v_axis.x == b.profile.v_axis.x && a.profile.v_axis.y == b.profile.v_axis.y &&
           a.profile.v_axis.z == b.profile.v_axis.z && a.profile.size_u == b.profile.size_u &&
           a.profile.size_v == b.profile.size_v && a.direction.x == b.direction.x &&
           a.direction.y == b.direction.y && a.direction.z == b.direction.z &&
           a.distance == b.distance;
}

// Everything observable about a document through its public reads.
struct Observed {
    std::vector<Level> levels;
    std::vector<StraightWall> walls;
    std::vector<std::optional<LinearExtrusionSpec>> geometry; // one per wall, same order
};

Observed Observe(const Document& document) {
    Observed observed;
    observed.levels = document.ListLevels();
    observed.walls = document.ListStraightWalls();
    for (const StraightWall& wall : observed.walls) {
        observed.geometry.push_back(document.FindWallGeometry(wall.id));
    }
    return observed;
}

bool SameObserved(const Observed& a, const Observed& b) {
    if (a.levels != b.levels || a.walls != b.walls || a.geometry.size() != b.geometry.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.geometry.size(); ++i) {
        if (a.geometry[i].has_value() != b.geometry[i].has_value()) {
            return false;
        }
        if (a.geometry[i].has_value() && !SpecEqual(*a.geometry[i], *b.geometry[i])) {
            return false;
        }
    }
    return true;
}

// Two Levels and four walls, with an elevation edit, a geometry edit and a
// deletion in the history so the saved state is not just a fresh insert.
Document BuildModel() {
    Document document;
    const ElementId ground = Id(0x10);
    const ElementId upper = Id(0x20);
    RequireOk(document.AddLevel(Level{.id = ground, .elevation = 0.0}));
    RequireOk(document.AddLevel(Level{.id = upper, .elevation = 2.5}));
    RequireOk(document.AddStraightWall(
        MakeWall(Id(0x31), ground, Point2D{0.0, 0.0}, Point2D{6.0, 0.0}, 0.2, 3.0, 0.0)));
    RequireOk(document.AddStraightWall(
        MakeWall(Id(0x32), ground, Point2D{6.0, 0.0}, Point2D{6.0, 4.0}, 0.3, 2.8, 0.1)));
    RequireOk(document.AddStraightWall(
        MakeWall(Id(0x41), upper, Point2D{-1.5, 2.25}, Point2D{3.125, -0.75}, 0.15, 2.4, -0.05)));
    RequireOk(document.AddStraightWall(
        MakeWall(Id(0x42), upper, Point2D{0.0, 0.0}, Point2D{1.0, 1.0}, 0.1, 1.0, 0.0)));
    RequireOk(document.UpdateLevelElevation(upper, 3.0));
    RequireOk(document.UpdateStraightWall(
        MakeWall(Id(0x32), ground, Point2D{6.0, 0.0}, Point2D{6.0, 5.5}, 0.3, 2.8, 0.1)));
    RequireOk(document.DeleteElement(Id(0x42)));
    return document;
}

std::int64_t JournalTransactionCount(const std::filesystem::path& path) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    SqliteStatement statement;
    REQUIRE(SqliteStatement::Prepare(connection.handle(),
                                     "SELECT count(*) FROM journal_transactions;", statement)
                .ok());
    bool has_row = false;
    REQUIRE(statement.Step(has_row).ok());
    REQUIRE(has_row);
    return statement.ColumnInt64(0);
}

void RunRaw(const std::filesystem::path& path, std::initializer_list<std::string_view> statements) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    for (const std::string_view sql : statements) {
        SqliteStatement statement;
        INFO(std::string(sql));
        REQUIRE(SqliteStatement::Prepare(connection.handle(), sql, statement).ok());
        bool has_row = false;
        REQUIRE(statement.Step(has_row).ok());
    }
}

} // namespace

TEST_CASE("lifecycle result codes are a separate contract and DocumentResultCode is not extended",
          "[p1_t004][save_reopen]") {
    REQUIRE(std::string(bim::document::ToString(DocumentLifecycleResultCode::Ok)) == "Ok");
    REQUIRE(std::string(bim::document::ToString(DocumentLifecycleResultCode::PersistenceFailure)) ==
            "PersistenceFailure");
    REQUIRE(std::string(
                bim::document::ToString(DocumentLifecycleResultCode::RuntimeCapacityExhausted)) ==
            "RuntimeCapacityExhausted");
    REQUIRE(std::string(bim::document::ToString(
                DocumentLifecycleResultCode::DerivedGeometryInvalid)) == "DerivedGeometryInvalid");
    REQUIRE(std::string(bim::document::ToString(DocumentLifecycleResultCode::RecomputeFailed)) ==
            "RecomputeFailed");
    REQUIRE(std::string(bim::document::ToString(DocumentLifecycleResultCode::InternalFailure)) ==
            "InternalFailure");
    REQUIRE(std::string(bim::document::ToString(static_cast<DocumentLifecycleResultCode>(200))) ==
            "Unknown");
    REQUIRE(DocumentLifecycleResult{}.ok());
    REQUIRE_FALSE(
        DocumentLifecycleResult{.code = DocumentLifecycleResultCode::PersistenceFailure}.ok());

    // The mutation result contract kept exactly its P1-T003 enumerators: the
    // last one is ElementHasDependents (12) and the next value is unknown.
    REQUIRE(static_cast<int>(DocumentResultCode::ElementHasDependents) == 12);
    REQUIRE(std::string(bim::document::ToString(static_cast<DocumentResultCode>(13))) == "Unknown");
}

TEST_CASE("a saved document reopens with identical values, order and regenerated geometry",
          "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document original = BuildModel();
    const Observed before = Observe(original);
    REQUIRE(before.levels.size() == 2);
    REQUIRE(before.walls.size() == 3);

    RequireLifecycleOk(original.Save(db_path.path()));
    // Saving changes nothing about the document itself.
    REQUIRE(SameObserved(Observe(original), before));

    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));
    const Observed after = Observe(reopened);
    REQUIRE(SameObserved(after, before));

    // Derived geometry is regenerated, not stored: every wall has it again and it
    // is the value the original computed.
    for (std::size_t i = 0; i < after.walls.size(); ++i) {
        REQUIRE(after.geometry[i].has_value());
    }
    // Finds work by ElementId as well.
    REQUIRE(reopened.FindLevel(Id(0x20)).has_value());
    REQUIRE(reopened.FindStraightWall(Id(0x41)).has_value());
    REQUIRE_FALSE(reopened.FindStraightWall(Id(0x42)).has_value()); // deleted before the save
}

TEST_CASE("save and open are stable across repeated cycles", "[p1_t004][save_reopen]") {
    const TempDatabasePath first;
    const TempDatabasePath second;
    Document original = BuildModel();
    const Observed expected = Observe(original);

    RequireLifecycleOk(original.Save(first.path()));
    Document one;
    RequireLifecycleOk(Document::Open(first.path(), one));
    RequireLifecycleOk(one.Save(second.path()));
    Document two;
    RequireLifecycleOk(Document::Open(second.path(), two));
    // Saving over the same file again is also stable.
    RequireLifecycleOk(two.Save(second.path()));
    Document three;
    RequireLifecycleOk(Document::Open(second.path(), three));

    REQUIRE(SameObserved(Observe(one), expected));
    REQUIRE(SameObserved(Observe(two), expected));
    REQUIRE(SameObserved(Observe(three), expected));
}

TEST_CASE("awkward double values round-trip exactly", "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document original;
    const ElementId level = Id(0x11);
    RequireOk(original.AddLevel(Level{.id = level, .elevation = -12345.678901234567}));
    const std::vector<std::pair<Point2D, Point2D>> axes = {
        {Point2D{0.1, 0.2}, Point2D{1.0 / 3.0, -2.0 / 7.0}},
        {Point2D{1.0e-300, -1.0e-300}, Point2D{1.0e-299, 2.0e-300}},
        {Point2D{-1.0e100, 3.0e100}, Point2D{2.5e100, -1.0e99}},
        {Point2D{123456789.123456789, -987654321.987654321}, Point2D{0.0, 0.0}},
    };
    std::uint8_t next = 0x50;
    for (const auto& axis : axes) {
        RequireOk(original.AddStraightWall(MakeWall(Id(next++), level, axis.first, axis.second,
                                                    0.1 + 0.2, 2.0 / 3.0, -1.0 / 3.0)));
    }
    const Observed expected = Observe(original);

    RequireLifecycleOk(original.Save(db_path.path()));
    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));
    REQUIRE(SameObserved(Observe(reopened), expected));
}

TEST_CASE("ElementId byte order, not insertion order, orders the reopened lists",
          "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document original;
    const std::vector<std::uint8_t> level_keys = {0xF0, 0x0A, 0x80, 0x7F, 0x01};
    for (const std::uint8_t key : level_keys) {
        RequireOk(original.AddLevel(Level{.id = Id(key), .elevation = static_cast<double>(key)}));
    }
    // Walls hosted by scrambled Levels, inserted in a scrambled order.
    const std::vector<std::uint8_t> wall_keys = {0xE1, 0x05, 0x90, 0x6C, 0x02};
    for (std::size_t i = 0; i < wall_keys.size(); ++i) {
        RequireOk(original.AddStraightWall(
            MakeWall(Id(wall_keys[i]), Id(level_keys[(i * 3) % level_keys.size()]),
                     Point2D{0.0, 0.0}, Point2D{1.0 + static_cast<double>(i), 0.5})));
    }

    RequireLifecycleOk(original.Save(db_path.path()));
    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));

    const std::vector<Level> levels = reopened.ListLevels();
    REQUIRE(levels.size() == level_keys.size());
    REQUIRE(std::is_sorted(levels.begin(), levels.end(),
                           [](const Level& a, const Level& b) { return a.id < b.id; }));
    const std::vector<StraightWall> walls = reopened.ListStraightWalls();
    REQUIRE(walls.size() == wall_keys.size());
    REQUIRE(
        std::is_sorted(walls.begin(), walls.end(),
                       [](const StraightWall& a, const StraightWall& b) { return a.id < b.id; }));
    REQUIRE(SameObserved(Observe(reopened), Observe(original)));
}

TEST_CASE("a larger generated model round-trips", "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document original;
    std::vector<ElementId> levels;
    for (int i = 0; i < 12; ++i) {
        const ElementId level = NewId();
        levels.push_back(level);
        RequireOk(original.AddLevel(Level{.id = level, .elevation = 3.0 * i}));
        for (int w = 0; w < 9; ++w) {
            RequireOk(original.AddStraightWall(MakeWall(
                NewId(), level, Point2D{static_cast<double>(w), 0.0},
                Point2D{static_cast<double>(w) + 4.0, 3.0 + w}, 0.2 + 0.01 * w, 3.0, 0.05 * i)));
        }
    }
    const Observed expected = Observe(original);
    REQUIRE(expected.levels.size() == 12);
    REQUIRE(expected.walls.size() == 108);

    RequireLifecycleOk(original.Save(db_path.path()));
    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));
    REQUIRE(SameObserved(Observe(reopened), expected));
}

TEST_CASE("an empty document saves and reopens as empty", "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document empty;
    RequireLifecycleOk(empty.Save(db_path.path()));
    REQUIRE(std::filesystem::exists(db_path.path()));

    // Opening it into a populated document replaces that content with nothing.
    Document target = BuildModel();
    REQUIRE_FALSE(target.ListLevels().empty());
    RequireLifecycleOk(Document::Open(db_path.path(), target));
    REQUIRE(target.ListLevels().empty());
    REQUIRE(target.ListStraightWalls().empty());
}

TEST_CASE("open replaces the target document completely", "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    Document stored;
    RequireOk(stored.AddLevel(Level{.id = Id(0x61), .elevation = 7.0}));
    RequireOk(
        stored.AddStraightWall(MakeWall(Id(0x62), Id(0x61), Point2D{0.0, 0.0}, Point2D{2.0, 0.0})));
    const Observed expected = Observe(stored);
    RequireLifecycleOk(stored.Save(db_path.path()));

    Document target = BuildModel(); // different ids, different content
    RequireLifecycleOk(Document::Open(db_path.path(), target));
    REQUIRE(SameObserved(Observe(target), expected));
    REQUIRE_FALSE(target.FindLevel(Id(0x10)).has_value());
    REQUIRE_FALSE(target.FindStraightWall(Id(0x31)).has_value());
}

TEST_CASE("a reopened document is fully live: dependencies are rebuilt and edits recompute",
          "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    {
        Document original = BuildModel();
        RequireLifecycleOk(original.Save(db_path.path()));
    }
    Document document;
    RequireLifecycleOk(Document::Open(db_path.path(), document));

    const ElementId ground = Id(0x10);
    const ElementId upper = Id(0x20);

    // A Level elevation change recomputes exactly the walls hosted by that Level.
    const auto ground_before = document.FindWallGeometry(Id(0x31));
    const auto upper_before = document.FindWallGeometry(Id(0x41));
    REQUIRE(ground_before.has_value());
    REQUIRE(upper_before.has_value());
    RequireOk(document.UpdateLevelElevation(upper, 10.0));
    const auto upper_after = document.FindWallGeometry(Id(0x41));
    const auto ground_after = document.FindWallGeometry(Id(0x31));
    REQUIRE(upper_after.has_value());
    REQUIRE(ground_after.has_value());
    // base_z = level.elevation + base_offset (-0.05 for wall 0x41).
    REQUIRE(upper_after->profile.origin.z == 10.0 + -0.05);
    REQUIRE(upper_after->profile.origin.z != upper_before->profile.origin.z);
    REQUIRE(SpecEqual(*ground_after, *ground_before));

    // A wall edit recomputes that wall only.
    RequireOk(document.UpdateStraightWall(
        MakeWall(Id(0x31), ground, Point2D{0.0, 0.0}, Point2D{8.0, 0.0}, 0.2, 3.0, 0.0)));
    const auto resized = document.FindWallGeometry(Id(0x31));
    REQUIRE(resized.has_value());
    REQUIRE(resized->profile.size_u == 8.0);

    // The Level -> StraightWall relationships exist again: a Level that still
    // hosts walls cannot be deleted, and the rejection changes nothing.
    const Observed before_reject = Observe(document);
    const DocumentResult rejected = document.DeleteElement(ground);
    REQUIRE(rejected.code == DocumentResultCode::ElementHasDependents);
    REQUIRE(SameObserved(Observe(document), before_reject));

    // Trimming the dependents makes the Level deletable.
    RequireOk(document.DeleteElement(Id(0x31)));
    RequireOk(document.DeleteElement(Id(0x32)));
    RequireOk(document.DeleteElement(ground));
    REQUIRE_FALSE(document.FindLevel(ground).has_value());

    // New elements can still be added, including onto a reopened Level, and a
    // reopened id is still a taken id.
    RequireOk(
        document.AddStraightWall(MakeWall(Id(0x77), upper, Point2D{0.0, 0.0}, Point2D{3.0, 4.0})));
    REQUIRE(
        document.AddStraightWall(MakeWall(Id(0x41), upper, Point2D{0.0, 0.0}, Point2D{1.0, 0.0}))
            .code == DocumentResultCode::DuplicateElementId);
    REQUIRE(document.AddLevel(Level{.id = Id(0x41), .elevation = 1.0}).code ==
            DocumentResultCode::DuplicateElementId);
}

TEST_CASE("open failures leave the target document unchanged", "[p1_t004][save_reopen]") {
    const TempDatabasePath good;
    const TempDatabasePath missing;
    const TempDatabasePath garbage;
    const TempDatabasePath damaged;

    {
        Document stored = BuildModel();
        RequireLifecycleOk(stored.Save(good.path()));
    }
    {
        std::ofstream file(garbage.path(), std::ios::binary);
        file << "not a database at all, just text\n";
    }
    {
        Document stored = BuildModel();
        RequireLifecycleOk(stored.Save(damaged.path()));
        // A hosted wall pointing at a Level that does not exist (foreign keys off so
        // the damage can be written at all): the content is not a valid document.
        RunRaw(damaged.path(),
               {"PRAGMA foreign_keys = OFF;",
                "UPDATE straight_walls SET level_id = '00000099-0000-0000-0000-000000000001' WHERE "
                "element_id = '31000000-0000-0000-0000-000000000001';"});
    }

    Document target;
    RequireOk(target.AddLevel(Level{.id = Id(0xA1), .elevation = 1.5}));
    RequireOk(
        target.AddStraightWall(MakeWall(Id(0xA2), Id(0xA1), Point2D{0.0, 0.0}, Point2D{5.0, 5.0})));
    const Observed expected = Observe(target);

    for (const std::filesystem::path& bad :
         {missing.path(), garbage.path(), damaged.path(), std::filesystem::temp_directory_path()}) {
        INFO(bad.string());
        const DocumentLifecycleResult result = Document::Open(bad, target);
        REQUIRE_FALSE(result.ok());
        REQUIRE(result.code == DocumentLifecycleResultCode::PersistenceFailure);
        REQUIRE(SameObserved(Observe(target), expected));
    }
    // A failed open never creates the file it was asked to open.
    REQUIRE_FALSE(std::filesystem::exists(missing.path()));

    // The target still works, and its pending journal survived the failed opens
    // (it is saved, with its own two mutations, by the next successful Save).
    const TempDatabasePath saved;
    RequireLifecycleOk(target.Save(saved.path()));
    REQUIRE(JournalTransactionCount(saved.path()) == 2);

    // And the good file still opens.
    RequireLifecycleOk(Document::Open(good.path(), target));
    REQUIRE(target.ListLevels().size() == 2);
}

TEST_CASE("opening a schema v1 database yields an empty document and migrates it",
          "[p1_t004][save_reopen]") {
    const TempDatabasePath db_path;
    {
        SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());
        bim::transactions::JournalTransaction transaction;
        transaction.id = "legacy";
        transaction.records.push_back(bim::transactions::JournalRecord{"legacy.kind", {}});
        REQUIRE(bim::persistence::detail::AppendJournalTransaction(connection.handle(), transaction)
                    .ok());
    }
    Document target = BuildModel();
    RequireLifecycleOk(Document::Open(db_path.path(), target));
    REQUIRE(target.ListLevels().empty());
    REQUIRE(target.ListStraightWalls().empty());
    REQUIRE(JournalTransactionCount(db_path.path()) == 1); // legacy history kept
    {
        SqliteConnection connection;
        REQUIRE(SqliteConnection::OpenFileWithPolicy(db_path.path(), connection).ok());
        SqliteStatement statement;
        REQUIRE(
            SqliteStatement::Prepare(connection.handle(), "PRAGMA user_version;", statement).ok());
        bool has_row = false;
        REQUIRE(statement.Step(has_row).ok());
        REQUIRE(has_row);
        REQUIRE(statement.ColumnInt64(0) == 2);
    }
}

TEST_CASE("save failures leave the document unchanged and its journal pending",
          "[p1_t004][save_reopen]") {
    Document document = BuildModel();
    const Observed expected = Observe(document);

    // A path whose parent directory does not exist cannot be created.
    const std::filesystem::path unwritable = std::filesystem::temp_directory_path() /
                                             "bim_p1_t004_no_such_directory_for_save" /
                                             "document.db";
    const DocumentLifecycleResult failed = document.Save(unwritable);
    REQUIRE_FALSE(failed.ok());
    REQUIRE(failed.code == DocumentLifecycleResultCode::PersistenceFailure);
    REQUIRE(SameObserved(Observe(document), expected));

    // The failed save did not consume the pending journal: a later successful save
    // writes every one of the model's nine mutations.
    const TempDatabasePath db_path;
    RequireLifecycleOk(document.Save(db_path.path()));
    REQUIRE(JournalTransactionCount(db_path.path()) == 9);
}
