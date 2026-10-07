// P1-T003 Commands & Query Vertical Slice - unit proof of the public
// bim::commands and bim::query APIs (Implementation Brief P1-T003-IB sections
// 4, 5 and 11). Links bim::commands, bim::query (and, through them,
// bim::document) and Catch2 only. Everything is proved through observable
// committed values: there is deliberately no way to see the document's private
// runtime identity or dependency graph from here, so survivor stability after a
// deletion is proved by its observable consequence - the surviving walls still
// recompute when their Level changes - and identity non-reuse is left to source
// review, as the brief requires.

#include "bim/commands/commands.hpp"
#include "bim/query/query.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

using bim::commands::CommandResult;
using bim::commands::CommandResultCode;
using bim::document::Document;
using bim::geometry_api::LinearExtrusionSpec;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::model::ValidationCode;

namespace {

ElementId Id(std::uint8_t last) {
    ElementId id;
    id.bytes[15] = last;
    return id;
}

Level MakeLevel(std::uint8_t id_last, double elevation) {
    return Level{.id = Id(id_last), .elevation = elevation};
}

// Binary-friendly defaults keep every expected derived value exactly
// representable, so axis-aligned cases are compared with ==.
StraightWall MakeWall(std::uint8_t id_last, std::uint8_t level_last, Point2D start, Point2D end,
                      double thickness = 0.5, double height = 3.0, double base_offset = 0.25) {
    return StraightWall{.id = Id(id_last),
                        .level_id = Id(level_last),
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr double kHuge = 1.0e308; // finite, but adding two of them overflows

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

// A complete observable copy of a document, built only from public const reads:
// the ordered query lists plus the derived geometry of every listed wall.
struct Snapshot {
    std::vector<Level> levels;
    std::vector<StraightWall> walls;
    std::vector<std::optional<LinearExtrusionSpec>> geometry;
};

Snapshot TakeSnapshot(const Document& document) {
    Snapshot snapshot;
    snapshot.levels = bim::query::ListLevels(document);
    snapshot.walls = bim::query::ListStraightWalls(document);
    for (const StraightWall& wall : snapshot.walls) {
        snapshot.geometry.push_back(document.FindWallGeometry(wall.id));
    }
    return snapshot;
}

bool SameSnapshot(const Snapshot& a, const Snapshot& b) {
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

LinearExtrusionSpec Geometry(const Document& document, const ElementId& wall_id) {
    const auto found = document.FindWallGeometry(wall_id);
    REQUIRE(found.has_value());
    return *found;
}

double BaseZ(const Document& document, const ElementId& wall_id) {
    return Geometry(document, wall_id).profile.origin.z;
}

// Runs a command that the test requires to succeed.
void RequireOk(const CommandResult& result) {
    REQUIRE(result.code == CommandResultCode::Ok);
    REQUIRE(result.validation == ValidationCode::Ok);
}

// --- compile-time contract ---------------------------------------------------

static_assert(std::is_trivially_copyable_v<CommandResult>,
              "a result must be allocation-free to build");
static_assert(noexcept(bim::commands::CreateLevel(std::declval<Document&>(),
                                                  std::declval<const Level&>())));
static_assert(noexcept(bim::commands::ChangeLevelElevation(std::declval<Document&>(),
                                                           std::declval<const ElementId&>(), 0.0)));
static_assert(noexcept(bim::commands::CreateStraightWall(std::declval<Document&>(),
                                                         std::declval<const StraightWall&>())));
static_assert(noexcept(bim::commands::ChangeStraightWallGeometry(
    std::declval<Document&>(), std::declval<const ElementId&>(), std::declval<const Point2D&>(),
    std::declval<const Point2D&>(), 0.0, 0.0, 0.0)));
static_assert(noexcept(bim::commands::DeleteElement(std::declval<Document&>(),
                                                    std::declval<const ElementId&>())));
static_assert(noexcept(bim::query::FindLevel(std::declval<const Document&>(),
                                             std::declval<const ElementId&>())));
static_assert(noexcept(bim::query::FindStraightWall(std::declval<const Document&>(),
                                                    std::declval<const ElementId&>())));
// Queries are only callable with a const document view; a mutable Document
// converts to it, but the query signatures themselves name no mutable reference.
static_assert(std::is_same_v<decltype(bim::query::ListLevels(std::declval<const Document&>())),
                             std::vector<Level>>);
static_assert(
    std::is_same_v<decltype(bim::query::ListStraightWalls(std::declval<const Document&>())),
                   std::vector<StraightWall>>);

} // namespace

// --- CreateLevel -------------------------------------------------------------

TEST_CASE("Commands: CreateLevel adds a caller-identified Level", "[unit][commands][p1-t003]") {
    Document document;
    const Level level = MakeLevel(1, 4.0);

    const CommandResult result = bim::commands::CreateLevel(document, level);
    CHECK(result.ok());
    CHECK(result.code == CommandResultCode::Ok);
    CHECK(result.validation == ValidationCode::Ok);

    // The command layer generates no identity: the Level is stored under exactly
    // the id the caller supplied.
    const auto stored = bim::query::FindLevel(document, level.id);
    REQUIRE(stored.has_value());
    CHECK(*stored == level);
    CHECK(bim::query::ListLevels(document).size() == 1);
}

TEST_CASE("Commands: an invalid Level is rejected with its validation code and zero mutation",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    const Snapshot before = TakeSnapshot(document);

    const CommandResult zero_id = bim::commands::CreateLevel(document, Level{ElementId{}, 1.0});
    CHECK(zero_id.code == CommandResultCode::InvalidElement);
    CHECK(zero_id.validation == ValidationCode::InvalidElementId);

    const CommandResult nan_elevation = bim::commands::CreateLevel(document, MakeLevel(2, kNaN));
    CHECK(nan_elevation.code == CommandResultCode::InvalidElement);
    CHECK(nan_elevation.validation == ValidationCode::NonFiniteElevation);

    const CommandResult inf_elevation = bim::commands::CreateLevel(document, MakeLevel(3, kInf));
    CHECK(inf_elevation.code == CommandResultCode::InvalidElement);
    CHECK(inf_elevation.validation == ValidationCode::NonFiniteElevation);

    CHECK_FALSE(bim::query::FindLevel(document, Id(2)).has_value());
    CHECK_FALSE(bim::query::FindLevel(document, Id(3)).has_value());
    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

TEST_CASE("Commands: a duplicate ElementId is rejected across both element kinds",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 8.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {5, 0})));
    const Snapshot before = TakeSnapshot(document);

    // Same id, different elevation: the original must survive untouched.
    const CommandResult same_level = bim::commands::CreateLevel(document, MakeLevel(1, 99.0));
    CHECK(same_level.code == CommandResultCode::DuplicateElementId);
    CHECK(same_level.validation == ValidationCode::Ok);

    // A Level may not take a wall's id, and a wall may not take a Level's id.
    CHECK(bim::commands::CreateLevel(document, MakeLevel(10, 2.0)).code ==
          CommandResultCode::DuplicateElementId);
    CHECK(bim::commands::CreateStraightWall(document, MakeWall(1, 2, {0, 0}, {1, 0})).code ==
          CommandResultCode::DuplicateElementId);
    // A wall that names itself as its own host is a structural error, not a duplicate.
    CHECK(bim::commands::CreateStraightWall(document, MakeWall(20, 20, {0, 0}, {1, 0})).code ==
          CommandResultCode::InvalidElement);
    CHECK(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 1}, {5, 1})).code ==
          CommandResultCode::DuplicateElementId);

    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

