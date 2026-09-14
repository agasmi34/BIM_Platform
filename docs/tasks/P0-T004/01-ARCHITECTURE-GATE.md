# P0-T004 - Architecture Gate

Gate:

BIM-AG-P0-T004 v1.0

Status:

APPROVED + LOCKED

Date:

2026-09-14

Authoritative baseline:

5bac905e29c1390c90cc6807a5d92ab217764396

Authoritative tree:

3121a74e2e5edf960adfe5f36d0684541c805a1a

Product Authority approval:

CONFIRMED

Architecture Authority approval:

CONFIRMED

ACR:

NONE

Implementation:

NOT AUTHORIZED

Packaging note:

The pre-commit approved package was semantically approved but accidentally
retained the words "Draft" in its title/document ID. Before any governance
commit, Architecture Authority corrected those two labels only. No
architecture decision, scope, acceptance criterion, dependency, schema rule,
or implementation authorization changed.


# 1. Gate purpose

P0-T004 exists to prove the first durable persistence slice of the BIM Platform without prematurely designing the complete BIM project database.

The task has two architectural objectives:

1. freeze a disciplined SQLite schema/versioning philosophy behind the existing `bim_persistence` boundary; and
2. prove durable, atomic transaction-journal behavior using a neutral first-party transaction contract.

This gate does **not** authorize a complete BIM data model, object serialization format, undo/redo product feature, collaboration/synchronization system, or final commercial project-file format.

The result must remain a Phase-0 spike that is strong enough to establish the persistence architecture while deliberately leaving higher-level BIM semantics for later tasks.

---

# 2. Discovery facts accepted by Architecture Authority

The read-only P0-T004 discovery established the following baseline facts:

- `src/persistence` already exists and currently contains only:
  - `CMakeLists.txt`;
  - public `probe.hpp`;
  - private `persistence_probe.cpp`.
- The current persistence probe opens SQLite `:memory:`, executes `SELECT 1`, closes, and exposes no SQLite type through the public header.
- `bim_persistence` is already the intended sole SQLite owner and currently links:
  - `bim::foundation` publicly;
  - `bim::model`, `bim::transactions`, and SQLite privately.
- `src/transactions` currently contains only a compilation anchor and has no transaction or undo-journal behavior.
- There is one existing persistence integration test:
  - `integration_persistence_sqlite_memory`.
- Discovery found no raw `sqlite3` leakage outside the persistence boundary.
- P0-T004 is already named in project governance as the task for:
  - SQLite schema philosophy; and
  - transaction journal proof.
- No P0-T004 branch or worktree currently exists.
- The baseline `main` was unchanged and clean after discovery.
- The current vcpkg baseline remains frozen.
- A prior Kimi note identified a maintenance issue in SQLite CMake target resolution: prefer `SQLite3::SQLite3` when provided instead of relying first on deprecated `SQLite::SQLite3`.

These facts are treated as the authoritative starting point for this gate.

---

# 3. Frozen scope

## 3.1 In scope

P0-T004 shall implement and verify:

1. a file-backed SQLite Phase-0 project database;
2. explicit schema versioning;
3. atomic bootstrap from schema version 0 to schema version 1;
4. validation/rejection of unsupported or unrecognized database state;
5. a neutral transaction-journal data contract owned by `bim_transactions`;
6. append-only persistent journal storage owned by `bim_persistence`;
7. all-or-nothing append of a transaction and its ordered records;
8. commit persistence across close/reopen;
9. rollback/no-visible-write behavior for an uncommitted transaction;
10. exact binary payload round-trip;
11. duplicate transaction-ID rejection;
12. deterministic journal read ordering;
13. SQLite configuration verification for the file-backed spike;
14. architecture rules mechanically enforcing sole SQLite ownership and public-header neutrality;
15. a standalone evidence executable producing machine-readable P0-T004 evidence;
16. regression verification of all previously accepted P0-T001/P0-T002/P0-T003 behavior.

## 3.2 Explicitly out of scope

