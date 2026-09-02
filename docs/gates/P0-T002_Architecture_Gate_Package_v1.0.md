# BIM Platform — P0-T002 Architecture Gate Package

**Document ID:** BIM-AG-P0-T002
**Version:** 1.0
**Date:** 2026-09-02
**Task:** P0-T002 — OCCT Geometry Spike
**Parent authority:** BIM Platform — Master Engineering Constitution v0.2, P0-T001 accepted architecture records, ADR-0001 and ADR-0002
**Product Authority:** APPROVED
**Architecture Authority:** Product Owner + ChatGPT
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi
**Baseline:** `main@8c1c38990f75d5b0122e90d85bb8757e83a553a1`
**Gate status:** APPROVED
**Implementation status:** NOT RELEASED

## 1. Objective

P0-T002 establishes evidence that Open CASCADE Technology can serve as the
native geometry-kernel foundation for the BIM Platform without leaking raw
kernel topology into the BIM/domain architecture.

The task is a geometry spike, not a BIM feature implementation.

It must answer:

1. Can building-scale neutral solids be constructed and validated reliably?
2. Can representative opening cuts and wall-like joins be executed with
   deterministic, classified outcomes?
3. What precision/tolerance policy should the platform adopt?
4. What coordinate-magnitude policy is appropriate for local modeling versus
   survey/georeferenced coordinates?
5. What is the proper boundary between general OCCT operations and
   geometry-semantic fast paths?
6. What useful evidence can be obtained from OCCT operation history without
   treating kernel topology as persistent BIM identity?

## 2. Phase 0 sequencing authority

ADR-0002 reconciles historical Phase 0 task-numbering differences.

The operative Phase 0 sequence is:

| Task | Purpose |
|---|---|
| P0-T001 | Repository & Toolchain Scaffold |
| P0-T002 | OCCT Geometry Spike |
| P0-T003 | Desktop + Viewport Spike |
| P0-T004 | Persistence Spike |
| P0-T005 | IFC Spike |
| P0-T006 | DWG / ODA Evaluation |
| P0-T007 | RVT / BimRv Evaluation |
| P0-T008 | Topological Reference Spike |
| P0-T009 | Dependency Graph Spike |
| P0-T010 | Phase 1 Architecture Gate |

The dependency/license-baseline work formerly represented as a separate task
in an older planning sequence was materially absorbed by P0-T001.

This sequencing decision does not authorize dependency-version changes.

## 3. Existing baseline

P0-T001 established:

- Windows x64 first target.
- C++20.
- Visual Studio 2022 / MSVC v143.
- CMake 4.4.2.
- Ninja 1.12.1.
- vcpkg manifest mode.
- frozen vcpkg baseline
  `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843`.
- OCCT 8.0.1.
- Catch2 v3 + CTest for Phase 0.
- project-owned `bim_geometry_api`.
- OCCT implementation isolated in `bim_geometry_occt`.
- architecture checker preventing raw OCCT leakage.
- existing OCCT primitive smoke probe.
- five regression verification jobs:
  format, configure-build-test, static-analysis, architecture,
  license-inventory.

These remain the starting contracts for P0-T002.

## 4. In scope

P0-T002 may implement only what is necessary for the geometry spike:

- a narrow project-owned Phase-0 geometry contract;
- neutral point/vector/frame concepts as required;
- neutral linear/prismatic/extrusion construction specifications;
- project-owned opaque solid representation;
- OCCT-backed solid construction;
- Boolean cut;
- Boolean fuse/join;
- shape-validity checking;
- neutral geometric measurements required for evidence;
- structured project-owned geometry-error classification;
- tolerance experiments;
- coordinate-scale and coordinate-location experiments;
- deterministic geometry corpus;
- Boolean success and failure corpus;
- OCCT Generated / Modified / Deleted history experiments;
- operation repeatability evidence;
- diagnostic elapsed-time capture without performance SLA;
- stronger architecture enforcement where needed;
- P0-T002 evidence and traceability records.

## 5. Explicitly out of scope

P0-T002 MUST NOT implement:

