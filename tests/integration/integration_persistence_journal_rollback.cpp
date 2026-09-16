#include "persistence_journal_test_support.hpp"

#include "bim/foundation/status.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

TEST_CASE("journal append rolls back completely when an entry insert fails mid-transaction, "
          "leaving no partial transaction after close/reopen",
          "[integration][persistence][journal]") {
    bim_p0_t004_test::TempDatabasePath db_path;

    {
        bim::persistence::detail::SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());

        // Test-only trigger, created only in this temporary test database
        // after normal schema-v1 creation/validation, using the same
        // private fixed-SQL infrastructure AppendJournalTransaction itself
        // uses (Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 Phase E
        // section 12.4). It is never part of production schema bootstrap -
        // schema_v1.cpp is untouched by Phase E - and exists solely to
        // force a normal SQLite failure partway through one
        // AppendJournalTransaction call (after its journal_transactions
        // row insert, during its first journal_entries row insert), so the
        // rollback path is proven via ordinary SQLite behavior rather than
        // a production test hook. RAISE(ABORT, ...) fails only the
        // triggering INSERT statement; AppendJournalTransaction's own
        // ROLLBACK (not SQLite's automatic per-statement undo) is what is
        // being exercised and proven here.
        REQUIRE(bim_p0_t004_test::RunFixedStatement(
                    connection.handle(), "CREATE TRIGGER bim_test_force_entry_failure "
                                         "BEFORE INSERT ON journal_entries "
                                         "WHEN NEW.transaction_id = 'rollback-target' "
                                         "BEGIN SELECT RAISE(ABORT, 'induced test failure'); END;")
                    .ok());

        bim::transactions::JournalTransaction transaction;
        transaction.id = "rollback-target";
        transaction.records = {
            bim::transactions::JournalRecord{"kind-one", std::vector<std::byte>{std::byte{0x01}}},
            bim::transactions::JournalRecord{"kind-two", std::vector<std::byte>{std::byte{0x02}}},
        };

        const bim::foundation::Status append_status =
            bim::persistence::detail::AppendJournalTransaction(connection.handle(), transaction);
        REQUIRE_FALSE(append_status.ok());
    } // `connection` (and its test-only trigger) closes here.

    bim::persistence::detail::SqliteConnection reopened;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), reopened).ok());

    std::vector<bim::transactions::JournalTransaction> read_back;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(reopened.handle(), read_back).ok());

    // The attempted transaction id and all of its records are absent -
    // not the transaction row, and not any entry row.
    REQUIRE(read_back.empty());
}
