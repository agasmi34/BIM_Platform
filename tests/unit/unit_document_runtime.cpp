// P1-T002 Document Runtime & Dependency Recompute - unit proof of the public
// bim::document API (Implementation Brief P1-T002-IB section 17). Links
// bim::document and Catch2 only: it uses the public document API exclusively
// and has no access to the private runtime identity, graph or state, so every
// atomicity claim below is proved through observable committed values.

#include "bim/document/document.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

using bim::document::Document;
using bim::document::DocumentResult;
using bim::document::DocumentResultCode;
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

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr double kHuge = 1.0e308; // finite, but doubling it overflows

// --- compile-time contract ---------------------------------------------------

static_assert(std::is_trivially_copyable_v<DocumentResult>,
              "a result must be allocation-free to build");
static_assert(!std::is_copy_constructible_v<Document>);
static_assert(!std::is_copy_assignable_v<Document>);
static_assert(std::is_nothrow_default_constructible_v<Document>);
static_assert(std::is_nothrow_move_constructible_v<Document>);
static_assert(std::is_nothrow_move_assignable_v<Document>);
static_assert(noexcept(std::declval<Document&>().AddLevel(std::declval<const Level&>())));
static_assert(
    noexcept(std::declval<Document&>().AddStraightWall(std::declval<const StraightWall&>())));
static_assert(noexcept(
    std::declval<Document&>().UpdateLevelElevation(std::declval<const ElementId&>(), 0.0)));
static_assert(
    noexcept(std::declval<Document&>().UpdateStraightWall(std::declval<const StraightWall&>())));
static_assert(
    noexcept(std::declval<const Document&>().FindLevel(std::declval<const ElementId&>())));
static_assert(
    noexcept(std::declval<const Document&>().FindStraightWall(std::declval<const ElementId&>())));
static_assert(
    noexcept(std::declval<const Document&>().FindWallGeometry(std::declval<const ElementId&>())));

} // namespace

TEST_CASE("Document: an empty document answers every lookup with nothing",
          "[unit][document][p1-t002]") {
    const Document document;
    CHECK_FALSE(document.FindLevel(Id(1)).has_value());
    CHECK_FALSE(document.FindStraightWall(Id(1)).has_value());
    CHECK_FALSE(document.FindWallGeometry(Id(1)).has_value());

    // The all-zero (invalid) id never names anything either.
    CHECK_FALSE(document.FindLevel(ElementId{}).has_value());
    CHECK_FALSE(document.FindStraightWall(ElementId{}).has_value());
    CHECK_FALSE(document.FindWallGeometry(ElementId{}).has_value());
}

TEST_CASE("Document: a valid Level is added and read back as committed value data",
          "[unit][document][p1-t002]") {
    Document document;
    const Level level = MakeLevel(1, 4.0);

    const DocumentResult result = document.AddLevel(level);
    CHECK(result.ok());
    CHECK(result.code == DocumentResultCode::Ok);
    CHECK(result.validation == ValidationCode::Ok);

    const auto stored = document.FindLevel(level.id);
    REQUIRE(stored.has_value());
    CHECK(*stored == level);

    // A Level is not a wall and generates no geometry.
    CHECK_FALSE(document.FindStraightWall(level.id).has_value());
    CHECK_FALSE(document.FindWallGeometry(level.id).has_value());

    // Zero and negative elevations are valid.
    CHECK(document.AddLevel(MakeLevel(2, 0.0)).ok());
    CHECK(document.AddLevel(MakeLevel(3, -3.5)).ok());
    CHECK(GetLevel(document, Id(2)).elevation == 0.0);
    CHECK(GetLevel(document, Id(3)).elevation == -3.5);
}

TEST_CASE("Document: a duplicate Level id is rejected and the original is untouched",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());

    const DocumentResult result = document.AddLevel(MakeLevel(1, 99.0));
    CHECK_FALSE(result.ok());
    CHECK(result.code == DocumentResultCode::DuplicateElementId);

    REQUIRE(document.FindLevel(Id(1)).has_value());
    CHECK(GetLevel(document, Id(1)).elevation == 4.0);
}

