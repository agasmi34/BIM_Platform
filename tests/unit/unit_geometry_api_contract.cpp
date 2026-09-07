// Proves the project-owned public geometry contract (bim::geometry_api) is
// independently consumable WITHOUT an OCCT adapter dependency (Implementation
// Brief Amendment 01 AA-C08). This translation unit links only
// bim::geometry_api and Catch2 - see tests/unit/CMakeLists.txt. It performs
// no kernel operation; MakeLinearExtrusion/Cut/Fuse/Inspect behavior is
// covered by the integration tests, which do link bim::geometry_occt.

#include "bim/geometry_api/geometry.hpp"

#include <catch2/catch_test_macros.hpp>

#include <limits>

using bim::geometry_api::GeometryError;
using bim::geometry_api::GeometryErrorCode;
using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::MetricsResult;
using bim::geometry_api::Point3;
using bim::geometry_api::RectangleProfile3;
using bim::geometry_api::SolidHandle;
using bim::geometry_api::SolidMetrics;
using bim::geometry_api::SolidResult;
using bim::geometry_api::Vector3;

TEST_CASE("Point3/Vector3 default-construct to the zero vector", "[unit][geometry_api]") {
    const Point3 p;
    const Vector3 v;
    REQUIRE(p.x == 0.0);
    REQUIRE(p.y == 0.0);
    REQUIRE(p.z == 0.0);
    REQUIRE(v.x == 0.0);
    REQUIRE(v.y == 0.0);
    REQUIRE(v.z == 0.0);
}

TEST_CASE("GeometryTolerance has no implicit default (Brief section 6.4)", "[unit][geometry_api]") {
    // A caller must always supply both fields explicitly; this is a
    // compile-time property (GeometryTolerance is an aggregate with no
    // default member initializers on linear/angular_radians), exercised
    // here simply by requiring an explicit initializer to compile at all.
    const GeometryTolerance tolerance{1.0e-6, 1.0e-8};
    REQUIRE(tolerance.linear == 1.0e-6);
    REQUIRE(tolerance.angular_radians == 1.0e-8);
}

TEST_CASE("GeometryError::ok() is true only for GeometryErrorCode::None", "[unit][geometry_api]") {
    const GeometryError ok_error{};
    REQUIRE(ok_error.code == GeometryErrorCode::None);
    REQUIRE(ok_error.ok());

    for (GeometryErrorCode code :
         {GeometryErrorCode::InvalidInput, GeometryErrorCode::DegenerateGeometry,
          GeometryErrorCode::NoIntersection, GeometryErrorCode::KernelOperationFailed,
          GeometryErrorCode::InvalidResult, GeometryErrorCode::UnsupportedOperation}) {
        const GeometryError error{code, "diagnostic text is not part of the contract"};
        REQUIRE_FALSE(error.ok());
    }
}

TEST_CASE("SolidResult::ok() requires both a None error and a non-null solid",
          "[unit][geometry_api]") {
    SolidHandle null_handle;
    const SolidResult null_but_ok_code{null_handle, GeometryError{GeometryErrorCode::None, ""}};
    REQUIRE_FALSE(null_but_ok_code.ok());

    const SolidResult failed{null_handle,
                             GeometryError{GeometryErrorCode::InvalidInput, "bad input"}};
    REQUIRE_FALSE(failed.ok());
}

TEST_CASE("MetricsResult::ok() reflects only the error code", "[unit][geometry_api]") {
    const MetricsResult ok_result{SolidMetrics{}, GeometryError{GeometryErrorCode::None, ""}};
    REQUIRE(ok_result.ok());

    const MetricsResult failed_result{SolidMetrics{},
                                      GeometryError{GeometryErrorCode::InvalidResult, "bad"}};
    REQUIRE_FALSE(failed_result.ok());
}

TEST_CASE(
    "RectangleProfile3 and LinearExtrusionSpec are plain aggregates with the locked field names",
    "[unit][geometry_api]") {
    RectangleProfile3 profile;
    profile.origin = Point3{1.0, 2.0, 3.0};
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 1.0, 0.0};
    profile.size_u = 4.0;
    profile.size_v = 5.0;

    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 0.0, 1.0};
    spec.distance = 6.0;

    REQUIRE(spec.profile.size_u == 4.0);
    REQUIRE(spec.profile.size_v == 5.0);
    REQUIRE(spec.distance == 6.0);
}

TEST_CASE("SolidMetrics counts are plain diagnostic fields, not an identity contract",
          "[unit][geometry_api]") {
    SolidMetrics metrics;
    metrics.valid = true;
    metrics.volume = 3.6;
    metrics.solid_count = 1;
    metrics.face_count = 6;
    metrics.edge_count = 12;

    REQUIRE(metrics.valid);
    REQUIRE(metrics.solid_count == 1);
    // No API on SolidMetrics claims these counts are stable/persistent
    // identity (Brief section 19); this test only proves the fields exist
    // and round-trip as plain data.
}

TEST_CASE("A default-constructed SolidHandle is null and comparable to nullptr",
          "[unit][geometry_api]") {
    const SolidHandle handle;
    REQUIRE(handle == nullptr);
}
