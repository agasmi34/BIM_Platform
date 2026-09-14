#pragma once

#include <utility>

// bim/viewport/error.hpp - project-owned, allocation-free public error
// contract for the P0-T003 neutral viewport module (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0, section 8).
//
// Deliberately NOT bim::foundation::Status: foundation's Status carries a
// std::string diagnostic message (fine for the P0-T001 adapter/probe
// boundary it was built for), but the Brief explicitly requires the public
// viewport error contract to be code-based and allocation-free - "Do not
// place dynamically allocated diagnostic strings inside the public error
// contract merely to carry third-party text." Detailed Qt/bgfx/backend
// diagnostics belong in private logging/evidence (see
// bim::viewport_bgfx::Renderer and the desktop evidence mode), never in this
// header.
//
// Forbidden here, per the locked neutral-contract boundary: Qt, bgfx, D3D,
// Windows native types, OCCT, and BIM semantic/model/query types. Only
// <cstdint> and this translation unit's own types are used.

namespace bim::viewport {

// The complete set of public viewport failure categories (Brief section 8).
// None is a "success" sentinel usable interchangeably with any of the
// others - Status::Ok()/Result<T>::Ok() are the only success spellings.
// RD1.6-02 (performance-enum-size): the default enum representation is
// intentionally preserved for the Phase-0 contract; not changed merely
// for storage-size optimization (Architecture Authority decision).
enum class ViewportErrorCode { // NOLINT(performance-enum-size)
    None,
    InvalidInput,
    InvalidState,
    SurfaceUnavailable,
    RendererInitializationFailed,
    UnsupportedBackend,
    InvalidMesh,
    ResourceCreationFailed,
    ResourceNotFound,
    BackendFailure,
};

// Status - carries the project-owned code only, nothing else. Cheap to
// copy/return by value; never allocates.
class Status {
public:
    [[nodiscard]] static Status Ok() noexcept { return Status(ViewportErrorCode::None); }
    [[nodiscard]] static Status Fail(ViewportErrorCode code) noexcept {
        // A caller passing None to Fail() is a contract violation, not a
        // recoverable case this header should paper over: treat it as
        // InvalidState so a failed Status can never silently read as Ok().
        return Status(code == ViewportErrorCode::None ? ViewportErrorCode::InvalidState : code);
    }

    [[nodiscard]] bool IsOk() const noexcept { return code_ == ViewportErrorCode::None; }
    [[nodiscard]] ViewportErrorCode Code() const noexcept { return code_; }

    explicit operator bool() const noexcept { return IsOk(); }

private:
    explicit Status(ViewportErrorCode code) noexcept : code_(code) {}
    ViewportErrorCode code_;
};

// Result<T> - either a value or a project-owned error code, never both.
// Constructing/destroying/accessing follow the usual "check IsOk() before
// Value()" discipline; Value()/Error() on the wrong side is a precondition
// violation (asserted in the .cpp-free, header-only implementation below via
// undefined behavior avoidance is not attempted here - callers are expected
// to check IsOk() first, exactly like Status).
//
// Per the Brief ("Do not declare allocating/moving Result factories
// noexcept unless the actual implementation makes that guarantee"): Ok(T)
// is NOT declared noexcept, because constructing/moving a caller-supplied T
// (e.g. a RenderMeshHandle - trivially copyable, but Result<T> is a template
// usable with any T some future caller might instantiate it with) may throw.
// Fail(code) never touches a T and IS noexcept.
template <typename T> class Result {
public:
    [[nodiscard]] static Result Ok(T value) { return Result(std::move(value)); }
    [[nodiscard]] static Result Fail(ViewportErrorCode code) noexcept {
        return Result(code == ViewportErrorCode::None ? ViewportErrorCode::InvalidState : code);
    }

    [[nodiscard]] bool IsOk() const noexcept { return has_value_; }
    explicit operator bool() const noexcept { return has_value_; }

    // Precondition: IsOk(). Calling this on a failed Result is a caller
    // error (mirrors Status::Code()/Err() discipline throughout this
    // module).
    [[nodiscard]] const T& Value() const& noexcept { return value_; }
    [[nodiscard]] T&& Value() && noexcept { return std::move(value_); }

    // Precondition: !IsOk().
    [[nodiscard]] ViewportErrorCode Code() const noexcept { return code_; }

private:
    explicit Result(T value)
        : has_value_(true), value_(std::move(value)), code_(ViewportErrorCode::None) {}
    explicit Result(ViewportErrorCode code) noexcept : has_value_(false), value_(), code_(code) {}

    bool has_value_;
    T value_;
    ViewportErrorCode code_;
};

} // namespace bim::viewport