TEST_CASE("Document: ElementIds are one namespace across Levels and StraightWalls",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 0.0)).ok());
    REQUIRE(document.AddLevel(MakeLevel(2, 0.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    const auto wall_geometry_before = document.FindWallGeometry(Id(10));
    REQUIRE(wall_geometry_before.has_value());

    // A wall may not reuse a Level's id (hosted on a different Level, so the
    // model-level "wall id equals its own level id" rule is not what fires).
    const DocumentResult wall_over_level =
        document.AddStraightWall(MakeWall(1, 2, {0.0, 0.0}, {5.0, 0.0}));
    CHECK(wall_over_level.code == DocumentResultCode::DuplicateElementId);
    CHECK_FALSE(document.FindStraightWall(Id(1)).has_value());

    // A Level may not reuse a wall's id.
    const DocumentResult level_over_wall = document.AddLevel(MakeLevel(10, 7.0));
    CHECK(level_over_wall.code == DocumentResultCode::DuplicateElementId);
    CHECK_FALSE(document.FindLevel(Id(10)).has_value());

    // A duplicate wall id is rejected too.
    const DocumentResult wall_over_wall =
        document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {99.0, 0.0}));
    CHECK(wall_over_wall.code == DocumentResultCode::DuplicateElementId);

    // Nothing changed.
    CHECK(GetLevel(document, Id(1)).elevation == 0.0);
    REQUIRE(document.FindStraightWall(Id(10)).has_value());
    CHECK(GetWall(document, Id(10)).end.x == 10.0);
    REQUIRE(document.FindWallGeometry(Id(10)).has_value());
    CHECK(SpecEqual(GetGeometry(document, Id(10)), *wall_geometry_before));
}

TEST_CASE("Document: a wall whose Level is missing is rejected with zero mutation",
          "[unit][document][p1-t002]") {
    Document document;
    const StraightWall wall = MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0});

    const DocumentResult result = document.AddStraightWall(wall);
    CHECK_FALSE(result.ok());
    CHECK(result.code == DocumentResultCode::LevelNotFound);
    CHECK_FALSE(document.FindStraightWall(wall.id).has_value());
    CHECK_FALSE(document.FindWallGeometry(wall.id).has_value());
    CHECK_FALSE(document.FindLevel(wall.level_id).has_value());

    // The rejection left no residue: the same wall is accepted once its Level exists.
    REQUIRE(document.AddLevel(MakeLevel(1, 0.0)).ok());
    CHECK(document.AddStraightWall(wall).ok());
    CHECK(document.FindStraightWall(wall.id).has_value());

    // A wall id is not a Level: hosting a wall on a wall is LevelNotFound.
    const DocumentResult on_wall =
        document.AddStraightWall(MakeWall(11, 10, {0.0, 0.0}, {1.0, 0.0}));
    CHECK(on_wall.code == DocumentResultCode::LevelNotFound);
    CHECK_FALSE(document.FindStraightWall(Id(11)).has_value());
}

TEST_CASE("Document: a valid wall registers, reads back and gets derived geometry",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    const StraightWall wall = MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0});

    const DocumentResult result = document.AddStraightWall(wall);
    CHECK(result.ok());

    const auto stored = document.FindStraightWall(wall.id);
    REQUIRE(stored.has_value());
    CHECK(*stored == wall);
    CHECK(document.FindWallGeometry(wall.id).has_value());
    CHECK_FALSE(document.FindLevel(wall.id).has_value());
}

