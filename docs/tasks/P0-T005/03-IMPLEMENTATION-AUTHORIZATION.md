# P0-T005 — IFC Spike Implementation Authorization

**Authorization ID:** BIM-AUTH-P0-T005-IMPLEMENTATION v1.0
**Date:** 2026-09-17
**Task:** P0-T005 — IFC Spike
**Architecture Gate:** BIM-AG-P0-T005 v1.0 — APPROVED
**ACR:** ACR-P0-T005-001 — APPROVED
**Implementation Brief:** BIM-TASK-P0-T005-CLAUDE v1.0 — RELEASED
**Implementation Engineer:** Claude
**Architecture Authority:** Product Authority + ChatGPT

---

## 1. Authorization decision

Architecture Authority authorizes Claude to begin P0-T005 production
implementation in the isolated P0-T005 task worktree, strictly against the
released Implementation Brief and the exact baseline below.

```text
Implementation = AUTHORIZED
```

This authorization permits implementation edits and local validation only.

It does not authorize:

```text
candidate staging
candidate commit
independent review
main integration
push
```

Those remain separate controlled lifecycle gates.

---

## 2. Exact implementation start baseline

Claude must begin from:

```text
Repository:
D:\Projects\BIM-Platform

Task worktree:
D:\Projects\BIM-Platform-WT-P0-T005

Task branch:
task/P0-T005-ifc-spike

Authorized task HEAD:
d6d84632cf467ce56f535c525db6e0d884c07207

Authorized task tree:
70721dbd111efcf692fb81fdb0d5b9b862520ce2

Main HEAD:
2c2b89f73651f7d5981d42546cbc0e321d6bb055

Main tree:
c2f6f8e463248f98aafe8109f71d85cf9003d5a9
```

Before editing, Claude must verify the task worktree is clean and exactly at
the authorized task HEAD/tree and that main remains unchanged and clean.

If any identity differs, STOP and return to Architecture Authority.

---

## 3. Governing implementation contract

The authoritative implementation contract is:

```text
BIM-TASK-P0-T005-CLAUDE v1.0
SHA256:
36c0abf18ca1a902d493ea4b771b70be55e0c5d3410559faf96342e6d507c8d9
```

Claude must implement minimum delta only.

The brief is not to be rewritten, reinterpreted, or broadened during
implementation.

---

## 4. Frozen dependency contract

```text
IfcOpenShell repository:
https://github.com/IfcOpenShell/IfcOpenShell.git

Ref:
refs/tags/ifcconvert-0.8.5

Commit:
16723d11cab9bc8a13b4e025a00d39445ccc462e

Tree:
3b3e7bd633c14333a07f6c0e498bbf8483d23ad6

IfcOpenShell dependency build:
C++17

BIM Platform first-party code:
C++20

Schema:
IFC4 ONLY

IfcGeom:
OFF

IfcPython:
OFF

IfcOpenShell OpenCascade:
OFF

CGAL:
OFF
```

The project vcpkg baseline remains unchanged:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

---

## 5. Authorized implementation scope

Claude may implement only the minimum-delta footprint permitted by the released
brief, including:

```text
vcpkg.json
CMakeLists.txt
src/interop/ifc/**
tests/integration/**
tests/unit/**
tests/fixtures/bad_architecture/**
tools/architecture_checker.cmake
scripts/ci/**
scripts/dependencies/**
third_party/licenses/**
docs/tasks/P0-T005/**
docs/project-control/**
```

A change outside that footprint requires STOP and Architecture Authority
classification before editing.

The presence of a path in this list does not itself justify modifying it.
Only changes required by BIM-TASK-P0-T005-CLAUDE v1.0 are authorized.

---

## 6. Implementation execution rules

Claude shall:

1. inspect existing repository conventions before editing;
2. preserve all accepted P0-T001 through P0-T004 architecture constraints;
3. keep IfcOpenShell APIs solely under `src/interop/ifc/**`;
4. keep public IFC contracts vendor-neutral;
5. keep first-party BIM targets at C++20;
6. use the approved pinned external IfcOpenShell dependency mechanism;
7. implement the required IFC4 semantic round-trip and failure behavior;
8. add R14 and R15 exactly as specified by the brief;
9. preserve existing architecture rules without weakening them;
10. run local targeted validation as needed during implementation.

Claude shall not:

```text
modify main
push
change the vcpkg baseline
patch pinned IfcOpenShell source
enable Python
enable IfcGeom
enable IfcOpenShell OpenCascade
enable CGAL
expand the canonical model contract
fix unrelated deferred findings
invoke Kimi
stage or commit the candidate
```

---

## 7. Build/test failure rule

Any build, test, static-analysis, architecture-checker, dependency, or harness
failure that appears to require a contract change must be returned to
Architecture Authority before changing architecture or dependency policy.

Claude may correct ordinary implementation defects that are clearly within the
released brief.

Claude must not self-authorize:

```text
new dependency
new architecture boundary
new public contract
new schema
baseline advancement
upstream source patch
scope expansion
test weakening
```

---

## 8. Required implementation endpoint

Claude's implementation turn ends with an uncommitted candidate in the isolated
task worktree plus a concise implementation report containing:

```text
baseline verification
changed path list
implemented contract summary
dependency/bootstrap status
targeted build/test results
known warnings
known failures, if any
working-tree status
```

Claude must not create the candidate commit.

The next gate after implementation is:

```text
Architecture Authority Candidate Inspection / Validation Authorization
```

Only Architecture Authority may authorize candidate freeze, staging, commit,
independent review, integration, or push.

---

## 9. Current lifecycle state

```text
P0-T005 Discovery                    = CLOSED / PASS
Architecture Gate                    = APPROVED
ACR-P0-T005-001                      = APPROVED
Dependency Resolution Phase C        = CLOSED / PASS
Implementation Brief                 = RELEASED
Implementation Authorization         = ISSUED
Claude implementation                = AUTHORIZED
Candidate staging                     = NOT AUTHORIZED
Candidate commit                      = NOT AUTHORIZED
Independent Review                    = NOT AUTHORIZED
Main integration                      = NOT AUTHORIZED
Push                                  = NOT AUTHORIZED
```
