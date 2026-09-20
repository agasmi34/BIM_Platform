# bim_ifc - the ONLY target permitted to include IfcOpenShell headers or
# link IfcOpenShell (ACR-P0-T005-001 section 7; Implementation Brief
# BIM-TASK-P0-T005-CLAUDE v1.0 section 7; enforced mechanically by
# tools/architecture_checker.cmake R14/IFC_OPEN_SHELL_ONLY_IFC_OWNER and
# R15/IFC_PUBLIC_NEUTRAL). Mirrors bim_persistence's public-include /
# private-detail split (src/persistence/CMakeLists.txt): the public
# bim::ifc contract lives under include/bim/ifc/**, the private IfcOpenShell
# adapter lives under src/detail/**.
#
# Unlike OCCT/SQLite/Qt/bgfx, IfcOpenShell is NOT available through the
# project's frozen vcpkg baseline (ACR-P0-T005-001 section 1: the baseline
# has no `ifcopenshell` port) - it is consumed from an exact pinned upstream
# source revision, built and installed to an external prefix outside this
# repository by scripts/dependencies/ifcopenshell/bootstrap-ifcopenshell.ps1
# (ACR-P0-T005-001 section 6). This directory is only ever added to the
# build when the root CMakeLists.txt's BIM_ENABLE_IFC option is explicitly
# turned on (default OFF) - see the root CMakeLists.txt comment on
# BIM_ENABLE_IFC for why an opt-in option is used here instead of the
# always-on add_subdirectory() every other first-party module uses.

find_package(IfcOpenShell CONFIG QUIET)

if(NOT TARGET IfcOpenShell::IfcParse)
    message(FATAL_ERROR
        "bim_ifc: BIM_ENABLE_IFC is ON but IfcOpenShell::IfcParse could not be "
        "resolved via find_package(IfcOpenShell CONFIG). Set IfcOpenShell_DIR "
        "to the external install prefix's CMake package directory produced by "
        "scripts/dependencies/ifcopenshell/bootstrap-ifcopenshell.ps1 (see "
        "that script's own README.md), for example:\n"
        "  cmake --preset <preset> -DBIM_ENABLE_IFC=ON "
        "-DIfcOpenShell_DIR=<install-prefix>/lib/cmake/IfcOpenShell\n"
        "This is a dependency-bootstrap failure, not an architecture defect - "
        "do not work around it by relaxing this REQUIRED resolution "
        "(Implementation Brief section 14).")
endif()

add_library(bim_ifc STATIC
    src/ifc_probe.cpp
    src/detail/scalar_property_codec.cpp
    src/detail/openshell_adapter.cpp
)
add_library(bim::ifc ALIAS bim_ifc)

target_include_directories(bim_ifc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # Lets src/detail/*.cpp use the project-relative quoted-include
        # style already established by src/persistence/src/detail/**
        # ("detail/schema_v1.hpp"-style), e.g. "detail/openshell_adapter.hpp".
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(bim_ifc
    PUBLIC
        bim::foundation
    PRIVATE
        IfcOpenShell::IfcParse
)

target_compile_features(bim_ifc PUBLIC cxx_std_20)

bim_apply_warnings(bim_ifc)
bim_apply_sanitizers(bim_ifc)
