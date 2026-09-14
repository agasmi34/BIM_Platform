#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/ray.hpp"

using bim::viewport::Camera;
using bim::viewport::Length;
using bim::viewport::ScreenToWorldRay;
using bim::viewport::ViewportErrorCode;
using Catch::Approx;

namespace {

Camera MakeConfiguredCamera() {
    Camera camera;
    // RD1.3-02: Camera::SetLookAt/SetPerspective/SetAspectRatio are all
    // [[nodiscard]] Status - MSVC's /WX build promotes the discarded-result
    // warning (C4834) to a hard error (C2220). Each call is also, in this
    // helper, an implicit precondition every TEST_CASE below depends on
    // (that MakeConfiguredCamera() actually returns a validly-configured
    // Camera - see Camera::IsConfigured()); asserting IsOk() here both
    // consumes the nodiscard result and gives a meaningful failure at the
    // real point of breakage if a future change to these fixed, in-range
    // literal arguments were ever to make one of them invalid, rather than
    // silently returning an unconfigured Camera that would then fail every
    // TEST_CASE below with a confusing, unrelated-looking assertion.
    REQUIRE(camera.SetLookAt({0, -5, 0}, {0, 0, 0}, {0, 0, 1}).IsOk());
    REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());
    REQUIRE(camera.SetAspectRatio(1.0).IsOk());
    return camera;
}

} // namespace

TEST_CASE("ScreenToWorldRay rejects a zero-sized viewport", "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto result = ScreenToWorldRay(camera, 100.0, 100.0, 0, 720);
    REQUIRE_FALSE(result.IsOk());
    REQUIRE(result.Code() == ViewportErrorCode::InvalidState);
}

TEST_CASE("ScreenToWorldRay rejects non-finite pixel coordinates", "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto result =
        ScreenToWorldRay(camera, std::numeric_limits<double>::infinity(), 100.0, 1280, 720);
    REQUIRE_FALSE(result.IsOk());
    REQUIRE(result.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("ScreenToWorldRay rejects an unconfigured camera", "[viewport][ray]") {
    const Camera camera; // never SetLookAt/SetPerspective
    const auto result = ScreenToWorldRay(camera, 640.0, 360.0, 1280, 720);
    REQUIRE_FALSE(result.IsOk());
    REQUIRE(result.Code() == ViewportErrorCode::InvalidState);
}

TEST_CASE("ScreenToWorldRay at the viewport center points toward the camera forward direction",
          "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto result = ScreenToWorldRay(camera, 640.0, 360.0, 1280, 720);
    REQUIRE(result.IsOk());

    const auto ray = result.Value();
    const auto expected_forward = camera.Target() - camera.Eye();
    const double expected_length = Length(expected_forward);
    const auto expected_forward_normalized = bim::viewport::Vector3{
        expected_forward.x / expected_length, expected_forward.y / expected_length,
        expected_forward.z / expected_length};

    REQUIRE(ray.direction.x == Approx(expected_forward_normalized.x).margin(1e-6));
    REQUIRE(ray.direction.y == Approx(expected_forward_normalized.y).margin(1e-6));
    REQUIRE(ray.direction.z == Approx(expected_forward_normalized.z).margin(1e-6));
}

TEST_CASE("ScreenToWorldRay's origin equals the camera's eye", "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto result = ScreenToWorldRay(camera, 200.0, 150.0, 1280, 720);
    REQUIRE(result.IsOk());

    const auto ray = result.Value();
    REQUIRE(ray.origin.x == Approx(camera.Eye().x));
    REQUIRE(ray.origin.y == Approx(camera.Eye().y));
    REQUIRE(ray.origin.z == Approx(camera.Eye().z));
}

TEST_CASE("ScreenToWorldRay returns a unit-length direction", "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto result = ScreenToWorldRay(camera, 50.0, 700.0, 1280, 720);
    REQUIRE(result.IsOk());

    const auto ray = result.Value();
    REQUIRE(Length(ray.direction) == Approx(1.0).margin(1e-6));
}

TEST_CASE("ScreenToWorldRay at the top edge points more upward (+Z) than at the bottom edge",
          "[viewport][ray]") {
    const Camera camera = MakeConfiguredCamera();
    const auto top = ScreenToWorldRay(camera, 640.0, 0.0, 1280, 720);
    const auto bottom = ScreenToWorldRay(camera, 640.0, 720.0, 1280, 720);
    REQUIRE(top.IsOk());
    REQUIRE(bottom.IsOk());

    // Top-of-screen pixel (physical Y == 0) maps to NDC Y == +1 (up), which
    // should push the ray direction's Z component higher than the
    // bottom-of-screen pixel's - the top-left-origin -> Y-up NDC flip
    // (ray.cpp) is exercised here.
    REQUIRE(top.Value().direction.z > bottom.Value().direction.z);
}
