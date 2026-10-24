// P1-T005 Geometry Tessellation & Viewport Bridge - unit proof of
// bim::presentation::BuildWallMesh (Architecture Gate BIM-AG-P1-T005; Implementation
// Brief P1-T005). Links ONLY bim::presentation (and through it geometry_api and
// viewport) and Catch2: no document, model or renderer is involved. Every spec is a
// neutral LinearExtrusionSpec written out by hand.
#include "bim/presentation/wall_mesh.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace {

using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::Point3;
using bim::geometry_api::Vector3;
using bim::presentation::BuildWallMesh;
using bim::presentation::WallMeshErrorCode;
using bim::presentation::WallMeshResult;
using bim::viewport::Normal3f;
using bim::viewport::Position3f;
using bim::viewport::RenderMeshData;

static_assert(noexcept(BuildWallMesh(std::declval<const LinearExtrusionSpec&>(),
                                     std::declval<const GeometryTolerance&>())));
static_assert(std::is_same_v<std::underlying_type_t<WallMeshErrorCode>, std::uint8_t>);

constexpr GeometryTolerance kTolerance{1.0e-9, 1.0e-9};
constexpr double kPi = 3.14159265358979323846;

LinearExtrusionSpec MakeSpec(Point3 origin, Vector3 u, Vector3 v, double size_u, double size_v,
                             Vector3 w, double distance) {
    LinearExtrusionSpec spec;
    spec.profile.origin = origin;
    spec.profile.u_axis = u;
    spec.profile.v_axis = v;
    spec.profile.size_u = size_u;
    spec.profile.size_v = size_v;
    spec.direction = w;
    spec.distance = distance;
    return spec;
}

// origin (1,2,3); U=+X length 4; V=+Y length 0.5; W=+Z height 3. All values are exactly
// representable in float32, so the expected mesh below is exact.
LinearExtrusionSpec AxisAlignedSpec() {
    return MakeSpec(Point3{1.0, 2.0, 3.0}, Vector3{1.0, 0.0, 0.0}, Vector3{0.0, 1.0, 0.0}, 4.0, 0.5,
                    Vector3{0.0, 0.0, 1.0}, 3.0);
}

LinearExtrusionSpec RotatedSpec(double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    return MakeSpec(Point3{-7.25, 3.5, 12.0}, Vector3{c, s, 0.0}, Vector3{-s, c, 0.0}, 6.0, 0.3,
                    Vector3{0.0, 0.0, 1.0}, 2.75);
}

WallMeshResult Build(const LinearExtrusionSpec& spec) {
    return BuildWallMesh(spec, kTolerance);
}

void RequireEmptyFailure(const WallMeshResult& result, WallMeshErrorCode code) {
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.code == code);
    REQUIRE(result.mesh.positions.empty());
    REQUIRE(result.mesh.normals.empty());
    REQUIRE(result.mesh.indices.empty());
}

void RequireSuccess(const WallMeshResult& result) {
    INFO(static_cast<int>(result.code));
    REQUIRE(result.ok());
    REQUIRE(result.code == WallMeshErrorCode::None);
}

struct V {
    double x, y, z;
};