P0-T004 shall **not** implement:

- Wall/Slab/Door/Window or any other BIM domain tables;
- complete model persistence;
- OCCT shape serialization;
- viewport state persistence;
- user preferences;
- IFC/DWG/RVT persistence;
- command execution;
- product undo/redo behavior;
- history branching;
- collaborative editing;
- multi-user locking;
- network filesystems;
- cloud synchronization;
- backup/restore UI;
- database encryption;
- compression;
- final BIM project-file extension or commercial file-format identity;
- schema version 2+;
- data migration from any released product format;
- arbitrary SQL exposed to callers;
- ORM introduction;
- replacement of SQLite;
- remote database servers;
- background autosave;
- performance optimization beyond correctness-oriented smoke measurements.

Any requirement from the above list requires a later task or Architecture Change Request as appropriate.

---

# 4. Module ownership and dependency law

The already-frozen dependency direction remains:

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

P0-T004 adds no reverse dependency.

## AG-P0T004-001 — SQLite sole owner

Only `src/persistence/**` may include SQLite headers, use `sqlite3*`, use `SQLITE_*` macros, call `sqlite3_*`, or link SQLite.

No first-party module outside `src/persistence/**` may directly reference SQLite APIs.

## AG-P0T004-002 — persistence public surface remains vendor-neutral

No public header under:

```text
src/persistence/include/**
```

may expose:

- `sqlite3`;
- `sqlite3_stmt`;
- any `sqlite3_*` symbol;
- any `SQLITE_*` macro/type;
- `<sqlite3.h>`;
- vendor-specific statement or connection handles.

Private SQLite usage remains an implementation detail.

## AG-P0T004-003 — transactions owns neutral journal semantics

`bim_transactions` owns the neutral journal record contract.

It must not depend on SQLite or on `bim_persistence`.

`bim_persistence` may consume the neutral contract because the existing dependency law already permits:

```text
persistence --> transactions
```

## AG-P0T004-004 — commands do not become a persistence client

P0-T004 shall not wire `bim_commands` directly to `bim_persistence`.

This task proves persistence and transaction-journal mechanics independently of the command execution layer.

---

# 5. Neutral transaction-journal contract

P0-T004 may introduce a small public header under:

```text
src/transactions/include/bim/transactions/
```

The contract shall remain free of SQLite, Qt, OCCT, bgfx, Windows, and persistence types.

The approved semantic shape is:

```text
JournalRecord
  kind       : opaque non-empty stable string key
  payload    : opaque byte sequence

JournalTransaction
  id         : opaque non-empty transaction identifier
  records    : ordered sequence of one or more JournalRecord
```

The concrete C++ spelling may be adjusted by the Implementation Engineer for safe C++20 ergonomics, but the semantics above are frozen.

### Required semantics

- `id` is opaque to persistence.
- `id` must be non-empty.
- `id` is unique in the journal.
- a transaction must contain at least one record.
- record `kind` must be non-empty.
- `payload` is opaque bytes; persistence must not interpret it.
- the input vector/order defines record ordinal `0..N-1`.
- P0-T004 does not place timestamps in the neutral contract.
- P0-T004 does not define command/event/domain serialization.
- P0-T004 does not claim that this contract is the final product undo/redo contract.

The transaction module may provide deterministic validation helpers, but it shall not perform I/O.

---

# 6. Schema philosophy

## AG-P0T004-005 — schema authority

SQLite `PRAGMA user_version` is the authoritative integer schema version for P0-T004.

The first schema is:

```text
schema version = 1
```

No alternative version source shall compete with `user_version`.

## AG-P0T004-006 — empty-database bootstrap

A new file or an SQLite database with:

```text
user_version = 0
```

may be initialized to version 1 **only if no pre-existing user tables are present**.

If version 0 is found together with unrelated/pre-existing user tables, the database must be rejected as unrecognized rather than silently adopted.

## AG-P0T004-007 — forward compatibility behavior

