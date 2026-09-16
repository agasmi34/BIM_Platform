#include "journal_store.hpp"

#include "sqlite_statement.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bim::persistence::detail {

namespace {

// One persisted journal_entries row, pending assembly into its owning
// JournalTransaction. `ordinal` is retained only for in-memory
// reconstruction; it never appears in the returned JournalTransaction/
// JournalRecord values (Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0
// Phase E section 10: "no database sequence value leaks into the neutral
// public journal contract" - the same holds for the ordinal column).
struct PendingRecord {
    std::int64_t ordinal;
    std::string kind;
    std::vector<std::byte> payload;
};

// Executes a fixed, first-party-authored statement that returns no row
// (BEGIN/COMMIT/ROLLBACK). `sql` must never be built from caller-controlled
// input. Deliberately not shared with schema_v1.cpp's own identically-named
// anonymous-namespace helper: each file's private helper is self-contained
// (no header exposes either), so there is no ODR concern and no coupling
// between them.
[[nodiscard]] bim::foundation::Status ExecuteFixedStatement(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status
InsertJournalTransactionRow(sqlite3* db, const std::string& transaction_id) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "INSERT INTO journal_transactions (transaction_id) VALUES (?1);", statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, transaction_id);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status
InsertJournalEntryRow(sqlite3* db, const std::string& transaction_id, std::int64_t ordinal,
                      const bim::transactions::JournalRecord& record) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db,
        "INSERT INTO journal_entries (transaction_id, ordinal, kind, payload) VALUES (?1, ?2, "
        "?3, ?4);",
        statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, transaction_id);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindInt64(2, ordinal);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(3, record.kind);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindBlob(4, record.payload);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status CountJournalTransactions(sqlite3* db,
                                                               std::int64_t& out_count) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "SELECT count(*) FROM journal_transactions;", statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    out_count = has_row ? statement.ColumnInt64(0) : 0;
    return bim::foundation::Status::Ok();
}

} // namespace

bim::foundation::Status
AppendJournalTransaction(sqlite3* db, const bim::transactions::JournalTransaction& transaction) {
    bim::foundation::Status validation = bim::transactions::ValidateJournalTransaction(transaction);
    if (!validation.ok()) {
        return validation;
    }

    bim::foundation::Status status = ExecuteFixedStatement(db, "BEGIN IMMEDIATE;");
    if (!status.ok()) {
        return status;
    }

    status = InsertJournalTransactionRow(db, transaction.id);
    for (std::size_t ordinal = 0; status.ok() && ordinal < transaction.records.size(); ++ordinal) {
        status = InsertJournalEntryRow(db, transaction.id, static_cast<std::int64_t>(ordinal),
                                       transaction.records[ordinal]);
    }

    if (!status.ok()) {
        // Preserve the original failure as the returned Status regardless
        // of whether the rollback itself succeeds - the caller needs to
        // know what actually went wrong first. Every row inserted by this
        // call (the transaction row and any entry rows so far) is undone
        // together; there are no per-record commits.
        const bim::foundation::Status rollback_status = ExecuteFixedStatement(db, "ROLLBACK;");
        (void)rollback_status;
        return status;
    }

    return ExecuteFixedStatement(db, "COMMIT;");
}

bim::foundation::Status
ReadJournalTransactions(sqlite3* db,
                        std::vector<bim::transactions::JournalTransaction>& out_transactions) {
    std::int64_t transaction_count = 0;
    bim::foundation::Status status = CountJournalTransactions(db, transaction_count);
    if (!status.ok()) {
        return status;
    }

    SqliteStatement statement;
    status = SqliteStatement::Prepare(
        db,
        "SELECT transaction_id, ordinal, kind, payload FROM journal_entries ORDER BY sequence ASC;",
        statement);
    if (!status.ok()) {
        return status;
    }

    // First-seen order while scanning journal_entries.sequence ASC is
    // exactly "ordered by first committed entry sequence": a single
    // AppendJournalTransaction call commits its transaction's entries
    // together, in ordinal order, under the frozen Phase-0
    // single-caller/externally-serialized assumption, so a transaction's
    // first-encountered entry here is its lowest-sequence (ordinal 0)
    // entry.
    std::vector<std::string> transaction_order;
    std::unordered_map<std::string, std::vector<PendingRecord>> entries_by_transaction;

    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }

        std::string transaction_id = statement.ColumnText(0);
        const std::int64_t ordinal = statement.ColumnInt64(1);
        std::string kind = statement.ColumnText(2);
        std::vector<std::byte> payload = statement.ColumnBlob(3);

        if (kind.empty()) {
            return bim::foundation::Status::Error(
                "journal read: a persisted entry for transaction \"" + transaction_id +
                "\" has an empty kind, violating the frozen journal contract");
        }
        if (ordinal < 0) {
            return bim::foundation::Status::Error(
                "journal read: a persisted entry for transaction \"" + transaction_id +
                "\" has a negative ordinal, violating the frozen journal contract");
        }

        auto [entry, inserted] = entries_by_transaction.try_emplace(transaction_id);
        if (inserted) {
            transaction_order.push_back(transaction_id);
        }
        entry->second.push_back(PendingRecord{ordinal, std::move(kind), std::move(payload)});
    }

    if (static_cast<std::int64_t>(transaction_order.size()) != transaction_count) {
        return bim::foundation::Status::Error(
            "journal read: journal_transactions contains a transaction with zero persisted "
            "entries, violating the frozen journal contract");
    }

    std::vector<bim::transactions::JournalTransaction> result;
    result.reserve(transaction_order.size());

    for (const std::string& transaction_id : transaction_order) {
        std::vector<PendingRecord>& pending = entries_by_transaction[transaction_id];

        bim::transactions::JournalTransaction transaction;
        transaction.id = transaction_id;
        transaction.records.resize(pending.size());

        std::vector<bool> ordinal_seen(pending.size(), false);
        for (PendingRecord& record : pending) {
            const auto index = static_cast<std::size_t>(record.ordinal);
            if (index >= pending.size() || ordinal_seen[index]) {
                return bim::foundation::Status::Error(
                    "journal read: transaction \"" + transaction_id +
                    "\" has a non-contiguous or duplicate ordinal sequence, violating the "
                    "frozen journal contract");
            }
            ordinal_seen[index] = true;
            transaction.records[index].kind = std::move(record.kind);
            transaction.records[index].payload = std::move(record.payload);
        }

        result.push_back(std::move(transaction));
    }

    out_transactions = std::move(result);
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence::detail