- BIM Wall, Slab, Door, Window or Room entities;
- Level/Grid domain behavior;
- hosted-element semantics;
- UUID BIM identity architecture;
- persistent/topological reference implementation;
- topological naming algorithm;
- dependency-graph behavior;
- BIM model mutation logic;
- commands/query behavior;
- transactions or undo/redo features;
- native project schema;
- new persistence behavior;
- Qt;
- viewport or renderer;
- bgfx;
- IFC;
- DWG;
- RVT/RFA;
- Python runtime;
- AI integration;
- cloud/collaboration;
- geometry serialization;
- geometry-cache architecture;
- multi-threaded kernel architecture;
- product-level performance optimization.

If implementation requires any of these, Claude MUST stop and raise an ACR.

## 6. Geometry dependency boundary

The locked dependency direction is:

```text
BIM / application layers
        |
        v
bim_geometry_api
        |
        v
bim_geometry_occt
        |
        v
      OCCT

The following dependencies are prohibited:

model ----------> OCCT
commands -------> OCCT
query ----------> OCCT
transactions ---> OCCT
persistence ----> OCCT
desktop --------> OCCT

bim_geometry_occt remains the only project target permitted to own the
OCCT implementation dependency.

7. Public-header rule

No public project header may expose:

TopoDS_*;
BRep*;
gp_*;
Handle(...);
OCCT smart handles;
OCCT exceptions;
OCCT-specific error/status codes;
OCCT headers.

Public geometry APIs describe project-owned geometric intent rather than OCCT
mechanics.

8. Semantic boundary

The geometry layer does not know BIM semantic types.

Permitted test/fixture terminology includes:

wall-like prism;
slab-like prism;
opening-like cutting solid.

Prohibited domain implementation includes:

Wall;
Slab;
Door;
Window;
hosted BIM relationships.

BIM semantics belong above the geometry layer.

9. Fast-path principle

Future specialization, if accepted, must be based on geometry semantics rather
than BIM class names.

Potential geometry-semantic fast paths:

linear extrusion;
planar opening cut;
prismatic union.

BIM-specific kernel paths are prohibited.

The final P0-T002 disposition must select:

general OCCT operations only;
general OCCT plus geometry-semantic fast paths; or
architecture reconsideration required.
10. Precision and tolerance experiment

P0-T002 must produce evidence sufficient to choose:

default linear geometry tolerance;
default angular geometry tolerance;
allowed override policy;
minimum supported feature size;
accepted coordinate-magnitude envelope.

The implementation must keep distinct:

kernel numerical tolerance;
model geometric tolerance;
user/construction tolerance.

No platform-wide policy may be defined by scattering
Precision::Confusion() or equivalent kernel constants throughout source.

Candidate tolerances must be exercised systematically.

11. Coordinate experiment

Representative geometry must be evaluated:

near a local origin;
at a moderately translated building coordinate;
at a large survey-style offset.

The experiment must supply evidence for or against a future policy based on:

site/georeference transform
        +
local geometry working coordinates

Public BIM unit policy is not decided by this task.

12. Mandatory geometry corpus

At minimum the corpus includes:

Primitive/body cases
rectangular wall-like prism;
thin slab-like prism;
tall narrow prism;
very small but valid feature.
Opening cases
normal through-opening;
opening completely outside host;
opening partially intersecting host;
opening touching host boundary.
Join cases
collinear overlapping prisms;
orthogonal L join;
T join;
coplanar-face contact;
small-gap pair;
near-coincident pair.
Failure cases
zero thickness;
degenerate axis/direction;
invalid dimension;
deliberately problematic Boolean configuration.

Not every case is required to succeed.

Expected failures must instead be deterministic and project-classified.

13. Structured failure contract

The public geometry caller must be able to distinguish at least the conceptual
classes:

invalid input;
degenerate geometry;
no intersection;
kernel operation failed;
invalid result;
unsupported operation.

Exact enum/type naming is delegated to the Implementation Brief.

OCCT Standard_Failure must not cross the adapter boundary.

A generic catch (...) -> false contract is insufficient.

14. Result-validity policy

A successful OCCT call does not automatically establish a successful platform
geometry operation.

Boolean operations must validate, where meaningful:

algorithm completion;
non-null result;
expected dimensionality;
OCCT shape validity;
plausible neutral measurements.
15. OCCT history experiment

Selected cut/fuse operations must inspect OCCT operation history including:

Generated;
Modified;
Deleted.

The evidence must record input/output subshape mapping behavior and repeat-run
consistency.

OCCT history is evidence only.

It is explicitly NOT persistent BIM identity.

Raw OCCT face/edge identity, enumeration index or TopoDS_* identity must not
be promoted to a persistent reference contract.

Persistent-reference architecture remains P0-T008.

16. Repeatability

Core corpus operations must execute repeatedly.

Evidence must show deterministic:

operation classification;
success/failure outcome;
neutral measurement within accepted tolerance.

Topology counts may be captured diagnostically but are not persistent
contracts.

17. Performance evidence

Representative operation timing may be captured for a baseline.

P0-T002 has no millisecond acceptance SLA.

No production performance conclusion may be made from this spike alone.

18. Ownership/lifetime law

Public geometry objects must use project-owned ownership semantics.

Callers may not receive mutable OCCT objects and may not mutate kernel shapes
outside the adapter.

A deliberately narrow opaque/value abstraction is preferred over a premature
general CAD API.

A broad public ABI/API expansion requires an ACR.

19. CMake dependency law

The target graph remains:

bim_geometry_api
    -> bim_foundation

bim_geometry_occt
    -> bim_geometry_api
    -> bim_foundation
    -> OCCT

bim_geometry_api may not link OCCT.

20. Toolchain freeze

P0-T002 does not change:

Windows x64;
C++20;
VS 2022 / MSVC v143;
CMake 4.4.2;
Ninja 1.12.1;
vcpkg manifest mode;
OCCT 8.0.1;
Catch2 v3 + CTest;
frozen vcpkg baseline
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843.

A change requires Architecture Authority approval through ACR/ADR.

The authoritative Windows verification must run in the locked MSVC x64
developer environment.

21. Expected implementation footprint

Implementation is expected primarily within:

src/geometry/api/
src/geometry/occt/
tests/unit/
tests/integration/
tests/architecture/
tests/fixtures/
docs/tasks/P0-T002/
docs/evidence/P0-T002/
docs/project-control/
docs/architecture/adr/

Substantial changes to model, commands, query, transactions or persistence are
not expected and require review or ACR.

22. Existing smoke probe

RunKernelSmokeProbe() may be retained if still useful or removed/replaced if
the P0-T002 contract supersedes it.

Removal must be explicit and test-covered.

The P0-T001 evidence remains historical and is not rewritten.

23. Mandatory test capabilities

The P0-T002 test set must cover at least:

neutral geometry primitives;
OCCT primitive construction;
opening cut;
joins;
failure corpus;
tolerance matrix;
coordinate-scale/location matrix;
OCCT operation history;
repeatability;
architecture prohibition on OCCT leakage;
architecture ownership of kernel dependency.

Representative test names should follow the existing CTest naming law, for
example:

unit_geometry_<behavior>
integration_geometry_occt_primitive
integration_geometry_occt_opening_cut
integration_geometry_occt_join
integration_geometry_occt_failure_corpus
integration_geometry_occt_tolerance_matrix
integration_geometry_occt_history
integration_geometry_occt_repeatability
arch_geometry_api_no_occt_leak
arch_geometry_occt_only_kernel_owner

Exact executable/test decomposition belongs to the Implementation Brief.

24. Verification contract

Authoritative Windows verification includes:

format;
configure-build-test;
static-analysis;
architecture;
license-inventory;
P0-T002 geometry corpus evidence;
P0-T002 tolerance/coordinate evidence;
P0-T002 OCCT-history evidence.

The existing P0-T001 jobs are mandatory regression gates.

A regression in any accepted P0-T001 gate blocks P0-T002 acceptance.

25. Evidence contract

The implementation handover must capture:

Repository provenance
branch;
HEAD;
base SHA;
Git status;
worktree list;
changed-file list;
diff stat.
Toolchain provenance
effective MSVC compiler;
CMake version;
Ninja version;
vcpkg baseline;
resolved OCCT version.
Geometry corpus evidence

For each case where applicable:

case ID;
operation;
input dimensions;
coordinate location/scale;
tolerance;
expected classification;
actual classification;
result validity;
neutral measurement;
elapsed time;
PASS/FAIL.
History evidence
Generated mapping/count;
Modified mapping/count;
Deleted mapping/count;
repeat-run behavior.
Architecture evidence
zero OCCT leakage outside authorized adapter boundary.

No success may be fabricated from unexecuted tests.

26. Execution provenance

The P0-T001 execution model continues:

Claude authors implementation and verification machinery.
Windows Execution Operator performs authoritative Windows execution.
Raw execution evidence is authoritative.
Kimi independently reviews.
Architecture Authority accepts and closes.

Claude must distinguish authored expectations from operator-executed evidence.

27. Independent-review focus

Kimi must explicitly review:

OCCT leakage;
accidental BIM semantics in geometry;
hard-coded undocumented tolerance values;
raw topology identity leakage;
unsupported claims about operation history;
hidden Boolean failures;
happy-path-only testing;
overly broad geometry API;
dependency/version drift;
unrelated refactors;
verification provenance.

Zero unresolved BLOCKER or MAJOR findings are required for acceptance.

28. Acceptance criteria

P0-T002 is acceptable only if all criteria pass.

AC-001 Repository configures on the locked Windows/MSVC toolchain.

AC-002 Build succeeds with first-party warnings-as-errors.

AC-003 All pre-existing P0-T001 tests continue to pass.

AC-004 All required P0-T002 CTest tests pass.

AC-005 bim_geometry_api public surface exposes zero OCCT types or headers.

AC-006 Only the authorized OCCT adapter owns the OCCT implementation
dependency.

AC-007 Representative wall-like and slab-like neutral solids are
constructed and validated successfully.

AC-008 Normal through-opening Boolean cut succeeds with a valid,
plausibly-measured result.

AC-009 Required L/T/overlap join corpus has deterministic documented
outcomes.

AC-010 Failure corpus returns structured project-owned classifications.

AC-011 Tolerance matrix is executed and evidence captured.

AC-012 Coordinate-scale/location experiment is executed and evidence
captured.

AC-013 Architecture Authority can choose a documented precision/tolerance
policy from the evidence without relying on unexplained magic constants.

AC-014 OCCT Generated/Modified/Deleted behavior is measured for selected
cut/fuse operations.

AC-015 No OCCT topology/history identity is promoted to persistent BIM
identity.

AC-016 Repeatability evidence demonstrates deterministic classifications
within the accepted tolerance policy.

AC-017 Architecture negative fixtures continue to detect prohibited kernel
leakage.

AC-018 No prohibited scope expansion exists.

AC-019 Dependency versions and frozen baseline remain unchanged unless
separately approved.

AC-020 Format, static-analysis, architecture and license gates pass.

AC-021 Kimi returns zero unresolved BLOCKER or MAJOR findings.

AC-022 Task worktree is clean after implementation commit and main
remains untouched before controlled integration.

AC-023 Architecture Authority issues final written kernel-fitness,
precision/tolerance, coordinate and fast-path disposition and task closure.

29. Required final architecture decisions

Closure must explicitly decide:

D2-A — Kernel fitness

One of:

OCCT accepted;
OCCT accepted with constraints;
architecture reconsideration required.
D2-B — Precision/tolerance

Record actual accepted:

linear tolerance;
angular tolerance;
override policy;
minimum feature size;
coordinate-magnitude envelope.
D2-C — Coordinate policy

Record the accepted coordinate strategy, including whether geometry uses a
local working frame plus an external georeference transform.

D2-D — Fast-path policy

One of:

general kernel only;
general kernel plus geometry-semantic fast paths;
architecture reconsideration required.
30. Git isolation

Canonical task branch:

task/P0-T002-occt-geometry-spike

Canonical task worktree:

D:\Projects\BIM-Platform-WT-P0-T002

Approved base:

8c1c38990f75d5b0122e90d85bb8757e83a553a1

No P0-T002 implementation commit may be written directly to main.

31. Implementation release rule

Approval of this Architecture Gate does not itself release implementation.

Architecture Authority must next issue the P0-T002 Claude Implementation
Brief defining the precise public contracts, corpus values, test/evidence
schema and implementation steps.

Until that brief is committed/released:

PRODUCTION IMPLEMENTATION IS NOT AUTHORIZED.

32. Product Authority approval

Product Authority approved P0-T002 Architecture Gate v1.0 on 2026-09-02.

Architecture Gate status:

APPROVED

Implementation status:

NOT RELEASED