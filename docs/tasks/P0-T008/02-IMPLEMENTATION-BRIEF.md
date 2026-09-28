# BIM Platform - P0-T008 Implementation Brief

**Brief:** `P0-T008-IB v1.0`
**Task:** `P0-T008 - Topological Reference Spike`
**Status:** FROZEN / APPROVED
**Architecture Gate:** `BIM-AG-P0-T008 v1.0` - FROZEN / APPROVED
**Governance baseline HEAD:** `df9536d20495d97034d3aef5b4e81adb54bfa520`
**Governance baseline TREE:** `20ff89625e0cf2a02d3d7e0052a8c03f9faa3ad3`
**Architecture Gate blob:** `50f98bdbe59239aff52d82131c44d1d2e71a48e1`
**Task Record blob:** `d02e37f382df24d9cc8f6ad63b6e55f7eadd076a`
**Product Authority approval:** APPROVED
**Implementation Authorization:** NOT ISSUED
**Claude:** NOT AUTHORIZED
**Kimi:** NOT AUTHORIZED
**ACR:** NONE

## 1. Authority and lifecycle

This brief is subordinate to the approved `BIM-AG-P0-T008 v1.0` Architecture Gate and
the BIM Platform engineering constitution. It refines implementation mechanics only.
It MUST NOT weaken, reinterpret, or expand the approved architecture contract.

Product Authority has approved this brief. Approval of the brief does NOT authorize
implementation. A separate Implementation Authorization against an exact committed
brief baseline is required before Claude may modify source, CMake, tests, or dependency
manifests.

## 2. Constitutional anchors

The implementation MUST preserve the following constitutional rules:

- raw geometry-kernel face/edge/vertex identifiers never become persistent BIM references;
- stable model identity is project-owned and independent from OCCT topology identity;
- persistent sub-references use stable application identity plus semantic role and/or
  feature-generated stable sub-ID, never raw OCCT index;
- failed or ambiguous remapping is explicit; silent nearest-face rebinding is forbidden;
- the geometry boundary owns OCCT interaction while application BIM semantics remain above it;
- one authoritative task brief governs implementation;
- Claude implements only from an approved exact baseline and may not redefine locked architecture;
- Kimi reviews the exact implementation candidate independently after implementation evidence exists.

The narrow `SemanticOwnerToken` used by this spike is a project-owned proof token and MUST
remain compatible with the platform's locked stable-element identity direction. It MUST NOT
be presented as a replacement for the platform-wide immutable element GUID contract.

## 3. Objective

Implement the minimum proof that a project-owned semantic Face reference can be
re-resolved after controlled solid regeneration without using raw OCCT subshape identity,
traversal position, object address, kernel handle, face enumeration order, or
`input_face_ordinal` as durable identity.

The proof MUST fail closed when semantic continuity cannot be established.

## 4. Frozen scope

In scope:

- persistent semantic references for Faces only;
- six semantic extrusion Face roles;
- neutral geometry observations;
- deterministic semantic resolution;
- explicit Missing/Ambiguous/Invalid failure states;
- focused proof tests and architecture enforcement;
- minimum CMake wiring required by the proof.

Out of scope:

- persistent Edge or Vertex references;
- platform-wide GUID/identity redesign;
- project database persistence or schema migration;
- dependency graph propagation;
- production Wall/Door/Slab/Window/Room implementation;
- IFC/DWG/RVT semantic-reference integration;
- renderer or documentation-engine work;
- dependency upgrades or toolchain changes.

## 5. Required dependency direction

The implementation MUST preserve this logical dependency direction:

```text
bim_model -> bim_geometry_api -> bim_foundation
bim_geometry_occt -> bim_geometry_api -> bim_foundation
bim_geometry_occt -> OCCT
```

`bim_model -> OCCT` is prohibited.

OCCT headers, types, handles, and topology classes MUST NOT appear in public
`bim_model` or `bim_geometry_api` interfaces.

