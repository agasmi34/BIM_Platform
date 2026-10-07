// P1-T003 Commands & Query Vertical Slice - integration proof of the whole
// slice (Implementation Brief P1-T003-IB sections 11 and 12): commands mutate a
// bim::document::Document, queries read it back, and the neutral derived wall
// geometry follows. Links bim::commands, bim::query (and so bim::document) and
// Catch2 only; it uses the public APIs exclusively and never sees the
// document's private runtime machinery.

#include "bim/commands/commands.hpp"
#include "bim/query/query.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

using bim::commands::CommandResult;
using bim::commands::CommandResultCode;
using bim::document::Document;
using bim::geometry_api::LinearExtrusionSpec;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;

namespace {

ElementId Id(std::uint8_t last) {
    ElementId id;
    id.bytes[15] = last;
    return id;
}

Level MakeLevel(std::uint8_t id_last, double elevation) {
    return Level{.id = Id(id_last), .elevation = elevation};
}

StraightWall MakeWall(std::uint8_t id_last, std::uint8_t level_last, Point2D start, Point2D end,
                      double thickness = 0.5, double height = 3.0, double base_offset = 0.0) {
    return StraightWall{.id = Id(id_last),
                        .level_id = Id(level_last),
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

constexpr double kHuge = 1.0e308;

void RequireOk(const CommandResult& result) {
    REQUIRE(result.code == CommandResultCode::Ok);
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

LinearExtrusionSpec Geometry(const Document& document, const ElementId& wall_id) {
    const auto found = document.FindWallGeometry(wall_id);
    REQUIRE(found.has_value());
    return *found;
}

// Complete observable state of a document, from public const reads only.
struct Snapshot {
    std::vector<Level> levels;
    std::vector<StraightWall> walls;
    std::vector<LinearExtrusionSpec> geometry; // one per listed wall, in list order
};

Snapshot TakeSnapshot(const Document& document) {
    Snapshot snapshot;
    snapshot.levels = bim::query::ListLevels(document);
    snapshot.walls = bim::query::ListStraightWalls(document);
    for (const StraightWall& wall : snapshot.walls) {
        snapshot.geometry.push_back(Geometry(document, wall.id));
    }
    return snapshot;
}

bool SameSnapshot(const Snapshot& a, const Snapshot& b) {
    if (a.levels != b.levels || a.walls != b.walls || a.geometry.size() != b.geometry.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.geometry.size(); ++i) {
        if (!SpecEqual(a.geometry[i], b.geometry[i])) {
            return false;
        }
    }
    return true;
}

// The same four-wall rectangle on the given Level, ids base..base+3.
void BuildRectangle(Document& document, std::uint8_t level_last, std::uint8_t first_wall) {
    RequireOk(bim::commands::CreateStraightWall(document,
                                                MakeWall(first_wall, level_last, {0, 0}, {10, 0})));
    RequireOk(bim::commands::CreateStraightWall(
        document,
        MakeWall(static_cast<std::uint8_t>(first_wall + 1), level_last, {10, 0}, {10, 6})));
    RequireOk(bim::commands::CreateStraightWall(
        document,
        MakeWall(static_cast<std::uint8_t>(first_wall + 2), level_last, {10, 6}, {0, 6})));
    RequireOk(bim::commands::CreateStraightWall(
        document, MakeWall(static_cast<std::uint8_t>(first_wall + 3), level_last, {0, 6}, {0, 0})));
}

} // namespace

TEST_CASE("P1-T003: a two-storey model is authored, queried, edited and trimmed by commands",
          "[integration][commands][query][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 3.0))); // inserted out of order
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    BuildRectangle(document, 1, 10); // walls 10..13 on Level 1
    BuildRectangle(document, 2, 20); // walls 20..23 on Level 2

    // Query: both Levels in ElementId order, all eight walls in ElementId order.
    const std::vector<Level> levels = bim::query::ListLevels(document);
    REQUIRE(levels.size() == 2);
    CHECK(levels[0].id == Id(1));
    CHECK(levels[1].id == Id(2));
    const std::vector<StraightWall> walls = bim::query::ListStraightWalls(document);
    REQUIRE(walls.size() == 8);
    CHECK(std::is_sorted(walls.begin(), walls.end(),
                         [](const StraightWall& a, const StraightWall& b) { return a.id < b.id; }));
    CHECK(walls.front().id == Id(10));
    CHECK(walls.back().id == Id(23));

    // The neutral geometry of the first Level-1 wall: along +X, base at z = 0.
    const LinearExtrusionSpec ground = Geometry(document, Id(10));
    CHECK(ground.profile.origin.z == 0.0);
    CHECK(ground.profile.size_u == 10.0);
    CHECK(ground.direction.z == 1.0);
    CHECK(ground.distance == 3.0);
    CHECK(Geometry(document, Id(20)).profile.origin.z == 3.0);

    // Raising Level 1 lifts exactly its four walls.
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 1.5));
    for (const std::uint8_t wall : {10, 11, 12, 13}) {
        CHECK(Geometry(document, Id(wall)).profile.origin.z == 1.5);
    }
    for (const std::uint8_t wall : {20, 21, 22, 23}) {
        CHECK(Geometry(document, Id(wall)).profile.origin.z == 3.0);
    }

    // Lengthening one wall changes only that wall's geometry.
    const LinearExtrusionSpec neighbour = Geometry(document, Id(11));
    RequireOk(bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{0, 0},
                                                        Point2D{14, 0}, 0.5, 0.0, 3.0));
    CHECK(Geometry(document, Id(10)).profile.size_u == 14.0);
    CHECK(SpecEqual(neighbour, Geometry(document, Id(11))));
    CHECK(bim::query::FindStraightWall(document, Id(10))->level_id == Id(1));

