#pragma once

// Deliberately broken fixture for the arch_checker_detects_violation CTest
// test (Implementation Brief Phase J). This header intentionally violates
// AG-006 / Architecture Gate section 8.2 by exposing a raw OCCT type from a
// simulated model public header. It is data for
// tools/architecture_checker.cmake to scan - it is NEVER compiled and MUST
// NEVER be included by any real target. If the architecture checker ever
// stops failing on this file, that is itself a checker regression, not a
// pass.

#include <TopoDS_Shape.hxx>

namespace bim::model {

struct LeakyPublicType {
    TopoDS_Shape shape;
};

} // namespace bim::model
