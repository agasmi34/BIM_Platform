#pragma once

#include <QtCore/QPointF>
#include <QtWidgets/QWidget>
#include <cstddef>
#include <memory>
#include <vector>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/lifecycle.hpp"
#include "bim/viewport_bgfx/renderer.hpp"
#include "viewport_bridge.hpp"

class QTimer;
class QScreen;
class QWindow;
class QMouseEvent;
class QWheelEvent;

namespace bim::desktop {
class ViewportSurface; // private QWindow-derived surface - defined only in
                       // viewport_window.cpp (Brief section 3: "embeds a
                       // private QWindow-derived native viewport surface");
                       // never a public type of this header.
} // namespace bim::desktop

// src/desktop/src/viewport_window.hpp - the Qt Widgets widget that owns
// the bgfx-backed renderer's native surface (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0, sections 9-11). Qt Widgets only (Phase A
// fact: Qt Quick forbidden, qtbase Widgets-only) - never a QQuickWindow/
// QQuickItem.
//
// AA Source Review Round 1 finding M01: the prior authoring pass rendered
// directly onto this QWidget's own native HWND via Qt::WA_PaintOnScreen +
// paintEngine() returning nullptr + winId(). That is a legacy/fragile
// pattern and not what the Brief requires. This revision instead embeds a
// private QWindow-derived surface (bim::desktop::ViewportSurface, defined
// in viewport_window.cpp) via the normal Qt window-container mechanism
// (QWidget::createWindowContainer, Brief section 11: "Viewport native
// surface: private QWindow-derived class embedded into the Widgets shell
// through the normal Qt window-container mechanism."). bgfx renders
// directly to that QWindow's native handle; this outer QWidget itself no
// longer disables Qt's paint pipeline or overrides paintEngine() - Qt owns
// painting for the surrounding widget chrome as normal, and the embedded
// QWindow is a real native child window Qt composites in place.
//
// Surface lifecycle + DPR handling (Brief section 11's exact bridge event
// list - surface creation; SurfaceAboutToBeDestroyed; exposure; resize;
// minimize/restore; devicePixelRatio changes; screen changes; application
// shutdown) is driven primarily from the embedded QWindow's own events
// (ViewportSurface's exposeEvent/resizeEvent/QPlatformSurfaceEvent/
// visibilityChanged/screenChanged - see viewport_window.cpp), not from
// this QWidget's showEvent/hideEvent/resizeEvent alone; those QWidget
// -level overrides are kept only as an outer-container fallback.
class ViewportWindow : public QWidget {
    Q_OBJECT

public:
    explicit ViewportWindow(QWidget* parent = nullptr);
    ~ViewportWindow() override;

    // RD1.6-03 (cppcoreguidelines-special-member-functions): a QObject-derived
    // UI object, not a value type - the existing non-copyable/non-movable
    // ownership contract (Qt parent/child ownership of the embedded surface
    // and container, plus the owned Renderer) is made explicit here rather
    // than implied by the user-declared destructor. Deleted, never
    // custom-implemented; destructor behavior and ownership semantics are
    // unchanged.
    ViewportWindow(const ViewportWindow&) = delete;
    ViewportWindow& operator=(const ViewportWindow&) = delete;
    ViewportWindow(ViewportWindow&&) = delete;
    ViewportWindow& operator=(ViewportWindow&&) = delete;

    // Replaces the mesh set the renderer should be showing (used when the
    // MainWindow's scene selector switches spike scenes - see
    // main_window.cpp). The requested meshes are remembered as
    // `pending_scene_meshes_` regardless of whether the renderer has a
    // live surface yet or right now: if the renderer is not yet
    // Initialize()'d (surface not realized, or between a
    // SurfaceAboutToBeDestroyed teardown and the next re-init),
    // ApplyPendingSceneMeshesIfReady() is retried automatically the moment
    // the renderer next becomes live, rather than the request being
    // silently dropped (AA Source Review Round 1 finding M04).
    void SetActiveSceneMeshes(const std::vector<bim::viewport::RenderMeshData>& meshes);

