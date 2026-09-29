#include "bim/model/face_reference_resolver.hpp"

#include <cmath>

// P0-T008 Topological Reference Spike - resolver implementation
// (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0). Pure arithmetic over
// bim::geometry_api's neutral types only: no CAD-kernel header is included
// here, and none is needed - the whole point of this translation unit is
// that it can derive "where the six faces of a linear extrusion currently
// ought to be" from nothing but the project-owned LinearExtrusionSpec, the
// same way bim_geometry_occt's own adapter derives the actual solid from
// that same spec (src/geometry/occt/src/geometry_occt_adapter.cpp's
// MakeLinearExtrusion: origin, normalized u_axis/v_axis, corners
// p0=origin, p1=p0+u*size_u, p2=p1+v*size_v, p3=p0+v*size_v, extruded along
// the normalized direction by distance) - this file mirrors that
// construction geometrically, never by calling into the kernel.

namespace bim::model {

namespace {

using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::Point3;
using bim::geometry_api::Vector3;

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

[[nodiscard]] Vec3 ToVec(const Point3& p) noexcept { return Vec3{p.x, p.y, p.z}; }
[[nodiscard]] Vec3 ToVec(const Vector3& v) noexcept { return Vec3{v.x, v.y, v.z}; }

[[nodiscard]] Vec3 Add(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}
[[nodiscard]] Vec3 Sub(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}
[[nodiscard]] Vec3 Scale(const Vec3& a, double s) noexcept { return Vec3{a.x * s, a.y * s, a.z * s}; }
[[nodiscard]] double Dot(const Vec3& a, const Vec3& b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }
[[nodiscard]] double Length(const Vec3& a) noexcept { return std::sqrt(Dot(a, a)); }
[[nodiscard]] Vec3 Cross(const Vec3& a, const Vec3& b) noexcept {
    return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// A caller-supplied axis/direction is only usable once normalized; a
// near-zero-length input cannot be normalized meaningfully. Rather than
// dividing by (near) zero, this returns false and lets the caller fail
// closed (InvalidReference) instead of producing an undefined-direction
// plane that could accidentally "match" an unrelated face.
[[nodiscard]] bool TryNormalize(const Vec3& a, double linear_tolerance, Vec3& out) noexcept {
    const double len = Length(a);
    if (!(len > linear_tolerance)) {
        return false;
    }
    out = Scale(a, 1.0 / len);
    return true;
}

struct ExpectedPlane {
    Vec3 normal;
    Vec3 point;
};

// Derives the six roles' expected planes purely from `spec`, exactly
// mirroring the rectangle/prism construction the adapter uses to build the
// real solid. Returns false (fail closed) if `spec` is too degenerate to
// even define a direction - the resolver never guesses.
//
// B13-C1 correction: MakeLinearExtrusion validates u_axis orthogonal to
// v_axis, but never requires the extrusion direction to be perpendicular
// to the profile plane - an oblique extrusion (D not parallel to
// cross(U,V)) is valid geometry. Plane normals are therefore derived from
// the actual geometric planes each face lies in, not simply from U/V/D
// themselves (which is only correct in the perpendicular special case):
// the cap plane is the profile plane itself, spanned by U and V, so its
// normal is cross(U,V); the U-side plane is spanned by V and D (sweeping
// the u=const edge along D), so its normal is cross(V,D); the V-side plane
// is spanned by D and U, so its normal is cross(D,U). Plane matching is
// sign-agnostic (PlaneMatches below), so the sign convention of each cross
// product is not itself semantic. Each cross product is normalized through
// the same TryNormalize/linear_tolerance fail-closed path as U/V/D
// themselves - no new tolerance is introduced - so a direction that is
// (near-)coplanar with the profile (making one of these cross products
// (near-)zero) fails closed as InvalidReference rather than producing an
// undefined-direction plane.
[[nodiscard]] bool DeriveExpectedPlane(const LinearExtrusionSpec& spec, FaceRole role,
                                       double linear_tolerance, ExpectedPlane& out) noexcept {
    Vec3 u_dir;
    Vec3 v_dir;
    Vec3 extrude_dir;
    if (!TryNormalize(ToVec(spec.profile.u_axis), linear_tolerance, u_dir) ||
        !TryNormalize(ToVec(spec.profile.v_axis), linear_tolerance, v_dir) ||
        !TryNormalize(ToVec(spec.direction), linear_tolerance, extrude_dir)) {
        return false;
    }

    Vec3 cap_normal;
    Vec3 u_side_normal; // plane spanned by V and D
    Vec3 v_side_normal; // plane spanned by D and U
    if (!TryNormalize(Cross(u_dir, v_dir), linear_tolerance, cap_normal) ||
        !TryNormalize(Cross(v_dir, extrude_dir), linear_tolerance, u_side_normal) ||
        !TryNormalize(Cross(extrude_dir, u_dir), linear_tolerance, v_side_normal)) {
        return false;
    }

    const Vec3 origin = ToVec(spec.profile.origin);
    const Vec3 p1 = Add(origin, Scale(u_dir, spec.profile.size_u));
    const Vec3 p3 = Add(origin, Scale(v_dir, spec.profile.size_v));
    const Vec3 end_point = Add(origin, Scale(extrude_dir, spec.distance));

    switch (role) {
        case FaceRole::StartCap:
            out = ExpectedPlane{cap_normal, origin};
            return true;
        case FaceRole::EndCap:
            out = ExpectedPlane{cap_normal, end_point};
            return true;
        case FaceRole::UMinSide:
            out = ExpectedPlane{u_side_normal, origin};
            return true;
        case FaceRole::UMaxSide:
            out = ExpectedPlane{u_side_normal, p1};
            return true;
        case FaceRole::VMinSide:
            out = ExpectedPlane{v_side_normal, origin};
            return true;
        case FaceRole::VMaxSide:
            out = ExpectedPlane{v_side_normal, p3};
            return true;
    }
    return false;
}

[[nodiscard]] bool PlaneMatches(const ExpectedPlane& expected, const bim::geometry_api::FaceObservation& face,
                                 const GeometryTolerance& tolerance) noexcept {
    if (face.surface_kind != bim::geometry_api::SurfaceKind::Planar) {
        return false;
    }
    const Vec3 face_normal = ToVec(face.normal);
    const double face_normal_len = Length(face_normal);
    if (!(face_normal_len > 0.0)) {
        return false;
    }
    // Sign-agnostic parallel test: an observed face's outward normal may
    // legitimately point either the same way as, or opposite to, the
    // expected axis direction chosen above (the choice of which way is
    // "outward" for a given role was made for readability, not because the
    // sign carries any identity meaning).
    const double cos_angle = Dot(expected.normal, face_normal) / face_normal_len;
    const double angular_cos_tolerance = std::cos(tolerance.angular_radians);
    if (std::fabs(cos_angle) < angular_cos_tolerance) {
        return false;
    }
    // Offset check: this is what disambiguates a pair of parallel planes
    // (e.g. the StartCap and EndCap planes, both normal-aligned with the
    // extrusion direction, but at different offsets along it).
    const double offset = std::fabs(Dot(Sub(ToVec(face.point_on_surface), expected.point), expected.normal));
    return offset <= tolerance.linear;
}

} // namespace

FaceResolutionResult ResolveFaceReference(const PersistentFaceReference& reference,
                                           const FeatureFrame& current_owner_frame,
                                           const bim::geometry_api::FaceObservationResult& observed_faces,
                                           const GeometryTolerance& tolerance) {
    FaceResolutionResult result;

    if (!IsValidFaceRole(reference.role)) {
        result.status = FaceResolutionStatus::InvalidReference;
        return result;
    }

    if (!(reference.owner == current_owner_frame.owner)) {
        result.status = FaceResolutionStatus::InvalidOwner;
        return result;
    }

    if (!observed_faces.ok()) {
        // An upstream observation failure is a reason the reference cannot
        // be resolved right now, not evidence the face is absent - fail
        // closed as InvalidReference rather than reporting a false
        // Missing.
        result.status = FaceResolutionStatus::InvalidReference;
        return result;
    }

    ExpectedPlane expected;
    if (!DeriveExpectedPlane(current_owner_frame.spec, reference.role, tolerance.linear, expected)) {
        result.status = FaceResolutionStatus::InvalidReference;
        return result;
    }

    std::size_t match_count = 0;
    std::size_t match_index = 0;
    for (std::size_t i = 0; i < observed_faces.faces.size(); ++i) {
        if (PlaneMatches(expected, observed_faces.faces[i], tolerance)) {
            ++match_count;
            match_index = i;
        }
    }

    if (match_count == 0) {
        result.status = FaceResolutionStatus::Missing;
    } else if (match_count == 1) {
        result.status = FaceResolutionStatus::Resolved;
        result.resolved_face_index = match_index;
    } else {
        result.status = FaceResolutionStatus::Ambiguous;
    }
    return result;
}

} // namespace bim::model
