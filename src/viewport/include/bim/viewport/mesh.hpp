#pragma once

#include <cstdint>
#include <vector>

#include "bim/viewport/error.hpp"
#include "bim/viewport/math.hpp"

// bim/viewport/mesh.hpp - the render-neutral, BIM-neutral mesh contract
// (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 4).
//
// Forbidden here: Qt, bgfx, D3D, Windows native types, OCCT, BIM
// semantic/model/query types. RenderMeshData is a plain value type - it
// owns no GPU resource and knows nothing about any renderer.

namespace bim::viewport {

// Only value authorized in P0-T003 (Brief section 4: "triangle list only").
// RD1.6-02 (performance-enum-size): the default enum representation is
// intentionally preserved for the Phase-0 contract; not changed merely
// for storage-size optimization (Architecture Authority decision).
enum class MeshTopology { // NOLINT(performance-enum-size)
    TriangleList,
};

// A caller-owned description of a single triangle mesh, expressed entirely
// in local render-frame coordinates (Brief section 4: "positions are
// already in a local render frame"; "survey/global coordinates are not
// submitted directly as GPU vertex positions" - that conversion, if any, has
// already happened before a RenderMeshData is constructed; this type cannot
// itself detect a caller who skipped it).
//
// Conventions (Brief section 4 / Implementation Authorization section 7):
//   - right-handed, +Z up, local plan-space +X/+Y (no East/North labeling);
//   - one render unit equals one model unit;
//   - triangle front face is CCW when viewed from outside;
//   - supplied normals are outward-facing and consistent with that winding;
//   - no GPU interleaving/layout rule is implied - positions/normals/indices
//     are separate arrays; bgfx buffer packing is a bim::viewport_bgfx
//     concern (see src/viewport/bgfx/src/renderer.cpp), never a public
//     concern of this header.
struct RenderMeshData {
    MeshTopology topology = MeshTopology::TriangleList;
    std::vector<Position3f> positions;
    std::vector<Normal3f> normals; // empty == "no normals supplied"
    std::vector<std::uint32_t> indices;
};

// Structural validation only (Brief section 4: "zero-area triangles and
// general geometry-quality validation are not viewport responsibilities in
// P0-T003"). Checks, in order:
//   - positions is non-empty (an empty mesh is InvalidMesh);
//   - indices.size() is a positive multiple of 3 (triangle list);
//   - every index is < positions.size();
//   - normals is either empty or exactly positions.size() long;
//   - positions.size() and indices.size() do not exceed uint32_t
//     addressability.
// Returns Status::Ok() only if every check passes; otherwise
// Status::Fail(ViewportErrorCode::InvalidMesh). Implemented in mesh.cpp
// (not header-only) because the checks below intentionally short-circuit in
// a specific, tested order.
[[nodiscard]] Status ValidateMesh(const RenderMeshData& mesh) noexcept;

} // namespace bim::viewport
