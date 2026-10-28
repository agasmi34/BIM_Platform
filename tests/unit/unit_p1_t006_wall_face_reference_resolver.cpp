// P1-T006 Desktop VS1 End-to-End Acceptance - unit proof of the production
// WallFaceReference resolver (Architecture Gate BIM-AG-P1-T006 sections 4-8;
// Implementation Brief P1-T006-IB section 7).
//
// Links only bim::model (and, transitively, the neutral bim::geometry_api
// contract): no kernel library is linked, so every FaceObservation here is
// hand-constructed analytic data. The real-kernel proof lives in
// integration_p1_t006_vs1_e2e. Only the production identity types
// (ElementId, WallFaceRole, WallFaceReference) are used; none of the Phase-0
// spike identity types is included or referenced.

#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/wall_face_reference.hpp"
#include "bim/model/wall_face_reference_resolver.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <vector>

using bim::geometry_api::FaceObservation;
using bim::geometry_api::FaceObservationResult;
using bim::geometry_api::GeometryError;
using bim::geometry_api::GeometryErrorCode;
using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::Point3;
using bim::geometry_api::SurfaceKind;
using bim::geometry_api::Vector3;
using bim::model::ElementId;
using bim::model::ResolveWallFaceReference;
using bim::model::WallFaceFrame;
using bim::model::WallFaceReference;
using bim::model::WallFaceResolutionResult;
using bim::model::WallFaceResolutionStatus;
using bim::model::WallFaceRole;

// --- contract shape ------------------------------------------------------------
static_assert(noexcept(ResolveWallFaceReference(std::declval<const WallFaceReference&>(),
                                                std::declval<const WallFaceFrame&>(),
                                                std::declval<const FaceObservationResult&>(),
                                                std::declval<const GeometryTolerance&>())));
static_assert(std::is_same_v<std::underlying_type_t<WallFaceResolutionStatus>, std::uint8_t>);
static_assert(static_cast<int>(WallFaceResolutionStatus::Resolved) == 0);
static_assert(static_cast<int>(WallFaceResolutionStatus::Missing) == 1);
static_assert(static_cast<int>(WallFaceResolutionStatus::Ambiguous) == 2);
static_assert(static_cast<int>(WallFaceResolutionStatus::InvalidReference) == 3);
static_assert(static_cast<int>(WallFaceResolutionStatus::InvalidOwner) == 4);

