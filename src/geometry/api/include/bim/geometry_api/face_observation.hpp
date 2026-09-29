#pragma once

#include "bim/geometry_api/geometry.hpp"

#include <cstddef>
#include <vector>

namespace bim::geometry_api {

// P0-T008 Topological Reference Spike - neutral, OCCT-free per-face
// observation contract (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0;
// Architecture Gate 01-ARCHITECTURE-GATE.md). This header must never
// include, forward-declare, alias or otherwise reference a CAD-kernel
// implementation type (no TopoDS_*, gp_*, BRep*, Handle(...), and no
// dependency on the underlying kernel library at all) - it is scanned by
// the same GEOMETRY_API_NO_OCCT_LEAK rule (tools/architecture_checker.cmake)
// that already governs every other header under this directory.
//
// ObserveFaces() is deliberately a PURE, tolerance-free extraction: it
// reports what geometry is actually present on a solid's faces (a
// representative on-surface point, a unit outward normal where the surface
// is planar, whether the surface is planar at all, and its area) without
// making any resolution decision. Deciding whether an observed face
// "matches" a persistent semantic role, and with what tolerance, is
// entirely bim_model's job (bim::model::ResolveFaceReference) - this
// contract only ever describes geometry, it never names or resolves
// anything (Brief: "bim_model -> bim_geometry_api -> bim_foundation";
// bim_geometry_api itself introduces no semantic naming).

// --- Per-face surface classification -----------------------------------------
// P0-T008's proof corpus only needs to distinguish "planar" (the only
// surface kind a rectangular linear-extrusion face ever produces) from
// everything else. A non-planar face is still reported (so a caller can see
// it exists and account for it), but its normal/point-on-surface values are
// not meaningful for plane-based resolution and callers must not rely on
// them.
enum class SurfaceKind { // NOLINT(performance-enum-size)
    Planar,
    NonPlanar,
};

// --- FaceObservation -----------------------------------------------------------
// One observed face, in NO particular guaranteed order (Brief: enumeration
// order must never be treated as identity by any consumer of this vector -
// bim_model's resolver must produce the same result regardless of the order
// these entries happen to arrive in).
struct FaceObservation {
    Point3 point_on_surface;
    Vector3 normal;
    SurfaceKind surface_kind = SurfaceKind::NonPlanar;
    double area = 0.0;
};

// --- FaceObservationResult -----------------------------------------------------
struct FaceObservationResult {
    std::vector<FaceObservation> faces;
    GeometryError error;

    [[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

// ObserveFaces(solid): enumerate every face of `solid` and report its
// neutral geometric observation. Implemented exclusively in
// bim_geometry_occt (src/geometry/occt/src/face_observation.cpp). No
// third-party (kernel/std) exception ever crosses this boundary - every
// exception is caught and translated to a GeometryError entirely inside the
// adapter, mirroring MakeLinearExtrusion/Cut/Fuse/Inspect above.
//
// This function reports observations only - it never assigns, persists, or
// compares against any semantic role. It also never returns a stable
// per-face identifier: the position of an entry in the returned vector is
// not a token any caller may treat as identity across two separate calls to
// this function (Brief section on prohibited identity sources - face
// enumeration position/order is explicitly listed there).
[[nodiscard]] FaceObservationResult ObserveFaces(const SolidHandle& solid);

} // namespace bim::geometry_api
