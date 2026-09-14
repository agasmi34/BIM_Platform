#include "bim/viewport/camera.hpp"

#include <algorithm>
#include <cmath>

namespace bim::viewport {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMinVectorLength = 1.0e-9;
} // namespace

Status Camera::SetLookAt(Vector3 eye, Vector3 target, Vector3 worldUp) noexcept {
    if (!IsFinite(eye) || !IsFinite(target) || !IsFinite(worldUp)) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }

    const Vector3 offset = target - eye;
    const double radius = Length(offset);
    if (radius < kMinVectorLength) {
        // eye == target (or too close to distinguish): degenerate, no
        // well-defined forward direction.
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }

    if (Length(worldUp) < kMinVectorLength) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }

    const Vector3 forward = Normalize(offset);
    const Vector3 up_normalized = Normalize(worldUp);
    // worldUp must not be (anti-)parallel to forward, or the right-vector
    // construction (Cross(forward, worldUp)) degenerates to zero and no
    // well-defined camera basis exists (Brief section 6: "up not
    // degenerate/parallel to forward").
    if (Length(Cross(forward, up_normalized)) < kMinVectorLength) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }

    eye_ = eye;
    target_ = target;
    world_up_ = worldUp;
    look_at_set_ = true;
    return Status::Ok();
}

Status Camera::SetPerspective(double verticalFovRadians, double nearPlane,
                              double farPlane) noexcept {
    if (!IsFinite(verticalFovRadians) || !IsFinite(nearPlane) || !IsFinite(farPlane)) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }
    if (!(verticalFovRadians > 0.0) || !(verticalFovRadians < kPi)) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }
    if (!(nearPlane > 0.0) || !(farPlane > nearPlane)) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }

    vertical_fov_radians_ = verticalFovRadians;
    near_plane_ = nearPlane;
    far_plane_ = farPlane;
    perspective_set_ = true;
    return Status::Ok();
}

Status Camera::SetAspectRatio(double aspect) noexcept {
    if (!IsFinite(aspect) || !(aspect > 0.0)) {
        return Status::Fail(ViewportErrorCode::InvalidInput);
    }
    aspect_ratio_ = aspect;
    return Status::Ok();
}

void Camera::Orbit(double deltaYawRadians, double deltaPitchRadians) noexcept {
    if (!IsFinite(deltaYawRadians) || !IsFinite(deltaPitchRadians)) {
        return; // infallible per contract; simply ignore a non-finite input
    }

    const Vector3 offset = eye_ - target_;
    const double radius = Length(offset);
    if (radius < kMinVectorLength) {
        return; // degenerate (should not happen if SetLookAt's invariant held)
    }

    const double horizontal_radius = std::sqrt(offset.x * offset.x + offset.y * offset.y);
    const double current_pitch = std::atan2(offset.z, horizontal_radius);
    const double current_yaw = std::atan2(offset.y, offset.x);

    const double new_yaw = current_yaw + deltaYawRadians;
    double new_pitch = current_pitch + deltaPitchRadians;
    new_pitch = std::clamp(new_pitch, -kMaxPitchRadians, kMaxPitchRadians);

    const double cos_pitch = std::cos(new_pitch);
    Vector3 new_offset{
        radius * cos_pitch * std::cos(new_yaw),
        radius * cos_pitch * std::sin(new_yaw),
        radius * std::sin(new_pitch),
    };

    eye_ = target_ + new_offset;
}

void Camera::Pan(double deltaRight, double deltaUp) noexcept {
    if (!IsFinite(deltaRight) || !IsFinite(deltaUp)) {
        return;
    }

    const Vector3 forward_offset = target_ - eye_;
    if (Length(forward_offset) < kMinVectorLength) {
        return;
    }
    const Vector3 forward = Normalize(forward_offset);
    const Vector3 right = Normalize(Cross(forward, world_up_));
    if (Length(right) < kMinVectorLength) {
        return; // forward (near-)parallel to world_up_; no well-defined right
    }
    const Vector3 up = Cross(right, forward);

    const Vector3 delta = right * deltaRight + up * deltaUp;
    eye_ = eye_ + delta;
    target_ = target_ + delta;
}

void Camera::Dolly(double deltaDistance) noexcept {
    if (!IsFinite(deltaDistance)) {
        return;
    }

    const Vector3 offset = target_ - eye_;
    const double radius = Length(offset);
    if (radius < kMinVectorLength) {
        return;
    }
    const Vector3 forward = Normalize(offset);

    // Positive deltaDistance moves the eye toward the target ("dolly in");
    // negative moves it away ("dolly out"). Clamped so the radius never
    // reaches zero or inverts through the target.
    double new_radius = radius - deltaDistance;
    new_radius = std::max(new_radius, kMinOrbitRadius);

    eye_ = target_ - forward * new_radius;
}

} // namespace bim::viewport
