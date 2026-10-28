#pragma once

#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/wall_face_reference.hpp"

#include <cstddef>
#include <cstdint>

// P1-T006 Desktop VS1 End-to-End Acceptance - the production WallFaceReference
// resolver (Architecture Gate BIM-AG-P1-T006 sections 4-8; Implementation Brief
// P1-T006-IB section 2).
//
// A durable face identity is exactly WallFaceReference.wall_id + .role. This
// header turns that identity into a transient observation-vector position by
// geometric plane matching against the neutral face observations of the CURRENT
// wall geometry; the position (resolved_face_index) is a transient output only
// - never persisted, never stored in a WallFaceReference, never stable across
// regeneration or save/reopen, and enumeration order is never identity.
//
// The contract is separate from the historical Phase-0 spike resolver
// (face_reference_resolver.hpp), whose types are neither used nor aliased here.
// Only bim::model production identity types, the project-owned neutral
// geometry_api extrusion specification and neutral face observations appear in
// this public surface: no CAD-kernel, UI, render, GPU, graph or vendor type.

namespace bim::model {

enum class WallFaceResolutionStatus : std::uint8_t {
    Resolved,         // exactly one observed face matches the semantic plane
    Missing,          // no observed face matches the semantic plane
    Ambiguous,        // more than one observed face matches the semantic plane
    InvalidReference, // the reference, tolerance, observation or wall frame is unusable
    InvalidOwner      // the reference names a different wall than the current one
};

struct WallFaceResolutionResult {
    WallFaceResolutionStatus status = WallFaceResolutionStatus::InvalidReference;

    // Transient output: the position of the matching entry in the observation
    // vector that was passed in. Meaningful only when status is Resolved.
    std::size_t resolved_face_index = 0;

    [[nodiscard]] bool ok() const noexcept { return status == WallFaceResolutionStatus::Resolved; }
};

// The current (regenerated) geometry of the wall that owns the reference.
struct WallFaceFrame {
    ElementId wall_id;
    bim::geometry_api::LinearExtrusionSpec spec;
};

// Resolves `reference` against `observed_faces` using the semantic planes of
// `current_wall.spec` (U = profile.u_axis, V = profile.v_axis, W = direction,
// each normalized):
//
//     Bottom : plane through the profile origin           (outward -W)
//     Top    : plane through origin + W * distance        (outward +W)
//     Start  : minimum-U plane                            (outward -U)
//     End    : maximum-U plane                            (outward +U)
//     SideA  : minimum-V plane                            (outward -V)
//     SideB  : maximum-V plane                            (outward +V)
//
// An observed planar face matches when its normal is parallel or anti-parallel
// to the expected plane normal within tolerance.angular_radians and its point
// on the surface lies within tolerance.linear of the expected plane. Zero
// matches -> Missing, one -> Resolved, more than one -> Ambiguous.
//
// Fails closed, in this order: InvalidReference for a structurally invalid
// reference; InvalidOwner when reference.wall_id != current_wall.wall_id;
// InvalidReference for an unusable tolerance, a failed observation, or an
// extrusion spec that cannot define six valid semantic planes. Invalid geometry
// is never repaired. No exception escapes.
[[nodiscard]] WallFaceResolutionResult
ResolveWallFaceReference(const WallFaceReference& reference, const WallFaceFrame& current_wall,
                         const bim::geometry_api::FaceObservationResult& observed_faces,
                         const bim::geometry_api::GeometryTolerance& tolerance) noexcept;

} // namespace bim::model
