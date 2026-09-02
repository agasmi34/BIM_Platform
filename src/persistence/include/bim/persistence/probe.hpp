#pragma once

#include "bim/foundation/status.hpp"

namespace bim::persistence {

// Runs a minimal in-memory SQLite smoke probe (Implementation Brief Phase I,
// "Persistence"): opens ":memory:", executes one trivial statement, closes
// cleanly, and returns a project-owned status. No BIM/project schema and no
// on-disk database file are created. No SQLite type appears in this header
// or anywhere outside src/persistence/src/persistence_probe.cpp.
[[nodiscard]] bim::foundation::Status RunSqliteMemoryProbe();

} // namespace bim::persistence
