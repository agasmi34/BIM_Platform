#pragma once

// P0-T009 Dependency Graph Spike - public API for bim::dependency_graph.
//
// Frozen contracts (Architecture Gate BIM-AG-P0-T009 v1.0, Implementation
// Brief P0-T009-IB v1.0, Implementation Authorization BIM-AUTH-P0-T009 v1.0):
//   - NodeId is project-owned, opaque, caller supplied, equality comparable
//     and deterministically orderable; a default-constructed NodeId (value
//     0) is invalid.
//   - Dependency provenance is ExplicitSemantic or Computed; the same
//     (upstream, downstream) endpoint pair may carry both provenances while
//     evaluation traversal treats the pair as one logical dependency.
//   - Evaluation edge direction is upstream -> downstream; downstream
//     depends on upstream. Dirty propagation follows the same arrow, from
//     changed prerequisite to affected dependent.
//   - Committed topology must remain acyclic; self/direct/transitive cycles
//     are rejected before mutation with zero graph change.
//   - This module depends only on bim::foundation (module dependency
//     boundary bim_dependency_graph -> bim::foundation). It does not
//     include or depend on bim/model, geometry, transactions, persistence,
//     commands, query, interop, desktop or viewport headers, and it carries
//     no third-party dependency of its own.
//
// This header intentionally exposes only the eight required operations
// (AddNode, AddDependency, ReplaceComputedDependencies, InvalidateDependents,
// MarkDirty, BuildRecomputePlan, CommitRecompute, Snapshot) plus the value
// types needed to call them, to keep the public surface clean per the
// Architecture Gate's enforcement requirements. No production dependency
// beyond the C++ standard library is used, so the accepted
// bim_dependency_graph -> bim::foundation boundary is trivially respected
// without needing to guess at foundation's own API shape.

#include <compare>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <vector>

namespace bim::dependency_graph {

// Project-owned opaque node identity. Independent of memory address,
// insertion position, container order and vendor/kernel identity.
// A default-constructed NodeId (value 0) is invalid; valid NodeIds are
// caller supplied via the explicit constructor.
class NodeId {
public:
    constexpr NodeId() noexcept = default;
    constexpr explicit NodeId(std::uint64_t value) noexcept : value_(value) {}

    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool is_valid() const noexcept { return value_ != 0; }

    friend constexpr bool operator==(const NodeId&, const NodeId&) noexcept = default;
    friend constexpr auto operator<=>(const NodeId&, const NodeId&) noexcept = default;

private:
    std::uint64_t value_ = 0;
};

// The invalid NodeId, equal to a default-constructed NodeId.
inline constexpr NodeId kInvalidNodeId{};

enum class DependencyProvenance {
    ExplicitSemantic,
    Computed,
};

enum class NodeState {
    Clean,
    Dirty,
};

enum class EvaluationStatus {
    Success,
    Failed,
};

// One caller-supplied recompute outcome for a single planned node.
struct NodeEvaluationOutcome {
    NodeId node;
    EvaluationStatus status = EvaluationStatus::Success;
};

// Result codes for every mutating DependencyGraph operation.
enum class GraphResultCode {
    Ok,
    InvalidNodeId,
    NodeNotFound,
    NodeAlreadyExists,
    CycleDetected,
    NoChange,
    InvalidRecomputePlan,
    EvaluationFailed,
};

// Uniform result/diagnostic type for every mutating DependencyGraph
// operation. Fields not meaningful for a given code are left default
// constructed (invalid NodeId / empty path).
struct GraphResult {
    GraphResultCode code = GraphResultCode::Ok;
    NodeId primary_node{};
    NodeId upstream{};
    NodeId downstream{};
    std::vector<NodeId> cycle_path{};

    [[nodiscard]] bool ok() const noexcept { return code == GraphResultCode::Ok; }
};

// A deterministic, topologically ordered plan over the current Dirty-node
// subgraph, tied to the graph revision it was computed against.
struct RecomputePlan {
    std::uint64_t revision = 0;
    std::vector<NodeId> nodes{};
};

struct SnapshotNode {
    NodeId id;
    NodeState state = NodeState::Clean;
};

struct SnapshotEdge {
    NodeId upstream;
    NodeId downstream;
    DependencyProvenance provenance = DependencyProvenance::ExplicitSemantic;
};

// A deterministic point-in-time view of the committed graph. Node order is
// NodeId ascending; edge order is upstream ascending, then downstream
// ascending, then provenance (ExplicitSemantic before Computed).
struct GraphSnapshot {
    std::uint64_t revision = 0;
    std::vector<SnapshotNode> nodes{};
    std::vector<SnapshotEdge> edges{};
};

// In-memory, single-threaded, deterministic dependency DAG with provenance,
// dirty propagation and atomic recompute planning/commit. See the P0-T009
// Implementation Brief for the complete frozen semantics; this class
// implements exactly that contract and nothing beyond it.
class DependencyGraph {
public:
    DependencyGraph() = default;

