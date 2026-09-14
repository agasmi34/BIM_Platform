#pragma once

#include "bim/viewport/error.hpp"

// bim/viewport/lifecycle.hpp - the neutral viewport lifecycle state machine
// (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 10).
//
// Exactly six stable public states. Surface creation/recreation/destruction
// are events driving transitions between these states, not additional
// persistent states themselves (Brief section 10: "Surface
// creation/recreation/destruction are events, not persistent public
// states."). ViewportLifecycle only tracks and validates the state machine
// - it owns no renderer/surface/backend resource itself; bim::viewport_bgfx
// drives it via the event methods below at the appropriate points in its
// own surface/device lifecycle.
//
// Forbidden here: Qt, bgfx, D3D, Windows native types, OCCT, BIM
// semantic/model/query types.

namespace bim::viewport {

// RD1.6-02 (performance-enum-size): the default enum representation is
// intentionally preserved for the Phase-0 contract; not changed merely
// for storage-size optimization (Architecture Authority decision).
enum class ViewportState { // NOLINT(performance-enum-size)
    Uninitialized,         // constructed, BeginInitialization() not yet called
    SurfaceUnavailable,    // initialization begun, but no live drawable surface
    Ready,                 // live surface, renderer initialized, can render
    Suspended,             // live surface retained, but rendering is paused
    ShuttingDown,          // shutdown in progress, no further transitions in
    Destroyed,             // terminal; only reachable from ShuttingDown
};

class ViewportLifecycle {
public:
    ViewportLifecycle() noexcept = default;

    [[nodiscard]] ViewportState State() const noexcept { return state_; }

    // Uninitialized -> SurfaceUnavailable. The first transition out of the
    // initial state; marks that initialization has begun but no drawable
    // surface exists yet (Brief section 10: initial state is
    // Uninitialized, and initialization always passes through
    // SurfaceUnavailable before a surface can make it Ready).
    [[nodiscard]] Status BeginInitialization() noexcept;

    // SurfaceUnavailable -> Ready. The backend has a live drawable surface
    // and has successfully initialized against it.
    [[nodiscard]] Status OnSurfaceAvailable() noexcept;

    // Ready -> SurfaceUnavailable, or Suspended -> SurfaceUnavailable. The
    // live surface is about to be destroyed or has been lost (e.g. the Qt
    // native window is being torn down for recreation, or the platform
    // reports device/surface loss). Brief section 10: "Ready/Suspended ->
    // SurfaceUnavailable on surface-about-to-be-destroyed."
    [[nodiscard]] Status OnSurfaceLost() noexcept;

    // Ready -> Suspended. Rendering is paused while the surface itself
    // remains valid (e.g. the desktop window is minimized/occluded).
    [[nodiscard]] Status OnSuspend() noexcept;

    // Suspended -> Ready. Rendering resumes on the still-valid surface.
    [[nodiscard]] Status OnResume() noexcept;

    // Any non-terminal state -> ShuttingDown. Idempotent-refusing: calling
    // this from ShuttingDown or Destroyed is InvalidState, not a silent
    // no-op, so a caller cannot mistakenly believe a second shutdown
    // request did anything.
    [[nodiscard]] Status BeginShutdown() noexcept;

    // ShuttingDown -> Destroyed. The only way to reach the terminal state.
    [[nodiscard]] Status CompleteShutdown() noexcept;

private:
    ViewportState state_ = ViewportState::Uninitialized;
};

} // namespace bim::viewport
