# bim_viewport_bgfx - the ONLY target permitted to include bgfx headers or
# link bgfx (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 9;
# mirrors the bim_geometry_occt / AG-006 "sole owner" pattern already used
# for OCCT - see src/geometry/occt/CMakeLists.txt). Depends on bim::viewport
# (the neutral contract) and bgfx; exposes exactly one public header,
# include/bim/viewport_bgfx/renderer.hpp, which itself contains zero bgfx
# types (pimpl - see renderer_impl.hpp, PRIVATE, not installed/exported).
#
# This rule is checked mechanically by tools/architecture_checker.cmake
# rule R10/BGFX_VIEWPORT_OWNER, and by the negative fixture
# tests/fixtures/p0_t003_bad_bgfx_owner (arch_p0_t003_bgfx_fixture_rejected).
#
# RISK / UNVERIFIED (disclosed per docs/evidence/P0-T003/CLAUDE_HANDOVER.md):
# the exact CMake package/target names exported by vcpkg's bgfx port
# (bgfx::bgfx / bgfx::bx / bgfx::bimg / bgfx::bimg_decode, or a differently
# -shaped bgfxConfig.cmake) and the exact shaderc executable name/location
# installed by the port's "tools" feature could not be confirmed without a
# real vcpkg install + find_package run (no execution channel in this
# session). The Operator's first configure will surface any mismatch here;
# fix the find_package/target_link_libraries/find_program lines below to
# match what vcpkg actually exports, not the CMake/module composition
# pattern, which does follow the established repository convention.

find_package(bgfx CONFIG REQUIRED)

if(NOT TARGET bgfx::bgfx)
    message(FATAL_ERROR
        "bim_viewport_bgfx: find_package(bgfx CONFIG REQUIRED) succeeded but "
        "did not define the bgfx::bgfx target. This means vcpkg's bgfx port "
        "exports a different target name than assumed when this file was "
        "authored (see the RISK/UNVERIFIED note above) - raise an ACR rather "
        "than guessing at a replacement target name.")
endif()

add_library(bim_viewport_bgfx STATIC
    src/renderer.cpp
)
add_library(bim::viewport_bgfx ALIAS bim_viewport_bgfx)

target_link_libraries(bim_viewport_bgfx
    PUBLIC
        bim::viewport
    PRIVATE
        bgfx::bgfx
)

