// P0-T008 Topological Reference Spike - model-level, kernel-free resolver
// tests (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0). This
// translation unit links only bim::model and bim::geometry_api - see
// tests/unit/CMakeLists.txt - and constructs every FaceObservation by hand;
// it never calls MakeLinearExtrusion or ObserveFaces. That is deliberate,
// not a shortcut: bim::model::ResolveFaceReference's only real input
// boundary is a bim::geometry_api::FaceObservationResult value, so testing
// directly at that boundary is a complete, dispositive proof of the
// resolver's own logic (Missing/Ambiguous/InvalidReference/InvalidOwner,
// and the mandatory enumeration-order-independence property), independent
// of whatever a real kernel solid happens to produce. The real-kernel
// proof corpus (A, B, C, D, E, G) lives in
// tests/integration/integration_p0_t008_face_reference_spike.cpp; this file
// covers corpus case F (a split face must resolve Ambiguous) and case H
// (an invalid owner or an invalid reference), plus the model-level,
// kernel-free enumeration-order-independence test the Brief requires.
//
// B13-C1 addition: an oblique-extrusion regression case (see
// ObliqueSixFaces() below) - MakeLinearExtrusion never requires the
// extrusion direction to be perpendicular to the profile plane, so an
// oblique extrusion is valid geometry that DeriveExpectedPlane
// (src/model/src/face_reference_resolver.cpp) must still resolve
// correctly.

#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/face_reference_resolver.hpp"
#include "bim/model/persistent_face_reference.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <vector>

using namespace bim::geometry_api;
using bim::model::FaceResolutionResult;
using bim::model::FaceResolutionStatus;
using bim::model::FaceRole;
using bim::model::FeatureFrame;
using bim::model::FeatureOwnerId;
using bim::model::IsValidFaceRole;
using bim::model::PersistentFaceReference;
using bim::model::ResolveFaceReference;