- `user_version == 1`: open and validate the required P0-T004 schema.
- `user_version == 0`: initialize only when the DB is otherwise empty.
- `user_version > 1`: fail closed as a newer unsupported schema.
- negative/corrupt/unreadable state: fail closed.
- there is no downgrade path.
- there is no version 2 migration in P0-T004.

## AG-P0T004-008 — migration atomicity

The `0 -> 1` bootstrap is a migration and must be atomic.

Required sequence:

```text
BEGIN IMMEDIATE
  create required schema
  validate bootstrap invariants
  set PRAGMA user_version = 1
COMMIT
```

Any error must cause rollback. Version 1 must never be visible together with a partially-created schema.

---

# 7. Schema v1

The Phase-0 schema is intentionally small.

It shall contain only journal persistence needed by this spike.

## 7.1 Journal transaction table

Conceptually:

```sql
CREATE TABLE journal_transactions (
    transaction_id TEXT PRIMARY KEY NOT NULL
);
```

## 7.2 Journal entry table

Conceptually:

```sql
CREATE TABLE journal_entries (
    sequence       INTEGER PRIMARY KEY AUTOINCREMENT,
    transaction_id TEXT NOT NULL,
    ordinal        INTEGER NOT NULL CHECK (ordinal >= 0),
    kind           TEXT NOT NULL CHECK (length(kind) > 0),
    payload        BLOB NOT NULL,
    FOREIGN KEY (transaction_id)
        REFERENCES journal_transactions(transaction_id)
        ON DELETE CASCADE,
    UNIQUE (transaction_id, ordinal)
);
```

The Implementation Engineer may make only small SQL spelling/index changes that preserve these semantics.

No domain-object table may be added under this gate.

## 7.3 Schema invariants

- one journal transaction row per transaction ID;
- one or more entry rows per committed transaction;
- ordinal starts at zero and is contiguous for records written by the P0-T004 API;
- `sequence` is persistence-generated;
- committed rows are read in ascending `sequence`;
- transaction record order is reproducible from `ordinal`;
- public API provides no update/delete journal operation;
- journal is append-only for this task.

P0-T004 must not claim that SQLite `AUTOINCREMENT` values allocated by an aborted transaction are themselves an externally meaningful guarantee. Only committed persisted ordering is contractual.

---

# 8. SQLite connection policy

## AG-P0T004-009 — per-connection configuration

For the file-backed P0-T004 store:

```text
foreign_keys = ON
busy timeout = 5000 ms
journal_mode = WAL
synchronous = FULL
```

The implementation must verify the effective journal mode rather than assuming the PRAGMA succeeded.

The store is scoped to local filesystem testing for P0-T004.

Network-share behavior is explicitly out of scope.

## AG-P0T004-010 — threading

The Phase-0 persistence object/implementation is single-caller / externally serialized.

P0-T004 shall not design a concurrent writer scheduler.

No test result may be interpreted as authorizing arbitrary simultaneous access from multiple application threads.

---

# 9. Journal transaction semantics

## AG-P0T004-011 — atomic append

Appending one `JournalTransaction` means:

```text
BEGIN IMMEDIATE
  insert journal_transactions row
  insert all ordered journal_entries rows
COMMIT
```

If any validation, prepare, bind, step, constraint, or commit operation fails, the whole append fails and no partial transaction may be observable.

## AG-P0T004-012 — duplicate ID

A second append using an already-committed transaction ID must fail.

It must not append extra records to the original transaction.

## AG-P0T004-013 — rollback proof

P0-T004 must prove an uncommitted journal write is absent after the connection is closed and the database is reopened.

This is a rollback/uncommitted-close proof.

It is **not** represented as a power-loss or operating-system crash certification.

## AG-P0T004-014 — reopen durability

After a successful commit:

- close the SQLite connection;
- reopen the same database;
- read the committed journal;
- prove transaction ID, ordinal, kind, sequence ordering, and payload bytes are preserved.

## AG-P0T004-015 — binary-safe payload

