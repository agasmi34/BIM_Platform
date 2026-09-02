# ==============================================================================
# tools/architecture_checker.cmake
#
# Dependency-free (no Python) mechanical architecture-boundary checker for
# BIM Platform, run via `cmake -P` (Implementation Brief Phase J: "Prefer a
# CMake script (cmake -P) or another dependency-free repository mechanism.
# Do not add Python only for this checker in P0-T001.").
#
# Usage:
#   cmake -DBIM_ARCH_CHECK_ROOT=<path> -P tools/architecture_checker.cmake
#
# BIM_ARCH_CHECK_ROOT is scanned as if it were the repository's src/
# directory: this script looks for <root>/<module>/... below it. This lets
# the SAME script be run both against the real src/ tree
# (CTest test: arch_repository_boundaries, must PASS) and against the
# controlled bad fixture under tests/fixtures/bad_architecture/
# (CTest test: arch_checker_detects_violation, the checker must FAIL here -
# see tests/architecture/CMakeLists.txt, which sets WILL_FAIL TRUE on that
# test so CTest reports it as passing only when this script correctly
# rejects the fixture).
#
# Rules enforced (Implementation Brief Phase J minimum list):
#   R1 - no OCCT/Qt/SQLite/ODA/IfcOpenShell tokens in <root>/model/include/**
#   R2 - foundation (<root>/foundation/include/** and <root>/foundation/src/**)
#        references no project module and no third-party dependency
#   R3 - no "sqlite3" token anywhere under <root>/commands/**
#   R4 - no raw OCCT tokens in <root>/viewport/** or <root>/desktop/**
#        headers, if any exist yet
#   R5 - <root>/foundation/CMakeLists.txt (if present) contains no
#        non-comment target_link_libraries() line
#
# On the first pass through all rules, every violation found is collected;
# if any exist, this script calls message(FATAL_ERROR ...), which makes
# `cmake -P` exit non-zero. On a clean pass it prints
# "ARCHITECTURE CHECK PASSED" and exits 0.
# ==============================================================================

if(NOT DEFINED BIM_ARCH_CHECK_ROOT)
    message(FATAL_ERROR "architecture_checker.cmake: BIM_ARCH_CHECK_ROOT was not set. Usage: cmake -DBIM_ARCH_CHECK_ROOT=<path> -P tools/architecture_checker.cmake")
endif()

get_filename_component(BIM_ARCH_CHECK_ROOT "${BIM_ARCH_CHECK_ROOT}" ABSOLUTE)

if(NOT EXISTS "${BIM_ARCH_CHECK_ROOT}")
    message(FATAL_ERROR "architecture_checker.cmake: root '${BIM_ARCH_CHECK_ROOT}' does not exist.")
endif()

set(BIM_VIOLATIONS "")

