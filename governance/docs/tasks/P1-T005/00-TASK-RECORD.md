# P1-T005 - Geometry Tessellation & Viewport Bridge

Status: ARCHITECTURE FROZEN; IMPLEMENTATION NOT AUTHORIZED

Baseline commit: 23d8e5274a488f4a54476ae2c9e03439c2460d18
Baseline tree: bf8e56585190a17de5f9110dfdef77fb93d43d51
Task branch: task/P1-T005-geometry-tessellation-viewport-bridge

## Objective

Add the Phase-1 production presentation boundary that converts the committed VS1 StraightWall neutral LinearExtrusionSpec into deterministic renderer-neutral RenderMeshData and proves that mesh through the existing headless viewport/bgfx path.

## In scope

- New bim::presentation production module.
- Deterministic analytic rectangular-prism tessellation.
- LinearExtrusionSpec -> RenderMeshData.
- Project-owned validation and failure classification.
- R23 PRESENTATION_BRIDGE_BOUNDARY.
- R24 VIEWPORT_UPSTREAM_ISOLATION.
- Unit, integration and architecture proof.

## Out of scope

- OCCT tessellation or BRepMesh.
- Modification of geometry_api or geometry_occt.
- Modification of document, model, commands or query.
- Modification of viewport or viewport_bgfx production code.
- Desktop BIM wiring; deferred to P1-T006.
- Persistence changes.
- GPU/BIM identity coupling.
- IFC, DWG, RVT or other interoperability.
- General-purpose CAD tessellation.
- Undo/redo, collaboration, autosave or threading changes.

## Frozen implementation footprint

Exactly 12 paths: 5 modified, 7 new, 0 deleted.
Any thirteenth implementation path is a STOP condition.

Governance files under governance/docs/tasks/P1-T005 are project-control artifacts and are not part of the 12-path implementation footprint.

ACR: NONE
