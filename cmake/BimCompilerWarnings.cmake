# ==============================================================================
# BimCompilerWarnings.cmake
#
# Applies the locked first-party MSVC compiler policy (Architecture Gate
# BIM-AG-P0-T001 section 4.2 / Implementation Brief Phase K):
#
#   /W4 /permissive- /Zc:__cplusplus /EHsc
#   UNICODE, _UNICODE, NOMINMAX
#
# Warnings become errors only when BIM_WARNINGS_AS_ERRORS is ON (CI presets).
# This function must be applied to first-party targets only. Third-party
# dependencies (OCCT, SQLite, Catch2, fmt, spdlog) are consumed via their own
# imported targets and are never routed through this function, so third-party
# header warnings cannot fail first-party CI.
# ==============================================================================

function(bim_apply_warnings target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "bim_apply_warnings: '${target_name}' is not a target.")
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /permissive-
            /Zc:__cplusplus
            /EHsc
        )
        target_compile_definitions(${target_name} PRIVATE
            UNICODE
            _UNICODE
            NOMINMAX
        )
        if(BIM_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        # P0-T001 is Windows/MSVC-first (Architecture Gate section 4). A
        # reasonable, non-MSVC-specific baseline is still applied so the
        # scaffold is not silently inert on other toolchains used for local
        # exploration; this is not a supported CI configuration in P0-T001.
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        if(BIM_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()
