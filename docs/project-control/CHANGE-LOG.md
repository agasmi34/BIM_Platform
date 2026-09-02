# CHANGE-LOG

Dated log of substantive changes to this repository and its governing
records. Entries are in UTC-agnostic session order (this project has not
yet established a commit-based timeline, since no implementation commit
exists yet); once Phase P commits land, this log should be read alongside
`git log`, not instead of it.

## 2026-08-26

- Architecture Gate `BIM-AG-P0-T001 v1.0` issued (per its own header date).

## 2026-08-27

- Implementation Brief `BIM-TASK-P0-T001-CLAUDE v1.0` released (per its own
  header date).
- ACR-P0T001-001 raised and resolved (Option B: controlled Windows
  Execution Operator execution; repository/worktree paths set to
  `D:\Projects\BIM-Platform` / `D:\Projects\BIM-Platform-WT-P0-T001`).
- Bootstrap Runbook A authored, reviewed, and revised through v1.0 → v1.1 →
  v1.2 → v1.3 across three architecture-review rounds (see
  `TASK-LEDGER.md` rows 5-9 for the specific defect found and fixed at each
  step).
- Bootstrap Runbook A v1.3 reported executed successfully: bootstrap commit
  `4b339248dd8b050e7b603ef0b5707440e582c315` on `main`; task worktree
  created at `D:\Projects\BIM-Platform-WT-P0-T001` on branch
  `task/P0-T001-repo-toolchain-scaffold`. (Reported, not independently
  verified by Claude — see `06-BOOTSTRAP-EVIDENCE.md`.)
- Phase C resumed. vcpkg registry baseline researched and resolved to
  commit `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843` (OCCT 8.0.1,
  port-version 0).
