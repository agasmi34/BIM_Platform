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
