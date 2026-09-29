# BIM Platform - P0-T009 Dependency Graph Spike Implementation Brief

**Brief:** `P0-T009-IB v1.0`
**Status:** FROZEN / APPROVED
**Date:** 2026-09-29
**Task:** P0-T009 - Dependency Graph Spike
**Task branch:** `task/P0-T009-dependency-graph-spike`
**Architecture Gate:** `BIM-AG-P0-T009 v1.0`
**Architecture Gate status:** FROZEN / APPROVED
**Governance baseline HEAD:** `623d1b1f220d1341a83fe67fa0e67f0a353e5623`
**Governance baseline TREE:** `fc956308d638263b766b80eeb193f1fcf3abe193`
**Product Authority approval:** APPROVED
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Authorization:** NOT ISSUED
**Implementation Engineer:** Claude - NOT AUTHORIZED TO EXECUTE
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## 1. Authority

This document is the authoritative implementation specification for
P0-T009 once committed as governance.

Its approval does not itself authorize implementation execution.

Claude may execute only after a separate committed
Implementation Authorization and explicit Architecture Authority
activation.

Kimi remains unauthorized until the later independent-review gate.

## 2. Objective

Implement the minimum project-owned dependency-graph primitive required
to prove the frozen P0-T009 Architecture Gate.

The implementation shall prove:

- explicit semantic dependencies;
- computed dependency tracking and refresh;
- downstream invalidation;
- deterministic topological recompute planning;
- cycle prevention before mutation;
- deterministic diagnostics;
- atomic graph-state recompute commit behavior.

This remains a Phase-0 spike and is not a production BIM feature graph.

## 3. Frozen production dependency boundary

The production dependency direction remains:

```text
bim_dependency_graph -> bim::foundation
```

No production dependency is authorized from dependency_graph to:

- model;
- transactions;
- commands;
- query;
- persistence;
- geometry_api;
- geometry_occt;
- OCCT;
- Qt;
- SQLite;
- ODA;
- IfcOpenShell;
- bgfx;
- interop modules;
- desktop/UI.

No new third-party dependency is authorized.

## 4. Public header

The first real public dependency-graph contract shall be:

```text
src/dependency_graph/include/bim/dependency_graph/dependency_graph.hpp
```

The public header shall expose project-owned and standard-library types
only.

## 5. NodeId contract

The public contract shall define the conceptual equivalent of:

```cpp
struct NodeId {
    std::uint64_t value = 0;

    friend constexpr bool operator==(
        const NodeId&, const NodeId&) noexcept = default;

    friend constexpr auto operator<=>(
        const NodeId&, const NodeId&) noexcept = default;
};
```

`NodeId{0}` is invalid.

A valid NodeId is:

- caller supplied;
- project owned;
- opaque to the graph algorithm;
- equality comparable;
- deterministically orderable;
- independent of memory address;
- independent of insertion position;
- independent of container iteration order;
- independent of vendor/kernel identity.

P0-T009 does not establish final platform BIM UUID architecture.

NodeId is not FeatureOwnerId.

NodeId is not PersistentFaceReference.

## 6. Provenance contract

The public contract shall define:

```cpp
enum class DependencyProvenance : std::uint8_t {
    ExplicitSemantic,
    Computed,
};
```

A stored dependency identity is conceptually:

```text
(upstream, downstream, provenance)
```

Therefore the same endpoint pair may simultaneously carry:

```text
A -> B / ExplicitSemantic
A -> B / Computed
```

Evaluation traversal treats that endpoint pair as one logical
upstream/downstream relation so dirty propagation and planning never
duplicate work.

Refreshing Computed provenance must never remove an ExplicitSemantic
dependency for the same pair.

No independent persistent EdgeId is introduced.

## 7. Node-state contract

The committed graph state shall define:

```cpp
enum class NodeState : std::uint8_t {
    Clean,
    Dirty,
};
```

Newly added nodes begin Clean.

Evaluation failure is an operation outcome and must never silently
convert a Dirty node to Clean.

## 8. Evaluation outcome contract

The public contract shall define an evaluation status equivalent to:

```cpp
enum class EvaluationStatus : std::uint8_t {
    Success,
    Failed,
};
```

and an outcome carrying exactly:

```text
NodeId node
EvaluationStatus status
```

The graph does not execute domain evaluation itself.

Tests simulate evaluation outcomes.

## 9. Operation result classification

