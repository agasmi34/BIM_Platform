# bim_document - P1-T002 Document Runtime & Dependency Recompute (Architecture
# Gate BIM-AG-P1-T002; Implementation Brief P1-T002-IB; Implementation
# Authorization BIM-AUTH-P1-T002). The first production document coordination
# layer: it privately owns the live Level / StraightWall state, the runtime
# ElementId <-> graph-node association, the Level -> StraightWall semantic
# dependencies and the derived neutral wall geometry, and it exposes only
# project-owned model / neutral-geometry values and a small result contract.
#
# Dependency surface (frozen by the Architecture Gate):
#   PUBLIC  bim::model         - ElementId / Level / StraightWall / validation
#   PUBLIC  bim::geometry_api  - the neutral LinearExtrusionSpec value only
#   PRIVATE bim::dependency_graph - an implementation detail: no public header
#                                   names a graph type (rule R18,
#                                   DOCUMENT_PUBLIC_RUNTIME_NEUTRAL)
# It deliberately does NOT link bim::geometry_occt (no OCCT solid is built in
# P1-T002), bim::transactions, bim::persistence, SQLite, Qt, bgfx, ODA or
# IfcOpenShell.

add_library(bim_document STATIC
    src/document.cpp
)
add_library(bim::document ALIAS bim_document)

target_link_libraries(bim_document
    PUBLIC
        bim::model
        bim::geometry_api
    PRIVATE
        bim::dependency_graph
)

target_include_directories(bim_document
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(bim_document PUBLIC cxx_std_20)

bim_apply_warnings(bim_document)
bim_apply_sanitizers(bim_document)
