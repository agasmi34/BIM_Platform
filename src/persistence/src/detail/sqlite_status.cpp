#include "sqlite_status.hpp"

#include <sqlite3.h>

#include <string>
#include <utility>

namespace bim::persistence::detail {

bim::foundation::Status TranslateSqliteError(const char* context, int result_code, sqlite3* db) {
    std::string message =
        std::string(context) + " failed (sqlite rc=" + std::to_string(result_code) + ")";
    if (db != nullptr) {
        const char* db_message = sqlite3_errmsg(db);
        if (db_message != nullptr) {
            message += ": ";
            message += db_message;
        }
    }
    return bim::foundation::Status::Error(std::move(message));
}

} // namespace bim::persistence::detail
