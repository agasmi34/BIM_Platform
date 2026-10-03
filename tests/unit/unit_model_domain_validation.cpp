// P1-T001 Core BIM Identity & Domain Model - Level / Point2D / StraightWall
// structural validation proof (Implementation Brief
// BIM-TASK-P1-T001-CLAUDE v1.0 sections 13, 16.2, 16.3). Links bim::model
// only; validation is structural, with no lookup, geometry or tolerance.

#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/model/validation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>

using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::model::ValidateElementId;
using bim::model::ValidateLevel;
using bim::model::ValidateStraightWall;
using bim::model::ValidationCode;
using bim::model::ValidationResult;

namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr double kPosInf = std::numeric_limits<double>::infinity();
constexpr double kNegInf = -std::numeric_limits<double>::infinity();

ElementId IdWithLastByte(std::uint8_t value) {
    ElementId id;
    id.bytes[15] = value;
    return id;
}

ElementId WallId() { return IdWithLastByte(0x01); }
ElementId LevelId() { return IdWithLastByte(0x02); }

Level ValidLevel() { return Level{.id = LevelId(), .elevation = 3.5}; }

StraightWall ValidWall() {
    return StraightWall{.id = WallId(),
                        .level_id = LevelId(),
                        .start = Point2D{.x = 0.0, .y = 0.0},
                        .end = Point2D{.x = 5.0, .y = 0.0},
                        .thickness = 0.2,
                        .height = 3.0,
                        .base_offset = 0.0};
}

} // namespace

// --- ElementId validation -----------------------------------------------------

TEST_CASE("ValidateElementId: zero is rejected, non-zero accepted", "[unit][model][p1-t001][validation]") {
    CHECK(ValidateElementId(ElementId{}).code == ValidationCode::InvalidElementId);
    CHECK_FALSE(ValidateElementId(ElementId{}).ok());
    CHECK(ValidateElementId(WallId()).ok());
}

TEST_CASE("ValidationCode: every code has a distinct, non-empty diagnostic name", "[unit][model][p1-t001][validation]") {
    using bim::model::ToString;
    CHECK(std::string_view(ToString(ValidationCode::Ok)) == "Ok");
    CHECK(std::string_view(ToString(ValidationCode::InvalidElementId)) == "InvalidElementId");
    CHECK(std::string_view(ToString(ValidationCode::InvalidWallFaceRole)) == "InvalidWallFaceRole");
    CHECK(std::string_view(ToString(static_cast<ValidationCode>(250))) == "Unknown");
}

// --- Level --------------------------------------------------------------------

TEST_CASE("Level: zero elevation is valid", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.elevation = 0.0;
    CHECK(ValidateLevel(level).ok());
    level.elevation = -0.0;
    CHECK(ValidateLevel(level).ok());
}

TEST_CASE("Level: positive elevation is valid", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.elevation = 12.75;
    CHECK(ValidateLevel(level).ok());
}

TEST_CASE("Level: negative elevation is valid", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.elevation = -4.25;
    CHECK(ValidateLevel(level).ok());
}

TEST_CASE("Level: zero ElementId is rejected", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.id = ElementId{};
    CHECK(ValidateLevel(level).code == ValidationCode::InvalidElementId);
    CHECK(ValidateLevel(Level{}).code == ValidationCode::InvalidElementId);
}

TEST_CASE("Level: NaN, +Inf and -Inf elevation are rejected", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.elevation = kNaN;
    CHECK(ValidateLevel(level).code == ValidationCode::NonFiniteElevation);
    level.elevation = kPosInf;
    CHECK(ValidateLevel(level).code == ValidationCode::NonFiniteElevation);
    level.elevation = kNegInf;
    CHECK(ValidateLevel(level).code == ValidationCode::NonFiniteElevation);
}

TEST_CASE("Level: extreme finite elevations are valid (no hidden range limit)", "[unit][model][p1-t001][level]") {
    Level level = ValidLevel();
    level.elevation = std::numeric_limits<double>::max();
    CHECK(ValidateLevel(level).ok());
    level.elevation = std::numeric_limits<double>::lowest();
    CHECK(ValidateLevel(level).ok());
}

// --- Point2D ------------------------------------------------------------------

TEST_CASE("Point2D: plain model-owned value with deterministic equality", "[unit][model][p1-t001][point2d]") {
    CHECK(Point2D{} == Point2D{.x = 0.0, .y = 0.0});
    CHECK(Point2D{.x = 1.0, .y = 2.0} == Point2D{.x = 1.0, .y = 2.0});
    CHECK(Point2D{.x = 1.0, .y = 2.0} != Point2D{.x = 2.0, .y = 1.0});
}

// --- StraightWall -------------------------------------------------------------

TEST_CASE("StraightWall: a normal wall is valid", "[unit][model][p1-t001][wall]") {
    CHECK(ValidateStraightWall(ValidWall()).ok());
}

TEST_CASE("StraightWall: positive, zero and negative plan coordinates are valid", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.start = Point2D{.x = -10.0, .y = -20.0};
    wall.end = Point2D{.x = 0.0, .y = 30.0};
    CHECK(ValidateStraightWall(wall).ok());
}

TEST_CASE("StraightWall: start -> end order is preserved", "[unit][model][p1-t001][wall]") {
    const StraightWall wall = ValidWall();
    CHECK(wall.start == Point2D{.x = 0.0, .y = 0.0});
    CHECK(wall.end == Point2D{.x = 5.0, .y = 0.0});

    // Copying and validating never reorders the stored axis.
    const StraightWall copy = wall;
    REQUIRE(ValidateStraightWall(copy).ok());
    CHECK(copy.start == wall.start);
    CHECK(copy.end == wall.end);

    // An axis whose start sorts after its end is kept exactly as given.
    StraightWall descending = ValidWall();
    descending.start = Point2D{.x = 9.0, .y = 9.0};
    descending.end = Point2D{.x = -9.0, .y = -9.0};
    REQUIRE(ValidateStraightWall(descending).ok());
    CHECK(descending.start == Point2D{.x = 9.0, .y = 9.0});
    CHECK(descending.end == Point2D{.x = -9.0, .y = -9.0});
}

TEST_CASE("StraightWall: swapped endpoints remain valid but are a semantically distinct wall",
          "[unit][model][p1-t001][wall]") {
    const StraightWall forward = ValidWall();
    StraightWall reversed = forward;
    reversed.start = forward.end;
    reversed.end = forward.start;

    CHECK(ValidateStraightWall(forward).ok());
    CHECK(ValidateStraightWall(reversed).ok());
    CHECK(forward != reversed);
    CHECK(reversed.start == forward.end);
    CHECK(reversed.end == forward.start);
}

TEST_CASE("StraightWall: zero wall id is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.id = ElementId{};
    CHECK(ValidateStraightWall(wall).code == ValidationCode::InvalidElementId);
}

TEST_CASE("StraightWall: zero level id is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.level_id = ElementId{};
    CHECK(ValidateStraightWall(wall).code == ValidationCode::InvalidLevelReference);
}

TEST_CASE("StraightWall: wall id equal to level id is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.level_id = wall.id;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::WallIdEqualsLevelId);
}

TEST_CASE("StraightWall: non-finite plan coordinates are rejected", "[unit][model][p1-t001][wall]") {
    for (const double bad : {kNaN, kPosInf, kNegInf}) {
        StraightWall a = ValidWall();
        a.start.x = bad;
        CHECK(ValidateStraightWall(a).code == ValidationCode::NonFiniteCoordinate);

        StraightWall b = ValidWall();
        b.start.y = bad;
        CHECK(ValidateStraightWall(b).code == ValidationCode::NonFiniteCoordinate);

        StraightWall c = ValidWall();
        c.end.x = bad;
        CHECK(ValidateStraightWall(c).code == ValidationCode::NonFiniteCoordinate);

        StraightWall d = ValidWall();
        d.end.y = bad;
        CHECK(ValidateStraightWall(d).code == ValidationCode::NonFiniteCoordinate);
    }
}

