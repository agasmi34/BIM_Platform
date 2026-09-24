# bim_dwg - the ONLY target permitted to include an ODA Drawings header or
# link an ODA Drawings library (Execution Packet BIM-AA-P0-T006 v1.0
# sections 3, 9; enforced mechanically by tools/architecture_checker.cmake
# R16/ODA_DRAWINGS_ONLY_DWG_OWNER and R17/DWG_PUBLIC_NEUTRAL). Mirrors
# src/interop/ifc/CMakeLists.txt's public-include / private-detail split:
# the public bim::dwg contract lives under include/bim/dwg/**, the private
# ODA adapter lives under src/detail/**.
#
# Unlike IfcOpenShell, the locked ODA Drawings 27.7.0.0 vc16_amd64dll trial
# toolkit ships NO CMake package/config of any kind (Packet section 3) - it
# is a raw, operator-supplied SDK tree (BIM_ODA_DRAWINGS_ROOT), so this file
# cannot mirror src/interop/ifc/CMakeLists.txt's find_package() call. Every
# include directory, library directory, and library name below is instead
# grounded directly in two real files read from that locked SDK tree under
# the operator-approved, read-only-granted root (see
# src/interop/dwg/src/detail/oda_adapter.hpp's own grounding-disclosure
# comment for the full list of headers checked the same way):
#   - Drawing/Examples/SimpleReadWriteEx/SimpleReadWrite.cpp, the vendor's
#     own minimal reference program for this exact toolkit/platform pairing;
#   - that same example's own generated build file,
#     Platforms/vc16_amd64dll/Drawing/Examples/SimpleReadWriteEx/SimpleReadWrite.vcxproj,
#     which is the one artifact in this locked SDK tree that has actually
#     been built and linked successfully against vc16_amd64dll - every
#     AdditionalIncludeDirectories / AdditionalLibraryDirectories /
#     AdditionalDependencies entry below is copied from it unchanged (paths
#     translated from vcxproj-relative to BIM_ODA_DRAWINGS_ROOT-relative;
#     see the path-translation note above target_include_directories()).
#
# This directory is only ever added to the build when the root
# CMakeLists.txt's BIM_ENABLE_DWG option is explicitly turned on (default
# OFF) - see that file's own comment on BIM_ENABLE_DWG.

if(NOT DEFINED BIM_ODA_DRAWINGS_ROOT OR BIM_ODA_DRAWINGS_ROOT STREQUAL "")
    message(FATAL_ERROR
        "bim_dwg: BIM_ENABLE_DWG is ON but BIM_ODA_DRAWINGS_ROOT is not set. "
        "Set it to the root of the locked ODA Drawings 27.7.0.0 "
        "vc16_amd64dll trial SDK tree (the directory that directly contains "
        "Drawing/, Kernel/, KernelBase/, ThirdParty/, Platforms/, exe/, and "
        "lib/), for example:\n"
        "  cmake --preset <preset> -DBIM_ENABLE_DWG=ON "
        "-DBIM_ODA_DRAWINGS_ROOT=<path-to-ODA-toolkit>\n"
        "This is a dependency-bootstrap failure, not an architecture defect "
        "(Execution Packet BIM-AA-P0-T006 v1.0 section 3).")
endif()

if(NOT EXISTS "${BIM_ODA_DRAWINGS_ROOT}/Drawing/Include/DbDatabase.h")
    message(FATAL_ERROR
        "bim_dwg: BIM_ODA_DRAWINGS_ROOT ('${BIM_ODA_DRAWINGS_ROOT}') does "
        "not look like the ODA Drawings SDK root - "
        "Drawing/Include/DbDatabase.h was not found underneath it. Check "
        "the path.")
endif()

add_library(bim_dwg STATIC
    src/dwg_probe.cpp
    src/detail/oda_adapter.cpp
)
add_library(bim::dwg ALIAS bim_dwg)

# Include-directory closure copied unchanged (path-translated) from
# SimpleReadWrite.vcxproj's AdditionalIncludeDirectories, minus the one
# entry that pointed at Drawing/Examples/SimpleReadWriteEx/../Common: this
# adapter defines its own minimal OdaServices class directly
# (src/detail/oda_adapter.cpp) rather than including the vendor examples'
# own MyServices/Common helper headers, so that directory is never
# `#include`d by any file in this target.
target_include_directories(bim_dwg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # Lets src/detail/*.cpp use the project-relative quoted-include
        # style already established by src/interop/ifc/src/detail/**, e.g.
        # "detail/oda_adapter.hpp".
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        "${BIM_ODA_DRAWINGS_ROOT}/KernelBase/Include"
        "${BIM_ODA_DRAWINGS_ROOT}/ThirdParty"
        "${BIM_ODA_DRAWINGS_ROOT}/ThirdParty/activation"
        "${BIM_ODA_DRAWINGS_ROOT}/Drawing/Extensions/ExServices"
        "${BIM_ODA_DRAWINGS_ROOT}/Kernel/Extensions/ExServices"
        "${BIM_ODA_DRAWINGS_ROOT}/Kernel/Include"
        "${BIM_ODA_DRAWINGS_ROOT}/Drawing/Include"
        "${BIM_ODA_DRAWINGS_ROOT}/KernelBase"
        "${BIM_ODA_DRAWINGS_ROOT}/KernelBase/Source"
)