TEST_CASE("Document: derived extrusion spec follows the frozen neutral semantics exactly",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());

    SECTION("wall along +X") {
        // S=(0,0) E=(10,0): D=(10,0) L=10 U=(1,0,0) W=(0,0,1) V=W x U=(0,1,0).
        // SideA = -V * 0.5/2 = (0,-0.25); base_z = 4.0 + 0.25.
        REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
        const auto spec = document.FindWallGeometry(Id(10));
        REQUIRE(spec.has_value());
        CHECK(spec->profile.origin.x == 0.0);
        CHECK(spec->profile.origin.y == -0.25);
        CHECK(spec->profile.origin.z == 4.25);
        CHECK(spec->profile.u_axis.x == 1.0);
        CHECK(spec->profile.u_axis.y == 0.0);
        CHECK(spec->profile.u_axis.z == 0.0);
        CHECK(spec->profile.v_axis.x == 0.0);
        CHECK(spec->profile.v_axis.y == 1.0);
        CHECK(spec->profile.v_axis.z == 0.0);
        CHECK(spec->profile.size_u == 10.0);
        CHECK(spec->profile.size_v == 0.5);
        CHECK(spec->direction.x == 0.0);
        CHECK(spec->direction.y == 0.0);
        CHECK(spec->direction.z == 1.0);
        CHECK(spec->distance == 3.0);
    }

    SECTION("wall along -Y, offset start point") {
        // S=(2,1) E=(2,-7): D=(0,-8) L=8 U=(0,-1,0) V=W x U=(1,0,0).
        // SideA = -V * 1/2 = (-0.5, 0): origin = (2-0.5, 1).
        REQUIRE(document.AddStraightWall(MakeWall(10, 1, {2.0, 1.0}, {2.0, -7.0}, 1.0, 2.5, -1.0))
                    .ok());
        const auto spec = document.FindWallGeometry(Id(10));
        REQUIRE(spec.has_value());
        CHECK(spec->profile.origin.x == 1.5);
        CHECK(spec->profile.origin.y == 1.0);
        CHECK(spec->profile.origin.z == 3.0); // 4.0 + (-1.0)
        CHECK(spec->profile.u_axis.x == 0.0);
        CHECK(spec->profile.u_axis.y == -1.0);
        CHECK(spec->profile.v_axis.x == 1.0);
        CHECK(spec->profile.v_axis.y == 0.0);
        CHECK(spec->profile.size_u == 8.0);
        CHECK(spec->profile.size_v == 1.0);
        CHECK(spec->direction.z == 1.0);
        CHECK(spec->distance == 2.5);
    }

    SECTION("diagonal wall: orthonormal right-handed frame, SideA on the -V side") {
        // S=(1,1) E=(4,5): D=(3,4) L=5 U=(0.6,0.8) V=(-0.8,0.6).
        REQUIRE(
            document.AddStraightWall(MakeWall(10, 1, {1.0, 1.0}, {4.0, 5.0}, 0.5, 3.0, 0.0)).ok());
        const auto spec = document.FindWallGeometry(Id(10));
        REQUIRE(spec.has_value());
        const auto& p = spec->profile;
        CHECK(p.size_u == Catch::Approx(5.0).margin(1e-12));
        CHECK(p.size_v == 0.5);
        CHECK(p.u_axis.x == Catch::Approx(0.6).margin(1e-12));
        CHECK(p.u_axis.y == Catch::Approx(0.8).margin(1e-12));
        CHECK(p.u_axis.z == 0.0);
        CHECK(p.v_axis.x == Catch::Approx(-0.8).margin(1e-12));
        CHECK(p.v_axis.y == Catch::Approx(0.6).margin(1e-12));
        CHECK(p.v_axis.z == 0.0);
        // U x V = W, with W = +Z.
        CHECK(p.u_axis.x * p.v_axis.y - p.u_axis.y * p.v_axis.x ==
              Catch::Approx(1.0).margin(1e-12));
        // origin = S - V * thickness / 2.
        CHECK(p.origin.x == Catch::Approx(1.0 - (-0.8) * 0.25).margin(1e-12));
        CHECK(p.origin.y == Catch::Approx(1.0 - 0.6 * 0.25).margin(1e-12));
        CHECK(p.origin.z == 4.0);
        CHECK(spec->direction.x == 0.0);
        CHECK(spec->direction.y == 0.0);
        CHECK(spec->direction.z == 1.0);
        CHECK(spec->distance == 3.0);
    }

    SECTION("a huge but representable axis is accepted: no spurious intermediate overflow") {
        // dx*dx + dy*dy would overflow a naive sqrt(dx*dx + dy*dy), yet the length
        // 1.414...e200 and every other derived value is representable.
        const StraightWall wall = MakeWall(10, 1, {0.0, 0.0}, {1.0e200, 1.0e200});
        REQUIRE(bim::model::ValidateStraightWall(wall).ok());
        REQUIRE(document.AddStraightWall(wall).ok());
        const auto spec = document.FindWallGeometry(Id(10));
        REQUIRE(spec.has_value());
        CHECK(spec->profile.size_u == Catch::Approx(std::sqrt(2.0) * 1.0e200).epsilon(1e-12));
        CHECK(spec->profile.u_axis.x == Catch::Approx(std::sqrt(0.5)).epsilon(1e-12));
        CHECK(spec->profile.u_axis.y == Catch::Approx(std::sqrt(0.5)).epsilon(1e-12));
    }

    SECTION("a very short but non-zero axis is accepted: there is no hidden epsilon") {
        REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {1.0e-9, 0.0})).ok());
        const auto spec = document.FindWallGeometry(Id(10));
        REQUIRE(spec.has_value());
        CHECK(spec->profile.size_u == 1.0e-9);
        CHECK(spec->profile.u_axis.x == 1.0);
    }
}

