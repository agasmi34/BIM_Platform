# Controlled NEGATIVE fixture for R24 (VIEWPORT_UPSTREAM_ISOLATION). This
# simulated viewport module links bim::presentation, an upward dependency the
# viewport must never have. arch_p1_t005_viewport_upstream_fixture_rejected
# expects the checker to FAIL here. It is never added to the build.
add_library(bim_viewport STATIC src/mesh.cpp)
target_link_libraries(bim_viewport PUBLIC bim::presentation)
