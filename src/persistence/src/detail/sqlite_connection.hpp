#pragma once

#include "bim/foundation/status.hpp"

#include <filesystem>

struct sqlite3;

namespace bim::persistence::detail {

// RAII owner of a single sqlite3* connection handle. Closes the
// connection on every path (normal return, early return, exception
// unwinding). Declared in a private detail header under
// src/persistence/src/detail/, never under src/persistence/include/**, so
// the raw handle type never crosses the public persistence boundary
// (Architecture Gate AG-P0T004-001/002).
class SqliteConnection {
public:
    SqliteConnection() = default;
    ~SqliteConnection();

    SqliteConnection(const SqliteConnection&) = delete;
    SqliteConnection& operator=(const SqliteConnection&) = delete;
    SqliteConnection(SqliteConnection&& other) noexcept;
    SqliteConnection& operator=(SqliteConnection&& other) noexcept;

    // Opens ":memory:". No file-backed runtime policy is applied or
    // meaningful here (Phase-0 memory smoke probe only); WAL is never
    // forced on an in-memory database.
    [[nodiscard]] static bim::foundation::Status OpenInMemory(SqliteConnection& out);

    // Opens a local file-backed database at `path` and establishes and
    // verifies the frozen Phase-0 file runtime policy (Implementation
    // Brief BIM-TASK-P0-T004-CLAUDE v1.0 section 8 / Architecture Gate
    // AG-P0T004-009):
    //   sqlite3_busy_timeout(..., 5000)  - checked via its own return code
    //   PRAGMA foreign_keys = ON         - verified enabled via read-back
    //   PRAGMA journal_mode = WAL        - verified effective mode is
    //                                      "wal" (case-insensitively)
    //   PRAGMA synchronous = FULL        - verified effective value is
    //                                      SQLite's FULL synchronous level
    // Any policy step that cannot be established fails closed: the
    // connection is closed and a failure Status is returned; `out` is left
    // unmodified. This function does not create any project schema/table
    // and does not read or write PRAGMA user_version (Phase D).
    [[nodiscard]] static bim::foundation::Status
    OpenFileWithPolicy(const std::filesystem::path& path, SqliteConnection& out);

    [[nodiscard]] bool is_open() const noexcept { return db_ != nullptr; }

    // Returns the raw handle for use ONLY by other private
    // src/persistence/** helpers (e.g. SqliteStatement::Prepare). Never
    // returned or forwarded through any public persistence header.
    [[nodiscard]] sqlite3* handle() const noexcept { return db_; }

private:
    explicit SqliteConnection(sqlite3* db) : db_(db) {}
    void Close() noexcept;

    sqlite3* db_ = nullptr;
};

} // namespace bim::persistence::detail
