# P1-T004 - Architecture Gate

Architecture ID: BIM-AG-P1-T004
Status: APPROVED / FROZEN
Date: 2026-10-10

Baseline commit:
104b0c9f82cf304cdd16ee0178f0d69486cd3b3b

Baseline tree:
5e9518ddc4470b26094189a04aa0aa46bba8b275

## 1. Ownership

P1-T004 owns:

- SQLite schema version 2;
- direct atomic empty-DB 0 -> 2 bootstrap;
- atomic valid schema-v1 1 -> 2 migration;
- Level and StraightWall durable authoritative state;
- canonical ElementId persistence;
- save/reopen lifecycle;
- runtime graph reconstruction inputs;
- regenerated derived geometry after reopen;
- durable coordination with the existing append-only journal.

## 2. Dependency law

persistence
  -> foundation
  -> model
  -> transactions
  -> SQLite

document
  -> model
  -> geometry_api
  -> dependency_graph
  -> transactions
  -> persistence

commands
  -> document

query
  -> document
  -> model

Forbidden:

- persistence -> document
- persistence -> commands
- persistence -> query
- commands -> persistence
- commands -> transactions
- query -> persistence

Document links transactions and persistence privately only.

No persistence, transaction, SQLite or runtime graph type may enter the public
Document contract.

## 3. Schema authority

PRAGMA user_version is the sole schema-version authority.

Current version:

user_version = 2

No competing metadata field may claim schema-version authority.

## 4. Version handling

user_version == 0 with no user tables:
  bootstrap complete schema v2 atomically.

user_version == 0 with any pre-existing user table:
  fail closed with zero mutation.

user_version == 1:
  validate the frozen schema-v1 journal structure, then migrate atomically to v2.

user_version == 2:
  validate complete schema v2 before use.

user_version > 2 or invalid negative version:
  fail closed.

Migration failure must leave version 1 and its committed journal fully intact.

## 5. Version-2 authoritative schema

Version 2 preserves the existing schema-v1 journal tables and adds:

document_metadata

elements

levels

straight_walls

Conceptual columns:

document_metadata:
  singleton INTEGER PRIMARY KEY CHECK(singleton = 1)
  format_tag TEXT NOT NULL

elements:
  element_id TEXT PRIMARY KEY NOT NULL
  element_type INTEGER NOT NULL CHECK(element_type IN (1,2))

levels:
  element_id TEXT PRIMARY KEY NOT NULL
  elevation REAL NOT NULL
  FK element_id -> elements(element_id) ON DELETE CASCADE

straight_walls:
  element_id TEXT PRIMARY KEY NOT NULL
  level_id TEXT NOT NULL
  start_x REAL NOT NULL
  start_y REAL NOT NULL
  end_x REAL NOT NULL
  end_y REAL NOT NULL
  thickness REAL NOT NULL
  base_offset REAL NOT NULL
  height REAL NOT NULL
  FK element_id -> elements(element_id) ON DELETE CASCADE
  FK level_id -> levels(element_id) ON DELETE RESTRICT

Element type encoding:

1 = Level
2 = StraightWall

The elements table supplies one cross-kind ElementId namespace.

## 6. ElementId law

ElementId is persisted in the production canonical textual form:

xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

Writes use exactly 36 lowercase canonical characters.

Reads must:

1. parse with the production ElementId parser;
2. reject invalid all-zero identity;
3. serialize the parsed ID again;
4. require byte-for-byte equality with the stored canonical lowercase text.

Non-canonical persisted IDs are rejected, not normalized.

## 7. Persisted authoritative BIM values

Level:

- ElementId
- elevation

StraightWall:

- ElementId
- level_id
- start.x
- start.y
- end.x
- end.y
- thickness
- base_offset
- height

Production model validation remains authoritative after decode.

## 8. State explicitly not persisted

Never persist:

- NodeId;
- ElementId <-> NodeId maps;
- next_node_value;
- DependencyGraph object/state;
- RecomputePlan;
- GraphSnapshot;
- wall_geometry cache;
- OCCT objects;
- viewport/GPU data;
- transient face indices.

These are reconstructed or regenerated.

## 9. Persistence public boundary

P1-T004 introduces project-owned public persistence values equivalent to:

DocumentSnapshot {
  vector<Level> levels;
  vector<StraightWall> straight_walls;
}

