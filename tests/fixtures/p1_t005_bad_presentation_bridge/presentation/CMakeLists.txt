# Controlled NEGATIVE fixture for R23 (PRESENTATION_BRIDGE_BOUNDARY). This
# simulated presentation module links bim::document, which the bridge must
# never depend on. arch_p1_t005_presentation_bridge_fixture_rejected expects the
# checker to FAIL here. It is never added to the build.
add_library(bim_presentation STATIC src/wall_mesh.cpp)
target_link_libraries(bim_presentation PUBLIC bim::geometry_api bim::viewport bim::document)
