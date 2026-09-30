# BIM Platform - P0-T009 Dependency Graph Spike Implementation Authorization

**Document ID:** `BIM-AUTH-P0-T009`
**Version:** `1.0`
**Status:** ISSUED / FROZEN
**Date:** 2026-09-29
**Task:** P0-T009 - Dependency Graph Spike
**Task branch:** `task/P0-T009-dependency-graph-spike`
**Architecture Gate:** `BIM-AG-P0-T009 v1.0`
**Implementation Brief:** `P0-T009-IB v1.0`
**Authorization baseline HEAD:** `58feb7b18f5f8a830de0717a8e57e0a256fc39e7`
**Authorization baseline TREE:** `63010fb864203c25d7cf8c474a4b43fcad1e8d99`
**Product Authority:** APPROVED
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude
**Execution Activation:** NOT ISSUED
**Claude Execution:** NOT ACTIVE / NOT AUTHORIZED TO START
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## 1. Purpose

This document freezes the implementation scope that may later be activated
for P0-T009.

Issuance of this document does not itself activate implementation.

Claude shall not modify implementation files until Architecture Authority
issues a separate explicit Execution Activation against the committed
authorization baseline.

## 2. Authority chain

The implementation authority chain is:

```text
BIM-AG-P0-T009 v1.0
    ->
P0-T009-IB v1.0
    ->
BIM-AUTH-P0-T009 v1.0
    ->
separate Architecture Authority Execution Activation
```

No stage may silently replace or broaden an earlier stage.

## 3. Frozen architecture and Brief

The Architecture Gate and Implementation Brief are authoritative and
unchanged by this Authorization.

This Authorization grants no right to redesign:

- NodeId semantics;
- dependency provenance;
- edge direction;
- cycle policy;
- invalidation semantics;
- recompute planning;
- computed dependency replacement;
- graph revision semantics;
- atomic recompute commit behavior;
- module dependency direction.

## 4. Initially authorized implementation path manifest

Upon later explicit Execution Activation, Claude may modify exactly these
eight implementation/test paths:

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

No ninth implementation/test/source path is authorized.

## 5. Explicitly unauthorized paths

The initial execution scope does not authorize modifications to:

```text
tools/architecture_checker.cmake
tests/architecture/**
tests/fixtures/**
src/model/**
src/transactions/**
src/commands/**
src/query/**
src/persistence/**
src/geometry/**
src/interop/**
src/viewport/**
src/desktop/**
vcpkg.json
vcpkg-configuration.json
```

Repository governance documents are also not part of Claude implementation
scope.

If any additional path appears necessary, Claude must STOP before editing
it and return evidence to Architecture Authority.

## 6. Production dependency boundary

The production module law remains:

```text
bim_dependency_graph -> bim::foundation
```

No production dependency on model, geometry, transactions, persistence,
commands, query, interop, desktop, viewport or any third-party subsystem
is authorized.

No new third-party dependency is authorized.

## 7. Frozen NodeId contract

Implementation must preserve the Brief contract:

- NodeId is project-owned;
- NodeId uses the frozen opaque uint64 value representation;
- NodeId{0} is invalid;
- valid NodeIds are caller supplied;
- deterministic total ordering is available;
- identity is independent of insertion order, container iteration,
  memory address and vendor/kernel identity;
- NodeId is not FeatureOwnerId;
- NodeId is not PersistentFaceReference;
- final platform BIM UUID architecture remains out of scope.

## 8. Frozen dependency semantics

Canonical evaluation direction remains:

```text
upstream -> downstream
```

where downstream depends on upstream.

Required provenance remains:

```text
ExplicitSemantic
Computed
```

Stored edge identity is conceptually:

```text
(upstream, downstream, provenance)
```

The same endpoint pair may carry both provenances while evaluation
traversal treats the endpoint pair as one logical dependency.

## 9. Frozen mutation semantics

Implementation shall preserve all Brief mutation rules, including:

