#pragma once

#include "bim/model/element_id.hpp"

#include <cstdint>

// P1-T001 Core BIM Identity & Domain Model - production wall semantic face
// roles and reference (Implementation Brief BIM-TASK-P1-T001-CLAUDE v1.0
// sections 9-12).
//
// These are production Phase-1 types, deliberately separate from the
// historical Phase-0 spike contract in persistent_face_reference.hpp, which
// remains unchanged. Nothing here aliases or reuses that spike's owner id or
// role enumeration.
//
// Frozen meaning of the six roles, in the wall frame (W = +Z,
// U = normalize(end - start), V = W x U):
//
//     Bottom = minimum W plane / wall base
//     Top    = maximum W plane / wall top
//     Start  = minimum U plane / start endpoint
//     End    = maximum U plane / end endpoint
//     SideA  = minimum V plane
//     SideB  = maximum V plane
//
// Enumeration order is never identity, and no seventh role is authorized.
// A reference holds only a durable wall ElementId and a role: no raw face
// index, handle, pointer, topology ordinal or transient geometry identifier.
// Resolving a reference against generated geometry is not part of P1-T001.

namespace bim::model {

enum class WallFaceRole : std::uint8_t {
    Bottom,
    Top,
    Start,
    End,
    SideA,
    SideB,
};

// True only for one of the six named enumerators. A value obtained by casting
// an out-of-range integer is not a valid role.
[[nodiscard]] constexpr bool IsValidWallFaceRole(WallFaceRole role) noexcept {
    switch (role) {
        case WallFaceRole::Bottom:
        case WallFaceRole::Top:
        case WallFaceRole::Start:
        case WallFaceRole::End:
        case WallFaceRole::SideA:
        case WallFaceRole::SideB:
            return true;
    }
    return false;
}

struct WallFaceReference {
    ElementId wall_id;
    WallFaceRole role = WallFaceRole::Bottom;

    friend constexpr bool operator==(const WallFaceReference&, const WallFaceReference&) noexcept = default;
};

} // namespace bim::model
