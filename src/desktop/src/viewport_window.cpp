#include "viewport_window.hpp"

#include <functional>

#include "bim/viewport/ray.hpp"

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QExposeEvent>
#include <QtGui/QHideEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPlatformSurfaceEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QVBoxLayout>

// UNVERIFIED - authored by Claude, the Implementation Engineer, against
// the Qt 6 QWindow/QSurface/QPlatformSurfaceEvent API as documented, but
// never compiled or run: this session has no Windows execution channel.
// The Operator must build and run this before it can be treated as
// verified - see docs/evidence/P0-T003/CLAUDE_HANDOVER.md. One specific
// Operator-verification item remains called out inline below: whether
// QEvent::DevicePixelRatioChange is delivered to a plain QWindow (not a
// QWidget) on the installed qtbase 6.11.1#1 - screenChanged is
// independently wired as a second trigger for the same recomputation, so
// a DPR change that only fires through the QWidget path is still caught
// when it also changes screens.
//
// AA Source Review Round 2 finding M08: the prior version of this file
// routed mouse/wheel input through an eventFilter installed on
// surface_container_ (the QWidget returned by createWindowContainer()),
// explicitly disclosed above as resting on an unconfirmed assumption about
// whether that container widget actually sees input meant for the native
// child window it wraps. This revision removes that assumption entirely:
// input is now sourced authoritatively from ViewportSurface's own QWindow
// virtual event overrides (mousePressEvent/mouseMoveEvent/
// mouseReleaseEvent/wheelEvent, below) - Qt's documented, guaranteed
// delivery path for a real native window, the same path exposeEvent/
// resizeEvent already used for surface lifecycle in the prior revision.
// eventFilter is no longer installed or overridden by this class.

namespace {
constexpr int kRenderTimerIntervalMs = 16; // ~60 Hz; not vsync-locked itself,
                                           // bgfx's own reset carries BGFX_RESET_VSYNC
} // namespace

namespace bim::desktop {

// Private QWindow-derived native viewport surface (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0 section 11: "Viewport native surface:
// private QWindow-derived class embedded into the Widgets shell through
// the normal Qt window-container mechanism."; AA Source Review Round 1
// finding M01). Defined only in this translation unit - viewport_window.hpp
// forward-declares the type and never exposes it, its winId(), or any
// bgfx PlatformData as a public type. ViewportWindow talks to it only
// through the callback members below plus QWindow's own inherited
// screenChanged/visibilityChanged signals.
class ViewportSurface : public QWindow {
public:
    explicit ViewportSurface(QWindow* parent = nullptr) : QWindow(parent) {
        // bgfx never goes through Qt's own graphics context on this
        // surface - Qt owns only the native window/HWND, bgfx's D3D11
        // backend is handed the raw native handle directly (see
        // ViewportWindow::EnsureRendererInitialized). RasterSurface avoids
        // Qt provisioning a GL context this spike never uses.
        setSurfaceType(QSurface::RasterSurface);
    }

    std::function<void()> onExposedOrHidden;
    std::function<void()> onSurfaceAboutToBeDestroyed;
    std::function<void(QSize)> onResized;
    std::function<void(qreal)> onDevicePixelRatioChanged;

    // AA Source Review Round 2 finding M08: authoritative input routing -
    // see the file-level comment above. QWindow::mousePressEvent/
    // mouseMoveEvent/mouseReleaseEvent/wheelEvent are Qt's own virtual
    // dispatch points for input targeting this exact native window; a real
    // native child window (which this is, once embedded via
    // createWindowContainer()) receives these calls directly from the
    // platform integration, with no dependency on how - or whether - the
    // wrapping QWidget container forwards anything.
    std::function<void(QMouseEvent&)> onMousePress;
    std::function<void(QMouseEvent&)> onMouseMove;
    std::function<void(QMouseEvent&)> onMouseRelease;
    std::function<void(QWheelEvent&)> onWheel;

protected:
    void exposeEvent(QExposeEvent* event) override {
        QWindow::exposeEvent(event);
        if (onExposedOrHidden) {
            onExposedOrHidden();
        }
    }

