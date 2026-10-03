#pragma once

#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/model/wall_face_reference.hpp"

#include <cstdint>

// P1-T001 Core BIM Identity & Domain Model - structural model validation
// (Implementation Brief BIM-TASK-P1-T001-CLAUDE v1.0 section 13).
//
// Validation is deterministic, side-effect free and vendor-neutral. It is
// structural only: it performs no document lookup, no graph lookup, no
// persistence access, no geometry generation and no tolerance-based
// geometric evaluation. Failure is reported as a project-owned code rather
// than an exception; each function reports the first violated rule, checked
// in the order the rules are listed on the function.

namespace bim::model {

enum class ValidationCode : std::uint8_t {
    Ok,

    // ElementId / identity references.
    InvalidElementId,      // an id (element, level or wall) is the all-zero value
    InvalidLevelReference, // a wall's level_id is the all-zero value
    WallIdEqualsLevelId,   // a wall's id and level_id are the same ElementId

    // Level.
    NonFiniteElevation,

    // StraightWall.
    NonFiniteCoordinate,
    CoincidentEndpoints, // start and end are exactly identical (no epsilon)
    NonFiniteThickness,
    NonPositiveThickness,
    NonFiniteHeight,
    NonPositiveHeight,
    NonFiniteBaseOffset,

    // WallFaceReference.
    InvalidWallFaceRole,
};

struct ValidationResult {
    ValidationCode code = ValidationCode::Ok;

    [[nodiscard]] constexpr bool ok() const noexcept { return code == ValidationCode::Ok; }

    friend constexpr bool operator==(const ValidationResult&, const ValidationResult&) noexcept = default;
};

// Stable diagnostic name of a code (e.g. "NonFiniteElevation"); an
// out-of-range value yields "Unknown".
[[nodiscard]] const char* ToString(ValidationCode code) noexcept;

// InvalidElementId when the id is all-zero.
[[nodiscard]] ValidationResult ValidateElementId(const ElementId& id) noexcept;

// Checks, in order: id valid (InvalidElementId); elevation finite
// (NonFiniteElevation). Zero, positive and negative elevation are valid.
[[nodiscard]] ValidationResult ValidateLevel(const Level& level) noexcept;

// Checks, in order: wall id valid (InvalidElementId); level id valid
// (InvalidLevelReference); id != level_id (WallIdEqualsLevelId); every plan
// coordinate finite (NonFiniteCoordinate); start and end not exactly
// identical (CoincidentEndpoints); thickness finite (NonFiniteThickness) and
// > 0 (NonPositiveThickness); height finite (NonFiniteHeight) and > 0
// (NonPositiveHeight); base_offset finite (NonFiniteBaseOffset). Negative
// base_offset is valid. There is no hidden epsilon: a non-zero but very short
// axis is structurally valid.
[[nodiscard]] ValidationResult ValidateStraightWall(const StraightWall& wall) noexcept;

// Checks, in order: wall_id valid (InvalidElementId); role is one of the six
// named roles (InvalidWallFaceRole).
[[nodiscard]] ValidationResult ValidateWallFaceReference(const WallFaceReference& reference) noexcept;

} // namespace bim::model
