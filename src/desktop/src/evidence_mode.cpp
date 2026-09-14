#include "evidence_mode.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QtGlobal>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/lifecycle.hpp"
#include "bim/viewport/ray.hpp"
#include "bim/viewport_bgfx/renderer.hpp"
#include "spike_scene.hpp"
// AA Source Review Round 3 finding M13: live evidence now drives the actual
// production ViewportWindow instance main.cpp constructs, via its Evidence*
// accessors, instead of this file constructing a second, separately-driven
// Renderer/ViewportLifecycle for the live path.
#include "viewport_window.hpp"

// UNVERIFIED - authored by Claude, the Implementation Engineer, without a
// build/execution channel (no Windows shell in this session). The Operator
// must build and run this before any PASS/FAIL it reports can be treated
// as verified evidence - see docs/evidence/P0-T003/CLAUDE_HANDOVER.md.

namespace bim::desktop {

namespace {

// ---------------------------------------------------------------------
// Minimal, dependency-free JSON tree + serializer. No JSON library is
// authorized by the frozen vcpkg manifest (Brief section 18: vcpkg.json
// adds only qtbase + bgfx), so this is hand-rolled rather than pulling in
// a new third-party dependency.
// ---------------------------------------------------------------------
class JsonValue {
public:
    static JsonValue MakeNull() { return JsonValue(Kind::Null); }
    static JsonValue MakeBool(bool v) {
        JsonValue j(Kind::Bool);
        j.bool_ = v;
        return j;
    }
    static JsonValue MakeInt(long long v) {
        JsonValue j(Kind::Int);
        j.int_ = v;
        return j;
    }
    static JsonValue MakeDouble(double v) {
        JsonValue j(Kind::Double);
        j.double_ = v;
        return j;
    }
    static JsonValue MakeString(std::string v) {
        JsonValue j(Kind::String);
        j.string_ = std::move(v);
        return j;
    }
    static JsonValue MakeArray() { return JsonValue(Kind::Array); }
    static JsonValue MakeObject() { return JsonValue(Kind::Object); }

    // Not-yet-exercised / environment-does-not-support-this-check marker
    // (Brief section 17: "the evidence must record NOT_AVAILABLE rather
    // than inventing a PASS"). Distinct from both true and false.
    static JsonValue MakeNotAvailable(std::string reason) {
        JsonValue j = MakeObject();
        j.Set("status", MakeString("NOT_AVAILABLE"));
        j.Set("reason", MakeString(std::move(reason)));
        return j;
    }

    void Set(std::string key, JsonValue value) {
        members_.emplace_back(std::move(key), std::move(value));
    }
    void Push(JsonValue value) { elements_.push_back(std::move(value)); }

    void WriteTo(std::ostream& out, int indent = 0) const {
        const std::string pad(static_cast<std::size_t>(indent) * 2, ' ');
        const std::string pad_inner(static_cast<std::size_t>(indent + 1) * 2, ' ');
        switch (kind_) {
            case Kind::Null:
                out << "null";
                break;
            case Kind::Bool:
                out << (bool_ ? "true" : "false");
                break;
            case Kind::Int:
                out << int_;
                break;
            case Kind::Double:
                out << double_;
                break;
            case Kind::String:
                WriteEscapedString(out, string_);
                break;
            case Kind::Array: {
                if (elements_.empty()) {
                    out << "[]";
                    break;
                }
                out << "[\n";
                for (std::size_t i = 0; i < elements_.size(); ++i) {
                    out << pad_inner;
                    elements_[i].WriteTo(out, indent + 1);
                    if (i + 1 < elements_.size()) {
                        out << ',';
                    }
                    out << '\n';
                }
                out << pad << "]";
                break;
            }
            case Kind::Object: {
                if (members_.empty()) {
                    out << "{}";
                    break;
                }
                out << "{\n";
                for (std::size_t i = 0; i < members_.size(); ++i) {
                    out << pad_inner;
                    WriteEscapedString(out, members_[i].first);
                    out << ": ";
                    members_[i].second.WriteTo(out, indent + 1);
                    if (i + 1 < members_.size()) {
                        out << ',';
                    }
                    out << '\n';
                }
                out << pad << "}";
                break;
            }
        }
    }

private:
    // RD1.6-02 (performance-enum-size): the default enum representation is
    // intentionally preserved for the Phase-0 contract; not changed merely
    // for storage-size optimization (Architecture Authority decision).
    // NEXTLINE form (not trailing) only because a trailing comment would push
    // this single-line enum past the column limit.
    // NOLINTNEXTLINE(performance-enum-size)
    enum class Kind { Null, Bool, Int, Double, String, Array, Object };
    explicit JsonValue(Kind kind) : kind_(kind) {}

    static void WriteEscapedString(std::ostream& out, const std::string& s) {
        out << '"';
        for (const char c : s) {
            switch (c) {
                case '"':
                    out << "\\\"";
                    break;
                case '\\':
                    out << "\\\\";
                    break;
                case '\n':
                    out << "\\n";
                    break;
                case '\r':
                    out << "\\r";
                    break;
                case '\t':
                    out << "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                        out << buf;
                    } else {
                        out << c;
                    }
                    break;
            }
        }
        out << '"';
    }

