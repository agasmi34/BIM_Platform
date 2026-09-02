add_executable(unit_foundation_smoke
    unit_foundation_smoke.cpp
)
target_link_libraries(unit_foundation_smoke
    PRIVATE
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(unit_foundation_smoke PRIVATE cxx_std_20)
bim_apply_warnings(unit_foundation_smoke)
add_test(NAME unit_foundation_smoke COMMAND unit_foundation_smoke)

add_executable(unit_model_links_foundation
    unit_model_links_foundation.cpp
)
target_link_libraries(unit_model_links_foundation
    PRIVATE
        bim::model
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(unit_model_links_foundation PRIVATE cxx_std_20)
bim_apply_warnings(unit_model_links_foundation)
add_test(NAME unit_model_links_foundation COMMAND unit_model_links_foundation)
