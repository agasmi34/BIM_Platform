// tests/fixtures/p0_t004_bad_sqlite_outside_persistence/model/src/leaky_sqlite.cpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R12/SQLITE_PERSISTENCE_ONLY must reject this file. It sits under
// model/src/** (outside persistence/**, mirroring the ownership-partition
// pattern R7/R9/R10 already use) and uses raw SQLite API tokens where only
// src/persistence/** is permitted to. This file is never compiled - it
// exists only to be scanned by the architecture checker (see
// tests/architecture/CMakeLists.txt, test
// arch_p0_t004_sqlite_fixture_rejected).

#include <sqlite3.h> // FORBIDDEN: only src/persistence/** may use raw SQLite

namespace bim::model {

int OpenNotAllowedHere() {
    sqlite3* db = nullptr;
    return sqlite3_open(":memory:", &db);
}

} // namespace bim::model
