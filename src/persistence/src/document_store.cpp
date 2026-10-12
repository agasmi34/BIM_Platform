#include "bim/persistence/document_store.hpp"

#include "detail/journal_store.hpp"
#include "detail/schema_v2.hpp"
#include "detail/sqlite_connection.hpp"
#include "detail/sqlite_statement.hpp"

#include "bim/model/element_id.hpp"
#include "bim/model/validation.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

// P1-T004 document store (Architecture Gate BIM-AG-P1-T004 sections 5-7, 9 and
// 13; Implementation Brief BIM-IB-P1-T004 sections 10-12). All SQLite work lives
// in this file and in src/persistence/src/detail/**; the public header
// document_store.hpp carries only project-owned values.

namespace bim::persistence {

namespace {

using bim::foundation::Status;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::StraightWall;
using detail::SqliteConnection;
using detail::SqliteStatement;

// Executes a fixed, first-party-authored statement that returns no row
// (BEGIN/COMMIT/ROLLBACK, DELETE). `sql` must never be built from
// caller-controlled input.
[[nodiscard]] Status ExecuteFixedStatement(sqlite3* db, std::string_view sql) {
    SqliteStatement statement;
    Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

[[nodiscard]] Status ReadSingleInt64(sqlite3* db, std::string_view sql, std::int64_t& out_value) {
    SqliteStatement statement;
    Status status = SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return Status::Error("document store: a fixed scalar query returned no row");
    }
    out_value = statement.ColumnInt64(0);
    return Status::Ok();
}

// Steps a prepared INSERT once and resets it so it can be re-bound for the next
// row. A step failure is returned as-is (the statement is finalized by its
// owner); the reset only follows a successful step.
[[nodiscard]] Status StepAndReset(SqliteStatement& statement) {
    bool has_row = false;
    Status status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    return statement.Reset();
}

// Parses persisted text as a production ElementId and requires the exact
// canonical lowercase spelling back: a valid but non-canonical id (upper case,
// for example) is rejected, never normalized.
[[nodiscard]] bool ParseCanonicalElementId(const std::string& text, ElementId& out) {
    const std::optional<ElementId> parsed = bim::model::ParseElementId(text);
    if (!parsed.has_value() || !bim::model::IsValid(*parsed)) {
        return false;
    }
    if (bim::model::ToString(*parsed) != text) {
        return false;
    }
    out = *parsed;
    return true;
}

// --- save ------------------------------------------------------------------

struct ValidatedSave {
    std::vector<const Level*> levels;       // ElementId ascending
    std::vector<const StraightWall*> walls; // ElementId ascending
};

[[nodiscard]] Status
ValidateForSave(const DocumentSnapshot& snapshot,
                const std::vector<bim::transactions::JournalTransaction>& pending_journal,
                ValidatedSave& out) {
    std::set<ElementId> level_ids;
    std::set<ElementId> all_ids;

    for (const Level& level : snapshot.levels) {
        const bim::model::ValidationResult validation = bim::model::ValidateLevel(level);
        if (!validation.ok()) {
            return Status::Error(std::string("document store save: invalid Level (") +
                                 bim::model::ToString(validation.code) + ")");
        }
        if (!all_ids.insert(level.id).second) {
            return Status::Error("document store save: duplicate ElementId " +
                                 bim::model::ToString(level.id));
        }
        level_ids.insert(level.id);
    }
    for (const StraightWall& wall : snapshot.straight_walls) {
        const bim::model::ValidationResult validation = bim::model::ValidateStraightWall(wall);
        if (!validation.ok()) {
            return Status::Error(std::string("document store save: invalid StraightWall (") +
                                 bim::model::ToString(validation.code) + ")");
        }
        if (!all_ids.insert(wall.id).second) {
            return Status::Error("document store save: duplicate ElementId " +
                                 bim::model::ToString(wall.id));
        }
        if (!level_ids.contains(wall.level_id)) {
            return Status::Error("document store save: StraightWall " +
                                 bim::model::ToString(wall.id) +
                                 " references a Level that is not in the snapshot");
        }
    }

    std::set<std::string> journal_ids;
    for (const bim::transactions::JournalTransaction& transaction : pending_journal) {
        const Status validation = bim::transactions::ValidateJournalTransaction(transaction);
        if (!validation.ok()) {
            return Status::Error("document store save: invalid pending journal transaction: " +
                                 validation.message());
        }
        if (!journal_ids.insert(transaction.id).second) {
            return Status::Error(
                "document store save: duplicate pending journal transaction id \"" +
                transaction.id + "\"");
        }
    }

    ValidatedSave validated;
    validated.levels.reserve(snapshot.levels.size());
    for (const Level& level : snapshot.levels) {
        validated.levels.push_back(&level);
    }
    validated.walls.reserve(snapshot.straight_walls.size());
    for (const StraightWall& wall : snapshot.straight_walls) {
        validated.walls.push_back(&wall);
    }
    std::sort(validated.levels.begin(), validated.levels.end(),
              [](const Level* a, const Level* b) { return a->id < b->id; });
    std::sort(validated.walls.begin(), validated.walls.end(),
              [](const StraightWall* a, const StraightWall* b) { return a->id < b->id; });
    out = std::move(validated);
    return Status::Ok();
}

[[nodiscard]] Status InsertElementRow(SqliteStatement& statement, const ElementId& id,
                                      std::int64_t element_type) {
    Status status = statement.BindText(1, bim::model::ToString(id));
    if (status.ok()) {
        status = statement.BindInt64(2, element_type);
    }
    if (!status.ok()) {
        return status;
    }
    return StepAndReset(statement);
}

[[nodiscard]] Status InsertLevelRow(SqliteStatement& statement, const Level& level) {
    Status status = statement.BindText(1, bim::model::ToString(level.id));
    if (status.ok()) {
        status = statement.BindDouble(2, level.elevation);
    }
    if (!status.ok()) {
        return status;
    }
    return StepAndReset(statement);
}

[[nodiscard]] Status InsertWallRow(SqliteStatement& statement, const StraightWall& wall) {
    Status status = statement.BindText(1, bim::model::ToString(wall.id));
    if (status.ok()) {
        status = statement.BindText(2, bim::model::ToString(wall.level_id));
    }
    if (status.ok()) {
        status = statement.BindDouble(3, wall.start.x);
    }
    if (status.ok()) {
        status = statement.BindDouble(4, wall.start.y);
    }
    if (status.ok()) {
        status = statement.BindDouble(5, wall.end.x);
    }
    if (status.ok()) {
        status = statement.BindDouble(6, wall.end.y);
    }
    if (status.ok()) {
        status = statement.BindDouble(7, wall.thickness);
    }
    if (status.ok()) {
        status = statement.BindDouble(8, wall.base_offset);
    }
    if (status.ok()) {
        status = statement.BindDouble(9, wall.height);
    }
    if (!status.ok()) {
        return status;
    }
    return StepAndReset(statement);
}

// The body of the single save transaction. The caller owns BEGIN IMMEDIATE and
// COMMIT/ROLLBACK; nothing here issues transaction control, and nothing here
// touches the journal tables except the append of the pending transactions, so
// committed journal history is never deleted or rewritten.
[[nodiscard]] Status
WriteSnapshotAndJournal(sqlite3* db, const ValidatedSave& validated,
                        const std::vector<bim::transactions::JournalTransaction>& pending_journal) {
    // Children before parents: walls reference levels (RESTRICT) and both
    // reference elements, so the old snapshot is removed walls -> levels ->
    // elements.
    Status status = ExecuteFixedStatement(db, "DELETE FROM straight_walls;");
    if (status.ok()) {
        status = ExecuteFixedStatement(db, "DELETE FROM levels;");
    }
    if (status.ok()) {
        status = ExecuteFixedStatement(db, "DELETE FROM elements;");
    }
    if (!status.ok()) {
        return status;
    }

    SqliteStatement insert_element;
    SqliteStatement insert_level;
    SqliteStatement insert_wall;
    status = SqliteStatement::Prepare(
        db, "INSERT INTO elements (element_id, element_type) VALUES (?1, ?2);", insert_element);
    if (status.ok()) {
        status = SqliteStatement::Prepare(
            db, "INSERT INTO levels (element_id, elevation) VALUES (?1, ?2);", insert_level);
    }
    if (status.ok()) {
        status = SqliteStatement::Prepare(
            db,
            "INSERT INTO straight_walls (element_id, level_id, start_x, start_y, end_x, end_y, "
            "thickness, base_offset, height) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);",
            insert_wall);
    }
    if (!status.ok()) {
        return status;
    }

    // elements first (the parent of both kind tables), then levels, then walls
    // (a wall's hosting Level row must already exist).
    for (const Level* level : validated.levels) {
        status = InsertElementRow(insert_element, level->id, detail::kElementTypeLevel);
        if (!status.ok()) {
            return status;
        }
    }
    for (const StraightWall* wall : validated.walls) {
        status = InsertElementRow(insert_element, wall->id, detail::kElementTypeStraightWall);
        if (!status.ok()) {
            return status;
        }
    }
    for (const Level* level : validated.levels) {
        status = InsertLevelRow(insert_level, *level);
        if (!status.ok()) {
            return status;
        }
    }
    for (const StraightWall* wall : validated.walls) {
        status = InsertWallRow(insert_wall, *wall);
        if (!status.ok()) {
            return status;
        }
    }

    for (const bim::transactions::JournalTransaction& transaction : pending_journal) {
        status = detail::InsertJournalTransactionWithinTransaction(db, transaction);
        if (!status.ok()) {
            return status;
        }
    }
    return Status::Ok();
}

// --- load ------------------------------------------------------------------

// Counts rows violating the storage-class expectations of one fixed query. The
// schema columns are declared NOT NULL with TEXT / INTEGER / REAL affinity, but
// SQLite still lets a differently-typed value (a text in a REAL column, for
// example) be stored when it cannot be converted, so the decoded classes are
// verified explicitly rather than trusted.
[[nodiscard]] Status RequireNoMistypedRows(sqlite3* db, std::string_view sql,
                                           std::string_view what) {
    std::int64_t count = 0;
    const Status status = ReadSingleInt64(db, sql, count);
    if (!status.ok()) {
        return status;
    }
    if (count != 0) {
        return Status::Error("document store load: " + std::string(what) +
                             " holds a value of an unexpected storage class");
    }
    return Status::Ok();
}

[[nodiscard]] Status ReadElements(sqlite3* db, std::map<ElementId, std::int64_t>& out) {
    SqliteStatement statement;
    Status status = SqliteStatement::Prepare(
        db, "SELECT element_id, element_type FROM elements ORDER BY element_id ASC;", statement);
    if (!status.ok()) {
        return status;
    }
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        ElementId id;
        if (!ParseCanonicalElementId(statement.ColumnText(0), id)) {
            return Status::Error(
                "document store load: elements holds a malformed or non-canonical ElementId");
        }
        const std::int64_t type = statement.ColumnInt64(1);
        if (type != detail::kElementTypeLevel && type != detail::kElementTypeStraightWall) {
            return Status::Error("document store load: elements holds an unknown element_type");
        }
        if (!out.emplace(id, type).second) {
            return Status::Error("document store load: duplicate ElementId in elements");
        }
    }
    return Status::Ok();
}

[[nodiscard]] Status ReadLevels(sqlite3* db, const std::map<ElementId, std::int64_t>& elements,
                                std::vector<Level>& out) {
    SqliteStatement statement;
    Status status = SqliteStatement::Prepare(
        db, "SELECT element_id, elevation FROM levels ORDER BY element_id ASC;", statement);
    if (!status.ok()) {
        return status;
    }
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        Level level;
        if (!ParseCanonicalElementId(statement.ColumnText(0), level.id)) {
            return Status::Error(
                "document store load: levels holds a malformed or non-canonical ElementId");
        }
        const auto element = elements.find(level.id);
        if (element == elements.end() || element->second != detail::kElementTypeLevel) {
            return Status::Error(
                "document store load: a levels row has no matching Level element record");
        }
        level.elevation = statement.ColumnDouble(1);
        const bim::model::ValidationResult validation = bim::model::ValidateLevel(level);
        if (!validation.ok()) {
            return Status::Error(std::string("document store load: invalid persisted Level (") +
                                 bim::model::ToString(validation.code) + ")");
        }
        out.push_back(level);
    }
    return Status::Ok();
}

