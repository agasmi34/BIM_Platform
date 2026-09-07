// Coordinate matrix experiments (Implementation Brief section 13): measure
// numerical behavior at C0/C1/C2 using the reference probe tolerance. This
// does not authorize storing all future BIM geometry at survey coordinates
// - it only measures how the kernel behaves at increasing coordinate
// magnitude for representative primitive/cut/fuse cases.

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

TEST_CASE("P01 primitive produces the analytically expected volume at C0/C1/C2",
          "[integration][geometry][occt][matrix][coordinate]") {
    for (const Point3& offset : kCoordinateOffsets) {
        const SolidResult result = MakeLinearExtrusion(MakeP01(offset), kReferenceTolerance);
        REQUIRE(result.ok());
        // A looser relative epsilon at C2 (survey-scale coordinates)
        // records rather than presupposes numerical behavior at large
        // coordinate magnitude, per Brief section 13 ("The purpose is to
        // measure numerical behavior").
        REQUIRE(Inspect(result.solid).metrics.volume ==
                Catch::Approx(kP01ExpectedVolume).epsilon(0.01));
    }
}

TEST_CASE("O01 opening cut produces the analytically expected volume at C0/C1/C2",
          "[integration][geometry][occt][matrix][coordinate]") {
    for (const Point3& offset : kCoordinateOffsets) {
        const SolidResult host = MakeLinearExtrusion(MakeP01(offset), kReferenceTolerance);
        const SolidResult tool = MakeLinearExtrusion(MakeO01(offset), kReferenceTolerance);
        REQUIRE(host.ok());
        REQUIRE(tool.ok());

        const SolidResult cut_result = Cut(host.solid, tool.solid, kReferenceTolerance);
        REQUIRE(cut_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(cut_result.solid).metrics.volume ==
                Catch::Approx(kO01ExpectedVolume).epsilon(0.01));
    }
}

TEST_CASE("J01 collinear-overlap fuse produces the analytically expected volume at C0/C1/C2",
          "[integration][geometry][occt][matrix][coordinate]") {
    for (const Point3& offset : kCoordinateOffsets) {
        const SolidResult a = MakeLinearExtrusion(MakeJ01A(offset), kReferenceTolerance);
        const SolidResult b = MakeLinearExtrusion(MakeJ01B(offset), kReferenceTolerance);
        REQUIRE(a.ok());
        REQUIRE(b.ok());

        const SolidResult fuse_result = Fuse(a.solid, b.solid, kReferenceTolerance);
        REQUIRE(fuse_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(fuse_result.solid).metrics.volume ==
                Catch::Approx(kJ01ExpectedVolume).epsilon(0.01));
    }
}
