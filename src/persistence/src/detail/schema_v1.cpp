#include "schema_v1.hpp"

#include "sqlite_statement.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bim::persistence::detail {

namespace {

// Fixed, first-party-authored DDL for the frozen Phase-0 schema v1
// (Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 sections 9-11;
// Architecture Gate AG-P0T004-005). Never built from caller-controlled
// input.
constexpr std::string_view kCreateJournalTransactions = "CREATE TABLE journal_transactions ("
                                                        "transaction_id TEXT PRIMARY KEY NOT NULL"
                                                        ");";

constexpr std::string_view kCreateJournalEntries =
    "CREATE TABLE journal_entries ("
    "sequence INTEGER PRIMARY KEY AUTOINCREMENT, "
    "transaction_id TEXT NOT NULL, "
    "ordinal INTEGER NOT NULL CHECK (ordinal >= 0), "
    "kind TEXT NOT NULL CHECK (length(kind) > 0), "
    "payload BLOB NOT NULL, "
    "UNIQUE (transaction_id, ordinal), "
    "FOREIGN KEY (transaction_id) REFERENCES journal_transactions(transaction_id) "
    "ON DELETE CASCADE"
    ");";

struct ExpectedColumn {
    std::string_view name;
    std::string_view type;
    bool not_null;
    bool primary_key;
};

// Strips whitespace and uppercases `sql`. SQLite has no structured PRAGMA
// for CHECK constraints or AUTOINCREMENT, so the frozen Phase-0 schema
// authority (see schema_v1.hpp) accepts a narrowly-scoped textual
// inspection of SQLite's own recorded CREATE TABLE SQL
// (sqlite_master.sql) as the verification mechanism for those two
// properties only.
std::string NormalizeSqlForCheckSearch(std::string_view sql) {
    std::string normalized;
    normalized.reserve(sql.size());
    for (unsigned char c : sql) {
        if (std::isspace(c) != 0) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(c)));
    }
    return normalized;
}

// Executes a fixed, first-party-authored statement that returns no row
// (DDL, BEGIN/COMMIT/ROLLBACK, a non-query-form pragma setter). `sql`
// must never be built from caller-controlled input.
[[nodiscard]] bim::foundation::Status ExecuteFixedStatement(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status VerifyTableExists(sqlite3* db, std::string_view table,
                                                        bool& out_exists) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name = ?1;", statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, table);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    out_exists = has_row && statement.ColumnInt64(0) > 0;
    return bim::foundation::Status::Ok();
}

// `table` is always one of the two fixed, code-controlled schema-v1 table
// names below - never caller/user-controlled - so building the PRAGMA
// text by concatenation is acceptable here (PRAGMA does not support bound
// parameters for its argument).
[[nodiscard]] bim::foundation::Status CountColumns(sqlite3* db, std::string_view table,
                                                   std::int64_t& out_count) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA table_info(" + std::string(table) + ");", statement);
    if (!status.ok()) {
        return status;
    }
    std::int64_t count = 0;
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        ++count;
    }
    out_count = count;
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status FetchTableSql(sqlite3* db, std::string_view table,
                                                    std::string& out_sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "SELECT sql FROM sqlite_master WHERE type='table' AND name = ?1;", statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, table);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return bim::foundation::Status::Error("schema v1: table \"" + std::string(table) +
                                              "\" not found in sqlite_master");
    }
    out_sql = statement.ColumnText(0);
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status VerifyColumn(sqlite3* db, std::string_view table,
                                                   const ExpectedColumn& expected,
                                                   bool& out_column_matches) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA table_info(" + std::string(table) + ");", statement);
    if (!status.ok()) {
        return status;
    }
    out_column_matches = false;
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        if (statement.ColumnText(1) != expected.name) {
            continue;
        }
        const std::string type = statement.ColumnText(2);
        const bool not_null = statement.ColumnInt64(3) != 0;
        const bool primary_key = statement.ColumnInt64(5) != 0;
        out_column_matches = (type == expected.type) && (not_null == expected.not_null) &&
                             (primary_key == expected.primary_key);
        return bim::foundation::Status::Ok();
    }
    return bim::foundation::Status::Ok(); // column not found: out_column_matches stays false
}

[[nodiscard]] bim::foundation::Status VerifyJournalTransactionsShape(sqlite3* db, bool& out_ok) {
    out_ok = false;

    bool exists = false;
    bim::foundation::Status status = VerifyTableExists(db, "journal_transactions", exists);
    if (!status.ok()) {
        return status;
    }
    if (!exists) {
        return bim::foundation::Status::Ok();
    }

    std::int64_t column_count = 0;
    status = CountColumns(db, "journal_transactions", column_count);
    if (!status.ok()) {
        return status;
    }
    if (column_count != 1) {
        return bim::foundation::Status::Ok();
    }

    return VerifyColumn(db, "journal_transactions",
                        ExpectedColumn{"transaction_id", "TEXT", true, true}, out_ok);
}