    void resizeEvent(QResizeEvent* event) override {
        QWindow::resizeEvent(event);
        if (onResized) {
            onResized(event->size());
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (onMousePress) {
            onMousePress(*event);
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (onMouseMove) {
            onMouseMove(*event);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (onMouseRelease) {
            onMouseRelease(*event);
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        if (onWheel) {
            onWheel(*event);
        }
    }

    bool event(QEvent* qt_event) override {
        if (qt_event->type() == QEvent::PlatformSurface) {
            auto* surface_event = static_cast<QPlatformSurfaceEvent*>(qt_event);
            if (surface_event->surfaceEventType() ==
                QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
                if (onSurfaceAboutToBeDestroyed) {
                    onSurfaceAboutToBeDestroyed();
                }
            }
        } else if (qt_event->type() == QEvent::DevicePixelRatioChange) {
            // See the file-level Operator-verification note above.
            if (onDevicePixelRatioChanged) {
                onDevicePixelRatioChanged(devicePixelRatio());
            }
        }
        return QWindow::event(qt_event);
    }
};

} // namespace bim::desktop

ViewportWindow::ViewportWindow(QWidget* parent) : QWidget(parent), bridge_(camera_) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(320, 240);

    surface_ = new bim::desktop::ViewportSurface();
    surface_->onExposedOrHidden = [this]() { EnsureRendererInitialized(); };
    surface_->onSurfaceAboutToBeDestroyed = [this]() { TeardownRenderer(); };
    surface_->onResized = [this](QSize) { HandlePhysicalSizeOrDprChange(); };
    surface_->onDevicePixelRatioChanged = [this](qreal) { HandlePhysicalSizeOrDprChange(); };
    surface_->onMousePress = [this](QMouseEvent& event) { HandleMousePress(event); };
    surface_->onMouseMove = [this](QMouseEvent& event) { HandleMouseMove(event); };
    surface_->onMouseRelease = [this](QMouseEvent& event) { HandleMouseRelease(event); };
    surface_->onWheel = [this](QWheelEvent& event) { HandleWheel(event); };
    connect(surface_, &QWindow::screenChanged, this,
            [this](QScreen*) { HandlePhysicalSizeOrDprChange(); });
    connect(surface_, &QWindow::visibilityChanged, this, [this](QWindow::Visibility visibility) {
        if (!renderer_.IsInitialized()) {
            return;
        }
        if (visibility == QWindow::Minimized) {
            if (lifecycle_.State() == bim::viewport::ViewportState::Ready) {
                render_timer_->stop();
                static_cast<void>(lifecycle_.OnSuspend());
            }
        } else if (lifecycle_.State() == bim::viewport::ViewportState::Suspended) {
            static_cast<void>(lifecycle_.OnResume());
            render_timer_->start(kRenderTimerIntervalMs);
        }
    });

    // QWidget::createWindowContainer reparents `surface_` and takes over
    // its lifetime as part of the resulting container widget's ownership
    // tree - surface_ is a non-owning observer pointer from this point on,
    // same convention as every other Qt child widget/window in this
    // codebase.
    surface_container_ = QWidget::createWindowContainer(surface_, this);
    surface_container_->setFocusPolicy(Qt::StrongFocus);
    // AA Source Review Round 2 finding M08: no longer installs an
    // eventFilter here - mouse/wheel input is wired directly above, from
    // surface_'s own QWindow virtual event overrides.

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(surface_container_);
    setLayout(layout);

    const bim::viewport::Status look_at =
        camera_.SetLookAt({0.0, -6.0, 3.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 1.0});
    const bim::viewport::Status perspective = camera_.SetPerspective(0.9, 0.01, 10000.0);
    // Both are called with known-good, hand-verified-finite literal
    // arguments; a failure here would indicate this widget's own
    // construction is broken, not a runtime/input-dependent condition, so
    // it is reported via Q_ASSERT_X rather than propagated as a Status
    // this constructor has no way to return.
    if (!look_at || !perspective) {
        Q_ASSERT_X(false, "ViewportWindow", "default camera configuration was rejected");
    }

    render_timer_ = std::make_unique<QTimer>(this);
    connect(render_timer_.get(), &QTimer::timeout, this, [this]() { RenderOnce(); });

    const bim::viewport::Status begin_init = lifecycle_.BeginInitialization();
    Q_ASSERT_X(static_cast<bool>(begin_init), "ViewportWindow",
               "lifecycle BeginInitialization on a fresh ViewportLifecycle cannot fail");

    // AA Source Review Round 5 finding M16: install the top-level-window
    // -state bridge (eventFilter() below) against whatever window() already
    // resolves to at this point - `this` itself, since no parent has taken
    // ownership yet even when `parent` was passed in (createWindowContainer
    // above reparents surface_, not this widget). showEvent() below
    // re-installs against the real top-level widget once this ViewportWindow
    // has actually been reparented under one (e.g. MainWindow::
    // setCentralWidget() - see main_window.cpp) and shown.
    InstallTopLevelWindowStateBridge();
}

ViewportWindow::~ViewportWindow() {
    TeardownRenderer();
    // AA Source Review Round 5 finding M16: explicitly detach the
    // eventFilter() bridge from whatever top-level widget it is currently
    // installed on. Qt would also handle this automatically if that
    // top-level widget is destroyed first (QObject disconnects filters on
    // either side's destruction), but ViewportWindow is typically destroyed
    // BEFORE its own top-level ancestor (as MainWindow's owned central
    // child), so this side must remove itself explicitly rather than rely
    // on that ordering.
    if (observed_top_level_window_ != nullptr) {
        observed_top_level_window_->removeEventFilter(this);
    }
}

void ViewportWindow::ClearActiveMeshHandles() {
    if (!renderer_.IsInitialized()) {
        active_mesh_handles_.clear();
        return;
    }
    for (bim::viewport_bgfx::RenderMeshHandle& handle : active_mesh_handles_) {
        static_cast<void>(renderer_.DestroyMesh(handle));
    }
    active_mesh_handles_.clear();
}

void ViewportWindow::SetActiveSceneMeshes(
    const std::vector<bim::viewport::RenderMeshData>& meshes) {
    // Remembered unconditionally - this is the source of truth for "what
    // the viewport should currently be showing," independent of whether
    // the renderer happens to have a live surface right now (AA Source
    // Review Round 1 finding M04: the prior version dropped this silently
    // whenever the renderer was not yet Ready, including the very first
    // scene selected before the widget was first shown).
    pending_scene_meshes_ = meshes;
    ClearActiveMeshHandles();
    ApplyPendingSceneMeshesIfReady();
}

void ViewportWindow::ApplyPendingSceneMeshesIfReady() {
    if (!renderer_.IsInitialized()) {
        return; // EnsureRendererInitialized() calls this again once the
                // renderer actually comes up - see below.
    }
    for (const bim::viewport::RenderMeshData& mesh : pending_scene_meshes_) {
        const auto result = renderer_.CreateMesh(mesh);
        if (result.IsOk()) {
            active_mesh_handles_.push_back(result.Value());
        }
        // A rejected mesh is simply not added to active_mesh_handles_ -
        // never treated as fatal to the whole scene switch.
    }
}

void ViewportWindow::EnsureRendererInitialized() {
    if (renderer_.IsInitialized()) {
        return;
    }
    if (!surface_->isExposed()) {
        return; // Brief section 11 "exposure": native handles are
                // acquired only after the native surface is realized AND
                // exposed.
    }
    const QSize logical_size = surface_->size();
    if (logical_size.width() <= 0 || logical_size.height() <= 0) {
        return;
    }

    bim::viewport_bgfx::RendererCreateInfo create_info;
    create_info.headless = false;
    // RD1.6-05 (performance-no-int-to-ptr): this is the required Qt/Win32
    // native-handle boundary - Qt exposes the native window handle as an
    // integer WId, and the neutral viewport contract carries it as an opaque
    // void* for bgfx; the int-to-pointer conversion is inherent to that
    // interop and is kept exactly as-is (no redesign, no new abstraction).
    // NEXTLINE form only because a trailing comment would push the statement
    // past the column limit.
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    create_info.nativeWindowHandle = reinterpret_cast<void*>(surface_->winId());
    const qreal dpr = surface_->devicePixelRatio();
    create_info.widthPx = static_cast<std::uint32_t>(logical_size.width() * dpr);
    create_info.heightPx = static_cast<std::uint32_t>(logical_size.height() * dpr);

    const bim::viewport::Status init_status = renderer_.Initialize(create_info);
    if (!init_status) {
        // RendererInitializationFailed/UnsupportedBackend: left in
        // SurfaceUnavailable (lifecycle_ is not advanced past
        // BeginInitialization). MainWindow's status bar (see
        // main_window.cpp) surfaces this - ViewportWindow itself has no UI
        // chrome of its own.
        return;
    }

    static_cast<void>(lifecycle_.OnSurfaceAvailable());
    const bim::viewport::Status aspect_status = camera_.SetAspectRatio(
        static_cast<double>(create_info.widthPx) / static_cast<double>(create_info.heightPx));
    static_cast<void>(aspect_status);

    // Re-apply whatever scene was most recently requested (including one
    // requested before this renderer ever came up, or one requested while
    // the surface was mid-recreation) - AA Source Review Round 1 M04.
    ApplyPendingSceneMeshesIfReady();

    render_timer_->start(kRenderTimerIntervalMs);
}

void ViewportWindow::TeardownRenderer() {
    render_timer_->stop();
    ClearActiveMeshHandles();
    if (renderer_.IsInitialized()) {
        static_cast<void>(renderer_.Shutdown());
    }
    if (lifecycle_.State() == bim::viewport::ViewportState::Ready ||
        lifecycle_.State() == bim::viewport::ViewportState::Suspended) {
        static_cast<void>(lifecycle_.OnSurfaceLost());
    }
}

void ViewportWindow::RenderOnce() {
    if (!renderer_.IsInitialized()) {
        return;
    }
    static_cast<void>(
        renderer_.RenderFrame(camera_, active_mesh_handles_.data(), active_mesh_handles_.size()));
}

void ViewportWindow::HandlePhysicalSizeOrDprChange() {
    if (!renderer_.IsInitialized()) {
        EnsureRendererInitialized();
        return;
    }
    const QSize logical_size = surface_->size();
    const qreal dpr = surface_->devicePixelRatio();
    const auto width_px = static_cast<std::uint32_t>(logical_size.width() * dpr);
    const auto height_px = static_cast<std::uint32_t>(logical_size.height() * dpr);
    if (width_px == 0 || height_px == 0) {
        return;
    }
    static_cast<void>(renderer_.Resize(width_px, height_px));
    static_cast<void>(
        camera_.SetAspectRatio(static_cast<double>(width_px) / static_cast<double>(height_px)));
}

void ViewportWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // AA Source Review Round 5 finding M16: re-bind the top-level-window
    // -state bridge here too (not only in the constructor). By the time
    // this widget is actually shown, any QMainWindow::setCentralWidget()
    // -style reparenting under a real top-level widget (MainWindow - see
    // main_window.cpp) has already completed, so window() now reliably
    // resolves to that real top-level widget rather than to this widget
    // itself (the constructor-time fallback). Idempotent - a no-op on the
    // common case where window() has not changed since the last call.
    InstallTopLevelWindowStateBridge();
    EnsureRendererInitialized();
}

void ViewportWindow::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // Primary minimize/restore handling is the surface's visibilityChanged
    // callback wired in the constructor (Brief section 11's explicit
    // "minimize/restore" bridge responsibility); this QWidget-level
    // override is kept only as an outer-container fallback for a future
    // embedding that hides ViewportWindow without minimizing the top
    // -level window.
    if (renderer_.IsInitialized() && lifecycle_.State() == bim::viewport::ViewportState::Ready) {
        render_timer_->stop();
        static_cast<void>(lifecycle_.OnSuspend());
    }
}

void ViewportWindow::InstallTopLevelWindowStateBridge() {
    // AA Source Review Round 5 finding M16: window() resolves to the actual
    // top-level widget in this widget's current parent chain (itself, if it
    // has none). This is deliberately re-resolved and re-compared on every
    // call rather than cached-and-assumed-stable, since the real production
    // shell reparents ViewportWindow after construction
    // (QMainWindow::setCentralWidget() - see main_window.cpp).
    QWidget* const top = window();
    if (top == observed_top_level_window_) {
        return; // Already installed on the current top-level widget.
    }
    if (observed_top_level_window_ != nullptr) {
        observed_top_level_window_->removeEventFilter(this);
    }
    observed_top_level_window_ = top;
    if (observed_top_level_window_ != nullptr) {
        observed_top_level_window_->installEventFilter(this);
    }
}

bool ViewportWindow::eventFilter(QObject* watched, QEvent* event) {
    // AA Source Review Round 5 finding M16 - full rationale is on this
    // override's declaration in viewport_window.hpp. This replaces Round
    // 4's changeEvent() override, which reacted to THIS widget's own
    // windowState() - only meaningful when this widget happens to be
    // top-level itself, which it is NOT in the real production shell
    // (MainWindow embeds it as a central child - see main_window.cpp).
    // `watched` is whatever window() resolved to as of the last
    // InstallTopLevelWindowStateBridge() call (constructor/showEvent()
    // above) - the actual top-level widget, so its QEvent::WindowStateChange
    // and windowState() are the real minimize/restore signal, regardless of
    // whether that top-level widget is MainWindow or (degenerate case) this
    // ViewportWindow itself.
    if (watched == observed_top_level_window_ && event != nullptr &&
        event->type() == QEvent::WindowStateChange) {
        if (renderer_.IsInitialized()) {
            QWidget* const top_level = qobject_cast<QWidget*>(watched);
            const bool is_minimized =
                top_level != nullptr && (top_level->windowState() & Qt::WindowMinimized) != 0;
            if (is_minimized) {
                if (lifecycle_.State() == bim::viewport::ViewportState::Ready) {
                    render_timer_->stop();
                    static_cast<void>(lifecycle_.OnSuspend());
                }
            } else if (lifecycle_.State() == bim::viewport::ViewportState::Suspended) {
                static_cast<void>(lifecycle_.OnResume());
                render_timer_->start(kRenderTimerIntervalMs);
            }
        }
        // Deliberately never consumes the event (returns false via the base
        // class below) - this filter only observes top-level window-state
        // transitions to drive lifecycle_/render_timer_; it must never
        // prevent the watched top-level widget's own normal event handling.
    }
    return QWidget::eventFilter(watched, event);
}

void ViewportWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // The embedded QWindow's own resizeEvent (ViewportSurface, above) is
    // the primary trigger for HandlePhysicalSizeOrDprChange(); this
    // QWidget-level override is kept as a fallback for the outer container
    // widget's own layout changes.
    HandlePhysicalSizeOrDprChange();
}

