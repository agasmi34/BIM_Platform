// P1-T001 Core BIM Identity & Domain Model - WallFaceRole / WallFaceReference
// proof (Implementation Brief BIM-TASK-P1-T001-CLAUDE v1.0 sections 9, 10,
// 16.4). Links bim::model only. This is structural validity only: no face
// resolution against geometry exists in P1-T001.

#include "bim/model/element_id.hpp"
#include "bim/model/validation.hpp"
#include "bim/model/wall_face_reference.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

using bim::model::ElementId;
using bim::model::IsValidWallFaceRole;
using bim::model::ValidateWallFaceReference;
using bim::model::ValidationCode;
using bim::model::WallFaceReference;
using bim::model::WallFaceRole;

namespace {

ElementId WallId(std::uint8_t last) {
    ElementId id;
    id.bytes[15] = last;
    return id;
}

constexpr std::array<WallFaceRole, 6> kAllRoles = {
    WallFaceRole::Bottom, WallFaceRole::Top,   WallFaceRole::Start,
    WallFaceRole::End,    WallFaceRole::SideA, WallFaceRole::SideB,
};

static_assert(std::is_trivially_copyable_v<WallFaceReference>, "a wall face reference is a plain value");
static_assert(std::is_same_v<std::underlying_type_t<WallFaceRole>, std::uint8_t>);

} // namespace

TEST_CASE("WallFaceRole: exactly the six named roles are valid", "[unit][model][p1-t001][wall-face]") {
    for (const WallFaceRole role : kAllRoles) {
        CHECK(IsValidWallFaceRole(role));
    }

    // Walk the entire underlying range: exactly six values are valid.
    std::size_t valid_count = 0;
    for (unsigned raw = 0; raw <= 255U; ++raw) {
        if (IsValidWallFaceRole(static_cast<WallFaceRole>(raw))) {
            ++valid_count;
        }
    }
    CHECK(valid_count == 6);
}

TEST_CASE("WallFaceRole: the six roles are distinct and named as frozen", "[unit][model][p1-t001][wall-face]") {
    for (std::size_t i = 0; i < kAllRoles.size(); ++i) {
        for (std::size_t j = i + 1; j < kAllRoles.size(); ++j) {
            CHECK(kAllRoles[i] != kAllRoles[j]);
        }
    }
    // Spelled out so a renamed or dropped enumerator fails to compile here.
    CHECK(WallFaceRole::Bottom != WallFaceRole::Top);
    CHECK(WallFaceRole::Start != WallFaceRole::End);
    CHECK(WallFaceRole::SideA != WallFaceRole::SideB);
}

TEST_CASE("WallFaceRole: out-of-range cast values are rejected", "[unit][model][p1-t001][wall-face]") {
    CHECK_FALSE(IsValidWallFaceRole(static_cast<WallFaceRole>(6)));
    CHECK_FALSE(IsValidWallFaceRole(static_cast<WallFaceRole>(7)));
    CHECK_FALSE(IsValidWallFaceRole(static_cast<WallFaceRole>(100)));
    CHECK_FALSE(IsValidWallFaceRole(static_cast<WallFaceRole>(255)));
}

TEST_CASE("WallFaceReference: a valid wall id with each valid role is accepted",
          "[unit][model][p1-t001][wall-face]") {
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceReference reference{.wall_id = WallId(0x01), .role = role};
        CHECK(ValidateWallFaceReference(reference).ok());
    }
}

TEST_CASE("WallFaceReference: a zero wall id is rejected", "[unit][model][p1-t001][wall-face]") {
    const WallFaceReference zero_owner{.wall_id = ElementId{}, .role = WallFaceRole::Top};
    const auto result = ValidateWallFaceReference(zero_owner);
    CHECK_FALSE(result.ok());
    CHECK(result.code == ValidationCode::InvalidElementId);
    CHECK(ValidateWallFaceReference(WallFaceReference{}).code == ValidationCode::InvalidElementId);
}

TEST_CASE("WallFaceReference: an invalid role is rejected", "[unit][model][p1-t001][wall-face]") {
    const WallFaceReference bad_role{.wall_id = WallId(0x01), .role = static_cast<WallFaceRole>(6)};
    const auto result = ValidateWallFaceReference(bad_role);
    CHECK_FALSE(result.ok());
    CHECK(result.code == ValidationCode::InvalidWallFaceRole);

    const WallFaceReference wild_role{.wall_id = WallId(0x01), .role = static_cast<WallFaceRole>(255)};
    CHECK(ValidateWallFaceReference(wild_role).code == ValidationCode::InvalidWallFaceRole);
}

TEST_CASE("WallFaceReference: a zero wall id is reported before an invalid role",
          "[unit][model][p1-t001][wall-face]") {
    const WallFaceReference both_bad{.wall_id = ElementId{}, .role = static_cast<WallFaceRole>(200)};
    CHECK(ValidateWallFaceReference(both_bad).code == ValidationCode::InvalidElementId);
}

TEST_CASE("WallFaceReference: equality and copies behave as deterministic values",
          "[unit][model][p1-t001][wall-face]") {
    const WallFaceReference a{.wall_id = WallId(0x01), .role = WallFaceRole::SideA};
    const WallFaceReference same{.wall_id = WallId(0x01), .role = WallFaceRole::SideA};
    const WallFaceReference other_role{.wall_id = WallId(0x01), .role = WallFaceRole::SideB};
    const WallFaceReference other_wall{.wall_id = WallId(0x02), .role = WallFaceRole::SideA};

    CHECK(a == same);
    CHECK(a != other_role);
    CHECK(a != other_wall);

    const WallFaceReference copy = a;
    CHECK(copy == a);

    // Same inputs always validate to the same result.
    CHECK(ValidateWallFaceReference(a) == ValidateWallFaceReference(same));
    CHECK(ValidateWallFaceReference(a) == ValidateWallFaceReference(a));
}

TEST_CASE("WallFaceReference: identity is the role itself, never its enumeration position",
          "[unit][model][p1-t001][wall-face]") {
    // The reference stores only a wall id and a role. Two references to the
    // same wall are the same face exactly when they name the same role.
    const ElementId wall = WallId(0x05);
    for (std::size_t i = 0; i < kAllRoles.size(); ++i) {
        for (std::size_t j = 0; j < kAllRoles.size(); ++j) {
            const WallFaceReference left{.wall_id = wall, .role = kAllRoles[i]};
            const WallFaceReference right{.wall_id = wall, .role = kAllRoles[j]};
            CHECK((left == right) == (i == j));
        }
    }
}
