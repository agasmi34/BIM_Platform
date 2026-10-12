#include "schema_v2.hpp"

#include "schema_v1.hpp"
#include "sqlite_statement.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bim::persistence::detail {

namespace {

// Fixed, first-party-authored DDL for the frozen schema-v2 BIM tables
// (Architecture Gate BIM-AG-P1-T004 section 5). Never built from
// caller-controlled input. The constants carry no trailing semicolon because
// SQLite records a table's CREATE text without one; ValidateSchemaV2 compares
// the recorded text against these constants after whitespace removal and
// upper-casing, so a table that was altered, re-created differently or
// hand-written to look similar is rejected rather than trusted.
constexpr std::string_view kCreateDocumentMetadata =
    "CREATE TABLE document_metadata ("
    "singleton INTEGER PRIMARY KEY CHECK (singleton = 1), "
    "format_tag TEXT NOT NULL"
    ")";

constexpr std::string_view kCreateElements =
    "CREATE TABLE elements ("
    "element_id TEXT PRIMARY KEY NOT NULL, "
    "element_type INTEGER NOT NULL CHECK (element_type IN (1, 2))"
    ")";

constexpr std::string_view kCreateLevels =
    "CREATE TABLE levels ("
    "element_id TEXT PRIMARY KEY NOT NULL, "
    "elevation REAL NOT NULL, "
    "FOREIGN KEY (element_id) REFERENCES elements(element_id) ON DELETE CASCADE"
    ")";

constexpr std::string_view kCreateStraightWalls =
    "CREATE TABLE straight_walls ("
    "element_id TEXT PRIMARY KEY NOT NULL, "
    "level_id TEXT NOT NULL, "
    "start_x REAL NOT NULL, "
    "start_y REAL NOT NULL, "
    "end_x REAL NOT NULL, "
    "end_y REAL NOT NULL, "
    "thickness REAL NOT NULL, "
    "base_offset REAL NOT NULL, "
    "height REAL NOT NULL, "
    "FOREIGN KEY (element_id) REFERENCES elements(element_id) ON DELETE CASCADE, "
    "FOREIGN KEY (level_id) REFERENCES levels(element_id) ON DELETE RESTRICT"
    ")";

struct ExpectedTable {
    std::string_view name;
    std::string_view create_sql;
};

constexpr std::array<ExpectedTable, 4> kBimTables = {{
    {"document_metadata", kCreateDocumentMetadata},
    {"elements", kCreateElements},
    {"levels", kCreateLevels},
    {"straight_walls", kCreateStraightWalls},
}};

// Every user table schema v2 may contain, in ascending name order (the order
// the sqlite_master query below returns them in).
constexpr std::array<std::string_view, 6> kExpectedUserTables = {
    "document_metadata",    "elements", "journal_entries",
    "journal_transactions", "levels",   "straight_walls",
};

// Strips every whitespace character and upper-cases the rest, so the stored
// CREATE text and the expected DDL compare independently of layout and case.
std::string NormalizeSql(std::string_view sql) {
    std::string normalized;
    normalized.reserve(sql.size());
    for (const unsigned char c : sql) {
        if (std::isspace(c) != 0) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(c)));
    }
    return normalized;
}

