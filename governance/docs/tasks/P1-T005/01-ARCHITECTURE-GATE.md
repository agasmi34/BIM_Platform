# BIM-AG-P1-T005 - Architecture Gate

Decision: APPROVED / FROZEN
ACR: NONE

## Production module

Path: src/presentation
Target: bim::presentation

Allowed first-party dependency direction:

bim::presentation -> bim::geometry_api
bim::presentation -> bim::viewport

Forbidden presentation dependencies include model, document, commands, query, dependency_graph, transactions, persistence, geometry_occt, OCCT, bgfx, Qt, D3D, Windows native APIs, SQLite and interoperability modules.

viewport and viewport_bgfx must not depend upward on presentation, geometry_api, geometry_occt, document or model.

## Production flow

Document-derived LinearExtrusionSpec
  -> bim::presentation
  -> deterministic RenderMeshData
  -> existing viewport/bgfx consumer

P1-T005 introduces no OCCT tessellation in the rendering path.

## Geometry semantics

The adapter normalizes profile.u_axis, profile.v_axis and spec.direction before multiplying them by size_u, size_v and distance, matching the existing geometry contract.

For the VS1 wall bridge the normalized frame must be finite, non-degenerate, U/V orthogonal within the supplied angular tolerance and right-handed with W aligned to U x V within the supplied angular tolerance.

All dimensions and distance are finite and strictly positive.
GeometryTolerance has no default; the caller supplies it explicitly.

Corners:
p0 = origin
p1 = origin + U * size_u
p2 = origin + U * size_u + V * size_v
p3 = origin + V * size_v
q0 = p0 + W * distance
q1 = p1 + W * distance
q2 = p2 + W * distance
q3 = p3 + W * distance

## Deterministic mesh

Topology: TriangleList
Faces: 6
Triangles: 12
Positions: 24
Normals: 24
Indices: 36

Face order is frozen:
0 Bottom: p0,p3,p2,p1 ; normal -W
1 Top:    q0,q1,q2,q3 ; normal +W
2 Start:  p0,q0,q3,p3 ; normal -U
3 End:    p1,p2,q2,q1 ; normal +U
4 SideA:  p0,p1,q1,q0 ; normal -V
5 SideB:  p3,q3,q2,p2 ; normal +V

Each face uses local triangle indices 0,1,2 and 0,2,3 with the face base offset.
Winding is CCW when viewed from outside.
Normals are outward-facing.

The conversion from double geometry values to float32 viewport positions/normals is checked and fail-closed. No clamping, silent scaling or silent recentering is permitted.
Every output floating zero is canonicalized to positive zero.

## Identity

RenderMeshData contains no ElementId, NodeId, dependency identity, OCCT topology identity or GPU handle.
BIM identity and GPU identity remain separate.

## Architecture enforcement

R23 = PRESENTATION_BRIDGE_BOUNDARY
R24 = VIEWPORT_UPSTREAM_ISOLATION

Each rule requires one positive real-tree proof and one WILL_FAIL negative fixture proof.