    // Trim: a Level cannot go while it hosts walls; walls can; then the Level can.
    CHECK(bim::commands::DeleteElement(document, Id(2)).code ==
          CommandResultCode::ElementHasDependents);
    for (const std::uint8_t wall : {20, 21, 22, 23}) {
        RequireOk(bim::commands::DeleteElement(document, Id(wall)));
    }
    RequireOk(bim::commands::DeleteElement(document, Id(2)));

    CHECK(bim::query::ListLevels(document).size() == 1);
    CHECK(bim::query::ListStraightWalls(document).size() == 4);
    CHECK_FALSE(bim::query::FindLevel(document, Id(2)).has_value());
    CHECK_FALSE(document.FindWallGeometry(Id(20)).has_value());

    // The surviving storey is still fully live after the trim.
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), -2.0));
    for (const std::uint8_t wall : {10, 11, 12, 13}) {
        CHECK(Geometry(document, Id(wall)).profile.origin.z == -2.0);
    }
}

TEST_CASE("P1-T003: every failed command is invisible to later queries",
          "[integration][commands][query][p1-t003]") {
    Document document;
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(1, 0.0)));
    RequireOk(bim::commands::CreateLevel(document, MakeLevel(2, 6.0)));
    BuildRectangle(document, 1, 10);
    RequireOk(bim::commands::CreateStraightWall(document, MakeWall(30, 2, {0, 0}, {8, 0})));

    const Snapshot committed = TakeSnapshot(document);

    struct Attempt {
        CommandResult result;
        CommandResultCode expected;
    };
    const std::vector<Attempt> attempts = {
        {bim::commands::CreateLevel(document, Level{ElementId{}, 1.0}),
         CommandResultCode::InvalidElement},
        {bim::commands::CreateLevel(document, MakeLevel(2, 99.0)),
         CommandResultCode::DuplicateElementId},
        {bim::commands::CreateStraightWall(document, MakeWall(40, 77, {0, 0}, {1, 0})),
         CommandResultCode::LevelNotFound},
        {bim::commands::CreateStraightWall(document, MakeWall(30, 1, {0, 0}, {1, 0})),
         CommandResultCode::DuplicateElementId},
        {bim::commands::CreateStraightWall(document, MakeWall(41, 1, {-kHuge, 0}, {kHuge, 0})),
         CommandResultCode::DerivedGeometryInvalid},
        {bim::commands::ChangeLevelElevation(document, Id(77), 1.0),
         CommandResultCode::ElementNotFound},
        {bim::commands::ChangeLevelElevation(document, Id(30), 1.0),
         CommandResultCode::ElementKindMismatch},
        {bim::commands::ChangeStraightWallGeometry(document, Id(1), Point2D{0, 0}, Point2D{1, 0},
                                                   0.5, 0.0, 3.0),
         CommandResultCode::ElementKindMismatch},
        {bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{2, 2}, Point2D{2, 2},
                                                   0.5, 0.0, 3.0),
         CommandResultCode::InvalidElement},
        {bim::commands::ChangeStraightWallGeometry(document, Id(10), Point2D{-kHuge, 0},
                                                   Point2D{kHuge, 0}, 0.5, 0.0, 3.0),
         CommandResultCode::DerivedGeometryInvalid},
        {bim::commands::DeleteElement(document, Id(77)), CommandResultCode::ElementNotFound},
        {bim::commands::DeleteElement(document, Id(1)), CommandResultCode::ElementHasDependents},
        {bim::commands::DeleteElement(document, Id(2)), CommandResultCode::ElementHasDependents},
    };

    for (const Attempt& attempt : attempts) {
        CHECK(attempt.result.code == attempt.expected);
        // After each rejected command the committed state is exactly the same.
        CHECK(SameSnapshot(committed, TakeSnapshot(document)));
    }

    // A rejection also does not poison later successful commands.
    RequireOk(bim::commands::ChangeLevelElevation(document, Id(1), 2.0));
    CHECK(Geometry(document, Id(12)).profile.origin.z == 2.0);
    CHECK(Geometry(document, Id(30)).profile.origin.z == 6.0);
}

