#include "bim/query/query.hpp"

// P1-T003 Commands & Query Vertical Slice - implementation of bim::query
// (Architecture Gate BIM-AG-P1-T003 section 9; Implementation Brief P1-T003-IB
// section 5). Read-only by construction: every function takes a const
// reference to the document and forwards to one of its const committed-value
// reads.

namespace bim::query {

std::optional<bim::model::Level> FindLevel(const bim::document::Document& document,
                                           const bim::model::ElementId& id) noexcept {
    return document.FindLevel(id);
}

std::optional<bim::model::StraightWall> FindStraightWall(const bim::document::Document& document,
                                                         const bim::model::ElementId& id) noexcept {
    return document.FindStraightWall(id);
}

std::vector<bim::model::Level> ListLevels(const bim::document::Document& document) {
    return document.ListLevels();
}

std::vector<bim::model::StraightWall> ListStraightWalls(const bim::document::Document& document) {
    return document.ListStraightWalls();
}

} // namespace bim::query
