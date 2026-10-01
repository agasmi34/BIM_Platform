# BIM Platform - P0-T010 Phase 1 Architecture Gate

**Document ID:** BIM-AG-P0-T010
**Version:** 1.0
**Date:** 2026-10-01
**Task:** P0-T010 - Phase 1 Architecture Gate
**Architecture Authority:** Product Authority + ChatGPT
**Product Authority decision:** APPROVED
**Gate status:** FROZEN / APPROVED
**Implementation status:** NOT AUTHORIZED
**ACR:** NONE

## 1. Authority and lifecycle

P0-T010 is the final Phase-0 governance gate before Phase-1 BIM feature
implementation may begin.

The Phase-0 roadmap defines P0-T010 as:

    Phase 1 Architecture Gate
    Locks Vertical Slice implementation contracts

Approval of this Gate does not authorize production implementation.

P0-T010 is governance-only.

Claude is not authorized to implement Phase-1 production code under this
task. Kimi is not authorized until Architecture Authority opens an
independent architecture-review checkpoint.

## 2. Authoritative baseline

    Branch = task/P0-T010-phase-1-architecture-gate
    HEAD   = af6201957b8b96382ec2fbd3bdf58c598da8dffb
    TREE   = d52bb08b0961e78a2dd933aa03609357270cfe3d

This is the accepted, integrated, pushed and cleaned P0-T009 baseline.

No dependency-baseline change is authorized by P0-T010.

## 3. Gate objective

The first production BIM vertical slice is:

**VS1 - Level-Constrained Straight Wall**

VS1 must prove one BIM element end-to-end across:

- durable BIM identity;
- model state;
- explicit semantic dependency evaluation;
- deterministic geometry regeneration;
- persistent semantic face references;
- command mutation;
- read-only query;
- transaction semantics;
- persistence;
- render-neutral presentation;
- desktop workflow.

VS1 is not a generic CAD extrusion demonstration.

## 4. Phase-0 contracts carried forward

### 4.1 Geometry

- OCCT remains owned by `bim_geometry_occt`.
- Public BIM/model contracts expose no raw kernel type.
- BIM parameters are authoritative.
- Generated geometry is derived.
- Raw topology identity is not durable BIM identity.

### 4.2 Persistent references

- semantic naming remains project-owned;
- topology enumeration order is non-authoritative;
- ambiguity fails closed;
- Missing and Ambiguous remain distinct;
- kernel object identity is not persistent semantic identity.

### 4.3 Dependency graph

- graph direction remains upstream -> downstream;
- downstream depends on upstream;
- cycle rejection remains fail-closed;
- planning remains deterministic;
- graph-state recompute commit remains atomic;
- DependencyGraph::NodeId remains runtime graph identity.

### 4.4 Persistence

- SQLite remains solely owned by `bim_persistence`;
- `PRAGMA user_version` remains schema-version authority;
- bootstrap/migration operations remain atomic and fail-closed;
- SQLite types remain behind persistence boundaries.

### 4.5 Viewport and desktop

- viewport APIs remain BIM-neutral and renderer-neutral;
- viewport does not couple directly to OCCT;
- Qt remains desktop-owned;
- bgfx/D3D remain viewport implementation concerns;
- GPU identity is never BIM identity.

## 5. VS1 domain scope

VS1 contains exactly two production BIM domain concepts.

### 5.1 Level

A Level is a durable BIM element defining elevation.

Minimum semantics:

- durable ElementId;
- project-owned name/label data as required later;
- elevation in model units.

A Level is an upstream dependency for a constrained StraightWall.

### 5.2 StraightWall

A StraightWall is a durable BIM element constrained to exactly one Level.

Minimum authoritative parameters:

- durable ElementId;
- referenced Level ElementId;
- ordered horizontal start point;
- ordered horizontal end point;
- thickness;
- height;
- base offset.

The wall axis direction is:

    start -> end

That direction is semantically significant.

## 6. Explicit VS1 non-goals

VS1 does not authorize:

- curved/sloped walls;
- variable thickness;
- compound layers;
- material systems;
- joins;
- openings;
- Door;
- Window;
- Slab;
- Roof;
- Column;
- Beam;
- Room;
- hosted elements;
- arbitrary constraints;
- dimensions;
- families;
- parametric formulas;
- snapping;
- grips;
- production selection architecture;
- collaboration;
- cloud synchronization;
- multi-user editing;
- IFC/DWG/RVT authoring;
- final documentation views.

## 7. Durable BIM identity - ElementId

Phase 1 introduces durable project-owned `ElementId`.

Required semantics:

- 128-bit identity space;
- first-party generated/assigned;
- stable across save/reopen;
- equality comparable;
- deterministically orderable;
- all-zero is invalid;
- independent of memory address;
- independent of container position;
- independent of database rowid;
- independent of OCCT identity;
- independent of DependencyGraph NodeId;
- independent of GPU identity;
- independent of IFC GlobalId;
- independent of DWG handle;
- independent of RVT ElementId.

A UUID-style canonical diagnostic/string form may be used.

No third-party UUID library is required.

Exact representation and generation mechanics belong to P1-T001.

## 8. P0-T008 owner-token transition

P0-T008 `FeatureOwnerId` remains a narrow historical spike token.

Production Phase-1 persistent references use durable `ElementId` as owner
identity.

The P0-T008 proof remains valid historical evidence and is not rewritten.

## 9. ElementId and DependencyGraph NodeId

The following distinction is permanent:

    ElementId != DependencyGraph::NodeId
    ElementId != raw kernel identity
    DependencyGraph::NodeId != PersistentFaceReference

NodeId remains runtime evaluation identity.

A higher-level runtime association owns:

    ElementId <-> NodeId

`bim_dependency_graph` remains model-neutral and must not depend on
ElementId.

Runtime NodeIds may be reconstructed after document reopen.

## 10. Phase-1 document coordination boundary

