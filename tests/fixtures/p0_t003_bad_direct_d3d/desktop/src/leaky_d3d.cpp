// tests/fixtures/p0_t003_bad_direct_d3d/desktop/src/leaky_d3d.cpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R11/NO_DIRECT_D3D must reject this file. Unlike R8-R10, R11 has no
// exception directory: no first-party file anywhere under src/ is
// permitted to touch D3D11/DXGI directly (Implementation Brief section
// 9: D3D11 is reached ONLY through bgfx, never called directly by any
// first-party BIM Platform code, including bim_viewport_bgfx itself -
// renderer.cpp/renderer_impl.hpp use only <bgfx/bgfx.h> and <bx/...>,
// never <d3d11.h>). This file is never compiled - it exists only to be
// scanned by the architecture checker (see tests/architecture/CMakeLists.txt,
// test arch_p0_t003_d3d_fixture_rejected).

#include <d3d11.h> // FORBIDDEN: no first-party file may include this directly
#include <dxgi.h>  // FORBIDDEN: same rule, deliberately stricter (Brief risk note)

namespace bim::desktop {

void NotAllowedHere() {
    ID3D11Device* device = nullptr; // FORBIDDEN token: ID3D11
    static_cast<void>(device);
}

} // namespace bim::desktop
