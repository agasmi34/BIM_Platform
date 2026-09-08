# P0-T003 - Architecture Gate

Gate:

BIM-AG-P0-T003 v1.0

Status:

APPROVED + LOCKED

Date:

2026-09-08

Authoritative baseline:

3f2230be5fcd796c370f485975547112ad52d2e3

Authoritative tree:

34b0d7f5c0bd75df58e8a12578fc37351e137ad8

## 1. Objective

P0-T003 is a Desktop + Viewport Spike.

The task proves that the BIM Platform has a viable long-lived Windows desktop
shell and graphics viewport foundation before persistence, IFC, topological
references, dependency-graph work, or Phase 1 architecture begins.

This is a foundation spike.

It is not the final BIM user interface and it is not a production rendering
engine.

## 2. Locked technology direction

Desktop framework:

Qt 6

P0-T003 desktop module:

Qt Widgets

Qt Quick:

OUT OF SCOPE

QRhi viewport path:

FORBIDDEN

Viewport rendering-abstraction candidate:

bgfx

Authoritative Windows Phase-0 graphics backend:

D3D11

Exact Qt and bgfx package versions are resolved only from the existing frozen
vcpkg baseline during Phase A.

A dependency/version change is not implicitly authorized.

If the required package, feature, backend, capability, or licensing posture
cannot be satisfied at the frozen baseline, implementation stops for
Architecture Authority disposition or ACR.

## 3. Dependency direction

Conceptual direction:

BIM / Model / Commands / Query
              |
              v
      viewport-neutral API
              |
              v
      viewport renderer
              |
              v
            bgfx
              |
              v
           D3D11

Desktop shell:

Qt Widgets
    |
    v
private Qt/native-window/viewport bridge

## 4. Boundary rules