// --- CreateStraightWall ------------------------------------------------------

TEST_CASE("Commands: CreateStraightWall adds a wall and derives its geometry",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 4.0)));
    const StraightWall wall = MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0});

    const CommandResult result = bim::commands::CreateStraightWall(document, wall);
    CHECK(result.ok());

    const auto stored = bim::query::FindStraightWall(document, wall.id);
    REQUIRE(stored.has_value());
    CHECK(*stored == wall);

    // base z = level elevation + base offset = 4.25; axis along +X, extruded +Z.
    const LinearExtrusionSpec spec = Geometry(document, wall.id);
    CHECK(spec.profile.origin.x == 0.0);
    CHECK(spec.profile.origin.y == -0.25);
    CHECK(spec.profile.origin.z == 4.25);
    CHECK(spec.profile.size_u == 10.0);
    CHECK(spec.profile.size_v == 0.5);
    CHECK(spec.direction.z == 1.0);
    CHECK(spec.distance == 3.0);
}

TEST_CASE("Commands: CreateStraightWall rejects a missing Level, an invalid wall and an "
          "overflowing wall with zero mutation",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    const Snapshot before = TakeSnapshot(document);

    const CommandResult missing_level =
        bim::commands::CreateStraightWall(document, MakeWall(10, 7, {0, 0}, {5, 0}));
    CHECK(missing_level.code == CommandResultCode::LevelNotFound);
    CHECK(missing_level.validation == ValidationCode::Ok);

    const CommandResult coincident =
        bim::commands::CreateStraightWall(document, MakeWall(11, 1, {2, 2}, {2, 2}));
    CHECK(coincident.code == CommandResultCode::InvalidElement);
    CHECK(coincident.validation == ValidationCode::CoincidentEndpoints);

    const CommandResult thin =
        bim::commands::CreateStraightWall(document, MakeWall(12, 1, {0, 0}, {5, 0}, 0.0));
    CHECK(thin.code == CommandResultCode::InvalidElement);
    CHECK(thin.validation == ValidationCode::NonPositiveThickness);

    // Structurally valid but the derived base height overflows.
    const CommandResult overflow =
        bim::commands::CreateStraightWall(document, MakeWall(13, 1, {-kHuge, 0}, {kHuge, 0}));
    CHECK(overflow.code == CommandResultCode::DerivedGeometryInvalid);
    // An overflow still leaves the Level able to host a valid wall afterwards.
    CHECK(SameSnapshot(before, TakeSnapshot(document)));
    CHECK(bim::commands::CreateStraightWall(document, MakeWall(13, 1, {0, 0}, {5, 0})).ok());
}