TEST_CASE("Document: changing a Level elevation recomputes the base Z of every hosted wall",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddLevel(MakeLevel(2, 100.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    REQUIRE(document.AddStraightWall(MakeWall(11, 1, {0.0, 5.0}, {10.0, 5.0}, 0.5, 3.0, 1.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(12, 2, {0.0, 9.0}, {10.0, 9.0})).ok());

    const auto before_a = GetGeometry(document, Id(10));
    const auto before_b = GetGeometry(document, Id(11));
    const auto before_other_level = GetGeometry(document, Id(12));
    REQUIRE(before_a.profile.origin.z == 4.25);
    REQUIRE(before_b.profile.origin.z == 5.0);

    const DocumentResult result = document.UpdateLevelElevation(Id(1), 10.0);
    CHECK(result.ok());

    // The committed Level value changed.
    CHECK(GetLevel(document, Id(1)).elevation == 10.0);
    CHECK(GetLevel(document, Id(2)).elevation == 100.0);

    // Both hosted walls recomputed: only base Z moved, everything else is as before.
    const auto after_a = GetGeometry(document, Id(10));
    const auto after_b = GetGeometry(document, Id(11));
    CHECK(after_a.profile.origin.z == 10.25);
    CHECK(after_b.profile.origin.z == 11.0);
    CHECK(after_a.profile.origin.x == before_a.profile.origin.x);
    CHECK(after_a.profile.origin.y == before_a.profile.origin.y);
    CHECK(after_a.profile.size_u == before_a.profile.size_u);
    CHECK(after_a.distance == before_a.distance);
    CHECK(after_b.profile.origin.y == before_b.profile.origin.y);

    // A wall on another Level is not affected.
    CHECK(SpecEqual(GetGeometry(document, Id(12)), before_other_level));

    // The wall values themselves are not touched by a Level change.
    CHECK(GetWall(document, Id(10)).base_offset == 0.25);
}

TEST_CASE("Document: a Level that hosts no wall updates with nothing to recompute",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddLevel(MakeLevel(2, 8.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 2, {0.0, 0.0}, {10.0, 0.0})).ok());
    const auto wall_geometry = GetGeometry(document, Id(10));

    CHECK(document.UpdateLevelElevation(Id(1), -2.0).ok()); // hosts nothing
    CHECK(GetLevel(document, Id(1)).elevation == -2.0);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), wall_geometry));

    // Setting the same value again is a valid no-op update.
    CHECK(document.UpdateLevelElevation(Id(2), 8.0).ok());
    CHECK(SpecEqual(GetGeometry(document, Id(10)), wall_geometry));
}

TEST_CASE("Document: updating one wall recomputes only that wall", "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    REQUIRE(document.AddStraightWall(MakeWall(11, 1, {0.0, 5.0}, {10.0, 5.0})).ok());
    const auto untouched = GetGeometry(document, Id(11));
    const auto old_geometry = GetGeometry(document, Id(10));

    const StraightWall changed = MakeWall(10, 1, {1.0, 0.0}, {1.0, 6.0}, 1.0, 4.0, 0.5);
    const DocumentResult result = document.UpdateStraightWall(changed);
    CHECK(result.ok());

    CHECK(GetWall(document, Id(10)) == changed);
    const auto updated = GetGeometry(document, Id(10));
    CHECK_FALSE(SpecEqual(updated, old_geometry));
    // S=(1,0) E=(1,6): U=(0,1,0), V=(-1,0,0), SideA = -V*0.5 -> origin x = 1+0.5.
    CHECK(updated.profile.origin.x == 1.5);
    CHECK(updated.profile.origin.y == 0.0);
    CHECK(updated.profile.origin.z == 4.5);
    CHECK(updated.profile.size_u == 6.0);
    CHECK(updated.profile.size_v == 1.0);
    CHECK(updated.distance == 4.0);

    // The sibling wall and the Level are exactly as they were.
    CHECK(SpecEqual(GetGeometry(document, Id(11)), untouched));
    CHECK(GetLevel(document, Id(1)).elevation == 4.0);
    CHECK(GetWall(document, Id(11)).end.x == 10.0);
}

