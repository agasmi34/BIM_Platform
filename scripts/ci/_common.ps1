<#
==============================================================================
 scripts/ci/_common.ps1 - shared helpers for BIM Platform provider-neutral CI
 scripts (scripts/ci/*.ps1). Dot-source this file; do not execute it
 directly.

 Invoke-Native applies the same $LASTEXITCODE-authoritative pattern
 established - and hardened through three real architecture-review rounds,
 including a genuine PowerShell 5.1 execution defect fix - in Bootstrap
 Runbook A v1.3 for P0-T001: native stderr captured via 2>&1 must not be
 allowed to become a terminating PowerShell ErrorRecord under
 $ErrorActionPreference = 'Stop', or intentional -AllowFailure probes and
 ordinary tool warnings on stderr get misclassified as script-terminating
 errors. $LASTEXITCODE is the sole authority for success/failure, decided
 only after $ErrorActionPreference is temporarily relaxed around the native
 call and restored via finally.
==============================================================================
#>

function Write-CiSection {
    param([Parameter(Mandatory)][string]$Title)
    Write-Host ''
    Write-Host "=== $Title ===" -ForegroundColor Cyan
}

function Invoke-Native {
    param(
        [Parameter(Mandatory)][string]$Exe,
        [Parameter(Mandatory)][string[]]$CmdArgs,
        [switch]$AllowFailure
    )
    $stdoutLines = [System.Collections.Generic.List[string]]::new()
    $stderrLines = [System.Collections.Generic.List[string]]::new()
    $previousEap = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Exe @CmdArgs 2>&1 | ForEach-Object {
            if ($_ -is [System.Management.Automation.ErrorRecord]) {
                $stderrLines.Add($_.ToString())
            } else {
                $stdoutLines.Add([string]$_)
            }
        }
        $code = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousEap
    }

    $result = [PSCustomObject]@{
        ExitCode = $code
        Stdout   = ($stdoutLines -join "`n")
        Stderr   = ($stderrLines -join "`n")
    }

    if ($result.Stdout) { Write-Host $result.Stdout }
    if ($result.Stderr) { Write-Host $result.Stderr }

    if ($code -ne 0 -and -not $AllowFailure) {
        throw "Command failed (exit $code): $Exe $($CmdArgs -join ' ')"
    }
    return $result
}

function Assert-CiEnvVar {
    param([Parameter(Mandatory)][string]$Name)
    $value = [System.Environment]::GetEnvironmentVariable($Name)
    if ([string]::IsNullOrWhiteSpace($value)) {
        throw "Required environment variable '$Name' is not set."
    }
    return $value
}