# ------------------------------------------------------------------------------
# Helper: scan a list of files for a list of forbidden (case-insensitive)
# substrings; append a human-readable entry to BIM_VIOLATIONS for every hit.
# Whole-file substring search (including comments) is intentional: a
# forbidden third-party type mentioned even in a comment inside a real
# public header is still a signal worth a human look, and this checker is a
# blunt, auditable, dependency-free mechanism, not a full parser.
# ------------------------------------------------------------------------------
function(bim_scan_files_for_tokens files tokens rule_id)
    foreach(f IN LISTS files)
        if(NOT EXISTS "${f}")
            continue()
        endif()
        file(READ "${f}" _content)
        string(TOLOWER "${_content}" _content_lower)
        foreach(tok IN LISTS tokens)
            string(TOLOWER "${tok}" _tok_lower)
            string(FIND "${_content_lower}" "${_tok_lower}" _idx)
            if(NOT _idx EQUAL -1)
                list(APPEND BIM_VIOLATIONS "[${rule_id}] token '${tok}' found in ${f}")
            endif()
        endforeach()
    endforeach()
    set(BIM_VIOLATIONS "${BIM_VIOLATIONS}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# R1 - model public headers must not leak third-party types
# ------------------------------------------------------------------------------
file(GLOB_RECURSE _model_public_headers
    "${BIM_ARCH_CHECK_ROOT}/model/include/*.h"
    "${BIM_ARCH_CHECK_ROOT}/model/include/*.hpp"
    "${BIM_ARCH_CHECK_ROOT}/model/include/*.hh"
    "${BIM_ARCH_CHECK_ROOT}/model/include/*.hxx"
)
set(_r1_tokens
    "TopoDS_" "gp_Pnt" "gp_Trsf" "gp_Ax" "BRep" "Handle("
    "<Qt" "QObject" "sqlite3" "IfcOpenShell" "OdDb" "OdString" "opencascade"
)
bim_scan_files_for_tokens("${_model_public_headers}" "${_r1_tokens}" "R1-model-public-header-leak")

# ------------------------------------------------------------------------------
# R2 - foundation has zero project/third-party dependency
# ------------------------------------------------------------------------------
file(GLOB_RECURSE _foundation_sources
    "${BIM_ARCH_CHECK_ROOT}/foundation/include/*.h"
    "${BIM_ARCH_CHECK_ROOT}/foundation/include/*.hpp"
    "${BIM_ARCH_CHECK_ROOT}/foundation/src/*.h"
    "${BIM_ARCH_CHECK_ROOT}/foundation/src/*.hpp"
    "${BIM_ARCH_CHECK_ROOT}/foundation/src/*.cpp"
)
set(_r2_tokens
    "bim/model" "bim/geometry" "bim/persistence" "bim/transactions"
    "bim/query" "bim/commands" "bim/dependency_graph"
    "Qt" "TopoDS" "gp_Pnt" "sqlite3" "IfcOpenShell" "OdDb" "spdlog" "fmt/"
)
bim_scan_files_for_tokens("${_foundation_sources}" "${_r2_tokens}" "R2-foundation-dependency")

# ------------------------------------------------------------------------------
# R3 - no direct SQLite in commands
# ------------------------------------------------------------------------------
file(GLOB_RECURSE _commands_sources
    "${BIM_ARCH_CHECK_ROOT}/commands/*.h"
    "${BIM_ARCH_CHECK_ROOT}/commands/*.hpp"
    "${BIM_ARCH_CHECK_ROOT}/commands/*.cpp"
    "${BIM_ARCH_CHECK_ROOT}/commands/CMakeLists.txt"
)
bim_scan_files_for_tokens("${_commands_sources}" "sqlite3" "R3-commands-direct-sqlite")

# ------------------------------------------------------------------------------
# R4 - no raw OCCT in viewport/desktop public headers (future modules; empty
# file lists in P0-T001 since only README.md exists there today)
# ------------------------------------------------------------------------------
file(GLOB_RECURSE _future_public_headers
    "${BIM_ARCH_CHECK_ROOT}/viewport/*.h"
    "${BIM_ARCH_CHECK_ROOT}/viewport/*.hpp"
    "${BIM_ARCH_CHECK_ROOT}/desktop/*.h"
    "${BIM_ARCH_CHECK_ROOT}/desktop/*.hpp"
)
set(_r4_tokens "TopoDS_" "BRep" "gp_Pnt" "opencascade")
bim_scan_files_for_tokens("${_future_public_headers}" "${_r4_tokens}" "R4-future-module-occt-leak")

# ------------------------------------------------------------------------------
# R5 - foundation's own CMakeLists.txt links against nothing (non-comment
# lines only, so a comment merely describing this rule cannot self-trigger it)
# ------------------------------------------------------------------------------
set(_foundation_cmake "${BIM_ARCH_CHECK_ROOT}/foundation/CMakeLists.txt")
if(EXISTS "${_foundation_cmake}")
    file(STRINGS "${_foundation_cmake}" _foundation_cmake_all_lines)
    foreach(_line IN LISTS _foundation_cmake_all_lines)
        string(STRIP "${_line}" _stripped)
        if(_stripped MATCHES "^#")
            continue()
        endif()
        if(_stripped MATCHES "target_link_libraries")
            list(APPEND BIM_VIOLATIONS
                "[R5-foundation-cmake-link] non-comment target_link_libraries() line in foundation/CMakeLists.txt: ${_stripped}")
        endif()
    endforeach()
endif()

# ------------------------------------------------------------------------------
# Verdict
# ------------------------------------------------------------------------------
list(LENGTH BIM_VIOLATIONS _violation_count)
if(_violation_count GREATER 0)
    message("architecture_checker.cmake: ${_violation_count} violation(s) found under '${BIM_ARCH_CHECK_ROOT}':")
    foreach(v IN LISTS BIM_VIOLATIONS)
        message("  - ${v}")
    endforeach()
    message(FATAL_ERROR "ARCHITECTURE CHECK FAILED")
endif()

message("architecture_checker.cmake: scanned root '${BIM_ARCH_CHECK_ROOT}', 0 violations.")
message("ARCHITECTURE CHECK PASSED")
