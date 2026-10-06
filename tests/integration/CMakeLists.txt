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

# --- P0-T002 Phase C-M geometry corpus/matrix/history/repeatability tests
# (Implementation Brief sections 14-19, 23). geometry_occt_test_constants.hpp
# is a shared, non-executable header (no add_executable of its own) included
# directly by every target below via a same-directory quote-include.

add_executable(integration_geometry_occt_primitive
    integration_geometry_occt_primitive.cpp
)
target_link_libraries(integration_geometry_occt_primitive
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_primitive PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_primitive)
add_test(NAME integration_geometry_occt_primitive COMMAND integration_geometry_occt_primitive)

add_executable(integration_geometry_occt_opening_cut
    integration_geometry_occt_opening_cut.cpp
)
target_link_libraries(integration_geometry_occt_opening_cut
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_opening_cut PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_opening_cut)
add_test(NAME integration_geometry_occt_opening_cut COMMAND integration_geometry_occt_opening_cut)

add_executable(integration_geometry_occt_join
    integration_geometry_occt_join.cpp
)
target_link_libraries(integration_geometry_occt_join
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_join PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_join)
add_test(NAME integration_geometry_occt_join COMMAND integration_geometry_occt_join)

add_executable(integration_geometry_occt_failure_corpus
    integration_geometry_occt_failure_corpus.cpp
)
target_link_libraries(integration_geometry_occt_failure_corpus
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_failure_corpus PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_failure_corpus)
add_test(NAME integration_geometry_occt_failure_corpus COMMAND integration_geometry_occt_failure_corpus)

add_executable(integration_geometry_occt_tolerance_matrix
    integration_geometry_occt_tolerance_matrix.cpp
)
target_link_libraries(integration_geometry_occt_tolerance_matrix
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_tolerance_matrix PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_tolerance_matrix)
add_test(NAME integration_geometry_occt_tolerance_matrix COMMAND integration_geometry_occt_tolerance_matrix)

add_executable(integration_geometry_occt_coordinate_matrix
    integration_geometry_occt_coordinate_matrix.cpp
)
target_link_libraries(integration_geometry_occt_coordinate_matrix
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_coordinate_matrix PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_coordinate_matrix)
add_test(NAME integration_geometry_occt_coordinate_matrix COMMAND integration_geometry_occt_coordinate_matrix)

add_executable(integration_geometry_occt_history
    integration_geometry_occt_history.cpp
)
target_link_libraries(integration_geometry_occt_history
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_history PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_history)
add_test(NAME integration_geometry_occt_history COMMAND integration_geometry_occt_history)

add_executable(integration_geometry_occt_repeatability
    integration_geometry_occt_repeatability.cpp
)
target_link_libraries(integration_geometry_occt_repeatability
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_geometry_occt_repeatability PRIVATE cxx_std_20)
bim_apply_warnings(integration_geometry_occt_repeatability)
add_test(NAME integration_geometry_occt_repeatability COMMAND integration_geometry_occt_repeatability)

# --- P0-T002 standalone evidence executable (Implementation Brief section
# 20; Amendment 01 AA-C07). Not a Catch2 target - it has its own main() and
# is wrapped by a CTest test that invokes it with an explicit --json path
# under the build tree (never the source tree), so `ctest -R
# "^integration_geometry_occt_evidence$"` both exercises the full corpus a
# second time (via the production code path, standalone from Catch2) and
# regenerates a fresh evidence JSON on every CTest run.

add_executable(p0_t002_geometry_evidence
    p0_t002_geometry_evidence.cpp
)
target_link_libraries(p0_t002_geometry_evidence
    PRIVATE
        bim::geometry_api
        bim::geometry_occt
)
target_compile_features(p0_t002_geometry_evidence PRIVATE cxx_std_20)
bim_apply_warnings(p0_t002_geometry_evidence)

