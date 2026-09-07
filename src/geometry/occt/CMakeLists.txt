# bim_geometry_occt - the ONLY target permitted to include OCCT headers or
# link OCCT (Architecture Gate section 8.1/8.2, AG-006). Implements
# bim::geometry_api::RunKernelSmokeProbe() (P0-T001) and, as of P0-T002
# Phase C-M, the full neutral geometry contract (MakeLinearExtrusion, Cut,
# Fuse, Inspect - Implementation Brief section 6.10) plus the P0-only
# operation-history diagnostic surface (bim::geometry_occt::spike_diagnostics,
# Brief section 19; Amendment 01 AA-C06). This rule is checked mechanically
# by tools/architecture_checker.cmake (R1-R5 plus, as of P0-T002,
# R6/GEOMETRY_API_NO_OCCT_LEAK and R7/GEOMETRY_OCCT_ONLY_KERNEL_OWNER).

find_package(OpenCASCADE CONFIG REQUIRED)

if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR
        "bim_geometry_occt: OpenCASCADE (OCCT) could not be resolved via "
        "find_package(OpenCASCADE CONFIG REQUIRED) even though vcpkg "
        "reported the 'opencascade' port as installed. This is a mandatory "
        "stop condition per Implementation Brief section 2.3 ('OCCT 8.0.1 "
        "cannot be resolved by the selected vcpkg baseline') - raise an ACR "
        "rather than working around this by weakening the REQUIRED find_package.")
endif()

add_library(bim_geometry_occt STATIC
    src/geometry_occt_probe.cpp
    src/geometry_occt_adapter.cpp
    src/spike_diagnostics.cpp
)
add_library(bim::geometry_occt ALIAS bim_geometry_occt)

target_link_libraries(bim_geometry_occt
    PRIVATE
        bim::geometry_api
        bim::foundation
        ${OpenCASCADE_LIBRARIES}
)

target_include_directories(bim_geometry_occt
    PUBLIC
        # bim/geometry_occt/spike_diagnostics.hpp (Brief section 19;
        # Amendment 01 AA-C06) is the only header this target exposes to
        # consumers outside itself - it is OCCT-free (Amendment 01 AA-C09
        # requires this specific header stay OCCT-free even though it
        # belongs to this adapter target) and may be consumed only by
        # P0-T002 tests/evidence tooling, never by production BIM code.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(bim_geometry_occt PRIVATE cxx_std_20)

bim_apply_warnings(bim_geometry_occt)
bim_apply_sanitizers(bim_geometry_occt)
