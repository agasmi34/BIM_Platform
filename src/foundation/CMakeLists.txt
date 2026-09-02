# bim_foundation - no project or third-party dependencies (Architecture Gate
# section 8.2 / AG-*). Contains only a minimal, genuinely reusable
# status/result utility used by the geometry and persistence adapter probes.

add_library(bim_foundation STATIC
    src/status.cpp
)
add_library(bim::foundation ALIAS bim_foundation)

target_include_directories(bim_foundation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(bim_foundation PUBLIC cxx_std_20)

bim_apply_warnings(bim_foundation)
bim_apply_sanitizers(bim_foundation)

# Deliberately no link directive against any other target in this file:
# foundation MUST NOT depend on any project module, Qt, OCCT, SQLite, ODA,
# IfcOpenShell, or Python (Architecture Gate section 8.2).
