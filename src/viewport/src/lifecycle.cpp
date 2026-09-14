#include "bim/viewport/lifecycle.hpp"

namespace bim::viewport {

Status ViewportLifecycle::BeginInitialization() noexcept {
    if (state_ != ViewportState::Uninitialized) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::SurfaceUnavailable;
    return Status::Ok();
}

Status ViewportLifecycle::OnSurfaceAvailable() noexcept {
    if (state_ != ViewportState::SurfaceUnavailable) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::Ready;
    return Status::Ok();
}

Status ViewportLifecycle::OnSurfaceLost() noexcept {
    if (state_ != ViewportState::Ready && state_ != ViewportState::Suspended) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::SurfaceUnavailable;
    return Status::Ok();
}

Status ViewportLifecycle::OnSuspend() noexcept {
    if (state_ != ViewportState::Ready) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::Suspended;
    return Status::Ok();
}

Status ViewportLifecycle::OnResume() noexcept {
    if (state_ != ViewportState::Suspended) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::Ready;
    return Status::Ok();
}

Status ViewportLifecycle::BeginShutdown() noexcept {
    if (state_ == ViewportState::ShuttingDown || state_ == ViewportState::Destroyed) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    // Reachable from Uninitialized, SurfaceUnavailable, Ready, or Suspended
    // - shutdown can be requested at any point in a live viewport's
    // lifetime, not only from a fully-Ready state (Brief section 10: "any
    // state -> ShuttingDown -> Destroyed").
    state_ = ViewportState::ShuttingDown;
    return Status::Ok();
}

Status ViewportLifecycle::CompleteShutdown() noexcept {
    if (state_ != ViewportState::ShuttingDown) {
        return Status::Fail(ViewportErrorCode::InvalidState);
    }
    state_ = ViewportState::Destroyed;
    return Status::Ok();
}

} // namespace bim::viewport
