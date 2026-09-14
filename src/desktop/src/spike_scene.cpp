#include "spike_scene.hpp"

#include <cstdint>

namespace bim::desktop {

namespace {

// Appends one axis-aligned box (halfExtents may differ per axis, so this
// covers both cubes and thin elongated "bar" boxes) centered at `center`
// into `mesh`, adding to whatever positions/normals/indices already exist.
// Each of the 6 faces gets its own 4 vertices (not shared across faces) so
// every vertex has an unambiguous per-face normal, and index bases are
// offset from the mesh's current vertex count - this is what lets V03
// combine 1,000 boxes into a single RenderMeshData (Brief section 14: "may
// construct a combined neutral mesh; a public instancing architecture is
// not required") and what lets V02 build one mesh each for the grid lines
// and for each axis bar. Winding is CCW when viewed from outside (Brief
// section 4), matching the RH +Z-up convention.
void AppendBox(bim::viewport::RenderMeshData& mesh, bim::viewport::Vector3 center,
               bim::viewport::Vector3 halfExtents) {
    struct Face {
        bim::viewport::Vector3 normal;
        // Corners in unit [-1,1] box space; scaled by halfExtents below.
        bim::viewport::Vector3 corners[4];
    };

    const Face unit_faces[6] = {
        // +X
        {{1, 0, 0}, {{1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1}}},
        // -X
        {{-1, 0, 0}, {{-1, 1, -1}, {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}}},
        // +Y
        {{0, 1, 0}, {{1, 1, -1}, {-1, 1, -1}, {-1, 1, 1}, {1, 1, 1}}},
        // -Y
        {{0, -1, 0}, {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}}},
        // +Z (Z-up: this is the "top" face)
        {{0, 0, 1}, {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}}},
        // -Z
        {{0, 0, -1}, {{-1, 1, -1}, {1, 1, -1}, {1, -1, -1}, {-1, -1, -1}}},
    };

    for (const Face& face : unit_faces) {
        const std::uint32_t base = static_cast<std::uint32_t>(mesh.positions.size());
        for (const bim::viewport::Vector3& corner : face.corners) {
            mesh.positions.push_back({
                static_cast<float>(center.x + corner.x * halfExtents.x),
                static_cast<float>(center.y + corner.y * halfExtents.y),
                static_cast<float>(center.z + corner.z * halfExtents.z),
            });
            mesh.normals.push_back({
                static_cast<float>(face.normal.x),
                static_cast<float>(face.normal.y),
                static_cast<float>(face.normal.z),
            });
        }
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    }
}

bim::viewport::RenderMeshData MakeBox(bim::viewport::Vector3 center,
                                      bim::viewport::Vector3 halfExtents) {
    bim::viewport::RenderMeshData mesh;
    mesh.topology = bim::viewport::MeshTopology::TriangleList;
    AppendBox(mesh, center, halfExtents);
    return mesh;
}

// AA Source Review Round 2 finding M10: this was previously a local
// `constexpr` declared inside MakeV05LargeCoordinateScenario(), invisible
// to BuildSpikeScene()'s switch statement below. Moved to anonymous-
// namespace scope so BuildSpikeScene() can also derive V05's camera
// eye/target from it - without a camera actually near this cluster, V05's
// geometry sat tens of thousands of local-frame units from the default
// near-origin camera and was never actually observable, defeating the
// point of the experiment (Brief section 14: "observational evidence").
// This is purely a WHERE-the-camera-stands change; the coordinates remain
// ordinary local-frame doubles and the locked float32-position/double-math
// precision contract (bim/viewport/math.hpp, camera.hpp) is unchanged.
constexpr bim::viewport::Vector3 kV05Base{84500.0, 132500.0, 620.0};

// V01: single indexed cube (Brief section 14). Unit cube resting on the
// Z=0 ground plane (Z-up, Brief section 4), so its base sits at Z=0 and
// its top at Z=1.
bim::viewport::RenderMeshData MakeV01SingleIndexedCube() {
    return MakeBox({0.0, 0.0, 0.5}, {0.5, 0.5, 0.5});
}