TEST_CASE("StraightWall: exactly coincident endpoints are rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.end = wall.start;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::CoincidentEndpoints);

    wall.start = Point2D{.x = 3.25, .y = -7.5};
    wall.end = Point2D{.x = 3.25, .y = -7.5};
    CHECK(ValidateStraightWall(wall).code == ValidationCode::CoincidentEndpoints);

    // Signed zeros compare equal, so they are the same point.
    wall.start = Point2D{.x = 0.0, .y = 0.0};
    wall.end = Point2D{.x = -0.0, .y = -0.0};
    CHECK(ValidateStraightWall(wall).code == ValidationCode::CoincidentEndpoints);
}

TEST_CASE("StraightWall: a tiny non-zero axis is accepted - no hidden tolerance", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.start = Point2D{.x = 0.0, .y = 0.0};

    wall.end = Point2D{.x = 1e-300, .y = 0.0};
    CHECK(ValidateStraightWall(wall).ok());

    wall.end = Point2D{.x = 0.0, .y = std::numeric_limits<double>::denorm_min()};
    CHECK(ValidateStraightWall(wall).ok());

    // Adjacent representable doubles around a large coordinate.
    wall.start = Point2D{.x = 1.0e6, .y = 1.0e6};
    wall.end = Point2D{.x = std::nextafter(1.0e6, 2.0e6), .y = 1.0e6};
    CHECK(ValidateStraightWall(wall).ok());
}

TEST_CASE("StraightWall: thickness <= 0 is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.thickness = 0.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveThickness);
    wall.thickness = -0.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveThickness);
    wall.thickness = -0.2;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveThickness);
}

TEST_CASE("StraightWall: tiny positive thickness is accepted", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.thickness = std::numeric_limits<double>::denorm_min();
    CHECK(ValidateStraightWall(wall).ok());
}

TEST_CASE("StraightWall: non-finite thickness is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.thickness = kNaN;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteThickness);
    wall.thickness = kPosInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteThickness);
    wall.thickness = kNegInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteThickness);
}

TEST_CASE("StraightWall: height <= 0 is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.height = 0.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveHeight);
    wall.height = -0.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveHeight);
    wall.height = -3.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveHeight);
}

TEST_CASE("StraightWall: non-finite height is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.height = kNaN;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteHeight);
    wall.height = kPosInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteHeight);
    wall.height = kNegInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteHeight);
}

TEST_CASE("StraightWall: finite positive, zero and negative base offset are accepted",
          "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.base_offset = 0.5;
    CHECK(ValidateStraightWall(wall).ok());
    wall.base_offset = 0.0;
    CHECK(ValidateStraightWall(wall).ok());
    wall.base_offset = -0.5;
    CHECK(ValidateStraightWall(wall).ok());
}

TEST_CASE("StraightWall: non-finite base offset is rejected", "[unit][model][p1-t001][wall]") {
    StraightWall wall = ValidWall();
    wall.base_offset = kNaN;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteBaseOffset);
    wall.base_offset = kPosInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteBaseOffset);
    wall.base_offset = kNegInf;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonFiniteBaseOffset);
}

TEST_CASE("StraightWall: validation reports the first violated rule in the documented order",
          "[unit][model][p1-t001][wall]") {
    // Everything wrong at once: the identity rule fires first.
    StraightWall wall{};
    wall.thickness = kNaN;
    wall.height = kNaN;
    wall.base_offset = kNaN;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::InvalidElementId);

    // Valid ids but coincident endpoints and a bad thickness: endpoints first.
    wall = ValidWall();
    wall.end = wall.start;
    wall.thickness = -1.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::CoincidentEndpoints);

    // Bad thickness and bad height: thickness first.
    wall = ValidWall();
    wall.thickness = 0.0;
    wall.height = 0.0;
    CHECK(ValidateStraightWall(wall).code == ValidationCode::NonPositiveThickness);
}

TEST_CASE("StraightWall: validation is deterministic and side-effect free", "[unit][model][p1-t001][wall]") {
    const StraightWall wall = ValidWall();
    const StraightWall before = wall;
    const ValidationResult first = ValidateStraightWall(wall);
    const ValidationResult second = ValidateStraightWall(wall);
    CHECK(first == second);
    CHECK(wall == before);
}
