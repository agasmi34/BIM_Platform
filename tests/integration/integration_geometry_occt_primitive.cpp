// Mandatory primitive corpus P01-P04 (Implementation Brief section 14).

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

namespace {

void RequireValidExtrusion(const LinearExtrusionSpec& spec, double expected_volume) {
    const SolidResult result = MakeLinearExtrusion(spec, kReferenceTolerance);
    REQUIRE(result.error.code == GeometryErrorCode::None);
    REQUIRE(result.ok());

    const MetricsResult metrics = Inspect(result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.valid);
    REQUIRE(metrics.metrics.solid_count == 1);
    REQUIRE(metrics.metrics.volume == Catch::Approx(expected_volume).epsilon(0.0001));
}

} // namespace

TEST_CASE("P01 wall-like prism produces a valid solid of volume 3.6",
          "[integration][geometry][occt][corpus][P01]") {
    RequireValidExtrusion(MakeP01(), kP01ExpectedVolume);
}

TEST_CASE("P02 slab-like prism produces a valid solid of volume 4.8",
          "[integration][geometry][occt][corpus][P02]") {
    RequireValidExtrusion(MakeP02(), kP02ExpectedVolume);
}

TEST_CASE("P03 tall narrow prism produces a valid solid of volume 0.27",
          "[integration][geometry][occt][corpus][P03]") {
    RequireValidExtrusion(MakeP03(), kP03ExpectedVolume);
}

TEST_CASE("P04 small valid feature produces a valid solid of volume 0.000001",
          "[integration][geometry][occt][corpus][P04]") {
    RequireValidExtrusion(MakeP04(), kP04ExpectedVolume);
}
