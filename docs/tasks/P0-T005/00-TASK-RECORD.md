# P0-T005 — Task Record

**Task:** P0-T005 — IFC Spike

**State:** ARCHITECTURE GATE APPROVED / DEPENDENCY RESOLUTION NEXT / IMPLEMENTATION NOT RELEASED

**Architecture Gate:** BIM-AG-P0-T005 v1.0

**Product Authority approval:** 2026-09-16

**Architecture Authority:** Product Authority + ChatGPT

**Implementation Engineer:** Claude

**Independent Reviewer:** Kimi

## Repository identity

- Base branch: `main`
- Approved base SHA: `2c2b89f73651f7d5981d42546cbc0e321d6bb055`
- Approved base tree: `c2f6f8e463248f98aafe8109f71d85cf9003d5a9`
- Task branch: `task/P0-T005-ifc-spike`
- Task worktree: `D:\Projects\BIM-Platform-WT-P0-T005`

## Objective

Prove a vendor-neutral IfcOpenShell C++ adapter boundary and controlled IFC
round trip without introducing final BIM model semantics or leaking
third-party types across project-owned public APIs.

## Current authority state

Architecture Gate:

**APPROVED**

Dependency-resolution evidence:

**NEXT / NOT YET EXECUTED**

Implementation Brief:

**NOT RELEASED**

Implementation:

**NOT AUTHORIZED**

The next activity after this governance commit is a read-only dependency
resolution preflight against the frozen vcpkg baseline.

## ACR-P0-T005-001

**Status:** APPROVED — 2026-09-16

The frozen vcpkg baseline does not contain IfcOpenShell.

Architecture Authority approved a scoped dependency exception permitting an
exact pinned IfcOpenShell 0.8.5-line upstream source revision to be built into
an external dependency prefix for P0-T005 only.

The exact upstream commit and build contract remain pending Phase B evidence.

Implementation remains NOT AUTHORIZED.

## Implementation Brief release — BIM-TASK-P0-T005-CLAUDE v1.0

**Status:** RELEASED — IMPLEMENTATION NOT AUTHORIZED
**Date:** 2026-09-17

Dependency Resolution Phase C closed PASS and froze the exact IfcOpenShell
integration contract. The P0-T005 implementation brief is released against
ACR-P0-T005-001.

Claude must wait for a separate Architecture Authority Implementation
Authorization before changing production implementation files.

## Implementation Authorization — BIM-AUTH-P0-T005-IMPLEMENTATION v1.0

**Status:** ISSUED
**Date:** 2026-09-17

Claude is authorized to begin minimum-delta P0-T005 implementation from task
commit `d6d84632cf467ce56f535c525db6e0d884c07207` and tree
`70721dbd111efcf692fb81fdb0d5b9b862520ce2`.

Candidate staging/commit, Kimi review, main integration, and push remain NOT
AUTHORIZED.
