// Proves the neutral, SQLite-independent journal-transaction validation
// contract (Architecture Gate BIM-AG-P0-T004 v1.0 section 5;
// Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 section 16). This
// executable links ONLY bim::transactions (never bim::persistence) -
// mirroring the unit_geometry_api_contract / unit_viewport_* "link only
// the neutral layer" pattern - so a successful build is itself mechanical
// proof this contract needs no persistence/SQLite symbol.

#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace {

using bim::transactions::JournalRecord;
using bim::transactions::JournalTransaction;
using bim::transactions::ValidateJournalTransaction;

JournalTransaction MakeValidTransaction() {
    JournalTransaction transaction;
    transaction.id = "txn-1";
    transaction.records.push_back(JournalRecord{"wall.create", {}});
    return transaction;
}

} // namespace

TEST_CASE("a valid transaction with one non-empty-kind record is accepted",
          "[unit][transactions]") {
    const JournalTransaction transaction = MakeValidTransaction();
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE(status.ok());
}

TEST_CASE("an empty transaction id is rejected", "[unit][transactions]") {
    JournalTransaction transaction = MakeValidTransaction();
    transaction.id.clear();
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE_FALSE(status.ok());
}

TEST_CASE("zero records is rejected", "[unit][transactions]") {
    JournalTransaction transaction = MakeValidTransaction();
    transaction.records.clear();
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE_FALSE(status.ok());
}

TEST_CASE("any record with an empty kind is rejected", "[unit][transactions]") {
    JournalTransaction transaction = MakeValidTransaction();
    transaction.records.push_back(JournalRecord{"", {std::byte{0x01}}});
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE_FALSE(status.ok());
}

TEST_CASE("an empty payload is accepted", "[unit][transactions]") {
    JournalTransaction transaction;
    transaction.id = "txn-empty-payload";
    transaction.records.push_back(JournalRecord{"empty.payload", {}});
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE(status.ok());
}

TEST_CASE("a binary payload including boundary byte values is accepted", "[unit][transactions]") {
    JournalTransaction transaction;
    transaction.id = "txn-binary-payload";
    JournalRecord record;
    record.kind = "binary.payload";
    record.payload = {
        std::byte{0x00}, std::byte{0x01}, std::byte{0x7F},
        std::byte{0x80}, std::byte{0xFE}, std::byte{0xFF},
    };
    transaction.records.push_back(record);
    const bim::foundation::Status status = ValidateJournalTransaction(transaction);
    REQUIRE(status.ok());
}

TEST_CASE("validation does not change record count, order, kind, or payload",
          "[unit][transactions]") {
    JournalTransaction transaction;
    transaction.id = "txn-order";
    transaction.records.push_back(JournalRecord{"first", {std::byte{0x01}}});
    transaction.records.push_back(JournalRecord{"second", {std::byte{0x02}, std::byte{0x03}}});
    transaction.records.push_back(JournalRecord{"third", {}});

    const std::size_t original_size = transaction.records.size();
    const std::string first_kind = transaction.records[0].kind;
    const std::string second_kind = transaction.records[1].kind;
    const std::string third_kind = transaction.records[2].kind;
    const std::vector<std::byte> second_payload = transaction.records[1].payload;

    const bim::foundation::Status status = ValidateJournalTransaction(transaction);

    REQUIRE(status.ok());
    REQUIRE(transaction.records.size() == original_size);
    REQUIRE(transaction.records[0].kind == first_kind);
    REQUIRE(transaction.records[1].kind == second_kind);
    REQUIRE(transaction.records[2].kind == third_kind);
    REQUIRE(transaction.records[1].payload == second_payload);
}
