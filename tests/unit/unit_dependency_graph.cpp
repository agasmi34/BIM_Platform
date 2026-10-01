// P0-T009 Dependency Graph Spike - unit-level coverage of bim::dependency_graph
// (Architecture Gate BIM-AG-P0-T009 v1.0; Implementation Brief P0-T009-IB
// v1.0; Implementation Authorization BIM-AUTH-P0-T009 v1.0).
//
// This file exercises DependencyGraph's public operations one at a time:
// NodeId identity/ordering, and the per-operation validation and mutation
// rules of AddNode, AddDependency, ReplaceComputedDependencies,
// InvalidateDependents, MarkDirty, BuildRecomputePlan, CommitRecompute and
// Snapshot, including the Gate section 19 "also prove" edge cases (invalid
// NodeId, missing endpoints, duplicate node, duplicate edge). The named
// mandatory proof corpus (A-H) and its whole-graph scenario properties
// (repeated invalidation/planning, empty dirty set, disconnected graph,
// deterministic diagnostics) live in
// tests/integration/integration_p0_t009_dependency_graph_spike.cpp.
//
// bim::dependency_graph is a pure, kernel-free, foundation-only module
// (Architecture Gate section 17), so - unlike P0-T008's split between a
// kernel-free unit level and a real-kernel integration level - the split
// here is purely organizational: fine-grained per-operation validation in
// this file, whole-graph named-proof scenarios in the integration file.

#include "bim/dependency_graph/dependency_graph.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace bim::dependency_graph;

namespace {

constexpr NodeId kA{1};
constexpr NodeId kB{2};
constexpr NodeId kC{3};
constexpr NodeId kD{4};

} // namespace

TEST_CASE("NodeId: default construction is invalid, explicit non-zero construction is valid",
          "[unit][dependency-graph][p0-t009][node-id]") {
    CHECK_FALSE(NodeId{}.is_valid());
    CHECK_FALSE(kInvalidNodeId.is_valid());
    CHECK(NodeId{}.value() == 0);
    CHECK(NodeId{42}.is_valid());
    CHECK(NodeId{42}.value() == 42);
}

TEST_CASE("NodeId: equality and total ordering are independent of construction order",
          "[unit][dependency-graph][p0-t009][node-id]") {
    CHECK(NodeId{7} == NodeId{7});
    CHECK(NodeId{1} != NodeId{2});
    CHECK(NodeId{1} < NodeId{2});
    CHECK(NodeId{2} > NodeId{1});
    CHECK((NodeId{1} <=> NodeId{2}) == std::strong_ordering::less);
}

TEST_CASE("AddNode: a valid new node succeeds and is Clean", "[unit][dependency-graph][p0-t009][add-node]") {
    DependencyGraph g;
    const GraphResult result = g.AddNode(kA);
    REQUIRE(result.ok());
    CHECK(result.primary_node == kA);
    const GraphSnapshot snap = g.Snapshot();
    REQUIRE(snap.nodes.size() == 1);
    CHECK(snap.nodes.front().id == kA);
    CHECK(snap.nodes.front().state == NodeState::Clean);
}

TEST_CASE("AddNode: NodeId{0} is InvalidNodeId, zero mutation", "[unit][dependency-graph][p0-t009][add-node]") {
    DependencyGraph g;
    const GraphResult result = g.AddNode(kInvalidNodeId);
    CHECK(result.code == GraphResultCode::InvalidNodeId);
    CHECK(g.Snapshot().nodes.empty());
}

TEST_CASE("AddNode: duplicate registration is NodeAlreadyExists, zero mutation",
          "[unit][dependency-graph][p0-t009][add-node]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    const GraphResult result = g.AddNode(kA);
    CHECK(result.code == GraphResultCode::NodeAlreadyExists);
    CHECK(result.primary_node == kA);
    CHECK(g.Snapshot().nodes.size() == 1);
}

