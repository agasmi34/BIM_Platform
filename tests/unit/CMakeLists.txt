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

# unit_geometry_api_contract - P0-T002 Phase C-M (Implementation Brief
# section 23; Amendment 01 AA-C08: "must link ONLY bim::geometry_api" - this
# target deliberately does NOT link bim::geometry_occt, so a successful
# build of this target is itself mechanical proof that the public
# geometry_api surface exercised here needs no OCCT symbol).
add_executable(unit_geometry_api_contract
    unit_geometry_api_contract.cpp
)
target_link_libraries(unit_geometry_api_contract
    PRIVATE
        bim::geometry_api
        Catch2::Catch2WithMain
)
target_compile_features(unit_geometry_api_contract PRIVATE cxx_std_20)
bim_apply_warnings(unit_geometry_api_contract)
add_test(NAME unit_geometry_api_contract COMMAND unit_geometry_api_contract)
