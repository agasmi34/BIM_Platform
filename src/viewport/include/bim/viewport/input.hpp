#pragma once

// bim/viewport/input.hpp - the smallest neutral input representation needed
// to drive Camera::Orbit/Pan/Dolly (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0, section 6: "Input types are neutral
// POD/value types only. Qt event types terminate in the desktop layer.").
//
// Deliberately NOT a general application-wide input/event system - just the
// two POD deltas orbit/pan/dolly logic actually needs. Translating
// QMouseEvent/QWheelEvent/Qt::MouseButton/Qt::KeyboardModifier into these
// types happens only in src/desktop/** (see viewport_bridge.cpp) - this
// header never includes, forward-declares, or otherwise depends on any Qt
// type.
//
// Header-only: no input.cpp exists in the authorized P0-T003 footprint.

namespace bim::viewport {

// RD1.6-02 (performance-enum-size): the default enum representation is
// intentionally preserved for the Phase-0 contract; not changed merely
// for storage-size optimization (Architecture Authority decision).
enum class PointerButton { // NOLINT(performance-enum-size)
    None,
    Primary,
    Secondary,
    Middle,
};

// A pointer-drag sample, expressed in physical framebuffer pixels (same
// convention as ScreenToWorldRay in ray.hpp) since that is what orbit/pan
// sensitivity is naturally tuned against.
struct PointerDelta {
    double dx = 0.0;
    double dy = 0.0;
    PointerButton button = PointerButton::None;
};

// A wheel/scroll sample. Sign convention: positive amount means "scroll
// toward the user" (the common wheel-forward gesture), which the desktop
// bridge maps to Camera::Dolly with a positive deltaDistance (dolly in) -
// see viewport_bridge.cpp for the exact mapping and sensitivity scale.
struct ScrollDelta {
    double amount = 0.0;
};

} // namespace bim::viewport
