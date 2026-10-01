// P0-T009 Dependency Graph Spike - the named mandatory proof corpus
// (Architecture Gate BIM-AG-P0-T009 v1.0 section 19 / Implementation
// Authorization BIM-AUTH-P0-T009 v1.0 section 14: proofs A-H) plus the
// accompanying whole-graph scenario properties from Gate section 19's
// "also prove" list (repeated invalidation/planning, empty dirty set,
// disconnected graph, deterministic diagnostics). Per-operation validation
// edge cases (invalid NodeId, missing endpoints, duplicate node, duplicate
// edge) are covered at finer grain in
// tests/unit/unit_dependency_graph.cpp; this file is the spike-level
// end-to-end demonstration that mirrors, in role, how P0-T008's
// integration file hosted that spike's own proof corpus - the difference
// being that bim::dependency_graph is a pure, kernel-free, foundation-only
// module (Architecture Gate section 17), so there is no separate "real
// kernel" dimension to add here: every proof below runs the real (not
// reconstructed or hand-simulated) DependencyGraph implementation.

#include "bim/dependency_graph/dependency_graph.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace bim::dependency_graph;

namespace {

constexpr NodeId kA{1};
constexpr NodeId kB{2};
constexpr NodeId kC{3};
constexpr NodeId kD{4};
constexpr NodeId kE{5};
constexpr NodeId kF{6};

} // namespace

// Proof A - simple chain propagation and B then C planning.
TEST_CASE("Proof A: a simple chain A->B->C propagates and plans as [B, C]",
          "[integration][dependency-graph][p0-t009][proof-a]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());

    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes == std::vector<NodeId>{kB, kC});
}

// Proof B - deterministic diamond planning and single D occurrence.
TEST_CASE("Proof B: a diamond A->{B,C}->D plans deterministically with D exactly once",
          "[integration][dependency-graph][p0-t009][proof-b]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC, kD}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kD, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kC, kD, DependencyProvenance::ExplicitSemantic).ok());

    REQUIRE(g.MarkDirty(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes == std::vector<NodeId>{kA, kB, kC, kD});
    CHECK(std::count(plan.nodes.begin(), plan.nodes.end(), kD) == 1);
}

// Proof C - unrelated branch isolation.
TEST_CASE("Proof C: dirtying one branch never plans an unrelated, disconnected branch",
          "[integration][dependency-graph][p0-t009][proof-c]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC, kD}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok()); // branch 1
    REQUIRE(g.AddDependency(kC, kD, DependencyProvenance::ExplicitSemantic).ok()); // branch 2, unrelated

    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes == std::vector<NodeId>{kB});
}

