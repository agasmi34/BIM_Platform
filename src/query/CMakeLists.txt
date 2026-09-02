# bim_query - private compilation anchor only in P0-T001 (IC-004). Depends on
# model + foundation, per the locked module dependency law.

add_library(bim_query STATIC
    src/query_anchor.cpp
)
add_library(bim::query ALIAS bim_query)

target_link_libraries(bim_query PRIVATE bim::model bim::foundation)
target_compile_features(bim_query PUBLIC cxx_std_20)

bim_apply_warnings(bim_query)
bim_apply_sanitizers(bim_query)