## 6. Project-owned persistent reference model

The spike MUST introduce the minimum project-owned concepts equivalent to:

```text
SemanticOwnerToken
TopologyKind::Face
ExtrusionFaceRole
PersistentFaceReference
```

The six required extrusion Face roles are:

```text
StartCap
EndCap
UMinSide
UMaxSide
VMinSide
VMaxSide
```

`PersistentFaceReference` MUST contain project-owned semantic information only:

```text
owner token
topology kind = Face
semantic extrusion Face role
```

It MUST NOT persist or encode:

```text
TopoDS_Shape / TopoDS_Face / TopoDS_Edge
OCCT object address or pointer value
OCCT handle identity
OCCT hash identity
TopExp traversal position
face enumeration order
input_face_ordinal
invocation-local transient face token
```

## 7. Neutral transient observation contract

`bim_geometry_api` may expose the minimum OCCT-free observation model needed by the resolver,
equivalent to:

```text
TransientFaceToken
SurfaceKind
PlaneObservation
FaceObservation
```

A Face observation may contain neutral values such as:

```text
invocation-local transient token
surface kind
plane normal and plane offset for planar faces
area or bounded diagnostic measurements when required
```

`TransientFaceToken` is valid only inside the current observation/resolution operation.
It MUST NOT be persisted, serialized, compared across regenerations, or treated as semantic
identity.

## 8. OCCT observation implementation

`bim_geometry_occt` translates current OCCT topology into the neutral observation contract.

It MAY use OCCT traversal internally to inspect the current solid, but traversal order is
non-authoritative. The semantic resolver MUST treat the neutral Face observation collection
as an unordered candidate set.

OCCT Generated/Modified/Deleted history MAY be captured for diagnostics or evidence only.
It MUST NOT be used as the durable identity contract.

## 9. Semantic extrusion frame

The resolver MUST derive expected semantic planes from the current project-owned extrusion
definition/frame for the owner being resolved.

The current owner definition MUST provide enough information to derive:

```text
StartCap
EndCap
UMinSide
UMaxSide
VMinSide
VMaxSide
```

Translation and dimension changes regenerate these expected semantic planes from current
semantic inputs. Previous kernel subshapes MUST NOT be required.

## 10. Deterministic Face resolution algorithm

For a `PersistentFaceReference`, resolution MUST:

1. validate the reference and topology kind;
2. validate the owner token against the current semantic owner;
3. derive the expected semantic plane for the requested Face role;
4. inspect the current neutral Face observation set;
5. select only planar candidates matching the expected semantic plane within the approved
   spike/local tolerance policy;
6. return `Resolved` only when exactly one candidate matches;
7. return `Missing` when zero candidates match;
8. return `Ambiguous` when more than one candidate matches;
9. return `InvalidReference` for malformed or unsupported reference semantics;
10. return `InvalidOwner` when the owner does not match.

Candidate ordering MUST NOT affect the result.

The resolver MUST NOT break ties using enumeration position, traversal order, area,
pointer identity, or any raw kernel identity.

## 11. Resolution result contract

The model layer MUST expose result semantics equivalent to:

```text
Resolved
Missing
Ambiguous
InvalidReference
InvalidOwner
```

A resolved result MAY carry an invocation-local neutral Face token for immediate downstream
use. That token MUST remain explicitly transient and MUST NOT become part of the persistent
reference.

## 12. Tolerance policy

The implementation MUST reuse an existing project geometry tolerance utility if a suitable
one exists at the authorized baseline.

If none exists, the spike MAY introduce a narrowly scoped named tolerance local to the
P0-T008 resolver/proof. It MUST NOT be presented as a new platform-wide tolerance contract.

Any need to change a locked global tolerance contract is a STOP condition for Architecture
Authority review.

## 13. Mandatory proof corpus

### A. Same-spec regeneration

Capture semantic references for all six Face roles, regenerate from the same semantic
specification, and resolve all six references successfully.