// Executes a fixed, first-party-authored statement that returns no row (DDL,
// BEGIN/COMMIT/ROLLBACK, a setter pragma). `sql` must never be built from
// caller-controlled input. Each schema file keeps its own private copy of this
// helper, exactly as schema_v1.cpp and journal_store.cpp do.
[[nodiscard]] bim::foundation::Status ExecuteFixedStatement(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] bim::foundation::Status ReadSingleInt64(sqlite3* db, std::string_view sql,
                                                      std::int64_t& out_value) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return bim::foundation::Status::Error("schema v2: a fixed scalar query returned no row");
    }
    out_value = statement.ColumnInt64(0);
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status FetchTableSql(sqlite3* db, std::string_view table,
                                                    bool& out_found, std::string& out_sql) {
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
    out_found = has_row;
    if (has_row) {
        out_sql = statement.ColumnText(0);
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status ValidateBimTableDefinitions(sqlite3* db) {
    for (const ExpectedTable& expected : kBimTables) {
        bool found = false;
        std::string recorded_sql;
        bim::foundation::Status status = FetchTableSql(db, expected.name, found, recorded_sql);
        if (!status.ok()) {
            return status;
        }
        if (!found) {
            return bim::foundation::Status::Error("schema v2: required table \"" +
                                                  std::string(expected.name) + "\" is missing");
        }
        if (NormalizeSql(recorded_sql) != NormalizeSql(expected.create_sql)) {
            return bim::foundation::Status::Error("schema v2: table \"" +
                                                  std::string(expected.name) +
                                                  "\" does not match its frozen definition");
        }
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status ValidateNoUnexpectedObjects(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db,
        "SELECT name FROM sqlite_master WHERE type='table' AND substr(name,1,7) <> 'sqlite_' "
        "ORDER BY name ASC;",
        statement);
    if (!status.ok()) {
        return status;
    }
    std::vector<std::string> names;
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        names.push_back(statement.ColumnText(0));
    }
    const bool same_set = names.size() == kExpectedUserTables.size() &&
                          std::equal(names.begin(), names.end(), kExpectedUserTables.begin());
    if (!same_set) {
        return bim::foundation::Status::Error(
            "schema v2: the set of user tables is not exactly the frozen schema-v2 set");
    }

    // A view or trigger could silently redirect or alter what the store reads
    // and writes; schema v2 defines neither, so either one is rejected.
    std::int64_t extra_objects = 0;
    status = ReadSingleInt64(
        db, "SELECT count(*) FROM sqlite_master WHERE type IN ('view','trigger');", extra_objects);
    if (!status.ok()) {
        return status;
    }
    if (extra_objects != 0) {
        return bim::foundation::Status::Error(
            "schema v2: the database contains a view or trigger that schema v2 does not define");
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status ValidateMetadata(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "SELECT singleton, format_tag, typeof(format_tag) FROM document_metadata;", statement);
    if (!status.ok()) {
        return status;
    }
    std::int64_t rows = 0;
    bool well_formed = false;
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        ++rows;
        well_formed = statement.ColumnInt64(0) == 1 && statement.ColumnText(2) == "text" &&
                      statement.ColumnText(1) == kDocumentFormatTag;
    }
    if (rows != 1 || !well_formed) {
        return bim::foundation::Status::Error(
            "schema v2: document_metadata must hold exactly one well-formed row");
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status CreateBimTables(sqlite3* db) {
    for (const ExpectedTable& table : kBimTables) {
        const bim::foundation::Status status = ExecuteFixedStatement(db, table.create_sql);
        if (!status.ok()) {
            return status;
        }
    }
    return bim::foundation::Status::Ok();
}

[[nodiscard]] bim::foundation::Status InsertMetadataRow(sqlite3* db) {
    SqliteStatement statement;
    bim::foundation::Status status = SqliteStatement::Prepare(
        db, "INSERT INTO document_metadata (singleton, format_tag) VALUES (1, ?1);", statement);
    if (!status.ok()) {
        return status;
    }
    status = statement.BindText(1, kDocumentFormatTag);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

// Completes a schema-v2 build that is already inside an open transaction:
// creates the BIM tables, inserts the metadata row, validates the complete
// result and sets user_version = 2. The caller owns BEGIN/COMMIT/ROLLBACK.
[[nodiscard]] bim::foundation::Status FinishSchemaV2InTransaction(sqlite3* db) {
    bim::foundation::Status status = CreateBimTables(db);
    if (status.ok()) {
        status = InsertMetadataRow(db);
    }
    if (status.ok()) {
        status = ValidateSchemaV2(db);
    }
    if (status.ok()) {
        status = ExecuteFixedStatement(db, "PRAGMA user_version = 2;");
    }
    return status;
}

// Runs `body` inside one BEGIN IMMEDIATE transaction: COMMIT on success, a
// ROLLBACK that preserves the original failure on any failure. The body issues
// no transaction control of its own.
template <typename Body>
[[nodiscard]] bim::foundation::Status RunInImmediateTransaction(sqlite3* db, Body&& body) {
    bim::foundation::Status status = ExecuteFixedStatement(db, "BEGIN IMMEDIATE;");
    if (!status.ok()) {
        return status;
    }
    status = body();
    if (!status.ok()) {
        // The original failure is what the caller needs to see, regardless of
        // whether the rollback itself succeeds.
        (void)ExecuteFixedStatement(db, "ROLLBACK;");
        return status;
    }
    status = ExecuteFixedStatement(db, "COMMIT;");
    if (!status.ok()) {
        (void)ExecuteFixedStatement(db, "ROLLBACK;");
    }
    return status;
}

} // namespace

bim::foundation::Status ValidateSchemaV2(sqlite3* db) {
    bim::foundation::Status status = ValidateSchemaV1(db);
    if (!status.ok()) {
        return status;
    }
    status = ValidateNoUnexpectedObjects(db);
    if (!status.ok()) {
        return status;
    }
    status = ValidateBimTableDefinitions(db);
    if (!status.ok()) {
        return status;
    }
    return ValidateMetadata(db);
}

bim::foundation::Status BootstrapSchemaV2(sqlite3* db) {
    return RunInImmediateTransaction(db, [db]() -> bim::foundation::Status {
        std::int64_t version = -1;
        bim::foundation::Status status = ReadUserVersion(db, version);
        if (!status.ok()) {
            return status;
        }
        bool has_user_tables = false;
        status = HasPreExistingUserTables(db, has_user_tables);
        if (!status.ok()) {
            return status;
        }
        if (version != 0 || has_user_tables) {
            return bim::foundation::Status::Error(
                "schema v2: bootstrap requires user_version 0 and no pre-existing user table");
        }
        status = CreateSchemaV1Tables(db);
        if (!status.ok()) {
            return status;
        }
        return FinishSchemaV2InTransaction(db);
    });
}

bim::foundation::Status MigrateSchemaV1ToV2(sqlite3* db) {
    return RunInImmediateTransaction(db, [db]() -> bim::foundation::Status {
        std::int64_t version = -1;
        bim::foundation::Status status = ReadUserVersion(db, version);
        if (!status.ok()) {
            return status;
        }
        if (version != kSchemaV1Version) {
            return bim::foundation::Status::Error(
                "schema v2: migration requires user_version 1, found " + std::to_string(version));
        }
        status = ValidateSchemaV1(db);
        if (!status.ok()) {
            return status;
        }
        return FinishSchemaV2InTransaction(db);
    });
}

bim::foundation::Status OpenOrCreateSchemaV2(const std::filesystem::path& path,
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
                "schema v2: user_version is 0 but pre-existing user table(s) were found; "
                "refusing to bootstrap over an unknown schema");
        }
        status = BootstrapSchemaV2(connection.handle());
        if (!status.ok()) {
            return status;
        }
    } else if (version == kSchemaV1Version) {
        status = MigrateSchemaV1ToV2(connection.handle());
        if (!status.ok()) {
            return status;
        }
    } else if (version == kSchemaV2Version) {
        status = ValidateSchemaV2(connection.handle());
        if (!status.ok()) {
            return status;
        }
    } else if (version > kSchemaV2Version) {
        return bim::foundation::Status::Error("schema v2: database user_version " +
                                              std::to_string(version) +
                                              " is newer than this build supports (max 2)");
    } else {
        return bim::foundation::Status::Error("schema v2: database user_version " +
                                              std::to_string(version) + " is invalid (negative)");
    }

    out = std::move(connection);
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence::detail