    // Repositions the camera's look-at (eye/target/world-up), used when the
    // MainWindow's scene selector switches spike scenes so each scene's
    // suggested viewpoint (see spike_scene.hpp's SpikeScene::cameraEye/
    // cameraTarget/cameraWorldUp) actually gets applied (AA Source Review
    // Round 2 finding M10: V05's large-coordinate cluster is tens of
    // thousands of local-frame units from the origin - the single
    // near-origin default camera this widget was constructed with can
    // never actually observe it). A rejected eye/target/up combination
    // (Camera::SetLookAt's own finite/non-degenerate validation) leaves the
    // camera unchanged and is logged, never silently ignored.
    void SetSceneCamera(const bim::viewport::Vector3& eye, const bim::viewport::Vector3& target,
                        const bim::viewport::Vector3& worldUp);

    [[nodiscard]] const bim::viewport::Camera& GetCamera() const noexcept { return camera_; }

    // ---- Evidence-mode-only accessors (AA Source Review Round 3 finding
    // M13) --------------------------------------------------------------
    // Live evidence mode (--evidence-mode-live, see evidence_mode.cpp) must
    // exercise this exact production ViewportWindow/ViewportSurface
    // lifecycle path - not a second, separately-constructed QWindow plus a
    // manually-driven Renderer/ViewportLifecycle architecture (the defect
    // this finding requires removing). Every accessor below is a thin
    // forward to the existing private renderer_/lifecycle_/camera_/surface_
    // state and the existing private EnsureRendererInitialized()/
    // TeardownRenderer() methods further down this class - evidence_mode.cpp
    // never duplicates this logic itself.

    // The embedded native surface (Brief section 11) this widget renders
    // into - non-owning, realized by the constructor. main.cpp uses this
    // (via windowHandle() on this widget itself for the top-level
    // minimize/restore/screen-move operations, and this accessor for
    // surface-level DPR/size observation) rather than constructing a
    // second QWindow of its own.
    [[nodiscard]] QWindow* EvidenceNativeSurface() const noexcept;

    [[nodiscard]] bool EvidenceIsRendererReady() const noexcept {
        return renderer_.IsInitialized();
    }

    [[nodiscard]] bim::viewport_bgfx::RendererBackendInfo EvidenceBackendInfo() const {
        return renderer_.BackendInfo();
    }

    [[nodiscard]] bim::viewport::ViewportState EvidenceLifecycleState() const noexcept {
        return lifecycle_.State();
    }

    // AA Source Review Round 4 finding M13 (final closure): these two
    // replace the removed EvidenceForceSurfaceRecreation(), which called
    // TeardownRenderer()/EnsureRendererInitialized() directly - proving
    // only that those two private methods behave as expected when called,
    // not that the real Qt-driven surface-loss/re-exposure path actually
    // exercises them. These instead call the real QWindow::destroy()/
    // create() on the exact production surface_, so Qt itself delivers a
    // genuine QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed (destroy())
    // and a genuine exposeEvent (create() + re-show) to ViewportSurface -
    // the same event() override and onSurfaceAboutToBeDestroyed/
    // onExposedOrHidden callbacks the interactive session's real surface
    // loss/recreation goes through (see viewport_window.cpp). Neither
    // method calls TeardownRenderer()/EnsureRendererInitialized() itself;
    // both are only ever reached as a side effect of the real Qt event.
    // evidence_mode.cpp is expected to pump the Qt event loop and observe
    // EvidenceLifecycleState()/EvidenceIsRendererReady() between and after
    // these two calls - see its own comments.

    // Destroys surface_'s native platform window. UNVERIFIED against the
    // real Windows/Qt platform plugin (this session has no execution
    // channel - see docs/evidence/P0-T003/CLAUDE_HANDOVER.md): destroy()
    // on a window embedded via QWidget::createWindowContainer() is
    // documented to tear down platform resources and is expected to
    // deliver SurfaceAboutToBeDestroyed, but its exact interaction with
    // the container's own ownership of the child window has not been
    // confirmed against a real build.
    void EvidenceDestroyNativeSurface();