TEST_CASE("Document: re-hosting a wall is rejected with zero mutation",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddLevel(MakeLevel(2, 20.0)).ok());
    const StraightWall original = MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0});
    REQUIRE(document.AddStraightWall(original).ok());
    const auto geometry_before = GetGeometry(document, Id(10));

    // Same defining geometry, different Level.
    const DocumentResult plain_rehost =
        document.UpdateStraightWall(MakeWall(10, 2, {0.0, 0.0}, {10.0, 0.0}));
    CHECK_FALSE(plain_rehost.ok());
    CHECK(plain_rehost.code == DocumentResultCode::RehostNotAllowed);

    // A re-host bundled with a geometry change is rejected as a whole.
    const DocumentResult bundled =
        document.UpdateStraightWall(MakeWall(10, 2, {0.0, 0.0}, {50.0, 0.0}));
    CHECK(bundled.code == DocumentResultCode::RehostNotAllowed);

    // Re-hosting onto a Level that does not exist is still a re-host.
    const DocumentResult to_missing =
        document.UpdateStraightWall(MakeWall(10, 9, {0.0, 0.0}, {10.0, 0.0}));
    CHECK(to_missing.code == DocumentResultCode::RehostNotAllowed);

    CHECK(GetWall(document, Id(10)) == original);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), geometry_before));

    // And the wall still follows its original Level, not the rejected one.
    REQUIRE(document.UpdateLevelElevation(Id(1), 6.0).ok());
    CHECK(GetGeometry(document, Id(10)).profile.origin.z == 6.25);
    REQUIRE(document.UpdateLevelElevation(Id(2), 99.0).ok());
    CHECK(GetGeometry(document, Id(10)).profile.origin.z == 6.25);
}

TEST_CASE("Document: update targets must exist and be of the right kind",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    const auto geometry_before = GetGeometry(document, Id(10));

    CHECK(document.UpdateLevelElevation(Id(77), 1.0).code == DocumentResultCode::ElementNotFound);
    CHECK(document.UpdateLevelElevation(Id(10), 1.0).code ==
          DocumentResultCode::ElementKindMismatch);
    CHECK(document.UpdateStraightWall(MakeWall(77, 1, {0.0, 0.0}, {1.0, 0.0})).code ==
          DocumentResultCode::ElementNotFound);
    CHECK(document.UpdateStraightWall(MakeWall(1, 1, {0.0, 0.0}, {1.0, 0.0})).code ==
          DocumentResultCode::ElementKindMismatch);

    // Nothing was created or changed by any of them.
    CHECK_FALSE(document.FindLevel(Id(77)).has_value());
    CHECK_FALSE(document.FindStraightWall(Id(77)).has_value());
    CHECK(GetLevel(document, Id(1)).elevation == 4.0);
    CHECK(GetWall(document, Id(10)).end.x == 10.0);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), geometry_before));
}

