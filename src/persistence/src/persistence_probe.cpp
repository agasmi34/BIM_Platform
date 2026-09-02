#include "bim/persistence/probe.hpp"

#include <sqlite3.h>

#include <string>
#include <utility>

namespace bim::persistence {

namespace {

bim::foundation::Status Fail(const std::string& context, int rc, sqlite3* db) {
    std::string message = context + " failed (sqlite rc=" + std::to_string(rc) + ")";
    if (db != nullptr) {
        const char* db_message = sqlite3_errmsg(db);
        if (db_message != nullptr) {
            message += ": ";
            message += db_message;
        }
    }
    return bim::foundation::Status::Error(std::move(message));
}

} // namespace

bim::foundation::Status RunSqliteMemoryProbe() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        bim::foundation::Status status = Fail("sqlite3_open", rc, db);
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return status;
    }

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT 1;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        bim::foundation::Status status = Fail("sqlite3_prepare_v2", rc, db);
        sqlite3_close(db);
        return status;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        bim::foundation::Status status = Fail("sqlite3_step", rc, db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return status;
    }

    const int value = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        return Fail("sqlite3_close", rc, nullptr);
    }

    if (value != 1) {
        return bim::foundation::Status::Error("sqlite in-memory probe returned unexpected value " +
                                              std::to_string(value));
    }

    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence
