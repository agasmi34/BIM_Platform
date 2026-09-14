#include "bim/viewport/mesh.hpp"

#include <cstddef>
#include <limits>

namespace bim::viewport {

Status ValidateMesh(const RenderMeshData& mesh) noexcept {
    // Empty mesh: nothing to render, refused rather than silently accepted
    // as a "does nothing" resource (Brief section 4: "empty mesh is
    // InvalidMesh").
    if (mesh.positions.empty()) {
        return Status::Fail(ViewportErrorCode::InvalidMesh);
    }

    // Triangle list: index count must be a positive multiple of three.
    const std::size_t index_count = mesh.indices.size();
    if (index_count == 0 || index_count % 3 != 0) {
        return Status::Fail(ViewportErrorCode::InvalidMesh);
    }

    // uint32_t addressability: neither array may require an index value
    // outside uint32_t range. positions.size()/indices.size() are size_t
    // (64-bit on the target platforms), so this is a real, checkable
    // condition, not dead code.
    constexpr std::size_t kMaxAddressable =
        static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max());
    if (mesh.positions.size() > kMaxAddressable || index_count > kMaxAddressable) {
        return Status::Fail(ViewportErrorCode::InvalidMesh);
    }

    // Every index must reference an existing position - never clamped,
    // never wrapped.
    const std::uint32_t vertex_count = static_cast<std::uint32_t>(mesh.positions.size());
    for (std::uint32_t index : mesh.indices) {
        if (index >= vertex_count) {
            return Status::Fail(ViewportErrorCode::InvalidMesh);
        }
    }

    // Normals are optional, but if present must cover every vertex exactly
    // (Brief section 4: "if normals are present, normal count equals
    // position count").
    if (!mesh.normals.empty() && mesh.normals.size() != mesh.positions.size()) {
        return Status::Fail(ViewportErrorCode::InvalidMesh);
    }

    return Status::Ok();
}

} // namespace bim::viewport
