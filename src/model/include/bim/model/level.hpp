#pragma once

#include "bim/model/element_id.hpp"

// P1-T001 Core BIM Identity & Domain Model - Level (Implementation Brief
// BIM-TASK-P1-T001-CLAUDE v1.0 section 5).
//
// Minimum authoritative state only: a durable identity and an elevation in
// model units. There is no name/label field in P1-T001. Elevation may be
// positive, zero or negative. Structural validity (valid id, finite
// elevation) is checked by ValidateLevel() in validation.hpp; no tolerance
// is involved.

namespace bim::model {

struct Level {
    ElementId id;
    double elevation = 0.0;

    friend constexpr bool operator==(const Level&, const Level&) noexcept = default;
};

} // namespace bim::model
