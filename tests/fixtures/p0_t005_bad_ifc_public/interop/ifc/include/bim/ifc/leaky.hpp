#pragma once

// Controlled negative fixture for R15/IFC_PUBLIC_NEUTRAL
// (tests/architecture/CMakeLists.txt: arch_p0_t005_ifc_public_fixture_rejected).
// Deliberately leaks a raw IfcOpenShell type into a simulated
// interop/ifc/include/** public header (Implementation Brief
// BIM-TASK-P0-T005-CLAUDE v1.0 section 8/13; ACR-P0-T005-001 section 8).
// Nothing under tests/fixtures/** is compiled; this file exists purely as
// scan input for tools/architecture_checker.cmake, mirroring
// tests/fixtures/p0_t004_bad_persistence_public_sqlite/persistence/include/bim/persistence/leaky.hpp's
// existing convention exactly.

namespace bim::ifc {

// A real bim::ifc public header must never declare a member of an
// IfcOpenShell type - this is exactly the leak R15 exists to reject.
struct LeakyPublicContract {
    // Forbidden token present in real code (not merely a comment) - R15
    // uses a comment-aware scan, so the violation must sit here, in an
    // actual declaration, to be caught. IfcParse::IfcFile is intentionally
    // left undeclared/unincluded: nothing under tests/fixtures/** is
    // compiled (see this file's own header comment).
    IfcParse::IfcFile* handle = nullptr;
};

} // namespace bim::ifc
