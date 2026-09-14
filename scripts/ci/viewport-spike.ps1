<#
==============================================================================
 scripts/ci/viewport-spike.ps1 - P0-T003 Desktop + Viewport Spike CI job
 (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0 section 23; exercised
 by Verification-RunbookD-v1.0.ps1 check 20 "viewport-spike evidence job").

 Mirrors scripts/ci/architecture.ps1's exact-name-anchored ctest pattern
 (that script's own header explains why a single broad `ctest -R "^..."`
 invocation is not trustworthy proof every required test ran - the same
 reasoning applies here). Runs, as separate exact-name-anchored
 invocations:

   - the four new unit tests (unit_viewport_mesh_contract, unit_viewport_camera,
     unit_viewport_ray, unit_viewport_lifecycle)
   - the two new integration tests (integration_viewport_bgfx_headless,
     integration_viewport_bgfx_resource_lifecycle)
   - the eight new/extended architecture tests (arch_viewport_public_neutral,
     arch_p0_t003_viewport_fixture_rejected, arch_qt_desktop_only,
     arch_p0_t003_qt_fixture_rejected, arch_bgfx_viewport_owner,
     arch_p0_t003_bgfx_fixture_rejected, arch_no_direct_d3d,
     arch_p0_t003_d3d_fixture_rejected)

 Then, per Brief section 23:
   - executes the required process-level repeatability cycles: invokes
     bim_desktop_spike.exe --evidence-mode <json-path> (headless/noop
     backend, CTest-safe) as $ProcessLevelRepeatabilityCycles (default 20,
     Brief section 16: "at least 20") SEPARATE OS process launches, each
     writing its own JSON evidence file and each required to exit 0 - this
     is the process-boundary variant of the repeatability requirement,
     distinct from (and in addition to) the in-process repeatability loop
     evidence_mode.cpp itself also runs on every HEADLESS invocation (see
     the "repeatability" field of each cycle's own JSON; RD1.7: live mode
     records that field as NOT_AVAILABLE / externally covered by THIS
     script, because the production ViewportWindow owns the one bgfx
     runtime for the whole live run);
   - AA RD1.7 (Full Runbook D Attempt 2, Checks 35/60): before launching
     any bim_desktop_spike.exe child, resolves the Qt platform plugin root
     from the CURRENT -BuildDir (<BuildDir>\vcpkg_installed\x64-windows\
     Qt6\plugins), requires platforms\qwindows.dll there (fail-closed),
     and gives the child processes a PROCESS-LOCAL QT_PLUGIN_PATH /
     QT_QPA_PLATFORM_PLUGIN_PATH / QT_QPA_PLATFORM=windows environment,
     restored afterward - never the user/system environment. Attempt 2
     failed before any evidence ran with 'Could not find the Qt platform
     plugin "windows"' because nothing wired this up; CI/runtime wiring
     only, no CMake deployment redesign;
   - executes the final live evidence mode: bim_desktop_spike.exe
     --evidence-mode-live <json-path>, validates the JSON exists and is
     well-formed, validates explicit D3D11 backend identity when a live
     surface was actually available, and reports cross-monitor
     differing-DPI availability honestly (Brief section 17: NOT_AVAILABLE
     is not a job failure, only a recorded gap) rather than failing the
     job over an environment limitation;
   - validates overall_passed on the final live evidence JSON and
     propagates a non-zero exit code if it is false;
   - never edits source/docs; never changes dependencies.

 AA Source Review Round 1 finding M05 correction: the prior version of
 this script drove the old plain-text `evidence_report.txt` / single
 headless-only invocation; this revision drives the new machine-readable
 JSON evidence mode (both headless and live), including the process-level
 repeatability requirement this script did not previously implement at
 all.

 UNVERIFIED: this script has never been executed in this session (no
 execution channel - see docs/evidence/P0-T003/CLAUDE_HANDOVER.md). It is
 authored against the exact _common.ps1/Invoke-Native/Write-CiSection
 pattern already proven out by scripts/ci/architecture.ps1 and
 scripts/ci/license-inventory.ps1, but the Operator's first real run is
 its first actual execution.

 Requires: an already-configured, built build directory (Run
 scripts\ci\configure-build-test.ps1 first, or pass -BuildDir to point at
 an existing one).

 Exit code: 0 only if every required CTest test is found registered AND
 passing, every process-level repeatability cycle exits 0, and the final
 live evidence JSON's overall_passed is true (or the live surface was
 honestly unavailable in this environment - see above).

 Usage: powershell -File scripts\ci\viewport-spike.ps1 [-BuildDir <path>] [-ProcessLevelRepeatabilityCycles <int>] [-RunLiveEvidence:$false]