[[nodiscard]] bim::foundation::Status VerifyJournalEntriesColumns(sqlite3* db, bool& out_ok) {
    out_ok = false;

    std::int64_t column_count = 0;
    bim::foundation::Status status = CountColumns(db, "journal_entries", column_count);
    if (!status.ok()) {
        return status;
    }
    if (column_count != 5) {
        return bim::foundation::Status::Ok();
    }

    // sequence is an INTEGER PRIMARY KEY rowid alias without an explicit
    // NOT NULL in its DDL, so SQLite's own PRAGMA table_info reports
    // notnull = 0 for it even though the rowid alias can never actually be
    // null.
    static constexpr ExpectedColumn kExpectedColumns[] = {
        {"sequence", "INTEGER", false, true}, {"transaction_id", "TEXT", true, false},
        {"ordinal", "INTEGER", true, false},  {"kind", "TEXT", true, false},
        {"payload", "BLOB", true, false},
    };

    for (const ExpectedColumn& expected : kExpectedColumns) {
        bool column_matches = false;
        status = VerifyColumn(db, "journal_entries", expected, column_matches);
        if (!status.ok()) {
            return status;
        }
        if (!column_matches) {
            return bim::foundation::Status::Ok();
        }
    }

    out_ok = true;
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status VerifyJournalEntriesAutoincrement(sqlite3* db, bool& out_ok) {
    std::string sql;
    bim::foundation::Status status = FetchTableSql(db, "journal_entries", sql);
    if (!status.ok()) {
        return status;
    }
    out_ok = NormalizeSqlForCheckSearch(sql).find("AUTOINCREMENT") != std::string::npos;
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status VerifyJournalEntriesCheckConstraints(sqlite3* db,
                                                                           bool& out_ok) {
    std::string sql;
    bim::foundation::Status status = FetchTableSql(db, "journal_entries", sql);
    if (!status.ok()) {
        return status;
    }
    const std::string normalized = NormalizeSqlForCheckSearch(sql);
    out_ok = normalized.find("CHECK(ORDINAL>=0)") != std::string::npos &&
             normalized.find("CHECK(LENGTH(KIND)>0)") != std::string::npos;
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status VerifyJournalEntriesForeignKey(sqlite3* db, bool& out_ok) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA foreign_key_list(journal_entries);", statement);
    if (!status.ok()) {
        return status;
    }

    out_ok = false;
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        const std::string table = statement.ColumnText(2);
        const std::string from = statement.ColumnText(3);
        const std::string to = statement.ColumnText(4);
        std::string on_delete = statement.ColumnText(6);
        std::transform(on_delete.begin(), on_delete.end(), on_delete.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        if (table == "journal_transactions" && from == "transaction_id" && to == "transaction_id" &&
            on_delete == "CASCADE") {
            out_ok = true;
            return bim::foundation::Status::Ok();
        }
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status VerifyJournalEntriesUniqueConstraint(sqlite3* db,
                                                                           bool& out_ok) {
    SqliteStatement list_statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA index_list(journal_entries);", list_statement);
    if (!status.ok()) {
        return status;
    }

    out_ok = false;
    for (;;) {
        bool has_row = false;
        status = list_statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        if (list_statement.ColumnInt64(2) == 0) {
            continue; // not a unique index
        }
        const std::string index_name = list_statement.ColumnText(1);

        // index_name is not a fixed literal (it is either an
        // SQLite-autogenerated name or the frozen schema's own inline
        // UNIQUE constraint name), but it is always SQLite-internal,
        // self-produced content read back from this same connection in
        // this private validation path - never external/caller-controlled
        // input - consistent with the narrowly-scoped self-inspection this
        // helper performs.
        SqliteStatement info_statement;
        status =
            SqliteStatement::Prepare(db, "PRAGMA index_info(" + index_name + ");", info_statement);
        if (!status.ok()) {
            return status;
        }

        std::vector<std::string> columns_in_order;
        for (;;) {
            bool info_has_row = false;
            status = info_statement.Step(info_has_row);
            if (!status.ok()) {
                return status;
            }
            if (!info_has_row) {
                break;
            }
            columns_in_order.push_back(info_statement.ColumnText(2));
        }

        if (columns_in_order.size() == 2 && columns_in_order[0] == "transaction_id" &&
            columns_in_order[1] == "ordinal") {
            out_ok = true;
            return bim::foundation::Status::Ok();
        }
    }
    return bim::foundation::Status::Ok();
}

} // namespace

bim::foundation::Status ReadUserVersion(sqlite3* db, std::int64_t& out_version) {
    SqliteStatement statement;
    bim::foundation::Status status =
        SqliteStatement::Prepare(db, "PRAGMA user_version;", statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return bim::foundation::Status::Error("schema v1: PRAGMA user_version returned no row");
    }
    out_version = statement.ColumnInt64(0);
    return bim::foundation::Status::Ok();
}

bim::foundation::Status HasPreExistingUserTables(sqlite3* db, bool& out_has_user_tables) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db,
        "SELECT count(*) FROM sqlite_master WHERE type='table' AND substr(name,1,7) <> "
        "'sqlite_';",
        statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    out_has_user_tables = has_row && statement.ColumnInt64(0) > 0;
    return bim::foundation::Status::Ok();
}

bim::foundation::Status ValidateSchemaV1(sqlite3* db) {
    bool transactions_ok = false;
    bim::foundation::Status status = VerifyJournalTransactionsShape(db, transactions_ok);
    if (!status.ok()) {
        return status;
    }
    if (!transactions_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_transactions table missing or does not match the frozen shape");
    }

    bool columns_ok = false;
    status = VerifyJournalEntriesColumns(db, columns_ok);
    if (!status.ok()) {
        return status;
    }
    if (!columns_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_entries columns do not match the frozen shape");
    }

    bool autoincrement_ok = false;
    status = VerifyJournalEntriesAutoincrement(db, autoincrement_ok);
    if (!status.ok()) {
        return status;
    }
    if (!autoincrement_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_entries.sequence is not AUTOINCREMENT");
    }

    bool foreign_key_ok = false;
    status = VerifyJournalEntriesForeignKey(db, foreign_key_ok);
    if (!status.ok()) {
        return status;
    }
    if (!foreign_key_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_entries is missing the frozen FOREIGN KEY (transaction_id) "
            "REFERENCES journal_transactions(transaction_id) ON DELETE CASCADE");
    }

    bool unique_ok = false;
    status = VerifyJournalEntriesUniqueConstraint(db, unique_ok);
    if (!status.ok()) {
        return status;
    }
    if (!unique_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_entries is missing the frozen UNIQUE (transaction_id, ordinal)");
    }

    bool check_ok = false;
    status = VerifyJournalEntriesCheckConstraints(db, check_ok);
    if (!status.ok()) {
        return status;
    }
    if (!check_ok) {
        return bim::foundation::Status::Error(
            "schema v1: journal_entries is missing one or both frozen CHECK constraints");
    }

    return bim::foundation::Status::Ok();
}

bim::foundation::Status BootstrapSchemaV1(sqlite3* db) {
    bim::foundation::Status status = ExecuteFixedStatement(db, "BEGIN IMMEDIATE;");
    if (!status.ok()) {
        return status;
    }

    status = ExecuteFixedStatement(db, kCreateJournalTransactions);
    if (status.ok()) {
        status = ExecuteFixedStatement(db, kCreateJournalEntries);
    }
    if (status.ok()) {
        status = ValidateSchemaV1(db);
    }
    if (status.ok()) {
        status = ExecuteFixedStatement(db, "PRAGMA user_version = 1;");
    }

    if (!status.ok()) {
        // Preserve the original failure as the returned Status regardless
        // of whether the rollback itself succeeds - the caller needs to
        // know what actually went wrong first. `db` is left with
        // user_version still 0 and none of journal_transactions/
        // journal_entries observable, either way.
        (void)ExecuteFixedStatement(db, "ROLLBACK;");
        return status;
    }

    return ExecuteFixedStatement(db, "COMMIT;");
}

bim::foundation::Status OpenOrCreateSchemaV1(const std::filesystem::path& path,
                                             SqliteConnection& out) {
    SqliteConnection connection;
    bim::foundation::Status status = SqliteConnection::OpenFileWithPolicy(path, connection);
    if (!status.ok()) {
        return status;
    }

    std::int64_t version = 0;
    status = ReadUserVersion(connection.handle(), version);
    if (!status.ok()) {
        return status;
    }

    if (version == 0) {
        bool has_user_tables = false;
        status = HasPreExistingUserTables(connection.handle(), has_user_tables);
        if (!status.ok()) {
            return status;
        }
        if (has_user_tables) {
            return bim::foundation::Status::Error(
                "schema v1: user_version is 0 but pre-existing user table(s) were found; "
                "refusing to bootstrap over an unknown schema");
        }
        status = BootstrapSchemaV1(connection.handle());
        if (!status.ok()) {
            return status;
        }
    } else if (version == 1) {
        status = ValidateSchemaV1(connection.handle());
        if (!status.ok()) {
            return status;
        }
    } else if (version > 1) {
        return bim::foundation::Status::Error("schema v1: database user_version " +
                                              std::to_string(version) +
                                              " is newer than this build supports (max 1)");
    } else {
        return bim::foundation::Status::Error("schema v1: database user_version " +
                                              std::to_string(version) + " is invalid (negative)");
    }

    out = std::move(connection);
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence::detail