The public result classification shall include exactly these semantic
cases:

```cpp
enum class GraphResultCode : std::uint8_t {
    Ok,
    NoChange,
    InvalidNodeId,
    NodeAlreadyExists,
    NodeNotFound,
    CycleDetected,
    InvalidRecomputePlan,
    EvaluationFailed,
};
```

Domain validation failures use explicit result codes rather than
exceptions as ordinary control flow.

Exact helper method naming around result inspection may be minimal but
must not alter these semantic distinctions.

## 10. Diagnostic contract

A project-owned diagnostic shall make applicable failure context
inspectable.

It shall be able to identify:

- primary/offending NodeId;
- upstream endpoint when applicable;
- downstream endpoint when applicable;
- deterministic cycle path when CycleDetected;
- GraphResultCode.

Unused NodeId fields may use invalid NodeId value 0 as not-applicable.

No third-party type may appear in diagnostics.

## 11. Snapshot contract

The graph shall expose a read-only project-owned snapshot containing:

- graph revision;
- NodeId + NodeState entries;
- dependency edges including provenance.

Snapshot ordering is canonical:

```text
nodes:
    NodeId ascending

edges:
    upstream ascending
    then downstream ascending
    then provenance enum order
```

Equivalent logical graph state must produce identical snapshot ordering
regardless of insertion history.

## 12. Graph revision

The graph maintains a monotonically increasing project-owned uint64
revision for stale-plan detection.

Revision starts at zero.

Revision increments exactly once for each successful operation that
changes committed topology or committed node state.

NoChange and failed operations do not increment revision.

P0-T009 does not define persistence or cross-session semantics for this
revision.

## 13. DependencyGraph public operations

The public class shall provide the conceptual equivalent of:

```cpp
AddNode(NodeId node)

AddDependency(
    NodeId upstream,
    NodeId downstream,
    DependencyProvenance provenance)

ReplaceComputedDependencies(
    NodeId downstream,
    std::span<const NodeId> upstreams)

InvalidateDependents(NodeId source)

MarkDirty(NodeId node)

BuildRecomputePlan() const

CommitRecompute(
    const RecomputePlan& plan,
    std::span<const NodeEvaluationOutcome> outcomes)

Snapshot() const
```

Exact const/reference qualifiers may follow normal C++20 practice but
must preserve these semantics.

## 14. Explicitly excluded graph operations

P0-T009 shall not implement RemoveNode or general topology deletion APIs.

They are unnecessary for the mandatory proof corpus and would introduce
additional mutation semantics outside this spike.

## 15. AddNode semantics

AddNode shall:

- reject NodeId{0} as InvalidNodeId;
- add a new valid node as Clean;
- return NodeAlreadyExists for an existing node;
- leave graph state unchanged on failure;
- increment revision only when a node is actually added.

## 16. AddDependency semantics

Canonical direction is:

```text
upstream -> downstream
```

meaning downstream depends on upstream.

AddDependency shall:

- reject invalid NodeIds;
- reject missing endpoints;
- never create missing nodes;
- reject self dependency as CycleDetected;
- reject any direct/transitive cycle before mutation;
- allow the same endpoint pair to carry the other provenance;
- return NoChange for an already-existing exact
  upstream/downstream/provenance edge;
- leave committed topology unchanged on every failure.

Cycle checks operate on logical endpoint relations regardless of
provenance multiplicity.

## 17. Deterministic cycle diagnostic

For proposed edge:

```text
upstream -> downstream
```

if a committed path already exists:

```text
downstream -> ... -> upstream
```

the mutation is rejected.

The reported conceptual cycle path is deterministic:

```text
upstream -> downstream -> ... -> upstream
```

Traversal used to construct diagnostic path shall visit candidate
neighbors in NodeId order so insertion order cannot affect diagnostics.

## 18. ReplaceComputedDependencies semantics

The operation replaces the complete Computed incoming dependency set
for one downstream node.

Algorithmic contract:

1. validate downstream NodeId and existence;
2. validate every supplied upstream NodeId and existence;
3. normalize supplied upstreams to unique NodeId ascending order;
4. form a candidate graph that removes only existing Computed incoming
   edges for the downstream node;
5. preserve every ExplicitSemantic edge;
6. add the normalized proposed Computed edge set to the candidate;
7. validate the entire candidate for cycles;
8. commit the candidate only if all checks succeed.