add_test(
    NAME integration_geometry_occt_evidence
    COMMAND p0_t002_geometry_evidence --json ${CMAKE_BINARY_DIR}/evidence/P0-T002/p0_t002_geometry_evidence.ctest.json
)

# --- P0-T003 Desktop + Viewport Spike (Implementation Brief
# BIM-TASK-P0-T003-CLAUDE v1.0 section 24; section 12 "headless/test
# split") - bim::viewport_bgfx integration tests. Both run against bgfx's
# headless/noop backend (RendererCreateInfo::headless = true), so they
# require no live D3D11 device and no visible window - safe under CI/CTest
# exactly like integration_geometry_occt_probe requires no interactive
# session.

add_executable(integration_viewport_bgfx_headless
    integration_viewport_bgfx_headless.cpp
)
target_link_libraries(integration_viewport_bgfx_headless
    PRIVATE
        bim::viewport
        bim::viewport_bgfx
        Catch2::Catch2WithMain
)
target_compile_features(integration_viewport_bgfx_headless PRIVATE cxx_std_20)
bim_apply_warnings(integration_viewport_bgfx_headless)
add_test(NAME integration_viewport_bgfx_headless COMMAND integration_viewport_bgfx_headless)

add_executable(integration_viewport_bgfx_resource_lifecycle
    integration_viewport_bgfx_resource_lifecycle.cpp
)
target_link_libraries(integration_viewport_bgfx_resource_lifecycle
    PRIVATE
        bim::viewport
        bim::viewport_bgfx
        Catch2::Catch2WithMain
)
target_compile_features(integration_viewport_bgfx_resource_lifecycle PRIVATE cxx_std_20)
bim_apply_warnings(integration_viewport_bgfx_resource_lifecycle)
add_test(NAME integration_viewport_bgfx_resource_lifecycle COMMAND integration_viewport_bgfx_resource_lifecycle)

# --- P0-T004 Phase D Schema v1 Bootstrap + Validation (Implementation
# Brief BIM-TASK-P0-T004-CLAUDE v1.0 Phase D). Exercises the private
# bim::persistence::detail schema-v1 logic directly via file-relative
# quoted includes into src/persistence/src/detail/ (no public persistence
# surface exposes this - Architecture Gate AG-P0T004-001/002), so this
# target links the same bim::persistence + bim::foundation libraries as
# integration_persistence_sqlite_memory above rather than any new library.

add_executable(integration_persistence_schema_v1
    integration_persistence_schema_v1.cpp
)
target_link_libraries(integration_persistence_schema_v1
    PRIVATE
        bim::persistence
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_schema_v1 PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_schema_v1)
add_test(NAME integration_persistence_schema_v1 COMMAND integration_persistence_schema_v1)

# --- P0-T004 Phase E Journal Persistence (Implementation Brief
# BIM-TASK-P0-T004-CLAUDE v1.0 Phase E). Exercises the private
# bim::persistence::detail journal_store (append/read) logic together with
# the frozen Phase D schema-v1 open/bootstrap logic, directly via
# file-relative quoted includes into src/persistence/src/detail/ (no
# public persistence surface exposes any of this - Architecture Gate
# AG-P0T004-001/002). persistence_journal_test_support.hpp is a shared,
# non-executable header (no add_executable of its own) included directly
# by every target below via a same-directory quote-include, following the
# existing geometry_occt_test_constants.hpp convention above.

add_executable(integration_persistence_journal_commit_reopen
    integration_persistence_journal_commit_reopen.cpp
)
target_link_libraries(integration_persistence_journal_commit_reopen
    PRIVATE
        bim::persistence
        bim::transactions
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_journal_commit_reopen PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_journal_commit_reopen)
add_test(NAME integration_persistence_journal_commit_reopen COMMAND integration_persistence_journal_commit_reopen)

