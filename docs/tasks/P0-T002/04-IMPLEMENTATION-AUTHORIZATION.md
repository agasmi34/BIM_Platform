# P0-T002 - Phase C-M Implementation Authorization

**Document ID:** BIM-TASK-P0-T002-IMPLEMENTATION-AUTH

**Version:** 1.0

**Date:** 2026-09-03

**Architecture Authority:** Product Owner + ChatGPT

**Status:** PHASE C-M AUTHORIZED

## 1. Authority chain

Architecture Gate:

BIM-AG-P0-T002 v1.0

Parent Implementation Brief:

BIM-TASK-P0-T002-CLAUDE v1.0

Implementation Brief Amendment:

BIM-TASK-P0-T002-CLAUDE-A01 v1.0

Amendment commit:

6081a30e03fb11d35d4920c3f7d35c253595270c

Amendment tree:

70a1ed80b0e1b9de0d319be8a688e5dd29c63b0e

## 2. Gate disposition

Phase A + B:

ACCEPTED

OCCT 8.0.1 API verification:

ACCEPTED

Current ACR:

NONE

Phase C-M:

AUTHORIZED

## 3. Implementation transport

Claude must implement only inside the writable file-transfer snapshot:

C:\Users\abdallah\source\P0-T002-IMPLEMENTATION-6081a30\work

This directory is NOT a Git worktree.

Claude must not represent edits there as Git commits, Git staging or live
repository modifications.

The live Git task worktree remains operator-controlled:

D:\Projects\BIM-Platform-WT-P0-T002

Main remains protected.

## 4. Verified OCCT implementation policy

The OCCT header readpack verified version 8.0.1.

Implementation must use API declarations verified from the actual readpack or
settled by authoritative Windows/MSVC compilation.

For the BRepAlgoAPI Boolean family:

- use HasErrors() as the verified Boolean algorithm failure gate;
- do not depend on an unverified IsDone() member on Cut/Fuse/Common;
- contain Standard_Failure, std::exception and unknown exceptions inside the
  adapter.

Builder-specific IsDone() may be used only where verified for that builder.

Do not use remembered-but-unverified OCCT method names.

## 5. Rectangle semantics

RectangleProfile3 axes must be:

- finite;
- non-degenerate;
- orthogonal within caller angular tolerance.

Axis magnitudes may be normalized.

Skew axes must not be silently corrected.

Skew input is DegenerateGeometry.

## 6. Cut semantics

Cut requires positive volumetric intersection.

Qualification:

Common(host, tool)
-> valid 3D common content
-> common volume

Use:

volume_epsilon = linear_tolerance ^ 3

No qualifying 3D volume, including boundary-only contact:

NoIntersection

Positive qualifying volume:

attempt Cut

Boolean algorithm failure:

KernelOperationFailed

Completed but invalid platform result:

InvalidResult

Valid result:

None

## 7. Fuse semantics

Do not use common volume as Fuse precondition.

Use verified geometric separation/proximity behavior.

If:

distance > linear_tolerance

return:

NoIntersection

If:

distance <= linear_tolerance

attempt Fuse.

Therefore:

- J04 exact face contact must reach Fuse;
- J05 gap 0.001 must remain NoIntersection for all mandatory tolerances;
- J06 remains observational.

After Fuse, connected-union success requires exactly one valid resulting 3D
solid.

## 8. Solid ownership

Public:

struct Solid;
using SolidHandle = std::shared_ptr<const Solid>;

The complete Solid definition belongs only to adapter-private implementation.

No public OCCT object accessor is authorized.

No mutable TopoDS_Shape may escape.

## 9. OCCT history

History diagnostics may use the verified Boolean history facilities:

Generated
Modified
IsDeleted
History

Use current NCollection container types.

Do not introduce deprecated TopTools_ListOfShape or
TopTools_IndexedMapOfShape aliases in new first-party code.

Face ordinals remain transient diagnostic observation labels only.

Persistent topology identity remains out of scope.

## 10. Exact implementation footprint

The following 24 implementation paths are authorized.

ADD:

src/geometry/api/include/bim/geometry_api/geometry.hpp
src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp
src/geometry/occt/src/solid_impl.hpp
src/geometry/occt/src/geometry_occt_adapter.cpp
src/geometry/occt/src/spike_diagnostics.cpp
tests/integration/geometry_occt_test_constants.hpp
tests/unit/unit_geometry_api_contract.cpp
tests/integration/integration_geometry_occt_primitive.cpp
tests/integration/integration_geometry_occt_opening_cut.cpp
tests/integration/integration_geometry_occt_join.cpp
tests/integration/integration_geometry_occt_failure_corpus.cpp
tests/integration/integration_geometry_occt_tolerance_matrix.cpp
tests/integration/integration_geometry_occt_coordinate_matrix.cpp
tests/integration/integration_geometry_occt_history.cpp
tests/integration/integration_geometry_occt_repeatability.cpp
tests/integration/p0_t002_geometry_evidence.cpp
scripts/ci/geometry-spike.ps1
Verification-RunbookC-v1.0.ps1

