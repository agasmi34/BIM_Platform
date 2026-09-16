#pragma once

#include "sqlite_connection.hpp"

#include "bim/foundation/status.hpp"

#include <cstdint>
#include <filesystem>

namespace bim::persistence::detail {

// Frozen Phase-0 schema authority is PRAGMA user_version (Architecture
// Gate AG-P0T004-005). This is the only frozen schema version for this
// task (Architecture Gate section 7; Implementation Brief
// BIM-TASK-P0-T004-CLAUDE v1.0 sections 9-11).
inline constexpr std::int64_t kSchemaV1Version = 1;

// Opens a local file-backed database at `path` (establishing and
// verifying the frozen Phase C file runtime policy via
// SqliteConnection::OpenFileWithPolicy), then applies the frozen Phase-0
// schema-authority rule:
//
//   user_version == 0, no pre-existing (non-"sqlite_"-prefixed) user
//     table  -> bootstrap schema v1 atomically, then validate it
//   user_version == 0, a pre-existing user table exists
//     -> reject, fail closed, no mutation
//   user_version == 1 -> structurally validate the required schema;
//     accept only if valid
//   user_version > 1  -> reject as an unsupported newer schema, no mutation
//   user_version < 0  -> reject as invalid, fail closed
//
// On any failure `out` is left unmodified and the underlying connection
// (if one was opened) is closed; no partial state is left observable to
// the caller. Does not implement journal transaction append/read.
[[nodiscard]] bim::foundation::Status OpenOrCreateSchemaV1(const std::filesystem::path& path,
                                                           SqliteConnection& out);

// Reads PRAGMA user_version from an already-open connection.
[[nodiscard]] bim::foundation::Status ReadUserVersion(sqlite3* db, std::int64_t& out_version);

// True if `db` contains any table not owned by SQLite itself, i.e. any
// table in sqlite_master whose name does not start with the reserved
// "sqlite_" prefix. SQLite-internal tables (e.g. sqlite_sequence) never
// count as pre-existing user tables.
[[nodiscard]] bim::foundation::Status HasPreExistingUserTables(sqlite3* db,
                                                               bool& out_has_user_tables);

// Structurally validates that `db` contains exactly the frozen Phase-0
// schema v1: journal_transactions (transaction_id TEXT PRIMARY KEY NOT
// NULL) and journal_entries (sequence INTEGER PRIMARY KEY AUTOINCREMENT,
// transaction_id TEXT NOT NULL, ordinal INTEGER NOT NULL CHECK (ordinal
// >= 0), kind TEXT NOT NULL CHECK (length(kind) > 0), payload BLOB NOT
// NULL, UNIQUE (transaction_id, ordinal), FOREIGN KEY (transaction_id)
// REFERENCES journal_transactions(transaction_id) ON DELETE CASCADE).
// Read-only: never creates or modifies anything.
[[nodiscard]] bim::foundation::Status ValidateSchemaV1(sqlite3* db);

// Creates the frozen schema-v1 tables and sets PRAGMA user_version = 1
// inside a single atomic BEGIN IMMEDIATE / COMMIT transaction, validating
// the created schema before setting user_version, and rolling back on any
// failure (including a post-creation ValidateSchemaV1 failure) so version
// 1 is never observable together with a partially or incorrectly created
// schema. Callers must have already established that bootstrapping is
// appropriate (see HasPreExistingUserTables).
[[nodiscard]] bim::foundation::Status BootstrapSchemaV1(sqlite3* db);

} // namespace bim::persistence::detail