namespace {

constexpr GeometryTolerance kTol{1.0e-6, 1.0e-6};

constexpr std::array<WallFaceRole, 6> kAllRoles{WallFaceRole::Bottom, WallFaceRole::Top,
                                                WallFaceRole::Start,  WallFaceRole::End,
                                                WallFaceRole::SideA,  WallFaceRole::SideB};

[[nodiscard]] ElementId Id(std::uint8_t first) {
    ElementId id;
    id.bytes[0] = first;
    id.bytes[15] = 1;
    return id;
}

struct Vec {
    double x;
    double y;
    double z;
};

[[nodiscard]] Vec Add(Vec a, Vec b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
[[nodiscard]] Vec Mul(Vec a, double s) {
    return {a.x * s, a.y * s, a.z * s};
}
[[nodiscard]] Vec Neg(Vec a) {
    return Mul(a, -1.0);
}
[[nodiscard]] Vec Unit(Vec a) {
    const double l = std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    return {a.x / l, a.y / l, a.z / l};
}

// A wall along `angle` (radians) in plan, built exactly the way the document
// derives it: U along the axis, W = +Z, V = W x U, profile origin shifted by
// -V * thickness / 2.
[[nodiscard]] LinearExtrusionSpec WallSpec(double length, double thickness, double height,
                                           double angle = 0.0, double base_z = 0.0,
                                           Vec start = {0.0, 0.0, 0.0}) {
    const Vec u{std::cos(angle), std::sin(angle), 0.0};
    const Vec v{-u.y, u.x, 0.0};
    const Vec origin = Add(Add(start, Mul(v, -thickness / 2.0)), Vec{0.0, 0.0, base_z});
    LinearExtrusionSpec spec;
    spec.profile.origin = Point3{origin.x, origin.y, origin.z};
    spec.profile.u_axis = Vector3{u.x, u.y, u.z};
    spec.profile.v_axis = Vector3{v.x, v.y, v.z};
    spec.profile.size_u = length;
    spec.profile.size_v = thickness;
    spec.direction = Vector3{0.0, 0.0, 1.0};
    spec.distance = height;
    return spec;
}

[[nodiscard]] FaceObservation PlanarFace(Vec point, Vec outward_normal) {
    FaceObservation f;
    f.point_on_surface = Point3{point.x, point.y, point.z};
    f.normal = Vector3{outward_normal.x, outward_normal.y, outward_normal.z};
    f.surface_kind = SurfaceKind::Planar;
    f.area = 1.0;
    return f;
}

// Analytic neutral observation of the prism described by `spec` (assumes a
// perpendicular extrusion). Order: Bottom, Top, Start, End, SideA, SideB.
[[nodiscard]] FaceObservationResult Observe(const LinearExtrusionSpec& spec) {
    const Vec o{spec.profile.origin.x, spec.profile.origin.y, spec.profile.origin.z};
    const Vec u = Unit({spec.profile.u_axis.x, spec.profile.u_axis.y, spec.profile.u_axis.z});
    const Vec v = Unit({spec.profile.v_axis.x, spec.profile.v_axis.y, spec.profile.v_axis.z});
    const Vec w = Unit({spec.direction.x, spec.direction.y, spec.direction.z});
    const double su = spec.profile.size_u;
    const double sv = spec.profile.size_v;
    const double d = spec.distance;
    const Vec centre = Add(o, Add(Mul(u, su / 2.0), Mul(v, sv / 2.0)));
    FaceObservationResult r;
    r.faces.push_back(PlanarFace(centre, Neg(w)));                                         // Bottom
    r.faces.push_back(PlanarFace(Add(centre, Mul(w, d)), w));                              // Top
    r.faces.push_back(PlanarFace(Add(Add(o, Mul(v, sv / 2.0)), Mul(w, d / 2.0)), Neg(u))); // Start
    r.faces.push_back(
        PlanarFace(Add(Add(o, Add(Mul(u, su), Mul(v, sv / 2.0))), Mul(w, d / 2.0)), u));   // End
    r.faces.push_back(PlanarFace(Add(Add(o, Mul(u, su / 2.0)), Mul(w, d / 2.0)), Neg(v))); // SideA
    r.faces.push_back(
        PlanarFace(Add(Add(o, Add(Mul(u, su / 2.0), Mul(v, sv))), Mul(w, d / 2.0)), v)); // SideB
    return r;
}

constexpr std::size_t IndexOf(WallFaceRole role) {
    return static_cast<std::size_t>(role);
}

[[nodiscard]] WallFaceResolutionResult Resolve(WallFaceRole role, const ElementId& wall,
                                               const LinearExtrusionSpec& spec,
                                               const FaceObservationResult& observed,
                                               const GeometryTolerance& tol = kTol) {
    return ResolveWallFaceReference(WallFaceReference{wall, role}, WallFaceFrame{wall, spec},
                                    observed, tol);
}

// The observation at `index` must be the face the role semantically names.
void RequireIsSemanticFace(WallFaceRole role, const FaceObservationResult& observed,
                           std::size_t index, const FaceObservationResult& reference_order) {
    REQUIRE(index < observed.faces.size());
    const FaceObservation& got = observed.faces[index];
    const FaceObservation& want = reference_order.faces[IndexOf(role)];
    REQUIRE(got.point_on_surface.x == want.point_on_surface.x);
    REQUIRE(got.point_on_surface.y == want.point_on_surface.y);
    REQUIRE(got.point_on_surface.z == want.point_on_surface.z);
    REQUIRE(got.normal.x == want.normal.x);
    REQUIRE(got.normal.y == want.normal.y);
    REQUIRE(got.normal.z == want.normal.z);
}

void RequireStatus(const WallFaceResolutionResult& r, WallFaceResolutionStatus status) {
    REQUIRE(r.status == status);
    REQUIRE(r.ok() == (status == WallFaceResolutionStatus::Resolved));
}

} // namespace

TEST_CASE("the six production roles resolve to the six semantic planes", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed = Observe(spec);
    REQUIRE(observed.ok());
    REQUIRE(observed.faces.size() == 6);

    std::array<bool, 6> index_seen{};
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceResolutionResult r = Resolve(role, wall, spec, observed);
        RequireStatus(r, WallFaceResolutionStatus::Resolved);
        REQUIRE(r.resolved_face_index == IndexOf(role)); // hand-built order: Bottom..SideB
        REQUIRE_FALSE(index_seen[r.resolved_face_index]);
        index_seen[r.resolved_face_index] = true;
    }
    REQUIRE(std::all_of(index_seen.begin(), index_seen.end(), [](bool b) { return b; }));
}

