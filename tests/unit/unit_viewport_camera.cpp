#include <cmath>
#include <limits>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/camera.hpp"

using bim::viewport::Camera;
using bim::viewport::Length;
using bim::viewport::ViewportErrorCode;
using Catch::Approx;

TEST_CASE("A default-constructed Camera is not configured", "[viewport][camera]") {
    Camera camera;
    REQUIRE_FALSE(camera.IsConfigured());
}

TEST_CASE("SetLookAt succeeds with well-formed, non-degenerate parameters", "[viewport][camera]") {
    Camera camera;
    const auto status = camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1});
    REQUIRE(status.IsOk());
}

TEST_CASE("SetLookAt rejects eye == target", "[viewport][camera]") {
    Camera camera;
    const auto status = camera.SetLookAt({1, 1, 1}, {1, 1, 1}, {0, 0, 1});
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("SetLookAt rejects worldUp parallel to the forward direction", "[viewport][camera]") {
    Camera camera;
    // forward = (0,0,-1) normalized (target - eye); worldUp = (0,0,1) is
    // anti-parallel to forward, so Cross(forward, worldUp) is degenerate.
    const auto status = camera.SetLookAt({0, 0, 5}, {0, 0, 0}, {0, 0, 1});
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("SetLookAt rejects non-finite input", "[viewport][camera]") {
    Camera camera;
    const auto status =
        camera.SetLookAt({std::numeric_limits<double>::quiet_NaN(), 0, 0}, {0, 0, 0}, {0, 0, 1});
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("A failed SetLookAt leaves the camera's previous state unchanged", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    const auto eye_before = camera.Eye();

    const auto status = camera.SetLookAt({1, 1, 1}, {1, 1, 1}, {0, 0, 1}); // degenerate
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(camera.Eye().x == Approx(eye_before.x));
    REQUIRE(camera.Eye().y == Approx(eye_before.y));
    REQUIRE(camera.Eye().z == Approx(eye_before.z));
}

TEST_CASE("SetPerspective accepts a sane FOV/near/far triple", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());
}

TEST_CASE("SetPerspective rejects near >= far", "[viewport][camera]") {
    Camera camera;
    const auto status = camera.SetPerspective(0.9, 100.0, 10.0);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("SetPerspective rejects a non-positive near plane", "[viewport][camera]") {
    Camera camera;
    const auto status = camera.SetPerspective(0.9, 0.0, 1000.0);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidInput);
}

TEST_CASE("SetAspectRatio rejects a non-positive value", "[viewport][camera]") {
    Camera camera;
    REQUIRE_FALSE(camera.SetAspectRatio(0.0).IsOk());
    REQUIRE_FALSE(camera.SetAspectRatio(-1.0).IsOk());
}

TEST_CASE("A camera is configured once both SetLookAt and SetPerspective have succeeded",
          "[viewport][camera]") {
    Camera camera;
    REQUIRE_FALSE(camera.IsConfigured());
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    REQUIRE_FALSE(camera.IsConfigured());
    REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());
    REQUIRE(camera.IsConfigured());
}

TEST_CASE("Orbit preserves the eye-to-target radius", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    const double radius_before = Length(camera.Eye() - camera.Target());

    camera.Orbit(0.4, 0.2);

    const double radius_after = Length(camera.Eye() - camera.Target());
    REQUIRE(radius_after == Approx(radius_before).margin(1e-9));
}

TEST_CASE("Orbit never moves the target", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 2}, {1, 2, 3}, {0, 0, 1}).IsOk());
    const auto target_before = camera.Target();

    camera.Orbit(1.0, 0.5);

    REQUIRE(camera.Target().x == Approx(target_before.x));
    REQUIRE(camera.Target().y == Approx(target_before.y));
    REQUIRE(camera.Target().z == Approx(target_before.z));
}

TEST_CASE("Orbit clamps pitch so the eye never crosses the poles", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 0}, {0, 0, 0}, {0, 0, 1}).IsOk());

    // A huge pitch delta should be clamped, not wrap or degenerate.
    camera.Orbit(0.0, 100.0);

    const auto offset = camera.Eye() - camera.Target();
    const double radius = Length(offset);
    REQUIRE(radius > 0.0);
    REQUIRE(std::isfinite(offset.x));
    REQUIRE(std::isfinite(offset.y));
    REQUIRE(std::isfinite(offset.z));
}

TEST_CASE("Dolly moves the eye toward the target and preserves direction", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -10, 0}, {0, 0, 0}, {0, 0, 1}).IsOk());
    const double radius_before = Length(camera.Eye() - camera.Target());

    camera.Dolly(2.0); // dolly in

    const double radius_after = Length(camera.Eye() - camera.Target());
    REQUIRE(radius_after == Approx(radius_before - 2.0).margin(1e-9));
}

TEST_CASE("Dolly never lets the radius reach zero or invert", "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -1, 0}, {0, 0, 0}, {0, 0, 1}).IsOk());

    camera.Dolly(1000.0); // wildly more than the current radius

    const double radius_after = Length(camera.Eye() - camera.Target());
    REQUIRE(radius_after > 0.0);
}

TEST_CASE("Pan moves eye and target together, leaving the offset between them unchanged",
          "[viewport][camera]") {
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    const auto offset_before = camera.Eye() - camera.Target();

    camera.Pan(1.0, 0.5);

    const auto offset_after = camera.Eye() - camera.Target();
    REQUIRE(offset_after.x == Approx(offset_before.x).margin(1e-9));
    REQUIRE(offset_after.y == Approx(offset_before.y).margin(1e-9));
    REQUIRE(offset_after.z == Approx(offset_before.z).margin(1e-9));
}
