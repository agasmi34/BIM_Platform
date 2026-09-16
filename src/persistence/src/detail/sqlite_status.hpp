#pragma once

#include "bim/foundation/status.hpp"

struct sqlite3;

namespace bim::persistence::detail {

// Translates a non-OK sqlite3 result code into a project-owned Status,
// including sqlite3_errmsg(db) when `db` is available. `context` names the
// failing operation (e.g. "sqlite3_open"). This is the shared, single
// place raw SQLite result codes are read and turned into
// bim::foundation::Status; no SQLite result code/type ever crosses the
// public persistence boundary (Architecture Gate AG-P0T004-002).
[[nodiscard]] bim::foundation::Status TranslateSqliteError(const char* context, int result_code,
                                                           sqlite3* db);

} // namespace bim::persistence::detail
