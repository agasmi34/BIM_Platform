#include "bim/commands/commands.hpp"

#include <optional>

// P1-T003 Commands & Query Vertical Slice - implementation of bim::commands
// (Architecture Gate BIM-AG-P1-T003; Implementation Brief P1-T003-IB section 4).
//
// Every command is a stateless delegation to one bim::document mutation, which
// is already strongly atomic, followed by a total translation of the document's
// result into the command contract. This file owns no state, performs no
// identity generation and touches no module other than bim::document and the
// model values it carries.

namespace bim::commands {

namespace {

using bim::document::DocumentResult;
using bim::document::DocumentResultCode;

[[nodiscard]] constexpr CommandResult Make(CommandResultCode code) noexcept {
    return CommandResult{.code = code, .validation = bim::model::ValidationCode::Ok};
}

// Total mapping of a document result onto the command contract. InvalidElement
// keeps the structural ValidationCode. The document's internal graph-rejection
// and re-host codes are implementation details that no command can legitimately
// provoke, so they surface as InternalFailure, as does any value outside the
// known set.
[[nodiscard]] constexpr CommandResult Translate(const DocumentResult& result) noexcept {
    switch (result.code) {
        case DocumentResultCode::Ok:
            return Make(CommandResultCode::Ok);
        case DocumentResultCode::InvalidElement:
            return CommandResult{.code = CommandResultCode::InvalidElement,
                                 .validation = result.validation};
        case DocumentResultCode::DuplicateElementId:
            return Make(CommandResultCode::DuplicateElementId);
        case DocumentResultCode::LevelNotFound:
            return Make(CommandResultCode::LevelNotFound);
        case DocumentResultCode::ElementNotFound:
            return Make(CommandResultCode::ElementNotFound);
        case DocumentResultCode::ElementKindMismatch:
            return Make(CommandResultCode::ElementKindMismatch);
        case DocumentResultCode::ElementHasDependents:
            return Make(CommandResultCode::ElementHasDependents);
        case DocumentResultCode::NodeIdExhausted:
            return Make(CommandResultCode::RuntimeCapacityExhausted);
        case DocumentResultCode::DerivedGeometryInvalid:
            return Make(CommandResultCode::DerivedGeometryInvalid);
        case DocumentResultCode::RecomputeFailed:
            return Make(CommandResultCode::RecomputeFailed);
        case DocumentResultCode::GraphRejected:
        case DocumentResultCode::RehostNotAllowed:
        case DocumentResultCode::InternalFailure:
            return Make(CommandResultCode::InternalFailure);
    }
    return Make(CommandResultCode::InternalFailure);
}

} // namespace

CommandResult CreateLevel(bim::document::Document& document,
                          const bim::model::Level& level) noexcept {
    return Translate(document.AddLevel(level));
}

CommandResult ChangeLevelElevation(bim::document::Document& document,
                                   const bim::model::ElementId& level_id,
                                   double elevation) noexcept {
    return Translate(document.UpdateLevelElevation(level_id, elevation));
}

CommandResult CreateStraightWall(bim::document::Document& document,
                                 const bim::model::StraightWall& wall) noexcept {
    return Translate(document.AddStraightWall(wall));
}

CommandResult ChangeStraightWallGeometry(bim::document::Document& document,
                                         const bim::model::ElementId& wall_id,
                                         const bim::model::Point2D& start,
                                         const bim::model::Point2D& end, double thickness,
                                         double base_offset, double height) noexcept {
    const std::optional<bim::model::StraightWall> existing = document.FindStraightWall(wall_id);
    if (!existing.has_value()) {
        // The id names no wall: it is either a Level or nothing at all.
        return Make(document.FindLevel(wall_id).has_value() ? CommandResultCode::ElementKindMismatch
                                                            : CommandResultCode::ElementNotFound);
    }

    // Start from the committed wall so its id and hosting Level are carried over
    // unchanged; only the five geometry values are replaced.
    bim::model::StraightWall changed = *existing;
    changed.start = start;
    changed.end = end;
    changed.thickness = thickness;
    changed.base_offset = base_offset;
    changed.height = height;
    return Translate(document.UpdateStraightWall(changed));
}

CommandResult DeleteElement(bim::document::Document& document,
                            const bim::model::ElementId& id) noexcept {
    return Translate(document.DeleteElement(id));
}

} // namespace bim::commands
