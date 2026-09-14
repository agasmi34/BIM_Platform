# BIM Platform — P0-T004 Persistence Spike
## Implementation Brief — Claude v1.0

**Document ID:** BIM-TASK-P0-T004-CLAUDE v1.0
**Task:** P0-T004 — Persistence Spike
**Implementation Engineer:** Claude
**Architecture Authority:** Product Owner + ChatGPT
**Independent Reviewer:** Kimi (read-only, minimum-delta)
**Status:** APPROVED / RELEASED
**Implementation authorization:** NOT YET GRANTED
**ACR:** NONE

**Authoritative task branch:** `task/P0-T004-persistence-spike`
**Authoritative worktree:** `D:\Projects\BIM-Platform-WT-P0-T004`
**Architecture-gate commit:** `14d3d66b89c7d7597777e3fcc0e2e32257499d93`
**Architecture-gate tree:** `b9f362929e47ae7a4342b8d60a2b638209e465d0`
**Architecture-gate parent:** `5bac905e29c1390c90cc6807a5d92ab217764396`
**Frozen Architecture Gate:** `BIM-AG-P0-T004 v1.0`
**Canonical gate file:** `docs/gates/P0-T004_Architecture_Gate_Package_v1.0.md`

---

# 1. Purpose

Implement the Phase-0 persistence spike exactly within the frozen P0-T004
Architecture Gate.

This brief converts the frozen architecture into a concrete, testable
implementation plan for Claude.

The implementation must prove:

1. SQLite is still owned only by `bim_persistence`;
2. the persistence public API remains SQLite-neutral;
3. `bim_transactions` owns a neutral journal contract;
4. a file-backed SQLite database bootstraps atomically to schema version 1;
5. unsupported or unrecognized database states fail closed;
6. a journal transaction appends atomically;
7. committed journal data survives close/reopen;
8. uncommitted journal data is absent after close/reopen;
9. duplicate transaction IDs are rejected atomically;
10. binary payloads round-trip exactly;
11. persistent journal ordering is deterministic;
12. R12/R13 architecture rules mechanically enforce the boundary;
13. a standalone evidence executable proves the behavior;
14. all prior P0-T001/P0-T002/P0-T003 behavior remains passing.

This is a **persistence architecture spike**, not the complete BIM project-file
implementation.

---

# 2. Non-negotiable frozen architecture

Claude must treat the following as fixed.

## 2.1 Dependency law

```text
foundation
    ^
    |
model
    ^
    |
transactions
    ^
    |
commands

persistence --> model + transactions + foundation + SQLite
```

No reverse dependency is authorized.

## 2.2 SQLite ownership

Only:

```text
src/persistence/**
```

may:

- include `<sqlite3.h>`;
- name `sqlite3`, `sqlite3_stmt` or other SQLite C types;
- call `sqlite3_*`;
- use `SQLITE_*`;
- link SQLite.

## 2.3 Public-header neutrality

No header under:

```text
src/persistence/include/**
```

may expose SQLite API/type/macro tokens.

## 2.4 Transactions contract ownership

`bim_transactions` owns neutral transaction-journal semantics.

It must not depend on:

- SQLite;
- `bim_persistence`;
- Qt;
- OCCT;
- bgfx;
- Windows APIs.

## 2.5 Schema authority

SQLite:

```text
PRAGMA user_version
```

is the authoritative schema version.

P0-T004 schema version:

```text
1
```

## 2.6 Explicit non-goals

Do not implement:

- BIM domain tables;
- Wall/Slab/Door/Window persistence;
- OCCT shape serialization;
- command execution;
- final undo/redo product behavior;
- autosave;
- collaboration;
- cloud sync;
- database encryption;
- final product file extension;
- schema version 2+;
- ORM;
- alternate database engines;
- new third-party persistence libraries;
- vcpkg baseline changes.

---

# 3. Implementation footprint

The expected production footprint is intentionally narrow.

## 3.1 `src/transactions`

Expected additions/changes:

```text
src/transactions/CMakeLists.txt
src/transactions/include/bim/transactions/journal.hpp
src/transactions/src/journal.cpp
```

The existing private `transactions_anchor.cpp` may be removed only if the
real implementation makes it unnecessary.

## 3.2 `src/persistence`

