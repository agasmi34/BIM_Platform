# ==============================================================================
# tools/architecture_checker.cmake
#
# Dependency-free (no Python) mechanical architecture-boundary checker for
# BIM Platform, run via `cmake -P` (Implementation Brief Phase J: "Prefer a
# CMake script (cmake -P) or another dependency-free repository mechanism.
# Do not add Python only for this checker in P0-T001.").
#
# Usage:
#   cmake -DBIM_ARCH_CHECK_ROOT=<path> [-DBIM_ARCH_CHECK_RULE=<rule>] -P tools/architecture_checker.cmake
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
# BIM_ARCH_CHECK_RULE (added P0-T002 Phase C-M, Amendment 01 AA-C09) selects
# which rule group to run:
#   ALL                          (default) - every rule below (R1-R7)
#   GEOMETRY_API_NO_OCCT_LEAK    - only R6
#   GEOMETRY_OCCT_ONLY_KERNEL_OWNER - only R7
# This selector exists so the two new P0-T002 CTest tests
# (arch_geometry_api_no_occt_leak, arch_geometry_occt_only_kernel_owner -
# tests/architecture/CMakeLists.txt) can each exercise exactly one rule,
# while arch_repository_boundaries and arch_checker_detects_violation
# continue to omit BIM_ARCH_CHECK_RULE entirely and so continue to run every
# rule (ALL), preserving their pre-P0-T002 behavior unchanged.
#
# Rules enforced:
#   R1 - no OCCT/Qt/SQLite/ODA/IfcOpenShell tokens in <root>/model/include/**
#   R2 - foundation (<root>/foundation/include/** and <root>/foundation/src/**)
#        references no project module and no third-party dependency
#   R3 - no "sqlite3" token anywhere under <root>/commands/**
#   R4 - no raw OCCT tokens in <root>/viewport/** or <root>/desktop/**
#        headers, if any exist yet
#   R5 - <root>/foundation/CMakeLists.txt (if present) contains no
#        non-comment target_link_libraries() line
#   R6 - (P0-T002; Amendment 01 AA-C09) no OCCT token in
#        <root>/geometry/api/include/** - the public geometry_api surface
#        must stay OCCT-free (Implementation Brief section 6, "no OCCT type
#        may appear here")
#   R7 - (P0-T002; Amendment 01 AA-C09) no OCCT token anywhere under
#        BIM_ARCH_CHECK_ROOT outside <root>/geometry/occt/** -
#        bim_geometry_occt remains the ONLY OCCT owner (Architecture Gate
#        AG-006, generalized repository-wide rather than only checked
#        against bim_geometry_occt's own CMakeLists.txt link graph)
#
# ==============================================================================
# ROUND 7 REVISION NOTE - R6/R7 COMMENT-INSENSITIVE SCANNING FALSE-POSITIVE FIX
# (post-format authoritative Windows build/test attempt reached real
# configure/build/CTest for the first time; Architecture Authority read-only
# classification; not an ACR; scope limited to this file plus the two
# handover documents - no other path changed).
#
# Finding (R7-01): the authoritative CTest run showed arch_geometry_api_no_occt_leak
# and arch_geometry_occt_only_kernel_owner both FAIL against the real
# geometry/api public headers (geometry.hpp, probe.hpp), even though neither
# header actually declares, aliases, includes, forward-declares, or exposes
# any OCCT type. Inspection of every reported token occurrence found each one
# was inside a C/C++ comment - explanatory documentation such as
# "must never ... reference an OCCT type (TopoDS_*, gp_*, BRep*, Handle(...),
# ...)" - never in executable code. R6 and R7 both previously called
# bim_scan_files_for_tokens(), which is a deliberately whole-file,
# comment-INSENSITIVE substring scan (see that function's own comment below)
# - correct and intentional for R1-R5, but a false-positive generator for
# R6/R7, whose contract is about actual source/API/kernel ownership, not
# about what a comment happens to mention while explaining that contract.
#
# Correction: R6 and R7 now call a new, separate function,
# bim_scan_files_for_tokens_ignoring_comments() (below), which strips C/C++
# "//" line comments and "/* ... */" block comments (including multi-line
# block comments) from each file's content before scanning - respecting
# string ("...") and character ('...') literals (with backslash-escape
# awareness) so a "//" or "/*" inside a real literal is never mistaken for a
# comment start. Real, non-comment code before and/or after a comment on the
# same line is preserved and still scanned (e.g. `TopoDS_Shape x; // comment`
# still fails; `// TopoDS_Shape is forbidden here` and a multi-line
# `/* ... BRep ... */` block do not). R1-R5 are completely untouched - they
# still call the original bim_scan_files_for_tokens() unchanged, preserving
# their existing, deliberately comment-inclusive behavior exactly as before;
# nothing about this correction weakens or removes any protected OCCT token
# from the R6 or R7 token sets, and any actual code-level OCCT occurrence
# (outside the authorized kernel owner, or anywhere in geometry/api's public
# headers) still fails exactly as before. The pre-existing negative fixture
# (tests/fixtures/bad_architecture/, exercised by CTest arch_checker_detects_violation)
# is not modified by this correction and is not itself one of this round's
# three authorized paths; since R1-R5's scanning behavior is byte-identical
# to before, and R6/R7 still fail on any genuine code-level violation, this
# correction does not weaken that fixture's ability to trigger a detected
# violation unless its specific violation were itself comment-only text
# scoped to R6/R7 alone - a fixture built to demonstrate a real architecture
# violation would not be designed that way. This was not independently
# re-inspected this round (out of this correction's authorized scope), and
# that is disclosed rather than silently assumed.
# ==============================================================================
#
# On the first pass through the selected rule(s), every violation found is
# collected; if any exist, this script calls message(FATAL_ERROR ...), which
# makes `cmake -P` exit non-zero. On a clean pass it prints
# "ARCHITECTURE CHECK PASSED" and exits 0.
# ==============================================================================

