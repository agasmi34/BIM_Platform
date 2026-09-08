# P0-T003 - Implementation Brief

Brief:

BIM-TASK-P0-T003-CLAUDE v1.0

Task:

P0-T003 - Desktop + Viewport Spike

Status:

RELEASED + LOCKED

Implementation authorization:

NOT YET GRANTED

Authoritative Architecture Gate commit:

cf7a971903d8103143b2b94e94a4d185d86ce42b

Architecture Gate tree:

daec9128dae8423880b28434b5bfc24b855d118c

Authoritative main baseline:

3f2230be5fcd796c370f485975547112ad52d2e3

Frozen vcpkg baseline:

f89a4a1da4e3176a8d1a14c1825b9b2f98e48843

## 1. Authority and release condition

This Brief implements the already-approved BIM-AG-P0-T003 v1.0.

Phase A dependency/capability verification is ACCEPTED.

Phase B Contract Design Check is ACCEPTED WITH Architecture Authority
clarifications.

ACR:

NONE

This Brief freezes the implementation contract and exact implementation
footprint. It does NOT itself authorize production implementation.

Production implementation remains blocked until Architecture Authority issues
a separate P0-T003 implementation-authorization record after the controlled
toolchain/execution preflight.

Claude must not write production code merely because this Brief exists.

## 2. Locked technology identities

qtbase:

6.11.1#1

Qt desktop technology:

Qt 6 + Qt Widgets

Qt Quick:

OUT OF SCOPE

QRhi viewport path:

FORBIDDEN

bgfx:

1.129.8940-496#1

bgfx Windows packaging:

STATIC ONLY

bgfx default feature at the frozen port:

multithreaded

Architecture Authority decision:

bgfx default-features = false

The bgfx multithreaded feature is NOT authorized for P0-T003.

P0-T003 may explicitly enable only the bgfx tools feature required to build
the P0-T003 shader assets. No other bgfx feature is implicitly authorized.

Authoritative live backend:

bgfx RendererType::Direct3D11

No first-party ID3D11 interface, d3d11.h integration, or direct D3D rendering
path is authorized.

Qt linkage posture:

dynamic through the existing x64-windows triplet.

Qt distribution/legal compliance:

separate distribution gate; not declared solved by vcpkg metadata.

## 3. Module and dependency shape

The implementation target graph is:

bim::viewport
    project-owned neutral viewport/camera/mesh/lifecycle contract
    no Qt
    no bgfx
    no D3D
    no Windows native types
    no OCCT
    no BIM semantic/model/query dependency in P0-T003

bim::viewport_bgfx
    depends on bim::viewport + bgfx
    owns all first-party bgfx includes
    owns renderer implementation and shader use
    exposes only project-owned neutral integration types
    exposes no bgfx type in its integration header

bim_desktop_spike
    Qt Widgets desktop executable
    owns Qt types
    embeds a private QWindow-derived native viewport surface
    links bim::viewport and bim::viewport_bgfx
    does not include bgfx headers
    does not include OCCT
    does not include raw D3D headers

Root composition adds viewport before desktop.

P0-T003 does not connect model/query/geometry_api to the viewport. Test and
evidence scenes are neutral spike meshes. The future BIM-to-render extraction
owner is intentionally deferred.

## 4. Neutral coordinate and mesh contract

Project world/render semantic convention:

right-handed

up axis:

+Z

+X / +Y:

project local plan axes only; do not label these East/North until a later
georeferencing contract exists.

RenderMeshData:

- triangle list only;
- Position3f = three float32 values;
- Normal3f = optional three-float32 per-vertex channel;
- if normals are present, normal count equals position count;
- indices are uint32;
- index count is a positive multiple of three;
- every index is less than position count;
- empty mesh is InvalidMesh;
- index/vertex count beyond uint32 addressability is InvalidMesh;
- zero-area triangles and general geometry-quality validation are not viewport
  responsibilities in P0-T003;
- positions are already in a local render frame;
- one render unit equals one model unit;
- survey/global coordinates are not submitted directly as GPU vertex
  positions;
- triangle front face is CCW when viewed from outside;
- supplied normals are outward-facing and consistent with the chosen winding.

