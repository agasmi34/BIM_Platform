# P1-T002 — Implementation Brief

Task: Document Runtime & Dependency Recompute

Implementation Authorization: NOT YET GRANTED

## 1. Goal

Implement the first production `bim_document` runtime coordination layer while preserving all frozen P0-T009, P0-T010 and P1-T001 contracts.

## 2. Exact implementation footprint

Only the following 11 candidate paths are eligible for later implementation authorization:

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

Any other candidate modification requires Architecture Authority review before editing.

## 3. Root composition

Add `src/document` to root CMake in dependency-safe order after its required lower-level modules are available.

Do not reorder unrelated existing modules unless strictly required.

## 4. Document target

Create:

- `bim_document`
- alias `bim::document`

Required dependency surface:

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

## 5. Public header

Create:

`src/document/include/bim/document/document.hpp`

The public header may include project-owned model and geometry-api value contracts.

It must not include or name dependency-graph runtime types.

## 6. Runtime ownership

The implementation privately owns:

- Levels;
- StraightWalls;
- ElementId -> NodeId;
- NodeId -> ElementId;
- DependencyGraph;
- wall derived geometry cache;
- next NodeId.

A pimpl or equivalent private-state mechanism is permitted.

## 7. Exception containment

Public mutation methods must catch allocation, standard-library and unknown exceptions.

No exception crosses a mutation API boundary.

The result type itself must remain safe to construct during error reporting.

Avoid dynamic diagnostic strings if they undermine the failure-containment guarantee.

## 8. Validation

Before staging a successful mutation, use the existing P1-T001 model validation contracts.

Additional document-level checks include:

- cross-kind duplicate ElementId rejection;
- referenced Level existence;
- correct element kind for update;
- no StraightWall level re-host;
- valid runtime mapping invariant.

No hidden tolerance may be introduced.

## 9. Runtime mapping

For every committed document element:

- exactly one ElementId -> NodeId entry exists;
- exactly one reciprocal NodeId -> ElementId entry exists;
- graph contains the corresponding NodeId.

No public API exposes NodeId.

## 10. Add Level

Required behavior:

1. validate Level;
2. reject duplicate ElementId across both element kinds;
3. allocate staged NodeId;
4. register graph node;
5. insert Level and reciprocal runtime mapping;
6. commit staged runtime atomically.

A Level is initially Clean.

No geometry is generated for a Level.

## 11. Add StraightWall

Required behavior:

1. validate StraightWall;
2. reject duplicate ElementId;
3. require existing referenced Level;
4. allocate staged NodeId;
5. add graph node;
6. add Level -> StraightWall `ExplicitSemantic` dependency;
7. insert wall and reciprocal mapping;
8. mark wall Dirty;
9. build recompute plan;
10. derive neutral wall geometry;
11. commit graph recompute;
12. atomically publish staged runtime.

## 12. Update Level elevation

Required behavior:

1. require existing Level;
2. construct/validate the updated Level value;
3. update staged Level;
4. call `InvalidateDependents(level_node)`;
5. build deterministic recompute plan;
6. recompute affected StraightWalls in plan order;
7. commit recompute only when all evaluations succeed;
8. atomically publish staged runtime.

A Level with no downstream wall may legitimately produce no Dirty plan.

## 13. Update StraightWall

Required behavior:

1. require existing StraightWall;
2. reject wrong-kind ElementId;
3. validate candidate wall;
4. require unchanged `id`;
5. require unchanged `level_id`;
6. update staged wall;
7. call `MarkDirty(wall_node)`;
8. build recompute plan;
9. evaluate in plan order;
10. commit recompute;
11. atomically publish staged runtime.

## 14. Derived wall geometry

Build only `bim::geometry_api::LinearExtrusionSpec`.

Use:

- axis delta = end - start;
- finite positive axis length;
- normalized U axis;
- W = +Z;
- V = W x U;
- profile origin on SideA at base Z;
- profile size U = wall length;
- profile size V = thickness;
- extrusion direction W;
- extrusion distance = height.

All arithmetic-derived values must be checked for finiteness.

If subtraction, normalization, offset multiplication or elevation/base-offset addition overflows or becomes non-finite:

return `DerivedGeometryInvalid`.

Do not call `MakeLinearExtrusion()` in P1-T002.

Do not link geometry_occt.

## 15. Recompute evaluator

A Dirty runtime node expected by P1-T002 must map to a committed/staged StraightWall.

For each planned wall:

- derive its neutral geometry spec;
- stage the geometry cache replacement;
- append successful `NodeEvaluationOutcome`.

Unexpected runtime-state inconsistency fails closed as `RecomputeFailed`.