==============================================================================
#>

param(
    [string]$BuildDir,
    [int]$ProcessLevelRepeatabilityCycles = 20,
    [bool]$RunLiveEvidence = $true
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}

$RequiredTests = @(
    'unit_viewport_mesh_contract',
    'unit_viewport_camera',
    'unit_viewport_ray',
    'unit_viewport_lifecycle',
    'integration_viewport_bgfx_headless',
    'integration_viewport_bgfx_resource_lifecycle',
    'arch_viewport_public_neutral',
    'arch_p0_t003_viewport_fixture_rejected',
    'arch_qt_desktop_only',
    'arch_p0_t003_qt_fixture_rejected',
    'arch_bgfx_viewport_owner',
    'arch_p0_t003_bgfx_fixture_rejected',
    'arch_no_direct_d3d',
    'arch_p0_t003_d3d_fixture_rejected'
)

function Find-SpikeExe {
    param([Parameter(Mandatory)][string]$InBuildDir)
    $candidates = @(
        (Join-Path $InBuildDir 'src\desktop\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'src\desktop\Release\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'src\desktop\Debug\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'src\desktop\RelWithDebInfo\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'Release\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'Debug\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'RelWithDebInfo\bim_desktop_spike.exe')
    )
    $found = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $found) {
        throw "bim_desktop_spike.exe was not found under '$InBuildDir' (checked common single-/multi-config layouts)."
    }
    return $found
}

# AA RD1.7 (Full Runbook D Attempt 2, Checks 35/60 - Qt platform plugin CI
# wiring): resolves the authoritative Qt6 plugin root from the CURRENT build
# directory (the accepted x64-windows vcpkg triplet's install tree - never a
# hard-coded worktree path) and fails closed if the Windows platform plugin
# is not there. See the header comment.
function Resolve-QtPluginRoot {
    param([Parameter(Mandatory)][string]$InBuildDir)
    $pluginRoot = Join-Path $InBuildDir 'vcpkg_installed\x64-windows\Qt6\plugins'
    $qwindows = Join-Path $pluginRoot 'platforms\qwindows.dll'
    if (-not (Test-Path -LiteralPath $qwindows -PathType Leaf)) {
        throw "Qt platform plugin not found at '$qwindows' (expected under the build's own vcpkg_installed tree, derived from -BuildDir '$InBuildDir'). Without it QApplication cannot start ('Could not find the Qt platform plugin ""windows""'). Fail-closed: not guessing an alternate location."
    }
    return $pluginRoot
}

# AA RD1.7: puts the three process-local Qt variables back exactly as they
# were before this script set them ($null restores "unset"). Called on the
# normal path once the evidence invocations are done, and from the outer
# catch on the failure path. No-op if the variables were never set.
$script:PreviousQtEnv = $null
function Restore-QtPluginEnvironment {
    if ($null -eq $script:PreviousQtEnv) { return }
    foreach ($name in $script:PreviousQtEnv.Keys) {
        [Environment]::SetEnvironmentVariable($name, $script:PreviousQtEnv[$name], 'Process')
    }
    $script:PreviousQtEnv = $null
}

