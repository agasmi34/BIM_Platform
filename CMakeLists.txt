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
# BIM_ENABLE_IFC (P0-T005 - IFC Spike; Implementation Brief
# BIM-TASK-P0-T005-CLAUDE v1.0 section 14; ACR-P0-T005-001) - default OFF,
# unlike every other first-party module's always-on add_subdirectory() below.
# IfcOpenShell is NOT available through the project's frozen vcpkg baseline
# (ACR-P0-T005-001 section 1: the baseline has no `ifcopenshell` port); it
# must be built and installed to an external prefix first via
# scripts/dependencies/ifcopenshell/bootstrap-ifcopenshell.ps1, then supplied
# via -DIfcOpenShell_DIR=<prefix>/lib/cmake/IfcOpenShell. Gating
# src/interop/ifc behind this option keeps the default configure/build
# (every engineer/CI job that has not bootstrapped IfcOpenShell) completely
# unaffected - mirroring this task's own footprint discipline (Brief section
# 17: minimum delta only) rather than making every existing developer's
# build newly depend on a dependency most of them have not opted into.
# ------------------------------------------------------------------------------
option(BIM_ENABLE_IFC
    "Build the P0-T005 IFC interoperability spike (src/interop/ifc); requires a pre-built external IfcOpenShell::IfcParse package - see scripts/dependencies/ifcopenshell/README.md"
    OFF)

# ------------------------------------------------------------------------------
# BIM_ENABLE_DWG (P0-T006 - DWG / ODA Evaluation; Execution Packet
# BIM-AA-P0-T006 v1.0 sections 3, 14) - default OFF, mirroring BIM_ENABLE_IFC
# immediately above for the same reason: the locked ODA Drawings 27.7.0.0
# vc16_amd64dll trial SDK is an operator-supplied external tree, not
# available through the project's frozen vcpkg baseline and not installed
# by default. Unlike IfcOpenShell, this SDK ships no CMake package/config of
# any kind (Packet section 3), so there is no find_package()/*_DIR variable
# to point at an installed prefix - the caller instead supplies
# BIM_ODA_DRAWINGS_ROOT directly as the SDK tree's root directory (see
# src/interop/dwg/CMakeLists.txt for the exact required layout). Gating
# src/interop/dwg behind this option keeps the default configure/build
# (every engineer/CI job that has not been granted this locked trial SDK)
# completely unaffected, exactly as BIM_ENABLE_IFC does for IfcOpenShell.
# ------------------------------------------------------------------------------
option(BIM_ENABLE_DWG
    "Build the P0-T006 DWG / ODA evaluation spike (src/interop/dwg); requires BIM_ODA_DRAWINGS_ROOT pointing at a locked ODA Drawings 27.7.0.0 vc16_amd64dll SDK tree - see src/interop/dwg/CMakeLists.txt"
    OFF)

# ------------------------------------------------------------------------------
# First-party module composition, in dependency-safe (topological) order.
#
# Allowed dependency direction (Architecture Gate section 8.1 / Implementation
# Brief section 7; extended P0-T003 - Implementation Brief
# BIM-TASK-P0-T003-CLAUDE v1.0 sections 4, 9, 11):
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
#   viewport        --> (nothing first-party, not even foundation) + stdlib only
#   viewport_bgfx   --> viewport + bgfx
#   desktop (bim_desktop_spike) --> viewport + viewport_bgfx + Qt6::Widgets
#   interop/ifc (bim_ifc) --> foundation + IfcOpenShell::IfcParse (external
#                              prefix, NOT vcpkg - see BIM_ENABLE_IFC above)
#   interop/dwg (bim_dwg) --> foundation + locked ODA Drawings SDK (raw
#                              operator-supplied tree via
#                              BIM_ODA_DRAWINGS_ROOT, NOT vcpkg, NOT a CMake
#                              package - see BIM_ENABLE_DWG above)
#
# viewport/viewport_bgfx/desktop are added as of P0-T003 - Desktop +
# Viewport Spike (previously boundary-README-only placeholder directories
# through P0-T001/P0-T002, per the prior revision of this comment block).
# src/interop/ifc is added as of P0-T005 - IFC Spike, but ONLY when
# BIM_ENABLE_IFC is ON (see that option's own comment above) - unlike every
# other module here, it is not unconditionally added, since its third-party
# dependency is not available by default. src/interop/dwg is added as of
# P0-T006 - DWG / ODA Evaluation, but ONLY when BIM_ENABLE_DWG is ON (see
# that option's own comment above), for the same reason. Remaining future
# modules (interop/rvt, documentation) are still NOT added here - they still
# contain only a boundary README.md (Implementation Brief Phase G).
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
add_subdirectory(src/viewport)
add_subdirectory(src/viewport/bgfx)
add_subdirectory(src/desktop)

if(BIM_ENABLE_IFC)
    add_subdirectory(src/interop/ifc)
endif()

if(BIM_ENABLE_DWG)
    add_subdirectory(src/interop/dwg)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
