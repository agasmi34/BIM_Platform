<#
==============================================================================
 scripts/ci/geometry-spike.ps1 - P0-T002 geometry evidence job
 (Implementation Brief section 21; Amendment 01 AA-C07).

 Locates the already-built p0_t002_geometry_evidence executable, runs it,
 and verifies the JSON evidence it produces. This script never mutates the
 source tree: all evidence is written under the build tree
 (recommended: build\ci-win-msvc\evidence\P0-T002\), never under docs/ or
 any tracked source path.

 Follows the same $LASTEXITCODE-authoritative discipline as
 scripts\ci\architecture.ps1 (Invoke-Native from _common.ps1): a non-zero
 exit from the evidence executable itself throws here and is caught below
 (exit 1) - there is no path through this script that reaches "PASSED"
 without the executable itself having exited 0 AND the JSON output having
 been independently verified to exist, be non-empty, and report
 overall_passed = true.

 Requires: an already-configured AND built build directory containing the
 p0_t002_geometry_evidence target (run scripts\ci\configure-build-test.ps1
 first, or pass -BuildDir to point at an existing one) - the same
 precondition scripts\ci\architecture.ps1 already documents for this repo's
 CI jobs.

 Usage: powershell -File scripts\ci\geometry-spike.ps1 [-BuildDir <path>]
        [-Configuration <name>]
==============================================================================
#>

param(
    [string]$BuildDir,
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}

$EvidenceDir = Join-Path $BuildDir 'evidence\P0-T002'
$EvidenceJsonPath = Join-Path $EvidenceDir 'p0_t002_geometry_evidence.json'

try {
    Write-CiSection "geometry-spike: locate build directory ($BuildDir)"
    if (-not (Test-Path $BuildDir)) {
        throw "Build directory '$BuildDir' does not exist. Run scripts\ci\configure-build-test.ps1 first, or pass -BuildDir."
    }

    Write-CiSection 'geometry-spike: locate p0_t002_geometry_evidence executable'
    # Checked in order: multi-config generator (Visual Studio) layout with
    # the requested -Configuration subdirectory, then a single-config
    # generator (Ninja) layout with no configuration subdirectory. Neither
    # path is assumed - both are probed, and the job fails closed if
    # neither exists rather than guessing.
    $candidatePaths = @(
        (Join-Path $BuildDir "tests\integration\$Configuration\p0_t002_geometry_evidence.exe"),
        (Join-Path $BuildDir 'tests\integration\p0_t002_geometry_evidence.exe')
    )
    $exePath = $candidatePaths | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $exePath) {
        throw "p0_t002_geometry_evidence executable not found under '$BuildDir'. Checked: $($candidatePaths -join ', '). Build the target first (scripts\ci\configure-build-test.ps1)."
    }
    Write-Host "Found evidence executable: $exePath"

    Write-CiSection "geometry-spike: prepare evidence output directory ($EvidenceDir)"
    # Build-tree only (Brief section 21: this script must not mutate
    # source). New-Item -Force is idempotent - safe to re-run this job.
    New-Item -ItemType Directory -Path $EvidenceDir -Force | Out-Null

    Write-CiSection "geometry-spike: run p0_t002_geometry_evidence --json $EvidenceJsonPath"
    # No -AllowFailure: a non-zero exit here (any mandatory-corpus
    # classification mismatch, or an exception escaping the geometry API
    # boundary - see p0_t002_geometry_evidence.cpp) throws and is caught by
    # this script's own catch block below.
    Invoke-Native -Exe $exePath -CmdArgs @('--json', $EvidenceJsonPath)

    Write-CiSection 'geometry-spike: verify evidence JSON output exists and is non-empty'
    if (-not (Test-Path $EvidenceJsonPath)) {
        throw "Evidence executable exited 0 but did not produce '$EvidenceJsonPath'."
    }
    $jsonItem = Get-Item $EvidenceJsonPath
    if ($jsonItem.Length -le 0) {
        throw "Evidence JSON file '$EvidenceJsonPath' is empty."
    }

    $jsonText = Get-Content -Raw -Path $EvidenceJsonPath
    try {
        $evidence = $jsonText | ConvertFrom-Json
    } catch {
        throw "Evidence JSON at '$EvidenceJsonPath' failed to parse as JSON: $($_.Exception.Message)"
    }

    Write-CiSection 'geometry-spike: corpus summary'
    $caseCount = 0
    $failedCount = 0
    $historyCount = 0
    $overallPassed = $false
    if ($evidence.PSObject.Properties.Name -contains 'case_count') { $caseCount = [int]$evidence.case_count }
    if ($evidence.PSObject.Properties.Name -contains 'failed_case_count') { $failedCount = [int]$evidence.failed_case_count }
    if ($evidence.PSObject.Properties.Name -contains 'history_record_count') { $historyCount = [int]$evidence.history_record_count }
    if ($evidence.PSObject.Properties.Name -contains 'overall_passed') { $overallPassed = [bool]$evidence.overall_passed }

    Write-Host ("  task                  : {0}" -f $evidence.task)
    Write-Host ("  case_count            : {0}" -f $caseCount)
    Write-Host ("  failed_case_count     : {0}" -f $failedCount)
    Write-Host ("  history_record_count  : {0}" -f $historyCount)
    Write-Host ("  overall_passed        : {0}" -f $overallPassed)

    if ($evidence.PSObject.Properties.Name -contains 'cases') {
        $byCategory = $evidence.cases | Group-Object -Property category
        foreach ($group in $byCategory) {
            $failedInGroup = @($group.Group | Where-Object { -not $_.passed }).Count
            Write-Host ("    category {0,-20} cases={1,-4} failed={2}" -f $group.Name, $group.Count, $failedInGroup)
        }
    }

    Write-Host ''
    Write-Host "Evidence output path: $EvidenceJsonPath"

    if (-not $overallPassed) {
        throw "Evidence JSON reports overall_passed=false ($failedCount of $caseCount case(s) failed). See $EvidenceJsonPath for the full per-case record."
    }

    Write-Host ''
    Write-Host "GEOMETRY-SPIKE JOB PASSED - evidence executable exited 0, JSON evidence was verified at $EvidenceJsonPath, and overall_passed=true." -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "geometry-spike.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