// Proof D - self/direct/transitive cycle rejection with zero mutation.
TEST_CASE("Proof D: self, direct and transitive cycles are all rejected with zero mutation",
          "[integration][dependency-graph][p0-t009][proof-d]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());

    const GraphResult self_cycle = g.AddDependency(kA, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(self_cycle.code == GraphResultCode::CycleDetected);
    CHECK(g.Snapshot().edges.empty());

    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    const std::uint64_t revision_after_ab = g.Snapshot().revision;

    const GraphResult direct_cycle = g.AddDependency(kB, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(direct_cycle.code == GraphResultCode::CycleDetected);
    CHECK(g.Snapshot().revision == revision_after_ab);
    CHECK(g.Snapshot().edges.size() == 1);

    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());
    const std::uint64_t revision_after_bc = g.Snapshot().revision;

    const GraphResult transitive_cycle = g.AddDependency(kC, kA, DependencyProvenance::ExplicitSemantic);
    CHECK(transitive_cycle.code == GraphResultCode::CycleDetected);
    CHECK(g.Snapshot().revision == revision_after_bc);
    CHECK(g.Snapshot().edges.size() == 2);
}

// Proof E - insertion-order independence.
TEST_CASE("Proof E: planning is independent of node and edge insertion order",
          "[integration][dependency-graph][p0-t009][proof-e]") {
    DependencyGraph forward;
    for (NodeId n : {kA, kB, kC, kD}) REQUIRE(forward.AddNode(n).ok());
    REQUIRE(forward.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(forward.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(forward.AddDependency(kB, kD, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(forward.AddDependency(kC, kD, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(forward.MarkDirty(kA).ok());

    DependencyGraph reversed;
    for (NodeId n : {kD, kC, kB, kA}) REQUIRE(reversed.AddNode(n).ok()); // reversed node insertion order
    REQUIRE(reversed.AddDependency(kC, kD, DependencyProvenance::ExplicitSemantic).ok()); // reversed edge order
    REQUIRE(reversed.AddDependency(kB, kD, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(reversed.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(reversed.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(reversed.MarkDirty(kA).ok());

    CHECK(forward.BuildRecomputePlan().nodes == reversed.BuildRecomputePlan().nodes);
}

// Proof F - atomic Computed dependency replacement.
TEST_CASE("Proof F: ReplaceComputedDependencies atomically replaces the Computed upstream set",
          "[integration][dependency-graph][p0-t009][proof-f]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC, kD}) REQUIRE(g.AddNode(n).ok());

    REQUIRE(g.ReplaceComputedDependencies(kD, {kA, kB}).ok());
    CHECK(g.Snapshot().edges.size() == 2);

    REQUIRE(g.ReplaceComputedDependencies(kD, {kB, kC}).ok());
    const GraphSnapshot snap = g.Snapshot();
    CHECK(snap.edges.size() == 2);
    bool has_a = false, has_b = false, has_c = false;
    for (const SnapshotEdge& e : snap.edges) {
        if (e.downstream != kD) continue;
        if (e.upstream == kA) has_a = true;
        if (e.upstream == kB) has_b = true;
        if (e.upstream == kC) has_c = true;
    }
    CHECK_FALSE(has_a); // fully removed
    CHECK(has_b);
    CHECK(has_c);
}

// Proof G - provenance preservation across Computed refresh.
TEST_CASE("Proof G: an ExplicitSemantic dependency survives a Computed refresh of the same pair",
          "[integration][dependency-graph][p0-t009][proof-g]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());

    REQUIRE(g.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.ReplaceComputedDependencies(kC, {kA, kB}).ok());
    CHECK(g.Snapshot().edges.size() == 3); // (A,C,Explicit) (A,C,Computed) (B,C,Computed)

    REQUIRE(g.ReplaceComputedDependencies(kC, {kB}).ok()); // drop A from the Computed set entirely
    const GraphSnapshot snap = g.Snapshot();
    bool explicit_a_survives = false;
    bool computed_a_survives = false;
    for (const SnapshotEdge& e : snap.edges) {
        if (e.upstream == kA && e.downstream == kC && e.provenance == DependencyProvenance::ExplicitSemantic)
            explicit_a_survives = true;
        if (e.upstream == kA && e.downstream == kC && e.provenance == DependencyProvenance::Computed)
            computed_a_survives = true;
    }
    CHECK(explicit_a_survives);
    CHECK_FALSE(computed_a_survives);

    // The logical A->C relation still exists via ExplicitSemantic, so C's
    // dirty propagation must still reach downstream of A through it.
    REQUIRE(g.AddNode(kD).ok());
    REQUIRE(g.AddDependency(kC, kD, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());
    bool c_dirty = false, d_dirty = false;
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        if (n.id == kC) c_dirty = (n.state == NodeState::Dirty);
        if (n.id == kD) d_dirty = (n.state == NodeState::Dirty);
    }
    CHECK(c_dirty);
    CHECK(d_dirty);
}

// Proof H - failed recompute has no partial Clean commit; retry succeeds.
TEST_CASE("Proof H: a Failed outcome leaves every planned node Dirty with no partial Clean commit, and retry succeeds",
          "[integration][dependency-graph][p0-t009][proof-h]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.AddDependency(kB, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g.InvalidateDependents(kA).ok());

    const RecomputePlan plan = g.BuildRecomputePlan();
    REQUIRE(plan.nodes == std::vector<NodeId>{kB, kC});

    const GraphResult failed = g.CommitRecompute(
        plan, {{.node = kB, .status = EvaluationStatus::Success}, {.node = kC, .status = EvaluationStatus::Failed}});
    CHECK(failed.code == GraphResultCode::EvaluationFailed);
    CHECK(failed.primary_node == kC); // deterministic first-failure reporting in plan order

    const GraphSnapshot after_failure = g.Snapshot();
    for (const SnapshotNode& n : after_failure.nodes) {
        if (n.id == kB || n.id == kC) CHECK(n.state == NodeState::Dirty);
    }
    CHECK(after_failure.revision == plan.revision); // unchanged: no partial Clean commit

    const GraphResult retried = g.CommitRecompute(
        plan, {{.node = kB, .status = EvaluationStatus::Success}, {.node = kC, .status = EvaluationStatus::Success}});
    CHECK(retried.ok());
    for (const SnapshotNode& n : g.Snapshot().nodes) {
        CHECK(n.state == NodeState::Clean);
    }
    CHECK(g.Snapshot().revision == plan.revision + 1);
}

// Gate section 19 "also prove": repeated invalidation/planning, empty dirty
// set, disconnected graph, deterministic diagnostics.

TEST_CASE("Whole-graph: an empty dirty set plans empty and commits as NoChange",
          "[integration][dependency-graph][p0-t009][empty-dirty-set]") {
    DependencyGraph g;
    REQUIRE(g.AddNode(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes.empty());
    CHECK(g.CommitRecompute(plan, {}).code == GraphResultCode::NoChange);
}

TEST_CASE("Whole-graph: unrelated disconnected nodes never appear in any plan",
          "[integration][dependency-graph][p0-t009][disconnected-graph]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB, kC, kD, kE, kF}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());
    // C, D, E, F remain disconnected from A/B and from each other.

    REQUIRE(g.InvalidateDependents(kA).ok());
    const RecomputePlan plan = g.BuildRecomputePlan();
    CHECK(plan.nodes == std::vector<NodeId>{kB});
}

TEST_CASE("Whole-graph: invalidation and planning can be repeated across successive clean cycles",
          "[integration][dependency-graph][p0-t009][repeated-planning]") {
    DependencyGraph g;
    for (NodeId n : {kA, kB}) REQUIRE(g.AddNode(n).ok());
    REQUIRE(g.AddDependency(kA, kB, DependencyProvenance::ExplicitSemantic).ok());

    for (int cycle = 0; cycle < 3; ++cycle) {
        REQUIRE(g.InvalidateDependents(kA).ok());
        const RecomputePlan plan = g.BuildRecomputePlan();
        CHECK(plan.nodes == std::vector<NodeId>{kB});
        CHECK(g.CommitRecompute(plan, {{.node = kB, .status = EvaluationStatus::Success}}).ok());
        for (const SnapshotNode& n : g.Snapshot().nodes) {
            CHECK(n.state == NodeState::Clean);
        }
    }
}

TEST_CASE("Whole-graph: diagnostics are deterministic across two independently built, logically equal graphs",
          "[integration][dependency-graph][p0-t009][deterministic-diagnostics]") {
    DependencyGraph g1;
    for (NodeId n : {kA, kB, kC}) REQUIRE(g1.AddNode(n).ok());
    REQUIRE(g1.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());
    REQUIRE(g1.AddDependency(kB, kC, DependencyProvenance::Computed).ok());

    DependencyGraph g2;
    for (NodeId n : {kC, kB, kA}) REQUIRE(g2.AddNode(n).ok()); // different insertion order
    REQUIRE(g2.AddDependency(kB, kC, DependencyProvenance::Computed).ok()); // different edge order
    REQUIRE(g2.AddDependency(kA, kC, DependencyProvenance::ExplicitSemantic).ok());

    const GraphSnapshot snap1 = g1.Snapshot();
    const GraphSnapshot snap2 = g2.Snapshot();
    REQUIRE(snap1.nodes.size() == snap2.nodes.size());
    REQUIRE(snap1.edges.size() == snap2.edges.size());
    for (std::size_t i = 0; i < snap1.nodes.size(); ++i) {
        CHECK(snap1.nodes[i].id == snap2.nodes[i].id);
    }
    for (std::size_t i = 0; i < snap1.edges.size(); ++i) {
        CHECK(snap1.edges[i].upstream == snap2.edges[i].upstream);
        CHECK(snap1.edges[i].downstream == snap2.edges[i].downstream);
        CHECK(snap1.edges[i].provenance == snap2.edges[i].provenance);
    }

    // Same-shape cycle rejection also reports the same deterministic path.
    const GraphResult cycle1 = g1.AddDependency(kC, kA, DependencyProvenance::ExplicitSemantic);
    const GraphResult cycle2 = g2.AddDependency(kC, kA, DependencyProvenance::ExplicitSemantic);
    REQUIRE(cycle1.code == GraphResultCode::CycleDetected);
    REQUIRE(cycle2.code == GraphResultCode::CycleDetected);
    CHECK(cycle1.cycle_path == cycle2.cycle_path);
}
