// Controlled negative fixture for R16/ODA_DRAWINGS_ONLY_DWG_OWNER
// (tests/architecture/CMakeLists.txt: arch_p0_t006_oda_owner_fixture_rejected).
// Deliberately places a raw ODA Drawings token under model/src/ - outside
// src/interop/dwg/**, the sole permitted ODA Drawings owner (Execution
// Packet BIM-AA-P0-T006 v1.0 section 3; src/interop/dwg/CMakeLists.txt's
// own comment). Nothing under tests/fixtures/** is compiled; this file
// exists purely as scan input for tools/architecture_checker.cmake,
// mirroring tests/fixtures/p0_t005_bad_ifc_owner/model/src/leaky_ifc.cpp's
// existing convention exactly.

#include "DbDatabase.h"

void LeakyOdaOwnerViolation() {
    OdDbDatabasePtr db;
    (void)db;
}
