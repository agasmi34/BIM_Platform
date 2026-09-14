# src/desktop/ (P0-T003 Desktop + Viewport Spike)

Built as of **P0-T003 - Desktop + Viewport Spike** (Implementation Brief
BIM-TASK-P0-T003-CLAUDE v1.0; Implementation Authorization
`cf7a971903d8103143b2b94e94a4d185d86ce42b`). Was a boundary placeholder
(no `CMakeLists.txt`, not added via `add_subdirectory()`) through P0-T001
and P0-T002; the root `CMakeLists.txt` now composes `bim_desktop_spike`
here (`src/desktop/CMakeLists.txt`), the interactive Qt Widgets spike
application (`MainWindow` / `ViewportWindow` / `ViewportBridge` /
spike scenes / headless evidence mode).

`bim_desktop_spike` is the ONLY target permitted to include Qt headers or
link Qt anywhere in this repository (`tools/architecture_checker.cmake`
rule R9/QT_DESKTOP_ONLY) - Qt Widgets only (Phase A fact: qtbase 6.11.1#1
Widgets-only, Qt Quick forbidden). It links `bim::viewport` and
`bim::viewport_bgfx` and never includes a bgfx or D3D/Windows native
header directly itself - all rendering goes through
`bim::viewport_bgfx::Renderer`'s pimpl-based public API
(`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`). Must never
hold a raw DB or raw OCCT dependency (Architecture Gate section 8.1) -
unchanged from the pre-P0-T003 boundary rule.

Per the Implementation Brief and Implementation Authorization, every file
under this directory authored during the P0-T003 authoring pass is
UNVERIFIED - written without a build/execution channel in this session -
see `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` for the full disclosure and
the Operator's required next steps.
