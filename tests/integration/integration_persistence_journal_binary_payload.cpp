#include "persistence_journal_test_support.hpp"

#include "bim/foundation/status.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

TEST_CASE("journal append/read round-trips a binary payload with boundary/embedded-zero bytes "
          "exactly, and an empty payload remains valid, across a close/reopen",
          "[integration][persistence][journal]") {
    bim_p0_t004_test::TempDatabasePath db_path;

    const std::vector<std::byte> boundary_payload{
        std::byte{0x00}, std::byte{0x01}, std::byte{0x7F},
        std::byte{0x80}, std::byte{0xFE}, std::byte{0xFF},
    };

    bim::transactions::JournalTransaction transaction;
    transaction.id = "binary-payload-txn";
    transaction.records = {
        bim::transactions::JournalRecord{"kind-boundary", boundary_payload},
        bim::transactions::JournalRecord{"kind-empty", std::vector<std::byte>{}},
    };

    {
        bim::persistence::detail::SqliteConnection connection;
        REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), connection).ok());
        REQUIRE(bim::persistence::detail::AppendJournalTransaction(connection.handle(), transaction)
                    .ok());
    } // `connection` closes here.

    bim::persistence::detail::SqliteConnection reopened;
    REQUIRE(bim::persistence::detail::OpenOrCreateSchemaV1(db_path.path(), reopened).ok());

    std::vector<bim::transactions::JournalTransaction> read_back;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(reopened.handle(), read_back).ok());

    REQUIRE(read_back.size() == 1);
    REQUIRE(read_back[0].id == "binary-payload-txn");
    REQUIRE(read_back[0].records.size() == 2);

    REQUIRE(read_back[0].records[0].kind == "kind-boundary");
    REQUIRE(read_back[0].records[0].payload.size() == boundary_payload.size());
    REQUIRE(read_back[0].records[0].payload == boundary_payload);

    REQUIRE(read_back[0].records[1].kind == "kind-empty");
    REQUIRE(read_back[0].records[1].payload.empty());
}
