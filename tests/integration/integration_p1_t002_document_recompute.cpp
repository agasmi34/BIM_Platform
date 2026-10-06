// P1-T002 Document Runtime & Dependency Recompute - multi-wall Level scenario
// (Implementation Brief P1-T002-IB section 18). Links bim::document and
// Catch2 only and uses the public document API exclusively: it never touches
// the private runtime identity or dependency graph, so every claim below is
// about committed, observable document values.

#include "bim/document/document.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

using bim::document::Document;
using bim::document::DocumentResult;
using bim::document::DocumentResultCode;
using bim::geometry_api::LinearExtrusionSpec;
using bim::model::ElementId;
using bim::model::GenerateElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;

namespace {

constexpr double kHuge = 1.0e308; // finite, but doubling it overflows

ElementId NewId() {
    const std::optional<ElementId> id = GenerateElementId();
    REQUIRE(id.has_value());
    return *id;
}

StraightWall MakeWall(const ElementId& id, const ElementId& level_id, Point2D start, Point2D end,
                      double thickness = 0.5, double height = 3.0, double base_offset = 0.25) {
    return StraightWall{.id = id,
                        .level_id = level_id,
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

// Reads a committed value that the test requires to exist, so a missing element
// fails the test cleanly instead of dereferencing an empty optional.
Level GetLevel(const Document& document, const ElementId& id) {
    const auto found = document.FindLevel(id);
    REQUIRE(found.has_value());
    return *found;
}

StraightWall GetWall(const Document& document, const ElementId& id) {
    const auto found = document.FindStraightWall(id);
    REQUIRE(found.has_value());
    return *found;
}

LinearExtrusionSpec GetGeometry(const Document& document, const ElementId& id) {
    const auto found = document.FindWallGeometry(id);
    REQUIRE(found.has_value());
    return *found;
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

// Everything observable about a fixed set of elements, captured through the
// public API only.
struct Observed {
    std::optional<Level> level;
    std::vector<std::optional<StraightWall>> walls;
    std::vector<std::optional<LinearExtrusionSpec>> geometry;
};

Observed Observe(const Document& document, const ElementId& level_id,
                 const std::vector<ElementId>& wall_ids) {
    Observed observed;
    observed.level = document.FindLevel(level_id);
    for (const ElementId& id : wall_ids) {
        observed.walls.push_back(document.FindStraightWall(id));
        observed.geometry.push_back(document.FindWallGeometry(id));
    }
    return observed;
}

bool SameObserved(const Observed& a, const Observed& b) {
    if (a.level.has_value() != b.level.has_value() || (a.level && !(*a.level == *b.level))) {
        return false;
    }
    if (a.walls.size() != b.walls.size() || a.geometry.size() != b.geometry.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.walls.size(); ++i) {
        if (a.walls[i].has_value() != b.walls[i].has_value() ||
            (a.walls[i] && !(*a.walls[i] == *b.walls[i]))) {
            return false;
        }
        if (a.geometry[i].has_value() != b.geometry[i].has_value() ||
            (a.geometry[i] && !SpecEqual(*a.geometry[i], *b.geometry[i]))) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("P1-T002: a Level with several hosted walls recomputes deterministically",
          "[integration][document][p1-t002]") {
    const ElementId level_id = NewId();
    const std::array<ElementId, 3> wall_ids = {NewId(), NewId(), NewId()};
    const std::vector<ElementId> all_walls(wall_ids.begin(), wall_ids.end());

    Document document;
    REQUIRE(document.AddLevel(Level{.id = level_id, .elevation = 3.0}).ok());

    // Three walls forming an open U, all hosted by the one Level.
    const StraightWall south = MakeWall(wall_ids[0], level_id, {0.0, 0.0}, {10.0, 0.0});
    const StraightWall east = MakeWall(wall_ids[1], level_id, {10.0, 0.0}, {10.0, 8.0});
    const StraightWall north =
        MakeWall(wall_ids[2], level_id, {10.0, 8.0}, {0.0, 8.0}, 0.5, 3.0, 0.0);
    REQUIRE(document.AddStraightWall(south).ok());
    REQUIRE(document.AddStraightWall(east).ok());
    REQUIRE(document.AddStraightWall(north).ok());

    SECTION("initial neutral geometry is derived for every wall") {
        // south: U=(1,0,0) V=(0,1,0) -> SideA offset (0,-0.25); base_z = 3 + 0.25.
        const auto g_south = document.FindWallGeometry(south.id);
        REQUIRE(g_south.has_value());
        CHECK(g_south->profile.origin.x == 0.0);
        CHECK(g_south->profile.origin.y == -0.25);
        CHECK(g_south->profile.origin.z == 3.25);
        CHECK(g_south->profile.size_u == 10.0);
        CHECK(g_south->profile.size_v == 0.5);
        CHECK(g_south->direction.z == 1.0);
        CHECK(g_south->distance == 3.0);

        // east: U=(0,1,0) V=(-1,0,0) -> SideA offset (+0.25,0); starts at x=10.
        const auto g_east = document.FindWallGeometry(east.id);
        REQUIRE(g_east.has_value());
        CHECK(g_east->profile.origin.x == 10.25);
        CHECK(g_east->profile.origin.y == 0.0);
        CHECK(g_east->profile.origin.z == 3.25);
        CHECK(g_east->profile.size_u == 8.0);

        // north: U=(-1,0,0) V=(0,-1,0) -> SideA offset (0,+0.25); base_offset 0.
        const auto g_north = document.FindWallGeometry(north.id);
        REQUIRE(g_north.has_value());
        CHECK(g_north->profile.origin.x == 10.0);
        CHECK(g_north->profile.origin.y == 8.25);
        CHECK(g_north->profile.origin.z == 3.0);
        CHECK(g_north->profile.size_u == 10.0);
    }

    SECTION("a Level elevation change recomputes every hosted wall") {
        REQUIRE(document.UpdateLevelElevation(level_id, 6.0).ok());
        CHECK(GetLevel(document, level_id).elevation == 6.0);
        CHECK(GetGeometry(document, south.id).profile.origin.z == 6.25);
        CHECK(GetGeometry(document, east.id).profile.origin.z == 6.25);
        CHECK(GetGeometry(document, north.id).profile.origin.z == 6.0);

        // Only base Z moved.
        CHECK(GetGeometry(document, south.id).profile.origin.y == -0.25);
        CHECK(GetGeometry(document, east.id).profile.origin.x == 10.25);
        CHECK(GetGeometry(document, north.id).profile.origin.y == 8.25);
    }

    SECTION("updating one wall leaves the others exactly as they were") {
        const auto south_before = GetGeometry(document, south.id);
        const auto north_before = GetGeometry(document, north.id);
        const auto east_before = GetGeometry(document, east.id);

        const StraightWall longer_east =
            MakeWall(east.id, level_id, {10.0, 0.0}, {10.0, 20.0}, 1.0, 4.5, 0.5);
        REQUIRE(document.UpdateStraightWall(longer_east).ok());

        const auto east_after = GetGeometry(document, east.id);
        CHECK_FALSE(SpecEqual(east_after, east_before));
        CHECK(east_after.profile.size_u == 20.0);
        CHECK(east_after.profile.size_v == 1.0);
        CHECK(east_after.profile.origin.x == 10.5); // SideA of a +Y wall is on +X
        CHECK(east_after.profile.origin.z == 3.5);
        CHECK(east_after.distance == 4.5);

        CHECK(SpecEqual(GetGeometry(document, south.id), south_before));
        CHECK(SpecEqual(GetGeometry(document, north.id), north_before));
        CHECK(GetWall(document, south.id) == south);
        CHECK(GetWall(document, north.id) == north);
        CHECK(GetWall(document, east.id) == longer_east);
    }

    SECTION("durable ElementIds are stable across every successful mutation") {
        const Observed first = Observe(document, level_id, all_walls);
        REQUIRE(document.UpdateLevelElevation(level_id, -1.5).ok());
        REQUIRE(document.UpdateStraightWall(MakeWall(east.id, level_id, {10.0, 0.0}, {10.0, 12.0}))
                    .ok());
        REQUIRE(document.UpdateLevelElevation(level_id, 2.0).ok());
        const Observed last = Observe(document, level_id, all_walls);

        REQUIRE(last.level.has_value());
        CHECK(last.level->id == level_id);
        for (std::size_t i = 0; i < wall_ids.size(); ++i) {
            REQUIRE(last.walls[i].has_value());
            CHECK(last.walls[i]->id == wall_ids[i]);
            CHECK(last.walls[i]->level_id == level_id);
            CHECK(first.walls[i]->id == last.walls[i]->id);
        }
    }

    SECTION("repeated successful mutations keep every wall consistent with its Level") {
        double elevation = 3.0;
        for (int step = 0; step < 20; ++step) {
            elevation = (step % 2 == 0) ? elevation + 1.5 : elevation - 0.5;
            REQUIRE(document.UpdateLevelElevation(level_id, elevation).ok());
            CHECK(GetGeometry(document, south.id).profile.origin.z == elevation + 0.25);
            CHECK(GetGeometry(document, east.id).profile.origin.z == elevation + 0.25);
            CHECK(GetGeometry(document, north.id).profile.origin.z == elevation + 0.0);

            if (step % 5 == 0) {
                const double reach = 10.0 + static_cast<double>(step);
                REQUIRE(
                    document
                        .UpdateStraightWall(MakeWall(south.id, level_id, {0.0, 0.0}, {reach, 0.0}))
                        .ok());
                CHECK(GetGeometry(document, south.id).profile.size_u == reach);
                CHECK(GetGeometry(document, south.id).profile.origin.z == elevation + 0.25);
            }
        }
        CHECK(GetLevel(document, level_id).elevation == elevation);
    }

    SECTION("rejected mutations leave no partial committed state") {
        const Observed before = Observe(document, level_id, all_walls);

        // Overflow in one hosted wall's update.
        const DocumentResult overflow_update =
            document.UpdateStraightWall(MakeWall(south.id, level_id, {-kHuge, 0.0}, {kHuge, 0.0}));
        CHECK(overflow_update.code == DocumentResultCode::DerivedGeometryInvalid);
        CHECK(SameObserved(before, Observe(document, level_id, all_walls)));

        // Re-hosting onto another real Level.
        const ElementId other_level = NewId();
        REQUIRE(document.AddLevel(Level{.id = other_level, .elevation = 50.0}).ok());
        const DocumentResult rehost =
            document.UpdateStraightWall(MakeWall(east.id, other_level, {10.0, 0.0}, {10.0, 8.0}));
        CHECK(rehost.code == DocumentResultCode::RehostNotAllowed);
        CHECK(SameObserved(before, Observe(document, level_id, all_walls)));

        // A duplicate wall id and a non-finite Level elevation.
        CHECK(document.AddStraightWall(MakeWall(north.id, level_id, {0.0, 0.0}, {1.0, 0.0})).code ==
              DocumentResultCode::DuplicateElementId);
        CHECK(document.UpdateLevelElevation(level_id, std::numeric_limits<double>::quiet_NaN())
                  .code == DocumentResultCode::InvalidElement);
        CHECK(SameObserved(before, Observe(document, level_id, all_walls)));

        // A new wall whose derived geometry overflows leaves nothing behind.
        const ElementId bad_wall = NewId();
        CHECK(document.AddStraightWall(MakeWall(bad_wall, level_id, {-kHuge, 0.0}, {kHuge, 0.0}))
                  .code == DocumentResultCode::DerivedGeometryInvalid);
        CHECK_FALSE(document.FindStraightWall(bad_wall).has_value());
        CHECK_FALSE(document.FindWallGeometry(bad_wall).has_value());
        CHECK(SameObserved(before, Observe(document, level_id, all_walls)));

        // The same id is still free, and a valid mutation still works afterwards.
        REQUIRE(
            document.AddStraightWall(MakeWall(bad_wall, level_id, {0.0, 0.0}, {4.0, 0.0})).ok());
        REQUIRE(document.UpdateLevelElevation(level_id, 9.0).ok());
        CHECK(GetGeometry(document, bad_wall).profile.origin.z == 9.25);
        CHECK(GetGeometry(document, south.id).profile.origin.z == 9.25);
    }

    SECTION("a Level change that overflows one hosted wall rejects the whole recompute") {
        // A fourth wall that only survives near elevation 3.
        const ElementId edge_wall = NewId();
        REQUIRE(document
                    .AddStraightWall(
                        MakeWall(edge_wall, level_id, {0.0, 20.0}, {5.0, 20.0}, 0.5, 3.0, kHuge))
                    .ok());
        std::vector<ElementId> four_walls = all_walls;
        four_walls.push_back(edge_wall);
        const Observed before = Observe(document, level_id, four_walls);

        CHECK(document.UpdateLevelElevation(level_id, kHuge).code ==
              DocumentResultCode::DerivedGeometryInvalid);
        CHECK(SameObserved(before, Observe(document, level_id, four_walls)));

        // The healthy walls were not partially recomputed.
        CHECK(GetLevel(document, level_id).elevation == 3.0);
        CHECK(GetGeometry(document, south.id).profile.origin.z == 3.25);
    }
}

TEST_CASE("P1-T002: two Levels are independent recompute domains",
          "[integration][document][p1-t002]") {
    const ElementId ground = NewId();
    const ElementId upper = NewId();
    const ElementId ground_wall = NewId();
    const ElementId upper_wall = NewId();

    Document document;
    REQUIRE(document.AddLevel(Level{.id = ground, .elevation = 0.0}).ok());
    REQUIRE(document.AddLevel(Level{.id = upper, .elevation = 3.0}).ok());
    REQUIRE(document.AddStraightWall(MakeWall(ground_wall, ground, {0.0, 0.0}, {6.0, 0.0})).ok());
    REQUIRE(document.AddStraightWall(MakeWall(upper_wall, upper, {0.0, 0.0}, {6.0, 0.0})).ok());

    const auto upper_before = GetGeometry(document, upper_wall);
    REQUIRE(document.UpdateLevelElevation(ground, 1.0).ok());
    CHECK(GetGeometry(document, ground_wall).profile.origin.z == 1.25);
    CHECK(SpecEqual(GetGeometry(document, upper_wall), upper_before));

    const auto ground_before = GetGeometry(document, ground_wall);
    REQUIRE(document.UpdateLevelElevation(upper, 4.0).ok());
    CHECK(GetGeometry(document, upper_wall).profile.origin.z == 4.25);
    CHECK(SpecEqual(GetGeometry(document, ground_wall), ground_before));
}

TEST_CASE("P1-T002: the final derived geometry does not depend on the order walls were added",
          "[integration][document][p1-t002]") {
    const ElementId level_id = NewId();
    const std::array<ElementId, 4> ids = {NewId(), NewId(), NewId(), NewId()};
    const std::array<StraightWall, 4> walls = {
        MakeWall(ids[0], level_id, {0.0, 0.0}, {10.0, 0.0}),
        MakeWall(ids[1], level_id, {10.0, 0.0}, {10.0, 8.0}),
        MakeWall(ids[2], level_id, {10.0, 8.0}, {0.0, 8.0}),
        MakeWall(ids[3], level_id, {0.0, 8.0}, {0.0, 0.0}),
    };
    const std::vector<ElementId> id_list(ids.begin(), ids.end());

    const auto build = [&](const std::array<std::size_t, 4>& order) {
        Document document;
        REQUIRE(document.AddLevel(Level{.id = level_id, .elevation = 3.0}).ok());
        for (const std::size_t index : order) {
            REQUIRE(document.AddStraightWall(walls[index]).ok());
        }
        REQUIRE(document.UpdateLevelElevation(level_id, 5.5).ok());
        REQUIRE(
            document.UpdateStraightWall(MakeWall(ids[2], level_id, {10.0, 8.0}, {0.0, 12.0})).ok());
        return Observe(document, level_id, id_list);
    };

    const Observed forward = build({0, 1, 2, 3});
    const Observed reverse = build({3, 2, 1, 0});
    const Observed shuffled = build({2, 0, 3, 1});
    CHECK(SameObserved(forward, reverse));
    CHECK(SameObserved(forward, shuffled));
}
