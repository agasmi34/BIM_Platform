#include "persistence_journal_test_support.hpp"

#include "bim/foundation/status.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

TEST_CASE("journal append/read preserves append order (not lexical id order), record order, "
          "kind, and payload across a close/reopen",
          "[integration][persistence][journal]") {
    bim_p0_t004_test::TempDatabasePath db_path;

    // "id-zebra" is appended before "id-apple" - lexical order would put
    // "id-apple" first, so a read that returns "id-zebra" first proves
    // transaction order tracks append/persistence sequence, not id.
    bim::transactions::JournalTransaction first_transaction;
    first_transaction.id = "id-zebra";
    first_transaction.records = {
        bim::transactions::JournalRecord{"kind-a",
                                         std::vector<std::byte>{std::byte{0x10}, std::byte{0x20}}},
    };

    bim::transactions::JournalTransaction second_transaction;
    second_transaction.id = "id-apple";
    second_transaction.records = {
        bim::transactions::JournalRecord{"kind-b1", std::vector<std::byte>{}},
        bim::transactions::JournalRecord{"kind-b2", std::vector<std::byte>{std::byte{0xAB}}},
    };

    {
        bim::persistence::detail::SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());
        REQUIRE(bim::persistence::detail::AppendJournalTransaction(connection.handle(),
                                                                   first_transaction)
                    .ok());
        REQUIRE(bim::persistence::detail::AppendJournalTransaction(connection.handle(),
                                                                   second_transaction)
                    .ok());
    } // `connection` closes here.

    bim::persistence::detail::SqliteConnection reopened;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), reopened).ok());

    std::vector<bim::transactions::JournalTransaction> read_back;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(reopened.handle(), read_back).ok());

    REQUIRE(read_back.size() == 2);

    REQUIRE(read_back[0].id == "id-zebra");
    REQUIRE(read_back[0].records.size() == 1);
    REQUIRE(read_back[0].records[0].kind == "kind-a");
    REQUIRE(read_back[0].records[0].payload == first_transaction.records[0].payload);

    REQUIRE(read_back[1].id == "id-apple");
    REQUIRE(read_back[1].records.size() == 2);
    REQUIRE(read_back[1].records[0].kind == "kind-b1");
    REQUIRE(read_back[1].records[0].payload.empty());
    REQUIRE(read_back[1].records[1].kind == "kind-b2");
    REQUIRE(read_back[1].records[1].payload == second_transaction.records[1].payload);
}