TEST_CASE("the semantic planes are orientation-sign agnostic", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    FaceObservationResult observed = Observe(spec);
    for (FaceObservation& f : observed.faces) {
        f.normal = Vector3{-f.normal.x, -f.normal.y, -f.normal.z};
    }
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceResolutionResult r = Resolve(role, wall, spec, observed);
        RequireStatus(r, WallFaceResolutionStatus::Resolved);
        REQUIRE(r.resolved_face_index == IndexOf(role));
    }
    // A non-unit observed normal is normalized before it is compared.
    FaceObservationResult scaled = Observe(spec);
    for (FaceObservation& f : scaled.faces) {
        f.normal = Vector3{f.normal.x * 7.0, f.normal.y * 7.0, f.normal.z * 7.0};
    }
    for (const WallFaceRole role : kAllRoles) {
        RequireStatus(Resolve(role, wall, spec, scaled), WallFaceResolutionStatus::Resolved);
    }
}

TEST_CASE("rotated and elevated walls resolve all six roles", "[p1_t006][resolver]") {
    const ElementId wall = Id(2);
    for (const double angle : {0.3, 1.0, std::numbers::pi / 2.0, 2.5, -0.7, std::numbers::pi}) {
        const LinearExtrusionSpec spec = WallSpec(5.0, 0.3, 2.7, angle, 1.25, Vec{3.0, -2.0, 0.0});
        const FaceObservationResult observed = Observe(spec);
        for (const WallFaceRole role : kAllRoles) {
            const WallFaceResolutionResult r = Resolve(role, wall, spec, observed);
            RequireStatus(r, WallFaceResolutionStatus::Resolved);
            REQUIRE(r.resolved_face_index == IndexOf(role));
        }
    }
}

TEST_CASE("every observation order gives the same semantic result", "[p1_t006][resolver]") {
    const ElementId wall = Id(3);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0, 0.4);
    const FaceObservationResult canonical = Observe(spec);

    std::array<std::size_t, 6> order{0, 1, 2, 3, 4, 5};
    std::size_t permutations = 0;
    do {
        FaceObservationResult shuffled;
        for (const std::size_t i : order) {
            shuffled.faces.push_back(canonical.faces[i]);
        }
        for (const WallFaceRole role : kAllRoles) {
            const WallFaceResolutionResult r = Resolve(role, wall, spec, shuffled);
            RequireStatus(r, WallFaceResolutionStatus::Resolved);
            RequireIsSemanticFace(role, shuffled, r.resolved_face_index, canonical);
        }
        ++permutations;
    } while (std::next_permutation(order.begin(), order.end()));
    REQUIRE(permutations == 720);
}