No GPU interleaving/layout rule appears in the public contract. bgfx buffer
packing remains private.

## 5. Precision contract

GPU mesh positions/normals:

float32

CPU camera/ray/math:

double precision

The public camera contract stores semantic parameters, not backend projection
matrices:

- eye;
- target;
- world-up;
- vertical FOV in radians;
- aspect ratio;
- near plane;
- far plane.

The public API does not expose a matrix with an assumed backend clip-space
depth convention.

Backend view/projection conversion belongs entirely to bim::viewport_bgfx.

## 6. Camera and input contract

Camera:

right-handed + Z-up, same local render frame as RenderMeshData.

Validation:

- eye/target/up values finite;
- eye != target;
- up not degenerate/parallel to forward;
- 0 < near < far;
- aspect finite and > 0;
- vertical FOV finite and in a sensible open interval.

Orbit:

moves eye around target; target remains fixed.

Pan:

moves eye and target together along camera right/up.

Wheel zoom:

dolly toward/away from target; not FOV zoom in P0-T003.

Input types are neutral POD/value types only. Qt event types terminate in the
desktop layer.

## 7. Screen-to-world ray contract

ScreenToWorldRay is CPU-only and deterministic.

Input coordinates:

physical framebuffer pixels.

Desktop converts Qt logical coordinates to physical pixels before calling the
neutral function.

Viewport origin:

top-left.

Output:

double-precision origin + normalized direction.

The function derives the ray from semantic camera parameters and viewport
dimensions; it does not depend on a public backend projection matrix.

Invalid zero-sized viewport:

InvalidState.

Non-finite input:

InvalidInput.

Invalid camera:

InvalidState.

No Qt/bgfx dependency is permitted.

## 8. Error contract

Public error identity is code-based and allocation-free:

ViewportErrorCode values:

- None
- InvalidInput
- InvalidState
- SurfaceUnavailable
- RendererInitializationFailed
- UnsupportedBackend
- InvalidMesh
- ResourceCreationFailed
- ResourceNotFound
- BackendFailure

Status contains the project-owned code only.

Result<T> contains either a value or a project-owned error code.

Do not place dynamically allocated diagnostic strings inside the public error
contract merely to carry third-party text.

Detailed Qt/bgfx/backend diagnostics belong in private logging/evidence.

Do not declare allocating/moving Result factories noexcept unless the actual
implementation makes that guarantee.

bool is reserved for non-failing predicates such as IsValid().

## 9. RenderMeshHandle contract

P0-T003 has one public rendering-resource handle:

RenderMeshHandle

It is:

- opaque;
- default-invalid;
- non-owning;
- renderer-session scoped;
- index + generation/epoch protected;
- explicitly destroyed through the renderer;
- never serialized;
- never a BIM ID;
- never a persistent geometry/topology reference.

Ordinary resize/reset does not invalidate handles.

A full renderer shutdown/re-initialization increments the renderer epoch and
invalidates all handles from the prior epoch.

Stale or double-destroy use returns ResourceNotFound and must never alias a
newer resource occupying the same slot.

No general public resource framework, material handle, texture handle, or
shader handle is authorized in P0-T003.

## 10. Viewport lifecycle

Stable neutral lifecycle states:

- Uninitialized
- SurfaceUnavailable
- Ready
- Suspended
- ShuttingDown
- Destroyed

Surface creation/recreation/destruction are events, not persistent public
states.

Minimum transitions:

Uninitialized -> SurfaceUnavailable

SurfaceUnavailable -> Ready
when a valid realized native surface and non-zero physical size are available.

Ready -> Ready
for ordinary non-zero resize / DPR change with the same native surface,
using renderer reset/reconfiguration.

Ready -> Suspended
for minimize or zero physical size.

Suspended -> Ready
after restore with a valid surface and non-zero size.

Ready or Suspended -> SurfaceUnavailable
when the native surface is about to be destroyed.

Native surface recreation:

controlled renderer shutdown -> native surface reacquisition -> renderer
re-init -> Ready.

Do not assume bgfx PlatformData may be hot-swapped after init.

Any live resource handles from the previous renderer epoch are invalid after
full re-init.

Any state -> ShuttingDown -> Destroyed for final shutdown.