TEST_CASE(
    "Document: structurally invalid values are rejected with the model reason and zero mutation",
    "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    const auto geometry_before = GetGeometry(document, Id(10));

    SECTION("AddLevel") {
        const DocumentResult zero_id = document.AddLevel(Level{});
        CHECK(zero_id.code == DocumentResultCode::InvalidElement);
        CHECK(zero_id.validation == ValidationCode::InvalidElementId);

        const DocumentResult nan_elevation = document.AddLevel(MakeLevel(2, kNaN));
        CHECK(nan_elevation.code == DocumentResultCode::InvalidElement);
        CHECK(nan_elevation.validation == ValidationCode::NonFiniteElevation);
        CHECK_FALSE(document.FindLevel(Id(2)).has_value());

        CHECK(document.AddLevel(MakeLevel(2, kInf)).validation ==
              ValidationCode::NonFiniteElevation);
        CHECK_FALSE(document.FindLevel(Id(2)).has_value());
    }

    SECTION("AddStraightWall") {
        const DocumentResult coincident =
            document.AddStraightWall(MakeWall(11, 1, {3.0, 3.0}, {3.0, 3.0}));
        CHECK(coincident.code == DocumentResultCode::InvalidElement);
        CHECK(coincident.validation == ValidationCode::CoincidentEndpoints);

        CHECK(document.AddStraightWall(MakeWall(11, 1, {0.0, 0.0}, {1.0, 0.0}, 0.0)).validation ==
              ValidationCode::NonPositiveThickness);
        CHECK(document.AddStraightWall(MakeWall(11, 1, {0.0, 0.0}, {1.0, 0.0}, 0.5, -1.0))
                  .validation == ValidationCode::NonPositiveHeight);
        CHECK(document.AddStraightWall(MakeWall(11, 1, {0.0, 0.0}, {1.0, 0.0}, 0.5, 3.0, kNaN))
                  .validation == ValidationCode::NonFiniteBaseOffset);
        CHECK(document.AddStraightWall(MakeWall(11, 1, {kNaN, 0.0}, {1.0, 0.0})).validation ==
              ValidationCode::NonFiniteCoordinate);
        CHECK(document.AddStraightWall(MakeWall(0, 1, {0.0, 0.0}, {1.0, 0.0})).validation ==
              ValidationCode::InvalidElementId);
        CHECK(document.AddStraightWall(MakeWall(11, 0, {0.0, 0.0}, {1.0, 0.0})).validation ==
              ValidationCode::InvalidLevelReference);
        CHECK(document.AddStraightWall(MakeWall(1, 1, {0.0, 0.0}, {1.0, 0.0})).validation ==
              ValidationCode::WallIdEqualsLevelId);
        CHECK_FALSE(document.FindStraightWall(Id(11)).has_value());
        CHECK_FALSE(document.FindWallGeometry(Id(11)).has_value());
    }

    SECTION("UpdateLevelElevation") {
        const DocumentResult nan_result = document.UpdateLevelElevation(Id(1), kNaN);
        CHECK(nan_result.code == DocumentResultCode::InvalidElement);
        CHECK(nan_result.validation == ValidationCode::NonFiniteElevation);
        CHECK(document.UpdateLevelElevation(Id(1), -kInf).validation ==
              ValidationCode::NonFiniteElevation);
        CHECK(GetLevel(document, Id(1)).elevation == 4.0);
    }

    SECTION("UpdateStraightWall") {
        const DocumentResult invalid =
            document.UpdateStraightWall(MakeWall(10, 1, {2.0, 2.0}, {2.0, 2.0}));
        CHECK(invalid.code == DocumentResultCode::InvalidElement);
        CHECK(invalid.validation == ValidationCode::CoincidentEndpoints);
        CHECK(
            document.UpdateStraightWall(MakeWall(10, 1, {0.0, 0.0}, {1.0, 0.0}, kNaN)).validation ==
            ValidationCode::NonFiniteThickness);
        CHECK(GetWall(document, Id(10)).end.x == 10.0);
    }

    // Whatever was attempted, the committed state is exactly the original.
    CHECK(GetLevel(document, Id(1)).elevation == 4.0);
    CHECK(GetWall(document, Id(10)).end.x == 10.0);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), geometry_before));
}

TEST_CASE("Document: derived-geometry overflow on a new wall is rejected with zero mutation",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());
    const auto geometry_before = GetGeometry(document, Id(10));

    // Each of these is structurally valid (finite, positive, distinct endpoints) so
    // it passes model validation, but a derived value overflows.
    SECTION("axis delta overflows") {
        const StraightWall wall = MakeWall(11, 1, {-kHuge, 0.0}, {kHuge, 0.0});
        REQUIRE(bim::model::ValidateStraightWall(wall).ok());
        const DocumentResult result = document.AddStraightWall(wall);
        CHECK_FALSE(result.ok());
        CHECK(result.code == DocumentResultCode::DerivedGeometryInvalid);
    }

    SECTION("axis length overflows although each delta is finite") {
        const StraightWall wall = MakeWall(11, 1, {0.0, 0.0}, {1.5e308, 1.5e308});
        REQUIRE(bim::model::ValidateStraightWall(wall).ok());
        CHECK(document.AddStraightWall(wall).code == DocumentResultCode::DerivedGeometryInvalid);
    }

    SECTION("profile origin offset overflows") {
        // Wall along +X: SideA offset is on Y. Y start is near the negative limit, so
        // subtracting the (positive) half-thickness offset overflows.
        const StraightWall wall = MakeWall(11, 1, {0.0, -1.7e308}, {10.0, -1.7e308}, 1.7e308);
        REQUIRE(bim::model::ValidateStraightWall(wall).ok());
        CHECK(document.AddStraightWall(wall).code == DocumentResultCode::DerivedGeometryInvalid);
    }

    SECTION("elevation plus base offset overflows") {
        REQUIRE(document.AddLevel(MakeLevel(2, 1.7e308)).ok());
        const StraightWall wall = MakeWall(11, 2, {0.0, 0.0}, {10.0, 0.0}, 0.5, 3.0, 1.7e308);
        REQUIRE(bim::model::ValidateStraightWall(wall).ok());
        CHECK(document.AddStraightWall(wall).code == DocumentResultCode::DerivedGeometryInvalid);
    }

    CHECK_FALSE(document.FindStraightWall(Id(11)).has_value());
    CHECK_FALSE(document.FindWallGeometry(Id(11)).has_value());
    CHECK(SpecEqual(GetGeometry(document, Id(10)), geometry_before));

    // The failed registration left no residue: the id is free and works.
    CHECK(document.AddStraightWall(MakeWall(11, 1, {0.0, 0.0}, {2.0, 0.0})).ok());
    CHECK(document.FindWallGeometry(Id(11)).has_value());
}

