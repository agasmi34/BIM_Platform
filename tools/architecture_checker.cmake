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
# BIM_ARCH_CHECK_RULE (added P0-T002 Phase C-M, Amendment 01 AA-C09; extended
# P0-T003) selects which rule group to run:
#   ALL                          (default) - every rule below (R1-R11)
#   GEOMETRY_API_NO_OCCT_LEAK    - only R6
#   GEOMETRY_OCCT_ONLY_KERNEL_OWNER - only R7
#   VIEWPORT_PUBLIC_NEUTRAL      - only R8  (P0-T003)
#   QT_DESKTOP_ONLY              - only R9  (P0-T003)
#   BGFX_VIEWPORT_OWNER          - only R10 (P0-T003)
#   NO_DIRECT_D3D                - only R11 (P0-T003)
# This selector exists so each single-rule CTest test (arch_geometry_api_no_occt_leak,
# arch_geometry_occt_only_kernel_owner, and, as of P0-T003,
# arch_viewport_public_neutral / arch_qt_desktop_only / arch_bgfx_viewport_owner /
# arch_no_direct_d3d - tests/architecture/CMakeLists.txt) can each exercise
# exactly one rule, while arch_repository_boundaries and
# arch_checker_detects_violation continue to omit BIM_ARCH_CHECK_RULE entirely
# and so continue to run every rule (ALL), preserving their pre-P0-T002
# behavior unchanged.
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
#   R8 - (P0-T003) bim::viewport's public contract stays neutral: no
#        Qt/bgfx/D3D/Windows/OCCT/BIM-semantic token under
#        <root>/viewport/include/** or <root>/viewport/bgfx/include/**
#        (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0 sections 4-9:
#        the neutral contract depends on nothing first-party and nothing
#        third-party beyond the standard library, and bim::viewport_bgfx's
#        one public header exposes zero bgfx types via pimpl)
#   R9 - (P0-T003) no Qt token anywhere under BIM_ARCH_CHECK_ROOT outside
#        <root>/desktop/** - bim_desktop_spike remains the ONLY Qt owner
#        (Brief section 11), mirroring R7's ownership-partition pattern
#   R10 - (P0-T003) no bgfx token anywhere under BIM_ARCH_CHECK_ROOT
#        outside <root>/viewport/bgfx/** - bim_viewport_bgfx remains the
#        ONLY bgfx owner (Brief section 9), mirroring R7's
#        ownership-partition pattern
#   R11 - (P0-T003) no direct D3D11/DXGI token anywhere under
#        BIM_ARCH_CHECK_ROOT, with NO exception directory (not even
#        <root>/viewport/bgfx/**) - D3D11 is reached ONLY through bgfx
#        itself, never called directly by any first-party BIM Platform
#        code (Brief section 9). Deliberately stricter than a bare "no
#        direct D3D11" framing: this also forbids the dxgi.h/IDXGI
#        tokens, disclosed as an intentional addition rather than a
#        literal restatement of the Brief text.
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
# P0-T003 ADDITION NOTE - R8/VIEWPORT_PUBLIC_NEUTRAL, R9/QT_DESKTOP_ONLY,
# R10/BGFX_VIEWPORT_OWNER, R11/NO_DIRECT_D3D
#
# Added for the P0-T003 Desktop + Viewport Spike (Implementation Brief
# BIM-TASK-P0-T003-CLAUDE v1.0 sections 4, 9, 11; Implementation
# Authorization section 6). R8 reuses bim_scan_files_for_tokens_ignoring_comments()
# (the round-7 comment-aware helper) deliberately, to avoid repeating the
# exact R7-01 false-positive defect that helper was written to fix - a
# design comment in a viewport header that merely names a forbidden token
# while explaining why it is forbidden (as this very file's comments do)
# must not self-trigger the rule. R9 and R10 both replicate R7's
# ownership-partition pattern (partition all first-party files by whether
# they live under the rule's owner directory, then scan only the
# outside-owner subset) rather than inventing a new mechanism - R9 for
# "Qt only inside desktop/**", R10 for "bgfx only inside viewport/bgfx/**".
# R11 has deliberately NO exception directory, unlike R9/R10: not even
# bim_viewport_bgfx itself may reference D3D11/DXGI tokens directly, since
# it is meant to reach D3D11 exclusively through bgfx's own abstraction.
#
# This addition was authored without a live CMake execution channel in
# this session (no execution channel - see docs/evidence/P0-T003/CLAUDE_HANDOVER.md)
# and could not be self-validated against real fixture input the way round
# 7's R7-01 correction was (that correction's self-validation is described
# above and remains accurate for R1-R7 only). The Operator's first real
# `ctest -R "^arch_"` run is this addition's first actual execution.
# ==============================================================================
# AA RUNBOOK D ATTEMPT 1 CORRECTION NOTE (RD1-05) - R7/R8/R10 LEXICAL
# FALSE-POSITIVE FIX
#
# The above addition's "first actual execution" happened: the authoritative
# first Runbook D attempt ran the real build and its real CTest suite, and
# R7, R8, and R10 rejected genuine, architecturally-allowed project code -
# not a fixture, and not a defect in the architecture itself:
#   - R7 (repo-wide OCCT-owner) and R8 (viewport public-neutral contract)
#     both matched their "Handle(" token as a plain substring, so real
#     identifiers merely ENDING with that text - `...windowHandle()` calls
#     in src/desktop/src/main.cpp, and RenderMeshHandle's own constructor
#     declarations inside src/viewport/bgfx/include/bim/viewport_bgfx/
#     renderer.hpp (R8's own scanned scope) - were mistaken for the
#     standalone OCCT `Handle(...)` construction these rules exist to
#     forbid.
#   - R10 (bgfx sole-owner) matched "bgfx::" as a plain substring, so the
#     legitimate first-party namespace `bim::viewport_bgfx::...` (used
#     throughout src/desktop/** to call the allowed adapter, e.g.
#     `bim::viewport_bgfx::Renderer`) was mistaken for a desktop-side raw
#     `bgfx::` call, which is exactly the ownership violation R10 exists to
#     catch - but this is not that violation.
#
# Correction: "Handle(" (R7, R8) and "bgfx::"/"BGFX_" (R10) are now scanned
# by a new bim_scan_files_for_boundary_tokens_ignoring_comments() (see
# above), which requires the match to sit at an identifier boundary (not
# immediately preceded by [a-z0-9_]) rather than matching anywhere as a
# substring. Every other token in R7/R8/R10's sets, and every token used by
# R1-R6/R9/R11, is completely untouched - still the original plain
# substring (or comment-aware substring) matcher, unchanged. The negative
# fixtures this correction must not weaken
# (tests/fixtures/p0_t003_bad_bgfx_owner's `bgfx::touch(0);`, and R7/R8's
# pre-existing OCCT fixture(s) outside this task's own footprint) were
# reasoned through rather than re-executed (still no execution channel in
# this session): each fixture's forbidden token is preceded by whitespace
# or a statement-boundary character, never by an identifier character, so
# the boundary condition this correction adds does not change whether they
# are caught - disclosed as reasoned-not-executed, consistent with every
# other UNVERIFIED disclosure in this file and in
# docs/evidence/P0-T003/CLAUDE_HANDOVER.md.
# ==============================================================================
# AA RD1.1 CORRECTION NOTE (RD1-05A) - R10 "BGFX_" MADE CASE-SENSITIVE
#
# Architecture Authority actually EXECUTED the RD1-05-corrected checker (the
# first time this had happened for this specific rule) against an isolated
# copy of the current candidate: GEOMETRY_OCCT_ONLY_KERNEL_OWNER and
# VIEWPORT_PUBLIC_NEUTRAL both PASS, but BGFX_VIEWPORT_OWNER FAILED with
# exactly one false positive - src/desktop/src/evidence_mode.cpp's real,
# legitimate evidence-JSON field key "bgfx_version_frozen_baseline"
# (lowercase, a string literal naming what it records - the frozen bgfx
# port version - not a macro use).
#
# Root cause: RD1-05's bim_scan_files_for_boundary_tokens_ignoring_comments()
# lowercases both file content and token before matching, which was correct
# for "Handle(" and "bgfx::" (neither is meant to be case-sensitive) but
# wrong for "BGFX_" - real bgfx macros (BGFX_STATE_DEFAULT and friends) are
# always upper-case; lowercasing the token turned "BGFX_" into "bgfx_",
# which then matched the leading six letters of the unrelated lowercase
# identifier/string above.
#
# Correction: added bim_scan_files_for_case_sensitive_boundary_tokens_ignoring_comments()
# (below, immediately after the case-insensitive variant), identical in
# every other respect (comment-stripped first, same identifier-boundary
# rule, same leading sentinel, same optional-whitespace-before-"("
# allowance) but WITHOUT the string(TOLOWER ...) calls. R10 now routes
# "BGFX_" through this exact-case path while "bgfx::" stays on the
# original case-insensitive boundary path, unchanged - "Handle(" (R7/R8)
# is untouched by this correction entirely, still case-insensitive as
# intended. src/desktop/src/evidence_mode.cpp was not modified and no
# fixture was modified or suppressed to hide this false positive - the
# checker's own matching logic was the defect.
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