If the normalized resulting Computed set equals the current Computed set,
return NoChange.

On InvalidNodeId, NodeNotFound or CycleDetected:

```text
committed topology = exactly unchanged
committed node state = exactly unchanged
revision = exactly unchanged
```

## 19. Topology edits versus dirty state

AddNode, AddDependency and ReplaceComputedDependencies do not
automatically dirty evaluation nodes.

Topology tracking and evaluation invalidation are separate operations in
this Phase-0 primitive.

A future higher model/evaluation layer explicitly calls
InvalidateDependents or MarkDirty when domain values change.

This prevents computed-dependency refresh during evaluation from
implicitly creating regeneration loops.

P0-T010 decides future integration semantics.

## 20. InvalidateDependents semantics

InvalidateDependents(source) requires an existing valid source.

It marks only the strict transitive downstream closure Dirty.

The source itself remains in its current state.

Unrelated graph components remain unchanged.

If no state changes are necessary, return NoChange.

The operation is idempotent.

## 21. MarkDirty semantics

MarkDirty(node) requires an existing valid node.

It marks:

- the node itself Dirty;
- its complete transitive downstream closure Dirty.

Unrelated components remain unchanged.

If every affected node is already Dirty, return NoChange.

The operation is idempotent.

## 22. RecomputePlan contract

RecomputePlan shall carry:

- graph revision captured when the plan was built;
- ordered vector of planned NodeIds.

Only Dirty nodes appear in the plan.

Each node appears at most once.

A Clean upstream node is considered already current and is not inserted
merely because a Dirty dependent references it.

## 23. BuildRecomputePlan semantics

The plan is a topological order over the current Dirty induced subgraph.

Every Dirty prerequisite appears before its Dirty dependent.

When multiple Dirty nodes are simultaneously ready, the lowest NodeId is
selected first.

Therefore planning is independent of:

- node insertion order;
- edge insertion order;
- map/hash/container traversal order.

An empty Dirty set produces a valid empty plan.

The committed graph is acyclic by invariant, so BuildRecomputePlan does
not invent a cycle-recovery path.

## 24. CommitRecompute validation

CommitRecompute is the Phase-0 atomic graph-state commit primitive.

Before changing any node state it shall validate:

1. plan revision equals current graph revision;
2. plan NodeIds exactly equal the current deterministic recompute plan;
3. outcomes contain exactly one entry for every planned node;
4. no outcome references a node outside the plan;
5. no outcome is duplicated;
6. no required outcome is missing.

Any mismatch returns InvalidRecomputePlan with zero committed mutation.

## 25. CommitRecompute evaluation failure

If any validated planned outcome is Failed:

- return EvaluationFailed;
- report the first failing node in deterministic plan order;
- leave every planned node Dirty;
- leave all other node state unchanged;
- leave revision unchanged.

Successful outcomes before the failure are staged information only and
must not partially commit Clean state.

## 26. CommitRecompute success

If every planned outcome is Success:

- all planned Dirty nodes become Clean as one committed state change;
- no unrelated node changes;
- revision increments exactly once.

For an empty valid plan with zero outcomes, return NoChange and do not
increment revision.

## 27. Required implementation storage properties

Implementation may use standard-library containers only.

Externally observable semantics shall never depend on unordered/hash
iteration.

Any internal unordered structure, if used at all, must be normalized
before observable planning/diagnostic/snapshot output.

A straightforward ordered-container implementation is preferred for this
Phase-0 proof.

## 28. P0-T008 isolation

Production P0-T009 code must not include:

```text
bim/model/persistent_face_reference.hpp
bim/model/face_reference_resolver.hpp
```

and must not depend on bim::model.

No P0-T008 public type or semantic behavior is modified.

## 29. Exact initially-authorized implementation path manifest

A later Implementation Authorization may authorize Claude to modify
exactly these eight implementation/test paths:

```text
M src/dependency_graph/CMakeLists.txt
A src/dependency_graph/include/bim/dependency_graph/dependency_graph.hpp
A src/dependency_graph/src/dependency_graph.cpp
D src/dependency_graph/src/dependency_graph_anchor.cpp
M tests/unit/CMakeLists.txt
A tests/unit/unit_dependency_graph.cpp
M tests/integration/CMakeLists.txt
A tests/integration/integration_p0_t009_dependency_graph_spike.cpp
```

No other path is initially authorized.

