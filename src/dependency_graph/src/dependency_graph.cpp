#include "bim/dependency_graph/dependency_graph.hpp"

#include <algorithm>
#include <cstddef>

// P0-T009 Dependency Graph Spike - implementation of bim::dependency_graph.
//
// See dependency_graph.hpp for the frozen public contract this file
// implements. Nothing here reaches beyond the C++ standard library, so the
// accepted bim_dependency_graph -> bim::foundation module boundary is
// respected by construction.

namespace bim::dependency_graph {

namespace {

[[nodiscard]] DependencyProvenance OtherProvenance(DependencyProvenance provenance) noexcept {
    return provenance == DependencyProvenance::Computed ? DependencyProvenance::ExplicitSemantic
                                                          : DependencyProvenance::Computed;
}

} // namespace

bool DependencyGraph::NodeExists(NodeId id) const { return nodes_.contains(id); }

void DependencyGraph::AddEdgeTuple(NodeId upstream, NodeId downstream) {
    // The logical (provenance-collapsed) adjacency only needs to record that
    // the pair is connected; inserting the pair again when a second
    // provenance edge is added for an already-connected pair is a no-op
    // against a std::set.
    downstream_of_[upstream].insert(downstream);
    upstream_of_[downstream].insert(upstream);
}

void DependencyGraph::RemoveEdgeTuple(NodeId upstream, NodeId downstream,
                                       DependencyProvenance removed_provenance) {
    // Only drop the logical adjacency entry once neither provenance edge for
    // this endpoint pair remains, so removing a Computed edge never erases a
    // surviving ExplicitSemantic relation (and vice versa).
    if (edges_.contains(EdgeTuple{upstream, downstream, OtherProvenance(removed_provenance)})) {
        return;
    }
    if (auto it = downstream_of_.find(upstream); it != downstream_of_.end()) {
        it->second.erase(downstream);
        if (it->second.empty()) {
            downstream_of_.erase(it);
        }
    }
    if (auto it = upstream_of_.find(downstream); it != upstream_of_.end()) {
        it->second.erase(upstream);
        if (it->second.empty()) {
            upstream_of_.erase(it);
        }
    }
}

std::set<NodeId> DependencyGraph::ForwardReachableSet(NodeId start) const {
    // Inclusive of `start` itself (zero-length reachability), which lets
    // callers treat a proposed self-relation as an ordinary reachability hit
    // rather than a separate special case.
    std::set<NodeId> visited{start};
    std::vector<NodeId> queue{start};
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const NodeId current = queue[head];
        const auto it = downstream_of_.find(current);
        if (it == downstream_of_.end()) {
            continue;
        }
        for (const NodeId next : it->second) { // std::set: ascending NodeId order
            if (visited.insert(next).second) {
                queue.push_back(next);
            }
        }
    }
    return visited;
}

std::vector<NodeId> DependencyGraph::FindForwardPath(NodeId from, NodeId to) const {
    if (from == to) {
        return {from};
    }
    std::map<NodeId, NodeId> predecessor;
    std::set<NodeId> visited{from};
    std::vector<NodeId> queue{from};
    bool found = false;
    for (std::size_t head = 0; head < queue.size() && !found; ++head) {
        const NodeId current = queue[head];
        const auto it = downstream_of_.find(current);
        if (it == downstream_of_.end()) {
            continue;
        }
        for (const NodeId next : it->second) { // ascending NodeId order: deterministic path
            if (visited.insert(next).second) {
                predecessor[next] = current;
                if (next == to) {
                    found = true;
                    break;
                }
                queue.push_back(next);
            }
        }
    }
    std::vector<NodeId> path{to};
    NodeId walker = to;
    while (walker != from) {
        walker = predecessor.at(walker);
        path.push_back(walker);
    }
    std::ranges::reverse(path);
    return path;
}

std::set<NodeId> DependencyGraph::DownstreamClosure(NodeId start, bool inclusive) const {
    std::set<NodeId> visited{start};
    std::vector<NodeId> queue{start};
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const NodeId current = queue[head];
        const auto it = downstream_of_.find(current);
        if (it == downstream_of_.end()) {
            continue;
        }
        for (const NodeId next : it->second) {
            if (visited.insert(next).second) {
                queue.push_back(next);
            }
        }
    }
    if (!inclusive) {
        visited.erase(start);
    }
    return visited;
}

GraphResult DependencyGraph::AddNode(NodeId id) {
    if (!id.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = id};
    }
    if (NodeExists(id)) {
        return GraphResult{.code = GraphResultCode::NodeAlreadyExists, .primary_node = id};
    }
    nodes_.emplace(id, NodeState::Clean);
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok, .primary_node = id};
}

