#include "bim/geometry_api/probe.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("bim_geometry_occt kernel smoke probe constructs a solid and reports a positive volume",
          "[integration][geometry][occt]") {
    const bim::geometry_api::ProbeResult result = bim::geometry_api::RunKernelSmokeProbe();

    REQUIRE(result.status.ok());
    // A 2 x 3 x 4 rectangular box has volume 24. OCCT's exact-primitive
    // volume integration should reproduce this to floating-point precision,
    // so a tight epsilon (rather than a loose tolerance) is appropriate.
    REQUIRE(result.neutral_value == Catch::Approx(24.0).epsilon(0.0001));
}
