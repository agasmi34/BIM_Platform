#include "main_window.hpp"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "viewport_window.hpp"

namespace {

// ---- P1-T006 VS1 authoring helpers ------------------------------------------

struct WallInput {
    double start_x = 0.0;
    double start_y = 0.0;
    double end_x = 5.0;
    double end_y = 0.0;
    double thickness = 0.2;
    double height = 3.0;
    double base_offset = 0.0;
};

struct Choice {
    QString label;
    bim::model::ElementId id;
};

constexpr double kSpinLimit = 1.0e6;

QString IdText(const bim::model::ElementId& id) {
    return QString::fromStdString(bim::model::ToString(id));
}

QString LevelLabel(const bim::model::Level& level) {
    return QStringLiteral("Level %1  (elevation %2)")
        .arg(IdText(level.id))
        .arg(level.elevation, 0, 'g', 10);
}

QString WallLabel(const bim::model::StraightWall& wall) {
    return QStringLiteral("Wall %1  (%2,%3) -> (%4,%5)")
        .arg(IdText(wall.id))
        .arg(wall.start.x, 0, 'g', 8)
        .arg(wall.start.y, 0, 'g', 8)
        .arg(wall.end.x, 0, 'g', 8)
        .arg(wall.end.y, 0, 'g', 8);
}

// Lets the user choose one entry. An empty `choices` list or a cancelled dialog
// yields no selection.
std::optional<bim::model::ElementId> PickElement(QWidget* parent, const QString& title,
                                                 const QString& prompt,
                                                 const std::vector<Choice>& choices) {
    if (choices.empty()) {
        return std::nullopt;
    }
    QStringList labels;
    for (const Choice& choice : choices) {
        labels.append(choice.label);
    }
    bool accepted = false;
    const QString picked =
        QInputDialog::getItem(parent, title, prompt, labels, 0, false, &accepted);
    if (!accepted) {
        return std::nullopt;
    }
    const auto index = labels.indexOf(picked);
    if (index < 0 || static_cast<std::size_t>(index) >= choices.size()) {
        return std::nullopt;
    }
    return choices[static_cast<std::size_t>(index)].id;
}

QDoubleSpinBox* MakeSpin(QWidget* parent, double minimum, double value) {
    auto* spin = new QDoubleSpinBox(parent);
    spin->setDecimals(4);
    spin->setRange(minimum, kSpinLimit);
    spin->setValue(value);
    return spin;
}

// Modal wall-parameter dialog. Returns true and fills `io` only when accepted.
// Range checks here are a convenience: bim::commands remains the authority and
// rejects every invalid value.
bool PromptWallInput(QWidget* parent, const QString& title, WallInput& io) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    auto* form = new QFormLayout(&dialog);
    QDoubleSpinBox* start_x = MakeSpin(&dialog, -kSpinLimit, io.start_x);
    QDoubleSpinBox* start_y = MakeSpin(&dialog, -kSpinLimit, io.start_y);
    QDoubleSpinBox* end_x = MakeSpin(&dialog, -kSpinLimit, io.end_x);
    QDoubleSpinBox* end_y = MakeSpin(&dialog, -kSpinLimit, io.end_y);
    QDoubleSpinBox* thickness = MakeSpin(&dialog, 0.0, io.thickness);
    QDoubleSpinBox* height = MakeSpin(&dialog, 0.0, io.height);
    QDoubleSpinBox* base_offset = MakeSpin(&dialog, -kSpinLimit, io.base_offset);
    form->addRow(QStringLiteral("Start X"), start_x);
    form->addRow(QStringLiteral("Start Y"), start_y);
    form->addRow(QStringLiteral("End X"), end_x);
    form->addRow(QStringLiteral("End Y"), end_y);
    form->addRow(QStringLiteral("Thickness"), thickness);
    form->addRow(QStringLiteral("Height"), height);
    form->addRow(QStringLiteral("Base offset"), base_offset);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    io.start_x = start_x->value();
    io.start_y = start_y->value();
    io.end_x = end_x->value();
    io.end_y = end_y->value();
    io.thickness = thickness->value();
    io.height = height->value();
    io.base_offset = base_offset->value();
    return true;
}

