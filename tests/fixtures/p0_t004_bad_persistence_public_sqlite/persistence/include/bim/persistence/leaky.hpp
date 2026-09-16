#pragma once

// tests/fixtures/p0_t004_bad_persistence_public_sqlite/persistence/include/bim/persistence/leaky.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R13/PERSISTENCE_PUBLIC_NEUTRAL must reject this file. It sits under
// persistence/include/** (mirroring the real
// src/persistence/include/bim/persistence/ layout the rule scans) and
// leaks a raw SQLite token into what R13 treats as bim::persistence's
// public surface - exactly the defect class R13 exists to catch (a stray
// #include<sqlite3.h> or similar creeping into the neutral contract).
// This file is never compiled - it exists only to be scanned by the
// architecture checker (see tests/architecture/CMakeLists.txt, test
// arch_p0_t004_persistence_public_fixture_rejected).

#include <sqlite3.h> // FORBIDDEN: bim::persistence's public headers must stay vendor-neutral

namespace bim::persistence {

class Leaky {
public:
    sqlite3* not_allowed = nullptr;
};

} // namespace bim::persistence
