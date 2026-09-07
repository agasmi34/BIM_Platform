#pragma once

// Shared P0-T002 corpus/matrix constants and fixture builders (Implementation
// Brief sections 10-18). Test-only: never linked into production code.
//
// Test length unit: 1.0 = 1 metre-equivalent (Brief section 10). This is a
// P0-T002 corpus convention only, NOT a future BIM unit-storage decision.

#include "bim/geometry_api/geometry.hpp"

#include <array>

namespace bim::geometry_occt::test_constants {

using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::Point3;
using bim::geometry_api::RectangleProfile3;
using bim::geometry_api::Vector3;

// --- section 11: reference probe tolerance ---------------------------------
inline constexpr GeometryTolerance kReferenceTolerance{1.0e-6, 1.0e-8};

// --- section 12: tolerance matrix -------------------------------------------
inline constexpr std::array<double, 4> kLinearToleranceCandidates{1.0e-7, 1.0e-6, 1.0e-5, 1.0e-4};
inline constexpr std::array<double, 3> kAngularToleranceCandidates{1.0e-10, 1.0e-8, 1.0e-6};

// --- section 13: coordinate matrix -------------------------------------------
inline constexpr Point3 kCoordinateOffsetC0{0.0, 0.0, 0.0};
inline constexpr Point3 kCoordinateOffsetC1{1000.0, 1000.0, 0.0};
inline constexpr Point3 kCoordinateOffsetC2{1000000.0, 1000000.0, 0.0};
inline constexpr std::array<Point3, 3> kCoordinateOffsets{kCoordinateOffsetC0, kCoordinateOffsetC1,
                                                          kCoordinateOffsetC2};

[[nodiscard]] inline Point3 Translate(const Point3& p, const Point3& offset) {
    return Point3{p.x + offset.x, p.y + offset.y, p.z + offset.z};
}

// Builds an axis-aligned box as a LinearExtrusionSpec from coordinate ranges:
// profile spans [xmin,xmax] x [zmin,zmax] in the u=(1,0,0)/v=(0,0,1) plane at
// y = ymin, extruded along +Y by (ymax - ymin). This reproduces every
// axis-aligned corpus fixture (P01/P02 use this shape directly; J01-J06/O01-
// O04/F04 describe their fixtures as x/y/z ranges, which this helper turns
// directly into the locked RectangleProfile3 + LinearExtrusionSpec shape).
[[nodiscard]] inline LinearExtrusionSpec
MakeAxisAlignedBox(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                   const Point3& offset = kCoordinateOffsetC0) {
    RectangleProfile3 profile;
    profile.origin = Translate(Point3{xmin, ymin, zmin}, offset);
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = xmax - xmin;
    profile.size_v = zmax - zmin;

    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 1.0, 0.0};
    spec.distance = ymax - ymin;
    return spec;
}

// --- section 14: mandatory primitive corpus ---------------------------------
[[nodiscard]] inline LinearExtrusionSpec MakeP01(const Point3& offset = kCoordinateOffsetC0) {
    // origin (0,0,0), u=(1,0,0) size_u=6.0, v=(0,0,1) size_v=3.0,
    // direction=(0,1,0) distance=0.2 -> expected volume 3.6.
    return MakeAxisAlignedBox(0.0, 6.0, 0.0, 0.2, 0.0, 3.0, offset);
}
inline constexpr double kP01ExpectedVolume = 3.6;

[[nodiscard]] inline LinearExtrusionSpec MakeP02(const Point3& offset = kCoordinateOffsetC0) {
    RectangleProfile3 profile;
    profile.origin = Translate(Point3{0.0, 0.0, 0.0}, offset);
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 1.0, 0.0};
    profile.size_u = 6.0;
    profile.size_v = 4.0;
    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 0.0, 1.0};
    spec.distance = 0.2;
    return spec;
}
inline constexpr double kP02ExpectedVolume = 4.8;

[[nodiscard]] inline LinearExtrusionSpec MakeP03(const Point3& offset = kCoordinateOffsetC0) {
    RectangleProfile3 profile;
    profile.origin = Translate(Point3{0.0, 0.0, 0.0}, offset);
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = 0.15;
    profile.size_v = 0.15;
    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 1.0, 0.0};
    spec.distance = 12.0;
    return spec;
}
inline constexpr double kP03ExpectedVolume = 0.27;

[[nodiscard]] inline LinearExtrusionSpec MakeP04(const Point3& offset = kCoordinateOffsetC0) {
    RectangleProfile3 profile;
    profile.origin = Translate(Point3{0.0, 0.0, 0.0}, offset);
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = 0.01;
    profile.size_v = 0.01;
    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 1.0, 0.0};
    spec.distance = 0.01;
    return spec;
}
inline constexpr double kP04ExpectedVolume = 0.000001;