// V02: grid + XYZ axes + cube (Brief section 14). Three separate meshes -
// a ground grid, one bar per axis, and a cube sitting on the grid - each
// its own RenderMeshData/handle, matching the Brief's three-part
// description literally. Only TriangleList is authorized (Brief section
// 4), so the grid and axes are built as thin extruded boxes rather than a
// line-list primitive.
std::vector<bim::viewport::RenderMeshData> MakeV02GridAxesCube() {
    std::vector<bim::viewport::RenderMeshData> meshes;

    // Ground grid: 11 lines each direction spanning [-5, 5] at spacing 1.0,
    // each line a thin flat box lying in the Z=0 plane, combined into one
    // mesh.
    {
        bim::viewport::RenderMeshData grid;
        grid.topology = bim::viewport::MeshTopology::TriangleList;
        constexpr double kHalfExtent = 5.0;
        constexpr double kSpacing = 1.0;
        constexpr double kLineHalfWidth = 0.015;
        constexpr double kLineHalfHeight = 0.01;
        for (double x = -kHalfExtent; x <= kHalfExtent + 1e-6; x += kSpacing) {
            AppendBox(grid, {x, 0.0, 0.0}, {kLineHalfWidth, kHalfExtent, kLineHalfHeight});
        }
        for (double y = -kHalfExtent; y <= kHalfExtent + 1e-6; y += kSpacing) {
            AppendBox(grid, {0.0, y, 0.0}, {kHalfExtent, kLineHalfWidth, kLineHalfHeight});
        }
        meshes.push_back(std::move(grid));
    }

    // XYZ axes: one bar per axis, from the origin outward, each its own
    // mesh so the evidence/handle accounting can distinguish them.
    constexpr double kAxisLength = 6.0;
    constexpr double kAxisHalfThickness = 0.04;
    meshes.push_back(MakeBox({kAxisLength / 2.0, 0.0, kAxisHalfThickness},
                             {kAxisLength / 2.0, kAxisHalfThickness, kAxisHalfThickness})); // +X
    meshes.push_back(MakeBox({0.0, kAxisLength / 2.0, kAxisHalfThickness},
                             {kAxisHalfThickness, kAxisLength / 2.0, kAxisHalfThickness})); // +Y
    meshes.push_back(MakeBox({0.0, 0.0, kAxisLength / 2.0},
                             {kAxisHalfThickness, kAxisHalfThickness, kAxisLength / 2.0})); // +Z

    // Cube sitting on the grid, offset from the origin so it does not
    // overlap the axis bars.
    meshes.push_back(MakeBox({2.0, 2.0, 0.5}, {0.5, 0.5, 0.5}));

    return meshes;
}

// V03: 1,000 repeated boxes (Brief section 14), combined into a single
// RenderMeshData rather than 1,000 separate CreateMesh calls (Brief
// section 14: "P0-T003 may construct a combined neutral mesh; a public
// instancing architecture is not required"). Arranged as a 10x10x10
// lattice (exactly 1,000 boxes) centered on the origin.
bim::viewport::RenderMeshData MakeV03OneThousandBoxes() {
    bim::viewport::RenderMeshData mesh;
    mesh.topology = bim::viewport::MeshTopology::TriangleList;

    constexpr int kPerAxis = 10; // 10 * 10 * 10 == 1,000
    constexpr double kSpacing = 2.0;
    constexpr double kHalfExtent = 0.4;
    const double origin_offset = -(static_cast<double>(kPerAxis - 1) * kSpacing) / 2.0;

    for (int ix = 0; ix < kPerAxis; ++ix) {
        for (int iy = 0; iy < kPerAxis; ++iy) {
            for (int iz = 0; iz < kPerAxis; ++iz) {
                const bim::viewport::Vector3 center{
                    origin_offset + static_cast<double>(ix) * kSpacing,
                    origin_offset + static_cast<double>(iy) * kSpacing,
                    origin_offset + static_cast<double>(iz) * kSpacing,
                };
                AppendBox(mesh, center, {kHalfExtent, kHalfExtent, kHalfExtent});
            }
        }
    }

    return mesh;
}

// V04: representative medium neutral mesh scene (Brief section 14). A
// subdivided ground-plane mesh at a resolution deliberately between V02's
// handful of primitives and V03's 1,000-box corpus - a single mesh with a
// non-trivial (but not extreme) vertex/index count, exercising a
// realistically-sized "medium" render payload.
bim::viewport::RenderMeshData MakeV04RepresentativeMediumMesh() {
    bim::viewport::RenderMeshData mesh;
    mesh.topology = bim::viewport::MeshTopology::TriangleList;

    constexpr std::uint32_t kSubdivisions = 24;
    constexpr float kHalfSize = 6.0f;
    const std::uint32_t verts_per_side = kSubdivisions + 1;
    const float step = (2.0f * kHalfSize) / static_cast<float>(kSubdivisions);

    for (std::uint32_t row = 0; row < verts_per_side; ++row) {
        for (std::uint32_t col = 0; col < verts_per_side; ++col) {
            const float x = -kHalfSize + static_cast<float>(col) * step;
            const float y = -kHalfSize + static_cast<float>(row) * step;
            mesh.positions.push_back({x, y, 0.0f});
            mesh.normals.push_back({0.0f, 0.0f, 1.0f});
        }
    }

    // AA Source Review Round 2 finding M09: this quad's two triangles were
    // previously wound (i0,i2,i1)/(i1,i2,i3), which is CW as seen from +Z -
    // the opposite of the {0,0,1} normals authored above and of AppendBox's
    // own "CCW when viewed from outside" convention (see this file's header
    // comment on AppendBox). Verified by explicit 2D cross-product sign
    // computation for the corner layout i0=(0,0), i1=(1,0), i2=(0,1),
    // i3=(1,1): (i0,i1,i2) and (i1,i3,i2) are the CCW-from-+Z orderings.
    // See tests/unit/unit_viewport_mesh_contract.cpp for regression
    // coverage mirroring this exact triangulation.
    for (std::uint32_t row = 0; row < kSubdivisions; ++row) {
        for (std::uint32_t col = 0; col < kSubdivisions; ++col) {
            const std::uint32_t i0 = row * verts_per_side + col;
            const std::uint32_t i1 = i0 + 1;
            const std::uint32_t i2 = i0 + verts_per_side;
            const std::uint32_t i3 = i2 + 1;
            mesh.indices.push_back(i0);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i3);
            mesh.indices.push_back(i2);
        }
    }

    return mesh;
}