TEST_CASE("Document: a Level change that overflows any hosted wall is rejected as a whole",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 0.0)).ok());
    // Wall A is harmless at any elevation; wall B only survives at elevation 0
    // because its base offset is near the representable limit.
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0}, 0.5, 3.0, 0.0)).ok());
    REQUIRE(
        document.AddStraightWall(MakeWall(11, 1, {0.0, 5.0}, {10.0, 5.0}, 0.5, 3.0, kHuge)).ok());
    const auto a_before = GetGeometry(document, Id(10));
    const auto b_before = GetGeometry(document, Id(11));

    const DocumentResult result =
        document.UpdateLevelElevation(Id(1), kHuge); // B: 1e308 + 1e308 overflows
    CHECK_FALSE(result.ok());
    CHECK(result.code == DocumentResultCode::DerivedGeometryInvalid);

    // Neither the Level nor either wall's geometry moved - wall A was NOT
    // partially recomputed even though it would have succeeded on its own.
    CHECK(GetLevel(document, Id(1)).elevation == 0.0);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), a_before));
    CHECK(SpecEqual(GetGeometry(document, Id(11)), b_before));

    // The document is still fully usable with a survivable value.
    CHECK(document.UpdateLevelElevation(Id(1), 5.0).ok());
    CHECK(GetGeometry(document, Id(10)).profile.origin.z == 5.0);
    CHECK(GetGeometry(document, Id(11)).profile.origin.z == 5.0 + kHuge);
}

TEST_CASE("Document: a wall update that overflows is rejected and leaves the wall as it was",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    const StraightWall original = MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0});
    REQUIRE(document.AddStraightWall(original).ok());
    REQUIRE(document.AddStraightWall(MakeWall(11, 1, {0.0, 5.0}, {10.0, 5.0})).ok());
    const auto geometry_before = GetGeometry(document, Id(10));
    const auto sibling_before = GetGeometry(document, Id(11));

    const DocumentResult result =
        document.UpdateStraightWall(MakeWall(10, 1, {-kHuge, 0.0}, {kHuge, 0.0}));
    CHECK_FALSE(result.ok());
    CHECK(result.code == DocumentResultCode::DerivedGeometryInvalid);

    CHECK(GetWall(document, Id(10)) == original);
    CHECK(SpecEqual(GetGeometry(document, Id(10)), geometry_before));
    CHECK(SpecEqual(GetGeometry(document, Id(11)), sibling_before));

    // A subsequent valid update of the same wall succeeds.
    CHECK(document.UpdateStraightWall(MakeWall(10, 1, {0.0, 0.0}, {20.0, 0.0})).ok());
    CHECK(GetGeometry(document, Id(10)).profile.size_u == 20.0);
}

