// P0-T008 Topological Reference Spike - real-kernel mandatory proof corpus
// (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0). Exercises the full,
// real pipeline end to end: bim::geometry_api::MakeLinearExtrusion/Cut
// (bim_geometry_occt) -> bim::geometry_api::ObserveFaces (bim_geometry_occt)
// -> bim::model::ResolveFaceReference (bim_model, kernel-free). Covers
// corpus cases A (same-spec regeneration), B (dimension change), C
// (translation), D (repeat regeneration), E (Boolean-cut continuity: a
// corner notch that never fully removes a named face) and G (deleted-face:
// a cut that fully removes the EndCap region, which must resolve as
// Missing). Cases F (split-face -> Ambiguous) and H (invalid owner /
// invalid reference) are proven at the model level instead
// (tests/unit/unit_model_face_reference_resolver.cpp) - both are pure
// resolver-contract properties that do not need, and are not made any more
// certain by, a real kernel solid; this file's job is proving continuity
// against the REAL kernel, which is what A-E/G specifically need.
//
// The FeatureFrame's LinearExtrusionSpec is deliberately never re-derived
// from the post-cut solid in the E/G cases below: it always describes the
// base extrusion's own unchanged defining geometry, exactly as a later
// Boolean feature (an opening, a notch) would never itself redefine the
// base feature it was cut into.

#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/face_reference_resolver.hpp"
#include "bim/model/persistent_face_reference.hpp"
#include "geometry_occt_test_constants.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <set>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;
using bim::model::FaceResolutionResult;
using bim::model::FaceResolutionStatus;
using bim::model::FaceRole;
using bim::model::FeatureFrame;
using bim::model::FeatureOwnerId;
using bim::model::PersistentFaceReference;
using bim::model::ResolveFaceReference;

namespace {

constexpr FeatureOwnerId kOwner{42};

// Looser than kReferenceTolerance (the kernel-build/inspection tolerance
// used elsewhere in this corpus, 1.0e-6/1.0e-8) so real Boolean-operation
// floating-point noise cannot spuriously fail a match, but still far
// tighter than any real separation between two of this corpus's named
// planes (the smallest is P01's 0.2 extrusion distance) - no two distinct
// named planes can ever be confused at this tolerance.
constexpr GeometryTolerance kResolutionTolerance{1.0e-4, 1.0e-6};

constexpr std::array<FaceRole, 6> kAllRoles{FaceRole::StartCap,  FaceRole::EndCap,  FaceRole::UMinSide,
                                            FaceRole::UMaxSide,  FaceRole::VMinSide, FaceRole::VMaxSide};

[[nodiscard]] SolidHandle BuildOrRequireOk(const LinearExtrusionSpec& spec) {
    const SolidResult result = MakeLinearExtrusion(spec, kReferenceTolerance);
    REQUIRE(result.ok());
    return result.solid;
}

[[nodiscard]] FaceObservationResult ObserveOrRequireOk(const SolidHandle& solid) {
    const FaceObservationResult observed = ObserveFaces(solid);
    REQUIRE(observed.ok());
    return observed;
}

// Resolves all six roles against `observed` using `frame`, requires every
// one to be Resolved, and requires the six resolved face indices to be
// pairwise distinct (each role names a DIFFERENT face of the solid, never
// the same face twice).
void RequireAllSixRolesResolveDistinctly(const FeatureFrame& frame,
                                         const FaceObservationResult& observed) {
    std::set<std::size_t> seen_indices;
    for (const FaceRole role : kAllRoles) {
        const PersistentFaceReference reference{kOwner, role};
        const FaceResolutionResult result =
            ResolveFaceReference(reference, frame, observed, kResolutionTolerance);
        REQUIRE(result.status == FaceResolutionStatus::Resolved);
        const auto [it, inserted] = seen_indices.insert(result.resolved_face_index);
        REQUIRE(inserted);
        (void)it;
    }
    REQUIRE(seen_indices.size() == 6);
}

// --- B13-C1 oblique-extrusion regression fixture -----------------------------
// Same U/V/size_u/size_v/distance as MakeP01() (origin (0,0,0); u=(1,0,0)
// size_u=6; v=(0,0,1) size_v=3; distance=0.2), but with an extrusion
// direction deliberately NOT perpendicular to the profile plane -
// MakeLinearExtrusion validates u_axis orthogonal to v_axis but never
// requires that of direction, so this is valid geometry. MakeAxisAlignedBox
// cannot express this (it hard-codes direction=(0,1,0)), so this fixture is
// built directly rather than reusing that helper. `direction` is supplied
// already unit-length so this fixture's arithmetic never depends on
// whether MakeLinearExtrusion normalizes it internally before scaling by
// `distance` - the model-level counterpart of this fixture
// (tests/unit/unit_model_face_reference_resolver.cpp's ObliqueSpec())
// makes the identical choice for the identical reason, and uses the exact
// same numeric direction so both fixtures describe the same prism.
[[nodiscard]] LinearExtrusionSpec MakeObliqueP01() {
    RectangleProfile3 profile;
    profile.origin = Point3{0.0, 0.0, 0.0};
    profile.u_axis = Vector3{1.0, 0.0, 0.0};
    profile.v_axis = Vector3{0.0, 0.0, 1.0};
    profile.size_u = 6.0;
    profile.size_v = 3.0;

    LinearExtrusionSpec spec;
    spec.profile = profile;
    // normalize((0.25, 1.0, 0.35)), precomputed - identical to
    // ObliqueSpec()'s direction in the model-level regression test.
    spec.direction = Vector3{0.229657614, 0.918630457, 0.321520660};
    spec.distance = 0.2;
    return spec;
}

} // namespace