// V05: large-coordinate scenario represented in local render coordinates
// (Brief section 14). Brief section 4 forbids submitting survey/global
// coordinates directly as GPU vertex positions - that conversion has
// already happened upstream - but a real localized site can still carry a
// large local-frame offset (e.g. a site several tens of kilometers across,
// already re-based to a local project origin). This scene places one
// "anchor" box and four small "marker" boxes near it at such a large - but
// still float32-representable and still local-frame, not global/survey -
// coordinate offset, so the evidence run can record how the renderer
// behaves at that magnitude (Brief section 14: observational evidence,
// not a pass/fail precision threshold).
bim::viewport::RenderMeshData MakeV05LargeCoordinateScenario() {
    bim::viewport::RenderMeshData mesh;
    mesh.topology = bim::viewport::MeshTopology::TriangleList;

    AppendBox(mesh, kV05Base, {1.0, 1.0, 1.0});

    constexpr double kMarkerOffset = 50.0;
    constexpr double kMarkerHalfExtent = 0.3;
    AppendBox(mesh, kV05Base + bim::viewport::Vector3{kMarkerOffset, 0.0, 0.0},
              {kMarkerHalfExtent, kMarkerHalfExtent, kMarkerHalfExtent});
    AppendBox(mesh, kV05Base + bim::viewport::Vector3{-kMarkerOffset, 0.0, 0.0},
              {kMarkerHalfExtent, kMarkerHalfExtent, kMarkerHalfExtent});
    AppendBox(mesh, kV05Base + bim::viewport::Vector3{0.0, kMarkerOffset, 0.0},
              {kMarkerHalfExtent, kMarkerHalfExtent, kMarkerHalfExtent});
    AppendBox(mesh, kV05Base + bim::viewport::Vector3{0.0, -kMarkerOffset, 0.0},
              {kMarkerHalfExtent, kMarkerHalfExtent, kMarkerHalfExtent});

    return mesh;
}

} // namespace

SpikeScene BuildSpikeScene(SpikeSceneId id) {
    SpikeScene scene;
    scene.id = id;

    switch (id) {
        case SpikeSceneId::V01_SingleIndexedCube:
            scene.name = "V01 - Single Indexed Cube";
            scene.meshes = {MakeV01SingleIndexedCube()};
            break;

        case SpikeSceneId::V02_GridAxesCube:
            scene.name = "V02 - Grid + XYZ Axes + Cube";
            scene.meshes = MakeV02GridAxesCube();
            break;

        case SpikeSceneId::V03_OneThousandBoxes:
            scene.name = "V03 - 1,000 Repeated Boxes";
            scene.meshes = {MakeV03OneThousandBoxes()};
            break;

        case SpikeSceneId::V04_RepresentativeMediumMesh:
            scene.name = "V04 - Representative Medium Neutral Mesh Scene";
            scene.meshes = {MakeV04RepresentativeMediumMesh()};
            break;

        case SpikeSceneId::V05_LargeCoordinateScenario:
            scene.name = "V05 - Large-Coordinate Scenario (Local Render Coordinates)";
            scene.meshes = {MakeV05LargeCoordinateScenario()};
            // AA Source Review Round 2 finding M10: point the camera at the
            // V05 cluster itself (kV05Base) rather than leaving the
            // SpikeScene default near-origin viewpoint, which can never see
            // geometry this far out. Eye is offset from kV05Base by
            // (0, -150, 75) local-frame units - distance ~= 167.7, giving a
            // half-width angle (~16.6 deg) comfortably inside the camera's
            // half-FOV (~25.78 deg, Brief section 14 default perspective),
            // so the anchor box and its four markers are actually framed.
            // cameraWorldUp stays the SpikeScene default {0, 0, 1}.
            scene.cameraEye = kV05Base + bim::viewport::Vector3{0.0, -150.0, 75.0};
            scene.cameraTarget = kV05Base;
            break;
    }

    return scene;
}

} // namespace bim::desktop