// --- ChangeLevelElevation ----------------------------------------------------

TEST_CASE("Commands: ChangeLevelElevation recomputes the geometry of every hosted wall",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 100.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 5}, {10, 5})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(12, 2, {0, 9}, {10, 9})));
    REQUIRE(BaseZ(document, Id(10)) == 0.25);
    REQUIRE(BaseZ(document, Id(11)) == 0.25);
    REQUIRE(BaseZ(document, Id(12)) == 100.25);

    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 7.0));

    CHECK(bim::query::FindLevel(document, Id(1))->elevation == 7.0);
    CHECK(BaseZ(document, Id(10)) == 7.25);
    CHECK(BaseZ(document, Id(11)) == 7.25);
    // The other Level and its wall are an independent domain.
    CHECK(bim::query::FindLevel(document, Id(2))->elevation == 100.0);
    CHECK(BaseZ(document, Id(12)) == 100.25);
}

TEST_CASE("Commands: ChangeLevelElevation fails closed with zero mutation",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 2.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document,
                                                MakeWall(11, 1, {0, 4}, {10, 4}, 0.5, 3.0, kHuge)));
    const Snapshot before = TakeSnapshot(document);

    CHECK(bim::commands::ChangeLevelElevation(document, Id(99), 1.0).code ==
          CommandResultCode::ElementNotFound);
    CHECK(bim::commands::ChangeLevelElevation(document, Id(10), 1.0).code ==
          CommandResultCode::ElementKindMismatch);

    const CommandResult nan_elevation = bim::commands::ChangeLevelElevation(document, Id(1), kNaN);
    CHECK(nan_elevation.code == CommandResultCode::InvalidElement);
    CHECK(nan_elevation.validation == ValidationCode::NonFiniteElevation);

    // Wall 11 hosts a base offset of 1e308: lifting its Level by 1e308 overflows
    // that wall, so the whole elevation change - including wall 10 - is refused.
    CHECK(bim::commands::ChangeLevelElevation(document, Id(1), kHuge).code ==
          CommandResultCode::DerivedGeometryInvalid);

    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

