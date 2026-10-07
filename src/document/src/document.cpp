#include "bim/document/document.hpp"

#include "bim/dependency_graph/dependency_graph.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

// P1-T002 Document Runtime & Dependency Recompute - implementation of
// bim::document (Architecture Gate BIM-AG-P1-T002; Implementation Brief
// P1-T002-IB), extended by P1-T003 (Architecture Gate BIM-AG-P1-T003 sections
// 5, 6 and 8) with deletion and the two ordered list reads.
//
// Everything graph-related lives in this file only. The public header never
// names a runtime graph type; ElementId is the only identity a caller sees.
//
// Strong atomicity: every mutation copies the whole committed runtime into a
// private staged State, performs every step (validation, runtime identity
// registration, graph mutation, recompute planning, derived-geometry
// evaluation, graph recompute commit) against that staged copy, and replaces
// the live State by a noexcept pointer move only after the complete operation
// succeeded and the runtime invariants hold. A failure at any step - a
// project-owned failure code or a contained exception - simply discards the
// staged copy, so no live container, mapping, graph revision, geometry-cache
// entry or identity-allocator value is ever changed before success. There is
// no mutate-then-rollback path anywhere.

namespace bim::document {

namespace dg = bim::dependency_graph;

using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::Point3;
using bim::geometry_api::RectangleProfile3;
using bim::geometry_api::Vector3;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::StraightWall;
using bim::model::ValidationCode;

namespace detail {

// The complete private runtime of one document. Copyable on purpose: the
// copy is the staging mechanism (every member is a value container or a
// copyable graph, so a copy shares nothing with the original).
struct State {
    std::map<ElementId, Level> levels;
    std::map<ElementId, StraightWall> walls;
    // Strict one-to-one runtime association between the durable ElementId and
    // the graph's runtime-only node identity. Levels and walls share one
    // ElementId namespace, so one pair of maps covers both kinds.
    std::map<ElementId, dg::NodeId> element_to_node;
    std::map<dg::NodeId, ElementId> node_to_element;
    dg::DependencyGraph graph;
    std::map<ElementId, LinearExtrusionSpec> wall_geometry;
    // Next runtime node value to hand out. The first value is 1; zero is never
    // emitted. After the largest value is handed out the counter wraps to 0,
    // which marks the identity space exhausted: values are never reused.
    std::uint64_t next_node_value = 1;
};

} // namespace detail

namespace {

[[nodiscard]] constexpr DocumentResult Fail(DocumentResultCode code) noexcept {
    return DocumentResult{.code = code, .validation = ValidationCode::Ok};
}

[[nodiscard]] constexpr DocumentResult FailInvalid(ValidationCode validation) noexcept {
    return DocumentResult{.code = DocumentResultCode::InvalidElement, .validation = validation};
}

[[nodiscard]] bool AllFinite(std::initializer_list<double> values) noexcept {
    for (const double value : values) {
        if (!std::isfinite(value)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool IsFiniteSpec(const LinearExtrusionSpec& spec) noexcept {
    const RectangleProfile3& p = spec.profile;
    return AllFinite({p.origin.x, p.origin.y, p.origin.z, p.u_axis.x, p.u_axis.y, p.u_axis.z,
                      p.v_axis.x, p.v_axis.y, p.v_axis.z, p.size_u, p.size_v, spec.direction.x,
                      spec.direction.y, spec.direction.z, spec.distance});
}

// Derives the neutral extrusion specification of a wall hosted by `level`
// (Architecture Gate section 12). Returns an empty optional when any derived
// scalar or vector component is not finite or the axis has no positive finite
// length. There is no tolerance and no clamping anywhere: an overflowing or
// non-finite intermediate is a failure, never a corrected value.
//
//   D = E - S, L = |D|, U = D / L, W = (0,0,1), V = W x U
//   origin = S - V * thickness / 2, at z = level.elevation + base_offset
//   u_axis = U, v_axis = V, size_u = L, size_v = thickness
//   direction = W, distance = height
[[nodiscard]] std::optional<LinearExtrusionSpec> DeriveWallGeometry(const StraightWall& wall,
                                                                    const Level& level) noexcept {
    const double dx = wall.end.x - wall.start.x;
    const double dy = wall.end.y - wall.start.y;
    if (!AllFinite({dx, dy})) {
        return std::nullopt;
    }

    // std::hypot avoids spurious intermediate overflow of dx*dx + dy*dy; a
    // genuinely unrepresentable length is still caught by the finiteness test.
    const double length = std::hypot(dx, dy);
    if (!std::isfinite(length) || !(length > 0.0)) {
        return std::nullopt;
    }

    const double ux = dx / length;
    const double uy = dy / length;
    if (!AllFinite({ux, uy})) {
        return std::nullopt;
    }

    // V = W x U with W = (0,0,1) and U = (ux, uy, 0):
    //   V = (0*0 - 1*uy, 1*ux - 0*0, 0*uy - 0*ux) = (-uy, ux, 0).
    // Written out so no signed zero is introduced into the z component.
    const double vx = 0.0 - uy;
    const double vy = ux - 0.0;
    if (!AllFinite({vx, vy})) {
        return std::nullopt;
    }

    // SideA is -V * thickness / 2 from the axis.
    const double half_thickness = wall.thickness / 2.0;
    const double offset_x = vx * half_thickness;
    const double offset_y = vy * half_thickness;
    if (!AllFinite({half_thickness, offset_x, offset_y})) {
        return std::nullopt;
    }

    const double origin_x = wall.start.x - offset_x;
    const double origin_y = wall.start.y - offset_y;
    const double base_z = level.elevation + wall.base_offset;
    if (!AllFinite({origin_x, origin_y, base_z})) {
        return std::nullopt;
    }

    const LinearExtrusionSpec spec{
        .profile = RectangleProfile3{.origin = Point3{origin_x, origin_y, base_z},
                                     .u_axis = Vector3{ux, uy, 0.0},
                                     .v_axis = Vector3{vx, vy, 0.0},
                                     .size_u = length,
                                     .size_v = wall.thickness},
        .direction = Vector3{0.0, 0.0, 1.0},
        .distance = wall.height,
    };
    if (!IsFiniteSpec(spec)) {
        return std::nullopt;
    }
    return spec;
}

// Hands out the next runtime node identity from the staged state. Fails
// (false) once the identity space is exhausted; the staged counter is left
// untouched on failure, and the live counter is never touched at all.
[[nodiscard]] bool AllocateNodeId(detail::State& state, dg::NodeId& out) noexcept {
    if (state.next_node_value == 0) {
        return false;
    }
    out = dg::NodeId{state.next_node_value};
    // Wraps to 0 after the largest value is issued: that is the deliberate
    // exhaustion marker (unsigned wrap-around is well defined).
    ++state.next_node_value;
    return true;
}

// Records the strict ElementId <-> node association in both directions.
[[nodiscard]] bool RegisterMapping(detail::State& state, const ElementId& id, dg::NodeId node) {
    const bool forward = state.element_to_node.emplace(id, node).second;
    const bool reverse = state.node_to_element.emplace(node, id).second;
    return forward && reverse;
}

[[nodiscard]] bool IsUsedElementId(const detail::State& state, const ElementId& id) {
    return state.levels.contains(id) || state.walls.contains(id);
}

// Evaluates every planned node in BuildRecomputePlan() order, then - only
// after every evaluation succeeded - commits the graph recompute and
// replaces the staged geometry-cache entries. No partial commit is ever
// issued: any evaluation failure returns before CommitRecompute() is called.
[[nodiscard]] DocumentResult RecomputeStaged(detail::State& state) {
    const dg::RecomputePlan plan = state.graph.BuildRecomputePlan();

    std::vector<dg::NodeEvaluationOutcome> outcomes;
    outcomes.reserve(plan.nodes.size());
    std::vector<std::pair<ElementId, LinearExtrusionSpec>> derived;
    derived.reserve(plan.nodes.size());

    for (const dg::NodeId node : plan.nodes) {
        const auto element = state.node_to_element.find(node);
        if (element == state.node_to_element.end()) {
            return Fail(DocumentResultCode::RecomputeFailed);
        }
        // Only StraightWalls are evaluated in P1-T002; a planned node that is
        // not a committed/staged wall is inconsistent runtime state.
        const auto wall = state.walls.find(element->second);
        if (wall == state.walls.end()) {
            return Fail(DocumentResultCode::RecomputeFailed);
        }
        const auto level = state.levels.find(wall->second.level_id);
        if (level == state.levels.end()) {
            return Fail(DocumentResultCode::RecomputeFailed);
        }
        const std::optional<LinearExtrusionSpec> spec =
            DeriveWallGeometry(wall->second, level->second);
        if (!spec.has_value()) {
            return Fail(DocumentResultCode::DerivedGeometryInvalid);
        }
        derived.emplace_back(element->second, *spec);
        outcomes.push_back(
            dg::NodeEvaluationOutcome{.node = node, .status = dg::EvaluationStatus::Success});
    }

    const dg::GraphResult commit = state.graph.CommitRecompute(plan, outcomes);
    // An empty plan is valid (nothing was dirty) and the graph reports it as
    // NoChange; a non-empty plan must commit cleanly.
    const bool commit_ok =
        plan.nodes.empty() ? commit.code == dg::GraphResultCode::NoChange : commit.ok();
    if (!commit_ok) {
        return Fail(DocumentResultCode::RecomputeFailed);
    }

    for (const auto& [wall_id, spec] : derived) {
        state.wall_geometry.insert_or_assign(wall_id, spec);
    }
    return DocumentResult{};
}

// Verifies the runtime invariants of a staged state before it is published:
// disjoint Level/wall namespaces; a strict reciprocal one-to-one
// ElementId <-> node mapping covering every element; one graph node per
// element, all Clean; exactly one ExplicitSemantic Level -> wall edge per
// wall; and one cached geometry entry per wall (none for a Level).
[[nodiscard]] bool InvariantsHold(const detail::State& state) {
    const std::size_t element_count = state.levels.size() + state.walls.size();
    if (state.element_to_node.size() != element_count ||
        state.node_to_element.size() != element_count ||
        state.wall_geometry.size() != state.walls.size()) {
        return false;
    }

    const auto mapped_both_ways = [&state](const ElementId& id) {
        const auto forward = state.element_to_node.find(id);
        if (forward == state.element_to_node.end() || !forward->second.is_valid()) {
            return false;
        }
        const auto reverse = state.node_to_element.find(forward->second);
        return reverse != state.node_to_element.end() && reverse->second == id;
    };

    for (const auto& [id, level] : state.levels) {
        if (level.id != id || state.walls.contains(id) || !mapped_both_ways(id) ||
            state.wall_geometry.contains(id)) {
            return false;
        }
    }
    for (const auto& [id, wall] : state.walls) {
        if (wall.id != id || !state.levels.contains(wall.level_id) || !mapped_both_ways(id) ||
            !state.wall_geometry.contains(id)) {
            return false;
        }
    }

    const dg::GraphSnapshot snapshot = state.graph.Snapshot();
    if (snapshot.nodes.size() != element_count || snapshot.edges.size() != state.walls.size()) {
        return false;
    }
    for (const dg::SnapshotNode& node : snapshot.nodes) {
        if (node.state != dg::NodeState::Clean || !state.node_to_element.contains(node.id)) {
            return false;
        }
    }
    for (const dg::SnapshotEdge& edge : snapshot.edges) {
        if (edge.provenance != dg::DependencyProvenance::ExplicitSemantic) {
            return false;
        }
        const auto downstream = state.node_to_element.find(edge.downstream);
        if (downstream == state.node_to_element.end()) {
            return false;
        }
        const auto wall = state.walls.find(downstream->second);
        if (wall == state.walls.end()) {
            return false;
        }
        const auto host = state.element_to_node.find(wall->second.level_id);
        if (host == state.element_to_node.end() || host->second != edge.upstream) {
            return false;
        }
    }
    return true;
}

// Removes the strict ElementId <-> node association of one element from both
// directions of the staged state. The allocator is not touched: a removed node
// identity is retired for good and is never handed out again.
[[nodiscard]] bool UnregisterMapping(detail::State& state, const ElementId& id) {
    const auto forward = state.element_to_node.find(id);
    if (forward == state.element_to_node.end()) {
        return false;
    }
    const bool reverse_erased = state.node_to_element.erase(forward->second) == 1;
    state.element_to_node.erase(forward);
    return reverse_erased;
}

// Replaces the staged graph by a freshly constructed one built only from the
// surviving staged state (Architecture Gate BIM-AG-P1-T003 section 6). The
// graph has no node-removal operation and gets none: after a deletion a new
// private graph is registered from the surviving identity mapping and the
// surviving Level -> StraightWall relationships. Every survivor is registered
// under the runtime node identity it already has - no identity is allocated,
// so next_node_value is neither read nor written here, and the node identity
// of a deleted element is not among the survivors and is never reused. Every
// surviving wall receives exactly one ExplicitSemantic Level -> wall edge, the
// same edge AddStraightWallStaged() creates. All rebuilt nodes are Clean, which
// is correct: deleting an element changes no surviving element's derived
// geometry, so the cached geometry of the survivors stays valid and no recompute
// is needed. The result still has to satisfy InvariantsHold() before RunStaged()
// may publish it. This runs against the staged copy only; the live graph is
// never rebuilt in place.
[[nodiscard]] DocumentResult RebuildGraphStaged(detail::State& state) {
    dg::DependencyGraph rebuilt;
    for (const auto& association : state.node_to_element) {
        if (!rebuilt.AddNode(association.first).ok()) {
            return Fail(DocumentResultCode::GraphRejected);
        }
    }
    for (const auto& entry : state.walls) {
        const auto host = state.element_to_node.find(entry.second.level_id);
        const auto node = state.element_to_node.find(entry.first);
        if (host == state.element_to_node.end() || node == state.element_to_node.end()) {
            return Fail(DocumentResultCode::InternalFailure);
        }
        if (!rebuilt
                 .AddDependency(host->second, node->second,
                                dg::DependencyProvenance::ExplicitSemantic)
                 .ok()) {
            return Fail(DocumentResultCode::GraphRejected);
        }
    }
    state.graph = std::move(rebuilt);
    return DocumentResult{};
}

// --- staged mutations: each operates on the private staged copy only --------

[[nodiscard]] DocumentResult AddLevelStaged(detail::State& state, const Level& level) {
    if (const auto validation = bim::model::ValidateLevel(level); !validation.ok()) {
        return FailInvalid(validation.code);
    }
    if (IsUsedElementId(state, level.id)) {
        return Fail(DocumentResultCode::DuplicateElementId);
    }

    dg::NodeId node{};
    if (!AllocateNodeId(state, node)) {
        return Fail(DocumentResultCode::NodeIdExhausted);
    }
    if (!state.graph.AddNode(node).ok()) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    state.levels.emplace(level.id, level);
    if (!RegisterMapping(state, level.id, node)) {
        return Fail(DocumentResultCode::InternalFailure);
    }
    // A new Level is Clean and has no derived geometry, so no recompute runs.
    return DocumentResult{};
}

[[nodiscard]] DocumentResult AddStraightWallStaged(detail::State& state, const StraightWall& wall) {
    if (const auto validation = bim::model::ValidateStraightWall(wall); !validation.ok()) {
        return FailInvalid(validation.code);
    }
    if (IsUsedElementId(state, wall.id)) {
        return Fail(DocumentResultCode::DuplicateElementId);
    }
    if (!state.levels.contains(wall.level_id)) {
        return Fail(DocumentResultCode::LevelNotFound);
    }
    const auto host = state.element_to_node.find(wall.level_id);
    if (host == state.element_to_node.end()) {
        return Fail(DocumentResultCode::InternalFailure);
    }
    const dg::NodeId level_node = host->second;

    dg::NodeId wall_node{};
    if (!AllocateNodeId(state, wall_node)) {
        return Fail(DocumentResultCode::NodeIdExhausted);
    }
    if (!state.graph.AddNode(wall_node).ok()) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    // Level -> StraightWall, ExplicitSemantic: the wall depends on its Level.
    if (!state.graph
             .AddDependency(level_node, wall_node, dg::DependencyProvenance::ExplicitSemantic)
             .ok()) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    state.walls.emplace(wall.id, wall);
    if (!RegisterMapping(state, wall.id, wall_node)) {
        return Fail(DocumentResultCode::InternalFailure);
    }
    if (!state.graph.MarkDirty(wall_node).ok()) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    return RecomputeStaged(state);
}

[[nodiscard]] DocumentResult UpdateLevelElevationStaged(detail::State& state, const ElementId& id,
                                                        double elevation) {
    const auto existing = state.levels.find(id);
    if (existing == state.levels.end()) {
        return Fail(state.walls.contains(id) ? DocumentResultCode::ElementKindMismatch
                                             : DocumentResultCode::ElementNotFound);
    }
    const Level candidate{.id = id, .elevation = elevation};
    if (const auto validation = bim::model::ValidateLevel(candidate); !validation.ok()) {
        return FailInvalid(validation.code);
    }
    const auto node = state.element_to_node.find(id);
    if (node == state.element_to_node.end()) {
        return Fail(DocumentResultCode::InternalFailure);
    }

    existing->second = candidate;

    // Strict downstream closure only: the Level itself is not marked Dirty.
    // NoChange (a Level that hosts no wall) is valid and yields an empty plan.
    const dg::GraphResult invalidated = state.graph.InvalidateDependents(node->second);
    if (!invalidated.ok() && invalidated.code != dg::GraphResultCode::NoChange) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    return RecomputeStaged(state);
}

[[nodiscard]] DocumentResult UpdateStraightWallStaged(detail::State& state,
                                                      const StraightWall& wall) {
    const auto existing = state.walls.find(wall.id);
    if (existing == state.walls.end()) {
        return Fail(state.levels.contains(wall.id) ? DocumentResultCode::ElementKindMismatch
                                                   : DocumentResultCode::ElementNotFound);
    }
    if (const auto validation = bim::model::ValidateStraightWall(wall); !validation.ok()) {
        return FailInvalid(validation.code);
    }
    // The id is the lookup key, so it is unchanged by construction. The
    // hosting Level may not change: no delete-and-recreate re-host exists.
    if (wall.level_id != existing->second.level_id) {
        return Fail(DocumentResultCode::RehostNotAllowed);
    }
    const auto node = state.element_to_node.find(wall.id);
    if (node == state.element_to_node.end()) {
        return Fail(DocumentResultCode::InternalFailure);
    }

    existing->second = wall;

    if (!state.graph.MarkDirty(node->second).ok()) {
        return Fail(DocumentResultCode::GraphRejected);
    }
    return RecomputeStaged(state);
}

// Deletes a StraightWall, or a Level that hosts no StraightWall. Level deletion
// never cascades and never re-hosts: a Level with any dependent wall is refused
// before anything is touched.
[[nodiscard]] DocumentResult DeleteElementStaged(detail::State& state, const ElementId& id) {
    const auto wall = state.walls.find(id);
    if (wall != state.walls.end()) {
        state.wall_geometry.erase(id);
        state.walls.erase(wall);
        if (!UnregisterMapping(state, id)) {
            return Fail(DocumentResultCode::InternalFailure);
        }
        return RebuildGraphStaged(state);
    }

    const auto level = state.levels.find(id);
    if (level == state.levels.end()) {
        return Fail(DocumentResultCode::ElementNotFound);
    }
    for (const auto& entry : state.walls) {
        if (entry.second.level_id == id) {
            return Fail(DocumentResultCode::ElementHasDependents);
        }
    }
    state.levels.erase(level);
    if (!UnregisterMapping(state, id)) {
        return Fail(DocumentResultCode::InternalFailure);
    }
    return RebuildGraphStaged(state);
}

// Runs `mutation` against a private staged copy of the runtime and publishes
// it only on complete success. The publish is a noexcept pointer move; every
// throwing step (copy, container growth, graph work) happens before it, on
// the staged copy, so a contained exception can never leave live state
// changed. A document that has never been mutated has no state object, and
// its first mutation stages from an empty one.
template <typename Mutation>
[[nodiscard]] DocumentResult RunStaged(std::unique_ptr<detail::State>& live,
                                       Mutation&& mutation) noexcept {
    try {
        std::unique_ptr<detail::State> staged =
            live ? std::make_unique<detail::State>(*live) : std::make_unique<detail::State>();
        const DocumentResult result = mutation(*staged);
        if (!result.ok()) {
            return result;
        }
        if (!InvariantsHold(*staged)) {
            return Fail(DocumentResultCode::InternalFailure);
        }
        live = std::move(staged);
        return result;
    } catch (...) {
        return Fail(DocumentResultCode::InternalFailure);
    }
}

} // namespace

const char* ToString(DocumentResultCode code) noexcept {
    switch (code) {
        case DocumentResultCode::Ok:
            return "Ok";
        case DocumentResultCode::InvalidElement:
            return "InvalidElement";
        case DocumentResultCode::DuplicateElementId:
            return "DuplicateElementId";
        case DocumentResultCode::LevelNotFound:
            return "LevelNotFound";
        case DocumentResultCode::ElementNotFound:
            return "ElementNotFound";
        case DocumentResultCode::ElementKindMismatch:
            return "ElementKindMismatch";
        case DocumentResultCode::RehostNotAllowed:
            return "RehostNotAllowed";
        case DocumentResultCode::NodeIdExhausted:
            return "NodeIdExhausted";
        case DocumentResultCode::GraphRejected:
            return "GraphRejected";
        case DocumentResultCode::DerivedGeometryInvalid:
            return "DerivedGeometryInvalid";
        case DocumentResultCode::RecomputeFailed:
            return "RecomputeFailed";
        case DocumentResultCode::InternalFailure:
            return "InternalFailure";
        case DocumentResultCode::ElementHasDependents:
            return "ElementHasDependents";
    }
    return "Unknown";
}

Document::Document() noexcept = default;
Document::~Document() = default;
Document::Document(Document&&) noexcept = default;
Document& Document::operator=(Document&&) noexcept = default;

DocumentResult Document::AddLevel(const Level& level) noexcept {
    return RunStaged(state_,
                     [&level](detail::State& staged) { return AddLevelStaged(staged, level); });
}

DocumentResult Document::AddStraightWall(const StraightWall& wall) noexcept {
    return RunStaged(
        state_, [&wall](detail::State& staged) { return AddStraightWallStaged(staged, wall); });
}

DocumentResult Document::UpdateLevelElevation(const ElementId& id, double elevation) noexcept {
    return RunStaged(state_, [&id, elevation](detail::State& staged) {
        return UpdateLevelElevationStaged(staged, id, elevation);
    });
}

DocumentResult Document::UpdateStraightWall(const StraightWall& wall) noexcept {
    return RunStaged(
        state_, [&wall](detail::State& staged) { return UpdateStraightWallStaged(staged, wall); });
}

DocumentResult Document::DeleteElement(const ElementId& id) noexcept {
    return RunStaged(state_,
                     [&id](detail::State& staged) { return DeleteElementStaged(staged, id); });
}

std::optional<Level> Document::FindLevel(const ElementId& id) const noexcept {
    if (!state_) {
        return std::nullopt;
    }
    const auto found = state_->levels.find(id);
    if (found == state_->levels.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::optional<StraightWall> Document::FindStraightWall(const ElementId& id) const noexcept {
    if (!state_) {
        return std::nullopt;
    }
    const auto found = state_->walls.find(id);
    if (found == state_->walls.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::optional<LinearExtrusionSpec>
Document::FindWallGeometry(const ElementId& wall_id) const noexcept {
    if (!state_) {
        return std::nullopt;
    }
    const auto found = state_->wall_geometry.find(wall_id);
    if (found == state_->wall_geometry.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::vector<Level> Document::ListLevels() const {
    std::vector<Level> levels;
    if (!state_) {
        return levels;
    }
    levels.reserve(state_->levels.size());
    // std::map<ElementId, ...> iterates in ElementId ascending order, which is
    // the order the public contract promises.
    for (const auto& entry : state_->levels) {
        levels.push_back(entry.second);
    }
    return levels;
}

std::vector<StraightWall> Document::ListStraightWalls() const {
    std::vector<StraightWall> walls;
    if (!state_) {
        return walls;
    }
    walls.reserve(state_->walls.size());
    for (const auto& entry : state_->walls) {
        walls.push_back(entry.second);
    }
    return walls;
}

} // namespace bim::document