# Library-directory closure copied unchanged (path-translated) from
# SimpleReadWrite.vcxproj's AdditionalLibraryDirectories. Named once here
# and reused below - never duplicated.
set(_bim_oda_exe_dir "${BIM_ODA_DRAWINGS_ROOT}/exe/vc16_amd64dll")
set(_bim_oda_lib_dir "${BIM_ODA_DRAWINGS_ROOT}/lib/vc16_amd64dll")

# [CP3-H2-L1 minimum-delta correction] bim_dwg is a STATIC library: its own
# archive step performs no linking, so a PRIVATE target_link_directories()
# never takes effect for bim_dwg itself, and - unlike PRIVATE
# target_link_libraries() on a static library, whose library NAMES are
# still propagated transitively to a final linked consumer - PRIVATE link
# *directories* are not propagated to consumers at all. The result was
# LNK1104: cannot open file 'TD_ExamplesCommon.lib' in every executable
# that links bim::dwg, because the already-transitive ODA library names
# reached the consumer's link line with no matching /LIBPATH entries.
# Declared INTERFACE instead so the two locked SDK directories become a
# link-only usage requirement of bim_dwg's consumers: this is a
# link-search-path requirement, not public ODA API exposure, so it does not
# weaken R16/R17.
target_link_directories(bim_dwg
    INTERFACE
        "${_bim_oda_exe_dir}"
        "${_bim_oda_lib_dir}"
)

# Link-library closure copied unchanged from SimpleReadWrite.vcxproj's
# AdditionalDependencies, including TD_ExamplesCommon.lib and
# TD_DrawingsExamplesCommon.lib even though src/detail/oda_adapter.cpp
# never includes either library's own headers: this vcxproj is the only
# artifact in the locked SDK tree with independent evidence of having
# actually linked against vc16_amd64dll, and pruning either entry as
# "probably unneeded" would be an unverified guess about this trial SDK's
# internal static-module link requirements that this session has no
# execution channel to check. If the Windows Execution Operator's first
# real build shows either library is not actually needed, that is a
# minimum-delta CMake fix to make then, grounded in a real link result
# rather than a guess made here.
target_link_libraries(bim_dwg
    PUBLIC
        bim::foundation
    PRIVATE
        TD_ExamplesCommon.lib
        TD_Key.lib
        TD_Db.lib
        TD_Gs.lib
        TD_Gi.lib
        TD_Root.lib
        TD_Ge.lib
        TD_DbRoot.lib
        RxRasterServices.lib
        TD_DrawingsExamplesCommon.lib
        TD_Alloc.lib
        RText.lib
        TD_DbEntities.lib
        TD_DbIO.lib
        TD_DbCore.lib
        ATEXT.lib
        WipeOut.lib
        ISM.lib
        AcMPolygonObj15.lib
        ACCAMERA.lib
        SCENEOE.lib
        UTF.lib
        Secur32.lib
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        uuid.lib
        comdlg32.lib
        advapi32.lib
)

# UNICODE/_UNICODE copied unchanged from SimpleReadWrite.vcxproj's own
# PreprocessorDefinitions - this locked ODA toolkit's own OdString headers
# are built expecting this pairing, so it is treated as a build-correctness
# necessity of consuming this exact SDK rather than a project-wide
# configuration change (it is PRIVATE to bim_dwg only, never propagated to
# any other target). This is distinct from, and in addition to,
# TEIGHA_TRIAL below.
#
# TEIGHA_TRIAL: AA-authorized (P0-T006 clarification response) as a PRIVATE
# compile definition of this target only, carried because
# SimpleReadWrite.vcxproj's own verified Release configuration for this
# exact locked trial SDK defines it. Private to bim_dwg; not exposed in any
# public header (include/bim/dwg/** never references it); not a
# platform-wide/global compile definition; introduces no new dependency, no
# activation material, and no commercial-redistribution claim; does not
# change the frozen ODA identity. Its actual use is disclosed again in this
# task's Section 18 handback per AA's explicit requirement. This session has
# no execution channel to confirm whether the locked trial SDK's headers
# actually fail to compile without it (AA: "If testing proves the define is
# unnecessary, omit it rather than carrying redundant vendor configuration")
# - that determination is left to the Windows Execution Operator's first
# real build, at which point omitting it (if proven unnecessary) is a
# minimum-delta follow-up, not a re-authorization.
#
# [CP3-H3-R1-L2 minimum-delta correction] _TOOLKIT_IN_DLL_: added on real
# link evidence, not proactively. The real MSVC build failed with
# LNK2001 unresolved externals OdString::kEmptyData and
# g_odDbRegistredDwgClasses, then LNK1120: 2 unresolved externals - accepted
# P0-T006 B6 evidence establishes the successful ODA vc16 DLL-consumer
# compile contract requires _TOOLKIT_IN_DLL_ together with TEIGHA_TRIAL, and
# the prior bim_dwg compile command carried TEIGHA_TRIAL but omitted
# _TOOLKIT_IN_DLL_. PRIVATE to bim_dwg only (same as TEIGHA_TRIAL above);
# not PUBLIC/INTERFACE; not global via add_compile_definitions(); not
# exposed through any public header. No other vendor-sample define
# (WIN64, _WIN64, _WINDOWS, PDFIUM_MODULE_ENABLED, WINDIRECTX_DISABLED, the
# /Zc:wchar_t- flag, or any further library) is added here - this corrects
# only this one evidence-backed discrepancy.
target_compile_definitions(bim_dwg
    PRIVATE
        UNICODE
        _UNICODE
        TEIGHA_TRIAL
        _TOOLKIT_IN_DLL_
)