namespace {

constexpr FeatureOwnerId kOwner{7};
constexpr GeometryTolerance kTolerance{1.0e-4, 1.0e-6};

// A hand-built stand-in for what ObserveFaces() would report for a P01-like
// axis-aligned box (origin (0,0,0); u=(1,0,0) size_u=6; v=(0,0,1) size_v=3;
// direction=(0,1,0) distance=0.2) - the same shape the real-kernel corpus
// uses, reproduced here as plain data so this file needs no kernel adapter.
[[nodiscard]] LinearExtrusionSpec CanonicalSpec() {
    RectangleProfile3 profile;
    profile.origin = Point3{0.0, 0.0, 0.0};
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = 6.0;
    profile.size_v = 3.0;

    LinearExtrusionSpec spec;
    spec.profile = profile;
    spec.direction = Vector3{0.0, 1.0, 0.0};
    spec.distance = 0.2;
    return spec;
}

[[nodiscard]] FaceObservation MakeFace(const Point3& point, const Vector3& normal, double area = 1.0) {
    FaceObservation face;
    face.point_on_surface = point;
    face.normal = normal;
    face.surface_kind = SurfaceKind::Planar;
    face.area = area;
    return face;
}

// The six faces of CanonicalSpec()'s box, in a fixed "canonical" order:
// StartCap, EndCap, UMinSide, UMaxSide, VMinSide, VMaxSide.
[[nodiscard]] std::vector<FaceObservation> CanonicalSixFaces() {
    return {
        MakeFace(Point3{3.0, 0.0, 1.5}, Vector3{0.0, 1.0, 0.0}),  // StartCap (y=0)
        MakeFace(Point3{3.0, 0.2, 1.5}, Vector3{0.0, 1.0, 0.0}),  // EndCap (y=0.2)
        MakeFace(Point3{0.0, 0.1, 1.5}, Vector3{1.0, 0.0, 0.0}),  // UMinSide (x=0)
        MakeFace(Point3{6.0, 0.1, 1.5}, Vector3{1.0, 0.0, 0.0}),  // UMaxSide (x=6)
        MakeFace(Point3{3.0, 0.1, 0.0}, Vector3{0.0, 0.0, 1.0}),  // VMinSide (z=0)
        MakeFace(Point3{3.0, 0.1, 3.0}, Vector3{0.0, 0.0, 1.0}),  // VMaxSide (z=3)
    };
}

constexpr std::array<FaceRole, 6> kAllRoles{FaceRole::StartCap,  FaceRole::EndCap,  FaceRole::UMinSide,
                                            FaceRole::UMaxSide,  FaceRole::VMinSide, FaceRole::VMaxSide};

// --- B13-C1 oblique-extrusion regression fixture -----------------------------
// Same U/V/size_u/size_v/distance as CanonicalSpec(), but with an extrusion
// direction that is deliberately NOT perpendicular to the profile plane
// (U=X, V=Z; D is neither parallel to cross(U,V)=(0,-1,0) nor coplanar with
// the profile - it has a non-zero component along cross(U,V), so it
// produces a genuine, non-degenerate oblique prism). D is supplied here
// already unit-length so this fixture's own arithmetic never depends on
// whether MakeLinearExtrusion normalizes `direction` internally before
// scaling by `distance` - the real-kernel counterpart of this fixture
// (tests/integration/integration_p0_t008_face_reference_spike.cpp) makes
// the same deliberate choice for the same reason.
[[nodiscard]] LinearExtrusionSpec ObliqueSpec() {
    RectangleProfile3 profile;
    profile.origin = Point3{0.0, 0.0, 0.0};
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = 6.0;
    profile.size_v = 3.0;

    LinearExtrusionSpec spec;
    spec.profile = profile;
    // normalize((0.25, 1.0, 0.35)), precomputed.
    spec.direction = Vector3{0.229657614, 0.918630457, 0.321520660};
    spec.distance = 0.2;
    return spec;
}

// The six faces of ObliqueSpec()'s prism, computed from the CORRECTED
// (B13-C1) plane derivation: cap normal = normalize(cross(U,V)); U-side
// normal = normalize(cross(V,D)); V-side normal = normalize(cross(D,U)) -
// see face_reference_resolver.cpp's own DeriveExpectedPlane comment for the
// full reasoning. These values were computed independently (Python/NumPy,
// this session's own sandbox) from the same U/V/D this fixture uses above,
// not copied from the production code under test.
//
// This is a genuine regression fixture, not merely a new fixture: the OLD
// (pre-B13-C1) algorithm used cap normal = D, U-side normal = U, V-side
// normal = V directly. For this D, those OLD normals diverge from the
// CORRECT ones computed below by roughly 23 degrees (cap), 14 degrees
// (U-side), and 19 degrees (V-side) - each several orders of magnitude
// past kTolerance's angular_radians (1.0e-6 rad, roughly 0.00006 degrees).
// Every one of the six matches below would therefore have failed the
// angular test under the OLD algorithm, and every role would have resolved
// Missing instead of Resolved - this fixture is only satisfied by the
// corrected geometry.
[[nodiscard]] std::vector<FaceObservation> ObliqueSixFaces() {
    return {
        MakeFace(Point3{0.0, 0.0, 0.0}, Vector3{0.0, -1.0, 0.0}),  // StartCap
        MakeFace(Point3{0.045931523, 0.183726082, 0.064304132}, Vector3{0.0, -1.0, 0.0}),  // EndCap
        MakeFace(Point3{0.0, 0.0, 0.0}, Vector3{-0.970142500, 0.242535625, 0.0}),  // UMinSide
        MakeFace(Point3{6.0, 0.0, 0.0}, Vector3{-0.970142500, 0.242535625, 0.0}),  // UMaxSide
        MakeFace(Point3{0.0, 0.0, 0.0}, Vector3{0.0, 0.330350423, -0.943858362}),  // VMinSide
        MakeFace(Point3{0.0, 0.0, 3.0}, Vector3{0.0, 0.330350423, -0.943858362}),  // VMaxSide
    };
}

} // namespace

TEST_CASE("IsValidFaceRole accepts only the six named roles",
          "[unit][model][p0-t008][face-reference]") {
    for (const FaceRole role : kAllRoles) {
        REQUIRE(IsValidFaceRole(role));
    }
    REQUIRE_FALSE(IsValidFaceRole(static_cast<FaceRole>(200)));
}

TEST_CASE("Sanity: the canonical six-face set resolves every role to a distinct face",
          "[unit][model][p0-t008][face-reference]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    observed.faces = CanonicalSixFaces();

    for (std::size_t i = 0; i < kAllRoles.size(); ++i) {
        const PersistentFaceReference reference{kOwner, kAllRoles[i]};
        const FaceResolutionResult result =
            ResolveFaceReference(reference, frame, observed, kTolerance);
        REQUIRE(result.status == FaceResolutionStatus::Resolved);
        REQUIRE(result.resolved_face_index == i);
    }
}

