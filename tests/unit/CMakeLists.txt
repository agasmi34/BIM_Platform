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

# unit_model_face_reference_resolver - P0-T008 Topological Reference Spike
# (Implementation Brief BIM-TASK-P0-T008-CLAUDE v1.0). Links bim::model and
# bim::geometry_api only, deliberately never bim::geometry_occt - a
# successful build of this target is itself mechanical proof that
# ResolveFaceReference() and every type it depends on need no OCCT symbol.
# Every FaceObservation this file exercises is hand-constructed data, never
# produced by a real kernel solid (the real-kernel proof corpus lives under
# tests/integration/ instead) - this is where the mandatory model-level,
# kernel-free enumeration-order-independence test lives, along with corpus
# cases F (split face -> Ambiguous) and H (invalid owner / invalid
# reference).
add_executable(unit_model_face_reference_resolver
    unit_model_face_reference_resolver.cpp
)
target_link_libraries(unit_model_face_reference_resolver
    PRIVATE
        bim::model
        bim::geometry_api
        Catch2::Catch2WithMain
)
target_compile_features(unit_model_face_reference_resolver PRIVATE cxx_std_20)
bim_apply_warnings(unit_model_face_reference_resolver)
add_test(NAME unit_model_face_reference_resolver COMMAND unit_model_face_reference_resolver)

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

# --- P0-T005 IFC Spike (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0
# section 9 / Execution Packet v1.1 section 9's "unit_ifc_probe_contract").
# Links bim::ifc, which is only defined when BIM_ENABLE_IFC is ON (root
# CMakeLists.txt) - see src/interop/ifc/CMakeLists.txt's own comment on why
# this module, unlike every other one here, is not always available.
if(BIM_ENABLE_IFC)
    add_executable(unit_ifc_probe_contract
        unit_ifc_probe_contract.cpp
    )
    target_link_libraries(unit_ifc_probe_contract
        PRIVATE
            bim::ifc
            Catch2::Catch2WithMain
    )
    target_compile_features(unit_ifc_probe_contract PRIVATE cxx_std_20)
    bim_apply_warnings(unit_ifc_probe_contract)
    add_test(NAME unit_ifc_probe_contract COMMAND unit_ifc_probe_contract)
endif()

# --- P0-T006 DWG / ODA Evaluation (Execution Packet BIM-AA-P0-T006 v1.0
# sections 6, 10; AA CP2B-H3 clarification's "unit_dwg_probe_contract").
# Links bim::dwg, which is only defined when BIM_ENABLE_DWG is ON (root
# CMakeLists.txt) - see src/interop/dwg/CMakeLists.txt's own comment on why
# this module, like bim::ifc above, is not always available.
if(BIM_ENABLE_DWG)
    add_executable(unit_dwg_probe_contract
        unit_dwg_probe_contract.cpp
    )
    target_link_libraries(unit_dwg_probe_contract
        PRIVATE
            bim::dwg
            Catch2::Catch2WithMain
    )
    target_compile_features(unit_dwg_probe_contract PRIVATE cxx_std_20)
    bim_apply_warnings(unit_dwg_probe_contract)
    add_test(NAME unit_dwg_probe_contract COMMAND unit_dwg_probe_contract)

    # [CP3-H4 minimum-delta correction] Stage the AA-authorized 21-module
    # controlled Phase-0 sufficient ODA runtime set beside this executable's
    # own build output. bim_dwg is STATIC, so the Windows loader resolves
    # these PE modules from the executable's own runtime directory, not
    # from beside a static archive (AA CP3-H4 section 9). The file list is
    # resolved/validated once in src/interop/dwg/CMakeLists.txt and
    # published via a GLOBAL PROPERTY, read back here rather than
    # rediscovered - one place names the 21 filenames. No wildcard/glob
    # copy; only this one DWG executable in this file receives it.
    get_property(_bim_p0_t006_dwg_runtime_files GLOBAL PROPERTY BIM_P0_T006_DWG_RUNTIME_FILES)
    add_custom_command(TARGET unit_dwg_probe_contract POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${_bim_p0_t006_dwg_runtime_files}
            "$<TARGET_FILE_DIR:unit_dwg_probe_contract>"
        COMMENT "Staging the AA CP3-H4 controlled 21-module ODA runtime set beside unit_dwg_probe_contract"
        VERBATIM
    )
endif()
