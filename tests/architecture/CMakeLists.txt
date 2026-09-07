# arch_repository_boundaries - runs the mechanical checker
# (tools/architecture_checker.cmake) against the real src/ tree. Must PASS
# (exit 0) for this CTest test to pass. No -DBIM_ARCH_CHECK_RULE is passed,
# so this runs every rule (R1-R7; default BIM_ARCH_CHECK_RULE=ALL), matching
# its pre-P0-T002 behavior of exercising the complete checker against the
# real tree.
add_test(
    NAME arch_repository_boundaries
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_checker_detects_violation - runs the SAME checker against a
# deliberately broken fixture (tests/fixtures/bad_architecture/). The
# checker itself must FAIL (non-zero exit) here; WILL_FAIL TRUE inverts that
# so CTest reports this test as passing only when the checker correctly
# rejects the fixture (Implementation Brief Phase J). Unmodified by P0-T002 -
# no new bad-architecture fixture was added for R6/R7 (Amendment 01 AA-C09).
add_test(
    NAME arch_checker_detects_violation
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/bad_architecture
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_checker_detects_violation PROPERTIES WILL_FAIL TRUE)

# arch_geometry_api_no_occt_leak - P0-T002 Phase C-M (Implementation Brief
# section 24; Amendment 01 AA-C09/AA-C10). Runs ONLY rule R6 (via the new
# BIM_ARCH_CHECK_RULE selector) against the real src/ tree: zero OCCT tokens
# under src/geometry/api/include/**.
add_test(
    NAME arch_geometry_api_no_occt_leak
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=GEOMETRY_API_NO_OCCT_LEAK
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_geometry_occt_only_kernel_owner - P0-T002 Phase C-M (Implementation
# Brief section 24; Amendment 01 AA-C09/AA-C10). Runs ONLY rule R7 against
# the real src/ tree: no OCCT token appears anywhere under src/ outside
# src/geometry/occt/** - bim_geometry_occt remains the sole OCCT owner.
add_test(
    NAME arch_geometry_occt_only_kernel_owner
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=GEOMETRY_OCCT_ONLY_KERNEL_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