[[nodiscard]] Status ReadWalls(sqlite3* db, const std::map<ElementId, std::int64_t>& elements,
                               const std::set<ElementId>& level_ids,
                               std::vector<StraightWall>& out) {
    SqliteStatement statement;
    Status status = SqliteStatement::Prepare(
        db,
        "SELECT element_id, level_id, start_x, start_y, end_x, end_y, thickness, base_offset, "
        "height FROM straight_walls ORDER BY element_id ASC;",
        statement);
    if (!status.ok()) {
        return status;
    }
    for (;;) {
        bool has_row = false;
        status = statement.Step(has_row);
        if (!status.ok()) {
            return status;
        }
        if (!has_row) {
            break;
        }
        StraightWall wall;
        if (!ParseCanonicalElementId(statement.ColumnText(0), wall.id) ||
            !ParseCanonicalElementId(statement.ColumnText(1), wall.level_id)) {
            return Status::Error(
                "document store load: straight_walls holds a malformed or non-canonical "
                "ElementId");
        }
        const auto element = elements.find(wall.id);
        if (element == elements.end() || element->second != detail::kElementTypeStraightWall) {
            return Status::Error(
                "document store load: a straight_walls row has no matching StraightWall element "
                "record");
        }
        wall.start.x = statement.ColumnDouble(2);
        wall.start.y = statement.ColumnDouble(3);
        wall.end.x = statement.ColumnDouble(4);
        wall.end.y = statement.ColumnDouble(5);
        wall.thickness = statement.ColumnDouble(6);
        wall.base_offset = statement.ColumnDouble(7);
        wall.height = statement.ColumnDouble(8);
        const bim::model::ValidationResult validation = bim::model::ValidateStraightWall(wall);
        if (!validation.ok()) {
            return Status::Error(
                std::string("document store load: invalid persisted StraightWall (") +
                bim::model::ToString(validation.code) + ")");
        }
        if (!level_ids.contains(wall.level_id)) {
            return Status::Error(
                "document store load: a StraightWall references a Level that does not exist");
        }
        out.push_back(wall);
    }
    return Status::Ok();
}

