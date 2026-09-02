cmake_minimum_required(VERSION 3.21)

# ==============================================================================
# BIM Platform - Root CMake project policy
#
# This file owns project policy, global options, and add_subdirectory()
# composition ONLY. It must not contain module implementation logic
# (Implementation Brief P0-T001, Phase E).
#
# CI reference toolchain version is CMake 4.4.2 (Architecture Gate BIM-AG-P0-T001
# section 4.1). The floor above (3.21) is kept low enough to not needlessly
# exclude earlier maintained CMake installations while presets still target the
# CI reference version explicitly (see CMakePresets.json).
# ==============================================================================

project(BIMPlatform LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Out-of-source build guard
# ------------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are prohibited. Configure into an out-of-source "
        "build directory, e.g.:\n"
        "  cmake --preset win-msvc-debug\n"
        "or manually:\n"
        "  cmake -S . -B build -G Ninja")
endif()

# ------------------------------------------------------------------------------
# Language baseline: C++20 required, no compiler extensions.
# C++23 is prohibited in P0-T001 without an ADR (Implementation Brief section 3).
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Testing
# ------------------------------------------------------------------------------
include(CTest)
enable_testing()

# ------------------------------------------------------------------------------
# Central options/warnings/sanitizer modules
# ------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(BimOptions)
include(BimCompilerWarnings)
include(BimSanitizers)

# ------------------------------------------------------------------------------
# First-party module composition, in dependency-safe (topological) order.
#
# Allowed dependency direction (Architecture Gate section 8.1 / Implementation
# Brief section 7):
#
#   foundation
#     ^
#     |-- geometry_api
#     |-- dependency_graph
#     `-- model
#           ^
#           |-- transactions
#           |      ^
#           |      `-- commands
#           `-- query
#
#   geometry_occt --> geometry_api + foundation + OCCT
#   persistence   --> model + transactions + foundation + SQLite
#
# Future modules (interop/ifc, interop/dwg, interop/rvt, documentation,
# viewport, desktop) are NOT added here in P0-T001. They contain only a
# boundary README.md (Implementation Brief Phase G).
# ------------------------------------------------------------------------------
add_subdirectory(src/foundation)
add_subdirectory(src/geometry/api)
add_subdirectory(src/dependency_graph)
add_subdirectory(src/model)
add_subdirectory(src/transactions)
add_subdirectory(src/query)
add_subdirectory(src/commands)
add_subdirectory(src/geometry/occt)
add_subdirectory(src/persistence)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
