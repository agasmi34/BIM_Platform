<#
==============================================================================
 scripts/ci/static-analysis.ps1 - clang-tidy static-analysis job
 (Implementation Brief Phase K / Phase N, logical job 3 of 5:
 "static-analysis", inserted between configure-build-test and architecture).

 Runs clang-tidy, report-only (never -fix), against every first-party C++
 translation unit recorded in the compile database (compile_commands.json)
 produced by the authoritative ci-win-msvc CMake configuration
 (CMAKE_EXPORT_COMPILE_COMMANDS=ON - see CMakePresets.json conf-common).
 Does NOT silently waive the check if clang-tidy is unavailable
 (Implementation Brief Phase K) - it records an explicit operational
 blocker and exits non-zero instead, exactly like scripts\ci\format.ps1
 does for clang-format.

 Scope (first-party only): a translation unit from the compile database is
 checked only if its resolved path is under src\ or tests\ AND is not
 under any \vcpkg_installed\ or \third_party\ segment. This is a positive
 allow-list (src\, tests\), not a denylist, so build\ output that is not a
 real first-party translation unit, vcpkg/system sources, and any future
 generated content are excluded by construction - they are either absent
 from the compile database entirely (vcpkg builds its own ports out of
 tree) or filtered out here. "Future-placeholder" module directories
 (interop/ifc, interop/dwg, interop/rvt, documentation, viewport, desktop)
 contain no .cpp files as of P0-T001, so they never appear in the compile
 database and need no special-case exclusion.

 First-party HEADER content reachable from those translation units is
 checked via --header-filter, scoped to src\ and tests\ only - vcpkg or
 other system headers pulled in transitively are not flagged even though
 they are necessarily parsed.

 This script never passes -fix or any other automatic-rewrite flag to
 clang-tidy. It only reports; it never modifies source files.

 This script only ever touches paths under $RepoRoot (resolved from its
 own location - no user-specific absolute paths) and $BuildDir (which
 defaults to a path under $RepoRoot); it does not write anywhere outside
 the task worktree and never touches D:\Projects\BIM-Platform (main).

 Requires: an already-configured build directory containing
 compile_commands.json (run scripts\ci\configure-build-test.ps1 first, or
 pass -BuildDir to point at an existing one).

 IMPORTANT evidentiary note on this job's own exit-code semantics
 (updated - Architecture Authority disposition, P0-T001 Architecture
 Review "Verification Runbook B v1.3" finding BLOCKER 2): the
 repository-authoritative .clang-tidy now sets `WarningsAsErrors: '*'`
 (previously `''`/none) - see docs/project-control/DECISION-LEDGER.md and
 CHANGE-LOG.md for the disposition record. This means enabled findings
 from the curated Checks list are themselves CI-gating: clang-tidy's own
 exit code is non-zero not only on tool failures (crash, bad config, an
 unparseable translation unit) but also whenever any enabled check fires
 on a checked first-party translation unit. This script still does not
 pass --warnings-as-errors on its own command line and does not need to -
 the gating now comes from the repository .clang-tidy's own
 WarningsAsErrors setting, which this script reads via --config-file, not
 from a flag this script adds unilaterally. "Return non-zero if any
 clang-tidy invocation fails" is implemented the same way as before
 (gate on each invocation's own exit code) - what changed is what that
 exit code now means, per the .clang-tidy config, not this script's logic.

 Exit code: 0 only if clang-tidy is found, the repository .clang-tidy and
 the compile database both exist, at least one first-party translation
 unit was found to check, and every clang-tidy invocation's own exit code
 is 0. Non-zero if clang-tidy is not found (operational blocker, not
 silently waived), if the repository .clang-tidy or compile database is
 missing, if zero first-party translation units are found (treated as a
 configuration failure, not a vacuous pass), or if any invocation fails.

 Usage: powershell -File scripts\ci\static-analysis.ps1 [-BuildDir <path>]
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
$ClangTidyConfig = Join-Path $RepoRoot '.clang-tidy'

try {
    Write-CiSection 'static-analysis: locate clang-tidy'
    $clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
    if (-not $clangTidy) {
        Write-Host 'OPERATIONAL BLOCKER: clang-tidy was not found on PATH.' -ForegroundColor Red
        Write-Host 'Per Implementation Brief Phase K, this check is not silently waived.' -ForegroundColor Red
        Write-Host 'Install clang-tidy (e.g. via the LLVM installer or the Visual Studio' -ForegroundColor Red
        Write-Host 'Individual Components "C++ Clang tools for Windows") with explicit user' -ForegroundColor Red
        Write-Host 'authorization, then re-run this script. Do not silently install it from' -ForegroundColor Red
        Write-Host 'inside this script and do not alter system software here.' -ForegroundColor Red
        exit 1
    }
    Invoke-Native -Exe $clangTidy.Source -CmdArgs @('--version') | Out-Null

    Write-CiSection 'static-analysis: locate repository .clang-tidy'
    if (-not (Test-Path $ClangTidyConfig)) {
        throw "Repository-authoritative .clang-tidy not found at '$ClangTidyConfig'."
    }
    Write-Host "Using repository .clang-tidy: $ClangTidyConfig"

    Write-CiSection "static-analysis: locate compile database ($BuildDir)"
    $compileDbPath = Join-Path $BuildDir 'compile_commands.json'
    if (-not (Test-Path $compileDbPath)) {
        throw "compile_commands.json not found at '$compileDbPath'. Run scripts\ci\configure-build-test.ps1 first (it verifies CMAKE_EXPORT_COMPILE_COMMANDS=ON produced this file), or pass -BuildDir."
    }

    Write-CiSection 'static-analysis: select first-party translation units from the compile database'
    $compileDbRaw = Get-Content -Raw -Path $compileDbPath
    $compileDb = $compileDbRaw | ConvertFrom-Json

    $repoRootFull = (Resolve-Path $RepoRoot).Path
    $srcRootFull = Join-Path $repoRootFull 'src'
    $testsRootFull = Join-Path $repoRootFull 'tests'

    $firstPartyFiles = New-Object System.Collections.Generic.List[string]
    foreach ($entry in $compileDb) {
        $entryFile = $entry.file
        if ([string]::IsNullOrWhiteSpace($entryFile)) { continue }

        if (-not [System.IO.Path]::IsPathRooted($entryFile)) {
            $entryFile = Join-Path $entry.directory $entryFile
        }

        $resolvedFile = $null
        try {
            $resolvedFile = (Resolve-Path -LiteralPath $entryFile -ErrorAction Stop).Path
        } catch {
            # A compile database entry that does not resolve to a real file
            # on disk cannot be a first-party translation unit we can check;
            # skip it rather than fail the whole job on a stale entry.
            continue
        }

        $isUnderSrc = $resolvedFile.StartsWith($srcRootFull + '\', [System.StringComparison]::OrdinalIgnoreCase)
        $isUnderTests = $resolvedFile.StartsWith($testsRootFull + '\', [System.StringComparison]::OrdinalIgnoreCase)
        $lowerResolved = $resolvedFile.ToLowerInvariant()
        $isVendoredOrGenerated = $lowerResolved.Contains('\vcpkg_installed\') -or $lowerResolved.Contains('\third_party\')

        if (($isUnderSrc -or $isUnderTests) -and -not $isVendoredOrGenerated) {
            if (-not $firstPartyFiles.Contains($resolvedFile)) {
                $firstPartyFiles.Add($resolvedFile)
            }
        }
    }

    $fileCount = $firstPartyFiles.Count
    Write-Host "Found $fileCount first-party translation unit(s) in the compile database to check."
    foreach ($f in $firstPartyFiles) { Write-Host "  - $f" }

    if ($fileCount -eq 0) {
        throw 'No first-party translation units were found in the compile database (looked under src\ and tests\, excluding vcpkg_installed\/third_party\). This is treated as a configuration failure, not a pass - either the compile database is empty/misconfigured or no first-party target was built.'
    }

    Write-CiSection 'static-analysis: clang-tidy (report-only, no -fix)'
    $headerFilter = '^' + [regex]::Escape($repoRootFull) + '\\(src|tests)\\.*'
    Write-Host "Header filter: $headerFilter"
    $failed = @()
    foreach ($file in $firstPartyFiles) {
        $tidyArgs = @(
            '-p', $BuildDir,
            "--config-file=$ClangTidyConfig",
            "--header-filter=$headerFilter",
            $file
        )
        $result = Invoke-Native -Exe $clangTidy.Source -CmdArgs $tidyArgs -AllowFailure
        if ($result.ExitCode -ne 0) {
            $failed += $file
        }
    }

    if ($failed.Count -gt 0) {
        Write-Host ''
        Write-Host "STATIC ANALYSIS FAILED: $($failed.Count) translation unit(s) where clang-tidy's own invocation did not exit 0:" -ForegroundColor Red
        $failed | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
        exit 1
    }

    Write-Host ''
    Write-Host 'STATIC ANALYSIS PASSED' -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "static-analysis.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
