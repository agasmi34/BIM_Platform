// Tolerance matrix experiments (Implementation Brief section 12). This is
// evidence-collection, not a policy decision: "Do not silently choose a
// preferred tolerance from these results. Record the evidence for
// Architecture Authority disposition." Assertions here are limited to what
// the brief actually locks (no crash; J05 stays NoIntersection at every
// linear candidate - already covered directly in
// integration_geometry_occt_join.cpp) plus basic internal consistency
// (successful volumes agree with the analytical corpus value within a
// tolerance-scaled epsilon).

#include "bim/geometry_api/geometry.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;

TEST_CASE("Core local-origin cases run cleanly across every mandatory linear tolerance candidate "
          "at angular 1.0e-8",
          "[integration][geometry][occt][matrix][tolerance]") {
    for (double linear : kLinearToleranceCandidates) {
        const GeometryTolerance tolerance{linear, 1.0e-8};

        const SolidResult p01 = MakeLinearExtrusion(MakeP01(), tolerance);
        REQUIRE(p01.ok());
        REQUIRE(Inspect(p01.solid).metrics.volume ==
                Catch::Approx(kP01ExpectedVolume).epsilon(0.001));

        const SolidResult host = MakeLinearExtrusion(MakeP01(), tolerance);
        const SolidResult tool = MakeLinearExtrusion(MakeO01(), tolerance);
        REQUIRE(host.ok());
        REQUIRE(tool.ok());
        const SolidResult cut_result = Cut(host.solid, tool.solid, tolerance);
        REQUIRE(cut_result.error.code == GeometryErrorCode::None);
        REQUIRE(Inspect(cut_result.solid).metrics.volume ==
                Catch::Approx(kO01ExpectedVolume).epsilon(0.001));
    }
}

TEST_CASE("Profile validation (RectangleProfile3 orthogonality) runs cleanly across every "
          "mandatory angular tolerance candidate at linear 1.0e-6",
          "[integration][geometry][occt][matrix][tolerance]") {
    for (double angular : kAngularToleranceCandidates) {
        const GeometryTolerance tolerance{1.0e-6, angular};

        // A clean, exactly-orthogonal profile must remain valid at every
        // angular candidate.
        const SolidResult orthogonal_result = MakeLinearExtrusion(MakeP01(), tolerance);
        REQUIRE(orthogonal_result.ok());

        // A profile whose axes are skewed by an angle well outside every
        // mandatory angular candidate (0.01 rad ~= 0.57 degrees, larger than
        // the largest candidate 1.0e-6 rad by four orders of magnitude)
        // must remain DegenerateGeometry at every candidate - this is a
        // deliberate, generously-separated negative case, not a
        // boundary-precision probe.
        LinearExtrusionSpec skewed = MakeP01();
        skewed.profile.v_axis = Vector3{std::sin(0.01), 0.0, std::cos(0.01)};
        const SolidResult skewed_result = MakeLinearExtrusion(skewed, tolerance);
        REQUIRE(skewed_result.error.code == GeometryErrorCode::DegenerateGeometry);
    }
}