std::optional<double> PromptElevation(QWidget* parent, const QString& title, double current) {
    bool accepted = false;
    const double value = QInputDialog::getDouble(parent, title, QStringLiteral("Elevation"),
                                                 current, -kSpinLimit, kSpinLimit, 4, &accepted);
    if (!accepted) {
        return std::nullopt;
    }
    return value;
}

std::vector<Choice> LevelChoices(const bim::desktop::Vs1Workflow& workflow) {
    std::vector<Choice> choices;
    for (const bim::model::Level& level : workflow.ListLevels()) {
        choices.push_back(Choice{LevelLabel(level), level.id});
    }
    return choices;
}

std::vector<Choice> WallChoices(const bim::desktop::Vs1Workflow& workflow) {
    std::vector<Choice> choices;
    for (const bim::model::StraightWall& wall : workflow.ListWalls()) {
        choices.push_back(Choice{WallLabel(wall), wall.id});
    }
    return choices;
}

const char* WorkflowCodeText(bim::desktop::WorkflowCode code) noexcept {
    switch (code) {
        case bim::desktop::WorkflowCode::Ok:
            return "ok";
        case bim::desktop::WorkflowCode::IdentityGenerationFailed:
            return "no element identity could be generated";
        case bim::desktop::WorkflowCode::CommandRejected:
            return "the command was rejected";
        case bim::desktop::WorkflowCode::SaveFailed:
            return "the document could not be saved";
        case bim::desktop::WorkflowCode::OpenFailed:
            return "the document could not be opened";
        case bim::desktop::WorkflowCode::RenderRebuildFailed:
            return "the scene could not be rebuilt";
        case bim::desktop::WorkflowCode::InternalFailure:
            return "internal failure";
    }
    return "unknown failure";
}

std::filesystem::path ToPath(const QString& text) {
    return std::filesystem::path(text.toStdWString());
}

// Frames the camera on the bounding box of `meshes` (Z-up world).
void FrameCamera(ViewportWindow& viewport,
                 const std::vector<bim::viewport::RenderMeshData>& meshes) {
    constexpr double kInf = std::numeric_limits<double>::infinity();
    double lo[3] = {kInf, kInf, kInf};
    double hi[3] = {-kInf, -kInf, -kInf};
    bool any = false;
    for (const bim::viewport::RenderMeshData& mesh : meshes) {
        for (const bim::viewport::Position3f& p : mesh.positions) {
            const double v[3] = {static_cast<double>(p.x), static_cast<double>(p.y),
                                 static_cast<double>(p.z)};
            for (int axis = 0; axis < 3; ++axis) {
                lo[axis] = std::min(lo[axis], v[axis]);
                hi[axis] = std::max(hi[axis], v[axis]);
            }
            any = true;
        }
    }
    if (!any) {
        return;
    }
    const bim::viewport::Vector3 center{(lo[0] + hi[0]) / 2.0, (lo[1] + hi[1]) / 2.0,
                                        (lo[2] + hi[2]) / 2.0};
    const double extent = std::max({hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2], 1.0});
    const double distance = extent * 2.0;
    const bim::viewport::Vector3 eye{center.x + distance, center.y - distance,
                                     center.z + distance * 0.8};
    viewport.SetSceneCamera(eye, center, bim::viewport::Vector3{0.0, 0.0, 1.0});
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("BIM Platform - Desktop VS1 Authoring"));
    resize(1280, 800);

    viewport_window_ = new ViewportWindow(this);
    setCentralWidget(viewport_window_);

    BuildMenus();
    BuildStatusBar();

    // The VS1 scene starts empty: the viewport shows nothing until a wall exists.
    // (The renderer remembers the request until its surface is live - see
    // ViewportWindow::SetActiveSceneMeshes.)
    RefreshScene(/*frame_camera=*/false);
    status_label_->setText(QStringLiteral("Ready - create a Level, then a wall"));
}

