<#
==============================================================================
 scripts/ci/run-all.ps1 - runs all five provider-neutral CI-equivalent jobs
 in order (Implementation Brief Phase L / Phase K): format,
 configure-build-test, static-analysis, architecture, license-inventory.
 Stops at the first failure and reports which job failed. Exit code: 0
 only if all five jobs pass.

 Usage: powershell -File scripts\ci\run-all.ps1
==============================================================================
#>

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$jobs = @(
    @{ Name = 'format'; Script = 'format.ps1' },
    @{ Name = 'configure-build-test'; Script = 'configure-build-test.ps1' },
    @{ Name = 'static-analysis'; Script = 'static-analysis.ps1' },
    @{ Name = 'architecture'; Script = 'architecture.ps1' },
    @{ Name = 'license-inventory'; Script = 'license-inventory.ps1' }
)

$results = [ordered]@{}

foreach ($job in $jobs) {
    Write-CiSection "run-all: $($job.Name)"
    $scriptPath = Join-Path $PSScriptRoot $job.Script
    & $scriptPath
    $code = $LASTEXITCODE
    $results[$job.Name] = $code
    if ($code -ne 0) {
        Write-Host ''
        Write-Host "run-all: job '$($job.Name)' FAILED (exit $code). Stopping." -ForegroundColor Red
        break
    }
}

Write-CiSection 'run-all: summary'
foreach ($name in $results.Keys) {
    $code = $results[$name]
    $status = if ($code -eq 0) { 'PASS' } else { "FAIL (exit $code)" }
    $color = if ($code -eq 0) { 'Green' } else { 'Red' }
    Write-Host ("  {0,-24} {1}" -f $name, $status) -ForegroundColor $color
}

$anyFailed = @($results.Values | Where-Object { $_ -ne 0 }).Count -gt 0
$allRan = $results.Count -eq $jobs.Count
if ($anyFailed -or -not $allRan) {
    exit 1
}
exit 0
