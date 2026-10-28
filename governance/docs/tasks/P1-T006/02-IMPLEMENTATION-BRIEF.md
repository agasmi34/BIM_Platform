# P1-T006 Implementation Brief

ID: P1-T006-IB

Architecture:
BIM-AG-P1-T006

Status:
FROZEN / NOT IMPLEMENTATION AUTHORIZATION

## 1. Exact implementation footprint

Exactly 17 implementation paths are authorized for a later implementation gate.

### Modify — 8

1. src/model/CMakeLists.txt
2. src/desktop/CMakeLists.txt
3. src/desktop/src/main_window.cpp
4. src/desktop/src/main_window.hpp
5. tests/unit/CMakeLists.txt
6. tests/integration/CMakeLists.txt
7. tests/architecture/CMakeLists.txt
8. tools/architecture_checker.cmake

### New — 9

9. src/model/include/bim/model/wall_face_reference_resolver.hpp
10. src/model/src/wall_face_reference_resolver.cpp
11. src/desktop/include/bim/desktop/vs1_workflow.hpp
12. src/desktop/src/vs1_workflow.cpp
13. tests/unit/unit_p1_t006_wall_face_reference_resolver.cpp
14. tests/integration/integration_p1_t006_vs1_e2e.cpp
15. tests/fixtures/p1_t006_bad_desktop_direct_document_mutation/desktop/src/bad.cpp
16. tests/fixtures/p1_t006_bad_desktop_dependency/desktop/CMakeLists.txt
17. tests/fixtures/p1_t006_bad_wall_face_resolution/model/include/bim/model/wall_face_reference_resolver.hpp

Delete:
NONE

Footprint manifest SHA256:

1CD748B871D404931B8C9D0DA1B50DDB6853AC28F4BFEC779A0251FE90216747

A different implementation path is a STOP condition.

## 2. Model implementation

Add wall_face_reference_resolver.cpp to bim_model.

Do not modify the existing production WallFaceReference contract.

Do not modify historical P0-T008 resolver files.

Implement the exact WallFaceResolutionStatus, WallFaceResolutionResult, WallFaceFrame and ResolveWallFaceReference contract frozen by BIM-AG-P1-T006.

The resolver must:
- validate WallFaceReference;
- validate owner equality;
- validate tolerance and current extrusion frame;
- reject failed FaceObservationResult;
- derive the semantic expected plane from current LinearExtrusionSpec;
- scan observed planar faces independently;
- return Missing / Resolved / Ambiguous deterministically;
- never use enumeration order as identity;
- never expose or include kernel types;
- never throw.

## 3. Desktop workflow target

Create:

bim_desktop_vs1_workflow

Alias:

bim::desktop_vs1_workflow

Source:
src/desktop/src/vs1_workflow.cpp

Public header:
src/desktop/include/bim/desktop/vs1_workflow.hpp

It is Qt-free.

It directly links only:
- bim::commands
- bim::query
- bim::document
- bim::presentation

The workflow owns its Document and coordinates VS1 user operations.

Durable IDs are generated through the already-approved ElementId generation contract reached through the public model value types exposed by the command/document API.

Identity generation failure must fail closed without partial mutation.

## 4. Required workflow behavior

The workflow must provide operations sufficient for:

- create Level;
- create StraightWall hosted by an existing Level;
- change Level elevation;
- edit StraightWall geometry;
- delete an element;
- save;
- open;
- list/read committed Levels and StraightWalls;
- rebuild current wall RenderMeshData.

All mutations use bim::commands only.

All domain enumeration/read operations use bim::query.

Document::FindWallGeometry is allowed only to obtain committed derived geometry for presentation.

Save/Open use Document lifecycle APIs.

No direct Document mutation is allowed.

## 5. Mesh refresh

For every committed StraightWall in deterministic query order:

1. obtain its committed derived LinearExtrusionSpec;
2. call bim::presentation::BuildWallMesh with explicit GeometryTolerance;
3. require successful WallMeshResult;
4. append its RenderMeshData.

Failure is explicit and must not publish a partially rebuilt render scene.

No tessellation logic may be duplicated in desktop.

## 6. MainWindow integration

Replace the normal interactive spike-only workflow with a minimum VS1 authoring workflow while preserving P0-T003 evidence modes and renderer/lifecycle infrastructure.

MainWindow must own/use the desktop VS1 workflow.

The existing ViewportWindow remains the renderer-facing widget.

After every successful mutation or Open:
- rebuild VS1 render meshes through the workflow;
- call ViewportWindow::SetActiveSceneMeshes only with a complete successful mesh set.

