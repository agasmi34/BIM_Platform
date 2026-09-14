#pragma once

#include <vector>

#include "bim/viewport/math.hpp"
#include "bim/viewport/mesh.hpp"

// src/desktop/src/spike_scene.hpp - procedural test-scene generation for
// the desktop spike's V01-V05 scenes (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0, section 14 "Spike scenes"). Pure
// bim::viewport::RenderMeshData production - no bgfx, no Renderer, no
// rendering side effects; MainWindow/ViewportWindow feed the returned
// meshes into Renderer::CreateMesh.
//
// AA Source Review Round 1 finding M02: the prior authoring pass shipped
// five scenes that were Claude's own invented interpretation of "a spike
// scene corpus," explicitly disclosed as not a verbatim transcription of
// Brief section 14. This revision replaces that corpus with the Brief's
// actual literal section 14 text:
//
//   V01: single indexed cube.
//   V02: grid + XYZ axes + cube.
//   V03: 1,000 repeated boxes. P0-T003 may construct a combined neutral
//        mesh; a public instancing architecture is not required.
//   V04: representative medium neutral mesh scene.
//   V05: large-coordinate scenario represented in local render coordinates.
//
// The Brief specifies these five scenes at exactly this level of detail -
// it does not pin exact vertex/box counts, grid spacing, or coordinate
// magnitudes. Those remain Claude's engineering parameters within the
// literal scene description (documented per-scene in spike_scene.cpp), not
// an invented replacement for the scene corpus itself. Per Brief section
// 14, "No performance threshold is pass/fail. Frame timings are
// observational evidence only" - the same is true of the exact
// vertex/triangle counts chosen here.
//
// AA Source Review Round 2 finding M09: V04's triangle winding was
// reversed relative to its own per-vertex {0,0,1} normals - every V04
// triangle was CW as seen from +Z despite being authored with +Z-outward
// normals, contradicting AppendBox's own documented "CCW when viewed from
// outside" convention used by every other scene here. Fixed in
// spike_scene.cpp; see tests/unit/unit_viewport_mesh_contract.cpp for the
// regression coverage this correction round added.

namespace bim::desktop {

// RD1.6-02 (performance-enum-size): the default enum representation is
// intentionally preserved for the Phase-0 contract; not changed merely
// for storage-size optimization (Architecture Authority decision).
enum class SpikeSceneId {         // NOLINT(performance-enum-size)
    V01_SingleIndexedCube,        // Brief section 14, V01: single indexed cube.
    V02_GridAxesCube,             // Brief section 14, V02: grid + XYZ axes + cube.
    V03_OneThousandBoxes,         // Brief section 14, V03: 1,000 repeated boxes,
                                  // combined into one neutral mesh (no public
                                  // instancing architecture required).
    V04_RepresentativeMediumMesh, // Brief section 14, V04: representative
                                  // medium neutral mesh scene.
    V05_LargeCoordinateScenario,  // Brief section 14, V05: large-coordinate
                                  // scenario represented in local render
                                  // coordinates.
};

struct SpikeScene {
    SpikeSceneId id = SpikeSceneId::V01_SingleIndexedCube;
    const char* name = "";
    std::vector<bim::viewport::RenderMeshData> meshes;

    // AA Source Review Round 2 finding M10: each scene now suggests the
    // camera look-at ViewportWindow should apply when this scene becomes
    // active (see ViewportWindow::SetSceneCamera, main_window.cpp's
    // OnSceneSelected). V01-V04 all default to the same near-origin
    // viewpoint ViewportWindow's own constructor previously hard-coded
    // (unchanged behavior for those four scenes); V05 overrides these in
    // BuildSpikeScene() with a viewpoint actually near its large-coordinate
    // cluster - without this, V05's geometry sits tens of thousands of
    // local-frame units from a camera that never moves away from the
    // origin, and is therefore never actually observable, defeating the
    // entire point of the V05 experiment (Brief section 14: "observational
    // evidence"). The coordinates themselves remain ordinary local-frame
    // doubles - Camera's own precision contract (bim/viewport/camera.hpp,
    // math.hpp) is unchanged by this; only WHERE the camera stands within
    // that local frame differs per scene.
    bim::viewport::Vector3 cameraEye{0.0, -6.0, 3.0};
    bim::viewport::Vector3 cameraTarget{0.0, 0.0, 0.5};
    bim::viewport::Vector3 cameraWorldUp{0.0, 0.0, 1.0};
};

[[nodiscard]] SpikeScene BuildSpikeScene(SpikeSceneId id);

} // namespace bim::desktop
