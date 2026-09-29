#pragma once

#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/persistent_face_reference.hpp"

#include <cstddef>

// P0-T008 Topological Reference Spike - semantic naming/resolution
// (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0; Architecture Gate
// 01-ARCHITECTURE-GATE.md). This header lives under bim_model's public
// surface and is scanned, including every comment, by the same
// model-public-header architecture rule persistent_face_reference.hpp's own
// header comment describes - so, like that header, this one never names the
// third-party CAD kernel or any of its symbols.
//
// ResolveFaceReference() never breaks a tie by kernel-assigned traversal
// position, by which candidate happens to come first in the observation
// list, by surface area, or by any address/handle/hash of a kernel object
// (Implementation Brief "prohibited identity sources" / "must not break
// ties by enumeration order"). Its only inputs are project-owned geometric
// data (bim::geometry_api's neutral types) and the reference itself; it
// derives the CURRENT expected plane for the reference's role fresh, every
// call, purely from the caller-supplied current feature frame - never from
// any cached or previously observed geometry - which is what lets
// resolution survive regeneration, a dimension change, and a translation.

namespace bim::model {

// --- FaceResolutionStatus --------------------------------------------------------
// Five states, per the Brief: a caller must be able to tell "found exactly
// one matching face" apart from every other outcome, and ambiguity must
// fail closed (never silently pick one of several candidates).
enum class FaceResolutionStatus : std::uint8_t {
    Resolved,
    Missing,
    Ambiguous,
    InvalidReference,
    InvalidOwner,
};

// --- FaceResolutionResult ---------------------------------------------------------
struct FaceResolutionResult {
    FaceResolutionStatus status = FaceResolutionStatus::InvalidReference;

    // Valid only when status == Resolved: the position, in the
    // ALREADY-fully-enumerated `observed_faces.faces` vector the caller
    // supplied, of the single face the resolver matched. This is an OUTPUT
    // convenience for the caller's own immediate use of that one call's
    // result (e.g. to then read that face's area) - it is never itself
    // treated as identity, never persisted, and never fed back into a
    // future resolution call. The matching decision that produced it was
    // made by a full, order-independent scan of every candidate, not by
    // this index.
    std::size_t resolved_face_index = 0;

    [[nodiscard]] bool ok() const noexcept { return status == FaceResolutionStatus::Resolved; }
};

// --- FeatureFrame -------------------------------------------------------------
// Everything the resolver needs to derive the CURRENT expected geometric
// plane for each of the six roles: which feature owns the reference, and
// that feature's current defining specification. The resolver never
// inspects `observed_faces` to figure out what the expected geometry
// "should" be - the expected geometry comes only from `spec`, recomputed
// fresh on every call.
struct FeatureFrame {
    FeatureOwnerId owner;
    bim::geometry_api::LinearExtrusionSpec spec;
};

// ResolveFaceReference(reference, current_owner_frame, observed_faces,
// tolerance): re-resolve a durable reference against a feature's current
// geometry.
//
// Order of checks (fail closed at the first that applies):
//   1. reference.role must be one of the six named roles
//      (bim::model::IsValidFaceRole) - otherwise InvalidReference.
//   2. reference.owner must equal current_owner_frame.owner - otherwise
//      InvalidOwner.
//   3. observed_faces must itself carry no upstream error
//      (observed_faces.ok()) - a face-observation failure is never silently
//      treated as "no faces", so it also yields InvalidReference rather
//      than a false Missing.
//   4. The role's expected (unit normal, point-on-plane) pair is derived
//      purely from current_owner_frame.spec. Every planar entry in
//      observed_faces.faces is tested against it: parallel within
//      tolerance.angular_radians (sign-agnostic) AND within
//      tolerance.linear of the expected plane's offset. The COUNT of faces
//      that pass both tests decides the result - zero is Missing, exactly
//      one is Resolved, more than one is Ambiguous. Every candidate is
//      scored independently of every other candidate and independently of
//      its position in the vector, so shuffling observed_faces.faces can
//      never change the result.
[[nodiscard]] FaceResolutionResult ResolveFaceReference(
    const PersistentFaceReference& reference, const FeatureFrame& current_owner_frame,
    const bim::geometry_api::FaceObservationResult& observed_faces,
    const bim::geometry_api::GeometryTolerance& tolerance);

} // namespace bim::model
