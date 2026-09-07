<#
==============================================================================
 Verification-RunbookC-v1.0.ps1 - P0-T002 Phase C-M Windows Operator
 Verification Runbook (Implementation Brief section 22; Amendment 01
 AA-C10).

 OPERATOR-RUN ONLY. Claude (the Implementation Engineer) has no Windows
 command-execution channel and has never run this script - it is delivered
 as a candidate for the Windows Execution Operator to run. Every step below
 drives a real, exit-code-checked command via scripts\ci\_common.ps1's
 Invoke-Native ($LASTEXITCODE is the sole authority for success/failure,
 the same discipline scripts\ci\architecture.ps1 already uses). A step that
 cannot be checked mechanically THROWS with an explicit message rather than
 being silently marked PASS - this script never fabricates a PASS for a
 command it did not actually execute and observe the exit code of. Steps
 run strictly in order; the first failure stops the runbook, so no step
 after a failure is ever reached, let alone marked PASS.

 This runbook is a new, additive P0-T002 artifact. It does not modify,
 replace, or supersede any P0-T001 runbook (e.g. Bootstrap Runbook A) -
 those files are untouched by this candidate.

 ============================================================================
 REVISION NOTE - MINIMUM-DELTA RUNBOOK C CORRECTION (dirty-candidate
 hygiene; Architecture Authority Windows read-only inspection, RBC-01
 through RBC-08). This is a correction to this runbook only - it does not
 rewrite the geometry/API/OCCT/tests/architecture implementation, and it is
 not an ACR.
   RBC-01/RBC-02 (Step 3 / Step 21): the candidate is intentionally
     PRE-COMMIT (6 tracked MODIFY + 20 untracked ADD/EVIDENCE, 0 real
     staged paths). "Clean worktree" was the wrong invariant for that
     state. Both steps now call the single reusable Test-CandidateInvariant
     function (below) instead, which asserts the EXACT locked pre-commit
     shape rather than requiring a clean tree.
   RBC-03 (Step 21): candidate scope is no longer computed as
     `git diff --name-only <main> HEAD` (which excludes the dirty
     candidate entirely and pulls in unrelated historical/docs commits
     between main and Authorization HEAD). Candidate scope is now the
     exact tracked-MODIFY/untracked-ADD partition of the real working
     tree, verified by Test-CandidateInvariant against ExpectedTaskHead
     plus working-tree content. main identity is verified as its own,
     separate invariant (21b).
   RBC-04: -TaskBranch default corrected from 'task/P0-T002' to the exact
     branch 'task/P0-T002-occt-geometry-spike'.
   RBC-05 (Step 9): CMake check corrected from "installed >= repository's
     own cmake_minimum_required floor" to "installed == the locked P0-T002
     verification-environment reference, exactly 4.4.2". This is a
     stricter runbook-level pin, separate from (and not a replacement for)
     CMakeLists.txt's own cmake_minimum_required(VERSION 3.21) floor.
   RBC-06 (Step 10): Ninja check corrected from "any non-empty version" to
     "installed == exactly 1.12.1".
   RBC-07 (Step 7): MSVC check corrected from proving only the generic
     "MSVC / VS2022" banner signature to also proving, from actual
     compiler-banner/environment-variable/vswhere evidence (never
     invented when evidence is genuinely unavailable - that condition
     fails closed instead), the locked effective reference: cl
     19.44.35228 (x64), VCTools 14.44.35207, VS Build Tools 2022 product
     version 17.14.39. Step 8 additionally checks the VCTools version
     appears in the resolved compiler path.
   RBC-08 (Step 12 / Step 14): OCCT-resolution checking is split in two so
     it no longer depends on build output that does not exist yet before
     Step 14 has run. Step 12 (pre-build) verifies Standard_Version.hxx
     directly from an operator-supplied -OcctIncludeRoot, or from a single
     unambiguous mechanically-discovered vcpkg-installed location - never
     a hard-coded P0-T001 worktree path. Step 14 (post-build) additionally
     re-confirms OCCT 8.0.1 from the now-populated build tree's own
     generated OpenCASCADEConfigVersion.cmake package evidence. The locked
     21-step conceptual sequence is unchanged - this is folded into the
     existing Step 12 and Step 14, not a new 22nd step.
   Dirty-candidate contract: added -ExpectedCandidateTree (fails closed
     when absent/empty - deliberately NOT defaulted, since this very
     correction changes the candidate tree) and -OcctIncludeRoot
     (optional). Added the single reusable Test-CandidateInvariant
     function, and $ExpectedTrackedModifyPaths / $ExpectedUntrackedAddPaths,
     mechanically sliced from the existing $AuthorizedCandidatePaths array
     (positions 0-17 = 18 ADD, 18-23 = 6 MODIFY, 24-25 = 2 EVIDENCE;
     evidence counts as ADD for git working-tree purposes) rather than
     hand-duplicated, so the 26-path authorized list has one source of
     truth. No standalone manifest file is introduced by this correction.

 ROUND 4 REVISION NOTE - RUNBOOK C VS VERSION FIX (live Windows vswhere
 precheck; Architecture Authority; not an ACR; scope limited to this file
 plus the two handover documents - no other path changed). Live evidence
 showed `vswhere.exe -property catalog.productDisplayVersion` returns a raw
 DISPLAY string such as '17.14.39 (August 2026)', not a bare version - the
 round-3 Step 7 compared that raw string directly against
 $RequiredVSProductVersion ('17.14.39') and would therefore fail closed on
 every genuinely correct installation. Step 7 now always captures and
 prints the raw display string as evidence, then extracts a normalized
 numeric version via the anchored regex '^([0-9]+\.[0-9]+\.[0-9]+)' -
 failing closed (never silently accepting or silently rejecting) if that
 regex does not match - and compares ONLY the normalized numeric value
 against $RequiredVSProductVersion. A future release such as
 '17.14.40 (...)' or '18.x.x (...)' still fails, since its normalized
 prefix differs from '17.14.39'. Nothing else in this file changed:
 candidate-invariant logic, Step 3, Step 21, ExpectedCandidateTree
 semantics, the CMake 4.4.2 and Ninja 1.12.1 exact pins, the cl
 19.44.35228 / VCTools 14.44.35207 pins, the OCCT Step 12/14 design, test
 execution, source footprint, and Git workflow are all unchanged from
 round 3.

 ROUND 5 REVISION NOTE - CANDIDATE-INVARIANT ENUMERATION + FAILURE-
 PROPAGATION FIX (first real authoritative Windows Runbook C execution
 attempt; Architecture Authority; not an ACR; scope limited to this file
 plus the two handover documents - no other path changed).
   R5-01 (Test-CandidateInvariant, untracked enumeration): the live run
     reached Step 3 and failed there even though the external Windows
     candidate gate independently proved the real working tree was
     exactly the authorized 6 tracked-MODIFY + 20 untracked-ADD/EVIDENCE
     shape. Cause: `git status --porcelain`'s short-form output can print
     an entire new untracked directory as ONE collapsed entry (e.g.
     'docs/evidence/P0-T002/' instead of the two files inside it, or
     'src/geometry/occt/include/' instead of the one exact header leaf
     path inside it) - a directory-prefix string is never `-contains`-
     equal to any of the 26 authorized LEAF paths, so a fully authorized
     working tree could still report a false "untracked set mismatch".
     This was a Runbook enumeration defect, not a candidate-scope
     violation. Fixed by replacing the single porcelain-parsing pass with
     three separate exact-leaf-file git calls, none of which can collapse
     a path to a directory prefix: `git diff --cached --name-only` for
     real staged paths (must be 0), `git diff --name-status` for tracked
     working-tree changes (accepts only 'M'; any 'D' now throws its own
     explicit "deleted candidate path" error instead of silently passing
     as a mere difference), and `git ls-files --others --exclude-standard`
     for untracked ADD/EVIDENCE leaf files - the exact command specified
     for this fix. The downstream exact-set-equality checks against
     $ExpectedTrackedModifyPaths / $ExpectedUntrackedAddPaths, the
     total-must-equal-26 check, and the isolated-temporary-index tree
     comparison are all unchanged.
   R5-02 (top-level failure handler): the same live run's wrapper printed
     "WINDOWS AUTHORITATIVE VERIFICATION = PASS" despite this script
     printing "VERIFICATION RUNBOOK C ABORTED: ..." to the console. Cause:
     `exit 1` alone only sets $LASTEXITCODE - it raises no PowerShell
     exception the CALLER can catch. A wrapper invoking this runbook as
     `& .\Verification-RunbookC-v1.0.ps1 ...` inside its own try/catch
     (rather than explicitly checking $LASTEXITCODE after every external
     call) never sees `exit 1` as a caught error and can fall through as
     if nothing failed. Fixed: the catch block still prints every
     diagnostic and the completed-step summary exactly as before, then
     re-raises with a bare `throw` instead of `exit 1` - a real, catchable
     PowerShell exception for a try/catch-based caller, which (because an
     uncaught terminating error at a script's top level also makes
     powershell.exe/pwsh.exe itself exit non-zero) still leaves
     $LASTEXITCODE non-zero for a caller that only checks the exit code.
     Either observation method now correctly reports FAIL. The success
     path (`exit 0` after all 21 steps pass) is unchanged.
   Round 3's and round 4's fixes are otherwise untouched: the default
     task branch, the isolated-temporary-index ExpectedCandidateTree
     contract, Step 3's and Step 21's locked-pre-commit-candidate design,
     the exact 6 MODIFY + 20 ADD footprint, the CMake 4.4.2 / Ninja
     1.12.1 exact pins, the cl 19.44.35228 / VCTools 14.44.35207 pins,
     the VS-product raw-display/normalized-numeric comparison, the OCCT
     Step 12/14 design, and every CTest/job/discipline requirement are
     all byte-identical to round 4 outside the two blocks described above.

 ROUND 6 REVISION NOTE - STEP 7 CL.EXE COMMAND-DISPATCH FIX (second real
 authoritative Windows Runbook C execution attempt; Architecture Authority;
 not an ACR; scope limited to this file plus the two handover documents -
 no other path changed).
   R6-01 (Step 7, cl.exe no-argument banner probe): the live run reached
     Step 7 and failed there with "Cannot bind argument to parameter
     'CmdArgs' because it is an empty array" - a PowerShell parameter-
     binding failure raised BEFORE cl.exe ever executed. The Windows
     wrapper independently proved the active compiler itself was exactly
     the locked reference (cl 19.44.35228 x64, resolved under
     ...\MSVC\14.44.35207\bin\HostX64\x64\cl.exe). Cause: Step 7 invoked
     the shared Invoke-Native helper (scripts\ci\_common.ps1, out of this
     correction's scope) with `-CmdArgs @()` for the deliberate
     no-source-input banner probe; Invoke-Native's own -CmdArgs parameter
     rejects an empty array argument at PowerShell's own binding stage, not
     only $null - so the probe never reached the point of executing cl.exe
     at all. This was a Runbook command-dispatch defect for this one
     always-empty-argument probe, not a compiler defect and not a defect
     in any locked version pin. Fixed by capturing the banner directly
     through a single `& $env:ComSpec /d /s /c '"<cl.exe path>" 2>&1'`
     invocation instead of Invoke-Native - the Windows-proven-safe pattern
     supplied for this fix - so no empty-array parameter binding is ever
     attempted again. cl.exe's expected non-zero, no-input exit code is
     still never treated as a failure signal; only the captured banner
     text is evidence, checked exactly as before for the locked cl
     19.44.35228 / "for x64" / VCToolsVersion 14.44.35207 / VS numeric
     17.14.39 references, each still failing closed on a mismatch or
     missing evidence. Invoke-Native itself is untouched; no other step's
     use of it is affected.
   Rounds 3, 4, and 5's fixes are otherwise untouched: the default task
     branch, the isolated-temporary-index ExpectedCandidateTree contract,
     Step 3's and Step 21's locked-pre-commit-candidate design via
     Test-CandidateInvariant (including the round-5 exact-leaf-file
     enumeration via `git ls-files --others --exclude-standard` /
     `git diff --name-status` / `git diff --cached --name-only`), the
     exact 6 MODIFY + 20 ADD footprint, the CMake 4.4.2 / Ninja 1.12.1
     exact pins, the cl 19.44.35228 / VCTools 14.44.35207 pins, the
     VS-product raw-display/normalized-numeric comparison, the OCCT Step
     12/14 design, the round-5 throw-based top-level failure propagation,
     and every CTest/job/discipline requirement are all byte-identical to
     round 5 outside the Step 7 block described above.
 ============================================================================

 Preconditions (matching the precedent scripts\ci\architecture.ps1 already
 establishes for this repo's CI jobs): Steps 13-20 assume an already
 configured-and-built build directory. If -BuildDir does not yet exist or
 has not been built, run scripts\ci\configure-build-test.ps1 first (or let
 Step 14 below perform that configure/build itself - it invokes that exact
 script).

 21 steps (Implementation Brief section 22; step descriptions below reflect
 this correction round):
   1.  exact task branch
   2.  expected pre-verification task HEAD
   3.  locked pre-commit candidate (Test-CandidateInvariant; NOT a clean
       worktree - see RBC-01/RBC-02 above)
   4.  main unchanged/clean
   5.  VS2022 developer environment active
   6.  x64 target architecture
   7.  effective compiler is MSVC, at the locked reference (cl 19.44.35228
       x64 / VCTools 14.44.35207 / VS product numeric version 17.14.39,
       compared against a normalized prefix extracted from vswhere's raw
       display string, never the raw string itself) where the active
       environment exposes that evidence - see RBC-07 and the round 4
       revision note
   8.  compiler path validated (now also checks the VCTools version
       appears in the resolved path)
   9.  CMake is exactly the locked reference version 4.4.2 (RBC-05)
   10. Ninja is exactly the locked reference version 1.12.1 (RBC-06)
   11. vcpkg baseline unchanged
   12. OCCT resolves to 8.0.1, pre-build, via -OcctIncludeRoot or a single
       unambiguous discovery (RBC-08)
   13. format job (scripts\ci\format.ps1)
   14. configure/build/CTest job (scripts\ci\configure-build-test.ps1),
       plus a post-build OCCT 8.0.1 package-evidence re-confirmation
       (RBC-08)
   15. static-analysis job (scripts\ci\static-analysis.ps1)
   16. architecture job (scripts\ci\architecture.ps1 - exactly
       arch_repository_boundaries, arch_checker_detects_violation,
       arch_geometry_api_no_occt_leak, arch_geometry_occt_only_kernel_owner,
       each exact-name-anchored with --no-tests=error)
   17. license-inventory job (scripts\ci\license-inventory.ps1)
   18. geometry-spike evidence job (scripts\ci\geometry-spike.ps1)
   19. all P0-T001 regression CTest names, individually exact-name-anchored
   20. all P0-T002 CTest names, individually exact-name-anchored
   21. final repository invariants: re-run Test-CandidateInvariant (proves
       no job above mutated the candidate - NOT "worktree clean again",
       see RBC-01/RBC-02/RBC-03 above); main still unchanged; the three
       authority documents unchanged; geometry_api stays OCCT-free; Solid
       stays incomplete in every public header.

 Usage:
   powershell -File Verification-RunbookC-v1.0.ps1 `
       -ExpectedTaskHead <sha> -ExpectedMainHead <sha> `
       -ExpectedCandidateTree <tree-sha> `
       [-TaskBranch <name>] [-OcctIncludeRoot <path>] `
       [-BuildDir <path>] [-Configuration <name>]

 -ExpectedCandidateTree is REQUIRED in practice: it has no default (Step 3
 fails closed if it is not supplied) because this correction itself
 produces a new candidate tree each time this runbook's own content
 changes, so the Architecture Authority supplies the current expected
 value when the runbook is executed rather than this script assuming its
 own prior tree is still current.

 Defaults for -ExpectedTaskHead / -ExpectedMainHead are the values recorded
 in this candidate's own authorization manifest (P0-T002 authorization
 commit 1c2e2d2b8912619b5ffb82c4178f7c2d191e2b7e; main baseline
 8c1c38990f75d5b0122e90d85bb8757e83a553a1) - pass explicit values if the
 operator's actual pre-verification state differs.
==============================================================================
#>

param(
    [string]$TaskBranch = 'task/P0-T002-occt-geometry-spike',
    [string]$ExpectedTaskHead = '1c2e2d2b8912619b5ffb82c4178f7c2d191e2b7e',
    [string]$ExpectedMainHead = '8c1c38990f75d5b0122e90d85bb8757e83a553a1',
    [string]$ExpectedCandidateTree,
    [string]$OcctIncludeRoot,
    [string]$BuildDir,
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'scripts\ci\_common.ps1')

$RepoRoot = $PSScriptRoot
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}