Expected additions/changes:

```text
src/persistence/CMakeLists.txt
src/persistence/include/bim/persistence/probe.hpp
src/persistence/src/persistence_probe.cpp
src/persistence/src/sqlite_connection.hpp
src/persistence/src/sqlite_connection.cpp
src/persistence/src/schema_v1.hpp
src/persistence/src/schema_v1.cpp
src/persistence/src/journal_store.hpp
src/persistence/src/journal_store.cpp
```

Exact private filenames may differ slightly if the same responsibilities are
preserved. Do not create a large framework.

## 3.3 Tests

Expected:

```text
tests/unit/unit_transactions_journal_contract.cpp

tests/integration/integration_persistence_schema_v1.cpp
tests/integration/integration_persistence_journal_commit_reopen.cpp
tests/integration/integration_persistence_journal_rollback.cpp
tests/integration/integration_persistence_journal_duplicate_id.cpp
tests/integration/integration_persistence_journal_binary_payload.cpp
tests/integration/p0_t004_persistence_evidence.cpp
```

The existing:

```text
tests/integration/integration_persistence_sqlite_memory.cpp
```

must remain passing.

## 3.4 Architecture tests / fixtures

Expected:

```text
tests/fixtures/p0_t004_bad_sqlite_owner/...
tests/fixtures/p0_t004_bad_persistence_api/...
tests/architecture/CMakeLists.txt
tools/architecture_checker.cmake
scripts/ci/architecture.ps1
```

## 3.5 Evidence / governance

Implementation may create/update:

```text
docs/evidence/P0-T004/**
```

Do not modify Architecture Gate decisions during implementation.

---

# 4. Neutral transaction journal contract

Create:

```text
src/transactions/include/bim/transactions/journal.hpp
```

Preferred semantic shape:

```cpp
namespace bim::transactions {

struct JournalRecord {
    std::string kind;
    std::vector<std::byte> payload;
};

struct JournalTransaction {
    std::string id;
    std::vector<JournalRecord> records;
};

[[nodiscard]] bim::foundation::Status ValidateJournalTransaction(
    const JournalTransaction& transaction);

} // namespace bim::transactions
```

Equivalent C++20 spellings are acceptable if semantics stay the same.

## 4.1 Required validation

`ValidateJournalTransaction` must reject:

- empty transaction ID;
- zero records;
- any record with empty `kind`.

Payload may be empty.

Payload is opaque bytes.

## 4.2 Required invariants

- caller-provided record order is preserved;
- transaction ID is opaque;
- record kind is opaque;
- transaction layer performs no SQLite I/O;
- transaction layer defines no timestamp requirement;
- no command/event/domain serialization is invented here.

---

# 5. Persistence public API

Keep the public P0-T004 surface deliberately narrow.

The existing public header:

```text
src/persistence/include/bim/persistence/probe.hpp
```

may be extended rather than creating a broad production repository API.

Preferred API semantics:

```cpp
struct PersistenceSpikeEvidence {
    // project-owned fields only
};

[[nodiscard]] bim::foundation::Status RunSqliteMemoryProbe();

[[nodiscard]] bim::foundation::Status RunPersistenceSpike(
    const std::filesystem::path& database_path,
    const std::filesystem::path& evidence_json_path);
```

Claude may choose a small alternative public shape if it is easier to test,
but it must satisfy all of the following:

- zero SQLite types in public headers;
- no generic SQL execution API;
- no large `ProjectStore` abstraction;
- no final product document API;
- no hidden implementation dependency leaks.

If a broader API appears necessary, STOP and return to Architecture Authority.

---

# 6. Private SQLite connection helper

Implement SQLite ownership privately under `src/persistence/src/**`.

A small RAII helper is preferred.

Required responsibilities:

- open database;
- close database on every path;
- run fixed PRAGMA statements;
- prepare statements;
- finalize statements;
- bind text/blob/integer parameters safely;
- map SQLite failures into project-owned `bim::foundation::Status`;
- expose no SQLite handle outside persistence private implementation.

All caller-controlled values must use bind parameters.

Do not concatenate transaction IDs, kinds, or payloads into SQL strings.

---

# 7. SQLite target resolution

Update `src/persistence/CMakeLists.txt` to prefer:

```text
1. SQLite3::SQLite3
2. SQLite::SQLite3
3. unofficial::sqlite3::sqlite3
4. FATAL_ERROR
```

This is an approved maintenance correction.

Do not:

- change the frozen vcpkg baseline;
- add another SQLite package;
- add a new persistence dependency.

---

# 8. File-backed connection policy

For P0-T004 file-backed tests/evidence configure and verify:

```text
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;
PRAGMA synchronous = FULL;
busy timeout = 5000 ms
```

Requirements:

- verify `foreign_keys` is actually enabled;
- verify effective `journal_mode` is `wal`;
- verify synchronous mode is not silently weaker than the frozen requirement;
- set busy timeout through SQLite API or equivalent private implementation;
- failure to establish the required connection policy is a spike failure.

Do not claim network-share support.

---

# 9. Schema version handling

Implement an internal schema open/validation flow.

## 9.1 Version 0

Read:

```text
PRAGMA user_version
```

If it is `0`, detect existing non-SQLite user tables.

If any pre-existing user table exists:

```text
FAIL CLOSED
```

Do not adopt, truncate, drop, or overwrite it.

If no user tables exist, perform atomic bootstrap to v1.

## 9.2 Version 1

Validate that the required schema exists and is structurally compatible with
the P0-T004 schema.

Do not silently create missing tables while claiming the database is already
valid v1.

## 9.3 Version > 1

Return project-owned error:

```text
unsupported newer schema
```

No mutation.

## 9.4 Other invalid/unreadable state

Fail closed.

---

# 10. Atomic bootstrap `0 -> 1`

The migration must be one explicit SQLite transaction.

Conceptual flow:

```text
BEGIN IMMEDIATE;

CREATE TABLE journal_transactions (...);
CREATE TABLE journal_entries (...);

PRAGMA user_version = 1;

COMMIT;
```

Any error:

```text
ROLLBACK
```

Never expose:

```text
user_version = 1
```

with only part of the required schema.

---

# 11. Schema v1

Implement only the Phase-0 journal schema.

## 11.1 `journal_transactions`

Required semantics:

```sql
CREATE TABLE journal_transactions (
    transaction_id TEXT PRIMARY KEY NOT NULL
);
```

## 11.2 `journal_entries`

Required semantics:

```sql
CREATE TABLE journal_entries (
    sequence        INTEGER PRIMARY KEY AUTOINCREMENT,
    transaction_id  TEXT NOT NULL,
    ordinal         INTEGER NOT NULL CHECK (ordinal >= 0),
    kind            TEXT NOT NULL CHECK (length(kind) > 0),
    payload         BLOB NOT NULL,
    FOREIGN KEY (transaction_id)
        REFERENCES journal_transactions(transaction_id)
        ON DELETE CASCADE,
    UNIQUE (transaction_id, ordinal)
);
```

A small SQL spelling/index adjustment is allowed only if these semantics are
preserved.

Do not add BIM object/domain tables.

---

# 12. Private journal store

Implement a private persistence helper for journal write/read.

Preferred responsibilities:

```text
AppendTransaction(...)
ReadCommittedJournal(...)
```

Do not expose these as a broad final product API unless required for tests.

## 12.1 Append algorithm

1. validate neutral transaction contract before opening a write transaction;
2. `BEGIN IMMEDIATE`;
3. insert into `journal_transactions`;
4. insert each journal record with ordinal `0..N-1`;
5. `COMMIT`;
6. on any failure, `ROLLBACK`;
7. return project-owned status.

## 12.2 Duplicate transaction ID

A duplicate transaction ID must:

- fail;
- preserve the original committed transaction;
- add no extra records.

## 12.3 Read ordering

Committed entries must be read deterministically using:

```text
ORDER BY sequence ASC
```

Transaction-local record order must be reconstructed using persisted `ordinal`.

Do not rely on unspecified row order.

---

# 13. Rollback proof hook

The integration test must prove absence of uncommitted data after reopen.

A private/test-only helper may deliberately:

```text
BEGIN IMMEDIATE
insert transaction row
insert one or more entry rows
close/abort without COMMIT
```

Then reopen and prove those rows are absent.

Do not mislabel this as a power-loss or OS-crash guarantee.

Do not add a production public “write without commit” API.

---

# 14. Binary payload handling

