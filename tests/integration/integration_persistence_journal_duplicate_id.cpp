#include "persistence_journal_test_support.hpp"

#include "bim/foundation/status.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

TEST_CASE("journal append rejects a duplicate transaction id completely and preserves the "
          "original transaction unchanged",
          "[integration][persistence][journal]") {
    bim_p0_t004_test::TempDatabasePath db_path;

    bim::transactions::JournalTransaction original;
    original.id = "duplicate-target";
    original.records = {
        bim::transactions::JournalRecord{"kind-original", std::vector<std::byte>{std::byte{0x01}}},
    };

    // Same id, distinguishable content: proves the duplicate is rejected
    // outright, not merged/upserted into the original.
    bim::transactions::JournalTransaction duplicate_attempt;
    duplicate_attempt.id = "duplicate-target";
    duplicate_attempt.records = {
        bim::transactions::JournalRecord{"kind-rejected",
                                         std::vector<std::byte>{std::byte{0x02}, std::byte{0x03}}},
        bim::transactions::JournalRecord{"kind-rejected-2", std::vector<std::byte>{}},
    };

    {
        bim::persistence::detail::SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());
        REQUIRE(
            bim::persistence::detail::AppendJournalTransaction(connection.handle(), original).ok());

        const bim::foundation::Status duplicate_status =
            bim::persistence::detail::AppendJournalTransaction(connection.handle(),
                                                               duplicate_attempt);
        REQUIRE_FALSE(duplicate_status.ok());
    } // `connection` closes here.

    bim::persistence::detail::SqliteConnection reopened;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), reopened).ok());

    std::vector<bim::transactions::JournalTransaction> read_back;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(reopened.handle(), read_back).ok());

    // The original transaction is present exactly once, unchanged.
    REQUIRE(read_back.size() == 1);
    REQUIRE(read_back[0].id == "duplicate-target");
    REQUIRE(read_back[0].records.size() == 1);
    REQUIRE(read_back[0].records[0].kind == "kind-original");
    REQUIRE(read_back[0].records[0].payload == original.records[0].payload);

    // No content from the rejected duplicate exists anywhere in the
    // reconstructed journal.
    for (const bim::transactions::JournalTransaction& transaction : read_back) {
        for (const bim::transactions::JournalRecord& record : transaction.records) {
            REQUIRE(record.kind != "kind-rejected");
            REQUIRE(record.kind != "kind-rejected-2");
        }
    }
}
