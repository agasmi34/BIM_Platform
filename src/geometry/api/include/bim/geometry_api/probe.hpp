#pragma once

#include "bim/foundation/status.hpp"

namespace bim::geometry_api {

// Project-owned, neutral result of the Phase 0 geometry-kernel toolchain
// smoke probe (Implementation Brief Phase I, "Geometry API / OCCT adapter").
//
// This is NOT the future geometry engine API - it is a Phase 0
// diagnostic/probe, named and documented as such to prevent accidental API
// ossification. It exists only to prove that bim_geometry_occt can
// construct a trivial OCCT solid internally and report one deterministic
// neutral value, without exposing any OCCT type (TopoDS_*, gp_*, BRep*,
// OCCT smart handles, or any OCCT header) through this header or any other
// bim_geometry_api / bim_model public header.
struct ProbeResult {
    bim::foundation::Status status;
    double neutral_value = 0.0;
};

// Declared here, implemented only in bim_geometry_occt
// (src/geometry/occt/src/geometry_occt_probe.cpp). bim_geometry_api itself
// has no implementation and links no third-party geometry kernel.
[[nodiscard]] ProbeResult RunKernelSmokeProbe();

} // namespace bim::geometry_api
