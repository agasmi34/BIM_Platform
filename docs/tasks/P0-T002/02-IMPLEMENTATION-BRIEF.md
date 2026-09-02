# P0-T002 - Claude Implementation Brief

**Document ID:** BIM-TASK-P0-T002-CLAUDE

**Version:** 1.0

**Task:** P0-T002 - OCCT Geometry Spike

**Architecture Gate:** BIM-AG-P0-T002 v1.0

**Architecture commit:** `ee0f47ceb5d29f995635594cbb33ee3dd82f6e96`

**Approved main baseline:** `8c1c38990f75d5b0122e90d85bb8757e83a553a1`

**Task branch:** `task/P0-T002-occt-geometry-spike`

**Task worktree:** `D:\Projects\BIM-Platform-WT-P0-T002`

**Implementation Engineer:** Claude

**Independent Reviewer:** Kimi

**Status:** RELEASED FOR IMPLEMENTATION

## 1. Authority

This brief is subordinate to:

1. BIM Platform Master Engineering Constitution v0.2;
2. accepted P0-T001 architecture and closure records;
3. ADR-0001;
4. ADR-0002;
5. BIM-AG-P0-T002 v1.0.

If this brief conflicts with a locked architecture decision, Claude MUST stop
that portion of work and raise an Architecture Clarification Request.

Claude may not silently weaken, reinterpret or expand this brief.

## 2. Mission

Implement the minimum geometry-kernel spike necessary to obtain objective
evidence for:

- OCCT geometry-kernel fitness;
- project-owned geometry boundary;
- precision/tolerance behavior;
- coordinate-magnitude behavior;
- Boolean opening and join behavior;
- structured failure translation;
- OCCT Generated/Modified/Deleted history;
- repeatability;
- geometry-semantic fast-path decision.

This is NOT a BIM Wall/Slab/Door implementation.

## 3. Locked toolchain

Do not change:

- Windows x64;
- C++20;
- Visual Studio 2022 / MSVC v143;
- CMake 4.4.2 reference version;
- Ninja 1.12.1 reference version;
- vcpkg manifest mode;
- OCCT 8.0.1;
- Catch2 v3 + CTest;
- frozen vcpkg baseline
  `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843`.

No dependency upgrade is authorized.

No new production third-party dependency is authorized.

## 4. Existing regression baseline

The accepted P0-T001 tests and verification jobs are mandatory regressions.

Existing tests must remain present and passing, including:

- `unit_foundation_smoke`;
- `unit_model_links_foundation`;
- `integration_geometry_occt_probe`;
- `integration_persistence_sqlite_memory`;
- `arch_repository_boundaries`;
- `arch_checker_detects_violation`.

Do not remove the existing kernel smoke probe during P0-T002.

It remains a P0-T001 regression test.

## 5. Geometry dependency law

The only allowed production dependency direction is:

    bim_geometry_api
        -> bim_foundation

    bim_geometry_occt
        -> bim_geometry_api
        -> bim_foundation
        -> OCCT

No other project module may acquire an OCCT dependency.

No raw OCCT type/header may appear in public project APIs.

## 6. Public geometry contract

Create:

`src/geometry/api/include/bim/geometry_api/geometry.hpp`

The header must remain OCCT-free.

The following names and conceptual shapes are locked for P0-T002.

### 6.1 GeometryErrorCode

Use:

```cpp
enum class GeometryErrorCode {
    None,
    InvalidInput,
    DegenerateGeometry,
    NoIntersection,
    KernelOperationFailed,
    InvalidResult,
    UnsupportedOperation
};

No OCCT error code may appear here.

6.2 GeometryError

Use a project-owned error record concept containing:

GeometryErrorCode code;
human-readable std::string message.

Messages must not be used as machine-readable classification.

6.3 Basic neutral types

Provide:

struct Point3 {
    double x;
    double y;
    double z;
};

struct Vector3 {
    double x;
    double y;
    double z;
};

All values must be checked for finiteness before kernel invocation.

6.4 GeometryTolerance

Provide:

struct GeometryTolerance {
    double linear;
    double angular_radians;
};

Do NOT supply a platform-wide default value in this task.

P0-T002 is collecting evidence needed to choose that future policy.

6.5 RectangleProfile3

Provide a project-owned rectangular profile:

struct RectangleProfile3 {
    Point3 origin;
    Vector3 u_axis;
    Vector3 v_axis;
    double size_u;
    double size_v;
};

Rules:

u_axis and v_axis must be finite;
both must be non-degenerate;
they must not be parallel within supplied angular tolerance;
dimensions must be finite and positive.

Claude may normalize axes internally.

No OCCT gp_* type may enter this contract.

6.6 LinearExtrusionSpec

Provide:

struct LinearExtrusionSpec {
    RectangleProfile3 profile;
    Vector3 direction;
    double distance;
};

Rules:

direction finite;
direction non-degenerate;
distance finite;
distance strictly positive.
6.7 Opaque solid ownership

Public API must contain an incomplete project-owned Solid concept and an
immutable shared handle.

Required conceptual form:

struct Solid;
using SolidHandle = std::shared_ptr<const Solid>;

The public header must not define the kernel payload.

The complete implementation may exist only inside the OCCT adapter.

Callers must not obtain mutable TopoDS_Shape.

6.8 SolidMetrics

Provide neutral diagnostic metrics containing:

validity flag;
volume;
bounding-box minimum;
bounding-box maximum;
number of solids;
number of faces;
number of edges.

Counts are diagnostic only.

They are NOT identity contracts.

6.9 Operation result

Provide project-owned result types sufficient to return:

SolidHandle on successful solid-producing operations;
GeometryError;
neutral metrics through inspection.

Do not introduce exceptions as the normal public error mechanism.

Third-party exceptions must be translated inside the adapter.

6.10 Required public operations

Provide the conceptual operations:

MakeLinearExtrusion(...)
Cut(...)
Fuse(...)
Inspect(...)

Exact reference/const syntax may follow normal C++20 style, but names and
semantics above are locked.

If Claude believes a materially different public contract is required, raise
an ACR before implementing it.

7. Public operation semantics
7.1 MakeLinearExtrusion

Must:

validate project-owned inputs before OCCT use;
construct the rectangular profile;
extrude it;
reject null/invalid output;
translate OCCT failures;
return an immutable project-owned handle.
7.2 Cut

Cut(host, tool, tolerance) means a volumetric Boolean subtraction.

Contract:

null handle -> InvalidInput;
volumetrically disjoint input -> NoIntersection;
boundary-only contact with zero volumetric overlap -> NoIntersection;
kernel execution failure -> KernelOperationFailed;
completed but invalid output -> InvalidResult;
valid subtraction -> None.

A NoIntersection result must not be disguised as kernel failure.

7.3 Fuse

For P0-T002, Fuse(a, b, tolerance) means a connected solid union.

Contract:

null handle -> InvalidInput;
clearly separated solids outside supplied tolerance -> NoIntersection;
kernel execution failure -> KernelOperationFailed;
result failing validity/expected dimensionality -> InvalidResult;
valid connected union -> None.

Do not silently accept a disconnected compound as successful connected union.

7.4 Inspect

Must report neutral geometry metrics without exposing OCCT objects.

8. Input-error classification

Use these rules:

InvalidInput

Use for:

NaN/infinite numeric input;
zero or negative physical dimensions;
zero or negative extrusion distance;
null solid handles.
DegenerateGeometry

Use for geometrically unusable finite definitions such as:

vector magnitude at or below supplied linear tolerance;
profile axes parallel within supplied angular tolerance;
geometry collapsing within the supplied tolerance.
NoIntersection

Use when a Boolean requiring volumetric/contact interaction has no qualifying
interaction under the operation contract.

KernelOperationFailed

Use only after validated input reaches OCCT and the kernel operation fails to
complete successfully.

InvalidResult

Use when OCCT returns a result but platform validation rejects it.

UnsupportedOperation

Reserve for an operation deliberately outside the implemented P0-T002
geometry capability.

Do not fabricate this status merely to satisfy a test.

9. Kernel-result validation

For a successful solid-producing operation, validate at least:

result is non-null;
expected three-dimensional solid content exists;
shape-validity analysis passes;
computed volume is finite and positive where appropriate;
metrics are internally plausible.

Use actual OCCT 8.0.1 APIs available in the installed headers.

Do not invent OCCT method names from memory.

If an assumed API is unavailable, inspect the installed OCCT 8.0.1 headers and
adapt without changing the public architecture.

If satisfying the required semantics would require an architecture change,
raise an ACR.

10. Spike test unit

For P0-T002 corpus values only:

1.0 test length unit = 1 metre-equivalent.

This is NOT a final product unit-storage decision.

Do not record it as the future BIM unit policy.

11. Reference tolerance

Use this value only as the reference probe setting:

linear          = 1.0e-6
angular_radians = 1.0e-8

It is NOT yet the accepted platform default.

The final default is an Architecture Authority closure decision after evidence.

12. Tolerance matrix

Mandatory linear candidates:

1.0e-7
1.0e-6
1.0e-5
1.0e-4

Mandatory angular candidates in radians:

1.0e-10
1.0e-8
1.0e-6

Experiments:

Run the core local-origin cases across every linear candidate using
angular 1.0e-8.
Run direction/profile validation cases across every angular candidate
using linear 1.0e-6.
Do not silently choose a preferred tolerance from these results.
Record the evidence for Architecture Authority disposition.
13. Coordinate matrix

Mandatory coordinate offsets:

C0 - local
(0, 0, 0)
C1 - moderately translated
(1000, 1000, 0)
C2 - large survey-style offset
(1000000, 1000000, 0)

Run representative primitive, cut and fuse cases at all three offsets using
the reference probe tolerance.

The purpose is to measure numerical behavior.

It does not authorize direct storage of all future BIM geometry at survey
coordinates.

14. Mandatory primitive corpus
P01 - wall-like prism

Profile:

origin = (0, 0, 0)
u      = (1, 0, 0)
v      = (0, 0, 1)
size_u = 6.0
size_v = 3.0

Extrusion:

direction = (0, 1, 0)
distance  = 0.2

Expected volume:

3.6

Expected classification:

None

P02 - slab-like prism

Profile:

origin = (0, 0, 0)
u      = (1, 0, 0)
v      = (0, 1, 0)
size_u = 6.0
size_v = 4.0

Extrusion:

direction = (0, 0, 1)
distance  = 0.2

Expected volume:

4.8

Expected classification:

None

P03 - tall narrow prism
profile size = 0.15 x 0.15
extrusion    = 12.0

Expected volume:

0.27

Expected classification:

None

P04 - small valid feature
profile size = 0.01 x 0.01
extrusion    = 0.01

Expected volume:

0.000001

Expected classification:

None

15. Mandatory opening corpus

Use P01 as host.

The cutting solid uses:

u = (1, 0, 0)
v = (0, 0, 1)
direction = (0, 1, 0)
O01 - normal through opening
origin   = (2.0, -0.05, 0.5)
size_u   = 1.0
size_v   = 2.0
distance = 0.30

Expected:

classification = None
result volume  = 3.2
O02 - completely outside host
origin   = (7.0, -0.05, 0.5)
size_u   = 1.0
size_v   = 2.0
distance = 0.30

Expected:

NoIntersection

O03 - partial host intersection
origin   = (5.5, -0.05, 0.5)
size_u   = 1.0
size_v   = 2.0
distance = 0.30

Expected:

classification = None
result volume  = 3.4
O04 - boundary-only contact
origin   = (6.0, -0.05, 0.5)
size_u   = 1.0
size_v   = 2.0
distance = 0.30

Expected:

NoIntersection

The platform contract treats zero-volume boundary-only contact as no
volumetric cutting intersection.

16. Mandatory join corpus

All join fixtures use height 3.0 unless stated otherwise.

J01 - collinear overlap

A:

x = 0.0 .. 4.0
y = 0.0 .. 0.2
z = 0.0 .. 3.0

B:

x = 3.0 .. 7.0
y = 0.0 .. 0.2
z = 0.0 .. 3.0

Expected:

classification = None
volume         = 4.2
J02 - orthogonal L join

A:

x = 0.0 .. 4.0
y = 0.0 .. 0.2
z = 0.0 .. 3.0

B:

x = 0.0 .. 0.2
y = 0.0 .. 4.0
z = 0.0 .. 3.0

Expected:

classification = None
volume         = 4.68
J03 - T join

A:

x = 0.0 .. 4.0
y = 0.0 .. 0.2
z = 0.0 .. 3.0

B:

x = 1.9 .. 2.1
y = 0.0 .. 2.0
z = 0.0 .. 3.0

Expected:

classification = None
volume         = 3.48
J04 - coplanar-face contact

A:

x = 0.0 .. 2.0

B:

x = 2.0 .. 4.0

Both use:

y = 0.0 .. 0.2
z = 0.0 .. 3.0

Expected:

classification = None
volume         = 2.4

The result must be validated as the connected-union contract requires.

J05 - small-gap pair

A ends at:

x = 2.0

B begins at:

x = 2.001

Gap:

0.001

Expected classification for the mandatory tolerance matrix:

NoIntersection

J06 - near-coincident overlap stress case

A ends at:

x = 2.0

B begins at:

x = 1.99995

Nominal overlap:

0.00005

This is an observational stress case.

Do NOT predeclare its kernel outcome across every tolerance.

Requirements:

no crash;
no leaked OCCT exception;
project-owned classification;
same classification across repeated identical runs for a given matrix cell;
result validity measured when successful.
17. Mandatory failure corpus
F01 - zero extrusion distance

Expected:

InvalidInput

F02 - zero extrusion direction
direction = (0, 0, 0)

Expected:

DegenerateGeometry

F03 - negative profile dimension

Example:

size_u = -1.0

Expected:

InvalidInput

F04 - numerically stressed Boolean

Construct a thin/near-coincident Boolean fixture using the same project-owned
API and exercise it at:

local coordinates;
C2 large offset;
every mandatory linear tolerance.

The exact resulting project-owned status is observational.

Requirements:

no process failure;
no exception crossing the adapter;
deterministic status per identical matrix cell;
valid result if status is None.

Do not create a deliberately corrupt OCCT object merely to force
KernelOperationFailed.

18. Repeatability contract

Reference repeat count:

20

Run at least these cases 20 times from freshly constructed inputs:

P01;
O01;
O02;
J01;
J04;
J05;
J06;
F02;
F04.

For each identical case/matrix cell, classification must be identical across
all repetitions.

Successful volume results must agree within the supplied test tolerance.

Do not treat raw face/edge ordering as a durable contract.

19. OCCT operation-history experiment

Add a P0-only adapter diagnostic boundary:

src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp

This header:

may be consumed only by P0-T002 tests/evidence tooling;
must expose only project-owned neutral diagnostics;
must expose no TopoDS_*, BRep*, gp_*, Handle(...) or other OCCT type.

Measure history for at least:

O01 Cut;
J01 Fuse;
J02 Fuse.

For deterministic pre-operation face traversal, record diagnostic tuples
conceptually equivalent to:

input side
input face ordinal
Generated count
Modified count
Deleted flag

The ordinal is a temporary observation label only.

It is NOT persistent identity.

Rebuild inputs and repeat each history case at least:

10 times

Evidence must state whether the normalized tuple set is identical across runs.

P0-T002 must not introduce:

PersistentFaceId;
PersistentEdgeId;
stable topology ID;
raw OCCT identity above the adapter.

Persistent reference design remains P0-T008.

20. Geometry evidence executable

Add a dedicated evidence executable under tests/integration/.

Recommended target name:

p0_t002_geometry_evidence

It must:

use the same production geometry API/adapter code exercised by tests;
execute the mandatory corpus and matrices;
return non-zero if a mandatory expected assertion fails;
support machine-readable JSON output;
require no new JSON dependency;
use standard-library output if necessary;
record elapsed time as diagnostic evidence only.

Suggested invocation:

p0_t002_geometry_evidence --json <output-path>

The JSON must contain at minimum:

task
case_id
category
operation
origin_offset
linear_tolerance
angular_tolerance
expected_classification
actual_classification
valid
volume
solid_count
face_count
edge_count
elapsed_microseconds
repeat_iteration

History records additionally contain:

input_side
input_face_ordinal
generated_count
modified_count
deleted
history_repeat

Timing is not pass/fail unless the process fails or produces invalid data.

21. Geometry verification script

Add:

scripts/ci/geometry-spike.ps1

Responsibilities:

do not mutate source;
locate the configured evidence executable;
execute it;
write evidence under the build tree, not source tree;
fail on non-zero executable result;
verify JSON output exists and is non-empty;
print the evidence output path;
print a concise corpus summary.

Recommended evidence location:

build/ci-win-msvc/evidence/P0-T002/

Do not commit generated build-tree evidence.

The authoritative operator transcript and selected machine-readable evidence
will be preserved later under docs/evidence/P0-T002/.

22. Authoritative verification runbook

Author a new root runbook:

Verification-RunbookC-v1.0.ps1

Do not modify historical P0-T001 runbooks merely to make P0-T002 work.

Runbook C must verify:

exact task branch;
expected pre-verification task HEAD supplied by Architecture Authority;
clean worktree;
main remains unchanged and clean;
Visual Studio 2022 developer environment is active;
target architecture is x64;
effective compiler is MSVC;
compiler path corresponds to validated cl.exe;
CMake reference version;
Ninja presence/version;
vcpkg baseline unchanged;
OCCT resolves to 8.0.1;
format job;
configure/build/CTest job;
static-analysis job;
architecture job;
license-inventory job;
geometry-spike evidence job;
all P0-T001 regression tests;
all P0-T002 tests;
final repository invariants.

Runbook C MUST use real exit codes.

It MUST NOT claim PASS for commands that were not executed.

23. Mandatory tests

Add/register tests covering at minimum:

unit_geometry_api_contract
integration_geometry_occt_primitive
integration_geometry_occt_opening_cut
integration_geometry_occt_join
integration_geometry_occt_failure_corpus
integration_geometry_occt_tolerance_matrix
integration_geometry_occt_coordinate_matrix
integration_geometry_occt_history
integration_geometry_occt_repeatability
integration_geometry_occt_evidence
arch_geometry_api_no_occt_leak
arch_geometry_occt_only_kernel_owner

Exact source-file grouping may vary only if every required capability remains
independently visible in CTest/evidence.

Do not replace or remove the six accepted P0-T001 CTest entries.

24. Architecture checker expansion

Strengthen the existing architecture checks so that:

src/geometry/api/** cannot include/link OCCT;
model/commands/query/transactions/persistence cannot include OCCT;
only the adapter boundary owns kernel implementation;
the negative fixture remains proven to fail;
the new P0-T002 diagnostic header leaks no OCCT types.

Do not weaken the existing negative test merely to accommodate new code.

25. No BIM semantics in geometry

Do not create production classes/types named:

Wall
Slab
Door
Window
Room
HostedElement
Level
Grid

Test comments/case descriptions may say wall-like or slab-like.

The geometry API must remain domain-neutral.

26. Allowed implementation footprint

Primary implementation may modify/add files only under:

src/geometry/api/
src/geometry/occt/
tests/unit/
tests/integration/
tests/architecture/
tests/fixtures/
scripts/ci/
docs/tasks/P0-T002/
docs/evidence/P0-T002/
docs/project-control/

Additionally authorized at repository root:

Verification-RunbookC-v1.0.ps1

Authorized CMake composition files as actually required:

CMakeLists.txt
tests/CMakeLists.txt

Do not modify:

src/model/
src/dependency_graph/
src/transactions/
src/commands/
src/query/
src/persistence/
src/desktop/
src/documentation/
src/interop/
vcpkg.json
vcpkg-configuration.json

unless Architecture Authority first approves an ACR.

If another file is materially required, stop and raise ACR.

27. CMake rules

Do not change the global C++ standard.

Do not change dependency versions.

Do not add FetchContent.

Do not add a submodule.

Do not vendor a geometry library.

Third-party OCCT compile/link details remain isolated to
bim_geometry_occt.

Warnings-as-errors remain applicable to first-party implementation.

28. Source-quality requirements

Production and test code must:

compile under C++20;
be formatted by repository .clang-format;
pass existing .clang-tidy;
use no unexplained warning suppression;
contain no source absolute paths;
contain no generated evidence;
contain no secrets;
avoid global mutable geometry state;
avoid static registries for opaque solid handles.
29. Error-handling requirements

At the OCCT boundary:

catch Standard_Failure;
translate it to project-owned failure;
catch ordinary standard exceptions where appropriate;
use unknown catch only as final containment;
never expose an OCCT exception to the caller.

Error strings are diagnostic only.

Classification enum is authoritative.

30. Numerical evidence rules

Use finite-value checks.

Do not compare floating values with raw equality where tolerance is required.

Expected analytical volumes may use tolerance-aware assertions.

The evidence tool must preserve the exact tolerance used for every observation.

Do not silently enlarge a caller-supplied tolerance to make a test pass.

Kernel-internal tolerances may be used internally only when documented and
must not be represented as the platform's final tolerance policy.

31. Timing rules

Elapsed times:

use a monotonic clock;
record microseconds;
are informational;
have no pass threshold in P0-T002.

Do not optimize implementation solely to improve this spike's timing.

32. Implementation phases

Claude must execute in this order.

Phase A - preflight

Inspect:

branch;
HEAD;
clean status;
worktree list;
Gate;
ADR-0002;
this brief;
current geometry code;
current architecture checker;
current test/CMake layout;
current CI scripts.

Report any conflict before editing.

Phase B - contract design check

Before writing substantial implementation, show the intended concrete file
list and public geometry declarations.

They must conform exactly to this brief.

If a public-contract deviation is required, STOP + ACR.

Phase C - neutral API

Implement project-owned geometry types/contracts with zero OCCT leakage.

Phase D - OCCT adapter

Implement opaque solid storage and OCCT-backed operations.

Phase E - validity/error translation

Implement input validation, kernel failure translation and result inspection.

Phase F - corpus tests

Implement primitive/opening/join/failure cases.

Phase G - numerical experiments

Implement tolerance and coordinate matrices.

Phase H - history diagnostics

Implement P0-only Generated/Modified/Deleted experiment.

Phase I - repeatability

Implement required repeat loops and deterministic classification checks.

Phase J - evidence executable/script

Implement machine-readable corpus evidence and geometry-spike.ps1.

Phase K - architecture enforcement

Strengthen boundary checks and negative fixtures.

Phase L - Runbook C

Author authoritative Windows verification runbook.

Phase M - local/static self-review

Before handover, inspect:

scope;
diff;
public headers;
OCCT leakage;
dependencies;
test registration;
evidence generation;
docs.

No commit is authorized merely by completing Phase M.

33. Stop conditions

Claude MUST stop and raise ACR if any of these occurs:

OCCT 8.0.1 cannot satisfy required semantics without public kernel leakage;
a new third-party production dependency appears necessary;
vcpkg baseline/version change appears necessary;
geometry API must gain BIM semantics;
model/transactions/commands/query/persistence must be changed;
a broad general-CAD API appears necessary;
persistent topology identity is needed;
a required expected corpus outcome proves architecturally invalid rather
than being a simple implementation defect;
the locked Windows toolchain cannot execute;
the test/evidence contract cannot be met without architecture expansion.

Do not work around a stop condition silently.

34. Git law

Work only in:

D:\Projects\BIM-Platform-WT-P0-T002

Branch:

task/P0-T002-occt-geometry-spike

Do not modify main.

Do not merge.

Do not rebase onto another base.

Do not delete or rewrite P0-T001 evidence.

Do not commit implementation until Architecture Authority explicitly authorizes
the implementation commit after verification/review staging policy is defined.

35. Verification provenance

Claude may author and statically inspect code in its own environment.

Only actual Windows Execution Operator output counts as authoritative
Windows/MSVC runtime evidence.

Handover must label evidence accurately as:

Claude-authored;
Claude-observed;
Operator-executed;
Architecture-Authority-attested;
Reviewer-observed.

Do not collapse those categories.

36. Handover requirements

Before independent review, Claude must produce:

docs/evidence/P0-T002/CLAUDE_HANDOVER.md

and:

docs/evidence/P0-T002/CLAUDE_HANDOVER.json

The handover must include:

architecture commit;
implementation candidate identity;
changed paths;
exact scope;
public API summary;
corpus summary;
tolerance matrix summary;
coordinate matrix summary;
history summary;
repeatability summary;
toolchain versions;
verification commands;
exit statuses;
evidence paths/hashes;
deviations;
unresolved questions;
Git state;
explicit statement that main remains untouched.

If authoritative Windows verification has not yet been executed, do not mark
the corresponding acceptance criteria PASS.

37. Acceptance mapping

Claude must map evidence explicitly to:

AC-001 through AC-023

from BIM-AG-P0-T002 v1.0.

Claude does not grant final PASS for:

AC-021, which is Kimi-owned;
AC-023, which is Architecture-Authority-owned.

Git cleanliness/integration-sensitive criteria remain subject to Architecture
Authority verification.

38. Independent review

Kimi will independently review after authoritative evidence exists.

Claude must not modify the candidate during Kimi review unless Architecture
Authority sends a specific correction request.

Any review correction must be minimum-delta.

Do not rebuild or rewrite the whole implementation when a localized correction
suffices.

39. Final architecture decisions

P0-T002 implementation must provide evidence, but must NOT self-decide:

D2-A kernel fitness;
D2-B final precision/tolerance policy;
D2-C coordinate policy;
D2-D fast-path policy.

Those decisions belong to Architecture Authority at closure.

40. Release disposition

This brief releases P0-T002 for implementation under its locked scope.

Claude is authorized to begin implementation only after this released brief is
committed on the P0-T002 task branch.

P0-T002 IMPLEMENTATION = RELEASED FOR IMPLEMENTATION