// --- section 15: mandatory opening corpus (host = P01) ---------------------
// Cutting-solid orientation for all O01-O04: u=(1,0,0) v=(0,0,1)
// direction=(0,1,0) - same axes as the host, per Brief section 15.
[[nodiscard]] inline LinearExtrusionSpec
MakeOpeningTool(double origin_x, double origin_y, double origin_z, double size_u, double size_v,
                double distance, const Point3& offset = kCoordinateOffsetC0) {
    RectangleProfile3 profile;
    profile.origin = Translate(Point3{origin_x, origin_y, origin_z}, offset);
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = size_u;
    profile.size_v = size_v;
    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 1.0, 0.0};
    spec.distance = distance;
    return spec;
}

[[nodiscard]] inline LinearExtrusionSpec MakeO01(const Point3& offset = kCoordinateOffsetC0) {
    return MakeOpeningTool(2.0, -0.05, 0.5, 1.0, 2.0, 0.30, offset);
}
inline constexpr double kO01ExpectedVolume = 3.2;

[[nodiscard]] inline LinearExtrusionSpec MakeO02(const Point3& offset = kCoordinateOffsetC0) {
    return MakeOpeningTool(7.0, -0.05, 0.5, 1.0, 2.0, 0.30, offset);
}

[[nodiscard]] inline LinearExtrusionSpec MakeO03(const Point3& offset = kCoordinateOffsetC0) {
    return MakeOpeningTool(5.5, -0.05, 0.5, 1.0, 2.0, 0.30, offset);
}
inline constexpr double kO03ExpectedVolume = 3.4;

[[nodiscard]] inline LinearExtrusionSpec MakeO04(const Point3& offset = kCoordinateOffsetC0) {
    return MakeOpeningTool(6.0, -0.05, 0.5, 1.0, 2.0, 0.30, offset);
}

// --- section 16: mandatory join corpus (height 3.0 unless stated) ----------
[[nodiscard]] inline LinearExtrusionSpec MakeJ01A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 4.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ01B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(3.0, 7.0, 0.0, 0.2, 0.0, 3.0, offset);
}
inline constexpr double kJ01ExpectedVolume = 4.2;

[[nodiscard]] inline LinearExtrusionSpec MakeJ02A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 4.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ02B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 0.2, 0.0, 4.0, 0.0, 3.0, offset);
}
inline constexpr double kJ02ExpectedVolume = 4.68;

[[nodiscard]] inline LinearExtrusionSpec MakeJ03A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 4.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ03B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(1.9, 2.1, 0.0, 2.0, 0.0, 3.0, offset);
}
inline constexpr double kJ03ExpectedVolume = 3.48;

[[nodiscard]] inline LinearExtrusionSpec MakeJ04A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 2.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ04B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(2.0, 4.0, 0.0, 0.2, 0.0, 3.0, offset);
}
inline constexpr double kJ04ExpectedVolume = 2.4;

[[nodiscard]] inline LinearExtrusionSpec MakeJ05A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 2.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ05B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(2.001, 4.001, 0.0, 0.2, 0.0, 3.0, offset);
}

[[nodiscard]] inline LinearExtrusionSpec MakeJ06A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(0.0, 2.0, 0.0, 0.2, 0.0, 3.0, offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeJ06B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeAxisAlignedBox(1.99995, 3.99995, 0.0, 0.2, 0.0, 3.0, offset);
}

// --- section 17: mandatory failure corpus -----------------------------------
[[nodiscard]] inline LinearExtrusionSpec
MakeF01ZeroDistance(const Point3& offset = kCoordinateOffsetC0) {
    LinearExtrusionSpec spec = MakeP04(offset);
    spec.distance = 0.0;
    return spec;
}

[[nodiscard]] inline LinearExtrusionSpec
MakeF02ZeroDirection(const Point3& offset = kCoordinateOffsetC0) {
    LinearExtrusionSpec spec = MakeP04(offset);
    spec.direction = Vector3{0.0, 0.0, 0.0};
    return spec;
}

[[nodiscard]] inline LinearExtrusionSpec
MakeF03NegativeDimension(const Point3& offset = kCoordinateOffsetC0) {
    LinearExtrusionSpec spec = MakeP04(offset);
    spec.profile.size_u = -1.0;
    return spec;
}

// F04: thin/near-coincident Boolean fixture, reusing the J06 near-coincident
// pair (Brief section 17: "Construct a thin/near-coincident Boolean fixture
// using the same project-owned API").
[[nodiscard]] inline LinearExtrusionSpec MakeF04A(const Point3& offset = kCoordinateOffsetC0) {
    return MakeJ06A(offset);
}
[[nodiscard]] inline LinearExtrusionSpec MakeF04B(const Point3& offset = kCoordinateOffsetC0) {
    return MakeJ06B(offset);
}

} // namespace bim::geometry_occt::test_constants