// --- ChangeStraightWallGeometry ----------------------------------------------

TEST_CASE("Commands: ChangeStraightWallGeometry replaces the geometry and keeps the host",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 4.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 50.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 2, {0, 0}, {6, 0})));
    const auto untouched_before = Geometry(document, Id(11));

    const CommandResult result = bim::commands::ChangeStraightWallGeometry(
        document, Id(10), Point2D{2.0, 1.0}, Point2D{2.0, 9.0}, 1.0, 0.5, 6.0);
    CHECK(result.ok());

    const auto changed = bim::query::FindStraightWall(document, Id(10));
    REQUIRE(changed.has_value());
    CHECK(changed->id == Id(10));
    CHECK(changed->level_id == Id(1)); // the hosting Level is preserved
    CHECK(changed->start == Point2D{2.0, 1.0});
    CHECK(changed->end == Point2D{2.0, 9.0});
    CHECK(changed->thickness == 1.0);
    CHECK(changed->base_offset == 0.5);
    CHECK(changed->height == 6.0);

    // The derived geometry follows the new values: along +Y, base z = 4.5.
    const LinearExtrusionSpec spec = Geometry(document, Id(10));
    CHECK(spec.profile.origin.z == 4.5);
    CHECK(spec.profile.size_u == 8.0);
    CHECK(spec.profile.size_v == 1.0);
    CHECK(spec.distance == 6.0);

    // Another wall is not recomputed or changed.
    CHECK(SpecEqual(untouched_before, Geometry(document, Id(11))));
    CHECK(bim::query::FindStraightWall(document, Id(11))->level_id == Id(2));
}