    // Re-creates surface_'s native platform window (against the same
    // logical parent() the container established - create() does not
    // clear that relationship) and re-exposes it so Qt drives
    // EnsureRendererInitialized() via the real onExposedOrHidden callback.
    //
    // AA Source Review Round 5 finding N03: surface_ is owned/managed by
    // QWidget::createWindowContainer() (surface_container_ below) - the
    // prior version of this method called surface_->setVisible(true)
    // directly after create(), bypassing the container's own
    // visibility/exposure bookkeeping for its embedded child window rather
    // than going through the container-controlled Qt Widgets path a real
    // interactive re-show would use. This now re-exposes surface_ by
    // toggling surface_container_'s own visibility (hide()+show()) - the
    // same container-owned mechanism Qt itself uses to (re)embed and expose
    // the wrapped native window - instead of driving surface_'s own
    // setVisible()/requestActivate() directly. Still causes a genuine
    // native-window creation and a genuine exposeEvent delivered to
    // ViewportSurface's own exposeEvent() override; only the mechanism used
    // to trigger that exposure changed. See viewport_window.cpp.
    void EvidenceRecreateNativeSurface();

    struct EvidenceSceneResult {
        bool create_mesh_passed = false;
        bool render_frame_passed = false;
        bool destroy_mesh_passed = false;
        std::size_t vertex_count = 0;
        std::size_t index_count = 0;
    };

    // Runs one evidence scene (CreateMesh for every mesh in `meshes`, one
    // RenderFrame against this ViewportWindow's own renderer_/camera_,
    // DestroyMesh for every handle) through the exact renderer_ instance
    // this widget renders through on its own render_timer_ tick - never a
    // second Renderer instance. The caller (evidence_mode.cpp) is expected
    // to call SetSceneCamera() with the scene's own suggested eye/target
    // (AA Source Review Round 3 finding M14) immediately before this, so
    // camera_ reflects that scene's viewpoint during the RenderFrame() this
    // performs.
    EvidenceSceneResult EvidenceRunScene(const std::vector<bim::viewport::RenderMeshData>& meshes);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    // AA Source Review Round 5 finding M16: Round 4's changeEvent() override
    // (removed - superseded by eventFilter() below) reacted to THIS
    // QWidget's own windowState(), which is only meaningful when this
    // widget happens to be top-level itself. The real production shell
    // (MainWindow, see main_window.hpp/.cpp) constructs ViewportWindow as
    // its CENTRAL CHILD widget, not as a top-level window - a child
    // widget's own windowState()/QEvent::WindowStateChange do not track its
    // top-level ancestor's minimize/restore state, so Round 4's fix did not
    // actually prove the real MainWindow-embedded minimize/restore path
    // (diagnosed this round, not merely an evidence-authoring gap - see
    // CLAUDE_HANDOVER.md section 11). eventFilter() below is installed
    // directly on window() - the actual top-level QWidget, whatever it is
    // (MainWindow in production and in --evidence-mode-live as of this
    // round; this widget itself in the degenerate case where it has no
    // parent) - and reacts to THAT widget's QEvent::WindowStateChange,
    // which is where Qt::WindowMinimized is actually meaningful. This is a
    // single, unambiguous top-level-lifecycle bridge (mirroring M08's own
    // "single, unambiguous sink" precedent for input, just for window state
    // instead) - not a second one alongside the constructor's existing
    // surface_->visibilityChanged lambda, which stays in place unchanged
    // and remains a harmless, independently-gated no-op on any platform
    // where it never fires for the reasons already diagnosed in Round 4.
    // Explicitly NOT a revival of the M08 input-eventFilter assumption this
    // finding's own instruction warns against: this filter only inspects
    // QEvent::WindowStateChange on the top-level widget, never touches
    // input events, and never re-installs itself on surface_container_ or
    // surface_.
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // Installs (or re-installs, if window() has changed since the last
    // call) the eventFilter() bridge above onto the current window() - the
    // actual top-level QWidget this ViewportWindow is embedded under right
    // now. Idempotent: a no-op if window() is unchanged. Called from the
    // constructor (fallback: window() resolves to this widget itself before
    // any parent is attached) and from showEvent() (by the time a widget is
    // actually shown, QMainWindow::setCentralWidget()-style reparenting -
    // see main_window.cpp - has already completed, so window() reliably
    // resolves to the real top-level widget at that point).
    void InstallTopLevelWindowStateBridge();