### B. Dimension change

Change one or more extrusion dimensions, regenerate, and prove that roles whose semantic
meaning remains valid resolve against the new geometry without old OCCT identity.

### C. Translation

Translate the semantic owner, regenerate, and prove that semantic references resolve against
the translated current semantic planes.

### D. Repeat regeneration

Repeat regeneration multiple times and prove deterministic resolution independent of kernel
object identity and candidate enumeration order.

### E. Boolean cut - unaffected Face continuity

Apply a controlled boolean cut that changes topology elsewhere while leaving a selected
semantic Face role represented by exactly one matching current Face. That reference MUST
resolve successfully.

### F. Split-Face ambiguity

Construct a controlled case where a semantic role is represented by more than one plausible
matching current Face. Resolution MUST return `Ambiguous`.

Choosing one candidate by traversal/enumeration order is a FAIL.

### G. Deleted Face

Construct a controlled case where the semantic Face no longer exists. Resolution MUST return
`Missing`.

### H. Invalid owner/reference

Prove explicit `InvalidOwner` and `InvalidReference` behavior.

## 14. Enumeration-order independence proof

At least one model-level test MUST feed the same neutral Face observations in different
orders and prove identical semantic resolution status.

This test MUST NOT require OCCT and is the direct proof that model resolution does not depend
on face enumeration order.

## 15. Architecture enforcement

Tests or build-time checks MUST prove:

- public `bim_model` headers expose no OCCT types;
- public `bim_geometry_api` headers expose no OCCT types;
- `bim_model` does not link directly to OCCT;
- persistent Face reference types expose no kernel handle, traversal index, or transient ordinal;
- semantic resolver ownership remains in `bim_model`;
- neutral OCCT extraction remains in `bim_geometry_occt`.

Existing repository architecture-test conventions MUST be reused where available.

## 16. Logical implementation footprint

Implementation is expected to touch only the minimum files needed in:

```text
bim_model
bim_geometry_api
bim_geometry_occt
P0-T008 focused tests
minimum CMake wiring required for those tests/targets
```

Exact physical filenames and directories MUST follow the repository conventions present at
the authorized baseline.

Claude MUST NOT reorganize modules, rename unrelated targets, rewrite unrelated public APIs,
or perform cleanup/refactoring outside this brief.

## 17. Toolchain and dependency constraints

The locked Windows toolchain remains unchanged:

```text
VS Build Tools 2022
VCTools 14.44.35207
cl 19.44.35228
link 14.44.35228
Windows SDK 10.0.26100.0
CMake 4.4.2
Ninja 1.12.1
C++20 platform baseline
```

Use Ninja only. Do not switch to a Visual Studio generator.

The frozen vcpkg baseline remains:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

No dependency version, manifest baseline, or package-source change is authorized.

## 18. Source and API restrictions

The implementation MUST NOT:

- expose any OCCT type in public model or geometry-api headers;
- persist traversal position or `input_face_ordinal`;
- derive identity from face enumeration order;
- use pointer/object address, OCCT hash, or kernel handle as identity;
- silently convert `Ambiguous` into `Resolved`;
- add Edge or Vertex persistent naming;
- implement dependency DAG behavior;
- add database/project-schema persistence;
- add production BIM element types;
- add IFC/DWG/RVT semantic-reference integration;
- alter ODA or other proprietary dependency handling.

## 19. Build and validation requirements

Once separately authorized, Claude MUST:

1. lock the exact authorized baseline before editing;
2. record pre-change branch/HEAD/TREE/clean state;
3. make only brief-scoped source/CMake/test changes;
4. build with the locked toolchain and Ninja;
5. run existing relevant tests;
6. run the complete P0-T008 proof corpus;
7. run architecture enforcement tests;
8. run `git diff --check`;
9. report the exact changed-path set;
10. report failures without automatically broadening or rewriting the design.