TEST_CASE("resolved_face_index is transient output, never identity", "[p1_t006][resolver]") {
    // The durable reference holds exactly a wall id and a role.
    static_assert(std::is_same_v<decltype(WallFaceReference::wall_id), ElementId>);
    static_assert(std::is_same_v<decltype(WallFaceReference::role), WallFaceRole>);
    WallFaceReference probe{Id(4), WallFaceRole::Top};
    const auto& [wall_member, role_member] = probe; // exactly two members, or this fails to compile
    REQUIRE(wall_member == Id(4));
    REQUIRE(role_member == WallFaceRole::Top);

    const ElementId wall = Id(4);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult canonical = Observe(spec);
    FaceObservationResult reversed;
    reversed.faces.assign(canonical.faces.rbegin(), canonical.faces.rend());

    const WallFaceReference reference{wall, WallFaceRole::Top};
    const WallFaceResolutionResult a =
        ResolveWallFaceReference(reference, WallFaceFrame{wall, spec}, canonical, kTol);
    const WallFaceResolutionResult b =
        ResolveWallFaceReference(reference, WallFaceFrame{wall, spec}, reversed, kTol);
    REQUIRE(a.ok());
    REQUIRE(b.ok());
    REQUIRE(a.resolved_face_index != b.resolved_face_index); // position is not identity
    REQUIRE(a.status == b.status);                           // semantic result is identical
    REQUIRE(reference == WallFaceReference{wall, WallFaceRole::Top}); // the reference never changed

    // A failed resolution carries no meaningful index.
    const WallFaceResolutionResult failed = ResolveWallFaceReference(
        WallFaceReference{Id(5), WallFaceRole::Top}, WallFaceFrame{wall, spec}, canonical, kTol);
    REQUIRE(failed.status == WallFaceResolutionStatus::InvalidOwner);
    REQUIRE_FALSE(failed.ok());
    REQUIRE(failed.resolved_face_index == 0);
}