Only after every planned node succeeds may `CommitRecompute()` execute.

## 16. Strong atomicity

Perform all mutable work against a private staged runtime copy.

Do not mutate live containers first and then attempt rollback.

Only after the complete operation succeeds may the staged state replace live state.

A failure must preserve:

- prior domain values;
- prior runtime mappings;
- prior graph state/revision;
- prior geometry cache;
- prior NodeId allocator value.

## 17. Unit test

Add exactly:

`unit_document_runtime`

It links:

- `bim::document`
- `Catch2::Catch2WithMain`

It must not link dependency_graph directly.

Minimum coverage:

- empty lookup behavior;
- valid Level add/read;
- duplicate Level rejection;
- cross-kind duplicate ElementId rejection;
- missing-Level wall rejection with zero mutation;
- valid wall registration;
- exact derived extrusion-spec semantics;
- Level elevation update changes dependent wall base Z;
- wall geometry update changes only that wall;
- re-host attempt rejected with zero mutation;
- structurally invalid mutation rejected;
- derived-arithmetic overflow rejected with zero mutation;
- successful state remains readable after prior rejected operations.

## 18. Integration test

Add exactly:

`integration_p1_t002_document_recompute`

It links:

- `bim::document`
- `Catch2::Catch2WithMain`

It must exercise a multi-wall Level scenario:

- add one Level;
- add at least two StraightWalls hosted by it;
- verify initial neutral geometry;
- change Level elevation;
- verify all hosted walls recompute;
- change one wall geometry;
- verify the other wall remains unchanged;
- verify committed durable ElementIds remain stable;
- exercise repeated successful mutations;
- verify no partial state appears after an intentionally rejected mutation.

This test must use only the public document API.

## 19. Architecture rule R18

Extend `tools/architecture_checker.cmake` with:

`DOCUMENT_PUBLIC_RUNTIME_NEUTRAL`

Update the ALL rule documentation/list from R1-R17 to R1-R18.

Do not alter the semantics or token sets of R1-R17.

R18 scans `document/include/**` actual code while ignoring comments.

Forbidden runtime/layer tokens are the ones frozen in the Architecture Gate.

## 20. R18 negative fixture

Create only:

`tests/fixtures/p1_t002_bad_document_public_runtime/document/include/bim/document/bad_document.hpp`

It must contain a real code-level dependency-graph runtime leak sufficient for R18 to reject.

Do not create a compiled fixture project.

## 21. Architecture test registration

Add exactly:

- `arch_document_public_runtime_neutral`
- `arch_p1_t002_document_public_fixture_rejected`

The real-tree test must PASS.

The fixture test must invoke the same R18 selector and use:

`WILL_FAIL TRUE`

## 22. Expected CTest registration

Baseline: 58.

New P1-T002 tests: 4.

Expected total: 62.

Expected architecture count: 26.

Focused P1-T002 count: 2.

P0-T009 graph regression remains:

- `unit_dependency_graph`
- `integration_p0_t009_dependency_graph_spike`

P1-T001 focused regression remains:

- `unit_model_element_id`
- `unit_model_domain_validation`
- `unit_model_wall_face_reference`

## 23. Regression protection

P1-T002 must not modify or weaken:

- P0-T009 graph API/behavior;
- P1-T001 ElementId/domain contracts;
- P0-T008 face-reference contracts;
- geometry API contracts;
- geometry OCCT ownership;
- transaction/journal contracts;
- persistence/SQLite ownership;
- IFC/DWG ownership rules.

## 24. Toolchain

No dependency or toolchain baseline change.

No vcpkg manifest change.

No CMake preset change.

No new third-party dependency.

C++20 remains mandatory.

## 25. Acceptance sequence

Later implementation validation must prove:

1. exact authorized-path manifest;
2. governance hashes unchanged;
3. toolchain hashes unchanged;
4. configure PASS with `BIM_ENABLE_DWG=OFF`;
5. native build PASS;
6. full CTest registration = 62;
7. focused P1-T002 = 2/2 PASS;
8. P0-T009 graph regression = 2/2 PASS;
9. P1-T001 regression = 3/3 PASS;
10. architecture = 26/26 PASS;
11. full regression = 62/62 PASS;
12. candidate files stable after validation;
13. canonical `main` unchanged until integration authorization.

## 26. Authorization boundary

This Implementation Brief is not implementation authorization.

Claude may not edit implementation paths until a separate
`03-IMPLEMENTATION-AUTHORIZATION.md` is materialized and frozen by Architecture Authority.

No commit is authorized.

No push is authorized.

ACR remains NONE.