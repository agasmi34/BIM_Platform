#pragma once

#include "bim/model/element_id.hpp"

// P1-T001 Core BIM Identity & Domain Model - Point2D and StraightWall
// (Implementation Brief BIM-TASK-P1-T001-CLAUDE v1.0 sections 6-8).
//
// Point2D is a model-owned plan coordinate: kernel-neutral, persistence-
// neutral and UI-neutral. StraightWall is the minimum authoritative
// parameter state of a level-constrained straight wall.
//
// The ordered wall axis is start -> end and that order is semantically
// significant: nothing here sorts, swaps or canonicalizes the endpoints, and
// a wall with its endpoints swapped describes the opposite direction (it is
// a different value, not an equal one).
//
// Conceptual meaning, frozen but NOT computed in P1-T001 (there is no Level
// lookup and no geometry here):
//
//     base_z = referenced_level.elevation + base_offset
//     top_z  = base_z + height
//
// Conceptual wall frame:
//
//     W = +Z
//     U = normalize(end - start)
//     V = W x U              (so U x V = W)
//
// The axis is the centerline and thickness is centered on it:
//
//     SideA = -V * thickness / 2
//     SideB = +V * thickness / 2
//
// Structural validity is checked by ValidateStraightWall() in
// validation.hpp. No geometric epsilon is involved anywhere: a very short
// but non-zero axis is structurally valid.

namespace bim::model {

struct Point2D {
    double x = 0.0;
    double y = 0.0;

    friend constexpr bool operator==(const Point2D&, const Point2D&) noexcept = default;
};

struct StraightWall {
    ElementId id;
    ElementId level_id;
    Point2D start;
    Point2D end;
    double thickness = 0.0;
    double height = 0.0;
    double base_offset = 0.0;

    friend constexpr bool operator==(const StraightWall&, const StraightWall&) noexcept = default;
};

} // namespace bim::model
