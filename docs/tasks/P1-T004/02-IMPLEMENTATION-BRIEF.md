# P1-T004 - Implementation Brief

Implementation Brief ID: BIM-IB-P1-T004
Architecture: BIM-AG-P1-T004 APPROVED / FROZEN
Status: PREPARED - IMPLEMENTATION NOT AUTHORIZED
Date: 2026-10-10

Baseline:
104b0c9f82cf304cdd16ee0178f0d69486cd3b3b

Tree:
5e9518ddc4470b26094189a04aa0aa46bba8b275

## 1. Exact implementation footprint

Exactly 23 paths.

### Modified existing - 13

1. src/persistence/CMakeLists.txt
2. src/persistence/src/detail/journal_store.cpp
3. src/persistence/src/detail/journal_store.hpp
4. src/persistence/src/detail/schema_v1.cpp
5. src/persistence/src/detail/schema_v1.hpp
6. src/persistence/src/detail/sqlite_statement.cpp
7. src/persistence/src/detail/sqlite_statement.hpp
8. src/document/CMakeLists.txt
9. src/document/include/bim/document/document.hpp
10. src/document/src/document.cpp
11. tests/integration/CMakeLists.txt
12. tests/architecture/CMakeLists.txt
13. tools/architecture_checker.cmake

### New - 10

14. src/persistence/include/bim/persistence/document_store.hpp
15. src/persistence/src/document_store.cpp
16. src/persistence/src/detail/schema_v2.hpp
17. src/persistence/src/detail/schema_v2.cpp
18. tests/integration/integration_p1_t004_schema_v2.cpp
19. tests/integration/integration_p1_t004_save_reopen.cpp
20. tests/integration/integration_p1_t004_save_atomicity.cpp
21. tests/integration/integration_p1_t004_journal_coordination.cpp
22. tests/fixtures/p1_t004_bad_persistence_document_dependency/persistence/src/bad.cpp
23. tests/fixtures/p1_t004_bad_document_lifecycle_boundary/document/CMakeLists.txt

Deleted paths: NONE.

Any need to modify any other path is a STOP and return to Architecture Authority.

## 2. Explicitly outside implementation footprint

Do not modify:

- CMakeLists.txt at repository root;
- src/persistence/include/bim/persistence/probe.hpp;
- src/persistence/src/persistence_probe.cpp;
- src/persistence/src/detail/sqlite_connection.hpp;
- src/persistence/src/detail/sqlite_connection.cpp;
- src/transactions/**;
- src/commands/**;
- src/query/**;
- model/**;
- dependency_graph/**;
- geometry/**;
- viewport/**;
- desktop/**.

Historical P0-T004 persistence-spike behavior must continue passing unchanged.

## 3. Public persistence API

Create:

src/persistence/include/bim/persistence/document_store.hpp

Frozen semantic surface:

namespace bim::persistence {

struct DocumentSnapshot {
    std::vector<bim::model::Level> levels;
    std::vector<bim::model::StraightWall> straight_walls;
};

[[nodiscard]] bim::foundation::Status
SaveDocument(
    const std::filesystem::path& path,
    const DocumentSnapshot& snapshot,
    const std::vector<bim::transactions::JournalTransaction>& pending_journal);

[[nodiscard]] bim::foundation::Status
LoadDocument(
    const std::filesystem::path& path,
    DocumentSnapshot& out_snapshot);

}

No SQLite type enters this header.

## 4. Document lifecycle API

Add a result contract separate from DocumentResultCode.

Frozen result codes:

Ok
PersistenceFailure
RuntimeCapacityExhausted
DerivedGeometryInvalid
RecomputeFailed
InternalFailure

Add lifecycle methods equivalent to:

[[nodiscard]]
DocumentLifecycleResult Save(
    const std::filesystem::path& path) noexcept;

[[nodiscard]]
static DocumentLifecycleResult Open(
    const std::filesystem::path& path,
    Document& out_document) noexcept;

Do not add PersistenceFailure to DocumentResultCode.

Do not modify commands mapping.

## 5. Private Document state

Extend detail::State with a private ordered pending journal queue:

std::vector<bim::transactions::JournalTransaction> pending_journal;

It participates in the same whole-State staged-copy / atomic-publication law as
domain state.

Each successful mutation appends one transaction to the staged pending queue
before staged state publication.

Any journal-transaction ID generation or allocation failure rejects the complete
mutation.

Open reconstruction itself does NOT create journal entries.

Save itself does NOT create journal entries.

## 6. P1-T004 journal record contract

Kinds:

bim.document.add_level.v1
bim.document.update_level_elevation.v1
bim.document.add_straight_wall.v1
bim.document.update_straight_wall.v1
bim.document.delete_element.v1

Exactly one record per Document mutation.

Payload:

exactly 16 bytes from the target ElementId.

Transaction ID:

canonical text from a newly generated project ElementId / UUID-v4 identity.

No journal replay implementation.

## 7. SQLite statement extension

Add only the missing numeric primitives required by schema v2:

BindDouble(index, double)

ColumnDouble(column)

Preserve all existing text/blob/int64 semantics.

No raw SQLite use outside persistence.

## 8. Schema-v1 refactor

Preserve every historical schema-v1 behavior.

Extract/reuse only the private primitives required so schema-v2 can:

- build the existing v1 journal tables without a separate observable v1 commit;
- validate historical v1 before migration.

OpenOrCreateSchemaV1 remains behaviorally unchanged for all P0-T004 tests.

## 9. Schema-v2 private API

Implement private equivalents of:

kSchemaV2Version = 2

OpenOrCreateSchemaV2(path, connection)

ValidateSchemaV2(db)

BootstrapSchemaV2(db)

MigrateSchemaV1ToV2(db)

The 0 -> 2 bootstrap creates journal-v1 tables plus the complete v2 BIM tables
inside one BEGIN IMMEDIATE / COMMIT transaction.

The 1 -> 2 migration:

- validates v1 first;
- begins one transaction;
- creates only new v2 BIM state;
- inserts metadata;
- validates complete v2 structure;
- sets user_version = 2;
- commits.

Any failure rolls back.

## 10. Document store save

Before database mutation validate:

- all Level values;
- all StraightWall values;
- ElementId validity/canonical serialization;
- one cross-kind ElementId namespace;
- every wall host exists;
- all pending JournalTransactions;
- deterministic ElementId ordering.

Open/migrate schema v2.

Then one BEGIN IMMEDIATE transaction performs:

1. remove old straight_walls;
2. remove old levels;
3. remove old elements;
4. insert complete authoritative elements/levels/walls snapshot;
5. append all pending journal transactions using a non-transaction-owning private
   journal insertion primitive;
6. COMMIT.

Never delete existing journal history.

On failure ROLLBACK.

## 11. Journal-store refactor

Keep the existing public/private behavior of:

AppendJournalTransaction(...)

ReadJournalTransactions(...)

Add a private insertion primitive that writes one already-validated
JournalTransaction using an existing caller-owned SQLite transaction.

Existing AppendJournalTransaction becomes:

validate
BEGIN IMMEDIATE
insert-within-current-transaction
COMMIT

with the same rollback behavior as before.

Document store uses the inner primitive while owning the wider snapshot+journal
transaction.

## 12. Document store load

Open/migrate schema v2.

Read durable BIM values in deterministic ElementId order.

Reject:

- malformed/non-canonical IDs;
- duplicate identity;
- mismatched element_type/subtable representation;
- invalid Level;
- invalid StraightWall;
- missing host Level;
- malformed metadata;
- malformed journal state.

Assign out_snapshot only after complete success.

## 13. Document Open reconstruction

Build a temporary Document.

Populate Levels first in ascending ElementId order.

Populate StraightWalls second in ascending ElementId order.

Use the existing staged domain/recompute primitives without generating new journal
records.

Fresh runtime NodeIds begin from the normal runtime allocator state.

Rebuild graph dependencies and wall_geometry.

Validate invariants.

Only then move-assign the temporary Document into out_document.

Any failure leaves out_document unchanged.

## 14. Architecture rules

Add R21:

PERSISTENCE_DOCUMENT_STORE_BOUNDARY

Add R22:

DOCUMENT_LIFECYCLE_BOUNDARY

CTest names exactly:

arch_persistence_document_store_boundary

arch_p1_t004_persistence_document_dependency_fixture_rejected

arch_document_lifecycle_boundary

arch_p1_t004_document_lifecycle_fixture_rejected

Both negative fixtures use WILL_FAIL TRUE.

R1-R20 semantics remain unchanged.

## 15. New runtime CTest registrations

Exactly four:

integration_p1_t004_schema_v2

integration_p1_t004_save_reopen

integration_p1_t004_save_atomicity

integration_p1_t004_journal_coordination

No new unit CTest executable is required.

## 16. Expected CTest totals

Baseline:
68 total
30 architecture

P1-T004 additions:
4 runtime
4 architecture

Expected after P1-T004:
76 total
34 architecture

Focused P1-T004 runtime:
4 / 4

Architecture:
34 / 34

Full:
76 / 76

Existing P0-T004 schema-v1 and journal regression tests must remain PASS.

P1-T003 runtime regression:
2 / 2

P1-T002:
2 / 2

P1-T001:
3 / 3

P0-T009:
2 / 2

BIM_ENABLE_DWG=OFF
BIM_ENABLE_IFC=OFF

Windows native validation remains authoritative.

## 17. Implementation sequence

Phase A:
CMake and private SQLite statement primitives.

Phase B:
schema-v1 internal reuse plus complete schema-v2 bootstrap/migration.

Phase C:
document-store snapshot save/load and journal inner append primitive.

Phase D:
Document lifecycle + pending journal integration + atomic reopen reconstruction.

Phase E:
integration tests, R21/R22, negative fixtures.

Do not stage, commit, push or request Kimi review during implementation.

## 18. Stop conditions

STOP and return to Architecture Authority if implementation requires:

- any 24th implementation path;
- root CMake modification;
- commands/query/transactions modification;
- new dependency;
- public SQLite type;
- NodeId persistence;
- journal replay;
- undo/redo;
- schema shape incompatible with this brief;
- new BIM domain semantics;
- non-atomic save/migration;
- ACR.

ACR currently:

NONE.
