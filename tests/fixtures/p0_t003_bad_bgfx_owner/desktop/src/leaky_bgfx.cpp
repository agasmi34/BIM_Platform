// tests/fixtures/p0_t003_bad_bgfx_owner/desktop/src/leaky_bgfx.cpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R10/BGFX_VIEWPORT_OWNER must reject this file. It sits under
// desktop/src/** (outside viewport/bgfx/**, mirroring R7's ownership
// -partition pattern) and uses a bgfx token where only
// src/viewport/bgfx/** is permitted to - exactly the "desktop reaches
// past bim::viewport_bgfx's pimpl boundary" defect R10 exists to catch.
// This file is never compiled - it exists only to be scanned by the
// architecture checker (see tests/architecture/CMakeLists.txt, test
// arch_p0_t003_bgfx_fixture_rejected).

#include <bgfx/bgfx.h> // FORBIDDEN: only src/viewport/bgfx/** may use bgfx

namespace bim::desktop {

void NotAllowedHere() {
    bgfx::touch(0); // desktop must never call into bgfx directly
}

} // namespace bim::desktop