SaveDocument(path, snapshot, pending_journal)

LoadDocument(path, out_snapshot)

No SQLite type crosses this API.

Persistence may expose model and neutral transaction values required by the store.

## 10. Document lifecycle boundary

Document receives lifecycle capabilities equivalent to:

DocumentLifecycleResult Save(const filesystem::path&) noexcept

static DocumentLifecycleResult Open(
    const filesystem::path&,
    Document& out_document) noexcept

Lifecycle uses a separate result contract.

DocumentResultCode used by mutation/commands is NOT extended for persistence.

Therefore commands receives no P1-T004 source modification.

Open failure leaves out_document unchanged.

Save failure leaves the in-memory Document and its pending journal unchanged.

## 11. Runtime reconstruction

Successful Open:

1. loads and validates the durable snapshot;
2. creates a fresh staged Document state;
3. inserts Levels in ElementId order;
4. inserts StraightWalls in ElementId order;
5. allocates fresh runtime NodeIds;
6. rebuilds Level -> StraightWall dependencies;
7. regenerates all neutral derived wall geometry;
8. validates normal Document invariants;
9. atomically publishes the reconstructed state.

Runtime NodeId continuity across reopen is explicitly not guaranteed.

## 12. Durable journal coordination

Every successful production Document mutation prepares exactly one private pending
JournalTransaction as part of the same staged Document state.

Failed mutation publishes no journal record.

Record kinds:

bim.document.add_level.v1
bim.document.update_level_elevation.v1
bim.document.add_straight_wall.v1
bim.document.update_straight_wall.v1
bim.document.delete_element.v1

Each P1-T004 transaction contains exactly one record.

Payload is exactly the target ElementId's 16 authoritative bytes.

The transaction ID is a canonical UUID-v4 string generated using the existing
project-owned ElementId generator.

Journal data is audit / transaction-boundary information.

It is NOT authoritative BIM state, a replay source, or undo/redo state.

## 13. Save atomicity

One successful Save persists in one SQLite transaction:

- the complete authoritative BIM snapshot;
- every currently pending Document journal transaction.

Existing committed journal rows are preserved.

Either snapshot plus pending journal commit together or neither becomes visible.

On failure:

- durable prior snapshot remains intact;
- no partial pending journal becomes visible;
- in-memory Document remains unchanged;
- pending journal remains pending.

Only after successful durable commit may Document clear the corresponding pending
journal queue.

An unchanged save creates no new journal transaction.

## 14. Version-1 compatibility

A valid v1 DB migrates to v2 and preserves every committed journal row.

Because v1 contained no durable BIM element tables, migration yields an empty BIM
snapshot.

No BIM state is inferred by replaying historical journal payloads.

## 15. Architecture enforcement

Existing R1-R20 remain semantically unchanged.

R21 - PERSISTENCE_DOCUMENT_STORE_BOUNDARY

Allows persistence dependencies on:

- foundation;
- model;
- transactions;
- SQLite.

Rejects persistence coupling to:

- document;
- commands;
- query;
- dependency_graph;
- geometry implementation;
- viewport;
- desktop;
- Qt;
- bgfx;
- OCCT;
- ODA;
- IfcOpenShell.

R22 - DOCUMENT_LIFECYCLE_BOUNDARY

Allows private document dependencies on:

- transactions;
- persistence.

Rejects exposure of transaction/persistence/SQLite/runtime implementation details
through public Document headers and rejects PUBLIC CMake coupling of
transactions/persistence.

Existing R18 remains authoritative for Document public runtime neutrality.

Each R21/R22 rule receives one real-tree positive test and one WILL_FAIL negative
fixture.

## 16. Exclusions

Not authorized:

- undo/redo;
- journal replay;
- event sourcing;
- autosave;
- background persistence;
- delta/incremental persistence;
- cloud sync;
- multi-user persistence;
- collaboration;
- new BIM element kinds;
- wall rehost or cascade semantics changes;
- NodeId persistence;
- derived geometry persistence;
- IFC/DWG/RVT persistence;
- new third-party dependencies;
- parallel mutation/save.

## 17. Threading

Single-threaded / externally serialized only.

## 18. ACR

ACR = NONE.

The Phase-1 authority explicitly permits Document to coordinate persistence and
transactions privately. This task therefore advances the frozen architecture and
does not conflict with it.
