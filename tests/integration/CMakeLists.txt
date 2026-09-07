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
