#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string_view>
#include <thread>

#include "evidence_mode.hpp"
// AA Source Review Round 5 finding M16: live evidence mode now constructs
// the actual production MainWindow (not a bare ViewportWindow as its own
// top-level window) so the real child-under-top-level shell relationship
// (ViewportWindow embedded as MainWindow's central widget - see
// main_window.cpp) is what evidence mode observes and drives - see the
// --evidence-mode-live branch below.
#include "main_window.hpp"
// Still needed directly: the --evidence-mode-live branch below holds a
// ViewportWindow* (obtained via MainWindow::EvidenceViewportWindow()) and
// calls its Evidence* accessors, which requires the complete type.
#include "viewport_window.hpp"

// src/desktop/src/main.cpp - entry point for bim_desktop_spike
// (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 11).
//
// Three modes:
//   (no arguments)                        - shows MainWindow, runs the
//                                            normal Qt event loop
//                                            (interactive spike).
//   --evidence-mode <json-path>           - headless/noop-backend evidence
//                                            pass (see evidence_mode.hpp),
//                                            no window shown, writes
//                                            machine-readable JSON to the
//                                            exact given path and exits
//                                            with the pass/fail exit code.
//                                            Safe to run from CTest/CI with
//                                            no display attached.
//   --evidence-mode-live <json-path>      - live, explicit-D3D11-backend
//                                            evidence pass against the real
//                                            production MainWindow/
//                                            ViewportWindow shell (AA Source
//                                            Review Round 5 finding M16 -
//                                            not a bare ViewportWindow
//                                            constructed as its own
//                                            top-level window, as Rounds
//                                            3/4 did).
//                                            Requires an attached display;
//                                            if the window never exposes,
//                                            the JSON honestly records the
//                                            live backend as unavailable
//                                            rather than silently
//                                            downgrading to headless (AA
//                                            Source Review Round 1 finding
//                                            M05).
// Used by scripts/ci/viewport-spike.ps1.

namespace {

// Pumps the Qt event loop briefly, waiting for `window` to report
// isExposed() (Brief section 11 "exposure": a native handle is only valid
// to acquire once the surface is actually realized/exposed) - bounded by a
// short deadline so a build agent with no compositor/display cannot hang
// the evidence run forever; the caller checks isExposed() itself
// afterward and records honestly if it never became true.
void WaitForExposure(QWindow& window, int timeoutMs) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!window.isExposed() && std::chrono::steady_clock::now() < deadline) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// RD1.6-04 (bugprone-exception-escape): stderr reporter for main()'s outer
// exception boundary below. Declared noexcept so the boundary's handlers
// themselves contain no potentially-throwing C++ operation (the exact
// reason P0-T002's round-8B follow-up had to drop its std::cerr handlers -
// see tests/integration/p0_t002_geometry_evidence.cpp); C stdio only, no
// stream/string construction. `detail` may be null.
void ReportFatalDiagnostic(const char* message, const char* detail) noexcept {
    std::fputs("bim_desktop_spike: fatal: ", stderr);
    std::fputs(message, stderr);
    if (detail != nullptr) {
        std::fputs(": ", stderr);
        std::fputs(detail, stderr);
    }
    std::fputs("\n", stderr);
}

} // namespace