Use SQLite BLOB binding.

At least one test payload must contain:

```text
0x00
0x01
0x7F
0x80
0xFE
0xFF
```

plus additional ordinary bytes.

Read back using blob-safe APIs and compare exact length and byte values.

No UTF-8/text conversion is allowed for payload.

---

# 15. Test filesystem isolation

Each file-backed integration test must use a unique temporary/build-tree
database path.

Rules:

- never create `.db`, `-wal`, or `-shm` files under the source tree;
- remove stale test output before use, or use a unique path;
- close all SQLite handles before cleanup;
- stale outputs must never make a later run pass;
- tests must not depend on a user-specific absolute path.

Prefer a small reusable test helper under `tests/integration/` only if it keeps
the implementation simpler.

---

# 16. Unit test

Add:

```text
unit_transactions_journal_contract
```

Required cases:

1. valid transaction accepted;
2. empty ID rejected;
3. zero records rejected;
4. empty record kind rejected;
5. empty payload accepted;
6. binary payload accepted;
7. input record order remains unchanged after validation.

The unit test must link only the minimum required first-party target(s).

---

# 17. Integration tests

Add exact tests:

```text
integration_persistence_schema_v1
integration_persistence_journal_commit_reopen
integration_persistence_journal_rollback
integration_persistence_journal_duplicate_id
integration_persistence_journal_binary_payload
```

## 17.1 `integration_persistence_schema_v1`

Must prove:

- fresh DB reaches user_version=1;
- required tables exist;
- required connection settings are effective;
- opening valid v1 succeeds;
- newer schema is rejected without mutation;
- version-0 DB containing a pre-existing user table is rejected without mutation.

## 17.2 `integration_persistence_journal_commit_reopen`

Must prove:

- append one transaction with multiple records;
- close DB;
- reopen DB;
- transaction remains;
- transaction ID preserved;
- kinds preserved;
- ordinals preserved;
- persisted sequence ordering deterministic.

## 17.3 `integration_persistence_journal_rollback`

Must prove:

- uncommitted transaction rows are created in a controlled private/test path;
- connection ends without commit;
- reopen;
- transaction and entries are absent.

## 17.4 `integration_persistence_journal_duplicate_id`

Must prove:

- first append succeeds;
- second append with same ID fails;
- first transaction remains unchanged;
- second transaction contributes zero rows.

## 17.5 `integration_persistence_journal_binary_payload`

Must prove exact byte-for-byte BLOB round-trip including embedded zero.

---

# 18. Preserve existing memory probe

Keep:

```text
integration_persistence_sqlite_memory
```

passing.

`RunSqliteMemoryProbe()` may remain as-is or receive only a minimum-delta
refactor needed to share safe private SQLite helpers.

Do not delete it.

---

# 19. Architecture checker — R12

Add selectable rule:

```text
SQLITE_PERSISTENCE_ONLY
```

It must reject actual first-party C/C++ SQLite use outside:

```text
src/persistence/**
```

At minimum protect:

```text
<sqlite3.h>
sqlite3_
sqlite3*
SQLITE_
```

Use the existing comment-aware/boundary-aware checker mechanisms as
appropriate.

Do not reintroduce known substring false-positive defects from P0-T002/P0-T003.

Add real-tree test:

```text
arch_sqlite_persistence_only
```

Add negative fixture test:

```text
arch_p0_t004_sqlite_owner_fixture_rejected
```

Fixture must contain a genuine code-level SQLite violation outside
`persistence/**`.

---

# 20. Architecture checker — R13

Add selectable rule:

```text
PERSISTENCE_PUBLIC_NEUTRAL
```

Scan:

```text
src/persistence/include/**
```

and reject SQLite leakage.

Add real-tree test:

```text
arch_persistence_public_neutral
```

Add negative fixture test:

```text
arch_p0_t004_persistence_api_fixture_rejected
```

Fixture must contain a genuine SQLite type/header/token leak in a persistence
public header.

Both fixture tests must use:

```text
WILL_FAIL TRUE
```

---

# 21. Architecture CI job

Update:

```text
scripts/ci/architecture.ps1
```

Add exact required names:

```text
arch_sqlite_persistence_only
arch_p0_t004_sqlite_owner_fixture_rejected
arch_persistence_public_neutral
arch_p0_t004_persistence_api_fixture_rejected
```