TEST_CASE("a mismatched wall id is InvalidOwner", "[p1_t006][resolver]") {
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed = Observe(spec);
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceResolutionResult r = ResolveWallFaceReference(
            WallFaceReference{Id(1), role}, WallFaceFrame{Id(2), spec}, observed, kTol);
        RequireStatus(r, WallFaceResolutionStatus::InvalidOwner);
        REQUIRE(r.resolved_face_index == 0);
    }
    // Ids that differ in a single (non-first) byte are still different owners.
    ElementId other = Id(1);
    other.bytes[7] = 9;
    RequireStatus(ResolveWallFaceReference(WallFaceReference{Id(1), WallFaceRole::Top},
                                           WallFaceFrame{other, spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidOwner);
}

TEST_CASE("a structurally invalid reference is InvalidReference", "[p1_t006][resolver]") {
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed = Observe(spec);

    // All-zero wall id (the single invalid identity), even when the frame carries
    // the same all-zero id.
    RequireStatus(ResolveWallFaceReference(WallFaceReference{ElementId{}, WallFaceRole::Top},
                                           WallFaceFrame{ElementId{}, spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidReference);
    RequireStatus(ResolveWallFaceReference(WallFaceReference{ElementId{}, WallFaceRole::Top},
                                           WallFaceFrame{Id(1), spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidReference);

    // An out-of-range role value is not one of the six roles.
    const auto bad_role = static_cast<WallFaceRole>(6);
    RequireStatus(ResolveWallFaceReference(WallFaceReference{Id(1), bad_role},
                                           WallFaceFrame{Id(1), spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidReference);
    const auto bad_role_255 = static_cast<WallFaceRole>(255);
    RequireStatus(ResolveWallFaceReference(WallFaceReference{Id(1), bad_role_255},
                                           WallFaceFrame{Id(1), spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidReference);

    // Structural validity is established before ownership: an invalid reference
    // for a different wall is InvalidReference, not InvalidOwner.
    RequireStatus(ResolveWallFaceReference(WallFaceReference{Id(1), bad_role},
                                           WallFaceFrame{Id(2), spec}, observed, kTol),
                  WallFaceResolutionStatus::InvalidReference);
}

TEST_CASE("a failed observation is InvalidReference, not Missing", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    for (const GeometryErrorCode code :
         {GeometryErrorCode::InvalidInput, GeometryErrorCode::DegenerateGeometry,
          GeometryErrorCode::NoIntersection, GeometryErrorCode::KernelOperationFailed,
          GeometryErrorCode::InvalidResult, GeometryErrorCode::UnsupportedOperation}) {
        FaceObservationResult failed = Observe(spec); // faces present, but upstream failed
        failed.error = GeometryError{code, "upstream failure"};
        REQUIRE_FALSE(failed.ok());
        for (const WallFaceRole role : kAllRoles) {
            RequireStatus(Resolve(role, wall, spec, failed),
                          WallFaceResolutionStatus::InvalidReference);
        }
        FaceObservationResult failed_empty;
        failed_empty.error = GeometryError{code, ""};
        RequireStatus(Resolve(WallFaceRole::Top, wall, spec, failed_empty),
                      WallFaceResolutionStatus::InvalidReference);
    }
}

TEST_CASE("an unusable tolerance is InvalidReference", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed = Observe(spec);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const std::vector<GeometryTolerance> bad{
        {nan, 1.0e-6},     {1.0e-6, nan},
        {inf, 1.0e-6},     {1.0e-6, inf},
        {-inf, 1.0e-6},    {0.0, 1.0e-6},
        {-1.0e-6, 1.0e-6}, {1.0e-6, 0.0},
        {1.0e-6, -1.0e-6}, {1.0e-6, std::numbers::pi / 2.0},
        {1.0e-6, 4.0},     {nan, nan},
    };
    for (const GeometryTolerance& tol : bad) {
        for (const WallFaceRole role : kAllRoles) {
            RequireStatus(Resolve(role, wall, spec, observed, tol),
                          WallFaceResolutionStatus::InvalidReference);
        }
    }
}

TEST_CASE("an extrusion frame that cannot define six planes is InvalidReference",
          "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const LinearExtrusionSpec good = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed = Observe(good);

    std::vector<LinearExtrusionSpec> bad;
    auto add = [&](auto&& mutate) {
        LinearExtrusionSpec s = good;
        mutate(s);
        bad.push_back(s);
    };
    add([](LinearExtrusionSpec& s) { s.profile.u_axis = Vector3{0.0, 0.0, 0.0}; });
    add([](LinearExtrusionSpec& s) { s.profile.v_axis = Vector3{0.0, 0.0, 0.0}; });
    add([](LinearExtrusionSpec& s) { s.direction = Vector3{0.0, 0.0, 0.0}; });
    add([](LinearExtrusionSpec& s) { s.profile.u_axis = Vector3{5.0e-7, 0.0, 0.0}; }); // <= linear
    add([&](LinearExtrusionSpec& s) { s.profile.u_axis.x = nan; });
    add([&](LinearExtrusionSpec& s) { s.profile.v_axis.y = inf; });
    add([&](LinearExtrusionSpec& s) { s.direction.z = -inf; });
    add([&](LinearExtrusionSpec& s) { s.profile.origin.x = nan; });
    add([&](LinearExtrusionSpec& s) { s.profile.origin.z = inf; });
    add([](LinearExtrusionSpec& s) { s.profile.size_u = 0.0; });
    add([](LinearExtrusionSpec& s) { s.profile.size_v = -0.2; });
    add([&](LinearExtrusionSpec& s) { s.profile.size_u = nan; });
    add([&](LinearExtrusionSpec& s) { s.profile.size_v = inf; });
    add([](LinearExtrusionSpec& s) { s.distance = 0.0; });
    add([](LinearExtrusionSpec& s) { s.distance = -3.0; });
    add([&](LinearExtrusionSpec& s) { s.distance = nan; });
    // U parallel / anti-parallel to V.
    add([](LinearExtrusionSpec& s) { s.profile.v_axis = Vector3{1.0, 0.0, 0.0}; });
    add([](LinearExtrusionSpec& s) { s.profile.v_axis = Vector3{-1.0, 0.0, 0.0}; });
    // U and V skew beyond the angular tolerance (never silently orthogonalized).
    add([](LinearExtrusionSpec& s) { s.profile.v_axis = Vector3{0.01, 1.0, 0.0}; });
    // Extrusion direction lying in the profile plane.
    add([](LinearExtrusionSpec& s) { s.direction = Vector3{1.0, 0.0, 0.0}; });
    add([](LinearExtrusionSpec& s) { s.direction = Vector3{0.3, 0.9, 0.0}; });
    // Opposite planes closer than the linear tolerance (indistinguishable).
    add([](LinearExtrusionSpec& s) { s.distance = 5.0e-7; });
    add([](LinearExtrusionSpec& s) { s.profile.size_u = 5.0e-7; });
    add([](LinearExtrusionSpec& s) { s.profile.size_v = 5.0e-7; });
    // Overflow while deriving a plane point.
    add([](LinearExtrusionSpec& s) {
        s.profile.origin.z = std::numeric_limits<double>::max();
        s.distance = std::numeric_limits<double>::max();
    });

    REQUIRE(bad.size() == 25);
    for (std::size_t i = 0; i < bad.size(); ++i) {
        CAPTURE(i);
        for (const WallFaceRole role : kAllRoles) {
            RequireStatus(Resolve(role, wall, bad[i], observed),
                          WallFaceResolutionStatus::InvalidReference);
        }
    }
}

TEST_CASE("an absent semantic face is Missing", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult canonical = Observe(spec);

    for (const WallFaceRole removed : kAllRoles) {
        FaceObservationResult partial = canonical;
        partial.faces.erase(partial.faces.begin() + static_cast<std::ptrdiff_t>(IndexOf(removed)));
        for (const WallFaceRole role : kAllRoles) {
            const WallFaceResolutionResult r = Resolve(role, wall, spec, partial);
            if (role == removed) {
                RequireStatus(r, WallFaceResolutionStatus::Missing);
            } else {
                RequireStatus(r, WallFaceResolutionStatus::Resolved);
            }
        }
    }

    // An empty (but successful) observation: every role is Missing.
    FaceObservationResult empty;
    for (const WallFaceRole role : kAllRoles) {
        RequireStatus(Resolve(role, wall, spec, empty), WallFaceResolutionStatus::Missing);
    }

    // A non-planar face never matches, whatever its (meaningless) values say.
    FaceObservationResult curved = canonical;
    curved.faces[IndexOf(WallFaceRole::Top)].surface_kind = SurfaceKind::NonPlanar;
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, curved),
                  WallFaceResolutionStatus::Missing);

    // A zero or non-finite observed normal never matches.
    FaceObservationResult zero_normal = canonical;
    zero_normal.faces[IndexOf(WallFaceRole::Top)].normal = Vector3{0.0, 0.0, 0.0};
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, zero_normal),
                  WallFaceResolutionStatus::Missing);
    FaceObservationResult nan_point = canonical;
    nan_point.faces[IndexOf(WallFaceRole::Top)].point_on_surface.z =
        std::numeric_limits<double>::quiet_NaN();
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, nan_point),
                  WallFaceResolutionStatus::Missing);
}

TEST_CASE("plane matching honours the linear and angular tolerances", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const std::size_t top = IndexOf(WallFaceRole::Top);

    FaceObservationResult inside = Observe(spec);
    inside.faces[top].point_on_surface.z += 0.5e-6; // < linear
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, inside),
                  WallFaceResolutionStatus::Resolved);

    FaceObservationResult outside = Observe(spec);
    outside.faces[top].point_on_surface.z += 2.0e-6; // > linear
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, outside),
                  WallFaceResolutionStatus::Missing);

    // Tilting the Top normal by less / more than the angular tolerance.
    FaceObservationResult tilted_ok = Observe(spec);
    tilted_ok.faces[top].normal = Vector3{0.5e-6, 0.0, 1.0};
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, tilted_ok),
                  WallFaceResolutionStatus::Resolved);
    FaceObservationResult tilted_bad = Observe(spec);
    tilted_bad.faces[top].normal = Vector3{2.0e-6, 0.0, 1.0};
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, tilted_bad),
                  WallFaceResolutionStatus::Missing);

    // A plane offset between two roles' planes is not any role's plane.
    FaceObservationResult floating = Observe(spec);
    floating.faces[top].point_on_surface.z = 1.5;
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, floating),
                  WallFaceResolutionStatus::Missing);
}

