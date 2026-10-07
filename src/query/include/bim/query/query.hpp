#pragma once

#include "bim/document/document.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"

#include <optional>
#include <vector>

// P1-T003 Commands & Query Vertical Slice - the production read-only query
// boundary (Architecture Gate BIM-AG-P1-T003 section 9; Implementation Brief
// P1-T003-IB section 5).
//
// Every query receives a const bim::document::Document& and returns copies of
// committed values. A query can neither mutate a document nor observe a staged
// or partially applied mutation: bim::document publishes a mutation only once
// it has fully succeeded, so a query always reads a complete committed state.
// Lookups return an empty optional when no such element (of that kind) exists.
// Enumeration is deterministic - ElementId ascending, a semantic guarantee that
// never depends on internal storage or on any private runtime identity.
//
// There is no mutation, no command, no persistence, no transaction and no
// vendor type here; the QUERY_READ_ONLY_BOUNDARY architecture rule
// (tools/architecture_checker.cmake) enforces that mechanically.

namespace bim::query {

[[nodiscard]] std::optional<bim::model::Level> FindLevel(const bim::document::Document& document,
                                                         const bim::model::ElementId& id) noexcept;

[[nodiscard]] std::optional<bim::model::StraightWall>
FindStraightWall(const bim::document::Document& document, const bim::model::ElementId& id) noexcept;

// Committed Levels / StraightWalls in ElementId ascending order. Building the
// returned vector can run out of memory, so these two are not noexcept.
[[nodiscard]] std::vector<bim::model::Level> ListLevels(const bim::document::Document& document);

[[nodiscard]] std::vector<bim::model::StraightWall>
ListStraightWalls(const bim::document::Document& document);

} // namespace bim::query
