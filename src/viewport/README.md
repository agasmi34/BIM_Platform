# src/viewport/ (boundary placeholder)

Reserved for the rendering/viewport module. **Not built in P0-T001** - this
directory intentionally contains no `CMakeLists.txt` and is not added via
`add_subdirectory()` from the root `CMakeLists.txt`.

Scope owner: **P0-T003 - Desktop + Viewport Spike** (bgfx vs Qt RHI decision;
see `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 23). bgfx
is not linked anywhere in this repository as of P0-T001. When built, this
module MUST depend only on `model`/`query` + `geometry_api` and must never
reach into `bim_geometry_occt` internals (Architecture Gate section 8.1).
