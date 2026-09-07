<#
==============================================================================
 scripts/ci/architecture.ps1 - architecture-boundary check job
 (Implementation Brief Phase L, logical job 4 of 5; extended P0-T002 Phase
 C-M, Amendment 01 AA-C10).

 P0-T001 required exactly two CTest tests (tools/architecture_checker.cmake)
 to prove the architecture-boundary checker actually works:
   - arch_repository_boundaries       (the positive check: the real src/
     tree has zero violations)
   - arch_checker_detects_violation   (the negative self-test: the checker
     actually flags the controlled bad fixture under
     tests/fixtures/bad_architecture/ - proving the checker itself isn't a
     no-op)

 P0-T002 Phase C-M adds two more required tests (Amendment 01 AA-C10),
 backed by the new BIM_ARCH_CHECK_RULE selector in
 tools/architecture_checker.cmake:
   - arch_geometry_api_no_occt_leak        (rule R6: src/geometry/api/include
     stays OCCT-free)
   - arch_geometry_occt_only_kernel_owner  (rule R7: no OCCT token appears
     anywhere under src/ outside src/geometry/occt/**)

 v1.5 fix (architecture review "P0-T001 Architecture Review - Verification
 Candidate v1.4" - ONE FINAL BLOCKER): a single broad
 `ctest -R "^arch_"` invocation does not prove all required tests were
 actually registered and executed - a regex that happens to match zero
 tests, or that only matches some of them, can still exit 0 depending on
 ctest's own no-tests-found semantics, producing a false PASS. This script
 runs each required test as its own explicit, exact-name-anchored
 invocation:

   ctest -R "^arch_repository_boundaries$"            --output-on-failure --no-tests=error
   ctest -R "^arch_checker_detects_violation$"         --output-on-failure --no-tests=error
   ctest -R "^arch_geometry_api_no_occt_leak$"         --output-on-failure --no-tests=error
   ctest -R "^arch_geometry_occt_only_kernel_owner$"   --output-on-failure --no-tests=error

 `--no-tests=error` makes ctest itself exit non-zero if the exact-name
 regex matches zero registered tests (i.e. the test is missing/not
 registered), not just if a matched test fails. Every invocation goes
 through Invoke-Native WITHOUT -AllowFailure, so a non-zero exit from any
 one of them throws and is caught by this script's own catch block below
 (exit 1) - there is no path through this script that reaches "PASSED"
 without every command having exited 0. Concretely, for EACH required test:
   - missing (not registered)          -> FAIL
   - registered but failing            -> FAIL
   - registered AND passing            -> that test's own check passes
 Only when every required test is registered and passing does this script
 report PASS.

 Requires: an already-configured build directory (Run
 scripts\ci\configure-build-test.ps1 first, or pass -BuildDir to point at
 an existing one).

 Exit code: 0 only if ALL required tests are found registered AND all pass.
 Non-zero otherwise (any missing test, or any found test failing).

 Usage: powershell -File scripts\ci\architecture.ps1 [-BuildDir <path>]
==============================================================================
#>

param(
    [string]$BuildDir
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}

$RequiredTests = @(
    'arch_repository_boundaries',
    'arch_checker_detects_violation',
    'arch_geometry_api_no_occt_leak',
    'arch_geometry_occt_only_kernel_owner'
)

try {
    Write-CiSection "architecture: locate build directory ($BuildDir)"
    if (-not (Test-Path $BuildDir)) {
        throw "Build directory '$BuildDir' does not exist. Run scripts\ci\configure-build-test.ps1 first, or pass -BuildDir."
    }

    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }

    Push-Location $BuildDir
    try {
        foreach ($testName in $RequiredTests) {
            $exactPattern = '^' + $testName + '$'
            Write-CiSection "architecture: ctest -R `"$exactPattern`" --output-on-failure --no-tests=error"
            # No -AllowFailure: a missing test (ctest exits non-zero under
            # --no-tests=error when the exact-name regex matches nothing)
            # or a failing test both throw here and are caught below -
            # either one aborts this job with a non-zero exit.
            Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('-R', $exactPattern, '--output-on-failure', '--no-tests=error')
        }
    } finally {
        Pop-Location
    }

    Write-Host ''
    Write-Host "ARCHITECTURE CHECK JOB PASSED - all required tests ($($RequiredTests -join ', ')) were found registered and passed." -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "architecture.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