TEST_CASE("Commands: invalid wall geometry is rejected with zero mutation",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    const Snapshot before = TakeSnapshot(document);

    const CommandResult coincident = bim::commands::ChangeStraightWallGeometry(
        document, Id(10), Point2D{3, 3}, Point2D{3, 3}, 0.5, 0.25, 3.0);
    CHECK(coincident.code == CommandResultCode::InvalidElement);
    CHECK(coincident.validation == ValidationCode::CoincidentEndpoints);

    const CommandResult flat = bim::commands::ChangeStraightWallGeometry(
        document, Id(10), Point2D{0, 0}, Point2D{10, 0}, 0.5, 0.25, -1.0);
    CHECK(flat.code == CommandResultCode::InvalidElement);
    CHECK(flat.validation == ValidationCode::NonPositiveHeight);

    const CommandResult nan_offset = bim::commands::ChangeStraightWallGeometry(
        document, Id(10), Point2D{0, 0}, Point2D{10, 0}, 0.5, kNaN, 3.0);
    CHECK(nan_offset.code == CommandResultCode::InvalidElement);
    CHECK(nan_offset.validation == ValidationCode::NonFiniteBaseOffset);

    // Valid structurally, but the derived geometry overflows.
    const CommandResult overflow = bim::commands::ChangeStraightWallGeometry(
        document, Id(10), Point2D{-kHuge, 0}, Point2D{kHuge, 0}, 0.5, 0.25, 3.0);
    CHECK(overflow.code == CommandResultCode::DerivedGeometryInvalid);

    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

TEST_CASE("Commands: ChangeStraightWallGeometry requires an existing wall",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    const Snapshot before = TakeSnapshot(document);

    CHECK(bim::commands::ChangeStraightWallGeometry(document, Id(99), Point2D{0, 0}, Point2D{1, 0},
                                                    0.5, 0.0, 3.0)
              .code == CommandResultCode::ElementNotFound);
    CHECK(bim::commands::ChangeStraightWallGeometry(document, ElementId{}, Point2D{0, 0},
                                                    Point2D{1, 0}, 0.5, 0.0, 3.0)
              .code == CommandResultCode::ElementNotFound);
    // The id of a Level is the wrong kind, not a missing element.
    CHECK(bim::commands::ChangeStraightWallGeometry(document, Id(1), Point2D{0, 0}, Point2D{1, 0},
                                                    0.5, 0.0, 3.0)
              .code == CommandResultCode::ElementKindMismatch);

    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

TEST_CASE("Commands: no command can re-host a wall", "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 30.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));

    // The geometry command has no host parameter: whatever it is given, the wall
    // stays on its Level and its base z keeps following that Level.
    RequireOk(bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{0, 0},
                                                        Point2D{20, 0}, 0.5, 0.25, 3.0));
    CHECK(bim::query::FindStraightWall(document, Id(10))->level_id == Id(1));
    CHECK(BaseZ(document, Id(10)) == 0.25);

    // The other Level's elevation does not reach the wall; its own Level's does.
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(2), 60.0));
    CHECK(BaseZ(document, Id(10)) == 0.25);
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 8.0));
    CHECK(BaseZ(document, Id(10)) == 8.25);

    // Re-creating the same wall under another Level is a duplicate id, not a move.
    CHECK(bim::commands::CreateStraightWall(document, MakeWall(10, 2, {0, 0}, {10, 0})).code ==
          CommandResultCode::DuplicateElementId);
    CHECK(bim::query::FindStraightWall(document, Id(10))->level_id == Id(1));
}

// --- DeleteElement -----------------------------------------------------------

TEST_CASE("Commands: DeleteElement removes a StraightWall and its derived geometry",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 5}, {10, 5})));
    const auto survivor_geometry = Geometry(document, Id(11));

    const CommandResult result = bim::commands::DeleteElement(document, Id(10));
    CHECK(result.ok());

    CHECK_FALSE(bim::query::FindStraightWall(document, Id(10)).has_value());
    CHECK_FALSE(document.FindWallGeometry(Id(10)).has_value());
    CHECK(bim::query::FindLevel(document, Id(1)).has_value());
    REQUIRE(bim::query::FindStraightWall(document, Id(11)).has_value());
    CHECK(SpecEqual(survivor_geometry, Geometry(document, Id(11))));
    CHECK(bim::query::ListStraightWalls(document).size() == 1);

    // Deleting the same id again finds nothing.
    CHECK(bim::commands::DeleteElement(document, Id(10)).code ==
          CommandResultCode::ElementNotFound);
}

TEST_CASE("Commands: DeleteElement removes an empty Level", "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 10.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 2, {0, 0}, {10, 0})));

    // Level 1 hosts nothing, so it may go; the other Level and its wall stay.
    CHECK(bim::commands::DeleteElement(document, Id(1)).ok());
    CHECK_FALSE(bim::query::FindLevel(document, Id(1)).has_value());
    CHECK(bim::query::FindLevel(document, Id(2)).has_value());
    CHECK(bim::query::FindStraightWall(document, Id(10)).has_value());
    CHECK(BaseZ(document, Id(10)) == 10.25);

    // A Level becomes deletable once its last wall is gone.
    CHECK(bim::commands::DeleteElement(document, Id(2)).code ==
          CommandResultCode::ElementHasDependents);
    RequireOk(bim::commands::DeleteElement(document, Id(10)));
    CHECK(bim::commands::DeleteElement(document, Id(2)).ok());
    CHECK(bim::query::ListLevels(document).empty());
    CHECK(bim::query::ListStraightWalls(document).empty());
}