# Locked P0-T001/P0-T002 verification-environment toolchain references
# (Implementation Brief; Architecture Authority Windows inspection RBC-05/
# RBC-06/RBC-07). These are exact runbook-level pins, not "new enough"
# floors - do not relax any of these to a >= comparison.
$RequiredCMakeVersion = '4.4.2'
$RequiredNinjaVersion = '1.12.1'
$RequiredClVersion = '19.44.35228'
$RequiredVCToolsVersion = '14.44.35207'
$RequiredVSProductVersion = '17.14.39'

# The exact 26-path authorized candidate footprint (18 ADD + 6 MODIFY + 2
# evidence paths) from TRANSPORT-RULES-v2.txt / 04-IMPLEMENTATION-
# AUTHORIZATION.md. Step 21 (and, post-correction, Step 3) use this to
# assert the working tree contains exactly this set and nothing else.
# Ordering is significant: positions 0-17 are the 18 ADD paths, 18-23 are
# the 6 MODIFY paths, 24-25 are the 2 EVIDENCE paths (evidence counts as
# ADD for git working-tree purposes) - $ExpectedTrackedModifyPaths and
# $ExpectedUntrackedAddPaths below are sliced mechanically from this single
# array rather than hand-duplicated, so there is one source of truth.
$AuthorizedCandidatePaths = @(
    'src/geometry/api/include/bim/geometry_api/geometry.hpp',
    'src/geometry/occt/src/solid_impl.hpp',
    'src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp',
    'src/geometry/occt/src/geometry_occt_adapter.cpp',
    'src/geometry/occt/src/spike_diagnostics.cpp',
    'tests/integration/geometry_occt_test_constants.hpp',
    'tests/unit/unit_geometry_api_contract.cpp',
    'tests/integration/integration_geometry_occt_primitive.cpp',
    'tests/integration/integration_geometry_occt_opening_cut.cpp',
    'tests/integration/integration_geometry_occt_join.cpp',
    'tests/integration/integration_geometry_occt_failure_corpus.cpp',
    'tests/integration/integration_geometry_occt_tolerance_matrix.cpp',
    'tests/integration/integration_geometry_occt_coordinate_matrix.cpp',
    'tests/integration/integration_geometry_occt_history.cpp',
    'tests/integration/integration_geometry_occt_repeatability.cpp',
    'tests/integration/p0_t002_geometry_evidence.cpp',
    'scripts/ci/geometry-spike.ps1',
    'Verification-RunbookC-v1.0.ps1',
    'src/geometry/occt/CMakeLists.txt',
    'tests/unit/CMakeLists.txt',
    'tests/integration/CMakeLists.txt',
    'tests/architecture/CMakeLists.txt',
    'tools/architecture_checker.cmake',
    'scripts/ci/architecture.ps1',
    'docs/evidence/P0-T002/CLAUDE_HANDOVER.md',
    'docs/evidence/P0-T002/CLAUDE_HANDOVER.json'
)