add_executable(integration_persistence_journal_duplicate_id
    integration_persistence_journal_duplicate_id.cpp
)
target_link_libraries(integration_persistence_journal_duplicate_id
    PRIVATE
        bim::persistence
        bim::transactions
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_journal_duplicate_id PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_journal_duplicate_id)
add_test(NAME integration_persistence_journal_duplicate_id COMMAND integration_persistence_journal_duplicate_id)

add_executable(integration_persistence_journal_binary_payload
    integration_persistence_journal_binary_payload.cpp
)
target_link_libraries(integration_persistence_journal_binary_payload
    PRIVATE
        bim::persistence
        bim::transactions
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_journal_binary_payload PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_journal_binary_payload)
add_test(NAME integration_persistence_journal_binary_payload COMMAND integration_persistence_journal_binary_payload)

add_executable(integration_persistence_journal_rollback
    integration_persistence_journal_rollback.cpp
)
target_link_libraries(integration_persistence_journal_rollback
    PRIVATE
        bim::persistence
        bim::transactions
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_persistence_journal_rollback PRIVATE cxx_std_20)
bim_apply_warnings(integration_persistence_journal_rollback)
add_test(NAME integration_persistence_journal_rollback COMMAND integration_persistence_journal_rollback)

# --- P0-T004 Phase G standalone evidence executable (Implementation Brief
# BIM-TASK-P0-T004-CLAUDE v1.0 section 22; Architecture Gate section 14;
# Phase G Execution Handoff v1.0). Not a Catch2 target - it has its own
# main() - mirroring p0_t002_geometry_evidence above exactly: links the
# SAME production bim::persistence implementation exercised by the Phase
# D/E integration tests above (never a separate copy), and every evidence
# field is recorded via hand-rolled JSON serialization inside the
# executable itself (bim::persistence::RunPersistenceSpike returns only a
# plain project-owned PersistenceSpikeEvidence struct - see
# src/persistence/include/bim/persistence/probe.hpp - so no JSON dependency
# is introduced anywhere in src/**). Registered under CTest name
# integration_persistence_evidence, following the
# integration_geometry_occt_evidence naming precedent, and writes its JSON
# into the build tree (never the source tree), matching
# p0_t002_geometry_evidence's own --json ${CMAKE_BINARY_DIR}/evidence/...
# convention exactly.

add_executable(p0_t004_persistence_evidence
    p0_t004_persistence_evidence.cpp
)
target_link_libraries(p0_t004_persistence_evidence
    PRIVATE
        bim::persistence
        bim::foundation
)
target_compile_features(p0_t004_persistence_evidence PRIVATE cxx_std_20)
bim_apply_warnings(p0_t004_persistence_evidence)

add_test(
    NAME integration_persistence_evidence
    COMMAND p0_t004_persistence_evidence --json ${CMAKE_BINARY_DIR}/evidence/P0-T004/p0_t004_persistence_evidence.ctest.json
)