Renderer shutdown must complete before the QWindow/native surface and
QApplication are torn down.

## 11. Qt/native-surface bridge

Desktop shell:

QMainWindow / Qt Widgets.

Viewport native surface:

private QWindow-derived class embedded into the Widgets shell through the
normal Qt window-container mechanism.

The desktop layer may use QWindow/QWidget/QEvent/QPlatformSurfaceEvent and Qt
DPI/screen APIs.

The desktop layer does NOT include bgfx headers.

The neutral integration value passed to bim::viewport_bgfx may contain:

- opaque native_window pointer;
- physical width;
- physical height;
- device-pixel ratio.

It must not expose HWND/QWindow/bgfx PlatformData as a public BIM/domain type.

The bridge responds to:

- surface creation;
- SurfaceAboutToBeDestroyed;
- exposure;
- resize;
- minimize/restore;
- devicePixelRatio changes;
- screen changes;
- application shutdown.

Native handles are acquired only after the native surface is realized and are
re-resolved after native surface recreation.

## 12. bgfx threading and renderer contract

vcpkg dependency:

bgfx default-features false.

Do not enable bgfx multithreaded.

No dedicated render thread exists in P0-T003.

bgfx init, submit/frame, reset and shutdown are owned by one application/API
thread.

D3D11 is selected explicitly. Auto-selecting a different backend and calling
that PASS is forbidden.

The concrete renderer adapter may expose project-owned operations equivalent
to:

- Initialize(surface)
- Shutdown()
- ResetSurface(metrics)
- CreateMesh(RenderMeshData)
- DestroyMesh(RenderMeshHandle)
- DrawFrame(camera, mesh handles)
- BackendInfo()

Signatures may be adjusted minimally for C++ correctness, but no bgfx/native
type may leak from the integration header.

## 13. Shaders

P0-T003 uses the accepted bgfx dependency with:

default-features = false

tools feature = explicitly enabled for P0-T003 shader compilation.

The multithreaded feature remains disabled.

Shader source belongs under:

src/viewport/bgfx/shaders/

Minimum assets:

varying.def.sc
vs_p0_t003.sc
fs_p0_t003.sc

The authoritative Windows live path compiles/uses the D3D11-compatible shader
artifact through the bgfx toolchain.

Do not hand-author fake DXBC blobs.

Do not introduce a direct D3D shader pipeline outside bgfx.

If the exact frozen bgfx package/tool targets differ from assumed CMake names,
Claude must use the actual installed package configuration without changing
the architecture or dependency baseline. A dependency-baseline change is an
ACR stop.

## 14. Spike scenes

Scene builders are desktop/evidence-only and do not create BIM semantics.

V01:
single indexed cube.

V02:
grid + XYZ axes + cube.

V03:
1,000 repeated boxes. P0-T003 may construct a combined neutral mesh; a public
instancing architecture is not required.

V04:
representative medium neutral mesh scene.

V05:
large-coordinate scenario represented in local render coordinates.

No performance threshold is pass/fail.

Frame timings are observational evidence only.

## 15. Headless and test split

CPU-only tests require no GPU and no Qt application:

- mesh validation;
- camera math;
- ray generation;
- lifecycle state machine.

bgfx headless/non-presented integration:

- exact frozen bgfx version only;
- verifies init/resource/frame/shutdown behavior without a presented desktop
  when supported by the exact package;
- must not silently fall back to an unverified architecture.

Authoritative live evidence:

Windows + Qt Widgets + native QWindow surface + explicit bgfx D3D11 backend.

## 16. Repeatability

The authoritative viewport-spike verification performs at least 20 clean
process-level initialize/render/shutdown cycles.

Each process must return success.

The final evidence run additionally records the complete scene/lifecycle
evidence.

This is correctness/repeatability evidence, not an FPS benchmark.

## 17. HiDPI / cross-monitor evidence

Unit/integration tests must exercise deterministic physical-size/DPR
transition logic.

Live evidence records:

- logical size;
- physical size;
- DPR;
- screen identity;
- cross-monitor attempt;
- whether two available screens have differing DPR;
- actual resulting DPR/physical-size transition.

