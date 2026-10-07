# bim_query - P1-T003 Commands & Query Vertical Slice (Architecture Gate
# BIM-AG-P1-T003; Implementation Brief P1-T003-IB). The production read-only
# query boundary: FindLevel, FindStraightWall, ListLevels and ListStraightWalls
# over a const Document. This replaces the P0-T001 compilation anchor (IC-004).
#
# Dependency surface (frozen by the Architecture Gate, section 2):
#   PUBLIC  bim::document - the const Document every query reads
#   PUBLIC  bim::model    - the Level / StraightWall / ElementId values it returns
# It deliberately links nothing that could mutate, persist or render: no
# bim::commands, bim::transactions, bim::persistence, bim::dependency_graph,
# bim::geometry_occt, SQLite, Qt, bgfx, ODA or IfcOpenShell (rule
# QUERY_READ_ONLY_BOUNDARY, tools/architecture_checker.cmake).

add_library(bim_query STATIC
    src/query.cpp
)
add_library(bim::query ALIAS bim_query)

target_include_directories(bim_query
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(bim_query
    PUBLIC
        bim::document
        bim::model
)
target_compile_features(bim_query PUBLIC cxx_std_20)

bim_apply_warnings(bim_query)
bim_apply_sanitizers(bim_query)
