// Mandatory join corpus J01-J06 (Implementation Brief section 16; Amendment
// 01 AA-C04). Fuse qualification uses geometric separation, never common
// volume, so that J04 exact face contact reaches Fuse.

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

TEST_CASE("J01 collinear overlap fuses to volume 4.2",
          "[integration][geometry][occt][corpus][J01]") {
    const SolidHandle a = BuildOrRequireOk(MakeJ01A());
    const SolidHandle b = BuildOrRequireOk(MakeJ01B());

    const SolidResult fuse_result = Fuse(a, b, kReferenceTolerance);
    REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
    REQUIRE(fuse_result.ok());

    const MetricsResult metrics = Inspect(fuse_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.solid_count == 1);
    REQUIRE(metrics.metrics.volume == Catch::Approx(kJ01ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("J02 orthogonal L join fuses to volume 4.68",
          "[integration][geometry][occt][corpus][J02]") {
    const SolidHandle a = BuildOrRequireOk(MakeJ02A());
    const SolidHandle b = BuildOrRequireOk(MakeJ02B());

    const SolidResult fuse_result = Fuse(a, b, kReferenceTolerance);
    REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
    REQUIRE(fuse_result.ok());

    const MetricsResult metrics = Inspect(fuse_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.volume == Catch::Approx(kJ02ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("J03 T join fuses to volume 3.48", "[integration][geometry][occt][corpus][J03]") {
    const SolidHandle a = BuildOrRequireOk(MakeJ03A());
    const SolidHandle b = BuildOrRequireOk(MakeJ03B());

    const SolidResult fuse_result = Fuse(a, b, kReferenceTolerance);
    REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
    REQUIRE(fuse_result.ok());

    const MetricsResult metrics = Inspect(fuse_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.volume == Catch::Approx(kJ03ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("J04 coplanar-face contact reaches Fuse and yields one connected solid of volume 2.4",
          "[integration][geometry][occt][corpus][J04]") {
    const SolidHandle a = BuildOrRequireOk(MakeJ04A());
    const SolidHandle b = BuildOrRequireOk(MakeJ04B());

    const SolidResult fuse_result = Fuse(a, b, kReferenceTolerance);
    REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
    REQUIRE(fuse_result.ok());

    const MetricsResult metrics = Inspect(fuse_result.solid);
    REQUIRE(metrics.ok());
    REQUIRE(metrics.metrics.solid_count == 1);
    REQUIRE(metrics.metrics.volume == Catch::Approx(kJ04ExpectedVolume).epsilon(0.0001));
}

TEST_CASE("J05 small-gap pair (0.001) remains NoIntersection for every mandatory linear tolerance",
          "[integration][geometry][occt][corpus][J05]") {
    for (double linear : kLinearToleranceCandidates) {
        const GeometryTolerance tolerance{linear, kReferenceTolerance.angular_radians};
        const SolidHandle a = BuildOrRequireOk(MakeJ05A());
        const SolidHandle b = BuildOrRequireOk(MakeJ05B());

        const SolidResult fuse_result = Fuse(a, b, tolerance);
        // The gap is 0.001; the largest mandatory linear candidate is
        // 1.0e-4, so every candidate must remain NoIntersection (Amendment
        // 01 AA-C04: "J05 gap 0.001 remains NoIntersection for all
        // mandatory linear tolerances, because the largest candidate is
        // 0.0001").
        REQUIRE(fuse_result.error.code == GeometryErrorCode::NoIntersection);
    }
}

TEST_CASE("J06 near-coincident overlap stress case: no crash, project-owned classification, "
          "deterministic per matrix cell",
          "[integration][geometry][occt][corpus][J06]") {
    // Observational (Brief section 16): the kernel outcome is not
    // predeclared. Requirements are: no crash, no leaked OCCT exception, a
    // project-owned classification, the same classification across repeated
    // identical runs for a given matrix cell, and a valid result when
    // successful.
    for (double linear : kLinearToleranceCandidates) {
        const GeometryTolerance tolerance{linear, kReferenceTolerance.angular_radians};

        GeometryErrorCode first_code = GeometryErrorCode::None;
        for (int run = 0; run < 3; ++run) {
            const SolidHandle a = BuildOrRequireOk(MakeJ06A());
            const SolidHandle b = BuildOrRequireOk(MakeJ06B());
            const SolidResult fuse_result = Fuse(a, b, tolerance);

            // A project-owned classification: not raising, not leaking an
            // OCCT type - the mere fact this call returned is the "no
            // crash" assertion. KernelOperationFailed here would indicate a
            // genuine adapter defect for a well-formed input pair, so it is
            // explicitly excluded as an acceptable outcome for this
            // corpus case.
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