V ToV(const Position3f& p) {
    return V{p.x, p.y, p.z};
}
V Sub(const V& a, const V& b) {
    return V{a.x - b.x, a.y - b.y, a.z - b.z};
}
V Cross(const V& a, const V& b) {
    return V{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
double Dot(const V& a, const V& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

std::uint32_t Bits(float f) {
    return std::bit_cast<std::uint32_t>(f);
}

bool SameBits(const RenderMeshData& a, const RenderMeshData& b) {
    if (a.positions.size() != b.positions.size() || a.normals.size() != b.normals.size() ||
        a.indices != b.indices || a.topology != b.topology) {
        return false;
    }
    for (std::size_t i = 0; i < a.positions.size(); ++i) {
        if (Bits(a.positions[i].x) != Bits(b.positions[i].x) ||
            Bits(a.positions[i].y) != Bits(b.positions[i].y) ||
            Bits(a.positions[i].z) != Bits(b.positions[i].z) ||
            Bits(a.normals[i].x) != Bits(b.normals[i].x) ||
            Bits(a.normals[i].y) != Bits(b.normals[i].y) ||
            Bits(a.normals[i].z) != Bits(b.normals[i].z)) {
            return false;
        }
    }
    return true;
}

// No component of the mesh may be negative zero.
void RequireNoNegativeZero(const RenderMeshData& mesh) {
    const auto positive_zero = [](float f) { return !(f == 0.0f && std::signbit(f)); };
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        REQUIRE(positive_zero(mesh.positions[i].x));
        REQUIRE(positive_zero(mesh.positions[i].y));
        REQUIRE(positive_zero(mesh.positions[i].z));
        REQUIRE(positive_zero(mesh.normals[i].x));
        REQUIRE(positive_zero(mesh.normals[i].y));
        REQUIRE(positive_zero(mesh.normals[i].z));
    }
}

// Every face: the two triangles are CCW seen from outside (their geometric normal
// agrees with the supplied normal and points away from the prism's centroid), and the
// four vertices of a face share one normal.
void RequireOutwardCcwFaces(const RenderMeshData& mesh) {
    REQUIRE(mesh.positions.size() == 24);
    V centroid{0.0, 0.0, 0.0};
    for (const Position3f& p : mesh.positions) {
        centroid.x += p.x / 24.0;
        centroid.y += p.y / 24.0;
        centroid.z += p.z / 24.0;
    }
    for (std::size_t tri = 0; tri < 12; ++tri) {
        const V a = ToV(mesh.positions[mesh.indices[tri * 3 + 0]]);
        const V b = ToV(mesh.positions[mesh.indices[tri * 3 + 1]]);
        const V c = ToV(mesh.positions[mesh.indices[tri * 3 + 2]]);
        const V geometric = Cross(Sub(b, a), Sub(c, a));
        const Normal3f& n = mesh.normals[mesh.indices[tri * 3 + 0]];
        INFO("triangle " << tri);
        REQUIRE(Dot(geometric, V{n.x, n.y, n.z}) > 0.0);
        const V center{(a.x + b.x + c.x) / 3.0, (a.y + b.y + c.y) / 3.0, (a.z + b.z + c.z) / 3.0};
        REQUIRE(Dot(Sub(center, centroid), V{n.x, n.y, n.z}) > 0.0);
    }
    for (std::size_t face = 0; face < 6; ++face) {
        for (std::size_t k = 1; k < 4; ++k) {
            REQUIRE(Bits(mesh.normals[face * 4 + k].x) == Bits(mesh.normals[face * 4].x));
            REQUIRE(Bits(mesh.normals[face * 4 + k].y) == Bits(mesh.normals[face * 4].y));
            REQUIRE(Bits(mesh.normals[face * 4 + k].z) == Bits(mesh.normals[face * 4].z));
        }
    }
}

} // namespace

TEST_CASE("an axis-aligned wall yields the exact frozen mesh", "[p1_t005][wall_mesh]") {
    const WallMeshResult result = Build(AxisAlignedSpec());
    RequireSuccess(result);
    const RenderMeshData& mesh = result.mesh;

    REQUIRE(mesh.topology == bim::viewport::MeshTopology::TriangleList);
    REQUIRE(mesh.positions.size() == 24);
    REQUIRE(mesh.normals.size() == 24);
    REQUIRE(mesh.indices.size() == 36);
    REQUIRE(mesh.indices.size() / 3 == 12);

    // p0=(1,2,3) p1=(5,2,3) p2=(5,2.5,3) p3=(1,2.5,3); q = p + (0,0,3).
    const std::array<Position3f, 8> corner{{
        {1.0f, 2.0f, 3.0f}, // p0
        {5.0f, 2.0f, 3.0f}, // p1
        {5.0f, 2.5f, 3.0f}, // p2
        {1.0f, 2.5f, 3.0f}, // p3
        {1.0f, 2.0f, 6.0f}, // q0
        {5.0f, 2.0f, 6.0f}, // q1
        {5.0f, 2.5f, 6.0f}, // q2
        {1.0f, 2.5f, 6.0f}, // q3
    }};
    enum { p0, p1, p2, p3, q0, q1, q2, q3 };
    // Face order Bottom, Top, Start, End, SideA, SideB.
    const std::array<std::array<int, 4>, 6> face_corners{{
        {p0, p3, p2, p1},
        {q0, q1, q2, q3},
        {p0, q0, q3, p3},
        {p1, p2, q2, q1},
        {p0, p1, q1, q0},
        {p3, q3, q2, p2},
    }};
    const std::array<Normal3f, 6> face_normal{{
        {0.0f, 0.0f, -1.0f}, // Bottom  -W
        {0.0f, 0.0f, 1.0f},  // Top     +W
        {-1.0f, 0.0f, 0.0f}, // Start   -U
        {1.0f, 0.0f, 0.0f},  // End     +U
        {0.0f, -1.0f, 0.0f}, // SideA   -V
        {0.0f, 1.0f, 0.0f},  // SideB   +V
    }};

    for (std::size_t face = 0; face < 6; ++face) {
        INFO("face " << face);
        for (std::size_t k = 0; k < 4; ++k) {
            const Position3f& expected_p = corner[static_cast<std::size_t>(face_corners[face][k])];
            const Position3f& p = mesh.positions[face * 4 + k];
            REQUIRE(Bits(p.x) == Bits(expected_p.x));
            REQUIRE(Bits(p.y) == Bits(expected_p.y));
            REQUIRE(Bits(p.z) == Bits(expected_p.z));
            const Normal3f& n = mesh.normals[face * 4 + k];
            REQUIRE(Bits(n.x) == Bits(face_normal[face].x));
            REQUIRE(Bits(n.y) == Bits(face_normal[face].y));
            REQUIRE(Bits(n.z) == Bits(face_normal[face].z));
        }
        const auto base = static_cast<std::uint32_t>(face * 4);
        const std::array<std::uint32_t, 6> expected_indices{base, base + 1, base + 2,
                                                            base, base + 2, base + 3};
        for (std::size_t k = 0; k < 6; ++k) {
            REQUIRE(mesh.indices[face * 6 + k] == expected_indices[k]);
        }
    }
    RequireNoNegativeZero(mesh);
    RequireOutwardCcwFaces(mesh);
}

TEST_CASE("the mesh passes the viewport structural validation", "[p1_t005][wall_mesh]") {
    for (const LinearExtrusionSpec& spec :
         {AxisAlignedSpec(), RotatedSpec(0.6), RotatedSpec(-2.4), RotatedSpec(kPi / 2.0)}) {
        const WallMeshResult result = Build(spec);
        RequireSuccess(result);
        REQUIRE(bim::viewport::ValidateMesh(result.mesh).IsOk());
        for (const std::uint32_t index : result.mesh.indices) {
            REQUIRE(index < result.mesh.positions.size());
        }
        REQUIRE(result.mesh.normals.size() == result.mesh.positions.size());
    }
}

TEST_CASE("a rotated plan wall is tessellated in its own frame", "[p1_t005][wall_mesh]") {
    for (const double angle : {0.3, 1.0, kPi / 4.0, 2.0, -0.7, kPi, 5.5}) {
        INFO("angle " << angle);
        const LinearExtrusionSpec spec = RotatedSpec(angle);
        const WallMeshResult result = Build(spec);
        RequireSuccess(result);
        const RenderMeshData& mesh = result.mesh;

        const double c = std::cos(angle);
        const double s = std::sin(angle);
        const double ox = spec.profile.origin.x;
        const double oy = spec.profile.origin.y;
        const double oz = spec.profile.origin.z;
        const double su = spec.profile.size_u;
        const double sv = spec.profile.size_v;
        const double h = spec.distance;
        constexpr double kEps = 1.0e-5;

        // Bottom face = p0, p3, p2, p1.
        const std::array<V, 4> bottom{{
            {ox, oy, oz},
            {ox - s * sv, oy + c * sv, oz},
            {ox + c * su - s * sv, oy + s * su + c * sv, oz},
            {ox + c * su, oy + s * su, oz},
        }};
        for (std::size_t k = 0; k < 4; ++k) {
            REQUIRE(std::fabs(mesh.positions[k].x - bottom[k].x) < kEps);
            REQUIRE(std::fabs(mesh.positions[k].y - bottom[k].y) < kEps);
            REQUIRE(std::fabs(mesh.positions[k].z - bottom[k].z) < kEps);
            // Top face (indices 4..7) = q0, q1, q2, q3: same plan point lifted by h.
        }
        REQUIRE(std::fabs(mesh.positions[4].z - (oz + h)) < kEps);
        REQUIRE(std::fabs(mesh.positions[4].x - ox) < kEps);
        // Start face normal is -U; End face normal is +U; SideA is -V; SideB is +V.
        REQUIRE(std::fabs(mesh.normals[8].x - (-c)) < kEps);
        REQUIRE(std::fabs(mesh.normals[8].y - (-s)) < kEps);
        REQUIRE(std::fabs(mesh.normals[12].x - c) < kEps);
        REQUIRE(std::fabs(mesh.normals[12].y - s) < kEps);
        REQUIRE(std::fabs(mesh.normals[16].x - s) < kEps);
        REQUIRE(std::fabs(mesh.normals[16].y - (-c)) < kEps);
        REQUIRE(std::fabs(mesh.normals[20].x - (-s)) < kEps);
        REQUIRE(std::fabs(mesh.normals[20].y - c) < kEps);

        RequireOutwardCcwFaces(mesh);
        RequireNoNegativeZero(mesh);
    }
}

TEST_CASE("non-unit axes and direction are normalized before scaling", "[p1_t005][wall_mesh]") {
    const WallMeshResult reference = Build(AxisAlignedSpec());
    RequireSuccess(reference);

    // Same wall with axes and direction of other lengths: normalization is exact here.
    const LinearExtrusionSpec scaled =
        MakeSpec(Point3{1.0, 2.0, 3.0}, Vector3{8.0, 0.0, 0.0}, Vector3{0.0, 0.125, 0.0}, 4.0, 0.5,
                 Vector3{0.0, 0.0, 32.0}, 3.0);
    const WallMeshResult result = Build(scaled);
    RequireSuccess(result);
    REQUIRE(SameBits(result.mesh, reference.mesh));

    // A rotated frame given with 5x longer vectors matches the unit frame (3-4-5).
    const LinearExtrusionSpec unit_345 =
        MakeSpec(Point3{0.0, 0.0, 0.0}, Vector3{0.6, 0.8, 0.0}, Vector3{-0.8, 0.6, 0.0}, 10.0, 2.0,
                 Vector3{0.0, 0.0, 1.0}, 3.0);
    const LinearExtrusionSpec long_345 =
        MakeSpec(Point3{0.0, 0.0, 0.0}, Vector3{3.0, 4.0, 0.0}, Vector3{-8.0, 6.0, 0.0}, 10.0, 2.0,
                 Vector3{0.0, 0.0, 7.0}, 3.0);
    const WallMeshResult a = Build(unit_345);
    const WallMeshResult b = Build(long_345);
    RequireSuccess(a);
    RequireSuccess(b);
    for (std::size_t i = 0; i < 24; ++i) {
        REQUIRE(std::fabs(a.mesh.positions[i].x - b.mesh.positions[i].x) < 1.0e-5f);
        REQUIRE(std::fabs(a.mesh.positions[i].y - b.mesh.positions[i].y) < 1.0e-5f);
        REQUIRE(std::fabs(a.mesh.positions[i].z - b.mesh.positions[i].z) < 1.0e-5f);
        REQUIRE(std::fabs(a.mesh.normals[i].x - b.mesh.normals[i].x) < 1.0e-6f);
        REQUIRE(std::fabs(a.mesh.normals[i].y - b.mesh.normals[i].y) < 1.0e-6f);
    }

    // The dimensions come from size_u / size_v / distance, never from the vector lengths.
    REQUIRE(a.mesh.positions[2].x == b.mesh.positions[2].x); // p2 of Bottom
    const double expected_p1x = 0.6 * 10.0;
    REQUIRE(std::fabs(b.mesh.positions[3].x - expected_p1x) < 1.0e-5); // p1 of Bottom

    // Axes far smaller or larger than one still normalize (scaled, no overflow/underflow).
    const LinearExtrusionSpec extreme =
        MakeSpec(Point3{0.0, 0.0, 0.0}, Vector3{1.0e150, 0.0, 0.0}, Vector3{0.0, 1.0e-150, 0.0},
                 2.0, 1.0, Vector3{0.0, 0.0, 1.0e200}, 1.0);
    const WallMeshResult extreme_result = BuildWallMesh(extreme, GeometryTolerance{1.0e-160, 1e-9});
    RequireSuccess(extreme_result);
    REQUIRE(extreme_result.mesh.positions[3].x == 2.0f); // p1 = origin + U * 2
    REQUIRE(extreme_result.mesh.positions[1].y == 1.0f); // p3 = origin + V * 1
}

TEST_CASE("conversion is deterministic down to the float bit patterns", "[p1_t005][wall_mesh]") {
    const LinearExtrusionSpec spec = RotatedSpec(0.123456789);
    const WallMeshResult first = Build(spec);
    RequireSuccess(first);
    for (int i = 0; i < 5; ++i) {
        const WallMeshResult again = Build(spec);
        RequireSuccess(again);
        REQUIRE(SameBits(first.mesh, again.mesh));
    }
    // A copy of the spec gives the same bits as the original.
    const LinearExtrusionSpec copy = spec;
    REQUIRE(SameBits(Build(copy).mesh, first.mesh));
    // A different wall gives a different mesh (the conversion is not constant).
    REQUIRE_FALSE(SameBits(Build(RotatedSpec(0.2)).mesh, first.mesh));
}

TEST_CASE("every produced zero is positive zero", "[p1_t005][wall_mesh]") {
    // Negative zero in origin and axes, and a negated zero normal component (-W has
    // -0 x/y components before canonicalization).
    LinearExtrusionSpec spec = AxisAlignedSpec();
    spec.profile.origin = Point3{-0.0, -0.0, -0.0};
    spec.profile.u_axis = Vector3{1.0, -0.0, -0.0};
    spec.profile.v_axis = Vector3{-0.0, 1.0, -0.0};
    spec.direction = Vector3{-0.0, -0.0, 1.0};
    const WallMeshResult result = Build(spec);
    RequireSuccess(result);
    RequireNoNegativeZero(result.mesh);
    // And there really are zero components that would have been negative zero.
    REQUIRE(Bits(result.mesh.positions[0].x) == Bits(0.0f));
    REQUIRE(Bits(result.mesh.normals[0].x) == Bits(0.0f)); // Bottom normal = -W, x = -(-0)

    // A tiny negative value underflows to zero in float32 and comes out as +0.0f.
    LinearExtrusionSpec tiny = AxisAlignedSpec();
    tiny.profile.origin = Point3{-1.0e-300, 1.0e-300, -1.0e-60};
    const WallMeshResult tiny_result = Build(tiny);
    RequireSuccess(tiny_result);
    REQUIRE(Bits(tiny_result.mesh.positions[0].x) == Bits(0.0f));
    REQUIRE(Bits(tiny_result.mesh.positions[0].y) == Bits(0.0f));
    REQUIRE(Bits(tiny_result.mesh.positions[0].z) == Bits(0.0f));
    RequireNoNegativeZero(tiny_result.mesh);
}

TEST_CASE("an invalid tolerance is rejected as invalid input", "[p1_t005][wall_mesh]") {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const std::vector<GeometryTolerance> bad = {
        {0.0, 1.0e-9},  {-1.0e-9, 1.0e-9}, {nan, 1.0e-9}, {inf, 1.0e-9},
        {1.0e-9, 0.0},  {1.0e-9, -1.0e-9}, {1.0e-9, nan}, {1.0e-9, inf},
        {-inf, 1.0e-9}, {1.0e-9, -inf},    {0.0, 0.0},
    };
    for (const GeometryTolerance& tolerance : bad) {
        INFO(tolerance.linear << " / " << tolerance.angular_radians);
        RequireEmptyFailure(BuildWallMesh(AxisAlignedSpec(), tolerance),
                            WallMeshErrorCode::InvalidInput);
    }
    // A valid tolerance with the same spec succeeds.
    RequireSuccess(BuildWallMesh(AxisAlignedSpec(), GeometryTolerance{1.0e-6, 1.0e-6}));
}

TEST_CASE("non-finite input values are rejected as invalid input", "[p1_t005][wall_mesh]") {
    const std::array<double, 3> bad_values = {std::numeric_limits<double>::quiet_NaN(),
                                              std::numeric_limits<double>::infinity(),
                                              -std::numeric_limits<double>::infinity()};
    // Every one of the 15 scalar fields, one at a time.
    for (const double bad : bad_values) {
        for (int field = 0; field < 15; ++field) {
            LinearExtrusionSpec spec = AxisAlignedSpec();
            double* targets[15] = {
                &spec.profile.origin.x, &spec.profile.origin.y, &spec.profile.origin.z,
                &spec.profile.u_axis.x, &spec.profile.u_axis.y, &spec.profile.u_axis.z,
                &spec.profile.v_axis.x, &spec.profile.v_axis.y, &spec.profile.v_axis.z,
                &spec.profile.size_u,   &spec.profile.size_v,   &spec.direction.x,
                &spec.direction.y,      &spec.direction.z,      &spec.distance};
            *targets[field] = bad;
            INFO("field " << field << " value " << bad);
            RequireEmptyFailure(Build(spec), WallMeshErrorCode::InvalidInput);
        }
    }
}

TEST_CASE("non-positive dimensions are rejected as invalid input", "[p1_t005][wall_mesh]") {
    for (const double bad : {0.0, -0.0, -1.0, -1.0e-300, -1.0e300}) {
        INFO("value " << bad);
        LinearExtrusionSpec su = AxisAlignedSpec();
        su.profile.size_u = bad;
        RequireEmptyFailure(Build(su), WallMeshErrorCode::InvalidInput);
        LinearExtrusionSpec sv = AxisAlignedSpec();
        sv.profile.size_v = bad;
        RequireEmptyFailure(Build(sv), WallMeshErrorCode::InvalidInput);
        LinearExtrusionSpec dist = AxisAlignedSpec();
        dist.distance = bad;
        RequireEmptyFailure(Build(dist), WallMeshErrorCode::InvalidInput);
    }
    // The smallest positive dimension is valid.
    LinearExtrusionSpec small = AxisAlignedSpec();
    small.profile.size_u = std::numeric_limits<double>::denorm_min();
    RequireSuccess(Build(small));
}

TEST_CASE("degenerate axes and direction are rejected", "[p1_t005][wall_mesh]") {
    const Vector3 zero{0.0, 0.0, 0.0};
    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.u_axis = zero;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::DegenerateGeometry);
    }
    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.v_axis = zero;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::DegenerateGeometry);
    }
    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.direction = zero;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::DegenerateGeometry);
    }
    // Length at or below the linear tolerance is degenerate; just above is not.
    const GeometryTolerance tolerance{1.0e-3, 1.0e-9};
    {
        LinearExtrusionSpec at = AxisAlignedSpec();
        at.profile.u_axis = Vector3{1.0e-3, 0.0, 0.0};
        RequireEmptyFailure(BuildWallMesh(at, tolerance), WallMeshErrorCode::DegenerateGeometry);
        LinearExtrusionSpec below = AxisAlignedSpec();
        below.profile.v_axis = Vector3{0.0, 5.0e-4, 0.0};
        RequireEmptyFailure(BuildWallMesh(below, tolerance), WallMeshErrorCode::DegenerateGeometry);
        LinearExtrusionSpec dir = AxisAlignedSpec();
        dir.direction = Vector3{0.0, 0.0, 1.0e-3};
        RequireEmptyFailure(BuildWallMesh(dir, tolerance), WallMeshErrorCode::DegenerateGeometry);
        LinearExtrusionSpec above = AxisAlignedSpec();
        above.profile.u_axis = Vector3{2.0e-3, 0.0, 0.0};
        above.profile.v_axis = Vector3{0.0, 2.0e-3, 0.0};
        above.direction = Vector3{0.0, 0.0, 2.0e-3};
        const WallMeshResult ok = BuildWallMesh(above, tolerance);
        RequireSuccess(ok);
        REQUIRE(SameBits(ok.mesh, Build(AxisAlignedSpec()).mesh)); // normalized: same wall
    }
    // Parallel and anti-parallel U / V.
    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.v_axis = Vector3{1.0, 0.0, 0.0};
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::DegenerateGeometry);
        spec.profile.v_axis = Vector3{-2.0, 0.0, 0.0};
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::DegenerateGeometry);
        // Even a huge angular tolerance cannot accept a collapsed profile.
        RequireEmptyFailure(BuildWallMesh(spec, GeometryTolerance{1.0e-9, 4.0}),
                            WallMeshErrorCode::DegenerateGeometry);
    }
}

