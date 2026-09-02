<#
==============================================================================
 scripts/ci/license-inventory.ps1 - third-party license inventory job
 (Implementation Brief Phase L / Phase M, logical job 5 of 5).

 Copies the real, vcpkg-installed copyright/usage notice files for every
 direct P0-T001 dependency into third_party/licenses/, then verifies all
 five are present. Fails if any is missing. Never fabricates or hand-retypes
 license text (Implementation Brief section 11: "Do not insert fake SHAs or
 fake PASS results.").

 Exit code: 0 only if all five direct dependencies have a captured license
 file.

 Usage: powershell -File scripts\ci\license-inventory.ps1 [-BuildDir <path>] [-Triplet <triplet>]
==============================================================================
#>

param(
    [string]$BuildDir,
    [string]$Triplet = 'x64-windows'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}
$LicensesDir = Join-Path $RepoRoot 'third_party\licenses'
$DirectDependencies = @('opencascade', 'sqlite3', 'catch2', 'fmt', 'spdlog')

try {
    Write-CiSection 'license-inventory: locate vcpkg installed tree'
    $vcpkgInstalled = Join-Path $BuildDir "vcpkg_installed\$Triplet"
    if (-not (Test-Path $vcpkgInstalled)) {
        throw "vcpkg installed tree not found at '$vcpkgInstalled'. Run scripts\ci\configure-build-test.ps1 first, or pass -BuildDir/-Triplet."
    }

    Write-CiSection 'license-inventory: remove previously generated outputs for direct dependencies'
    # A stale *.LICENSE.txt/*.USAGE.txt from an earlier run must not be able
    # to make this run's completeness check pass by accident (e.g. if this
    # run's vcpkg_installed tree is broken and the copy below silently finds
    # nothing to copy). Remove exactly the five dependencies' generated
    # filenames before regenerating - nothing else in $LicensesDir (such as
    # third_party\licenses\README.md, which is a real repository file, not
    # a generated one) is touched.
    New-Item -ItemType Directory -Force -Path $LicensesDir | Out-Null
    foreach ($dep in $DirectDependencies) {
        $staleLicense = Join-Path $LicensesDir "$dep.LICENSE.txt"
        $staleUsage = Join-Path $LicensesDir "$dep.USAGE.txt"
        if (Test-Path $staleLicense) {
            Remove-Item -Force $staleLicense
            Write-Host "Removed previously generated '$dep.LICENSE.txt' before regenerating."
        }
        if (Test-Path $staleUsage) {
            Remove-Item -Force $staleUsage
            Write-Host "Removed previously generated '$dep.USAGE.txt' before regenerating."
        }
    }

    Write-CiSection 'license-inventory: copy copyright/usage files from the CURRENT vcpkg_installed tree'
    foreach ($dep in $DirectDependencies) {
        $shareDir = Join-Path $vcpkgInstalled "share\$dep"
        $copyrightSrc = Join-Path $shareDir 'copyright'
        $usageSrc = Join-Path $shareDir 'usage'

        if (Test-Path $copyrightSrc) {
            Copy-Item -Force $copyrightSrc (Join-Path $LicensesDir "$dep.LICENSE.txt")
            Write-Host "Copied copyright for '$dep'."
        } else {
            Write-Host "WARNING: no 'copyright' file found for '$dep' under '$shareDir'." -ForegroundColor Yellow
        }

        if (Test-Path $usageSrc) {
            Copy-Item -Force $usageSrc (Join-Path $LicensesDir "$dep.USAGE.txt")
        }
    }

    Write-CiSection 'license-inventory: verify completeness (from the current run only - stale outputs were removed above)'
    $missing = @()
    foreach ($dep in $DirectDependencies) {
        $expected = Join-Path $LicensesDir "$dep.LICENSE.txt"
        if (-not (Test-Path $expected)) {
            $missing += $dep
        }
    }

    if ($missing.Count -gt 0) {
        Write-Host ''
        Write-Host "LICENSE INVENTORY FAILED: missing copyright entries for: $($missing -join ', ')" -ForegroundColor Red
        exit 1
    }

    Write-Host ''
    Write-Host 'LICENSE INVENTORY PASSED' -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "license-inventory.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