TEST_CASE("P0-T008 B13-C1 regression: an oblique extrusion (direction not perpendicular to the "
          "profile) resolves all six roles correctly",
          "[unit][model][p0-t008][face-reference][B13-C1]") {
    // ObliqueSpec()'s direction is deliberately not perpendicular to its
    // U/V profile plane, and ObliqueSixFaces() reproduces (as hand-built
    // data) exactly what a real kernel solid built from that spec would
    // present. See ObliqueSixFaces()'s own comment for why this fixture is
    // only satisfiable by the corrected (cross-product) plane derivation,
    // not the prior direct-axis one.
    const FeatureFrame frame{kOwner, ObliqueSpec()};
    FaceObservationResult observed;
    observed.faces = ObliqueSixFaces();

    std::vector<std::size_t> resolved_indices;
    for (std::size_t i = 0; i < kAllRoles.size(); ++i) {
        const PersistentFaceReference reference{kOwner, kAllRoles[i]};
        const FaceResolutionResult result =
            ResolveFaceReference(reference, frame, observed, kTolerance);
        REQUIRE(result.status == FaceResolutionStatus::Resolved);
        REQUIRE(result.resolved_face_index == i);
        resolved_indices.push_back(result.resolved_face_index);
    }
    // No role relied on vector/enumeration position to be picked out from
    // any other - each of the six indices above is distinct (they happen
    // to equal 0..5 in this fixture's own construction order, but the
    // property being checked is that six DIFFERENT faces were matched, not
    // that any particular index arrived).
    for (std::size_t i = 0; i < resolved_indices.size(); ++i) {
        for (std::size_t j = i + 1; j < resolved_indices.size(); ++j) {
            REQUIRE(resolved_indices[i] != resolved_indices[j]);
        }
    }
}

TEST_CASE("Enumeration order never changes which role resolves or its outcome",
          "[unit][model][p0-t008][face-reference][enumeration-order-independence]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    const std::vector<FaceObservation> canonical = CanonicalSixFaces();

    // Three differently-ordered copies of the exact same six observations,
    // plus (in the shuffled copies) two irrelevant non-planar decoy faces
    // interleaved, so this is not merely "the same order in reverse" but a
    // genuinely different arrival order with noise mixed in.
    std::vector<FaceObservation> forward = canonical;

    std::vector<FaceObservation> reversed(canonical.rbegin(), canonical.rend());

    std::vector<FaceObservation> shuffled;
    FaceObservation decoy;
    decoy.surface_kind = SurfaceKind::NonPlanar;
    shuffled.push_back(decoy);
    shuffled.push_back(canonical[2]);
    shuffled.push_back(canonical[5]);
    shuffled.push_back(canonical[0]);
    shuffled.push_back(decoy);
    shuffled.push_back(canonical[4]);
    shuffled.push_back(canonical[1]);
    shuffled.push_back(canonical[3]);

    const std::array<std::vector<FaceObservation>, 3> orderings{forward, reversed, shuffled};

    for (const FaceRole role : kAllRoles) {
        const PersistentFaceReference reference{kOwner, role};
        FaceResolutionStatus first_status = FaceResolutionStatus::InvalidReference;
        Point3 first_matched_point{};
        bool have_first = false;

        for (const std::vector<FaceObservation>& ordering : orderings) {
            FaceObservationResult observed;
            observed.faces = ordering;
            const FaceResolutionResult result =
                ResolveFaceReference(reference, frame, observed, kTolerance);
            REQUIRE(result.status == FaceResolutionStatus::Resolved);

            const Point3 matched_point = ordering[result.resolved_face_index].point_on_surface;
            if (!have_first) {
                first_status = result.status;
                first_matched_point = matched_point;
                have_first = true;
            } else {
                REQUIRE(result.status == first_status);
                // Different orderings resolve to a different INDEX (that is
                // expected and fine - the index is a property of the
                // vector, not of identity), but they must land on the same
                // underlying face, identified here by its geometric
                // position, which is the same regardless of where in the
                // vector it was placed.
                REQUIRE(matched_point.x == first_matched_point.x);
                REQUIRE(matched_point.y == first_matched_point.y);
                REQUIRE(matched_point.z == first_matched_point.z);
            }
        }
    }
}

TEST_CASE("A role with no matching face resolves Missing", "[unit][model][p0-t008][face-reference]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    const std::vector<FaceObservation> six = CanonicalSixFaces();
    // Every face EXCEPT EndCap (index 1).
    for (std::size_t i = 0; i < six.size(); ++i) {
        if (i != 1) {
            observed.faces.push_back(six[i]);
        }
    }

    const PersistentFaceReference end_cap_reference{kOwner, FaceRole::EndCap};
    const FaceResolutionResult result =
        ResolveFaceReference(end_cap_reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::Missing);

    // The other five roles are unaffected.
    for (const FaceRole role : kAllRoles) {
        if (role == FaceRole::EndCap) {
            continue;
        }
        const PersistentFaceReference reference{kOwner, role};
        const FaceResolutionResult other_result =
            ResolveFaceReference(reference, frame, observed, kTolerance);
        REQUIRE(other_result.status == FaceResolutionStatus::Resolved);
    }
}