TEST_CASE("skew U and V are rejected, never orthogonalized", "[p1_t005][wall_mesh]") {
    const GeometryTolerance tight{1.0e-9, 1.0e-6};
    for (const double skew : {1.0e-3, 0.05, 0.3, -0.3, 1.0}) {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.v_axis = Vector3{std::sin(skew), std::cos(skew), 0.0};
        INFO("skew " << skew);
        RequireEmptyFailure(BuildWallMesh(spec, tight), WallMeshErrorCode::DegenerateGeometry);
    }
    // A skew inside the angular tolerance is accepted AS SUPPLIED: p3 uses the given V.
    const double tiny = 1.0e-8;
    LinearExtrusionSpec spec = AxisAlignedSpec();
    spec.profile.v_axis = Vector3{std::sin(tiny), std::cos(tiny), 0.0};
    const WallMeshResult result = BuildWallMesh(spec, tight);
    RequireSuccess(result);
    const double p3x = 1.0 + std::sin(tiny) * 0.5;
    REQUIRE(Bits(result.mesh.positions[1].x) == Bits(static_cast<float>(p3x))); // Bottom: p0,p3,..
    REQUIRE(Bits(result.mesh.positions[1].y) ==
            Bits(static_cast<float>(2.0 + std::cos(tiny) * 0.5)));
    // The same skew with a tolerance below it is rejected.
    RequireEmptyFailure(BuildWallMesh(spec, GeometryTolerance{1.0e-9, 1.0e-10}),
                        WallMeshErrorCode::DegenerateGeometry);
}

