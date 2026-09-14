#pragma once

#include <cstdint>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/input.hpp"
#include "bim/viewport/ray.hpp"

class QMouseEvent;
class QWheelEvent;

// src/desktop/src/viewport_bridge.hpp - the ONLY place in the entire P0-T003
// footprint that reads QMouseEvent/QWheelEvent/Qt::MouseButton (Brief
// section 6: "Qt event types terminate in the desktop layer"; forward
// -declared here rather than #include <QtWidgets/...> so this header stays
// cheap to include from viewport_window.hpp). Converts Qt input into the
// neutral bim::viewport::PointerDelta/ScrollDelta types and drives
// Camera::Orbit/Pan/Dolly directly - never stores a raw QEvent.
//
// Button-mapping convention (an implementation decision made here, not
// dictated verbatim by a specific brief clause beyond "Orbit/Pan/Dolly
// wheel-zoom" - chosen to match the common CAD/DCC viewport convention so
// it needs no retraining for anyone familiar with similar tools):
//   left-button drag   -> Camera::Orbit
//   middle-button drag -> Camera::Pan
//   right-button drag  -> reserved, currently a no-op (no context menu or
//                          alternate navigation is in scope for P0-T003)
//   wheel               -> Camera::Dolly

namespace bim::desktop {

class ViewportBridge {
public:
    explicit ViewportBridge(bim::viewport::Camera& camera) noexcept;

    void OnMousePress(const QMouseEvent& event) noexcept;
    void OnMouseMove(const QMouseEvent& event) noexcept;
    void OnMouseRelease(const QMouseEvent& event) noexcept;
    void OnWheel(const QWheelEvent& event) noexcept;

    // Converts a position in physical framebuffer pixels (already DPR
    // -scaled by the caller - see ViewportWindow) into a world-space ray.
    // Thin pass-through to bim::viewport::ScreenToWorldRay; kept as a
    // bridge method so ViewportWindow does not need to know Camera is
    // involved at all.
    [[nodiscard]] bim::viewport::Result<bim::viewport::Ray>
    PickRay(double physicalPixelX, double physicalPixelY, std::uint32_t viewportWidthPx,
            std::uint32_t viewportHeightPx) const noexcept;

private:
    bim::viewport::Camera& camera_;
    bim::viewport::PointerButton active_button_ = bim::viewport::PointerButton::None;
    double last_x_ = 0.0;
    double last_y_ = 0.0;
    bool dragging_ = false;

    // Arbitrary-but-sane sensitivity constants; not derived from any
    // measured/specified value in the Brief - the Operator should tune
    // these by feel once the spike actually runs.
    static constexpr double kOrbitRadiansPerPixel = 0.005;
    static constexpr double kPanUnitsPerPixel = 0.01;
    static constexpr double kDollyUnitsPerWheelStep = 0.5;
};

} // namespace bim::desktop
