#include "bim/model/wall_face_reference_resolver.hpp"

#include "bim/model/validation.hpp"

#include <array>
#include <cmath>
#include <numbers>

// P1-T006 production WallFaceReference resolver (Architecture Gate
// BIM-AG-P1-T006 sections 4-8). Pure arithmetic over project-owned neutral
// types: no CAD-kernel header is included or needed. The six semantic planes
// are derived from the current LinearExtrusionSpec exactly the way the rectangle
// prism is built (profile origin, U/V corners, extrusion along W), then every
// observed planar face is tested against the requested plane independently, so
// the observation order can never influence the outcome.

namespace bim::model {

namespace {

using bim::geometry_api::FaceObservation;
using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::SurfaceKind;

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

[[nodiscard]] Vec3 Add(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}
[[nodiscard]] Vec3 Sub(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}
[[nodiscard]] Vec3 Scale(const Vec3& a, double s) noexcept {
    return Vec3{a.x * s, a.y * s, a.z * s};
}
[[nodiscard]] double Dot(const Vec3& a, const Vec3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
[[nodiscard]] Vec3 Cross(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
[[nodiscard]] bool IsFinite(const Vec3& a) noexcept {
    return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z);
}

// Euclidean length without intermediate overflow/underflow for finite input:
// scale by the largest component first.
[[nodiscard]] double Length(const Vec3& a) noexcept {
    const double m = std::fmax(std::fabs(a.x), std::fmax(std::fabs(a.y), std::fabs(a.z)));
    if (!(m > 0.0) || !std::isfinite(m)) {
        return m;
    }
    const Vec3 s = Scale(a, 1.0 / m);
    return m * std::sqrt(Dot(s, s));
}

// Normalizes `a`; false when it is not finite or its length is not strictly
// greater than `min_length`.
[[nodiscard]] bool TryNormalize(const Vec3& a, double min_length, Vec3& out) noexcept {
    if (!IsFinite(a)) {
        return false;
    }
    const double len = Length(a);
    if (!std::isfinite(len) || !(len > min_length)) {
        return false;
    }
    const double m = std::fmax(std::fabs(a.x), std::fmax(std::fabs(a.y), std::fabs(a.z)));
    const Vec3 s = Scale(a, 1.0 / m);
    const double sl = std::sqrt(Dot(s, s));
    out = Scale(s, 1.0 / sl);
    return IsFinite(out);
}

// Angle between two unit-ish vectors ignoring the sign of `b`.
[[nodiscard]] double AngleIgnoringSign(const Vec3& a, const Vec3& b) noexcept {
    return std::atan2(Length(Cross(a, b)), std::fabs(Dot(a, b)));
}

struct ExpectedPlane {
    Vec3 normal; // unit, sign not meaningful
    Vec3 point;  // a point on the plane
};

struct SemanticPlanes {
    // Index: Bottom, Top, Start, End, SideA, SideB.
    std::array<ExpectedPlane, 6> planes;
};

[[nodiscard]] bool UsableTolerance(const GeometryTolerance& tolerance) noexcept {
    return std::isfinite(tolerance.linear) && tolerance.linear > 0.0 &&
           std::isfinite(tolerance.angular_radians) && tolerance.angular_radians > 0.0 &&
           tolerance.angular_radians < std::numbers::pi / 2.0;
}

// Derives the six semantic planes of `spec`. False (fail closed) when the spec
// cannot define six distinct, well-formed planes: any non-finite value, a
// non-positive size or distance, an axis no longer than the linear tolerance,
// U and V not orthogonal within the angular tolerance, the extrusion direction
// (nearly) lying in the profile plane, or two opposite planes closer than the
// linear tolerance (the faces would be indistinguishable). Nothing is repaired.
[[nodiscard]] bool DeriveSemanticPlanes(const LinearExtrusionSpec& spec,
                                        const GeometryTolerance& tolerance,
                                        SemanticPlanes& out) noexcept {
    const auto& p = spec.profile;
    if (!std::isfinite(p.size_u) || !std::isfinite(p.size_v) || !std::isfinite(spec.distance) ||
        !(p.size_u > 0.0) || !(p.size_v > 0.0) || !(spec.distance > 0.0)) {
        return false;
    }

    const Vec3 origin{p.origin.x, p.origin.y, p.origin.z};
    if (!IsFinite(origin)) {
        return false;
    }

    Vec3 u;
    Vec3 v;
    Vec3 w;
    if (!TryNormalize(Vec3{p.u_axis.x, p.u_axis.y, p.u_axis.z}, tolerance.linear, u) ||
        !TryNormalize(Vec3{p.v_axis.x, p.v_axis.y, p.v_axis.z}, tolerance.linear, v) ||
        !TryNormalize(Vec3{spec.direction.x, spec.direction.y, spec.direction.z}, tolerance.linear,
                      w)) {
        return false;
    }

    // U and V must be orthogonal within the angular tolerance (the same rule the
    // kernel contract applies to the profile).
    if (!(AngleIgnoringSign(u, v) >= std::numbers::pi / 2.0 - tolerance.angular_radians)) {
        return false;
    }

    // Plane normals of the prism. For a perpendicular extrusion these equal
    // +-W, +-U and +-V; they are derived from the planes' spanning directions so
    // an oblique (but valid) extrusion is still described correctly.
    Vec3 n_cap;   // plane spanned by U and V      (Bottom / Top)
    Vec3 n_start; // plane spanned by V and W      (Start / End)
    Vec3 n_side;  // plane spanned by W and U      (SideA / SideB)
    const double min_sine = std::sin(tolerance.angular_radians);
    if (!TryNormalize(Cross(u, v), min_sine, n_cap) ||
        !TryNormalize(Cross(v, w), min_sine, n_start) ||
        !TryNormalize(Cross(w, u), min_sine, n_side)) {
        return false;
    }

    const Vec3 end_u = Add(origin, Scale(u, p.size_u));
    const Vec3 end_v = Add(origin, Scale(v, p.size_v));
    const Vec3 top = Add(origin, Scale(w, spec.distance));
    if (!IsFinite(end_u) || !IsFinite(end_v) || !IsFinite(top)) {
        return false;
    }

    // Opposite planes must be separable by more than the linear tolerance.
    if (!(std::fabs(Dot(Sub(top, origin), n_cap)) > tolerance.linear) ||
        !(std::fabs(Dot(Sub(end_u, origin), n_start)) > tolerance.linear) ||
        !(std::fabs(Dot(Sub(end_v, origin), n_side)) > tolerance.linear)) {
        return false;
    }

    out.planes[0] = ExpectedPlane{n_cap, origin};   // Bottom
    out.planes[1] = ExpectedPlane{n_cap, top};      // Top
    out.planes[2] = ExpectedPlane{n_start, origin}; // Start (minimum U)
    out.planes[3] = ExpectedPlane{n_start, end_u};  // End   (maximum U)
    out.planes[4] = ExpectedPlane{n_side, origin};  // SideA (minimum V)
    out.planes[5] = ExpectedPlane{n_side, end_v};   // SideB (maximum V)
    return true;
}

[[nodiscard]] bool PlaneMatches(const ExpectedPlane& expected, const FaceObservation& face,
                                const GeometryTolerance& tolerance) noexcept {
    if (face.surface_kind != SurfaceKind::Planar) {
        return false;
    }
    const Vec3 normal{face.normal.x, face.normal.y, face.normal.z};
    const Vec3 point{face.point_on_surface.x, face.point_on_surface.y, face.point_on_surface.z};
    if (!IsFinite(normal) || !IsFinite(point)) {
        return false;
    }
    Vec3 unit_normal;
    if (!TryNormalize(normal, 0.0, unit_normal)) {
        return false;
    }
    // Orientation-sign agnostic: parallel or anti-parallel both match.
    if (!(AngleIgnoringSign(unit_normal, expected.normal) <= tolerance.angular_radians)) {
        return false;
    }
    const double offset = std::fabs(Dot(Sub(point, expected.point), expected.normal));
    return offset <= tolerance.linear;
}

[[nodiscard]] std::size_t PlaneIndex(WallFaceRole role) noexcept {
    switch (role) {
        case WallFaceRole::Bottom:
            return 0;
        case WallFaceRole::Top:
            return 1;
        case WallFaceRole::Start:
            return 2;
        case WallFaceRole::End:
            return 3;
        case WallFaceRole::SideA:
            return 4;
        case WallFaceRole::SideB:
            return 5;
    }
    return 6; // unreachable for a validated role
}

} // namespace

WallFaceResolutionResult
ResolveWallFaceReference(const WallFaceReference& reference, const WallFaceFrame& current_wall,
                         const bim::geometry_api::FaceObservationResult& observed_faces,
                         const GeometryTolerance& tolerance) noexcept {
    WallFaceResolutionResult result;
    try {
        // 1. Structural validity of the reference (valid wall id, one of six roles).
        if (!ValidateWallFaceReference(reference).ok()) {
            result.status = WallFaceResolutionStatus::InvalidReference;
            return result;
        }

        // 2. Ownership, decided only once the reference itself is valid.
        if (!(reference.wall_id == current_wall.wall_id)) {
            result.status = WallFaceResolutionStatus::InvalidOwner;
            return result;
        }

        // 3. Tolerance, observation and wall frame must all be usable; none of
        // them is evidence that the face is merely absent.
        if (!UsableTolerance(tolerance) || !observed_faces.ok()) {
            result.status = WallFaceResolutionStatus::InvalidReference;
            return result;
        }

        SemanticPlanes planes;
        if (!DeriveSemanticPlanes(current_wall.spec, tolerance, planes)) {
            result.status = WallFaceResolutionStatus::InvalidReference;
            return result;
        }

        const std::size_t plane_index = PlaneIndex(reference.role);
        if (plane_index >= planes.planes.size()) {
            result.status = WallFaceResolutionStatus::InvalidReference;
            return result;
        }
        const ExpectedPlane& expected = planes.planes[plane_index];

        // 4. Every observed face is tested independently of its position.
        std::size_t match_count = 0;
        std::size_t match_index = 0;
        for (std::size_t i = 0; i < observed_faces.faces.size(); ++i) {
            if (PlaneMatches(expected, observed_faces.faces[i], tolerance)) {
                ++match_count;
                match_index = i;
            }
        }

        if (match_count == 0) {
            result.status = WallFaceResolutionStatus::Missing;
        } else if (match_count == 1) {
            result.status = WallFaceResolutionStatus::Resolved;
            result.resolved_face_index = match_index;
        } else {
            result.status = WallFaceResolutionStatus::Ambiguous;
        }
        return result;
    } catch (...) {
        return WallFaceResolutionResult{};
    }
}

} // namespace bim::model
