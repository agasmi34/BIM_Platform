// Mandatory opening corpus O01-O04 (Implementation Brief section 15;
// Amendment 01 AA-C03). Host is P01; O02/O04 must reach NoIntersection
// without being disguised as a kernel failure.

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

namespace {

[[nodiscard]] SolidHandle BuildOrRequireOk(const LinearExtrusionSpec& spec) {
    const SolidResult result = MakeLinearExtrusion(spec, kReferenceTolerance);
    REQUIRE(result.ok());
    return result.solid;
}

} // namespace

TEST_CASE("O01 normal through opening cuts host to volume 3.2",
          "[integration][geometry][occt][corpus][O01]") {
    const SolidHandle host = BuildOrRequireOk(MakeP01());
    const SolidHandle tool = BuildOrRequireOk(MakeO01());

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.error.code == GeometryErrorCode::None);
    REQUIRE(cut_result.ok());

    const MetricsResult metrics = Inspect(cut_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.valid);
    REQUIRE(metrics.metrics.volume == Catch::Approx(kO01ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("O02 tool completely outside host yields NoIntersection",
          "[integration][geometry][occt][corpus][O02]") {
    const SolidHandle host = BuildOrRequireOk(MakeP01());
    const SolidHandle tool = BuildOrRequireOk(MakeO02());

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.error.code == GeometryErrorCode::NoIntersection);
    REQUIRE_FALSE(cut_result.ok());
}

TEST_CASE("O03 partial host intersection cuts host to volume 3.4",
          "[integration][geometry][occt][corpus][O03]") {
    const SolidHandle host = BuildOrRequireOk(MakeP01());
    const SolidHandle tool = BuildOrRequireOk(MakeO03());

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.error.code == GeometryErrorCode::None);
    REQUIRE(cut_result.ok());

    const MetricsResult metrics = Inspect(cut_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.volume == Catch::Approx(kO03ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("O04 boundary-only contact yields NoIntersection, not a kernel failure",
          "[integration][geometry][occt][corpus][O04]") {
    const SolidHandle host = BuildOrRequireOk(MakeP01());
    const SolidHandle tool = BuildOrRequireOk(MakeO04());

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.error.code == GeometryErrorCode::NoIntersection);
    REQUIRE_FALSE(cut_result.ok());
}
