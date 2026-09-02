add_executable(integration_geometry_occt_probe
    integration_geometry_occt_probe.cpp
)
target_link_libraries(integration_geometry_occt_probe
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_probe PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_probe)
add_test(NAME integration_geometry_occt_probe COMMAND integration_geometry_occt_probe)

add_executable(integration_persistence_sqlite_memory
    integration_persistence_sqlite_memory.cpp
)
target_link_libraries(integration_persistence_sqlite_memory
    PRIVATE
        bim::persistence
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_sqlite_memory PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_sqlite_memory)
add_test(NAME integration_persistence_sqlite_memory COMMAND integration_persistence_sqlite_memory)