set(_bim_valid_rules
    "ALL"
    "GEOMETRY_API_NO_OCCT_LEAK"
    "GEOMETRY_OCCT_ONLY_KERNEL_OWNER"
    "VIEWPORT_PUBLIC_NEUTRAL"
    "QT_DESKTOP_ONLY"
    "BGFX_VIEWPORT_OWNER"
    "NO_DIRECT_D3D"
)
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

# ------------------------------------------------------------------------------
# Helper (AA Runbook D Attempt 1 correction, RD1-05): same contract as
# bim_scan_files_for_tokens_ignoring_comments() above (comments stripped
# first via bim_strip_cpp_comments()), but additionally requires each match
# to sit at an identifier boundary - the character immediately before the
# token's first character must NOT be an identifier character
# ([a-z0-9_], checked post-tolower). This is what R7/R8's "Handle(" token
# and R10's "bgfx::"/"BGFX_" tokens actually need and did not have: the
# authoritative first Runbook D attempt found R7/R8 rejecting real,
# project-owned code such as `... .windowHandle();` (main.cpp) and
# `RenderMeshHandle(...)` (renderer.hpp/.cpp) purely because those
# identifiers END with the substring "Handle(" - neither is the standalone
# OCCT `Handle(...)` construction R7/R8 exist to forbid. The same class of
# false positive applies to R10's "bgfx::" against the legitimate
# "bim::viewport_bgfx::..." namespace (the raw token is a substring of
# "..._bgfx::...").
#
# Only the SPECIFIC tokens a caller opts in via `boundary_tokens` go through
# this stricter path; every other token in a rule's token set keeps using
# the existing, unchanged substring matcher above - this does not change
# matching behavior for any token not explicitly opted in, and does not
# weaken any rule's ability to catch a genuine, boundary-correct violation
# (the negative fixtures' own tokens - e.g. leaky_bgfx.cpp's
# `bgfx::touch(0);` - are themselves already preceded by whitespace/
# punctuation, never by an identifier character, so they remain caught).
#
# A single leading sentinel character is prepended to each file's
# (comment-stripped, lowercased) content before matching, so a real token
# occurring at byte offset 0 is still correctly boundary-matched - this
# avoids relying on CMake regex "^" mid-string, which anchors to the start
# of the whole subject string, not to each candidate match position.
#
# A token ending in a literal "(" (currently only "Handle(") additionally
# allows optional horizontal whitespace between the identifier and "(" -
# "Handle (" still counts as the same OCCT construction pattern. Every
# other boundary token here (":: "/"_"-suffixed, no trailing "(") is
# matched as an exact literal suffix. The known boundary_tokens values
# passed by R7/R8/R10 below contain no regex metacharacters other than the
# ":"/"("/"_" already accounted for, so this helper does not attempt
# general-purpose regex escaping - a future boundary token containing
# other regex-special characters would need this helper extended first.
# ------------------------------------------------------------------------------
function(bim_scan_files_for_boundary_tokens_ignoring_comments files boundary_tokens rule_id)
    foreach(f IN LISTS files)
        if(NOT EXISTS "${f}")
            continue()
        endif()
        file(READ "${f}" _content)
        bim_strip_cpp_comments("${_content}" _code_only)
        string(TOLOWER "${_code_only}" _content_lower)
        # Leading sentinel - see comment above.
        set(_content_lower " ${_content_lower}")
        foreach(tok IN LISTS boundary_tokens)
            string(TOLOWER "${tok}" _tok_lower)
            string(REGEX MATCH "\\($" _ends_with_paren "${_tok_lower}")
            if(NOT _ends_with_paren STREQUAL "")
                string(LENGTH "${_tok_lower}" _tok_len)
                math(EXPR _prefix_len "${_tok_len} - 1")
                string(SUBSTRING "${_tok_lower}" 0 ${_prefix_len} _tok_ident)
                set(_pattern "[^a-z0-9_]${_tok_ident}[ \t]*\\(")
            else()
                set(_pattern "[^a-z0-9_]${_tok_lower}")
            endif()
            string(REGEX MATCH "${_pattern}" _match "${_content_lower}")
            if(NOT _match STREQUAL "")
                list(APPEND BIM_VIOLATIONS "[${rule_id}] boundary-matched token '${tok}' found in ${f} (code, not comment)")
            endif()
        endforeach()
    endforeach()
    set(BIM_VIOLATIONS "${BIM_VIOLATIONS}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# Helper (AA RD1.1 correction, RD1-05A): identical contract to
# bim_scan_files_for_boundary_tokens_ignoring_comments() immediately above -
# same identifier-boundary rule, same leading sentinel, same optional
# -whitespace-before-"(" allowance - EXCEPT this variant does not
# lowercase either the file content or the token before matching, so it is
# exact-case. Added specifically for R10's "BGFX_" macro-prefix token:
# Architecture Authority's own executed-checker run found the
# case-insensitive path (which R10 used for "BGFX_" until this correction)
# lowercasing src/desktop/src/evidence_mode.cpp's legitimate lowercase
# metadata string/identifier "bgfx_version_frozen_baseline" down to a false
# match against the lowercased "bgfx_" token - "BGFX_" is, in real bgfx
# headers, always an upper-case macro prefix (e.g. BGFX_STATE_DEFAULT), and
# a lowercase identifier that merely happens to start with the same six
# letters is not that macro and must not be flagged. "Handle(" (R7/R8) and
# raw "bgfx::" (R10) are deliberately NOT switched to this path - the AA's
# instruction is explicit that both remain case-insensitive as already
# intended - only "BGFX_" moves here.
# ------------------------------------------------------------------------------
function(bim_scan_files_for_case_sensitive_boundary_tokens_ignoring_comments files boundary_tokens rule_id)
    foreach(f IN LISTS files)
        if(NOT EXISTS "${f}")
            continue()
        endif()
        file(READ "${f}" _content)
        bim_strip_cpp_comments("${_content}" _code_only)
        # No string(TOLOWER ...) here - this path is deliberately exact-case
        # (RD1-05A).
        set(_content_exact " ${_code_only}")
        foreach(tok IN LISTS boundary_tokens)
            string(REGEX MATCH "\\($" _ends_with_paren "${tok}")
            if(NOT _ends_with_paren STREQUAL "")
                string(LENGTH "${tok}" _tok_len)
                math(EXPR _prefix_len "${_tok_len} - 1")
                string(SUBSTRING "${tok}" 0 ${_prefix_len} _tok_ident)
                set(_pattern "[^A-Za-z0-9_]${_tok_ident}[ \t]*\\(")
            else()
                set(_pattern "[^A-Za-z0-9_]${tok}")
            endif()
            string(REGEX MATCH "${_pattern}" _match "${_content_exact}")
            if(NOT _match STREQUAL "")
                list(APPEND BIM_VIOLATIONS "[${rule_id}] case-sensitive boundary-matched token '${tok}' found in ${f} (code, not comment)")
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
    # AA Runbook D Attempt 1 correction (RD1-05): "Handle(" moved out to a
    # separate boundary-aware pass below - see
    # bim_scan_files_for_boundary_tokens_ignoring_comments() above. The
    # authoritative first attempt found this plain substring match
    # rejecting real project code such as `...windowHandle();` (main.cpp)
    # and `RenderMeshHandle(...)` (renderer.cpp), neither of which is the
    # standalone OCCT `Handle(...)` construction this token exists to catch.
    set(_r7_tokens
        "TopoDS_" "gp_Pnt" "gp_Vec" "gp_Dir" "gp_Trsf" "gp_Ax" "BRep" "opencascade"
    )
    set(_r7_boundary_tokens "Handle(")
    bim_scan_files_for_tokens_ignoring_comments("${_non_occt_owner_files}" "${_r7_tokens}" "R7-occt-outside-kernel-owner")
    bim_scan_files_for_boundary_tokens_ignoring_comments("${_non_occt_owner_files}" "${_r7_boundary_tokens}" "R7-occt-outside-kernel-owner")
endif()

# ------------------------------------------------------------------------------
# R8 - bim::viewport's public contract (VIEWPORT_PUBLIC_NEUTRAL, P0-T003) -
# no Qt/bgfx/D3D/Windows/OCCT/BIM-semantic token under
# <root>/viewport/include/** or <root>/viewport/bgfx/include/**. Selectable
# alone via BIM_ARCH_CHECK_RULE=VIEWPORT_PUBLIC_NEUTRAL. Comment-aware (see
# P0-T003 ADDITION NOTE above) for the same reason R6/R7 are.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "VIEWPORT_PUBLIC_NEUTRAL")
    file(GLOB_RECURSE _viewport_public_headers
        "${BIM_ARCH_CHECK_ROOT}/viewport/include/*.h"
        "${BIM_ARCH_CHECK_ROOT}/viewport/include/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/viewport/include/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/viewport/include/*.hxx"
        "${BIM_ARCH_CHECK_ROOT}/viewport/bgfx/include/*.h"
        "${BIM_ARCH_CHECK_ROOT}/viewport/bgfx/include/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/viewport/bgfx/include/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/viewport/bgfx/include/*.hxx"
    )
    # AA Runbook D Attempt 1 correction (RD1-05): "Handle(" moved out to a
    # separate boundary-aware pass below (same rationale as R7's above - the
    # authoritative first attempt found this token, as a plain substring,
    # rejecting real project code such as `RenderMeshHandle(...)`, declared
    # right inside this rule's own scanned scope in renderer.hpp). The
    # bgfx/Qt/D3D/BIM-semantic tokens are untouched - RD1-05 did not report
    # a false positive against any of them within viewport/include/** or
    # viewport/bgfx/include/**, the neutral contract's own public headers
    # never legitimately reference bim::viewport_bgfx (or any other
    # first-party module) by design, so R8's "bgfx::" token has no
    # equivalent RD1-05 finding the way R10's does below.
    set(_r8_tokens
        # Qt
        "<Qt" "QObject" "QWidget" "QString" "Qt::"
        # bgfx
        "<bgfx/" "bgfx::" "BGFX_"
        # D3D / Windows native
        "d3d11.h" "ID3D11" "D3D11_" "dxgi.h" "IDXGI" "<Windows.h>" "HWND"
        # OCCT
        "TopoDS_" "gp_Pnt" "gp_Vec" "gp_Dir" "gp_Trsf" "gp_Ax" "BRep" "opencascade"
        # BIM semantic/model/query modules (bim::viewport/bim::viewport_bgfx
        # themselves are never matched by these tokens - see the namespace
        # -qualified forms chosen below)
        "bim::model" "bim::query" "bim::transactions" "bim::commands"
        "bim::persistence" "bim::foundation" "bim::geometry_api" "bim::geometry_occt"
        "bim/model/" "bim/query/" "bim/transactions/" "bim/commands/"
        "bim/persistence/" "bim/foundation/" "bim/geometry/"
    )
    set(_r8_boundary_tokens "Handle(")
    bim_scan_files_for_tokens_ignoring_comments("${_viewport_public_headers}" "${_r8_tokens}" "R8-viewport-public-neutral")
    bim_scan_files_for_boundary_tokens_ignoring_comments("${_viewport_public_headers}" "${_r8_boundary_tokens}" "R8-viewport-public-neutral")
endif()

# ------------------------------------------------------------------------------
# R9 - Qt is owned exclusively by src/desktop/** (QT_DESKTOP_ONLY, P0-T003).
# Mirrors R7's ownership-partition pattern: partition all first-party files
# by whether they live under <root>/desktop/**, then scan only the
# outside-owner subset for Qt tokens. Selectable alone via
# BIM_ARCH_CHECK_RULE=QT_DESKTOP_ONLY.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "QT_DESKTOP_ONLY")
    file(GLOB_RECURSE _all_source_files_r9
        "${BIM_ARCH_CHECK_ROOT}/*.h"
        "${BIM_ARCH_CHECK_ROOT}/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/*.hxx"
        "${BIM_ARCH_CHECK_ROOT}/*.cpp"
        "${BIM_ARCH_CHECK_ROOT}/*.cc"
    )
    set(_desktop_dir "${BIM_ARCH_CHECK_ROOT}/desktop/")
    set(_non_desktop_owner_files "")
    foreach(f IN LISTS _all_source_files_r9)
        string(FIND "${f}" "${_desktop_dir}" _owner_idx)
        if(_owner_idx EQUAL -1)
            list(APPEND _non_desktop_owner_files "${f}")
        endif()
    endforeach()
    # AA Source Review Round 2 MINOR: R9's token list previously covered
    # only a handful of the most common Qt Widgets symbols, giving weaker
    # coverage than R8's own grouped Qt list just above (which additionally
    # matches "<Qt" - header includes - and "Qt::"). Expanded here to also
    # catch QWindow/QScreen-based surface code (src/desktop's own M08/M11
    # corrections use these directly) and the other Qt Gui/Widgets/Core
    # symbols this codebase actually uses, mirroring R8's grouped style.
    set(_r9_tokens
        "<Qt" "Qt::" "QObject" "QWidget" "QString" "QApplication"
        "QGuiApplication" "QCoreApplication" "QWindow" "QMainWindow"
        "QTimer" "QScreen" "QMouseEvent" "QWheelEvent" "QResizeEvent"
        "QShowEvent" "QHideEvent" "QCloseEvent" "QExposeEvent"
        "QPlatformSurfaceEvent" "QDebug" "qDebug(" "Q_OBJECT" "Q_ASSERT"
        "QComboBox" "QLabel" "QStatusBar" "QToolBar" "QVBoxLayout"
    )
    bim_scan_files_for_tokens_ignoring_comments("${_non_desktop_owner_files}" "${_r9_tokens}" "R9-qt-outside-desktop-owner")
endif()

# ------------------------------------------------------------------------------
# R10 - bgfx is owned exclusively by src/viewport/bgfx/** (BGFX_VIEWPORT_OWNER,
# P0-T003). Mirrors R7's ownership-partition pattern, symmetric to R9.
# Selectable alone via BIM_ARCH_CHECK_RULE=BGFX_VIEWPORT_OWNER.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "BGFX_VIEWPORT_OWNER")
    file(GLOB_RECURSE _all_source_files_r10
        "${BIM_ARCH_CHECK_ROOT}/*.h"
        "${BIM_ARCH_CHECK_ROOT}/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/*.hxx"
        "${BIM_ARCH_CHECK_ROOT}/*.cpp"
        "${BIM_ARCH_CHECK_ROOT}/*.cc"
    )
    set(_viewport_bgfx_dir "${BIM_ARCH_CHECK_ROOT}/viewport/bgfx/")
    set(_non_bgfx_owner_files "")
    foreach(f IN LISTS _all_source_files_r10)
        string(FIND "${f}" "${_viewport_bgfx_dir}" _owner_idx)
        if(_owner_idx EQUAL -1)
            list(APPEND _non_bgfx_owner_files "${f}")
        endif()
    endforeach()
    # AA Runbook D Attempt 1 correction (RD1-05): "bgfx::" and "BGFX_" moved
    # out to a separate boundary-aware pass below - the authoritative first
    # attempt found "bgfx::", as a plain substring, matching the tail of the
    # legitimate first-party namespace "bim::viewport_bgfx::..." (e.g.
    # "bim::viewport_bgfx::Renderer" used throughout src/desktop/**), which
    # is exactly the adapter usage the frozen architecture allows - not a
    # desktop-side raw bgfx:: call. "<bgfx/" has no equivalent false
    # positive (no legitimate identifier ends with that literal, "<"
    # -prefixed substring) and stays on the plain substring path.
    #
    # AA RD1.1 correction (RD1-05A): "BGFX_" split out from "bgfx::" into
    # its own, exact-case pass. Architecture Authority's own executed run
    # of the RD1-05 checker found "BGFX_", via the shared case-insensitive
    # boundary path both tokens previously used, lowercasing to "bgfx_" and
    # false-matching src/desktop/src/evidence_mode.cpp's legitimate
    # lowercase metadata string/identifier "bgfx_version_frozen_baseline" -
    # a real bgfx macro prefix is always upper-case (e.g.
    # BGFX_STATE_DEFAULT), so a lowercase identifier merely starting with
    # the same letters is not that macro. "bgfx::" keeps the
    # case-insensitive path unchanged (still correctly rejects
    # tests/fixtures/p0_t003_bad_bgfx_owner's `bgfx::touch(0);` and still
    # correctly allows `bim::viewport_bgfx::Renderer`) - only "BGFX_"
    # moves to the new exact-case helper, per the AA's explicit instruction
    # not to make "BGFX_" case-insensitive.
    set(_r10_tokens "<bgfx/")
    set(_r10_case_insensitive_boundary_tokens "bgfx::")
    set(_r10_case_sensitive_boundary_tokens "BGFX_")
    bim_scan_files_for_tokens_ignoring_comments("${_non_bgfx_owner_files}" "${_r10_tokens}" "R10-bgfx-outside-viewport-owner")
    bim_scan_files_for_boundary_tokens_ignoring_comments("${_non_bgfx_owner_files}" "${_r10_case_insensitive_boundary_tokens}" "R10-bgfx-outside-viewport-owner")
    bim_scan_files_for_case_sensitive_boundary_tokens_ignoring_comments("${_non_bgfx_owner_files}" "${_r10_case_sensitive_boundary_tokens}" "R10-bgfx-outside-viewport-owner")
endif()

# ------------------------------------------------------------------------------
# R11 - no first-party file, anywhere, includes D3D11/DXGI directly
# (NO_DIRECT_D3D, P0-T003). Deliberately has NO exception directory - not
# even <root>/viewport/bgfx/** - D3D11 is reached ONLY through bgfx's own
# abstraction. Selectable alone via BIM_ARCH_CHECK_RULE=NO_DIRECT_D3D.
# ------------------------------------------------------------------------------
if(BIM_ARCH_CHECK_RULE STREQUAL "ALL" OR BIM_ARCH_CHECK_RULE STREQUAL "NO_DIRECT_D3D")
    file(GLOB_RECURSE _all_source_files_r11
        "${BIM_ARCH_CHECK_ROOT}/*.h"
        "${BIM_ARCH_CHECK_ROOT}/*.hpp"
        "${BIM_ARCH_CHECK_ROOT}/*.hh"
        "${BIM_ARCH_CHECK_ROOT}/*.hxx"
        "${BIM_ARCH_CHECK_ROOT}/*.cpp"
        "${BIM_ARCH_CHECK_ROOT}/*.cc"
    )
    set(_r11_tokens "d3d11.h" "ID3D11" "D3D11_" "dxgi.h" "IDXGI")
    bim_scan_files_for_tokens_ignoring_comments("${_all_source_files_r11}" "${_r11_tokens}" "R11-no-direct-d3d")
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
