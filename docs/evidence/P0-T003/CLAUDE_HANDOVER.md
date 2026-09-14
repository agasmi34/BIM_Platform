# P0-T003 - Desktop + Viewport Spike - Claude Implementation Handover

**Role:** Claude, the Implementation Engineer
**Task:** P0-T003 Desktop + Viewport Spike (production implementation)
**Authorization:** Architecture Gate commit `cf7a971903d8103143b2b94e94a4d185d86ce42b`; main baseline `3f2230be5fcd796c370f485975547112ad52d2e3`; task branch `task/P0-T003-desktop-viewport-spike`; task worktree `D:\Projects\BIM-Platform-WT-P0-T003`
**Companion document:** `docs/evidence/P0-T003/CLAUDE_HANDOVER.json` (machine-readable manifest: every one of the 59 authorized paths with its disposition, byte size, and SHA256)

---

## AA Source Review Round 1 correction round - summary (read this first if you already know section 0)

A correction-round instruction from the user identified finding **B01** (literal
`</content>`/`</invoke>` tag contamination trailing 35 files) and **M01-M07**
(seven "major" functional/architectural gaps), and asked Claude to resolve
three additional **MINOR** findings of its own determination. This round
applied **minimum-delta corrections only** to the existing 59-path candidate:
**37 of the 59 files were touched**, **20 were left byte-for-byte
unmodified**, and **zero paths outside the authorized 59 were touched**. No
file was regenerated/rewritten beyond what a finding actually required. Full
detail - the exact finding-by-finding resolution, the disclosure that no
"AA Source Review Round 1" document actually exists on disk (the review
content came entirely from the user's own chat message), and the complete
changed-path summary - is in **section 7** below and in this document's JSON
companion under the `aa_source_review_round_1_corrections` key. **ACR status
for this round: NONE** - no requested correction proved impossible under the
locked architecture. **Nothing in this round has been compiled, linked, or
run** - see section 0, unchanged by this round: this session still has no
execution channel to the Windows worktree.

---

## AA Source Review Round 2 correction round - summary (read this first if you already know sections 0/7)

A second correction-round instruction from the user identified finding
**B02** and **M08-M12** (five "major" functional/architectural gaps),
plus asked Claude to close the three remaining self-determined **MINOR**
findings disclosed but not yet fully closed after Round 1. This round again
applied **minimum-delta corrections only** to the existing 59-path
candidate: **15 of the 59 files were touched**, the remaining 44 were left
byte-for-byte unmodified, and **zero paths outside the authorized 59 were
touched**. Full detail is in **section 8** below and in this document's JSON
companion under the `aa_source_review_round_2_corrections` key. **ACR status
for this round: NONE.** Per the user's explicit instruction, **this round did
not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`** - B02 and M12
changed that script's own logic (real vcpkg-configuration.json baseline read,
Ninja fallback resolution, a real fresh configure+build ahead of its
dependent checks, hard-fail VS/VCTools mismatch, exact qtbase/bgfx
version#portversion enforcement, main-worktree discovery/clean check), but
none of it has been executed - see section 0, still unchanged: this session
still has no execution channel to the Windows worktree.

---

## AA Source Review Round 3 correction round - summary (read this first if you already know sections 0/7/8)

A third correction-round instruction from the user identified findings **B03** and
**M13-M15** (three "major" functional/architectural gaps) plus a four-part
self-determined **MINOR** bundle (two `noexcept` removals, a differing-DPI
baseline fix, and this exact handover-wording correction). This round again
applied **minimum-delta corrections only**: **8 of the 57 non-handover
authorized paths were touched** (`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`,
`src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/viewport_window.hpp`,
`src/desktop/src/viewport_window.cpp`, `src/desktop/src/evidence_mode.hpp`,
`src/desktop/src/evidence_mode.cpp`, `src/desktop/src/main.cpp`,
`Verification-RunbookD-v1.0.ps1`), plus this document and its JSON companion
(which, per this round's own MINOR wording correction below, are properly
counted as part of the 59-path footprint, not separately from it) - **10 of
the 59 authorized paths touched in total**, the remaining 49 left
byte-for-byte unmodified, and **zero paths outside the authorized 59 were
touched**. Full detail is in **section 9** below and in this document's JSON
companion under the `aa_source_review_round_3_corrections` key. **ACR status
for this round: NONE** - no requested correction proved impossible under the
locked architecture. Per the user's explicit instruction, **this round did
not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`** either - B03
and M15 changed that script's own logic (a process-local x64 VS developer
environment import via `vcvarsall.bat`, forced `VCPKG_ROOT`/Ninja/`cl.exe`
selection, exact `CMAKE_TOOLCHAIN_FILE`/`VCPKG_TARGET_TRIPLET`/clang-tooling
-version fail-closed checks, and an execution-order fix so every
tool-identity check that can run before a configure now does), but none of
it has been executed - see section 0, still unchanged: this session still
has no execution channel to the Windows worktree.

---

## AA Source Review Round 4 correction round - summary (read this first if you already know sections 0/7/8/9)

A fourth correction-round instruction from the user identified **M13 final
closure** (surface-recreation, minimize/restore, and differing-DPI live
evidence must each prove the actual locked `ViewportWindow`/`ViewportSurface`
lifecycle callbacks really executed, not merely equivalent method calls or a
top-level visibility change) plus a three-part **MINOR** hardening bundle for
`Verification-RunbookD-v1.0.ps1` (exact-only VS `installationVersion`
comparison; exact normalized full-path `cl.exe` comparison; genuine
semantic-version parsing for clang-format/clang-tidy) and an optional stale
-comment correction in `scripts/ci/viewport-spike.ps1`. This round again
applied **minimum-delta corrections only**: **5 of the 57 non-handover
authorized paths were touched** (`src/desktop/src/viewport_window.hpp`,
`src/desktop/src/viewport_window.cpp`, `src/desktop/src/evidence_mode.cpp`,
`Verification-RunbookD-v1.0.ps1`, `scripts/ci/viewport-spike.ps1`), plus this
document and its JSON companion - **7 of the 59 authorized paths touched in
total**, the remaining 52 left byte-for-byte unmodified, and **zero paths
outside the authorized 59 were touched**. Full detail is in **section 10**
below and in this document's JSON companion under the
`aa_source_review_round_4_corrections` key. **ACR status for this round:
NONE** - no requested correction proved impossible under the locked
architecture. Per the user's explicit instruction, **this round did not run
or claim a PASS for `Verification-RunbookD-v1.0.ps1`** either - see section
0, still unchanged: this session still has no execution channel to the
Windows worktree.

---

## AA Source Review Round 5 correction round - summary (read this first if you already know sections 0/7/8/9/10)

