# P0-T009 - Task Record

**Task:** P0-T009 - Dependency Graph Spike
**Task branch:** `task/P0-T009-dependency-graph-spike`
**Task worktree:** `D:\Projects\BIM-Platform-WT-P0-T009`
**Architecture Gate:** `BIM-AG-P0-T009 v1.0`
**Gate status:** FROZEN / APPROVED
**Product Authority approval:** 2026-09-29
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude - NOT AUTHORIZED
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## 1. Authoritative baseline

P0-T009 starts from the accepted P0-T008 integration baseline:

```text
HEAD = 96c0cc2fd9d90ccf453d6e4bd7eb8ce9f57a64ac
TREE = 9d73da74e990b981e413f85ed9be579bbe2ade2b
```

At bootstrap the task worktree, canonical main and origin/main matched
that baseline. The task worktree was clean and no remote P0-T009 branch
existed.

## 2. Objective

P0-T009 proves the minimum project-owned dependency-graph architecture
required before the Phase-1 Architecture Gate:

- explicit semantic dependency relations;
- computed dependency tracking;
- affected-downstream dirty propagation;
- deterministic topological dirty-subgraph planning;
- cycle rejection before mutation;
- atomic computed-dependency refresh;
- atomic recompute-state commit semantics;
- deterministic inspectable diagnostics.

P0-T009 is an in-memory Phase-0 architecture spike.

## 3. Existing module boundary

Baseline dependency_graph is scaffold-only:

```text
src/dependency_graph/CMakeLists.txt
src/dependency_graph/src/dependency_graph_anchor.cpp
```

Canonical target:

```text
bim_dependency_graph
bim::dependency_graph
```

Accepted production first-party dependency:

```text
bim::foundation
```

P0-T009 preserves this boundary.

## 4. Frozen graph semantics

The graph is a directed acyclic evaluation graph.

Canonical direction:

```text
upstream -> downstream
```

Meaning:

```text
downstream depends on upstream
```

Dirty propagation follows that evaluation direction.

## 5. Node identity

NodeId is project-owned, opaque, caller supplied and deterministically
orderable. It is independent of memory address, insertion order,
container iteration and vendor/kernel identity.

NodeId is not the final BIM UUID architecture.

NodeId, FeatureOwnerId and PersistentFaceReference are distinct concepts.

## 6. Dependency provenance

Required provenance includes:

- ExplicitSemantic;
- Computed.

No separate persistent EdgeId is required for this spike.

## 7. Mutation semantics

- duplicate node registration is an explicit no-mutation failure;
- duplicate exact dependency is idempotent/no-change;
- dependency addition never implicitly creates missing nodes;
- missing or invalid endpoints fail explicitly;
- self, direct and transitive cycles fail before mutation;
- rejected operations preserve committed topology.

## 8. Dirty semantics

InvalidateDependents(source) dirties only the strict downstream closure.

MarkDirty(node) dirties the node plus its downstream closure.

Both are idempotent. Unrelated components remain unchanged.

## 9. Recompute planning

Only Dirty nodes participate in planning.

Plans are topological, prerequisite-before-dependent, duplicate-free and
deterministic. NodeId order resolves simultaneously-ready ties.

Insertion order and container iteration must not affect the plan.

## 10. Computed dependency refresh

P0-T009 requires semantics equivalent to:

```text
ReplaceComputedDependencies(dependent, new_upstream_set)
```

The operation replaces only Computed incoming dependencies, preserves
ExplicitSemantic dependencies, requires existing nodes, normalizes
duplicates, validates the full replacement, cycle-checks it and commits
atomically only on success.

Failure preserves previous topology exactly.

Automatic runtime dependency discovery is out of scope.

## 11. Recompute atomicity

P0-T009 proves graph-state atomicity only:

```text
dirty graph
  -> deterministic plan
  -> staged evaluation results
  -> atomic Clean-state commit
```

A simulated mid-plan failure must not partially clean the committed graph.

## 12. P0-T008 boundary

P0-T008 persistent Face-reference ownership remains in bim_model.

Production dependency_graph code shall not depend on bim::model,
bim::geometry_api, bim::geometry_occt or P0-T008 model reference headers.

Future association belongs above this primitive and is deferred to
P0-T010 / Phase 1.

## 13. Mandatory proof corpus

A - simple chain A -> B -> C.
B - deterministic diamond ordering.
C - unrelated branch isolation.
D - self/direct/transitive cycle rejection with zero mutation.
E - insertion-order independence.
F - atomic Computed dependency replacement.
G - ExplicitSemantic provenance survives Computed refresh.
H - failed recompute produces no partial Clean commit; retry succeeds.

Additional mandatory cases include invalid NodeId, missing endpoints,
duplicate node, duplicate edge, repeated invalidation, repeated planning,
empty dirty set, disconnected graph and deterministic diagnostics.

## 14. Architecture enforcement

Later validation must prove:

- project-owned dependency_graph public API;
- dependency_graph retains only allowed first-party dependencies;
- no model, transactions, commands, query, persistence or geometry link;
- no OCCT, Qt, SQLite, ODA, IfcOpenShell or bgfx dependency;
- no first-party circular dependency;
- P0-T008 public reference contracts remain unchanged;
- no persistence/schema change;
- no new third-party dependency.

## 15. Lifecycle

```text
Architecture Gate         = FROZEN / APPROVED
Implementation Brief      = FROZEN / APPROVED
Implementation Auth       = NOT ISSUED
Implementation            = NOT AUTHORIZED
Claude                    = NOT AUTHORIZED
Kimi                      = NOT AUTHORIZED
Candidate commit          = NOT AUTHORIZED
Main integration          = NOT AUTHORIZED
Push                      = NOT AUTHORIZED
ACR                       = NONE
```

Architecture Gate approval does not authorize implementation.

## 16. Implementation Brief approval

`P0-T009-IB v1.0` was approved by Product Authority on 2026-09-29.

```text
Implementation Brief      = FROZEN / APPROVED
Implementation Auth       = NOT ISSUED
Implementation            = NOT AUTHORIZED
Claude                    = NOT AUTHORIZED
Kimi                      = NOT AUTHORIZED
Candidate commit          = NOT AUTHORIZED
Push                      = NOT AUTHORIZED
ACR                       = NONE
```

Approval of the Implementation Brief does not authorize execution.

## 17. Implementation Authorization approval

`BIM-AUTH-P0-T009 v1.0` was approved by Product Authority on 2026-09-29.

The Authorization freezes the implementation scope but does not activate
Claude execution.

```text
Implementation Brief      = FROZEN / APPROVED
Implementation Auth       = ISSUED / FROZEN
Execution Activation      = NOT ISSUED
Implementation            = NOT AUTHORIZED TO START
Claude Execution          = NOT ACTIVE
Kimi                      = NOT AUTHORIZED
Candidate commit          = NOT AUTHORIZED
Main integration          = NOT AUTHORIZED
Push                      = NOT AUTHORIZED
ACR                       = NONE
```

Exact initial execution manifest = 8 implementation/test paths.

A separate Architecture Authority Activation is required before any
implementation-file mutation.