GraphResult DependencyGraph::AddDependency(NodeId upstream, NodeId downstream,
                                            DependencyProvenance provenance) {
    if (!upstream.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = upstream};
    }
    if (!downstream.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = downstream};
    }
    if (!NodeExists(upstream)) {
        return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = upstream};
    }
    if (!NodeExists(downstream)) {
        return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = downstream};
    }
    if (upstream == downstream) {
        return GraphResult{.code = GraphResultCode::CycleDetected,
                            .primary_node = upstream,
                            .upstream = upstream,
                            .downstream = downstream,
                            .cycle_path = {upstream, upstream}};
    }
    if (const std::set<NodeId> reachable_from_downstream = ForwardReachableSet(downstream);
        reachable_from_downstream.contains(upstream)) {
        std::vector<NodeId> cycle_path{upstream};
        const std::vector<NodeId> closing_path = FindForwardPath(downstream, upstream);
        cycle_path.insert(cycle_path.end(), closing_path.begin(), closing_path.end());
        return GraphResult{.code = GraphResultCode::CycleDetected,
                            .primary_node = upstream,
                            .upstream = upstream,
                            .downstream = downstream,
                            .cycle_path = std::move(cycle_path)};
    }
    const EdgeTuple candidate{upstream, downstream, provenance};
    if (edges_.contains(candidate)) {
        return GraphResult{.code = GraphResultCode::NoChange,
                            .primary_node = downstream,
                            .upstream = upstream,
                            .downstream = downstream};
    }
    edges_.insert(candidate);
    AddEdgeTuple(upstream, downstream);
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok,
                        .primary_node = downstream,
                        .upstream = upstream,
                        .downstream = downstream};
}

GraphResult DependencyGraph::ReplaceComputedDependencies(NodeId downstream,
                                                          std::vector<NodeId> new_upstreams) {
    if (!downstream.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = downstream};
    }
    if (!NodeExists(downstream)) {
        return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = downstream};
    }
    for (const NodeId upstream : new_upstreams) {
        if (!upstream.is_valid()) {
            return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = upstream};
        }
    }
    for (const NodeId upstream : new_upstreams) {
        if (!NodeExists(upstream)) {
            return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = upstream};
        }
    }

    const std::set<NodeId> normalized(new_upstreams.begin(), new_upstreams.end());

    std::set<NodeId> current_computed;
    for (const auto& [u, d, p] : edges_) {
        if (d == downstream && p == DependencyProvenance::Computed) {
            current_computed.insert(u);
        }
    }

    if (normalized == current_computed) {
        return GraphResult{.code = GraphResultCode::NoChange, .primary_node = downstream, .downstream = downstream};
    }

    // downstream's outgoing edges are unaffected by replacing its incoming
    // edges, so its pre-mutation forward-reachable set already determines
    // whether any proposed upstream would close a cycle; no candidate-graph
    // reconstruction is needed.
    const std::set<NodeId> reachable_from_downstream = ForwardReachableSet(downstream);
    for (const NodeId upstream : normalized) { // ascending: smallest offending id reported first
        if (reachable_from_downstream.contains(upstream)) {
            std::vector<NodeId> cycle_path{upstream};
            const std::vector<NodeId> closing_path = FindForwardPath(downstream, upstream);
            cycle_path.insert(cycle_path.end(), closing_path.begin(), closing_path.end());
            return GraphResult{.code = GraphResultCode::CycleDetected,
                                .primary_node = upstream,
                                .upstream = upstream,
                                .downstream = downstream,
                                .cycle_path = std::move(cycle_path)};
        }
    }

    for (const NodeId old_upstream : current_computed) {
        edges_.erase(EdgeTuple{old_upstream, downstream, DependencyProvenance::Computed});
        RemoveEdgeTuple(old_upstream, downstream, DependencyProvenance::Computed);
    }
    for (const NodeId new_upstream : normalized) {
        edges_.insert(EdgeTuple{new_upstream, downstream, DependencyProvenance::Computed});
        AddEdgeTuple(new_upstream, downstream);
    }
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok, .primary_node = downstream, .downstream = downstream};
}

GraphResult DependencyGraph::InvalidateDependents(NodeId source) {
    if (!source.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = source};
    }
    if (!NodeExists(source)) {
        return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = source};
    }
    bool changed = false;
    for (const NodeId dependent : DownstreamClosure(source, /*inclusive=*/false)) {
        if (auto it = nodes_.find(dependent); it != nodes_.end() && it->second != NodeState::Dirty) {
            it->second = NodeState::Dirty;
            changed = true;
        }
    }
    if (!changed) {
        return GraphResult{.code = GraphResultCode::NoChange, .primary_node = source};
    }
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok, .primary_node = source};
}

GraphResult DependencyGraph::MarkDirty(NodeId node) {
    if (!node.is_valid()) {
        return GraphResult{.code = GraphResultCode::InvalidNodeId, .primary_node = node};
    }
    if (!NodeExists(node)) {
        return GraphResult{.code = GraphResultCode::NodeNotFound, .primary_node = node};
    }
    bool changed = false;
    for (const NodeId affected : DownstreamClosure(node, /*inclusive=*/true)) {
        if (auto it = nodes_.find(affected); it != nodes_.end() && it->second != NodeState::Dirty) {
            it->second = NodeState::Dirty;
            changed = true;
        }
    }
    if (!changed) {
        return GraphResult{.code = GraphResultCode::NoChange, .primary_node = node};
    }
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok, .primary_node = node};
}

