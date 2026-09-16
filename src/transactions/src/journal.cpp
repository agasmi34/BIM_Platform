#include "bim/transactions/journal.hpp"

#include <cstddef>
#include <string>

namespace bim::transactions {

bim::foundation::Status ValidateJournalTransaction(const JournalTransaction& transaction) {
    if (transaction.id.empty()) {
        return bim::foundation::Status::Error("JournalTransaction.id must not be empty");
    }

    if (transaction.records.empty()) {
        return bim::foundation::Status::Error(
            "JournalTransaction.records must contain at least one record");
    }

    for (std::size_t ordinal = 0; ordinal < transaction.records.size(); ++ordinal) {
        if (transaction.records[ordinal].kind.empty()) {
            return bim::foundation::Status::Error(
                "JournalRecord.kind must not be empty (record ordinal " + std::to_string(ordinal) +
                ")");
        }
    }

    return bim::foundation::Status::Ok();
}

} // namespace bim::transactions
