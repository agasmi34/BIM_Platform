#pragma once

#include "bim/foundation/status.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bim::transactions {

// Neutral, SQLite-independent transaction-journal contract (Architecture
// Gate BIM-AG-P0-T004 v1.0 section 5 / AG-P0T004-003; Implementation Brief
// BIM-TASK-P0-T004-CLAUDE v1.0 section 4). `id`, `kind`, and `payload` are
// opaque Phase-0 values: this module performs no I/O, defines no
// timestamp, and does not claim to be the final product undo/redo
// contract. bim_transactions must not depend on SQLite or bim_persistence.

// One opaque, ordered journal record within a JournalTransaction.
struct JournalRecord {
    // Non-empty, opaque stable string key. Interpreted only by callers
    // above this module; persistence must not interpret it.
    std::string kind;

    // Opaque byte sequence. May be empty. Never interpreted here.
    std::vector<std::byte> payload;
};

// An ordered, uniquely-identified group of one or more JournalRecord
// values. Caller-provided record order defines ordinal 0..N-1 and is
// preserved by ValidateJournalTransaction.
struct JournalTransaction {
    // Opaque, non-empty transaction identifier. Uniqueness is enforced by
    // the persistence layer in a later phase, not by this contract.
    std::string id;

    // Ordered records; must contain at least one entry.
    std::vector<JournalRecord> records;
};

// Validates the neutral journal-transaction contract without performing
// any I/O:
//   - `transaction.id` must be non-empty;
//   - `transaction.records` must contain at least one record;
//   - every record's `kind` must be non-empty;
//   - `payload` is unconstrained opaque bytes (may be empty).
// Never mutates or reorders `transaction`.
[[nodiscard]] bim::foundation::Status
ValidateJournalTransaction(const JournalTransaction& transaction);

} // namespace bim::transactions