$AddPathCount = 18
$ModifyPathCount = 6
$EvidencePathCount = 2
if ($AuthorizedCandidatePaths.Count -ne ($AddPathCount + $ModifyPathCount + $EvidencePathCount)) {
    throw "Internal error: `$AuthorizedCandidatePaths has $($AuthorizedCandidatePaths.Count) entries; expected exactly $($AddPathCount + $ModifyPathCount + $EvidencePathCount) (18 ADD + 6 MODIFY + 2 EVIDENCE). This is a script defect, not a candidate defect - fix the array before running this runbook."
}
$ExpectedUntrackedAddPaths = @($AuthorizedCandidatePaths[0..($AddPathCount - 1)]) + @($AuthorizedCandidatePaths[($AddPathCount + $ModifyPathCount)..($AuthorizedCandidatePaths.Count - 1)])
$ExpectedTrackedModifyPaths = @($AuthorizedCandidatePaths[$AddPathCount..($AddPathCount + $ModifyPathCount - 1)])

# Brief sections 14/23; confirmed directly from tests/unit/CMakeLists.txt,
# tests/integration/CMakeLists.txt and tests/architecture/CMakeLists.txt as
# they existed prior to this candidate's MODIFY diffs.
$RequiredP0T001RegressionTests = @(
    'unit_foundation_smoke',
    'unit_model_links_foundation',
    'integration_geometry_occt_probe',
    'integration_persistence_sqlite_memory',
    'arch_repository_boundaries',
    'arch_checker_detects_violation'
)

# The complete P0-T002 CTest surface this candidate registers (Brief
# section 23; Amendment 01 AA-C09/AA-C10).
$RequiredP0T002Tests = @(
    'unit_geometry_api_contract',
    'integration_geometry_occt_primitive',
    'integration_geometry_occt_opening_cut',
    'integration_geometry_occt_join',
    'integration_geometry_occt_failure_corpus',
    'integration_geometry_occt_tolerance_matrix',
    'integration_geometry_occt_coordinate_matrix',
    'integration_geometry_occt_history',
    'integration_geometry_occt_repeatability',
    'integration_geometry_occt_evidence',
    'arch_geometry_api_no_occt_leak',
    'arch_geometry_occt_only_kernel_owner'
)

$stepLog = [ordered]@{}

function Invoke-RunbookStep {
    param(
        [Parameter(Mandatory)][int]$Number,
        [Parameter(Mandatory)][string]$Name,
        [Parameter(Mandatory)][scriptblock]$Action
    )
    $label = ('{0:D2} - {1}' -f $Number, $Name)
    Write-CiSection "Step $label"
    & $Action
    $stepLog[$label] = 'PASS'
    Write-Host "Step $label PASS" -ForegroundColor Green
}

function Invoke-CtestExact {
    param(
        [Parameter(Mandatory)][string]$TestName
    )
    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }
    $exactPattern = '^' + $TestName + '$'
    Push-Location $BuildDir
    try {
        Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('-R', $exactPattern, '--output-on-failure', '--no-tests=error')
    } finally {
        Pop-Location
    }
}

