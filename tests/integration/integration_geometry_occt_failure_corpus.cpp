// Mandatory failure corpus F01-F04 (Implementation Brief section 17).

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

TEST_CASE("F01 zero extrusion distance is InvalidInput",
          "[integration][geometry][occt][corpus][F01]") {
    const SolidResult result = MakeLinearExtrusion(MakeF01ZeroDistance(), kReferenceTolerance);
    REQUIRE(result.error.code == GeometryErrorCode::InvalidInput);
    REQUIRE_FALSE(result.ok());
}

TEST_CASE("F02 zero extrusion direction is DegenerateGeometry",
          "[integration][geometry][occt][corpus][F02]") {
    const SolidResult result = MakeLinearExtrusion(MakeF02ZeroDirection(), kReferenceTolerance);
    REQUIRE(result.error.code == GeometryErrorCode::DegenerateGeometry);
    REQUIRE_FALSE(result.ok());
}

TEST_CASE("F03 negative profile dimension is InvalidInput",
          "[integration][geometry][occt][corpus][F03]") {
    const SolidResult result = MakeLinearExtrusion(MakeF03NegativeDimension(), kReferenceTolerance);
    REQUIRE(result.error.code == GeometryErrorCode::InvalidInput);
    REQUIRE_FALSE(result.ok());
}

TEST_CASE("F04 numerically stressed Boolean: no process failure, no leaked exception, "
          "deterministic status per matrix cell, valid result when None",
          "[integration][geometry][occt][corpus][F04]") {
    // Observational (Brief section 17): the exact resulting status is not
    // predeclared, and this case must NOT be forced into
    // KernelOperationFailed by deliberately constructing a corrupt OCCT
    // object. Exercised at local coordinates, the C2 large offset, and
    // every mandatory linear tolerance.
    const std::array<Point3, 2> offsets{kCoordinateOffsetC0, kCoordinateOffsetC2};
    for (const Point3& offset : offsets) {
        for (double linear : kLinearToleranceCandidates) {
            const GeometryTolerance tolerance{linear, kReferenceTolerance.angular_radians};

            const SolidResult a_result = MakeLinearExtrusion(MakeF04A(offset), tolerance);
            const SolidResult b_result = MakeLinearExtrusion(MakeF04B(offset), tolerance);
            REQUIRE(a_result.ok());
            REQUIRE(b_result.ok());

            GeometryErrorCode first_code = GeometryErrorCode::None;
            for (int run = 0; run < 2; ++run) {
                const SolidResult fuse_result = Fuse(a_result.solid, b_result.solid, tolerance);
                REQUIRE(fuse_result.error.code != GeometryErrorCode::KernelOperationFailed);
                if (run == 0) {
                    first_code = fuse_result.error.code;
                } else {
                    REQUIRE(fuse_result.error.code == first_code);
                }
                if (fuse_result.error.code == GeometryErrorCode::None) {
                    const MetricsResult metrics = Inspect(fuse_result.solid);
                    REQUIRE(metrics.ok());
                    REQUIRE(metrics.metrics.valid);
                }
            }
        }
    }
}