At least one test payload must contain bytes that would expose accidental text handling, including embedded `0x00`.

Round-trip must be byte-exact.

---

# 10. Persistence API strategy

P0-T004 is a spike, not the final storage API.

Therefore the gate forbids prematurely freezing a large production repository interface.

The preferred implementation is:

- private SQLite connection/schema/journal classes under `src/persistence/src/**`;
- a narrow project-owned P0-T004 spike/evidence entry point under the persistence public namespace;
- neutral journal input types from `bim_transactions`;
- project-owned status/diagnostic output;
- no SQLite object or error-code leakage.

A full general-purpose `ProjectStore`, repository layer, query layer, autosave service, or document lifecycle API is deferred.

If Claude believes a larger public API is required, that is an Architecture Authority decision and implementation must stop before introducing it.

---

# 11. SQLite CMake resolution correction

P0-T004 is allowed to resolve the existing maintenance note in `src/persistence/CMakeLists.txt`.

The required preference order is:

```text
1. SQLite3::SQLite3
2. SQLite::SQLite3          (legacy compatibility fallback)
3. unofficial::sqlite3::sqlite3
4. otherwise fail configuration
```

This is a toolchain-maintenance correction within the existing SQLite decision and is **not** an ACR.

The frozen vcpkg registry baseline must not move.

No new persistence dependency may be added.

---

# 12. Architecture enforcement additions

P0-T004 shall extend the existing architecture checker rather than introduce a parallel checker.

## R12 — SQLite sole owner

Add selectable rule:

```text
SQLITE_PERSISTENCE_ONLY
```

It must reject actual first-party C/C++ SQLite API tokens outside:

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

Token matching must avoid known comment-only false-positive classes already learned in P0-T002/P0-T003.

## R13 — persistence public API neutral

Add selectable rule:

```text
PERSISTENCE_PUBLIC_NEUTRAL
```

It must scan:

```text
src/persistence/include/**
```

and reject SQLite API/type leakage.

## Required negative fixtures

Add controlled fixtures that prove the rules are real:

```text
tests/fixtures/p0_t004_bad_sqlite_owner/...
tests/fixtures/p0_t004_bad_persistence_api/...
```

Each negative fixture must be rejected by its exact rule and registered as a `WILL_FAIL TRUE` CTest test.

## Required architecture CTest names

Use exact-name anchored tests analogous to P0-T003:

```text
arch_sqlite_persistence_only
arch_p0_t004_sqlite_owner_fixture_rejected
arch_persistence_public_neutral
arch_p0_t004_persistence_api_fixture_rejected
```

`scripts/ci/architecture.ps1` must add these exact names to its required-test list.

---

# 13. Test contract

P0-T004 must add targeted tests without weakening any existing test.

Minimum required coverage:

## 13.1 Unit

```text
unit_transactions_journal_contract
```

Must verify neutral validation semantics:

- empty transaction ID rejected;
- zero records rejected;
- empty record kind rejected;
- valid binary payload accepted;
- record order preserved by the neutral contract.

## 13.2 Integration

At minimum:

```text
integration_persistence_schema_v1
integration_persistence_journal_commit_reopen
integration_persistence_journal_rollback
integration_persistence_journal_duplicate_id
integration_persistence_journal_binary_payload
```

The pre-existing:

```text
integration_persistence_sqlite_memory
```

must remain passing unless Architecture Authority explicitly approves its retirement. For this task it is expected to stay.

## 13.3 Architecture

The four R12/R13 tests listed in section 12 are mandatory.

## 13.4 Full regression

All P0-T001/P0-T002/P0-T003 registered tests must continue to pass.

No prior architecture rule may be weakened to make P0-T004 pass.

---

# 14. Standalone evidence executable

P0-T004 shall provide a standalone evidence executable, analogous in governance intent to prior Phase-0 evidence executables.

Suggested target:

```text
p0_t004_persistence_evidence
```

It shall create its database only under an explicitly supplied output/test directory, never inside the source tree.