TEST_CASE("AddDependency: invalid endpoint ids are rejected before existence is checked",
          "[unit][dependency-graph][p0-t009][add-dependency]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());

    const GraphResult bad_upstream = g.AddDependency(kInvalidNodeId, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(bad_upstream.code == GraphResultCode::InvalidNodeId);
    CHECK(bad_upstream.primary_node == kInvalidNodeId);

    const GraphResult bad_downstream = g.AddDependency(kA, kInvalidNodeId, DependencyProvenance::ExplicitSemantic);
    CHECK(bad_downstream.code == GraphResultCode::InvalidNodeId);
    CHECK(bad_downstream.primary_node == kInvalidNodeId);
}

TEST_CASE("AddDependency: missing endpoints are never implicitly created",
          "[unit][dependency-graph][p0-t009][add-dependency]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());

    const GraphResult missing_upstream = g.AddDependency(kB, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(missing_upstream.code == GraphResultCode::NodeNotFound);
    CHECK(missing_upstream.primary_node == kB);
    CHECK(g.Snapshot().nodes.size() == 1); // kB was never created

    const GraphResult missing_downstream = g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic);
    CHECK(missing_downstream.code == GraphResultCode::NodeNotFound);
    CHECK(missing_downstream.primary_node == kB);
    CHECK(g.Snapshot().nodes.size() == 1);
}

TEST_CASE("AddDependency: a self-dependency is a trivial rejected cycle",
          "[unit][dependency-graph][p0-t009][add-dependency][cycle]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    const GraphResult result = g.AddDependency(kA, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(result.code == GraphResultCode::CycleDetected);
    CHECK(result.cycle_path == std::vector<NodeId>{kA, kA});
    CHECK(g.Snapshot().edges.empty());
}

TEST_CASE("AddDependency: a direct 2-cycle is rejected with a deterministic path",
          "[unit][dependency-graph][p0-t009][add-dependency][cycle]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());

    const GraphSnapshot before = g.Snapshot();
    const GraphResult result = g.AddDependency(kB, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(result.code == GraphResultCode::CycleDetected);
    CHECK(result.cycle_path == std::vector<NodeId>{kB, kA, kB});
    const GraphSnapshot after = g.Snapshot();
    CHECK(before.edges.size() == after.edges.size());
    CHECK(before.revision == after.revision);
}

TEST_CASE("AddDependency: a transitive 3-cycle is rejected with a deterministic path",
          "[unit][dependency-graph][p0-t009][add-dependency][cycle]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());

    const GraphResult result = g.AddDependency(kC, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(result.code == GraphResultCode::CycleDetected);
    CHECK(result.cycle_path == std::vector<NodeId>{kC, kA, kB, kC});
    CHECK(g.Snapshot().edges.size() == 2); // prior topology exactly preserved
}

TEST_CASE("AddDependency: an exact duplicate edge is idempotent NoChange",
          "[unit][dependency-graph][p0-t009][add-dependency]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    const std::uint64_t revision_before = g.BuildRecomputePlan().revision;

    const GraphResult result = g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic);
    CHECK(result.code == GraphResultCode::NoChange);
    CHECK(g.Snapshot().edges.size() == 1);
    CHECK(g.BuildRecomputePlan().revision == revision_before);
}

TEST_CASE("AddDependency: the same endpoint pair may separately carry the other provenance",
          "[unit][dependency-graph][p0-t009][add-dependency][provenance]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());

    const GraphResult result = g.AddDependency(kA, kB, DependencyProvenance::Computed);
    CHECK(result.ok());
    CHECK(g.Snapshot().edges.size() == 2);
}

TEST_CASE("ReplaceComputedDependencies: invalid/missing downstream and proposed upstreams are rejected",
          "[unit][dependency-graph][p0-t009][replace-computed]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());

    CHECK(g.ReplaceComputedDependencies(kInvalidNodeId, {}).code == GraphResultCode::InvalidNodeId);
    CHECK(g.ReplaceComputedDependencies(kB, {}).code == GraphResultCode::NodeNotFound);
    CHECK(g.ReplaceComputedDependencies(kA, {kInvalidNodeId}).code == GraphResultCode::InvalidNodeId);
    CHECK(g.ReplaceComputedDependencies(kA, {kB}).code == GraphResultCode::NodeNotFound);
}

TEST_CASE("ReplaceComputedDependencies: a normalized set equal to the current Computed set is NoChange",
          "[unit][dependency-graph][p0-t009][replace-computed]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.ReplaceComputedDependencies(kC, {kA, kB}).ok());
    const std::uint64_t revision_before = g.BuildRecomputePlan().revision;

    // Reordered and containing a duplicate: normalizes to the same {A, B}.
    const GraphResult result = g.ReplaceComputedDependencies(kC, {kB, kA, kA});
    CHECK(result.code == GraphResultCode::NoChange);
    CHECK(g.BuildRecomputePlan().revision == revision_before);
}

TEST_CASE("ReplaceComputedDependencies: a proposal that would close a cycle is rejected with zero mutation",
          "[unit][dependency-graph][p0-t009][replace-computed][cycle]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());

    const GraphSnapshot before = g.Snapshot();
    const GraphResult result = g.ReplaceComputedDependencies(kA, {kC}); // would close A->B->C->A
    CHECK(result.code == GraphResultCode::CycleDetected);
    const GraphSnapshot after = g.Snapshot();
    CHECK(before.edges.size() == after.edges.size());
    CHECK(before.revision == after.revision);
}

TEST_CASE("InvalidateDependents: dirties the strict downstream closure only, is idempotent",
          "[unit][dependency-graph][p0-t009][dirty]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());

    REQUIRE(g.InvalidateDependents(kA).ok());
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        if (n.id == kA) {
            CHECK(n.state == NodeState::Clean); // source itself is not dirtied
        } else {
            CHECK(n.state == NodeState::Dirty);
        }
    }
    CHECK(g.InvalidateDependents(kA).code == GraphResultCode::NoChange);
}

TEST_CASE("MarkDirty: dirties the node and its complete downstream closure, is idempotent",
          "[unit][dependency-graph][p0-t009][dirty]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());

    REQUIRE(g.MarkDirty(kA).ok());
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        CHECK(n.state == NodeState::Dirty); // inclusive of A itself
    }
    CHECK(g.MarkDirty(kA).code == GraphResultCode::NoChange);
}