void ViewportWindow::closeEvent(QCloseEvent* event) {
    TeardownRenderer();
    if (lifecycle_.State() != bim::viewport::ViewportState::Destroyed &&
        lifecycle_.State() != bim::viewport::ViewportState::ShuttingDown) {
        static_cast<void>(lifecycle_.BeginShutdown());
    }
    if (lifecycle_.State() == bim::viewport::ViewportState::ShuttingDown) {
        static_cast<void>(lifecycle_.CompleteShutdown());
    }
    QWidget::closeEvent(event);
}

void ViewportWindow::SetSceneCamera(const bim::viewport::Vector3& eye,
                                    const bim::viewport::Vector3& target,
                                    const bim::viewport::Vector3& worldUp) {
    const bim::viewport::Status status = camera_.SetLookAt(eye, target, worldUp);
    if (!status) {
        // Never silently ignored (AA Source Review Round 1's own
        // never-fabricate/never-silently-drop discipline, applied here to
        // M10): a scene-supplied eye/target/up that Camera::SetLookAt
        // rejects (non-finite, eye == target, or a degenerate up vector)
        // leaves the camera at whatever it was previously showing, logged
        // so the gap is visible rather than hidden.
        qDebug() << "ViewportWindow: scene-supplied camera eye/target/worldUp rejected, code="
                 << static_cast<int>(status.Code());
    }
}