TEST_CASE("P0-T008 sanity: a fresh P01 extrusion resolves all six roles distinctly",
          "[integration][model][p0-t008][face-reference]") {
    const LinearExtrusionSpec spec = MakeP01();
    const FeatureFrame frame{kOwner, spec};
    const SolidHandle solid = BuildOrRequireOk(spec);
    const FaceObservationResult observed = ObserveOrRequireOk(solid);

    // ObserveFaces must report a real face for every one of the primitive
    // box's six sides, all planar.
    REQUIRE(observed.faces.size() == 6);
    for (const FaceObservation& face : observed.faces) {
        REQUIRE(face.surface_kind == SurfaceKind::Planar);
    }

    RequireAllSixRolesResolveDistinctly(frame, observed);
}

TEST_CASE("P0-T008 corpus A: same-spec regeneration re-resolves every role",
          "[integration][model][p0-t008][face-reference][corpusA]") {
    const LinearExtrusionSpec spec = MakeP01();
    const FeatureFrame frame{kOwner, spec};

    // A SECOND, independent MakeLinearExtrusion call from the identical
    // spec - a fresh kernel solid, not a copy of the first one.
    const SolidHandle regenerated = BuildOrRequireOk(spec);
    const FaceObservationResult observed = ObserveOrRequireOk(regenerated);

    RequireAllSixRolesResolveDistinctly(frame, observed);
}

TEST_CASE("P0-T008 corpus B: a dimension change moves the matched planes with it",
          "[integration][model][p0-t008][face-reference][corpusB]") {
    LinearExtrusionSpec spec = MakeP01();
    // Enlarge every dimension P01 has: size_u 6.0 -> 9.0, size_v 3.0 -> 5.0,
    // distance 0.2 -> 0.5.
    spec.profile.size_u = 9.0;
    spec.profile.size_v = 5.0;
    spec.distance = 0.5;

    const FeatureFrame frame{kOwner, spec};
    const SolidHandle solid = BuildOrRequireOk(spec);
    const FaceObservationResult observed = ObserveOrRequireOk(solid);

    RequireAllSixRolesResolveDistinctly(frame, observed);

    // The ORIGINAL (pre-resize) spec's expected planes must now NOT match
    // this new, larger solid's faces at UMaxSide/VMaxSide/EndCap (they have
    // moved) - proving the resolver is actually using the current spec, not
    // silently reusing whatever it resolved last time.
    const LinearExtrusionSpec original_spec = MakeP01();
    const FeatureFrame stale_frame{kOwner, original_spec};
    const PersistentFaceReference end_cap_reference{kOwner, FaceRole::EndCap};
    const FaceResolutionResult stale_result =
        ResolveFaceReference(end_cap_reference, stale_frame, observed, kResolutionTolerance);
    REQUIRE(stale_result.status == FaceResolutionStatus::Missing);
}

TEST_CASE("P0-T008 corpus C: translation moves the matched planes with it",
          "[integration][model][p0-t008][face-reference][corpusC]") {
    const Point3 translation{500.0, -250.0, 75.0};
    const LinearExtrusionSpec spec = MakeP01(translation);

    const FeatureFrame frame{kOwner, spec};
    const SolidHandle solid = BuildOrRequireOk(spec);
    const FaceObservationResult observed = ObserveOrRequireOk(solid);

    RequireAllSixRolesResolveDistinctly(frame, observed);
}

