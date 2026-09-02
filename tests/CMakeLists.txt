# Test composition. Individual Catch2 executables live under tests/unit and
# tests/integration; the architecture checker is invoked directly via CTest
# under tests/architecture (no C++ compilation is needed for those two
# tests - Implementation Brief Phase J: "Prefer a CMake script (cmake -P)...
# Do not add Python only for this checker in P0-T001.").
#
# Tests are registered with plain add_test() against each Catch2 executable
# target rather than catch_discover_tests()/include(Catch), to avoid a
# dependency on locating Catch2's bundled CMake module file inside the vcpkg
# install tree - a real, unverified detail this scaffold intentionally does
# not gamble on. This still fully satisfies "registered in CTest and
# runnable by the CI preset" (Architecture Gate section 12.1).

find_package(Catch2 3 CONFIG REQUIRED)

add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(architecture)
