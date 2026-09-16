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

# --- P0-T003 Desktop + Viewport Spike (Implementation Brief
# BIM-TASK-P0-T003-CLAUDE v1.0 section 24) - the neutral bim::viewport
# contract's unit tests. Each links ONLY bim::viewport (never
# bim::viewport_bgfx) - a successful build of these targets is itself
# mechanical proof that bim::viewport needs no bgfx/Qt/D3D symbol, mirroring
# unit_geometry_api_contract's "link only the neutral layer" pattern above.

add_executable(unit_viewport_mesh_contract
    unit_viewport_mesh_contract.cpp
)
target_link_libraries(unit_viewport_mesh_contract
    PRIVATE
        bim::viewport
        Catch2::Catch2WithMain
)
target_compile_features(unit_viewport_mesh_contract PRIVATE cxx_std_20)
bim_apply_warnings(unit_viewport_mesh_contract)
add_test(NAME unit_viewport_mesh_contract COMMAND unit_viewport_mesh_contract)

add_executable(unit_viewport_camera
    unit_viewport_camera.cpp
)
target_link_libraries(unit_viewport_camera
    PRIVATE
        bim::viewport
        Catch2::Catch2WithMain
)
target_compile_features(unit_viewport_camera PRIVATE cxx_std_20)
bim_apply_warnings(unit_viewport_camera)
add_test(NAME unit_viewport_camera COMMAND unit_viewport_camera)

add_executable(unit_viewport_ray
    unit_viewport_ray.cpp
)
target_link_libraries(unit_viewport_ray
    PRIVATE
        bim::viewport
        Catch2::Catch2WithMain
)
target_compile_features(unit_viewport_ray PRIVATE cxx_std_20)
bim_apply_warnings(unit_viewport_ray)
add_test(NAME unit_viewport_ray COMMAND unit_viewport_ray)

add_executable(unit_viewport_lifecycle
    unit_viewport_lifecycle.cpp
)
target_link_libraries(unit_viewport_lifecycle
    PRIVATE
        bim::viewport
        Catch2::Catch2WithMain
)
target_compile_features(unit_viewport_lifecycle PRIVATE cxx_std_20)
bim_apply_warnings(unit_viewport_lifecycle)
add_test(NAME unit_viewport_lifecycle COMMAND unit_viewport_lifecycle)

# unit_transactions_journal_contract - P0-T004 Phase B (Implementation
# Brief BIM-TASK-P0-T004-CLAUDE v1.0 section 16 / Architecture Gate
# BIM-AG-P0-T004 v1.0 section 5). Links ONLY bim::transactions (never
# bim::persistence) - a successful build is itself mechanical proof the
# neutral journal contract needs no persistence/SQLite symbol.
add_executable(unit_transactions_journal_contract
    unit_transactions_journal_contract.cpp
)
target_link_libraries(unit_transactions_journal_contract
    PRIVATE
        bim::transactions
        Catch2::Catch2WithMain
)
target_compile_features(unit_transactions_journal_contract PRIVATE cxx_std_20)
bim_apply_warnings(unit_transactions_journal_contract)
add_test(NAME unit_transactions_journal_contract COMMAND unit_transactions_journal_contract)
