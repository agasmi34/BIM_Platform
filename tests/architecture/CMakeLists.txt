# arch_repository_boundaries - runs the mechanical checker
# (tools/architecture_checker.cmake) against the real src/ tree. Must PASS
# (exit 0) for this CTest test to pass.
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
# rejects the fixture (Implementation Brief Phase J).
add_test(
    NAME arch_checker_detects_violation
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/bad_architecture
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_checker_detects_violation PROPERTIES WILL_FAIL TRUE)
