#include "sqlite_connection.hpp"

#include "sqlite_statement.hpp"
#include "sqlite_status.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace bim::persistence::detail {

namespace {

// SQLite's own synchronous levels (PRAGMA synchronous documentation):
// OFF=0, NORMAL=1, FULL=2, EXTRA=3. Frozen policy requires FULL.
constexpr std::int64_t kSqliteSynchronousFull = 2;
constexpr int kBusyTimeoutMs = 5000;

std::string ToLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

// Executes a fixed, first-party-authored setter pragma that returns no
// row (e.g. "PRAGMA foreign_keys = ON;"). Not for pragmas that return a
// value (journal_mode's setter form does; see ApplyAndVerifyJournalModeWal).
bim::foundation::Status RunSetterPragma(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

bim::foundation::Status VerifyForeignKeysEnabled(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA foreign_keys;", statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row || statement.ColumnInt64(0) != 1) {
        return bim::foundation::Status::Error(
            "sqlite file policy: PRAGMA foreign_keys did not report enabled");
    }
    return bim::foundation::Status::Ok();
}

// "PRAGMA journal_mode = WAL;" is itself a query-form pragma: SQLite
// always returns one row with the resulting effective mode, even when
// used with '=' to set it. That single statement therefore both applies
// and verifies the mode in one round trip.
bim::foundation::Status ApplyAndVerifyJournalModeWal(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA journal_mode = WAL;", statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return bim::foundation::Status::Error(
            "sqlite file policy: PRAGMA journal_mode = WAL returned no row");
    }
    const std::string mode = ToLowerAscii(statement.ColumnText(0));
    if (mode != "wal") {
        return bim::foundation::Status::Error("sqlite file policy: effective journal_mode is \"" +
                                              mode + "\", expected \"wal\"");
    }
    return bim::foundation::Status::Ok();
}

bim::foundation::Status VerifySynchronousFull(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, "PRAGMA synchronous;", statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row || statement.ColumnInt64(0) != kSqliteSynchronousFull) {
        return bim::foundation::Status::Error(
            "sqlite file policy: PRAGMA synchronous did not report FULL");
    }
    return bim::foundation::Status::Ok();
}

} // namespace

SqliteConnection::~SqliteConnection() {
    Close();
}

SqliteConnection::SqliteConnection(SqliteConnection&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

SqliteConnection& SqliteConnection::operator=(SqliteConnection&& other) noexcept {
    if (this != &other) {
        Close();
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

void SqliteConnection::Close() noexcept {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bim::foundation::Status SqliteConnection::OpenInMemory(SqliteConnection& out) {
    sqlite3* db = nullptr;
    const int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        bim::foundation::Status status = TranslateSqliteError("sqlite3_open", rc, db);
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return status;
    }
    out = SqliteConnection(db);
    return bim::foundation::Status::Ok();
}

bim::foundation::Status SqliteConnection::OpenFileWithPolicy(const std::filesystem::path& path,
                                                             SqliteConnection& out) {
    sqlite3* db = nullptr;
    const int open_rc = sqlite3_open(path.string().c_str(), &db);
    if (open_rc != SQLITE_OK) {
        bim::foundation::Status status = TranslateSqliteError("sqlite3_open", open_rc, db);
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return status;
    }

    const int busy_rc = sqlite3_busy_timeout(db, kBusyTimeoutMs);
    if (busy_rc != SQLITE_OK) {
        bim::foundation::Status status = TranslateSqliteError("sqlite3_busy_timeout", busy_rc, db);
        sqlite3_close(db);
        return status;
    }

    bim::foundation::Status status = RunSetterPragma(db, "PRAGMA foreign_keys = ON;");
    if (status.ok()) {
        status = VerifyForeignKeysEnabled(db);
    }
    if (status.ok()) {
        status = ApplyAndVerifyJournalModeWal(db);
    }
    if (status.ok()) {
        status = RunSetterPragma(db, "PRAGMA synchronous = FULL;");
    }
    if (status.ok()) {
        status = VerifySynchronousFull(db);
    }

    if (!status.ok()) {
        sqlite3_close(db);
        return status;
    }

    out = SqliteConnection(db);
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence::detail
