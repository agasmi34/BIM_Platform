// Repeatability contract (Implementation Brief section 18): P01, O01, O02,
// J01, J04, J05, J06, F02, F04 each run at least 20 times from freshly
// constructed inputs; classification must be identical across all
// repetitions for a given case/matrix cell, and successful volumes must
// agree within the test tolerance. Raw face/edge ordering is explicitly not
// a durable contract.

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

namespace {

constexpr int kRepeatCount = 20;

void RequireRepeatableExtrusion(const LinearExtrusionSpec& spec, GeometryErrorCode expected_code,
                                double expected_volume_or_negative_for_na) {
    GeometryErrorCode first_code = expected_code;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult result = MakeLinearExtrusion(spec, kReferenceTolerance);
        REQUIRE(result.error.code == expected_code);
        if (i == 0) {
            first_code = result.error.code;
        } else {
            REQUIRE(result.error.code == first_code);
        }
        if (result.ok() && expected_volume_or_negative_for_na >= 0.0) {
            REQUIRE(Inspect(result.solid).metrics.volume ==
                    Catch::Approx(expected_volume_or_negative_for_na).epsilon(0.0001));
        }
    }
}

} // namespace

TEST_CASE("P01 classification and volume are identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][P01]") {
    RequireRepeatableExtrusion(MakeP01(), GeometryErrorCode::None, kP01ExpectedVolume);
}

TEST_CASE("O01 Cut classification and volume are identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][O01]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult host = MakeLinearExtrusion(MakeP01(), kReferenceTolerance);
        const SolidResult tool = MakeLinearExtrusion(MakeO01(), kReferenceTolerance);
        REQUIRE(host.ok());
        REQUIRE(tool.ok());

        const SolidResult cut_result = Cut(host.solid, tool.solid, kReferenceTolerance);
        if (i == 0) {
            first_code = cut_result.error.code;
        } else {
            REQUIRE(cut_result.error.code == first_code);
        }
        REQUIRE(cut_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(cut_result.solid).metrics.volume ==
                Catch::Approx(kO01ExpectedVolume).epsilon(0.0001));
    }
}

TEST_CASE("O02 Cut NoIntersection classification is identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][O02]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult host = MakeLinearExtrusion(MakeP01(), kReferenceTolerance);
        const SolidResult tool = MakeLinearExtrusion(MakeO02(), kReferenceTolerance);
        REQUIRE(host.ok());
        REQUIRE(tool.ok());

        const SolidResult cut_result = Cut(host.solid, tool.solid, kReferenceTolerance);
        if (i == 0) {
            first_code = cut_result.error.code;
        } else {
            REQUIRE(cut_result.error.code == first_code);
        }
        REQUIRE(cut_result.error.code == GeometryErrorCode::NoIntersection);
    }
}

TEST_CASE("J01 Fuse classification and volume are identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][J01]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult a = MakeLinearExtrusion(MakeJ01A(), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeJ01B(), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        if (i == 0) {
            first_code = fuse_result.error.code;
        } else {
            REQUIRE(fuse_result.error.code == first_code);
        }
        REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(fuse_result.solid).metrics.volume ==
                Catch::Approx(kJ01ExpectedVolume).epsilon(0.0001));
    }
}

TEST_CASE("J04 Fuse classification and volume are identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][J04]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult a = MakeLinearExtrusion(MakeJ04A(), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeJ04B(), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        if (i == 0) {
            first_code = fuse_result.error.code;
        } else {
            REQUIRE(fuse_result.error.code == first_code);
        }
        REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(fuse_result.solid).metrics.volume ==
                Catch::Approx(kJ04ExpectedVolume).epsilon(0.0001));
    }
}

TEST_CASE("J05 Fuse NoIntersection classification is identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][J05]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult a = MakeLinearExtrusion(MakeJ05A(), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeJ05B(), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        if (i == 0) {
            first_code = fuse_result.error.code;
        } else {
            REQUIRE(fuse_result.error.code == first_code);
        }
        REQUIRE(fuse_result.error.code == GeometryErrorCode::NoIntersection);
    }
}

TEST_CASE("J06 Fuse classification is identical (project-owned, no crash) across 20 repetitions",
          "[integration][geometry][occt][repeatability][J06]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult a = MakeLinearExtrusion(MakeJ06A(), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeJ06B(), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        REQUIRE(fuse_result.error.code != GeometryErrorCode::KernelOperationFailed);
        if (i == 0) {
            first_code = fuse_result.error.code;
        } else {
            REQUIRE(fuse_result.error.code == first_code);
        }
    }
}

TEST_CASE("F02 zero-direction DegenerateGeometry classification is identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][F02]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult result = MakeLinearExtrusion(MakeF02ZeroDirection(), kReferenceTolerance);
        if (i == 0) {
            first_code = result.error.code;
        } else {
            REQUIRE(result.error.code == first_code);
        }
        REQUIRE(result.error.code == GeometryErrorCode::DegenerateGeometry);
    }
}

TEST_CASE("F04 numerically stressed Boolean classification is identical across 20 repetitions",
          "[integration][geometry][occt][repeatability][F04]") {
    GeometryErrorCode first_code = GeometryErrorCode::None;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidResult a = MakeLinearExtrusion(MakeF04A(), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeF04B(), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        REQUIRE(fuse_result.error.code != GeometryErrorCode::KernelOperationFailed);
        if (i == 0) {
            first_code = fuse_result.error.code;
        } else {
            REQUIRE(fuse_result.error.code == first_code);
        }
    }
}
