#pragma once

#include <cstdint>

// P0-T008 Topological Reference Spike - project-owned, kernel-free
// persistent Face reference (Implementation Brief BIM-TASK-P0-T008-CLAUDE
// v1.0; Architecture Gate 01-ARCHITECTURE-GATE.md). Every type in this
// header must stay free of any raw CAD-kernel construct, any memory
// address/pointer/hash of a kernel object, any kernel-assigned traversal
// index, and any transient per-call face ordinal - none of those may ever
// be treated as durable identity (see this task's Implementation Brief,
// "prohibited identity sources"). This header lives under bim_model's
// public surface and is scanned, word for word including every comment,
// by the mechanical architecture checker's model-public-header rule
// (tools/architecture_checker.cmake) - so this file, deliberately, never
// names the third-party kernel or any of its symbols, even in prose.
//
// The durable identity this spike introduces has exactly two parts: WHICH
// owning feature a face belongs to (FeatureOwnerId), and WHICH of that
// feature's six fixed extrusion roles the face plays (FaceRole). Neither
// part depends on how the feature's geometry happens to be represented
// internally, or on the order its faces happen to be produced in during any
// particular regeneration.

namespace bim::model {

// --- FaceRole -----------------------------------------------------------------
// The six semantic roles a face of a single linear-extrusion feature can
// play (Implementation Brief). This enumeration is closed and fixed for
// P0-T008: a split face cannot be represented by inventing a seventh role
// here (that would be a frozen-contract change, and any implementation that
// needed one would have to stop and escalate rather than add one silently -
// see the resolver's Ambiguous status instead, which is exactly the
// mechanism this spike uses for a face that has been split).
enum class FaceRole : std::uint8_t {
    StartCap,
    EndCap,
    UMinSide,
    UMaxSide,
    VMinSide,
    VMaxSide,
};

// True only for one of the six named enumerators above. A FaceRole value
// obtained by casting an out-of-range integer (which C++ does not prevent
// for a plain enum class) is exactly the "structurally invalid reference"
// case the resolver's InvalidReference status exists to catch, distinct
// from a well-formed reference that simply cannot currently be found
// (Missing) or that currently matches more than one face (Ambiguous).
[[nodiscard]] constexpr bool IsValidFaceRole(FaceRole role) noexcept {
    switch (role) {
        case FaceRole::StartCap:
        case FaceRole::EndCap:
        case FaceRole::UMinSide:
        case FaceRole::UMaxSide:
        case FaceRole::VMinSide:
        case FaceRole::VMaxSide:
            return true;
    }
    return false;
}

// --- FeatureOwnerId -------------------------------------------------------------
// A small, project-assigned opaque tag distinguishing one linear-extrusion
// feature instance from another. This is deliberately NOT a general
// platform element-id or UUID scheme - introducing one is out of scope for
// this spike (Implementation Brief out-of-scope list: no new platform-wide
// identity/persistence architecture). FeatureOwnerId exists only so this
// spike's resolver has something to check a reference's claimed owner
// against, producing the InvalidOwner status when a reference is presented
// against a feature it does not belong to.
struct FeatureOwnerId {
    std::uint64_t value = 0;

    friend constexpr bool operator==(const FeatureOwnerId&, const FeatureOwnerId&) noexcept = default;
};

// --- PersistentFaceReference -----------------------------------------------------
// The durable reference itself: exactly two project-owned, plain-data
// fields, nothing else. Deliberately copyable, deliberately comparable,
// deliberately safe to store, serialize, or pass between calls without ever
// touching the kernel - re-resolving a reference after the owning feature's
// geometry has been regenerated is the entire point of
// bim::model::ResolveFaceReference (face_reference_resolver.hpp).
struct PersistentFaceReference {
    FeatureOwnerId owner;
    FaceRole role = FaceRole::StartCap;

    friend constexpr bool operator==(const PersistentFaceReference&,
                                      const PersistentFaceReference&) noexcept = default;
};

} // namespace bim::model