TEST_CASE("Commands: a Level with dependent walls is refused with zero mutation",
          "[unit][commands][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 3.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 5}, {10, 5})));
    const Snapshot before = TakeSnapshot(document);

    const CommandResult result = bim::commands::DeleteElement(document, Id(1));
    CHECK(result.code == CommandResultCode::ElementHasDependents);
    CHECK(result.validation == ValidationCode::Ok);
    CHECK(SameSnapshot(before, TakeSnapshot(document)));

    // One dependent is enough, and there is no cascade: deleting one wall still
    // leaves the other blocking the Level.
    RequireOk(bim::commands::DeleteElement(document, Id(10)));
    CHECK(bim::commands::DeleteElement(document, Id(1)).code ==
          CommandResultCode::ElementHasDependents);
    CHECK(bim::query::FindStraightWall(document, Id(11)).has_value());
    CHECK(bim::query::FindLevel(document, Id(1)).has_value());
}

TEST_CASE("Commands: a failed deletion changes nothing observable", "[unit][commands][p1-t003]") {
    Document document;
    CHECK(bim::commands::DeleteElement(document, Id(1)).code ==
          CommandResultCode::ElementNotFound); // an empty document

    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 3.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    const Snapshot before = TakeSnapshot(document);

    CHECK(bim::commands::DeleteElement(document, Id(99)).code ==
          CommandResultCode::ElementNotFound);
    CHECK(bim::commands::DeleteElement(document, ElementId{}).code ==
          CommandResultCode::ElementNotFound);
    CHECK(bim::commands::DeleteElement(document, Id(1)).code ==
          CommandResultCode::ElementHasDependents);

    CHECK(SameSnapshot(before, TakeSnapshot(document)));
}

TEST_CASE("Commands: surviving walls keep recomputing after deletions",
          "[unit][commands][p1-t003]") {
    // The document rebuilds its private dependency graph on every deletion. That
    // is private, so the observable consequence is what is proved: every
    // survivor is still a dependent of its Level, and new walls still attach.
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 100.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(3, 200.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 5}, {10, 5})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(12, 1, {0, 9}, {10, 9})));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(13, 2, {0, 0}, {10, 0})));

    RequireOk(bim::commands::DeleteElement(document, Id(11))); // a middle wall
    RequireOk(bim::commands::DeleteElement(document, Id(3)));  // an empty Level

    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 40.0));
    CHECK(BaseZ(document, Id(10)) == 40.25);
    CHECK(BaseZ(document, Id(12)) == 40.25);
    CHECK(BaseZ(document, Id(13)) == 100.25); // other Level: unaffected

    RequireOk(bim::commands::ChangeLevelElevation(document, Id(2), 120.0));
    CHECK(BaseZ(document, Id(13)) == 120.25);
    CHECK(BaseZ(document, Id(10)) == 40.25);

    // A deleted wall id is free again, and the new wall joins the rebuilt graph.
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 7}, {10, 7})));
    CHECK(BaseZ(document, Id(11)) == 40.25);
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), -5.0));
    CHECK(BaseZ(document, Id(10)) == -4.75);
    CHECK(BaseZ(document, Id(11)) == -4.75);
    CHECK(BaseZ(document, Id(12)) == -4.75);

    // A single-wall edit after deletions still recomputes just that wall.
    RequireOk(bim::commands::ChangeStraightWallGeometry(document, Id(12), Point2D{0, 9},
                                                        Point2D{4, 9}, 0.5, 1.0, 2.0));
    CHECK(BaseZ(document, Id(12)) == -4.0);
    CHECK(BaseZ(document, Id(10)) == -4.75);
}

// --- queries -----------------------------------------------------------------