TEST_CASE("P0-T008 corpus F: a role with two matching candidate faces resolves Ambiguous, never "
          "guesses",
          "[unit][model][p0-t008][face-reference][corpusF]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    observed.faces = CanonicalSixFaces();

    // Simulate UMaxSide (x=6 plane) having been split into two coplanar
    // sub-faces by some other operation: append a SECOND face at the exact
    // same plane as the existing UMaxSide entry (index 3).
    FaceObservation split_sibling = observed.faces[3];
    split_sibling.area = 0.4; // a genuinely different sub-area, same plane
    observed.faces.push_back(split_sibling);

    const PersistentFaceReference u_max_reference{kOwner, FaceRole::UMaxSide};
    const FaceResolutionResult result =
        ResolveFaceReference(u_max_reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::Ambiguous);

    // Every other role is unaffected - only the role whose plane now has
    // two candidates goes Ambiguous.
    for (const FaceRole role : kAllRoles) {
        if (role == FaceRole::UMaxSide) {
            continue;
        }
        const PersistentFaceReference reference{kOwner, role};
        const FaceResolutionResult other_result =
            ResolveFaceReference(reference, frame, observed, kTolerance);
        REQUIRE(other_result.status == FaceResolutionStatus::Resolved);
    }
}

TEST_CASE("P0-T008 corpus H: a reference presented against the wrong owner is InvalidOwner, "
          "never resolved by falling back to geometry",
          "[unit][model][p0-t008][face-reference][corpusH]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    observed.faces = CanonicalSixFaces();

    const FeatureOwnerId wrong_owner{kOwner.value + 1};
    const PersistentFaceReference reference{wrong_owner, FaceRole::StartCap};

    const FaceResolutionResult result = ResolveFaceReference(reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::InvalidOwner);
}

TEST_CASE("P0-T008 corpus H: a structurally invalid role is InvalidReference before any owner "
          "or geometry check runs",
          "[unit][model][p0-t008][face-reference][corpusH]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    observed.faces = CanonicalSixFaces();

    // An out-of-range role smuggled in via a cast - IsValidFaceRole()
    // already proves this value is not one of the six named roles; here we
    // prove the resolver itself rejects it, even though the owner below is
    // correct and the observation set is perfectly healthy.
    const PersistentFaceReference reference{kOwner, static_cast<FaceRole>(200)};

    const FaceResolutionResult result = ResolveFaceReference(reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::InvalidReference);
}

TEST_CASE("An upstream face-observation failure fails closed as InvalidReference, never a false "
          "Missing",
          "[unit][model][p0-t008][face-reference]") {
    const FeatureFrame frame{kOwner, CanonicalSpec()};
    FaceObservationResult observed;
    observed.error = GeometryError{GeometryErrorCode::KernelOperationFailed, "simulated upstream failure"};
    // Even if some faces happen to be present, an unhealthy result must not
    // be trusted.
    observed.faces = CanonicalSixFaces();

    const PersistentFaceReference reference{kOwner, FaceRole::StartCap};
    const FaceResolutionResult result = ResolveFaceReference(reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::InvalidReference);
}

TEST_CASE("A degenerate current-owner spec fails closed as InvalidReference rather than dividing "
          "by (near) zero",
          "[unit][model][p0-t008][face-reference]") {
    LinearExtrusionSpec degenerate_spec = CanonicalSpec();
    degenerate_spec.profile.u_axis = Vector3{0.0, 0.0, 0.0}; // zero-length axis
    const FeatureFrame frame{kOwner, degenerate_spec};

    FaceObservationResult observed;
    observed.faces = CanonicalSixFaces();

    const PersistentFaceReference reference{kOwner, FaceRole::UMinSide};
    const FaceResolutionResult result = ResolveFaceReference(reference, frame, observed, kTolerance);
    REQUIRE(result.status == FaceResolutionStatus::InvalidReference);
}

TEST_CASE("PersistentFaceReference and FeatureOwnerId are equality-comparable plain data",
          "[unit][model][p0-t008][face-reference]") {
    const PersistentFaceReference a{kOwner, FaceRole::VMaxSide};
    const PersistentFaceReference b{kOwner, FaceRole::VMaxSide};
    const PersistentFaceReference c{kOwner, FaceRole::VMinSide};
    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);

    const FeatureOwnerId owner_a{1};
    const FeatureOwnerId owner_b{1};
    const FeatureOwnerId owner_c{2};
    REQUIRE(owner_a == owner_b);
    REQUIRE_FALSE(owner_a == owner_c);
}