In particular, tools/architecture_checker.cmake and architecture fixtures
are NOT initially authorized for modification.

If existing mechanical architecture enforcement proves insufficient,
Claude must STOP and return evidence to Architecture Authority for a
separate minimum-delta authorization.

## 30. CMake implementation requirements

bim_dependency_graph shall become a real public C++20 library.

Its public include directory shall follow existing repository convention:

```text
src/dependency_graph/include
```

The obsolete private compilation anchor shall be removed from the target
and deleted under the authorized manifest.

Production first-party linkage remains foundation only.

No root CMakeLists.txt modification is authorized.

No dependency manifest modification is authorized.

## 31. Test integration requirements

The new unit and integration sources shall be wired through the existing
tests/unit and tests/integration CMake conventions.

No new test framework or dependency is authorized.

Tests shall consume the public dependency_graph header rather than
private implementation internals.

## 32. Mandatory unit proof

Unit coverage shall prove at minimum:

- NodeId{0} rejection;
- node insertion and initial Clean state;
- duplicate node failure with no mutation;
- missing upstream/downstream failure;
- exact duplicate edge NoChange;
- same endpoint pair with both provenances;
- deterministic snapshot normalization;
- self-cycle rejection;
- direct-cycle rejection;
- transitive-cycle rejection;
- deterministic cycle diagnostic;
- ReplaceComputedDependencies success;
- duplicate proposed computed upstream normalization;
- missing-node computed replacement rollback;
- cyclic computed replacement rollback;
- preservation of ExplicitSemantic edges;
- InvalidateDependents idempotence;
- MarkDirty idempotence;
- empty recompute plan;
- deterministic topological planning;
- stale revision rejection;
- missing outcome rejection;
- duplicate outcome rejection;
- extra outcome rejection;
- EvaluationFailed atomic rollback;
- successful atomic Clean commit.

## 33. Mandatory integration proof corpus

### A - Chain

```text
A -> B -> C
```

InvalidateDependents(A) leaves A current, dirties B/C and plans B then C.

### B - Diamond

```text
A -> B
A -> C
B -> D
C -> D
```

D occurs once. B/C order follows NodeId ordering and both precede D.

### C - Unrelated branch

Invalidating one component does not alter an unrelated component.

### D - Cycle rejection

Self, direct and transitive cycle attempts leave topology and revision
unchanged.

### E - Insertion-order independence

Equivalent logical graphs constructed under different node and edge
insertion permutations produce identical snapshots, invalidation state
and recompute plans.

### F - Computed replacement

Successful replacement is atomic. Invalid/missing/cyclic replacement
preserves the prior computed dependency set exactly.

### G - Provenance

ExplicitSemantic and Computed edges are independently inspectable.
Computed refresh never removes the explicit edge for the same relation.

### H - Recompute failure/retry

A simulated mid-plan failure produces zero partial Clean commit.
A later full-success retry atomically cleans the complete current plan.

## 34. Determinism repetitions

Insertion-order and planning proofs shall be repeated across multiple
construction permutations, not demonstrated through one accidental order.

Repeated BuildRecomputePlan calls on unchanged state must return identical
revision and NodeId order.

Repeated Snapshot calls on unchanged state must be identical.

## 35. Mechanical architecture proof

Authoritative validation must mechanically prove at minimum:

- dependency_graph public headers contain no third-party includes/types;
- dependency_graph production files contain no model dependency;
- dependency_graph target links no first-party target except foundation;
- no OCCT/Qt/SQLite/ODA/IfcOpenShell/bgfx ownership leak;
- no prohibited source path outside the eight-path manifest changed;
- P0-T008 public headers are byte/commit unchanged;
- vcpkg manifest and baseline are unchanged.

Existing repository checker shall be reused where sufficient.

If additional checker implementation is required, STOP before editing it.

## 36. Build requirements

Authoritative Windows validation remains on the locked project toolchain:

```text
Visual Studio Build Tools 2022
VCTools 14.44.35207
MSVC 19.44.35228 x64
Windows SDK 10.0.26100.0
CMake 4.4.2
Ninja 1.12.1
C++20
```

No toolchain or dependency version change is authorized.

Final authoritative evidence must use a fresh build directory.

## 37. Regression requirement

After targeted P0-T009 tests pass, the full applicable default-off test
suite shall pass before independent review.

A failure is not automatically an implementation defect.

Architecture Authority classifies the exact first failure before any
correction is authorized.

