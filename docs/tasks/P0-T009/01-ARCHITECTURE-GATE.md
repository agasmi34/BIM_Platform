# BIM Platform - P0-T009 Dependency Graph Spike Architecture Gate

**Document ID:** BIM-AG-P0-T009
**Version:** 1.0
**Date:** 2026-09-29
**Task:** P0-T009 - Dependency Graph Spike
**Task branch:** `task/P0-T009-dependency-graph-spike`
**Architecture Authority:** Product Authority + ChatGPT
**Product Authority decision:** APPROVED
**Gate status:** FROZEN / APPROVED
**Implementation status:** NOT AUTHORIZED
**ACR:** NONE

## 1. Authority

This Gate freezes the P0-T009 dependency-graph architecture.

Approval does not authorize implementation.

A separate Implementation Brief and Implementation Authorization are
required before Claude may modify implementation files.

Kimi is not authorized until the later candidate review gate.

## 2. Objective

P0-T009 proves an explicit project-owned semantic dependency DAG and
computed dependency tracking primitive sufficient for the later Phase-1
Architecture Gate.

## 3. Repository baseline

```text
Branch = task/P0-T009-dependency-graph-spike
HEAD   = 96c0cc2fd9d90ccf453d6e4bd7eb8ce9f57a64ac
TREE   = 9d73da74e990b981e413f85ed9be579bbe2ade2b
```

## 4. Existing dependency-graph state

Baseline dependency_graph is scaffold-only and its canonical target is
bim_dependency_graph / bim::dependency_graph.

Its accepted production first-party dependency is bim::foundation.

## 5. Graph ownership

bim_dependency_graph owns graph topology, provenance, cycle validation,
dirty propagation, deterministic planning, computed dependency refresh,
diagnostics and graph-level recompute commit/abort state.

It does not own BIM domain objects, geometry or persistence.

## 6. Evaluation edge direction

```text
upstream -> downstream
downstream depends on upstream
```

Dirty propagation follows the arrow from changed prerequisite to affected
dependent.

Evaluation direction must not be confused with textual/domain relation
direction.

## 7. Node identity

NodeId is project-owned, opaque, caller supplied, equality comparable and
deterministically orderable.

It is independent of memory address, insertion position, container order
and vendor/kernel identity.

This Gate does not freeze final platform BIM UUID architecture.

## 8. Dependency provenance

Required provenance:

```text
ExplicitSemantic
Computed
```

No separate persistent EdgeId is required.

## 9. Duplicate and missing semantics

Duplicate node registration is an explicit no-mutation failure.

Exact duplicate dependency registration is idempotent/no-change.

Missing endpoints are explicit failures and are never implicitly created.

## 10. Dirty-state model

Committed node state requires at minimum Clean and Dirty.

InvalidateDependents(source) dirties strict downstream dependents only.

MarkDirty(node) dirties the node plus its downstream closure.

Both operations are idempotent.

## 11. Dirty-subgraph planning

Only Dirty nodes are planned.

Planning is topological, prerequisite-before-dependent, duplicate-free and
deterministic.

NodeId ordering is the deterministic tie breaker.

Insertion/container order must not affect results.

## 12. Cycle policy

Committed topology must remain acyclic.

Self, direct and transitive cycles are rejected before mutation.

Explicit additions and Computed replacements that create cycles are
rejected atomically.

Cycle diagnostics must identify the rejected relation and relevant context.

## 13. Computed dependency tracking

Required conceptual operation:

```text
ReplaceComputedDependencies(dependent, new_upstream_set)
```

It replaces only Computed incoming edges, preserves ExplicitSemantic
incoming edges, requires existing nodes, normalizes duplicates, validates
the complete replacement, rejects cycles and commits only on success.

Failure preserves old topology exactly.

Automatic runtime dependency discovery/read tracing is not required.

## 14. Recompute atomicity

```text
Dirty graph
  -> deterministic plan
  -> staged evaluation outcomes
  -> atomic Clean-state commit
```

Mid-plan failure may not leak a partially-clean committed graph.

Affected nodes remain Dirty and retry remains possible.

## 15. Diagnostic determinism

Diagnostics must inspect registered NodeIds, edges, endpoints, provenance,
Dirty/Clean state, recompute plan, operation failures and cycle context.

Equivalent logical state must produce deterministic diagnostic ordering.

## 16. P0-T008 relationship

```text
DependencyGraph NodeId != FeatureOwnerId
DependencyGraph NodeId != PersistentFaceReference
PersistentFaceReference != DependencyGraph edge
```

Production dependency_graph code shall not include or depend on P0-T008
model headers.

Future higher-layer association is deferred to P0-T010 / Phase 1.

## 17. Dependency boundary

For this spike:

```text
bim_dependency_graph -> bim::foundation
```

No production dependency is authorized on model, transactions, commands,
query, persistence, geometry, OCCT, Qt, SQLite, ODA, IfcOpenShell, bgfx,
interop modules or desktop/UI.

No new third-party dependency is authorized.

## 18. Persistence, transaction, geometry and concurrency boundaries

P0-T009 is in-memory only and does not reopen P0-T004.

No production transaction integration is authorized.

No geometry generation/regeneration is implemented.

P0-T009 is single-threaded and deterministic.

## 19. Mandatory proof corpus

A - simple chain propagation and B then C planning.
B - deterministic diamond planning and single D occurrence.
C - unrelated branch isolation.
D - self/direct/transitive cycle rejection with zero mutation.
E - insertion-order independence.
F - atomic Computed dependency replacement.
G - provenance preservation across Computed refresh.
H - failed recompute has no partial Clean commit; retry succeeds.

Also prove invalid NodeId, missing endpoints, duplicate node, duplicate
edge, repeated invalidation/planning, empty dirty set, disconnected graph
and deterministic diagnostics.

## 20. Architecture enforcement requirements

Mechanical validation must prove public API cleanliness, accepted module
dependency direction, no model/geometry/transactions/persistence coupling,
no vendor dependency leak, no first-party cycle, unchanged P0-T008 public
reference contracts, no schema change and no new third-party dependency.

## 21. Explicit non-goals

No production BIM elements, final BIM UUID architecture, model integration,
production transaction integration, persistence, serialization, geometry
generation, automatic runtime dependency discovery, parallel scheduling,
cache architecture, IFC/DWG/RVT integration, UI/viewport work, cloud
collaboration, dependency-version change or Phase-1 feature implementation.

## 22. Stop conditions

STOP and return to Architecture Authority if implementation requires:

- changing accepted module dependency direction;
- direct dependency_graph -> model coupling;
- changing P0-T008 reference semantics;
- dependency-version or third-party dependency changes;
- persistence/schema change;
- production BIM semantics;
- automatic read-tracing architecture;
- parallel scheduler architecture;
- unrelated refactoring.

## 23. Phase-1 boundary

P0-T009 completion alone does not authorize Phase-1 implementation.

The next architecture task remains P0-T010 - Phase 1 Architecture Gate.

## 24. Architecture Change Record status

```text
ACR = NONE
```

## 25. Approval

Product Authority approved BIM-AG-P0-T009 v1.0 on 2026-09-29.

```text
Architecture Gate = FROZEN / APPROVED
Implementation    = NOT AUTHORIZED
```
