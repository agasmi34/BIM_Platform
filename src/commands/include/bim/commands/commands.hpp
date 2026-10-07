#pragma once

#include "bim/document/document.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/model/validation.hpp"

#include <cstdint>

// P1-T003 Commands & Query Vertical Slice - the production command mutation
// boundary (Architecture Gate BIM-AG-P1-T003 sections 3-5 and 10; Implementation
// Brief P1-T003-IB section 4).
//
// Exactly five mutation semantics exist: CreateLevel, ChangeLevelElevation,
// CreateStraightWall, ChangeStraightWallGeometry and DeleteElement. Each one is
// a thin, stateless translation onto the matching bim::document operation, so
// one command is one atomic Document mutation: it either commits completely or
// leaves every committed value exactly as it was. No command can throw.
//
// Durable identity is supplied by the caller. CreateLevel and CreateStraightWall
// receive model values that already carry their ElementId; the command layer
// never generates, allocates or rewrites an identity. ChangeStraightWallGeometry
// preserves the wall's existing hosting Level - re-hosting is not a command - and
// DeleteElement never cascades: a Level that still hosts a wall is refused.
//
// This header is a public boundary. It exposes only project-owned model values
// and the bim::document handle that a command acts upon; the document's private
// runtime machinery, and any persistence, transaction, journal, SQL, kernel,
// UI, render or vendor type, is not part of it. The COMMANDS_PUBLIC_BOUNDARY
// architecture rule (tools/architecture_checker.cmake) enforces that
// mechanically. There is no journal, persistence or undo/redo behaviour here.

namespace bim::commands {

// Machine-readable outcome of a command. Callers branch on the code; there is
// no diagnostic text to parse.
enum class CommandResultCode : std::uint8_t {
    Ok,
    InvalidElement,           // the supplied value failed structural model validation
    DuplicateElementId,       // the ElementId is already used by a Level or a StraightWall
    LevelNotFound,            // the referenced hosting Level does not exist
    ElementNotFound,          // the target element does not exist
    ElementKindMismatch,      // the target exists, but as the other element kind
    ElementHasDependents,     // a Level cannot be deleted while any StraightWall is hosted by it
    RuntimeCapacityExhausted, // the document's internal identity capacity is used up
    DerivedGeometryInvalid,   // derived wall geometry overflowed or was not finite
    RecomputeFailed,          // dependent recompute failed or found inconsistent state
    InternalFailure,          // an allocation, library or internal-invariant failure was contained
};

// Result of a command. `validation` is meaningful only when `code` is
// InvalidElement, where it carries the first structural rule the supplied value
// violated; otherwise it is ValidationCode::Ok. Trivially copyable and
// allocation-free, so it is always safe to build while reporting a failure.
struct CommandResult {
    CommandResultCode code = CommandResultCode::Ok;
    bim::model::ValidationCode validation = bim::model::ValidationCode::Ok;

    [[nodiscard]] constexpr bool ok() const noexcept { return code == CommandResultCode::Ok; }

    friend constexpr bool operator==(const CommandResult&, const CommandResult&) noexcept = default;
};

// Adds a caller-identified Level. Fails with InvalidElement (structural
// validation) or DuplicateElementId (the id is already used by any Level or
// StraightWall).
[[nodiscard]] CommandResult CreateLevel(bim::document::Document& document,
                                        const bim::model::Level& level) noexcept;

// Changes the elevation of an existing Level; every wall it hosts is recomputed
// as part of the same atomic mutation. Fails with ElementNotFound,
// ElementKindMismatch (the id names a wall), InvalidElement (non-finite
// elevation) or DerivedGeometryInvalid.
[[nodiscard]] CommandResult ChangeLevelElevation(bim::document::Document& document,
                                                 const bim::model::ElementId& level_id,
                                                 double elevation) noexcept;

// Adds a caller-identified StraightWall hosted by an existing Level. Fails with
// InvalidElement, DuplicateElementId, LevelNotFound or DerivedGeometryInvalid.
[[nodiscard]] CommandResult CreateStraightWall(bim::document::Document& document,
                                               const bim::model::StraightWall& wall) noexcept;

// Replaces the defining geometry values of an existing StraightWall - start,
// end, thickness, base_offset and height - and nothing else. The wall's id and
// its hosting Level are read from the existing wall and preserved, so the
// command cannot re-host a wall. Fails with ElementNotFound, ElementKindMismatch
// (the id names a Level), InvalidElement or DerivedGeometryInvalid.
[[nodiscard]] CommandResult
ChangeStraightWallGeometry(bim::document::Document& document, const bim::model::ElementId& wall_id,
                           const bim::model::Point2D& start, const bim::model::Point2D& end,
                           double thickness, double base_offset, double height) noexcept;

// Deletes a StraightWall, or a Level that hosts no StraightWall. A Level with
// one or more hosted walls is refused with ElementHasDependents; there is no
// cascade and no automatic re-host. Fails with ElementNotFound when the id
// names nothing.
[[nodiscard]] CommandResult DeleteElement(bim::document::Document& document,
                                          const bim::model::ElementId& id) noexcept;

} // namespace bim::commands
