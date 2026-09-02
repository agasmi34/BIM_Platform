<#
==============================================================================
 scripts/ci/format.ps1 - formatting conformance job
 (Implementation Brief Phase L, logical job 1 of 5: "format").

 Checks every first-party C/C++ source/header under src/, tests/, tools/
 against the repository-authoritative .clang-format using
 `clang-format --dry-run --Werror`. Does NOT silently waive the check if
 clang-format is unavailable (Implementation Brief Phase K) - it records an
 explicit operational blocker and exits non-zero instead.

 Exit code: 0 on a clean format pass; non-zero on any format violation or on
 an environment gap (clang-format not found).

 Usage: powershell -File scripts\ci\format.ps1
==============================================================================
#>

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')

try {
    Write-CiSection 'format: locate clang-format'
    $clangFormat = Get-Command clang-format -ErrorAction SilentlyContinue
    if (-not $clangFormat) {
        Write-Host 'OPERATIONAL BLOCKER: clang-format was not found on PATH.' -ForegroundColor Red
        Write-Host "Per Implementation Brief Phase K, this check is not silently waived." -ForegroundColor Red
        Write-Host 'Install clang-format (e.g. via the LLVM installer or the Visual Studio' -ForegroundColor Red
        Write-Host 'Individual Components "C++ Clang tools for Windows") with explicit user' -ForegroundColor Red
        Write-Host 'authorization, then re-run this script.' -ForegroundColor Red
        exit 1
    }
    Invoke-Native -Exe $clangFormat.Source -CmdArgs @('--version') | Out-Null

    Write-CiSection 'format: collect first-party files'
    $extensions = @('.h', '.hpp', '.hh', '.hxx', '.cpp', '.cc', '.cxx')
    $roots = @('src', 'tests', 'tools') | ForEach-Object { Join-Path $RepoRoot $_ }
    $files = foreach ($root in $roots) {
        if (Test-Path $root) {
            Get-ChildItem -Path $root -Recurse -File | Where-Object { $extensions -contains $_.Extension }
        }
    }
    $fileCount = @($files).Count
    Write-Host "Found $fileCount first-party file(s) to check."

    if ($fileCount -eq 0) {
        Write-Host 'No first-party C/C++ files found to format-check.'
        exit 0
    }

    Write-CiSection 'format: clang-format --dry-run --Werror'
    $failed = @()
    foreach ($file in $files) {
        $result = Invoke-Native -Exe $clangFormat.Source -CmdArgs @('--dry-run', '--Werror', $file.FullName) -AllowFailure
        if ($result.ExitCode -ne 0) {
            $failed += $file.FullName
        }
    }

    if ($failed.Count -gt 0) {
        Write-Host ''
        Write-Host "FORMAT CHECK FAILED: $($failed.Count) file(s) not conforming to .clang-format:" -ForegroundColor Red
        $failed | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
        exit 1
    }

    Write-Host ''
    Write-Host 'FORMAT CHECK PASSED' -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "format.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