If the authoritative operator machine does not provide two displays with
different DPR, the evidence must record NOT_AVAILABLE rather than inventing a
PASS. AC-019 remains pending until real differing-DPI live evidence is
obtained.

## 18. vcpkg manifest shape

vcpkg-configuration.json:

MUST NOT CHANGE.

vcpkg.json adds only:

qtbase:
- default-features false
- features: widgets

bgfx:
- default-features false
- features: tools

The frozen registry determines the exact accepted versions.

No override or new baseline is authorized.

Current x64-windows triplet remains unchanged.

Qt remains dynamically linked through the normal x64-windows posture.

bgfx remains static on Windows because the accepted port requires it.

## 19. License inventory

The direct-dependency license inventory expands from five to seven direct
dependencies by adding:

- qtbase
- bgfx

scripts/ci/license-inventory.ps1 must continue to copy the actual current-run
vcpkg-installed copyright file and fail closed if it is missing.

Do not fabricate or hand-retype license text.

Expected new generated tracked artifacts:

third_party/licenses/qtbase.LICENSE.txt
third_party/licenses/bgfx.LICENSE.txt

LICENSES.md and third_party/licenses/README.md are updated to record the two
dependencies and the separate Qt distribution/legal gate.

A successful technical P0-T003 spike is not a legal conclusion about external
Qt distribution.

## 20. Architecture enforcement

Extend tools/architecture_checker.cmake using the existing comment-aware
helper for code-level lexical checks where lexical checks are necessary.

Do not repeat the P0-T002 comment false-positive defect.

Add rule selectors equivalent to:

VIEWPORT_PUBLIC_NEUTRAL

- viewport public headers contain no Qt/bgfx/D3D/Windows/OCCT tokens;
- viewport public headers contain no BIM semantic/model/query/geometry
  includes.

QT_DESKTOP_ONLY

- Qt include/API tokens in first-party src belong only under src/desktop/**.

BGFX_VIEWPORT_OWNER

- first-party bgfx include/API tokens belong only under src/viewport/bgfx/**.

NO_DIRECT_D3D

- first-party code must not include d3d11.h or use ID3D11*/D3D11_* API
  directly.

Existing R1-R7 remain effective.

Every new rule has:

- a positive CTest against the real src tree;
- a dedicated negative fixture test proving the rule rejects a real code-level
  violation;
- exact-name anchoring in scripts/ci/architecture.ps1.

## 21. Exact P0-T003 test names

New unit tests:

unit_viewport_mesh_contract
unit_viewport_camera
unit_viewport_ray
unit_viewport_lifecycle

New integration tests:

integration_viewport_bgfx_headless
integration_viewport_bgfx_resource_lifecycle

New architecture tests:

arch_viewport_public_neutral
arch_qt_desktop_only
arch_bgfx_viewport_owner
arch_no_direct_d3d

arch_p0_t003_viewport_fixture_rejected
arch_p0_t003_qt_fixture_rejected
arch_p0_t003_bgfx_fixture_rejected
arch_p0_t003_d3d_fixture_rejected

All existing P0-T001 and P0-T002 CTest tests remain registered and passing.

The live desktop evidence executable is not disguised as a generic headless
CTest. It is exercised by scripts/ci/viewport-spike.ps1 and the authoritative
Runbook D.

## 22. Evidence executable / desktop evidence mode

bim_desktop_spike owns a P0-T003 evidence mode.

The evidence mode writes only to an explicit caller-supplied JSON path under
the build/evidence tree.

Required JSON fields include:

- task/gate/brief identity;
- Qt version;
- bgfx version;
- selected renderer backend;
- GPU adapter/backend information available from bgfx;
- logical/physical window dimensions;
- DPR;
- screen count and screen identities;
- scene IDs;
- vertex/index/triangle counts;
- camera semantic parameters;
- lifecycle event results;
- surface recreation result;
- minimize/restore result;
- resize result;
- DPR change result;
- cross-monitor result / availability;
- orbit/pan/dolly result;
- ray result;
- repeatability cycle results;
- frame timing samples marked observational;
- overall_passed.

The executable must return non-zero if any required correctness result fails.

Performance timing alone never fails the run.

## 23. CI viewport-spike script

scripts/ci/viewport-spike.ps1:

- requires an already-built authoritative build directory;
- locates bim_desktop_spike deterministically;
- writes evidence only under build/.../evidence/P0-T003;
- executes the required process-level repeatability cycles;
- executes final live evidence mode;
- validates JSON existence/non-empty structure;
- validates explicit D3D11 backend identity;
- validates overall_passed;
- propagates non-zero exit codes;
- reports cross-monitor differing-DPI availability honestly;
- never edits source/docs;
- never changes dependencies.

This is a task-specific evidence job, like P0-T002 geometry-spike.ps1. It does
not have to be inserted into the generic five-job run-all.ps1.

## 24. Verification Runbook D

Verification-RunbookD-v1.0.ps1 is part of the authorized implementation
footprint.

It must fail closed and verify at least:

1. exact P0-T003 task branch;
2. expected pre-verification HEAD supplied by Architecture Authority;
3. exact authorized implementation footprint;
4. main baseline unchanged and clean;
5. x64 / VS 2022 17.14 / v143 environment;
6. effective compiler MSVC and matching validated cl.exe;
7. CMake 4.4.2;
8. Ninja 1.12.1;
9. clang-format / clang-tidy presence;
10. frozen vcpkg baseline unchanged;
11. resolved qtbase exactly 6.11.1#1;
12. resolved bgfx exactly 1.129.8940-496#1;
13. bgfx multithreaded feature absent from the resolved feature set;
14. Qt Widgets present;
15. format job;
16. configure/build/full CTest job;
17. static-analysis job;
18. architecture job with every required exact architecture test;
19. license-inventory job;
20. viewport-spike evidence job;
21. all P0-T001 regression tests exact-name anchored;
22. all P0-T002 tests exact-name anchored;
23. all P0-T003 tests exact-name anchored;
24. evidence JSON schema and backend identity;
25. final task/main Git invariants and candidate identity.

Runbook output/transcript is authoritative evidence.

Runbook D must not stage or commit.

## 25. Independent review

After authoritative verification passes, Kimi performs independent read-only
review.

Kimi must review:

- exact candidate footprint;
- public contract neutrality;
- Qt/bgfx/D3D/OCCT ownership;
- lifecycle/surface recreation logic;
- threading discipline;
- error translation;
- shader build path;
- evidence correctness;
- architecture checker;
- license provenance;
- Runbook transcript;
- regression results.

Acceptance requires:

BLOCKER = 0
MAJOR = 0

Kimi does not modify production code.

Any return to Claude after review is minimum-delta only.

## 26. ACR stop conditions

Stop and raise ACR if implementation requires any of the following:

- vcpkg baseline advancement;
- a different Qt major/minor technology direction;
- Qt Quick / QRhi viewport path;
- a different renderer abstraction;
- enabling bgfx multithreaded;
- a dedicated render thread;
- direct D3D rendering outside bgfx;
- direct OCCT access from viewport/desktop;
- BIM semantic/model dependency inside P0-T003 viewport;
- public Qt/bgfx/native/OCCT type leakage;
- changing RH/Z-up or mesh winding conventions;
- changing the local-coordinate precision policy;
- adding a persistent resource/BIM identity system;
- changing the task's performance evidence into an SLA;
- inability to implement surface recreation without materially changing the
  locked lifecycle architecture.

Ordinary C++ signature corrections, actual installed CMake target names,
warning fixes, or exact private helper decomposition do not require an ACR if
the locked contract and exact path footprint remain unchanged.

## 27. Exact implementation footprint

The implementation candidate and implementation commit may touch exactly the
following 59 paths and no others.

MODIFY - 12:

1. CMakeLists.txt
2. vcpkg.json
3. LICENSES.md
4. src/desktop/README.md
5. src/viewport/README.md
6. tests/unit/CMakeLists.txt
7. tests/integration/CMakeLists.txt
8. tests/architecture/CMakeLists.txt
9. tools/architecture_checker.cmake
10. scripts/ci/architecture.ps1
11. scripts/ci/license-inventory.ps1
12. third_party/licenses/README.md

ADD - 47:

13. third_party/licenses/bgfx.LICENSE.txt
14. third_party/licenses/qtbase.LICENSE.txt
15. src/viewport/CMakeLists.txt
16. src/viewport/include/bim/viewport/error.hpp
17. src/viewport/include/bim/viewport/math.hpp
18. src/viewport/include/bim/viewport/mesh.hpp
19. src/viewport/include/bim/viewport/camera.hpp
20. src/viewport/include/bim/viewport/ray.hpp
21. src/viewport/include/bim/viewport/input.hpp
22. src/viewport/include/bim/viewport/lifecycle.hpp
23. src/viewport/src/mesh.cpp
24. src/viewport/src/camera.cpp
25. src/viewport/src/ray.cpp
26. src/viewport/src/lifecycle.cpp
27. src/viewport/bgfx/CMakeLists.txt
28. src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp
29. src/viewport/bgfx/src/renderer.cpp
30. src/viewport/bgfx/src/renderer_impl.hpp
31. src/viewport/bgfx/shaders/varying.def.sc
32. src/viewport/bgfx/shaders/vs_p0_t003.sc
33. src/viewport/bgfx/shaders/fs_p0_t003.sc
34. src/desktop/CMakeLists.txt
35. src/desktop/src/main.cpp
36. src/desktop/src/main_window.hpp
37. src/desktop/src/main_window.cpp
38. src/desktop/src/viewport_window.hpp
39. src/desktop/src/viewport_window.cpp
40. src/desktop/src/viewport_bridge.hpp
41. src/desktop/src/viewport_bridge.cpp
42. src/desktop/src/spike_scene.hpp
43. src/desktop/src/spike_scene.cpp
44. src/desktop/src/evidence_mode.hpp
45. src/desktop/src/evidence_mode.cpp
46. tests/unit/unit_viewport_mesh_contract.cpp
47. tests/unit/unit_viewport_camera.cpp
48. tests/unit/unit_viewport_ray.cpp
49. tests/unit/unit_viewport_lifecycle.cpp
50. tests/integration/integration_viewport_bgfx_headless.cpp
51. tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp
52. tests/fixtures/p0_t003_bad_viewport_api/viewport/include/bim/viewport/leaky.hpp
53. tests/fixtures/p0_t003_bad_qt_owner/model/src/leaky_qt.cpp
54. tests/fixtures/p0_t003_bad_bgfx_owner/desktop/src/leaky_bgfx.cpp
55. tests/fixtures/p0_t003_bad_direct_d3d/desktop/src/leaky_d3d.cpp
56. scripts/ci/viewport-spike.ps1
57. Verification-RunbookD-v1.0.ps1
58. docs/evidence/P0-T003/CLAUDE_HANDOVER.md
59. docs/evidence/P0-T003/CLAUDE_HANDOVER.json

No path in this list is conditional.

scripts/ci/run-all.ps1 remains byte-identical: it is the provider-neutral
five-job core gate.

scripts/ci/README.md remains byte-identical unless a later Architecture
Authority amendment explicitly authorizes a documentation-only update.

viewport-spike.ps1 is task-specific and is invoked directly by Runbook D.

No other path may be changed without Architecture Authority disposition.

## 28. Implementation phases after authorization

C0:
controlled toolchain / VCPKG_ROOT execution preflight.

C1:
dependency manifest and CMake integration.

D:
neutral viewport contract.

E:
bgfx renderer adapter + shaders.

F:
Qt Widgets desktop shell + QWindow native surface bridge.

G:
spike scene/evidence mode.

H:
unit/integration tests.

I:
architecture checker rules + negative fixtures.

J:
license inventory.

K:
viewport-spike evidence script.

L:
Runbook D.

M:
Claude handover.

N:
operator-controlled import into the authoritative P0-T003 worktree.

O:
authoritative Windows verification.

P:
independent Kimi review.

Q:
Architecture Authority implementation acceptance / commit authorization.

R:
closure and controlled integration.

## 29. Current disposition

BIM-TASK-P0-T003-CLAUDE v1.0

RELEASED + LOCKED

Phase A:

ACCEPTED

Phase B:

ACCEPTED

ACR:

NONE

Production implementation:

NOT YET AUTHORIZED

Next:

controlled C0 VCPKG_ROOT/toolchain execution preflight and then explicit
P0-T003 implementation authorization.