TEST_CASE("P1-T003: the committed model does not depend on how it was reached",
          "[integration][commands][query][p1-t003]") {
    // Direct: build the final model in one pass.
    Document direct;
    RequireOk(bim::commands::CreateLevel(direct, MakeLevel(1, 4.0)));
    RequireOk(bim::commands::CreateLevel(direct, MakeLevel(2, 8.0)));
    RequireOk(bim::commands::CreateStraightWall(direct, MakeWall(10, 1, {0, 0}, {6, 0})));
    RequireOk(bim::commands::CreateStraightWall(direct, MakeWall(11, 1, {0, 3}, {6, 3})));
    RequireOk(bim::commands::CreateStraightWall(direct, MakeWall(20, 2, {0, 0}, {5, 0})));

    // Detour: reach the same model through creates in another order, a scratch
    // Level and scratch walls that are later deleted, edits that are later
    // overwritten, and rejected commands in between.
    Document detour;
    RequireOk(bim::commands::CreateLevel(detour, MakeLevel(2, 0.0)));
    RequireOk(bim::commands::CreateLevel(detour, MakeLevel(9, 50.0))); // scratch Level
    RequireOk(bim::commands::CreateLevel(detour, MakeLevel(1, 1.0)));
    RequireOk(bim::commands::CreateStraightWall(detour, MakeWall(20, 2, {0, 0}, {1, 0})));
    RequireOk(bim::commands::CreateStraightWall(detour, MakeWall(11, 1, {7, 7}, {8, 8})));
    RequireOk(bim::commands::CreateStraightWall(
        detour, MakeWall(10, 9, {0, 0}, {2, 0}))); // wrong host, scratch
    RequireOk(
        bim::commands::CreateStraightWall(detour, MakeWall(12, 9, {1, 1}, {2, 2}))); // scratch wall
    CHECK(bim::commands::DeleteElement(detour, Id(9)).code ==
          CommandResultCode::ElementHasDependents);
    RequireOk(bim::commands::DeleteElement(detour, Id(10))); // the wrong-host wall goes
    RequireOk(bim::commands::DeleteElement(detour, Id(12)));
    RequireOk(bim::commands::DeleteElement(detour, Id(9))); // the scratch Level goes
    RequireOk(bim::commands::CreateStraightWall(detour, MakeWall(10, 1, {0, 0}, {6, 0})));
    CHECK_FALSE(bim::commands::CreateStraightWall(detour, MakeWall(10, 2, {0, 0}, {6, 0})).ok());
    RequireOk(bim::commands::ChangeStraightWallGeometry(detour, Id(11), Point2D{0, 3},
                                                        Point2D{6, 3}, 0.5, 0.0, 3.0));
    RequireOk(bim::commands::ChangeStraightWallGeometry(detour, Id(20), Point2D{0, 0},
                                                        Point2D{5, 0}, 0.5, 0.0, 3.0));
    RequireOk(bim::commands::ChangeLevelElevation(detour, Id(1), 4.0));
    RequireOk(bim::commands::ChangeLevelElevation(detour, Id(2), 8.0));

    // Same committed values, same order, same derived geometry.
    CHECK(SameSnapshot(TakeSnapshot(direct), TakeSnapshot(detour)));
    CHECK(bim::query::ListLevels(detour).size() == 2);
    CHECK(bim::query::ListStraightWalls(detour).size() == 3);
}