// RD1.6-04 (bugprone-exception-escape): clang-tidy flags int main(int,
// char**) because nothing in this translation unit previously guaranteed
// that every exception is caught before it would unwind out of main(). The
// fix follows the already-accepted P0-T002 evidence entry-point pattern
// (tests/integration/p0_t002_geometry_evidence.cpp, SA-R8-04/8B): this
// small extraction, not a suppression. The body below is byte-for-byte the
// original main() body, unchanged - the three modes, argument handling,
// QApplication construction, evidence-mode options/semantics, Qt lifecycle,
// and every normal-path return value are exactly as before. main() itself
// (after this function) does nothing but forward here inside its own
// try/catch.
static int RunDesktopMain(int argc, char** argv) {
    std::string_view mode_flag;
    std::string_view json_path;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--evidence-mode" || arg == "--evidence-mode-live") {
            if (i + 1 >= argc) {
                return 2; // missing required <json-path> argument
            }
            mode_flag = arg;
            json_path = argv[i + 1];
            break;
        }
    }

    if (!mode_flag.empty()) {
        // QApplication (not just QCoreApplication) is constructed even in
        // headless evidence mode: several Qt-adjacent subsystems (e.g.
        // plugin loading paths, style/theme init) are only correctly
        // initialized once a QApplication exists, even when no top-level
        // widget is ever shown and no event loop is ever run to
        // completion.
        QApplication app(argc, argv);

        bim::desktop::EvidenceModeOptions options;
        options.jsonOutputPath = std::string(json_path);

        if (mode_flag != "--evidence-mode-live") {
            return bim::desktop::RunEvidenceMode(options);
        }

        options.live = true;

        // AA Source Review Round 5 finding M16: the live evidence path now
        // constructs the actual production MainWindow (a QMainWindow that
        // embeds ViewportWindow as its central child - see
        // main_window.cpp), not a bare ViewportWindow constructed as its
        // own top-level window. The prior (Round 3/4) construction made
        // ViewportWindow the top-level widget itself - a shell relationship
        // the real interactive application never actually has - so it could
        // not exercise the real MainWindow-embedded child-under-top-level
        // lifecycle path (in particular minimize/restore - see
        // viewport_window.hpp's own M16 comment on eventFilter()). Surface
        // loss/recreation, renderer teardown/re-init, minimize/restore,
        // resize/DPR, and D3D11 backend evidence are all still produced
        // through the exact ViewportWindow instance MainWindow itself
        // constructs and owns (via EvidenceViewportWindow(), a thin forward
        // - see main_window.hpp) and inside RunEvidenceMode (via its
        // Evidence* accessors - see viewport_window.hpp), never a parallel
        // architecture.
        MainWindow evidence_main_window;
        evidence_main_window.show();

        QWindow* const top_level_window = evidence_main_window.windowHandle();
        if (top_level_window != nullptr) {
            WaitForExposure(*top_level_window, /*timeoutMs=*/3000);
        }

        // The real production ViewportWindow child MainWindow constructed
        // and embedded as its central widget - not a second one built for
        // evidence. Null only in the (never-expected, defensively-checked)
        // case that MainWindow's own construction did not produce one.
        ViewportWindow* const viewport_window = evidence_main_window.EvidenceViewportWindow();

        // Beyond the top-level widget's own exposure, also wait (bounded)
        // for the production ViewportWindow's renderer to actually come up
        // via its real EnsureRendererInitialized() path - triggered by the
        // embedded ViewportSurface's own exposeEvent, not by anything this
        // function drives directly (see viewport_window.cpp).
        if (viewport_window != nullptr) {
            const auto deadline =
                std::chrono::steady_clock::now() + std::chrono::milliseconds(2000);
            while (!viewport_window->EvidenceIsRendererReady() &&
                   std::chrono::steady_clock::now() < deadline) {
                QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        // AA Source Review Round 5 finding M16 (necessary consequence):
        // widthPx/heightPx must reflect the VIEWPORT's own physical size,
        // not MainWindow's full top-level window size - Round 3/4's
        // evidence_window WAS the viewport with no surrounding chrome, but
        // MainWindow's own width()/height() now additionally includes its
        // toolbar and status bar, which are not part of the rendered
        // viewport. devicePixelRatio() is a per-screen value, unaffected by
        // which widget it is queried from, so the top-level's is kept.
        const qreal dpr = evidence_main_window.devicePixelRatio();
        options.devicePixelRatio = dpr;
        if (viewport_window != nullptr) {
            options.widthPx = static_cast<std::uint32_t>(viewport_window->width() * dpr);
            options.heightPx = static_cast<std::uint32_t>(viewport_window->height() * dpr);
        } else {
            options.widthPx = static_cast<std::uint32_t>(evidence_main_window.width() * dpr);
            options.heightPx = static_cast<std::uint32_t>(evidence_main_window.height() * dpr);
        }

        const QList<QScreen*> screens = QGuiApplication::screens();
        options.screenCount = static_cast<int>(screens.size());
        // AA Source Review Round 3 finding MINOR: compare candidate screens
        // against the actual exposed evidence window's CURRENT screen DPR,
        // not screens.front() - screens.front() is an arbitrary
        // enumeration-order screen that need not be the one the evidence
        // window is actually showing on, which could both miss a real
        // differing-DPI pair and misreport
        // crossMonitorDifferingDpiAvailable against the wrong baseline.
        QScreen* const current_screen = (top_level_window != nullptr)
                                            ? top_level_window->screen()
                                            : evidence_main_window.screen();
        const double baseline_dpr =
            (current_screen != nullptr)
                ? current_screen->devicePixelRatio()
                : (screens.isEmpty() ? 0.0 : screens.front()->devicePixelRatio());
        options.crossMonitorDifferingDpiAvailable = false;
        // AA Source Review Round 2 finding M11: remember an actual QScreen
        // that reports a different devicePixelRatio() than the baseline,
        // not just the boolean fact that one exists - RunEvidenceMode
        // needs a real target screen to move the live window to for its
        // DPR-transition evidence check.
        QScreen* differing_screen = nullptr;
        for (QScreen* screen : screens) {
            options.screenIdentities.push_back(screen->name().toStdString());
            if (screen->devicePixelRatio() != baseline_dpr) {
                options.crossMonitorDifferingDpiAvailable = true;
                if (differing_screen == nullptr) {
                    differing_screen = screen;
                }
            }
        }
        options.differingDpiScreen = differing_screen;

        QWindow* const native_surface =
            (viewport_window != nullptr) ? viewport_window->EvidenceNativeSurface() : nullptr;
        if (native_surface != nullptr && native_surface->isExposed()) {
            // The embedded ViewportSurface - not the top-level widget's own
            // native handle - is what EnsureRendererInitialized() actually
            // attaches bgfx to (see viewport_window.cpp), so this is the
            // correct handle for the repeatability section's own raw
            // Renderer instances (unaffected by M13 - see evidence_mode.cpp)
            // to render into as well.
            //
            // RD1.6-05 (performance-no-int-to-ptr): this is the required
            // Qt/Win32 native-handle boundary - Qt exposes the native window
            // handle as an integer WId, and the neutral viewport contract
            // carries it as an opaque void* for bgfx; the int-to-pointer
            // conversion is inherent to that interop and is kept exactly
            // as-is (no redesign, no new abstraction). NEXTLINE form only
            // because a trailing comment would push the statement past the
            // column limit.
            // NOLINTNEXTLINE(performance-no-int-to-ptr)
            options.nativeWindowHandle = reinterpret_cast<void*>(native_surface->winId());
            // AA Source Review Round 2 finding M11 (unchanged this round):
            // the live evidence run's minimize/restore and DPR-transition
            // checks need the actual top-level QWindow, not just its native
            // handle, so they can call showMinimized()/showNormal()/
            // setScreen() on the real locked surface rather than
            // synthesizing NOT_AVAILABLE. This is now MainWindow's own
            // windowHandle() (AA Source Review Round 5 finding M16) - the
            // real top-level window whose minimize/restore
            // ViewportWindow::eventFilter() actually observes - not the
            // embedded ViewportWindow's own windowHandle() as it was when
            // ViewportWindow was itself constructed as the top-level widget.
            options.liveWindow = top_level_window;
            // AA Source Review Round 3 finding M13 (M16 this round: now the
            // real production child ViewportWindow embedded under
            // MainWindow, obtained via EvidenceViewportWindow(), not a
            // ViewportWindow constructed as its own top-level window) -
            // RunEvidenceMode's live path drives surface/lifecycle/backend/
            // scene evidence through this object's Evidence* accessors
            // instead of a second architecture.
            options.viewportWindow = viewport_window;
        }
        // else: nativeWindowHandle/liveWindow/viewportWindow stay null -
        // RunEvidenceMode records the live backend as unavailable rather
        // than inventing a PASS (Brief section 17 posture).

        const int result = bim::desktop::RunEvidenceMode(options);
        evidence_main_window.hide();
        return result;
    }

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return QApplication::exec();
}

// RD1.6-04 (bugprone-exception-escape): the outer exception boundary. Same
// shape as P0-T002's accepted round-8B main(): declared noexcept, forwards
// to RunDesktopMain() inside try/catch, and each handler does nothing but
// call the noexcept stderr reporter above and return non-zero - so no
// exception, std::exception-derived or otherwise, can escape main(), and
// nothing in the handlers is itself a potentially-throwing operation.
// Exceptions are never swallowed into a success return: both handlers
// return 1 (the same non-zero failure value the P0-T002 pattern uses).
// Normal successful execution is untouched - it is exactly
// RunDesktopMain()'s return value.
int main(int argc, char** argv) noexcept {
    try {
        return RunDesktopMain(argc, argv);
    } catch (const std::exception& ex) {
        ReportFatalDiagnostic("unhandled std::exception", ex.what());
        return 1;
    } catch (...) {
        ReportFatalDiagnostic("unhandled unknown (non-std::exception) exception", nullptr);
        return 1;
    }
}