Each must continue to be invoked by exact anchored name with:

```text
--no-tests=error
```

No broad regex-only shortcut is allowed.

---

# 22. Standalone evidence executable

Add:

```text
p0_t004_persistence_evidence
```

It must:

- accept explicit output/database location arguments;
- never write into the source tree;
- create a fresh controlled DB;
- execute the mandatory evidence scenarios;
- write deterministic JSON;
- exit `0` only when all mandatory checks pass;
- exit non-zero if any mandatory check fails.

Minimum semantic JSON:

```json
{
  "schema_version": 1,
  "foreign_keys_enabled": true,
  "journal_mode": "wal",
  "synchronous_mode_verified": true,
  "committed_transaction_visible_after_reopen": true,
  "rolled_back_transaction_absent_after_reopen": true,
  "duplicate_transaction_rejected": true,
  "binary_payload_roundtrip_exact": true,
  "journal_order_preserved": true,
  "newer_schema_rejected": true,
  "unrecognized_version_zero_database_rejected": true,
  "overall_passed": true
}
```

Use deterministic transaction IDs/payloads.

Do not put wall-clock timestamps in acceptance-critical evidence.

---

# 23. Evidence directory

Create:

```text
docs/evidence/P0-T004/
```

At minimum implementation may prepare:

```text
CLAUDE_HANDOVER.md
CLAUDE_HANDOVER.json
```

These are implementation/evidence handover records, not substitutes for
executed verification.

Never write fake PASS results.

If Claude cannot execute a Windows-only validation step, mark it explicitly
UNVERIFIED and leave it for the Windows Execution Operator / Architecture
Authority.

---

# 24. Error handling

SQLite errors stay behind the persistence boundary.

A project-owned error string may include numeric SQLite result code/context
for diagnostics, but public signatures must not expose SQLite enums/types.

Required fail-closed cases include:

- open error;
- PRAGMA policy failure;
- schema read error;
- unsupported newer schema;
- non-empty unrecognized version-0 database;
- malformed/missing version-1 schema;
- neutral transaction validation failure;
- duplicate transaction ID;
- statement prepare/bind/step failure;
- commit failure;
- evidence mismatch.

Do not silently:

- delete tables;
- recreate an unknown DB;
- truncate;
- downgrade;
- “repair” unknown schema.

---

# 25. Statement / transaction resource safety

All SQLite statements must be finalized on every path.

All database handles must close on every path.

Every started write transaction must have one clear outcome:

```text
COMMIT
or
ROLLBACK
```

Prefer RAII private helpers.

Do not use `NOLINT`, warning suppression, disabled tests or fixture weakening
as a substitute for fixing code.

---

# 26. Build integration

Update target sources only where required.

`bim_transactions` must gain its public include directory once
`journal.hpp` exists.

`bim_persistence` remains the sole SQLite-linked target.

No new third-party dependency.

No vcpkg baseline movement.

No source-wide unrelated cleanup.

---

# 27. Formatting and static analysis

All changed C++/headers must pass the repository's existing formatting and
static-analysis requirements.

Do not weaken:

```text
scripts/ci/static-analysis.ps1
```

or repository warning policy.

If clang-tidy exposes a source defect, correct the source with minimum delta.

If a verification harness fails, stop and let Architecture Authority classify
source defect vs harness defect before changing production code.

---

# 28. Required implementation phases

Claude must work in controlled phases.

## Phase A — baseline inspection

Before editing:

- confirm branch:
  `task/P0-T004-persistence-spike`;
- confirm HEAD:
  `14d3d66b89c7d7597777e3fcc0e2e32257499d93`;
- confirm tree:
  `b9f362929e47ae7a4342b8d60a2b638209e465d0`;
- confirm worktree clean;
- confirm main remains:
  `5bac905e29c1390c90cc6807a5d92ab217764396`;
- inspect frozen Gate and this brief.

## Phase B — neutral transaction contract

Implement:

- journal neutral types;
- validation;
- unit tests.

No persistence implementation yet beyond compile integration.

## Phase C — SQLite private infrastructure

Implement:

- SQLite target-resolution correction;
- private connection/resource helpers;
- required PRAGMA policy;
- SQLite-neutral public boundary preserved.

