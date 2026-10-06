# P1-T002 — Implementation Authorization

Status: MATERIALIZED — PENDING A6-R1 BYTE LOCK

Architecture Authority Decision:
IMPLEMENTATION AUTHORIZED ONLY AFTER A6-R1 BYTE-LOCK PASS

Implementation Engineer: Claude

Task: P1-T002 — Document Runtime & Dependency Recompute

Branch:
`task/P1-T002-document-runtime-dependency-recompute`

Worktree:
`D:\Projects\BIM-Platform-WT-P1-T002`

Baseline commit:
`463a0e0f2625b041c9cf31efef90c47daab7eeb7`

Baseline tree:
`ebff88e4cf901bf3c9192bbb1230ca5264a6c983`

ACR: NONE

## 1. Effective-condition gate

This authorization record is not effective merely because this file exists.

Implementation authorization becomes effective only after the Architecture Authority completes P1-T002 A6-R1 and explicitly declares:

`P1-T002 A6 = PASS / CLOSED`

Until that declaration:

- Claude must not modify implementation files;
- no implementation staging is authorized;
- no implementation commit is authorized;
- no push is authorized.

## 2. Frozen governance identities

The following governance files are byte-frozen and must not be modified by Claude:

`docs/tasks/P1-T002/00-TASK-RECORD.md`

SHA256:
`4A4744A8568CD5AD62A471F31948A5BD4AC8F505976F4B5A1E859A96B0B473BF`

`docs/tasks/P1-T002/01-ARCHITECTURE-GATE.md`

SHA256:
`4B600BCF24946407BC35B72256B3917678E3990EDF53571DE12CD79860903D53`

`docs/tasks/P1-T002/02-IMPLEMENTATION-BRIEF.md`

SHA256:
`3EB00E44E33D641973B975450695612FA6A546265B4636FC338BED09639BB1A8`

This authorization file becomes byte-frozen separately at A6-R1.

## 3. Frozen candidate implementation manifest

After A6-R1 PASS, Claude is authorized to create or modify only these 11 candidate implementation paths:

1. `CMakeLists.txt`
2. `src/document/CMakeLists.txt`
3. `src/document/include/bim/document/document.hpp`
4. `src/document/src/document.cpp`
5. `tests/unit/CMakeLists.txt`
6. `tests/unit/unit_document_runtime.cpp`
7. `tests/integration/CMakeLists.txt`
8. `tests/integration/integration_p1_t002_document_recompute.cpp`
9. `tools/architecture_checker.cmake`
10. `tests/architecture/CMakeLists.txt`
11. `tests/fixtures/p1_t002_bad_document_public_runtime/document/include/bim/document/bad_document.hpp`

No other candidate path is authorized.

## 4. Frozen architecture contract

Claude must implement the already-approved P1-T002 architecture without reinterpretation.

The controlling documents are:

- `00-TASK-RECORD.md`
- `01-ARCHITECTURE-GATE.md`
- `02-IMPLEMENTATION-BRIEF.md`
- this authorization record

Core frozen laws include:

- `ElementId` is durable BIM identity.
- `DependencyGraph::NodeId` is runtime-only identity.
- `ElementId != NodeId`.
- document owns the runtime one-to-one ElementId/NodeId mapping.
- runtime NodeIds start at 1, never use zero and are not reused within one document lifetime.
- Level -> StraightWall is `ExplicitSemantic`.
- graph remains model-neutral.
- document runtime uses existing deterministic `BuildRecomputePlan()`.
- document runtime uses existing atomic `CommitRecompute()`.
- successful committed runtime ends clean.
- failure preserves strong live-state atomicity.
- no hidden tolerance.
- no wall re-hosting.
- no element deletion.
- no computed-edge production use.
- no parallel/background recompute.

## 5. Document target

Create:

- target `bim_document`
- alias `bim::document`

Required first-party links:

PUBLIC:
- `bim::model`
- `bim::geometry_api`

PRIVATE:
- `bim::dependency_graph`

Do not link:

- `bim::geometry_occt`
- `bim::transactions`
- `bim::persistence`
- SQLite
- Qt
- bgfx
- ODA
- IfcOpenShell

## 6. Geometry boundary

P1-T002 derives only project-owned neutral:

`bim::geometry_api::LinearExtrusionSpec`

Do not call:

`MakeLinearExtrusion()`

Do not invoke OCCT.

Every arithmetic-derived scalar and vector component must be finite.

Derived overflow/non-finite results fail closed as the document-owned derived-geometry failure result.

## 7. Runtime atomicity

Implementation must stage complete private runtime state before mutation.

A failed operation must preserve the previous live:

- Levels;
- StraightWalls;
- ElementId -> NodeId mapping;
- NodeId -> ElementId mapping;
- DependencyGraph state/revision;
- wall geometry cache;
- next runtime NodeId allocator value.

Do not mutate live state first and attempt rollback afterward.

## 8. Public-surface neutrality

Public document headers must not expose dependency-graph runtime identities or persistence/journal internals.

R18 selector:

`DOCUMENT_PUBLIC_RUNTIME_NEUTRAL`

must be added without altering R1-R17 semantics.

A dedicated negative fixture must prove the rule rejects a real code-level leak.

## 9. Exact P1-T002 CTest delta

Add exactly these four registrations:

- `unit_document_runtime`
- `integration_p1_t002_document_recompute`
- `arch_document_public_runtime_neutral`
- `arch_p1_t002_document_public_fixture_rejected`

Frozen registration oracle:

- baseline full CTest count = 58
- P1-T002 new tests = 4
- expected full CTest count = 62
- expected architecture count = 26
- focused P1-T002 count = 2

## 10. Regression obligations

The implementation must preserve:

- P0-T009 dependency-graph contracts;
- P1-T001 ElementId/domain contracts;
- P0-T008 face-reference contracts;
- geometry-api contracts;
- geometry-OCCT ownership;
- transaction/journal contracts;
- persistence/SQLite ownership;
- IFC/DWG ownership rules.

Required later validation includes:

- focused P1-T002 = 2/2
- P0-T009 regression = 2/2
- P1-T001 regression = 3/3
- architecture = 26/26
- full regression = 62/62

## 11. Toolchain lock

No dependency or toolchain change is authorized.

Do not modify:

- `vcpkg.json`
- `vcpkg-configuration.json`
- `CMakePresets.json`

No new third-party dependency is authorized.

C++20 remains mandatory.

## 12. STOP / ACR rule

Claude must STOP and report to Architecture Authority before making any change outside the frozen contract.

STOP is mandatory if implementation appears to require changing:

- P0-T009 graph semantics or public API;
- P1-T001 model contracts;
- geometry-api public contracts;
- transaction contracts;
- persistence schema;
- dependency/toolchain baseline;
- Level -> StraightWall semantic direction;
- document ownership;
- the 11-path candidate manifest.

Such a condition is not implementation discretion.

It requires Architecture Authority review and, if genuinely architectural, an ACR.

Current ACR status is:

`NONE`

## 13. Git authority boundary

Implementation authorization does not authorize Git publication actions.

Claude must not:

- stage files;
- commit;
- amend;
- merge;
- rebase;
- push;
- force push;
- create/delete remote branches;
- delete worktrees;
- delete task branches;
- alter canonical `main`.

Commit, integration, push and cleanup are separate later Architecture Authority gates.

## 14. Delivery requirement

After implementation, Claude must stop before staging or committing and provide:

- exact modified/untracked path list;
- `git status --short`;
- `git diff --stat`;
- relevant diff summary;
- any self-validation performed;
- any implementation concern;
- explicit ACR status.

No claim of acceptance is permitted.

Only Architecture Authority validation and independent review can accept the candidate.

## 15. Current authorization state

At materialization time:

Implementation authorization:
`PENDING A6-R1 BYTE LOCK`

Commit authorization:
`NONE`

Push authorization:
`NONE`

ACR:
`NONE`