TEST_CASE("duplicate matching planes are Ambiguous", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult canonical = Observe(spec);

    for (const WallFaceRole doubled : kAllRoles) {
        FaceObservationResult split = canonical;
        FaceObservation copy = canonical.faces[IndexOf(doubled)];
        copy.normal =
            Vector3{-copy.normal.x, -copy.normal.y, -copy.normal.z}; // opposite sign, same plane
        copy.point_on_surface.x +=
            (doubled == WallFaceRole::Start || doubled == WallFaceRole::End) ? 0.0 : 0.3;
        split.faces.push_back(copy);
        for (const WallFaceRole role : kAllRoles) {
            const WallFaceResolutionResult r = Resolve(role, wall, spec, split);
            if (role == doubled) {
                RequireStatus(r, WallFaceResolutionStatus::Ambiguous);
                REQUIRE(r.resolved_face_index == 0);
            } else {
                RequireStatus(r, WallFaceResolutionStatus::Resolved);
            }
        }
    }
}

TEST_CASE("geometry change is followed by semantic re-resolution", "[p1_t006][resolver]") {
    const ElementId wall = Id(7);
    const LinearExtrusionSpec before = WallSpec(4.0, 0.2, 3.0);
    const FaceObservationResult observed_before = Observe(before);

    // The wall is made longer, thicker, shorter and raised.
    const LinearExtrusionSpec after = WallSpec(6.5, 0.4, 2.5, 0.0, 1.0);
    const FaceObservationResult observed_after = Observe(after);

    for (const WallFaceRole role : kAllRoles) {
        // The same durable reference resolves against the regenerated geometry.
        const WallFaceReference reference{wall, role};
        const WallFaceResolutionResult now =
            ResolveWallFaceReference(reference, WallFaceFrame{wall, after}, observed_after, kTol);
        RequireStatus(now, WallFaceResolutionStatus::Resolved);
        RequireIsSemanticFace(role, observed_after, now.resolved_face_index, observed_after);
    }

    // Stale observations are judged against the NEW frame, never silently matched.
    for (const WallFaceRole moved : {WallFaceRole::Top, WallFaceRole::End, WallFaceRole::SideB,
                                     WallFaceRole::SideA, WallFaceRole::Bottom}) {
        RequireStatus(ResolveWallFaceReference(WallFaceReference{wall, moved},
                                               WallFaceFrame{wall, after}, observed_before, kTol),
                      WallFaceResolutionStatus::Missing);
    }
    // The Start plane (minimum-U, x = 0) did not move, so it still resolves.
    RequireStatus(ResolveWallFaceReference(WallFaceReference{wall, WallFaceRole::Start},
                                           WallFaceFrame{wall, after}, observed_before, kTol),
                  WallFaceResolutionStatus::Resolved);

    // Regeneration in a different enumeration order changes the index only.
    FaceObservationResult reordered;
    reordered.faces.assign(observed_after.faces.rbegin(), observed_after.faces.rend());
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceReference reference{wall, role};
        const WallFaceResolutionResult a =
            ResolveWallFaceReference(reference, WallFaceFrame{wall, after}, observed_after, kTol);
        const WallFaceResolutionResult b =
            ResolveWallFaceReference(reference, WallFaceFrame{wall, after}, reordered, kTol);
        REQUIRE(a.status == b.status);
        RequireIsSemanticFace(role, reordered, b.resolved_face_index, observed_after);
    }
}