- Phase C scaffold authored: 63 files across root policy, `cmake/`, `docs/`,
  `src/` (nine first-party targets plus six future-module boundary
  READMEs), `tests/` (six required CTest tests plus the architecture
  checker's controlled bad fixture), `tools/architecture_checker.cmake`,
  `scripts/ci/` (five provider-neutral jobs), `third_party/licenses/`.
  Self-verified where possible without Windows/MSVC/vcpkg (see
  `TASK-LEDGER.md` row 13).
- Phase C scaffold transferred into `D:\Projects\BIM-Platform-WT-P0-T001`
  via the file bridge, in 6 batches, zero files rejected. Directly
  confirmed by the bridge tool and a subsequent recursive directory listing
  plus targeted spot-checks of the two deepest paths.
- Verification Runbook B authored and delivered to the operator. Not yet
  executed.
- Architecture Addendum A1 issued (project-record & traceability system).
  `docs/project-control/` (this file and its four siblings) and
  `docs/tasks/P0-T001/` (13 files) authored and transferred into the
  isolated worktree only. **No commit made.** `main` untouched.
- "P0-T001 A1 — WINDOWS VERIFICATION CONFIRMED" received: Architecture
  Authority attested (with a stated provenance model — see
  `TASK-LEDGER.md`, "Windows verification attestation") that a raw
  PowerShell transcript from the Windows Execution Operator confirms the
  worktree's current file count (81), `docs/project-control`/`docs/tasks`
  file counts (5 / 13), branch, HEAD, untracked status, and absence of an
  implementation commit. `PROJECT-STATUS.md`, `TASK-LEDGER.md`, and
  `06-BOOTSTRAP-EVIDENCE.md` amended to record this attestation, scoped
  strictly to worktree state and explicitly not extended to the separate
  Bootstrap Runbook A v1.2/v1.3 execution narrative. Claude has not seen
  the transcript itself; no transcript content was invented. Verification
  Runbook B **not executed**; no Windows build/test begun; no commit made.

## 2026-08-30

- "P0-T001 Architecture Review — Verification Runbook B: REVISION REQUIRED"
  received. Two categories of change:
  - **Traceability correction:** Architecture Authority states it
    possesses and has reviewed actual raw PowerShell console transcripts
    (not narrated summaries) of both the v1.2 failed run and the v1.3
    successful bootstrap run. `06-BOOTSTRAP-EVIDENCE.md` (new "Bootstrap
    execution transcript provenance (correction)" section plus updated top
    banner and table headers), `TASK-LEDGER.md` (row 18 added; rows 8 and
    10 cross-referenced to it), and `PROJECT-STATUS.md` (bootstrap row)
    amended to record this using Architecture Authority's exact provenance
    model: evidence source Windows Execution Operator / Product Authority;
    evidence form raw PowerShell console transcript; Architecture Authority
    verification VERIFIED; Claude reproduction NOT AVAILABLE (Windows
    command channel unavailable); independent reviewer verification
    PENDING (Kimi). No underlying fact was changed — only the evidentiary
    label. Claude has still not seen either transcript; none of its
    content was invented. The raw transcript files themselves do not yet
    exist in this worktree; Architecture Authority states they will be
    supplied separately for `docs/evidence/P0-T001/`.
  - **Verification Runbook B revision required:** the previously delivered
    runbook was sent back with 2 BLOCKERs and 4 MAJORs (pre-mutation Git
    preflight against the approved bootstrap SHA; verifying the actual
    `vcpkg-configuration.json` baseline and effective CMake cache values
    rather than only printing expected constants; isolating each CI job in
    its own child PowerShell process; verifying an active x64 MSVC
    developer environment; requiring, not allowing failure of, required
    tool version checks; and asserting final Git invariants without
    requiring the task worktree to be clean). Verification Runbook B v1.1
    authored addressing all six findings. **Not executed.** No Windows
    build/test begun. No commit made.

- "ARCHITECTURE AUTHORITY DISPOSITION" received, three items:
  1. vcpkg schema clarification (`default-registry.baseline` vs
     `builtin-baseline`) accepted as correct for P0-T001/IC-002; no change
     made — the field stays as `default-registry.baseline`.
  2. Governing-document conflict resolved: Master Engineering Constitution
     v0.2 `D-028` (GoogleTest LOCKED) vs. the approved P0-T001 Architecture
     Gate/Implementation Brief (Catch2 v3). `docs/architecture/adr/ADR-0001-phase0-test-framework-baseline.md`
     (authoritative location — see the placement-correction entry below)
     authored: Catch2 v3 + CTest approved for all Phase 0 tasks, explicitly
     superseding `D-028` for Phase 0 only; Phase 1+ framework choice left
     PROVISIONAL; no replacement of the existing Catch2 implementation.
     `DECISION-LEDGER.md` updated with an "ADRs issued" table and a
     non-destructive supersession note. The Master Engineering Constitution
     v0.2 itself is not held in this repository and was not altered.
  3. Verification Runbook B v1.1 remains **NOT AUTHORIZED TO EXECUTE**.
     Architecture Authority requested the complete literal v1.1 script
     (not a summary), delivered as a file attachment this round. No
     build/vcpkg/CMake/CTest run. No commit made.
- "P0-T001 Architecture Record Placement Correction" received: Master
  Engineering Constitution v0.2 defines `docs/architecture/adr/` as the
  authoritative ADR location, while the P0-T001 scaffold requires
  `docs/adr/` to exist. ADR-0001 relocated: authoritative copy created at
  `docs/architecture/adr/ADR-0001-phase0-test-framework-baseline.md` (a
  "Record history" section added noting the relocation; no technical
  decision content altered), old copy removed from `docs/adr/`.
  `docs/adr/README.md` rewritten to state it holds no ADR content and to
  point to `docs/architecture/adr/`. `DECISION-LEDGER.md` and this file's
  prior entry updated to reference the new path. No build. No commit.

- "P0-T001 Architecture Review — Verification Runbook B v1.3" received:
  REVISION REQUIRED, producing v1.4. All v1.0→v1.3 findings not listed
  below unchanged/not regressed. Five fixes:
  1. **BLOCKER (`CMakePresets.json`):** removed the `$comment` field
     (invalid at the declared presets schema version 3 — `$comment` is a
     version-10 feature); presets schema version kept at 3 as intentional
     policy (earlier-maintained-CMake compatibility), not raised merely to
     keep a comment. `ci-win-msvc` preset semantics unchanged. The
     comment's content (CMake 4.4.2 as CI reference version) remains
     documented in `CMakeLists.txt`'s own header comment and the
     Architecture Gate.
  2. **BLOCKER (`.clang-tidy`):** `WarningsAsErrors` changed `'' -> '*'`
     per explicit Architecture Authority disposition — enabled clang-tidy
     findings are now CI-gating. Recorded as `DECISION-LEDGER.md` row
     D-005. Curated `Checks` list unchanged. `scripts/ci/static-analysis.ps1`
     logic unchanged (still report-only, still no `-fix`); its header
     comment updated to describe the new exit-code semantics.
  3. **MAJOR (Runbook B Step 2):** added `$env:VCToolsVersion` capture and
     a hard assertion that it belongs to the VS 2022 v143 toolset family
     (`14.3*`/`14.4*`), so a VS 17.14 shell misconfigured to an older
     v142 component can no longer pass. Added parsing of `cmake
     --version`'s actual reported version and a hard assertion that it
     equals exactly `4.4.2` (Architecture Gate section 4.1's locked P0
     reference/CI CMake version), rather than only printing it as
     unparsed evidence. Both verified values added to the Step 8 Evidence
     Summary.
  4. **MAJOR (`scripts/ci/license-inventory.ps1`):** now removes any
     previously generated `<dep>.LICENSE.txt`/`<dep>.USAGE.txt` for the
     five direct dependencies before regenerating from the current
     `vcpkg_installed` tree, so a stale artifact from an earlier run
     cannot make the current run's completeness check pass by accident.
     Non-generated files (e.g. `third_party/licenses/README.md`) are
     untouched.
  5. **MINOR:** Runbook B's final success message now says "ALL FIVE JOBS
     PASSED"; its Usage section names the actual `Verification-RunbookB-v1.4.ps1`
     script; `format.ps1`/`configure-build-test.ps1`/`architecture.ps1`/
     `license-inventory.ps1` header comments updated from the historical
     "logical job N of 4" to "N of 5" (1, 2, 4, 5 respectively —
     `static-analysis.ps1` was already correctly "3 of 5").
  Verification Runbook B v1.4 produced. **Not executed.** No
  CMake/vcpkg/MSVC/CTest/clang-format/clang-tidy run. No commit made.

- "P0-T001 Architecture Review — Verification Candidate v1.4" received: ONE
  FINAL BLOCKER, producing v1.5. All v1.0→v1.4 findings CLOSED and
  unregressed. `scripts/ci/architecture.ps1`'s single broad
  `ctest -R "^arch_"` invocation did not prove both required tests
  (`arch_repository_boundaries`, `arch_checker_detects_violation`) were
  actually registered and executed. Replaced with two explicit,
  exact-name-anchored invocations —
  `ctest -R "^arch_repository_boundaries$" --output-on-failure --no-tests=error`
  then `ctest -R "^arch_checker_detects_violation$" --output-on-failure --no-tests=error`
  — both through `Invoke-Native` without `-AllowFailure`, so a missing OR
  a failing required test both fail the job; only both found-and-passing
  reaches `ARCHITECTURE CHECK JOB PASSED`. Verification Runbook B v1.5
  produced — no change to Runbook B's own verification logic, only its
  revision history/version binding to the corrected architecture.ps1 (see
  the script's own header comment for the full before/after behavior).
  **Not executed.** No build. No stage or commit. `main` untouched.

- "P0-T001 — Verification Runbook B v1.5 First Execution Disposition"
  received. **First real Windows execution of any Verification Runbook B
  version in this task.** Findings:
  - Runbook v1.5 passed Step 0 (pre-mutation preflight) and entered
    Step 2, producing genuine evidence Claude has not previously had
    access to: `VSCMD_ARG_TGT_ARCH = x64`, `VSCMD_VER = 17.14.39`,
    `VCToolsVersion = 14.44.35207`, `cl.exe` resolved correctly.
  - It then **aborted before any of the five CI jobs ran**, with:
    `Cannot bind argument to parameter 'CmdArgs' because it is an empty
    array.`
  - **Classification: VERIFICATION HARNESS DEFECT.** Root cause: the
    runbook-local `Invoke-Native`'s
    `[Parameter(Mandatory)][string[]]$CmdArgs` rejects an empty array
    under PowerShell's Mandatory-parameter binding rules; the Step 2
    compiler-banner probe intentionally calls `Invoke-Native ... -CmdArgs
    @() -AllowFailure` (cl.exe with no arguments, by design) and was
    rejected by the parameter binder before the native call ever ran.
  - **Implementation NOT TESTED by this run.** No CI job (format,
    configure-build-test, static-analysis, architecture,
    license-inventory) executed; no CMake/vcpkg/MSVC/CTest/clang-format/
    clang-tidy evidence exists from this attempt. This is a defect in the
    harness itself, not a finding about the P0-T001 implementation.
  - **Correction:** Verification Runbook B v1.6 produced. Minimum fix
    only: the runbook-local `Invoke-Native`'s `$CmdArgs` parameter is now
    `[Parameter(Mandatory)][AllowEmptyCollection()][string[]]$CmdArgs`.
    The compiler-banner probe call itself is unchanged (same arguments,
    same `-AllowFailure`, same semantics). `scripts/ci/_common.ps1` was
    **not** modified — no child CI job currently calls `Invoke-Native`
    with an empty argument array. Diffed against v1.5: only the version
    header/changelog block, the `Usage` line, and this single
    `AllowEmptyCollection()` addition changed; every other line of
    previously-accepted Runbook B logic is unchanged.
  **Not executed.** No build. No child CI script modified. No stage or
  commit. `main` untouched.

- "P0-T001 — Verification v1.6 Failure Disposition" received. **First
  complete Windows verification execution of Verification Runbook B (v1.6)
  in this task** — Step 0/Step 2 passed as before, and this run proceeded
  into the CI job sequence rather than aborting in the harness itself.
  **Classification: IMPLEMENTATION CORRECTION REQUIRED — no longer a
  verification-harness defect.** Two confirmed independent failures:
  1. **Formatting:** `scripts/ci/format.ps1` (clang-format 19.1.5, the
     Windows-side authoritative gate) rejected exactly 3 of the 16
     first-party files: `src/geometry/occt/src/geometry_occt_probe.cpp`,
     `src/persistence/src/persistence_probe.cpp`,
     `tests/integration/integration_persistence_sqlite_memory.cpp`.
     Independently corroborated locally with clang-format 18.1.3 against
     the repository's `.clang-format` (a version delta from the
     authoritative 19.1.5 exists and is noted, not reconciled) — same 3
     files flagged, same other 13 clean.
  2. **vcpkg / Catch2:** configure failed loading `catch2@3.0.0` (`"3.0.0"
     is not a valid version-database entry`). Root cause: `vcpkg.json`'s
     catch2 dependency carried an explicit `"version>=": "3.0.0"`
     constraint, and `"3.0.0"` itself does not exist as a version-database
     entry in the frozen registry baseline
     (`f89a4a1da4e3176a8d1a14c1825b9b2f98e48843`), which Architecture
     Authority independently verified resolves catch2 to baseline `3.15.3`
     (and opencascade to `8.0.1`, consistent with `DECISION-LEDGER.md`
     row D-002). The locked contract requires Catch2 major version 3, not
     the literal string `3.0.0`.
  - **Downstream jobs** (`static-analysis`, `architecture`,
    `license-inventory`) did not produce independent evidence this run —
    classified as **cascade failures** of the vcpkg/Catch2 configure
    failure (configure did not succeed, so no build directory, no compile
    database, and no `vcpkg_installed` tree existed for those jobs to
    operate against), not as separate implementation defects.
  - A "Ninja message" observed in this run's output was investigated and
    **remediation deferred as unnecessary**: an independent child-process
    probe in this run found Ninja `1.12.1` resolved successfully, so no
    correction was authorized or made in this round.
  - **Correction authorized and applied** (scope strictly limited to the
    two root causes above):
    - `vcpkg.json`: catch2's dependency entry changed from
      `{"name": "catch2", "version>=": "3.0.0"}` to the bare string
      `"catch2"`, matching the existing bare-string style already used for
      `sqlite3`/`fmt`/`spdlog`. No other dependency, ordering (beyond what
      the replacement itself required), registry configuration, baseline
      SHA, or the `opencascade` entry was touched. The frozen baseline now
      resolves catch2 to `3.15.3` — major version 3, satisfying
      `DECISION-LEDGER.md` row D-003 / ADR-0001.
    - The 3 flagged files reformatted to `.clang-format` conformance:
      purely mechanical whitespace/continuation-layout and
      `SortIncludes`-driven include-order corrections (one `#include` pair
      reordered in the integration test). No identifier, expression,
      control-flow, literal, or API change in any of the three files.
  - `vcpkg-configuration.json`, `CMakeLists.txt`, `CMakePresets.json`,
    `Verification-RunbookB-v1.6.ps1`, `scripts/ci/*`, `.clang-format`, and
    `.clang-tidy` were **not modified** in this round. **Not executed.** No
    CMake/vcpkg/MSVC/CTest/clang-format/clang-tidy run from this side. No
    stage or commit. `main` untouched.

- "P0-T001 Architecture Authority Correction Directive" (round v1.6-r3
  preparation) received. **Completed v1.6-r2 verification**: with the
  formatting and Catch2-manifest corrections from the previous round
  applied, this run reported successful dependency resolution (including
  OCCT 8.0.1 successfully built and installed via vcpkg) and proceeded
  substantially further than any prior Windows execution of this task.
  Two confirmed independent defects surfaced by that run:
  - **F-01 (OCCT source defect):** `configure-build-test` failed — and
    `static-analysis` failed from the same underlying cause — because
    `src/geometry/occt/src/geometry_occt_probe.cpp` called
    `Standard_Failure::GetMessageString()`, which OCCT 8.0.1 marks
    deprecated in favor of `what()`; the authoritative Windows build
    treats warnings as errors, so the deprecation warning failed the
    build. **Correction applied:** the single line changed to
    `failure.what()`, preserving the existing null fallback and all
    surrounding behavior — no other geometry behavior changed.
  - **F-02 (effective compiler baseline enforcement gap):** the same run
    additionally revealed, via evidence this task had not previously had
    access to, that CMake — under the Ninja generator — actually selected
    `CMAKE_CXX_COMPILER_ID = Clang` / `CMAKE_CXX_COMPILER_VERSION = 19.1.5`
    / `CMAKE_CXX_SIMULATE_ID = MSVC` (a clang-cl/clang++ compiler
    simulating MSVC), despite running inside a correctly verified x64 VS
    2022 17.14 v143 developer shell with `cl.exe` resolving correctly.
    Neither `CMakePresets.json` nor `scripts/ci/configure-build-test.ps1`
    previously forced or independently verified that CMake's *effective*
    compiler was the real MSVC compiler — only that a command named
    `cl.exe` existed on PATH. **Classified as a compiler-baseline
    enforcement gap** (enforcement of the already-locked MSVC v143
    baseline, not an architecture change), not a new locked decision.
    **Correction applied:**
    - `CMakePresets.json`: `conf-common`'s `cacheVariables` now pins
      `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` to the literal command name
      `"cl"` (resolved via PATH inside the active developer shell — not a
      hard-coded absolute path), so CMake's own auto-detection can no
      longer resolve to an earlier-on-PATH clang-cl/clang++.
    - `scripts/ci/configure-build-test.ps1`: after a reported-successful
      configure and before build is allowed to proceed, the job now
      parses the actual `CMAKE_CXX_COMPILER_ID` (plus, for evidence,
      `CMAKE_CXX_COMPILER`, `CMAKE_CXX_COMPILER_VERSION`, and
      `CMAKE_CXX_SIMULATE_ID` when present). **Corrected per "P0-T001
      Architecture Authority Review" BLOCKER R3-B01:** these three
      identity fields are written by CMake into the generated file
      `CMakeFiles/<cmake-version>/CMakeCXXCompiler.cmake`, not into
      `CMakeCache.txt` (an earlier draft of this correction incorrectly
      assumed `CMakeCache.txt`, which would have made this gate a false
      negative on every configure). The job now locates that file at
      runtime (not a hard-coded `4.4.2` path segment) and requires BOTH,
      independently: `CMAKE_CXX_COMPILER_ID` exactly `MSVC`, AND the
      effective `CMAKE_CXX_COMPILER` path, normalized and compared
      case-insensitively, is the same executable as the already-validated
      `cl.exe` resolved earlier in the same script — a compiler reporting
      `CMAKE_CXX_SIMULATE_ID=MSVC` does not satisfy either check.
    - `Verification-RunbookB-v1.7.ps1` produced (v1.6 is **not** modified
      in place). Adds an independent second check of the same
      corrected evidence source inside Step 5 (which already re-verifies
      the effective triplet/generator rather than trusting
      `configure-build-test`'s exit code alone): `CMAKE_CXX_COMPILER_ID`
      must equal `MSVC` AND the effective compiler path must match the
      Step-2-validated `cl.exe` path, both whenever `configure-build-test`
      reported success, feeding the same `$verificationFailures` gate;
      the effective compiler ID/executable/version, the validated Step-2
      `cl.exe` path, and `CMAKE_CXX_SIMULATE_ID` (informational)
      are added to the Step 8 Evidence Summary. Every other already-
      accepted v1.6 gate is unchanged in substance.
  - Out of scope for this round and **not touched**: the frozen vcpkg
    baseline, vcpkg registry kind, OpenCASCADE version, Catch2
    version/baseline resolution, SQLite version, architecture layering or
    architecture tests, `.clang-tidy` policy, `.clang-format` policy, the
    warnings-as-error policy, the test-framework decision, `main`, the
    task HEAD commit, and any source file other than the single F-01
    line. The pre-existing `SQLite::SQLite3` deprecation warning noted in
    evidence was **not** fixed this round per explicit instruction —
    recorded here as the suitable place to defer it; it remains open for
    a future round.
  - **Not executed.** No CMake/vcpkg/MSVC/CTest/clang-format/clang-tidy
    run from this side. No stage or commit. `main` untouched.

## Pending entries

- Verification Runbook B execution result (date TBD, by operator).
- Phase O scope audit.
- Phase P implementation commit.
- `docs/evidence/P0-T001/CLAUDE_HANDOVER.md` / `.json`.
- Kimi independent review outcome.
- Architecture Disposition / Closure / Integration.

## 2026-09-02 - P0-T002 Architecture Gate

- Recorded final P0-T001 state as ACCEPTED + CLOSED + INTEGRATED.
- Recorded final P0-T001 integration-record commit
  `8c1c38990f75d5b0122e90d85bb8757e83a553a1`.
- Created the isolated P0-T002 branch/worktree from that accepted main
  baseline.
- Issued and approved `BIM-AG-P0-T002 v1.0` for the OCCT Geometry Spike.
- Accepted `ADR-0002 - Phase 0 Task Sequence Reconciliation`.
- Defined P0-T002 geometry-kernel scope and prohibitions.
- Defined tolerance and coordinate experiments.
- Defined Boolean success/failure corpus requirements.
- Defined OCCT history experiments.
- Defined verification and evidence requirements.
- Defined AC-001 through AC-023.
- Implementation remains NOT RELEASED pending the Architecture Authority
  Claude Implementation Brief.
- No production source, CMake, dependency or test implementation was modified
  by this architecture-record commit.

## 2026-09-02 - P0-T002 Implementation Brief Release

- Released `BIM-TASK-P0-T002-CLAUDE v1.0`.
- Locked the project-owned geometry contract for the OCCT Geometry Spike.
- Locked the primitive, opening, join and failure corpus.
- Locked the tolerance and coordinate experiment matrices.
- Locked repeatability and OCCT history evidence requirements.
- Locked evidence-executable and verification-script requirements.
- Locked the implementation footprint and ACR stop conditions.
- P0-T002 implementation is RELEASED FOR IMPLEMENTATION.
- No production implementation is part of this documentation-only release.