The evidence JSON must contain deterministic factual fields including at least:

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

Exact JSON key spelling may be finalized in the implementation brief, but the semantic evidence above is mandatory.

A failed sub-check must make:

```text
overall_passed = false
```

and the executable must exit non-zero.

No evidence file may be hand-authored or treated as PASS without an executed run.

---

# 15. Failure and error policy

SQLite result codes are private diagnostic inputs.

They may appear inside project-owned error messages for debugging, but no public signature may expose an SQLite enum/type.

Required failure behavior is fail-closed.

Examples:

- open failure -> error;
- unsupported newer schema -> error;
- non-empty unrecognized version-zero DB -> error;
- schema validation mismatch -> error;
- transaction validation failure -> error before write;
- duplicate ID -> error with no partial write;
- bind/step/commit error -> rollback + error;
- journal-mode requirement not achieved -> spike failure;
- evidence inconsistency -> non-zero executable exit.

P0-T004 must not silently recreate, overwrite, truncate, or “repair” an unrecognized existing database.

---

# 16. Determinism requirements

P0-T004 acceptance must not depend on wall-clock timestamps.

Tests/evidence shall use fixed transaction IDs and fixed record payloads.

Journal order must be derived from persisted sequence/ordinal, not container/hash iteration order.

No random UUID generator is required by this task.

---

# 17. Filesystem and test isolation

All file-backed database tests must use unique temporary/build-tree locations.

They must not:

- write `.db`, `-wal`, or `-shm` files into the source tree;
- depend on a user-specific absolute path;
- reuse a stale DB from a previous run as a source of PASS;
- leave a stale DB able to make a later test pass accidentally.

Tests must remove or uniquely isolate prior outputs before use.

If cleanup cannot occur because SQLite still owns a handle, that is a test/implementation defect, not a reason to ignore cleanup.

---

# 18. Security/correctness constraints

Every SQL value derived from journal data must be bound with prepared statements.

String concatenation of caller-controlled transaction ID, kind, or payload into SQL is forbidden.

Schema SQL itself may be fixed compile-time SQL.

All SQLite statements must be finalized on every path.

All transactions must have explicit commit/rollback ownership.

The database handle must be closed on every path.

RAII private helpers are encouraged.

No `NOLINT`, warning suppression, disabled test, or architecture-fixture weakening may be used to bypass a defect without AA authorization.

---

# 19. Source footprint guidance

