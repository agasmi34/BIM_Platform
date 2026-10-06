# P1-T002 — Document Runtime & Dependency Recompute

Status: GOVERNANCE MATERIALIZED — IMPLEMENTATION NOT AUTHORIZED

## 1. Identity

- Task: P1-T002
- Title: Document Runtime & Dependency Recompute
- Branch: `task/P1-T002-document-runtime-dependency-recompute`
- Isolated worktree: `D:\Projects\BIM-Platform-WT-P1-T002`
- Baseline commit: `463a0e0f2625b041c9cf31efef90c47daab7eeb7`
- Baseline tree: `ebff88e4cf901bf3c9192bbb1230ca5264a6c983`

## 2. Governance lineage

P1-T002 starts only after P1-T001 was:

- independently reviewed and approved;
- committed;
- fast-forward integrated into `main`;
- validated on canonical `main`;
- pushed to `origin/main`;
- cleaned locally.

P1-T002 checkpoints:

- A1 — baseline/worktree bootstrap: PASS / CLOSED
- A2 — architecture + contract discovery: PASS / CLOSED
- A3 — architecture contract resolution: APPROVED / FROZEN / CLOSED
- A4 — implementation-footprint discovery: PASS / CLOSED

Harness finding `F-P1T002-A4-01` was a PowerShell automatic-variable collision only and is CLOSED.

## 3. Purpose

Introduce the first production Phase-1 `bim_document` runtime coordination boundary.

P1-T002 owns:

- live Level and StraightWall document state;
- runtime `ElementId <-> DependencyGraph::NodeId` association;
- Level -> StraightWall explicit semantic dependencies;
- dirty propagation;
- deterministic dependency recompute;
- derived neutral wall geometry specification;
- atomic in-memory document mutation.

## 4. Frozen identity law

`ElementId` remains durable BIM identity.

`DependencyGraph::NodeId` remains runtime-only graph identity.

They are never interchangeable.

The document layer owns a strict runtime one-to-one mapping:

`ElementId <-> NodeId`

Runtime NodeIds:

- start at `NodeId{1}`;
- never use zero;
- allocate monotonically;
- are never reused during one document lifetime;
- are not persisted;
- are not external identifiers;
- may be reconstructed after document reopen in a later task.

## 5. Frozen semantic dependency

For each StraightWall:

`Level -> StraightWall`

The graph edge provenance is:

`DependencyProvenance::ExplicitSemantic`

The graph remains model-neutral.

## 6. Allowed P1-T002 mutations

- Add Level
- Add StraightWall
- Update Level elevation
- Update StraightWall defining geometry

StraightWall re-hosting is not allowed.

Deletion is not part of P1-T002.

## 7. Derived geometry boundary

P1-T002 does not invoke OCCT.

It derives and caches project-owned neutral
`bim::geometry_api::LinearExtrusionSpec` values only.

For a StraightWall:

- `W = +Z`
- `U = normalize(end - start)`
- `V = W x U`
- profile origin is the start point shifted by `-V * thickness / 2`
- profile Z is `level.elevation + base_offset`
- `size_u = axis length`
- `size_v = thickness`
- extrusion direction is `+Z`
- extrusion distance is `height`

Any non-finite derived value or arithmetic overflow fails closed.

No hidden tolerance is introduced.

## 8. Atomicity

Every mutation is evaluated against staged runtime state.

The live document state changes only after all required validation, graph mutation, recompute planning, derived-geometry evaluation and graph recompute commit succeed.

On any failure:

- domain state is unchanged;
- mapping state is unchanged;
- dependency graph is unchanged;
- derived geometry is unchanged;
- NodeId allocator state is unchanged.

## 9. Explicit non-goals

P1-T002 does not implement:

- schema-v2 persistence;
- save/reopen;
- durable journal write;
- undo/redo;
- commands;
- query module implementation;
- desktop/UI;
- viewport/tessellation;
- raw OCCT geometry;
- IFC/DWG/RVT work;
- element deletion;
- wall re-hosting;
- computed dependency edges;
- parallel/background recompute.

## 10. Exact candidate implementation footprint

Only these implementation paths may be modified by the later implementation authorization:

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

No other production or test path is in candidate scope.

## 11. Expected test delta

Baseline full CTest count: 58.

P1-T002 adds exactly:

- `unit_document_runtime`
- `integration_p1_t002_document_recompute`
- `arch_document_public_runtime_neutral`
- `arch_p1_t002_document_public_fixture_rejected`

Expected full CTest registration after implementation: 62.

Expected architecture registration: 26.

## 12. Authorization state

Governance materialization does not authorize implementation.

Implementation authorization: NONE.

Commit authorization: NONE.

Push authorization: NONE.

ACR: NONE.