    // Registers a new node in the Clean state. Fails closed (zero mutation)
    // on an invalid NodeId or an already-registered NodeId.
    [[nodiscard]] GraphResult AddNode(NodeId id);

    // Adds one (upstream, downstream, provenance) dependency edge. Fails
    // closed on an invalid or unregistered endpoint, or a cycle (including
    // the trivial self-dependency case). An exact duplicate of an existing
    // edge is a no-mutation NoChange. The same endpoint pair may separately
    // carry the other provenance.
    [[nodiscard]] GraphResult AddDependency(NodeId upstream, NodeId downstream,
                                             DependencyProvenance provenance);

    // Atomically replaces every Computed incoming edge of `downstream` with
    // exactly the (deduplicated) `new_upstreams` set, leaving every
    // ExplicitSemantic incoming edge of `downstream` untouched. Fails closed
    // (zero mutation) on an invalid/unregistered endpoint or a cycle; is
    // NoChange when the normalized set already matches the current Computed
    // set.
    [[nodiscard]] GraphResult ReplaceComputedDependencies(NodeId downstream,
                                                           std::vector<NodeId> new_upstreams);

    // Marks the strict downstream closure of `source` Dirty (not `source`
    // itself). Idempotent; NoChange if nothing was newly dirtied.
    [[nodiscard]] GraphResult InvalidateDependents(NodeId source);

    // Marks `node` and its complete downstream closure Dirty (inclusive of
    // `node`). Idempotent; NoChange if nothing was newly dirtied.
    [[nodiscard]] GraphResult MarkDirty(NodeId node);

    // Builds a deterministic topological plan over exactly the Dirty-node
    // subgraph (prerequisite before dependent, NodeId ascending as the
    // simultaneously-ready tie breaker). Independent of insertion order. A
    // graph with no Dirty nodes yields a valid empty plan. Does not mutate
    // graph state.
    [[nodiscard]] RecomputePlan BuildRecomputePlan() const;

    // Validates `plan` against the current revision and current recompute
    // plan membership, and validates `outcomes` (exactly one outcome per
    // planned node, none outside the plan, no duplicates). On any
    // validation failure: InvalidRecomputePlan, zero mutation. On the first
    // Failed outcome in plan order: EvaluationFailed, zero mutation, every
    // planned node remains Dirty, revision unchanged. On full success:
    // every planned node atomically becomes Clean and the revision
    // increments exactly once.
    [[nodiscard]] GraphResult CommitRecompute(const RecomputePlan& plan,
                                               const std::vector<NodeEvaluationOutcome>& outcomes);

    // A deterministic point-in-time view of the committed graph. Does not
    // mutate graph state.
    [[nodiscard]] GraphSnapshot Snapshot() const;

private:
    using EdgeTuple = std::tuple<NodeId, NodeId, DependencyProvenance>;

    [[nodiscard]] bool NodeExists(NodeId id) const;
    void AddEdgeTuple(NodeId upstream, NodeId downstream);
    void RemoveEdgeTuple(NodeId upstream, NodeId downstream, DependencyProvenance removed_provenance);
    [[nodiscard]] std::set<NodeId> ForwardReachableSet(NodeId start) const;
    [[nodiscard]] std::vector<NodeId> FindForwardPath(NodeId from, NodeId to) const;
    [[nodiscard]] std::set<NodeId> DownstreamClosure(NodeId start, bool inclusive) const;

    std::map<NodeId, NodeState> nodes_{};
    std::set<EdgeTuple> edges_{};
    std::map<NodeId, std::set<NodeId>> upstream_of_{};
    std::map<NodeId, std::set<NodeId>> downstream_of_{};
    std::uint64_t revision_ = 0;
};

} // namespace bim::dependency_graph