MainWindow::~MainWindow() = default;

void MainWindow::BuildMenus() {
    QToolBar* toolbar = addToolBar(QStringLiteral("VS1"));
    toolbar->setMovable(false);

    const auto add_action = [this, toolbar](const QString& text, void (MainWindow::*slot)()) {
        QAction* action = toolbar->addAction(text);
        connect(action, &QAction::triggered, this, slot);
    };
    add_action(QStringLiteral("Create Level"), &MainWindow::OnCreateLevel);
    add_action(QStringLiteral("Create Wall"), &MainWindow::OnCreateWall);
    add_action(QStringLiteral("Edit Wall"), &MainWindow::OnEditWallGeometry);
    add_action(QStringLiteral("Level Elevation"), &MainWindow::OnChangeLevelElevation);
    add_action(QStringLiteral("Delete"), &MainWindow::OnDeleteElement);
    toolbar->addSeparator();
    add_action(QStringLiteral("Save"), &MainWindow::OnSave);
    add_action(QStringLiteral("Open"), &MainWindow::OnOpen);
}

void MainWindow::BuildStatusBar() {
    status_label_ = new QLabel(QStringLiteral("Ready"), this);
    statusBar()->addPermanentWidget(status_label_);
}

bool MainWindow::ReportResult(const QString& action, const bim::desktop::WorkflowResult& result) {
    if (result.ok()) {
        status_label_->setText(QStringLiteral("%1: done").arg(action));
        return true;
    }
    QString text = QStringLiteral("%1 failed: %2")
                       .arg(action, QString::fromUtf8(WorkflowCodeText(result.code)));
    if (result.code == bim::desktop::WorkflowCode::CommandRejected) {
        text += QStringLiteral(" (command code %1, validation code %2)")
                    .arg(static_cast<int>(result.command.code))
                    .arg(static_cast<int>(result.command.validation));
    } else if (result.code == bim::desktop::WorkflowCode::SaveFailed ||
               result.code == bim::desktop::WorkflowCode::OpenFailed) {
        text += QStringLiteral(" (lifecycle code %1)").arg(static_cast<int>(result.lifecycle.code));
    }
    status_label_->setText(text);
    return false;
}

bool MainWindow::RefreshScene(bool frame_camera) {
    bim::desktop::SceneMeshes scene = workflow_.RebuildWallMeshes();
    if (!scene.result.ok()) {
        // Never publish a partial scene: the previous scene stays on screen.
        status_label_->setText(QStringLiteral("Scene refresh failed: %1 - previous scene kept")
                                   .arg(QString::fromUtf8(WorkflowCodeText(scene.result.code))));
        return false;
    }
    viewport_window_->SetActiveSceneMeshes(scene.meshes);
    const bool had_content = scene_has_content_;
    scene_has_content_ = !scene.meshes.empty();
    if (scene_has_content_ && (frame_camera || !had_content)) {
        FrameCamera(*viewport_window_, scene.meshes);
    }
    return true;
}

void MainWindow::OnCreateLevel() {
    const std::optional<double> elevation =
        PromptElevation(this, QStringLiteral("Create Level"), 0.0);
    if (!elevation) {
        return;
    }
    const bim::desktop::CreatedElement created = workflow_.CreateLevel(*elevation);
    if (ReportResult(QStringLiteral("Create Level"), created.result)) {
        // A Level alone has no render geometry, but the scene is rebuilt anyway so
        // the displayed scene always equals the committed document.
        if (RefreshScene(/*frame_camera=*/false)) {
            status_label_->setText(QStringLiteral("Level %1 created").arg(IdText(created.id)));
        }
    }
}

