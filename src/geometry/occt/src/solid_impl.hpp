#pragma once

// Adapter-private complete definition of bim::geometry_api::Solid
// (Implementation Brief section 6.7; Amendment 01 AA-C05). Included ONLY by
// translation units inside bim_geometry_occt
// (geometry_occt_adapter.cpp, spike_diagnostics.cpp) - never by any header
// under src/geometry/api/, and never installed as a public header. This is
// the sole place in the repository authorized to hold a TopoDS_Shape behind
// bim::geometry_api::Solid.
//
// GEOMETRY_OCCT_ONLY_KERNEL_OWNER (Amendment 01 AA-C09) verifies mechanically
// that no production source/header outside this adapter implementation owns
// or includes an OCCT implementation type.

#include "bim/geometry_api/geometry.hpp"

#include <TopoDS_Shape.hxx>

#include <utility>

namespace bim::geometry_api {

// Completes the incomplete public forward declaration `struct Solid;` from
// geometry_api/geometry.hpp. `shape` is intentionally a plain, non-const
// member so the adapter's own translation units can move/inspect it
// directly; nothing outside bim_geometry_occt can ever see this type (it
// is never reachable through a public geometry_api header), so no mutable
// TopoDS_Shape ever escapes the adapter boundary (Amendment 01 AA-C05: "No
// mutable TopoDS_Shape may escape.").
struct Solid {
    TopoDS_Shape shape;

    explicit Solid(TopoDS_Shape s) : shape(std::move(s)) {}
};

} // namespace bim::geometry_api
