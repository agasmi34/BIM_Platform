#pragma once

#include <QtWidgets/QMainWindow>
#include <memory>

#include "bim/desktop/vs1_workflow.hpp"

class ViewportWindow;
class QLabel;
class QString;

// src/desktop/src/main_window.hpp - top-level Qt Widgets window of the desktop
// executable (P0-T003 spike shell; Implementation Brief BIM-TASK-P0-T003-CLAUDE
// v1.0, sections 11, 14). Qt Widgets only (Phase A fact). Owns a
// ViewportWindow as its central widget.
//
// P1-T006 (Architecture Gate BIM-AG-P1-T006): the normal interactive mode is the
// minimum Vertical Slice 1 authoring workflow. The window owns one
// bim::desktop::Vs1Workflow (the Qt-free workflow that owns the live Document)
// and offers the VS1 actions: create Level, create StraightWall, edit wall
// geometry, change Level elevation, delete an element, Save and Open. Every
// mutation goes through the workflow, which uses bim::commands only; this class
// never touches the Document, the geometry kernel or the model layer directly.
// After a successful operation the window asks the workflow for the complete
// wall mesh set and hands that set to ViewportWindow::SetActiveSceneMeshes; if
// the rebuild fails the previous scene stays on screen and the failure is
// reported in the status bar. The P0-T003 evidence modes (main.cpp /
// evidence_mode.cpp) construct this window unchanged and drive the embedded
// ViewportWindow themselves.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // RD1.6-03 (cppcoreguidelines-special-member-functions): a QObject-derived
    // UI object, not a value type - the existing non-copyable/non-movable
    // ownership contract (Qt parent/child ownership of viewport_window_ and
    // the child widgets) is made explicit here rather than implied by the
    // user-declared destructor. Deleted, never custom-implemented; destructor
    // behavior and ownership semantics are unchanged.
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    // AA Source Review Round 5 finding M16: --evidence-mode-live (main.cpp)
    // now constructs the actual production MainWindow as its top-level
    // window (previously it constructed a bare ViewportWindow directly as
    // its own top-level window, which is not the real application's
    // top-level/child-widget shell relationship and could not exercise
    // ViewportWindow's real MainWindow-embedded minimize/restore lifecycle
    // path - see viewport_window.hpp's own M16 comment). This accessor is a
    // thin forward to the existing private viewport_window_ member - the
    // exact same ViewportWindow instance MainWindow's constructor already
    // creates and embeds as its central widget - so evidence mode observes
    // and drives the real child-under-top-level relationship rather than a
    // second one built just for evidence.
    [[nodiscard]] ViewportWindow* EvidenceViewportWindow() const noexcept {
        return viewport_window_;
    }

private:
    void BuildMenus();
    void BuildStatusBar();

    // VS1 actions (toolbar). Each one prompts for its input, runs one workflow
    // operation and then reports the outcome and refreshes the scene.
    void OnCreateLevel();
    void OnCreateWall();
    void OnEditWallGeometry();
    void OnChangeLevelElevation();
    void OnDeleteElement();
    void OnSave();
    void OnOpen();

    // Reports `result` in the status bar. Returns result.ok().
    bool ReportResult(const QString& action, const bim::desktop::WorkflowResult& result);

    // Rebuilds the complete wall mesh set and, only when that succeeded, replaces
    // the viewport scene with it. On failure the previous scene is preserved.
    // `frame_camera` re-aims the camera at the new scene.
    bool RefreshScene(bool frame_camera);

    bim::desktop::Vs1Workflow workflow_;
    bool scene_has_content_ = false;
    ViewportWindow* viewport_window_ = nullptr;
    QLabel* status_label_ = nullptr;
};