Expected implementation areas include:

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
docs/tasks/P0-T004/**
```

Possible controlled edits:

```text
CMakeLists.txt
tests/*/CMakeLists.txt
vcpkg.json
```

However:

- the vcpkg baseline must not change;
- no new third-party dependency is expected;
- any footprint outside the persistence/transactions/test/governance areas requires justification;
- changes to model/commands/query/viewport/geometry/desktop production code are presumptively out of scope.

---

# 20. Acceptance criteria

P0-T004 may be accepted only when all of the following are evidenced:

**AC-001** Baseline ancestry and task footprint are controlled.
**AC-002** SQLite API use exists only under `src/persistence/**`.
**AC-003** persistence public headers expose zero SQLite types/tokens.
**AC-004** transactions journal contract is SQLite-independent.
**AC-005** version-zero empty DB bootstraps atomically to schema version 1.
**AC-006** unsupported newer schema is rejected without mutation.
**AC-007** non-empty unrecognized version-zero DB is rejected without mutation.
**AC-008** required SQLite connection settings are verified.
**AC-009** one valid journal transaction commits atomically.
**AC-010** committed journal survives close/reopen.
**AC-011** uncommitted journal work is absent after close/reopen.
**AC-012** duplicate transaction ID is rejected atomically.
**AC-013** binary payload round-trips byte-exactly.
**AC-014** persisted journal ordering is deterministic.
**AC-015** R12 real-tree positive check passes.
**AC-016** R12 negative fixture is rejected.
**AC-017** R13 real-tree positive check passes.
**AC-018** R13 negative fixture is rejected.
**AC-019** standalone evidence executable reports `overall_passed=true` and exits 0.
**AC-020** existing persistence memory probe still passes.
**AC-021** full pre-existing test suite passes.
**AC-022** formatting/static-analysis gates pass for changed source.
**AC-023** no source-tree DB/WAL/SHM artifact is produced.
**AC-024** no dependency baseline change occurred.
**AC-025** no architecture change outside this approved gate occurred.
**AC-026** independent Kimi review returns `BLOCKER=0` and `MAJOR=0`.
**AC-027** Architecture Authority explicitly authorizes integration after verification/review.

---

# 21. Verification phases

The implementation brief shall separate verification into controlled phases.

## Phase A — baseline/preflight
- branch/worktree identity;
- frozen baseline;
- toolchain;
- clean index;
- dependency resolution.

## Phase B — architecture contract
- R12/R13 positive and negative fixtures;
- dependency direction;
- public-header token audit.

## Phase C — transactions contract
- neutral journal unit tests.

## Phase D — persistence schema
- bootstrap;
- version handling;
- unrecognized DB rejection;
- connection PRAGMAs.

## Phase E — journal semantics
- commit/reopen;
- rollback;
- duplicate ID;
- binary roundtrip;
- deterministic ordering.

## Phase F — evidence executable
- clean output location;
- JSON validation;
- exit-code consistency.

## Phase G — full regression
- complete configure/build/CTest;
- architecture CI job;
- format/static analysis;
- license inventory as applicable.

## Phase H — candidate freeze
- exact changed-path manifest;
- SHA256 identities;
- staged=0;
- main unchanged.

## Phase I — independent review
- Kimi read-only;
- minimum-delta correction only if genuine;
- acceptance requires BLOCKER=0 / MAJOR=0.

No implementation commit or main integration occurs before these gates close.

---

# 22. Deferred items recorded from earlier tasks

P0-T004 must not opportunistically rewrite unrelated P0-T003 deferred findings.

The following remain separate unless a direct P0-T004 touch makes a minimum documentation correction unavoidable and AA explicitly approves it:

```text
P0T003-R2-MIN-01
P0T003-R2-NOTE-01
P0T003-R2-NOTE-02
P0T003-R2-NOTE-03
```

The SQLite target-name maintenance note from P0-T001 **is** directly relevant to `src/persistence/CMakeLists.txt` and is included in this gate under section 11.

---

# 23. Architecture Change Request status

**ACR = NONE**

Rationale:

- SQLite was already selected and linked behind `bim_persistence`;
- persistence was already authorized to depend on model + transactions + foundation + SQLite;
- P0-T004 was already reserved for schema philosophy and journal proof;
- the new work makes that existing boundary real rather than replacing it.

An ACR becomes necessary if implementation proposes, for example:

- replacing SQLite;
- adding another database engine;
- reversing module dependencies;
- exposing SQLite through public interfaces;
- introducing a new third-party persistence/serialization framework;
- moving persistence responsibility into model/commands/query;
- changing the frozen vcpkg baseline solely to make P0-T004 work.

---

# 24. Architecture Authority disposition

Discovery status:

```text
READY_FOR_ARCHITECTURE_GATE_DRAFT
```

Architecture Gate v1.0 disposition:

```text
APPROVED / FROZEN
IMPLEMENTATION NOT YET AUTHORIZED
```

Following Architecture Authority approval on 2026-09-14, the next controlled steps are:

1. materialize `task/P0-T004-persistence-spike` from the locked `main` baseline;
2. create `D:\Projects\BIM-Platform-WT-P0-T004`;
3. record the approved architecture gate and task record as governance-only changes;
4. commit the architecture-gate governance delta;
5. issue the implementation brief to Claude;
6. independently review the brief for minimum necessary scope;
7. record explicit implementation authorization;
8. only then begin source implementation.

---

## End of P0-T004 Architecture Gate Package v1.0
