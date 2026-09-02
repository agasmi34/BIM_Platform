<#
==============================================================================
 scripts/ci/architecture.ps1 - architecture-boundary check job
 (Implementation Brief Phase L, logical job 4 of 5).

 P0-T001 requires exactly two CTest tests (tools/architecture_checker.cmake)
 to prove the architecture-boundary checker actually works:
   - arch_repository_boundaries       (the positive check: the real src/
     tree has zero violations)
   - arch_checker_detects_violation   (the negative self-test: the checker
     actually flags the controlled bad fixture under
     tests/fixtures/bad_architecture/ - proving the checker itself isn't a
     no-op)

 v1.5 fix (architecture review "P0-T001 Architecture Review - Verification
 Candidate v1.4" - ONE FINAL BLOCKER): a single broad
 `ctest -R "^arch_"` invocation does not prove both required tests were
 actually registered and executed - a regex that happens to match zero
 tests, or that only matches one of the two, can still exit 0 depending on
 ctest's own no-tests-found semantics, producing a false PASS. This script
 now runs each required test as its own explicit, exact-name-anchored
 invocation:

   ctest -R "^arch_repository_boundaries$"     --output-on-failure --no-tests=error
   ctest -R "^arch_checker_detects_violation$" --output-on-failure --no-tests=error

 `--no-tests=error` makes ctest itself exit non-zero if the exact-name
 regex matches zero registered tests (i.e. the test is missing/not
 registered), not just if a matched test fails. Both invocations go
 through Invoke-Native WITHOUT -AllowFailure, so a non-zero exit from
 either one throws and is caught by this script's own catch block below
 (exit 1) - there is no path through this script that reaches "PASSED"
 without both commands having exited 0. Concretely:
   - arch_repository_boundaries missing (not registered)      -> FAIL
   - arch_checker_detects_violation missing (not registered)  -> FAIL
   - arch_repository_boundaries registered but failing        -> FAIL
   - arch_checker_detects_violation registered but failing    -> FAIL
   - both registered AND both passing                         -> PASS
 This replaces the prior single `ctest -R "^arch_"` invocation, which is
 removed - it did not enforce this distinction.

 Requires: an already-configured build directory (Run
 scripts\ci\configure-build-test.ps1 first, or pass -BuildDir to point at
 an existing one).

 Exit code: 0 only if BOTH required tests are found registered AND both
 pass. Non-zero otherwise (missing test, or a found test failing).

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

$RequiredTests = @('arch_repository_boundaries', 'arch_checker_detects_violation')

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
    Write-Host "ARCHITECTURE CHECK JOB PASSED - both required tests ($($RequiredTests -join ', ')) were found registered and passed." -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "architecture.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