Provide minimum user-accessible actions for:
- create Level;
- create StraightWall;
- edit wall;
- change Level elevation;
- delete;
- Save;
- Open.

A compact deterministic acceptance-oriented UI is sufficient.
P1-T006 does not require a complete production authoring UX.

Do not modify viewport_window.cpp/.hpp.

Do not modify spike/evidence-mode source files.

## 7. Production wall-face resolution proof

unit_p1_t006_wall_face_reference_resolver must cover at minimum:

- all six production WallFaceRole values;
- correct resolution against valid neutral observations;
- wall ElementId owner mismatch -> InvalidOwner;
- invalid WallFaceReference -> InvalidReference;
- failed FaceObservationResult -> InvalidReference;
- invalid tolerance -> InvalidReference;
- missing face -> Missing;
- duplicate matching planes -> Ambiguous;
- shuffled observation order does not change semantic result/status;
- current wall geometry changes cause re-resolution against the new plane;
- resolved_face_index is transient result only;
- production types do not use P0-T008 identity types.

## 8. VS1 end-to-end integration proof

integration_p1_t006_vs1_e2e must demonstrate through public production boundaries:

1. create Level through bim::commands;
2. create StraightWall through bim::commands;
3. verify durable ElementIds;
4. verify Level -> StraightWall behavior;
5. verify derived geometry;
6. create all six production WallFaceReference values;
7. make real geometry through bim::geometry_api;
8. obtain real neutral FaceObservationResult;
9. resolve all six production references;
10. edit wall geometry through command;
11. verify recomputed geometry;
12. re-resolve references against regenerated geometry;
13. change Level elevation through command;
14. verify wall derived geometry moves with Level;
15. attempt an invalid command and prove no partial committed state;
16. prove query reads committed state;
17. Save schema-v2 document;
18. Open into a new Document;
19. prove durable ElementIds survive;
20. prove wall parameters survive;
21. prove dependency behavior is reconstructed after reopen;
22. prove derived wall geometry is regenerated;
23. re-resolve semantic wall-face references after reopen;
24. BuildWallMesh from reopened derived geometry;
25. consume the mesh through the real viewport/bgfx path used by the existing integration infrastructure.

The integration test may link additional modules needed for test composition, including:
- bim::geometry_api
- bim::geometry_occt
- bim::model
- bim::commands
- bim::query
- bim::document
- bim::presentation
- bim::viewport
- bim::viewport_bgfx
- Catch2

This composition is test-only and does not widen desktop production dependencies.

## 9. Architecture tests

Add exactly these six registrations:

arch_desktop_vs1_command_boundary
arch_p1_t006_desktop_direct_document_mutation_fixture_rejected
arch_desktop_vs1_dependency_boundary
arch_p1_t006_desktop_dependency_fixture_rejected
arch_production_wall_face_resolution_boundary
arch_p1_t006_wall_face_resolution_fixture_rejected

The real-tree tests must PASS.

Each bad fixture must be rejected and registered with WILL_FAIL TRUE.

## 10. Expected CTest totals

Baseline full:
82

Baseline architecture:
38

Added runtime:
2

Added architecture:
6

Expected full:
90

Expected architecture:
44

Focused runtime:
2 / 2

Focused architecture:
6 / 6

## 11. Protected paths

The following existing contracts are frozen and MUST NOT be modified:

src/model/include/bim/model/wall_face_reference.hpp
src/commands/include/bim/commands/commands.hpp
src/document/include/bim/document/document.hpp
src/query/include/bim/query/query.hpp
src/presentation/include/bim/presentation/wall_mesh.hpp
src/geometry/api/include/bim/geometry_api/geometry.hpp
src/geometry/api/include/bim/geometry_api/face_observation.hpp
src/viewport/include/bim/viewport/mesh.hpp
src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp

Historical P0-T008 files are also frozen.

## 12. No scope expansion

No:
- root CMake change;
- document source change;
- commands source change;
- query source change;
- presentation source change;
- viewport source change;
- geometry API/OCCT change;
- persistence/schema change;
- dependency_graph change;
- transaction change;
- new package;
- threading change;
- new BIM element type;
- worker pool;
- undo/redo;
- IFC/DWG implementation.

## 13. Validation posture

Claude local validation, once separately authorized, is non-authoritative.

Authoritative validation is a later native Windows gate.

No staging, commit, push, integration or cleanup is implied by implementation authorization.

Kimi review is a later independent gate.

ACR:
NONE
