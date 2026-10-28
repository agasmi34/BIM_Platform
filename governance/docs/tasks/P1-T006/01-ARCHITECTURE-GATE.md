# P1-T006 Architecture Gate

ID: BIM-AG-P1-T006

Status: APPROVED / FROZEN

## 1. Scope

P1-T006 owns the final Phase-1 VS1 desktop end-to-end acceptance wiring.

It does not introduce a new BIM element type, new persistence schema, new geometry kernel, new renderer or new threading architecture.

The frozen VS1 production flow is:

UI
-> desktop VS1 workflow
-> bim::commands
-> bim::document
-> committed BIM state
-> derived LinearExtrusionSpec
-> bim::presentation::BuildWallMesh
-> RenderMeshData
-> existing viewport/bgfx consumer

Read access is through bim::query.

Save/Open are document lifecycle operations.

## 2. Production mutation law

Desktop/UI code MUST NOT call Document mutation methods directly.

Forbidden from src/desktop production code:

Document::AddLevel
Document::AddStraightWall
Document::UpdateLevelElevation
Document::UpdateStraightWall
Document::DeleteElement

Production mutation MUST use:

bim::commands::CreateLevel
bim::commands::CreateStraightWall
bim::commands::ChangeLevelElevation
bim::commands::ChangeStraightWallGeometry
bim::commands::DeleteElement

Queries remain read-only.

## 3. Desktop VS1 workflow target

A project-owned Qt-free orchestration target is added inside src/desktop:

bim_desktop_vs1_workflow

Its public alias is:

bim::desktop_vs1_workflow

Direct target dependencies are limited to:

bim::commands
bim::query
bim::document
bim::presentation

Model value types may appear only through those already-approved public contracts; there is no direct bim::model target dependency.

The workflow MUST NOT directly link:

bim::geometry_occt
bim::dependency_graph
bim::transactions
bim::persistence
SQLite
OCCT
Qt
bgfx
D3D
Windows rendering API
ODA
IfcOpenShell

The existing Qt executable remains bim_desktop_spike for P0-T003 regression compatibility.

The Qt executable may link:

bim::desktop_vs1_workflow
bim::viewport
bim::viewport_bgfx
Qt6::Widgets

## 4. Production WallFaceReference resolution

P1-T006 materializes the deferred Phase-1 production wall-face resolver in bim::model.

The production contract is separate from P0-T008 historical types.

Required public contract:

namespace bim::model {

enum class WallFaceResolutionStatus : std::uint8_t {
    Resolved,
    Missing,
    Ambiguous,
    InvalidReference,
    InvalidOwner
};

struct WallFaceResolutionResult {
    WallFaceResolutionStatus status =
        WallFaceResolutionStatus::InvalidReference;

    std::size_t resolved_face_index = 0;

    [[nodiscard]] bool ok() const noexcept {
        return status == WallFaceResolutionStatus::Resolved;
    }
};

struct WallFaceFrame {
    ElementId wall_id;
    bim::geometry_api::LinearExtrusionSpec spec;
};

[[nodiscard]] WallFaceResolutionResult ResolveWallFaceReference(
    const WallFaceReference& reference,
    const WallFaceFrame& current_wall,
    const bim::geometry_api::FaceObservationResult& observed_faces,
    const bim::geometry_api::GeometryTolerance& tolerance) noexcept;

}

The exact production files are:

src/model/include/bim/model/wall_face_reference_resolver.hpp
src/model/src/wall_face_reference_resolver.cpp

## 5. Production face identity

Durable identity is:

WallFaceReference.wall_id + WallFaceReference.role

The six roles remain exactly:

Bottom
Top
Start
End
SideA
SideB

No seventh role is authorized.

resolved_face_index is transient output only.

It MUST NOT be:
- persisted;
- stored in WallFaceReference;
- treated as durable identity;
- assumed stable across geometry regeneration;
- assumed stable across save/reopen.

Enumeration order is never identity.

## 6. Wall frame semantics

For the current wall LinearExtrusionSpec:

U = normalize(spec.profile.u_axis)
V = normalize(spec.profile.v_axis)
W = normalize(spec.direction)

The semantic planes are:

Bottom:
profile origin plane, outward semantic direction -W

Top:
origin + W * distance plane, outward semantic direction +W

Start:
minimum-U plane, outward semantic direction -U

End:
maximum-U plane, outward semantic direction +U

SideA:
minimum-V plane, outward semantic direction -V

SideB:
maximum-V plane, outward semantic direction +V

Resolution is geometric plane matching against neutral FaceObservationResult.

Observed normals may be parallel or anti-parallel for plane matching; identity comes from the semantic expected plane, not observed enumeration or kernel orientation.

A candidate face matches only when:
- its plane normal is parallel within tolerance.angular_radians; and
- its point-on-plane lies within tolerance.linear of the semantic expected plane.

Zero matching faces -> Missing.
Exactly one -> Resolved.
More than one -> Ambiguous.

## 7. Resolver fail-closed law

ResolveWallFaceReference MUST return InvalidReference when:
- WallFaceReference is structurally invalid;
- tolerance is invalid/non-finite;
- neutral observations contain an upstream error;
- the supplied extrusion frame cannot define the six valid semantic planes.

It MUST return InvalidOwner when:
reference.wall_id != current_wall.wall_id.

No exception may escape.

No OCCT, bgfx, Qt, NodeId, GPU type, kernel handle or historical P0-T008 identity type may appear in the production resolver contract.

## 8. P0-T008 historical isolation

These remain unchanged:

src/model/include/bim/model/persistent_face_reference.hpp
src/model/include/bim/model/face_reference_resolver.hpp
src/model/src/face_reference_resolver.cpp
tests/unit/unit_model_face_reference_resolver.cpp
tests/integration/integration_p0_t008_face_reference_spike.cpp

Production P1-T006 MUST NOT reinterpret or alias:

FeatureOwnerId
FaceRole
PersistentFaceReference
FeatureFrame

as Phase-1 production identity.

## 9. Rendering law

Desktop VS1 rendering refresh is:

bim::query::ListStraightWalls
-> Document::FindWallGeometry
-> bim::presentation::BuildWallMesh
-> std::vector<RenderMeshData>
-> ViewportWindow::SetActiveSceneMeshes

Desktop does not tessellate.

Desktop does not call OCCT.

Desktop does not persist render meshes or GPU handles.

GeometryTolerance is supplied explicitly.

## 10. Save/Open

Save uses Document::Save.

Open uses Document::Open into a side document and replaces workflow state only on success.

Successful Open must rebuild runtime identities/dependencies/derived geometry through the already-approved Document lifecycle.

P1-T006 adds no persistence format or schema change.

## 11. Architecture rules

R25 = DESKTOP_VS1_COMMAND_BOUNDARY

R26 = DESKTOP_VS1_DEPENDENCY_BOUNDARY

R27 = PRODUCTION_WALL_FACE_RESOLUTION_BOUNDARY

Each rule requires:
- real-tree positive proof;
- deliberately bad fixture negative proof.

## 12. Stop conditions

STOP and return to Architecture Authority if implementation requires:
- an 18th implementation path;
- modification of a protected production contract;
- modification of P0-T008 historical assets;
- new third-party dependency;
- persistence schema change;
- direct desktop -> OCCT coupling;
- direct viewport -> OCCT coupling;
- direct desktop mutation of Document;
- durable NodeId;
- durable transient face index;
- kernel/GPU identity persistence;
- first-party dependency cycle;
- worker-thread/background mutation architecture;
- any contradiction with BIM-AG-P1-T006.

ACR = NONE unless such a contradiction genuinely requires architecture change.
