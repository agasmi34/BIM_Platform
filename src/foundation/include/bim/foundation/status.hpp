#pragma once

#include <string>
#include <utility>

namespace bim::foundation {

// A minimal, dependency-free project-owned result/status type used at
// adapter/probe boundaries (Implementation Brief Phase I, "Foundation").
//
// This is intentionally NOT a general BIM error-handling framework. It
// exists because the geometry and persistence smoke probes
// (bim_geometry_occt, bim_persistence) need a project-owned way to report
// success/failure without leaking OCCT exceptions or SQLite error codes
// across a module boundary. foundation has zero project or third-party
// dependencies (Architecture Gate section 8.2); this header must stay that
// way.
class Status {
public:
    [[nodiscard]] static Status Ok();
    [[nodiscard]] static Status Error(std::string message);

    [[nodiscard]] bool ok() const noexcept { return ok_; }
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

    explicit operator bool() const noexcept { return ok_; }

private:
    Status(bool ok, std::string message) : ok_(ok), message_(std::move(message)) {}

    bool ok_;
    std::string message_;
};

} // namespace bim::foundation