    void EnsureRendererInitialized();
    void TeardownRenderer();
    void RenderOnce();
    void ClearActiveMeshHandles();
    void ApplyPendingSceneMeshesIfReady();

    // AA Source Review Round 2 finding M08: mouse/wheel input is now routed
    // authoritatively from the embedded ViewportSurface's own QWindow input
    // -event overrides (mousePressEvent/mouseMoveEvent/mouseReleaseEvent/
    // wheelEvent - see viewport_window.cpp), not from an eventFilter
    // installed on surface_container_. A real native child window created
    // via QWidget::createWindowContainer() receives its own input events
    // directly through Qt's normal QWindow event-delivery path; the prior
    // authoring pass's eventFilter-on-the-container approach rested on an
    // explicitly-disclosed, never-confirmed assumption (see this file's
    // former header comment) about whether the container widget would also
    // see those events. These four methods are the single, unambiguous
    // sink bridge_ and the click/pick-ray heuristic below now go through.
    void HandleMousePress(QMouseEvent& event);
    void HandleMouseMove(QMouseEvent& event);
    void HandleMouseRelease(QMouseEvent& event);
    void HandleWheel(QWheelEvent& event);

    // Recomputes the surface's current physical (device-pixel) size from
    // its logical size and devicePixelRatio(), and either brings the
    // renderer up (first time) or issues an ordinary Renderer::Resize +
    // Camera::SetAspectRatio (already live) - shared by the surface's
    // resize, devicePixelRatio-change, and screen-change callbacks (Brief
    // section 11 lists all three as distinct bridge responsibilities, but
    // physically they all reduce to "the physical viewport size may have
    // changed").
    void HandlePhysicalSizeOrDprChange();

    bim::viewport::Camera camera_;
    bim::viewport::ViewportLifecycle lifecycle_;
    bim::viewport_bgfx::Renderer renderer_;
    bim::desktop::ViewportBridge bridge_;
    std::unique_ptr<QTimer> render_timer_;

    // Owned via the normal Qt parent/child widget-tree mechanism once
    // reparented by QWidget::createWindowContainer() (constructor), not by
    // this class directly - same ownership convention as every other Qt
    // child widget/window in this codebase (e.g. MainWindow's
    // scene_selector_/status_label_).
    bim::desktop::ViewportSurface* surface_ = nullptr;
    QWidget* surface_container_ = nullptr;

    // AA Source Review Round 5 finding M16: the top-level widget the
    // eventFilter() above is currently installed on - window() as of the
    // last InstallTopLevelWindowStateBridge() call. Tracked here so that
    // method can remove the filter from the old top-level and re-install on
    // the new one if window() ever changes, and so the destructor can
    // remove the filter cleanly. Non-owning - may equal `this` (degenerate
    // top-level case) or the enclosing MainWindow (production/
    // evidence-mode-live case, see main_window.cpp).
    QWidget* observed_top_level_window_ = nullptr;

    std::vector<bim::viewport::RenderMeshData> pending_scene_meshes_;
    std::vector<bim::viewport_bgfx::RenderMeshHandle> active_mesh_handles_;

    // AA Source Review Round 1 MINOR fix: ViewportBridge::PickRay (Brief
    // section 7's ScreenToWorldRay, reached through the bridge) was
    // defined but never actually invoked anywhere in the interactive
    // session - only evidence_mode.cpp's single fixed-pixel ray check
    // exercised it. HandleMouseRelease (AA Source Review Round 2 finding
    // M08 renamed/relocated this from the removed eventFilter) now calls
    // it on a left-button click (press+release with negligible movement,
    // i.e. not a drag) so the picking surface this bridge method exists
    // for is actually live; see viewport_window.cpp. These two fields are
    // that click-vs-drag heuristic's only state.
    QPointF press_pos_;
    bool press_was_left_button_ = false;
};