TEST_CASE("Document: committed state stays readable and mutable after many rejected operations",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());

    // A barrage of every rejection class.
    CHECK_FALSE(document.AddLevel(MakeLevel(1, 0.0)).ok());
    CHECK_FALSE(document.AddLevel(Level{}).ok());
    CHECK_FALSE(document.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {1.0, 0.0})).ok());
    CHECK_FALSE(document.AddStraightWall(MakeWall(12, 9, {0.0, 0.0}, {1.0, 0.0})).ok());
    CHECK_FALSE(document.AddStraightWall(MakeWall(12, 1, {-kHuge, 0.0}, {kHuge, 0.0})).ok());
    CHECK_FALSE(document.UpdateLevelElevation(Id(55), 1.0).ok());
    CHECK_FALSE(document.UpdateLevelElevation(Id(10), 1.0).ok());
    CHECK_FALSE(document.UpdateLevelElevation(Id(1), kNaN).ok());
    CHECK_FALSE(document.UpdateStraightWall(MakeWall(10, 7, {0.0, 0.0}, {1.0, 0.0})).ok());
    CHECK_FALSE(document.UpdateStraightWall(MakeWall(1, 1, {0.0, 0.0}, {1.0, 0.0})).ok());

    // Committed state is untouched and readable ...
    CHECK(GetLevel(document, Id(1)).elevation == 4.0);
    CHECK(GetWall(document, Id(10)).end.x == 10.0);
    CHECK(GetGeometry(document, Id(10)).profile.origin.z == 4.25);

    // ... and every successful operation still works.
    CHECK(document.AddLevel(MakeLevel(2, 8.0)).ok());
    CHECK(document.AddStraightWall(MakeWall(12, 2, {0.0, 0.0}, {3.0, 0.0})).ok());
    CHECK(document.UpdateLevelElevation(Id(1), 5.0).ok());
    CHECK(document.UpdateStraightWall(MakeWall(12, 2, {0.0, 0.0}, {6.0, 0.0})).ok());
    CHECK(GetGeometry(document, Id(10)).profile.origin.z == 5.25);
    CHECK(GetGeometry(document, Id(12)).profile.size_u == 6.0);
    CHECK(GetGeometry(document, Id(12)).profile.origin.z == 8.25);
}

TEST_CASE("Document: many hosted walls all recompute and ElementIds stay stable",
          "[unit][document][p1-t002]") {
    Document document;
    REQUIRE(document.AddLevel(MakeLevel(1, 0.0)).ok());
    for (std::uint8_t i = 0; i < 12; ++i) {
        const double y = static_cast<double>(i);
        REQUIRE(document
                    .AddStraightWall(
                        MakeWall(static_cast<std::uint8_t>(100 + i), 1, {0.0, y}, {10.0, y}))
                    .ok());
    }
    REQUIRE(document.UpdateLevelElevation(Id(1), 7.0).ok());
    for (std::uint8_t i = 0; i < 12; ++i) {
        const ElementId id = Id(static_cast<std::uint8_t>(100 + i));
        REQUIRE(document.FindStraightWall(id).has_value());
        CHECK(GetWall(document, id).id == id);
        CHECK(GetGeometry(document, id).profile.origin.z == 7.25);
    }
}

TEST_CASE("Document: DocumentResultCode names are stable", "[unit][document][p1-t002]") {
    using bim::document::ToString;
    CHECK(std::string_view{ToString(DocumentResultCode::Ok)} == "Ok");
    CHECK(std::string_view{ToString(DocumentResultCode::InvalidElement)} == "InvalidElement");
    CHECK(std::string_view{ToString(DocumentResultCode::DuplicateElementId)} ==
          "DuplicateElementId");
    CHECK(std::string_view{ToString(DocumentResultCode::LevelNotFound)} == "LevelNotFound");
    CHECK(std::string_view{ToString(DocumentResultCode::ElementNotFound)} == "ElementNotFound");
    CHECK(std::string_view{ToString(DocumentResultCode::ElementKindMismatch)} ==
          "ElementKindMismatch");
    CHECK(std::string_view{ToString(DocumentResultCode::RehostNotAllowed)} == "RehostNotAllowed");
    CHECK(std::string_view{ToString(DocumentResultCode::NodeIdExhausted)} == "NodeIdExhausted");
    CHECK(std::string_view{ToString(DocumentResultCode::GraphRejected)} == "GraphRejected");
    CHECK(std::string_view{ToString(DocumentResultCode::DerivedGeometryInvalid)} ==
          "DerivedGeometryInvalid");
    CHECK(std::string_view{ToString(DocumentResultCode::RecomputeFailed)} == "RecomputeFailed");
    CHECK(std::string_view{ToString(DocumentResultCode::InternalFailure)} == "InternalFailure");
    CHECK(std::string_view{ToString(static_cast<DocumentResultCode>(250))} == "Unknown");
}

TEST_CASE("Document: a moved-to document owns the committed state", "[unit][document][p1-t002]") {
    Document source;
    REQUIRE(source.AddLevel(MakeLevel(1, 4.0)).ok());
    REQUIRE(source.AddStraightWall(MakeWall(10, 1, {0.0, 0.0}, {10.0, 0.0})).ok());

    Document target = std::move(source);
    CHECK(target.FindLevel(Id(1)).has_value());
    CHECK(target.FindWallGeometry(Id(10)).has_value());
    CHECK(target.UpdateLevelElevation(Id(1), 6.0).ok());
    CHECK(GetGeometry(target, Id(10)).profile.origin.z == 6.25);
}
