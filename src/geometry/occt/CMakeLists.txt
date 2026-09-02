# bim_geometry_occt - the ONLY target permitted to include OCCT headers or
# link OCCT (Architecture Gate section 8.1/8.2, AG-006). Implements
# bim::geometry_api::RunKernelSmokeProbe() by constructing one trivial OCCT
# solid and returning a project-owned neutral result. This rule is checked
# mechanically by tools/architecture_checker.cmake.

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
)
add_library(bim::geometry_occt ALIAS bim_geometry_occt)

target_link_libraries(bim_geometry_occt
    PRIVATE
        bim::geometry_api
        bim::foundation
        ${OpenCASCADE_LIBRARIES}
)

target_include_directories(bim_geometry_occt
    PRIVATE
        ${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(bim_geometry_occt PRIVATE cxx_std_20)

bim_apply_warnings(bim_geometry_occt)
bim_apply_sanitizers(bim_geometry_occt)
