# P0-T004 - Implementation Authorization

Authorization:

BIM-AUTH-P0-T004 v1.0

Task:

P0-T004 - Persistence Spike

Date:

2026-09-14

Architecture Gate:

BIM-AG-P0-T004 v1.0

Implementation Brief:

BIM-TASK-P0-T004-CLAUDE v1.0

Authoritative authorization baseline commit:

4bc4bed53c9f8c4c1e31a247fc4dc23a5d691e68

Authoritative authorization baseline tree:

f9d0db2bbf99fb6e3f58473c19a82859c6e28dfd

Task branch:

task/P0-T004-persistence-spike

Task worktree:

D:\Projects\BIM-Platform-WT-P0-T004

Implementation Engineer:

Claude

Architecture Authority:

Product Owner + ChatGPT

Independent Reviewer:

Kimi - read-only / minimum-delta only

ACR:

NONE

## Decision

IMPLEMENTATION AUTHORIZED

Claude is authorized to implement P0-T004 only within the frozen Architecture
Gate and the released Implementation Brief.

This authorization permits production-source edits required by the brief.

It does not authorize an implementation commit, integration to `main`, remote
push, scope expansion, vcpkg baseline change, new third-party dependency,
SQLite leakage outside persistence, unrelated refactoring, final BIM
project-file design, product undo/redo behavior, or schema version 2+.

## Authorized source scope

```text
src/transactions/**
src/persistence/**
tests/unit/**
tests/integration/**
tests/architecture/**
tests/fixtures/p0_t004_*
tools/architecture_checker.cmake
scripts/ci/architecture.ps1
docs/evidence/P0-T004/**
```

Controlled build-file edits required to register those targets/tests are
authorized only where necessary.

Presumptively out-of-scope production modules are `src/model/**`,
`src/commands/**`, `src/query/**`, `src/geometry/**`, `src/viewport/**` and
`src/desktop/**`. If any out-of-scope production edit appears necessary,
Claude must STOP and request Architecture Authority direction before making it.

## Required implementation phases

Claude must follow the released brief phases in order:

```text
A - baseline inspection
B - neutral transaction contract
C - SQLite private infrastructure
D - schema v1
E - journal persistence
F - architecture enforcement
G - evidence executable
H - targeted validation
I - full regression
J - candidate freeze
K - Architecture Authority review
```

## Baseline lock

Before any source edit Claude must verify:

```text
branch: task/P0-T004-persistence-spike
HEAD:   4bc4bed53c9f8c4c1e31a247fc4dc23a5d691e68
tree:   f9d0db2bbf99fb6e3f58473c19a82859c6e28dfd
worktree: clean
```

`main` must remain at
`5bac905e29c1390c90cc6807a5d92ab217764396` and clean during implementation.

## Architecture locks

Implementation must preserve SQLite sole ownership in `src/persistence/**`,
SQLite-neutral persistence public headers, SQLite-independent
`bim_transactions`, `PRAGMA user_version` as schema authority, schema version
1 only, atomic bootstrap, fail-closed handling, atomic journal append,
close/reopen durability, rollback proof, duplicate-ID rejection, exact BLOB
round-trip, deterministic ordering, R12 `SQLITE_PERSISTENCE_ONLY`, R13
`PERSISTENCE_PUBLIC_NEUTRAL`, and all previously accepted regressions.

## Verification discipline

Claude must not turn a failing build/test/harness result into broad source
changes without Architecture Authority classification. No PASS may be asserted
for an unexecuted command. Unavailable Windows validation must be marked
UNVERIFIED.

## Candidate freeze

At the end of implementation Claude must STOP before commit and provide exact
changed paths/hashes, build/test/architecture evidence, standalone P0-T004
evidence, handover, staged count zero, task identity, and proof `main`
remained unchanged.

## Independent review

Claude must not independently hand the implementation to Kimi. Architecture
Authority decides when the verified frozen candidate is ready. Kimi remains
read-only / minimum-delta.

## Commit / integration prohibition

Even after tests pass:

```text
NO IMPLEMENTATION COMMIT
NO MAIN INTEGRATION
NO REMOTE PUSH
```

until Architecture Authority explicitly authorizes each later operation.

## Authorization state

Architecture Gate: APPROVED + LOCKED

Implementation Brief: APPROVED + RELEASED

Implementation: AUTHORIZED

Implementation commit: NOT AUTHORIZED

Main integration: NOT AUTHORIZED

Remote push: NOT AUTHORIZED

ACR: NONE

## Start instruction

Claude may begin P0-T004 at Phase A only after this authorization commit is
successfully created and the task worktree is clean at that commit.

## End
