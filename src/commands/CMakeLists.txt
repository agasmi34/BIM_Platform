# bim_commands - P1-T003 Commands & Query Vertical Slice (Architecture Gate
# BIM-AG-P1-T003; Implementation Brief P1-T003-IB). The production command
# mutation boundary: CreateLevel, ChangeLevelElevation, CreateStraightWall,
# ChangeStraightWallGeometry and DeleteElement, each one atomic Document
# mutation. This replaces the P0-T001 compilation anchor (IC-004).
#
# Dependency surface (frozen by the Architecture Gate, section 2):
#   PUBLIC  bim::document - the Document a command acts on; its public headers
#                           carry the model values the commands take.
# It deliberately does NOT link bim::transactions, bim::persistence,
# bim::dependency_graph, bim::geometry_occt, SQLite, Qt, bgfx, ODA or
# IfcOpenShell (rules R3 and COMMANDS_PUBLIC_BOUNDARY,
# tools/architecture_checker.cmake).

add_library(bim_commands STATIC
    src/commands.cpp
)
add_library(bim::commands ALIAS bim_commands)

target_include_directories(bim_commands
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(bim_commands
    PUBLIC
        bim::document
)
target_compile_features(bim_commands PUBLIC cxx_std_20)

bim_apply_warnings(bim_commands)
bim_apply_sanitizers(bim_commands)