TEST_CASE("Query: lookups return committed copies by element kind", "[unit][query][p1-t003]") {
    Document document;
    CHECK_FALSE(bim::query::FindLevel(document, Id(1)).has_value());
    CHECK_FALSE(bim::query::FindStraightWall(document, Id(1)).has_value());
    CHECK(bim::query::ListLevels(document).empty());
    CHECK(bim::query::ListStraightWalls(document).empty());

    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 4.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));

    const auto level = bim::query::FindLevel(document, Id(1));
    REQUIRE(level.has_value());
    CHECK(*level == MakeLevel(1, 4.0));
    const auto wall = bim::query::FindStraightWall(document, Id(10));
    REQUIRE(wall.has_value());
    CHECK(*wall == MakeWall(10, 1, {0, 0}, {10, 0}));

    // Each lookup answers only for its own kind; unknown and invalid ids are empty.
    CHECK_FALSE(bim::query::FindLevel(document, Id(10)).has_value());
    CHECK_FALSE(bim::query::FindStraightWall(document, Id(1)).has_value());
    CHECK_FALSE(bim::query::FindLevel(document, Id(99)).has_value());
    CHECK_FALSE(bim::query::FindStraightWall(document, Id(99)).has_value());
    CHECK_FALSE(bim::query::FindLevel(document, ElementId{}).has_value());
    CHECK_FALSE(bim::query::FindStraightWall(document, ElementId{}).has_value());

    // The query layer answers exactly what the document itself answers.
    CHECK(bim::query::FindLevel(document, Id(1)) == document.FindLevel(Id(1)));
    CHECK(bim::query::FindStraightWall(document, Id(10)) == document.FindStraightWall(Id(10)));
}

TEST_CASE("Query: Levels are enumerated in ElementId ascending order", "[unit][query][p1-t003]") {
    // ElementId order is lexicographic over the sixteen bytes, so a difference in
    // an early byte outranks any difference in a later one. Insertion is
    // deliberately neither ascending nor grouped.
    ElementId high_first = Id(1);
    high_first.bytes[0] = 0x80;
    ElementId low_last = Id(0xFF);
    ElementId mid = Id(2);
    mid.bytes[7] = 0x01;
    const std::vector<Level> expected = {
        Level{.id = Id(1), .elevation = 10.0},    Level{.id = Id(2), .elevation = 20.0},
        Level{.id = low_last, .elevation = 30.0}, // byte 15 = 0xFF, still below any byte-7
                                                  // difference
        Level{.id = mid, .elevation = 40.0},      Level{.id = high_first, .elevation = 50.0},
    };
    // Sanity: the expectation above really is ascending.
    REQUIRE(std::is_sorted(expected.begin(), expected.end(),
                           [](const Level& a, const Level& b) { return a.id < b.id; }));

    Document document;
    for (const std::size_t index : {4U, 1U, 3U, 0U, 2U}) {
        RequireOk(bim::commands::CreateLevel(document, expected[index]));
    }
    CHECK(bim::query::ListLevels(document) == expected);

    // Another insertion order yields the identical sequence.
    Document reversed;
    for (auto it = expected.rbegin(); it != expected.rend(); ++it) {
        RequireOk(bim::commands::CreateLevel(reversed, *it));
    }
    CHECK(bim::query::ListLevels(reversed) == expected);
}