if(NOT DEFINED BIM_ARCH_CHECK_ROOT)
    message(FATAL_ERROR "architecture_checker.cmake: BIM_ARCH_CHECK_ROOT was not set. Usage: cmake -DBIM_ARCH_CHECK_ROOT=<path> -P tools/architecture_checker.cmake")
endif()

get_filename_component(BIM_ARCH_CHECK_ROOT "${BIM_ARCH_CHECK_ROOT}" ABSOLUTE)

if(NOT EXISTS "${BIM_ARCH_CHECK_ROOT}")
    message(FATAL_ERROR "architecture_checker.cmake: root '${BIM_ARCH_CHECK_ROOT}' does not exist.")
endif()

if(NOT DEFINED BIM_ARCH_CHECK_RULE OR BIM_ARCH_CHECK_RULE STREQUAL "")
    set(BIM_ARCH_CHECK_RULE "ALL")
endif()

set(_bim_valid_rules "ALL" "GEOMETRY_API_NO_OCCT_LEAK" "GEOMETRY_OCCT_ONLY_KERNEL_OWNER")
list(FIND _bim_valid_rules "${BIM_ARCH_CHECK_RULE}" _bim_rule_idx)
if(_bim_rule_idx EQUAL -1)
    message(FATAL_ERROR "architecture_checker.cmake: unknown BIM_ARCH_CHECK_RULE '${BIM_ARCH_CHECK_RULE}'. Valid values: ${_bim_valid_rules}")
endif()

set(BIM_VIOLATIONS "")

