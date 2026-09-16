#pragma once

#include "sqlite_connection.hpp"

#include "bim/foundation/status.hpp"
#include "bim/transactions/journal.hpp"

#include <vector>

namespace bim::persistence::detail {

// Persists and reads the neutral Phase B journal contract
// (bim::transactions::JournalTransaction) on top of the frozen Phase D
// schema v1 (Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 Phase E;
// Architecture Gate AG-P0T004-005). `db` must already have schema v1
// established (see schema_v1.hpp::OpenOrCreateSchemaV1) - neither
// function here opens, creates, or bootstraps a database itself. Neither
// function reinterprets JournalRecord::kind or JournalRecord::payload:
// both remain opaque to this layer, exactly as the Phase B contract
// requires.

// Appends `transaction` atomically:
//   1. bim::transactions::ValidateJournalTransaction(transaction) must
//      succeed first; on failure, returns immediately and performs no
//      database access.
//   2. BEGIN IMMEDIATE.
//   3. INSERT one journal_transactions row for transaction.id.
//      transaction_id is the schema's PRIMARY KEY, so a prior committed
//      transaction using the same id makes this INSERT fail - no silent
//      replace, merge, update, or upsert exists anywhere in this path.
//   4. INSERT one journal_entries row per record, in `transaction.records`
//      order, with ordinal 0..N-1 exactly matching that order. All values
//      (including `payload`, bound as a BLOB of its exact length - an
//      empty payload binds as a valid zero-length BLOB) are bound via
//      prepared-statement parameters; no caller data is ever concatenated
//      into SQL text.
//   5. COMMIT on full success. On any failure from step 2 onward, issues
//      ROLLBACK and returns the original failure Status regardless of the
//      rollback's own outcome - no per-record commits, and every row for
//      one call commits together or not at all.
// `transaction` is never mutated.
[[nodiscard]] bim::foundation::Status
AppendJournalTransaction(sqlite3* db, const bim::transactions::JournalTransaction& transaction);

// Reconstructs every committed journal transaction on `db` in
// deterministic order: transactions are ordered by their first committed
// entry's journal_entries.sequence (never lexical id order, never
// SQLite's implicit row order - see journal_entries.sequence ASC), and
// each transaction's records are placed in exact persisted ordinal order.
// No bim_persistence-internal sequence value is exposed in the returned
// JournalTransaction values.
//
// Fails closed - returns an error Status and leaves `out_transactions`
// unmodified - if the persisted data violates any frozen invariant: a
// transaction with zero entries, a non-contiguous or duplicate ordinal
// sequence within a transaction, or an empty record kind.
[[nodiscard]] bim::foundation::Status
ReadJournalTransactions(sqlite3* db,
                        std::vector<bim::transactions::JournalTransaction>& out_transactions);

} // namespace bim::persistence::detail