TEST_CASE("an oblique but valid extrusion still resolves its six planes", "[p1_t006][resolver]") {
    const ElementId wall = Id(1);
    LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    spec.direction = Vector3{0.3, 0.0, 1.0}; // sheared prism, not perpendicular to the profile

    // Build the sheared prism's face planes by hand: caps stay parallel to the
    // profile plane, the U-side planes are spanned by V and the direction.
    const Vec o{spec.profile.origin.x, spec.profile.origin.y, spec.profile.origin.z};
    const Vec d = Mul(Unit({0.3, 0.0, 1.0}), spec.distance);
    FaceObservationResult observed;
    observed.faces.push_back(PlanarFace(Add(o, Vec{2.0, 0.1, 0.0}), {0.0, 0.0, -1.0}));
    observed.faces.push_back(PlanarFace(Add(Add(o, Vec{2.0, 0.1, 0.0}), d), {0.0, 0.0, 1.0}));
    const Vec n_start = Unit({1.0, 0.0, -0.3 / 1.0});
    observed.faces.push_back(
        PlanarFace(Add(o, Add(Vec{0.0, 0.1, 0.0}, Mul(d, 0.5))), Neg(n_start)));
    observed.faces.push_back(PlanarFace(Add(o, Add(Vec{4.0, 0.1, 0.0}, Mul(d, 0.5))), n_start));
    observed.faces.push_back(
        PlanarFace(Add(o, Add(Vec{2.0, 0.0, 0.0}, Mul(d, 0.5))), {0.0, -1.0, 0.0}));
    observed.faces.push_back(
        PlanarFace(Add(o, Add(Vec{2.0, 0.2, 0.0}, Mul(d, 0.5))), {0.0, 1.0, 0.0}));
    for (const WallFaceRole role : kAllRoles) {
        RequireStatus(Resolve(role, wall, spec, observed), WallFaceResolutionStatus::Resolved);
    }
}