# ------------------------------------------------------------------------------
# Helper: scan a list of files for a list of forbidden (case-insensitive)
# substrings; append a human-readable entry to BIM_VIOLATIONS for every hit.
# Whole-file substring search (including comments) is intentional: a
# forbidden third-party type mentioned even in a comment inside a real
# public header is still a signal worth a human look, and this checker is a
# blunt, auditable, dependency-free mechanism, not a full parser.
# Used by R1-R5 only (unchanged since before round 7) - see
# bim_scan_files_for_tokens_ignoring_comments() below, used by R6/R7 only,
# for the comment-aware variant added in round 7's R7-01 correction.
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
# Helper (added round 7, R7-01 correction): strip C/C++ "//" line comments and
# "/* ... */" block comments (including multi-line block comments) from a
# single string of file content, respecting string ("...") and character
# ('...') literals - each scanned with backslash-escape awareness - so a
# "//" or "/*" occurring inside a real literal is never mistaken for the
# start of a comment. Real code before and/or after a comment on the same
# line is preserved unchanged; only the comment text itself is removed.
# Implemented as a position-search loop (string(FIND ...) for each candidate
# marker, advancing past whichever is nearest) rather than a single regex, so
# it does not depend on how far a given CMake regex engine lets "." match
# across a literal newline - block-comment ends are located by literal
# substring search, so multi-line block comments are handled correctly
# regardless of that regex behavior.
# ------------------------------------------------------------------------------
function(bim_strip_cpp_comments content_in out_var)
    set(_remaining "${content_in}")
    set(_out "")
    # A sentinel variable (rather than a bare `while(TRUE)`) is used
    # deliberately: with CMP0012 unset (this script's default, since it sets
    # no cmake_minimum_required/policy of its own), if()/while() fall back to
    # the pre-CMP0012-NEW behavior of dereferencing a bareword condition as a
    # variable name rather than recognizing it as the boolean constant TRUE -
    # `while(TRUE)` then looks up an unset variable named TRUE (empty/false)
    # and the loop body never runs at all, silently. This was caught by
    # actually executing this function against real test input in this
    # sandbox (CMake 3.28.3) during round 7's self-validation, not by static
    # review alone - see section 12 of the handover.
    set(_bim_loop_active TRUE)
    while(_bim_loop_active)
        string(LENGTH "${_remaining}" _rem_len)
        if(_rem_len EQUAL 0)
            set(_bim_loop_active FALSE)
            break()
        endif()

        string(FIND "${_remaining}" "\"" _pos_dq)
        string(FIND "${_remaining}" "'" _pos_sq)
        string(FIND "${_remaining}" "//" _pos_line)
        string(FIND "${_remaining}" "/*" _pos_block)

        set(_min_pos -1)
        set(_kind "")
        foreach(_cand_kind dq sq line block)
            if(_cand_kind STREQUAL "dq")
                set(_cand_pos ${_pos_dq})
            elseif(_cand_kind STREQUAL "sq")
                set(_cand_pos ${_pos_sq})
            elseif(_cand_kind STREQUAL "line")
                set(_cand_pos ${_pos_line})
            else()
                set(_cand_pos ${_pos_block})
            endif()
            if(NOT _cand_pos EQUAL -1)
                if(_min_pos EQUAL -1 OR _cand_pos LESS _min_pos)
                    set(_min_pos ${_cand_pos})
                    set(_kind "${_cand_kind}")
                endif()
            endif()
        endforeach()

        if(_min_pos EQUAL -1)
            # No more quotes/comment-openers in the remainder - it is all
            # real code (or real code with no further literals/comments).
            string(APPEND _out "${_remaining}")
            break()
        endif()

        # Everything strictly before the marker is real code - keep it.
        string(SUBSTRING "${_remaining}" 0 ${_min_pos} _prefix)
        string(APPEND _out "${_prefix}")
        string(SUBSTRING "${_remaining}" ${_min_pos} -1 _tail)

        if(_kind STREQUAL "dq")
            string(REGEX MATCH "^\"(\\\\.|[^\"\\\\])*\"" _match "${_tail}")
            if(_match STREQUAL "")
                # Unterminated string literal - not a comment; keep verbatim.
                string(APPEND _out "${_tail}")
                break()
            endif()
            string(APPEND _out "${_match}")
            string(LENGTH "${_match}" _match_len)
            string(SUBSTRING "${_tail}" ${_match_len} -1 _remaining)
        elseif(_kind STREQUAL "sq")
            string(REGEX MATCH "^'(\\\\.|[^'\\\\])*'" _match "${_tail}")
            if(_match STREQUAL "")
                string(APPEND _out "${_tail}")
                break()
            endif()
            string(APPEND _out "${_match}")
            string(LENGTH "${_match}" _match_len)
            string(SUBSTRING "${_tail}" ${_match_len} -1 _remaining)
        elseif(_kind STREQUAL "line")
            # "//" line comment: discard through end of line, preserving the
            # newline itself so line-oriented content is otherwise unaffected.
            string(FIND "${_tail}" "\n" _nl_pos)
            if(_nl_pos EQUAL -1)
                set(_remaining "")
            else()
                string(SUBSTRING "${_tail}" ${_nl_pos} -1 _remaining)
            endif()
        else() # block comment
            string(SUBSTRING "${_tail}" 2 -1 _after_open)
            string(FIND "${_after_open}" "*/" _close_pos)
            if(_close_pos EQUAL -1)
                # Unterminated block comment - nothing after it is real code.
                set(_remaining "")
            else()
                math(EXPR _skip "${_close_pos} + 2")
                string(SUBSTRING "${_after_open}" ${_skip} -1 _remaining)
            endif()
        endif()
    endwhile()
    set(${out_var} "${_out}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# Helper (added round 7, R7-01 correction): same contract as
# bim_scan_files_for_tokens() above, except each file's content is passed
# through bim_strip_cpp_comments() first, so a forbidden token appearing only
# inside a "//" or "/* ... */" comment is never reported as a violation.
# Real, non-comment code occurrences of a forbidden token still fail exactly
# as before. Used by R6 and R7 only; R1-R5 continue to use the original
# comment-inclusive bim_scan_files_for_tokens() above, unchanged.
# ------------------------------------------------------------------------------
function(bim_scan_files_for_tokens_ignoring_comments files tokens rule_id)
    foreach(f IN LISTS files)
        if(NOT EXISTS "${f}")
            continue()
        endif()
        file(READ "${f}" _content)
        bim_strip_cpp_comments("${_content}" _code_only)
        string(TOLOWER "${_code_only}" _content_lower)
        foreach(tok IN LISTS tokens)
            string(TOLOWER "${tok}" _tok_lower)
            string(FIND "${_content_lower}" "${_tok_lower}" _idx)
            if(NOT _idx EQUAL -1)
                list(APPEND BIM_VIOLATIONS "[${rule_id}] token '${tok}' found in ${f} (code, not comment)")
            endif()
        endforeach()
    endforeach()
    set(BIM_VIOLATIONS "${BIM_VIOLATIONS}" PARENT_SCOPE)
endfunction()

if(BIM_ARCH_CHECK_RULE STREQUAL "ALL")

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

endif() # BIM_ARCH_CHECK_RULE STREQUAL "ALL"

# ------------------------------------------------------------------------------
# R6 - geometry_api public headers must not leak OCCT types (P0-T002;
# Amendment 01 AA-C09). Selectable alone via
# BIM_ARCH_CHECK_RULE=GEOMETRY_API_NO_OCCT_LEAK. Comment-aware since round 7
# (R7-01 correction): scans actual C/C++ code only, ignoring "//" and
# "/* ... */" comments, so architecture documentation that merely names a
# forbidden OCCT token while explaining this very rule does not self-trigger
# it - see bim_scan_files_for_tokens_ignoring_comments() above.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "GEOMETRY_API_NO_OCCT_LEAK")
    file(GLOB_RECURSE _geometry_api_public_headers
        "${BIM_ARCH_CHECK_ROOT}/geometry/api/include/*.h"
        "${BIM_ARCH_CHECK_ROOT}/geometry/api/include/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/geometry/api/include/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/geometry/api/include/*.hxx"
    )
    set(_r6_tokens
        "TopoDS_" "gp_Pnt" "gp_Vec" "gp_Dir" "gp_Trsf" "gp_Ax" "BRep" "Handle(" "opencascade"
    )
    bim_scan_files_for_tokens_ignoring_comments("${_geometry_api_public_headers}" "${_r6_tokens}" "R6-geometry-api-occt-leak")
endif()

# ------------------------------------------------------------------------------
# R7 - bim_geometry_occt is the ONLY owner of OCCT: no OCCT token may appear
# anywhere under BIM_ARCH_CHECK_ROOT outside geometry/occt/** (P0-T002;
# Amendment 01 AA-C09). Selectable alone via
# BIM_ARCH_CHECK_RULE=GEOMETRY_OCCT_ONLY_KERNEL_OWNER. Comment-aware since
# round 7 (R7-01 correction) for the same reason as R6 above - see
# bim_scan_files_for_tokens_ignoring_comments() above.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "GEOMETRY_OCCT_ONLY_KERNEL_OWNER")
    file(GLOB_RECURSE _all_source_files
        "${BIM_ARCH_CHECK_ROOT}/*.h"
        "${BIM_ARCH_CHECK_ROOT}/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/*.hxx"
        "${BIM_ARCH_CHECK_ROOT}/*.cpp"
        "${BIM_ARCH_CHECK_ROOT}/*.cc"
    )
    set(_geometry_occt_dir "${BIM_ARCH_CHECK_ROOT}/geometry/occt/")
    set(_non_occt_owner_files "")
    foreach(f IN LISTS _all_source_files)
        string(FIND "${f}" "${_geometry_occt_dir}" _owner_idx)
        if(_owner_idx EQUAL -1)
            list(APPEND _non_occt_owner_files "${f}")
        endif()
    endforeach()
    set(_r7_tokens
        "TopoDS_" "gp_Pnt" "gp_Vec" "gp_Dir" "gp_Trsf" "gp_Ax" "BRep" "Handle(" "opencascade"
    )
    bim_scan_files_for_tokens_ignoring_comments("${_non_occt_owner_files}" "${_r7_tokens}" "R7-occt-outside-kernel-owner")
endif()

# ------------------------------------------------------------------------------
# Verdict
# ------------------------------------------------------------------------------
list(LENGTH BIM_VIOLATIONS _violation_count)
if(_violation_count GREATER 0)
    message("architecture_checker.cmake: ${_violation_count} violation(s) found under '${BIM_ARCH_CHECK_ROOT}' (rule=${BIM_ARCH_CHECK_RULE}):")
    foreach(v IN LISTS BIM_VIOLATIONS)
        message("  - ${v}")
    endforeach()
    message(FATAL_ERROR "ARCHITECTURE CHECK FAILED")
endif()

message("architecture_checker.cmake: scanned root '${BIM_ARCH_CHECK_ROOT}' (rule=${BIM_ARCH_CHECK_RULE}), 0 violations.")
message("ARCHITECTURE CHECK PASSED")
