#pragma once

#include "bim/viewport/error.hpp"
#include "bim/viewport/math.hpp"

// bim/viewport/camera.hpp - the renderer-neutral camera contract
// (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, sections 5-6).
//
// Stores semantic parameters only - eye, target, world-up, vertical FOV,
// aspect ratio, near/far - never a view or projection matrix (Brief section
// 5: "The public API does not expose a matrix with an assumed backend
// clip-space depth convention. Backend view/projection conversion belongs
// entirely to bim::viewport_bgfx."). All values are double precision (Brief
// section 5 precision contract). Forbidden here: Qt, bgfx, D3D, Windows
// native types, OCCT, BIM semantic/model/query types.

namespace bim::viewport {

class Camera {
public:
    Camera() noexcept = default;

    // Validates and, on success, replaces eye/target/world-up together
    // (Brief section 6: "eye/target/up values finite; eye != target; up not
    // degenerate/parallel to forward"). On InvalidInput, the camera's
    // previous state is left unchanged.
    [[nodiscard]] Status SetLookAt(Vector3 eye, Vector3 target, Vector3 worldUp) noexcept;

    // Validates and, on success, replaces the perspective parameters (Brief
    // section 6: "0 < near < far"; "vertical FOV finite and in a sensible
    // open interval" - taken here as (0, pi) exclusive, since a FOV of 0 or
    // >= pi radians is not a usable perspective frustum).
    [[nodiscard]] Status SetPerspective(double verticalFovRadians, double nearPlane,
                                        double farPlane) noexcept;

    // Validates and, on success, replaces the aspect ratio (Brief section 6:
    // "aspect finite and > 0"). The viewport lifecycle (see lifecycle.hpp)
    // governs when a caller may reasonably compute an aspect ratio at all -
    // this method itself only validates the value it is given.
    [[nodiscard]] Status SetAspectRatio(double aspect) noexcept;

    // Orbit rotates eye around target at the current fixed radius; target
    // never moves (Brief section 6: "Orbit: moves eye around target; target
    // remains fixed."). Infallible: internally clamped so pitch never
    // reaches the poles (avoiding a degenerate up-vector) and the orbit
    // radius is preserved exactly.
    void Orbit(double deltaYawRadians, double deltaPitchRadians) noexcept;

    // Pan translates eye and target together along the camera's current
    // right/up basis (Brief section 6: "Pan: moves eye and target together
    // along camera right/up."). Infallible.
    void Pan(double deltaRight, double deltaUp) noexcept;

    // Dolly moves eye toward/away from target along the view direction,
    // changing the orbit radius (Brief section 6: "Wheel zoom: dolly toward
    // /away from target; not FOV zoom in P0-T003."). Clamped so the radius
    // never reaches zero or crosses through target. Infallible.
    void Dolly(double deltaDistance) noexcept;

    [[nodiscard]] Vector3 Eye() const noexcept { return eye_; }
    [[nodiscard]] Vector3 Target() const noexcept { return target_; }
    [[nodiscard]] Vector3 WorldUp() const noexcept { return world_up_; }
    [[nodiscard]] double VerticalFovRadians() const noexcept { return vertical_fov_radians_; }
    [[nodiscard]] double AspectRatio() const noexcept { return aspect_ratio_; }
    [[nodiscard]] double NearPlane() const noexcept { return near_plane_; }
    [[nodiscard]] double FarPlane() const noexcept { return far_plane_; }

    // True once SetLookAt and SetPerspective have both succeeded at least
    // once (SetAspectRatio is not required for IsConfigured(), since a
    // caller may reasonably configure look-at/perspective before the first
    // known viewport size). ray.cpp treats an unconfigured camera as
    // InvalidState.
    [[nodiscard]] bool IsConfigured() const noexcept { return look_at_set_ && perspective_set_; }

private:
    Vector3 eye_{0.0, -5.0, 2.0};
    Vector3 target_{0.0, 0.0, 0.0};
    Vector3 world_up_{0.0, 0.0, 1.0};   // +Z up (Brief section 4/6)
    double vertical_fov_radians_ = 0.9; // ~51.6 degrees, an arbitrary-but-sane default
    double aspect_ratio_ = 1.0;
    double near_plane_ = 0.01;
    double far_plane_ = 10000.0;
    bool look_at_set_ = false;
    bool perspective_set_ = false;

    static constexpr double kMinOrbitRadius = 1.0e-3;
    static constexpr double kMaxPitchRadians =
        1.55334; // ~89 degrees; keeps a well-defined up vector
};

} // namespace bim::viewport