# Single reusable candidate-invariant function (RBC dirty-candidate
# contract). Used by both Step 3 (before any verification job runs) and
# Step 21 (after all of them have), so both call sites are guaranteed to
# apply exactly the same rule rather than two hand-maintained copies
# drifting apart. Verifies, in order:
#   1. current HEAD == ExpectedTaskHead
#   2. current branch == TaskBranch
#   3. real Git index has 0 staged paths
#   4. exactly the authorized tracked MODIFY set differs from HEAD
#   5. exactly the authorized ADD/EVIDENCE set is untracked
#   6. no deleted candidate path
#   7. no extra candidate path
#   8. exact total candidate path set = 26
#   9. construct the candidate tree using an ISOLATED temporary Git index
#      based on ExpectedTaskHead (GIT_INDEX_FILE is saved and restored;
#      the real index is never touched)
#  10. candidate tree == ExpectedCandidateTree
# Throws with a specific message on the first violation found; returns
# normally (no output value) when every invariant holds.
function Test-CandidateInvariant {
    param(
        [Parameter(Mandatory)][string]$RepoRoot,
        [Parameter(Mandatory)][string]$TaskBranch,
        [Parameter(Mandatory)][string]$ExpectedTaskHead,
        # Deliberately NOT [Parameter(Mandatory)]: the outer script's own
        # -ExpectedCandidateTree has no default, so an operator who omits
        # it leaves this as $null here. A Mandatory string parameter
        # without [AllowNull()] would reject an explicit $null argument
        # (or, worse, block on PowerShell's own interactive mandatory-
        # parameter prompt) instead of reaching the manual fail-closed
        # check below - so this follows the same convention already used
        # by the outer script's own -ExpectedTaskHead/-ExpectedMainHead
        # (Steps 2 and 4): an ordinary optional parameter, checked
        # explicitly with IsNullOrWhiteSpace as the first thing this
        # function does.
        [AllowNull()][AllowEmptyString()][string]$ExpectedCandidateTree,
        [Parameter(Mandatory)][string[]]$ExpectedTrackedModifyPaths,
        [Parameter(Mandatory)][string[]]$ExpectedUntrackedAddPaths,
        [Parameter(Mandatory)][string]$Context
    )

    if ([string]::IsNullOrWhiteSpace($ExpectedCandidateTree)) {
        throw "-ExpectedCandidateTree was not supplied and has no usable default (deliberately - this correction round itself changes the candidate tree, so no prior value can safely be assumed). This step cannot be evaluated, so it fails closed rather than being skipped. [$Context]"
    }

    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH.' }

    # 1. current HEAD == ExpectedTaskHead
    $headResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', 'HEAD')
    $currentHead = $headResult.Stdout.Trim()
    if ($currentHead -ne $ExpectedTaskHead) {
        throw "Current HEAD '$currentHead' does not match expected task HEAD '$ExpectedTaskHead'. [$Context]"
    }

    # 2. current branch == TaskBranch
    $branchResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', '--abbrev-ref', 'HEAD')
    $currentBranch = $branchResult.Stdout.Trim()
    if ($currentBranch -ne $TaskBranch) {
        throw "Current branch '$currentBranch' does not match expected task branch '$TaskBranch'. [$Context]"
    }

    # 3-8 (round 5, R5-01 fix): three separate exact leaf-file git calls,
    # never a single directory-collapsing `git status --porcelain` pass.
    # A real authoritative Windows run showed porcelain's short-form output
    # can print an entire new untracked directory as ONE collapsed entry
    # (e.g. 'docs/evidence/P0-T002/' instead of the two individual files
    # inside it, or 'src/geometry/occt/include/' instead of the one exact
    # header leaf path inside it) - that silently defeated the exact-path-
    # set comparison below (a directory-prefix string is never `-contains`-
    # equal to any of the 26 authorized LEAF paths, so a byte-identical,
    # fully authorized working tree could still report a false "set
    # mismatch"). None of the three commands below can collapse a path to
    # a directory prefix - each always enumerates individual leaf files.

    # 3. real staged paths (must be exactly 0).
    $stagedResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'diff', '--cached', '--name-only')
    $actualStagedPaths = @($stagedResult.Stdout -split "`n" | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | ForEach-Object { $_.Trim() })
    if ($actualStagedPaths.Count -gt 0) {
        throw "Real Git index has $($actualStagedPaths.Count) staged path(s), expected 0: $($actualStagedPaths -join ', '). [$Context]"
    }

    # 4/6 (tracked side): `git diff --name-status` reports both the leaf
    # path AND a status letter for every tracked file that differs from
    # the index (equivalent to `git diff --name-only`, but additionally
    # lets a deletion be distinguished from an ordinary modification -
    # required so "no deleted candidate path" is its own explicit,
    # checked condition rather than a deleted MODIFY-set path silently
    # passing as merely "differing"). Only a plain 'M' is accepted; any
    # other status (A/D/R###/C###/T/U) fails closed.
    $trackedDiffResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'diff', '--name-status')
    $trackedDiffLines = @($trackedDiffResult.Stdout -split "`n" | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    $actualModifyPaths = [System.Collections.Generic.List[string]]::new()
    $deletedTrackedPaths = [System.Collections.Generic.List[string]]::new()
    foreach ($line in $trackedDiffLines) {
        $fields = $line -split "`t"
        if ($fields.Count -lt 2) {
            throw "Unparseable 'git diff --name-status' line '$line'. [$Context]"
        }
        $status = $fields[0]
        $trackedPath = $fields[1].Trim()
        if ($status -eq 'M') {
            $actualModifyPaths.Add($trackedPath)
        } elseif ($status -eq 'D') {
            $deletedTrackedPaths.Add($trackedPath)
        } else {
            throw "Unexpected tracked change type '$status' for '$trackedPath' from 'git diff --name-status'. Only plain modifications ('M') are permitted by the locked pre-commit candidate contract; anything else (additions, deletions, renames, copies, type changes) means the real Git-tracked tree has drifted from the authorized dirty-candidate shape. [$Context]"
        }
    }
    if ($deletedTrackedPaths.Count -gt 0) {
        throw "Deleted candidate path(s) detected among tracked files: $($deletedTrackedPaths -join ', '). No candidate path may be deleted. [$Context]"
    }

    # 5 (untracked side, R5-01's actual fix): `git ls-files --others
    # --exclude-standard` enumerates untracked leaf files one per line -
    # it has no directory-collapsed short form, unlike `git status
    # --porcelain`. This is the exact command the correction specified.
    $untrackedResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'ls-files', '--others', '--exclude-standard')
    $actualUntrackedAddPaths = @($untrackedResult.Stdout -split "`n" | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | ForEach-Object { $_.Trim() })

    $missingModify = $ExpectedTrackedModifyPaths | Where-Object { $actualModifyPaths -notcontains $_ }
    $extraModify = $actualModifyPaths | Where-Object { $ExpectedTrackedModifyPaths -notcontains $_ }
    if ($missingModify.Count -gt 0 -or $extraModify.Count -gt 0) {
        throw "Tracked MODIFY set mismatch. Missing (expected but not modified): [$($missingModify -join ', ')]. Unauthorized/extra (modified but not authorized): [$($extraModify -join ', ')]. [$Context]"
    }

    $missingAdd = $ExpectedUntrackedAddPaths | Where-Object { $actualUntrackedAddPaths -notcontains $_ }
    $extraAdd = $actualUntrackedAddPaths | Where-Object { $ExpectedUntrackedAddPaths -notcontains $_ }
    if ($missingAdd.Count -gt 0 -or $extraAdd.Count -gt 0) {
        throw "Untracked ADD/EVIDENCE set mismatch. Missing (expected but not present untracked): [$($missingAdd -join ', ')]. Unauthorized/extra (untracked but not authorized): [$($extraAdd -join ', ')]. [$Context]"
    }

    $totalCandidatePaths = $ExpectedTrackedModifyPaths.Count + $ExpectedUntrackedAddPaths.Count
    if ($totalCandidatePaths -ne 26) {
        throw "Internal error: the supplied expected path sets total $totalCandidatePaths, not 26 ($($ExpectedTrackedModifyPaths.Count) MODIFY + $($ExpectedUntrackedAddPaths.Count) ADD/EVIDENCE). This is a script defect, not a candidate defect. [$Context]"
    }

    # 9-10: construct the candidate tree in an ISOLATED temporary Git
    # index seeded from ExpectedTaskHead, then compare it to
    # ExpectedCandidateTree. The real index (and GIT_INDEX_FILE) is saved
    # before this block and restored in finally, whatever happens.
    $previousIndexFile = $env:GIT_INDEX_FILE
    $previousIndexFileWasSet = Test-Path Env:\GIT_INDEX_FILE
    $tempIndexFile = Join-Path ([System.IO.Path]::GetTempPath()) ('runbookc-candidate-index-' + [System.Guid]::NewGuid().ToString('N') + '.tmp')
    $observedTree = $null
    try {
        $env:GIT_INDEX_FILE = $tempIndexFile
        Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'read-tree', $ExpectedTaskHead)
        $allCandidatePaths = @($ExpectedTrackedModifyPaths) + @($ExpectedUntrackedAddPaths)
        Invoke-Native -Exe $gitCmd.Source -CmdArgs (@('-C', $RepoRoot, 'add', '--') + $allCandidatePaths)
        $writeTreeResult = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'write-tree')
        $observedTree = $writeTreeResult.Stdout.Trim()
    } finally {
        if ($previousIndexFileWasSet) {
            $env:GIT_INDEX_FILE = $previousIndexFile
        } else {
            Remove-Item Env:\GIT_INDEX_FILE -ErrorAction SilentlyContinue
        }
        if (Test-Path $tempIndexFile) {
            Remove-Item -Path $tempIndexFile -Force -ErrorAction SilentlyContinue
        }
    }

    if ([string]::IsNullOrWhiteSpace($observedTree)) {
        throw "'git write-tree' against the isolated temporary index produced no tree SHA. [$Context]"
    }
    if ($observedTree -ne $ExpectedCandidateTree) {
        throw "Isolated-index candidate tree '$observedTree' (built from ExpectedTaskHead '$ExpectedTaskHead' plus the working-tree content of the 26 authorized candidate paths) does not match -ExpectedCandidateTree '$ExpectedCandidateTree'. [$Context]"
    }
}