## Phase D — schema v1

Implement:

- version inspection;
- version-0 empty DB detection;
- atomic 0->1 bootstrap;
- v1 schema validation;
- newer/unrecognized DB rejection.

## Phase E — journal persistence

Implement:

- atomic append;
- deterministic read;
- duplicate rejection;
- BLOB-safe payload handling;
- rollback test hook.

## Phase F — architecture enforcement

Implement:

- R12;
- R13;
- four exact CTest architecture tests;
- two negative fixtures;
- architecture CI required-test additions.

## Phase G — evidence executable

Implement:

- standalone executable;
- deterministic JSON;
- non-zero failure behavior;
- controlled output path.

## Phase H — targeted validation

Run targeted:

- unit transaction tests;
- all P0-T004 integration tests;
- four new architecture tests;
- evidence executable;
- formatting/static analysis for changed source.

Do not commit.

## Phase I — full regression

Run full repository configure/build/CTest and required CI jobs.

Do not alter code merely because a wrapper/harness reports FAIL until the
actual failure is classified.

## Phase J — candidate freeze

Produce:

- exact changed-path manifest;
- source hashes;
- build/test evidence;
- handover;
- staged count = 0;
- main unchanged.

No implementation commit.

## Phase K — Architecture Authority review

Return candidate/evidence to Architecture Authority.

Only AA may:

- authorize correction;
- authorize Kimi review;
- authorize task commit;
- authorize main integration.

---

# 29. Implementation Engineer reporting format

At the end of each implementation phase Claude must report:

```text
PHASE:
STATUS:
CHANGED PATHS:
BUILD:
TESTS:
ARCHITECTURE:
EVIDENCE:
KNOWN ISSUES:
ACR:
STOP/READY:
```

Do not report a PASS for any command not actually executed.

Do not replace raw evidence with prose when a concrete command/output identity
can be given.

---

# 30. Source-change discipline

Claude must use minimum necessary delta.

Forbidden without explicit AA approval:

- unrelated refactors;
- broad renames;
- reformatting untouched modules;
- changing P0-T003 viewport code;
- changing P0-T002 geometry code;
- changing model/commands/query semantics;
- toolchain upgrade;
- vcpkg baseline update;
- new dependency;
- public API expansion beyond this brief.

If any such change appears necessary:

```text
STOP
```

and explain why.

---

# 31. Expected acceptance mapping

Implementation evidence must allow AA to map the candidate directly to:

```text
AC-001 through AC-027
```

from `BIM-AG-P0-T004 v1.0`.

Claude must not redefine those acceptance criteria.

---

# 32. Independent review policy

Kimi is independent reviewer only.

Kimi must receive a frozen review packet after authoritative verification.

Kimi must not:

- implement production code;
- rewrite the solution;
- expand scope.

If Kimi identifies a genuine defect, Architecture Authority will request
minimum-delta correction only.

Acceptance requires:

```text
BLOCKER = 0
MAJOR   = 0
```

MINOR/NOTE findings may be deferred by Architecture Authority.

---

# 33. Implementation commit policy

Claude must not commit implementation code unless Architecture Authority
explicitly authorizes it after:

1. authoritative full verification PASS;
2. candidate freeze;
3. independent review with BLOCKER=0 / MAJOR=0;
4. Architecture Authority integration decision.

Until then:

```text
NO IMPLEMENTATION COMMIT
NO MAIN INTEGRATION
NO REMOTE PUSH
```

---

# 34. ACR status

```text
ACR = NONE
```

The brief implements an already-approved architecture.

An ACR is required only if Claude proposes a material architecture change,
including but not limited to:

- replacing SQLite;
- new DB/ORM/serialization dependency;
- exposing SQLite in public interfaces;
- changing dependency direction;
- moving SQLite ownership outside persistence;
- changing frozen toolchain/dependency baseline;
- turning the spike into the complete BIM project-file architecture.

---

# 35. Exit condition for this brief

This brief is ready to be released to Claude only after Architecture Authority
approves it and records a governance-only brief-release commit.

That release commit still does **not** authorize implementation.

A separate P0-T004 Implementation Authorization document/commit is required
before Claude may edit production source.

---

## End of P0-T004 Implementation Brief — Claude v1.0
