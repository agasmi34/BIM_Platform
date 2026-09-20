// Controlled negative fixture for R14/IFC_OPEN_SHELL_ONLY_IFC_OWNER
// (tests/architecture/CMakeLists.txt: arch_p0_t005_ifc_owner_fixture_rejected).
// Deliberately places a raw IfcOpenShell token under model/src/ - outside
// src/interop/ifc/**, the sole permitted IfcOpenShell owner
// (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0 section 7;
// ACR-P0-T005-001 section 7). Nothing under tests/fixtures/** is compiled;
// this file exists purely as scan input for
// tools/architecture_checker.cmake, mirroring
// tests/fixtures/p0_t004_bad_sqlite_outside_persistence/model/src/leaky_sqlite.cpp's
// existing convention exactly.

#include <ifcparse/IfcFile.h>

void LeakyIfcOwnerViolation() {
    IfcOpenShell::IfcParse::IfcFile* file = nullptr;
    (void)file;
}
