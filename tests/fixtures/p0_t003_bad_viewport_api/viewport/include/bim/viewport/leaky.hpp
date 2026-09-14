#pragma once

// tests/fixtures/p0_t003_bad_viewport_api/viewport/include/bim/viewport/leaky.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R8/VIEWPORT_PUBLIC_NEUTRAL must reject this file. It sits under a
// viewport/include/** path (mirroring the real
// src/viewport/include/bim/viewport/ layout the rule scans) and leaks a
// Qt token into what R8 treats as bim::viewport's public surface - exactly
// the defect class R8 exists to catch (a stray #include<QtCore/QObject>
// or similar creeping into the neutral contract). This file is never
// compiled - it exists only to be scanned by the architecture checker (see
// tests/architecture/CMakeLists.txt, test arch_p0_t003_viewport_fixture_rejected).

#include <QtCore/QObject> // FORBIDDEN: bim::viewport must stay Qt-free

namespace bim::viewport {

class Leaky {
public:
    QObject* not_allowed = nullptr;
};

} // namespace bim::viewport