// Reads and validates the whole durable snapshot inside the caller's open read
// transaction. Assigns `out` only after every check passed.
[[nodiscard]] Status ReadSnapshot(sqlite3* db, DocumentSnapshot& out) {
    Status status =
        RequireNoMistypedRows(db,
                              "SELECT count(*) FROM elements WHERE typeof(element_id) <> 'text' OR "
                              "typeof(element_type) <> 'integer';",
                              "elements");
    if (status.ok()) {
        status = RequireNoMistypedRows(
            db,
            "SELECT count(*) FROM levels WHERE typeof(element_id) <> 'text' OR "
            "typeof(elevation) <> 'real';",
            "levels");
    }
    if (status.ok()) {
        status = RequireNoMistypedRows(
            db,
            "SELECT count(*) FROM straight_walls WHERE typeof(element_id) <> 'text' OR "
            "typeof(level_id) <> 'text' OR typeof(start_x) <> 'real' OR "
            "typeof(start_y) <> 'real' OR typeof(end_x) <> 'real' OR typeof(end_y) <> 'real' OR "
            "typeof(thickness) <> 'real' OR typeof(base_offset) <> 'real' OR "
            "typeof(height) <> 'real';",
            "straight_walls");
    }
    if (!status.ok()) {
        return status;
    }

    std::map<ElementId, std::int64_t> elements;
    status = ReadElements(db, elements);
    if (!status.ok()) {
        return status;
    }

    DocumentSnapshot loaded;
    status = ReadLevels(db, elements, loaded.levels);
    if (!status.ok()) {
        return status;
    }
    std::set<ElementId> level_ids;
    for (const Level& level : loaded.levels) {
        level_ids.insert(level.id);
    }
    status = ReadWalls(db, elements, level_ids, loaded.straight_walls);
    if (!status.ok()) {
        return status;
    }

    // Every element record must be backed by exactly one kind row of its type:
    // each kind row was matched to a distinct, correctly typed element above, so
    // equal totals prove there is no orphan element record.
    if (loaded.levels.size() + loaded.straight_walls.size() != elements.size()) {
        return Status::Error(
            "document store load: an element record has no matching Level or StraightWall row");
    }

    // The durable journal must be well-formed; its content is audit data and is
    // not part of the snapshot.
    std::vector<bim::transactions::JournalTransaction> journal;
    status = detail::ReadJournalTransactions(db, journal);
    if (!status.ok()) {
        return Status::Error("document store load: malformed journal state: " + status.message());
    }

    std::sort(loaded.levels.begin(), loaded.levels.end(),
              [](const Level& a, const Level& b) { return a.id < b.id; });
    std::sort(loaded.straight_walls.begin(), loaded.straight_walls.end(),
              [](const StraightWall& a, const StraightWall& b) { return a.id < b.id; });
    out = std::move(loaded);
    return Status::Ok();
}

} // namespace

