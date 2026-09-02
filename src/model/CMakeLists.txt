# bim_model - private compilation anchor only in P0-T001 (IC-004). When a
# public contract is introduced by a later task, it MUST live under
# include/bim/model/... and MUST NOT expose OCCT/Qt/SQLite/ODA/IfcOpenShell
# types (Architecture Gate section 8.2 / AG-006). This rule is checked
# mechanically by tools/architecture_checker.cmake (arch_repository_boundaries).

add_library(bim_model STATIC
    src/model_anchor.cpp
)
add_library(bim::model ALIAS bim_model)

target_link_libraries(bim_model PRIVATE bim::foundation)
target_compile_features(bim_model PUBLIC cxx_std_20)

bim_apply_warnings(bim_model)
bim_apply_sanitizers(bim_model)
