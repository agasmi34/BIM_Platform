# P1-T002 — Architecture Gate

Architecture Authority Status: APPROVED / FROZEN

Implementation Authorization: NONE

ACR: NONE

## 1. Module ownership

P1-T002 introduces:

- `src/document/**`
- target `bim_document`
- alias `bim::document`

The document module owns live document/runtime coordination.

It does not absorb model semantics, graph semantics, persistence implementation, CAD-kernel implementation or UI behavior.

## 2. Frozen module dependencies

`bim_document` may depend on:

- PUBLIC `bim::model`
- PUBLIC `bim::geometry_api`
- PRIVATE `bim::dependency_graph`

P1-T002 does not require `bim::transactions`, `bim::persistence` or `bim::geometry_occt`.

Forbidden reverse coupling remains unchanged:

- model -> document
- dependency_graph -> document
- dependency_graph -> model

No first-party cycle is permitted.

## 3. Durable identity versus runtime graph identity

`ElementId != DependencyGraph::NodeId`

`ElementId` is durable BIM identity.

`NodeId` is runtime evaluation identity.

The document owns both-direction runtime maps:

- ElementId -> NodeId
- NodeId -> ElementId

The mapping is strict one-to-one.

A Level and StraightWall cannot share one ElementId.

## 4. Runtime NodeId allocator

The allocator is document-owned.

Rules:

- first valid allocation is `NodeId{1}`;
- zero is never emitted;
- IDs increase monotonically;
- an allocated NodeId is not reused during that document lifetime;
- exhaustion fails before live mutation;
- allocator state is part of the staged atomic transaction;
- NodeIds are never persisted by this task.

## 5. Document state

The committed runtime state contains:

- Level map keyed by ElementId;
- StraightWall map keyed by ElementId;
- ElementId -> NodeId map;
- NodeId -> ElementId map;
- DependencyGraph;
- StraightWall ElementId -> LinearExtrusionSpec cache;
- next NodeId allocator state.

Model value types remain owned by `bim::model`.

## 6. Public document surface

The document public API exposes controlled runtime operations equivalent to:

- AddLevel(Level)
- AddStraightWall(StraightWall)
- UpdateLevelElevation(ElementId, elevation)
- UpdateStraightWall(StraightWall)
- FindLevel(ElementId)
- FindStraightWall(ElementId)
- FindWallGeometry(ElementId)

The exact C++ spelling may use conventional const/reference/optional forms, but may not widen semantics.

The public surface must not expose:

- NodeId;
- DependencyGraph;
- GraphResult;
- RecomputePlan;
- GraphSnapshot;
- persistence implementation;
- transaction journal implementation;
- third-party/vendor types.

## 7. Document result contract

The document API uses a project-owned, exception-contained result contract with machine-readable codes covering at least:

- Ok
- InvalidElement
- DuplicateElementId
- LevelNotFound
- ElementNotFound
- ElementKindMismatch
- RehostNotAllowed
- NodeIdExhausted
- GraphRejected
- DerivedGeometryInvalid
- RecomputeFailed
- InternalFailure

Exceptions must not escape mutation operations.

Diagnostics must not require parsing vendor/kernel messages.

## 8. Level -> StraightWall graph law

A StraightWall may be added only when its referenced Level exists in the same document.

Registration creates:

`Level Node -> StraightWall Node`

with:

`DependencyProvenance::ExplicitSemantic`

No computed edge is invented in P1-T002.

## 9. Dirty propagation

Adding a StraightWall:

- registers its runtime node;
- registers the semantic Level -> StraightWall edge;
- marks the wall Dirty;
- recomputes the wall before commit.

Updating Level elevation:

- updates staged Level state;
- invalidates strict downstream dependents;
- does not mark the Level itself Dirty;
- recomputes affected walls.

Updating StraightWall defining geometry:

- may not change `level_id`;
- marks that wall and its downstream closure Dirty;
- recomputes before commit.

Successful committed document state ends with no planned Dirty node left partially evaluated.

## 10. Deterministic recompute

The document must use the existing DependencyGraph contract.

It must call `BuildRecomputePlan()` and evaluate in plan order.

The graph remains single-threaded.

The existing NodeId ascending tie-break law is preserved.

No custom second scheduler is permitted.

## 11. Recompute commit

`CommitRecompute()` is called only after every planned node has a successful staged evaluation result.

If any evaluation fails:

- live graph is not partially cleaned;
- live model state is not partially committed;
- live derived geometry is not partially committed.

## 12. Neutral StraightWall geometry

P1-T002 derives a `LinearExtrusionSpec`.

For wall start S and end E:

`D = E - S`

`L = length(D)`

`U = D / L`

`W = (0,0,1)`

`V = W x U`

The rectangle profile is:

- origin = `S - V * thickness/2` at `base_z`;
- `u_axis = U`;
- `v_axis = V`;
- `size_u = L`;
- `size_v = thickness`.

Where:

`base_z = level.elevation + wall.base_offset`

Extrusion:

- direction = W
- distance = wall.height

Every derived scalar/vector component must remain finite.

No OCCT solid is produced by P1-T002.

## 13. Geometry tolerance

No hidden epsilon is permitted.

P1-T002 does not need a tolerance to derive the exact neutral extrusion specification.

If any later operation requires `GeometryTolerance`, it must receive an explicit caller/document value; no default may be invented.

## 14. Atomic staging model

Implementation shall stage the complete runtime state before mutation.

A valid implementation may copy the current private runtime state, execute the mutation against that staged copy, then atomically replace the live state only after success.

Allocation/copy failures are caught and reported as `InternalFailure`.

The live runtime must retain strong failure atomicity.

## 15. Query/read boundary

Read APIs return committed value data only.

Returned Level, StraightWall and LinearExtrusionSpec values must not permit callers to mutate internal containers.

No runtime graph identity is query-facing.

## 16. Architecture enforcement — R18

P1-T002 adds:

`DOCUMENT_PUBLIC_RUNTIME_NEUTRAL`

The rule scans `document/include/**` code while ignoring comments.

It rejects runtime/layer leakage including:

- `bim/dependency_graph/`
- `bim::dependency_graph`
- `NodeId`
- `GraphResult`
- `RecomputePlan`
- `GraphSnapshot`
- `bim/persistence/`
- `bim::persistence`
- `bim/transactions/`
- `bim::transactions`
- `JournalTransaction`
- `JournalRecord`

The rule is added to the existing ALL rule set without modifying R1-R17 semantics.

A dedicated negative fixture must prove R18 rejects an actual code-level leak.

## 17. Architecture CTest names

Exactly:

- `arch_document_public_runtime_neutral`
- `arch_p1_t002_document_public_fixture_rejected`

The fixture test uses `WILL_FAIL TRUE`.

## 18. Non-goals

No element deletion.

No wall re-hosting.

No computed-edge production use.

No persistence/schema-v2.

No journal persistence.

No command/query implementation.

No OCCT invocation.

No desktop/viewport implementation.

No parallel recompute.

## 19. Architecture conflict rule

Any implementation need to change:

- P0-T009 graph semantics;
- P1-T001 model contracts;
- geometry_api public contracts;
- transaction contracts;
- persistence schema;
- dependency/toolchain versions;
- Level -> StraightWall semantic direction;
- document ownership;

requires STOP and Architecture Authority review.

No such conflict currently exists.

ACR remains NONE.