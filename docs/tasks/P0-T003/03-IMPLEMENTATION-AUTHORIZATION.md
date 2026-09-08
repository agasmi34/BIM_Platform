# P0-T003 - Implementation Authorization

Authorization:

BIM-TASK-P0-T003-IMPLEMENTATION-AUTH v1.0

Task:

P0-T003 - Desktop + Viewport Spike

Status:

AUTHORIZED

Architecture Gate:

BIM-AG-P0-T003 v1.0

Architecture Gate commit:

cf7a971903d8103143b2b94e94a4d185d86ce42b

Implementation Brief:

BIM-TASK-P0-T003-CLAUDE v1.0

Implementation Brief commit:

40a569b6588525e57e1a3f51502670bbd78f96bb

Implementation Brief tree:

dce5abe95c10387cadf05443241552c89bf7844b

Implementation Brief SHA256:

DC930CD3C44DD7796E055FD3CEF0CB983132314DEFD75BBA7F74A329A933D5A8

Authoritative main baseline:

3f2230be5fcd796c370f485975547112ad52d2e3

Frozen vcpkg baseline:

f89a4a1da4e3176a8d1a14c1825b9b2f98e48843

ACR:

NONE

## 1. Authorization decision

Architecture Authority authorizes production implementation of P0-T003 under
the exact contract and exact 59-path footprint frozen by
BIM-TASK-P0-T003-CLAUDE v1.0.

This authorization does not reopen architecture.

Claude is the Implementation Engineer.

Claude may create/modify production, test, verification, evidence, CMake and
task-specific CI content only within the exact 59-path footprint.

Claude must not modify any other path.

Claude must not stage, commit, merge, rebase, push, fast-forward main, or
alter Git history.

Architecture Authority retains commit, acceptance, closure and integration
authority.

## 2. Mandatory repository identity

Authorized repository/worktree:

D:\Projects\BIM-Platform-WT-P0-T003

Authorized branch:

task/P0-T003-desktop-viewport-spike

The implementation session must have direct read/write access to this exact
worktree.

If Claude cannot access this exact worktree, it must STOP.

Claude must not implement P0-T003 in:

D:\Projects\BIM-Platform-WT-P0-T001

D:\Projects\BIM-Platform-WT-P0-T002

D:\Projects\BIM-Platform

or any substitute/copy unless Architecture Authority explicitly authorizes a
controlled snapshot/import workflow.

Before writing production code Claude must verify:

- exact branch;
- exact authorization HEAD supplied by Architecture Authority after this
  authorization commit;
- clean worktree;
- zero staged paths;
- main remains untouched.

## 3. Accepted Phase A / Phase B state

Phase A:

ACCEPTED

Phase B:

ACCEPTED

C0 toolchain execution preflight:

PASS

C0 ACR:

NONE

No further discovery or architecture redesign is authorized.

Implementation must follow the locked Brief.

## 4. Accepted execution environment

Runtime mode:

VISUAL_STUDIO_BUNDLED_VCPKG

VCPKG_ROOT:

C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\vcpkg

vcpkg executable version:

2025-11-19-da1f056dc0775ac651bea7e3fbbf4066146a55f3

Target triplet:

x64-windows

Frozen baseline:

f89a4a1da4e3176a8d1a14c1825b9b2f98e48843

CMake:

4.4.2

Ninja:

1.12.1

Historically validated Ninja executable:

C:\PROGRA~2\MICROS~4\2022\BUILDT~1\Common7\IDE\COMMON~1\MICROS~1\CMake\Ninja\ninja.exe

Visual Studio Build Tools:

17.14.37614.0

VCTools:

14.44.35207

Authoritative x64 compiler:

C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe

## 5. vcpkg/CMake execution authorization

CMake configure/build/test is now authorized in the P0-T003 task worktree.

Manifest-mode dependency resolution is authorized through the accepted
Visual Studio bundled vcpkg runtime.

CMake/vcpkg may download/build/install packages required by the frozen
manifest into the task build tree/cache as normal manifest-mode side effects.

The following remain forbidden:

- vcpkg baseline advancement;
- vcpkg update/upgrade of the registry baseline;
- classic/global package installation as a substitute for manifest mode;
- editing vcpkg-configuration.json;
- changing the target triplet;
- adding overlays or alternate registries without an ACR;
- changing accepted qtbase/bgfx identities;
- enabling bgfx multithreaded.

If frozen dependency resolution fails in a way that requires any forbidden
change, Claude must STOP and return evidence for Architecture Authority.

## 6. Locked dependency contract

qtbase:

6.11.1#1

Qt:

Qt Widgets desktop shell.

Qt Quick:

FORBIDDEN.

QRhi viewport:

FORBIDDEN.

Qt linkage posture:

dynamic through x64-windows.

bgfx:

1.129.8940-496#1

bgfx manifest posture:

default-features = false

authorized explicit feature:

tools

for shader compilation only.

bgfx multithreaded:

NOT AUTHORIZED.

Authoritative live renderer:

Direct3D11 selected through bgfx.

Direct first-party D3D11 rendering/API ownership:

FORBIDDEN.

## 7. Locked implementation architecture

Claude must preserve all Brief contracts including:

- RH + Z-up local render semantics;
- CCW outward mesh winding;
- float32 GPU mesh positions/normals;
- uint32 indices;
- double-precision CPU camera/ray math;
- semantic camera API with no backend clip-space leakage;
- project-owned code-only public errors;
- one public RenderMeshHandle only;
- renderer epoch invalidation after full re-init;
- Qt Widgets shell;
- private QWindow-derived native viewport surface;
- native surface recreation by controlled renderer shutdown/re-init;
- single-owner bgfx lifecycle;
- no dedicated render thread;
- no Qt/bgfx/D3D/Windows/OCCT/BIM semantic leakage from neutral viewport
  public headers;
- no OCCT access from viewport/desktop;
- neutral spike scenes only;
- no performance SLA.

## 8. Exact implementation footprint

Exactly 59 paths are authorized.

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

No path is conditional.

scripts/ci/run-all.ps1 remains byte-identical.

scripts/ci/README.md remains byte-identical.

vcpkg-configuration.json remains byte-identical.

No project-control, Gate, Brief or Authorization document may be changed by
Claude during implementation.

## 9. Implementation execution phases

Claude executes minimum-delta implementation in this order:

C1:
dependency manifest and CMake integration.

D:
neutral viewport contract.

E:
bgfx renderer adapter and shaders.

F:
Qt Widgets desktop shell and QWindow bridge.

G:
spike scenes and evidence mode.

H:
unit/integration tests.

I:
architecture checker rules and negative fixtures.

J:
license inventory.

K:
viewport-spike evidence script.

L:
Verification Runbook D.

M:
Claude handover.

Claude may iterate locally within the authorized footprint to resolve build,
test, static-analysis or architecture-checker failures.

Claude must not expand scope to solve unrelated repository issues.

## 10. Required implementation verification

Before handover Claude must run the strongest locally available verification
without changing architecture:

- format;
- configure;
- build;
- full CTest regression;
- static analysis;
- architecture tests;
- license inventory;
- CPU-only viewport tests;
- bgfx headless/resource-lifecycle tests;
- task-specific viewport-spike evidence where the environment supports live
  Qt/D3D11 execution.

Claude must record commands, versions, pass/fail results and any environment
limitation.

Claude must not convert unavailable live evidence into PASS.

Authoritative Windows operator verification remains separate and is executed
later using Verification-RunbookD-v1.0.ps1.

## 11. Git discipline

Claude must not:

- git add;
- git commit;
- git merge;
- git rebase;
- git push;
- switch branch;
- reset;
- clean;
- stash;
- modify main.

Claude may use read-only Git commands to verify identity/diff/status.

At handover the worktree may be dirty because implementation files are
modified/untracked, but the index must remain empty.

## 12. Handover requirements

Claude writes exactly:

docs/evidence/P0-T003/CLAUDE_HANDOVER.md

docs/evidence/P0-T003/CLAUDE_HANDOVER.json

The handover must contain:

- authorization identity;
- starting HEAD/tree;
- exact changed path list;
- added/modified classification;
- dependency resolution identities;
- build/test/analysis evidence;
- architecture-test evidence;
- license provenance;
- live/headless evidence availability;
- unresolved warnings/limitations;
- ACR status;
- explicit statement that no file outside the authorized 59 paths changed;
- explicit statement that the Git index is empty;
- explicit statement that main was not modified.

Claude must return control to Architecture Authority after handover.

## 13. Stop / ACR conditions

Claude must STOP immediately if:

- the exact P0-T003 worktree cannot be accessed;
- starting authorization HEAD/tree does not match;
- worktree is dirty before implementation begins;
- any required implementation needs a 60th path;
- frozen qtbase/bgfx identities cannot be resolved without changing the
  baseline/registry;
- bgfx multithreaded appears necessary;
- Qt Quick/QRhi appears necessary;
- direct D3D appears necessary;
- OCCT access from viewport/desktop appears necessary;
- BIM model/query semantics appear necessary in the viewport spike;
- native-surface recreation cannot be implemented under the locked
  shutdown/re-init contract;
- a public third-party type leak appears necessary;
- a material architecture change is required.

Ordinary compiler-warning fixes, private helper decomposition, actual package
target-name adaptation and implementation-detail signature corrections within
the locked contract/59 paths do not require an ACR.

## 14. Acceptance authority

Claude implementation completion does not equal acceptance.

After handover:

1. operator/AA inspects exact candidate;
2. authoritative Runbook D is executed;
3. Kimi performs independent read-only review;
4. BLOCKER and MAJOR must both be zero;
5. any correction returned to Claude is minimum-delta only;
6. Architecture Authority alone authorizes staging/commit;
7. Architecture Authority alone accepts/closes/integrates P0-T003.

## 15. Final authorization

BIM-TASK-P0-T003-IMPLEMENTATION-AUTH v1.0

STATUS:

AUTHORIZED

IMPLEMENTATION ENGINEER:

CLAUDE

AUTHORIZED WORKTREE:

D:\Projects\BIM-Platform-WT-P0-T003

AUTHORIZED BRANCH:

task/P0-T003-desktop-viewport-spike

AUTHORIZED FOOTPRINT:

59 PATHS EXACTLY

MAIN:

MUST REMAIN UNTOUCHED

ACR:

NONE

PRODUCTION IMPLEMENTATION:

AUTHORIZED AFTER THIS AUTHORIZATION RECORD IS COMMITTED

RETURN TO ARCHITECTURE AUTHORITY AFTER CLAUDE HANDOVER.