A build, harness, or test failure MUST be returned to Architecture Authority for classification
before any scope-expanding correction.

## 20. Evidence required from implementation

The implementation evidence packet MUST include:

```text
authorized baseline HEAD/TREE
final working HEAD/TREE before any candidate commit
exact dirty path set
build command and result
test command(s) and result(s)
proof result for cases A-H
enumeration-order independence result
architecture enforcement result
git diff --check result
dependency/toolchain status
ACR status
```

Claude self-review does not substitute for Architecture Authority validation.

## 21. STOP conditions

Claude MUST STOP and return evidence if:

- the authorized baseline does not match;
- the worktree is not clean before implementation;
- the Architecture Gate would need to change;
- dependency versions or baselines would need to change;
- direct `bim_model -> OCCT` dependency appears necessary;
- a public OCCT type appears necessary;
- continuity can only be achieved by kernel identity or enumeration order;
- the split-Face case cannot be represented without changing the frozen contract;
- a new platform-wide UUID, persistence, tolerance, or dependency-graph architecture is required;
- work expands into P0-T009, Phase 1 BIM features, or interoperability;
- unexpected pre-existing source changes are discovered.

Such a condition does not automatically authorize an ACR. Architecture Authority classifies it first.

## 22. Git and lifecycle restrictions

This brief does NOT authorize:

```text
source modification
CMake modification
test modification
git add
git commit
merge
rebase
push
main integration
Kimi review
```

Claude MUST NOT stage, commit, merge, rebase, push, or invoke Kimi during implementation.

Those actions become available only through explicit later lifecycle gates.

## 23. Acceptance criteria

P0-T008 implementation is acceptable only when:

1. project-owned persistent Face reference exists;
2. all six extrusion Face roles are represented;
3. no raw OCCT identity is stored in persistent references;
4. same-spec regeneration resolves correctly;
5. dimension-change regeneration resolves correctly where semantics remain valid;
6. translation resolves correctly;
7. repeat regeneration is deterministic;
8. unaffected Face continuity through the controlled boolean case is proven;
9. split-Face case returns `Ambiguous`;
10. deleted Face case returns `Missing`;
11. invalid owner/reference failures are explicit;
12. candidate ordering does not affect the result;
13. public model/geometry-api surfaces remain OCCT-free;
14. `bim_model` has no direct OCCT link;
15. P0-T009/Phase-1/interoperability/persistence boundaries remain intact;
16. locked toolchain/dependency baselines remain unchanged;
17. all required tests pass;
18. `git diff --check` passes;
19. ACR remains `NONE` unless Architecture Authority explicitly classifies otherwise;
20. no candidate commit or push occurs without a later explicit gate.

## 24. Rollback strategy

Until candidate acceptance, implementation rollback is ordinary worktree restoration to the
authorized committed brief baseline. No persistence/schema migration is in scope.

If an implementation attempt violates the contract, preserve evidence first, then Architecture
Authority classifies the failure before restore/rework instructions are issued.

## 25. Definition of done

Implementation is done only when:

- all acceptance criteria pass;
- required evidence is complete;
- scope and dependency boundaries are intact;
- Architecture Authority authorizes independent review;
- Kimi reviews the exact candidate and issues an acceptable verdict;
- blocking findings are remediated and revalidated;
- a candidate commit is separately authorized and created;
- closure/integration gates are completed.

## 26. Brief disposition

```text
Implementation Brief            = FROZEN / APPROVED
Product Authority approval      = APPROVED
Implementation Authorization    = NOT ISSUED
Claude                          = NOT AUTHORIZED
Kimi                            = NOT AUTHORIZED
Candidate commit                = NOT AUTHORIZED
Main integration                = NOT AUTHORIZED
Push                            = NOT AUTHORIZED
ACR                             = NONE
```

The next lifecycle step after controlled materialization and governance commit of this brief
is a separate Implementation Authorization against that exact committed brief baseline.
