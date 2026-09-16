#pragma once

#include "bim/foundation/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace bim::persistence::detail {

// RAII owner of a single prepared sqlite3_stmt*. Finalizes the statement
// on every path (normal return, early return, exception unwinding).
// Provides only the minimal bind/step/read primitives needed by later
// Phase D/E schema and journal work (Implementation Brief
// BIM-TASK-P0-T004-CLAUDE v1.0 section 6.5) - deliberately not a generic
// public SQL execution API: this type is declared in a private detail
// header under src/persistence/src/detail/, never under
// src/persistence/include/**, so it cannot leak into the public surface.
class SqliteStatement {
public:
    SqliteStatement() = default;
    ~SqliteStatement();

    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;
    SqliteStatement(SqliteStatement&& other) noexcept;
    SqliteStatement& operator=(SqliteStatement&& other) noexcept;

    // Prepares `sql` against `db`. `db` must outlive the returned
    // statement. `sql` must be a fixed, first-party-authored string -
    // caller-controlled values must be bound with Bind*, never
    // concatenated into `sql`.
    [[nodiscard]] static bim::foundation::Status Prepare(sqlite3* db, std::string_view sql,
                                                         SqliteStatement& out);

    // 1-based parameter index, matching SQLite's own sqlite3_bind_* convention.
    [[nodiscard]] bim::foundation::Status BindText(int index, std::string_view value);
    [[nodiscard]] bim::foundation::Status BindBlob(int index, const std::vector<std::byte>& value);
    [[nodiscard]] bim::foundation::Status BindInt64(int index, std::int64_t value);

    // Executes one step. `out_has_row` is set true when a row is available
    // (SQLITE_ROW), false when the statement is exhausted (SQLITE_DONE).
    // Any other result is returned as a failure Status.
    [[nodiscard]] bim::foundation::Status Step(bool& out_has_row);

    // Column readers; valid only immediately after a Step() call that
    // produced a row (out_has_row == true).
    [[nodiscard]] std::string ColumnText(int column) const;
    [[nodiscard]] std::vector<std::byte> ColumnBlob(int column) const;
    [[nodiscard]] std::int64_t ColumnInt64(int column) const;

    // Resets the statement so it may be Step()-ped again from the start.
    // Matches sqlite3_reset() semantics: bound parameter values are NOT
    // cleared: call the Bind* functions again before re-stepping if fresh
    // values are required.
    [[nodiscard]] bim::foundation::Status Reset();

private:
    explicit SqliteStatement(sqlite3_stmt* stmt) : stmt_(stmt) {}
    void Finalize() noexcept;

    sqlite3_stmt* stmt_ = nullptr;
};

} // namespace bim::persistence::detail
