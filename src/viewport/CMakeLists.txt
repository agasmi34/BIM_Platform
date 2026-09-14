# bim_viewport - the renderer-neutral viewport contract (Implementation
# Brief BIM-TASK-P0-T003-CLAUDE v1.0, sections 4-10; Implementation
# Authorization section 4). Depends on nothing first-party and nothing
# third-party: no Qt, no bgfx, no D3D/Windows native headers, no OCCT, and
# no BIM semantic/model/query types (not even bim::foundation - this
# target's public headers must remain includable by a consumer that links
# none of the rest of the BIM Platform). Standard library only.
#
# This rule is checked mechanically by tools/architecture_checker.cmake
# rule R8/VIEWPORT_PUBLIC_NEUTRAL, and by the negative fixture
# tests/fixtures/p0_t003_bad_viewport_api (arch_p0_t003_viewport_fixture_rejected).
#
# bim::viewport_bgfx (src/viewport/bgfx/CMakeLists.txt) is the only target
# permitted to depend on this one and on bgfx together.

add_library(bim_viewport STATIC
    src/mesh.cpp
    src/camera.cpp
    src/ray.cpp
    src/lifecycle.cpp
)
add_library(bim::viewport ALIAS bim_viewport)

target_include_directories(bim_viewport
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(bim_viewport PRIVATE cxx_std_20)

bim_apply_warnings(bim_viewport)
bim_apply_sanitizers(bim_viewport)
