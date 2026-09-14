# src/viewport/ (P0-T003 Desktop + Viewport Spike)

Built as of **P0-T003 - Desktop + Viewport Spike** (Implementation Brief
BIM-TASK-P0-T003-CLAUDE v1.0; Implementation Authorization
`cf7a971903d8103143b2b94e94a4d185d86ce42b`). Was a boundary placeholder
(no `CMakeLists.txt`, not added via `add_subdirectory()`) through P0-T001
and P0-T002; the root `CMakeLists.txt` now composes two targets here:

- `bim::viewport` (`src/viewport/CMakeLists.txt`) - the renderer-neutral
  contract (mesh/camera/ray/error/lifecycle/input types). Depends on
  nothing first-party and nothing third-party beyond the standard library -
  not even `bim::foundation`. Never includes Qt, bgfx, D3D/Windows native
  headers, OCCT, or any BIM semantic/model/query type. Mechanically
  enforced by `tools/architecture_checker.cmake` rule
  R8/VIEWPORT_PUBLIC_NEUTRAL.
- `bim::viewport_bgfx` (`src/viewport/bgfx/CMakeLists.txt`) - the bgfx
  -backed renderer adapter. The ONLY target permitted to include bgfx
  headers or link bgfx (rule R10/BGFX_VIEWPORT_OWNER); its one public
  header (`include/bim/viewport_bgfx/renderer.hpp`) exposes zero bgfx
  types (pimpl). D3D11 is the sole authoritative backend (Phase A fact);
  D3D11/DXGI are reached ONLY through bgfx itself, never included directly
  anywhere in this repository (rule R11/NO_DIRECT_D3D, no exception
  directory).

This module never depends on `bim_geometry_occt` internals and is not
reached into by anything outside `src/desktop/` (Architecture Gate section
8.1). Per the Implementation Brief and Implementation Authorization, every
file under this directory authored during the P0-T003 authoring pass is
UNVERIFIED - written without a build/execution channel in this session -
see `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` for the full disclosure and
the Operator's required next steps.
