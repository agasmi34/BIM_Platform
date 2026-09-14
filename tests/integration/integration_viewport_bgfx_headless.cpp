#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/camera.hpp"
#include "bim/viewport_bgfx/renderer.hpp"

// integration_viewport_bgfx_headless.cpp - exercises bim::viewport_bgfx::
// Renderer end-to-end against bgfx's headless/noop backend (Implementation
// Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 12: "headless/test split" -
// this test must run under CTest with no live D3D11 device and no visible
// window, exactly like CI). UNVERIFIED: authored against the bgfx API as
// documented, never compiled/run in this session (no execution channel) -
// see docs/evidence/P0-T003/CLAUDE_HANDOVER.md.

using bim::viewport::Camera;
using bim::viewport_bgfx::Renderer;
using bim::viewport_bgfx::RendererCreateInfo;

namespace {

Camera MakeConfiguredCamera() {
    Camera camera;
    // RD1.3-02: Camera::SetLookAt/SetPerspective/SetAspectRatio are all
    // [[nodiscard]] Status - MSVC's /WX build promotes the discarded-result
    // warning (C4834) to a hard error (C2220). As throughout the rest of
    // this file (e.g. REQUIRE(renderer.Initialize(info).IsOk()) below),
    // each call's result is asserted rather than discarded: this helper's
    // whole contract is to hand back a validly-configured Camera, and every
    // TEST_CASE below that calls it relies on that being true, so a failure
    // here should surface at its real point of breakage, not as a
    // confusing, unrelated-looking failure two calls later.
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());
    REQUIRE(camera.SetAspectRatio(16.0 / 9.0).IsOk());
    return camera;
}

} // namespace

TEST_CASE("Renderer::Initialize succeeds in headless mode without a native window handle",
          "[viewport_bgfx][headless]") {
    Renderer renderer;
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 1280;
    info.heightPx = 720;

    REQUIRE(renderer.Initialize(info).IsOk());
    REQUIRE(renderer.IsInitialized());

    REQUIRE(renderer.Shutdown().IsOk());
    REQUIRE_FALSE(renderer.IsInitialized());
}

TEST_CASE("Renderer::Initialize rejects a zero-sized headless viewport",
          "[viewport_bgfx][headless]") {
    Renderer renderer;
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 0;
    info.heightPx = 720;

    const auto status = renderer.Initialize(info);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == bim::viewport::ViewportErrorCode::InvalidInput);
}

TEST_CASE("A double Initialize() is refused with InvalidState", "[viewport_bgfx][headless]") {
    Renderer renderer;
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;

    REQUIRE(renderer.Initialize(info).IsOk());
    const auto second = renderer.Initialize(info);
    REQUIRE_FALSE(second.IsOk());
    REQUIRE(second.Code() == bim::viewport::ViewportErrorCode::InvalidState);

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("Shutdown() on a never-initialized Renderer is refused with InvalidState",
          "[viewport_bgfx][headless]") {
    Renderer renderer;
    const auto status = renderer.Shutdown();
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == bim::viewport::ViewportErrorCode::InvalidState);
}

TEST_CASE("RenderFrame requires an initialized renderer and a configured camera",
          "[viewport_bgfx][headless]") {
    Renderer renderer;
    const Camera configured_camera = MakeConfiguredCamera();

    SECTION("uninitialized renderer") {
        const auto status = renderer.RenderFrame(configured_camera, nullptr, 0);
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(status.Code() == bim::viewport::ViewportErrorCode::InvalidState);
    }

    SECTION("unconfigured camera") {
        RendererCreateInfo info;
        info.headless = true;
        info.widthPx = 640;
        info.heightPx = 480;
        REQUIRE(renderer.Initialize(info).IsOk());

        const Camera unconfigured_camera;
        const auto status = renderer.RenderFrame(unconfigured_camera, nullptr, 0);
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(status.Code() == bim::viewport::ViewportErrorCode::InvalidState);

        REQUIRE(renderer.Shutdown().IsOk());
    }
}

TEST_CASE("RenderFrame with zero meshes succeeds (a clear-only frame)",
          "[viewport_bgfx][headless]") {
    Renderer renderer;
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;
    REQUIRE(renderer.Initialize(info).IsOk());

    const Camera camera = MakeConfiguredCamera();
    REQUIRE(renderer.RenderFrame(camera, nullptr, 0).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("Resize succeeds on a live headless renderer and fails when not initialized",
          "[viewport_bgfx][headless]") {
    Renderer renderer;

    REQUIRE_FALSE(renderer.Resize(800, 600).IsOk());

    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;
    REQUIRE(renderer.Initialize(info).IsOk());

    REQUIRE(renderer.Resize(1920, 1080).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}