try {
    Write-CiSection "viewport-spike: locate build directory ($BuildDir)"
    if (-not (Test-Path $BuildDir)) {
        throw "Build directory '$BuildDir' does not exist. Run scripts\ci\configure-build-test.ps1 first, or pass -BuildDir."
    }

    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }

    Push-Location $BuildDir
    try {
        foreach ($testName in $RequiredTests) {
            $exactPattern = '^' + $testName + '$'
            Write-CiSection "viewport-spike: ctest -R `"$exactPattern`" --output-on-failure --no-tests=error"
            # No -AllowFailure: a missing test (ctest exits non-zero under
            # --no-tests=error when the exact-name regex matches nothing)
            # or a failing test both throw here and are caught below.
            Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('-R', $exactPattern, '--output-on-failure', '--no-tests=error')
        }
    } finally {
        Pop-Location
    }

    $spikeExe = Find-SpikeExe -InBuildDir $BuildDir
    $evidenceDir = Join-Path $BuildDir 'evidence\P0-T003'
    New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null

    # ---- AA RD1.7: process-local Qt platform plugin environment for every
    # bim_desktop_spike.exe child launched below (fail-closed resolution;
    # previous values captured so Restore-QtPluginEnvironment can put them
    # back once the evidence invocations are done - this script keeps
    # running afterward, and the outer catch restores on the failure path
    # too). 'Process' scope only: inherited by the children, never written
    # to the user/system environment.
    Write-CiSection 'viewport-spike: resolve Qt platform plugin root from the build directory (process-local)'
    $qtPluginRoot = Resolve-QtPluginRoot -InBuildDir $BuildDir
    $qtPlatformsDir = Join-Path $qtPluginRoot 'platforms'
    $script:PreviousQtEnv = @{
        'QT_PLUGIN_PATH'              = [Environment]::GetEnvironmentVariable('QT_PLUGIN_PATH', 'Process')
        'QT_QPA_PLATFORM_PLUGIN_PATH' = [Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM_PLUGIN_PATH', 'Process')
        'QT_QPA_PLATFORM'             = [Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM', 'Process')
    }
    [Environment]::SetEnvironmentVariable('QT_PLUGIN_PATH', $qtPluginRoot, 'Process')
    [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM_PLUGIN_PATH', $qtPlatformsDir, 'Process')
    [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM', 'windows', 'Process')
    Write-Host "  QT_PLUGIN_PATH              = $qtPluginRoot"
    Write-Host "  QT_QPA_PLATFORM_PLUGIN_PATH = $qtPlatformsDir"
    Write-Host '  QT_QPA_PLATFORM             = windows'

    # ---- Brief section 16/23: process-level repeatability cycles ----
    Write-CiSection "viewport-spike: $ProcessLevelRepeatabilityCycles process-level repeatability cycles (headless)"
    for ($cycle = 1; $cycle -le $ProcessLevelRepeatabilityCycles; $cycle++) {
        $cycleJson = Join-Path $evidenceDir "process-cycle-$cycle.json"
        Write-Host "  process-level cycle $cycle / $ProcessLevelRepeatabilityCycles -> $cycleJson"
        # No -AllowFailure: Brief section 16 requires "each process must
        # return success" - a non-zero exit here throws and is caught
        # below, failing the whole job, not just this one cycle.
        Invoke-Native -Exe $spikeExe -CmdArgs @('--evidence-mode', $cycleJson) | Out-Null
        if (-not (Test-Path $cycleJson)) {
            throw "process-level repeatability cycle $cycle exited 0 but did not write the expected JSON at '$cycleJson'."
        }
    }
    Write-Host "  all $ProcessLevelRepeatabilityCycles process-level repeatability cycles exited 0." -ForegroundColor Green

    # ---- Brief section 23: final live evidence mode ----
    if ($RunLiveEvidence) {
        Write-CiSection 'viewport-spike: bim_desktop_spike.exe --evidence-mode-live (final live evidence)'
        $liveJsonPath = Join-Path $evidenceDir 'live-evidence.json'
        # No -AllowFailure here either: overall_passed is validated below
        # from the JSON itself (a richer signal than the bare exit code
        # alone - e.g. distinguishing "everything failed" from "the live
        # surface was honestly unavailable in this environment"), but a
        # non-zero exit with no JSON at all is still a hard failure.
        $liveExitCode = 0
        try {
            Invoke-Native -Exe $spikeExe -CmdArgs @('--evidence-mode-live', $liveJsonPath) | Out-Null
        } catch {
            $liveExitCode = 1
        }

        if (-not (Test-Path $liveJsonPath)) {
            throw "bim_desktop_spike.exe --evidence-mode-live did not write the expected JSON at '$liveJsonPath' (process exit code was $(if ($liveExitCode -eq 0) { 0 } else { 'non-zero' }))."
        }

        $liveJsonText = Get-Content $liveJsonPath -Raw
        $liveEvidence = $liveJsonText | ConvertFrom-Json

        Write-Host ''
        Write-Host "--- $liveJsonPath (selected fields) ---"
        Write-Host "  evidence_mode           : $($liveEvidence.evidence_mode)"
        Write-Host "  backend.selected_backend: $($liveEvidence.backend.selected_renderer_backend)"
        Write-Host "  overall_passed          : $($liveEvidence.overall_passed)"

        $backendName = $liveEvidence.backend.selected_renderer_backend
        $liveOnSurfaceAvailable = $liveEvidence.lifecycle.on_surface_available
        $liveSurfaceWasAvailable = -not ($liveOnSurfaceAvailable -and
            ($liveOnSurfaceAvailable.PSObject.Properties.Name -contains 'status') -and
            ($liveOnSurfaceAvailable.status -eq 'NOT_AVAILABLE'))

        # AA RD1.8 Amendment A1: the authoritative runtime backend identity
        # bgfx reports (and evidence_mode.cpp's own live gate now requires,
        # RD1.8-02) is the canonical spelling 'Direct3D 11' - with the
        # space. This comparison previously used 'Direct3D11', which never
        # matches the real name and would independently fail this job even
        # after a fully passing live run. Only the spelling changes: a live
        # surface with any backend other than exactly 'Direct3D 11' still
        # fails closed.
        if ($liveSurfaceWasAvailable -and $backendName -ne 'Direct3D 11') {
            throw "live evidence run had a native surface available but resolved backend was '$backendName', not the authoritative 'Direct3D 11' (Brief section 2/9: auto-selecting a different backend and calling that PASS is forbidden)."
        }
        if (-not $liveSurfaceWasAvailable) {
            # AA Source Review Round 4 MINOR correction: this comment
            # previously claimed the job "does not fail solely for that
            # reason." That was stale/inaccurate - evidence_mode.cpp
            # explicitly records a RecordResult(false) whenever no live
            # surface/production ViewportWindow was available (Brief
            # section 17's NOT_AVAILABLE-is-honestly-recorded posture does
            # not mean NOT_AVAILABLE-is-exempted-from-overall_passed), so
            # overall_passed will be false in this case and the check just
            # below (`if (-not $liveEvidence.overall_passed) { throw ... }`)
            # does fail the job. That fail-closed behavior is the intended
            # one, not a bug - this NOTE now says so instead of contradicting
            # the code beneath it.
            Write-Host '  NOTE: no native surface was available in this environment for the live evidence run - recorded as NOT_AVAILABLE, not silently passed (Brief section 17 posture). This job DOES fail as a result (overall_passed will be false - see the check below): the Operator must obtain a real live D3D11 run on an attached-display machine before this job can pass and before AC acceptance.' -ForegroundColor Yellow
        }

        # Brief section 23: "reports cross-monitor differing-DPI
        # availability honestly" - never a job failure by itself.
        $crossMonitor = $liveEvidence.window.cross_monitor_differing_dpi_available
        Write-Host "  cross_monitor_differing_dpi_available: $crossMonitor"

        if (-not $liveEvidence.overall_passed) {
            throw "live evidence JSON reported overall_passed = false - see '$liveJsonPath' for the per-check breakdown."
        }
    } else {
        Write-CiSection 'viewport-spike: -RunLiveEvidence:$false - skipping the final live evidence mode'
    }

    # AA RD1.7: all bim_desktop_spike.exe children have run - restore the
    # process-local Qt plugin variables to their previous values.
    Restore-QtPluginEnvironment

    Write-Host ''
    Write-Host "VIEWPORT SPIKE CI JOB PASSED - all required tests ($($RequiredTests -join ', ')) were found registered and passed, $ProcessLevelRepeatabilityCycles process-level repeatability cycles exited 0$(if ($RunLiveEvidence) { ', and the final live evidence JSON reported overall_passed = true' })." -ForegroundColor Green
    exit 0
} catch {
    # AA RD1.7: failure path - restore the process-local Qt plugin
    # variables too (no-op if they were never set).
    Restore-QtPluginEnvironment
    Write-Host ''
    Write-Host "viewport-spike.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
