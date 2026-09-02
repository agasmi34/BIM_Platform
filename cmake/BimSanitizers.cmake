# ==============================================================================
# BimSanitizers.cmake
#
# No-op unless BIM_ENABLE_SANITIZERS is explicitly turned ON. P0-T001 does not
# require sanitizer instrumentation; this module exists so later tasks have a
# single, centralized place to enable it rather than copy-pasted target flags.
# ==============================================================================

function(bim_apply_sanitizers target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "bim_apply_sanitizers: '${target_name}' is not a target.")
    endif()

    if(NOT BIM_ENABLE_SANITIZERS)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /fsanitize=address)
    else()
        target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()