TEST_CASE("InvalidateDependents/MarkDirty: invalid and unregistered source ids are rejected",
          "[unit][dependency-graph][p0-t009][dirty]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    CHECK(g.InvalidateDependents(kInvalidNodeId).code == GraphResultCode::InvalidNodeId);
    CHECK(g.InvalidateDependents(kB).code == GraphResultCode::NodeNotFound);
    CHECK(g.MarkDirty(kInvalidNodeId).code == GraphResultCode::InvalidNodeId);
    CHECK(g.MarkDirty(kB).code == GraphResultCode::NodeNotFound);
}

TEST_CASE("BuildRecomputePlan: a graph with no Dirty nodes yields a valid empty plan",
          "[unit][dependency-graph][p0-t009][recompute-plan]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes.empty());
    CHECK(plan.revision == g.Snapshot().revision);
}

TEST_CASE("BuildRecomputePlan: NodeId ascending order breaks simultaneous-ready ties",
          "[unit][dependency-graph][p0-t009][recompute-plan]") {
    DependencyGraph g;
    for (NodeId n : {kD, kC, kB, kA}) REQUIRE(g.AddNode(n).ok()); // insertion order reversed vs value order
    REQUIRE(g.MarkDirty(kA).ok());
    REQUIRE(g.MarkDirty(kB).ok());
    REQUIRE(g.MarkDirty(kC).ok());
    REQUIRE(g.MarkDirty(kD).ok());
    // No dependencies among A, B, C, D: all four are simultaneously ready.
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes == std::vector<NodeId>{kA, kB, kC, kD});
}

TEST_CASE("CommitRecompute: an empty plan with empty outcomes is NoChange",
          "[unit][dependency-graph][p0-t009][commit-recompute]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes.empty());
    CHECK(g.CommitRecompute(plan, {}).code == GraphResultCode::NoChange);
}