src/desktop/** may own Qt.

src/viewport/** may own viewport rendering implementation.

A private platform bridge may know Qt native-window details, native Windows
surface handles and bgfx platform integration.

The following must not depend on Qt or bgfx:

- src/model/**
- src/geometry/api/**
- src/commands/**
- src/query/**
- src/transactions/**
- src/dependency_graph/**
- src/persistence/**

Public BIM/domain APIs must not expose:

- QWidget*
- QWindow*
- QRhi*
- QMouseEvent*
- bgfx::*
- ID3D11*
- HWND
- TopoDS_*
- BRep*

Direct viewport-to-OCCT coupling is forbidden.

## 5. Qt / viewport integration policy

The desktop shell uses Qt Widgets.

The viewport renders into a native surface obtained privately by the
desktop/viewport platform bridge.

Qt Quick is not part of P0-T003.

QRhi is not used by the viewport path.

Native handles are private integration details and must not appear in public
BIM/domain contracts.

## 6. Render-neutral mesh contract

P0-T003 must use one minimal render-neutral mesh seam for V01-V05.

The contract is renderer-neutral and BIM-neutral.

Minimum contract:

Topology:

triangle list

Positions:

3-component float32 values in a local render frame

Normals:

3-component float32 optional per-vertex channel

If normals are present:

normal count equals vertex count

Indices:

uint32

Coordinate policy:

local render coordinates

1 render unit equals 1 model unit

Global/survey coordinates are not submitted directly to GPU vertex buffers.

BIM semantic classes:

FORBIDDEN

OCCT types:

FORBIDDEN

Qt types:

FORBIDDEN

bgfx types:

FORBIDDEN

Exact handedness and triangle-winding convention must be explicitly resolved
and frozen during Phase B Contract Design Check before production
implementation.

## 7. Desktop scope

P0-T003 may implement only the minimum desktop shell required to prove:

- application startup;
- application shutdown;
- main window;
- central viewport;
- minimal menu/status shell;
- keyboard and mouse routing;
- resize;
- minimize/restore;
- close lifecycle;
- HiDPI behavior.

The final ribbon, project browser, properties system and final docking
architecture are outside this task.

## 8. Viewport scope

The spike must prove:

- background clear;
- reference grid;
- XYZ axes;
- deterministic indexed mesh;
- perspective camera;
- orbit;
- pan;
- zoom;
- viewport resize;
- HiDPI-aware physical/logical dimensions;
- stable frame lifecycle;
- screen-to-world ray generation.

Full BIM selection, snapping, grips and object-manipulation systems are not
part of P0-T003.

## 9. Scene corpus

V01:

single indexed cube

V02:

grid + axes + cube

V03:

1,000 repeated box instances

V04:

representative medium mesh scene

V05:

large-coordinate-view experiment represented and rendered using local
coordinates

V03 and V04 are observational performance evidence only.

No elapsed-time or FPS pass/fail threshold may enter Phase-0 CI.

## 10. Lifecycle requirements

The spike must explicitly verify:

- native surface/handle acquisition occurs only after the Qt platform window
  exists;
- native handle is not assumed permanently valid across surface recreation;
- handle is re-resolved when platform-surface recreation requires it;
- resize is stable;
- zero-size resize during minimize is handled;
- minimize/restore is stable;
- devicePixelRatio changes are handled;
- cross-monitor movement between differing-DPI displays is tested;
- renderer reset/reconfiguration follows physical-pixel-size changes;
- surface destruction ordering is explicit;
- renderer shutdown completes before QApplication teardown;
- no frame submission occurs after surface invalidation.

## 11. Headless / automated-test policy

P0-T003 must not require an interactive GPU desktop session for all automated
tests.

Camera mathematics, screen-to-world ray generation, neutral-mesh validation
and other CPU-owned viewport logic must run without a live desktop/GPU.

Phase A must verify which non-presented test mechanism the frozen bgfx
baseline actually provides, such as a Noop or offscreen-capable path.

No unverified bgfx capability is assumed by this Gate.

A separate production renderer must not be invented merely to satisfy tests.

The authoritative live desktop/render gate remains Windows + D3D11.

## 12. Failure corpus

At minimum:

- invalid viewport dimensions;
- zero width/height;
- renderer initialization failure;
- unsupported backend selection;
- invalid mesh indices;
- empty mesh;
- invalid camera near/far relationship;
- platform surface unavailable;
- platform surface recreated;
- surface destroyed before renderer shutdown;
- zero-size resize during minimize;
- DPI change during active session.

Failures must be structured at the project-owned boundary.

Qt/bgfx/native/D3D errors must not leak as BIM/domain public contracts.

## 13. Threading policy

Qt UI ownership:

main thread

Renderer lifecycle:

explicitly controlled by the approved viewport/platform integration layer

P0-T003 does not introduce a speculative multithreaded rendering
architecture.

Future renderer-thread expansion requires separate architecture disposition.

## 14. Geometry boundary

P0-T003 does not implement the final OCCT tessellation pipeline.

Test scenes use render-neutral test mesh data.

The intended future direction remains:

BIM/model
   ->
geometry/render extraction
   ->
render-neutral mesh data
   ->
viewport

Persistent BIM identity must not depend on graphics-resource identity.

## 15. Performance policy

Performance measurements are observational only.

Evidence may include:

- startup timing;
- first-frame timing;
- frame samples;
- scene triangle count;
- draw/submit observations;
- GPU/backend identity;
- resolution;
- DPI scale;
- memory observations when practical.

No production FPS SLA is established.

## 16. Known future considerations

The Gate does not claim that P0-T003 solves:

- final multi-viewport design;
- GPU picking;
- CAD hidden-line rendering;
- 2D documentation rendering;
- production offscreen rendering;
- production tessellation;
- large-model streaming.

Any bgfx single-surface or multi-window constraint discovered during the
frozen-baseline capability probe must be recorded explicitly as evidence and
must not be hidden by a workaround.

Grid and axis linework may use geometry suitable for the renderer rather than
assuming a future production CAD-line primitive.

## 17. Qt licensing / maintenance evidence

Phase A must record:

- exact Qt version;
- exact Qt package/features used;
- license identity;
- intended dynamic-linking posture;
- available maintenance/patching posture at the frozen baseline.

No licensing conclusion may be fabricated from package name alone.

## 18. Architecture enforcement

CI architecture checks must prevent at least:

- Qt includes outside approved desktop/platform areas;
- bgfx includes outside approved viewport implementation areas;
- D3D headers outside private Windows renderer/platform code;
- OCCT includes in viewport public API;
- desktop dependencies entering model/geometry/foundation;
- viewport implementation depending on BIM semantic classes.

## 19. Non-goals

Explicitly outside P0-T003:

- Wall / Door / Slab;
- BIM entity implementation;
- IFC;
- DWG;
- RVT;
- SQLite persistence;
- project-file format;
- undo/redo implementation;
- command-system implementation;
- dependency-graph implementation;
- persistent BIM IDs;
- topological references;
- production OCCT tessellation;
- complete selection system;
- selection sets;
- snapping;
- grips;
- measure tools;
- section boxes;
- final clipping architecture;
- annotations;
- 2D sheets;
- production PBR/material system;
- final ribbon/UI;
- plugin framework;
- AI interaction.

## 20. Acceptance criteria

AC-001:

Work starts exactly from 3f2230be5fcd796c370f485975547112ad52d2e3.

AC-002:

One isolated P0-T003 worktree and task branch.

AC-003:

Exact Qt version/features resolved from the frozen vcpkg baseline.

AC-004:

Exact bgfx version/features resolved from the frozen baseline or implementation
stops for Architecture Authority disposition.

AC-005:

Dependency and license evidence recorded.

AC-006:

Qt confined to approved Desktop/platform boundary.

AC-007:

bgfx confined to approved viewport implementation/private bridge boundary.

AC-008:

No Qt/bgfx/D3D/native-window types leak into BIM/domain public APIs.

AC-009:

No direct OCCT dependency from viewport public API.

AC-010:

Neutral RenderMeshData seam satisfies the Gate contract.

AC-011:

Desktop application builds on the authoritative Windows x64 toolchain.

AC-012:

Desktop application launches successfully.

AC-013:

Authoritative renderer backend reports D3D11.

AC-014:

V01 and V02 render successfully.

AC-015:

V03 and V04 execute successfully with observational performance evidence only.

AC-016:

Orbit, pan and zoom pass.

AC-017:

Resize, minimize/restore and close lifecycle pass.

AC-018:

Post-show native-handle acquisition and surface recreation behavior pass.

AC-019:

HiDPI, DPR change and cross-monitor DPI behavior pass.

AC-020:

Renderer-shutdown-before-QApplication-teardown behavior passes.

AC-021:

Screen-to-world ray generation passes.

AC-022:

CPU-owned camera/ray/mesh tests run without requiring an interactive GPU
desktop session; any bgfx no-present test capability used is first verified in
Phase A.

AC-023:

Build, unit, integration, architecture, static-analysis and license gates pass.

AC-024:

Independent implementation review ends with BLOCKER=0 and MAJOR=0, followed
by a clean implementation commit/worktree and Architecture Authority closure.

## 21. Required evidence

Evidence must record, where applicable:

- exact Qt version/features;
- exact bgfx version/features;
- licensing identities;
- renderer backend;
- GPU adapter;
- logical window dimensions;
- physical-pixel dimensions;
- DPI scale;
- scene ID;
- vertex count;
- index count;
- triangle count;
- camera parameters;
- frame samples;
- first-frame result;
- resize result;
- minimize/restore result;
- DPI-change result;
- cross-monitor result;
- native-handle lifecycle result;
- shutdown-order result;
- orbit result;
- pan result;
- zoom result;
- ray-generation result;
- headless/CPU-test result;
- repeatability result;
- overall_passed.

## 22. Repeatability

The spike must exercise at least 20 initialize/render/shutdown cycles in the
authoritative test path.

Camera/ray transformations must have deterministic automated tests.

## 23. Independent architecture consultation

Targeted Kimi architecture consultation:

PASS

BLOCKER:

0

MAJOR:

0

MINOR:

4

NOTE:

6

Architecture Authority disposition:

P0T003-MIN-01:

ACCEPTED

P0T003-MIN-02:

ACCEPTED

P0T003-MIN-03:

ACCEPTED

P0T003-MIN-04:

ACCEPTED WITH CLARIFICATION

The headless requirement means testable non-GPU CPU logic plus a verified
non-presented renderer test path when available from the frozen bgfx
baseline. It does not authorize creation of a second production renderer.

Notes 01-06:

RECORDED / NON-BLOCKING

ACR:

NONE

## 24. Implementation phases

Phase A:

Read-only baseline, dependency, license and renderer-capability probe.

Phase B:

Contract Design Check, including RenderMeshData handedness/winding and
platform-lifecycle contract.

Phase C:

Dependency/CMake integration.

Phase D:

Viewport-neutral public contract.

Phase E:

bgfx renderer adapter.

Phase F:

Qt Widgets desktop shell.

Phase G:

private Qt/native-surface/bgfx bridge.

Phase H:

test scene and camera/input.

Phase I:

resize/lifecycle/HiDPI.

Phase J:

failure corpus and repeatability.

Phase K:

architecture/static/license enforcement.

Phase L:

machine-readable evidence.

Phase M:

Claude implementation handover.

Phase N:

authoritative Windows verification.

Phase O:

Kimi independent implementation review.

Phase P:

Architecture Authority acceptance, closure and controlled integration.

## 25. Gate disposition

BIM-AG-P0-T003 v1.0

APPROVED + LOCKED

Product Authority approval:

CONFIRMED

Architecture Authority approval:

CONFIRMED

Targeted architecture consultation:

PASS

Implementation:

NOT YET AUTHORIZED

The only next authorized task action is Phase A read-only dependency and
capability verification, followed by Phase B Contract Design Check.

No production implementation may start until Architecture Authority releases
the P0-T003 Implementation Brief / authorization.