# bim_dependency_graph - P0-T009 Dependency Graph Spike: an in-memory,
# single-threaded, deterministic dependency DAG with provenance, dirty
# propagation and atomic recompute planning/commit (Architecture Gate
# BIM-AG-P0-T009 v1.0; Implementation Brief P0-T009-IB v1.0; Implementation
# Authorization BIM-AUTH-P0-T009 v1.0). Depends only on foundation, per the
# locked module dependency law (Architecture Gate section 17 /
# Implementation Authorization section 6).

add_library(bim_dependency_graph STATIC
    src/dependency_graph.cpp
)
add_library(bim::dependency_graph ALIAS bim_dependency_graph)

target_include_directories(bim_dependency_graph PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bim_dependency_graph PRIVATE bim::foundation)
target_compile_features(bim_dependency_graph PUBLIC cxx_std_20)

bim_apply_warnings(bim_dependency_graph)
bim_apply_sanitizers(bim_dependency_graph)
