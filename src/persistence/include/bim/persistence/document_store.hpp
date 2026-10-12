#pragma once

#include "bim/foundation/status.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/transactions/journal.hpp"

#include <filesystem>
#include <vector>

// P1-T004 Persistence Schema v2 - the project-owned, storage-neutral document
// store API (Architecture Gate BIM-AG-P1-T004 sections 9 and 13; Implementation
// Brief BIM-IB-P1-T004 sections 3, 10 and 12).
//
// The store persists the authoritative BIM state of one document - its Levels
// and StraightWalls, identified by canonical ElementId text - and it appends the
// document's pending transaction-journal records in the same atomic step. It
// persists nothing derived or runtime-only: no runtime graph identity, no
// identity map, no dependency-graph state, no recompute plan or snapshot, no
// derived wall geometry and no kernel or viewport value. Those are rebuilt by
// the document layer when a document is opened.
//
// Only project-owned values cross this boundary (bim::model values, the neutral
// bim::transactions journal values and bim::foundation::Status). No storage
// engine type, handle or result code appears in this header; the mechanical
// architecture checker (rules R13 and R21) enforces that.
//
// The store is single-threaded / externally serialized: the caller must not
// save to or load from one path concurrently.

namespace bim::persistence {

// The complete authoritative BIM state of one document. Order inside the two
// vectors carries no meaning for SaveDocument (it sorts internally); LoadDocument
// returns both vectors in ElementId ascending order, the canonical byte order of
// bim::model::ElementId.
struct DocumentSnapshot {
    std::vector<bim::model::Level> levels;
    std::vector<bim::model::StraightWall> straight_walls;
};

// Atomically replaces the durable snapshot at `path` with `snapshot` and appends
// every transaction of `pending_journal` to the durable journal, in one
// transaction: either the complete new snapshot AND all of the pending journal
// become visible together, or none of it does.
//
// A missing database file is created with the complete schema; a database with
// an older supported schema is migrated atomically first. Committed journal
// history is never deleted or rewritten.
//
// Everything is validated BEFORE any durable mutation: every Level and
// StraightWall must satisfy the production model validation, every ElementId
// must be valid and canonical, Levels and StraightWalls must share one
// ElementId namespace without duplicates, every wall's hosting Level must be
// present in the snapshot, and every pending journal transaction must satisfy
// bim::transactions::ValidateJournalTransaction with unique ids. A pending
// transaction id that already exists in the durable journal is rejected by the
// store and rolls the whole save back.
//
// On any failure the previously committed database content is unchanged, no
// partial snapshot and no partial journal becomes visible, and a failure Status
// is returned. `snapshot` and `pending_journal` are never modified.
[[nodiscard]] bim::foundation::Status
SaveDocument(const std::filesystem::path& path, const DocumentSnapshot& snapshot,
             const std::vector<bim::transactions::JournalTransaction>& pending_journal);

// Loads the durable snapshot at `path` into `out_snapshot`. The database must
// exist; its schema is validated, and a valid older-schema database is migrated
// atomically (a migrated older database holds no BIM state, so it loads as an
// empty snapshot).
//
// The loaded content is validated before it is returned: every persisted
// ElementId must parse and re-serialize to exactly the stored canonical
// lowercase text (a non-canonical id is rejected, never normalized), every
// element must have exactly one matching kind record, every Level and
// StraightWall must satisfy the production model validation, every hosting
// Level must exist, and the durable journal must be well-formed. `out_snapshot`
// is assigned only after every check succeeded; on failure it is left unchanged.
[[nodiscard]] bim::foundation::Status LoadDocument(const std::filesystem::path& path,
                                                   DocumentSnapshot& out_snapshot);

} // namespace bim::persistence
