#pragma once

#include <QtWidgets/QMainWindow>
#include <memory>

#include "spike_scene.hpp"

class ViewportWindow;
class QComboBox;
class QLabel;

// src/desktop/src/main_window.hpp - top-level Qt Widgets window for the
// P0-T003 desktop spike (Implementation Brief BIM-TASK-P0-T003-CLAUDE
// v1.0, sections 11, 14). Qt Widgets only (Phase A fact). Owns a
// ViewportWindow as its central widget and a scene selector that switches
// between the five spike scenes (spike_scene.hpp).
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
    void OnSceneSelected(int index);

    ViewportWindow* viewport_window_ = nullptr;
    QComboBox* scene_selector_ = nullptr;
    QLabel* status_label_ = nullptr;
};