target_include_directories(bim_viewport_bgfx
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(bim_viewport_bgfx PRIVATE cxx_std_20)

# AA Targeted Full Project Build v3 correction (RD1.3-01) - BUILD BLOCKER:
# the authoritative full build failed compiling src/renderer.cpp through
# bx/platform.h with MSVC fatal error C1189 ("When using MSVC you must set
# /Zc:preprocessor compiler option."). bx (bgfx's own base library, an
# indirect dependency this target does not itself target_link_libraries()
# on directly) checks for MSVC's conforming preprocessor at its own
# platform-detection header and refuses to compile without it - not a bug
# in bx/bgfx to route around, an MSVC compiler option this target's actual
# translation units are consumed under and were missing. Scoped to
# bim_viewport_bgfx only (not global/CMAKE_CXX_FLAGS) since it is the sole
# target that includes any bgfx/bx header (Brief section 9's "sole owner"
# rule, enforced by tools/architecture_checker.cmake rule R10) and
# therefore the only target that can ever hit this bx check.
if(MSVC)
    target_compile_options(bim_viewport_bgfx PRIVATE /Zc:preprocessor)
endif()

bim_apply_warnings(bim_viewport_bgfx)
bim_apply_sanitizers(bim_viewport_bgfx)

# ------------------------------------------------------------------------------
# Shader compilation (.sc -> .bin per backend) via bgfx's shaderc, installed
# by vcpkg's bgfx port "tools" feature (vcpkg.json declares
# bgfx[tools] - see the root vcpkg.json MODIFY in this same task footprint).
# Only the D3D11 (dx11) profile is compiled - the sole authoritative backend
# (Phase A fact). Output lands under
# ${CMAKE_BINARY_DIR}/shaders/dx11/<name>.bin, matching the runtime lookup
# path in renderer.cpp's LoadCompiledShader (relative to the running
# executable's working directory) - src/desktop/CMakeLists.txt copies this
# directory next to bim_desktop_spike.exe as a POST_BUILD step.
#
# AA Runbook D Attempt 1 correction (RD1-01) - BUILD BLOCKER: both
# shaders/vs_p0_t003.sc and shaders/fs_p0_t003.sc `#include <bgfx_shader.sh>`
# (a header installed by the same bgfx[tools] vcpkg feature, not authored by
# this task), but no shaderc invocation below previously told shaderc where
# to find it, so the authoritative first Runbook D attempt failed shaderc
# with an unresolved #include. The authoritative diagnostic located the
# installed header at
# ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/bgfx/bgfx_shader.sh
# under the fresh build's own vcpkg_installed tree - VCPKG_INSTALLED_DIR and
# VCPKG_TARGET_TRIPLET are the standard cache variables vcpkg's own
# toolchain file (vcpkg.cmake) sets once CMake is configured through it
# (Implementation Authorization's accepted C0 configuration always
# configures through that toolchain file), so deriving the include
# directory from them tracks the current vcpkg install tree/triplet
# automatically rather than hard-coding a user-profile or package-build
# scratch path that would not survive a different machine, triplet, or
# vcpkg_installed relocation.
# ------------------------------------------------------------------------------
find_program(BGFX_SHADERC shaderc REQUIRED)

if(NOT DEFINED VCPKG_INSTALLED_DIR OR NOT DEFINED VCPKG_TARGET_TRIPLET)
    message(FATAL_ERROR
        "bim_viewport_bgfx: VCPKG_INSTALLED_DIR and/or VCPKG_TARGET_TRIPLET "
        "are not defined - this target's shaders #include <bgfx_shader.sh>, "
        "which is only resolvable via the installed bgfx[tools] headers "
        "under the current vcpkg install tree, and this CMake configuration "
        "was not run through vcpkg's own toolchain file (scripts/buildsystems/"
        "vcpkg.cmake), which is what normally defines both of these "
        "variables. Raise an ACR rather than guessing at a replacement path.")
endif()

set(BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/bgfx")

if(NOT EXISTS "${BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR}/bgfx_shader.sh")
    message(FATAL_ERROR
        "bim_viewport_bgfx: expected the bgfx[tools]-installed "
        "bgfx_shader.sh at "
        "'${BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR}/bgfx_shader.sh' but it "
        "does not exist. This means either the bgfx port did not install "
        "its shader-include headers at the path this file assumes, or the "
        "vcpkg[tools] feature was not actually resolved for this build - "
        "raise an ACR rather than guessing at a replacement path (see the "
        "RD1-01 correction note above).")
endif()

set(BIM_VIEWPORT_BGFX_SHADER_OUT_DIR "${CMAKE_BINARY_DIR}/shaders/dx11")
file(MAKE_DIRECTORY "${BIM_VIEWPORT_BGFX_SHADER_OUT_DIR}")

function(bim_compile_bgfx_shader NAME TYPE)
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${NAME}.sc")
    set(varying "${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc")
    set(out "${BIM_VIEWPORT_BGFX_SHADER_OUT_DIR}/${NAME}.bin")
    add_custom_command(
        OUTPUT "${out}"
        COMMAND "${BGFX_SHADERC}"
            -f "${src}"
            -o "${out}"
            --varyingdef "${varying}"
            --type ${TYPE}
            --platform windows
            --profile s_5_0
            -i "${BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR}"
        # RD1-01: BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR/bgfx_shader.sh is a
        # real dependency of every compiled shader (both .sc files #include
        # it) - listed explicitly so a change to the installed header (e.g.
        # a different resolved bgfx port version) invalidates this custom
        # command's output the same way a change to src/varying already does.
        DEPENDS "${src}" "${varying}" "${BIM_VIEWPORT_BGFX_SHADER_INCLUDE_DIR}/bgfx_shader.sh"
        COMMENT "shaderc: compiling ${NAME}.sc (${TYPE}, dx11/s_5_0)"
        VERBATIM
    )
    list(APPEND BIM_VIEWPORT_BGFX_SHADER_OUTPUTS "${out}")
    set(BIM_VIEWPORT_BGFX_SHADER_OUTPUTS "${BIM_VIEWPORT_BGFX_SHADER_OUTPUTS}" PARENT_SCOPE)
endfunction()

bim_compile_bgfx_shader(vs_p0_t003 vertex)
bim_compile_bgfx_shader(fs_p0_t003 fragment)

add_custom_target(bim_viewport_bgfx_shaders ALL
    DEPENDS ${BIM_VIEWPORT_BGFX_SHADER_OUTPUTS}
)
add_dependencies(bim_viewport_bgfx bim_viewport_bgfx_shaders)