A fifth, "final pre-Runbook minimum delta" correction-round instruction from
the user identified finding **M16** (live evidence constructed
`ViewportWindow` itself as the top-level window, so Round 4's
`changeEvent()`-based minimize/restore fix did not actually prove the real
production `MainWindow`-embedded child lifecycle path) plus **N03** (surface
-recreation re-exposure must go through the `QWidget::createWindowContainer()`
container's own visibility path, not `surface_->setVisible(true)` directly),
**N04** (`cmake --version` must be parsed and compared exactly, not matched
via an unanchored substring regex), and **N05** (Runbook D must additionally
require the resolved Visual Studio `installationPath` to exactly match the
one accepted Build Tools 2022 root). This round again applied
**minimum-delta corrections only**: **7 of the 57 non-handover authorized
paths were touched** (`src/desktop/src/viewport_window.hpp`,
`src/desktop/src/viewport_window.cpp`, `src/desktop/src/main.cpp`,
`src/desktop/src/main_window.hpp`, `src/desktop/src/evidence_mode.cpp`,
`src/desktop/src/evidence_mode.hpp`, `Verification-RunbookD-v1.0.ps1`), plus
this document and its JSON companion - **9 of the 59 authorized paths
touched in total**, the remaining 50 left byte-for-byte unmodified, and
**zero paths outside the authorized 59 were touched**. Full detail is in
**section 11** below and in this document's JSON companion under the
`aa_source_review_round_5_corrections` key. **ACR status for this round:
NONE** - no requested correction proved impossible under the locked
architecture. Per the user's explicit instruction, **this round did not run
or claim a PASS for `Verification-RunbookD-v1.0.ps1`** - see section 0,
still unchanged: this session still has no execution channel to the Windows
worktree.

---

## AA Runbook D Attempt 1 correction round - summary (read this first if you already know sections 0/7/8/9/10/11)

For the first time since work on P0-T003 began, `Verification-RunbookD-v1.0.ps1`
was actually run by Architecture Authority against a real Windows build -
**Runbook D Attempt 1 FAILED.** This is a genuinely different situation
from Rounds 1-5, which were all pre-execution reads of the source. The AA's
correction directive identified one real **BUILD BLOCKER** (**RD1-01**:
shaderc could not resolve `#include <bgfx_shader.sh>` - no shader include
directory was ever passed to shaderc) and five confirmed defects in
`Verification-RunbookD-v1.0.ps1`/`tools/architecture_checker.cmake`
themselves, exposed only by that real run: **RD1-02** (Check 3 falsely
flagged parent directories such as `src/desktop/src/` as out-of-scope -
`git status --porcelain` collapses an entirely-untracked directory into one
line instead of recursing into it), **RD1-03** (the "effective compiler"
check required `CMAKE_CXX_COMPILER` inside `CMakeCache.txt`, which the real
configure did not populate there, even though the genuinely-resolved
compiler identity was available in CMake's own generated
`CMakeCXXCompiler.cmake`), **RD1-04** (items 11-14's `vcpkg list` calls hit
the wrong/default install root and reported "No packages are installed";
separately, `vcpkg list`'s default output does not even print feature
names, so items 13/14 could never have matched regardless), and **RD1-05**
(the architecture checker's R7/R8/R10 lexical substring matching rejected
real, allowed project code - `windowHandle()`, `RenderMeshHandle(...)`,
`bim::viewport_bgfx::...` - because each merely ends with, or contains as a
tail substring, a forbidden token). A sixth, purely cosmetic finding,
**RD1-06**, corrected one clang-format line-length violation
(`renderer.hpp` around line 153). **RD1-07** (four clang-tidy diagnostics)
was explicitly deferred by the AA's own instruction - no speculative source
change was made for it.

This round applied **minimum-delta corrections only**: **4 of the 57
non-handover authorized paths were touched**
(`src/viewport/bgfx/CMakeLists.txt`,
`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`,
`tools/architecture_checker.cmake`, `Verification-RunbookD-v1.0.ps1`), plus
this document and its JSON companion - **6 of the 59 authorized paths
touched this round** (13 of 59 touched across all rounds to date; 46
remain byte-for-byte unmodified since the original candidate). **Zero
paths outside the authorized 59 were touched.** Full detail is in
**section 12** below and in this document's JSON companion under the
`aa_runbook_d_attempt_1_corrections` key. **ACR status for this round:
NONE** - no requested correction proved impossible under the locked
architecture. Per the user's explicit instruction, **Runbook D Attempt 1 is
recorded here as FAIL, and this round did not re-run or claim a PASS for
`Verification-RunbookD-v1.0.ps1`** - see section 0, still unchanged: this
session still has no execution channel to the Windows worktree (Attempt 1
itself was run by Architecture Authority, not by this session).

---

## AA RD1.1 final arch-checker correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12)

Architecture Authority accepted the RD1 six-file source review (RD1-01
through RD1-06) except for one residual, deterministic false positive: for
the first time, the AA actually EXECUTED the RD1-05-corrected checker
against an isolated copy of the candidate, and `BGFX_VIEWPORT_OWNER`
(R10) FAILED with exactly one false positive -
`src/desktop/src/evidence_mode.cpp`'s legitimate lowercase evidence-JSON
field key `"bgfx_version_frozen_baseline"`. Finding **RD1-05A**: RD1-05's
shared boundary-matching helper lowercased both file content and token,
which was correct for `Handle(` and raw `bgfx::` but wrong for the
`BGFX_` macro-prefix token, which is only ever meaningful in its real,
upper-case form. This round's correction is a single, narrowly-scoped fix:
a new exact-case variant of the boundary-matching helper, used only for
`BGFX_`; `Handle(` and `bgfx::` are completely untouched and remain
case-insensitive exactly as RD1-05 left them. **Exactly 1 of the 57
non-handover authorized paths was touched this round**:
`tools/architecture_checker.cmake`, plus this document and its JSON
companion - **3 of the 59 authorized paths touched this round** (the exact
3-path delta the AA's instruction asked for). **Zero paths outside the
authorized 59 were touched**, and
`src/desktop/src/evidence_mode.cpp` (the file whose legitimate content
triggered the false positive) was deliberately NOT modified - the checker's
own matching logic was the defect, not the source it scans. Full detail is
in **section 13** below and in this document's JSON companion under the
`aa_rd1_1_corrections` key. **ACR status for this round: NONE.** Per the
user's explicit instruction, **this round did not run or claim a PASS for
`Verification-RunbookD-v1.0.ps1`**, and Runbook D Attempt 1 remains recorded
as **FAIL** (unchanged from section 12) - this session still has no
execution channel to the Windows worktree.

**RD1 source review status as of this round: RD1-01/02/03/04/06
source-accepted; RD1-05A applied this round (closing out RD1-05's residual
false positive); RD1-07 (clang-tidy) still deferred.**

---

## AA RD1.2 shader varying definition correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13)

Architecture Authority executed the authorized **Targeted Verification**
after RD1.1 - the furthest any real run has reached so far. Seven gates
PASSED in sequence before the first failure: disk gate, the exact 59-path
Git authority gate, exact Windows toolchain bootstrap, the production
architecture checker, negative bgfx fixture rejection, RD1.1's
case-sensitive `BGFX_` synthetic regressions, and incremental CMake
configure. This also **runtime-confirms RD1-01's shaderc include wiring
fix** (Round "AA Runbook D Attempt 1") actually works: `shaderc` now
resolves `<bgfx_shader.sh>` through the installed vcpkg bgfx include
directory, and `src/viewport/bgfx/CMakeLists.txt` needed no further change
this round.

**Targeted Verification Attempt 1: FAIL**, at the next gate - fresh vertex
shader compilation - with `D3DCompile ... error X3004: undeclared
identifier 'a_position'`. Finding **RD1.2**: `varying.def.sc` (the actual
vertex attribute/varying declaration file bgfx's `shaderc` parses) still
carried comments/prose at the time of this run; bgfx `shaderc`'s documented
contract requires this file to be declaration-only, with no comments -
having them caused `shaderc` to silently fail to register `a_position` as
a real input, so the HLSL `shaderc` generated declared `a_normal` but never
`a_position`. Correction: `varying.def.sc` was rewritten to be
declaration-only - the same three declarations, same attribute/varying
names, same semantics (`POSITION`/`NORMAL`), same `v_normal` default value
- with every comment and blank line removed. **`vs_p0_t003.sc`,
`fs_p0_t003.sc`, the vertex layout, shaderc's flags/profile/platform, and
`src/viewport/bgfx/CMakeLists.txt` (already source-accepted, RD1-01) were
all deliberately left untouched.** No generated/precompiled shader binary
was introduced.

**Exactly 1 of the 57 non-handover authorized paths was touched this
round:** `src/viewport/bgfx/shaders/varying.def.sc`. Plus this document and
its JSON companion - **the exact 3-path delta the AA's instruction asked
for.** Zero paths outside the authorized 59 were touched. Full detail is in
**section 14** below and in this document's JSON companion under the
`aa_rd1_2_corrections` key. **ACR status for this round: NONE.** Per the
user's explicit instruction, **this round does not run Full Runbook D and
does not claim a shader/build PASS** - Targeted Verification Attempt 1 is
recorded as **FAIL** at fresh vertex shader compilation, after every prior
gate passed; this session still has no execution channel to the Windows
worktree (both Attempt 1's passing gates and its shader-compile failure
were run and reported by Architecture Authority, not by this session).

---

## AA RD1.3 MSVC/bgfx + nodiscard build conformance correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14)

Architecture Authority classified **Targeted Full Project Build v3** and
reported two confirmed project-level findings from the authoritative build,
reached after RD1.2's shader fix (independently verified and CLOSED - `vs_p0_t003.bin`/
`fs_p0_t003.bin` now compile, the prior `a_position` error is gone) let the
build proceed further than any prior attempt. **RD1.3-01**: compiling
`src/viewport/bgfx/src/renderer.cpp` fails through `bx/platform.h` with
MSVC fatal error C1189 ("you must set /Zc:preprocessor") - the effective
compile command had `/Zc:__cplusplus` but not `/Zc:preprocessor`.
Correction: `if(MSVC) target_compile_options(bim_viewport_bgfx PRIVATE
/Zc:preprocessor) endif()`, added immediately after
`target_compile_features(bim_viewport_bgfx ...)` in
`src/viewport/bgfx/CMakeLists.txt` - scoped to `bim_viewport_bgfx` only
(the sole bgfx/bx-owning target, per R10), not global. **RD1.3-02**: MSVC's
`/WX` promotes discarded-`[[nodiscard]]`-result warning C4834 to hard error
C2220 at `tests/unit/unit_viewport_ray.cpp` (lines 19-21) and
`tests/integration/integration_viewport_bgfx_headless.cpp` (lines 22-24) -
both files' `MakeConfiguredCamera()` helper called
`Camera::SetLookAt`/`SetPerspective`/`SetAspectRatio` (all `[[nodiscard]]
Status`) without consuming the result. Correction: each call is now
wrapped in `REQUIRE(...IsOk())` (already this exact pattern elsewhere in
both files) - a meaningful assertion that the helper's implicit
precondition (a validly-configured `Camera`) actually held, not a
cast-to-void or suppression.

Per explicit instruction, `src/viewport/bgfx/shaders/varying.def.sc`
(RD1.2, already independently verified and CLOSED) was **not** modified.
**Exactly 3 of the 57 non-handover authorized paths were touched this
round:** `src/viewport/bgfx/CMakeLists.txt`,
`tests/unit/unit_viewport_ray.cpp`,
`tests/integration/integration_viewport_bgfx_headless.cpp`. Plus this
document and its JSON companion - **5 of the 59 authorized paths touched
this round.** Zero paths outside the authorized 59 were touched, and no
additional path was found genuinely required (both authorized production/
test paths per finding were sufficient). Full detail is in **section 15**
below and in this document's JSON companion under the
`aa_rd1_3_corrections` key. **ACR status for this round: NONE.** No
staging, no `git add`, no commit; `main` remains untouched. Per the user's
explicit instruction, **this round does not run Full Runbook D and does
not claim an authoritative build/test PASS** - only non-authoritative,
engineer-reported static checks were performed (no local compiler/build
toolchain exists in this session - see section 15.4); Architecture
Authority will independently execute the authoritative verification.

---

## AA RD1.4 Renderer MSVC/build conformance correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14/15)

Architecture Authority classified **Targeted Full Project Build v5** and
reported three confirmed findings from the authoritative build, all in
`src/viewport/bgfx/src/renderer.cpp`, reached after RD1.3's two findings
were independently runtime-confirmed fixed (VS bootstrap + stdlib probe
PASSED; `/Zc:preprocessor` confirmed present in the effective compile
command, C1189 gone; `unit_viewport_ray.cpp`'s target compiled and linked) -
closed and explicitly not reopened this round. **RD1.4-01**: `std::fopen`
in `LoadCompiledShader()` (was line 46) triggers MSVC C4996, promoted by
`/WX` to C2220. Correction: an `#if defined(_MSC_VER)` branch now calls
`fopen_s(&file, path, "rb")`, leaving `file` null on any non-zero return so
the existing `file == nullptr` check is unchanged; the non-MSVC branch
keeps the exact original `std::fopen` call. **RD1.4-02**: `std::length_error`
is caught in two places (`Impl::ReleaseSlot`, was line 113; `CreateMesh`,
was line 347) but the file never directly included the standard header that
declares it. Correction: added `#include <stdexcept>` to renderer.cpp's own
include list, alongside the existing `<cstdio>`/`<cstring>`/`<new>`/
`<vector>` - no behavioral change, both catch clauses are otherwise
untouched. **RD1.4-03**: `bx::kRadToDeg` (was line 426) does not exist in
the installed bx API (C2039) - the real API exposes a conversion function,
`bx::toDeg(float)`, instead. Correction: `fovy_degrees` is now computed as
`bx::toDeg(static_cast<float>(camera.VerticalFovRadians()))` - the exact
same input value, the exact same resulting degrees value, no hard-coded
180/pi constant and no locally-defined `kRadToDeg` replacement.

Per explicit instruction, `src/viewport/bgfx/CMakeLists.txt`,
`tests/unit/unit_viewport_ray.cpp`,
`tests/integration/integration_viewport_bgfx_headless.cpp`, and
`src/viewport/bgfx/shaders/varying.def.sc` (all independently verified/
runtime-confirmed and CLOSED in RD1.2/RD1.3) were **not** modified.
**Exactly 1 of the 57 non-handover authorized paths was touched this
round:** `src/viewport/bgfx/src/renderer.cpp`. Plus this document and its
JSON companion - **3 of the 59 authorized paths touched this round** (the
full authorized RD1.4 delta). Zero paths outside the authorized 59 were
touched, and no additional path was found genuinely required. Full detail
is in **section 16** below and in this document's JSON companion under the
`aa_rd1_4_corrections` key. **ACR status for this round: NONE.** No
staging, no `git add`, no commit; `main` remains untouched. Per the user's
explicit instruction, this round does not run Full Runbook D and does not
claim an authoritative build/test PASS - only non-authoritative,
engineer-reported static checks were performed (no local compiler/build
toolchain exists in this session - see section 16.4); Architecture
Authority will independently execute the authoritative verification.

---

## AA RD1.5 exact clang-format 19.1.5 conformance correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14/15/16) - BLOCKED in Claude environment (historical, preserved below), subsequently OPERATOR_EXECUTED_PASS (see the "Update" paragraph at the end of this block and section 17.5)

Architecture Authority classified the authoritative clang-format gate and
reported a true format-conformance failure: `clang-format --dry-run --Werror
--style=file` on **clang-format version 19.1.5** checked 35 C/C++ candidate
files, 10 passed, exactly 25 failed. AA authorized correction of exactly
those 25 files (plus the handover pair) using **only** the exact output of
`clang-format 19.1.5 -i --style=file`, and explicitly instructed: "Do not
substitute another clang-format version... If clang-format 19.1.5 is NOT
available in your environment: STOP. Do not manually approximate hundreds
of formatting edits. Report that exact-tool execution is unavailable to
Architecture Authority."

This session's Linux environment was checked before touching any of the 25
files: only `clang-format` 18.1.3 (Ubuntu package `clang-format-18`,
`1:18.1.3-1ubuntu1`) is actually installed. `clang-format-19` exists only as
an *uninstalled* apt candidate at `1:19.1.1-1ubuntu1~24.04.2` - a different
patch version, not `19.1.5` - and versions 14/15/16/17/20 are likewise only
apt candidates, none matching. No `clang-format` package matching `19.1.5`
was found via `pip` or `npm`'s registries reachable from this environment,
and a direct fetch of LLVM's official `llvmorg-19.1.5` GitHub release (where
such a binary would normally be published) was blocked by this session's
network policy (HTTP 403 - GitHub release downloads are not on this
environment's allowlist) and was not retried through any workaround. **No
exact match for clang-format 19.1.5 is available in this environment.**

Per AA's own explicit instruction for exactly this case, this round **STOPS
before any file is touched**: zero of the 25 authorized C/C++ files were
opened, edited, or reformatted (no manual approximation was attempted, as
instructed); the 10 already-passing files were likewise untouched. **Only
the handover pair (2 of the 27 max authorized paths) is updated this
round**, to record this blocked disposition. Full detail is in **section
17** below and in this document's JSON companion under the
`aa_rd1_5_corrections` key. **ACR status for this round: NONE** (a tool-
availability blocker, not an architecture/design issue). No staging, no
`git add`, no commit; `main` remains untouched. This round did not run
clang-tidy, Full Runbook D, or involve any other reviewer, per instruction.
Architecture Authority's own environment - where clang-format 19.1.5 is
confirmed present - remains the correct place to either run the exact tool
directly, or to supply this session with that exact binary/package so a
future round can apply it.

**Update (handover-only round, after the blocked delivery above - the
blocked record above is preserved verbatim as historical evidence):**
Architecture Authority subsequently authorized the **Windows Execution
Operator** to perform the exact format correction on the authoritative
Windows operator environment, and the Operator has completed it. Per
Architecture Authority's report (values supplied by AA, not independently
verified by this session): the Operator's pre-format checks passed
(authority lock, branch/HEAD/tree match, staged paths = 0, `main` HEAD/tree
match and clean, RD1.4 authoritative manifest PASS, candidate boundary
exactly 59 paths, pre-format delta vs RD1.4 exactly the 2 handover paths,
all 25 format targets byte-identical to their RD1.4 authoritative hashes);
the Operator then ran the exact formatter
`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin\clang-format.exe`
(**clang-format version 19.1.5**) with `-i --style=file` on exactly the 25
previously authorized failing files - 25/25 completed; the authoritative
post-format `--dry-run --Werror --style=file` recheck is **25/25 PASS**;
the exact working delta vs RD1.4 is **27 paths** (the 25 formatted
source/test files + the handover pair), no additional paths; staged paths =
0; `main` unchanged + clean; task HEAD/tree unchanged (no commit/staging
occurred). **RD1.5 format execution: OPERATOR_EXECUTED_PASS.** This
session's own environment did **not** perform the format operation - that
remains true and is not being rewritten. **ACR = NONE.** This handover-only
round wrote zero source/test files (the 25 formatted files already hold
the Operator's authoritative output and were not opened, resent, or
normalized by this session). **Gate status: AA authoritative RD1.5 audit
PENDING; post-RD1.5 Targeted Full Project Build re-certification PENDING;
post-RD1.5 Architecture CTests re-certification PENDING; clang-tidy NOT
AUTHORIZED / PENDING; Full Runbook D Attempt 2 NOT AUTHORIZED; Kimi NOT
AUTHORIZED; implementation commit FORBIDDEN.** The previously recorded
Targeted Full Project Build v6 PASS and Architecture CTests 12/12 PASS
were achieved against the pre-format RD1.4 bytes and are explicitly NOT
post-RD1.5 certification. Full detail in section 17.5.

---

## AA RD1.6 clang-tidy source correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14/15/16/17) - DELIVERED_FOR_AA_VERIFICATION

Architecture Authority reported **clang-tidy v6 = TRUE SOURCE FAIL**
(clang-tidy 19.1.5; 17 exact translation units; 60 raw project diagnostic
lines; 19 unique source findings) after RD1.5 delta/source audit PASS,
clang-format 19.1.5 35/35 PASS, Targeted Full Build v7 PASS, and
Architecture CTests 12/12 PASS (all AA-supplied; authoritative RD1.5
snapshot `P0-T003-RD1-5-CORRECTION-20260912-154503`). AA authorized exactly
11 source paths plus the handover pair (13 max) and six finding classes:
**RD1.6-01** readability-braces-around-statements (6 sites: braces only);
**RD1.6-02** performance-enum-size (6 enums: narrow `NOLINT` + reason,
underlying representation NOT changed, per AA decision); **RD1.6-03**
cppcoreguidelines-special-member-functions (`MainWindow`, `ViewportWindow`:
copy/move ctor+assign `= delete`); **RD1.6-04** bugprone-exception-escape
(`main()` body extracted to file-local `static int RunDesktopMain(int,
char**)` byte-for-byte; `main()` is now the outer try/catch boundary with
a concise stderr diagnostic and non-zero return, following the accepted
P0-T002 evidence entry-point pattern); **RD1.6-05** performance-no-int-to-ptr
(2 exact Qt `winId()` -> `void*` interop casts: narrow `NOLINTNEXTLINE` +
reason, semantics unchanged); **RD1.6-06** bugprone-empty-catch (the two
empty handlers, which live in `Renderer::Impl::ReleaseSlot` - `void ...
noexcept`, no failure result to return - made explicit with `return;`,
current semantics preserved exactly; `CreateMesh()`'s handlers already
return `Fail(ResourceCreationFailed)` and are unchanged).
`clang-diagnostic-unused-command-line-argument` (`/Zc:preprocessor`) is
**HARNESS-ONLY per AA** - not source-fixed; `src/viewport/bgfx/CMakeLists.txt`
untouched.

**Before editing, all 11 authorized files were re-staged from the Windows
worktree** (this session's local copies were stale pre-RD1.5 bytes - see
17.5's standing warning); 10 of 11 differed from the stale copies exactly
as expected (`input.hpp`, one of RD1.5's 10 already-passing files, was
identical). Every edit was applied on top of the Operator's clang-format
19.1.5 output. **Exactly 13 of the 59 authorized paths were touched this
round**: the 11 authorized source files plus this document and its JSON
companion. Zero paths outside the 13 were written. Exact clang-format 19.1.5
is still unavailable in this environment: per instruction, clang-format
18.x was **not** run, formatting was **not** approximated, and no
already-formatted unrelated region was rewritten - every new line was kept
within the 100-column limit and each new suppression/brace/declaration is
the minimum text delta; **exact operator-side clang-format 19.1.5 remains
required after RD1.6**. **RD1.6 status: DELIVERED_FOR_AA_VERIFICATION. ACR =
NONE.** No staging, no `git add`, no commit; `main` untouched; no Full
Runbook D; no Kimi; no CMake/.clang-tidy/harness change. Build
re-certification PENDING; Architecture CTests re-certification PENDING;
clang-format re-certification PENDING after RD1.6; clang-tidy re-run
PENDING; Full Runbook D Attempt 2 NOT AUTHORIZED; commit FORBIDDEN. Full
detail in **section 18** and in the JSON companion's `aa_rd1_6_corrections`.

**Update (handover-only round, after the delivery above - the record above
is preserved verbatim):** Architecture Authority's **RD1.6 pre-format
authoritative delta audit v2 = PASS** (13 exact changed paths: 11
source/header + 2 handover; 46 untouched; 6 enum-size + 2 native-handle
NOLINTs; enum representations PRESERVED; Qt special members EXPLICITLY
DELETED; main exception boundary PASS; Qt/native-handle semantics
PRESERVED; renderer catch correction EXPLICIT RETURN; `/Zc:preprocessor`
PRESERVED; CMake/.clang-tidy UNTOUCHED; staged 0; `main` UNCHANGED +
CLEAN). AA then authorized the **Windows Execution Operator** to apply
exact clang-format 19.1.5 to the 11 RD1.6 source/header files only:
**OPERATOR_FORMAT_PASS** - 11 files formatted, post-format dry-run **11/11
PASS**, delta vs RD1.5 still 13 exact paths, handover pair byte-identical
during formatting, staged 0, `main` UNCHANGED + CLEAN (all AA-supplied; run
`RD1-6-FORMAT-20260912-164100-395fbbc6`). **This session did not execute
clang-format 19.1.5** (it never had the exact formatter) - the sequence is:
(1) Claude delivered the RD1.6 source correction; (2) AA pre-format audit
v2 passed; (3) the Operator applied exact clang-format 19.1.5; (4) the
Operator's post-format dry-run passed 11/11; (5) **the final AA RD1.6
authoritative audit is still PENDING.** Post-RD1.6 build re-certification
PENDING; Architecture CTests PENDING; clang-tidy re-run PENDING; Full
Runbook D Attempt 2 NOT AUTHORIZED; Kimi NOT AUTHORIZED; commit FORBIDDEN;
ACR = NONE. This handover-only round wrote zero source/header/test files.
Detail in section 18.6.

---

## AA RD1.7 Full Runbook D Attempt 2 failure correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14/15/16/17/18) - DELIVERED_FOR_AA_VERIFICATION

**Full Runbook D Attempt 2 = HISTORICAL FAIL, 60 PASS / 3 FAIL** (Attempt 1
= HISTORICAL FAIL too - section 12; neither is ever rewritten as PASS). It
was run by Architecture Authority after every pre-RD1.7 targeted gate had
passed (RD1.6 final audit + snapshot `P0-T003-RD1-6-CORRECTION-20260912-173736`;
clang-format 19.1.5 35/35; Targeted Full Build v8; Architecture CTests v3
12/12; clang-tidy v7 17/17 TUs, 0 project diagnostics - all AA-supplied).
The three failures and AA's classification: **Check 24 (clang-tidy) =
HARNESS/COMPDB ONLY, CLOSED** - exact clang-tidy 19.1.5 fails
`renderer.cpp` solely on `argument unused during compilation:
'/Zc:preprocessor'` (clang-cl has no use for the MSVC-only flag the
production build genuinely needs); removing exactly that one token from a
temporary analysis-only compdb, same source bytes, gives exit=0 and no
project diagnostic. **Checks 35/60 (viewport-spike CI, direct headless
evidence JSON) = two defects**: (B) *Qt platform plugin CI wiring* - the
evidence processes died before running (`Could not find the Qt platform
plugin "windows"`) because `qwindows.dll` sits under
`<BuildDir>\vcpkg_installed\x64-windows\Qt6\plugins\platforms\` but
`QT_PLUGIN_PATH`/`QT_QPA_PLATFORM_PLUGIN_PATH` were empty and nothing was
staged beside the exe; (C) *repeatability orchestration defect* - once the
Operator supplied process-local plugin paths, headless evidence ran and
every section PASSED except `repeatability` (cycles[0..19].passed=false):
`RunRepeatabilityCycles()` constructs a nested short-lived `Renderer` per
cycle while the primary headless renderer (headless) / the production
`ViewportWindow`'s renderer (live) already owns the single process-wide
bgfx runtime. Not a Renderer/API/architecture defect.

**RD1.7 corrections (authorized max 5 paths, exactly 5 touched):**
`src/desktop/src/evidence_mode.cpp` - headless: the in-process
repeatability loop now runs BEFORE the primary headless renderer is
initialized (no overlapping bgfx ownership; per-cycle
initialize/render/destroy/shutdown, the >= 20 default, and the JSON schema
`requested_cycle_count`/`cycles`/`all_cycles_passed` unchanged; emitted at
the same JSON position); live: no nested Renderer is ever constructed -
`repeatability.cycles` is recorded as NOT_AVAILABLE with a precise reason
plus `externally_covered_by: scripts/ci/viewport-spike.ps1`, and does not
touch `overall_passed`. `Verification-RunbookD-v1.0.ps1` - Check 24 now
analyses against a temporary, analysis-only copy of `compile_commands.json`
with the single `/Zc:preprocessor` token removed from `renderer.cpp`'s
command only (raw DB asserted to contain exactly one token, exactly one
carrying entry, that entry asserted to be `renderer.cpp`, exactly one
removal, zero left; raw DB hash asserted unchanged; fail-closed on any
mismatch); Checks 35/60 launch their child processes inside a
process-local Qt plugin environment derived from `$BuildDir`/`$Triplet`
(fail-closed on missing `qwindows.dll`; previous values restored).
`scripts/ci/viewport-spike.ps1` - resolves
`<BuildDir>\vcpkg_installed\x64-windows\Qt6\plugins` (fail-closed on
missing `platforms\qwindows.dll`), sets process-local `QT_PLUGIN_PATH`,
`QT_QPA_PLATFORM_PLUGIN_PATH`, `QT_QPA_PLATFORM=windows` for every
`bim_desktop_spike.exe` child, restores them afterward (normal and failure
paths). Plus the handover pair. **Not touched:** `renderer.cpp`,
`src/viewport/bgfx/CMakeLists.txt` (`/Zc:preprocessor` stays in the
production MSVC build), public viewport API, Renderer/ViewportWindow/
Camera/lifecycle/mesh-handle/bgfx ownership model, architecture checker,
`.clang-tidy`, CMake, global/user/system environment. **All five files
were fresh-staged from the worktree before editing** (every one was
byte-identical to this session's copy - the Operator's RD1.6 clang-format
run left `evidence_mode.cpp` unchanged). Exact clang-format 19.1.5 is
still unavailable here: 18.x was not run, nothing was approximated; the
`evidence_mode.cpp` delta is delivered unformatted-by-tool (all new lines
<= 100 columns) and **the Windows Execution Operator will apply exact
19.1.5 afterward**. **RD1.7 status: DELIVERED_FOR_AA_VERIFICATION. ACR =
NONE.** No staging, no commit, `main` untouched; Full Runbook D Attempt 3
NOT AUTHORIZED (not run); Kimi NOT AUTHORIZED; commit FORBIDDEN. No PASS is
claimed for anything this session did not execute. Detail: **section 19**
and the JSON companion's `aa_rd1_7_corrections`.

---

## AA RD1.8 minimum-delta correction round - summary (read this first if you already know sections 0/7/8/9/10/11/12/13/14/15/16/17/18/19) - DELIVERED_FOR_AA_VERIFICATION

**RD1.7 remains the frozen historical authoritative baseline**
(`P0-T003-RD1-7-CORRECTION-20260913-005610`, ZIP SHA-256
`73000641ce0dbc296550673fbd3be74c4003cb453f254de9e4ea3e3e7a444940`; task
HEAD `82367706691567b0148ee0e7b4eac3a3e30b33d1` / tree
`0f5845e02a5f48d64a56f467c26d5ac526bf61dc` - all AA-supplied). Targeted
post-RD1.7 results (AA-supplied): full build PASS; full CTest PASS;
clang-tidy 19.1.5 17/17 TUs, zero project diagnostics; direct headless
evidence PASS; in-process repeatability 20/20 PASS; external process-level
repeatability 20/20 PASS. Targeted viewport-spike remained blocked at the
final live evidence step. AA read-only diagnostics then proved two things:
**(RD1.8-01)** `LoadCompiledShader()` built the shader path as the relative
string `shaders/<backend>/<name>.bin`, i.e. relative to the process CWD -
a repo-root CWD found no shaders (renderer_initialize=false,
backend=Uninitialized) while a build-root or exe-directory CWD initialized
Direct3D 11 correctly (`CWD_DEPENDENT_RUNTIME_ASSET_DEFECT = PROVEN`); and
**(RD1.8-02)** once shaders resolved, every substantive live check passed
(renderer_initialize=true, backend "Direct3D 11", lifecycle Ready,
minimize Ready->Suspended, restore Suspended->Ready, DPR 1.25->1.0 with
renderer surviving, V01-V05, resize, surface recreation
Ready->SurfaceUnavailable->Ready, shutdown Ready->Destroyed, live
repeatability NOT_AVAILABLE/externally covered as designed) yet
`overall_passed` was forced false by one gate comparing
`backend_info.backendName == "Direct3D11"` against the actual canonical
bgfx name `"Direct3D 11"` (with a space).

**RD1.8 corrections (exactly the 4 authorized paths touched, zero
others):** `src/viewport/bgfx/src/renderer.cpp` - new private
`RunningExecutableDirectory()` (Win32 `GetModuleFileNameW(nullptr, ...)`,
wide-character, buffer grown until the full path fits, capped at 32768;
empty path = fail-closed; non-Windows deliberately returns empty - no CWD
fallback anywhere) and `LoadCompiledShader()` now builds
`<executable-dir>/shaders/<backend>/<name>.bin` as a
`std::filesystem::path` and opens it with `_wfopen_s` on the native
`wchar_t` path under MSVC (Unicode-correct), `std::fopen(path.string())`
elsewhere; fails closed if the executable directory or the file cannot be
resolved/opened. Headless/Noop never calls it (unchanged); shader names,
backend subdirectory, deployed layout, `RendererCreateInfo`, public API,
CMake, ownership/lifecycle all unchanged. `src/desktop/src/evidence_mode.cpp`
- the live backend gate is now `RecordResult(!renderer_ready ||
backend_info.backendName == "Direct3D 11");` with its comment updated to
the canonical spelling; gate not weakened, `homogeneous_depth` stays
observational, no other `RecordResult` touched, live repeatability exactly
as RD1.7. Plus the handover pair. All four files were fresh-staged from
the worktree before editing (every one byte-identical to this session's
RD1.7 delivery). **STOP-and-report item for AA (path NOT modified, out of
scope):** `scripts/ci/viewport-spike.ps1` line 261 applies the same
mismatched comparison (`$backendName -ne 'Direct3D11'`) to the live JSON
and will throw once the live run actually reports "Direct3D 11" - see
section 20.3. Exact clang-format 19.1.5 was unavailable here (18.x not
run, nothing approximated; Operator exact-format was required - since
completed, see the post-format sync paragraph below). **RD1.8
status: DELIVERED_FOR_AA_VERIFICATION. ACR = NONE.** No staging, no
commit, `main` untouched; Full Runbook D Attempt 1 and Attempt 2 remain
HISTORICAL FAIL; Attempt 3 NOT AUTHORIZED; Kimi NOT AUTHORIZED; commit
FORBIDDEN. Detail: **section 20** and the JSON companion's
`aa_rd1_8_corrections`.

**RD1.8 Amendment A1 (scope extension, after the delivery above - the
record above is preserved):** Architecture Authority accepted the reported
`scripts/ci/viewport-spike.ps1` backend-name mismatch as part of the same
RD1.8 backend-identity root cause and extended the total RD1.8 changed-path
set relative to frozen RD1.7 from 4 to **exactly 5** (adding
`scripts/ci/viewport-spike.ps1`). Under A1 only that script and the
handover pair were modified: the live backend comparison is now
`if ($liveSurfaceWasAvailable -and $backendName -ne 'Direct3D 11') {` and
its throw text names the authoritative `'Direct3D 11'` - only the
canonical spelling changed; a live surface with any other backend still
fails closed. Qt plugin handling, the >= 20 process-level cycles, the live
invocation, `overall_passed` validation, CTest lists, build behavior, CWD,
and everything else in the script are untouched; no shader-path workaround
or environment dependency was added (the CWD defect stays fixed in
`renderer.cpp`). `renderer.cpp` (28808 bytes,
`a1789435143a21b85f8b410d920208064fda757e849e75d29573940879b727bd`) and
`evidence_mode.cpp` (65880 bytes,
`082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`)
remain **byte-identical** to the initial RD1.8 delivery - re-staged from
the worktree and hash-verified under A1. `Verification-RunbookD-v1.0.ps1`,
CMake, tests, and architecture checks untouched. ACR = NONE; RD1.8 remains
DELIVERED_FOR_AA_VERIFICATION; Attempt 3 NOT AUTHORIZED; Kimi NOT
AUTHORIZED; commit FORBIDDEN; Attempts 1 and 2 remain HISTORICAL FAIL.
Detail: section 20.7.

**RD1.8 post-format handover-only sync (after A1; records above
preserved):** Architecture Authority's **RD1.8 Pre-Format Authoritative
Delta Audit v1 = PASS** (`PRE-FORMAT-v1-20260913-103326-2c726509`: 59
candidate paths, RD1.8 changed 5 exact / untouched 54, Claude delivery SHA
lock 5/5 MATCH, renderer RD1.8-01 semantic contract PASS, evidence backend
identity contract PASS, viewport-spike PowerShell parser PASS, viewport
backend identity contract PASS, handover JSON parse PASS, staged 0, `main`
unchanged + clean, ACR NONE). The **Windows Operator exact clang-format
19.1.5 run = PASS** (`RD1-8-FORMAT-20260913-103505-55b82011`): 2 targets
(`evidence_mode.cpp`, `renderer.cpp`), dry-run 2/2 PASS, **formatter
changed targets = 0** - both RD1.8 C++ targets were already exactly
compliant, pre/post SHA identical; 57/57 non-target candidate hashes
unchanged; staged 0; `main` UNCHANGED + CLEAN. **Exact clang-format 19.1.5
is therefore no longer pending for RD1.8.** The three RD1.8 implementation
files are byte-locked and were re-verified on the worktree under this
sync: `renderer.cpp` 28808 /
`a1789435143a21b85f8b410d920208064fda757e849e75d29573940879b727bd`;
`evidence_mode.cpp` 65880 /
`082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`;
`viewport-spike.ps1` 17393 /
`1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb`.
This sync modified only the handover pair. RD1.8 remains
**DELIVERED_FOR_AA_VERIFICATION** - NOT yet accepted or frozen; targeted
post-RD1.8 verification has NOT yet run; Attempt 3 NOT AUTHORIZED; Kimi
NOT AUTHORIZED; commit FORBIDDEN; Attempts 1 and 2 remain HISTORICAL FAIL;
ACR = NONE. Detail: section 20.8.

**RD1.8 Amendment A2 (exception-boundary minimum delta, after the
post-format sync; records above preserved):** Architecture Authority
reported **RD1.8 Targeted Post-Correction Verification v3** against the
frozen correction baseline `P0-T003-RD1-8-CORRECTION-20260913-105611` (ZIP
SHA-256 `b243419473c1b520760027d5cb372476507453918580ff2e1a276d9f46d34946`,
400631 bytes; NOT an accepted implementation release; Full Runbook D
Attempt 3 remains NOT AUTHORIZED): full build PASS; full CTest PASS;
clang-tidy 19.1.5 **16/17 TUs PASS**, exactly one TU failing -
`src/viewport/bgfx/src/renderer.cpp`, diagnostic
`renderer.cpp:282:33: error: an exception may be thrown in function
'Initialize' which should not throw exceptions
[bugprone-exception-escape,-warnings-as-errors]` on
`bim::viewport::Status Renderer::Initialize(const RendererCreateInfo&
info) noexcept`, reproduced independently by AA with the same sanitized
compile database. AA classified this as a **REAL SOURCE FINDING**, not a
harness failure. **Root cause (AA's diagnosis):** RD1.8-01's new
executable-relative shader-path resolution (`RunningExecutableDirectory()`,
`std::filesystem::path` construction/concatenation,
`std::wstring`/`std::string` allocation) can throw, and that call chain -
via `LoadCompiledShader()` - was reachable from `Renderer::Initialize()`
`noexcept` without being caught anywhere; before RD1.8 this TU passed
clang-tidy. **Amendment A2 correction (exactly the 3 authorized paths
touched, zero others):** `LoadCompiledShader()` in `renderer.cpp` is now
itself declared `noexcept` and its entire existing body - the
`RunningExecutableDirectory()` call, path construction/concatenation,
string allocation, the file open/read, and shader creation - runs inside a
`try { ... } catch (...) { return BGFX_INVALID_HANDLE; }` block, so any
exception is caught at this private implementation boundary and converted
to the function's own pre-existing fail-closed sentinel
(`BGFX_INVALID_HANDLE`), which `Renderer::Initialize()` already treats as
an ordinary shader-load failure via its existing
`!bgfx::isValid(vs) || !bgfx::isValid(fs)` check - no new failure
vocabulary, no weakened fail-closed behavior. The exception is genuinely
contained here, not merely declared away: nothing past the file-open call
can throw (the remaining operations are C-style `std::fseek`/`std::ftell`/
`std::fread`/`bgfx::alloc`/`bgfx::createShader`), so wrapping the whole
body introduces no resource-leak risk. `Renderer::Initialize()` itself is
untouched and remains `noexcept`; `RunningExecutableDirectory()`,
executable-relative shader resolution intent
(`<exe-dir>\shaders\dx11\<shader>.bin`), Windows Unicode-correct
`_wfopen_s` wide-path opening, headless/Noop never requiring D3D11 shader
files, and all existing bgfx ownership/lifecycle behavior are all
preserved exactly. No `SetCurrentDirectory`/`chdir`, no CWD fallback, no
environment-variable fallback, no hard-coded repo/build path, no public
API change, no NOLINT, no clang-tidy suppression, no warning-policy
change. `src/desktop/src/evidence_mode.cpp` (65880 bytes,
`082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`) and
`scripts/ci/viewport-spike.ps1` (17393 bytes,
`1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb`) remain
**byte-identical** to their existing locked hashes - re-staged from the
worktree and hash-verified before and after this amendment; neither was
modified. **This amendment is NOT described as accepted or verified** -
RD1.8 remains **DELIVERED_FOR_AA_VERIFICATION**; the frozen snapshot
`P0-T003-RD1-8-CORRECTION-20260913-105611` remains the A2 comparison
baseline; Full Runbook D Attempt 3 remains NOT AUTHORIZED; Kimi remains
NOT AUTHORIZED; commit remains FORBIDDEN; Full Runbook D Attempts 1 and 2
remain HISTORICAL FAIL; ACR = NONE. No build, CTest, clang-tidy,
viewport-spike, or Runbook D was run by this session - the v3 results
above are entirely AA-supplied and not independently executed here. No
staging; `main` untouched. Detail: section 20.9.

**RD1.8 Amendment A2 post-format handover-only sync (after A2; records
above preserved):** Architecture Authority's **RD1.8 A2 Pre-Format
Authoritative Delta Audit v2 = PASS**
(`PRE-FORMAT-v2-20260913-120348-569735e1`: 59 candidate paths, A2 changed 3
exact / untouched 56, delivery SHA lock 3/3 MATCH, `evidence_mode.cpp` lock
PASS, `viewport-spike.ps1` lock PASS, private renderer exception boundary
PASS, `Renderer::Initialize` remains `noexcept`, executable-relative
shader resolution preserved, no CWD/env fallback, handover JSON parse
PASS, staged 0, `main` unchanged + clean, ACR NONE). The **Windows
Operator exact clang-format 19.1.5 run = PASS**
(`A2-FORMAT-v1-20260913-120757-fc2d69b1`): 1 target (`renderer.cpp`),
dry-run 1/1 PASS, **formatter changed renderer = False** - the A2 delta was
already exactly compliant, pre/post SHA identical (31272 bytes,
`8ac2366140ef06294872f18947635fdbe0b827c82d2359ba151a1ee5a491ad4b`);
58/58 non-target candidate hashes unchanged; staged 0; `main` UNCHANGED +
CLEAN. `evidence_mode.cpp` (65880 bytes,
`082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`) and
`viewport-spike.ps1` (17393 bytes,
`1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb`) remain
byte-identical, re-staged and re-verified under this sync. This sync
modified only the handover pair. Amendment A2 remains **NOT** marked
accepted; RD1.8 remains **DELIVERED_FOR_AA_VERIFICATION**; A2 targeted
post-correction verification has NOT yet run; Attempt 3 NOT AUTHORIZED;
Kimi NOT AUTHORIZED; commit FORBIDDEN; Attempts 1 and 2 remain HISTORICAL
FAIL; ACR = NONE. Detail: section 20.10.

**RD1.8 Amendment A3 - Runbook D verifier correction (after A2; records
above preserved; supersedes the "Attempt 3 NOT AUTHORIZED" language above,
which was written before Attempt 3 occurred):** Full Runbook D **Attempt
3 = HISTORICAL FAIL**, against the frozen candidate
`P0-T003-RD1-8-A2-CORRECTION-20260913-121601` (410056 bytes, ZIP SHA-256
`b414b67100607143ed3aa546bef08a249361e8569fa527ffc8ab59314caee8ac`).
Attempt 3's authoritative launcher run
(`ATTEMPT3-20260913-125946-459d70c1`) exited 0 with an explicit global
verdict of PASS, yet its own final summary printed 62 PASS rows and 1 FAIL
row (Check 24) alongside `Total checks: 63 PASS: 63` and `VERIFICATION
RUNBOOK D = PASS` - an internally contradictory result. Architecture
Authority therefore classified Attempt 3 itself as FAIL and diagnosed
**two proven Runbook-only defects**, neither touching production
source/CMake: **(1)** Check 24's compile-database parsing -
`$rawEntries = @($rawText | ConvertFrom-Json)` - was not reliably
normalizing the parsed top-level JSON array into individual compile-entry
objects under Windows PowerShell 5.1, so the entry-level `/Zc:preprocessor`
filter observed zero entries even though the fresh raw compdb genuinely
has 42 entries with exactly one such token, owned by `renderer.cpp`
(independently reproduced: 11/11 first-party TUs exit 0, zero project
diagnostics, temporary analysis compdb correctly sanitized, raw compdb
hash unchanged) - classified
`RUNBOOK_CHECK24_HARNESS_OR_COMPDB_WIRING_DEFECT`; and **(2)** the final
tally/verdict block's `$failed = $script:Checks | Where-Object {...}`
collapses to a scalar (not an array) whenever exactly one check fails - a
documented PowerShell behavior - so `$failed.Count` silently read `$null`,
`$total - $null` arithmetic-coerced to the full total, and `$null -gt 0`
evaluated `$false`, letting a real single-check failure print as a full
PASS with exit code 0. **Amendment A3 correction (exactly the 1 authorized
source path touched, plus the handover pair, zero others):**
`Verification-RunbookD-v1.0.ps1` Check 24 now calls `ConvertFrom-Json`
directly (not piped) and explicitly rebuilds its result into a flat array
one item at a time regardless of the parsed root's shape, with a new
`> 0` fail-closed assertion before the existing one-entry/one-token/
renderer.cpp-owner assertions (all preserved, still fail-closed, still
never touching the raw compdb or renderer.cpp); and the final verdict
block now computes an explicit boolean pass/fail array for every check,
derives total/pass/fail counts and every printed row from that one array
via `@(...)`-forced materialization (never a bare `Where-Object` result),
and requires four invariants - total == 63, pass == total, fail == 0,
pass + fail == total - before printing a global PASS, failing closed
(non-zero exit) on any inconsistency. Neither correction touches
`renderer.cpp`, `evidence_mode.cpp`, `scripts/ci/viewport-spike.ps1`,
CMake, `.clang-tidy`, tests, or any other candidate path - all re-staged
and hash-verified byte-identical to their locked values before and after
this amendment. **Self-tested read-only before delivery** (no PowerShell
runtime in this sandbox, so as a faithful Python transliteration of both
corrected algorithms, clearly disclosed as such - not an execution of the
real `.ps1` file or toolchain): Check 24's normalization logic proved
correct against a synthetic 42-entry/1-token/renderer.cpp-owner compdb (6/6
assertions passed); the tally logic proved correct for a synthetic 63-PASS
set (total 63 / pass 63 / fail 0 / verdict PASS / exit 0) and a synthetic
62-PASS-plus-1-FAIL set (total 63 / pass 62 / fail 1 / verdict FAIL / exit
1) - both match AA's required contract exactly. Full Runbook D was NOT run
by this session; no full-Runbook execution is authorized during this
delivery. **Amendment A3 is NOT marked accepted or verified.** RD1.8
remains **DELIVERED_FOR_AA_VERIFICATION**; Attempts 1, 2, and 3 all remain
**HISTORICAL FAIL**; Full Runbook D Attempt 4 is **NOT AUTHORIZED**; Kimi
remains **NOT AUTHORIZED**; commit remains **FORBIDDEN**; ACR = **NONE**.
No staging; `main` untouched. Detail: section 20.11.

**RD1.8 Amendment A4 - Runbook Check 24 native-stderr correction (after
A3; records above preserved; supersedes the "Attempt 4 NOT AUTHORIZED"
implication of "Full Runbook D Attempt 4 is NOT AUTHORIZED" in the A3
paragraph above, which was written before Attempt 4 occurred):** Full
Runbook D **Attempt 4 = HISTORICAL FAIL**, against the frozen candidate
`P0-T003-RD1-8-A3-CORRECTION-20260913-170644` (411302 bytes, ZIP SHA-256
`842eb5e836387b82b8dbf9acfdc4b567e1d35ef68ce3f5203540d608a225106a`).
Attempt 4's authoritative run
(`ATTEMPT4-20260913-171538-1513edf4`) proved **the A3 tally/global-verdict
correction now works correctly**: 63 total checks, 62 PASS, 1 FAIL (Check
24), global verdict FAIL, process exit 1 - an internally *consistent*
result this time, unlike Attempt 3. The only failing check was Check 24
itself. A formally closed diagnostic
(`CHECK24-v1-20260913-172551-879d526c`, with `result.json` and
`completion-receipt.json`) classified the cause as
**RUNBOOK_CHECK24_NATIVE_STDERR_ERRORACTIONPREFERENCE_DEFECT**: the fresh
Attempt-4 raw `compile_commands.json` (SHA-256
`966b0197b81d3330851296865abb75d95caab2ee3fe426ae621bf61a5dd245fc`) still
has exactly 42 entries and exactly one `/Zc:preprocessor` token owned by
`renderer.cpp` - A3's normalization is correct on this fresh database, and
the temporary analysis copy still removes exactly that one token, raw
compdb unchanged - but with native stderr handled as non-terminating, the
exact Check 24 first-party set of 11 TUs is 11/11 exit 0 with zero project
diagnostics, while under a Runbook-like `$ErrorActionPreference = 'Stop'`
probe all 11 clang-tidy invocations *throw*, each on nothing but
clang-tidy's own routine stderr line (e.g. `evidence_mode.cpp: exception:
63246 warnings generated.`; `renderer.cpp: exception: 55996 warnings
generated.`) - PowerShell promoting ordinary native stderr into a
terminating exception, not a real diagnostic. Production source, CMake,
`.clang-tidy`, A3's compdb normalization, and A3's tally/verdict logic all
require no correction. **Amendment A4 correction (exactly the 1
authorized source path touched, plus the handover pair, zero others):**
Check 24's inline `& $clangTidy.Source ... 2>&1 | Out-Null` /
`$LASTEXITCODE` invocation is replaced with a call to this project's own
pre-existing `Invoke-Native` helper (`scripts/ci/_common.ps1`, already
dot-sourced by this script and already used for every other native call
in it - cmake, ctest, the PowerShell child scripts, evidence-mode - and
already hardened for exactly this class of defect: it scopes
`$ErrorActionPreference = 'Continue'` only around the native call,
restored via `finally`, captures stdout/stderr as text without discarding
either, and reads `$LASTEXITCODE` as the sole success/failure authority),
passing `-AllowFailure` so a single failing TU returns a result object
instead of throwing and aborting the loop - preserving the pre-A4 code's
exact per-file `$failed` accumulation shape and its exact
non-zero-exit-code failure semantics (this repo's `.clang-tidy`
`WarningsAsErrors` policy is still what turns a real diagnostic into a
non-zero clang-tidy exit; nothing new is parsed). The script-wide
`$ErrorActionPreference = 'Stop'` at the top of the file is untouched.
Neither this correction nor anything else in this amendment touches
`renderer.cpp`, `evidence_mode.cpp`, `scripts/ci/viewport-spike.ps1`,
CMake, `.clang-tidy`, tests, or any other candidate path - all re-staged
and hash-verified byte-identical to their locked values before and after.
**A3's compdb-normalization code and A3's final tally/verdict logic are
both byte-for-byte unchanged** (confirmed by diff against the pre-edit
worktree baseline). **Self-test disclosure:** this sandbox has no Windows
PowerShell runtime at all (only a Linux clang-tidy 18.1.3 is present,
neither the exact required 19.1.5 nor reachable by any `.ps1` execution
path), so per Architecture Authority's explicit instruction this session
did **not** substitute another runtime and call it authoritative - only a
**static self-audit** was performed (diff-scoped-to-exactly-one-block
review, brace/paren/bracket balance check, confirmation that
`Invoke-Native`'s own already-existing implementation already applies the
required `$ErrorActionPreference`-scoping/exit-code-authority pattern).
**Windows Operator validation of Check 24's actual runtime behavior
remains required** and has not been performed by this session. Full
Runbook D was NOT run. **Amendment A4 is NOT marked accepted or
verified.** RD1.8 remains **DELIVERED_FOR_AA_VERIFICATION**; Attempts 1,
2, 3, and 4 all remain **HISTORICAL FAIL**; Full Runbook D Attempt 5 is
**NOT AUTHORIZED**; Kimi remains **NOT AUTHORIZED**; commit remains
**FORBIDDEN**; ACR = **NONE**. No staging; `main` untouched. Detail:
section 20.12.

---

## 0. The single most important fact in this document

**Nothing in this footprint has been built, compiled, run, or tested.** This
session has no command-execution channel of any kind to the Windows task
worktree - confirmed via `ToolSearch` (no `device_bash`-equivalent shell
tool exists in this session's tool catalog) and via `get_device_info`
(before folder access was granted, the worktree path was not even a
connected folder). The only tools available to this session for this task
are file transfer (`SendUserFile` + `mcp__remote-devices__device_commit_files`),
directory listing (`device_list_dir`), file staging for reading
(`device_stage_files`), and folder-access requests
(`device_request_folder_access`).

Presented with this gap - after reading the Implementation Brief and
Implementation Authorization in full and confirming both assume a build/
test/git-execution capability this session does not have - Claude used
`AskUserQuestion` to ask how to proceed rather than silently fabricating
PASS results or silently refusing the task. **The user selected "Author
unverified source, Operator builds/verifies (Recommended)."** Everything
below is the result of that selection: all 59 authorized paths were
hand-authored as text, written to a local staging tree, and committed
directly to the worktree via `device_commit_files` - never compiled, never
linked, never run. This mirrors the pattern already established for
P0-T002 (Claude authors, the Operator executes and reports real results
back).

---

## 1. Authorized footprint

59 paths total: 12 MODIFY, 47 ADD, exactly as listed in
`docs/tasks/P0-T003/02-IMPLEMENTATION-BRIEF.md` and
`docs/tasks/P0-T003/03-IMPLEMENTATION-AUTHORIZATION.md`. No path outside
this list was touched. The full per-path manifest (disposition, byte size,
SHA256 as committed) is in `CLAUDE_HANDOVER.json`'s `footprint.modify` and
`footprint.add` arrays - not repeated in full here to avoid two
independently-maintained copies of the same 59-row table drifting apart.

At a phase level, the footprint breaks down as:

- **Phase D - neutral `bim::viewport` contract** (12 files): `error.hpp`,
  `math.hpp`, `mesh.hpp`/`.cpp`, `camera.hpp`/`.cpp`, `input.hpp`,
  `ray.hpp`/`.cpp`, `lifecycle.hpp`/`.cpp`, `src/viewport/CMakeLists.txt`.
  Depends on nothing first-party (not even `bim::foundation`) and nothing
  third-party beyond the standard library.
- **Phase E - `bim::viewport_bgfx` renderer adapter** (7 files):
  `src/viewport/bgfx/CMakeLists.txt`, `renderer.hpp` (the one public
  header, pimpl, zero bgfx types exposed), `renderer.cpp`,
  `renderer_impl.hpp` (private, holds real bgfx state), and the three
  shader source files (`varying.def.sc`, `vs_p0_t003.sc`, `fs_p0_t003.sc`).
- **Phase F - Qt Widgets desktop shell** (12 files):
  `src/desktop/CMakeLists.txt`, `main.cpp`, `main_window.hpp`/`.cpp`,
  `viewport_window.hpp`/`.cpp`, `viewport_bridge.hpp`/`.cpp`,
  `spike_scene.hpp`/`.cpp`, `evidence_mode.hpp`/`.cpp`.
- **Phase H - tests** (10 files): 4 unit tests, 2 integration tests, 4
  negative architecture fixtures, plus the `tests/unit/CMakeLists.txt`,
  `tests/integration/CMakeLists.txt`, and `tests/architecture/CMakeLists.txt`
  MODIFY edits wiring them all into CTest.
- **Phase I - architecture checker extension** (rules R8-R11 added to
  `tools/architecture_checker.cmake`, MODIFY; `tests/architecture/CMakeLists.txt`
  and `scripts/ci/architecture.ps1` MODIFY to register/require the 8 new
  CTest tests this adds).
- **Phase J - licenses**: two placeholder license files (`bgfx.LICENSE.txt`,
  `qtbase.LICENSE.txt`, both explicitly labeled NOT YET POPULATED - never
  fabricated text) plus MODIFY edits to `scripts/ci/license-inventory.ps1`,
  `LICENSES.md`, `third_party/licenses/README.md`.
- **Phase K/L** - `scripts/ci/viewport-spike.ps1` (new CI job) and
  `Verification-RunbookD-v1.0.ps1` (>= 25 independent numbered checks -
  actually 34; see section 4).
- **Phase M** - this document and its JSON companion.
- Two more root-level MODIFY edits (`CMakeLists.txt` adds the three new
  `add_subdirectory()` calls in dependency order; `vcpkg.json` adds
  `qtbase[widgets]` and `bgfx[tools]`, both `default-features: false` per
  the accepted Phase A facts) plus the `src/viewport/README.md` and
  `src/desktop/README.md` MODIFY edits replacing their P0-T001 boundary
  -placeholder text with real module descriptions.

---

## 2. Design decisions made or corrected during this authoring pass

- **Camera exposes no matrix.** `Camera` stores only semantic parameters
  (eye/target/worldUp/verticalFOV/aspect/near/far); all backend
  view/projection matrix construction (`bx::mtxLookAt`/`bx::mtxProj`) lives
  entirely inside `bim::viewport_bgfx::Renderer::RenderFrame`
  (`renderer.cpp`) per Implementation Brief section 5. This **corrects**
  Claude's own earlier Phase B design report, which had proposed
  `ViewMatrix()`/`ProjectionMatrix()` accessors directly on `Camera`.
- **Lifecycle has exactly 6 stable states**, with surface
  creation/recreation/destruction modeled as *events*
  (`BeginInitialization`/`OnSurfaceAvailable`/`OnSurfaceLost`/`OnSuspend`/
  `OnResume`/`BeginShutdown`/`CompleteShutdown`) rather than additional
  persistent states. This **corrects** Claude's own earlier Phase B
  design, which had a 7th state (`SurfaceRecreated`).
- **`RenderMeshHandle` epoch semantics**: an ordinary `Resize()` never
  invalidates a handle; only a full `Shutdown()`+`Initialize()` cycle
  increments the renderer's epoch and permanently invalidates every handle
  issued under the prior epoch. A stale-epoch, already-destroyed, or
  default-constructed handle always yields `ResourceNotFound` from
  `DestroyMesh` - never aliases whatever now occupies the same slot index.
- **`ViewportErrorCode`/`Status`/`Result<T>` carry a code only**, no
  diagnostic string - deliberately different from the existing
  `bim::foundation::Status` (which has a `std::string message_`), matching
  this module's own contract rather than reusing the foundation type.
- **R8-R11 (the four new architecture-checker rules) all reuse
  `bim_scan_files_for_tokens_ignoring_comments()`** - the comment-aware
  helper `tools/architecture_checker.cmake` already added in its "round 7"
  revision specifically to fix a real false-positive defect where R6/R7
  were flagging forbidden tokens mentioned only inside explanatory
  comments. R8-R11 deliberately avoid repeating that same defect class
  against their own explanatory comments (including the comments in this
  very handover's companion files).
- **R9 (`QT_DESKTOP_ONLY`) and R10 (`BGFX_VIEWPORT_OWNER`) both replicate
  R7's ownership-partition pattern** (partition all first-party files by
  whether they live under the rule's owner directory, scan only the
  outside-owner subset) rather than inventing a new mechanism.
- **R11 (`NO_DIRECT_D3D`) deliberately has no exception directory** - not
  even `bim_viewport_bgfx` itself may reference D3D11/DXGI tokens
  directly; D3D11 is reached exclusively through bgfx's own abstraction.
  R11 also forbids `dxgi.h`/`IDXGI` tokens, a disclosed addition stricter
  than a bare "no direct D3D11" reading of the Brief.
- **License placeholders, never fabricated text.** `third_party/licenses/qtbase.LICENSE.txt`
  and `bgfx.LICENSE.txt` are explicitly labeled
  "PLACEHOLDER - NOT YET POPULATED" and explain exactly how
  `scripts/ci/license-inventory.ps1`'s existing "remove previously
  generated outputs" step will delete and regenerate them from the
  Operator's real `vcpkg_installed` tree - following the same
  never-fabricate-license-text policy already established for the five
  P0-T001 dependencies.

---

## 3. Risk areas the Operator's real build will resolve

These are the specific points where an actual CMake configure/build/run
could disagree with what was authored here. Each is also recorded in
`CLAUDE_HANDOVER.json`'s `risk_areas` array with an id (`RISK-01`
through `RISK-08`).

1. **vcpkg CMake target names for qtbase/bgfx are assumed, not confirmed**
   (`Qt6::Widgets`, `bgfx::bgfx`) - the standard upstream shapes, but never
   checked against the actual installed `*Config.cmake` files.
   `src/viewport/bgfx/CMakeLists.txt` includes an explicit
   `if(NOT TARGET bgfx::bgfx) message(FATAL_ERROR ...)` guard that raises a
   loud, ACR-worthy error rather than silently guessing at a replacement
   name if this assumption is wrong.
2. **shaderc executable name/flags** (`-f`/`-o`/`--varyingdef`/`--type`/
   `--platform windows`/`--profile s_5_0`) are remembered from bgfx's
   documentation, not confirmed against the installed shaderc binary's
   actual `--help` output.
3. **bgfx/bx C++ API surface in `renderer.cpp`/`renderer_impl.hpp`** -
   `bgfx::init`, `bgfx::createVertexBuffer`, `bx::mtxProj`, and related
   calls are written from documented/remembered API shape for bgfx
   1.129.8940-496#1, not confirmed against the installed headers.
   `bx::mtxProj`'s parameter order/homogeneous-depth argument in
   particular is a historically version-sensitive shape.
4. **Qt Widgets API surface in the desktop shell** - `winId()`,
   `devicePixelRatioF()`, `QMouseEvent::position()`,
   `QWheelEvent::angleDelta()` and related calls, written against Qt6's
   documented API, not confirmed against qtbase 6.11.1#1's installed
   headers.
5. **`spike_scene.cpp`'s V01-V05 scenes are now restored against
   Implementation Brief section 14** (AA Source Review Round 1 correction
   M02 - see section 7). The prior authoring pass's comments cited "section
   13," which is actually "Shaders," not "Spike scenes"; this correction
   round re-read the Brief in full and rebuilt the scene corpus directly
   against section 14's literal text. What remains Claude's own engineering
   judgment, disclosed in `spike_scene.hpp`'s header comment, is the exact
   numeric parameters (box counts, spacing, coordinate magnitudes) within
   that literal description - the Brief describes the scenes qualitatively,
   not as exact literals. The Operator should still diff scene behavior
   against section 14 directly.
6. **`evidence_mode` still cannot produce a visual screenshot in headless
   mode** - bgfx's headless/noop backend cannot rasterize to a readable
   framebuffer, so `--evidence-mode` still runs a functional smoke pass and
   reports PASS/FAIL per check. AA Source Review Round 1 correction M05
   (section 7) adds a genuinely live path, `--evidence-mode-live <path>`:
   it constructs a real `QWindow`, bounds the wait for exposure to 3000ms,
   and populates real backend/adapter-identity fields when a window surface
   is actually available - honestly recording `nativeWindowHandle=nullptr`
   and `NOT_AVAILABLE` fields rather than fabricating a pass if the window
   is never exposed. This path has never been run by this session; its
   first real execution is the Operator's.
7. **No Qt runtime deployment step** (no `windeployqt` `POST_BUILD`) was
   added to `src/desktop/CMakeLists.txt` - the assumption is that the
   Brief's accepted `VISUAL_STUDIO_BUNDLED_VCPKG` build mode already makes
   Qt DLLs discoverable. If the real build cannot locate them at run time,
   this is a real gap to close.
8. **Button-mapping convention** in `viewport_bridge.hpp`/`.cpp`
   (left-drag=Orbit, middle-drag=Pan, wheel=Dolly) is an implementation
   decision beyond what the Brief dictates verbatim, disclosed in that
   file's own header comment.

---

## 4. Verification status

Every row below is honestly `NOT_AVAILABLE` - this session performed no
compilation, no linking, no execution of any kind against this footprint.

| Verification category | Status |
|---|---|
| CMake configure | NOT_AVAILABLE |
| Full build | NOT_AVAILABLE |
| Unit tests (4 new, CTest) | NOT_AVAILABLE |
| Integration tests (2 new, CTest) | NOT_AVAILABLE |
| Architecture-checker tests (8 new/extended, CTest) | NOT_AVAILABLE |
| Static analysis | NOT_AVAILABLE |
| `scripts/ci/license-inventory.ps1` job | NOT_AVAILABLE |
| Headless evidence-mode run | NOT_AVAILABLE |
| Live windowed spike run | NOT_AVAILABLE |
| shaderc shader compilation | NOT_AVAILABLE |
| `Verification-RunbookD-v1.0.ps1` run (34 checks authored, 0 executed) | NOT_AVAILABLE |
| `git status`/`git diff` review against the 59-path footprint | NOT_AVAILABLE (no git-execution channel this session) |

What **was** done as self-verification, honestly described: every one of
the 59 committed files was read back via `device_list_dir` size comparison
against its locally-staged copy immediately after each `device_commit_files`
batch (byte-for-byte match confirmed, not content re-inspection); every new
CMake target name, CTest test name, and architecture-checker rule name used
across the footprint was cross-checked for internal consistency against
every other file that references it (e.g. every test name in
`Verification-RunbookD-v1.0.ps1` and `scripts/ci/viewport-spike.ps1` has a
matching `add_test()` in `tests/*/CMakeLists.txt`); the existing repository
conventions this footprint had to match (`src/geometry/occt/CMakeLists.txt`,
`tests/*/CMakeLists.txt`, `scripts/ci/*.ps1`, and the exact current
`tools/architecture_checker.cmake` R1-R7 implementation) were re-read
directly from the worktree before authoring each corresponding new file,
not reconstructed from memory of the pattern.

`Verification-RunbookD-v1.0.ps1` was authored to run **34** independent
numbered checks (Implementation Authorization required >= 25), spanning
repository/footprint sanity, vcpkg dependency resolution, CMake configure,
full build, target/artifact existence, all 14 new CTest tests as
exact-name-anchored invocations, the headless evidence-mode smoke pass, the
license-inventory job, the full `architecture.ps1` job, and a `git`
-based footprint-discipline spot check. It has never been run.

---

## 5. Operator next steps

1. Review `git status`/`git diff` in the worktree against the authorized
   59-path footprint (`02-IMPLEMENTATION-BRIEF.md` /
   `03-IMPLEMENTATION-AUTHORIZATION.md`) - confirm no out-of-scope path
   changed.
2. Run a real CMake configure (the accepted `VISUAL_STUDIO_BUNDLED_VCPKG`
   toolchain mode); resolve risk items 1-2 above if they surface as
   configure/build errors.
3. Build the full tree; resolve any risk items 3-4 compile errors.
4. Run `powershell -File Verification-RunbookD-v1.0.ps1` (or the narrower
   `scripts\ci\viewport-spike.ps1`) and capture its real PASS/FAIL output -
   this is the first real execution of every check section 4 currently
   reports as `NOT_AVAILABLE`.
5. Run `scripts\ci\license-inventory.ps1`; confirm it replaces the two
   disclosed placeholder license files with the real captured copyright
   text, and correct `LICENSES.md`'s SPDX cells if they disagree.
6. Diff `spike_scene.cpp`'s V01-V05 scenes against Implementation Brief
   section 13 (risk item 5) and adjust if they do not match.
7. Decide, and if needed implement, the "live evidence" requirement (risk
   item 6) beyond `evidence_mode.cpp`'s functional smoke report.
8. Only after steps 1-7 produce real PASS results should this task be
   represented to the Architecture Authority as verified - this
   document's own verification status (section 4) must not be quoted as
   evidence that anything has actually passed.

---

## 6. What was NOT done

- No file outside the authorized 59-path footprint was created, modified,
  or staged for commit.
- `git add`/`git commit`/`git merge`/`git rebase`/`git push`/`git reset`/
  `git clean`/`git stash`/branch-switch were never invoked - this session
  has no git-execution channel at all, so this is a structural fact, not
  a discipline choice.
- `main` was never touched.
- No dependency was installed, no vcpkg command was run.
- No PASS result, no SHA/hash claimed to come from a real build, and no
  test-run count anywhere in this document or its JSON companion was
  fabricated - every verification-category field is honestly
  `NOT_AVAILABLE`.
- This correction round did not compile, link, or run anything either -
  the regression test added under M03 has never been executed.

---

## 7. AA Source Review Round 1 correction round

### 7.0 Provenance disclosure

**No file matching "AA Source Review Round 1" (or any actual review report
beyond the generic P0-T001-era placeholder at `docs/reviews/README.md`)
exists anywhere under `docs/` in this worktree** - confirmed by direct
listing, not assumed. Finding **B01**'s exact 35-file count and every
**M01-M07** description therefore came entirely from the user's own
correction-round chat message, not from a document Claude read. Where the
user's finding was independently checkable, Claude verified it directly
against the worktree rather than taking it on faith: an independent
`grep`-based scan of the pre-correction tree found **exactly 35** files with
trailing `</content>`/`</invoke>` contamination, matching the user's claim
precisely. The three **MINOR** findings (MINOR-1/2/3) were **Claude's own
determination** via code review during this pass, not sourced from any
document - flagged explicitly below for Architecture Authority confirmation
rather than presented as independently-verified review findings.

### 7.1 Finding B01 - trailing tag contamination

35 files ended with a literal `</content>\n</invoke>\n` byte sequence - an
artifact of a prior authoring pass's own tool-call transcript leaking into
file content. Each file's exact tail was verified to match that sequence
before stripping, and re-verified afterward to contain no residual
`</content>`/`</invoke>` substring anywhere in the file. A repository-wide
scan after all corrections (`grep -rlE '</?(content|invoke|function_calls|parameter|antml:[a-zA-Z_]+)>'`
across the full 59-file tree) returned zero matches. The 35 affected files
are listed with a `B01` tag in the per-file finding tags in section 7.3 and
in the JSON companion's `footprint.modify`/`footprint.add` entries
(`aa_source_review_round_1_findings_addressed`).

### 7.2 Findings M01-M07 and MINOR-1/2/3 - what changed and why

| Finding | What it required | Resolution |
|---|---|---|
| M01 | Locked private `QWindow`-derived viewport surface, Qt surface-lifecycle/DPR handling | `viewport_window.cpp` now defines `bim::desktop::ViewportSurface : public QWindow` (private, defined only in the `.cpp`), embedded via `QWidget::createWindowContainer()`. Lifecycle events (exposure, `SurfaceAboutToBeDestroyed`, resize, DPR change, screen change, visibility) wired via `std::function` callbacks + `QObject::connect`. The prior `Qt::WA_PaintOnScreen`/`paintEngine()==nullptr`/raw-`winId()` pattern is removed. |
| M02 | Restore the exact V01-V05 scene corpus from the Brief | Re-read the Brief in full; the scene requirement is section **14** ("Spike scenes"), not section 13 ("Shaders") as previously miscited. `spike_scene.hpp`/`.cpp` rewritten against section 14's literal text; `main_window.cpp`'s scene table and `main_window.hpp`'s citation updated to match. |
| M03 | Per-slot generation on `RenderMeshHandle`; regression test | `MeshSlot::generation` added, bumped by `AllocateSlot()` on every allocation (fresh or reused). `RenderMeshHandle` now carries `(epoch_, slot_, generation_, valid_)`; `CreateMesh`/`DestroyMesh`/`RenderFrame` all check it. `RenderFrame`'s validation loop now runs unconditionally (headless included) so the new regression test can exercise it without a live backend. New `TEST_CASE` added to `integration_viewport_bgfx_resource_lifecycle.cpp` covering same-epoch slot reuse. |
| M04 | Preserve/re-upload the initially selected scene after renderer init | `SetActiveSceneMeshes()` unconditionally remembers `pending_scene_meshes_`; `ApplyPendingSceneMeshesIfReady()`, called from `EnsureRendererInitialized()`, re-applies them once the renderer goes live - fixing the original silent-drop bug. |
| M05 | Machine-readable evidence JSON; >=20 process-level repeatability cycles; live D3D11 evidence path | `evidence_mode.cpp` rewritten around a dependency-free hand-rolled `JsonValue` tree (no JSON library is authorized by the frozen vcpkg manifest) covering every Brief section 22 field, using `NOT_AVAILABLE` rather than inventing a PASS. Repeatability implemented at two levels: an in-process >=20-cycle loop in `evidence_mode.cpp`, plus `scripts/ci/viewport-spike.ps1` launching 20 separate OS-process invocations of `--evidence-mode` (genuine process-boundary coverage). `main.cpp` gained `--evidence-mode-live <path>` (real `QWindow`, bounded exposure wait, honest `nullptr`/`NOT_AVAILABLE` on no-exposure). `Renderer` gained a neutral `RendererBackendInfo`/`BackendInfo()` (justified by Brief section 12) so evidence can report backend identity without a bgfx type leaking from the public header. |
| M06 | Bring `Verification-RunbookD-v1.0` fully into conformance with Brief section 24 | Rewritten (~450 lines) around a Brief-section-24-numbered check-group structure, embedding the literal 59-path footprint and exact P0-T001/P0-T002/P0-T003 CTest name lists directly (no more "compare by hand"). Added mandatory `-ExpectedPreVerificationHead` (no default, per Brief item 2). Fixed three PowerShell-7-only `?.` usages (script runs under Windows PowerShell 5.1 via `powershell -File`, not `pwsh`). Removed a dead/broken `$missingItems` computation. |
| M07 | Explicitly use right-handed `bx::mtxLookAt` | `renderer.cpp` now calls `bx::mtxLookAt(view, bx_eye, bx_target, bx_up, bx::Handedness::Right)` explicitly rather than relying on the default handedness argument. |
| MINOR-1 (self-determined) | Stale "Brief section 13" citation on `renderer_impl.hpp`'s `kViewId` comment | Reworded to disclose the single-view-id choice is this pass's own design decision, not a Brief mandate from any section. |
| MINOR-2 (self-determined) | `ViewportBridge::PickRay` defined but never invoked interactively | `viewport_window.cpp`'s `eventFilter` now detects a left-button click (press+release, negligible movement) and calls `bridge_.PickRay(...)`, logging the result via `qDebug()`. |
| MINOR-3 (self-determined) | Wrong "section 24" citations for architecture rules R8-R11 | `tests/architecture/CMakeLists.txt` and `scripts/ci/architecture.ps1` corrected to cite section **20** ("Architecture enforcement"); section 24 is actually "Verification Runbook D." |

### 7.3 Exact changed-path summary

**37 of the 59 authorized paths were touched this round** (grouped by the
finding(s) each addresses; every file below also received the B01
tag-contamination strip unless marked "no B01"):

- **M01 + MINOR-2** (+B01): `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp` (also M04)
- **M02** (+B01): `src/desktop/src/spike_scene.hpp`, `src/desktop/src/spike_scene.cpp`, `src/desktop/src/main_window.cpp`, `src/desktop/src/main_window.hpp`
- **M03**: `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp` (+B01, +M05), `src/viewport/bgfx/src/renderer_impl.hpp` (+B01, +MINOR-1), `src/viewport/bgfx/src/renderer.cpp` (+B01, +M05, +M07), `tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp` (+B01)
- **M05**: `src/desktop/src/evidence_mode.hpp`, `src/desktop/src/evidence_mode.cpp`, `src/desktop/src/main.cpp`, `scripts/ci/viewport-spike.ps1` (all +B01; `renderer.hpp`/`renderer.cpp` already listed under M03)
- **M06** (+B01): `Verification-RunbookD-v1.0.ps1`
- **MINOR-3** (no B01 - these two files were not among the 35 contaminated): `tests/architecture/CMakeLists.txt`, `scripts/ci/architecture.ps1`
- **B01 only** (no functional/architectural finding beyond the tag strip): `src/desktop/CMakeLists.txt`, `src/desktop/src/viewport_bridge.cpp`, `src/desktop/src/viewport_bridge.hpp`, `src/viewport/CMakeLists.txt`, `src/viewport/bgfx/CMakeLists.txt`, `src/viewport/bgfx/shaders/fs_p0_t003.sc`, `src/viewport/bgfx/shaders/varying.def.sc`, `src/viewport/bgfx/shaders/vs_p0_t003.sc`, `src/viewport/include/bim/viewport/lifecycle.hpp`, `src/viewport/src/lifecycle.cpp`, `src/viewport/src/ray.cpp`, `tests/fixtures/p0_t003_bad_bgfx_owner/desktop/src/leaky_bgfx.cpp`, `tests/fixtures/p0_t003_bad_direct_d3d/desktop/src/leaky_d3d.cpp`, `tests/fixtures/p0_t003_bad_qt_owner/model/src/leaky_qt.cpp`, `tests/fixtures/p0_t003_bad_viewport_api/viewport/include/bim/viewport/leaky.hpp`, `tests/integration/integration_viewport_bgfx_headless.cpp`, `tests/unit/unit_viewport_camera.cpp`, `tests/unit/unit_viewport_lifecycle.cpp`, `tests/unit/unit_viewport_mesh_contract.cpp`, `tests/unit/unit_viewport_ray.cpp`
- **This document and its JSON companion** (updated - see section 9's Round 3 correction of this exact wording: these two files ARE counted as part of the 59-path footprint, as its own items 58-59, not separately from it - the sentence that previously stood here was wrong): `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`

**20 of the 59 authorized paths were left completely untouched** (no finding
implicated them; byte-for-byte identical to the pre-correction candidate):
`CMakeLists.txt`, `LICENSES.md`, `scripts/ci/license-inventory.ps1`,
`src/desktop/README.md`, `src/viewport/README.md`,
`src/viewport/include/bim/viewport/camera.hpp`,
`src/viewport/include/bim/viewport/error.hpp`,
`src/viewport/include/bim/viewport/input.hpp`,
`src/viewport/include/bim/viewport/math.hpp`,
`src/viewport/include/bim/viewport/mesh.hpp`,
`src/viewport/include/bim/viewport/ray.hpp`, `src/viewport/src/camera.cpp`,
`src/viewport/src/mesh.cpp`, `tests/integration/CMakeLists.txt`,
`tests/unit/CMakeLists.txt`, `third_party/licenses/README.md`,
`third_party/licenses/bgfx.LICENSE.txt`,
`third_party/licenses/qtbase.LICENSE.txt`,
`tools/architecture_checker.cmake`, `vcpkg.json`.

**Zero paths outside the authorized 59-path footprint were touched or
created.** Per-file updated byte size and SHA256 for all 37 touched files
are in the JSON companion's `footprint.modify`/`footprint.add` arrays
(`aa_source_review_round_1_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_source_review_round_1_corrections.findings_addressed`.

### 7.4 What this round did NOT do

- Did not regenerate or rewrite any of the 20 untouched files.
- Did not compile, link, or run anything - same structural constraint as
  section 0 (no execution channel to the Windows worktree this session).
- Did not raise an ACR - no requested correction proved impossible under
  the locked architecture.
- Did not stage or commit to git - no git-execution channel exists this
  session, same as the original authoring pass.
- Did not touch `main`.

---

## 8. AA Source Review Round 2 correction round

### 8.0 Provenance and scope disclosure

This round's findings (**B02**, **M08-M12**, and closure of Round 1's three
self-determined **MINOR** items) came directly from the user's own Round 2
chat instruction, quoted in full at the start of that instruction - not from
any document Claude read on disk. As in Round 1, where a finding rested on a
checkable fact, Claude verified it directly against the worktree rather than
taking it on faith: `vcpkg-configuration.json`'s exact shape (`{"default-registry":
{"kind": "builtin", "baseline": "f89a4a1da4e3176a8d1a14c1825b9b2f98e48843"},
"registries": []}`) was confirmed by staging and reading both `vcpkg.json`
and `vcpkg-configuration.json` directly from the task worktree (B02); the
historically validated Ninja path was located by re-reading
`03-IMPLEMENTATION-AUTHORIZATION.md` section 4 directly rather than guessed
(B02); and the exact CMake preset name/`binaryDir` that
`Verification-RunbookD-v1.0.ps1`'s item 16 fix now drives was confirmed by
staging and reading `CMakePresets.json` directly from the worktree (M12) -
this file is a read-only reference, like `vcpkg-configuration.json`, not
part of the 59-path footprint and never edited.

**Explicit constraints from the user's instruction, honored throughout:**
stay strictly within the authorized 59-path footprint; no staging, no git
commit, no `main` mutation; ACR remains NONE unless a correction genuinely
requires an architecture change (none did); update this document and its
JSON companion with the exact delta; **do not run or claim
`Verification-RunbookD-v1.0.ps1` PASS.** All five are honored - see 8.4 for
the exact changed-path summary and 8.5 for what this round explicitly did
not do.

### 8.1 Findings B02 and M08-M12 - what changed and why

| Finding | What it required | Resolution |
|---|---|---|
| B02 | Fix `Verification-RunbookD-v1.0.ps1`'s vcpkg-baseline check (reads a nonexistent field); resolve Ninja off PATH | Item 10 rewritten to read `vcpkg-configuration.json`'s `default-registry.baseline` (confirmed shape above) instead of `vcpkg.json`'s nonexistent `builtin-baseline` - Round 1's version would always have thrown `'vcpkg.json has no builtin-baseline field'` and never actually verified the real baseline; a defensive check now also fails loudly if `vcpkg.json` ever does gain a `builtin-baseline` field, rather than silently preferring either value. Item 8 (Ninja version) now resolves via a new `Resolve-NinjaExecutable` helper: PATH first, then the historically validated path from Implementation Authorization section 4 (new `-HistoricalNinjaPath` param), then an existing build directory's `CMakeCache.txt` `CMAKE_MAKE_PROGRAM`. |
| M08 | Route mouse/wheel input authoritatively from the embedded `ViewportSurface : QWindow`, not `surface_container_`'s `eventFilter` | `ViewportSurface` now overrides `mousePressEvent`/`mouseMoveEvent`/`mouseReleaseEvent`/`wheelEvent` directly, forwarding via `std::function` callbacks to four new `ViewportWindow` methods (`HandleMousePress`/`HandleMouseMove`/`HandleMouseRelease`/`HandleWheel`). The prior `eventFilter` installed on `surface_container_` - which rested on an explicitly-disclosed, never-confirmed assumption about whether the container widget would also see events delivered to its embedded native child window - is removed entirely; a real native child window created via `QWidget::createWindowContainer()` receives its own input events directly, which is Qt's documented/guaranteed delivery path. The click-vs-drag pick-ray heuristic (Round 1 MINOR-2) now lives in `HandleMouseRelease`. |
| M09 | Correct V04's triangle winding (CCW from +Z, agreeing with its +Z normals); add regression coverage | Verified the defect via explicit 2D cross-product sign computation for the quad corner layout `i0=(0,0)`/`i1=(1,0)`/`i2=(0,1)`/`i3=(1,1)`: the correct CCW-from-+Z orderings are `(i0,i1,i2)` and `(i1,i3,i2)`, not the `(i0,i2,i1)`/`(i1,i2,i3)` Round 1 shipped (CW as seen from +Z, contradicting both its own `{0,0,1}` normals and `AppendBox`'s documented "CCW when viewed from outside" convention). `spike_scene.cpp`'s `MakeV04RepresentativeMediumMesh()` corrected accordingly. A new self-contained `TEST_CASE` was added to `tests/unit/unit_viewport_mesh_contract.cpp` - which deliberately links only `bim::viewport` (no bgfx/Qt/D3D) - mirroring V04's exact quad-triangulation algorithm generically rather than widening that target's scope to also link `spike_scene.cpp`; it asserts every produced triangle is CCW as seen from +Z. Disclosed tradeoff: this duplicates rather than calls the production triangulation, so it pins the winding convention itself, not `spike_scene.cpp`'s literal source line-for-line. |
| M10 | Make V05 an actual observable high-coordinate experiment via an appropriate camera eye/target, precision contract unchanged | `SpikeScene` gained `cameraEye`/`cameraTarget`/`cameraWorldUp` fields (defaulting to the existing near-origin viewpoint for V01-V04 - unchanged behavior). V05's `kBase` constant moved from a local inside `MakeV05LargeCoordinateScenario()` to anonymous-namespace scope (`kV05Base`) so `BuildSpikeScene()`'s V05 case can derive a camera from it: `eye = kV05Base + (0, -150, 75)`, `target = kV05Base` (distance ~167.7, half-width angle ~16.6°, comfortably inside the ~25.78° half-FOV). `ViewportWindow` gained `SetSceneCamera(eye, target, worldUp)`, wired from `MainWindow::OnSceneSelected()` right after `SetActiveSceneMeshes()`. Purely a WHERE-the-camera-stands change - GPU mesh positions stay float32, camera/math stay double (`bim::viewport::Vector3`); the locked precision contract is unchanged. |
| M11 | Make live evidence exercise the real locked surface/lifecycle path; authoritative live D3D11 unavailable must not read as PASS | Fixed a real logic bug in `evidence_mode.cpp`'s `RunEvidenceMode()`: when a live run (`--evidence-mode-live`) could not obtain a real D3D11 surface, `on_surface_available` was recorded `NOT_AVAILABLE` but `RecordResult(false)` was never called, so `overall_passed` could incorrectly stay `true` - fixed with an explicit `RecordResult(false)` on that branch. The backend-identity check's `\|\| !live_surface_available` auto-pass clause (a second path to the same bug) was removed (`!init_status` alone is kept, to avoid double-penalizing the same root cause). Minimize/restore and DPR-transition evidence now actually drive the real live `QWindow` (`EvidenceModeOptions` gained `liveWindow`/`differingDpiScreen` fields, populated by `main.cpp`) via `showMinimized()`/`showNormal()`/`setScreen()`/`setGeometry()`, pumping the Qt event loop (new `PumpEventsFor` helper) and comparing real `visibility()`/`devicePixelRatio()` before/after, recording real pass/fail JSON. `NOT_AVAILABLE` is now reserved for (a) no live window at all (already recorded as a failure via the `on_surface_available` fix) and (b) the previously-approved differing-DPI cross-monitor carve-out (AC-019) specifically, when that hardware genuinely is not present. |
| M12 | Runbook D must actually configure+build fresh; verify the real main worktree; fail on VS/VCTools mismatch; enforce exact qtbase/bgfx versions; never accept stale artifacts | Item 16 now wipes `$BuildDir` and drives a real `cmake --preset ci-win-msvc` + `cmake --build --preset ci-win-msvc` (the exact preset this repo's own `CMakePresets.json` ships) - moved to run immediately **before** items 6/8/11-14 in execution order (still tagged Brief item 16 in the tally), since those read `CMakeCache.txt` / the resolved `vcpkg_installed` tree and must inspect this run's own fresh configure, not a leftover build directory. A new `Assert-FileFresh` helper then requires every item-16 artifact to be newer than the moment this build started, so a stale artifact cannot silently pass. Item 5's VS/VCTools version mismatch is now a hard failure (`throw`) instead of a Yellow warning that always let the check pass regardless of the actual installed version. Items 11+12 now require the exact `qtbase 6.11.1#1` / `bgfx 1.129.8940-496#1` version#portversion tokens (new `-ExpectedQtBasePortVersion`/`-ExpectedBgfxPortVersion` params, tokenizing the `vcpkg list` line on whitespace) rather than a bare version substring match that would also accept a stale/mismatched portversion. Item 4 now also discovers any separately checked-out main/master git worktree (`git worktree list --porcelain`) and, if one exists, verifies it is at the frozen main baseline and has a clean `git status` - a no-op when no such separate worktree exists, since Implementation Authorization names only one authorized worktree for this task. |

### 8.2 Closing Round 1's remaining self-determined MINOR findings

| Finding | What it required | Resolution |
|---|---|---|
| MINOR (RenderFrame status contract) | `renderer.hpp`'s doc comment promised stale-handle rejection is reported via the returned `Status`, but the implementation always returned `Ok()` | `renderer.cpp`'s `RenderFrame()` now tracks `any_handle_rejected` across its validation loop and returns `Status::Fail(ResourceNotFound)` when set, while still completing the frame for valid handles (record, don't abort - not a fatal condition). The M03 integration regression test was updated to assert the now-correct `Status`, plus a new mixed-valid-and-stale-handle case proving the fix records rather than aborts the whole frame. |
| MINOR (unjustified noexcept allocations) | Functions marked `noexcept` that heap-allocate, with no internal exception safety | `CreateMesh()`'s vertex-building/`bgfx::copy`/`AllocateSlot()` region is now wrapped in `try/catch(std::bad_alloc, std::length_error)`, converting to the existing `Result<T>::Fail(ResourceCreationFailed)` vocabulary. `Renderer::Impl::ReleaseSlot()` is now explicitly `noexcept` with its own internal `try/catch` around `free_slots.push_back` that no-ops silently on failure (freeing a slot must not itself throw) - header/impl declarations updated to match. |
| MINOR (R9 Qt token coverage) | R9's Qt token list was weaker than R8's own grouped Qt list | `tools/architecture_checker.cmake`'s `_r9_tokens` expanded to also catch `QWindow`/`QScreen`-based surface code (which M08/M11 now use directly) and the other Qt Gui/Widgets/Core symbols this codebase actually uses (`QGuiApplication`, `QCoreApplication`, `QMainWindow`, `QTimer`, `QMouseEvent`, `QWheelEvent`, `QResizeEvent`, `QShowEvent`, `QHideEvent`, `QCloseEvent`, `QExposeEvent`, `QPlatformSurfaceEvent`, `QDebug`, `qDebug(`, `Q_OBJECT`, `Q_ASSERT`, `QComboBox`, `QLabel`, `QStatusBar`, `QToolBar`, `QVBoxLayout`), mirroring R8's grouped style. |
| MINOR (non-owning handle comment) | `RenderMeshHandle`'s doc comment called it an "Owning handle" when `Renderer` actually owns the GPU resource | `renderer.hpp`'s `RenderMeshHandle` class doc comment corrected to "Non-owning handle", with the true ownership semantics spelled out (`Renderer` owns the resource; the handle is a non-RAII capability/index; `DestroyMesh()` must be called explicitly). |

### 8.3 What this round explicitly did not do (per the user's instruction)

- **Did not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`.** B02
  and M12 changed that script's own logic substantially, but this session
  still has no execution channel to the Windows worktree - every PASS/FAIL
  it would report remains hypothetical until the Operator actually runs it.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as Rounds 0/1.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.

### 8.4 Exact changed-path summary

**15 of the 59 authorized paths were touched this round:**

- **B02**: `Verification-RunbookD-v1.0.ps1`
- **M08**: `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`
- **M09**: `src/desktop/src/spike_scene.hpp`, `src/desktop/src/spike_scene.cpp` (also M10), `tests/unit/unit_viewport_mesh_contract.cpp`
- **M10**: `src/desktop/src/spike_scene.hpp`, `src/desktop/src/spike_scene.cpp` (also M09), `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp` (also M08), `src/desktop/src/main_window.cpp`
- **M11**: `src/desktop/src/evidence_mode.hpp`, `src/desktop/src/evidence_mode.cpp`, `src/desktop/src/main.cpp`
- **M12**: `Verification-RunbookD-v1.0.ps1` (same file as B02 - both findings target this one script)
- **MINOR (RenderFrame status contract)**: `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp` (also MINOR non-owning-handle-comment), `src/viewport/bgfx/src/renderer.cpp` (also MINOR noexcept-allocations), `tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp`
- **MINOR (noexcept allocations)**: `src/viewport/bgfx/src/renderer.cpp`, `src/viewport/bgfx/src/renderer_impl.hpp`
- **MINOR (R9 Qt token coverage)**: `tools/architecture_checker.cmake`
- **MINOR (non-owning handle comment)**: `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`
- **This document and its JSON companion** (updated - see section 9's Round 3 correction: these two files ARE counted as part of the 59-path footprint, as its own items 58-59, not separately from it, contrary to what this sentence and its Round 1 counterpart above previously said): `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`

The complete de-duplicated list of touched paths (15): `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`,
`src/viewport/bgfx/src/renderer.cpp`, `src/viewport/bgfx/src/renderer_impl.hpp`,
`tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp`,
`src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`,
`src/desktop/src/spike_scene.hpp`, `src/desktop/src/spike_scene.cpp`,
`src/desktop/src/main_window.cpp`, `tests/unit/unit_viewport_mesh_contract.cpp`,
`src/desktop/src/evidence_mode.hpp`, `src/desktop/src/evidence_mode.cpp`,
`src/desktop/src/main.cpp`, `tools/architecture_checker.cmake`,
`Verification-RunbookD-v1.0.ps1`.

**44 of the 59 authorized paths were left completely untouched this round**
(no B02/M08-M12/MINOR finding implicated them; byte-for-byte identical to
the post-Round-1 candidate) - every path in section 1's 59-path footprint
not listed in the de-duplicated list immediately above, including every file
Round 1 touched that this round's findings did not re-implicate (e.g.
`src/desktop/src/viewport_bridge.hpp`/`.cpp`, `src/desktop/CMakeLists.txt`,
all three shader source files, all four negative architecture fixtures, and
the 20 files Round 1 itself already left untouched).

Per-file updated byte size and SHA256 for all 15 touched files are in the
JSON companion's `footprint.modify`/`footprint.add` arrays
(`aa_source_review_round_2_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_source_review_round_2_corrections.findings_addressed`.

### 8.5 Verification status (unchanged posture, restated)

Same structural fact as sections 0 and 4: this session has no
command-execution channel to the Windows task worktree. Nothing this round
touched has been compiled, linked, or run. `CMakePresets.json` and
`vcpkg-configuration.json` were read directly from the worktree (via
`device_stage_files`, read-only, not part of the footprint) to confirm exact
facts this round's corrections depend on - never executed. Every
`Verification-RunbookD-v1.0.ps1` check remains `NOT_AVAILABLE` in
`CLAUDE_HANDOVER.json`'s `verification_status`, including the ones B02/M12
changed this round. **This document does not claim, and must not be read
as claiming, that `Verification-RunbookD-v1.0.ps1` has passed.**

---

## 9. AA Source Review Round 3 correction round

### 9.0 Scope disclosure

This round's findings (**B03**, **M13-M15**, and a four-part self-determined
**MINOR** bundle) came directly from the user's own Round 3 chat instruction,
quoted in full at the start of that instruction - not from any document
Claude read on disk, same provenance pattern as Rounds 1 and 2. The
handover-wording bug specifically (MINOR bundle, 4th item below) was
confirmed directly against `Verification-RunbookD-v1.0.ps1`'s own embedded,
self-validated 59-entry `$AuthorizedFootprint` array (`if
($AuthorizedFootprint.Count -ne 59) { throw ... }`) - its literal items 58
and 59 are `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` and
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json` - and against this JSON
companion's own `footprint.modify`/`footprint.add` arrays, which (before
this round) totaled only 57 entries (12 modify + 45 add), one MODIFY-count
and one ADD-count short of matching this very document's section-1 claim of
"12 MODIFY, 47 ADD" (= 59) - the missing 2 ADD entries were always exactly
this document and its JSON companion. That is: section 1's "47 ADD" figure
was arithmetically correct only if the handover pair is counted as part of
the 59, directly contradicting sections 7.3/8.4's own "not counted against
the 59-path footprint" wording elsewhere in the very same document - an
internal self-contradiction, not merely an isolated wrong sentence.

**Explicit constraints from the user's instruction, honored throughout:**
stay strictly within the authorized 59-path footprint; no staging, no git
commit, no `main` mutation; ACR remains NONE unless a correction genuinely
requires an architecture change (none did); update this document and its
JSON companion with the exact delta and return to Architecture Authority;
**do not run or claim `Verification-RunbookD-v1.0.ps1` PASS.** All five are
honored - see 9.4 for the exact changed-path summary and 9.5 for what this
round explicitly did not do.

### 9.1 Findings B03 and M13-M15 - what changed and why

| Finding | What it required | Resolution |
|---|---|---|
| B03 | Before any fresh configure/build, establish and verify the accepted process-local Windows toolchain environment (VS Build Tools 17.14.37614.0, VCTools 14.44.35207, x64 cl.exe, accepted bundled `VCPKG_ROOT`, Ninja 1.12.1 at the accepted C0 path), without relying on the caller's PATH/VCPKG_ROOT/CXX state; tool identity checks that can run before configure must run before configure | `Verification-RunbookD-v1.0.ps1` gained a new `Import-VsDevEnvironment` helper (runs `vcvarsall.bat x64` via `cmd.exe`, parses its `set` output, and applies every `KEY=VALUE` pair to the current process's `$env:` scope via `Set-Item`) and a new check, positioned immediately after the existing item-5 VS-Build-Tools-version check, that: locates the VS installation via `vswhere` and calls `Import-VsDevEnvironment` against its `vcvarsall.bat`; forces `$env:VCPKG_ROOT` to the new `-AcceptedVcpkgRoot` parameter (verifying it exists first); resolves Ninja specifically at the accepted `-HistoricalNinjaPath` (deliberately not via item 8's lenient PATH-first `Resolve-NinjaExecutable` - B03 asks for the accepted path directly), verifies its version, and prepends its directory to `$env:PATH`; verifies `cl.exe` resolves via `Get-Command cl` with a path containing the expected VCTools version, then forces `$env:CC`/`$env:CXX` to that resolved path. Items 7 (CMake version), 8 (Ninja version), and 9 (clang-format/clang-tidy - see M15 below) - every tool-identity check that does not need `$BuildDir`/`CMakeCache.txt` to exist - were moved to run immediately after this new check and before item 16's fresh configure/build, so every check that CAN run before a configure now does. |
| M13 | Live evidence must exercise the actual production `ViewportWindow`/private `ViewportSurface` lifecycle path, not a second, separately-constructed `QWindow` plus a manually-driven `Renderer`/`ViewportLifecycle` | `ViewportWindow` gained a small set of evidence-mode-only public accessors (`EvidenceNativeSurface()`, `EvidenceIsRendererReady()`, `EvidenceBackendInfo()`, `EvidenceLifecycleState()`, `EvidenceForceSurfaceRecreation()`, `EvidenceRunScene()`) - each a thin forward to the existing private `renderer_`/`lifecycle_`/`camera_`/`surface_` members and the existing private `EnsureRendererInitialized()`/`TeardownRenderer()` methods, no logic duplicated. `main.cpp`'s `--evidence-mode-live` branch now constructs a real `ViewportWindow` (not a bare `QWindow`), waits for its renderer to actually become ready via `EvidenceIsRendererReady()`, and populates a new `EvidenceModeOptions::viewportWindow` field with it; `liveWindow` now points at this same object's top-level `windowHandle()` (for minimize/restore/screen-move, which are properly top-level-window concerns), while `nativeWindowHandle` now comes from the embedded `ViewportSurface`'s own `winId()` (the actual bgfx render target - previously this used the bare `QWindow`'s own handle, which would have been the wrong native surface once a real `ViewportWindow` replaced it). `evidence_mode.cpp`'s `RunEvidenceMode()` now branches live-mode's renderer-initialize/lifecycle/backend-identity/scenes/resize/surface-recreation sections to read from `options.viewportWindow`'s Evidence* accessors instead of a locally-instantiated `Renderer`/`Camera`/`ViewportLifecycle` - the local `renderer` stays uninitialized for the entire live path (used only by headless mode and, unaffected by this finding, the pre-existing process-boundary-style repeatability loop, which legitimately keeps constructing its own short-lived `Renderer` instances unrelated to `ViewportWindow` ownership). Live-mode shutdown evidence now calls `close()` on the real `ViewportWindow`, exercising its actual `closeEvent()`-driven `TeardownRenderer()`+`BeginShutdown()`+`CompleteShutdown()` sequence, rather than driving a throwaway local `ViewportLifecycle`; the repeatability section was moved to run just before this close()-driven shutdown (not after, as previously), so its own still-legitimate raw `Renderer` cycles do not run against an already-closed window. Headless mode is completely unchanged. |
| M14 | `RunOneScene`/live scene evidence must apply each `SpikeScene`'s suggested camera - in particular V05 must use its own camera eye/target so the large-coordinate scene is actually observable, not merely submitted outside the frustum | `RunOneScene()` (headless path) now builds a per-scene copy of the shared camera and calls `SetLookAt(scene.cameraEye, scene.cameraTarget, scene.cameraWorldUp)` on it before `RenderFrame()`, instead of unconditionally reusing the shared camera's fixed default look-at; the per-scene camera's own `SetLookAt` result now also gates that scene's `overall_passed`. Each scene's JSON entry gained `camera_applied`/`camera_eye`/`camera_target` fields. A new `RunOneLiveScene()` does the live-mode equivalent via `ViewportWindow::SetSceneCamera()` (already existed, from Round 2/M10) followed by the new `EvidenceRunScene()` (M13). V01-V04's suggested cameras equal the previous shared default, so this changes nothing observable for them; V05 is the one scene this fixes. |
| M15 | Runbook D must additionally fail closed on the fresh CMake cache's exact `CMAKE_TOOLCHAIN_FILE`, `VCPKG_TARGET_TRIPLET=x64-windows`, effective compiler/VCTools identity (already covered by the existing item 6), and exact clang-format + clang-tidy `19.1.5` - no partially different toolchain may PASS | Item 9's check body extended to also verify `clang-format --version`/`clang-tidy --version` output contains the new `-ExpectedClangToolsVersion` (`19.1.5`) parameter, not merely PATH presence. A new check, positioned immediately after the existing item-6 compiler-identity check, reads the fresh `CMakeCache.txt` for `CMAKE_TOOLCHAIN_FILE` and normalizes it (slashes, case) against `Join-Path $AcceptedVcpkgRoot 'scripts\buildsystems\vcpkg.cmake'`, and separately reads `VCPKG_TARGET_TRIPLET` and requires it to be exactly `x64-windows` - both fail closed (`throw`) on any mismatch. |

### 9.2 The four-part self-determined MINOR bundle

| Finding | What it required | Resolution |
|---|---|---|
| MINOR (unjustified `noexcept`) | `Renderer::Renderer()` and `Renderer::BackendInfo()` were marked `noexcept` without being genuinely non-throwing | Both allocate (`std::make_unique<Impl>()` in the constructor; `std::string` assignment in `BackendInfo()`) with no internal exception-safety wrapping that would make `noexcept` honest - unlike `CreateMesh()`, a constructor has no `Result<T>`/`Status` vocabulary to return, and catching `bad_alloc` internally would leave `impl_ == nullptr` while every other method assumes it is always valid (a broken invariant, worse than propagating); `BackendInfo()`'s plain `RendererBackendInfo` struct likewise has no error field to signal an allocation failure through. Per the user's own instruction ("...unless you make them genuinely non-throwing"), `noexcept` was simply removed from both the declarations (`renderer.hpp`) and definitions (`renderer.cpp`). |
| MINOR (differing-DPI screen-selection baseline) | `main.cpp`'s differing-DPI screen-selection loop compared candidate screens against `screens.front()`'s DPR - an arbitrary enumeration-order screen, not necessarily the one the evidence window is actually showing on | Now compares against the evidence window's own current screen's `devicePixelRatio()` (via its top-level `windowHandle()->screen()`, falling back to `evidence_window.screen()`), evaluated after the window is shown/exposed - not `screens.front()`. |
| MINOR (handover wording) | Correct wording implying the two handover files are outside the 59-path allowlist | Sections 7.3 and 8.4's "not counted against the 59-path footprint" claims were factually wrong - `Verification-RunbookD-v1.0.ps1`'s own embedded, self-validated 59-entry `$AuthorizedFootprint` array lists the handover pair as its own items 58-59, and this document's own section-1 "47 ADD" figure only reconciles to 59 if they are counted. Both sentences corrected in place (see the diff in sections 7.3/8.4 above); this section (9.0) documents the underlying arithmetic contradiction that made the bug independently checkable, not merely asserted. `CLAUDE_HANDOVER.json`'s `footprint.add` array (which had never actually listed either handover file - the root cause `footprint_discipline` prose never explicitly claimed exclusion, but the array's own incompleteness was the same underlying gap) gained two new ADD entries for them this round - see the JSON companion. |

### 9.3 What this round explicitly did not do (per the user's instruction)

- **Did not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`.** B03
  and M15 changed that script's own logic substantially, but this session
  still has no execution channel to the Windows worktree - every PASS/FAIL
  it would report remains hypothetical until the Operator actually runs it.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as Rounds 0/1/2.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.

### 9.4 Exact changed-path summary

**10 of the 59 authorized paths were touched this round:**

- **B03 + M15**: `Verification-RunbookD-v1.0.ps1` (same file - both findings target this one script)
- **M13**: `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`, `src/desktop/src/evidence_mode.hpp`, `src/desktop/src/evidence_mode.cpp` (also M14), `src/desktop/src/main.cpp` (also MINOR differing-DPI)
- **M14**: `src/desktop/src/evidence_mode.cpp` (same file as M13 above)
- **MINOR (noexcept)**: `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`, `src/viewport/bgfx/src/renderer.cpp`
- **MINOR (differing-DPI baseline)**: `src/desktop/src/main.cpp` (same file as M13 above)
- **MINOR (handover wording)**: `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.json` - both properly counted as part of the 59-path footprint per this very finding's own correction (see 9.0/9.2), not listed separately from it the way Rounds 1/2 incorrectly did

The complete de-duplicated list of touched paths (10): `Verification-RunbookD-v1.0.ps1`,
`src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`,
`src/desktop/src/evidence_mode.hpp`, `src/desktop/src/evidence_mode.cpp`,
`src/desktop/src/main.cpp`, `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`,
`src/viewport/bgfx/src/renderer.cpp`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`.

**49 of the 59 authorized paths were left completely untouched this round**
(no B03/M13-M15/MINOR finding implicated them; byte-for-byte identical to
the post-Round-2 candidate) - every path in section 1's 59-path footprint
not listed in the de-duplicated list immediately above.

Per-file updated byte size and SHA256 for all touched files are in the JSON
companion's `footprint.modify`/`footprint.add` arrays
(`aa_source_review_round_3_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_source_review_round_3_corrections.findings_addressed`.

### 9.5 Verification status (unchanged posture, restated)

Same structural fact as sections 0, 4, and 8.5: this session has no
command-execution channel to the Windows task worktree this round either.
Nothing this round touched has been compiled, linked, or run. Every
`Verification-RunbookD-v1.0.ps1` check remains `NOT_AVAILABLE` in
`CLAUDE_HANDOVER.json`'s `verification_status`, including the ones B03/M15
changed this round. **This document does not claim, and must not be read as
claiming, that `Verification-RunbookD-v1.0.ps1` has passed.** The
M13/M14/M15/B03/MINOR corrections above are, like every prior round, authored
text only - the Operator's real build is still the first actual verification
of any of it.

---

## 10. AA Source Review Round 4 correction round

### 10.0 Scope disclosure

This round's findings (**M13 final closure** plus a three-part
self-determined-scope **MINOR** hardening bundle, since the user's own Round
4 instruction specified each MINOR item precisely) came directly from the
user's own Round 4 chat instruction, quoted in full at the start of that
instruction - not from any document Claude read on disk, same provenance
pattern as Rounds 1-3. The M13 "final closure" framing is the user's own
characterization: Round 3's `EvidenceForceSurfaceRecreation()` (removed this
round) called the private `TeardownRenderer()`/`EnsureRendererInitialized()`
methods directly - a synthetic proof that those two methods behave correctly
when invoked, not that the real Qt-driven
`QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed`/re-expose event path
actually triggers them. Similarly, Round 2/3's minimize/restore evidence
passed on the top-level `QWindow::visibility()` transition alone, and the
DPR-transition evidence observed only the top-level window's
`devicePixelRatio()` - neither proved the production `lifecycle_`/`renderer_`
members were actually driven by that transition.

During this round's own code review (ahead of authoring the fix), Claude
diagnosed a genuine, previously-undisclosed production defect: the
constructor's `connect(surface_, &QWindow::visibilityChanged, ...)` bridge
(`viewport_window.cpp`) listens on the embedded CHILD `surface_`'s own
visibility, checking for `QWindow::Minimized` - but `Minimized`/`Maximized`/
`FullScreen` are documented as top-level-only Qt window-visibility states, so
a child window embedded via `QWidget::createWindowContainer()` would not
itself be expected to report `Minimized` when only its top-level ancestor is
minimized. This means the existing production suspend/resume bridge likely
never fired in response to a real top-level minimize, in the interactive
session as well as in evidence mode - not merely an evidence-authoring gap.
Per the user's explicit instruction ("...or minimally correct the existing
production visibility bridge inside the authorized path"), this was corrected
with a new `ViewportWindow::changeEvent()` override reacting to the
QWidget's own `QEvent::WindowStateChange`/`windowState() & Qt::WindowMinimized`
- see 10.1's M13 row. The original `surface_`-visibility lambda was left in
place (not removed) rather than assumed dead code on a platform this session
cannot execute: both bridges now independently gate on `lifecycle_`'s current
state before calling `OnSuspend()`/`OnResume()`, so if the child window does
report `Minimized` on some platform, the double-firing is harmless, and if it
never does, the new top-level bridge is what actually protects the guarantee.

**Explicit constraints from the user's instruction, honored throughout:**
stay strictly within the authorized 59-path footprint; do not regenerate
unaffected files; no staging, no git commit, no `main` mutation; ACR remains
NONE unless a correction genuinely requires an architecture change (none
did); update this document and its JSON companion with the exact Round-4
delta and return to Architecture Authority; **do not run or claim
`Verification-RunbookD-v1.0.ps1` PASS.** All are honored - see 10.4 for the
exact changed-path summary and 10.5 for what this round explicitly did not
do.

### 10.1 Finding M13 (final closure) - what changed and why

| Point | What it required | Resolution |
|---|---|---|
| M13.1 (surface recreation) | Replace the synthetic `EvidenceForceSurfaceRecreation()` with evidence that causes an actual native `ViewportSurface` destruction/recreation through Qt, so the real `QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed` production callback actually executes; verify the intermediate state is `SurfaceUnavailable` with the renderer down, then verify the final state is `Ready` with the renderer initialized; no second renderer/lifecycle architecture | `ViewportWindow::EvidenceForceSurfaceRecreation()` (declared `viewport_window.hpp`, defined `viewport_window.cpp`) is removed and replaced by two methods: `EvidenceDestroyNativeSurface()` (calls the real `surface_->destroy()`) and `EvidenceRecreateNativeSurface()` (calls the real `surface_->create()` + `setVisible(true)` + `requestActivate()`). Neither method calls `TeardownRenderer()`/`EnsureRendererInitialized()` itself - both are reached only as a side effect of Qt delivering the real `SurfaceAboutToBeDestroyed`/exposeEvent to `ViewportSurface`'s existing `event()`/`exposeEvent()` overrides, which invoke the existing `onSurfaceAboutToBeDestroyed`/`onExposedOrHidden` callbacks exactly as the real interactive surface-loss/recreation path does. `evidence_mode.cpp`'s `surface_recreation_result` block now: calls `EvidenceDestroyNativeSurface()`, pumps the event loop (200ms), and requires the intermediate `EvidenceLifecycleState() == SurfaceUnavailable && !EvidenceIsRendererReady()`; then calls `EvidenceRecreateNativeSurface()`, pumps again (300ms), and requires the final `EvidenceLifecycleState() == Ready && EvidenceIsRendererReady()`. `passed` requires both; both intermediate and final states are recorded in the JSON (`state_before_destroy`/`state_after_destroy`/`intermediate_surface_unavailable_and_renderer_down`/`state_after_recreate`/`final_ready_and_renderer_initialized`), not only the final PASS/FAIL bit. |
| M13.2 (minimize/restore) | Do not pass merely because top-level `QWindow::visibility()` changed; require the actual production `ViewportWindow::EvidenceLifecycleState()` to be `Suspended` after minimize and `Ready` (renderer initialized) after restore; let evidence fail rather than invent a pass, or minimally correct the existing production visibility bridge | Diagnosed and fixed the real production defect described in 10.0 above: `ViewportWindow` gained a `changeEvent(QEvent*)` override (declared `viewport_window.hpp`'s `protected:` section, defined `viewport_window.cpp`) that reacts to `QEvent::WindowStateChange` on the QWidget itself (the top-level-window-driven state, unlike the child-`surface_`-scoped lambda already in the constructor) and drives the identical `lifecycle_.OnSuspend()`/`OnResume()` + `render_timer_` stop/start. `evidence_mode.cpp`'s `minimize_restore_result` block now additionally gates on `options.viewportWindow != nullptr` (not just `options.liveWindow != nullptr`) and requires `EvidenceLifecycleState() == Suspended` after minimize and `EvidenceLifecycleState() == Ready && EvidenceIsRendererReady()` after restore, in addition to the pre-existing `visibility()` checks - `passed` requires all four conditions. The lifecycle states observed before/at-minimize/at-restore are all recorded in the JSON, not only the final bit. |
| M13.3 (differing-DPI) | Observe the embedded production `EvidenceNativeSurface()` DPR before/after the cross-monitor move, not only the top-level window DPR; require the production renderer to remain initialized/Ready after the transition; the only allowed `NOT_AVAILABLE` case remains absence of two real screens with differing DPR | `evidence_mode.cpp`'s `dpr_change_result` block now reads `options.viewportWindow->EvidenceNativeSurface()->devicePixelRatio()` before/after the `setScreen()`/`setGeometry()` transition (falling back to `options.liveWindow`'s DPR only if `EvidenceNativeSurface()` is unexpectedly null, which is recorded via a new `observed_on` field), and additionally requires `EvidenceIsRendererReady() && EvidenceLifecycleState() == Ready` after the transition (`renderer_survived_transition`), factored into `passed` alongside the pre-existing `dpr_actually_changed` check. The AC-019 differing-DPI-hardware-absent carve-out (`NOT_AVAILABLE`) is otherwise unchanged - it remains the only case permitted to stay `NOT_AVAILABLE` rather than pass/fail. |

All three points keep surface recreation, minimize/restore, and DPR evidence
on the exact same production `ViewportWindow`/`ViewportSurface`
implementation - no second architecture was introduced anywhere in this
round, consistent with Round 3's M13 and this round's own instruction.

### 10.2 The three-part MINOR hardening bundle (`Verification-RunbookD-v1.0.ps1`) and the optional `viewport-spike.ps1` correction

| Finding | What it required | Resolution |
|---|---|---|
| MINOR (VS `installationVersion` exact match) | Compare `installationVersion` exactly to `17.14.37614.0`, no prefix acceptance | Item 5's original check (`$installVersion -notlike "$ExpectedVsBuildToolsVersion*" -and $installVersion -ne $ExpectedVsBuildToolsVersion`) accepted any version sharing the expected value as a mere prefix (e.g. a later servicing release beginning with the same digits) without throwing. Now `if ($installVersion -ne $ExpectedVsBuildToolsVersion) { throw ... }` - exact-only, no prefix branch. (This is distinct from the B03-addendum "process-local environment established" check, also tagged item 5, which was not touched this round - it establishes the environment via `vcvarsall.bat` rather than validating `installationVersion` itself.) |
| MINOR (`cl.exe` exact normalized full-path match) | Compare the resolved `cl.exe` normalized full path against the exact expected `VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe` | Item 6's check previously only verified the resolved compiler path (from `CMakeCache.txt`'s `CMAKE_CXX_COMPILER`) *contained* the VCTools version as a substring (`-notmatch [regex]::Escape($ExpectedVcToolsVersion)`) - which would also accept an unrelated path segment merely embedding that version string, or a differing host/target architecture pair under the correct version. This check now independently resolves the VS installation root via its own `vswhere` call (not read from the B03 check's own scriptblock-local `$installPath`, which does not persist outside that scriptblock's own scope under `Invoke-Check`'s `& $Body` invocation), builds the one specific expected path `<installationPath>\VC\Tools\MSVC\$ExpectedVcToolsVersion\bin\Hostx64\x64\cl.exe`, asserts it exists, normalizes both the resolved and expected paths via `[System.IO.Path]::GetFullPath()`, and requires an exact case-insensitive (`OrdinalIgnoreCase` - Windows paths) match, not a substring/contains match. |
| MINOR (clang-format/clang-tidy exact semantic version) | Parse clang-format/clang-tidy semantic version and require exactly `19.1.5`, not merely "contains the substring" | Item 9's check previously matched raw `--version` output against `[regex]::Escape($ExpectedClangToolsVersion)` (substring containment), which would also accept an unrelated longer version string embedding `19.1.5` (e.g. a hypothetical `19.1.50`). Now parses the actual `X.Y.Z` semantic-version token out of each tool's `--version` output via `-match '(\d+\.\d+\.\d+)'`, and requires the captured `$Matches[1]` to `-eq` (exact) `$ExpectedClangToolsVersion`, for both `clang-format` and `clang-tidy` independently; a `--version` output with no parseable `X.Y.Z` token throws with an explicit "not parseable" message rather than silently failing the substring test. |
| MINOR (optional: stale `viewport-spike.ps1` comment) | The comment claiming live-surface unavailability does not fail the job is stale; current fail-closed behavior is intended | The `NOTE:` `Write-Host` message printed when `$liveSurfaceWasAvailable` is false previously claimed "This job does not fail solely for that reason" - stale/inaccurate, since `evidence_mode.cpp` explicitly calls `RecordResult(false)` whenever no live surface/production `ViewportWindow` was available (Brief section 17's "record `NOT_AVAILABLE` honestly" posture does not mean "exempt from `overall_passed`"), so `overall_passed` is false in that case and the script's own subsequent `if (-not $liveEvidence.overall_passed) { throw ... }` check does fail the job. The message now says the job DOES fail as a result and points at that check, instead of contradicting the code immediately beneath it. |

### 10.3 What this round explicitly did not do (per the user's instruction)

- **Did not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`.** This
  round hardened three of its checks substantially, but this session still
  has no execution channel to the Windows worktree - every PASS/FAIL it
  would report remains hypothetical until the Operator actually runs it.
- Did not regenerate or rewrite any file this round's findings did not
  implicate - `src/desktop/src/evidence_mode.hpp` in particular was reviewed
  and confirmed to need no change (its `EvidenceModeOptions::viewportWindow`
  field and forward declaration already support everything this round's
  `evidence_mode.cpp`/`viewport_window.hpp`/`.cpp` changes needed).
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as Rounds 0/1/2/3.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.

### 10.4 Exact changed-path summary

**7 of the 59 authorized paths were touched this round:**

- **M13 (all three points)**: `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`, `src/desktop/src/evidence_mode.cpp`
- **MINOR (VS installationVersion exact match, cl.exe exact path, clang-tooling semantic version)**: `Verification-RunbookD-v1.0.ps1`
- **MINOR (optional viewport-spike.ps1 stale-comment correction)**: `scripts/ci/viewport-spike.ps1`
- **This document and its JSON companion** (updated, properly counted as part of the 59-path footprint per Round 3's own correction - see 9.0/9.2): `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`

The complete de-duplicated list of touched paths (7): `src/desktop/src/viewport_window.hpp`,
`src/desktop/src/viewport_window.cpp`, `src/desktop/src/evidence_mode.cpp`,
`Verification-RunbookD-v1.0.ps1`, `scripts/ci/viewport-spike.ps1`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`.

**52 of the 59 authorized paths were left completely untouched this round**
(no M13/MINOR finding implicated them; byte-for-byte identical to the
post-Round-3 candidate) - every path in section 1's 59-path footprint not
listed in the de-duplicated list immediately above, including
`src/desktop/src/evidence_mode.hpp` (reviewed, confirmed to need no change -
see 10.3) and every other file Rounds 1-3 touched that this round's findings
did not re-implicate.

Per-file updated byte size and SHA256 for all touched files are in the JSON
companion's `footprint.modify`/`footprint.add` arrays
(`aa_source_review_round_4_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_source_review_round_4_corrections.findings_addressed`.

### 10.5 Verification status (unchanged posture, restated)

Same structural fact as sections 0, 4, 8.5, and 9.5: this session has no
command-execution channel to the Windows task worktree this round either.
Nothing this round touched has been compiled, linked, or run. Every
`Verification-RunbookD-v1.0.ps1` check remains `NOT_AVAILABLE` in
`CLAUDE_HANDOVER.json`'s `verification_status`, including the three this
round hardened. **This document does not claim, and must not be read as
claiming, that `Verification-RunbookD-v1.0.ps1` has passed.** The M13
final-closure logic and the MINOR hardening bundle above are, like every
prior round, authored text only, explicitly disclosed as `UNVERIFIED` inline
at each new method - the Operator's real build is still the first actual
verification of any of it, in particular whether `QWindow::destroy()`/
`create()` on a `createWindowContainer()`-embedded window actually behaves as
this round's authoring assumes (see the `UNVERIFIED` comments on
`EvidenceDestroyNativeSurface()`/`EvidenceRecreateNativeSurface()` in
`viewport_window.hpp`) and whether the new `changeEvent()` override actually
receives `QEvent::WindowStateChange` reliably across minimize/restore on the
real Windows platform plugin.

---

## 11. AA Source Review Round 5 correction round ("final pre-Runbook minimum delta")

### 11.0 Scope disclosure

This round's findings (**M16**, **N03**, **N04**, **N05**) came directly
from the user's own Round 5 chat instruction, quoted in full at the start of
that instruction - not from any document Claude read on disk, same
provenance pattern as Rounds 1-4. M16 is a genuine architectural gap in
Round 4's own M13.2 fix, identified by the user: Round 4's
`ViewportWindow::changeEvent()` override reacted to `windowState()` on
`ViewportWindow` itself, which is only meaningful when `ViewportWindow` is
the top-level widget. That was true of Round 3/4's `--evidence-mode-live`
construction (`ViewportWindow evidence_window;` constructed directly as
`main()`'s own top-level window) but is **not** true of the real interactive
application, where `MainWindow` (a `QMainWindow`) constructs `ViewportWindow`
and embeds it as `setCentralWidget(viewport_window_)` (see
`main_window.cpp`, unchanged since Round 1). A child widget's own
`windowState()`/`QEvent::WindowStateChange` do not track its top-level
ancestor's minimize/restore state, so Round 4's fix, while a correct
diagnosis of the Round 1-3 defect it replaced, still did not prove the real
production minimize/restore path - it proved a path that only existed
because evidence mode's own shell construction diverged from the real
application's shell construction. This round closes that gap from both
ends: `ViewportWindow` now observes the real top-level widget directly
(whichever one it is), and `--evidence-mode-live` now constructs the real
`MainWindow` shell so the two are exercising the same relationship.

**Explicit constraints from the user's instruction, honored throughout:** do
not redesign or regenerate unaffected files; stay strictly within the
authorized 59-path footprint; no staging, no git commit, no `main` mutation;
ACR remains NONE; update the handover pair with only this round's delta and
return to Architecture Authority; **do not run or claim
`Verification-RunbookD-v1.0.ps1` PASS.** All are honored - see 11.4 for the
exact changed-path summary and 11.5 for what this round explicitly did not
do.

### 11.1 Finding M16 - what changed and why

| Component | What it required | Resolution |
|---|---|---|
| Production lifecycle bridge (`ViewportWindow`) | Correct the bridge so the child `ViewportWindow` observes the actual top-level application window's `WindowStateChange`/minimize state, not only its own `QWidget::windowState()` when it happens to be top-level; do not reintroduce the old input-event-filter assumption - this is only for top-level window lifecycle state | Round 4's `changeEvent()` override is removed and replaced by `bool eventFilter(QObject* watched, QEvent* event) override` plus a new private `InstallTopLevelWindowStateBridge()` helper and `observed_top_level_window_` member. `InstallTopLevelWindowStateBridge()` installs the filter directly on `window()` - Qt's own accessor for "the actual top-level widget in this widget's current parent chain" (resolves to the enclosing `MainWindow` once embedded, or to `ViewportWindow` itself in the degenerate top-level case) - removing it from any previously-observed top-level first, so it stays correctly bound even across reparenting. Called from the constructor (fallback binding, before any parent is attached) and from `showEvent()` (re-binds once real reparenting under `MainWindow::setCentralWidget()` has completed and the widget is actually shown); idempotent (no-op if `window()` is unchanged). `eventFilter()` reacts only to `QEvent::WindowStateChange` observed on the tracked top-level widget and drives the identical `lifecycle_.OnSuspend()`/`OnResume()` + `render_timer_` stop/start Round 4's `changeEvent()` performed - same logic, correct observation target, and it never touches or consumes any other event (returns the base-class result unchanged), so it cannot interfere with input or any other event this widget or its top-level ancestor needs to handle - explicitly not a revival of the M08 input-`eventFilter` assumption Round 2 removed. The constructor's pre-existing `surface_->visibilityChanged` lambda (Round 1, diagnosed-but-kept in Round 4) is left in place unchanged - both bridges independently gate on `lifecycle_`'s current state, so a hypothetical double-fire on some platform stays harmless. The destructor now explicitly removes the filter from `observed_top_level_window_`. | `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp` |
| `--evidence-mode-live` shell relationship (`main.cpp`) | Make `--evidence-mode-live` exercise the same production shell relationship - prefer constructing the real `MainWindow` and observing its contained `ViewportWindow` | `main.cpp` now constructs `MainWindow evidence_main_window;` (previously a bare `ViewportWindow evidence_window;` constructed as its own top-level window) and obtains the real embedded child via a new thin-forward accessor, `MainWindow::EvidenceViewportWindow()` (`main_window.hpp`) - the exact same `ViewportWindow` instance `MainWindow`'s own constructor creates and embeds via `setCentralWidget()`, not a second one built for evidence. `options.liveWindow` is now `evidence_main_window.windowHandle()` (the real top-level `MainWindow`'s native window - the one `ViewportWindow::eventFilter()` above actually observes), and `options.viewportWindow` is the child accessor's return value. Necessary consequences of this shell change, both fixed within this same finding: (1) `options.widthPx`/`heightPx` now come from the child `ViewportWindow`'s own `width()`/`height()`, not `MainWindow`'s full window size, since `MainWindow`'s size additionally includes its toolbar/status-bar chrome that Round 3/4's bare-`ViewportWindow`-as-top-level construction never had; (2) `evidence_mode.cpp`'s resize-evidence block (see below) had to change from resizing `options.viewportWindow` directly to resizing the real top-level window instead. | `src/desktop/src/main.cpp`, `src/desktop/src/main_window.hpp` |
| Resize evidence (necessary consequence, `evidence_mode.cpp`) | (not separately named by the user, but a direct correctness consequence of the M16 shell change - documented explicitly rather than left as a latent bug) | `options.viewportWindow` is now `MainWindow`'s central widget, whose geometry is owned by `QMainWindow`'s own layout - Round 3/4's direct `options.viewportWindow->resize(...)` call would fight that layout manager rather than exercise a realistic resize path once `ViewportWindow` is no longer itself top-level. The resize-evidence block now resizes the real top-level `options.liveWindow` (a `QWindow*`) instead, letting Qt's own layout cascade resize the embedded `ViewportWindow` exactly as a real interactive resize would, and additionally records whether the child's logical size actually changed (`viewport_logical_size_changed`) as part of `passed`, not only whether the renderer stayed ready. | `src/desktop/src/evidence_mode.cpp` |
| Doc-comment consistency (`evidence_mode.hpp`) | (not separately named by the user - a documentation-accuracy consequence of M16) | `EvidenceModeOptions::viewportWindow`'s doc comment previously said `liveWindow` "remains the same object's [viewportWindow's] top-level QWindow (windowHandle())" - no longer true once `viewportWindow` is a child widget. Corrected to describe the current (Round 5) relationship: `liveWindow` is `MainWindow`'s own top-level `windowHandle()`, and `viewportWindow` is its embedded central-widget child, obtained via `EvidenceViewportWindow()`. | `src/desktop/src/evidence_mode.hpp` |

All of the above keep every check on the exact same production
`MainWindow`/`ViewportWindow`/`ViewportSurface` objects the real interactive
application constructs - no second architecture, no second shell
relationship built just for evidence.

### 11.2 Findings N03, N04, N05 - what changed and why

| Finding | What it required | Resolution |
|---|---|---|
| N03 | `ViewportSurface` is owned by `QWidget::createWindowContainer()` - avoid relying on direct `surface_->setVisible(true)` after recreation; use the container-controlled Qt Widgets visibility/exposure path while still causing a genuine native `QWindow` destroy/create and genuine production callbacks | `ViewportWindow::EvidenceRecreateNativeSurface()` no longer calls `surface_->setVisible(true)` directly after `surface_->create()`. It now toggles `surface_container_`'s own Qt Widgets visibility (`hide()` then `show()`) - the container-owned mechanism Qt itself uses to (re)embed and expose its wrapped native child window - which is expected to re-expose the recreated `surface_` through that normal container path rather than bypassing it. `surface_->create()` (the genuine native-window recreation) and `surface_->requestActivate()` (keyboard focus, unrelated to the visibility/exposure concern N03 raises) are both kept; only the exposure-triggering mechanism changed. `EvidenceDestroyNativeSurface()` is unchanged - N03 concerns only re-creation/re-exposure, not destruction. |
| N04 | Parse `cmake --version` and require semantic version exactly `4.4.2`; do not use an unanchored substring regex | Item 7's check previously matched `"cmake version $([regex]::Escape($ExpectedCMakeVersion))"` against the raw `--version` output via `-notmatch` - an UNANCHORED substring match (no `^`/`$`), so an output like `"cmake version 4.4.20"` or `"cmake version 4.4.2-rc1"` would also satisfy it as a false PASS. Now parses the actual `X.Y.Z` semantic-version token following `"cmake version"` via `-match 'cmake version (\d+\.\d+\.\d+)'` and requires the captured token to `-eq` (exact) `$ExpectedCMakeVersion` - the same parse-then-compare pattern Round 4 already established for clang-format/clang-tidy (item 9). |
| N05 | Require the resolved Visual Studio `installationPath` to be exactly `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`, in addition to exact `installationVersion=17.14.37614.0`, exact VCTools `14.44.35207`, and exact Hostx64/x64 `cl.exe` path | A new parameter `-ExpectedVsInstallationPath` (default `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`) was added. Item 5's original check (already exact-only on `installationVersion` since Round 4) now additionally compares `$installJson[0].installationPath` (trailing separators normalized away before comparing) against this parameter exactly (case-insensitive - Windows paths), so a second/decoy Visual Studio installation reporting the same `installationVersion` from a different install root can no longer silently satisfy this check. This is additive to, not a replacement for, Round 4's exact `installationVersion` check and item 6's exact `cl.exe` path check (`VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe`, unchanged this round) - together they now require every VS-identity fact this check group covers (root path, version, VCTools version, exact `cl.exe`) to match exactly. |

### 11.3 What this round explicitly did not do (per the user's instruction)

- **Did not run or claim a PASS for `Verification-RunbookD-v1.0.ps1`.** N04
  and N05 changed two of its checks, but this session still has no
  execution channel to the Windows worktree - every PASS/FAIL it would
  report remains hypothetical until the Operator actually runs it.
- Did not redesign or regenerate any file this round's findings did not
  implicate - `src/desktop/src/main_window.cpp` in particular needed no
  change (`EvidenceViewportWindow()` is a one-line inline accessor added
  only to the header, forwarding to the existing `viewport_window_` member
  the `.cpp` file already constructs and owns), `src/desktop/src/spike_scene.*`,
  `viewport_bridge.*`, and every renderer/`bim::viewport` file were
  untouched.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as Rounds 0-4.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.

### 11.4 Exact changed-path summary

**9 of the 59 authorized paths were touched this round:**

- **M16**: `src/desktop/src/viewport_window.hpp`, `src/desktop/src/viewport_window.cpp`, `src/desktop/src/main.cpp`, `src/desktop/src/main_window.hpp`, `src/desktop/src/evidence_mode.cpp` (resize-evidence consequence), `src/desktop/src/evidence_mode.hpp` (doc-comment consequence)
- **N03**: `src/desktop/src/viewport_window.cpp` (also M16)
- **N04**: `Verification-RunbookD-v1.0.ps1`
- **N05**: `Verification-RunbookD-v1.0.ps1` (same file as N04)
- **This document and its JSON companion** (updated, properly counted as part of the 59-path footprint per Round 3's own correction - see 9.0/9.2): `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`

The complete de-duplicated list of touched paths (9): `src/desktop/src/viewport_window.hpp`,
`src/desktop/src/viewport_window.cpp`, `src/desktop/src/main.cpp`,
`src/desktop/src/main_window.hpp`, `src/desktop/src/evidence_mode.cpp`,
`src/desktop/src/evidence_mode.hpp`, `Verification-RunbookD-v1.0.ps1`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`.

**50 of the 59 authorized paths were left completely untouched this round**
(no M16/N03/N04/N05 finding implicated them; byte-for-byte identical to the
post-Round-4 candidate) - every path in section 1's 59-path footprint not
listed in the de-duplicated list immediately above, including
`src/desktop/src/main_window.cpp` (reviewed, confirmed to need no change -
see 11.3), `scripts/ci/viewport-spike.ps1` (not implicated by this round's
findings), and every other file Rounds 1-4 touched that this round's
findings did not re-implicate.

Per-file updated byte size and SHA256 for all touched files are in the JSON
companion's `footprint.modify`/`footprint.add` arrays
(`aa_source_review_round_5_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_source_review_round_5_corrections.findings_addressed`.

### 11.5 Verification status (unchanged posture, restated)

Same structural fact as sections 0, 4, 8.5, 9.5, and 10.5: this session has
no command-execution channel to the Windows task worktree this round
either. Nothing this round touched has been compiled, linked, or run. Every
`Verification-RunbookD-v1.0.ps1` check remains `NOT_AVAILABLE` in
`CLAUDE_HANDOVER.json`'s `verification_status`, including the two (items 5
and 7) this round hardened further. **This document does not claim, and
must not be read as claiming, that `Verification-RunbookD-v1.0.ps1` has
passed.** The M16 lifecycle-bridge/shell-relationship logic and the N03/N04/
N05 corrections above are, like every prior round, authored text only - in
particular, whether `window()` reliably resolves to the real `MainWindow`
by the time `showEvent()` fires, whether `QMainWindow::setCentralWidget()`
reparenting interacts with `installEventFilter()`/`removeEventFilter()` the
way this round's authoring assumes, and whether toggling
`surface_container_`'s visibility (N03) actually re-exposes the recreated
native surface through Qt's container mechanism, are all first confirmed by
the Operator's real build, not by this authoring pass.

---

## 12. AA Runbook D Attempt 1 correction round ("minimum delta only")

### 12.0 Scope disclosure

Unlike Rounds 1-5, this round's trigger was not Architecture Authority
reading the source - it was **Architecture Authority actually running
`Verification-RunbookD-v1.0.ps1` against a real Windows build for the first
time, and that run FAILING.** The AA's correction directive supplied the
authoritative diagnostic for each failure directly (the exact installed
`bgfx_shader.sh` path; the exact false-positive Check-3 directory
collapsing; the exact generated `CMakeCXXCompiler.cmake` contents; the
exact wrong/default vcpkg install-root behavior; the exact false-positive
architecture-checker matches; the exact clang-format line). Claude's task
this round was to apply the confirmed root-cause correction for each, not
to re-diagnose from scratch - and, per the directive, to make **no**
speculative change for the one item (RD1-07, clang-tidy) whose diagnostics
could not yet be reproduced.

### 12.1 Findings addressed

| ID | Title | Resolution | File(s) |
|----|-------|------------|---------|
| RD1-01 | shaderc include wiring (BUILD BLOCKER) | `src/viewport/bgfx/CMakeLists.txt` now derives the bgfx-installed shader-include directory from `VCPKG_INSTALLED_DIR`/`VCPKG_TARGET_TRIPLET` (fail-closed if either is undefined, or if `bgfx_shader.sh` is not found there), passes it to every `shaderc` invocation via `-i`, and adds the header itself to each shader's `DEPENDS`. Authoritative Windows profile (`s_5_0`), platform (`windows`), varying definition, and real shader compilation are all unchanged - no fake/precompiled binary was introduced. | `src/viewport/bgfx/CMakeLists.txt` |
| RD1-02 | Runbook Check 3 false-positive path enumeration | Replaced the `git status --porcelain` parse (which collapses an entirely-untracked directory into one collapsed line) with deterministic per-file enumeration: `git diff --name-only HEAD --` (tracked, modified-vs-HEAD) unioned with `git ls-files --others --exclude-standard --` (untracked, always per-file), normalized and compared against the unchanged, still-59-entry `$AuthorizedFootprint` exactly as before. The allowlist itself was not weakened. | `Verification-RunbookD-v1.0.ps1` |
| RD1-03 | Runbook Check 6 ("item 11" in the AA's own numbering) compiler identity | No longer requires `CMAKE_CXX_COMPILER` inside `CMakeCache.txt`. Now reads CMake's own generated `CMakeFiles/$ExpectedCMakeVersion/CMakeCXXCompiler.cmake` and fails closed on the exact `CMAKE_CXX_COMPILER` path (still cross-checked against the vswhere-resolved exact Hostx64/x64 cl.exe path, unchanged from Round 4), exact `CMAKE_CXX_COMPILER_ID` (`MSVC`), and exact `CMAKE_CXX_COMPILER_VERSION` (new parameter `-ExpectedMsvcCompilerVersion`, default `19.44.35228.0`, the value the AA's diagnostic supplied). | `Verification-RunbookD-v1.0.ps1` |
| RD1-04 | Runbook Checks 11-14 vcpkg installed state | Item 11+12 (package/portversion identity) now resolves `vcpkg.exe` explicitly and only from `$AcceptedVcpkgRoot` (no `$env:VCPKG_ROOT`/PATH fallback) and fails closed if the explicit install root (`$BuildDir/vcpkg_installed`) does not exist. Items 13/14 (per-feature presence/absence) no longer use `vcpkg list` at all - its default output does not print feature names - and instead parse `vcpkg_installed/vcpkg/status` directly via two new helpers (`Get-VcpkgStatusStanzas`, `Test-VcpkgPackageInstalled`), fail closed if the target package itself is not found installed (so an empty/missing package can never make a feature-absence check vacuously pass), and now also require bgfx's `tools` feature present (not previously checked) alongside the existing `multithreaded`-absent and qtbase `widgets`-present requirements. | `Verification-RunbookD-v1.0.ps1` |
| RD1-05 | Architecture checker (R7/R8/R10) lexical false positives | Added `bim_scan_files_for_boundary_tokens_ignoring_comments()`, which requires a match not be immediately preceded by an identifier character. R7 and R8's `"Handle("` token, and R10's `"bgfx::"`/`"BGFX_"` tokens, now go through this boundary-aware path; every other token in every rule (R1-R6, R9, R11, and the rest of R7/R8/R10's own sets) is completely unchanged. `bim::viewport_bgfx::Renderer` and other allowed adapter usage from `src/desktop/**` are no longer flagged; raw `bgfx`/OCCT `Handle(...)` usage outside the correct owner is still caught (reasoned against the existing negative fixtures - not re-executed, no execution channel). | `tools/architecture_checker.cmake` |
| RD1-06 | clang-format violation | Wrapped `Renderer::Resize()`'s declaration (previously a single line exceeding the column limit) onto multiple lines, matching this same file's existing wrapped-declaration style used by `CreateMesh()`/`RenderFrame()` immediately below it. No other formatting in the file was touched. | `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp` |
| RD1-07 | clang-tidy (4 diagnostics) | **Explicitly deferred per the AA's own instruction.** The diagnostic that reached this round could not reproduce/capture the four clang-tidy diagnostics' exact text because it did not establish the Runbook's process-local LLVM environment. No speculative source change was made. Architecture Authority will run targeted Windows verification and classify any remaining real diagnostics from exact output in a future round. | (none - deferred) |

### 12.2 What this round did not do

- Did not touch any file outside the six listed above (4 code/script files
  plus this document and its JSON companion).
- Did not modify `src/viewport/bgfx/shaders/vs_p0_t003.sc`,
  `fs_p0_t003.sc`, or `varying.def.sc` - RD1-01 is a CMake wiring fix, not a
  shader-source change; the `#include <bgfx_shader.sh>` lines that
  triggered the build blocker were already correct as authored.
- Did not touch `src/viewport/bgfx/src/renderer.cpp` or
  `src/viewport/bgfx/src/renderer_impl.hpp` - RD1-06 is isolated to
  `renderer.hpp`'s own declaration formatting.
- Did not make any speculative change for RD1-07 (clang-tidy) - see 12.1's
  table row.
- Did not touch any of the R1-R6/R9/R11 architecture-checker rules, or any
  token in R7/R8/R10 other than the two RD1-05 identifies.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.
- Did not re-run, and does not claim, a PASS for
  `Verification-RunbookD-v1.0.ps1` - Runbook D Attempt 1 is recorded as
  **FAIL** (see 12.4), and remains unrun by this session.

### 12.3 Exact changed-path summary

**4 of the 57 non-handover authorized paths were touched this round:**
`src/viewport/bgfx/CMakeLists.txt` (RD1-01),
`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp` (RD1-06),
`tools/architecture_checker.cmake` (RD1-05),
`Verification-RunbookD-v1.0.ps1` (RD1-02, RD1-03, RD1-04). Plus this
document and its JSON companion - **6 of the 59 authorized paths touched
this round.**

Cumulative across all rounds to date: **13 of the 59 authorized paths have
now been touched at least once** (the 9 touched through Round 5, plus the 3
newly-touched-this-round non-handover files -
`src/viewport/bgfx/CMakeLists.txt`,
`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`,
`tools/architecture_checker.cmake` - `Verification-RunbookD-v1.0.ps1` and
the handover pair were already in that touched set); **46 of the 59
authorized paths remain byte-for-byte unmodified** since the original
candidate.

Per-file updated byte size and SHA256 for all six files touched this round
are in the JSON companion's `footprint.modify`/`footprint.add` arrays
(`aa_runbook_d_attempt_1_findings_addressed` tag on each updated entry);
full finding-by-finding detail is in
`aa_runbook_d_attempt_1_corrections.findings_addressed`.

### 12.4 Runbook D Attempt 1 result and verification status (unchanged posture, restated)

**Runbook D Attempt 1: FAIL.** This is Architecture Authority's own report
of a real run against a real Windows build - it is recorded here as a fact,
not inferred or reconstructed by this session. This document and its JSON
companion do not, and must not be read to, claim a PASS for that attempt or
for any subsequent run of `Verification-RunbookD-v1.0.ps1`.

Separately, and unchanged from every prior round: this session still has no
command-execution channel to the Windows task worktree. Nothing this round
touched has been compiled, linked, or run BY THIS SESSION - the corrected
shaderc wiring (RD1-01), the corrected Runbook checks (RD1-02/03/04), the
corrected architecture-checker matching (RD1-05), and the corrected
formatting (RD1-06) are all, like every prior round's authoring, text
changes reasoned from the AA's own authoritative diagnostic, not
independently re-executed. In particular: whether `-i "<dir>"` is genuinely
the correct shaderc include-path flag/syntax for the exact bundled shaderc
version; whether `CMakeCXXCompiler.cmake`'s `set(NAME "value")` line syntax
is byte-for-byte what this round's regexes assume; and whether
`vcpkg_installed/vcpkg/status`'s stanza shape is exactly as this round's
parser assumes; are all first confirmed by the Operator's next real run,
not by this authoring pass. **This document does not claim, and must not be
read as claiming, that `Verification-RunbookD-v1.0.ps1` has passed.**

---

## 13. AA RD1.1 final arch-checker correction round ("minimum delta only")

### 13.0 Scope disclosure

This round's trigger, like the RD1 round before it, was a real executed
result, not a source review: Architecture Authority ran the RD1-05
-corrected `tools/architecture_checker.cmake` against an isolated copy of
the current candidate. `GEOMETRY_OCCT_ONLY_KERNEL_OWNER` and
`VIEWPORT_PUBLIC_NEUTRAL` both PASSED - direct, real confirmation that
RD1-05's "Handle(" boundary fix works as intended, for the first time
independently of this session's own reasoning. `BGFX_VIEWPORT_OWNER`
FAILED with exactly one deterministic false positive, diagnosed precisely
by the AA down to the exact file, exact string, and exact root cause (case
-folding of the `BGFX_` token). This round's task was narrowly to correct
that one root cause without touching anything else RD1-05 already fixed
correctly.

### 13.1 Finding addressed

| ID | Title | Resolution | File(s) |
|----|-------|------------|---------|
| RD1-05A | R10 "BGFX_" case-folding false positive | Added `bim_scan_files_for_case_sensitive_boundary_tokens_ignoring_comments()` - identical to RD1-05's boundary-matching helper (comment-stripped first, same identifier-boundary rule, same leading sentinel) except it does not lowercase file content or token, so matching is exact-case. R10 now routes `"BGFX_"` through this new exact-case path while `"bgfx::"` stays on the original, unchanged case-insensitive boundary path. `"Handle("` (R7/R8) is untouched by this correction - still case-insensitive, as the AA's instruction explicitly requires. `src/desktop/src/evidence_mode.cpp`'s legitimate `"bgfx_version_frozen_baseline"` field key is lowercase and no longer matches the now-exact-case `"BGFX_"` token; a real macro use such as `BGFX_STATE_DEFAULT` outside `src/viewport/bgfx/**` still matches (exact case, boundary-correct) and still fails R10. | `tools/architecture_checker.cmake` |

Confirmed-unaffected behavior (verified by re-reading the affected code
paths, not re-executed - no execution channel this session):
- `bim::viewport_bgfx::Renderer` and other adapter usage under
  `src/desktop/**`: still PASSES R10 (the case-insensitive `"bgfx::"` path,
  unchanged, still excludes it via the identifier-boundary rule - `"bgfx::"`
  is preceded by `"_"`).
- `tests/fixtures/p0_t003_bad_bgfx_owner`'s `bgfx::touch(0);`: still FAILS
  R10 (same unchanged case-insensitive `"bgfx::"` path; preceded by
  whitespace, not an identifier character).
- `windowHandle()` / `RenderMeshHandle(...)`: still PASS R7/R8 (the
  `"Handle("` boundary path is untouched by this correction).
- Standalone OCCT `Handle(...)`: still FAILS R7/R8 (same, untouched).

### 13.2 What this round did not do

- Did not modify `src/desktop/src/evidence_mode.cpp` or any other source
  file - the false positive was the checker's own matching logic, not
  anything wrong with the scanned code, and the AA's instruction explicitly
  forbade modifying the fixture/source to hide it.
- Did not make `"BGFX_"` case-insensitive (the opposite of what was asked)
  and did not weaken `"<bgfx/"` or raw `"bgfx::"` detection - both are
  byte-for-byte unchanged from RD1-05.
- Did not touch `"Handle("` boundary matching (R7/R8) at all - it remains
  case-insensitive, as instructed.
- Did not touch any of R1-R6, R9, or R11.
- Did not re-open or modify anything already source-accepted from RD1
  (RD1-01/02/03/04/06) - `src/viewport/bgfx/CMakeLists.txt`,
  `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`, and
  `Verification-RunbookD-v1.0.ps1` are untouched this round.
- Did not make any change for RD1-07 (clang-tidy) - still deferred.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.
- Did not re-run, and does not claim, a PASS for
  `Verification-RunbookD-v1.0.ps1` - Runbook D Attempt 1 remains recorded as
  **FAIL** (section 12.4, unchanged by this round).

### 13.3 Exact changed-path summary

**The exact 3-path delta for this round:** `tools/architecture_checker.cmake`
(RD1-05A), `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. No other path - of the 59
authorized, or otherwise - was touched this round. This round did not
re-touch `src/viewport/bgfx/CMakeLists.txt`,
`src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`, or
`Verification-RunbookD-v1.0.ps1` (all three source-accepted per the AA's
own instruction, RD1-01/02/03/04/06 - left exactly as delivered in the RD1
round), and did not touch `src/desktop/src/evidence_mode.cpp` (the file
whose content triggered the false positive - the defect was in the checker,
not the source).

RD1 source review status as of this round: **RD1-01/02/03/04/06
source-accepted; RD1-05A applied (closes RD1-05's residual false
positive); RD1-07 deferred.**

Per-file updated byte size and SHA256 for `tools/architecture_checker.cmake`
and the handover pair are in the JSON companion's
`footprint.modify`/`footprint.add` arrays (`aa_rd1_1_findings_addressed`
tag on each updated entry); full finding detail is in
`aa_rd1_1_corrections.findings_addressed`.

### 13.4 Verification status (unchanged posture, restated)

Runbook D Attempt 1 remains recorded as **FAIL** (section 12.4) - this
round did not re-run Runbook D and does not claim a PASS for it. This
session still has no command-execution channel to the Windows task
worktree; RD1-05A's fix was authored by reasoning from Architecture
Authority's own executed-checker diagnostic (a real result this session
did not itself produce), not independently re-run. Specifically
unverified by this session and first confirmed only by the Operator's next
real `ctest -R "^arch_bgfx"` (or full `^arch_`) run: whether
`bim_scan_files_for_case_sensitive_boundary_tokens_ignoring_comments()`'s
CMake regex syntax (`[^A-Za-z0-9_]`, the optional-whitespace-before-"("
allowance) behaves identically to its case-insensitive sibling once
actually executed by CMake, and whether any other exact-case `BGFX_` macro
use elsewhere in the real `src/` tree (outside this task's own 59-path
footprint, and therefore never read by this session) exists and would now
correctly fail R10. **This document does not claim, and must not be read
as claiming, that `Verification-RunbookD-v1.0.ps1` or any individual
architecture-checker CTest has passed.**

---

## 14. AA RD1.2 shader varying definition correction round ("minimum delta only")

### 14.0 Scope disclosure

This round's trigger is the furthest real, executed progress on P0-T003 to
date: Architecture Authority's **Targeted Verification Attempt 1** passed
seven consecutive gates - disk gate; the exact 59-path Git authority gate;
exact Windows toolchain bootstrap; the production architecture checker
(confirming RD1.1's `BGFX_` case-sensitivity fix, RD1-05A, is correct in a
real, non-isolated run); negative bgfx fixture rejection; RD1.1's
case-sensitive `BGFX_` synthetic regressions; and incremental CMake
configure (confirming RD1-01's shaderc include-directory wiring is
correct - `shaderc` now resolves `<bgfx_shader.sh>`). It then FAILED at the
next gate, fresh vertex shader compilation, with a real D3DCompile
diagnostic (`error X3004: undeclared identifier 'a_position'`) and a real
HLSL excerpt showing `a_normal` declared as an input but `a_position`
missing. As with every AA-executed-run finding so far, the diagnostic
supplied the confirmed root cause directly - a documented `shaderc`
contract this task's authoring did not previously know/apply (`varying.def.sc`
must be declaration-only, no comments) - so this round's task was to apply
that one correction, not to re-diagnose.

### 14.1 Finding addressed

| ID | Title | Resolution | File(s) |
|----|-------|------------|---------|
| RD1.2 | Shader varying definition contained comments, breaking shaderc's `a_position` registration | `src/viewport/bgfx/shaders/varying.def.sc` rewritten to be declaration-only: the same three declarations (`vec3 a_position : POSITION;`, `vec3 a_normal : NORMAL;`, `vec3 v_normal : NORMAL = vec3(0.0, 0.0, 1.0);`), same attribute/varying names, same semantics, same `v_normal` default value - every comment line and blank line removed. Nothing about the vertex layout, the attribute set, or the varying semantics changed - only the file's comment/prose content was removed. | `src/viewport/bgfx/shaders/varying.def.sc` |

### 14.2 What this round did not do

- Did not modify `src/viewport/bgfx/shaders/vs_p0_t003.sc` or
  `fs_p0_t003.sc` - both already correctly reference `a_position`/`a_normal`/
  `v_normal` (confirmed by re-reading `vs_p0_t003.sc`'s `$input a_position,
  a_normal` / `$output v_normal` directives before this edit); the defect
  was entirely in `varying.def.sc`'s comment content, not in either shader
  source.
- Did not change the vertex layout (`PosNormalVertex` in
  `renderer_impl.hpp`/`renderer.cpp` - untouched, not part of this round's
  footprint).
- Did not change shaderc's flags, profile (`s_5_0`), or platform
  (`windows`) in `src/viewport/bgfx/CMakeLists.txt` - that file needed no
  further change this round; RD1-01's fix from the prior round is
  runtime-confirmed working (shaderc successfully resolves
  `<bgfx_shader.sh>`), so it was left exactly as delivered.
- Did not rename any attribute or varying.
- Did not introduce any generated or precompiled shader binary - the
  `.bin` outputs remain something only a real `shaderc` invocation
  produces, never authored or faked by this session.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR.
- Did not touch any path outside the authorized 59.
- Did not run Full Runbook D and does not claim a shader/build PASS -
  Targeted Verification Attempt 1 is recorded as **FAIL** (section 14.4).

### 14.3 Exact changed-path summary

**The exact 3-path delta for this round:**
`src/viewport/bgfx/shaders/varying.def.sc`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. No other path - of the 59
authorized, or otherwise - was touched this round.

### 14.4 Targeted Verification Attempt 1 result and verification status (unchanged posture, restated)

**Targeted Verification Attempt 1: FAIL, at fresh vertex shader
compilation, after seven prior gates PASSED.** This is Architecture
Authority's own report of a real run against a real Windows build - both
the seven passing gates and the shader-compile failure are recorded here
as facts this session did not itself produce, not inferred or
reconstructed. This document and its JSON companion do not, and must not
be read to, claim a shader PASS, a build PASS, or a Full Runbook D PASS
for this or any prior round.

Unchanged from every prior round: this session still has no
command-execution channel to the Windows task worktree. The RD1.2
correction is, like every prior round's authoring, a text change reasoned
from the AA's own authoritative diagnostic (the exact D3DCompile error, the
exact incomplete HLSL excerpt, and the exact documented `shaderc` contract
requirement), not independently re-executed. In particular: whether a
declaration-only `varying.def.sc` (with the exact formatting this round
produced) is sufficient on its own to make `shaderc` correctly register
`a_position`, versus some other latent cause also being present, is first
confirmed by the Operator's next real run, not by this authoring pass.
**This document does not claim, and must not be read as claiming, that any
shader has compiled, that the build has succeeded, or that
`Verification-RunbookD-v1.0.ps1` has passed.**

---

## 15. AA RD1.3 MSVC/bgfx + nodiscard build conformance correction round ("strict minimum delta only")

### 15.0 Scope disclosure

This round's trigger is Architecture Authority's classification of
**Targeted Full Project Build v3**: a Visual Studio environment bootstrap
and a standalone C++ standard-library probe both PASSED, closing out the
prior rounds' missing-standard-header failures as environment-only, not
source findings (no source change was made or is needed for that closure).
The authoritative full build then reached two confirmed project-level
findings - RD1.3-01 (a real MSVC/bx compiler-option requirement) and
RD1.3-02 (a real `/WX`-promoted discarded-`[[nodiscard]]`-result error at
six specific, AA-identified lines across two files). Both diagnostics were
supplied with exact error codes, exact file paths, and exact line numbers;
this round's task was to apply the confirmed correction for each, not to
re-diagnose. RD1.2's shader correction (`varying.def.sc`) is independently
verified and CLOSED - not reopened, not touched, exactly as instructed.

### 15.1 Findings addressed

| ID | Title | Resolution | File(s) |
|----|-------|------------|---------|
| RD1.3-01 | bgfx/bx MSVC preprocessor requirement (BUILD BLOCKER) | Added `if(MSVC) target_compile_options(bim_viewport_bgfx PRIVATE /Zc:preprocessor) endif()` immediately after `target_compile_features(bim_viewport_bgfx PRIVATE cxx_std_20)` and before `bim_apply_warnings(bim_viewport_bgfx)`, using the AA's own preferred minimum-delta form verbatim (no established local `if(MSVC)` pattern was found elsewhere in this session's locally-staged files to prefer instead). Scoped to the `bim_viewport_bgfx` target only - not global, not `CMAKE_CXX_FLAGS`, per R10's "sole bgfx/bx owner" rule (the only target that can ever compile a bx/bgfx header). bgfx/bx/vcpkg source was not modified; the bx check itself was not suppressed; no installed dependency file was patched; no compiler/toolchain version was changed. | `src/viewport/bgfx/CMakeLists.txt` |
| RD1.3-02 | Discarded `[[nodiscard]]` results under `/WX` (C4834 -> C2220) | In both files' `MakeConfiguredCamera()` helper, each of the three discarded calls (`SetLookAt`/`SetPerspective`/`SetAspectRatio`, all `[[nodiscard]] bim::viewport::Status`) is now wrapped in `REQUIRE(...IsOk())`. This consumes the `[[nodiscard]]` result (resolving C4834/C2220) via a meaningful assertion of the helper's actual contract - that it returns a validly-configured `Camera`, which every `TEST_CASE` calling it implicitly depends on (see `Camera::IsConfigured()`) - rather than a `static_cast<void>(...)` that would silence the warning without expressing that intent. No assertion elsewhere in either file was weakened; no `[[nodiscard]]` annotation was removed; `/WX` was not disabled; no warning suppression was added. | `tests/unit/unit_viewport_ray.cpp`, `tests/integration/integration_viewport_bgfx_headless.cpp` |

### 15.2 Exact handling of each previously-discarded `[[nodiscard]]` result

`tests/unit/unit_viewport_ray.cpp`, `MakeConfiguredCamera()` (was lines 19-21, AA-reported):
- `camera.SetLookAt({0, -5, 0}, {0, 0, 0}, {0, 0, 1});` -> `REQUIRE(camera.SetLookAt({0, -5, 0}, {0, 0, 0}, {0, 0, 1}).IsOk());`
- `camera.SetPerspective(0.9, 0.01, 1000.0);` -> `REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());`
- `camera.SetAspectRatio(1.0);` -> `REQUIRE(camera.SetAspectRatio(1.0).IsOk());`

`tests/integration/integration_viewport_bgfx_headless.cpp`, `MakeConfiguredCamera()` (was lines 22-24, AA-reported):
- `camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1});` -> `REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());`
- `camera.SetPerspective(0.9, 0.01, 1000.0);` -> `REQUIRE(camera.SetPerspective(0.9, 0.01, 1000.0).IsOk());`
- `camera.SetAspectRatio(16.0 / 9.0);` -> `REQUIRE(camera.SetAspectRatio(16.0 / 9.0).IsOk());`

All six literal argument sets are byte-for-byte unchanged from before this
round - only the discarded call became an asserted one. No test's expected
behavior, camera parameters, or downstream `TEST_CASE` assertions were
altered.

### 15.3 What this round did not do

- Did not modify `src/viewport/bgfx/shaders/varying.def.sc` (RD1.2,
  independently verified and CLOSED) - explicitly out of scope per
  instruction.
- Did not add `/Zc:preprocessor` globally, to `CMAKE_CXX_FLAGS`, or to any
  target other than `bim_viewport_bgfx`.
- Did not modify any bgfx/bx/vcpkg source, patch any installed dependency
  file, suppress the bx `C1189` check, or change any compiler/toolchain
  version.
- Did not remove any `[[nodiscard]]` annotation, remove `/WX`, disable
  `C4834`, or add any compiler warning suppression.
- Did not use a cast-to-void or any other discard mechanism where the
  result had testable semantics - every discarded result identified had a
  meaningful pass/fail outcome and is now asserted, not silenced.
- Did not touch any path outside the three authorized production/test
  paths plus the handover pair - no additional path was found genuinely
  required; nothing was reported to Architecture Authority as needing an
  out-of-scope change.
- Did not perform discovery, redesign, or architecture change of any kind.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR.
- Did not run Full Runbook D and does not claim an authoritative
  build/test PASS.

### 15.4 Local verification performed (engineer-reported, NON-AUTHORITATIVE)

No C++ compiler, MSVC toolchain, or bgfx/Qt/vcpkg dependency tree exists in
this session's own environment (Linux, no execution channel to the Windows
task worktree - unchanged, standing fact since Round 1). No actual compile
or build was attempted or is claimed. The following static, non-executing
checks were performed instead, and are disclosed as engineer-reported and
non-authoritative - Architecture Authority's own independent execution is
the only authoritative verification:

- Re-read `bim/viewport/camera.hpp` directly to confirm `SetLookAt`,
  `SetPerspective`, and `SetAspectRatio` are declared
  `[[nodiscard]] Status` (not, e.g., `void` or a different return type),
  and that `Status::IsOk()` exists with the exact signature used in the
  new `REQUIRE(...)` calls - confirming the fix targets the actual
  declared contract rather than an assumed one.
- A brace/paren balance sweep (`{}`/`()`, matched `if(`/`endif(` and
  `function(`/`endfunction(` counts for the CMake file) across all three
  touched source/script files, confirming no stray unbalanced token was
  introduced by this round's edits - the same discipline used in every
  prior round.
- Re-read the full, unmodified remainder of both test files to confirm no
  other `TEST_CASE`, assertion, or expected value was altered - only the
  three-line `MakeConfiguredCamera()` body in each file changed.

None of this constitutes compilation, linking, or execution. Whether
`/Zc:preprocessor` actually resolves the real MSVC `C1189` failure, and
whether the six new `REQUIRE(...)` calls actually compile clean under
`/WX` (no new warning introduced by the assertion itself), are both first
confirmed by the Operator's/Architecture Authority's next real build - not
by this authoring pass. **This document does not claim, and must not be
read as claiming, that any build, compile, or test has authoritatively
passed.**

---

## 16. AA RD1.4 Renderer MSVC/build conformance correction round ("strict minimum delta only")

### 16.0 Scope disclosure

This round's trigger is Architecture Authority's classification of
**Targeted Full Project Build v5**. RD1.3's two findings are explicitly
closed and were not reopened: `/Zc:preprocessor` (RD1.3-01) runtime-
confirmed present in the effective compile command, C1189 gone; RD1.3-02 in
`unit_viewport_ray.cpp` runtime-confirmed fixed (target compiled and
linked). The authoritative build then reached three new, confirmed
project-level findings - RD1.4-01, RD1.4-02, RD1.4-03 - all located in
`src/viewport/bgfx/src/renderer.cpp`, all supplied with exact error codes
and approximate line numbers. This round's task was to apply the confirmed
correction for each at its exact call/use site, not to re-diagnose, perform
discovery, or refactor surrounding logic. Authorized scope was capped at
exactly 3 paths: `src/viewport/bgfx/src/renderer.cpp` plus this document and
its JSON companion. `src/viewport/bgfx/CMakeLists.txt`,
`tests/unit/unit_viewport_ray.cpp`,
`tests/integration/integration_viewport_bgfx_headless.cpp`, and
`src/viewport/bgfx/shaders/varying.def.sc` were explicitly listed as
must-not-modify and were not touched. No additional path beyond the
authorized 3 was found genuinely necessary - all three findings were
resolvable entirely within `renderer.cpp`.

### 16.1 Findings addressed

| ID | Title | Resolution | File(s) |
|----|-------|------------|---------|
| RD1.4-01 | `std::fopen` MSVC C4996 under `/WX` (C2220, BUILD BLOCKER) | In `LoadCompiledShader()`, the single `std::fopen(path, "rb")` call is now guarded by `#if defined(_MSC_VER)`: the MSVC branch calls `fopen_s(&file, path, "rb")` and leaves `file` explicitly `nullptr` on any non-zero (failure) return; the non-MSVC branch keeps the original `std::fopen(path, "rb")` call byte-for-byte. The existing `if (file == nullptr) { return BGFX_INVALID_HANDLE; }` check immediately below is unchanged and now correctly catches failure from either branch. No existing local safe-file-open helper was found elsewhere in this file or in `renderer_impl.hpp` to reuse - `LoadCompiledShader()` is the file's only file-open call site, so the guard was added inline at that one call, which is also the smallest possible delta. `/WX` was not disabled, C4996 was not suppressed, `_CRT_SECURE_NO_WARNINGS` was not defined, and no other warning policy was changed. | `src/viewport/bgfx/src/renderer.cpp` |
| RD1.4-02 | `std::length_error` used without a direct `<stdexcept>` include (C2039) | Added `#include <stdexcept>` to the file's own include block (after `<new>`, before `<vector>`, preserving the existing alphabetical grouping of standard headers). Both existing `catch (const std::length_error&) {}` clauses (`Impl::ReleaseSlot`, `CreateMesh`) are byte-for-byte unchanged - this is a header-visibility fix only, not a behavioral change. `<new>` (already included, for `std::bad_alloc`) does not transitively guarantee `std::length_error`'s declaration on every standard-library implementation; relying on transitive inclusion from `<vector>` or elsewhere was exactly what AA's instruction said not to do. | `src/viewport/bgfx/src/renderer.cpp` |
| RD1.4-03 | `bx::kRadToDeg` does not exist in the installed bx API (C2039) | `const float fovy_degrees = static_cast<float>(camera.VerticalFovRadians()) * bx::kRadToDeg;` is replaced with `const float fovy_degrees = bx::toDeg(static_cast<float>(camera.VerticalFovRadians()));` - `camera.VerticalFovRadians()` remains the sole source value, `fovy_degrees` still feeds `bx::mtxProj(...)` exactly as before (same call, same other arguments, same handedness, same homogeneous-depth flag), so FOV semantics, camera contract, and projection convention are all unchanged. No hard-coded `180/pi`-style constant was introduced and no local `kRadToDeg` replacement was defined, per AA's explicit instruction. | `src/viewport/bgfx/src/renderer.cpp` |

### 16.2 Exact replacement detail

**RD1.4-01 (`LoadCompiledShader()`, was line 46):**
```cpp
// before
std::FILE* file = std::fopen(path, "rb");

// after
#if defined(_MSC_VER)
std::FILE* file = nullptr;
if (fopen_s(&file, path, "rb") != 0) {
    file = nullptr;
}
#else
std::FILE* file = std::fopen(path, "rb");
#endif
```
The subsequent `if (file == nullptr) { return BGFX_INVALID_HANDLE; }` line is
unchanged.

**RD1.4-02 (top-of-file includes, was after line 5):**
```cpp
// before
#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

// after
#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>
#include <vector>
```

**RD1.4-03 (`RenderFrame()`, was line 426):**
```cpp
// before
const float fovy_degrees = static_cast<float>(camera.VerticalFovRadians()) * bx::kRadToDeg;

// after
const float fovy_degrees = bx::toDeg(static_cast<float>(camera.VerticalFovRadians()));
```

### 16.3 What this round did not do

- Did not modify `src/viewport/bgfx/CMakeLists.txt`,
  `tests/unit/unit_viewport_ray.cpp`,
  `tests/integration/integration_viewport_bgfx_headless.cpp`, or
  `src/viewport/bgfx/shaders/varying.def.sc` - all explicitly listed as
  must-not-modify this round, all independently verified/runtime-confirmed
  and CLOSED in RD1.2/RD1.3.
- Did not disable `/WX`, suppress C4996, define
  `_CRT_SECURE_NO_WARNINGS`, or change any other global or per-target
  warning policy.
- Did not refactor `LoadCompiledShader()`'s surrounding shader/file-loading
  logic beyond the single guarded `fopen`/`fopen_s` call.
- Did not rely on transitive standard-library includes for
  `std::length_error` - added the direct `<stdexcept>` include instead.
- Did not introduce a hard-coded `180/pi`-style constant or a locally
  defined `kRadToDeg` replacement for RD1.4-03 - used the real bx API
  function `bx::toDeg(float)` instead.
- Did not change camera/FOV semantics, handedness, or projection
  conventions, and did not change the bx/bgfx dependency version.
- Did not touch any path outside the authorized 3-path RD1.4 delta - no
  additional path was found genuinely required; nothing was reported to
  Architecture Authority as needing an out-of-scope change.
- Did not perform discovery, redesign, or architecture change of any kind.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR.
- Did not run Full Runbook D and does not claim an authoritative
  build/test PASS.
- Did not rewrite, edit, or renumber any prior round's summary block or
  numbered section (0-15) - this round's changes are additive only (a new
  summary block before section 0, and this new section 16).

### 16.4 Local verification performed (engineer-reported, NON-AUTHORITATIVE)

No C++ compiler, MSVC toolchain, or bgfx/Qt/vcpkg dependency tree exists in
this session's own environment (Linux, no execution channel to the Windows
task worktree - unchanged, standing fact since Round 1). No actual compile
or build was attempted or is claimed. The following static, non-executing
checks were performed instead, and are disclosed as engineer-reported and
non-authoritative - Architecture Authority's own independent execution
(preferably `cmake --build build/ci-win-msvc --target bim_viewport_bgfx
--verbose`, per AA's instruction) is the only authoritative verification:

- Read the full, current content of `src/viewport/bgfx/src/renderer.cpp`
  (492 lines before this round's edits) end to end before making any
  change, confirming the exact prior content at each AA-reported
  approximate line number (the `std::fopen` call, both
  `catch (const std::length_error&)` clauses, and the `bx::kRadToDeg` use)
  and confirming no existing local safe-file-open helper was present to
  reuse for RD1.4-01.
- A brace/paren/bracket balance sweep (`{}`/`()`/`[]` counts, all balanced:
  73/73, 284/284, 23/23) and an `#if`/`#endif` pair check (exactly one
  matched pair, from the new RD1.4-01 guard) across the full edited file,
  confirming no stray unbalanced token was introduced - the same discipline
  used in every prior round.
- A targeted re-grep of the edited file for `kRadToDeg`, `fopen`,
  `length_error`, `#if`/`#endif`, and `stdexcept` to confirm: no remaining
  use of `bx::kRadToDeg` anywhere in the file; the MSVC/non-MSVC `fopen`/
  `fopen_s` branch structure is exactly as intended; both pre-existing
  `std::length_error` catch clauses are untouched; `<stdexcept>` is present
  in the include list.

None of this constitutes compilation, linking, or execution. Whether
`fopen_s`'s exact signature/behavior matches what this file now assumes,
whether `bx::toDeg(float)` is in fact the installed bx API's actual
replacement for the removed `kRadToDeg` constant (as AA's own diagnostic
states), and whether these three fixes clear the authoritative MSVC `/WX`
build with no further diagnostic in this file, are all first confirmed by
Architecture Authority's next real build - not by this authoring pass.
**This document does not claim, and must not be read as claiming, that any
build, compile, or test has authoritatively passed.**

---

## 17. AA RD1.5 exact clang-format 19.1.5 conformance correction round ("format-only") - 17.0-17.4: BLOCKED in Claude environment, no changes applied (historical, preserved); 17.5: OPERATOR_EXECUTED_PASS

### 17.0 Scope disclosure

This round's trigger is Architecture Authority's classification of the
authoritative clang-format gate: `clang-format --dry-run --Werror
--style=file`, run with **clang-format version 19.1.5**, checked 35 C/C++
candidate files project-wide and found exactly 25 failing formatting
conformance (10 already passed). AA authorized correction of exactly those
25 files, listed in full in AA's directive, plus the handover pair (27 max
paths total), with the mandatory constraint that the *only* permitted
production/test transformation is the exact output of `clang-format 19.1.5
-i --style=file` run per-file - explicitly not a manual approximation, not
a different clang-format version, and not a recursive/repo-wide run.

### 17.1 Tool-availability check performed before touching any file

Per AA's own explicit instruction ("If clang-format 19.1.5 is NOT available
in your environment: STOP. Do not manually approximate hundreds of
formatting edits. Report that exact-tool execution is unavailable to
Architecture Authority"), this session checked for the exact tool **before**
opening or editing any of the 25 authorized files:

- `clang-format --version` (the default `/usr/bin/clang-format`) reports
  **"Ubuntu clang-format version 18.1.3 (1ubuntu1)"** - installed, but not
  19.1.5.
- `dpkg -l` confirms only two clang-format packages are actually installed
  in this environment: `clang-format` (`1:18.0-59~exp2`, a version-agnostic
  wrapper) and `clang-format-18` (`1:18.1.3-1ubuntu1`). No `clang-format-19`
  package is installed.
- `apt-cache policy clang-format-19` shows `19.1.1-1ubuntu1~24.04.2` as an
  *available but not installed* candidate - a different patch version from
  the required `19.1.5`, and per AA's explicit "do not substitute another
  clang-format version" instruction, not an acceptable stand-in even if
  installed. `clang-format-14` through `clang-format-17` and
  `clang-format-20` are likewise only apt candidates, none matching
  `19.1.5`.
- `pip install clang-format` (unpinned, to check for any match at all) and
  `pip download clang-format==19.1.5` both returned "No matching
  distribution found" - no such package exists in this environment's
  reachable Python package index.
- `npm view clang-format-node versions` returned an HTTP 403 from this
  environment's package-registry proxy for that host.
- A direct HTTPS request for LLVM's official `llvmorg-19.1.5` GitHub release
  page (where a matching prebuilt binary would normally be published)
  returned HTTP 403 - `github.com` release downloads are not on this
  session's network allowlist. Per this environment's own standing rule
  against working around a blocked fetch, this was not retried through any
  other channel (no alternate mirror, no cache, no unblocking attempt).

**Conclusion: clang-format 19.1.5 is not available anywhere in this
session's environment**, and no equivalent-output substitute was
substituted, per instruction.

### 17.2 Disposition: STOPPED before any edit

Per AA's own explicit instruction for exactly this condition, this round
stopped **before** opening or modifying any of the 25 authorized C/C++
files:

- Zero of the 25 authorized files were run through any clang-format
  version, including the locally available 18.1.3.
- Zero manual/hand-edited formatting changes were made to approximate
  clang-format 19.1.5's output - explicitly forbidden by AA's instruction,
  and not attempted.
- Zero of the 10 already-passing files were touched (trivially true - no
  file in the 35-file candidate set was touched).
- No path outside the 27 max authorized paths was touched or considered.

**Only this document and its JSON companion (2 of the 27 max authorized
paths) are updated this round**, solely to record this blocked disposition
for the evidence trail - not to record any formatting correction, since
none was applied.

### 17.3 What this round did not do

- Did not run `clang-format` (any version) against any of the 25 authorized
  files, or against any other file.
- Did not manually approximate clang-format's formatting output by hand.
- Did not substitute clang-format 18.1.3 (or any other installed/
  candidate version) for the required 19.1.5.
- Did not modify `.clang-format`.
- Did not change any behavior, semantics, identifiers, expressions,
  constants, comments' meaning, test logic, lifecycle behavior, camera/FOV
  behavior, viewport behavior, bgfx behavior, Qt behavior, error handling,
  include sets, CMake, shaders, dependencies, the architecture checker, or
  warning flags (`/WX` untouched) - none of the 25 files were opened for
  editing at all.
- Did not touch any path outside the handover pair this round.
- Did not run clang-tidy, Full Runbook D, or any other review.
- Did not stage, `git add`, or commit anything - no git-execution channel
  exists this session, same as every prior round.
- Did not touch `main`.
- Did not raise an ACR - this is a tool-availability blocker, not an
  architecture or design finding.
- Did not rewrite, edit, or renumber any prior round's summary block or
  numbered section (0-16) - this round's changes are additive only (a new
  summary block before section 0, and this new section 17).

### 17.4 Local verification performed (engineer-reported, NON-AUTHORITATIVE)

- Ran `which clang-format` and `clang-format --version` - confirmed
  18.1.3 is the only version on `PATH`.
- Ran `dpkg -l | grep -i clang-format` - confirmed only `clang-format`
  (`1:18.0-59~exp2`) and `clang-format-18` (`1:18.1.3-1ubuntu1`) are
  installed; attempted invocation of `clang-format-14` through
  `clang-format-17` confirmed each is "command not found" (not installed).
- Ran `apt-cache policy clang-format-19` and `apt list --all-versions
  clang-format*` - confirmed the only 19.x candidate available through this
  environment's apt sources is `19.1.1-1ubuntu1~24.04.2`, not installed and
  not version 19.1.5.
- Attempted `pip install clang-format` (unpinned) and `pip download
  clang-format==19.1.5` - both reported no matching distribution.
- Attempted `npm view clang-format-node versions` - HTTP 403 from the
  registry proxy.
- Attempted a direct HTTPS request to
  `github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.5` - HTTP 403
  (host not on this environment's allowlist); not retried via any
  alternate route, per this environment's standing policy against working
  around a blocked fetch.

None of this constitutes a formatting run, a compile, a link, or an
execution of application code - it is exclusively tool-discovery evidence
supporting the conclusion that the exact required tool is unavailable.
**This document does not claim, and must not be read as claiming, that
clang-format 19.1.5 was run, that any of the 25 authorized files were
reformatted, or that the authoritative clang-format gate's finding (25
of 35 files failing) has been addressed in any way this round.** Applying
the correction remains pending either Architecture Authority independently
running clang-format 19.1.5 directly, or this session being supplied with
that exact binary/package for a future round.

### 17.5 Operator completion (handover-only round; 17.0-17.4 above preserved unchanged as historical evidence)

**Sequence of events, both preserved:** (1) this session's RD1.5 delivery
correctly stopped as `BLOCKED_TOOL_UNAVAILABLE` (17.1-17.4 above) because
exact clang-format 19.1.5 was not available in this environment; (2)
Architecture Authority then authorized the **Windows Execution Operator**
to perform the exact format correction on the authoritative Windows
operator environment using the locked Windows toolchain, and the Operator
has completed it. This subsection records event (2). It does not rewrite
event (1): this session's environment did not, and still has not, performed
any format operation on any source/test file.

**Everything below is supplied by Architecture Authority from the
Operator's run. None of it was independently verified by this session,
which has no execution channel to the Windows worktree and did not read
the Operator's evidence files.**

Operator run folder:
`C:\Users\abdallah\Downloads\P0-T003-RD1-5-OPERATOR\RD1-5-FORMAT-20260912-153631-51823c4f`
(pre-format backup in `pre-format-backup\`; `pre-format-hashes.tsv`;
`post-format-hashes.tsv`; `result.json` - all under that folder).
Authoritative pre-RD1.5 snapshot: `P0-T003-RD1-4-CORRECTION-20260912-150615`.

Exact formatter:
`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin\clang-format.exe`,
**clang-format version 19.1.5**, invoked as `-i --style=file` per file.

Pre-format checks (Operator): authority lock PASS; branch/HEAD/tree matched
the AA locks (`task/P0-T003-desktop-viewport-spike`, HEAD
`82367706691567b0148ee0e7b4eac3a3e30b33d1`, tree
`0f5845e02a5f48d64a56f467c26d5ac526bf61dc`); staged paths = 0; `main`
HEAD/tree matched (`3f2230be5fcd796c370f485975547112ad52d2e3` /
`34b0d7f5c0bd75df58e8a12578fc37351e137ad8`); `main` clean = True; RD1.4
authoritative manifest PASS; candidate boundary exactly 59 paths;
pre-format delta vs RD1.4 exactly 2 paths (`CLAUDE_HANDOVER.md`,
`CLAUDE_HANDOVER.json` - i.e. this session's blocked-round handover
update); all 25 authorized format targets byte-identical to their RD1.4
authoritative hashes before formatting.

Exactly the 25 files formatted (no other source/test file was formatted):

1. `src/desktop/src/evidence_mode.cpp`
2. `src/desktop/src/main.cpp`
3. `src/desktop/src/main_window.cpp`
4. `src/desktop/src/main_window.hpp`
5. `src/desktop/src/spike_scene.cpp`
6. `src/desktop/src/spike_scene.hpp`
7. `src/desktop/src/viewport_bridge.cpp`
8. `src/desktop/src/viewport_bridge.hpp`
9. `src/desktop/src/viewport_window.cpp`
10. `src/desktop/src/viewport_window.hpp`
11. `src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp`
12. `src/viewport/bgfx/src/renderer.cpp`
13. `src/viewport/include/bim/viewport/camera.hpp`
14. `src/viewport/include/bim/viewport/error.hpp`
15. `src/viewport/include/bim/viewport/lifecycle.hpp`
16. `src/viewport/include/bim/viewport/mesh.hpp`
17. `src/viewport/include/bim/viewport/ray.hpp`
18. `src/viewport/src/camera.cpp`
19. `src/viewport/src/ray.cpp`
20. `tests/integration/integration_viewport_bgfx_headless.cpp`
21. `tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp`
22. `tests/unit/unit_viewport_camera.cpp`
23. `tests/unit/unit_viewport_lifecycle.cpp`
24. `tests/unit/unit_viewport_mesh_contract.cpp`
25. `tests/unit/unit_viewport_ray.cpp`

Post-format result (Operator): `clang-format -i` 25/25 completed
successfully; authoritative post-format `clang-format 19.1.5 --dry-run
--Werror --style=file` recheck **25/25 PASS**; exact post-format delta vs
RD1.4 **27 paths exactly** (the 25 files above + `CLAUDE_HANDOVER.md` +
`CLAUDE_HANDOVER.json`), no additional paths; staged paths = 0; `main`
UNCHANGED + CLEAN; task HEAD and tree unchanged (no commit/staging occurred).

**Status recorded:**

| Item | Status |
|------|--------|
| RD1.5 format execution | **OPERATOR_EXECUTED_PASS** |
| RD1.5 authoritative AA delta/source audit | PENDING |
| Authoritative clang-format recheck by AA | PENDING (Operator's own 25/25 PASS recorded above is the Operator's result, not AA's independent re-audit) |
| Post-RD1.5 Targeted Full Project Build re-certification | PENDING |
| Post-RD1.5 Architecture CTests re-certification | PENDING |
| clang-tidy gate | NOT AUTHORIZED / PENDING |
| Full Runbook D Attempt 2 | NOT AUTHORIZED |
| Kimi | NOT AUTHORIZED |
| Implementation commit | FORBIDDEN |
| ACR | NONE |

**Explicitly NOT post-RD1.5 certification:** the Targeted Full Project
Build v6 PASS and Architecture CTests 12/12 PASS recorded before RD1.5 were
achieved against the pre-format RD1.4 bytes. The 25 files above now carry
new (formatted) bytes, so Architecture Authority will independently rerun,
in order: the RD1.5 authoritative delta/source audit, the authoritative
clang-format recheck, Targeted Full Project Build re-certification,
Architecture CTests re-certification, and the clang-tidy gate. Only after
those pass may AA consider Full Runbook D Attempt 2. No prior PASS is being
carried forward as applying to the formatted bytes.

**What this handover-only round did:** modified exactly 2 paths -
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md` (this summary-block update, the
section 17 title suffix, and this subsection 17.5; 17.0-17.4 and every
earlier section untouched) and `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`
(new `operator_completion` object inside `aa_rd1_5_corrections`, new
`verification_status.rd1_5_operator_completion_result`, per-entry
annotation on the 25 formatted files' footprint entries, `.md` entry
size/hash refresh). It wrote **zero** source/test files: the 25 formatted
files were not opened for editing, not resent, not reformatted, not
normalized - they already contain the Operator's authoritative output on
the Windows worktree. No staging, no `git add`, no commit; `main` untouched;
no formatter run; no clang-tidy; no Full Runbook D; no Kimi.

**Standing warning for any future round (important):** this session's
*local staging copies* of the 25 files above are now **stale** - they hold
the pre-format RD1.4 bytes, not the Operator's formatted bytes. Their
`size_bytes`/`sha256` in the JSON companion's footprint likewise still
describe the pre-format RD1.4 bytes (annotated as such there; the
authoritative post-format hashes live in the Operator's
`post-format-hashes.tsv`, which this session did not receive and does not
reproduce). Any future round that needs to edit one of these 25 files must
first re-stage the current file from the Windows worktree and must never
deliver from the stale local copy - doing so would silently revert the
Operator's formatting. This was deliberately not "fixed" this round: no
source/test write of any kind was authorized.

---

## 18. AA RD1.6 clang-tidy source correction round ("minimum delta only") - DELIVERED_FOR_AA_VERIFICATION

### 18.0 Scope disclosure

Trigger: Architecture Authority's **clang-tidy v6 = TRUE SOURCE FAIL**
(tool clang-tidy 19.1.5; 17 exact translation units; 60 raw project
diagnostic lines; 19 unique source findings), reported after the RD1.5
delta/source audit PASS, clang-format 19.1.5 35/35 PASS, Targeted Full
Build v7 PASS and Architecture CTests 12/12 PASS - all AA-supplied values,
none independently verified by this session (no execution channel to the
Windows worktree; no MSVC/Qt/bgfx toolchain here). Authoritative RD1.5
snapshot: `P0-T003-RD1-5-CORRECTION-20260912-154503`; task HEAD
`82367706691567b0148ee0e7b4eac3a3e30b33d1` (AA-supplied).

Authorized: exactly 11 source paths (`src/desktop/src/evidence_mode.cpp`,
`src/desktop/src/main.cpp`, `src/desktop/src/main_window.hpp`,
`src/desktop/src/spike_scene.hpp`, `src/desktop/src/viewport_window.cpp`,
`src/desktop/src/viewport_window.hpp`, `src/viewport/bgfx/src/renderer.cpp`,
`src/viewport/include/bim/viewport/error.hpp`,
`src/viewport/include/bim/viewport/input.hpp`,
`src/viewport/include/bim/viewport/lifecycle.hpp`,
`src/viewport/include/bim/viewport/mesh.hpp`) plus the handover pair - 13
max. Exactly those 13 were touched; zero others. No additional path was
found necessary.

**Stale-copy handling (per 17.5's standing warning):** before any edit, all
11 authorized files were re-staged from the Windows worktree and their
SHA-256 compared against this session's local copies. 10 of 11 differed
(they were among RD1.5's 25 Operator-formatted files; this session's copies
were pre-format); `input.hpp` was identical (one of RD1.5's 10
already-passing files). The re-staged, Operator-formatted bytes were
adopted as the editing baseline for every file; the stale copies were
discarded. Pre-edit worktree hashes and staging mtimes are recorded in the
JSON companion (`aa_rd1_6_corrections.pre_edit_worktree_baseline`) and the
delivery used those mtimes as `expectedMtimeMs` guards so a newer worktree
change could not be silently overwritten.

### 18.1 Findings addressed

| ID | clang-tidy check | Correction | File(s) |
|----|------------------|------------|---------|
| RD1.6-01 | readability-braces-around-statements | Braces added to exactly the six reported single-statement `if` bodies: `evidence_mode.cpp` `JsonValue::WriteTo` Array branch (`if (i + 1 < elements_.size()) out << ',';`) and Object branch (`if (i + 1 < members_.size()) out << ',';`); `renderer.cpp` `CreateMesh()` failure path (`if (bgfx::isValid(slot.vertex_buffer)) bgfx::destroy(...)`, `if (bgfx::isValid(slot.index_buffer)) bgfx::destroy(...)`) and `DestroyMesh()` (same two). Conditions and statements byte-for-byte unchanged; no branches combined; nothing else touched. | `evidence_mode.cpp`, `renderer.cpp` |
| RD1.6-02 | performance-enum-size | Per AA decision, **no enum underlying type was changed** (no `: std::uint8_t` or any other explicit type). Each of the six declarations - `JsonValue::Kind` (evidence_mode.cpp), `SpikeSceneId` (spike_scene.hpp), `ViewportErrorCode` (error.hpp), `PointerButton` (input.hpp), `ViewportState` (lifecycle.hpp), `MeshTopology` (mesh.hpp) - carries a narrow check-specific suppression with an adjacent 3-line reason ("the default enum representation is intentionally preserved for the Phase-0 contract; not changed merely for storage-size optimization (Architecture Authority decision)"). Five use a trailing inline `// NOLINT(performance-enum-size)` on the `enum class X {` line (each ≤ 100 columns). `Kind` is a single-line enum whose declaration line would exceed the column limit with a trailing comment, so it uses `// NOLINTNEXTLINE(performance-enum-size)` on the immediately preceding line instead (equally narrow; documented inline). No blanket NOLINT, no NOLINTBEGIN/END, no .clang-tidy change, no warning flag. | 6 files |
| RD1.6-03 | cppcoreguidelines-special-member-functions | `MainWindow` and `ViewportWindow` (both QObject-derived UI objects, not value types) now declare `X(const X&) = delete; X& operator=(const X&) = delete; X(X&&) = delete; X& operator=(X&&) = delete;` immediately after their existing constructor/destructor declarations, with a short comment. No custom copy/move implementation; destructor and ownership semantics unchanged. | `main_window.hpp`, `viewport_window.hpp` |
| RD1.6-04 | bugprone-exception-escape | Not suppressed. Following the accepted P0-T002 pattern (`tests/integration/p0_t002_geometry_evidence.cpp`, SA-R8-04 and its 8B follow-up, staged read-only for reference): the entire previous `main()` body was moved byte-for-byte into file-local `static int RunDesktopMain(int argc, char** argv)`; `int main(int argc, char** argv) noexcept` now only does `try { return RunDesktopMain(argc, argv); } catch (const std::exception& ex) { ReportFatalDiagnostic("unhandled std::exception", ex.what()); return 1; } catch (...) { ReportFatalDiagnostic("unhandled unknown (non-std::exception) exception", nullptr); return 1; }`. `ReportFatalDiagnostic` is a new anonymous-namespace helper declared `noexcept`, writing a concise `bim_desktop_spike: fatal: ...` line to stderr with C stdio (`std::fputs`) only - no stream/string construction - so the handlers contain no potentially-throwing operation (the exact reason P0-T002 round 8B had to drop its `std::cerr` handlers) while still emitting the diagnostic AA asked for. Added `#include <cstdio>` and `#include <exception>` (required by the new code). Normal-path behavior, Qt lifecycle, evidence-mode semantics and every existing return value are unchanged; no exception is swallowed into a success return. | `main.cpp` |
| RD1.6-05 | performance-no-int-to-ptr | The two exact Qt `WId` -> `void*` interop casts (`main.cpp`: `options.nativeWindowHandle = reinterpret_cast<void*>(native_surface->winId());`; `viewport_window.cpp`: `create_info.nativeWindowHandle = reinterpret_cast<void*>(surface_->winId());`) are unchanged in text and semantics. Each now has a narrow `// NOLINTNEXTLINE(performance-no-int-to-ptr)` on the immediately preceding line plus an adjacent reason comment stating this is the required Qt/Win32 native-handle boundary (integer `WId` from Qt, opaque `void*` in the neutral viewport contract for bgfx). NEXTLINE rather than trailing-inline only because a trailing comment would push both statements past the 100-column limit and invite clang-format to re-break the Operator-formatted statement lines. No global suppression; no redesign; no new abstraction; public viewport API untouched. | `main.cpp`, `viewport_window.cpp` |
| RD1.6-06 | bugprone-empty-catch | **Inspection result:** the only empty catch handlers in `renderer.cpp` are in `Renderer::Impl::ReleaseSlot(std::uint32_t) noexcept` (`catch (const std::bad_alloc&) {}` / `catch (const std::length_error&) {}` around `free_slots.push_back`). `CreateMesh()`'s `bad_alloc`/`length_error` handlers already `return Result<RenderMeshHandle>::Fail(ResourceCreationFailed)` and were not empty - unchanged. Because `ReleaseSlot` is `void ... noexcept` with no failure vocabulary, AA's preferred "return the existing failure result directly from each catch" form is not applicable there. The current, documented semantics (allocation failure -> the slot index is deliberately not returned to the free-list; the function returns normally; never a crash, never a leaked bgfx resource) are preserved exactly and made explicit with an explicit `return;` in each handler plus a trailing note and an adjacent explanation comment - the handling decision stated in code, not a `NOLINT`, not a dummy expression statement. No new error code, no new state (would have required `renderer_impl.hpp`, not authorized), no widening of the caught set, no change to allocation order or to successful `CreateMesh` behavior. **Flagged for AA review** (18.3). | `renderer.cpp` |

### 18.2 Exact NOLINT inventory (8 markers, all check-specific and single-line)

| File | Line (post-edit) | Marker | Adjacent reason |
|------|------------------|--------|-----------------|
| `src/desktop/src/evidence_mode.cpp` | 150 (applies to 151, `enum class Kind {...};`) | `// NOLINTNEXTLINE(performance-enum-size)` | Phase-0 default representation intentionally preserved; not changed for storage-size optimization (AA decision); NEXTLINE form because a trailing comment would exceed the column limit. |
| `src/desktop/src/spike_scene.hpp` | 50 | `enum class SpikeSceneId { // NOLINT(performance-enum-size)` | same Phase-0 reason (3 lines above) |
| `src/viewport/include/bim/viewport/error.hpp` | 31 | `enum class ViewportErrorCode { // NOLINT(performance-enum-size)` | same |
| `src/viewport/include/bim/viewport/input.hpp` | 22 | `enum class PointerButton { // NOLINT(performance-enum-size)` | same |
| `src/viewport/include/bim/viewport/lifecycle.hpp` | 25 | `enum class ViewportState { // NOLINT(performance-enum-size)` | same |
| `src/viewport/include/bim/viewport/mesh.hpp` | 22 | `enum class MeshTopology { // NOLINT(performance-enum-size)` | same |
| `src/desktop/src/main.cpp` | 256 (applies to 257, the `winId()` cast) | `// NOLINTNEXTLINE(performance-no-int-to-ptr)` | required Qt/Win32 native-handle boundary (integer `WId` -> opaque `void*` for bgfx); conversion inherent to the interop; no redesign/abstraction. |
| `src/desktop/src/viewport_window.cpp` | 305 (applies to 306, the `winId()` cast) | `// NOLINTNEXTLINE(performance-no-int-to-ptr)` | same |

No other `NOLINT` token exists in any of the 11 files (verified by grep; one
prose occurrence of the word in a comment was reworded so it cannot be
parsed as a blanket suppression).

### 18.3 Items flagged for Architecture Authority's attention

- **RD1.6-06 location/form.** AA's directive described the empty catches as
  translating failures "into the existing renderer failure vocabulary" and
  pointed at the `ResourceCreationFailed` / `Result<RenderMeshHandle>::Fail`
  path. On inspection those (CreateMesh) handlers already do exactly that
  and are not empty; the empty ones are in `Impl::ReleaseSlot`, a `void
  noexcept` function. The applied form (`return;` + comment) preserves
  semantics exactly and is not a lint suppression, but AA may reasonably
  judge whether an explicit `return;` at handler end satisfies "no
  meaningless dummy statements". If not, the semantics-preserving
  alternatives are (a) authorize `src/viewport/bgfx/src/renderer_impl.hpp`
  so a dropped-free-slot counter can be recorded on `Impl` (real state, a
  real observable), or (b) treat it harness-side via `.clang-tidy`
  (`bugprone-empty-catch.AllowEmptyCatchForExceptions` /
  `IgnoreCatchWithKeywords`) - neither done here (out of scope / forbidden).
- **RD1.6-04 vs. P0-T002 round 8B.** AA asked for a stderr diagnostic in
  the handlers; P0-T002 round 8B recorded that clang-tidy 19.1.5 kept
  flagging `main()` when its handlers contained `std::cerr` output. This
  round reconciles both by routing the diagnostic through a `noexcept`,
  C-stdio-only helper (calls to `noexcept` functions are treated as
  non-throwing by the check's exception analyzer). That reconciliation is
  authored from the check's documented/observed behavior, not verified
  here - AA's clang-tidy re-run decides.
- **Return code.** Both boundary handlers return `1`, matching the P0-T002
  pattern verbatim (AA asked only for "non-zero"). `2` remains the existing
  usage-error return; evidence mode's pass/fail return is unchanged.
- **Formatting.** clang-format 19.1.5 is still unavailable here; 18.x was
  not run and formatting was not approximated. All new lines are ≤ 100
  columns and follow the surrounding Operator-formatted style, but exact
  operator-side clang-format 19.1.5 remains required after RD1.6.

### 18.4 What this round did not do

- Did not touch `src/viewport/bgfx/CMakeLists.txt` or any CMake file; did
  not remove or alter `/Zc:preprocessor` (the
  `clang-diagnostic-unused-command-line-argument` finding is HARNESS-ONLY
  per AA and is not source-fixed); did not touch `.clang-tidy`,
  `.clang-format`, the clang-tidy harness, warning flags, or `/WX`.
- Did not change any enum's underlying representation; did not change the
  public viewport API or the native-handle contract; did not add any
  abstraction; did not change Qt lifecycle, evidence-mode semantics,
  camera/FOV, lifecycle, bgfx, or error-handling behavior.
- Did not run clang-format 18.x, approximate clang-format 19.1.5 output,
  or rewrite any already-formatted unrelated region.
- Did not run clang-tidy (any version), Full Runbook D, or involve Kimi.
- Did not write any path outside the authorized 13.
- Did not stage, `git add`, or commit; did not touch `main`.
- Did not raise an ACR.
- Did not rewrite, edit, or renumber any prior summary block or numbered
  section (0-17) - additive only (new summary block before section 0, and
  this section 18).

### 18.5 Local verification performed (engineer-reported, NON-AUTHORITATIVE)

- Re-staged all 11 files from the worktree and diffed each edited file
  against its re-staged (Operator-formatted) baseline; reviewed every hunk
  (line counts per file are in the JSON companion). Only the intended
  hunks are present.
- Brace/paren/bracket balance sweep over all 11 edited files (comment- and
  string-stripped): all balanced; the one pre-existing `(`/`)` count
  asymmetry in `evidence_mode.cpp` is identical in the untouched baseline
  (a character-literal artifact of the rough stripper), not introduced here.
- `g++ -std=c++20 -fsyntax-only -Wall -Wextra` on the four Qt/bgfx-free
  headers (`error.hpp`, `input.hpp`, `lifecycle.hpp`, `mesh.hpp`): clean.
  `main.cpp`, `viewport_window.*`, `main_window.hpp`, `spike_scene.hpp`,
  `evidence_mode.cpp`, `renderer.cpp` cannot be compiled here (no Qt/bgfx).
- Grep inventory: exactly 8 `NOLINT`/`NOLINTNEXTLINE(...)` markers, all
  check-specific; no bare `NOLINT`.
- New-line column check: no added line exceeds 100 columns.

None of this is compilation of the Qt/bgfx translation units, a clang-tidy
run, a build, or a test. **RD1.6 status is DELIVERED_FOR_AA_VERIFICATION;
this document does not claim that clang-tidy, clang-format, the build, or
Architecture CTests pass on these bytes.**

### 18.6 Operator clang-format completion (handover-only round; 18.0-18.5 above preserved unchanged)

**Provenance - the sequence is explicit and must stay explicit:**

1. Claude (this session) delivered the RD1.6 source correction (18.0-18.5)
   - **DELIVERED_FOR_AA_VERIFICATION**. Claude did **not** have exact
   clang-format 19.1.5 and did not run any formatter (18.3, "Formatting").
2. Architecture Authority's **RD1.6 pre-format authoritative delta audit
   v2 = PASS**. It proved (AA-supplied): candidate paths 59; RD1.6 changed
   13 exact; RD1.6 untouched 46; source/header delta 11 exact; handover
   delta 2 exact; enum-size NOLINT 6 exact; native-handle NOLINT 2 exact;
   enum representations PRESERVED; Qt special members EXPLICITLY DELETED;
   main exception boundary PASS; Qt/native-handle semantics PRESERVED;
   renderer catch correction EXPLICIT RETURN; `/Zc:preprocessor` PRESERVED;
   CMake/.clang-tidy UNTOUCHED; staged paths 0; `main` UNCHANGED + CLEAN;
   ACR NONE.
3. AA then authorized the **Windows Execution Operator** to apply exact
   operator-side clang-format to the 11 RD1.6 source/header files only.
   Operator run
   `C:\Users\abdallah\Downloads\P0-T003-RD1-6-OPERATOR\RD1-6-FORMAT-20260912-164100-395fbbc6`,
   executable
   `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin\clang-format.exe`,
   **clang-format version 19.1.5**. Result **OPERATOR_FORMAT_PASS**;
   formatted files 11 exact.
4. Operator post-format dry-run: **11/11 PASS**. RD1.6 delta vs RD1.5
   after formatting: **13 exact paths**. Handover pair during operator
   formatting: **UNCHANGED / byte-identical**. Staged 0. `main` UNCHANGED +
   CLEAN.
5. **Final AA RD1.6 authoritative audit: PENDING.**

Operator evidence (AA-supplied paths; not read by this session): backup
`...\RD1-6-FORMAT-20260912-164100-395fbbc6\pre-format-backup`; pre-format
hashes `...\pre-format-hashes.tsv`; post-format hashes
`...\post-format-hashes.tsv`; result `...\result.json` (all under the run
folder above).

**Status recorded:**

| Item | Status |
|------|--------|
| RD1.6 implementation delivery | DELIVERED_FOR_AA_VERIFICATION |
| RD1.6 pre-format AA audit (v2) | PASS |
| Operator exact clang-format | **OPERATOR_FORMAT_PASS** (clang-format 19.1.5) |
| Formatted source/header files | 11 exact |
| Post-format dry-run | 11/11 PASS |
| Current delta vs RD1.5 | 13 exact paths |
| Handover during formatting | UNCHANGED |
| Staged | 0 |
| `main` | UNCHANGED + CLEAN |
| Final RD1.6 AA audit | PENDING |
| Post-RD1.6 build re-certification | PENDING |
| Post-RD1.6 Architecture CTests | PENDING |
| clang-tidy re-run | PENDING |
| Full Runbook D Attempt 2 | NOT AUTHORIZED |
| Kimi | NOT AUTHORIZED |
| Commit | FORBIDDEN |
| ACR | NONE |

Everything above from AA's audit and the Operator's run is AA-supplied and
was not independently verified by this session (no execution channel; the
Operator's evidence files were not read).

**What this handover-only round did:** modified exactly 2 paths -
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md` (an "Update" paragraph appended
to the RD1.6 summary block and this subsection 18.6; 18.0-18.5 and every
earlier section untouched) and `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`
(`operator_format_completion` inside `aa_rd1_6_corrections`,
`verification_status.rd1_6_operator_format_completion_result`, per-entry
annotation on the 11 source/header footprint entries, `.md` entry
size/hash refresh). It wrote **zero** source/header/test/CMake/config files:
nothing was re-staged, resent, rewritten, normalized, or formatted. No
staging, no `git add`, no commit; `main` untouched; no formatter, no
clang-tidy, no Full Runbook D, no Kimi.

**Standing warning (renewed):** this session's *local copies* of the 11
RD1.6 source/header files are now **stale again** - they hold the RD1.6
pre-format bytes as delivered by this session, not the Operator's
clang-format 19.1.5 output. Their `size_bytes`/`sha256` in the JSON
companion likewise describe the pre-format RD1.6 bytes (annotated as such;
authoritative post-format hashes live only in the Operator's
`post-format-hashes.tsv`, which this session did not receive and does not
reproduce - no post-format hash is fabricated here). Any future round that
edits one of these files must first re-stage the current file from the
Windows worktree (as RD1.6 itself did - 18.0) and must never deliver from
the stale local copy.

---

## 19. AA RD1.7 Full Runbook D Attempt 2 failure correction round ("minimum delta only") - DELIVERED_FOR_AA_VERIFICATION

### 19.0 Scope and historical record

- **Full Runbook D Attempt 1 = HISTORICAL FAIL** (section 12; never
  rewritten).
- **Full Runbook D Attempt 2 = HISTORICAL FAIL, 60 PASS / 3 FAIL** (Checks
  24, 35, 60). Never rewritten as PASS. Run by Architecture Authority on
  the authoritative Windows environment - NOT this session.
- Pre-RD1.7 targeted gates (AA-supplied, not verified here): RD1.6 final
  authoritative audit + snapshot PASS
  (`P0-T003-RD1-6-CORRECTION-20260912-173736`); clang-format 19.1.5 35/35
  PASS; Targeted Full Build v8 PASS; Architecture CTests v3 12/12 PASS;
  clang-tidy v7 17/17 TUs, 0 project diagnostics PASS. Task HEAD
  `82367706691567b0148ee0e7b4eac3a3e30b33d1` / tree
  `0f5845e02a5f48d64a56f467c26d5ac526bf61dc` (AA-supplied). Footprint 12
  MODIFY + 47 ADD = 59, 0 DELETE, 0 RENAME.
- Authorized paths (max 5) - exactly these 5 were touched, zero others:
  `src/desktop/src/evidence_mode.cpp`, `scripts/ci/viewport-spike.ps1`,
  `Verification-RunbookD-v1.0.ps1`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
  `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`.
- **Stale-copy rule honored:** all five files (plus `evidence_mode.hpp`,
  read-only, for the options contract) were fresh-staged from
  `D:\Projects\BIM-Platform-WT-P0-T003` before any edit; each was
  SHA-256-compared to this session's local copy and every one was
  byte-identical (the Operator's RD1.6 clang-format 19.1.5 run produced no
  change to `evidence_mode.cpp`, so this session's RD1.6 delivery bytes
  were already the current worktree bytes). Staging mtimes were used as
  `expectedMtimeMs` guards on delivery. Pre-edit hashes are in the JSON
  companion (`aa_rd1_7_corrections.pre_edit_worktree_baseline`).

### 19.1 Attempt-2 classification (Architecture Authority) and correction per item

| Check | AA classification | Correction | File |
|-------|-------------------|------------|------|
| 24 clang-tidy | **HARNESS / COMPDB ONLY - CLOSED.** Exact clang-tidy 19.1.5; raw `compile_commands.json` contains exactly one `/Zc:preprocessor` token (renderer.cpp's command - the MSVC option `src/viewport/bgfx/CMakeLists.txt` applies to `bim_viewport_bgfx`, RD1.3-01, required by `bx/platform.h` and proven by the passing production build); `renderer.cpp` exit=1 with the single error `argument unused during compilation: '/Zc:preprocessor' [clang-diagnostic-unused-command-line-argument]`; with that one token removed from a temporary analysis-only compdb, same source bytes, exit=0 and no project diagnostic. clang-cl analysis compatibility only. | Check 24 in `Verification-RunbookD-v1.0.ps1` now: reads the raw DB and records its SHA-256; asserts the raw text contains exactly one `/Zc:preprocessor` token; parses the DB and asserts exactly one compile entry carries the token (in `command` or `arguments` form) and that its `file` ends in `src\viewport\bgfx\src\renderer.cpp`; removes the token textually (exactly one regex match asserted; leading whitespace or leading comma consumed so the DB stays valid JSON and every other byte is preserved); asserts zero tokens remain; writes the result to `<BuildDir>\evidence\P0-T003\runbook-d\clang-tidy-analysis-compdb\compile_commands.json`; runs clang-tidy with `-p` pointed at that temporary directory for the same source set as before; re-hashes the raw DB and fails if it changed. Every unexpected count/path throws (fail-closed). Documented inline as clang-cl analysis compatibility only. **`renderer.cpp`, `src/viewport/bgfx/CMakeLists.txt`, the raw compile_commands.json and the production MSVC `/Zc:preprocessor` are untouched; no NOLINT.** | `Verification-RunbookD-v1.0.ps1` |
| 35 viewport-spike CI / 60 direct headless evidence JSON - part B | **Qt platform plugin CI wiring.** `qwindows.dll` exists at `<BuildDir>\vcpkg_installed\x64-windows\Qt6\plugins\platforms\qwindows.dll`, but `QT_PLUGIN_PATH` and `QT_QPA_PLATFORM_PLUGIN_PATH` were empty and no `platforms\qwindows.dll` existed beside the executable, so `QApplication` aborted before any evidence ran. With Operator-supplied process-local paths, the evidence ran and produced valid JSON. CI/runtime wiring, not product source. | `scripts/ci/viewport-spike.ps1`: new `Resolve-QtPluginRoot` derives `<BuildDir>\vcpkg_installed\x64-windows\Qt6\plugins` from the current `-BuildDir` (no hard-coded worktree path) and throws if `platforms\qwindows.dll` is missing; before the first `bim_desktop_spike.exe` child the script captures the previous process values and sets, **Process scope only**, `QT_PLUGIN_PATH=<plugin-root>`, `QT_QPA_PLATFORM_PLUGIN_PATH=<plugin-root>\platforms`, `QT_QPA_PLATFORM=windows` (inherited by the 20+ headless cycles and the live run); `Restore-QtPluginEnvironment` puts the previous values back (`$null` = unset) on the normal path after the last evidence invocation and from the outer `catch` on the failure path. `Verification-RunbookD-v1.0.ps1`: new `Resolve-QtPluginRoot -InBuildDir -ForTriplet` (uses the script's `$Triplet` param, default `x64-windows`) and `Invoke-WithQtPluginEnvironment` (save / set Process-scope / run body / restore in `finally`); Check 60's direct `--evidence-mode` launch and Check 35's `viewport-spike.ps1` launch both run inside it. Global/user/system environment never written; no CMake deployment redesign. | `scripts/ci/viewport-spike.ps1`, `Verification-RunbookD-v1.0.ps1` |
| 35 / 60 - part C | **Real evidence-mode orchestration defect.** Headless evidence (evidence_mode=headless, backend=Noop) PASSED lifecycle initialization, surface recreation, resize, camera, ray, V01-V05, lifecycle tail, final shutdown, and both integration tests; the only failing area was `repeatability` (requested_cycle_count=20, cycles[0..19].passed=false, all_cycles_passed=false -> overall_passed=false). Root cause: `RunRepeatabilityCycles()` constructs its own short-lived `Renderer` per cycle - a full bgfx runtime init each time - while another Renderer already owns the single process-wide bgfx runtime (headless: this function's own primary renderer, still initialized when the loop ran; live: the production `ViewportWindow`'s renderer). Orchestration defect, not a Renderer API/architecture defect. `backend.homogeneous_depth=false` is a capability value, not a failure - untouched. | `src/desktop/src/evidence_mode.cpp` - see 19.2. | `src/desktop/src/evidence_mode.cpp` |

### 19.2 Exact source behavior correction (`evidence_mode.cpp`)

1. **No nested/secondary Renderer while another bgfx renderer is active.**
   The repeatability block was moved from its former position (after
   `surface_recreation_result`, while the primary headless `renderer` was
   initialized) to immediately after `create_info`/`live_viewport_ready`
   are computed and **before** `bim::viewport_bgfx::Renderer renderer;`
   is declared/initialized. In headless mode the cycles therefore run
   while no other Renderer exists in the process; the primary headless
   renderer is initialized only after the last cycle has shut down.
2. **Headless mode:** `RunRepeatabilityCycles(create_info,
   options.repeatabilityCycleCount, cycles_json)` - the same function, the
   same per-cycle initialize -> CreateMesh -> RenderFrame -> DestroyMesh ->
   Shutdown sequence, the same `create_info` (headless, width/height), the
   same default (`options.repeatabilityCycleCount`, `= 20` in
   `evidence_mode.hpp`, unchanged), the same JSON schema
   (`requested_cycle_count`, `cycles[]` with `cycle_index`/`passed`/
   `duration_ms_observational`, `all_cycles_passed`), and the same
   `RecordResult(repeat_ok)` contribution to `overall_passed`.
3. **Live mode:** no second Renderer is constructed alongside the
   production `ViewportWindow`'s renderer. `repeatability` is emitted as
   `requested_cycle_count` + `cycles` = `MakeNotAvailable("live mode: the
   production ViewportWindow owns the single process-wide bgfx runtime for
   the whole evidence run, so an in-process nested Renderer repeatability
   loop cannot run alongside it (a second bgfx init while one is active
   fails); process-level repeatability is covered externally by
   scripts/ci/viewport-spike.ps1 (>= 20 separate headless OS processes,
   each required to exit successfully) - not a live-run failure")` +
   `externally_covered_by: "scripts/ci/viewport-spike.ps1"`. **No
   `RecordResult(false)`** for this intentionally-not-applicable case;
   every actual live D3D11 / lifecycle / scene / resize / recreation /
   minimize-restore requirement and its `RecordResult` is unchanged.
4. **JSON key order preserved:** the `repeatability_json` object is built
   early but `root.Set("repeatability", ...)` still happens at the
   original position (after `surface_recreation_result`, before
   `lifecycle_tail`), so Check 60's required-field set and the emitted key
   order are unchanged.
5. **Removed:** the `live_surface_available` flag (it existed only to gate
   the live-mode in-process loop, which no longer exists; leaving it would
   be an unreferenced local under `/W4 /WX`). Its only side effect -
   conditionally assigning `create_info.nativeWindowHandle` - is now an
   unconditional `create_info.nativeWindowHandle =
   options.nativeWindowHandle;`, which is semantically identical (a null
   handle stays null either way; `RendererCreateInfo::nativeWindowHandle`
   defaults to `nullptr`). Also removed the now-dead `repeat_create_info`
   copy and `repeat_available` flag.
6. **Untouched:** `Renderer`, `ViewportWindow`, `Camera`, lifecycle, mesh
   handle model, bgfx ownership model, `renderer.cpp`,
   `src/viewport/bgfx/CMakeLists.txt`, public viewport API, architecture
   checker, `.clang-tidy`, `evidence_mode.hpp`. Successful headless
   CreateMesh/scene behavior, allocation order, and every other evidence
   section are unchanged. Comment updates only elsewhere in the file (the
   `RunRepeatabilityCycles` header comment and the former gate comment).

### 19.3 What this round did not do

- Did not modify `src/viewport/bgfx/src/renderer.cpp`,
  `src/viewport/bgfx/CMakeLists.txt`, the public viewport API, the
  architecture checker, `.clang-tidy`, any CMake file, or `_common.ps1`.
- Did not remove `/Zc:preprocessor` from the production MSVC build, did
  not add a NOLINT for it, did not modify the raw `compile_commands.json`.
- Did not redesign the native-handle contract, Renderer/ViewportWindow
  architecture, Camera, lifecycle, mesh-handle model, or bgfx ownership.
- Did not modify global/user/system environment variables (Process scope
  only, restored); did not hard-code the current `D:\` worktree path; did
  not redesign CMake deployment.
- Did not run clang-format 18.x or approximate clang-format 19.1.5; did
  not rewrite already-formatted unrelated regions.
- Did not run Full Runbook D Attempt 3 (NOT AUTHORIZED), clang-tidy, or
  Kimi; did not claim Attempt 2 PASS or any PASS this session did not
  execute.
- Did not stage, `git add`, or commit; did not touch `main`; did not
  raise an ACR.
- Did not rewrite any prior summary block or numbered section (0-18) -
  additive only.

### 19.4 Local verification performed (engineer-reported, NON-AUTHORITATIVE)

No Windows/MSVC/Qt/bgfx/PowerShell execution exists in this session. The
following static checks only:

- Fresh-staged all five authorized files + `evidence_mode.hpp`;
  SHA-256-compared to local copies (all identical); edited on the
  re-staged bytes; unified diff of each edited file reviewed hunk by hunk
  (`evidence_mode.cpp` +72/-49 lines, `scripts/ci/viewport-spike.ps1`
  +68/-2, `Verification-RunbookD-v1.0.ps1` +134/-3 - final counts, after
  the two self-caught fixes noted below).
- Brace/paren balance: `evidence_mode.cpp` `{}` 30/30 (unchanged count),
  `()` delta identical to baseline; both `.ps1` files checked with a
  PowerShell-aware scanner (block/line comments, single-/double-quoted
  strings with `''`/`""`/backtick escapes stripped): `{}`/`()`/`[]` all
  balanced in both baseline and edited versions.
- Grep: no remaining reference to the removed `live_surface_available`,
  `repeat_create_info`, `repeat_available`; no added `evidence_mode.cpp`
  line exceeds 100 columns.
- Self-caught and fixed before delivery: (1) two throw messages initially
  used `\"windows\"` inside PowerShell double-quoted strings (a C-style
  escape that would have terminated the string) - changed to
  `""windows""`; (2) Check 24 initially called a non-existent static
  `[regex]::Replace(input, pattern, replacement, count)` overload - changed
  to the instance form `([regex]$pattern).Replace($text, '', 1)`.
- Check 24's removal regex validated offline (Python `re`, same semantics
  for these constructs) against synthetic `compile_commands.json` samples
  in both the `command`-string and `arguments`-array forms: exactly one
  raw token, exactly one removal match, zero tokens after removal, output
  still valid JSON, only renderer.cpp's command altered.

None of this is compilation, a Qt/bgfx run, a PowerShell run, or Runbook
D. **RD1.7 status is DELIVERED_FOR_AA_VERIFICATION; this document does not
claim that Checks 24/35/60, the headless repeatability cycles, the build,
or any Runbook D attempt pass on these bytes.** Full Runbook D Attempt 3
remains NOT AUTHORIZED until AA targeted verification passes.

### 19.5 Status recorded

| Item | Status |
|------|--------|
| Full Runbook D Attempt 1 | HISTORICAL FAIL |
| Full Runbook D Attempt 2 | HISTORICAL FAIL - 60 PASS / 3 FAIL (Checks 24, 35, 60) |
| Check 24 | HARNESS / COMPDB ONLY (clang-cl analysis compatibility) - harness-corrected, not source-fixed |
| Checks 35/60 - Qt plugin environment defect | CI/runtime wiring corrected (process-local, fail-closed) |
| Checks 35/60 - repeatability orchestration defect | source-corrected in `evidence_mode.cpp` |
| RD1.7 | **DELIVERED_FOR_AA_VERIFICATION** |
| clang-format 19.1.5 on the RD1.7 `evidence_mode.cpp` delta | PENDING - Windows Execution Operator (unavailable here; 18.x not run, not approximated) |
| ACR | NONE |
| Full Runbook D Attempt 3 | NOT AUTHORIZED |
| Kimi | NOT AUTHORIZED |
| Commit | FORBIDDEN (none performed) |
| Staging | NONE |
| `main` | UNTOUCHED |

---

## 20. AA RD1.8 minimum-delta correction round - DELIVERED_FOR_AA_VERIFICATION

### 20.0 Baseline, gates, and scope

- **RD1.7 remains the frozen historical authoritative baseline:**
  `P0-T003-RD1-7-CORRECTION-20260913-005610`, ZIP SHA-256
  `73000641ce0dbc296550673fbd3be74c4003cb453f254de9e4ea3e3e7a444940`. Task
  HEAD `82367706691567b0148ee0e7b4eac3a3e30b33d1`, tree
  `0f5845e02a5f48d64a56f467c26d5ac526bf61dc`. Unrelated paths were not
  restored or normalized.
- **Targeted post-RD1.7 results (AA-supplied, not verified here):** full
  build PASS; full CTest PASS; clang-tidy 19.1.5 17/17 TUs, zero project
  diagnostics; direct headless evidence PASS; in-process repeatability
  20/20 PASS; external process-level repeatability 20/20 PASS. **Targeted
  viewport-spike remained blocked at the final live evidence step.**
- **AA read-only diagnostics (AA-supplied):** CWD-dependent shader loading
  (repo-root CWD: `shaders/dx11` absent, renderer_initialize=false,
  backend=Uninitialized; build-root and exe-directory CWD: present,
  renderer_initialize=true, backend=Direct3D 11); build-root/exe-dir
  recover Direct3D 11 initialization; all substantive live evidence passes
  once shaders resolve; backend identity string mismatch - actual
  `"Direct3D 11"`, old gate `"Direct3D11"`.
- **Historical record preserved:** Full Runbook D Attempt 1 = HISTORICAL
  FAIL; Full Runbook D Attempt 2 = HISTORICAL FAIL (60 PASS / 3 FAIL).
  Neither rewritten. Attempt 3 NOT AUTHORIZED; Kimi NOT AUTHORIZED; commit
  FORBIDDEN; ACR = NONE.
- **Authorized scope (exactly four paths, all four touched, zero others):**
  `src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/evidence_mode.cpp`,
  `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
  `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. No CMake, public API,
  neutral viewport API, `RendererCreateInfo`, architecture checker,
  `.clang-tidy`, test-source, or CI-script change; no global/process
  current-directory mutation.
- **Stale-copy rule honored:** all four files (plus `renderer_impl.hpp`,
  `renderer.hpp`, and `.clang-tidy`, read-only) were fresh-staged from the
  worktree before any edit; each of the four was byte-identical to this
  session's RD1.7 delivery bytes (no Operator format change had landed on
  them). Staging mtimes were used as `expectedMtimeMs` guards on
  delivery. Pre-edit hashes are in the JSON companion
  (`aa_rd1_8_corrections.pre_edit_worktree_baseline`).

### 20.1 RD1.8-01 - CWD-independent runtime shader asset resolution (`renderer.cpp`)

**Exact approach:** a new private helper in `renderer.cpp`'s anonymous
namespace,

```cpp
std::filesystem::path RunningExecutableDirectory();
```

- On `_WIN32`: calls `::GetModuleFileNameW(nullptr, buffer.data(),
  static_cast<DWORD>(buffer.size()))` into a `std::wstring` that starts at
  260 characters and doubles while the API reports truncation (return
  value `>= buffer.size()`), capped at 32768 characters (the Windows
  long-path limit); returns an empty path on a zero return, on exceeding
  the cap, or if the resulting path has no parent; otherwise returns
  `std::filesystem::path(buffer).parent_path()`. `<windows.h>` is included
  in `renderer.cpp` only, guarded by `WIN32_LEAN_AND_MEAN` and `NOMINMAX`,
  after the bgfx/bx headers; nothing from it escapes the translation unit
  (`renderer.hpp`/`renderer_impl.hpp` untouched, so the architecture
  checker's R8 neutral-contract rule - which forbids `<Windows.h>`/`HWND`
  only in the public headers - still holds; R11 forbids only D3D/DXGI
  tokens, none added).
- On non-Windows: returns an empty path deliberately (this spike's only
  authoritative backend is D3D11 on Windows), so shader loading fails
  closed there rather than silently depending on the CWD again.

`LoadCompiledShader(backend_dir, name)` now does:

```cpp
const std::filesystem::path executable_dir = RunningExecutableDirectory();
if (executable_dir.empty()) {
    return BGFX_INVALID_HANDLE;   // fail closed
}
const std::filesystem::path shader_path =
    executable_dir / "shaders" / backend_dir / (std::string(name) + ".bin");
#if defined(_MSC_VER)
std::FILE* file = nullptr;
if (_wfopen_s(&file, shader_path.c_str(), L"rb") != 0) {   // native wchar_t path
    file = nullptr;
}
#else
std::FILE* file = std::fopen(shader_path.string().c_str(), "rb");
#endif
if (file == nullptr) {
    return BGFX_INVALID_HANDLE;   // fail closed
}
// ... size / read / bgfx::createShader unchanged ...
```

replacing the former `char path[512]; std::snprintf(path, sizeof(path),
"shaders/%s/%s.bin", backend_dir, name);` + `fopen_s(&file, path, "rb")`.
Resolved layout: `<executable-dir>\shaders\dx11\vs_p0_t003.bin` /
`fs_p0_t003.bin` - the layout the existing POST_BUILD step already
creates (no deployment redesign). Added includes: `<filesystem>`,
`<string>`, and the guarded `<windows.h>`. Constraints honored: helper
private to the adapter; no Qt; no public/API expansion;
`RendererCreateInfo` unchanged; no `SetCurrentDirectory`/`chdir`; no
hard-coded repository or build path; no environment variable; no
CWD-based fallback; fail-closed on unresolvable directory or file;
headless/Noop never calls this function (unchanged); shader names and
backend subdirectory preserved; renderer ownership/lifecycle untouched.

### 20.2 RD1.8-02 - live backend identity aggregate bug (`evidence_mode.cpp`)

**Exact expression after correction:**

```cpp
RecordResult(!renderer_ready || backend_info.backendName == "Direct3D 11");
```

(was `... == "Direct3D11"`). The immediately-related comment now states
the canonical bgfx spelling `"Direct3D 11"` (the string
`bgfx::getRendererName()` returns for `RendererType::Direct3D11`, and the
exact `selected_renderer_backend` value the successful authoritative live
evidence recorded) and explains the former mismatch. The gate is not
weakened: a ready live renderer whose backend is anything other than
`"Direct3D 11"` still fails. `homogeneous_depth` remains recorded and
observational, never pass/fail. No other `RecordResult` was touched. Live
repeatability is exactly as RD1.7: `cycles` NOT_AVAILABLE in-process,
`externally_covered_by: scripts/ci/viewport-spike.ps1`, no
`RecordResult(false)`.

### 20.3 STOP-and-report: additional path found necessary, NOT modified (at initial RD1.8 delivery; see 20.7 for the A1 resolution)

`scripts/ci/viewport-spike.ps1` (RD1.7 baseline, line 261-262) applies the
same mismatched comparison to the live JSON:

```powershell
if ($liveSurfaceWasAvailable -and $backendName -ne 'Direct3D11') {
    throw "live evidence run had a native surface available but resolved backend was '$backendName', not the authoritative 'Direct3D11' ..."
}
```

Once RD1.8-01/-02 land and the live run reports
`selected_renderer_backend = "Direct3D 11"` with `overall_passed = true`,
this script-side check will throw and fail the viewport-spike job (and
therefore Full Runbook D Check 35) for the same string-mismatch reason.
CI-script changes are explicitly outside RD1.8's authorized scope ("No
CI-script changes"), so this path was **not** modified; it is reported
here for Architecture Authority's decision (a one-token change of
`'Direct3D11'` to `'Direct3D 11'` in both the condition and the message
would align it with the corrected `evidence_mode.cpp` gate). No other
path was found necessary; `Verification-RunbookD-v1.0.ps1` only
references "Direct3D11" in a comment.

### 20.4 What this round did not do

- Did not modify any CMake file, the public viewport API, the neutral
  viewport API, `RendererCreateInfo`, the architecture checker,
  `.clang-tidy`, any test source, or any CI script (see 20.3 for the one
  CI-script finding reported instead).
- Did not add a Qt dependency, a global/process current-directory
  mutation, a hard-coded repository/build path, an environment-variable
  dependency, or any CWD-based fallback.
- Did not make `homogeneous_depth` pass/fail; did not weaken the backend
  gate; did not change any other `RecordResult`; did not alter the proven
  live functional paths (initialize, lifecycle, minimize/restore, DPR
  transition, V01-V05, resize, surface recreation, shutdown, live
  repeatability NOT_AVAILABLE) beyond the two authorized edits.
- Did not restore or normalize unrelated paths.
- Did not run clang-format 18.x or approximate 19.1.5; did not
  mass-format.
- Did not run Full Runbook D Attempt 3, clang-tidy, or Kimi; did not claim
  any PASS this session did not execute; did not rewrite Attempt 1/2.
- Did not stage, `git add`, or commit; did not touch `main`; did not raise
  an ACR.
- Did not rewrite any prior summary block or numbered section (0-19) -
  additive only.

### 20.5 Self-audit (engineer-reported, NON-AUTHORITATIVE)

No git-execution channel exists in this session, so "git status" could
not be run; the changed-path set is established instead by (a) the
fresh-stage hash comparison before editing (all four files identical to
the RD1.7 delivery), (b) the delivery itself (`device_commit_files`
wrote exactly these four paths, zero rejections, mtime-guarded), and (c)
`device_list_dir` byte-size verification of the four written files and
unchanged mtimes on their neighbours.

- **Exactly four paths changed** relative to frozen RD1.7:
  `src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/evidence_mode.cpp`,
  `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
  `docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. **Zero other paths
  changed.** Staged paths: 0 (none staged). `main`: untouched. ACR: NONE.
- Byte sizes / SHA-256 of the four changed files as delivered: recorded
  in the JSON companion's footprint entries and repeated in
  `aa_rd1_8_corrections.delivered_files` (the JSON's own hash is
  self-referential and is verified by byte size, per the Round 3
  convention).
- Per-file unified diff against the re-staged baseline reviewed hunk by
  hunk: `renderer.cpp` +92/-10 lines, `evidence_mode.cpp` +20/-8 lines;
  only the intended hunks are present.
- Brace/paren balance: `renderer.cpp` `{}` 88/88, `()` 260/260;
  `evidence_mode.cpp` `{}` 30/30, `()` count asymmetry identical to the
  untouched baseline (rough-stripper artifact). No added line exceeds 100
  columns. Preprocessor guards paired (`#if`/`#ifndef`/`#else`/`#endif`).
- Grep: no `snprintf` remains in `renderer.cpp`; no remaining
  `"Direct3D11"` string in `evidence_mode.cpp`; no identifier in
  `renderer.cpp`/`renderer_impl.hpp`/`renderer.hpp`/neutral headers
  collides with common `<windows.h>` macros (`min`/`max`/`near`/`far`/
  `ERROR`/`DELETE`/`interface`/... checked).
- A stand-alone extract of the new path-building logic (non-Windows
  branch, `std::filesystem` + `std::fopen`) compiles clean with `g++
  -std=c++20 -Wall -Wextra -Werror -fsyntax-only` here. The Windows branch
  (`GetModuleFileNameW`, `_wfopen_s`, `<windows.h>`) and the bgfx/Qt
  translation units cannot be compiled in this environment.
- `.clang-tidy` (staged read-only) reviewed against the new code:
  `bugprone-*`/`performance-*`/`portability-*`/`readability-braces-around-
  statements`/`readability-else-after-return`: braces on every branch, no
  else-after-return, explicit `static_cast` on every narrowing, `nullptr`
  throughout, no C array (the former `char path[512]` is gone).
- **Exact clang-format 19.1.5: NOT available** in this environment
  (18.1.3 only). Not substituted, not mass-formatted; the two source
  deltas were delivered unformatted-by-tool. **Operator exact-format was
  required** - since completed with PASS and zero changed targets (20.8);
  no longer pending.

None of this is a Windows build, a Qt/bgfx run, clang-tidy 19.1.5,
clang-format 19.1.5, or Runbook D. **RD1.8 status is
DELIVERED_FOR_AA_VERIFICATION; this document does not claim that the live
evidence, viewport-spike, or any Runbook D check passes on these bytes.**

### 20.6 Status recorded

| Item | Status |
|------|--------|
| RD1.7 | Frozen historical authoritative baseline (`P0-T003-RD1-7-CORRECTION-20260913-005610`) |
| Post-RD1.7 targeted: full build / full CTest / clang-tidy 19.1.5 17/17 / direct headless evidence / in-process repeatability 20/20 / external process-level repeatability 20/20 | PASS (AA-supplied) |
| Targeted viewport-spike | blocked at final live evidence (pre-RD1.8) |
| RD1.8-01 CWD-independent shader resolution | delivered (`renderer.cpp`) |
| RD1.8-02 live backend identity gate `"Direct3D 11"` | delivered (`evidence_mode.cpp`) |
| `scripts/ci/viewport-spike.ps1` `'Direct3D11'` comparison | REPORTED under RD1.8 (20.3); ACCEPTED by AA and corrected to `'Direct3D 11'` under RD1.8 Amendment A1 (20.7) |
| RD1.8 | **DELIVERED_FOR_AA_VERIFICATION** |
| clang-format 19.1.5 on the RD1.8 deltas | PASS - Windows Operator exact 19.1.5 (`RD1-8-FORMAT-20260913-103505-55b82011`), 2 targets, dry-run 2/2, formatter changed targets = 0 (20.8) |
| ACR | NONE |
| Full Runbook D Attempt 1 | HISTORICAL FAIL |
| Full Runbook D Attempt 2 | HISTORICAL FAIL (60 PASS / 3 FAIL) |
| Full Runbook D Attempt 3 | NOT AUTHORIZED |
| Kimi | NOT AUTHORIZED |
| Commit | FORBIDDEN (none performed) |
| Staging | NONE |
| `main` | UNTOUCHED |

### 20.7 RD1.8 Amendment A1 - minimum-delta scope extension (`scripts/ci/viewport-spike.ps1`)

Architecture Authority received the initial RD1.8 delivery and **accepted
the 20.3 STOP-and-report finding** as part of the same RD1.8
backend-identity root cause: `scripts/ci/viewport-spike.ps1` compared the
live JSON's `selected_renderer_backend` against `'Direct3D11'` while the
authoritative runtime identity is `'Direct3D 11'`, which would
independently fail viewport-spike / Runbook Check 35 even after the RD1.8
source corrections succeed. **Amendment A1 extended the total RD1.8
changed-path set relative to frozen RD1.7 from 4 to exactly 5:**
`src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/evidence_mode.cpp`,
`scripts/ci/viewport-spike.ps1`, and the handover pair. Zero other paths.
Frozen baseline unchanged (`P0-T003-RD1-7-CORRECTION-20260913-005610`, ZIP
SHA-256 `73000641ce0dbc296550673fbd3be74c4003cb453f254de9e4ea3e3e7a444940`).

**Locked source files - byte-identical to the initial RD1.8 delivery,
re-staged from the worktree and hash-verified under A1 before and after
this amendment's work:**

- `src/viewport/bgfx/src/renderer.cpp` - 28808 bytes -
  `a1789435143a21b85f8b410d920208064fda757e849e75d29573940879b727bd`
- `src/desktop/src/evidence_mode.cpp` - 65880 bytes -
  `082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`

Neither was modified under A1 (the CWD shader defect stays fixed in
`renderer.cpp` itself; no shader-path workaround and no
environment-variable shader-path dependency was added to the CI script).

**Exact A1 change in `scripts/ci/viewport-spike.ps1`** (fresh-staged from
the worktree first; identical to the RD1.7 delivery bytes): the live
backend comparison and its directly-associated failure text, previously

```powershell
if ($liveSurfaceWasAvailable -and $backendName -ne 'Direct3D11') {
    throw "live evidence run had a native surface available but resolved backend was '$backendName', not the authoritative 'Direct3D11' (Brief section 2/9: ...)."
}
```

are now

```powershell
if ($liveSurfaceWasAvailable -and $backendName -ne 'Direct3D 11') {
    throw "live evidence run had a native surface available but resolved backend was '$backendName', not the authoritative 'Direct3D 11' (Brief section 2/9: ...)."
}
```

with an 8-line explanatory comment immediately above. Only the canonical
spelling changed (two occurrences: the condition and the error text); the
check is not weakened - if a live surface is available and the backend is
not exactly `'Direct3D 11'`, the job still fails closed. Untouched: Qt
plugin handling (`Resolve-QtPluginRoot` / process-local variables /
`Restore-QtPluginEnvironment`), the >= 20 process-level repeatability
cycles, the live evidence invocation, `overall_passed` validation, the
CTest list, build behavior, current working directory. Also untouched
under A1: `Verification-RunbookD-v1.0.ps1` (its only `Direct3D11` is in a
comment), CMake, tests, architecture checks.

**A1 self-audit (engineer-reported, NON-AUTHORITATIVE):** diff of
`viewport-spike.ps1` against the re-staged baseline is exactly -2/+10
lines (the two rewritten lines plus the comment); the only remaining
`Direct3D11` token in the script is inside the new comment ("previously
used 'Direct3D11'"); PowerShell-aware brace/paren balance unchanged; no
PowerShell execution is possible here. Exact clang-format 19.1.5 is not
applicable to a `.ps1` and was in any case not substituted (18.1.3 not
run); for the two C++ targets it has since been run by the Operator with
PASS and zero changed targets (20.8). No staging, no commit, `main` untouched, ACR = NONE. RD1.8 remains
**DELIVERED_FOR_AA_VERIFICATION**; Full Runbook D Attempt 3 NOT
AUTHORIZED; Kimi NOT AUTHORIZED; commit FORBIDDEN; Attempts 1 and 2 remain
HISTORICAL FAIL. No historical evidence was rewritten.

### 20.8 RD1.8 pre-format authoritative audit + Operator exact clang-format 19.1.5 (handover-only sync)

Documentation-only synchronization after the authoritative Windows
Operator exact clang-format 19.1.5 gate. **Only the handover pair was
modified**; every other path, including the three byte-locked RD1.8
implementation files, was left untouched. Everything below is AA-supplied
from the audit and Operator runs and was not independently verified by
this session (no execution channel; the audit/Operator evidence files were
not read).

**1. RD1.8 Pre-Format Authoritative Delta Audit v1 = PASS.** Audit:
`C:\Users\abdallah\Downloads\P0-T003-RD1-8-AUDIT\PRE-FORMAT-v1-20260913-103326-2c726509`.
It proved: candidate paths = 59 exact; RD1.8 changed = 5 exact; RD1.8
untouched = 54; Claude delivery SHA lock = 5/5 MATCH; renderer RD1.8-01
semantic contract = PASS; evidence backend identity contract = PASS;
viewport-spike PowerShell parser = PASS; viewport backend identity contract
= PASS; handover JSON parse = PASS; staged = 0; `main` unchanged + clean;
ACR = NONE.

**2. Windows Operator exact clang-format 19.1.5 = PASS** ("P0-T003 RD1.8
OPERATOR EXACT CLANG-FORMAT 19.1.5 v1: PASS"). Run:
`C:\Users\abdallah\Downloads\P0-T003-RD1-8-OPERATOR\RD1-8-FORMAT-20260913-103505-55b82011`
(hash lock `rd1-8-operator-format-hash-lock.tsv`, result `result.json`,
both under that folder). Evidence: exact clang-format 19.1.5 confirmed;
formatted targets = 2 exact (`src/desktop/src/evidence_mode.cpp`,
`src/viewport/bgfx/src/renderer.cpp`); dry-run = 2/2 PASS; **formatter
changed targets = 0**; `evidence_mode.cpp` pre/post SHA identical;
`renderer.cpp` pre/post SHA identical; candidate paths = 59 exact; RD1.8
changed paths = 5 exact; non-target candidate hashes = 57/57 unchanged;
staged = 0; `main` = UNCHANGED + CLEAN; ACR = NONE.

**3. Both RD1.8 C++ targets were already exactly compliant** - formatter
changed target count = 0. (Because the Operator formatted the *current*
`evidence_mode.cpp`, this also covers the RD1.7-era delta to that file
recorded as pending in section 19.5 - byte-identical result; section 19 is
left as the historical record it is.)

**4./5. Byte-locked RD1.8 implementation files** - re-staged from the
worktree and SHA-256-verified under this sync, and again unchanged after
delivery (`device_list_dir` sizes/mtimes):

| File | Bytes | SHA-256 |
|------|-------|---------|
| `src/viewport/bgfx/src/renderer.cpp` | 28808 | `a1789435143a21b85f8b410d920208064fda757e849e75d29573940879b727bd` |
| `src/desktop/src/evidence_mode.cpp` | 65880 | `082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981` |
| `scripts/ci/viewport-spike.ps1` | 17393 | `1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb` |

**6.** Every statement in this RD1.8 record that said exact clang-format
19.1.5 was still pending for RD1.8 has been updated (summary block, 20.5,
20.6, 20.7) - it is no longer pending. Frozen comparison baseline remains
`P0-T003-RD1-7-CORRECTION-20260913-005610`; total RD1.8 changed-path set
relative to it remains exactly the authorized five paths.

**7.-13. Status:** RD1.8 remains **DELIVERED_FOR_AA_VERIFICATION** - NOT
yet accepted or frozen. **Targeted post-RD1.8 verification has NOT yet
run.** Full Runbook D Attempt 3 NOT AUTHORIZED; Kimi NOT AUTHORIZED; commit
FORBIDDEN (none performed); Full Runbook D Attempts 1 and 2 remain
HISTORICAL FAIL; ACR = NONE. No build, CTest, clang-tidy, viewport-spike,
or Runbook D was run by this session. No staging; `main` untouched. No
historical evidence rewritten or reinterpreted.

### 20.9 RD1.8 Amendment A2 - exception-boundary minimum delta (`renderer.cpp`)

Frozen correction baseline for this amendment:
`P0-T003-RD1-8-CORRECTION-20260913-105611` (ZIP SHA-256
`b243419473c1b520760027d5cb372476507453918580ff2e1a276d9f46d34946`, 400631
bytes) - AA-supplied, NOT an accepted implementation release. Full Runbook
D Attempt 3 remains NOT AUTHORIZED.

**1. AA-reported RD1.8 Targeted Post-Correction Verification v3
(AA-supplied, not independently executed by this session):** frozen
snapshot lock 59/59 PASS; exact Windows toolchain PASS; Full Project Build
PASS; Full CTest PASS; clang-tidy 19.1.5 **16/17 TUs PASS**.

**2. The one failing TU:** `src/viewport/bgfx/src/renderer.cpp`, exact
diagnostic as supplied by AA:

```
renderer.cpp:282:33:
error: an exception may be thrown in function 'Initialize'
which should not throw exceptions
[bugprone-exception-escape,-warnings-as-errors]
```

on `bim::viewport::Status Renderer::Initialize(const RendererCreateInfo&
info) noexcept`. AA reproduced this independently with the same sanitized
compile database and classified it as a **REAL SOURCE FINDING**, not a
harness failure.

**3. Root cause (AA's diagnosis, adopted here):** RD1.8-01's new
executable-relative shader-path resolution - `RunningExecutableDirectory()`,
`std::filesystem::path` construction/concatenation,
`std::wstring`/`std::string` allocation/construction - can throw
(`std::bad_alloc`, `std::filesystem::filesystem_error`), and this call
chain, reached through `LoadCompiledShader()`, was called from
`Renderer::Initialize(...) noexcept` without being caught anywhere. Before
RD1.8 this TU passed clang-tidy.

**4. Authorized scope for this amendment:** exactly 3 paths -
`src/viewport/bgfx/src/renderer.cpp`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. Must NOT modify:
`renderer.hpp`, `renderer_impl.hpp`, `RendererCreateInfo`,
`evidence_mode.cpp`, `scripts/ci/viewport-spike.ps1`,
`Verification-RunbookD-v1.0.ps1`, CMake, tests, architecture checker,
`.clang-tidy`, any public viewport API.

**5. The correction applied, in `renderer.cpp` only:**
`LoadCompiledShader(const char* backend_dir, const char* name)` is now
declared `noexcept` (it was not before), and its entire existing body -
the `RunningExecutableDirectory()` call, `std::filesystem::path`
construction/concatenation, the shader path's `std::string` allocation,
the platform-specific file open (`_wfopen_s` under MSVC / `std::fopen`
elsewhere, both unchanged), `std::fseek`/`std::ftell`/`std::fread`, and
`bgfx::createShader(mem)` - now executes inside
`try { ... } catch (...) { return BGFX_INVALID_HANDLE; }`. Every
potentially-throwing operation RD1.8-01 introduced is inside this one try
block, so an exception raised anywhere in it is caught right here and
converted to `LoadCompiledShader()`'s own pre-existing fail-closed
sentinel, `BGFX_INVALID_HANDLE` - the same sentinel every other failure
path in this function already returns (empty executable directory, failed
file open, non-positive file size, short read). `Renderer::Initialize()`
already treats `BGFX_INVALID_HANDLE` from either shader load as an
ordinary failure via its existing
`if (!bgfx::isValid(vs) || !bgfx::isValid(fs))` check (unchanged), so no
new failure vocabulary was introduced and no fail-closed behavior was
weakened. The catch clause is the only new control-flow construct; no
other statement, comment, or line in the file outside this one function
was touched.

**6. Why this genuinely closes the finding (not just relabels it):** the
critical risk AA flagged is a function marked `noexcept` while still
containing uncaught throwing operations, which would replace a compile-time
clang-tidy diagnostic with a runtime `std::terminate()`. That is not what
was done here - every operation capable of throwing inside
`LoadCompiledShader()` executes inside the `try` block, and the `catch (...)`
handles every exception type, so no exception can propagate out of this
`noexcept` function. Nothing after the file-open call can throw in the
first place (`std::fseek`, `std::ftell`, `std::fread`, `bgfx::alloc`, and
`bgfx::createShader` are all C-style bgfx/libc APIs, none of which throw
C++ exceptions), so wrapping the whole function body - rather than only the
path-construction prefix - introduces no resource-leak risk beyond what
already existed, while keeping the exception boundary a single, easily
auditable block.

**7. Confirmations:**

| Requirement | Status |
|---|---|
| `Renderer::Initialize()` remains `noexcept`, otherwise untouched | Confirmed - zero bytes of `Initialize()` itself changed |
| Executable-relative shader resolution intent preserved (`<exe-dir>\shaders\dx11\<shader>.bin`) | Confirmed - path construction logic unchanged, only wrapped |
| Windows Unicode-correct wide-path `_wfopen_s` open preserved | Confirmed - unchanged, now inside the try block |
| Headless/Noop still never requires D3D11 shader files | Confirmed - `LoadCompiledShader()` is still only called from the `!info.headless` branch of `Initialize()`, unchanged |
| No CWD fallback / no `SetCurrentDirectory` / no `chdir` | Confirmed - none added |
| No environment-variable fallback | Confirmed - none added |
| No hard-coded repo/build path | Confirmed - none added |
| No public API change | Confirmed - `LoadCompiledShader()` and `RunningExecutableDirectory()` are both private, anonymous-namespace, translation-unit-local |
| No NOLINT / no clang-tidy suppression / no warning-policy change | Confirmed - none added; the fix is a genuine exception boundary, not a suppression |
| `evidence_mode.cpp` byte-identical to locked hash | Confirmed - `082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981`, re-staged and re-verified, untouched |
| `scripts/ci/viewport-spike.ps1` byte-identical to locked hash | Confirmed - `1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb`, re-staged and re-verified, untouched |
| Full Runbook D was not run | Confirmed |
| Kimi was not invoked | Confirmed |
| No staging performed | Confirmed |
| No commit performed | Confirmed |

**8. Exact clang-format 19.1.5 availability:** unavailable in this Linux
sandbox (only 18.x is present; not substituted, no mass-format performed).
Windows Operator exact-format is required for `renderer.cpp` again as a
result of this amendment's edit, exactly as it was after every prior
RD1.x/A1 source change.

**9. Delivered files for this amendment (this session's self-reported
values; independently re-verified via `device_list_dir` after delivery -
see the JSON companion's `aa_rd1_8_corrections.amendment_a2` for the exact
recorded sizes/hashes):**

| File | Bytes | SHA-256 |
|------|-------|---------|
| `src/viewport/bgfx/src/renderer.cpp` | 31272 | `8ac2366140ef06294872f18947635fdbe0b827c82d2359ba151a1ee5a491ad4b` |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` | *(self-referential - see file size on disk; this document cannot record its own post-write hash)* | *(self-referential)* |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.json` | *(self-referential)* | *(self-referential)* |

**10. Status:** RD1.8 remains **DELIVERED_FOR_AA_VERIFICATION**. This
amendment is **NOT** described as accepted or verified. Frozen snapshot
`P0-T003-RD1-8-CORRECTION-20260913-105611` remains the A2 comparison
baseline. Full Runbook D Attempt 3 remains **NOT AUTHORIZED**; Kimi
remains **NOT AUTHORIZED**; commit remains **FORBIDDEN**; Full Runbook D
Attempts 1 and 2 remain **HISTORICAL FAIL**; ACR = **NONE**. No build,
CTest, clang-tidy, viewport-spike, or Runbook D was run by this session -
section 20.9.1's v3 results are entirely AA-supplied. No staging; `main`
untouched. No historical evidence (sections 0-20.8) rewritten or
reinterpreted.

### 20.10 RD1.8 Amendment A2 post-format handover-only sync

Documentation-only synchronization after the authoritative Windows
Operator exact clang-format 19.1.5 gate on the Amendment A2 renderer
delta. **Only the handover pair was modified by this sync**; `renderer.cpp`,
`evidence_mode.cpp`, and `viewport-spike.ps1` were left untouched -
re-staged from the worktree and SHA-256-verified before this sync's
handover edits, and confirmed unchanged after delivery. Everything below
is AA-supplied from the audit and Operator runs and was not independently
verified by this session (no execution channel; the audit/Operator
evidence files were not read).

**1. RD1.8 A2 Pre-Format Authoritative Delta Audit v2 = PASS.** Audit:
`C:\Users\abdallah\Downloads\P0-T003-RD1-8-A2-AUDIT\PRE-FORMAT-v2-20260913-120348-569735e1`.
It proved: candidate paths = 59 exact; A2 changed = 3 exact; A2 untouched =
56; delivery SHA lock = 3/3 MATCH; `evidence_mode.cpp` lock = PASS;
`viewport-spike.ps1` lock = PASS; private renderer exception boundary =
PASS; `Renderer::Initialize` remains `noexcept`; executable-relative
shader resolution preserved; no CWD/env fallback; handover JSON parse =
PASS; staged = 0; `main` unchanged + clean; ACR = NONE.

**2. Windows Operator exact clang-format 19.1.5 = PASS** ("P0-T003 RD1.8
A2 OPERATOR EXACT CLANG-FORMAT 19.1.5 v1: PASS"). Run:
`C:\Users\abdallah\Downloads\P0-T003-RD1-8-A2-OPERATOR\A2-FORMAT-v1-20260913-120757-fc2d69b1`
(hash lock `a2-operator-format-hash-lock.tsv` under that folder). Evidence:
exact clang-format 19.1.5 confirmed; formatted targets = 1 exact
(`src/viewport/bgfx/src/renderer.cpp`); dry-run = 1/1 PASS; **formatter
changed renderer = False**; renderer pre/post SHA identical; renderer
pre/post bytes = 31272; candidate paths = 59 exact; A2 changed paths = 3
exact; non-target candidate hashes = 58/58 unchanged; staged = 0; `main` =
UNCHANGED + CLEAN; ACR = NONE.

**3. `renderer.cpp`'s Amendment A2 delta was already exactly
format-compliant** - formatter changed renderer = False, pre/post SHA
identical. Exact clang-format 19.1.5 is therefore no longer pending for
Amendment A2.

**4. Final locked renderer identity (unchanged by this sync, re-verified):**

| File | Bytes | SHA-256 |
|------|-------|---------|
| `src/viewport/bgfx/src/renderer.cpp` | 31272 | `8ac2366140ef06294872f18947635fdbe0b827c82d2359ba151a1ee5a491ad4b` |
| `src/desktop/src/evidence_mode.cpp` | 65880 | `082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981` |
| `scripts/ci/viewport-spike.ps1` | 17393 | `1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb` |

**5.** Every Amendment A2 statement that said exact clang-format 19.1.5
was still pending for A2 (section 20.9 item 8, and the amendment_a2
`clang_format_availability_disclosure` in the JSON companion) has been
updated - it is no longer pending as a *tool-availability* blocker for
this specific A2 delta (the Operator has now run it and it passed clean);
this session's own Linux sandbox still has no exact clang-format 19.1.5
available, which remains disclosed as-is for any *future* edit to this
file.

**6.-12. Status:** Amendment A2 is **NOT** marked accepted. **A2 targeted
post-correction verification has NOT yet run.** Full Runbook D Attempt 3
remains **NOT AUTHORIZED**; Kimi remains **NOT AUTHORIZED**; commit
remains **FORBIDDEN** (none performed); Full Runbook D Attempts 1 and 2
remain **HISTORICAL FAIL**; ACR = **NONE**. No build, CTest, clang-tidy,
viewport-spike, or Runbook D was run by this session. No staging; `main`
untouched. No historical evidence (sections 0-20.9) rewritten or
reinterpreted.

### 20.11 RD1.8 Amendment A3 - Runbook D verifier correction (`Verification-RunbookD-v1.0.ps1`)

**Note on the previous subsection:** section 20.10 item 6 said "Full
Runbook D Attempt 3 remains NOT AUTHORIZED" because it was written before
Attempt 3 occurred. That statement is left as the historical record it is;
Attempt 3 has since run and is recorded below.

**1. Frozen candidate for this amendment:**
`P0-T003-RD1-8-A2-CORRECTION-20260913-121601` (410056 bytes, ZIP SHA-256
`b414b67100607143ed3aa546bef08a249361e8569fa527ffc8ab59314caee8ac`). The
candidate production/source implementation remained frozen throughout -
this amendment touches only the Runbook verifier script and the handover
pair.

**2. Full Runbook D attempt history:** Attempt 1 = **HISTORICAL FAIL**.
Attempt 2 = **HISTORICAL FAIL**. Attempt 3 = **HISTORICAL FAIL** (new this
round). None of the three is rewritten as PASS.

**3. Attempt 3 authoritative facts (AA-supplied; run
`C:\Users\abdallah\Downloads\P0-T003-RUNBOOK-D-LOGS\ATTEMPT3-20260913-125946-459d70c1`):**
process exit code = 0; explicit global verdict printed = PASS; final
summary displayed 62 PASS rows and 1 FAIL row (Check 24); yet the Runbook
also printed `Total checks: 63  PASS: 63` and `VERIFICATION RUNBOOK D =
PASS`. Architecture Authority classified Attempt 3 itself as **FAIL**
because of this internal contradiction, independent of Check 24's own
result.

**4. Check 24 root cause (AA's diagnosis, adopted here):** the raw
`compile_commands.json` genuinely has 42 entries with exactly one
`/Zc:preprocessor` token, owned by `src/viewport/bgfx/src/renderer.cpp`
(fresh Attempt-3 raw compdb SHA-256
`966b0197b81d3330851296865abb75d95caab2ee3fe426ae621bf61a5dd245fc`). A
read-only diagnostic normalized the parsed top-level array correctly,
removed exactly that one token from a *temporary* analysis-only compdb,
and ran exact clang-tidy 19.1.5 against the exact Runbook first-party
source set: 11 translation units, 11/11 exit 0, zero TU failures, zero
project diagnostic lines, zero remaining `/Zc` tokens in the temporary DB,
raw compdb unchanged. The Runbook's own `$rawEntries = @($rawText |
ConvertFrom-Json)` line was not reliably normalizing the parsed top-level
JSON array into individual compile-entry objects under Windows PowerShell
5.1, so the entry-level filter immediately below it observed zero
entries. Classification: **RUNBOOK_CHECK24_HARNESS_OR_COMPDB_WIRING_DEFECT.**
Production source/CMake/raw compile DB require no correction.

**5. Second proven Runbook defect (final tally/verdict):** Attempt 3's
contradictory result (a printed `[FAIL] Check 24` row alongside `Total
checks: 63 PASS: 63` and a PASS verdict, exit 0) is impossible for a
correct tally. Root cause: `$failed = $script:Checks | Where-Object {
$_.Status -eq 'FAIL' }` returns a scalar `PSCustomObject`, not a
one-element array, whenever exactly one check fails - a documented
PowerShell `Where-Object` behavior (zero matches -> `$null`; exactly one
match -> that single object; more than one -> an array). A scalar has no
`.Count` property, so `$failed.Count` silently evaluated to `$null`;
`$total - $null` arithmetic-coerces the `$null` to `0`, so the printed
PASS count came out as the full total (63) and the printed FAIL count
came out blank; `$null -gt 0` is `$false` in PowerShell, so the `exit 1`
branch was never taken.

**6. Authorized A3 scope:** exactly 3 paths -
`Verification-RunbookD-v1.0.ps1`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. Must NOT modify:
`src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/evidence_mode.cpp`,
`scripts/ci/viewport-spike.ps1`, any `CMakeLists.txt`, `.clang-tidy`,
tests, architecture scripts, production source/header/API, shaders, or
vcpkg files.

**7. Correction 1 applied - Check 24 JSON-array normalization
(`Verification-RunbookD-v1.0.ps1` only):** the raw textual single-token
assertion (`$rawTokenMatches.Count -ne 1`) is unchanged. Immediately after
it, `ConvertFrom-Json` is now called directly on `$rawText` via
`-InputObject` (not piped, so no pipeline-driven reshaping of its output
can occur), and the parsed root is explicitly rebuilt into a flat
one-dimensional array one item at a time:

```powershell
$parsedRoot = ConvertFrom-Json -InputObject $rawText
$rawEntriesList = New-Object System.Collections.Generic.List[object]
if ($null -ne $parsedRoot) {
    if (($parsedRoot -is [System.Collections.IEnumerable]) -and -not ($parsedRoot -is [string])) {
        foreach ($item in $parsedRoot) { [void]$rawEntriesList.Add($item) }
    } else {
        [void]$rawEntriesList.Add($parsedRoot)
    }
}
$rawEntries = @($rawEntriesList.ToArray())
if ($rawEntries.Count -eq 0) {
    throw "raw compile_commands.json normalized to zero parsed compile entries (expected > 0 - ...). Fail-closed: not proceeding with an empty entry set."
}
```

This normalizes correctly whether the parsed root came back as an array,
a single object (a documented `ConvertFrom-Json` single-element-array
collapse), or anything else enumerable - it never assumes the pipe-based
form yields entries directly. Every existing downstream assertion is
preserved exactly, unchanged in meaning: `$entriesWithToken.Count -ne 1`
(now also reporting the normalized entry count in its error message),
`$expectedFileSuffix`/owner check, the exactly-one-removable-occurrence
check, the temporary-DB-zero-remaining-tokens check, and the
authoritative-raw-hash-unchanged check. No production CMake flag was
changed; `/Zc:preprocessor` was not removed from the raw DB;
`renderer.cpp` was not edited; no NOLINT was added; no clang-tidy check
was disabled; none of the one-entry/one-token fail-closed assertions were
weakened. Exact clang-tidy 19.1.5 execution across the existing
first-party source set is unchanged.

**8. Correction 2 applied - authoritative final tally/verdict
(`Verification-RunbookD-v1.0.ps1` only):**

```powershell
$allChecks = @($script:Checks)
$checkIsPass = @($allChecks | ForEach-Object { [bool]($_.Status -eq 'PASS') })

$totalCount = $allChecks.Count
$passCount = @($checkIsPass | Where-Object { $_ -eq $true }).Count
$failCount = @($checkIsPass | Where-Object { $_ -eq $false }).Count

# ... per-check row printing driven by $checkIsPass, not by re-querying
# $script:Checks for its Status string a second time ...

$invariantsHold = (
    ($totalCount -eq $script:ExpectedTotalCheckCount) -and
    ($passCount -eq $totalCount) -and
    ($failCount -eq 0) -and
    (($passCount + $failCount) -eq $totalCount)
)

if (-not $invariantsHold) {
    Write-Host "VERIFICATION RUNBOOK D = FAIL (total=$totalCount expected=$($script:ExpectedTotalCheckCount) pass=$passCount fail=$failCount)" -ForegroundColor Red
    exit 1
}
Write-Host 'VERIFICATION RUNBOOK D = PASS' -ForegroundColor Green
exit 0
```

`$script:ExpectedTotalCheckCount = 63` is declared once near
`$script:Checks`'s initialization, documented as this frozen candidate's
fixed check inventory. Every collection is forced through `@(...)` before
`.Count` is read, so a single-match (or zero-match) `Where-Object` result
can never again silently collapse to a non-array scalar or `$null`. Each
check's status is read as an explicit boolean; the printed per-check rows,
the printed counts, the exit code, and the global verdict are all derived
from that one same boolean array - none independently recomputed or able
to disagree with each other. Check 24 is not hard-coded as PASS; no failed
check is bypassed; the meaning/scope of all 63 checks is unchanged.

**9. Self-test (read-only, isolated; no full Runbook D execution; no
PowerShell runtime available in this sandbox, so performed as a faithful
Python transliteration of both corrected algorithms - explicitly not an
execution of the real `.ps1` file or the real toolchain):**

*Check 24 normalization* - synthetic 42-entry compdb, exactly one
`/Zc:preprocessor` token, owner `renderer.cpp`: parsed entry count = 42
(> 0, PASS); entries carrying the token = 1 (PASS); owner = renderer.cpp
(PASS); exactly one token textually removed from the temporary copy,
zero remaining in it (PASS); authoritative raw text SHA-256 unchanged
before/after (PASS). All 6 required assertions held.

*Tally logic* - two synthetic result sets:

| Case | Input | total | pass | fail | verdict | exit |
|---|---|---|---|---|---|---|
| 1 | 63 PASS | 63 | 63 | 0 | PASS | 0 |
| 2 | 62 PASS + 1 FAIL | 63 | 62 | 1 | FAIL | 1 |

Both match AA's required contract exactly. This trace validates the
corrected algorithm's arithmetic/boolean logic; it does not reproduce the
specific PowerShell `Where-Object` scalar-collapse defect in Python
(Python's list comprehensions never collapse a one-element result to a
scalar), so it is a forward validation of the new code, not a
reproduction of the old bug.

**10. Locked files confirmed byte-identical, untouched, re-staged and
hash-verified before and after this amendment:**

| File | Bytes | SHA-256 |
|------|-------|---------|
| `src/viewport/bgfx/src/renderer.cpp` | 31272 | `8ac2366140ef06294872f18947635fdbe0b827c82d2359ba151a1ee5a491ad4b` |
| `src/desktop/src/evidence_mode.cpp` | 65880 | `082c7869e96ff8d70b596829033d9ca307692ff5f0c718387ed8f95c57a00981` |
| `scripts/ci/viewport-spike.ps1` | 17393 | `1712e44c46c70cd041b6c7c85a10983744fb10f99476abb2b6d9a7b6ffe569fb` |

**11. Delivered file for this amendment (this session's self-reported
value; independently re-verified via `device_list_dir` after delivery -
see the JSON companion's `aa_rd1_8_corrections.amendment_a3` for the exact
recorded size/hash):**

| File | Bytes | SHA-256 |
|------|-------|---------|
| `Verification-RunbookD-v1.0.ps1` | *(see delivery self-audit in this session's response)* | *(see delivery self-audit)* |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` | *(self-referential)* | *(self-referential)* |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.json` | *(self-referential)* | *(self-referential)* |

**12. Status:** **Amendment A3 is NOT marked accepted or verified.** RD1.8
remains **DELIVERED_FOR_AA_VERIFICATION**. Attempts 1, 2, and 3 all remain
**HISTORICAL FAIL**. Full Runbook D Attempt 4 is **NOT AUTHORIZED**. Kimi
remains **NOT AUTHORIZED**; not invoked. Commit remains **FORBIDDEN**;
none performed. ACR = **NONE**. No staging; `main` untouched. No
historical evidence (sections 0-20.10) rewritten or reinterpreted. No
build, CTest, clang-tidy, viewport-spike, or Full Runbook D was run by
this session.

### 20.12 RD1.8 Amendment A4 - Runbook Check 24 native-stderr correction (`Verification-RunbookD-v1.0.ps1`)

**Note on the previous subsection:** section 20.11 item 12 said "Full
Runbook D Attempt 4 is NOT AUTHORIZED" because it was written before
Attempt 4 occurred. That statement is left as the historical record it
is; Attempt 4 has since run and is recorded below.

**1. Frozen candidate for this amendment:**
`P0-T003-RD1-8-A3-CORRECTION-20260913-170644` (411302 bytes, ZIP SHA-256
`842eb5e836387b82b8dbf9acfdc4b567e1d35ef68ce3f5203540d608a225106a`).
Candidate footprint unchanged: 12 MODIFY + 47 ADD = 59 total.

**2. Full Runbook D attempt history:** Attempt 1 = **HISTORICAL FAIL**.
Attempt 2 = **HISTORICAL FAIL**. Attempt 3 = **HISTORICAL FAIL**. Attempt
4 = **HISTORICAL FAIL** (new this round). None of the four is rewritten
as PASS.

**3. Attempt 4 authoritative result (AA-supplied; run
`C:\Users\abdallah\Downloads\P0-T003-RUNBOOK-D-LOGS\ATTEMPT4-20260913-171538-1513edf4`):**
63 total checks, 62 PASS, 1 FAIL (Check 24), global verdict **FAIL**,
process exit **1**. This is an internally *consistent* result - unlike
Attempt 3's contradiction, Attempt 4's printed tally, printed rows, exit
code, and global verdict all agree. **This is direct evidence that the
RD1.8 Amendment A3 final tally/global-verdict correction is working
correctly in a real Runbook run**, and that correction is therefore left
completely untouched by this amendment (see item 9 below).

**4. Attempt 4 Check 24 diagnostic (formally closed; AA-supplied, not
independently verified by this session):** diagnostic run
`C:\Users\abdallah\Downloads\P0-T003-ATTEMPT4-DIAGNOSTIC\CHECK24-v1-20260913-172551-879d526c`,
with `result.json` and `completion-receipt.json`. Final classification:
**`RUNBOOK_CHECK24_NATIVE_STDERR_ERRORACTIONPREFERENCE_DEFECT`.** Proven
facts: (1) A3's `compile_commands.json` normalization is correct on the
fresh Attempt 4 database; (2) the fresh raw compdb SHA-256 is
`966b0197b81d3330851296865abb75d95caab2ee3fe426ae621bf61a5dd245fc`; (3) it
contains exactly 42 entries; (4) exactly one `/Zc:preprocessor` occurrence
exists; (5) its owner is `src/viewport/bgfx/src/renderer.cpp`; (6) the
temporary analysis copy removes exactly that one token; (7) the raw
compdb remains unchanged; (8) with native stderr handled as
non-terminating, the exact Check 24 first-party source set (11 TUs) is
11/11 exit 0 with zero project diagnostics; (9) under a Runbook-like
`$ErrorActionPreference = 'Stop'` probe, all 11 clang-tidy invocations
*throw* - examples: `evidence_mode.cpp: exception: 63246 warnings
generated.`; `main.cpp: exception: 61768 warnings generated.`;
`renderer.cpp: exception: 55996 warnings generated.`; (10) the final
diagnostic left the frozen A3 snapshot at 59/59 PASS, staged = 0, `main`
UNCHANGED + CLEAN, ACR = NONE.

**5. Root cause:** Check 24 was not failing because of production source,
`renderer.cpp`, real clang-tidy diagnostics, CMake, compdb normalization,
`/Zc` token count, or the tally logic. It failed because the Runbook
invoked native clang-tidy directly while this script's global
`$ErrorActionPreference = 'Stop'` promoted clang-tidy's own routine stderr
("N warnings generated.") into terminating PowerShell exceptions - a
Runbook-only native-process handling defect.

**6. Authorized A4 scope:** exactly 3 paths -
`Verification-RunbookD-v1.0.ps1`, `docs/evidence/P0-T003/CLAUDE_HANDOVER.md`,
`docs/evidence/P0-T003/CLAUDE_HANDOVER.json`. Must NOT modify:
`src/viewport/bgfx/src/renderer.cpp`, `src/desktop/src/evidence_mode.cpp`,
`scripts/ci/viewport-spike.ps1`, CMake, tests, shaders, `.clang-tidy`,
architecture checker, public API, vcpkg files, or any other production
source/header.

**7. Correction applied
(`Verification-RunbookD-v1.0.ps1` only, inside Check 24's clang-tidy
loop):** the prior inline invocation

```powershell
& $clangTidy.Source '-p' $analysisDir $file.FullName 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    $failed += $file.FullName
}
```

is replaced with a call to this project's own pre-existing `Invoke-Native`
helper, already defined in `scripts/ci/_common.ps1` (dot-sourced at the
top of this script) and already used for every other native invocation in
this file (cmake configure/build, ctest, the PowerShell child scripts,
evidence-mode) - the "existing project/runbook native-command handling
pattern" AA's instruction pointed to:

```powershell
$tidyResult = Invoke-Native -Exe $clangTidy.Source -CmdArgs @('-p', $analysisDir, $file.FullName) -AllowFailure
if ($tidyResult.ExitCode -ne 0) {
    $failed += $file.FullName
}
```

`Invoke-Native`'s own (unmodified, read-only-inspected) implementation
already does exactly what A4 requires: it scopes
`$ErrorActionPreference = 'Continue'` only for the duration of the native
call (saved and restored via `finally`, so this Runbook's script-wide
`$ErrorActionPreference = 'Stop'` at the top of the file is never
touched), separates stdout and stderr text without discarding either
(both are printed via `Write-Host`, matching how every other check in
this script surfaces native output), and reads `$LASTEXITCODE`
immediately after the call as the sole authority for success/failure -
stderr presence alone is never treated as failure. `-AllowFailure` makes
`Invoke-Native` return its result object instead of throwing internally,
so one failing TU cannot abort this `foreach` loop - the exact same
per-file `$failed` accumulation shape as before is preserved. A non-zero
`$tidyResult.ExitCode` still fails the file exactly as before: this
repo's `.clang-tidy` `WarningsAsErrors` policy is what turns a real
project diagnostic into a non-zero clang-tidy exit code, unchanged and
not newly parsed by this amendment.

**8. Requirements satisfied, item by item:** (1) exact clang-tidy 19.1.5
still invoked, unchanged flags (`-p $analysisDir $file.FullName`); (2)
stdout+stderr captured as text by `Invoke-Native`, not discarded; (3)
native exit code captured reliably via `$LASTEXITCODE`, read immediately
after the call; (4) stderr presence alone is never treated as failure -
only `$tidyResult.ExitCode -ne 0` is; (5) non-zero native exit code still
fails the check; (6) project-diagnostic detection is unchanged (still
exit-code-based, per `.clang-tidy`'s `WarningsAsErrors` policy - nothing
newly parsed); (7) the exact 11-TU Check 24 source set
(`Get-ChildItem ... src\viewport, src\desktop -Recurse -Include '*.cpp'`)
is byte-identical, unchanged; (8) the A3 temporary analysis compdb
(`$analysisDir`/`$analysisCompileCommands`, the removal-pattern logic, the
raw-hash-unchanged assertion) is byte-identical, unchanged; (9) every A3
fail-closed assertion (raw token count exactly one; normalized entry set
non-empty; exactly one entry carries the token; owner is renderer.cpp;
exactly one token removed; temp compdb has zero remaining tokens; raw
compdb hash unchanged) is byte-identical, unchanged.

**9. A3 logic confirmed unchanged:** both the Check 24 JSON-array
normalization block (§20.11 item 7) and the final tally/global-verdict
block (§20.11 item 8, including `$script:ExpectedTotalCheckCount = 63`)
are **byte-for-byte identical** to their A3-delivered state - confirmed by
diffing this amendment's full file against the freshly re-staged pre-edit
worktree baseline, which shows exactly one changed region (Check 24's
clang-tidy invocation block, item 7 above) and nothing else. This is
consistent with Attempt 4's own evidence that the A3 tally/verdict
correction already works correctly (item 3 above) and therefore needed no
further change.

**10. What was explicitly NOT done:** the script-wide
`$ErrorActionPreference = 'Stop'` was not weakened or removed; no
PowerShell errors were globally suppressed; clang-tidy's exit code is not
ignored; stderr is not discarded; no actual diagnostic is hidden; no
NOLINT was added; no warning policy was changed; Check 24 is not
bypassed; `renderer.cpp`, `evidence_mode.cpp`, `scripts/ci/viewport-spike.ps1`,
CMake, tests, shaders, `.clang-tidy`, the architecture checker, any public
API, and vcpkg files were all left untouched.

**11. Self-test (read-only, isolated; explicitly NOT a substitute-runtime
validation):** this sandbox has no Windows PowerShell runtime of any kind
(confirmed: neither `pwsh` nor `powershell` is present), so the corrected
`.ps1` file cannot be executed here regardless of any other tooling
present. A Linux `clang-tidy` binary does exist in this sandbox (version
18.1.3, confirmed via `clang-tidy --version`) but it is neither the exact
required 19.1.5 nor reachable through any PowerShell execution path, and
the defect this amendment fixes is a PowerShell-specific
`$ErrorActionPreference`/native-stderr-promotion behavior that does not
exist in a bash/Linux invocation - so per Architecture Authority's
explicit instruction, this session did **not** substitute the Linux
clang-tidy (or any other runtime) for the real Windows PowerShell/clang-tidy
19.1.5 combination and did **not** call any such substitute
authoritative. Only a **static self-audit** was performed: (a) the diff
against the freshly re-staged pre-edit worktree baseline was reviewed and
confirmed scoped to exactly the one intended block (item 9 above); (b) a
PowerShell-aware brace/paren/bracket balance sweep (established
convention: a Python stripper handling `<# #>` block comments, `#` line
comments, `'...'` with `''`-escaping, and `"..."` with backtick/`""`
escaping) was run against the full file and reported balanced; (c)
`Invoke-Native`'s own pre-existing implementation in
`scripts/ci/_common.ps1` was read (not modified) and confirmed to already
implement the exact `$ErrorActionPreference`-scoping / `finally`-restore /
`$LASTEXITCODE`-authority / stdout-stderr-capture pattern A4 requires,
already proven safe for every other native call in this Runbook. **None
of AA's ten enumerated self-test items (PowerShell parser clean; fresh
compdb unchanged; 42 normalized entries; exactly one `/Zc` token; owner =
renderer.cpp; temp copy removes exactly one token; the 11 first-party TUs
run through the new invocation; 11/11 native exits = 0; project diagnostic
count = 0; stderr text captured without itself throwing) was behaviorally
executed or proven by this session** - items concerning A3's compdb logic
(3-6) are supported only by that code being byte-identical and unchanged
(item 9), and items requiring actual execution (1, 2, 7-10) were not
performed at all. **Windows Operator validation of Check 24's actual
runtime behavior against the real toolchain remains required** and has
not occurred as part of this delivery.

**12. Delivered file for this amendment (this session's self-reported
value; independently re-verified via `device_list_dir` after delivery -
see the JSON companion's `aa_rd1_8_corrections.amendment_a4` for the exact
recorded size/hash):**

| File | Bytes | SHA-256 |
|------|-------|---------|
| `Verification-RunbookD-v1.0.ps1` | *(see delivery self-audit in this session's response)* | *(see delivery self-audit)* |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.md` | *(self-referential)* | *(self-referential)* |
| `docs/evidence/P0-T003/CLAUDE_HANDOVER.json` | *(self-referential)* | *(self-referential)* |

**13. Status:** **Amendment A4 is NOT marked accepted or verified.** RD1.8
remains **DELIVERED_FOR_AA_VERIFICATION**. Attempts 1, 2, 3, and 4 all
remain **HISTORICAL FAIL**. Full Runbook D Attempt 5 is **NOT
AUTHORIZED**. Kimi remains **NOT AUTHORIZED**; not invoked. Commit remains
**FORBIDDEN**; none performed. ACR = **NONE**. No staging; `main`
untouched. No historical evidence (sections 0-20.11) rewritten or
reinterpreted. No build, CTest, clang-tidy, viewport-spike, or Full
Runbook D was run by this session.
