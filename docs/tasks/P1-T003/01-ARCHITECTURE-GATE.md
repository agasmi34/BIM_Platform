# P1-T003 Architecture Gate

ID: BIM-AG-P1-T003
Status: APPROVED / FROZEN
ACR: NONE

## 1. Purpose

P1-T003 establishes the production command mutation boundary and read-only
query vertical slice over the P1-T002 Document runtime.

## 2. Dependency law

Allowed:

- bim_commands -> bim_document
- bim_query -> bim_document
- bim_query -> bim_model

Forbidden for bim_commands:

- bim_transactions
- bim_persistence
- bim_dependency_graph
- bim_geometry_occt
- SQLite
- Qt
- bgfx
- ODA Drawings
- IfcOpenShell

Forbidden for bim_query:

- mutation of Document;
- dependency-graph access;
- transaction/journal mutation;
- persistence/SQLite access;
- kernel/render/UI/vendor dependencies.

## 3. Command surface

Exactly five mutation semantics are in scope:

- CreateLevel
- ChangeLevelElevation
- CreateStraightWall
- ChangeStraightWallGeometry
- DeleteElement

CreateLevel and CreateStraightWall receive model values carrying caller-supplied
durable ElementId values. Command-side identity generation is out of scope.

ChangeStraightWallGeometry changes only:

- start;
- end;
- thickness;
- base_offset;
- height.

The existing hosting level_id is preserved. Rehost is forbidden.

## 4. Command result contract

Public command result codes:

- Ok
- InvalidElement
- DuplicateElementId
- LevelNotFound
- ElementNotFound
- ElementKindMismatch
- ElementHasDependents
- RuntimeCapacityExhausted
- DerivedGeometryInvalid
- RecomputeFailed
- InternalFailure

InvalidElement preserves the model ValidationCode.

Runtime graph terminology is private. In particular NodeId, GraphResult,
RecomputePlan and GraphSnapshot may not appear in the public command contract.

Document NodeIdExhausted maps to RuntimeCapacityExhausted.
Document GraphRejected maps to InternalFailure.
Unexpected RehostNotAllowed maps to InternalFailure.

## 5. DeleteElement semantics

StraightWall deletion is allowed.

Level deletion is allowed only when no StraightWall references the Level.

A Level with one or more dependent StraightWalls is rejected with
ElementHasDependents and zero mutation.

No cascade delete and no automatic rehost exist in VS1.

## 6. Graph deletion strategy

DependencyGraph receives no public RemoveNode/DeleteNode extension.

Document deletion executes against a staged State.

After removing the selected authoritative element and its private runtime
mapping/cache entry, Document constructs a fresh private DependencyGraph from
all surviving mappings.

Every surviving ElementId retains its existing runtime node identity.

Every surviving Level and StraightWall node is registered in the rebuilt graph.
Every surviving wall receives exactly one ExplicitSemantic Level -> Wall edge.

The rebuilt graph must satisfy the same P1-T002 runtime invariants before the
staged State may be published.

Deleted runtime node identities are never reused.
The allocator is never decremented.

## 7. Atomicity

All mutation remains strongly atomic.

Failure leaves unchanged:

- Levels;
- StraightWalls;
- ElementId/runtime mappings;
- dependency graph;
- derived geometry cache;
- runtime identity allocator.

No mutate-then-rollback path is permitted.

## 8. Document extension

P1-T003 may add only:

- DocumentResultCode::ElementHasDependents
- Document::DeleteElement(ElementId)
- Document::ListLevels()
- Document::ListStraightWalls()

List operations expose committed values only and return copies.

List order is ElementId ascending.

No public runtime graph identity may be exposed.

## 9. Query boundary

Public query operations are:

- FindLevel
- FindStraightWall
- ListLevels
- ListStraightWalls

Every query receives const Document&.

Query never mutates Document and never observes staged/partial state.

Enumeration is deterministic by ElementId ascending, never by runtime node id.

## 10. Transaction decision

One command is one atomic Document mutation boundary.

P1-T003 does not modify src/transactions/** and does not materialize a durable
journal transaction.

Persistence, schema-v2, save/reopen and durable journal coordination belong to
P1-T004.

Undo/redo is out of VS1 scope.

## 11. Architecture enforcement

R1-R18 semantics remain unchanged.

P1-T003 adds:

R19 / COMMANDS_PUBLIC_BOUNDARY

R19 prevents command coupling/leakage to dependency_graph, transactions,
persistence, SQLite, geometry_occt, Qt, bgfx, ODA and IfcOpenShell, and keeps
runtime graph identities/types out of the public command surface.

R20 / QUERY_READ_ONLY_BOUNDARY

R20 rejects mutable Document references and command/document mutation calls
from query code, and rejects dependency_graph, transaction, persistence,
SQLite, kernel/render/UI/vendor coupling.

Each new rule has one real-tree positive test and one controlled negative
fixture test.

## 12. Explicit exclusions

Out of scope:

- schema-v2 persistence;
- save/reopen;
- SQLite changes;
- transaction module changes;
- durable journal changes;
- undo/redo;
- desktop/UI;
- tessellation;
- OCCT solid generation;
- cascade deletion;
- wall rehost;
- curved walls;
- multi-level walls;
- openings/families;
- constraints solver;
- parallel mutation/recompute.

## 13. Stop conditions

Return to Architecture Authority before implementation if fulfilling this
contract requires:

- modifying DependencyGraph public API;
- modifying src/transactions/**;
- modifying persistence/schema;
- exposing runtime NodeId;
- introducing cascade/rehost semantics;
- modifying any path outside the frozen footprint;
- weakening R1-R18;
- changing the dependency direction above.