TEST_CASE("the direction must follow U x V; the frame is never flipped", "[p1_t005][wall_mesh]") {
    const GeometryTolerance tolerance{1.0e-9, 1.0e-6};
    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.direction = Vector3{0.0, 0.0, -1.0}; // anti-parallel to U x V
        RequireEmptyFailure(BuildWallMesh(spec, tolerance), WallMeshErrorCode::DegenerateGeometry);
        // Not even an absurdly loose angular tolerance accepts an anti-parallel direction.
        RequireEmptyFailure(BuildWallMesh(spec, GeometryTolerance{1.0e-9, 10.0}),
                            WallMeshErrorCode::DegenerateGeometry);
    }
    {
        // A left-handed frame (V flipped) makes +Z anti-parallel to U x V.
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.v_axis = Vector3{0.0, -1.0, 0.0};
        RequireEmptyFailure(BuildWallMesh(spec, tolerance), WallMeshErrorCode::DegenerateGeometry);
        // With the direction flipped too the frame is right-handed again and valid.
        spec.direction = Vector3{0.0, 0.0, -1.0};
        const WallMeshResult flipped = BuildWallMesh(spec, tolerance);
        RequireSuccess(flipped);
        RequireOutwardCcwFaces(flipped.mesh);
    }
    for (const Vector3 bad :
         {Vector3{1.0, 0.0, 0.0}, Vector3{0.0, 1.0, 0.0}, Vector3{1.0, 1.0, 1.0},
          Vector3{0.0, std::sin(0.2), std::cos(0.2)}, Vector3{0.0, 0.0, 1.0}}) {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.direction = bad;
        const WallMeshResult result = BuildWallMesh(spec, tolerance);
        if (bad.z == 1.0 && bad.x == 0.0 && bad.y == 0.0) {
            RequireSuccess(result); // the valid direction, as a control
        } else {
            RequireEmptyFailure(result, WallMeshErrorCode::DegenerateGeometry);
        }
    }
    // A direction off by less than the tolerance is accepted as supplied.
    LinearExtrusionSpec near = AxisAlignedSpec();
    near.direction = Vector3{std::sin(1.0e-8), 0.0, std::cos(1.0e-8)};
    RequireSuccess(BuildWallMesh(near, tolerance));
    near.direction = Vector3{std::sin(1.0e-4), 0.0, std::cos(1.0e-4)};
    RequireEmptyFailure(BuildWallMesh(near, tolerance), WallMeshErrorCode::DegenerateGeometry);
}