TEST_CASE("P0-T008 corpus D: repeated regeneration is stable across every repeat",
          "[integration][model][p0-t008][face-reference][corpusD]") {
    const LinearExtrusionSpec spec = MakeP01();
    const FeatureFrame frame{kOwner, spec};

    constexpr int kRepeatCount = 5;
    for (int i = 0; i < kRepeatCount; ++i) {
        const SolidHandle solid = BuildOrRequireOk(spec);
        const FaceObservationResult observed = ObserveOrRequireOk(solid);
        RequireAllSixRolesResolveDistinctly(frame, observed);
    }
}

TEST_CASE("P0-T008 corpus E: a corner notch cut leaves every named role resolvable",
          "[integration][model][p0-t008][face-reference][corpusE]") {
    const LinearExtrusionSpec spec = MakeP01();
    const FeatureFrame frame{kOwner, spec};
    const SolidHandle host = BuildOrRequireOk(spec);

    // A small notch straddling the origin corner: spans P01's entire
    // y-thickness (0 to 0.2) so it cuts clean through, and a small x/z
    // footprint (0 to 0.5 on each axis after intersecting with the host)
    // so it only bites a corner of StartCap/EndCap/UMinSide/VMinSide,
    // never consuming any of the six named faces entirely. UMaxSide
    // (x=6) and VMaxSide (z=3) are untouched by construction.
    const LinearExtrusionSpec notch_tool = MakeAxisAlignedBox(-0.5, 0.5, -0.5, 0.7, -0.5, 0.5);
    const SolidHandle tool = BuildOrRequireOk(notch_tool);

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.ok());

    const FaceObservationResult observed = ObserveOrRequireOk(cut_result.solid);
    // The notch introduces new faces belonging to the tool's own cut walls
    // (none of which lie on any of the six expected planes - the tool's own
    // side/end walls are all offset from every P01 plane by at least 0.3),
    // so more than six faces are expected here; the point of this case is
    // that every one of the six NAMED roles still resolves to exactly one
    // face despite that.
    REQUIRE(observed.faces.size() >= 6);

    RequireAllSixRolesResolveDistinctly(frame, observed);
}

TEST_CASE("P0-T008 corpus G: a cut that fully removes EndCap resolves it as Missing",
          "[integration][model][p0-t008][face-reference][corpusG]") {
    const LinearExtrusionSpec spec = MakeP01();
    const FeatureFrame frame{kOwner, spec};
    const SolidHandle host = BuildOrRequireOk(spec);

    // A slab tool spanning well beyond P01's x/z extent (x:[0,6], z:[0,3])
    // and y:[0.15, 1.0] - past P01's y=0.2 boundary entirely, so this
    // removes every last trace of the y=0.2 EndCap plane, leaving the
    // solid's new far boundary at y=0.15 instead.
    const LinearExtrusionSpec removal_tool = MakeAxisAlignedBox(-1.0, 7.0, 0.15, 1.0, -1.0, 4.0);
    const SolidHandle tool = BuildOrRequireOk(removal_tool);

    const SolidResult cut_result = Cut(host, tool, kReferenceTolerance);
    REQUIRE(cut_result.ok());

    const FaceObservationResult observed = ObserveOrRequireOk(cut_result.solid);

    const PersistentFaceReference end_cap_reference{kOwner, FaceRole::EndCap};
    const FaceResolutionResult end_cap_result =
        ResolveFaceReference(end_cap_reference, frame, observed, kResolutionTolerance);
    REQUIRE(end_cap_result.status == FaceResolutionStatus::Missing);

    // Every other role is untouched by this cut (the tool's x/z extent
    // covers the whole host, so it never clips UMinSide/UMaxSide/
    // VMinSide/VMaxSide's own planes, and StartCap at y=0 is far from the
    // y:[0.15,1.0] removal).
    for (const FaceRole role : kAllRoles) {
        if (role == FaceRole::EndCap) {
            continue;
        }
        const PersistentFaceReference reference{kOwner, role};
        const FaceResolutionResult result =
            ResolveFaceReference(reference, frame, observed, kResolutionTolerance);
        REQUIRE(result.status == FaceResolutionStatus::Resolved);
    }
}

TEST_CASE("P0-T008 B13-C1 regression: a real-kernel oblique extrusion resolves all six roles "
          "to six distinct faces",
          "[integration][model][p0-t008][face-reference][B13-C1]") {
    const LinearExtrusionSpec spec = MakeObliqueP01();
    const FeatureFrame frame{kOwner, spec};

    const SolidHandle solid = BuildOrRequireOk(spec);
    const FaceObservationResult observed = ObserveOrRequireOk(solid);

    REQUIRE(observed.faces.size() == 6);
    for (const FaceObservation& face : observed.faces) {
        REQUIRE(face.surface_kind == SurfaceKind::Planar);
    }

    RequireAllSixRolesResolveDistinctly(frame, observed);
}