// AA Source Review Round 3 finding M13 - evidence-mode-only accessors. Each
// is a thin forward to the same private renderer_/lifecycle_/camera_/
// surface_ state and EnsureRendererInitialized()/TeardownRenderer() methods
// this widget's own interactive path already uses above; no logic is
// duplicated for evidence_mode.cpp's benefit.

QWindow* ViewportWindow::EvidenceNativeSurface() const noexcept {
    return surface_;
}

void ViewportWindow::EvidenceDestroyNativeSurface() {
    // UNVERIFIED - see this method's declaration in viewport_window.hpp.
    // The real QWindow::destroy() call on surface_ is expected to route
    // through the platform integration's normal teardown path, delivering a
    // genuine QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed to
    // ViewportSurface::event() (above), which invokes
    // onSurfaceAboutToBeDestroyed -> TeardownRenderer() (wired in the
    // constructor) purely as a side effect of that real event - this method
    // itself never calls TeardownRenderer() directly.
    surface_->destroy();
}

void ViewportWindow::EvidenceRecreateNativeSurface() {
    // UNVERIFIED - see this method's declaration in viewport_window.hpp.
    // create() re-realizes the native platform window against the same
    // parent() the window-container mechanism established in the
    // constructor (destroy() does not clear parent()).
    //
    // AA Source Review Round 5 finding N03: re-exposure previously called
    // surface_->setVisible(true) directly - bypassing surface_container_'s
    // (the QWidget::createWindowContainer() wrapper's) own
    // visibility/exposure bookkeeping for its embedded child window, since
    // surface_ is owned/managed by that container, not independently by
    // this method. This now toggles surface_container_'s own Qt Widgets
    // visibility (hide() then show()) instead - the container-controlled
    // path a real interactive re-show would go through - which is expected
    // to cause Qt to re-embed/re-expose the now-recreated surface_ through
    // its normal container mechanism, delivering a genuine exposeEvent to
    // ViewportSurface::exposeEvent() (above), which invokes
    // onExposedOrHidden -> EnsureRendererInitialized() (wired in the
    // constructor) purely as a side effect of that real event - this method
    // itself still never calls EnsureRendererInitialized() directly.
    // requestActivate() (keyboard-focus only, unrelated to the
    // visibility/exposure concern N03 raises) is kept, now issued after the
    // container has re-shown the surface.
    surface_->create();
    surface_container_->hide();
    surface_container_->show();
    surface_->requestActivate();
}

