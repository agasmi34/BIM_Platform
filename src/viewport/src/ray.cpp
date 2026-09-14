#include "bim/viewport/ray.hpp"

#include <cmath>

namespace bim::viewport {

namespace {
constexpr double kMinVectorLength = 1.0e-9;
} // namespace

Result<Ray> ScreenToWorldRay(const Camera& camera, double physicalPixelX, double physicalPixelY,
                             std::uint32_t viewportWidthPx,
                             std::uint32_t viewportHeightPx) noexcept {
    // Degenerate viewport: no well-defined pixel-to-NDC mapping exists.
    if (viewportWidthPx == 0 || viewportHeightPx == 0) {
        return Result<Ray>::Fail(ViewportErrorCode::InvalidState);
    }

    if (!std::isfinite(physicalPixelX) || !std::isfinite(physicalPixelY)) {
        return Result<Ray>::Fail(ViewportErrorCode::InvalidInput);
    }

    // A camera that has never had SetLookAt/SetPerspective succeed has no
    // well-defined eye/forward/FOV to build a ray from.
    if (!camera.IsConfigured()) {
        return Result<Ray>::Fail(ViewportErrorCode::InvalidState);
    }

    // physicalPixelX/Y are in physical framebuffer pixels, origin top-left
    // (Brief section 7). Map to normalized device coordinates: X in
    // [-1, 1] left-to-right, Y in [-1, 1] bottom-to-top (NDC is Y-up, so the
    // vertical axis is flipped relative to the screen's top-left origin).
    const double width = static_cast<double>(viewportWidthPx);
    const double height = static_cast<double>(viewportHeightPx);
    const double ndc_x = (2.0 * physicalPixelX / width) - 1.0;
    const double ndc_y = 1.0 - (2.0 * physicalPixelY / height);

    // Camera-local orthonormal basis. Built locally here rather than
    // exposed by Camera itself (Camera's public contract is limited to
    // semantic parameters only - see camera.hpp).
    const Vector3 forward_unnormalized = camera.Target() - camera.Eye();
    const double forward_length = Length(forward_unnormalized);
    if (forward_length < kMinVectorLength) {
        // Should not be reachable if SetLookAt's invariant held, but this
        // function never assumes a caller-invisible invariant without
        // checking it directly.
        return Result<Ray>::Fail(ViewportErrorCode::InvalidState);
    }
    const Vector3 forward = Normalize(forward_unnormalized);
    const Vector3 right_unnormalized = Cross(forward, camera.WorldUp());
    const double right_length = Length(right_unnormalized);
    if (right_length < kMinVectorLength) {
        return Result<Ray>::Fail(ViewportErrorCode::InvalidState);
    }
    const Vector3 right = Normalize(right_unnormalized);
    const Vector3 up = Cross(right, forward);

    // Scale NDC X/Y into view-space offsets using the vertical FOV and
    // aspect ratio (Brief section 7: "derived from Camera's semantic
    // parameters" - matches the same tan(fov/2) relationship the backend's
    // symmetric perspective projection uses, so a ray through pixel (x, y)
    // matches what was actually rendered at that pixel).
    const double tan_half_fov = std::tan(camera.VerticalFovRadians() * 0.5);
    const double view_x = ndc_x * tan_half_fov * camera.AspectRatio();
    const double view_y = ndc_y * tan_half_fov;

    const Vector3 direction_unnormalized = (right * view_x) + (up * view_y) + forward;
    const double direction_length = Length(direction_unnormalized);
    if (direction_length < kMinVectorLength) {
        return Result<Ray>::Fail(ViewportErrorCode::InvalidState);
    }

    Ray ray;
    ray.origin = camera.Eye();
    ray.direction = Normalize(direction_unnormalized);
    return Result<Ray>::Ok(ray);
}

} // namespace bim::viewport
