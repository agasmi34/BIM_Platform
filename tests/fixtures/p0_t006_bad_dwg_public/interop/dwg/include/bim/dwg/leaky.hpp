#pragma once

// tests/fixtures/p0_t006_bad_dwg_public/interop/dwg/include/bim/dwg/leaky.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R17/DWG_PUBLIC_NEUTRAL must reject this file. It sits under
// interop/dwg/include/** (mirroring the real
// src/interop/dwg/include/bim/dwg/ layout the rule scans) and leaks a raw
// ODA Drawings token into what R17 treats as bim::dwg's public surface -
// exactly the defect class R17 exists to catch (a stray ODA type creeping
// into the neutral contract). This file is never compiled - it exists
// only to be scanned by the architecture checker (see
// tests/architecture/CMakeLists.txt, test
// arch_p0_t006_dwg_public_fixture_rejected).

#include "OdaCommon.h" // FORBIDDEN: bim::dwg's public headers must stay vendor-neutral

namespace bim::dwg {

class Leaky {
public:
    OdDbObjectId not_allowed;
};

} // namespace bim::dwg
