#include "bim/model/validation.hpp"

#include <cmath>

// P1-T001 Core BIM Identity & Domain Model - structural validation
// (Implementation Brief BIM-TASK-P1-T001-CLAUDE v1.0 section 13). Structural
// checks only: no lookup, no geometry, and no epsilon anywhere.

namespace bim::model {

namespace {

[[nodiscard]] ValidationResult Fail(ValidationCode code) noexcept {
    return ValidationResult{code};
}

} // namespace

const char* ToString(ValidationCode code) noexcept {
    switch (code) {
        case ValidationCode::Ok:
            return "Ok";
        case ValidationCode::InvalidElementId:
            return "InvalidElementId";
        case ValidationCode::InvalidLevelReference:
            return "InvalidLevelReference";
        case ValidationCode::WallIdEqualsLevelId:
            return "WallIdEqualsLevelId";
        case ValidationCode::NonFiniteElevation:
            return "NonFiniteElevation";
        case ValidationCode::NonFiniteCoordinate:
            return "NonFiniteCoordinate";
        case ValidationCode::CoincidentEndpoints:
            return "CoincidentEndpoints";
        case ValidationCode::NonFiniteThickness:
            return "NonFiniteThickness";
        case ValidationCode::NonPositiveThickness:
            return "NonPositiveThickness";
        case ValidationCode::NonFiniteHeight:
            return "NonFiniteHeight";
        case ValidationCode::NonPositiveHeight:
            return "NonPositiveHeight";
        case ValidationCode::NonFiniteBaseOffset:
            return "NonFiniteBaseOffset";
        case ValidationCode::InvalidWallFaceRole:
            return "InvalidWallFaceRole";
    }
    return "Unknown";
}

ValidationResult ValidateElementId(const ElementId& id) noexcept {
    if (!IsValid(id)) {
        return Fail(ValidationCode::InvalidElementId);
    }
    return ValidationResult{};
}

ValidationResult ValidateLevel(const Level& level) noexcept {
    if (!IsValid(level.id)) {
        return Fail(ValidationCode::InvalidElementId);
    }
    if (!std::isfinite(level.elevation)) {
        return Fail(ValidationCode::NonFiniteElevation);
    }
    return ValidationResult{};
}

ValidationResult ValidateStraightWall(const StraightWall& wall) noexcept {
    if (!IsValid(wall.id)) {
        return Fail(ValidationCode::InvalidElementId);
    }
    if (!IsValid(wall.level_id)) {
        return Fail(ValidationCode::InvalidLevelReference);
    }
    if (wall.id == wall.level_id) {
        return Fail(ValidationCode::WallIdEqualsLevelId);
    }
    if (!std::isfinite(wall.start.x) || !std::isfinite(wall.start.y) || !std::isfinite(wall.end.x) ||
        !std::isfinite(wall.end.y)) {
        return Fail(ValidationCode::NonFiniteCoordinate);
    }
    // Exact comparison on purpose: the coordinates are known finite here, and
    // no tolerance is authorized, so only coordinates that compare exactly
    // equal (which includes +0.0 versus -0.0) count as coincident.
    if (wall.start == wall.end) {
        return Fail(ValidationCode::CoincidentEndpoints);
    }
    if (!std::isfinite(wall.thickness)) {
        return Fail(ValidationCode::NonFiniteThickness);
    }
    if (!(wall.thickness > 0.0)) {
        return Fail(ValidationCode::NonPositiveThickness);
    }
    if (!std::isfinite(wall.height)) {
        return Fail(ValidationCode::NonFiniteHeight);
    }
    if (!(wall.height > 0.0)) {
        return Fail(ValidationCode::NonPositiveHeight);
    }
    if (!std::isfinite(wall.base_offset)) {
        return Fail(ValidationCode::NonFiniteBaseOffset);
    }
    return ValidationResult{};
}

ValidationResult ValidateWallFaceReference(const WallFaceReference& reference) noexcept {
    if (!IsValid(reference.wall_id)) {
        return Fail(ValidationCode::InvalidElementId);
    }
    if (!IsValidWallFaceRole(reference.role)) {
        return Fail(ValidationCode::InvalidWallFaceRole);
    }
    return ValidationResult{};
}

} // namespace bim::model
