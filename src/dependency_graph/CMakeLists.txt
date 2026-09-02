# bim_dependency_graph - private compilation anchor only in P0-T001 (IC-004).
# Depends only on foundation, per the locked module dependency law
# (Architecture Gate section 8.1 / Implementation Brief section 7).

add_library(bim_dependency_graph STATIC
    src/dependency_graph_anchor.cpp
)
add_library(bim::dependency_graph ALIAS bim_dependency_graph)

target_link_libraries(bim_dependency_graph PRIVATE bim::foundation)
target_compile_features(bim_dependency_graph PUBLIC cxx_std_20)

bim_apply_warnings(bim_dependency_graph)
bim_apply_sanitizers(bim_dependency_graph)