TEST_CASE("Query: StraightWalls are enumerated in ElementId ascending order",
          "[unit][query][p1-t003]") {
    ElementId first = Id(5);
    ElementId second = Id(200);
    ElementId third = Id(3);
    third.bytes[0] = 0x01;
    ElementId fourth = Id(1);
    fourth.bytes[0] = 0xF0;

    const auto wall_with = [](const ElementId& id, double y) {
        StraightWall wall = MakeWall(0, 1, {0.0, y}, {10.0, y});
        wall.id = id;
        return wall;
    };
    const std::vector<StraightWall> expected = {wall_with(first, 1.0), wall_with(second, 2.0),
                                                wall_with(third, 3.0), wall_with(fourth, 4.0)};
    REQUIRE(
        std::is_sorted(expected.begin(), expected.end(),
                       [](const StraightWall& a, const StraightWall& b) { return a.id < b.id; }));

    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    for (const std::size_t index : {2U, 0U, 3U, 1U}) {
        RequireOk(bim::commands::CreateStraightWall(document, expected[index]));
    }
    CHECK(bim::query::ListStraightWalls(document) == expected);

    // Listing is by ElementId, so deleting and re-adding does not reorder it.
    RequireOk(bim::commands::DeleteElement(document, first));
    RequireOk(bim::commands::CreateStraightWall(document, expected[0]));
    CHECK(bim::query::ListStraightWalls(document) == expected);
}

TEST_CASE("Query: a failed command leaves every query result unchanged", "[unit][query][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 9.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));
    const Snapshot before = TakeSnapshot(document);

    const std::vector<CommandResult> failures = {
        bim::commands::CreateLevel(document, Level{ElementId{}, 0.0}),
        bim::commands::CreateLevel(document, MakeLevel(1, 5.0)),
        bim::commands::CreateStraightWall(document, MakeWall(11, 9, {0, 0}, {1, 0})),
        bim::commands::CreateStraightWall(document, MakeWall(11, 1, {0, 0}, {0, 0})),
        bim::commands::CreateStraightWall(document, MakeWall(11, 1, {-kHuge, 0}, {kHuge, 0})),
        bim::commands::ChangeLevelElevation(document, Id(99), 1.0),
        bim::commands::ChangeLevelElevation(document, Id(1), kNaN),
        bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{0, 0}, Point2D{0, 0},
                                                  0.5, 0.0, 3.0),
        bim::commands::ChangeStraightWallGeometry(document, Id(99), Point2D{0, 0}, Point2D{1, 0},
                                                  0.5, 0.0, 3.0),
        bim::commands::DeleteElement(document, Id(99)),
        bim::commands::DeleteElement(document, Id(1)),
    };
    for (const CommandResult& failure : failures) {
        CHECK_FALSE(failure.ok());
        CHECK(SameSnapshot(before, TakeSnapshot(document)));
    }
}

TEST_CASE("Query: results are committed-state copies, never live views", "[unit][query][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 2.0)));
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(10, 1, {0, 0}, {10, 0})));

    // Copies taken now describe the state at this moment, whatever happens next.
    const auto level_copy = bim::query::FindLevel(document, Id(1));
    const auto wall_copy = bim::query::FindStraightWall(document, Id(10));
    const std::vector<Level> levels_copy = bim::query::ListLevels(document);
    const std::vector<StraightWall> walls_copy = bim::query::ListStraightWalls(document);

    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 50.0));
    RequireOk(bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{0, 0},
                                                        Point2D{3, 0}, 0.5, 0.25, 3.0));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 6.0)));

    REQUIRE(level_copy.has_value());
    REQUIRE(wall_copy.has_value());
    CHECK(level_copy->elevation == 2.0);
    CHECK(wall_copy->end == Point2D{10, 0});
    CHECK(levels_copy.size() == 1);
    CHECK(walls_copy.size() == 1);

    // New queries see exactly the newly committed state.
    CHECK(bim::query::FindLevel(document, Id(1))->elevation == 50.0);
    CHECK(bim::query::FindStraightWall(document, Id(10))->end == Point2D{3, 0});
    CHECK(bim::query::ListLevels(document).size() == 2);

    // A deletion is visible to a new query and to nothing taken before it.
    RequireOk(bim::commands::DeleteElement(document, Id(10)));
    CHECK_FALSE(bim::query::FindStraightWall(document, Id(10)).has_value());
    CHECK(wall_copy->id == Id(10));
}