ViewportWindow::EvidenceSceneResult
ViewportWindow::EvidenceRunScene(const std::vector<bim::viewport::RenderMeshData>& meshes) {
    EvidenceSceneResult result;
    std::vector<bim::viewport_bgfx::RenderMeshHandle> handles;
    handles.reserve(meshes.size());

    bool all_created_ok = true;
    for (const bim::viewport::RenderMeshData& mesh : meshes) {
        result.vertex_count += mesh.positions.size();
        result.index_count += mesh.indices.size();
        const auto created = renderer_.CreateMesh(mesh);
        if (created.IsOk()) {
            handles.push_back(created.Value());
        } else {
            all_created_ok = false;
        }
    }
    result.create_mesh_passed = all_created_ok;

    bool render_ok = true;
    if (!handles.empty()) {
        const bim::viewport::Status render_status =
            renderer_.RenderFrame(camera_, handles.data(), handles.size());
        render_ok = static_cast<bool>(render_status);
    }
    result.render_frame_passed = render_ok;

    bool all_destroyed_ok = true;
    for (bim::viewport_bgfx::RenderMeshHandle& handle : handles) {
        if (!renderer_.DestroyMesh(handle)) {
            all_destroyed_ok = false;
        }
    }
    result.destroy_mesh_passed = all_destroyed_ok;

    return result;
}

