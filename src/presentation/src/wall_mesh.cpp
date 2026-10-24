#include "bim/presentation/wall_mesh.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <utility>

namespace bim::presentation {
namespace {

using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::viewport::Normal3f;
using bim::viewport::Position3f;
using bim::viewport::RenderMeshData;

// Calculations run in double precision on this small private vector type. It is
// deliberately not the viewport's own Vector3: that type's Normalize() silently
// substitutes a zero vector for a short input, and this module must instead
// classify the failure.
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

[[nodiscard]] Vec3 Add(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

[[nodiscard]] Vec3 Scale(const Vec3& a, double s) noexcept {
    return Vec3{a.x * s, a.y * s, a.z * s};
}

[[nodiscard]] Vec3 Negate(const Vec3& a) noexcept {
    return Vec3{-a.x, -a.y, -a.z};
}

[[nodiscard]] double Dot(const Vec3& a, const Vec3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] Vec3 Cross(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

[[nodiscard]] bool AllFinite(const Vec3& v) noexcept {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

[[nodiscard]] WallMeshResult Fail(WallMeshErrorCode code) noexcept {
    // A default-constructed mesh owns no storage: a failed result never carries
    // any part of a partially built mesh.
    WallMeshResult result;
    result.code = code;
    return result;
}

// Unit vector of a finite input together with its length. The components are
// scaled by the largest magnitude first, so a huge-but-finite input does not
// overflow while squaring and a subnormal one does not underflow to zero. The
// returned length may legitimately be +infinity for an input near the double
// limit; it is only ever compared with the linear tolerance.
struct Unit {
    Vec3 direction;
    double length = 0.0;
};

[[nodiscard]] Unit Normalize(const Vec3& v) noexcept {
    const double largest = std::max({std::fabs(v.x), std::fabs(v.y), std::fabs(v.z)});
    if (!(largest > 0.0)) {
        return Unit{Vec3{}, 0.0};
    }
    const Vec3 scaled{v.x / largest, v.y / largest, v.z / largest};
    const double scaled_length = std::sqrt(Dot(scaled, scaled));
    return Unit{Vec3{scaled.x / scaled_length, scaled.y / scaled_length, scaled.z / scaled_length},
                largest * scaled_length};
}

// Checked double -> float32. A value that is not finite, lies outside the finite
// float range, or converts to a non-finite float is refused (never clamped). The
// range test comes before the cast because converting an out-of-range double is
// undefined behavior. A finite value too small for float underflows to zero,
// which is accepted; every zero is returned as positive zero.
[[nodiscard]] bool ToFloat(double value, float& out) noexcept {
    constexpr double kFloatMax = static_cast<double>(std::numeric_limits<float>::max());
    if (!std::isfinite(value) || std::fabs(value) > kFloatMax) {
        return false;
    }
    float converted = static_cast<float>(value);
    if (!std::isfinite(converted)) {
        return false;
    }
    if (converted == 0.0f) {
        converted = 0.0f; // canonicalize -0.0f to +0.0f
    }
    out = converted;
    return true;
}

[[nodiscard]] bool ToPosition(const Vec3& v, Position3f& out) noexcept {
    return ToFloat(v.x, out.x) && ToFloat(v.y, out.y) && ToFloat(v.z, out.z);
}

[[nodiscard]] bool ToNormal(const Vec3& v, Normal3f& out) noexcept {
    return ToFloat(v.x, out.x) && ToFloat(v.y, out.y) && ToFloat(v.z, out.z);
}

constexpr std::size_t kFaceCount = 6;
constexpr std::size_t kVerticesPerFace = 4;

struct Face {
    std::array<Vec3, kVerticesPerFace> corners;
    Vec3 normal;
};

} // namespace

WallMeshResult BuildWallMesh(const LinearExtrusionSpec& spec,
                             const GeometryTolerance& tolerance) noexcept {
    try {
        const auto& profile = spec.profile;

        // --- InvalidInput: tolerance, finiteness, positive sizes -------------------
        if (!std::isfinite(tolerance.linear) || !(tolerance.linear > 0.0) ||
            !std::isfinite(tolerance.angular_radians) || !(tolerance.angular_radians > 0.0)) {
            return Fail(WallMeshErrorCode::InvalidInput);
        }

        const Vec3 origin{profile.origin.x, profile.origin.y, profile.origin.z};
        const Vec3 u_axis{profile.u_axis.x, profile.u_axis.y, profile.u_axis.z};
        const Vec3 v_axis{profile.v_axis.x, profile.v_axis.y, profile.v_axis.z};
        const Vec3 direction{spec.direction.x, spec.direction.y, spec.direction.z};
        if (!AllFinite(origin) || !AllFinite(u_axis) || !AllFinite(v_axis) ||
            !AllFinite(direction) || !std::isfinite(profile.size_u) ||
            !std::isfinite(profile.size_v) || !std::isfinite(spec.distance)) {
            return Fail(WallMeshErrorCode::InvalidInput);
        }
        if (!(profile.size_u > 0.0) || !(profile.size_v > 0.0) || !(spec.distance > 0.0)) {
            return Fail(WallMeshErrorCode::InvalidInput);
        }

        // --- DegenerateGeometry: axes, orthogonality, right-handed direction -------
        const Unit u_unit = Normalize(u_axis);
        const Unit v_unit = Normalize(v_axis);
        const Unit w_unit = Normalize(direction);
        if (u_unit.length <= tolerance.linear || v_unit.length <= tolerance.linear ||
            w_unit.length <= tolerance.linear) {
            return Fail(WallMeshErrorCode::DegenerateGeometry);
        }
        const Vec3& u = u_unit.direction;
        const Vec3& v = v_unit.direction;
        const Vec3& w = w_unit.direction;

        // U and V: the deviation from a right angle is atan2(|U.V|, |U x V|). The
        // frame is judged as supplied; it is never orthogonalized.
        const Vec3 uv_cross = Cross(u, v);
        const double uv_cross_length = std::sqrt(Dot(uv_cross, uv_cross));
        if (!(uv_cross_length > 0.0) ||
            std::atan2(std::fabs(Dot(u, v)), uv_cross_length) > tolerance.angular_radians) {
            return Fail(WallMeshErrorCode::DegenerateGeometry);
        }

        // W must follow U x V (right-handed). The angle between them is
        // atan2(|W x N|, W.N) with N = U x V normalized; an anti-parallel W is
        // rejected outright (W.N must be positive whatever the tolerance), never
        // flipped.
        const Vec3 normal_uv = Scale(uv_cross, 1.0 / uv_cross_length);
        const Vec3 w_cross = Cross(w, normal_uv);
        const double w_dot = Dot(w, normal_uv);
        if (!(w_dot > 0.0) ||
            std::atan2(std::sqrt(Dot(w_cross, w_cross)), w_dot) > tolerance.angular_radians) {
            return Fail(WallMeshErrorCode::DegenerateGeometry);
        }

        // --- Corners (double) --------------------------------------------------------
        const Vec3 u_step = Scale(u, profile.size_u);
        const Vec3 v_step = Scale(v, profile.size_v);
        const Vec3 w_step = Scale(w, spec.distance);

        const Vec3 p0 = origin;
        const Vec3 p1 = Add(origin, u_step);
        const Vec3 p2 = Add(Add(origin, u_step), v_step);
        const Vec3 p3 = Add(origin, v_step);
        const Vec3 q0 = Add(p0, w_step);
        const Vec3 q1 = Add(p1, w_step);
        const Vec3 q2 = Add(p2, w_step);
        const Vec3 q3 = Add(p3, w_step);

        // Frozen face order: Bottom, Top, Start, End, SideA, SideB.
        const std::array<Face, kFaceCount> faces{{
            Face{{p0, p3, p2, p1}, Negate(w)},
            Face{{q0, q1, q2, q3}, w},
            Face{{p0, q0, q3, p3}, Negate(u)},
            Face{{p1, p2, q2, q1}, u},
            Face{{p0, p1, q1, q0}, Negate(v)},
            Face{{p3, q3, q2, p2}, v},
        }};

        // --- Checked float32 conversion into a local mesh ------------------------------
        RenderMeshData mesh;
        mesh.topology = bim::viewport::MeshTopology::TriangleList;
        mesh.positions.reserve(kFaceCount * kVerticesPerFace);
        mesh.normals.reserve(kFaceCount * kVerticesPerFace);
        mesh.indices.reserve(kFaceCount * 6);

        for (std::size_t face_index = 0; face_index < kFaceCount; ++face_index) {
            const Face& face = faces[face_index];
            Normal3f normal;
            if (!ToNormal(face.normal, normal)) {
                return Fail(WallMeshErrorCode::CoordinateOutOfRange);
            }
            const auto base = static_cast<std::uint32_t>(face_index * kVerticesPerFace);
            for (const Vec3& corner : face.corners) {
                Position3f position;
                if (!ToPosition(corner, position)) {
                    return Fail(WallMeshErrorCode::CoordinateOutOfRange);
                }
                mesh.positions.push_back(position);
                mesh.normals.push_back(normal);
            }
            // Two triangles per face: (0,1,2) and (0,2,3), CCW seen from outside.
            for (const std::uint32_t local : {0U, 1U, 2U, 0U, 2U, 3U}) {
                mesh.indices.push_back(base + local);
            }
        }

        if (!bim::viewport::ValidateMesh(mesh).IsOk()) {
            return Fail(WallMeshErrorCode::InvalidRenderMesh);
        }

        WallMeshResult result;
        result.mesh = std::move(mesh);
        return result;
    } catch (...) {
        // Allocation or standard-library failure: nothing partial escapes.
        return Fail(WallMeshErrorCode::InternalFailure);
    }
}

} // namespace bim::presentation