# --- P0-T005 IFC Spike (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0
# sections 9-15; Execution Packet v1.1 sections 9, 12). Every target below
# links bim::ifc, which is only defined when BIM_ENABLE_IFC is ON (root
# CMakeLists.txt) - see src/interop/ifc/CMakeLists.txt's own comment.
# ifc_test_support.hpp is a shared, non-executable header (no
# add_executable of its own) included directly by every target below via a
# same-directory quote-include, following the existing
# geometry_occt_test_constants.hpp / persistence_journal_test_support.hpp
# convention above. BIM_P0_T005_IFC_FIXTURES_DIR is the absolute
# source-tree path to the malformed/unsupported-schema fixtures (Brief
# section 11) - injected at compile time rather than derived at runtime, so
# every target below resolves the exact same fixture files regardless of
# the working directory CTest launches them from. Declared once via
# add_compile_definitions() (directory scope, applies to every target added
# below this point in this file only - every other target in this file was
# already defined earlier and is unaffected) rather than repeated per
# target, since ifc_test_support.hpp's own #ifndef/#error guard (see that
# header) requires the macro to be defined in every translation unit that
# includes it, not only the ones that call FixturesDir().
if(BIM_ENABLE_IFC)
    set(BIM_P0_T005_IFC_FIXTURES_DIR "${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t005_ifc")
    add_compile_definitions(BIM_P0_T005_IFC_FIXTURES_DIR="${BIM_P0_T005_IFC_FIXTURES_DIR}")

    add_executable(integration_ifc_round_trip
        integration_ifc_round_trip.cpp
    )
    target_link_libraries(integration_ifc_round_trip
        PRIVATE
            bim::ifc
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_ifc_round_trip PRIVATE cxx_std_20)
    bim_apply_warnings(integration_ifc_round_trip)
    add_test(NAME integration_ifc_round_trip COMMAND integration_ifc_round_trip)

    add_executable(integration_ifc_missing_file
        integration_ifc_missing_file.cpp
    )
    target_link_libraries(integration_ifc_missing_file
        PRIVATE
            bim::ifc
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_ifc_missing_file PRIVATE cxx_std_20)
    bim_apply_warnings(integration_ifc_missing_file)
    add_test(NAME integration_ifc_missing_file COMMAND integration_ifc_missing_file)

    add_executable(integration_ifc_malformed_input
        integration_ifc_malformed_input.cpp
    )
    target_link_libraries(integration_ifc_malformed_input
        PRIVATE
            bim::ifc
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_ifc_malformed_input PRIVATE cxx_std_20)
    bim_apply_warnings(integration_ifc_malformed_input)
    add_test(NAME integration_ifc_malformed_input COMMAND integration_ifc_malformed_input)

    add_executable(integration_ifc_unsupported_schema
        integration_ifc_unsupported_schema.cpp
    )
    target_link_libraries(integration_ifc_unsupported_schema
        PRIVATE
            bim::ifc
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_ifc_unsupported_schema PRIVATE cxx_std_20)
    bim_apply_warnings(integration_ifc_unsupported_schema)
    add_test(NAME integration_ifc_unsupported_schema COMMAND integration_ifc_unsupported_schema)

    # Standalone evidence executable (Brief section 15; Packet section 12) -
    # mirrors p0_t002_geometry_evidence.cpp / p0_t004_persistence_evidence.cpp
    # exactly: not a Catch2 target, has its own main(), links the SAME
    # production bim::ifc implementation exercised by the Catch2 targets
    # above (never a separate copy). Registered under CTest name
    # integration_ifc_evidence, matching Brief section 12's required test
    # name directly (unlike P0-T002/P0-T004, no separate p0_t00N_* /
    # integration_* naming split is needed here).
    add_executable(p0_t005_ifc_evidence
        p0_t005_ifc_evidence.cpp
    )
    target_link_libraries(p0_t005_ifc_evidence
        PRIVATE
            bim::ifc
    )
    target_compile_features(p0_t005_ifc_evidence PRIVATE cxx_std_20)
    bim_apply_warnings(p0_t005_ifc_evidence)

    add_test(
        NAME integration_ifc_evidence
        COMMAND p0_t005_ifc_evidence --json ${CMAKE_BINARY_DIR}/evidence/P0-T005/p0_t005_ifc_evidence.ctest.json
    )
endif()