void ViewportWindow::HandleMousePress(QMouseEvent& event) {
    press_pos_ = event.position();
    press_was_left_button_ = (event.button() == Qt::LeftButton);
    bridge_.OnMousePress(event);
}

void ViewportWindow::HandleMouseMove(QMouseEvent& event) {
    bridge_.OnMouseMove(event);
}

void ViewportWindow::HandleMouseRelease(QMouseEvent& event) {
    // A "click" (as opposed to a drag that just ended) is a left-button
    // release close to where the left button was pressed - Brief section
    // 7's ScreenToWorldRay is only meaningful as a discrete pick, not on
    // every drag sample (Camera::Orbit already consumes left-button drags).
    constexpr qreal kClickMaxMovementPx = 4.0;
    const QPointF release_pos = event.position();
    const bool was_click = press_was_left_button_ && event.button() == Qt::LeftButton &&
                           (release_pos - press_pos_).manhattanLength() <= kClickMaxMovementPx;
    bridge_.OnMouseRelease(event);
    if (was_click && renderer_.IsInitialized()) {
        const qreal dpr = surface_->devicePixelRatio();
        const auto pick = bridge_.PickRay(release_pos.x() * dpr, release_pos.y() * dpr,
                                          static_cast<std::uint32_t>(surface_->width() * dpr),
                                          static_cast<std::uint32_t>(surface_->height() * dpr));
        if (pick.IsOk()) {
            const bim::viewport::Ray& ray = pick.Value();
            qDebug() << "ViewportWindow: pick ray origin=(" << ray.origin.x << ',' << ray.origin.y
                     << ',' << ray.origin.z << ") direction=(" << ray.direction.x << ','
                     << ray.direction.y << ',' << ray.direction.z << ')';
        } else {
            qDebug() << "ViewportWindow: pick ray rejected, code=" << static_cast<int>(pick.Code());
        }
    }
}

void ViewportWindow::HandleWheel(QWheelEvent& event) {
    bridge_.OnWheel(event);
}
