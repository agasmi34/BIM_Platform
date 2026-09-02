# bim_geometry_api - project-owned, OCCT-free public surface. INTERFACE
# library: it declares the Phase 0 kernel smoke-probe contract only; the
# implementation lives exclusively in bim_geometry_occt (Architecture Gate
# section 7 table: "INTERFACE/STATIC as required by first real types").

add_library(bim_geometry_api INTERFACE)
add_library(bim::geometry_api ALIAS bim_geometry_api)

target_include_directories(bim_geometry_api
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(bim_geometry_api INTERFACE bim::foundation)
target_compile_features(bim_geometry_api INTERFACE cxx_std_20)

# No bim_apply_warnings()/bim_apply_sanitizers(): an INTERFACE library has no
# compiled sources of its own to apply target_compile_options() to.