Phase 1 introduces:

    src/document/**
    bim_document
    bim::document

`bim_document` owns live document/runtime coordination without absorbing
lower-level module semantics.

It may coordinate:

- model/document state;
- ElementId lifecycle;
- ElementId <-> NodeId association;
- dependency invalidation/recompute;
- derived geometry state;
- transaction/journal coordination;
- persistence-facing open/save lifecycle.

Public document APIs remain first-party and vendor-neutral.

No raw OCCT, SQLite, Qt, bgfx, D3D or native Windows type may enter the
public document API.

## 11. Phase-1 dependency direction

Conceptually:

    persistence -> model + transactions + foundation + SQLite
    document -> model
    document -> transactions
    document -> dependency_graph
    document -> persistence
    document -> geometry_api
    commands -> document
    query -> document + model
    desktop -> commands + query + document-facing lifecycle + viewport

Hard rules:

- model does not depend on document;
- dependency_graph does not depend on document/model;
- transactions does not depend on document/persistence;
- persistence does not depend on document;
- viewport does not depend on document/model/OCCT;
- desktop does not directly mutate model;
- no first-party circular dependency.

## 12. Level -> StraightWall dependency

VS1 freezes:

    Level -> StraightWall

Meaning:

    StraightWall depends on Level

Changing Level elevation dirties dependent walls.

Changing wall defining parameters dirties that wall.

P0-T009 deterministic graph semantics remain authoritative.

## 13. Wall geometry frame

StraightWall geometry is a rectangular prism derived from BIM parameters.

Conceptual frame:

    U = ordered wall start -> wall end
    V = horizontal perpendicular to U
    W = vertical / +Z

The authoritative state is the BIM parameter set.

Derived geometry is regenerated after reopen.

Raw kernel topology is not authoritative persisted project data.

## 14. Command mutation boundary

Production BIM application/UI mutation occurs through `bim_commands`.

VS1 requires semantics equivalent to:

- CreateLevel
- ChangeLevelElevation
- CreateStraightWall
- ChangeStraightWallGeometry
- DeleteElement

Exact C++ naming belongs to later task briefs.

A command validates before committed mutation.

Desktop/UI code may not directly edit model containers.

## 15. Query boundary

`bim_query` remains read-only.

VS1 requires:

- lookup by ElementId;
- Level read view;
- StraightWall read view;
- deterministic element enumeration.

Query must not mutate model, document, dependency graph, geometry, journal
or persistence state.

## 16. Transaction and recompute atomicity

Conceptual lifecycle:

    validate command
      ->
    stage domain mutation
      ->
    stage dependency topology/invalidation
      ->
    build deterministic recompute plan
      ->
    evaluate derived geometry
      ->
    validate required semantic-reference continuity
      ->
    prepare journal record
      ->
    atomic document commit

Failure before commit must not leave:

- partially committed BIM state;
- falsely Clean graph state;
- partially committed journal state;
- inconsistent domain and derived state.

A successful command produces one committed transaction boundary.

Full user-facing undo/redo is outside VS1.

## 17. Geometry tolerance and authority

BIM parameters are authoritative and geometry is derived.

P0-T010 introduces no hidden universal tolerance.

VS1 uses an explicit project/document tolerance contract until separately
frozen.

OCCT remains behind the approved geometry boundary.

## 18. Persistent wall-face semantics

Production wall face references use durable ElementId ownership plus a
project-owned semantic role.

Conceptual roles:

- Bottom
- Top
- Start
- End
- SideA
- SideB

Exact enum spelling belongs to P1-T001.

After permitted edits, resolution must explicitly produce Resolved,
Missing, Ambiguous or an invalid reference/owner failure.

Enumeration order is never a tie breaker.

## 19. Persistence schema evolution

P0-T004 schema version 1 remains accepted historical architecture.

The first Phase-1 BIM persistence schema is:

    PRAGMA user_version = 2

Version 2 durably represents at minimum:

- required document metadata;
- ElementId;
- element type;
- Level authoritative state;
- StraightWall authoritative state;
- existing journal semantics.

A valid version-1 database advances only through explicit atomic:

    1 -> 2

migration.

A new empty database may initialize directly to the current supported schema
under existing fail-closed empty-database rules.

Not authoritative persisted BIM state:

- raw OCCT shapes/handles;
- DependencyGraph NodeIds;
- GPU resources;
- viewport buffers;
- transient face enumeration indices;
- derived geometry caches.

Exact SQL schema belongs to P1-T004.

## 20. Rendering integration

P0-T003 `RenderMeshData` remains renderer-neutral and BIM-neutral.

Required direction:

    BIM parameters
      ->
    derived geometry
      ->
    project-owned tessellation/extraction
      ->
    render-neutral mesh
      ->
    viewport

Direct viewport-to-OCCT coupling remains prohibited.

The presentation adapter is frozen later under P1-T005.

## 21. Desktop vertical-slice behavior

Final VS1 must allow a controlled workflow to:

1. create a Level;
2. create one StraightWall on that Level;
3. inspect the wall through query-facing data;
4. edit a wall dimension;
5. edit Level elevation;
6. observe dependent wall regeneration;
7. save;
8. close/reopen;
9. observe preserved ElementIds and BIM parameters;
10. regenerate derived geometry;
11. render through the neutral viewport path.

Final Ribbon, Project Browser, Properties palette and production editing UX
are not required.

## 22. Threading model

VS1 mutation, dependency evaluation, geometry recompute and transaction
commit are single-threaded and deterministic.

Not authorized:

- worker-pool recompute;
- background document mutation;
- parallel dependency evaluation;
- speculative geometry evaluation;
- renderer-thread architecture changes.

## 23. Interoperability boundary

IFC, DWG and RVT identifiers do not replace ElementId.

Future interop may maintain mappings/provenance to external IDs.

VS1 includes no IFC/DWG/RVT import/export implementation.

Closed Phase-0 evaluations remain closed.

## 24. Phase-1 test framework - ADR-0003

ADR-0001 left Phase-1+ framework choice provisional.

P0-T010 resolves it for VS1:

    Catch2 v3 + CTest = APPROVED

No framework replacement is authorized merely because Phase 1 begins.

A later framework migration requires its own ADR.

## 25. Mechanical architecture enforcement

Later Phase-1 tasks must mechanically prove, as applicable:

- no raw OCCT leak outside geometry ownership;
- no SQLite leak outside persistence;
- no Qt leak outside desktop;
- no bgfx/D3D leak outside viewport implementation;
- dependency_graph remains model-neutral;
- model remains persistence/UI/vendor neutral;
- ElementId contains no vendor identity;
- runtime NodeId is not durable identity;
- desktop cannot bypass commands;
- query remains read-only;
- persistence does not serialize raw kernel/GPU state;
- no first-party dependency cycle.

## 26. End-to-end VS1 acceptance proof

Final VS1 must prove:

1. Level creation;
2. StraightWall creation;
3. stable valid ElementIds;
4. Level -> StraightWall dependency;
5. deterministic wall geometry generation;
6. semantic wall-face reference resolution;
7. wall edit and recompute;
8. Level elevation edit and dependent recompute;
9. failed recompute gives no partial committed state;
10. query exposes committed state only;
11. schema-v2 save;
12. close/reopen;
13. preserved ElementIds;
14. preserved BIM parameters;
15. runtime graph reconstruction;
16. derived geometry regeneration;
17. render-neutral geometry reaches viewport;
18. regression tests pass;
19. architecture enforcement passes;
20. independent review has no unresolved BLOCKER or MAJOR finding.

## 27. Initial Phase-1 implementation sequence

### P1-T001 - Core BIM Identity & Domain Model

Owns ElementId, Level, StraightWall, wall semantic face-role contract and
model validation.

No graph integration, persistence or UI.

### P1-T002 - Document Runtime & Dependency Recompute

Owns bim_document, ElementId <-> NodeId association, Level -> StraightWall
graph registration, dirty propagation, deterministic recompute, derived
wall geometry lifecycle and atomic runtime commit semantics.

No schema-v2 persistence or desktop UX.

### P1-T003 - Commands & Query Vertical Slice

Owns the five VS1 mutation semantics and read-only Level/Wall query surfaces.

### P1-T004 - Persistence Schema v2

Owns schema v2, atomic 1 -> 2 migration, Level/Wall durable state,
ElementId persistence, save/reopen and graph reconstruction inputs.

### P1-T005 - Geometry Tessellation & Viewport Bridge

Owns project-owned geometry-to-render extraction and neutral wall mesh
delivery to viewport.

### P1-T006 - Desktop VS1 End-to-End Acceptance

Owns minimal desktop VS1 wiring and final integrated acceptance evidence.

No P1 task is implementation-authorized by P0-T010.

Each requires its own release.

## 28. Stop conditions

Return to Architecture Authority if implementation requires:

- raw kernel identity as ElementId;
- NodeId as durable identity;
- dependency_graph -> model dependency;
- persistence owning model semantics;
- direct UI model mutation;
- query mutation;
- raw kernel/GPU state persistence;
- direct viewport-to-OCCT dependency;
- changing SQLite schema-version authority;
- changing Level -> StraightWall dependency semantics;
- unauthorized third-party dependency;
- parallel document mutation/recompute;
- expansion into non-VS1 BIM features;
- first-party dependency cycle.

## 29. ACR status

    ACR = NONE

P0-T010 resolves decisions explicitly deferred to the Phase-1 Architecture
Gate and does not rewrite Phase-0 evidence.

## 30. Approval / lifecycle

Product Authority approved this architecture contract on 2026-10-01.

    Architecture intake                 = CLOSED / PASS
    Product Authority approval          = APPROVED
    Architecture contract               = FROZEN
    Gate materialization                = AUTHORIZED
    Governance candidate commit         = NOT AUTHORIZED
    Independent architecture review     = NOT AUTHORIZED
    Main integration                    = NOT AUTHORIZED
    Push                                = NOT AUTHORIZED
    Phase-1 production implementation   = NOT AUTHORIZED
    Claude                              = NOT AUTHORIZED
    Kimi                                = NOT AUTHORIZED
    ACR                                 = NONE

No later lifecycle action is authorized by this document.
