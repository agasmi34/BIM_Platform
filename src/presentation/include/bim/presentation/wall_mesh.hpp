#pragma once

#include "bim/geometry_api/geometry.hpp"
#include "bim/viewport/mesh.hpp"

#include <cstdint>

// P1-T005 Geometry Tessellation & Viewport Bridge - the project-owned wall-mesh
// presentation contract (Architecture Gate BIM-AG-P1-T005; Implementation Brief
// P1-T005).
//
// BuildWallMesh converts the neutral extrusion specification of a VS1 wall - a
// rectangular profile swept along a direction - into a deterministic
// bim::viewport::RenderMeshData by analytic rectangular-prism tessellation. It
// uses no geometry kernel, no tessellator and no external library: only the
// supplied values and the standard library.
//
// Frame. The three input vectors (profile.u_axis, profile.v_axis, direction) are
// normalized before they are scaled by size_u, size_v and distance. For the wall
// bridge the normalized frame must be right-handed: U and V orthogonal within
// tolerance.angular_radians and the extrusion direction W aligned with U x V
// within the same tolerance. An anti-parallel direction is invalid; the frame is
// never flipped, projected, rotated, clamped, scaled or recentered.
//
// Mesh. The result has exactly 24 positions, 24 normals and 36 indices (a
// TriangleList of 12 triangles): six faces of four independent vertices, in the
// frozen face order Bottom, Top, Start, End, SideA, SideB. Triangles wind
// counter-clockwise seen from outside and the normals point outward. Positions
// and normals are float32; every double-to-float conversion is checked and a
// value that does not fit is a failure, never clamped. Every produced floating
// zero is positive zero.
//
// The mesh carries no element identity, no runtime graph identity, no kernel
// topology identity and no GPU handle: BIM identity and renderer identity stay
// separate. On any failure `mesh` is empty and `code` names the failure class; a
// partially built mesh is never returned. No exception crosses this boundary.

namespace bim::presentation {

enum class WallMeshErrorCode : std::uint8_t {
    None,
    InvalidInput,         // non-finite value, non-positive size / distance or bad tolerance
    DegenerateGeometry,   // zero-length axis, skewed frame or direction not aligned with U x V
    CoordinateOutOfRange, // a position or normal does not fit a finite float32
    InvalidRenderMesh,    // the finished mesh failed bim::viewport::ValidateMesh
    InternalFailure,      // an allocation or library failure was contained
};

struct WallMeshResult {
    bim::viewport::RenderMeshData mesh;
    WallMeshErrorCode code = WallMeshErrorCode::None;

    [[nodiscard]] bool ok() const noexcept { return code == WallMeshErrorCode::None; }
};

// `tolerance` has no default: the caller supplies it explicitly. Both members must
// be finite and strictly positive.
[[nodiscard]] WallMeshResult
BuildWallMesh(const bim::geometry_api::LinearExtrusionSpec& spec,
              const bim::geometry_api::GeometryTolerance& tolerance) noexcept;

} // namespace bim::presentation