MODIFY:

src/geometry/occt/CMakeLists.txt
tests/unit/CMakeLists.txt
tests/integration/CMakeLists.txt
tests/architecture/CMakeLists.txt
tools/architecture_checker.cmake
scripts/ci/architecture.ps1

No other implementation path may be changed without ACR.

## 11. Separately authorized handover evidence

The exact implementation footprint above governs production/test/CI/runbook
work.

Parent Brief section 36 separately requires and authorizes these two handover
evidence outputs:

docs/evidence/P0-T002/CLAUDE_HANDOVER.md

docs/evidence/P0-T002/CLAUDE_HANDOVER.json

These are evidence outputs, not implementation-scope expansion.

No other docs/evidence file is authorized during Claude implementation unless
Architecture Authority explicitly approves it.

## 12. Evidence behavior

p0_t002_geometry_evidence must use no new JSON dependency.

integration_geometry_occt_evidence must pass an explicit --json output path.

Authoritative geometry-spike evidence remains under the configured Windows
build tree.

Generated build-tree evidence must not be placed into the source candidate.

## 13. Architecture enforcement

The implementation must add:

arch_geometry_api_no_occt_leak

arch_geometry_occt_only_kernel_owner

and retain:

arch_repository_boundaries

arch_checker_detects_violation

scripts/ci/architecture.ps1 must fail if any required architecture test is
missing.

## 14. Existing P0-T001 regression

The six accepted P0-T001 CTest entries must remain intact:

unit_foundation_smoke
unit_model_links_foundation
integration_geometry_occt_probe
integration_persistence_sqlite_memory
arch_repository_boundaries
arch_checker_detects_violation

RunKernelSmokeProbe remains intact.

## 15. Source quality

Do not:

- add a dependency;
- modify vcpkg files;
- modify root CMakeLists.txt;
- modify tests/CMakeLists.txt;
- modify src/geometry/api/CMakeLists.txt;
- introduce BIM semantics into geometry;
- introduce persistent topology identity;
- introduce hidden platform tolerance defaults;
- introduce deprecated OCCT aliases in new first-party code;
- weaken the existing architecture checker to make tests pass.

## 16. Execution provenance

Claude may author implementation and perform whatever self-inspection its
environment permits.

Claude must not claim Windows/MSVC runtime evidence that it did not execute.

Authoritative Windows build/test/evidence execution occurs only after the
candidate is imported into the live task worktree by the Windows Execution
Operator.

## 17. Handover requirement

Before returning the candidate, Claude must produce:

docs/evidence/P0-T002/CLAUDE_HANDOVER.md

docs/evidence/P0-T002/CLAUDE_HANDOVER.json

The handover must identify:

- baseline Amendment commit;
- every changed/added path;
- SHA256 for every changed/added candidate file;
- implemented public API;
- implemented OCCT APIs;
- corpus/test mapping;
- tolerance and coordinate matrices;
- history implementation;
- evidence executable/script behavior;
- Runbook C behavior;
- architecture-checker changes;
- any deviations;
- any unresolved issue;
- tests actually executed by Claude, if any;
- tests NOT executed;
- explicit statement that Windows authoritative verification remains pending;
- explicit statement that live Git main/task worktrees were not modified by
  Claude.

## 18. Stop conditions

Stop and request Architecture Authority action if implementation requires:

- any path outside the authorized implementation/evidence list;
- any new dependency;
- any vcpkg change;
- any BIM semantic type;
- persistent topology identity;
- public OCCT types;
- architecture weakening;
- a change to a locked corpus expected outcome;
- an inability to implement AA-C02, AA-C03 or AA-C04;
- material redesign of the accepted public geometry contract.

## 19. Completion boundary

Claude is authorized to execute Implementation Brief Phases C through M.

Claude is NOT authorized to:

- modify live Git;
- stage;
- commit;
- merge;
- rebase;
- integrate to main;
- self-accept P0-T002.

At completion, status may be no stronger than:

IMPLEMENTED / CANDIDATE READY FOR OPERATOR IMPORT

Final verification, independent review, acceptance, closure and integration
remain separate controlled gates.

**P0-T002 PHASE C-M IMPLEMENTATION = AUTHORIZED**