# --- P0-T006 DWG / ODA Evaluation (Execution Packet BIM-AA-P0-T006 v1.0
# sections 6-11; AA CP2B-H3 clarification's required test list). Every
# target below links bim::dwg, which is only defined when BIM_ENABLE_DWG is
# ON (root CMakeLists.txt) - see src/interop/dwg/CMakeLists.txt's own
# comment. dwg_test_support.hpp is a shared, non-executable header (no
# add_executable of its own) included directly by every target below via a
# same-directory quote-include, following the existing
# ifc_test_support.hpp convention above exactly.
#
# BIM_P0_T006_DWG_FIXTURE is the absolute path to the locked, vendor
# -supplied AC1018 controlled fixture ("OdWriteEx XRef.dwg", shipped inside
# the ODA Drawings SDK's own Drawing/Examples/OdWriteEx/ tree - Execution
# Packet section 8's "Controlled fixture" / "Repository/config interfaces
# remain: BIM_ODA_DRAWINGS_ROOT, BIM_P0_T006_DWG_FIXTURE"). Unlike
# BIM_P0_T005_IFC_FIXTURES_DIR (first-party-authored fixtures living inside
# this repository's own tests/fixtures/), this fixture is third-party
# vendor example content under the locked trial SDK's own license and is
# never copied into this repository - so its absolute path must be supplied
# explicitly by the Windows Execution Operator (e.g.
# -DBIM_P0_T006_DWG_FIXTURE="D:/Dependencies/ODA/.../OdWriteEx XRef.dwg"),
# and this build fails clearly (FATAL_ERROR) rather than silently
# proceeding if BIM_ENABLE_DWG is ON but this variable was left unset.
# Injected at compile time via add_compile_definitions() (directory scope,
# applies only to the targets added below this point in this file), exactly
# mirroring BIM_P0_T005_IFC_FIXTURES_DIR's own injection pattern.
if(BIM_ENABLE_DWG)
    if(NOT DEFINED BIM_P0_T006_DWG_FIXTURE OR BIM_P0_T006_DWG_FIXTURE STREQUAL "")
        message(FATAL_ERROR
            "tests/integration: BIM_ENABLE_DWG is ON but BIM_P0_T006_DWG_FIXTURE "
            "is not set. Set it to the absolute path of the locked vendor AC1018 "
            "fixture 'OdWriteEx XRef.dwg' (Execution Packet BIM-AA-P0-T006 v1.0 "
            "section 8), for example:\n"
            "  cmake --preset <preset> -DBIM_ENABLE_DWG=ON "
            "-DBIM_ODA_DRAWINGS_ROOT=<path> "
            "-DBIM_P0_T006_DWG_FIXTURE=\"<path>/Drawing/Examples/OdWriteEx/OdWriteEx XRef.dwg\"")
    endif()
    # [CP3-H1 minimum-delta correction] BIM_P0_T006_DWG_FIXTURE arrives from
    # the operator as a native Windows path (backslash-separated). Injecting
    # that raw value into a C++ string literal via add_compile_definitions()
    # makes MSVC interpret "\D", "\O", "\E" etc. as (invalid) escape
    # sequences - C4129, promoted to C2220 under /WX. Normalize to
    # forward slashes first (CMake accepts '/' on Windows) and inject the
    # normalized variable instead. One normalized variable, reused below by
    # every P0-T006 integration/evidence target via this same directory-scope
    # add_compile_definitions() call - no other target/target-registration
    # structure in this file changes.
    string(REPLACE "\\" "/" _bim_p0_t006_dwg_fixture_cpp "${BIM_P0_T006_DWG_FIXTURE}")
    add_compile_definitions(BIM_P0_T006_DWG_FIXTURE="${_bim_p0_t006_dwg_fixture_cpp}")

    add_executable(integration_dwg_missing_file
        integration_dwg_missing_file.cpp
    )
    target_link_libraries(integration_dwg_missing_file
        PRIVATE
            bim::dwg
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_dwg_missing_file PRIVATE cxx_std_20)
    bim_apply_warnings(integration_dwg_missing_file)
    add_test(NAME integration_dwg_missing_file COMMAND integration_dwg_missing_file)

    add_executable(integration_dwg_same_path_rejected
        integration_dwg_same_path_rejected.cpp
    )
    target_link_libraries(integration_dwg_same_path_rejected
        PRIVATE
            bim::dwg
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_dwg_same_path_rejected PRIVATE cxx_std_20)
    bim_apply_warnings(integration_dwg_same_path_rejected)
    add_test(NAME integration_dwg_same_path_rejected COMMAND integration_dwg_same_path_rejected)

    add_executable(integration_dwg_vendor_fixture_read
        integration_dwg_vendor_fixture_read.cpp
    )
    target_link_libraries(integration_dwg_vendor_fixture_read
        PRIVATE
            bim::dwg
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_dwg_vendor_fixture_read PRIVATE cxx_std_20)
    bim_apply_warnings(integration_dwg_vendor_fixture_read)
    add_test(NAME integration_dwg_vendor_fixture_read COMMAND integration_dwg_vendor_fixture_read)

    add_executable(integration_dwg_same_version_round_trip
        integration_dwg_same_version_round_trip.cpp
    )
    target_link_libraries(integration_dwg_same_version_round_trip
        PRIVATE
            bim::dwg
            Catch2::Catch2WithMain
    )
    target_compile_features(integration_dwg_same_version_round_trip PRIVATE cxx_std_20)
    bim_apply_warnings(integration_dwg_same_version_round_trip)
    add_test(NAME integration_dwg_same_version_round_trip COMMAND integration_dwg_same_version_round_trip)

    # Standalone evidence executable (AA CP2B-H3 clarification's
    # "integration_dwg_evidence") - mirrors p0_t005_ifc_evidence.cpp exactly:
    # not a Catch2 target, has its own main(), links the SAME production
    # bim::dwg implementation exercised by the Catch2 targets above (never a
    # separate copy).
    add_executable(p0_t006_dwg_evidence
        p0_t006_dwg_evidence.cpp
    )
    target_link_libraries(p0_t006_dwg_evidence
        PRIVATE
            bim::dwg
    )
    target_compile_features(p0_t006_dwg_evidence PRIVATE cxx_std_20)
    bim_apply_warnings(p0_t006_dwg_evidence)

    add_test(
        NAME integration_dwg_evidence
        COMMAND p0_t006_dwg_evidence --json ${CMAKE_BINARY_DIR}/evidence/P0-T006/p0_t006_dwg_evidence.ctest.json
    )

    # [CP3-H4 minimum-delta correction] Stage the AA-authorized 21-module
    # controlled Phase-0 sufficient ODA runtime set beside each of these
    # five DWG executables' own build output. bim_dwg is STATIC, so the
    # Windows loader resolves these PE modules from each executable's own
    # runtime directory, not from beside a static archive (AA CP3-H4
    # section 9). The file list is resolved/validated once in
    # src/interop/dwg/CMakeLists.txt and published via a GLOBAL PROPERTY,
    # read back here rather than rediscovered - one place names the 21
    # filenames. No wildcard/glob copy; only these five DWG executables in
    # this file receive it (integration_dwg_same_path_rejected and
    # integration_dwg_missing_file never actually reach ODA - they are
    # rejected/fail before any ODA call - but they still link bim::dwg and
    # therefore still need these modules resolvable at process load time).
    get_property(_bim_p0_t006_dwg_runtime_files GLOBAL PROPERTY BIM_P0_T006_DWG_RUNTIME_FILES)
    foreach(_bim_p0_t006_dwg_runtime_target IN ITEMS
        integration_dwg_missing_file
        integration_dwg_same_path_rejected
        integration_dwg_vendor_fixture_read
        integration_dwg_same_version_round_trip
        p0_t006_dwg_evidence
    )
        add_custom_command(TARGET ${_bim_p0_t006_dwg_runtime_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_bim_p0_t006_dwg_runtime_files}
                "$<TARGET_FILE_DIR:${_bim_p0_t006_dwg_runtime_target}>"
            COMMENT "Staging the AA CP3-H4 controlled 21-module ODA runtime set beside ${_bim_p0_t006_dwg_runtime_target}"
            VERBATIM
        )
    endforeach()
