#include "viewport_bridge.hpp"

#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

namespace bim::desktop {

namespace {

bim::viewport::PointerButton MapButton(Qt::MouseButton button) noexcept {
    switch (button) {
        case Qt::LeftButton:
            return bim::viewport::PointerButton::Primary;
        case Qt::RightButton:
            return bim::viewport::PointerButton::Secondary;
        case Qt::MiddleButton:
            return bim::viewport::PointerButton::Middle;
        default:
            return bim::viewport::PointerButton::None;
    }
}

} // namespace

ViewportBridge::ViewportBridge(bim::viewport::Camera& camera) noexcept : camera_(camera) {}

void ViewportBridge::OnMousePress(const QMouseEvent& event) noexcept {
    active_button_ = MapButton(event.button());
    if (active_button_ == bim::viewport::PointerButton::None) {
        return;
    }
    dragging_ = true;
    last_x_ = event.position().x();
    last_y_ = event.position().y();
}

void ViewportBridge::OnMouseMove(const QMouseEvent& event) noexcept {
    if (!dragging_) {
        return;
    }

    const double x = event.position().x();
    const double y = event.position().y();
    const double dx = x - last_x_;
    const double dy = y - last_y_;
    last_x_ = x;
    last_y_ = y;

    switch (active_button_) {
        case bim::viewport::PointerButton::Primary:
            // Screen-right drag orbits yaw positively; screen-down drag
            // orbits pitch negatively (looking "down" at the model as the
            // mouse moves down), matching the common CAD orbit feel.
            camera_.Orbit(-dx * kOrbitRadiansPerPixel, -dy * kOrbitRadiansPerPixel);
            break;
        case bim::viewport::PointerButton::Middle:
            // Screen-right drag pans the view right; screen-down drag pans
            // the view down (content follows the cursor).
            camera_.Pan(-dx * kPanUnitsPerPixel, dy * kPanUnitsPerPixel);
            break;
        case bim::viewport::PointerButton::Secondary:
        case bim::viewport::PointerButton::None:
        default:
            break; // reserved / no-op, see header comment
    }
}

void ViewportBridge::OnMouseRelease(const QMouseEvent& event) noexcept {
    if (MapButton(event.button()) == active_button_) {
        dragging_ = false;
        active_button_ = bim::viewport::PointerButton::None;
    }
}

void ViewportBridge::OnWheel(const QWheelEvent& event) noexcept {
    // angleDelta().y() is in eighths of a degree, +-120 per traditional
    // wheel "click"; normalized to a small integer step count so
    // high-resolution/trackpad wheels do not dolly implausibly fast.
    const double steps = static_cast<double>(event.angleDelta().y()) / 120.0;
    camera_.Dolly(steps * kDollyUnitsPerWheelStep);
}

bim::viewport::Result<bim::viewport::Ray>
ViewportBridge::PickRay(double physicalPixelX, double physicalPixelY, std::uint32_t viewportWidthPx,
                        std::uint32_t viewportHeightPx) const noexcept {
    return bim::viewport::ScreenToWorldRay(camera_, physicalPixelX, physicalPixelY, viewportWidthPx,
                                           viewportHeightPx);
}

} // namespace bim::desktop