TEST_CASE("coordinates that do not fit a finite float32 are rejected", "[p1_t005][wall_mesh]") {
    constexpr double kFloatMax = static_cast<double>(std::numeric_limits<float>::max());
    const double just_over = std::nextafter(kFloatMax, std::numeric_limits<double>::infinity());

    {
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin.x = 1.0e39;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
        spec.profile.origin.x = -1.0e39;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
    }
    {
        // The largest finite float is accepted ...
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin = Point3{kFloatMax, -kFloatMax, kFloatMax};
        spec.profile.size_u = 1.0;
        const WallMeshResult edge = Build(spec);
        RequireSuccess(edge);
        REQUIRE(edge.mesh.positions[0].x == std::numeric_limits<float>::max());
        REQUIRE(edge.mesh.positions[0].y == -std::numeric_limits<float>::max());
        // ... and the next double above it is refused, not clamped.
        spec.profile.origin.x = just_over;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
        spec.profile.origin.x = -just_over;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
    }
    {
        // Values between FLT_MAX and the double that would round to infinity are
        // outside the finite float range too: no silent rounding down.
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin.z = kFloatMax * (1.0 + 1.0e-10);
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
    }
    {
        // A derived corner out of range although every input is in range.
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin.x = 3.0e38;
        spec.profile.size_u = 3.0e38;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
        spec = AxisAlignedSpec();
        spec.profile.size_v = 1.0e39;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
        spec = AxisAlignedSpec();
        spec.distance = 1.0e39;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
    }
    {
        // A double overflow inside the calculation is a range failure, not a crash.
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin.x = 1.7e308;
        spec.profile.size_u = 1.7e308;
        RequireEmptyFailure(Build(spec), WallMeshErrorCode::CoordinateOutOfRange);
    }
    {
        // Underflow is not an error: a finite value too small for float becomes zero.
        LinearExtrusionSpec spec = AxisAlignedSpec();
        spec.profile.origin = Point3{1.0e-50, -1.0e-50, 1.0e-300};
        const WallMeshResult result = Build(spec);
        RequireSuccess(result);
        REQUIRE(Bits(result.mesh.positions[0].x) == Bits(0.0f));
        RequireNoNegativeZero(result.mesh);
    }
}

TEST_CASE("a failure never exposes a partial mesh", "[p1_t005][wall_mesh]") {
    // The out-of-range corner is the LAST one converted (q3 of the last face); the
    // earlier faces were already built when it failed.
    LinearExtrusionSpec spec = AxisAlignedSpec();
    spec.distance = 1.0e39;
    const WallMeshResult result = Build(spec);
    RequireEmptyFailure(result, WallMeshErrorCode::CoordinateOutOfRange);

    // A default result is a successful empty value only by construction; the
    // contract: ok() <=> code == None.
    const WallMeshResult fresh;
    REQUIRE(fresh.ok());
    REQUIRE(fresh.code == WallMeshErrorCode::None);
    REQUIRE(fresh.mesh.positions.empty());
    for (const WallMeshErrorCode code :
         {WallMeshErrorCode::InvalidInput, WallMeshErrorCode::DegenerateGeometry,
          WallMeshErrorCode::CoordinateOutOfRange, WallMeshErrorCode::InvalidRenderMesh,
          WallMeshErrorCode::InternalFailure}) {
        WallMeshResult failed;
        failed.code = code;
        REQUIRE_FALSE(failed.ok());
    }
}