endif()

# --- P0-T008 Topological Reference Spike (Implementation Brief
# BIM-TASK-P0-T008-CLAUDE v1.0) - the real-kernel end of the mandatory proof
# corpus (cases A/B/C/D/E/G). Links bim::model, bim::geometry_api, and
# bim::geometry_occt together - the only place in this test suite all three
# are linked into one executable, since this is the only place that needs
# to build real solids (bim::geometry_occt), observe their real faces
# (bim::geometry_api::ObserveFaces, implemented in bim::geometry_occt), and
# resolve persistent references against them (bim::model), all in the same
# process. geometry_occt_test_constants.hpp is reused unchanged for its
# kReferenceTolerance/MakeP01/MakeAxisAlignedBox helpers (same
# same-directory quote-include convention integration_geometry_occt_*.cpp
# already use above). Registered unconditionally, like every other
# geometry/model target in this file - bim_model and bim_geometry_api/occt
# are always available, gated by nothing.
add_executable(integration_p0_t008_face_reference_spike
    integration_p0_t008_face_reference_spike.cpp
)
target_link_libraries(integration_p0_t008_face_reference_spike
    PRIVATE
        bim::model
        bim::geometry_api
        bim::geometry_occt
        Catch2::Catch2WithMain
)
target_compile_features(integration_p0_t008_face_reference_spike PRIVATE cxx_std_20)
bim_apply_warnings(integration_p0_t008_face_reference_spike)
add_test(NAME integration_p0_t008_face_reference_spike COMMAND integration_p0_t008_face_reference_spike)

