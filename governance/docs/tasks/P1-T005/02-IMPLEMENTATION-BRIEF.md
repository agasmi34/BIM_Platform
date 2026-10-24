# P1-T005 Implementation Brief

Architecture gate: BIM-AG-P1-T005 APPROVED / FROZEN
Implementation authorization: PENDING

## Exact implementation footprint

MODIFY:
1. CMakeLists.txt
2. tests/unit/CMakeLists.txt
3. tests/integration/CMakeLists.txt
4. tests/architecture/CMakeLists.txt
5. tools/architecture_checker.cmake

NEW:
6. src/presentation/CMakeLists.txt
7. src/presentation/include/bim/presentation/wall_mesh.hpp
8. src/presentation/src/wall_mesh.cpp
9. tests/unit/unit_p1_t005_wall_mesh.cpp
10. tests/integration/integration_p1_t005_geometry_viewport_bridge.cpp
11. tests/fixtures/p1_t005_bad_presentation_bridge/presentation/CMakeLists.txt
12. tests/fixtures/p1_t005_bad_viewport_upstream/viewport/CMakeLists.txt

DELETE: NONE

No implementation path outside this list may be modified or created.

## Root composition

The root order is:
add_subdirectory(src/viewport)
add_subdirectory(src/presentation)
add_subdirectory(src/viewport/bgfx)
add_subdirectory(src/desktop)

Do not opportunistically correct unrelated historical comments or files.

## Public presentation contract

Provide a small project-owned wall-mesh result contract in bim/presentation/wall_mesh.hpp.

The operation accepts:
- const bim::geometry_api::LinearExtrusionSpec&
- const bim::geometry_api::GeometryTolerance&

The successful value is bim::viewport::RenderMeshData.

Machine-readable failures distinguish at least invalid input, degenerate/incompatible wall geometry, float32 coordinate range failure, invalid render mesh and contained internal failure.

No third-party type and no BIM/runtime/GPU identity enters this public header.

## Implementation requirements

- Normalize U, V and W before scaling.
- Validate supplied tolerance.
- Validate finiteness and positive dimensions.
- Validate U/V orthogonality.
- Validate the VS1 right-handed wall frame.
- Build exactly 24 positions, 24 normals and 36 indices.
- Use the frozen face order and winding from the Architecture Gate.
- Check every double-to-float conversion.
- Canonicalize output zero to +0.0f.
- Call bim::viewport::ValidateMesh before returning success.
- Contain allocation/internal exceptions behind the result contract.
- Produce no partial successful mesh on failure.

## Unit proof

CTest: unit_p1_t005_wall_mesh

Cover at least:
- axis-aligned valid wall mesh exact counts/order.
- rotated plan wall.
- non-unit U/V/W inputs normalize deterministically.
- exact outward normals and CCW winding.
- repeated conversion yields identical values and float bit patterns.
- invalid tolerance.
- NaN/infinite input.
- non-positive dimensions.
- degenerate axes.
- skew U/V.
- incompatible/right-handed-frame failure.
- float32 out-of-range failure.
- ValidateMesh success on valid output.

## Integration proof

CTest: integration_p1_t005_geometry_viewport_bridge

The test may link document + presentation + viewport_bgfx for test composition only.

It must create a real Level and StraightWall, obtain Document::FindWallGeometry(), convert it through bim::presentation, validate the RenderMeshData, submit it to the existing headless bgfx Renderer, and prove CreateMesh / DestroyMesh success.

It must also prove that a committed wall/level geometry change produces the corresponding deterministic replacement mesh.

No production dependency from presentation to document is permitted.

## Architecture tests

1. arch_presentation_bridge_boundary
2. arch_p1_t005_presentation_bridge_fixture_rejected
3. arch_viewport_upstream_isolation
4. arch_p1_t005_viewport_upstream_fixture_rejected

Both negative tests use WILL_FAIL TRUE.

R23 scans the presentation source/header/CMake surface for forbidden upstream, vendor, persistence, UI and renderer coupling while allowing only geometry_api and viewport.

R24 proves viewport/viewport_bgfx do not depend upward on presentation, geometry_api, geometry_occt, document or model.

## Expected registration delta

Baseline full CTest count: 76
Baseline architecture count: 34
New runtime tests: 2
New architecture tests: 4
Expected candidate full count: 82
Expected candidate architecture count: 38

## Governance

No staging, commit, integration, push or independent review is authorized by this brief.
Any thirteenth implementation path is a STOP condition.
ACR is NONE unless a genuine frozen-architecture contradiction is discovered.
