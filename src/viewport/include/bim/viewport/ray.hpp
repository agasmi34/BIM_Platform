#pragma once

#include <cstdint>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/math.hpp"

// bim/viewport/ray.hpp - deterministic, CPU-only screen-to-world ray
// generation (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 7).
//
// Pure/stateless: ScreenToWorldRay is a free function with no owned state.
// Derived entirely from Camera's semantic parameters and viewport
// dimensions - never from a public backend projection matrix (none exists;
// see camera.hpp). Forbidden here: Qt, bgfx, D3D, Windows native types,
// OCCT, BIM semantic/model/query types.

namespace bim::viewport {

struct Ray {
    Vector3 origin{};    // == camera.Eye()
    Vector3 direction{}; // normalized
};

// physicalPixelX/physicalPixelY: physical framebuffer pixels, origin
// top-left (Brief section 7: "Input coordinates: physical framebuffer
// pixels."; "Viewport origin: top-left."). The desktop layer is
// responsible for converting Qt's logical-pixel event coordinates to
// physical pixels (multiplying by devicePixelRatio) before calling this
// function - this function itself has no notion of DPR.
//
// viewportWidthPx/viewportHeightPx: the current physical framebuffer size.
//
// Failure semantics (Brief section 7):
//   - viewportWidthPx == 0 || viewportHeightPx == 0            -> InvalidState
//   - non-finite physicalPixelX/physicalPixelY                 -> InvalidInput
//   - !camera.IsConfigured() (look-at/perspective never set)   -> InvalidState
[[nodiscard]] Result<Ray> ScreenToWorldRay(const Camera& camera, double physicalPixelX,
                                           double physicalPixelY, std::uint32_t viewportWidthPx,
                                           std::uint32_t viewportHeightPx) noexcept;

} // namespace bim::viewport