# --- P0-T009 Dependency Graph Spike (Architecture Gate BIM-AG-P0-T009 v1.0
# section 19 / Implementation Authorization BIM-AUTH-P0-T009 v1.0 section
# 14) - the named mandatory proof corpus (A-H) plus the accompanying
# whole-graph scenario properties. Links ONLY bim::dependency_graph and
# bim::foundation, mirroring unit_dependency_graph's "link only the
# accepted module boundary" pattern - bim::dependency_graph is a pure,
# kernel-free, foundation-only module (Architecture Gate section 17), so
# unlike every geometry/model integration target above, this target needs
# no additional kernel/vendor library to reach its real (not reconstructed
# or hand-simulated) implementation under real-kernel-equivalent,
# end-to-end conditions.
add_executable(integration_p0_t009_dependency_graph_spike
    integration_p0_t009_dependency_graph_spike.cpp
)
target_link_libraries(integration_p0_t009_dependency_graph_spike
    PRIVATE
        bim::dependency_graph
        bim::foundation
        Catch2::Catch2WithMain
)
target_compile_features(integration_p0_t009_dependency_graph_spike PRIVATE cxx_std_20)
bim_apply_warnings(integration_p0_t009_dependency_graph_spike)
add_test(NAME integration_p0_t009_dependency_graph_spike COMMAND integration_p0_t009_dependency_graph_spike)

# --- P1-T002 Document Runtime & Dependency Recompute (Implementation Brief
# P1-T002-IB section 18) - the multi-wall Level scenario: initial neutral
# geometry, Level elevation recompute of every hosted wall, single-wall
# update isolation, ElementId stability, repeated mutations and
# no-partial-state-after-rejection. Links ONLY bim::document and Catch2 and
# uses only the public document API - it never links bim::dependency_graph, so
# the private runtime identity and graph are not reachable from this target.
add_executable(integration_p1_t002_document_recompute
    integration_p1_t002_document_recompute.cpp
)
target_link_libraries(integration_p1_t002_document_recompute
    PRIVATE
        bim::document
        Catch2::Catch2WithMain
)
target_compile_features(integration_p1_t002_document_recompute PRIVATE cxx_std_20)
bim_apply_warnings(integration_p1_t002_document_recompute)
add_test(NAME integration_p1_t002_document_recompute COMMAND integration_p1_t002_document_recompute)