void MainWindow::OnCreateWall() {
    const std::optional<bim::model::ElementId> level_id = PickElement(
        this, QStringLiteral("Create Wall"), QStringLiteral("Host Level"), LevelChoices(workflow_));
    if (!level_id) {
        status_label_->setText(QStringLiteral("Create Wall: a Level is required"));
        return;
    }
    WallInput input;
    if (!PromptWallInput(this, QStringLiteral("Create Wall"), input)) {
        return;
    }
    const bim::desktop::CreatedElement created =
        workflow_.CreateStraightWall(*level_id, bim::model::Point2D{input.start_x, input.start_y},
                                     bim::model::Point2D{input.end_x, input.end_y}, input.thickness,
                                     input.height, input.base_offset);
    if (ReportResult(QStringLiteral("Create Wall"), created.result)) {
        if (RefreshScene(/*frame_camera=*/false)) {
            status_label_->setText(QStringLiteral("Wall %1 created").arg(IdText(created.id)));
        }
    }
}

void MainWindow::OnEditWallGeometry() {
    const std::optional<bim::model::ElementId> wall_id = PickElement(
        this, QStringLiteral("Edit Wall"), QStringLiteral("Wall"), WallChoices(workflow_));
    if (!wall_id) {
        return;
    }
    const std::optional<bim::model::StraightWall> wall = workflow_.FindWall(*wall_id);
    if (!wall) {
        status_label_->setText(QStringLiteral("Edit Wall failed: the wall no longer exists"));
        return;
    }
    WallInput input{wall->start.x,   wall->start.y, wall->end.x,      wall->end.y,
                    wall->thickness, wall->height,  wall->base_offset};
    if (!PromptWallInput(this, QStringLiteral("Edit Wall Geometry"), input)) {
        return;
    }
    const bim::desktop::WorkflowResult result =
        workflow_.ChangeWallGeometry(*wall_id, bim::model::Point2D{input.start_x, input.start_y},
                                     bim::model::Point2D{input.end_x, input.end_y}, input.thickness,
                                     input.base_offset, input.height);
    if (ReportResult(QStringLiteral("Edit Wall"), result)) {
        RefreshScene(/*frame_camera=*/false);
    }
}

void MainWindow::OnChangeLevelElevation() {
    const std::optional<bim::model::ElementId> level_id = PickElement(
        this, QStringLiteral("Level Elevation"), QStringLiteral("Level"), LevelChoices(workflow_));
    if (!level_id) {
        return;
    }
    const std::optional<bim::model::Level> level = workflow_.FindLevel(*level_id);
    if (!level) {
        status_label_->setText(
            QStringLiteral("Level Elevation failed: the Level no longer exists"));
        return;
    }
    const std::optional<double> elevation =
        PromptElevation(this, QStringLiteral("Level Elevation"), level->elevation);
    if (!elevation) {
        return;
    }
    const bim::desktop::WorkflowResult result =
        workflow_.ChangeLevelElevation(*level_id, *elevation);
    if (ReportResult(QStringLiteral("Level Elevation"), result)) {
        RefreshScene(/*frame_camera=*/false);
    }
}

void MainWindow::OnDeleteElement() {
    std::vector<Choice> choices = WallChoices(workflow_);
    for (Choice& level : LevelChoices(workflow_)) {
        choices.push_back(std::move(level));
    }
    const std::optional<bim::model::ElementId> id =
        PickElement(this, QStringLiteral("Delete"), QStringLiteral("Element"), choices);
    if (!id) {
        return;
    }
    // A Level that still hosts walls is refused by the delete command.
    const bim::desktop::WorkflowResult result = workflow_.RemoveElement(*id);
    if (ReportResult(QStringLiteral("Delete"), result)) {
        RefreshScene(/*frame_camera=*/false);
    }
}

void MainWindow::OnSave() {
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save Document"));
    if (path.isEmpty()) {
        return;
    }
    ReportResult(QStringLiteral("Save"), workflow_.Save(ToPath(path)));
}

void MainWindow::OnOpen() {
    const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Open Document"));
    if (path.isEmpty()) {
        return;
    }
    // A failed Open leaves the live document (and the displayed scene) unchanged.
    if (ReportResult(QStringLiteral("Open"), workflow_.Open(ToPath(path)))) {
        RefreshScene(/*frame_camera=*/true);
    }
}