    Kind kind_;
    bool bool_ = false;
    long long int_ = 0;
    double double_ = 0.0;
    std::string string_;
    std::vector<JsonValue> elements_;
    std::vector<std::pair<std::string, JsonValue>> members_;
};

JsonValue MakeResultValue(const bim::viewport::Status& status) {
    JsonValue j = JsonValue::MakeObject();
    j.Set("passed", JsonValue::MakeBool(static_cast<bool>(status)));
    j.Set("code", JsonValue::MakeInt(static_cast<long long>(status.Code())));
    return j;
}

const char* SceneName(SpikeSceneId id) {
    switch (id) {
        case SpikeSceneId::V01_SingleIndexedCube:
            return "V01_SingleIndexedCube";
        case SpikeSceneId::V02_GridAxesCube:
            return "V02_GridAxesCube";
        case SpikeSceneId::V03_OneThousandBoxes:
            return "V03_OneThousandBoxes";
        case SpikeSceneId::V04_RepresentativeMediumMesh:
            return "V04_RepresentativeMediumMesh";
        case SpikeSceneId::V05_LargeCoordinateScenario:
            return "V05_LargeCoordinateScenario";
    }
    return "Unknown";
}

// AA Source Review Round 3 finding M13: lifecycle.hpp deliberately has no
// to-string helper of its own (Brief section 10: it is a neutral,
// dependency-free state machine) - this is a local, evidence-JSON-only
// convenience, not a duplication of any lifecycle logic.
const char* LifecycleStateName(bim::viewport::ViewportState state) {
    switch (state) {
        case bim::viewport::ViewportState::Uninitialized:
            return "Uninitialized";
        case bim::viewport::ViewportState::SurfaceUnavailable:
            return "SurfaceUnavailable";
        case bim::viewport::ViewportState::Ready:
            return "Ready";
        case bim::viewport::ViewportState::Suspended:
            return "Suspended";
        case bim::viewport::ViewportState::ShuttingDown:
            return "ShuttingDown";
        case bim::viewport::ViewportState::Destroyed:
            return "Destroyed";
    }
    return "Unknown";
}

// AA Source Review Round 2 finding M11: showMinimized()/showNormal()/
// setScreen()/setGeometry() are requests to the underlying platform
// plugin, not synchronous state changes - visibility()/devicePixelRatio()
// can still report the pre-request value immediately afterward unless the
// Qt event loop actually runs so the platform's response is processed.
// Pumps events for approximately `totalMs` milliseconds before the live
// minimize/restore and DPR-transition evidence checks below take their
// "after" observation.
void PumpEventsFor(int totalMs) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(totalMs);
    while (std::chrono::steady_clock::now() < deadline) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

constexpr SpikeSceneId kAllSceneIds[] = {
    SpikeSceneId::V01_SingleIndexedCube,       SpikeSceneId::V02_GridAxesCube,
    SpikeSceneId::V03_OneThousandBoxes,        SpikeSceneId::V04_RepresentativeMediumMesh,
    SpikeSceneId::V05_LargeCoordinateScenario,
};

// Runs one spike scene end-to-end (CreateMesh for every mesh in the scene,
// one RenderFrame, DestroyMesh for every handle) and appends a JSON
// summary to `out_scenes`. Every V01-V05 scene (Brief section 14) is now
// well-formed by construction (unlike the prior authoring pass's V05,
// which was deliberately an invalid mesh) - CreateMesh is expected to
// succeed for all five.
bool RunOneScene(SpikeSceneId id, bim::viewport_bgfx::Renderer& renderer,
                 const bim::viewport::Camera& camera, JsonValue& out_scenes) {
    const SpikeScene scene = BuildSpikeScene(id);
    std::vector<bim::viewport_bgfx::RenderMeshHandle> handles;

    // AA Source Review Round 3 finding M14: apply this scene's own
    // suggested camera (scene.cameraEye/cameraTarget/cameraWorldUp -
    // BuildSpikeScene() populates these per scene; see spike_scene.hpp)
    // before rendering it, rather than unconditionally reusing the
    // caller's shared `camera` parameter unmodified. V01-V04's suggested
    // cameras equal the shared default (so this changes nothing observable
    // for them), but V05's large-coordinate scene previously rendered
    // against a camera that could never actually observe it - CreateMesh/
    // RenderFrame/DestroyMesh all reported success even though the scene
    // was entirely outside the view frustum.
    bim::viewport::Camera scene_camera = camera;
    const bim::viewport::Status scene_camera_status =
        scene_camera.SetLookAt(scene.cameraEye, scene.cameraTarget, scene.cameraWorldUp);

    std::size_t total_positions = 0;
    std::size_t total_indices = 0;
    bool all_created_ok = true;
    for (const bim::viewport::RenderMeshData& mesh : scene.meshes) {
        total_positions += mesh.positions.size();
        total_indices += mesh.indices.size();
        const auto result = renderer.CreateMesh(mesh);
        if (result.IsOk()) {
            handles.push_back(result.Value());
        } else {
            all_created_ok = false;
        }
    }

    bool render_ok = true;
    if (!handles.empty()) {
        const bim::viewport::Status render_status =
            renderer.RenderFrame(scene_camera, handles.data(), handles.size());
        render_ok = static_cast<bool>(render_status);
    }

    bool all_destroyed_ok = true;
    for (bim::viewport_bgfx::RenderMeshHandle& handle : handles) {
        if (!renderer.DestroyMesh(handle)) {
            all_destroyed_ok = false;
        }
    }

    const bool scene_ok =
        all_created_ok && render_ok && all_destroyed_ok && static_cast<bool>(scene_camera_status);

    JsonValue entry = JsonValue::MakeObject();
    entry.Set("id", JsonValue::MakeString(SceneName(id)));
    entry.Set("name", JsonValue::MakeString(scene.name));
    entry.Set("mesh_count", JsonValue::MakeInt(static_cast<long long>(scene.meshes.size())));
    entry.Set("vertex_count", JsonValue::MakeInt(static_cast<long long>(total_positions)));
    entry.Set("index_count", JsonValue::MakeInt(static_cast<long long>(total_indices)));
    entry.Set("triangle_count", JsonValue::MakeInt(static_cast<long long>(total_indices / 3)));
    entry.Set("camera_applied", MakeResultValue(scene_camera_status));
    entry.Set("camera_eye", JsonValue::MakeString(std::to_string(scene.cameraEye.x) + ", " +
                                                  std::to_string(scene.cameraEye.y) + ", " +
                                                  std::to_string(scene.cameraEye.z)));
    entry.Set("camera_target", JsonValue::MakeString(std::to_string(scene.cameraTarget.x) + ", " +
                                                     std::to_string(scene.cameraTarget.y) + ", " +
                                                     std::to_string(scene.cameraTarget.z)));
    entry.Set("create_mesh_passed", JsonValue::MakeBool(all_created_ok));
    entry.Set("render_frame_passed", JsonValue::MakeBool(render_ok));
    entry.Set("destroy_mesh_passed", JsonValue::MakeBool(all_destroyed_ok));
    entry.Set("overall_passed", JsonValue::MakeBool(scene_ok));
    out_scenes.Push(std::move(entry));

    return scene_ok;
}

// AA Source Review Round 3 finding M13/M14: the live-mode equivalent of
// RunOneScene() above - drives the exact production ViewportWindow's own
// renderer_/camera_ (via SetSceneCamera() + EvidenceRunScene(), see
// viewport_window.hpp) instead of a second, separately-constructed
// Renderer, and applies each scene's own suggested camera first (M14),
// exactly like RunOneScene() does for the headless path.
bool RunOneLiveScene(SpikeSceneId id, ViewportWindow& viewport_window, JsonValue& out_scenes) {
    const SpikeScene scene = BuildSpikeScene(id);
    viewport_window.SetSceneCamera(scene.cameraEye, scene.cameraTarget, scene.cameraWorldUp);
    const ViewportWindow::EvidenceSceneResult result =
        viewport_window.EvidenceRunScene(scene.meshes);
    const bool scene_ok =
        result.create_mesh_passed && result.render_frame_passed && result.destroy_mesh_passed;

    JsonValue entry = JsonValue::MakeObject();
    entry.Set("id", JsonValue::MakeString(SceneName(id)));
    entry.Set("name", JsonValue::MakeString(scene.name));
    entry.Set("mesh_count", JsonValue::MakeInt(static_cast<long long>(scene.meshes.size())));
    entry.Set("vertex_count", JsonValue::MakeInt(static_cast<long long>(result.vertex_count)));
    entry.Set("index_count", JsonValue::MakeInt(static_cast<long long>(result.index_count)));
    entry.Set("triangle_count", JsonValue::MakeInt(static_cast<long long>(result.index_count / 3)));
    entry.Set("camera_eye", JsonValue::MakeString(std::to_string(scene.cameraEye.x) + ", " +
                                                  std::to_string(scene.cameraEye.y) + ", " +
                                                  std::to_string(scene.cameraEye.z)));
    entry.Set("camera_target", JsonValue::MakeString(std::to_string(scene.cameraTarget.x) + ", " +
                                                     std::to_string(scene.cameraTarget.y) + ", " +
                                                     std::to_string(scene.cameraTarget.z)));
    entry.Set("create_mesh_passed", JsonValue::MakeBool(result.create_mesh_passed));
    entry.Set("render_frame_passed", JsonValue::MakeBool(result.render_frame_passed));
    entry.Set("destroy_mesh_passed", JsonValue::MakeBool(result.destroy_mesh_passed));
    entry.Set("overall_passed", JsonValue::MakeBool(scene_ok));
    out_scenes.Push(std::move(entry));

    return scene_ok;
}

// Brief section 16: "at least 20 clean process-level initialize/render/
// shutdown cycles. Each process must return success." Run here as an
// in-process repeatability loop against a lean single-triangle mesh (not
// the full V01-V05 corpus, to keep each cycle cheap enough that 20+ of
// them is a reasonable default even against a live D3D11 device) -
// scripts/ci/viewport-spike.ps1 separately re-invokes this executable as
// 20+ distinct OS processes for the process-boundary variant of the same
// requirement. RD1.7: headless-only, and only ever called while NO other
// Renderer is initialized in this process - each cycle's Initialize() is a
// full bgfx runtime init, and bgfx is a single process-wide runtime (see
// the repeatability section in RunEvidenceMode()).
bool RunRepeatabilityCycles(const bim::viewport_bgfx::RendererCreateInfo& create_info,
                            int cycle_count, JsonValue& out_cycles) {
    bim::viewport::RenderMeshData triangle;
    triangle.topology = bim::viewport::MeshTopology::TriangleList;
    triangle.positions = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    triangle.indices = {0, 1, 2};

    bim::viewport::Camera camera;
    static_cast<void>(camera.SetLookAt({0.0, -5.0, 2.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}));
    static_cast<void>(camera.SetPerspective(0.9, 0.01, 1000.0));
    static_cast<void>(camera.SetAspectRatio(static_cast<double>(create_info.widthPx) /
                                            static_cast<double>(create_info.heightPx)));

    bool all_cycles_ok = true;
    for (int cycle = 0; cycle < cycle_count; ++cycle) {
        const auto cycle_start = std::chrono::steady_clock::now();

        bim::viewport_bgfx::Renderer renderer;
        bool cycle_ok = static_cast<bool>(renderer.Initialize(create_info));

        if (cycle_ok) {
            const auto created = renderer.CreateMesh(triangle);
            cycle_ok = created.IsOk();
            if (created.IsOk()) {
                bim::viewport_bgfx::RenderMeshHandle handle = created.Value();
                cycle_ok = cycle_ok && static_cast<bool>(renderer.RenderFrame(camera, &handle, 1));
                cycle_ok = cycle_ok && static_cast<bool>(renderer.DestroyMesh(handle));
            }
        }
        if (renderer.IsInitialized()) {
            cycle_ok = static_cast<bool>(renderer.Shutdown()) && cycle_ok;
        }

        const auto cycle_end = std::chrono::steady_clock::now();
        const double duration_ms =
            std::chrono::duration<double, std::milli>(cycle_end - cycle_start).count();

        JsonValue entry = JsonValue::MakeObject();
        entry.Set("cycle_index", JsonValue::MakeInt(cycle));
        entry.Set("passed", JsonValue::MakeBool(cycle_ok));
        entry.Set("duration_ms_observational", JsonValue::MakeDouble(duration_ms));
        out_cycles.Push(std::move(entry));

        all_cycles_ok = all_cycles_ok && cycle_ok;
    }

    return all_cycles_ok;
}

} // namespace