- no implicit node creation;
- duplicate exact dependency is NoChange;
- same endpoint pair may carry the other provenance;
- self/direct/transitive cycles are rejected before mutation;
- failed mutation preserves committed graph state;
- ReplaceComputedDependencies replaces only Computed incoming edges;
- ExplicitSemantic dependencies survive computed refresh;
- computed replacement is atomic.

## 10. Frozen dirty-state semantics

Topology mutation does not automatically dirty nodes.

InvalidateDependents(source) dirties only the strict downstream closure.

MarkDirty(node) dirties the node and its complete downstream closure.

Both operations are idempotent.

Unrelated components remain unchanged.

## 11. Frozen planning semantics

BuildRecomputePlan shall:

- include Dirty nodes only;
- include each node once;
- order prerequisites before dependents;
- use NodeId ordering as the simultaneously-ready tie breaker;
- be independent of insertion/container iteration order;
- produce a valid empty plan when nothing is Dirty.

## 12. Frozen recompute commit semantics

CommitRecompute shall validate the current graph revision, exact current
plan membership and exactly one outcome for each planned node before
changing committed state.

Invalid/stale plans fail with zero mutation.

Any Failed evaluation produces EvaluationFailed with:

- zero partial Clean commit;
- every planned node remaining Dirty;
- unchanged graph revision;
- deterministic first-failure reporting in plan order.

Full success atomically converts all planned nodes to Clean and increments
revision exactly once.

## 13. Frozen diagnostic and snapshot semantics

Observable node, edge, cycle and plan data must be deterministic.

Canonical snapshot order remains:

```text
NodeId ascending

edge upstream ascending
then downstream ascending
then provenance
```

Cycle-path construction shall be deterministic.

## 14. Mandatory proof corpus

Activated implementation must prove:

A - chain propagation;
B - deterministic diamond planning;
C - unrelated branch isolation;
D - self/direct/transitive cycle rejection with zero mutation;
E - insertion-order independence;
F - atomic Computed dependency replacement;
G - ExplicitSemantic provenance survives Computed refresh;
H - recompute failure causes no partial Clean commit and retry succeeds.

All additional edge cases frozen in P0-T009-IB v1.0 remain mandatory.

## 15. Architecture checker policy

Claude may execute existing architecture checks.

Claude may not modify:

```text
tools/architecture_checker.cmake
tests/architecture/**
tests/fixtures/**
```

under the initial authorization.

If the existing checker cannot prove a required invariant, Claude must
STOP and return the exact gap to Architecture Authority.

Architecture Authority may later issue a separate minimum-delta scope
extension if warranted.

## 16. Locked toolchain

The authoritative implementation and validation environment remains:

```text
Visual Studio Build Tools 2022
VSCMD_VER = 17.14.39
VCToolsVersion = 14.44.35207
MSVC cl = 19.44.35228 x64
Windows SDK = 10.0.26100.0
CMake = 4.4.2
Ninja = 1.12.1
C++ = C++20
vcpkg baseline = f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

No toolchain or dependency baseline change is authorized.

## 17. Execution location

Activated implementation shall operate only in:

```text
D:\Projects\BIM-Platform-WT-P0-T009
```

The canonical main worktree must remain untouched.

## 18. Git prohibitions

Claude is explicitly forbidden from executing:

```text
git add
git commit
git merge
git rebase
git push
```

Claude shall not:

- modify main;
- publish a remote task branch;
- amend governance commits;
- rewrite Git history;
- clean/reset/restore unrelated state.

Implementation remains an uncommitted candidate until separate
Architecture Authority authorization.

## 19. Independent-review prohibition

Claude shall not invoke Kimi.

Kimi remains unauthorized until Architecture Authority completes
authoritative implementation validation and freezes the independent-review
candidate baseline.

## 20. Validation responsibility

Claude may perform implementation self-checks, targeted build/tests and
existing repository checks after activation.

Claude self-validation is evidence input only and does not constitute
Architecture Authority acceptance.

Authoritative Windows validation remains an Architecture Authority gate.

## 21. Failure classification rule

A build, test, checker or harness failure does not automatically establish
an implementation defect.

On the first material failure:

1. stop scope-expanding work;
2. capture the exact first failure;
3. preserve the candidate state;
4. report evidence to Architecture Authority;
5. wait for failure classification and minimum-delta correction authority.

No speculative correction outside the authorized manifest is permitted.

## 22. Minimum-delta correction rule

Corrections remain limited to the minimum delta required by the classified
failure.

No rewrite, redesign or scope expansion is implied by a failing check.

Architecture changes require explicit Architecture Authority disposition
and ACR treatment when applicable.

## 23. Required Claude handover

At the end of activated implementation Claude shall return a handover that
includes at minimum:

- implementation baseline HEAD/TREE;
- exact changed-path set;
- file identities/hashes available to Claude;
- implementation summary mapped to Brief sections;
- test/build commands actually executed;
- exact results actually observed;
- any checks not executed;
- any limitations or environmental gaps;
- any STOP condition encountered;
- confirmation that no Git staging/commit/push was performed;
- confirmation that no unauthorized path was intentionally modified.

Claude shall not self-declare task acceptance.

## 24. Candidate commit prohibition

This Authorization does not authorize an implementation commit.

Candidate staging and commit remain separate Architecture Authority gates
after authoritative validation and independent-review disposition.

## 25. Main integration prohibition

This Authorization does not authorize:

- merge or fast-forward into main;
- push of task branch;
- push of main;
- cleanup of worktree or local branch.

Each remains a later separate gate.

## 26. Execution Activation gate

Implementation may begin only after Architecture Authority issues an
explicit Execution Activation.

The Activation must lock at minimum:

- committed Authorization HEAD;
- committed Authorization TREE;
- task branch identity;
- clean task worktree;
- zero staged paths;
- unchanged canonical main;
- unchanged origin/main;
- remote task branch state;
- committed Architecture Gate blob;
- committed Implementation Brief blob;
- committed Implementation Authorization blob;
- exact eight-path implementation manifest.

Until that Activation is issued:

```text
Claude Execution = NOT ACTIVE
Implementation   = NOT AUTHORIZED TO START
```

## 27. Activation does not broaden scope

The later Activation may start execution only.

It may not silently alter this Authorization, Brief or Architecture Gate.

Any scope change requires explicit Architecture Authority action before
the additional path or behavior is touched.

## 28. Stop conditions

Claude must STOP if activated work requires:

- a ninth implementation/test path;
- architecture-checker changes;
- architecture fixture changes;
- dependency_graph -> model coupling;
- persistence or schema changes;
- transaction integration;
- geometry integration;
- changes to P0-T008 public contracts;
- new third-party dependencies;
- dependency/toolchain version changes;
- final BIM identity architecture;
- production BIM semantics;
- runtime automatic dependency discovery;
- parallel scheduler architecture;
- unrelated refactoring;
- any semantic departure from P0-T009-IB v1.0.

## 29. Architecture Change Record

```text
ACR = NONE
```

This Authorization changes no accepted architecture, dependency baseline,
toolchain, persistence contract or P0-T008 semantics.

## 30. Authorization lifecycle

```text
BIM-AUTH-P0-T009 v1.0 = ISSUED / FROZEN
Architecture Gate       = FROZEN / APPROVED
Implementation Brief    = FROZEN / APPROVED
Execution Activation    = NOT ISSUED
Implementation          = NOT AUTHORIZED TO START
Claude Execution        = NOT ACTIVE
Kimi                    = NOT AUTHORIZED
Candidate commit        = NOT AUTHORIZED
Main integration        = NOT AUTHORIZED
Push                    = NOT AUTHORIZED
ACR                     = NONE
```

Product Authority approved this Authorization on 2026-09-29.