target_compile_features(bim_dwg PUBLIC cxx_std_20)

bim_apply_warnings(bim_dwg)
bim_apply_sanitizers(bim_dwg)

# ------------------------------------------------------------------------------
# [CP3-H4 minimum-delta correction] Controlled Phase-0 sufficient ODA runtime
# module set (AA CP3-H4 authorization sections 2-5). The Windows runtime
# failure was classified as an ODA PE-module runtime-staging closure defect,
# not an architecture or dependency-contract defect: bim_dwg is a STATIC
# library, so the Windows loader must resolve these modules from each
# consumer EXECUTABLE's own runtime directory, not from beside the static
# archive (section 9). This block only resolves and validates the exact 21
# AA-authorized filenames against the locked external ODA runtime directory
# and publishes their resolved paths via a GLOBAL PROPERTY - it stages
# nothing itself. Actual POST_BUILD staging onto the six controlled DWG
# executables is wired in tests/unit/CMakeLists.txt and
# tests/integration/CMakeLists.txt (the only other two files this
# correction is authorized to touch - section 6), which read this property
# back rather than re-deriving the list, so there is exactly one place that
# names the 21 filenames. No global search, PATH change, or repository-wide
# runtime-deployment policy is introduced: the source directory is exactly
# ${BIM_ODA_DRAWINGS_ROOT}/exe/vc16_amd64dll - reusing _bim_oda_exe_dir
# already defined above for the CP3-H2-L1 link-directory correction rather
# than duplicating it - and only these 21 exact filenames are ever
# resolved, never a wildcard/glob and never the whole directory (section
# 7.A/7.C/7.D).
set(_bim_p0_t006_dwg_runtime_filenames
    "ACCAMERA_27.7_16.tx"
    "AcMPolygonObj15_27.7_16.tx"
    "ATEXT_27.7_16.tx"
    "ISM_27.7_16.tx"
    "OdTrial.dll"
    "RecomputeDimBlock_27.7_16.tx"
    "RText_27.7_16.tx"
    "SCENEOE_27.7_16.tx"
    "TD_Alloc_27.7_16.dll"
    "TD_Br_27.7_16.dll"
    "TD_Db_27.7_16.dll"
    "TD_DbCore_27.7_16.dll"
    "TD_DbEntities_27.7_16.tx"
    "TD_DbIO_27.7_16.tx"
    "TD_DbRoot_27.7_16.dll"
    "TD_Ge_27.7_16.dll"
    "TD_Gi_27.7_16.dll"
    "TD_Root_27.7_16.dll"
    "TD_SpatialIndex_27.7_16.dll"
    "TD_Zlib.dll"
    "WipeOut_27.7_16.tx"
)

set(_bim_p0_t006_dwg_runtime_files "")
foreach(_bim_p0_t006_dwg_runtime_filename IN LISTS _bim_p0_t006_dwg_runtime_filenames)
    set(_bim_p0_t006_dwg_runtime_file "${_bim_oda_exe_dir}/${_bim_p0_t006_dwg_runtime_filename}")
    if(NOT EXISTS "${_bim_p0_t006_dwg_runtime_file}")
        message(FATAL_ERROR
            "bim_dwg: required controlled-runtime-set module "
            "'${_bim_p0_t006_dwg_runtime_filename}' was not found at "
            "'${_bim_p0_t006_dwg_runtime_file}'. AA CP3-H4 authorizes exactly "
            "21 named modules from BIM_ODA_DRAWINGS_ROOT/exe/vc16_amd64dll - "
            "this is a runtime-staging closure defect (or a mismatched "
            "BIM_ODA_DRAWINGS_ROOT), not an architecture defect. Check the "
            "path and the locked SDK tree; do not substitute a different "
            "module set without a new AA authorization.")
    endif()
    list(APPEND _bim_p0_t006_dwg_runtime_files "${_bim_p0_t006_dwg_runtime_file}")
endforeach()

set_property(GLOBAL PROPERTY BIM_P0_T006_DWG_RUNTIME_FILES "${_bim_p0_t006_dwg_runtime_files}")
