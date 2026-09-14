#include "main_window.hpp"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>

#include "viewport_window.hpp"

namespace {

struct SceneEntry {
    bim::desktop::SpikeSceneId id;
    const char* label;
};

// AA Source Review Round 1 finding M02: scene ids/labels restored to the
// Implementation Brief section 14 corpus (see spike_scene.hpp/.cpp).
constexpr SceneEntry kScenes[] = {
    {bim::desktop::SpikeSceneId::V01_SingleIndexedCube, "V01 - Single Indexed Cube"},
    {bim::desktop::SpikeSceneId::V02_GridAxesCube, "V02 - Grid + XYZ Axes + Cube"},
    {bim::desktop::SpikeSceneId::V03_OneThousandBoxes, "V03 - 1,000 Repeated Boxes"},
    {bim::desktop::SpikeSceneId::V04_RepresentativeMediumMesh,
     "V04 - Representative Medium Neutral Mesh Scene"},
    {bim::desktop::SpikeSceneId::V05_LargeCoordinateScenario,
     "V05 - Large-Coordinate Scenario (Local Render Coordinates)"},
};

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("BIM Platform - P0-T003 Desktop + Viewport Spike"));
    resize(1280, 800);

    viewport_window_ = new ViewportWindow(this);
    setCentralWidget(viewport_window_);

    BuildMenus();
    BuildStatusBar();

    // Default to the first scene once the widget is constructed. The
    // selection is remembered by ViewportWindow::SetActiveSceneMeshes
    // regardless of whether the renderer has a live surface yet, and is
    // automatically (re-)applied the moment ViewportWindow's renderer
    // actually becomes ready (see ViewportWindow::EnsureRendererInitialized
    // / ApplyPendingSceneMeshesIfReady) - it is never silently dropped, so
    // no manual re-selection is needed (AA Source Review Round 1 finding
    // M04; this used to require re-selecting the scene in the combo box).
    OnSceneSelected(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::BuildMenus() {
    QToolBar* toolbar = addToolBar(QStringLiteral("Scenes"));
    toolbar->setMovable(false);

    scene_selector_ = new QComboBox(toolbar);
    for (const SceneEntry& entry : kScenes) {
        scene_selector_->addItem(QString::fromUtf8(entry.label));
    }
    connect(scene_selector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::OnSceneSelected);
    toolbar->addWidget(scene_selector_);
}

void MainWindow::BuildStatusBar() {
    status_label_ = new QLabel(QStringLiteral("Ready"), this);
    statusBar()->addPermanentWidget(status_label_);
}

void MainWindow::OnSceneSelected(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= std::size(kScenes)) {
        return;
    }
    const SceneEntry& entry = kScenes[index];
    const bim::desktop::SpikeScene scene = bim::desktop::BuildSpikeScene(entry.id);
    viewport_window_->SetActiveSceneMeshes(scene.meshes);
    // AA Source Review Round 2 finding M10: apply each scene's suggested
    // camera look-at (SpikeScene::cameraEye/cameraTarget/cameraWorldUp) -
    // previously this scene switch never touched the camera at all, so V05
    // (spike_scene.cpp) was never actually observable at its large local-
    // frame offset.
    viewport_window_->SetSceneCamera(scene.cameraEye, scene.cameraTarget, scene.cameraWorldUp);
    status_label_->setText(QString::fromUtf8(entry.label));
}
