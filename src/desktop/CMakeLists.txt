# bim_desktop_spike - the ONLY target permitted to include Qt headers or
# link Qt (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, section 11;
# mirrors the bim_geometry_occt / bim_viewport_bgfx "sole owner" pattern).
# Widgets only (Phase A fact: qtbase 6.11.1#1 Widgets-only, Qt Quick
# forbidden) - never QtQuick/QtQml/QtQuickWidgets.
#
# This rule is checked mechanically by tools/architecture_checker.cmake
# rule R9/QT_DESKTOP_ONLY, and by the negative fixture
# tests/fixtures/p0_t003_bad_qt_owner (arch_p0_t003_qt_fixture_rejected).
#
# RISK / UNVERIFIED (disclosed per docs/evidence/P0-T003/CLAUDE_HANDOVER.md):
# the exact CMake package/component names exported by vcpkg's qtbase port
# (Qt6::Widgets via find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets),
# as assumed below, is the standard upstream Qt6 CMake shape and should
# match, but was not independently confirmed against the actual installed
# vcpkg qtbase config files in this session - no execution channel).

find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets)

add_executable(bim_desktop_spike
    src/main.cpp
    src/main_window.cpp
    src/viewport_window.cpp
    src/viewport_bridge.cpp
    src/spike_scene.cpp
    src/evidence_mode.cpp
)

set_target_properties(bim_desktop_spike PROPERTIES
    AUTOMOC ON
)

target_link_libraries(bim_desktop_spike
    PRIVATE
        bim::viewport
        bim::viewport_bgfx
        Qt6::Widgets
)

target_compile_features(bim_desktop_spike PRIVATE cxx_std_20)

bim_apply_warnings(bim_desktop_spike)
bim_apply_sanitizers(bim_desktop_spike)

# Shader binaries compiled by src/viewport/bgfx/CMakeLists.txt
# (bim_viewport_bgfx_shaders) land under ${CMAKE_BINARY_DIR}/shaders;
# renderer.cpp's LoadCompiledShader resolves them relative to the running
# executable's working directory, so they are copied next to
# bim_desktop_spike's output on every build.
add_dependencies(bim_desktop_spike bim_viewport_bgfx_shaders)
add_custom_command(TARGET bim_desktop_spike POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:bim_desktop_spike>/shaders"
    COMMENT "bim_desktop_spike: staging compiled shader binaries next to the executable"
    VERBATIM
)

# Qt runtime deployment (windeployqt or equivalent, to place Qt6Widgets.dll
# etc. next to the executable) is intentionally NOT wired here - the
# Implementation Brief's build-environment section describes a
# VISUAL_STUDIO_BUNDLED_VCPKG mode where vcpkg's dynamic-triplet Qt DLLs
# are already discoverable via the build's PATH/output layout. If the
# Operator's actual build cannot locate the Qt DLLs at run time, that is a
# real gap to close here (e.g. a windeployqt POST_BUILD step), not
# something this authoring pass could verify without a live build.