try {
    # --- Step 1: exact task branch -----------------------------------------
    Invoke-RunbookStep -Number 1 -Name 'exact task branch' -Action {
        $gitCmd = Get-Command git -ErrorAction SilentlyContinue
        if (-not $gitCmd) { throw 'git was not found on PATH.' }
        $result = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', '--abbrev-ref', 'HEAD')
        $currentBranch = $result.Stdout.Trim()
        if ($currentBranch -ne $TaskBranch) {
            throw "Current branch '$currentBranch' does not match expected task branch '$TaskBranch'. Pass -TaskBranch to override if this is intentional."
        }
        Write-Host "Confirmed current branch is '$currentBranch'."
    }

    # --- Step 2: expected pre-verification task HEAD ------------------------
    Invoke-RunbookStep -Number 2 -Name 'expected pre-verification task HEAD' -Action {
        if ([string]::IsNullOrWhiteSpace($ExpectedTaskHead)) {
            throw '-ExpectedTaskHead was not supplied and has no usable default. This step cannot be evaluated, so it fails closed rather than being skipped.'
        }
        $gitCmd = Get-Command git -ErrorAction SilentlyContinue
        if (-not $gitCmd) { throw 'git was not found on PATH.' }
        $result = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', 'HEAD')
        $currentHead = $result.Stdout.Trim()
        if ($currentHead -ne $ExpectedTaskHead) {
            throw "Current task HEAD '$currentHead' does not match expected pre-verification HEAD '$ExpectedTaskHead'."
        }
        Write-Host "Confirmed task HEAD is '$currentHead'."
    }

    # --- Step 3: locked pre-commit candidate (RBC-01/RBC-02) -----------------
    Invoke-RunbookStep -Number 3 -Name 'locked pre-commit candidate' -Action {
        Test-CandidateInvariant -RepoRoot $RepoRoot -TaskBranch $TaskBranch -ExpectedTaskHead $ExpectedTaskHead `
            -ExpectedCandidateTree $ExpectedCandidateTree -ExpectedTrackedModifyPaths $ExpectedTrackedModifyPaths `
            -ExpectedUntrackedAddPaths $ExpectedUntrackedAddPaths -Context 'Step 3 (pre-verification candidate shape)'
        Write-Host "Confirmed locked pre-commit candidate: $($ExpectedTrackedModifyPaths.Count) tracked MODIFY, $($ExpectedUntrackedAddPaths.Count) untracked ADD/EVIDENCE, 0 real staged paths, isolated-index candidate tree matches -ExpectedCandidateTree. This step deliberately does NOT require a clean worktree."
    }

    # --- Step 4: main unchanged/clean -----------------------------------------
    Invoke-RunbookStep -Number 4 -Name 'main unchanged/clean' -Action {
        if ([string]::IsNullOrWhiteSpace($ExpectedMainHead)) {
            throw '-ExpectedMainHead was not supplied and has no usable default. This step cannot be evaluated, so it fails closed rather than being skipped.'
        }
        $gitCmd = Get-Command git -ErrorAction SilentlyContinue
        if (-not $gitCmd) { throw 'git was not found on PATH.' }
        $refCandidates = @('refs/heads/main', 'refs/remotes/origin/main')
        $resolved = $null
        foreach ($ref in $refCandidates) {
            $probe = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', '--verify', '--quiet', $ref) -AllowFailure
            if ($probe.ExitCode -eq 0 -and -not [string]::IsNullOrWhiteSpace($probe.Stdout)) {
                $resolved = $probe.Stdout.Trim()
                break
            }
        }
        if (-not $resolved) {
            throw "Could not resolve either 'refs/heads/main' or 'refs/remotes/origin/main' in this repository."
        }
        if ($resolved -ne $ExpectedMainHead) {
            throw "main resolves to '$resolved', which does not match expected main baseline '$ExpectedMainHead'. main must remain unmodified by this candidate."
        }
        Write-Host "Confirmed main resolves to unchanged baseline '$resolved'."
    }

    # --- Step 5: VS2022 developer environment active ---------------------------
    Invoke-RunbookStep -Number 5 -Name 'VS2022 developer environment active' -Action {
        $vscmdVer = $env:VSCMD_VER
        if ([string]::IsNullOrWhiteSpace($vscmdVer)) {
            throw 'VSCMD_VER is not set. This script must be run from a Visual Studio 2022 Developer PowerShell / Developer Command Prompt session (run vcvars64.bat or the VS 2022 "Developer PowerShell" shortcut first).'
        }
        if ($vscmdVer -notmatch '^17\.') {
            throw "VSCMD_VER='$vscmdVer' does not correspond to Visual Studio 2022 (toolset major version 17). A different VS version's developer shell appears to be active."
        }
        Write-Host "Confirmed VS2022 developer environment active (VSCMD_VER=$vscmdVer)."
    }

    # --- Step 6: x64 target architecture -----------------------------------------
    Invoke-RunbookStep -Number 6 -Name 'x64 target architecture' -Action {
        $targetArch = $env:VSCMD_ARG_TGT_ARCH
        if ([string]::IsNullOrWhiteSpace($targetArch)) {
            throw 'VSCMD_ARG_TGT_ARCH is not set. Launch the x64 Native Tools / x64 Developer PowerShell for VS 2022 variant, not the x86 or ARM64 variant.'
        }
        if ($targetArch -ne 'x64') {
            throw "VSCMD_ARG_TGT_ARCH='$targetArch', expected 'x64'."
        }
        Write-Host 'Confirmed x64 target architecture is active.'
    }

    # --- Step 7: effective compiler is MSVC, at the locked reference (RBC-07) ----
    Invoke-RunbookStep -Number 7 -Name 'effective compiler is MSVC (locked reference)' -Action {
        $clCmd = Get-Command cl -ErrorAction SilentlyContinue
        if (-not $clCmd) { throw 'cl.exe was not found on PATH.' }
        # v1.4 fix (round 6, R6-01): cl.exe with no arguments prints its
        # version banner to stderr and exits non-zero (no input files) -
        # that non-zero exit is expected and is itself the evidence, never
        # a failure signal. Routing this deliberate no-argument probe
        # through Invoke-Native -CmdArgs @() fails PowerShell parameter
        # binding itself ("Cannot bind argument to parameter 'CmdArgs'
        # because it is an empty array") before cl.exe ever executes -
        # Invoke-Native's own -CmdArgs parameter (scripts/ci/_common.ps1,
        # out of this correction's scope) rejects an empty array argument,
        # not only $null. This was a command-dispatch defect in how Step 7
        # invoked the shared helper for this specific always-empty-argument
        # probe, not a defect in cl.exe, the compiler banner content, or
        # any locked version pin below. Fix: capture the banner directly
        # through a single cmd.exe /d /s /c invocation (the
        # Windows-proven-safe pattern below), never through Invoke-Native,
        # so no empty-array parameter binding is ever attempted again.
        # Invoke-Native itself is untouched, and no other step's use of it
        # is affected - this is a minimum-delta, Step-7-only fix.
        $clCommand = '"' + $clCmd.Source + '" 2>&1'
        $clBanner = (
            & $env:ComSpec `
                /d `
                /s `
                /c `
                $clCommand
        ) | Out-String
        $banner = $clBanner
        if ($banner -notmatch 'Microsoft \(R\) C/C\+\+ Optimizing Compiler') {
            throw "cl.exe banner did not match the expected MSVC signature. Banner was:`n$banner"
        }

        $clVersionMatch = [regex]::Match($banner, 'Compiler Version\s+([0-9]+\.[0-9]+\.[0-9]+)')
        if (-not $clVersionMatch.Success) {
            throw "Could not parse a cl.exe version number from the compiler banner - this check fails closed rather than accepting an unparsed banner as a PASS. Banner was:`n$banner"
        }
        $observedClVersion = $clVersionMatch.Groups[1].Value
        if ($observedClVersion -ne $RequiredClVersion) {
            throw "cl.exe reports version '$observedClVersion' from its banner, which does not match the locked P0-T001/P0-T002 reference '$RequiredClVersion'."
        }
        if ($banner -notmatch 'for x64') {
            throw "cl.exe banner does not report an x64 target ('for x64' not found in the banner text). Banner was:`n$banner"
        }

        # VCToolsVersion is set automatically by vcvars64.bat / the VS 2022
        # Developer PowerShell shortcut. If it is genuinely unset, that
        # evidence is not invented - this fails closed instead of skipping.
        $vcToolsVersion = $env:VCToolsVersion
        if ([string]::IsNullOrWhiteSpace($vcToolsVersion)) {
            throw "VCToolsVersion environment variable is not set, so the locked VCTools reference '$RequiredVCToolsVersion' cannot be mechanically verified. This check fails closed rather than skipping or inventing a value; re-run from a proper VS 2022 Developer PowerShell session."
        }
        if ($vcToolsVersion -ne $RequiredVCToolsVersion) {
            throw "VCToolsVersion='$vcToolsVersion' does not match the locked P0-T001/P0-T002 reference '$RequiredVCToolsVersion'."
        }

        # VS Build Tools product version, via vswhere.exe (the standard
        # mechanical way to query installed Visual Studio product
        # versions; ships with every VS2022/Build Tools install under the
        # Visual Studio Installer directory). If vswhere cannot be found
        # or produces no evidence, this fails closed rather than skipping.
        $vswhereCmd = Get-Command vswhere -ErrorAction SilentlyContinue
        $vswherePath = $null
        if ($vswhereCmd) {
            $vswherePath = $vswhereCmd.Source
        } else {
            $programFilesX86 = ${env:ProgramFiles(x86)}
            if (-not [string]::IsNullOrWhiteSpace($programFilesX86)) {
                $defaultVsWherePath = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
                if (Test-Path $defaultVsWherePath) { $vswherePath = $defaultVsWherePath }
            }
        }
        if (-not $vswherePath) {
            throw "Could not locate vswhere.exe (neither on PATH nor at its default Visual Studio Installer location) to verify the locked VS Build Tools product version '$RequiredVSProductVersion'. This check fails closed rather than skipping."
        }
        $vswhereResult = Invoke-Native -Exe $vswherePath -CmdArgs @('-latest', '-products', '*', '-property', 'catalog.productDisplayVersion')
        $vsProductRawDisplay = $vswhereResult.Stdout.Trim()
        if ([string]::IsNullOrWhiteSpace($vsProductRawDisplay)) {
            throw 'vswhere.exe produced no catalog.productDisplayVersion output.'
        }

        # v1.2 fix (round 4, live Windows vswhere evidence): a real
        # catalog.productDisplayVersion value is a raw DISPLAY string, not a
        # bare version - e.g. '17.14.39 (August 2026)' - so it must never be
        # compared directly, character-for-character, against the locked
        # numeric reference '17.14.39'. The raw display string is always
        # captured and printed as evidence; the authoritative comparison
        # uses only an anchored numeric prefix extracted from it, and
        # extraction failure fails closed rather than silently accepting
        # (or silently rejecting) the raw string.
        $vsProductMatch = [regex]::Match($vsProductRawDisplay, '^([0-9]+\.[0-9]+\.[0-9]+)')
        if (-not $vsProductMatch.Success) {
            throw "Could not extract a leading numeric product version (pattern '^([0-9]+\.[0-9]+\.[0-9]+)') from vswhere's raw catalog.productDisplayVersion output '$vsProductRawDisplay'. This check fails closed rather than accepting an unparsed display string as a PASS."
        }
        $installedVsNumericVersion = $vsProductMatch.Groups[1].Value
        if ($installedVsNumericVersion -ne $RequiredVSProductVersion) {
            throw "vswhere reports installed VS product numeric version '$installedVsNumericVersion' (raw display '$vsProductRawDisplay'), which does not match the locked P0-T001/P0-T002 reference '$RequiredVSProductVersion'."
        }

        Write-Host "Confirmed effective compiler is MSVC at the locked reference: cl $observedClVersion (x64), VCTools $vcToolsVersion, VS product $installedVsNumericVersion (raw display: '$vsProductRawDisplay')."
    }

    # --- Step 8: compiler path validated -------------------------------------------
    Invoke-RunbookStep -Number 8 -Name 'compiler path validated' -Action {
        $clCmd = Get-Command cl -ErrorAction SilentlyContinue
        if (-not $clCmd) { throw 'cl.exe was not found on PATH.' }
        $clPath = $clCmd.Source
        if (-not (Test-Path $clPath)) {
            throw "Resolved cl.exe path '$clPath' does not exist on disk."
        }
        if ($clPath -notmatch '\\2022\\') {
            throw "Resolved cl.exe path '$clPath' does not appear to be under a Visual Studio 2022 install (expected a '\2022\' path segment)."
        }
        $vcToolsDir = $env:VCToolsInstallDir
        if (-not [string]::IsNullOrWhiteSpace($vcToolsDir) -and -not $clPath.StartsWith($vcToolsDir, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Resolved cl.exe path '$clPath' is not under VCToolsInstallDir '$vcToolsDir' - the active developer shell and the resolved compiler disagree on toolset location."
        }
        # RBC-07 path-based version evidence: the locked VCTools reference
        # should also appear as a literal path segment (standard MSVC
        # toolset layout is ...\VC\Tools\MSVC\<version>\bin\Hostx64\x64\).
        if ($clPath -notlike "*$RequiredVCToolsVersion*") {
            throw "Resolved cl.exe path '$clPath' does not contain the locked VCTools version '$RequiredVCToolsVersion' as a path segment."
        }
        Write-Host "Confirmed compiler path: $clPath"
    }

    # --- Step 9: CMake is exactly the locked reference version (RBC-05) ------------
    Invoke-RunbookStep -Number 9 -Name 'CMake locked reference version' -Action {
        $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
        if (-not $cmakeCmd) { throw 'cmake was not found on PATH.' }
        $result = Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--version')
        $versionMatch = [regex]::Match($result.Stdout, 'cmake version ([0-9]+(?:\.[0-9]+){1,2})')
        if (-not $versionMatch.Success) {
            throw "Could not parse installed CMake version from output:`n$($result.Stdout)"
        }
        $installedVersion = $versionMatch.Groups[1].Value
        if ($installedVersion -ne $RequiredCMakeVersion) {
            throw "Installed CMake is '$installedVersion'; the P0-T002 locked verification-environment reference is exactly '$RequiredCMakeVersion'. This is a stricter runbook-level pin than this repository's own cmake_minimum_required(VERSION 3.21) floor in CMakeLists.txt (which remains a separate, lower bound the source itself declares) - do not relax this check to '>= $RequiredCMakeVersion'."
        }
        Write-Host "Confirmed CMake is exactly the locked reference version $installedVersion."
    }

    # --- Step 10: Ninja is exactly the locked reference version (RBC-06) -----------
    Invoke-RunbookStep -Number 10 -Name 'Ninja locked reference version' -Action {
        $ninjaCmd = Get-Command ninja -ErrorAction SilentlyContinue
        if (-not $ninjaCmd) { throw 'ninja was not found on PATH.' }
        $result = Invoke-Native -Exe $ninjaCmd.Source -CmdArgs @('--version')
        $ninjaVersion = $result.Stdout.Trim()
        if ($ninjaVersion -ne $RequiredNinjaVersion) {
            throw "Installed ninja is '$ninjaVersion'; the P0-T002 locked verification-environment reference is exactly '$RequiredNinjaVersion'. Do not relax this check to 'any non-empty version'."
        }
        Write-Host "Confirmed ninja is exactly the locked reference version $ninjaVersion."
    }

    # --- Step 11: vcpkg baseline unchanged ------------------------------------------
    Invoke-RunbookStep -Number 11 -Name 'vcpkg baseline unchanged' -Action {
        $gitCmd = Get-Command git -ErrorAction SilentlyContinue
        if (-not $gitCmd) { throw 'git was not found on PATH.' }
        $manifestCandidates = @('vcpkg.json', 'vcpkg-configuration.json') |
            Where-Object { Test-Path (Join-Path $RepoRoot $_) }
        if ($manifestCandidates.Count -eq 0) {
            throw 'Neither vcpkg.json nor vcpkg-configuration.json was found in the repository root.'
        }
        foreach ($manifestFile in $manifestCandidates) {
            $diffArgs = @('-C', $RepoRoot, 'diff', '--quiet', $ExpectedMainHead, '--', $manifestFile)
            $result = Invoke-Native -Exe $gitCmd.Source -CmdArgs $diffArgs -AllowFailure
            if ($result.ExitCode -ne 0) {
                throw "'$manifestFile' differs from the main baseline ($ExpectedMainHead). This candidate's authorized write scope does not include vcpkg manifest changes."
            }
        }
        Write-Host "Confirmed unchanged from main baseline: $($manifestCandidates -join ', ')"
    }

    # --- Step 12: OCCT resolves to 8.0.1, pre-build (RBC-08) ------------------------
    Invoke-RunbookStep -Number 12 -Name 'OCCT resolves to 8.0.1 (pre-build)' -Action {
        # v1.1 fix (RBC-08): the v1.0 design depended on finding
        # OpenCASCADEConfigVersion.cmake recursively under BuildDir/RepoRoot,
        # which does not exist yet on a fresh P0-T002 build tree before
        # Step 14 has configured/built it - a real ordering defect, not a
        # source defect. This step now verifies Standard_Version.hxx
        # directly, either from an explicit -OcctIncludeRoot or from a
        # single unambiguous mechanically-discovered vcpkg-installed
        # location - never a hard-coded P0-T001 worktree path. Step 14
        # below adds a SECOND, post-build confirmation from the now-
        # populated build tree's own generated package evidence, so this
        # pre-build step is deliberately not the only OCCT-version gate in
        # this runbook.
        $versionFile = $null

        if (-not [string]::IsNullOrWhiteSpace($OcctIncludeRoot)) {
            if (-not (Test-Path $OcctIncludeRoot)) {
                throw "-OcctIncludeRoot '$OcctIncludeRoot' does not exist."
            }
            $found = Get-ChildItem -Path $OcctIncludeRoot -Recurse -Filter 'Standard_Version.hxx' -ErrorAction SilentlyContinue | Select-Object -First 1
            if (-not $found) {
                throw "Could not find Standard_Version.hxx under -OcctIncludeRoot '$OcctIncludeRoot'."
            }
            $versionFile = $found
        } else {
            # Unambiguous mechanical discovery only: this repo's own
            # vcpkg-installed tree (manifest mode installs to
            # <RepoRoot>\vcpkg_installed\<triplet>\include\opencascade\
            # once a configure has run at least once) and VCPKG_ROOT's
            # installed tree if that environment variable is set. Zero or
            # more than one candidate fails closed rather than guessing -
            # pass -OcctIncludeRoot explicitly instead.
            $discoveryRoots = @()
            $vcpkgInstalledDir = Join-Path $RepoRoot 'vcpkg_installed'
            if (Test-Path $vcpkgInstalledDir) { $discoveryRoots += $vcpkgInstalledDir }
            if (-not [string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
                $vcpkgRootInstalledDir = Join-Path $env:VCPKG_ROOT 'installed'
                if (Test-Path $vcpkgRootInstalledDir) { $discoveryRoots += $vcpkgRootInstalledDir }
            }
            $candidates = @()
            foreach ($root in $discoveryRoots) {
                $candidates += @(Get-ChildItem -Path $root -Recurse -Filter 'Standard_Version.hxx' -ErrorAction SilentlyContinue)
            }
            if ($candidates.Count -eq 0) {
                $rootsDescription = if ($discoveryRoots.Count -gt 0) { $discoveryRoots -join "' or '" } else { '<none existed to search>' }
                throw "Could not discover an installed OCCT include tree (no Standard_Version.hxx found under '$rootsDescription'). Pass -OcctIncludeRoot <path> explicitly, or run scripts\ci\configure-build-test.ps1 (or Step 14 below) first so the post-build check in Step 14 can confirm OCCT instead."
            }
            if ($candidates.Count -gt 1) {
                throw "OCCT include tree discovery is ambiguous: found $($candidates.Count) Standard_Version.hxx candidates. Pass -OcctIncludeRoot <path> explicitly to disambiguate: $(($candidates | ForEach-Object { $_.FullName }) -join ', ')"
            }
            $versionFile = $candidates[0]
        }

        $versionText = Get-Content -Raw -Path $versionFile.FullName
        $majorMatch = [regex]::Match($versionText, '#define\s+OCC_VERSION_MAJOR\s+([0-9]+)')
        $minorMatch = [regex]::Match($versionText, '#define\s+OCC_VERSION_MINOR\s+([0-9]+)')
        $maintMatch = [regex]::Match($versionText, '#define\s+OCC_VERSION_MAINTENANCE\s+([0-9]+)')
        if (-not ($majorMatch.Success -and $minorMatch.Success -and $maintMatch.Success)) {
            throw "Could not parse OCC_VERSION_MAJOR/MINOR/MAINTENANCE from '$($versionFile.FullName)'."
        }
        $resolvedVersion = '{0}.{1}.{2}' -f $majorMatch.Groups[1].Value, $minorMatch.Groups[1].Value, $maintMatch.Groups[1].Value
        if ($resolvedVersion -ne '8.0.1') {
            throw "Resolved OCCT version is '$resolvedVersion' (from '$($versionFile.FullName)'), expected '8.0.1'."
        }
        Write-Host "Confirmed OCCT pre-build include-tree version is 8.0.1 ($($versionFile.FullName))."
    }

    # --- Step 13: format job --------------------------------------------------------
    Invoke-RunbookStep -Number 13 -Name 'format job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\format.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath
        if ($LASTEXITCODE -ne 0) { throw "format.ps1 exited $LASTEXITCODE." }
    }

    # --- Step 14: configure/build/CTest job, plus post-build OCCT re-confirmation (RBC-08) --
    Invoke-RunbookStep -Number 14 -Name 'configure/build/CTest job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\configure-build-test.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath
        if ($LASTEXITCODE -ne 0) { throw "configure-build-test.ps1 exited $LASTEXITCODE." }

        # RBC-08 post-build confirmation: now that the build tree is
        # populated, mechanically re-confirm it actually resolved OCCT
        # 8.0.1 from its own generated/vcpkg package evidence (the
        # original v1.0 Step 12 logic, now safely placed after the tree it
        # searches actually exists).
        $configVersionFile = Get-ChildItem -Path $BuildDir -Recurse -Filter 'OpenCASCADEConfigVersion.cmake' -ErrorAction SilentlyContinue | Select-Object -First 1
        if (-not $configVersionFile) {
            throw "Could not find OpenCASCADEConfigVersion.cmake under build directory '$BuildDir' after configure/build completed. The build does not appear to have resolved an OCCT package."
        }
        $configVersionText = Get-Content -Raw -Path $configVersionFile.FullName
        $configVersionMatch = [regex]::Match($configVersionText, 'PACKAGE_VERSION\s+"?([0-9]+\.[0-9]+\.[0-9]+)"?')
        if (-not $configVersionMatch.Success) {
            throw "Could not parse PACKAGE_VERSION from '$($configVersionFile.FullName)'."
        }
        $configResolvedVersion = $configVersionMatch.Groups[1].Value
        if ($configResolvedVersion -ne '8.0.1') {
            throw "Post-build OCCT package evidence resolves to '$configResolvedVersion' ($($configVersionFile.FullName)), expected '8.0.1'."
        }
        Write-Host "Confirmed post-build OCCT package evidence resolves to 8.0.1 ($($configVersionFile.FullName))."
    }

    # --- Step 15: static-analysis job -------------------------------------------------
    Invoke-RunbookStep -Number 15 -Name 'static-analysis job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\static-analysis.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath
        if ($LASTEXITCODE -ne 0) { throw "static-analysis.ps1 exited $LASTEXITCODE." }
    }

    # --- Step 16: architecture job ----------------------------------------------------
    Invoke-RunbookStep -Number 16 -Name 'architecture job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\architecture.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath
        if ($LASTEXITCODE -ne 0) { throw "architecture.ps1 exited $LASTEXITCODE." }
        Write-Host 'architecture.ps1 itself exact-name-anchors arch_repository_boundaries, arch_checker_detects_violation, arch_geometry_api_no_occt_leak and arch_geometry_occt_only_kernel_owner with --no-tests=error (Amendment 01 AA-C10); its exit 0 above is this step''s evidence.'
    }

    # --- Step 17: license-inventory job ------------------------------------------------
    Invoke-RunbookStep -Number 17 -Name 'license-inventory job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\license-inventory.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath
        if ($LASTEXITCODE -ne 0) { throw "license-inventory.ps1 exited $LASTEXITCODE." }
    }

    # --- Step 18: geometry-spike evidence job --------------------------------------------
    Invoke-RunbookStep -Number 18 -Name 'geometry-spike evidence job' -Action {
        $scriptPath = Join-Path $RepoRoot 'scripts\ci\geometry-spike.ps1'
        if (-not (Test-Path $scriptPath)) { throw "'$scriptPath' does not exist." }
        & $scriptPath -BuildDir $BuildDir -Configuration $Configuration
        if ($LASTEXITCODE -ne 0) { throw "geometry-spike.ps1 exited $LASTEXITCODE." }
    }

    # --- Step 19: all P0-T001 regression CTest names ------------------------------------
    Invoke-RunbookStep -Number 19 -Name 'all P0-T001 regression tests' -Action {
        foreach ($testName in $RequiredP0T001RegressionTests) {
            Write-Host "  ctest -R ""^$testName`$"" --output-on-failure --no-tests=error"
            Invoke-CtestExact -TestName $testName
        }
        Write-Host "Confirmed all $($RequiredP0T001RegressionTests.Count) P0-T001 regression test(s) registered and passing."
    }

    # --- Step 20: all P0-T002 tests ------------------------------------------------------
    Invoke-RunbookStep -Number 20 -Name 'all P0-T002 tests' -Action {
        foreach ($testName in $RequiredP0T002Tests) {
            Write-Host "  ctest -R ""^$testName`$"" --output-on-failure --no-tests=error"
            Invoke-CtestExact -TestName $testName
        }
        Write-Host "Confirmed all $($RequiredP0T002Tests.Count) P0-T002 test(s) registered and passing."
    }

    # --- Step 21: final repository invariants (RBC-01/RBC-02/RBC-03) ---------------------
    Invoke-RunbookStep -Number 21 -Name 'final repository invariants' -Action {
        $gitCmd = Get-Command git -ErrorAction SilentlyContinue
        if (-not $gitCmd) { throw 'git was not found on PATH.' }

        # 21a: re-run the same candidate-invariant function Step 3 used.
        # This proves HEAD, branch, the real Git index, the exact 26-path
        # candidate shape, and the isolated-index candidate tree are all
        # still exactly what they were before any verification job ran -
        # i.e. no job (format/build/static-analysis/architecture/license/
        # evidence) mutated the candidate source. This deliberately does
        # NOT require a clean worktree (RBC-01/RBC-02): the locked
        # pre-commit candidate shape is the correct invariant here.
        Test-CandidateInvariant -RepoRoot $RepoRoot -TaskBranch $TaskBranch -ExpectedTaskHead $ExpectedTaskHead `
            -ExpectedCandidateTree $ExpectedCandidateTree -ExpectedTrackedModifyPaths $ExpectedTrackedModifyPaths `
            -ExpectedUntrackedAddPaths $ExpectedUntrackedAddPaths -Context 'Step 21 (post-verification candidate shape)'

        # 21b: main unchanged/clean, re-checked post-jobs. RBC-03: main
        # identity is verified purely as its own invariant here - it is
        # never used to compute the candidate path set (that is 21a's
        # job, via Test-CandidateInvariant, not `git diff <main> HEAD`).
        $refCandidates = @('refs/heads/main', 'refs/remotes/origin/main')
        $resolvedMain = $null
        foreach ($ref in $refCandidates) {
            $probe = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'rev-parse', '--verify', '--quiet', $ref) -AllowFailure
            if ($probe.ExitCode -eq 0 -and -not [string]::IsNullOrWhiteSpace($probe.Stdout)) {
                $resolvedMain = $probe.Stdout.Trim()
                break
            }
        }
        if (-not $resolvedMain) {
            throw "Could not resolve either 'refs/heads/main' or 'refs/remotes/origin/main' in this repository."
        }
        if ($resolvedMain -ne $ExpectedMainHead) {
            throw "main resolves to '$resolvedMain' after running the CI-equivalent jobs, which no longer matches expected main baseline '$ExpectedMainHead'. main must remain unmodified."
        }

        # 21c: authority boundary documents are byte-for-byte unchanged
        # from ExpectedTaskHead, the immutable P0-T002 authorization state.
        # These documents are intentionally absent from the older main
        # baseline and are never part of the candidate's authorized write scope.
        $authorityDocs = @(
            'docs/gates/P0-T002_Architecture_Gate_Package_v1.0.md',
            'docs/tasks/P0-T002/02-IMPLEMENTATION-BRIEF.md',
            'docs/tasks/P0-T002/03-IMPLEMENTATION-BRIEF-AMENDMENT-01.md'
        )
        foreach ($docPath in $authorityDocs) {
            $docDiff = Invoke-Native -Exe $gitCmd.Source -CmdArgs @('-C', $RepoRoot, 'diff', '--quiet', $ExpectedTaskHead, '--', $docPath) -AllowFailure
            if ($docDiff.ExitCode -ne 0) {
                throw "Authority document '$docPath' differs from expected task authorization HEAD ($ExpectedTaskHead). Authority documents are never in the candidate's authorized write scope."
            }
        }

        # 21d: geometry_api stays OCCT-free. Re-run the authoritative
        # architecture test already used in Step 16 instead of duplicating
        # its semantics with a raw lexical scan that also matches comments.
        & ctest --test-dir $BuildDir -R '^arch_geometry_api_no_occt_leak$' --output-on-failure --no-tests=error
        if ($LASTEXITCODE -ne 0) {
            throw "Final geometry_api OCCT-leak architecture invariant failed."
        }

        # 21e: Solid stays incomplete in every public geometry_api header
        # (Brief section 6.7; Amendment 01 AA-C05) - its complete
        # definition may only exist in the adapter-private solid_impl.hpp.
        $publicGeometryApiDir = Join-Path $RepoRoot 'src\geometry\api'
        if (-not (Test-Path -LiteralPath $publicGeometryApiDir -PathType Container)) {
            throw "Public geometry_api directory not found: $publicGeometryApiDir"
        }
        $solidDefinitionHits = Get-ChildItem -Path $publicGeometryApiDir -Recurse -Include '*.hpp', '*.h' -ErrorAction SilentlyContinue |
            Select-String -Pattern 'struct\s+Solid\s*\{'
        if ($solidDefinitionHits) {
            throw "Found a complete 'struct Solid { ... }' definition under src\geometry\api (must stay forward-declared only): $(($solidDefinitionHits | ForEach-Object { "$($_.Path):$($_.LineNumber)" }) -join ', ')"
        }

        Write-Host 'Confirmed final repository invariants: candidate shape unchanged (re-verified via Test-CandidateInvariant), main unchanged, authority documents unchanged, geometry_api OCCT-free, Solid incomplete publicly.'
    }

    Write-Host ''
    Write-Host '================================================================' -ForegroundColor Green
    Write-Host 'VERIFICATION RUNBOOK C = ALL 21 STEPS PASSED' -ForegroundColor Green
    Write-Host '================================================================' -ForegroundColor Green
    foreach ($label in $stepLog.Keys) {
        Write-Host ("  {0} : {1}" -f $label, $stepLog[$label]) -ForegroundColor Green
    }
    exit 0
} catch {
    Write-Host ''
    Write-Host '================================================================' -ForegroundColor Red
    Write-Host "VERIFICATION RUNBOOK C ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host '================================================================' -ForegroundColor Red
    if ($stepLog.Count -gt 0) {
        Write-Host 'Steps completed before this failure:' -ForegroundColor Yellow
        foreach ($label in $stepLog.Keys) {
            Write-Host ("  {0} : {1}" -f $label, $stepLog[$label]) -ForegroundColor Yellow
        }
    }
    # v1.3 fix (round 5, R5-02): `exit 1` alone only sets $LASTEXITCODE - it
    # raises no PowerShell exception in the CALLER's scope. A wrapper that
    # invokes this runbook as `& .\Verification-RunbookC-v1.0.ps1 ...`
    # inside its own try/catch (rather than explicitly checking
    # $LASTEXITCODE after every external call) never sees `exit 1` as a
    # caught error, falls through its try block as if nothing failed, and
    # can incorrectly report overall PASS - exactly the authoritative
    # Windows defect this round found. A bare `throw` here re-raises the
    # original terminating error after the diagnostics above are printed,
    # so it propagates as a real, catchable PowerShell exception to any
    # caller that wraps this script in try/catch, AND (because an uncaught
    # terminating error at a script's top level also makes
    # powershell.exe/pwsh.exe itself exit non-zero) still leaves
    # $LASTEXITCODE non-zero for a caller that only checks the exit code.
    # Either mechanical observation method now correctly sees FAIL. Never
    # swallow this exception and return normally instead.
    throw
}