int RunEvidenceMode(const EvidenceModeOptions& options) {
    bool overall_passed = true;
    const auto RecordResult = [&overall_passed](bool passed) {
        overall_passed = overall_passed && passed;
        return passed;
    };

    JsonValue root = JsonValue::MakeObject();

    // ---- task/gate/brief identity (Brief section 22) ----
    root.Set("task_id", JsonValue::MakeString("P0-T003"));
    root.Set("gate_id", JsonValue::MakeString("BIM-AG-P0-T003"));
    root.Set("brief_id", JsonValue::MakeString("BIM-TASK-P0-T003-CLAUDE v1.0"));
    root.Set("evidence_mode", JsonValue::MakeString(options.live ? "live" : "headless"));
    root.Set("authoring_disclosure",
             JsonValue::MakeString(
                 "This code was authored by Claude, the Implementation Engineer, without a "
                 "build/execution channel. Whether the PASS/FAIL values below reflect a real "
                 "run, or are simply what this authoring pass expects the code to do once it "
                 "actually runs, depends entirely on whether an Operator actually built and "
                 "executed this binary to produce this exact JSON file - see "
                 "docs/evidence/P0-T003/CLAUDE_HANDOVER.md."));

    // ---- Qt / bgfx version identity ----
    root.Set("qt_version", JsonValue::MakeString(qVersion()));
    // bgfx has no single stable runtime "version string" API exposed the
    // same way qVersion() is - this records the frozen vcpkg baseline
    // identity from Brief section 2 rather than querying bgfx at runtime;
    // the Operator should cross-check this against the actually-resolved
    // vcpkg install if a runtime-queried value is later found to be
    // available.
    root.Set("bgfx_version_frozen_baseline", JsonValue::MakeString("1.129.8940-496#1"));

    // ---- camera semantic parameters ----
    bim::viewport::Camera camera;
    {
        const bim::viewport::Status look_at =
            camera.SetLookAt({0.0, -6.0, 3.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 1.0});
        const bim::viewport::Status perspective = camera.SetPerspective(0.9, 0.01, 10000.0);
        const double initial_aspect =
            (options.heightPx != 0)
                ? static_cast<double>(options.widthPx) / static_cast<double>(options.heightPx)
                : 16.0 / 9.0;
        const bim::viewport::Status aspect = camera.SetAspectRatio(initial_aspect);

        JsonValue camera_json = JsonValue::MakeObject();
        camera_json.Set("set_look_at", MakeResultValue(look_at));
        camera_json.Set("set_perspective", MakeResultValue(perspective));
        camera_json.Set("set_aspect_ratio", MakeResultValue(aspect));
        camera_json.Set("eye", JsonValue::MakeString(std::to_string(camera.Eye().x) + ", " +
                                                     std::to_string(camera.Eye().y) + ", " +
                                                     std::to_string(camera.Eye().z)));
        camera_json.Set("target", JsonValue::MakeString(std::to_string(camera.Target().x) + ", " +
                                                        std::to_string(camera.Target().y) + ", " +
                                                        std::to_string(camera.Target().z)));
        camera_json.Set("world_up",
                        JsonValue::MakeString(std::to_string(camera.WorldUp().x) + ", " +
                                              std::to_string(camera.WorldUp().y) + ", " +
                                              std::to_string(camera.WorldUp().z)));
        camera_json.Set("vertical_fov_radians", JsonValue::MakeDouble(camera.VerticalFovRadians()));
        camera_json.Set("aspect_ratio", JsonValue::MakeDouble(camera.AspectRatio()));
        camera_json.Set("near_plane", JsonValue::MakeDouble(camera.NearPlane()));
        camera_json.Set("far_plane", JsonValue::MakeDouble(camera.FarPlane()));

        // Orbit/pan/dolly result (Brief section 22): each is infallible
        // (camera.hpp), so "result" here means "did the semantic state
        // actually change as expected," not a Status.
        const auto eye_before_orbit = camera.Eye();
        camera.Orbit(0.2, 0.05);
        const bool orbit_changed_eye_not_target =
            (camera.Eye().x != eye_before_orbit.x || camera.Eye().y != eye_before_orbit.y ||
             camera.Eye().z != eye_before_orbit.z);
        camera_json.Set("orbit_result",
                        JsonValue::MakeBool(RecordResult(orbit_changed_eye_not_target)));

        const auto eye_before_pan = camera.Eye();
        const auto target_before_pan = camera.Target();
        camera.Pan(0.5, 0.25);
        const bool pan_moved_both =
            (camera.Eye().x != eye_before_pan.x || camera.Eye().y != eye_before_pan.y) &&
            (camera.Target().x != target_before_pan.x || camera.Target().y != target_before_pan.y);
        camera_json.Set("pan_result", JsonValue::MakeBool(RecordResult(pan_moved_both)));

        const auto eye_before_dolly = camera.Eye();
        camera.Dolly(0.5);
        const bool dolly_moved_eye =
            (camera.Eye().x != eye_before_dolly.x || camera.Eye().y != eye_before_dolly.y ||
             camera.Eye().z != eye_before_dolly.z);
        camera_json.Set("dolly_result", JsonValue::MakeBool(RecordResult(dolly_moved_eye)));

        root.Set("camera", std::move(camera_json));

        RecordResult(static_cast<bool>(look_at));
        RecordResult(static_cast<bool>(perspective));
        RecordResult(static_cast<bool>(aspect));
    }

    // ---- lifecycle event results / renderer initialize ----
    // AA Source Review Round 3 finding M13: live mode (options.live ==
    // true) no longer constructs its own local Renderer/ViewportLifecycle
    // here - it defers to options.viewportWindow, the actual production
    // ViewportWindow instance main.cpp already constructed, exposed, and
    // waited to become ready before calling RunEvidenceMode(). Headless
    // mode is completely unchanged: it still owns and drives the local
    // `lifecycle`/`renderer` objects below directly, exactly as before -
    // headless has no ViewportWindow/QWidget surface to speak of at all,
    // so there is no "second lifecycle architecture" concern for it.
    bim::viewport::ViewportLifecycle lifecycle;
    JsonValue lifecycle_json = JsonValue::MakeObject();

    bim::viewport_bgfx::RendererCreateInfo create_info;
    create_info.headless = !options.live;
    create_info.widthPx = options.widthPx;
    create_info.heightPx = options.heightPx;
    // Brief section 17's "NOT_AVAILABLE rather than inventing a PASS"
    // posture: a live evidence run with no realized native surface (e.g.
    // no attached display on this build agent) cannot honestly claim a
    // live D3D11 backend result. `live_viewport_ready` below gates every
    // M13-affected section (renderer-initialize/lifecycle/backend/scenes/
    // resize/surface-recreation). RD1.7: the former `live_surface_available`
    // flag existed only to gate the in-process repeatability loop in live
    // mode; that loop no longer runs in live mode at all (see the
    // repeatability section immediately below), so the flag is gone and the
    // handle is simply carried over (identical semantics: a null handle
    // stays null either way - RendererCreateInfo's default).
    create_info.nativeWindowHandle = options.nativeWindowHandle;
    const bool live_viewport_ready = options.live && options.viewportWindow != nullptr &&
                                     options.viewportWindow->EvidenceIsRendererReady();

    // ---- repeatability: >= 20 process-level-style initialize/render/
    // shutdown cycles (Brief section 16) ----
    // RD1.7 (Full Runbook D Attempt 2, Checks 35/60 - repeatability
    // orchestration defect): RunRepeatabilityCycles() constructs its own
    // short-lived Renderer per cycle, and each cycle's Initialize() is a
    // full bgfx runtime init. bgfx is a single process-wide runtime: a
    // second init while another Renderer already owns the live runtime
    // fails, which is exactly what the authoritative headless run showed
    // (cycles[0..19].passed=false, everything else PASS) because the loop
    // used to run far below, AFTER this function's own headless `renderer`
    // had been initialized and while it was still live. The loop is
    // therefore executed HERE, before the primary headless renderer below
    // is initialized, so no two bgfx Renderers are ever owned/active at the
    // same time - each cycle's initialize/render/destroy/shutdown sequence,
    // the cycle count default (>= 20, options.repeatabilityCycleCount), and
    // the JSON schema (requested_cycle_count / cycles / all_cycles_passed)
    // are unchanged. The result object is inserted into the output at the
    // same position as before (see the "repeatability" set further below),
    // so the emitted JSON key order is unchanged too.
    //
    // Live mode: the production ViewportWindow owns the active bgfx
    // renderer for the whole evidence run, so a nested in-process Renderer
    // can never be constructed alongside it. The Brief's process-level
    // repeatability requirement is owned and validated externally by
    // scripts/ci/viewport-spike.ps1, which invokes
    // `bim_desktop_spike.exe --evidence-mode <json>` as >= 20 separate OS
    // processes, each required to exit successfully. Live mode records this
    // honestly as NOT_AVAILABLE / externally covered; it is not a failure
    // of the live run, so it does not touch overall_passed.
    JsonValue repeatability_json = JsonValue::MakeObject();
    repeatability_json.Set("requested_cycle_count",
                           JsonValue::MakeInt(options.repeatabilityCycleCount));
    if (!options.live) {
        JsonValue cycles_json = JsonValue::MakeArray();
        const bool repeat_ok =
            RunRepeatabilityCycles(create_info, options.repeatabilityCycleCount, cycles_json);
        repeatability_json.Set("cycles", std::move(cycles_json));
        repeatability_json.Set("all_cycles_passed", JsonValue::MakeBool(repeat_ok));
        RecordResult(repeat_ok);
    } else {
        repeatability_json.Set(
            "cycles",
            JsonValue::MakeNotAvailable(
                "live mode: the production ViewportWindow owns the single process-wide bgfx "
                "runtime for the whole evidence run, so an in-process nested Renderer "
                "repeatability loop cannot run alongside it (a second bgfx init while one is "
                "active fails); process-level repeatability is covered externally by "
                "scripts/ci/viewport-spike.ps1 (>= 20 separate headless OS processes, each "
                "required to exit successfully) - not a live-run failure"));
        repeatability_json.Set("externally_covered_by",
                               JsonValue::MakeString("scripts/ci/viewport-spike.ps1"));
    }

    bim::viewport_bgfx::Renderer renderer;
    bool renderer_ready = false;
    if (options.live) {
        if (live_viewport_ready) {
            const bim::viewport::ViewportState state =
                options.viewportWindow->EvidenceLifecycleState();
            lifecycle_json.Set("begin_initialization",
                               JsonValue::MakeString("driven by the production ViewportWindow's "
                                                     "own constructor before this evidence "
                                                     "run began, not by this function - see "
                                                     "ViewportWindow::EvidenceLifecycleState()"));
            lifecycle_json.Set("renderer_initialize", JsonValue::MakeBool(true));
            lifecycle_json.Set(
                "on_surface_available",
                JsonValue::MakeString(std::string("lifecycle_state=") + LifecycleStateName(state)));
            RecordResult(state == bim::viewport::ViewportState::Ready);
            renderer_ready = true;
        } else {
            lifecycle_json.Set(
                "begin_initialization",
                JsonValue::MakeNotAvailable(
                    "no realized/exposed production ViewportWindow was available for the live "
                    "evidence run"));
            lifecycle_json.Set("renderer_initialize", JsonValue::MakeBool(false));
            lifecycle_json.Set(
                "on_surface_available",
                JsonValue::MakeNotAvailable(
                    "no realized/exposed native surface was available for the live evidence run "
                    "(Brief section 17 posture: recorded honestly, not silently downgraded to a "
                    "headless pass)"));
            // AA Source Review Round 2 finding M11 (unchanged this round):
            // an authoritative live-mode run (--evidence-mode-live) that
            // could not obtain a real D3D11 surface must not leave
            // overall_passed true.
            RecordResult(false);
        }
    } else {
        lifecycle_json.Set("begin_initialization",
                           MakeResultValue(lifecycle.BeginInitialization()));
        RecordResult(lifecycle.State() == bim::viewport::ViewportState::SurfaceUnavailable);

        const bim::viewport::Status init_status = renderer.Initialize(create_info);
        lifecycle_json.Set("renderer_initialize", MakeResultValue(init_status));
        if (init_status) {
            lifecycle_json.Set("on_surface_available",
                               MakeResultValue(lifecycle.OnSurfaceAvailable()));
            RecordResult(lifecycle.State() == bim::viewport::ViewportState::Ready);
            renderer_ready = true;
        } else {
            RecordResult(false);
        }
    }

    root.Set("lifecycle", std::move(lifecycle_json));

    // ---- backend/adapter identity (Brief section 22) ----
    // AA Source Review Round 3 finding M13: in live mode this reads the
    // production ViewportWindow's own renderer_ backend identity
    // (EvidenceBackendInfo(), a thin forward - see viewport_window.hpp)
    // rather than a second Renderer this file initialized itself.
    // renderer.BackendInfo() on the never-initialized local `renderer`
    // safely returns its documented default ("Uninitialized") when not
    // live, matching prior behavior exactly.
    const bim::viewport_bgfx::RendererBackendInfo backend_info =
        live_viewport_ready ? options.viewportWindow->EvidenceBackendInfo()
                            : renderer.BackendInfo();
    JsonValue backend_json = JsonValue::MakeObject();
    backend_json.Set("selected_renderer_backend", JsonValue::MakeString(backend_info.backendName));
    backend_json.Set("vendor_id", JsonValue::MakeInt(backend_info.vendorId));
    backend_json.Set("device_id", JsonValue::MakeInt(backend_info.deviceId));
    backend_json.Set("homogeneous_depth", JsonValue::MakeBool(backend_info.homogeneousDepth));
    if (options.live) {
        // Brief section 9: auto-selecting a different backend and calling
        // that PASS is forbidden - the live run's resolved backend must be
        // exactly the canonical bgfx Direct3D 11 backend name, "Direct3D 11"
        // (with the space - the string bgfx::getRendererName() actually
        // returns for RendererType::Direct3D11, and the exact value the
        // successful authoritative live evidence recorded under
        // selected_renderer_backend), or this is recorded as a failure, not
        // silently accepted. AA RD1.8-02: this gate previously compared
        // against "Direct3D11" (no space), which never matches the real
        // name, so an otherwise fully passing live run was forced to
        // overall_passed=false by this one comparison - the hidden
        // aggregate defect. The gate is not weakened: a ready live renderer
        // whose backend is anything other than "Direct3D 11" still fails.
        // homogeneous_depth (recorded above) is backend capability/
        // convention evidence and stays observational, never pass/fail.
        // `!renderer_ready` alone is kept: when the renderer never became
        // ready, backend_info carries no meaningful backend identity to
        // check, and that not-ready case is already recorded as a failure
        // elsewhere (the RecordResult(false) calls above), so this clause
        // exists only to avoid double-penalizing the same root cause a
        // second time.
        RecordResult(!renderer_ready || backend_info.backendName == "Direct3D 11");
    }
    root.Set("backend", std::move(backend_json));

    // ---- window/screen dimensions, DPR, screen identities (Brief 17/22) ----
    JsonValue window_json = JsonValue::MakeObject();
    window_json.Set("physical_width_px", JsonValue::MakeInt(options.widthPx));
    window_json.Set("physical_height_px", JsonValue::MakeInt(options.heightPx));
    window_json.Set("device_pixel_ratio", JsonValue::MakeDouble(options.devicePixelRatio));
    window_json.Set("logical_width_px",
                    JsonValue::MakeDouble(options.devicePixelRatio > 0.0
                                              ? options.widthPx / options.devicePixelRatio
                                              : 0.0));
    window_json.Set("logical_height_px",
                    JsonValue::MakeDouble(options.devicePixelRatio > 0.0
                                              ? options.heightPx / options.devicePixelRatio
                                              : 0.0));
    if (options.live) {
        window_json.Set("screen_count", JsonValue::MakeInt(options.screenCount));
        JsonValue screens_json = JsonValue::MakeArray();
        for (const std::string& identity : options.screenIdentities) {
            screens_json.Push(JsonValue::MakeString(identity));
        }
        window_json.Set("screen_identities", std::move(screens_json));
        window_json.Set("cross_monitor_differing_dpi_available",
                        JsonValue::MakeBool(options.crossMonitorDifferingDpiAvailable));
        if (!options.crossMonitorDifferingDpiAvailable) {
            // Brief section 17: "If the authoritative operator machine
            // does not provide two displays with different DPR, the
            // evidence must record NOT_AVAILABLE rather than inventing a
            // PASS. AC-019 remains pending until real differing-DPI live
            // evidence is obtained." - never treated as a failure of this
            // evidence run itself, only as a recorded gap.
            window_json.Set(
                "cross_monitor_dpi_transition_result",
                JsonValue::MakeNotAvailable(
                    "this run's environment did not provide two screens with differing DPR"));
        } else {
            window_json.Set(
                "cross_monitor_dpi_transition_result",
                JsonValue::MakeString(
                    "differing-DPI screens were present; the Operator's live run transcript is "
                    "the authoritative record of the actual observed transition, not this field"));
        }
        // AA Source Review Round 2 finding M11: minimize/restore and DPR-
        // transition evidence must actually exercise the locked live
        // surface/lifecycle path, not synthesize NOT_AVAILABLE regardless
        // of whether a live window exists. options.liveWindow is the same
        // realized, exposed QWindow whose native handle already backs
        // create_info/options.nativeWindowHandle above.
        //
        // AA Source Review Round 4 finding M13 point 2: passing on the
        // top-level QWindow::visibility() change alone was ruled
        // insufficient - it does not prove the production lifecycle_ was
        // actually driven, only that Qt/the platform reported a visibility
        // transition. This now additionally gates on
        // options.viewportWindow != nullptr and requires the actual
        // production ViewportWindow::EvidenceLifecycleState() to reach
        // Suspended after minimize and Ready (with EvidenceIsRendererReady())
        // after restore. If the production callbacks do not drive those
        // states, this fails closed rather than passing on visibility()
        // alone.
        //
        // AA Source Review Round 5 finding M16: the bridge that actually
        // drives those states is now ViewportWindow::eventFilter(), watching
        // the real top-level widget's QEvent::WindowStateChange (Round 4's
        // changeEvent() override watched this widget's OWN windowState(),
        // which is only meaningful when it is itself top-level - not true
        // once ViewportWindow is embedded as MainWindow's central child, the
        // real production shell as of this round; see
        // viewport_window.hpp/.cpp). options.liveWindow is now MainWindow's
        // own windowHandle() (see main.cpp) - the actual top-level window
        // eventFilter() observes - not ViewportWindow's own, as it was in
        // Round 3/4.
        if (options.liveWindow != nullptr && options.viewportWindow != nullptr) {
            const QWindow::Visibility visibility_before = options.liveWindow->visibility();
            const bim::viewport::ViewportState lifecycle_state_before =
                options.viewportWindow->EvidenceLifecycleState();
            options.liveWindow->showMinimized();
            PumpEventsFor(200);
            const bool reached_minimized = options.liveWindow->visibility() == QWindow::Minimized;
            const bim::viewport::ViewportState lifecycle_state_minimized =
                options.viewportWindow->EvidenceLifecycleState();
            const bool reached_suspended_lifecycle =
                lifecycle_state_minimized == bim::viewport::ViewportState::Suspended;
            options.liveWindow->showNormal();
            PumpEventsFor(200);
            const bool restored_to_windowed =
                options.liveWindow->visibility() != QWindow::Minimized &&
                options.liveWindow->visibility() != QWindow::Hidden;
            const bim::viewport::ViewportState lifecycle_state_restored =
                options.viewportWindow->EvidenceLifecycleState();
            const bool restored_ready_lifecycle =
                lifecycle_state_restored == bim::viewport::ViewportState::Ready &&
                options.viewportWindow->EvidenceIsRendererReady();
            const bool minimize_restore_ok = reached_minimized && restored_to_windowed &&
                                             reached_suspended_lifecycle &&
                                             restored_ready_lifecycle;

            JsonValue minimize_restore_json = JsonValue::MakeObject();
            minimize_restore_json.Set(
                "visibility_before", JsonValue::MakeInt(static_cast<long long>(visibility_before)));
            minimize_restore_json.Set("reached_minimized", JsonValue::MakeBool(reached_minimized));
            minimize_restore_json.Set("restored_to_windowed",
                                      JsonValue::MakeBool(restored_to_windowed));
            minimize_restore_json.Set(
                "lifecycle_state_before",
                JsonValue::MakeString(LifecycleStateName(lifecycle_state_before)));
            minimize_restore_json.Set(
                "lifecycle_state_minimized",
                JsonValue::MakeString(LifecycleStateName(lifecycle_state_minimized)));
            minimize_restore_json.Set(
                "lifecycle_state_restored",
                JsonValue::MakeString(LifecycleStateName(lifecycle_state_restored)));
            minimize_restore_json.Set("reached_suspended_lifecycle",
                                      JsonValue::MakeBool(reached_suspended_lifecycle));
            minimize_restore_json.Set("restored_ready_lifecycle",
                                      JsonValue::MakeBool(restored_ready_lifecycle));
            minimize_restore_json.Set(
                "note",
                JsonValue::MakeString(
                    "requires the production ViewportWindow::EvidenceLifecycleState() to actually "
                    "reach Suspended after minimize and Ready (renderer initialized) after "
                    "restore, "
                    "not merely a top-level QWindow::visibility() transition (AA Source Review "
                    "Round 4 finding M13 point 2); as of AA Source Review Round 5 finding M16, "
                    "options.liveWindow is the real top-level MainWindow (not ViewportWindow "
                    "itself), "
                    "and the lifecycle bridge observing it is ViewportWindow::eventFilter(), "
                    "proving "
                    "the actual production MainWindow-embedded child minimize/restore path"));
            minimize_restore_json.Set("passed", JsonValue::MakeBool(minimize_restore_ok));
            window_json.Set("minimize_restore_result", std::move(minimize_restore_json));
            RecordResult(minimize_restore_ok);

            if (options.crossMonitorDifferingDpiAvailable &&
                options.differingDpiScreen != nullptr) {
                // AA Source Review Round 4 finding M13 point 3: observe the
                // embedded production surface's own devicePixelRatio()
                // (EvidenceNativeSurface(), the actual bgfx-facing surface -
                // see viewport_window.hpp), not only the top-level window's
                // DPR - and require the renderer to remain
                // initialized/Ready after the transition, not merely that
                // some DPR value changed somewhere.
                QWindow* const native_surface = options.viewportWindow->EvidenceNativeSurface();
                const double dpr_before = (native_surface != nullptr)
                                              ? native_surface->devicePixelRatio()
                                              : options.liveWindow->devicePixelRatio();
                QScreen* const original_screen = options.liveWindow->screen();
                options.liveWindow->setScreen(options.differingDpiScreen);
                options.liveWindow->setGeometry(options.differingDpiScreen->geometry());
                PumpEventsFor(300);
                const double dpr_after = (native_surface != nullptr)
                                             ? native_surface->devicePixelRatio()
                                             : options.liveWindow->devicePixelRatio();
                const bool dpr_actually_changed = dpr_after != dpr_before;
                const bool renderer_survived_transition =
                    options.viewportWindow->EvidenceIsRendererReady() &&
                    options.viewportWindow->EvidenceLifecycleState() ==
                        bim::viewport::ViewportState::Ready;
                const bool dpr_change_ok = dpr_actually_changed && renderer_survived_transition;

                // Return the window to its original screen so the
                // resize/surface-recreation evidence still ahead in this
                // function is not left running against the wrong monitor.
                if (original_screen != nullptr) {
                    options.liveWindow->setScreen(original_screen);
                    options.liveWindow->setGeometry(original_screen->geometry());
                    PumpEventsFor(200);
                }

                JsonValue dpr_json = JsonValue::MakeObject();
                dpr_json.Set("device_pixel_ratio_before", JsonValue::MakeDouble(dpr_before));
                dpr_json.Set("device_pixel_ratio_after", JsonValue::MakeDouble(dpr_after));
                dpr_json.Set(
                    "observed_on",
                    JsonValue::MakeString(
                        native_surface != nullptr
                            ? "embedded production ViewportSurface (EvidenceNativeSurface())"
                            : "top-level liveWindow (EvidenceNativeSurface() was null)"));
                dpr_json.Set("renderer_survived_transition",
                             JsonValue::MakeBool(renderer_survived_transition));
                dpr_json.Set("passed", JsonValue::MakeBool(dpr_change_ok));
                window_json.Set("dpr_change_result", std::move(dpr_json));
                RecordResult(dpr_change_ok);
            } else {
                // Brief section 17's own AC-019 carve-out: only the
                // differing-DPI cross-monitor case may stay NOT_AVAILABLE
                // when that specific hardware genuinely is not present -
                // the live window itself is real and was already exercised
                // by the minimize/restore check above.
                window_json.Set(
                    "dpr_change_result",
                    JsonValue::MakeNotAvailable(
                        "this run's environment did not provide two screens with differing DPR "
                        "to move the live window between (Brief section 17 AC-019 carve-out)"));
            }
        } else {
            // No live window and/or no production ViewportWindow at all:
            // already recorded as a failure above (the on_surface_available
            // / backend-identity RecordResult(false) calls) - these two
            // fields stay NOT_AVAILABLE for that same root cause rather than
            // each independently failing the run a second time. (AA Source
            // Review Round 4 finding M13 point 2 added the
            // options.viewportWindow != nullptr gate above, since these
            // checks now require EvidenceLifecycleState().)
            window_json.Set("minimize_restore_result",
                            JsonValue::MakeNotAvailable("no realized/exposed live window and/or "
                                                        "production ViewportWindow was available "
                                                        "for this live evidence run"));
            window_json.Set("dpr_change_result",
                            JsonValue::MakeNotAvailable("no realized/exposed live window and/or "
                                                        "production ViewportWindow was available "
                                                        "for this live evidence run"));
        }
    } else {
        window_json.Set("screen_count", JsonValue::MakeNotAvailable("headless run: no screens"));
        window_json.Set("cross_monitor_dpi_transition_result",
                        JsonValue::MakeNotAvailable("headless run: no screens"));
        window_json.Set("minimize_restore_result",
                        JsonValue::MakeNotAvailable("headless run: no window to minimize/restore"));
        window_json.Set("dpr_change_result",
                        JsonValue::MakeNotAvailable("headless run: no screen DPR to change"));
    }
    root.Set("window", std::move(window_json));

    // ---- scenes: V01-V05, vertex/index/triangle counts (Brief 14/22) ----
    // AA Source Review Round 3 finding M13: live mode drives each scene
    // through RunOneLiveScene() (the production ViewportWindow's own
    // renderer_/camera_), never RunOneScene()'s local `renderer` - which
    // stays uninitialized for the entire live path, exactly as intended.
    JsonValue scenes_json = JsonValue::MakeArray();
    if (renderer_ready) {
        if (options.live) {
            for (SpikeSceneId id : kAllSceneIds) {
                RecordResult(RunOneLiveScene(id, *options.viewportWindow, scenes_json));
            }
        } else {
            for (SpikeSceneId id : kAllSceneIds) {
                RecordResult(RunOneScene(id, renderer, camera, scenes_json));
            }
        }
    }
    root.Set("scenes", std::move(scenes_json));

    // ---- resize result ----
    // AA Source Review Round 3 finding M13 (original): live-mode resize
    // evidence resizes the real top-level window, exercising the embedded
    // ViewportWindow's actual resizeEvent()-driven
    // HandlePhysicalSizeOrDprChange() path (viewport_window.cpp) - not
    // Renderer::Resize() called directly with no real surface behind it.
    //
    // AA Source Review Round 5 finding M16 (necessary consequence): Round
    // 3/4's version called QWidget::resize() directly on
    // options.viewportWindow because it WAS the top-level window at the
    // time. Since M16 now embeds it as MainWindow's central widget (see
    // main_window.cpp), its geometry is owned by QMainWindow's own layout -
    // a direct resize() on a layout-managed central widget fights that
    // layout manager rather than exercising a realistic resize path. This
    // now resizes the real top-level QWindow (options.liveWindow) instead,
    // letting Qt's normal layout cascade resize the embedded ViewportWindow
    // child exactly as a real interactive user resizing the application
    // window would - still driving the same real resizeEvent()-driven
    // HandlePhysicalSizeOrDprChange() path, just reached the way MainWindow
    // actually reaches it.
    if (renderer_ready && options.live) {
        const QSize logical_before = options.viewportWindow->size();
        JsonValue resize_json = JsonValue::MakeObject();
        bool resize_ok = false;
        if (options.liveWindow != nullptr) {
            options.liveWindow->resize(options.liveWindow->width() + 96,
                                       options.liveWindow->height() + 72);
            PumpEventsFor(200);
            const QSize logical_after = options.viewportWindow->size();
            const bool viewport_size_changed = logical_after != logical_before;
            resize_ok = options.viewportWindow->EvidenceIsRendererReady() && viewport_size_changed;
            resize_json.Set("top_level_resized", JsonValue::MakeBool(true));
            resize_json.Set("viewport_logical_size_changed",
                            JsonValue::MakeBool(viewport_size_changed));
            resize_json.Set(
                "note",
                JsonValue::MakeString(
                    "driven by an actual QWindow::resize() on the real top-level window "
                    "(MainWindow - "
                    "AA Source Review Round 5 finding M16), letting Qt's own layout cascade resize "
                    "the "
                    "embedded ViewportWindow central widget, exercising its real "
                    "resizeEvent()-driven "
                    "HandlePhysicalSizeOrDprChange() path - not a direct QWidget::resize() on a "
                    "layout-managed central widget"));
        } else {
            // Should not happen whenever renderer_ready && options.live,
            // since that already implies a realized top-level window -
            // recorded distinctly rather than silently reusing the
            // no-live-window NOT_AVAILABLE wording used elsewhere in this
            // function.
            resize_json.Set("top_level_resized", JsonValue::MakeBool(false));
            resize_json.Set(
                "note",
                JsonValue::MakeString(
                    "no live top-level window (options.liveWindow) was available to resize"));
        }
        resize_json.Set("passed", JsonValue::MakeBool(resize_ok));
        root.Set("resize_result", std::move(resize_json));
        RecordResult(resize_ok);
    } else if (renderer_ready) {
        const std::uint32_t resized_width = options.widthPx + 128;
        const std::uint32_t resized_height = options.heightPx + 96;
        const bim::viewport::Status resize_status = renderer.Resize(resized_width, resized_height);
        root.Set("resize_result", MakeResultValue(resize_status));
        RecordResult(static_cast<bool>(resize_status));
    } else {
        root.Set("resize_result", JsonValue::MakeNotAvailable("renderer never initialized"));
    }

    // ---- surface recreation result ----
    // AA Source Review Round 4 finding M13 (final closure): the prior
    // ViewportWindow::EvidenceForceSurfaceRecreation() call was a synthetic
    // proof - it invoked the private TeardownRenderer()/
    // EnsureRendererInitialized() methods directly, proving only that those
    // two methods behave correctly when called, not that the real
    // Qt-driven QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed/re-expose
    // path actually exercises them. This now calls
    // EvidenceDestroyNativeSurface() (the real QWindow::destroy() on the
    // production surface_ - see viewport_window.hpp/.cpp), pumps the event
    // loop, and requires the INTERMEDIATE state to actually reach
    // SurfaceUnavailable with the renderer down before calling
    // EvidenceRecreateNativeSurface() (the real QWindow::create()+
    // re-expose), pumping again, and requiring the FINAL state to reach
    // Ready with the renderer initialized. Neither intermediate nor final
    // state is assumed - both are observed via EvidenceLifecycleState()/
    // EvidenceIsRendererReady(), the same accessors the rest of this file
    // uses, so a platform on which destroy()/create() do not actually drive
    // the production callbacks fails this closed rather than passing.
    if (renderer_ready && options.live) {
        const bim::viewport::ViewportState state_before_destroy =
            options.viewportWindow->EvidenceLifecycleState();
        options.viewportWindow->EvidenceDestroyNativeSurface();
        PumpEventsFor(200);
        const bim::viewport::ViewportState state_after_destroy =
            options.viewportWindow->EvidenceLifecycleState();
        const bool intermediate_ok =
            state_after_destroy == bim::viewport::ViewportState::SurfaceUnavailable &&
            !options.viewportWindow->EvidenceIsRendererReady();

        options.viewportWindow->EvidenceRecreateNativeSurface();
        PumpEventsFor(300);
        const bim::viewport::ViewportState state_after_recreate =
            options.viewportWindow->EvidenceLifecycleState();
        const bool final_ok = state_after_recreate == bim::viewport::ViewportState::Ready &&
                              options.viewportWindow->EvidenceIsRendererReady();

        const bool recreation_ok = intermediate_ok && final_ok;

        JsonValue recreation_json = JsonValue::MakeObject();
        recreation_json.Set("state_before_destroy",
                            JsonValue::MakeString(LifecycleStateName(state_before_destroy)));
        recreation_json.Set("state_after_destroy",
                            JsonValue::MakeString(LifecycleStateName(state_after_destroy)));
        recreation_json.Set("intermediate_surface_unavailable_and_renderer_down",
                            JsonValue::MakeBool(intermediate_ok));
        recreation_json.Set("state_after_recreate",
                            JsonValue::MakeString(LifecycleStateName(state_after_recreate)));
        recreation_json.Set("final_ready_and_renderer_initialized", JsonValue::MakeBool(final_ok));
        recreation_json.Set("passed", JsonValue::MakeBool(recreation_ok));
        recreation_json.Set(
            "note",
            JsonValue::MakeString(
                "driven by "
                "ViewportWindow::EvidenceDestroyNativeSurface()/EvidenceRecreateNativeSurface()"
                ", which call the real QWindow::destroy()/create() on the production surface_ so "
                "Qt "
                "itself delivers a genuine QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed and a "
                "genuine exposeEvent to the existing onSurfaceAboutToBeDestroyed/onExposedOrHidden "
                "callbacks - not a direct call to TeardownRenderer()/EnsureRendererInitialized() "
                "(AA "
                "Source Review Round 4 finding M13 final closure)"));
        root.Set("surface_recreation_result", std::move(recreation_json));
        RecordResult(recreation_ok);
    } else if (renderer_ready) {
        const bim::viewport::Status shutdown_status = renderer.Shutdown();
        const bim::viewport::Status reinit_status = renderer.Initialize(create_info);
        JsonValue recreation_json = JsonValue::MakeObject();
        recreation_json.Set("shutdown", MakeResultValue(shutdown_status));
        recreation_json.Set("reinitialize", MakeResultValue(reinit_status));
        root.Set("surface_recreation_result", std::move(recreation_json));
        RecordResult(static_cast<bool>(shutdown_status) && static_cast<bool>(reinit_status));
    } else {
        root.Set("surface_recreation_result",
                 JsonValue::MakeNotAvailable("renderer never initialized"));
    }

    // ---- repeatability (emission point) ----
    // RD1.7: the repeatability cycles themselves now run near the top of
    // this function, BEFORE the primary headless renderer is initialized
    // (see the "repeatability" section there for why - no overlapping bgfx
    // Renderer ownership). Only the emission into the output object stays
    // here, at its original position, so the JSON key order is unchanged.
    root.Set("repeatability", std::move(repeatability_json));

    // ---- remaining lifecycle transitions ----
    // AA Source Review Round 3 finding M13: live mode no longer drives a
    // throwaway local ViewportLifecycle through on_suspend/on_resume/
    // begin_shutdown/complete_shutdown - that would be exactly the "second
    // lifecycle architecture" this finding forbids, disconnected from the
    // production ViewportWindow's own lifecycle_ member. on_suspend/
    // on_resume are already exercised for real by the minimize/restore
    // evidence above (ViewportSurface's visibilityChanged callback drives
    // lifecycle_.OnSuspend()/OnResume() in production - see
    // viewport_window.cpp's constructor). Shutdown is exercised here by
    // actually calling close() on the production ViewportWindow, driving
    // its real closeEvent()-triggered TeardownRenderer()+BeginShutdown()+
    // CompleteShutdown() sequence.
    JsonValue lifecycle_tail_json = JsonValue::MakeObject();
    if (options.live) {
        if (live_viewport_ready) {
            const bim::viewport::ViewportState state_before_close =
                options.viewportWindow->EvidenceLifecycleState();
            options.viewportWindow->close();
            PumpEventsFor(200);
            const bim::viewport::ViewportState state_after_close =
                options.viewportWindow->EvidenceLifecycleState();
            const bool reached_destroyed =
                state_after_close == bim::viewport::ViewportState::Destroyed;
            lifecycle_tail_json.Set("state_before_close",
                                    JsonValue::MakeString(LifecycleStateName(state_before_close)));
            lifecycle_tail_json.Set("state_after_close",
                                    JsonValue::MakeString(LifecycleStateName(state_after_close)));
            lifecycle_tail_json.Set("reached_destroyed", JsonValue::MakeBool(reached_destroyed));
            lifecycle_tail_json.Set(
                "note",
                JsonValue::MakeString(
                    "driven by calling close() on the actual production ViewportWindow, exercising "
                    "its real closeEvent()-driven TeardownRenderer()+BeginShutdown()+"
                    "CompleteShutdown() path - not a second, separately-driven ViewportLifecycle "
                    "instance"));
            RecordResult(reached_destroyed);
        } else {
            lifecycle_tail_json.Set(
                "on_suspend",
                JsonValue::MakeNotAvailable(
                    "no realized/exposed production ViewportWindow was available for the live "
                    "evidence run"));
            lifecycle_tail_json.Set(
                "on_resume",
                JsonValue::MakeNotAvailable(
                    "no realized/exposed production ViewportWindow was available for the live "
                    "evidence run"));
            lifecycle_tail_json.Set(
                "shutdown",
                JsonValue::MakeNotAvailable(
                    "no realized/exposed production ViewportWindow was available for the live "
                    "evidence run"));
        }
    } else {
        lifecycle_tail_json.Set("on_suspend", MakeResultValue(lifecycle.OnSuspend()));
        lifecycle_tail_json.Set("on_resume", MakeResultValue(lifecycle.OnResume()));
        lifecycle_tail_json.Set("begin_shutdown", MakeResultValue(lifecycle.BeginShutdown()));
        lifecycle_tail_json.Set("complete_shutdown", MakeResultValue(lifecycle.CompleteShutdown()));
        RecordResult(lifecycle.State() == bim::viewport::ViewportState::Destroyed);
    }
    root.Set("lifecycle_tail", std::move(lifecycle_tail_json));

    // Headless-only: shuts down this function's own local `renderer`. Live
    // mode's renderer_ is owned by the production ViewportWindow (already
    // torn down above via close(), or torn down by its own destructor when
    // main.cpp's evidence_window goes out of scope) - this file must not
    // shut it down a second time, so `renderer` (never initialized for the
    // live path - see above) correctly leaves this a no-op there.
    if (renderer.IsInitialized()) {
        const bim::viewport::Status final_shutdown = renderer.Shutdown();
        root.Set("final_shutdown_result", MakeResultValue(final_shutdown));
        RecordResult(static_cast<bool>(final_shutdown));
    }

    // ---- ray result ----
    {
        const auto ray_result = bim::viewport::ScreenToWorldRay(
            camera, static_cast<double>(options.widthPx) / 2.0,
            static_cast<double>(options.heightPx) / 2.0, options.widthPx, options.heightPx);
        JsonValue ray_json = JsonValue::MakeObject();
        ray_json.Set("center_pixel_passed", JsonValue::MakeBool(ray_result.IsOk()));
        root.Set("ray_result", std::move(ray_json));
        RecordResult(ray_result.IsOk());
    }

    // ---- frame timing samples: observational only, never pass/fail
    // (Brief section 14: "No performance threshold is pass/fail. Frame
    // timings are observational evidence only.") ----
    root.Set("frame_timing_note",
             JsonValue::MakeString(
                 "Per-cycle duration_ms_observational values under repeatability.cycles are "
                 "observational only and never contribute to overall_passed."));

    root.Set("overall_passed", JsonValue::MakeBool(overall_passed));

    std::error_code ec;
    const std::filesystem::path json_path(options.jsonOutputPath);
    if (json_path.has_parent_path()) {
        std::filesystem::create_directories(json_path.parent_path(), ec);
    }

    std::ofstream out(options.jsonOutputPath);
    if (!out.is_open()) {
        return 1;
    }
    root.WriteTo(out, 0);
    out << '\n';
    out.close();

    return overall_passed ? 0 : 1;
}

} // namespace bim::desktop