TEST_CASE("the resolver reports through status only", "[p1_t006][resolver]") {
    // Fail-closed paths do not throw (the function is noexcept; this exercises
    // the large-input path too).
    const ElementId wall = Id(1);
    const LinearExtrusionSpec spec = WallSpec(4.0, 0.2, 3.0);
    FaceObservationResult many = Observe(spec);
    for (int i = 0; i < 2000; ++i) {
        many.faces.push_back(PlanarFace(Vec{double(i), 0.0, 0.0}, Vec{1.0, 0.0, 0.0}));
    }
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceResolutionResult r = Resolve(role, wall, spec, many);
        REQUIRE((r.status == WallFaceResolutionStatus::Resolved ||
                 r.status == WallFaceResolutionStatus::Ambiguous));
    }
    // The 2000 extra +X faces include one exactly on the End plane (x = 4) and one on
    // the Start plane (x = 0): those two roles become Ambiguous, the rest stay Resolved.
    RequireStatus(Resolve(WallFaceRole::Start, wall, spec, many),
                  WallFaceResolutionStatus::Ambiguous);
    RequireStatus(Resolve(WallFaceRole::End, wall, spec, many),
                  WallFaceResolutionStatus::Ambiguous);
    RequireStatus(Resolve(WallFaceRole::Top, wall, spec, many), WallFaceResolutionStatus::Resolved);
}