TEST_CASE("CommitRecompute: a stale plan (revision no longer matches) is InvalidRecomputePlan",
          "[unit][dependency-graph][p0-t009][commit-recompute]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes == std::vector<NodeId>{kB});

    REQUIRE(g.AddNode(kC).ok()); // advances the revision without affecting B's dirtiness
    const GraphResult result = g.CommitRecompute(plan, {{.node = kB, .status = EvaluationStatus::Success}});
    CHECK(result.code == GraphResultCode::InvalidRecomputePlan);
    // B must remain Dirty: the stale attempt must not have mutated state.
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        if (n.id == kB) CHECK(n.state == NodeState::Dirty);
    }
}

TEST_CASE("CommitRecompute: an outcome naming a node outside the plan is InvalidRecomputePlan",
          "[unit][dependency-graph][p0-t009][commit-recompute]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes == std::vector<NodeId>{kB});

    const GraphResult result = g.CommitRecompute(
        plan, {{.node = kB, .status = EvaluationStatus::Success}, {.node = kA, .status = EvaluationStatus::Success}});
    CHECK(result.code == GraphResultCode::InvalidRecomputePlan);
}

TEST_CASE("CommitRecompute: a missing or duplicated outcome for a planned node is InvalidRecomputePlan",
          "[unit][dependency-graph][p0-t009][commit-recompute]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes == std::vector<NodeId>{kB});

    CHECK(g.CommitRecompute(plan, {}).code == GraphResultCode::InvalidRecomputePlan); // missing
    CHECK(g.CommitRecompute(plan, {{.node = kB, .status = EvaluationStatus::Success},
                                    {.node = kB, .status = EvaluationStatus::Success}})
              .code == GraphResultCode::InvalidRecomputePlan); // duplicate
}

TEST_CASE("CommitRecompute: full success atomically cleans every planned node and advances the revision by one",
          "[unit][dependency-graph][p0-t009][commit-recompute]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes == std::vector<NodeId>{kB, kC});

    const GraphResult result = g.CommitRecompute(
        plan, {{.node = kB, .status = EvaluationStatus::Success}, {.node = kC, .status = EvaluationStatus::Success}});
    CHECK(result.ok());
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        CHECK(n.state == NodeState::Clean);
    }
    CHECK(g.Snapshot().revision == plan.revision + 1);
}

TEST_CASE("Snapshot: node order is NodeId ascending; edge order is upstream, then downstream, then provenance",
          "[unit][dependency-graph][p0-t009][snapshot]") {
    DependencyGraph g;
    for (NodeId n : {kC, kA, kB}) REQUIRE(g.AddNode(n).ok()); // insertion order C, A, B
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kA, kC, DependencyProvenance::Computed).ok());
    REQUIRE(g.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());

    const GraphSnapshot snap = g.Snapshot();
    REQUIRE(snap.nodes.size() == 3);
    CHECK(snap.nodes[0].id == kA);
    CHECK(snap.nodes[1].id == kB);
    CHECK(snap.nodes[2].id == kC);

    REQUIRE(snap.edges.size() == 4);
    CHECK(snap.edges[0].upstream == kA);
    CHECK(snap.edges[0].downstream == kB);
    CHECK(snap.edges[0].provenance == DependencyProvenance::ExplicitSemantic);
    CHECK(snap.edges[1].upstream == kA);
    CHECK(snap.edges[1].downstream == kC);
    CHECK(snap.edges[1].provenance == DependencyProvenance::ExplicitSemantic);
    CHECK(snap.edges[2].upstream == kA);
    CHECK(snap.edges[2].downstream == kC);
    CHECK(snap.edges[2].provenance == DependencyProvenance::Computed);
    CHECK(snap.edges[3].upstream == kB);
    CHECK(snap.edges[3].downstream == kC);
    CHECK(snap.edges[3].provenance == DependencyProvenance::ExplicitSemantic);
}