RecomputePlan DependencyGraph::BuildRecomputePlan() const {
    std::set<NodeId> dirty;
    for (const auto& [id, state] : nodes_) {
        if (state == NodeState::Dirty) {
            dirty.insert(id);
        }
    }

    std::map<NodeId, std::size_t> in_degree;
    for (const NodeId d : dirty) {
        std::size_t count = 0;
        if (const auto it = upstream_of_.find(d); it != upstream_of_.end()) {
            for (const NodeId u : it->second) {
                if (dirty.contains(u)) {
                    ++count;
                }
            }
        }
        in_degree.emplace(d, count);
    }

    std::set<NodeId> ready;
    for (const auto& [id, degree] : in_degree) {
        if (degree == 0) {
            ready.insert(id);
        }
    }

    RecomputePlan plan;
    plan.revision = revision_;
    plan.nodes.reserve(dirty.size());

    while (!ready.empty()) {
        const NodeId next = *ready.begin();
        ready.erase(ready.begin());
        plan.nodes.push_back(next);

        if (const auto it = downstream_of_.find(next); it != downstream_of_.end()) {
            for (const NodeId dependent : it->second) {
                const auto degree_it = in_degree.find(dependent);
                if (degree_it == in_degree.end()) {
                    continue; // dependent is not Dirty: outside the planned subgraph.
                }
                if (--degree_it->second == 0) {
                    ready.insert(dependent);
                }
            }
        }
    }

    // The committed graph is acyclic by invariant, so every induced subgraph
    // is acyclic too: the loop above is guaranteed to drain every Dirty node
    // and no cycle-recovery path is needed here.
    return plan;
}

GraphResult DependencyGraph::CommitRecompute(const RecomputePlan& plan,
                                             const std::vector<NodeEvaluationOutcome>& outcomes) {
    if (plan.revision != revision_) {
        return GraphResult{.code = GraphResultCode::InvalidRecomputePlan};
    }
    const RecomputePlan fresh = BuildRecomputePlan();
    if (fresh.nodes != plan.nodes) {
        return GraphResult{.code = GraphResultCode::InvalidRecomputePlan};
    }
    if (outcomes.size() != plan.nodes.size()) {
        return GraphResult{.code = GraphResultCode::InvalidRecomputePlan};
    }

    const std::set<NodeId> planned(plan.nodes.begin(), plan.nodes.end());
    std::map<NodeId, EvaluationStatus> status_by_node;
    for (const NodeEvaluationOutcome& outcome : outcomes) {
        if (!planned.contains(outcome.node)) {
            return GraphResult{.code = GraphResultCode::InvalidRecomputePlan, .primary_node = outcome.node};
        }
        if (!status_by_node.emplace(outcome.node, outcome.status).second) {
            return GraphResult{.code = GraphResultCode::InvalidRecomputePlan, .primary_node = outcome.node};
        }
    }
    // outcomes.size() == plan.nodes.size() == planned.size() (plan.nodes has
    // no duplicates) and status_by_node now holds exactly planned.size()
    // distinct planned nodes, so every planned node has exactly one outcome
    // and no outcome lies outside the plan.

    if (plan.nodes.empty()) {
        return GraphResult{.code = GraphResultCode::NoChange};
    }

    for (const NodeId node : plan.nodes) { // first failure in plan order
        if (status_by_node.at(node) == EvaluationStatus::Failed) {
            return GraphResult{.code = GraphResultCode::EvaluationFailed, .primary_node = node};
        }
    }

    for (const NodeId node : plan.nodes) {
        nodes_.at(node) = NodeState::Clean;
    }
    ++revision_;
    return GraphResult{.code = GraphResultCode::Ok};
}

GraphSnapshot DependencyGraph::Snapshot() const {
    GraphSnapshot snapshot;
    snapshot.revision = revision_;
    snapshot.nodes.reserve(nodes_.size());
    for (const auto& [id, state] : nodes_) { // std::map: NodeId ascending
        snapshot.nodes.push_back(SnapshotNode{.id = id, .state = state});
    }
    snapshot.edges.reserve(edges_.size());
    for (const auto& [upstream, downstream, provenance] : edges_) {
        // std::set<EdgeTuple>: upstream ascending, then downstream ascending,
        // then provenance in enum declaration order (ExplicitSemantic before
        // Computed), which matches the frozen canonical snapshot order.
        snapshot.edges.push_back(
            SnapshotEdge{.upstream = upstream, .downstream = downstream, .provenance = provenance});
    }
    return snapshot;
}

} // namespace bim::dependency_graph