Status SaveDocument(const std::filesystem::path& path, const DocumentSnapshot& snapshot,
                    const std::vector<bim::transactions::JournalTransaction>& pending_journal) {
    try {
        // Everything that can be rejected without touching the database is
        // rejected first: no file is created and nothing is opened for an
        // invalid snapshot or journal.
        ValidatedSave validated;
        Status status = ValidateForSave(snapshot, pending_journal, validated);
        if (!status.ok()) {
            return status;
        }

        SqliteConnection connection;
        status = detail::OpenOrCreateSchemaV2(path, connection);
        if (!status.ok()) {
            return status;
        }
        sqlite3* db = connection.handle();

        status = ExecuteFixedStatement(db, "BEGIN IMMEDIATE;");
        if (!status.ok()) {
            return status;
        }
        status = WriteSnapshotAndJournal(db, validated, pending_journal);
        if (!status.ok()) {
            // The original failure is what the caller needs to see, regardless
            // of whether the rollback itself succeeds. Everything the
            // transaction wrote - the replaced snapshot and any journal rows -
            // is undone together.
            (void)ExecuteFixedStatement(db, "ROLLBACK;");
            return status;
        }
        status = ExecuteFixedStatement(db, "COMMIT;");
        if (!status.ok()) {
            (void)ExecuteFixedStatement(db, "ROLLBACK;");
        }
        return status;
    } catch (const std::exception& error) {
        return Status::Error(std::string("document store save: unexpected failure: ") +
                             error.what());
    } catch (...) {
        return Status::Error("document store save: unexpected non-standard failure");
    }
}

Status LoadDocument(const std::filesystem::path& path, DocumentSnapshot& out_snapshot) {
    try {
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error)) {
            return Status::Error(
                "document store load: the database file does not exist (a load never creates "
                "one)");
        }

        SqliteConnection connection;
        Status status = detail::OpenOrCreateSchemaV2(path, connection);
        if (!status.ok()) {
            return status;
        }
        sqlite3* db = connection.handle();

        // One read transaction so the three kind tables and the journal are read
        // from a single consistent view.
        status = ExecuteFixedStatement(db, "BEGIN;");
        if (!status.ok()) {
            return status;
        }
        DocumentSnapshot loaded;
        status = ReadSnapshot(db, loaded);
        (void)ExecuteFixedStatement(db, status.ok() ? "COMMIT;" : "ROLLBACK;");
        if (!status.ok()) {
            return status;
        }
        out_snapshot = std::move(loaded);
        return Status::Ok();
    } catch (const std::exception& error) {
        return Status::Error(std::string("document store load: unexpected failure: ") +
                             error.what());
    } catch (...) {
        return Status::Error("document store load: unexpected non-standard failure");
    }
}

} // namespace bim::persistence