## 38. Claude execution constraints

Even after later activation Claude MUST NOT:

- run git add;
- run git commit;
- run git merge;
- run git rebase;
- run git push;
- modify main;
- create or publish a remote task branch;
- invoke Kimi;
- broaden the authorized path manifest;
- change architecture decisions silently;
- add dependencies;
- modify persistence/schema;
- modify P0-T008 contracts.

Claude implementation handover remains uncommitted until Architecture
Authority validation and later commit authorization.

## 39. Minimum-delta correction rule

If validation finds a defect, correction authority is limited to the
minimum delta needed for the classified failure.

No rewrite or redesign is authorized unless Architecture Authority
explicitly changes the contract.

## 40. Independent review gate

Kimi review occurs only after:

- implementation is complete;
- authoritative build passes;
- mandatory targeted proof passes;
- full applicable regression passes;
- Architecture Authority freezes the candidate review baseline;
- review material is explicitly provided.

Kimi review is read-only and minimum-delta.

## 41. Candidate commit gate

Claude shall not create the implementation commit.

Candidate staging and commit are separate Architecture Authority
checkpoints after validation and independent-review disposition.

## 42. Main integration and push

Main integration is not implied by candidate approval.

Local integration and push remain separate Architecture Authority gates.

No push is authorized by this Brief.

## 43. Explicit non-goals

P0-T009 does not implement:

- final BIM UUID/element identity architecture;
- production Wall/Door/Window/Slab/Room semantics;
- model-module integration;
- production transaction integration;
- command/query integration;
- persistence or serialization;
- geometry regeneration;
- persistent-reference redesign;
- automatic runtime dependency read tracing;
- parallel/multithreaded scheduling;
- cache architecture;
- IFC/DWG/RVT dependency semantics;
- UI/viewport behavior;
- cloud/collaboration behavior;
- Phase-1 features.

## 44. Stop conditions

Claude must STOP and return to Architecture Authority if implementation
requires any of the following:

- a ninth candidate path;
- architecture-checker modification;
- architecture fixture modification;
- dependency_graph -> model coupling;
- transaction/persistence/geometry coupling;
- final platform identity decisions;
- a new third-party library;
- toolchain/dependency version changes;
- schema changes;
- production BIM semantics;
- automatic dependency discovery architecture;
- parallel scheduler design;
- change to P0-T008 public semantics;
- any Architecture Gate ambiguity that affects behavior.

## 45. Acceptance criteria

**AC-001** Exact committed governance baseline is preserved before execution.

**AC-002** Candidate path set remains within the authorized eight paths.

**AC-003** NodeId and public API remain project-owned/vendor-neutral.

**AC-004** Production target dependency remains foundation only.

**AC-005** Chain dirty propagation passes.

**AC-006** Diamond deterministic planning passes.

**AC-007** Unrelated branch isolation passes.

**AC-008** Cycle attempts fail before mutation.

**AC-009** Insertion-order independence passes.

**AC-010** Computed dependency replacement is atomic.

**AC-011** ExplicitSemantic provenance survives Computed refresh.

**AC-012** Invalid/missing/duplicate cases match frozen semantics.

**AC-013** Snapshot and planning outputs are deterministic.

**AC-014** Stale/invalid recompute plans fail closed.

**AC-015** Evaluation failure produces zero partial Clean commit.

**AC-016** Successful retry atomically cleans the complete current plan.

**AC-017** Locked Windows build passes.

**AC-018** Mechanical architecture proof passes.

**AC-019** Full applicable regression suite passes.

**AC-020** Independent review has no unresolved blocking/material finding.

**AC-021** Architecture Authority separately authorizes candidate commit.

## 46. Architecture Change Record

```text
ACR = NONE
```

This Brief changes no frozen toolchain, dependency version, module law,
persistent-reference architecture or persistence contract.

## 47. Lifecycle state

```text
P0-T009-IB v1.0          = FROZEN / APPROVED
Architecture Gate         = FROZEN / APPROVED
Implementation Auth       = NOT ISSUED
Implementation            = NOT AUTHORIZED
Claude                    = NOT AUTHORIZED
Kimi                      = NOT AUTHORIZED
Candidate commit          = NOT AUTHORIZED
Main integration          = NOT AUTHORIZED
Push                      = NOT AUTHORIZED
ACR                       = NONE
```

Product Authority approved this Brief on 2026-09-29.
