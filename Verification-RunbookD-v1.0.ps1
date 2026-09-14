<#
==============================================================================
 Verification-RunbookD-v1.0.ps1 - P0-T003 Desktop + Viewport Spike
 independent verification runbook (Implementation Brief
 BIM-TASK-P0-T003-CLAUDE v1.0 section 24 - "Verification Runbook D").

 AA Source Review Round 1 finding M06 correction: the prior version of
 this script ran 34 checks but did not map cleanly onto Brief section 24's
 25 enumerated items - several were missing entirely (exact task branch;
 expected pre-verification HEAD; exact 59-path footprint diff instead of a
 "something changed" check; frozen vcpkg baseline identity; exact
 toolchain/compiler/CMake/Ninja/qtbase/bgfx version checks; bgfx
 multithreaded-feature-absence; format job; static-analysis job; P0-T001/
 P0-T002 regression tests exact-name anchored; the evidence job driving
 the new JSON schema instead of the old plain-text report). This revision
 is organized as 25 numbered check GROUPS, one per Brief section 24 item,
 each producing one or more of the underlying numbered Invoke-Check
 entries (so "check N" in the final tally and "Brief section 24 item M"
 are both traceable, but are not required to be a 1:1 numbering - several
 items need more than one concrete check to verify honestly).

 UNVERIFIED / NEVER EXECUTED IN THIS SESSION - this is the single most
 important disclosure attached to this file. Claude, the Implementation
 Engineer, authored every check below against the CTest test names,
 target names, file paths, and toolchain identities this same authoring
 pass (and the Implementation Brief / Implementation Authorization
 documents) established - but this script itself has no build/execution
 channel available to it in this session (no device_bash-equivalent tool;
 the task worktree's toolchain was never invoked). Every PASS/FAIL this
 script would report is hypothetical until the Operator actually runs it.
 See docs/evidence/P0-T003/CLAUDE_HANDOVER.md for the full disclosure.

 Every check is independent: a failure in one does not stop later checks
 from running (unlike scripts/ci/architecture.ps1, which is intentionally
 fail-fast) - this script's job is to produce a COMPLETE picture across
 every check in a single run, then report a final verdict. Exit code 0
 only if every check passed.

 -ExpectedPreVerificationHead is REQUIRED (Brief section 24 item 2:
 "expected pre-verification HEAD supplied by Architecture Authority") -
 this script deliberately does not default it, since inventing a default
 would defeat the point of an Architecture-Authority-supplied value.

 Usage: powershell -File Verification-RunbookD-v1.0.ps1 -ExpectedPreVerificationHead <sha> [-BuildDir <path>] [-Triplet <triplet>]
==============================================================================
#>

param(
    [Parameter(Mandatory)][string]$ExpectedPreVerificationHead,
    [string]$BuildDir,
    [string]$Triplet = 'x64-windows',
    [string]$TaskBranch = 'task/P0-T003-desktop-viewport-spike',
    [string]$MainBaseline = '3f2230be5fcd796c370f485975547112ad52d2e3',
    [string]$VcpkgBaseline = 'f89a4a1da4e3176a8d1a14c1825b9b2f98e48843',
    [string]$ExpectedCMakeVersion = '4.4.2',
    [string]$ExpectedNinjaVersion = '1.12.1',
    # AA Source Review Round 2 finding B02: historically validated Ninja
    # path (Implementation Authorization section 4) - fallback resolution
    # target when `ninja` is not on PATH. Kept in its 8.3 short-path form
    # exactly as the Implementation Authorization document records it.
    [string]$HistoricalNinjaPath = 'C:\PROGRA~2\MICROS~4\2022\BUILDT~1\Common7\IDE\COMMON~1\MICROS~1\CMake\Ninja\ninja.exe',
    [string]$ExpectedVsBuildToolsVersion = '17.14.37614.0',
    [string]$ExpectedVcToolsVersion = '14.44.35207',
    # AA Runbook D Attempt 1 correction (RD1-03): the authoritative fresh
    # configure's own generated CMakeFiles/<cmake-version>/
    # CMakeCXXCompiler.cmake recorded this exact CMAKE_CXX_COMPILER_VERSION
    # for the resolved cl.exe - item 6's "effective compiler" check below
    # now fails closed on this exact value in addition to the resolved
    # compiler path and CMAKE_CXX_COMPILER_ID.
    [string]$ExpectedMsvcCompilerVersion = '19.44.35228.0',
    # AA Source Review Round 5 finding N05: the accepted exact VS Build
    # Tools 2022 installation root - item 5 below now requires vswhere's
    # resolved installationPath to match this exactly, in addition to its
    # pre-existing exact installationVersion check, so a second/decoy VS
    # installation at a different path cannot silently satisfy the version
    # check alone.
    [string]$ExpectedVsInstallationPath = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools',
    # AA Source Review Round 3 finding B03: the accepted bundled vcpkg root
    # (Implementation Authorization's accepted C0 configuration) - this
    # script forces $env:VCPKG_ROOT to exactly this path before any fresh
    # configure/build, rather than trusting whatever VCPKG_ROOT (if any)
    # happens to already be set in the caller's own shell.
    [string]$AcceptedVcpkgRoot = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\vcpkg',
    # AA Source Review Round 3 finding M15: exact clang-format/clang-tidy
    # version required - item 9 (moved earlier in this script's execution
    # order per B03) now enforces this in addition to mere PATH presence.
    [string]$ExpectedClangToolsVersion = '19.1.5',
    # Kept as accepted parameters for CLI/back-compat, but AA Source Review
    # Round 2 finding M12 superseded their actual use in the item 11+12
    # check below with the exact-token $ExpectedQtBasePortVersion /
    # $ExpectedBgfxPortVersion params (a bare version substring match was
    # too loose - see that check's comment).
    [string]$ExpectedQtBaseVersion = '6.11.1',
    [string]$ExpectedBgfxVersion = '1.129.8940-496',
    # AA Source Review Round 2 finding M12: exact resolved
    # "<version>#<portversion>" tokens (Brief section 2's frozen pins),
    # distinct from the bare version-substring params above which Round 1
    # checked too loosely (a substring match also accepts a stale/mismatched
    # portversion).
    [string]$ExpectedQtBasePortVersion = '6.11.1#1',
    [string]$ExpectedBgfxPortVersion = '1.129.8940-496#1',
    # AA Source Review Round 2 finding M12: the CMake preset (see this
    # repo's own CMakePresets.json) that item 16's fresh-configure-and-build
    # step drives directly, rather than this script inventing its own
    # separate -S/-B/-G/toolchain flags that could silently drift from the
    # presets the rest of this repo actually ships. Its CMakePresets.json
    # binaryDir ("${sourceDir}/build/<ConfigurePreset>") must match -BuildDir
    # - see that check's own validation.
    [string]$ConfigurePreset = 'ci-win-msvc'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'scripts\ci\_common.ps1')

$RepoRoot = Resolve-Path $PSScriptRoot
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc'
}

# Brief section 27 / Implementation Authorization section 8 - the exact 59
# -path footprint, embedded literally here (not merely referenced) so
# Brief section 24 item 3 ("exact authorized implementation footprint")
# can be verified mechanically rather than deferred to the Operator's own
# manual comparison, as the prior version of this check did.
$AuthorizedFootprint = @(
    'CMakeLists.txt'
    'vcpkg.json'
    'LICENSES.md'
    'src/desktop/README.md'
    'src/viewport/README.md'
    'tests/unit/CMakeLists.txt'
    'tests/integration/CMakeLists.txt'
    'tests/architecture/CMakeLists.txt'
    'tools/architecture_checker.cmake'
    'scripts/ci/architecture.ps1'
    'scripts/ci/license-inventory.ps1'
    'third_party/licenses/README.md'
    'third_party/licenses/bgfx.LICENSE.txt'
    'third_party/licenses/qtbase.LICENSE.txt'
    'src/viewport/CMakeLists.txt'
    'src/viewport/include/bim/viewport/error.hpp'
    'src/viewport/include/bim/viewport/math.hpp'
    'src/viewport/include/bim/viewport/mesh.hpp'
    'src/viewport/include/bim/viewport/camera.hpp'
    'src/viewport/include/bim/viewport/ray.hpp'
    'src/viewport/include/bim/viewport/input.hpp'
    'src/viewport/include/bim/viewport/lifecycle.hpp'
    'src/viewport/src/mesh.cpp'
    'src/viewport/src/camera.cpp'
    'src/viewport/src/ray.cpp'
    'src/viewport/src/lifecycle.cpp'
    'src/viewport/bgfx/CMakeLists.txt'
    'src/viewport/bgfx/include/bim/viewport_bgfx/renderer.hpp'
    'src/viewport/bgfx/src/renderer.cpp'
    'src/viewport/bgfx/src/renderer_impl.hpp'
    'src/viewport/bgfx/shaders/varying.def.sc'
    'src/viewport/bgfx/shaders/vs_p0_t003.sc'
    'src/viewport/bgfx/shaders/fs_p0_t003.sc'
    'src/desktop/CMakeLists.txt'
    'src/desktop/src/main.cpp'
    'src/desktop/src/main_window.hpp'
    'src/desktop/src/main_window.cpp'
    'src/desktop/src/viewport_window.hpp'
    'src/desktop/src/viewport_window.cpp'
    'src/desktop/src/viewport_bridge.hpp'
    'src/desktop/src/viewport_bridge.cpp'
    'src/desktop/src/spike_scene.hpp'
    'src/desktop/src/spike_scene.cpp'
    'src/desktop/src/evidence_mode.hpp'
    'src/desktop/src/evidence_mode.cpp'
    'tests/unit/unit_viewport_mesh_contract.cpp'
    'tests/unit/unit_viewport_camera.cpp'
    'tests/unit/unit_viewport_ray.cpp'
    'tests/unit/unit_viewport_lifecycle.cpp'
    'tests/integration/integration_viewport_bgfx_headless.cpp'
    'tests/integration/integration_viewport_bgfx_resource_lifecycle.cpp'
    'tests/fixtures/p0_t003_bad_viewport_api/viewport/include/bim/viewport/leaky.hpp'
    'tests/fixtures/p0_t003_bad_qt_owner/model/src/leaky_qt.cpp'
    'tests/fixtures/p0_t003_bad_bgfx_owner/desktop/src/leaky_bgfx.cpp'
    'tests/fixtures/p0_t003_bad_direct_d3d/desktop/src/leaky_d3d.cpp'
    'scripts/ci/viewport-spike.ps1'
    'Verification-RunbookD-v1.0.ps1'
    'docs/evidence/P0-T003/CLAUDE_HANDOVER.md'
    'docs/evidence/P0-T003/CLAUDE_HANDOVER.json'
)
if ($AuthorizedFootprint.Count -ne 59) {
    throw "internal error: `$AuthorizedFootprint has $($AuthorizedFootprint.Count) entries, expected exactly 59 - this script's own embedded footprint list is corrupt, fix before trusting any other result."
}

# Brief section 24 items 21/22 - the full P0-T001 and P0-T002 regression
# test names, exact-name anchored (tests/unit/CMakeLists.txt,
# tests/integration/CMakeLists.txt, tests/architecture/CMakeLists.txt are
# the source of truth this list was read from).
$P0T001Tests = @(
    'unit_foundation_smoke'
    'unit_model_links_foundation'
    'integration_geometry_occt_probe'
    'integration_persistence_sqlite_memory'
    'arch_repository_boundaries'
    'arch_checker_detects_violation'
)
$P0T002Tests = @(
    'unit_geometry_api_contract'
    'integration_geometry_occt_primitive'
    'integration_geometry_occt_opening_cut'
    'integration_geometry_occt_join'
    'integration_geometry_occt_failure_corpus'
    'integration_geometry_occt_tolerance_matrix'
    'integration_geometry_occt_coordinate_matrix'
    'integration_geometry_occt_history'
    'integration_geometry_occt_repeatability'
    'integration_geometry_occt_evidence'
    'arch_geometry_api_no_occt_leak'
    'arch_geometry_occt_only_kernel_owner'
)
$P0T003Tests = @(
    'unit_viewport_mesh_contract', 'unit_viewport_camera', 'unit_viewport_ray', 'unit_viewport_lifecycle'
    'integration_viewport_bgfx_headless', 'integration_viewport_bgfx_resource_lifecycle'
    'arch_viewport_public_neutral', 'arch_p0_t003_viewport_fixture_rejected'
    'arch_qt_desktop_only', 'arch_p0_t003_qt_fixture_rejected'
    'arch_bgfx_viewport_owner', 'arch_p0_t003_bgfx_fixture_rejected'
    'arch_no_direct_d3d', 'arch_p0_t003_d3d_fixture_rejected'
)

$script:Checks = [System.Collections.Generic.List[PSCustomObject]]::new()
$script:CheckNumber = 0
# AA RD1.8 Amendment A3 (final tally/verdict correction): this frozen
# candidate's Runbook check inventory is fixed at exactly 63 total checks
# (Brief section 24's 25 items, several combined into one check below and
# several expanded per-test via foreach over the fixed $P0T00xTests arrays
# above - see the "coveredItems" cross-check the verdict section prints).
# The final verdict below requires the run's actual total to equal this
# exact expected count before a global PASS is even considered; this does
# not change the meaning or scope of any individual check.
$script:ExpectedTotalCheckCount = 63
# AA Source Review Round 2 finding M12: set by item 16's fresh-configure-
# -and-build check, immediately before it drives cmake --preset / cmake
# --build; Assert-FileFresh (below) uses it to reject any artifact older
# than this run's own build. $null here means "no fresh build has run yet
# in this invocation" - Assert-FileFresh treats that as an internal-error
# condition (it must never be called before item 16's build check runs).
$script:PreBuildTimestampUtc = $null

function Invoke-Check {
    param(
        [Parameter(Mandatory)][string]$Name,
        [Parameter(Mandatory)][string]$BriefSection24Item,
        [Parameter(Mandatory)][scriptblock]$Body
    )
    $script:CheckNumber++
    $n = $script:CheckNumber
    Write-CiSection "Check $n (Brief 24.$BriefSection24Item) - $Name"
    try {
        & $Body
        Write-Host "Check $n PASSED: $Name" -ForegroundColor Green
        $script:Checks.Add([PSCustomObject]@{ Number = $n; Item = $BriefSection24Item; Name = $Name; Status = 'PASS'; Detail = '' })
    } catch {
        Write-Host "Check $n FAILED: $Name -- $($_.Exception.Message)" -ForegroundColor Red
        $script:Checks.Add([PSCustomObject]@{ Number = $n; Item = $BriefSection24Item; Name = $Name; Status = 'FAIL'; Detail = $_.Exception.Message })
    }
}

function Assert-FileExists {
    param([Parameter(Mandatory)][string]$Path, [string]$What = 'file')
    if (-not (Test-Path $Path)) {
        throw "Expected $What not found: '$Path'"
    }
}

# AA Source Review Round 2 finding M12: item 16's artifact-existence checks
# must not accept a stale artifact left over from an earlier, unrelated
# build - each build artifact checked after the fresh
# configure-and-build step (see that check) must be newer than
# $script:PreBuildTimestampUtc, the moment this run's fresh build actually
# started.
function Assert-FileFresh {
    param([Parameter(Mandatory)][string]$Path, [string]$What = 'file')
    Assert-FileExists -Path $Path -What $What
    if (-not $script:PreBuildTimestampUtc) {
        throw "internal error: Assert-FileFresh called for '$What' before `$script:PreBuildTimestampUtc was set - the fresh-configure-and-build check must run first."
    }
    $lastWriteUtc = (Get-Item -LiteralPath $Path).LastWriteTimeUtc
    if ($lastWriteUtc -lt $script:PreBuildTimestampUtc) {
        throw "$What at '$Path' was last written $lastWriteUtc (UTC), before this run's fresh build started at $($script:PreBuildTimestampUtc) (UTC) - this is a stale artifact left over from a previous build, not evidence this run's build actually produced it."
    }
}

# AA Runbook D Attempt 1 correction (RD1-04): parses vcpkg's own installed
# -package status database (<install-root>/vcpkg/status) directly, instead
# of trusting `vcpkg list`'s plain-text output for per-feature information.
# The authoritative first attempt's diagnostic traced items 13/14's failure
# to two separate problems: (1) `vcpkg list`'s default one-line-per-package
# output does not print installed FEATURE names at all (only
# name/triplet/version/short description), so the prior "multithreaded"/
# "widgets" substring checks against that output could never have matched
# even with a correct install root; and (2) items 11-14 all resolved
# vcpkg.exe via $env:VCPKG_ROOT / a PATH fallback rather than this
# Runbook's own bundled/accepted vcpkg root, which could pick a different
# vcpkg.exe than the one that actually produced this build's
# vcpkg_installed tree.
#
# vcpkg's status file is a Debian-control-file-style stanza list (stanzas
# separated by a blank line; "Field: value" lines within each) - the base
# package gets one stanza (no Feature: field) and each installed OPTIONAL
# feature gets its own additional stanza with the same Package: name plus
# a Feature: field, exactly as Brief/RD1-04 describes ("bgfx must include
# Feature: tools"). A stanza only counts as actually installed when its
# Status: field contains vcpkg/dpkg's canonical fully-installed phrase
# "install ok installed" - a half-installed/removed/error stanza must not
# count.
#
# UNVERIFIED: this exact stanza shape (blank-line-separated Debian-control
# -file stanzas, one addition stanza per feature, "install ok installed"
# as the fully-installed Status value) is vcpkg's own long-documented
# status-file format, not something this task authored, but it was not
# confirmed against a real generated status file in this session (no
# execution channel).
function Get-VcpkgStatusStanzas {
    param([Parameter(Mandatory)][string]$StatusFilePath)
    Assert-FileExists $StatusFilePath 'vcpkg_installed/vcpkg/status (explicit install root - RD1-04; a missing status file must fail this check, not be treated as "no packages installed yet is fine")'
    $raw = Get-Content $StatusFilePath -Raw
    $normalized = $raw -replace "`r`n", "`n"
    $stanzaTexts = @($normalized -split "`n`n" | Where-Object { $_.Trim() -ne '' })
    $stanzas = @()
    foreach ($stanzaText in $stanzaTexts) {
        $fields = @{}
        foreach ($line in ($stanzaText -split "`n")) {
            if ($line -match '^([A-Za-z-]+):\s*(.*)$') {
                $fields[$Matches[1]] = $Matches[2].Trim()
            }
        }
        if ($fields.Count -gt 0) {
            $stanzas += [PSCustomObject]$fields
        }
    }
    return $stanzas
}

# Returns $true only if $Stanzas contains a stanza for $PackageName (at
# $Triplet) whose Status is fully-installed - the base package stanza when
# $Feature is empty/omitted, or that exact feature's own stanza otherwise.
# Never returns $true on an empty/absent match - this is the fail-closed
# behavior RD1-04 requires ("never let an empty package query make the
# 'multithreaded absent' test pass vacuously": callers must check package
# presence explicitly before trusting a feature's absence as meaningful).
function Test-VcpkgPackageInstalled {
    param(
        [Parameter(Mandatory)]$Stanzas,
        [Parameter(Mandatory)][string]$PackageName,
        [string]$Triplet = 'x64-windows',
        [string]$Feature = ''
    )
    foreach ($s in $Stanzas) {
        if (-not $s.PSObject.Properties['Package'] -or $s.Package -ne $PackageName) { continue }
        if (-not $s.PSObject.Properties['Architecture'] -or $s.Architecture -ne $Triplet) { continue }
        $stanzaFeature = if ($s.PSObject.Properties['Feature']) { $s.Feature } else { '' }
        if ($stanzaFeature -ne $Feature) { continue }
        if ($s.PSObject.Properties['Status'] -and $s.Status -match 'install ok installed') {
            return $true
        }
    }
    return $false
}

# AA Source Review Round 2 finding B02: resolves a usable ninja.exe even
# when `ninja` is not on PATH - falls back to the historically validated
# path (Implementation Authorization section 4), then to the
# CMAKE_MAKE_PROGRAM already recorded in an existing build directory's
# CMakeCache.txt, rather than failing outright the moment PATH alone
# doesn't have it.
function Resolve-NinjaExecutable {
    param([string]$HistoricalPath, [string]$InBuildDir)
    $ninjaCmd = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninjaCmd) { return $ninjaCmd.Source }
    if ($HistoricalPath -and (Test-Path $HistoricalPath)) {
        return $HistoricalPath
    }
    if ($InBuildDir) {
        $cachePath = Join-Path $InBuildDir 'CMakeCache.txt'
        if (Test-Path $cachePath) {
            $cache = Get-Content $cachePath -Raw
            if ($cache -match 'CMAKE_MAKE_PROGRAM:FILEPATH=(.*ninja\.exe)') {
                $candidate = $Matches[1].Trim()
                if (Test-Path $candidate) { return $candidate }
            }
        }
    }
    return $null
}

# AA Source Review Round 3 finding B03: imports the x64 Visual Studio
# developer environment (PATH/INCLUDE/LIB/LIBPATH/VCToolsVersion/etc.) into
# THIS PowerShell process's own $env: scope, by shelling out to
# vcvarsall.bat and capturing its resulting `set` output - rather than this
# script relying on whatever toolchain state (if any) the caller's own shell
# already happened to have before invoking this script.
function Import-VsDevEnvironment {
    param([Parameter(Mandatory)][string]$VcvarsallPath)
    if (-not (Test-Path $VcvarsallPath)) {
        throw "vcvarsall.bat not found at '$VcvarsallPath' - cannot import the x64 VS developer environment"
    }
    $cmdOutput = & cmd.exe /c "`"$VcvarsallPath`" x64 && set"
    if ($LASTEXITCODE -ne 0) {
        throw "vcvarsall.bat x64 exited with code $LASTEXITCODE - the x64 VS developer environment could not be imported"
    }
    $importedCount = 0
    foreach ($line in $cmdOutput) {
        if ($line -match '^([^=]+)=(.*)$') {
            $name = $Matches[1]
            $value = $Matches[2]
            Set-Item -Path "Env:$name" -Value $value
            $importedCount++
        }
    }
    if ($importedCount -eq 0) {
        throw "vcvarsall.bat x64 produced no parseable KEY=VALUE environment output - import failed"
    }
    Write-Host "  imported $importedCount environment variable(s) from vcvarsall.bat x64"
}

function Invoke-ExactCTest {
    param([Parameter(Mandatory)][string]$TestName, [Parameter(Mandatory)][string]$InBuildDir)
    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }
    $exactPattern = '^' + $TestName + '$'
    Push-Location $InBuildDir
    try {
        Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('-R', $exactPattern, '--output-on-failure', '--no-tests=error') | Out-Null
    } finally {
        Pop-Location
    }
}

function Find-BuiltFile {
    param([string[]]$Candidates)
    foreach ($c in $Candidates) {
        if (Test-Path $c) { return $c }
    }
    return $null
}

function Find-SpikeExe {
    param([Parameter(Mandatory)][string]$InBuildDir)
    return Find-BuiltFile @(
        (Join-Path $InBuildDir 'src\desktop\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'src\desktop\Release\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'src\desktop\Debug\bim_desktop_spike.exe'),
        (Join-Path $InBuildDir 'bim_desktop_spike.exe')
    )
}

# AA RD1.7 (Full Runbook D Attempt 2, Checks 35/60 - Qt platform plugin CI
# wiring): Attempt 2 failed before any evidence executed with 'Could not
# find the Qt platform plugin "windows"'. The authoritative diagnostic
# proved qwindows.dll exists under the build's own vcpkg_installed tree
# (<BuildDir>\vcpkg_installed\<triplet>\Qt6\plugins\platforms\qwindows.dll)
# but nothing told Qt where to look: QT_PLUGIN_PATH and
# QT_QPA_PLATFORM_PLUGIN_PATH were both empty and no platforms\qwindows.dll
# was staged beside the executable. The two helpers below resolve the
# authoritative plugin root from the CURRENT $BuildDir (never a hard-coded
# worktree path), fail closed if qwindows.dll is missing, and hand child
# desktop evidence processes a PROCESS-LOCAL Qt plugin environment for the
# duration of one invocation only - restored afterward, never written to the
# user/system environment. CI/runtime wiring only: no CMake deployment
# redesign in RD1.7.
function Resolve-QtPluginRoot {
    param([Parameter(Mandatory)][string]$InBuildDir, [Parameter(Mandatory)][string]$ForTriplet)
    $pluginRoot = Join-Path $InBuildDir (Join-Path 'vcpkg_installed' (Join-Path $ForTriplet (Join-Path 'Qt6' 'plugins')))
    $qwindows = Join-Path $pluginRoot (Join-Path 'platforms' 'qwindows.dll')
    if (-not (Test-Path -LiteralPath $qwindows -PathType Leaf)) {
        throw "Qt platform plugin not found at '$qwindows' (expected under the build's own vcpkg_installed tree, derived from -BuildDir '$InBuildDir' / triplet '$ForTriplet'). Without it QApplication cannot start ('Could not find the Qt platform plugin ""windows""' - Full Runbook D Attempt 2, Checks 35/60). Fail-closed: not guessing an alternate location."
    }
    return $pluginRoot
}

function Invoke-WithQtPluginEnvironment {
    param(
        [Parameter(Mandatory)][string]$InBuildDir,
        [Parameter(Mandatory)][string]$ForTriplet,
        [Parameter(Mandatory)][scriptblock]$Body
    )
    $pluginRoot = Resolve-QtPluginRoot -InBuildDir $InBuildDir -ForTriplet $ForTriplet
    $platformsDir = Join-Path $pluginRoot 'platforms'
    # Save whatever the caller's process already had (may be unset) so it
    # can be restored exactly - this script continues running many checks
    # after each evidence invocation.
    $previous = @{
        'QT_PLUGIN_PATH'              = [Environment]::GetEnvironmentVariable('QT_PLUGIN_PATH', 'Process')
        'QT_QPA_PLATFORM_PLUGIN_PATH' = [Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM_PLUGIN_PATH', 'Process')
        'QT_QPA_PLATFORM'             = [Environment]::GetEnvironmentVariable('QT_QPA_PLATFORM', 'Process')
    }
    try {
        # 'Process' scope only: inherited by child processes launched from
        # here, invisible to the user/system environment.
        [Environment]::SetEnvironmentVariable('QT_PLUGIN_PATH', $pluginRoot, 'Process')
        [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM_PLUGIN_PATH', $platformsDir, 'Process')
        [Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM', 'windows', 'Process')
        Write-Host "  Qt plugin environment (process-local): QT_PLUGIN_PATH=$pluginRoot; QT_QPA_PLATFORM_PLUGIN_PATH=$platformsDir; QT_QPA_PLATFORM=windows"
        & $Body
    } finally {
        foreach ($name in $previous.Keys) {
            # $null restores "unset"; a previous value restores that value.
            [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process')
        }
    }
}

# ==============================================================================
# Brief section 24, item 1: exact P0-T003 task branch.
# ==============================================================================
Invoke-Check "git branch is exactly '$TaskBranch'" '1' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $branch = (& $gitCmd.Source rev-parse --abbrev-ref HEAD).Trim()
    } finally {
        Pop-Location
    }
    if ($branch -ne $TaskBranch) {
        throw "current branch is '$branch', expected exactly '$TaskBranch'"
    }
}

# ==============================================================================
# Brief section 24, item 2: expected pre-verification HEAD supplied by
# Architecture Authority.
# ==============================================================================
Invoke-Check "git HEAD matches -ExpectedPreVerificationHead ($ExpectedPreVerificationHead)" '2' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $head = (& $gitCmd.Source rev-parse HEAD).Trim()
    } finally {
        Pop-Location
    }
    if ($head -ne $ExpectedPreVerificationHead) {
        throw "HEAD is '$head', expected exactly the Architecture-Authority-supplied '$ExpectedPreVerificationHead'"
    }
}

# ==============================================================================
# Brief section 24, item 3: exact authorized implementation footprint (all
# 59 paths, no more, no fewer) - a real Compare-Object against `git status
# --porcelain`, not merely "something changed" (AA Source Review Round 1
# finding M06: the prior version explicitly deferred this comparison to
# the Operator).
# ==============================================================================
Invoke-Check 'git-reported changed paths equal exactly the authorized 59-path footprint' '3' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        # AA Runbook D Attempt 1 correction (RD1-02): `git status --porcelain`
        # collapses an entirely-untracked directory into a single
        # "?? <dir>/" entry instead of recursing into it - this is git's
        # own documented porcelain behavior, not a bug in the prior version
        # of this check, but it meant a brand-new directory such as
        # src/desktop/src/ was reported as ONE changed path (the directory
        # itself) rather than as every individual new file beneath it, so
        # this check's Compare-Object-style diff against $AuthorizedFootprint
        # (which lists individual files) saw every one of those files as
        # "missing" and falsely flagged the collapsed parent directory
        # itself as an out-of-scope path. Replaced with deterministic
        # per-file enumeration: `git diff --name-only HEAD --` (tracked
        # files modified relative to HEAD) unioned with
        # `git ls-files --others --exclude-standard --` (untracked new
        # files - ls-files, unlike status --porcelain, always lists
        # individual files and never a collapsed containing directory).
        $modifiedTracked = @(& $gitCmd.Source diff --name-only HEAD --)
        if ($LASTEXITCODE -ne 0) {
            throw "git diff --name-only HEAD -- exited $LASTEXITCODE"
        }
        $untracked = @(& $gitCmd.Source ls-files --others --exclude-standard --)
        if ($LASTEXITCODE -ne 0) {
            throw "git ls-files --others --exclude-standard -- exited $LASTEXITCODE"
        }
    } finally {
        Pop-Location
    }
    # git always emits forward-slash-separated paths for both commands
    # above, on Windows included; the backslash replacement below is a
    # defensive normalization only, not evidence either command actually
    # needs it.
    $changedPaths = @(($modifiedTracked + $untracked) | Where-Object { $_ -and $_.Trim() -ne '' } | ForEach-Object {
        ($_.Trim() -replace '\\', '/')
    } | Sort-Object -Unique)
    $extra = @($changedPaths | Where-Object { $_ -notin $AuthorizedFootprint })
    $missing = @($AuthorizedFootprint | Where-Object { $_ -notin $changedPaths })
    if ($extra.Count -gt 0) {
        throw "changed path(s) outside the authorized 59-path footprint: $($extra -join ', ')"
    }
    if ($missing.Count -gt 0) {
        throw "authorized path(s) with no recorded change (git diff/ls-files silent on them): $($missing -join ', ')"
    }
    Write-Host "  all $($changedPaths.Count) changed paths are exactly the authorized 59-path footprint."
}

# ==============================================================================
# Brief section 24, item 4: main baseline unchanged and clean.
# ==============================================================================
Invoke-Check "main branch ref matches the frozen main baseline ($MainBaseline) and working tree outside the footprint is clean" '4' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $mainRef = $null
        foreach ($candidate in @('main', 'origin/main', 'master', 'origin/master')) {
            $rev = & $gitCmd.Source rev-parse --verify --quiet $candidate 2>$null
            if ($LASTEXITCODE -eq 0 -and $rev) { $mainRef = $rev.Trim(); break }
        }
        if (-not $mainRef) { throw 'no main/master ref could be resolved (checked main, origin/main, master, origin/master)' }

        # AA Source Review Round 2 finding M12: also verify the real main
        # worktree - if one is separately checked out anywhere - is clean
        # and unchanged, not merely that the ref `main` resolves to matches
        # the frozen baseline (a resolvable ref says nothing about whether a
        # second, separately checked-out working tree for that branch has
        # local modifications). 03-IMPLEMENTATION-AUTHORIZATION.md names
        # only one authorized worktree for this task
        # (D:\Projects\BIM-Platform-WT-P0-T003) and no separate literal
        # main-worktree path, so this is deliberately discovery-based via
        # `git worktree list` rather than a hard-coded second path this
        # script cannot confirm exists in every Operator environment - a
        # no-op (informational only) when no such separate worktree exists,
        # rather than fabricating a check against a directory that may not
        # be real.
        $worktreeLines = & $gitCmd.Source worktree list --porcelain
        $mainWorktreePaths = New-Object System.Collections.Generic.List[string]
        $currentWorktreePath = $null
        foreach ($line in $worktreeLines) {
            if ($line -match '^worktree (.+)$') {
                $currentWorktreePath = $Matches[1].Trim()
            } elseif ($line -match '^branch (.+)$') {
                $branchRef = ($Matches[1].Trim()) -replace '^refs/heads/', ''
                if (($branchRef -eq 'main' -or $branchRef -eq 'master') -and $currentWorktreePath) {
                    $mainWorktreePaths.Add($currentWorktreePath)
                }
            }
        }
        if ($mainWorktreePaths.Count -eq 0) {
            Write-Host '  no separately checked-out main/master worktree was found (only ref resolution above applies) - informational, not a failure.'
        }
        foreach ($mainWorktreePath in $mainWorktreePaths) {
            if (-not (Test-Path $mainWorktreePath)) {
                Write-Host "  NOTE: git worktree list reports a main/master worktree at '$mainWorktreePath', but that path is not visible from this script's vantage point - skipping (cannot verify remotely)." -ForegroundColor Yellow
                continue
            }
            Push-Location $mainWorktreePath
            try {
                $mainWorktreeHead = (& $gitCmd.Source rev-parse HEAD).Trim()
                $mainWorktreeStatusRaw = & $gitCmd.Source status --porcelain
            } finally {
                Pop-Location
            }
            if ($mainWorktreeHead -ne $MainBaseline) {
                throw "separately checked-out main worktree at '$mainWorktreePath' is at HEAD '$mainWorktreeHead', expected the frozen main baseline '$MainBaseline'"
            }
            $mainWorktreeChanges = @($mainWorktreeStatusRaw | Where-Object { $_ -and $_.Trim() -ne '' })
            if ($mainWorktreeChanges.Count -gt 0) {
                throw "separately checked-out main worktree at '$mainWorktreePath' is not clean: $($mainWorktreeChanges -join '; ')"
            }
            Write-Host "  main worktree at '$mainWorktreePath' verified clean and at the frozen baseline."
        }
    } finally {
        Pop-Location
    }
    if ($mainRef -ne $MainBaseline) {
        throw "resolved main ref is '$mainRef', expected the frozen main baseline '$MainBaseline' - main must remain untouched"
    }
}

# ==============================================================================
# Brief section 24, item 5: x64 / VS 2022 17.14 / v143 environment.
# ==============================================================================
Invoke-Check "environment is x64 / VS Build Tools $ExpectedVsBuildToolsVersion / v143" '5' {
    if ($env:PROCESSOR_ARCHITECTURE -ne 'AMD64' -and $env:PROCESSOR_ARCHITECTURE -ne 'ARM64') {
        throw "PROCESSOR_ARCHITECTURE is '$env:PROCESSOR_ARCHITECTURE', expected an x64-capable host (AMD64/ARM64 with x64 target)"
    }
    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    $vswhere = $vswhereCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $vswhere) {
        throw 'vswhere.exe was not found - cannot verify installed Visual Studio Build Tools version/v143 toolset'
    }
    $installJson = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
    if (-not $installJson -or $installJson.Count -eq 0) {
        throw 'vswhere reported no Visual Studio installation with the v143 (VC.Tools.x86.x64) component'
    }
    $installVersion = $installJson[0].installationVersion
    # AA Source Review Round 4 MINOR hardening: the prior condition
    # (`-notlike "$ExpectedVsBuildToolsVersion*" -and -ne
    # $ExpectedVsBuildToolsVersion`) accepted any installationVersion
    # sharing the expected value as a mere PREFIX (e.g. a later servicing
    # release '17.14.37614.1' or an unrelated longer string starting with
    # the same digits) as a silent pass, not only the exact accepted
    # version. This is now an exact-only comparison.
    if ($installVersion -ne $ExpectedVsBuildToolsVersion) {
        # AA Source Review Round 2 finding M12: Round 1 only warned (Yellow
        # Write-Host) on a VS/VCTools version mismatch here, so this check
        # always PASSED regardless of the actual installed version - a
        # silent no-op check. A mismatch is now a hard failure, matching
        # every other exact-version check in this script (items 6, 7, 8,
        # 11+12).
        throw "vswhere reports installationVersion '$installVersion', expected exactly '$ExpectedVsBuildToolsVersion' - VS Build Tools/VCTools version mismatches are not an acceptable match and must fail this check, not merely warn."
    }

    # AA Source Review Round 5 finding N05: exact installationVersion alone
    # does not rule out a second/decoy Visual Studio installation on this
    # machine reporting the same version string from a different install
    # root. Require the resolved installationPath itself to match the one
    # accepted VS Build Tools 2022 root exactly (trailing separators
    # normalized away before comparing, since vswhere's own output and a
    # hand-typed expected path can differ only in that respect without
    # being a meaningfully different location).
    $installPathForVersionCheck = ($installJson[0].installationPath).TrimEnd('\', '/')
    $expectedInstallationPathTrimmed = $ExpectedVsInstallationPath.TrimEnd('\', '/')
    if (-not [string]::Equals($installPathForVersionCheck, $expectedInstallationPathTrimmed, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "vswhere reports installationPath '$($installJson[0].installationPath)', expected exactly '$ExpectedVsInstallationPath' - a Visual Studio installation at an unexpected path is not an acceptable match even if its installationVersion happens to match."
    }
}

# ==============================================================================
# Brief section 24, item 5 (B03 addendum): before any fresh configure/build,
# establish and verify the accepted PROCESS-LOCAL Windows toolchain
# environment - Visual Studio Build Tools $ExpectedVsBuildToolsVersion
# (already verified above), VCTools $ExpectedVcToolsVersion, x64 cl.exe,
# the accepted bundled $AcceptedVcpkgRoot, and Ninja $ExpectedNinjaVersion
# at the accepted C0 path ($HistoricalNinjaPath) - rather than relying on
# the caller's own PATH/VCPKG_ROOT/CXX state (AA Source Review Round 3
# finding B03). This check, and items 7/8/9 immediately after it (moved
# earlier in this script's execution order for the same reason), are
# deliberately every tool-identity check that CAN run before a configure
# exists - so all four now run before item 16's fresh configure/build
# further below, not after it.
# ==============================================================================
Invoke-Check "process-local x64 VS $ExpectedVsBuildToolsVersion / VCTools $ExpectedVcToolsVersion / vcpkg / Ninja $ExpectedNinjaVersion environment established" '5' {
    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    $vswhere = $vswhereCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $vswhere) {
        throw 'vswhere.exe was not found - cannot locate vcvarsall.bat to establish the x64 VS developer environment'
    }
    $installPath = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if (-not $installPath) {
        throw 'vswhere reported no Visual Studio installation with the v143 (VC.Tools.x86.x64) component - cannot establish the environment'
    }
    $vcvarsallPath = Join-Path $installPath 'VC\Auxiliary\Build\vcvarsall.bat'

    # Import the x64 VS developer environment into THIS process, not the
    # caller's - B03 explicitly requires not relying on the caller's own
    # PATH/VCPKG_ROOT/CXX state.
    Import-VsDevEnvironment -VcvarsallPath $vcvarsallPath

    # Force/verify the accepted bundled vcpkg root, rather than trusting
    # whatever VCPKG_ROOT (if any) vcvarsall.bat or the caller's shell left
    # in place.
    if (-not (Test-Path $AcceptedVcpkgRoot)) {
        throw "accepted bundled VCPKG_ROOT '$AcceptedVcpkgRoot' does not exist on this machine"
    }
    $env:VCPKG_ROOT = $AcceptedVcpkgRoot
    Write-Host "  VCPKG_ROOT forced to accepted bundled path: $AcceptedVcpkgRoot"

    # Resolve Ninja specifically at the accepted C0 path - deliberately NOT
    # via Resolve-NinjaExecutable's lenient PATH-first fallback chain (used
    # by item 8 below), since B03 requires establishing the environment
    # from the accepted path, not trusting whatever the caller's PATH
    # already happens to resolve.
    if (-not (Test-Path $HistoricalNinjaPath)) {
        throw "accepted Ninja path '$HistoricalNinjaPath' does not exist on this machine - cannot establish the process-local toolchain environment"
    }
    $ninjaVersion = (& $HistoricalNinjaPath --version).Trim()
    if ($ninjaVersion -ne $ExpectedNinjaVersion) {
        throw "Ninja at the accepted path '$HistoricalNinjaPath' reports version '$ninjaVersion', expected exactly '$ExpectedNinjaVersion'"
    }
    $ninjaDir = Split-Path $HistoricalNinjaPath -Parent
    $env:PATH = "$ninjaDir;$env:PATH"
    Write-Host "  Ninja $ExpectedNinjaVersion verified and prepended to process PATH: $ninjaDir"

    # Force/verify the accepted x64 cl.exe selection.
    $clCmd = Get-Command cl -ErrorAction SilentlyContinue
    if (-not $clCmd) {
        throw 'cl.exe does not resolve on PATH after importing the x64 VS developer environment'
    }
    if ($clCmd.Source -notmatch [regex]::Escape($ExpectedVcToolsVersion)) {
        throw "cl.exe resolved to '$($clCmd.Source)', which does not contain the expected VCTools version '$ExpectedVcToolsVersion'"
    }
    $env:CC = $clCmd.Source
    $env:CXX = $clCmd.Source
    Write-Host "  cl.exe verified and CC/CXX forced to: $($clCmd.Source)"
}

# ==============================================================================
# Brief section 24, item 7: CMake 4.4.2.
# ==============================================================================
Invoke-Check "CMake version is exactly $ExpectedCMakeVersion" '7' {
    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCmd) { throw 'cmake was not found on PATH.' }
    $versionOutput = (& $cmakeCmd.Source --version) -join ' '
    # AA Source Review Round 5 finding N04: the prior check
    # (`-notmatch "cmake version $([regex]::Escape($ExpectedCMakeVersion))"`)
    # was an UNANCHORED substring regex against the full --version output -
    # -match has no ^/$ anchoring here, so "cmake version 4.4.2" would also
    # be found (and this check would silently PASS) inside an output like
    # "cmake version 4.4.20" or "cmake version 4.4.2-rc1", neither of which
    # is actually version 4.4.2. This now parses the actual X.Y.Z
    # semantic-version token following "cmake version" and requires an
    # exact match against $ExpectedCMakeVersion, the same parse-then-compare
    # pattern already used for clang-format/clang-tidy (item 9, Round 4).
    if ($versionOutput -notmatch 'cmake version (\d+\.\d+\.\d+)') {
        throw "cmake --version reported '$versionOutput', which does not contain a parseable 'cmake version X.Y.Z' token"
    }
    $cmakeVersion = $Matches[1]
    if ($cmakeVersion -ne $ExpectedCMakeVersion) {
        throw "cmake --version reported version '$cmakeVersion' (from '$versionOutput'), expected exactly version '$ExpectedCMakeVersion'"
    }
}

# ==============================================================================
# Brief section 24, item 8: Ninja 1.12.1.
#
# AA Source Review Round 2 finding B02: Round 1 only looked on PATH and
# failed outright otherwise - but the accepted C0 configuration
# (03-IMPLEMENTATION-AUTHORIZATION.md section 4) resolves Ninja from a
# specific historically validated path bundled with VS Build Tools' CMake
# component, not necessarily from PATH. Resolve-NinjaExecutable now also
# tries that historical path, then an existing build directory's own
# CMakeCache.txt CMAKE_MAKE_PROGRAM, before giving up.
# ==============================================================================
Invoke-Check "Ninja version is exactly $ExpectedNinjaVersion (PATH, historically-validated path, or CMAKE_MAKE_PROGRAM)" '8' {
    $ninjaExe = Resolve-NinjaExecutable -HistoricalPath $HistoricalNinjaPath -InBuildDir $BuildDir
    if (-not $ninjaExe) {
        throw "ninja was not found on PATH, at the historically-validated path '$HistoricalNinjaPath', or via an existing build directory's CMakeCache.txt CMAKE_MAKE_PROGRAM"
    }
    $versionOutput = (& $ninjaExe --version).Trim()
    if ($versionOutput -ne $ExpectedNinjaVersion) {
        throw "ninja ('$ninjaExe') --version reported '$versionOutput', expected exactly '$ExpectedNinjaVersion'"
    }
    Write-Host "  resolved ninja: '$ninjaExe'"
}

# ==============================================================================
# Brief section 24, item 9: clang-format / clang-tidy presence and exact
# version.
#
# AA Source Review Round 3 finding M15: presence alone (Round 1/2's version
# of this check) is not enough - Runbook D must fail closed on a partially
# different toolchain, including a clang-format/clang-tidy version drift.
# ==============================================================================
Invoke-Check "clang-format and clang-tidy are present on PATH and exactly version $ExpectedClangToolsVersion" '9' {
    $clangFormatCmd = Get-Command clang-format -ErrorAction SilentlyContinue
    $clangTidyCmd = Get-Command clang-tidy -ErrorAction SilentlyContinue
    $missing = @()
    if (-not $clangFormatCmd) { $missing += 'clang-format' }
    if (-not $clangTidyCmd) { $missing += 'clang-tidy' }
    if ($missing.Count -gt 0) {
        throw "not found on PATH: $($missing -join ', ')"
    }
    # AA Source Review Round 4 MINOR hardening: the prior check only tested
    # whether the raw --version output CONTAINED the expected version as a
    # substring ([regex]::Escape($ExpectedClangToolsVersion) against the
    # whole output line) - which would also accept, e.g., an unrelated
    # longer version string that happens to embed '19.1.5' as a substring
    # (such as a hypothetical '19.1.50' or '119.1.5'), or a match against an
    # unrelated part of the banner text. This now parses the actual X.Y.Z
    # semantic version token out of the output and compares it exactly.
    $clangFormatVersionOutput = (& $clangFormatCmd.Source --version) -join ' '
    if ($clangFormatVersionOutput -notmatch '(\d+\.\d+\.\d+)') {
        throw "clang-format --version reported '$clangFormatVersionOutput', which does not contain a parseable X.Y.Z version number"
    }
    $clangFormatVersion = $Matches[1]
    if ($clangFormatVersion -ne $ExpectedClangToolsVersion) {
        throw "clang-format --version reported version '$clangFormatVersion' (from '$clangFormatVersionOutput'), expected exactly version '$ExpectedClangToolsVersion'"
    }
    $clangTidyVersionOutput = (& $clangTidyCmd.Source --version) -join ' '
    if ($clangTidyVersionOutput -notmatch '(\d+\.\d+\.\d+)') {
        throw "clang-tidy --version reported '$clangTidyVersionOutput', which does not contain a parseable X.Y.Z version number"
    }
    $clangTidyVersion = $Matches[1]
    if ($clangTidyVersion -ne $ExpectedClangToolsVersion) {
        throw "clang-tidy --version reported version '$clangTidyVersion' (from '$clangTidyVersionOutput'), expected exactly version '$ExpectedClangToolsVersion'"
    }
}

# ==============================================================================
# Brief section 24, item 16 (moved ahead of items 6-15 - see that item's own
# section further below for why): configure/build job's actual fresh
# configure+build.
#
# AA Source Review Round 2 finding M12: Round 1's version of item 16 only
# checked that a build directory / CMakeCache.txt / artifacts already
# existed somewhere - it never actually drove a configure or build itself,
# so a stale build directory left over from an earlier, unrelated run (or
# built against different sources entirely) could pass every check below
# it without this run ever having compiled anything. This wipes $BuildDir
# and drives a real 'cmake --preset <ConfigurePreset>' + 'cmake --build
# --preset <ConfigurePreset>' - the exact same CMake preset this repo's own
# CMakePresets.json ships and CMakeLists.txt's own in-source-build error
# message documents (see CMakePresets.json's "ci-win-msvc" preset, whose
# binaryDir "${sourceDir}/build/ci-win-msvc" is this script's own -BuildDir
# default) - rather than this script inventing a separate set of
# -S/-B/-G/toolchain flags that could silently drift from the presets the
# rest of this repo actually uses.
#
# This runs here, before item 6 (compiler identity) and items 11+12/13/14
# (resolved vcpkg package versions/features), because those checks read
# CMakeCache.txt / $BuildDir's vcpkg_installed tree and must be inspecting
# THIS run's own fresh configure, not whatever was already sitting in
# $BuildDir before this script started. Every artifact-existence check
# under item 16's own section further below also requires the artifact to
# be newer than the moment this build started (Assert-FileFresh), so a
# leftover artifact from a prior build cannot silently satisfy this run
# either.
#
# AA Source Review Round 3 findings B03/M15: this check now ALSO runs after
# item 5's new B03 environment-establishment check and items 7/8/9 (CMake
# version, Ninja version, clang-format/clang-tidy presence+version) - every
# tool-identity check that can run before a configure exists now does,
# including B03's process-local PATH/VCPKG_ROOT/CC/CXX environment, which
# this configure/build step depends on actually being established first.
# ==============================================================================

Invoke-Check "Build directory was wiped and freshly configured+built via 'cmake --preset $ConfigurePreset' / 'cmake --build --preset $ConfigurePreset'" '16' {
    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCmd) { throw 'cmake was not found on PATH.' }

    $expectedBinaryDir = (Join-Path $RepoRoot ('build\' + $ConfigurePreset)).TrimEnd('\', '/')
    $normalizedBuildDir = $BuildDir.TrimEnd('\', '/')
    if ($normalizedBuildDir -ne $expectedBinaryDir) {
        throw "-BuildDir ('$BuildDir') does not match -ConfigurePreset '$ConfigurePreset''s CMakePresets.json binaryDir ('$expectedBinaryDir') - this check drives the named CMake preset directly and requires -BuildDir to stay aligned with -ConfigurePreset rather than guessing a separate configure command for a mismatched pair. Pass a matching -BuildDir/-ConfigurePreset pair, or omit both to use the defaults."
    }

    if (Test-Path $BuildDir) {
        Remove-Item -LiteralPath $BuildDir -Recurse -Force
    }

    # Recorded strictly before the configure/build runs so every artifact
    # this build produces is provably newer than this timestamp -
    # Assert-FileFresh (below, and in item 16's own section further down)
    # rejects anything older.
    $script:PreBuildTimestampUtc = [DateTime]::UtcNow

    Push-Location $RepoRoot
    try {
        Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--preset', $ConfigurePreset) | Out-Null
        Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--build', '--preset', $ConfigurePreset) | Out-Null
    } finally {
        Pop-Location
    }

    Assert-FileExists (Join-Path $BuildDir 'CMakeCache.txt') 'CMakeCache.txt (the fresh configure above should have produced this)'
}

# ==============================================================================
# Brief section 24, item 6: effective compiler MSVC and matching validated
# cl.exe.
# ==============================================================================
Invoke-Check "effective compiler is MSVC $ExpectedVcToolsVersion / $ExpectedMsvcCompilerVersion (validated cl.exe, via CMake's own generated CMakeCXXCompiler.cmake)" '6' {
    # AA Runbook D Attempt 1 correction (RD1-03): do not require
    # CMAKE_CXX_COMPILER to exist in CMakeCache.txt - the authoritative
    # fresh configure did not record it there in a form this check's prior
    # regex could read, even though the configure genuinely succeeded and
    # genuinely resolved MSVC. CMake's own per-language compiler
    # -identification step always writes a small, generated
    # CMakeFiles/<cmake-version>/CMakeCXXCompiler.cmake file once C++ is
    # successfully enabled (project()/enable_language()), independent of
    # exactly which cache variables happen to be persisted to
    # CMakeCache.txt - the authoritative diagnostic confirmed this file
    # exists at CMakeFiles/4.4.2/CMakeCXXCompiler.cmake (4.4.2 ==
    # $ExpectedCMakeVersion, already required exact by item 7 below) and
    # contains the exact CMAKE_CXX_COMPILER / CMAKE_CXX_COMPILER_ID /
    # CMAKE_CXX_COMPILER_VERSION values quoted in this correction. This
    # check now reads that generated file directly and fails closed on all
    # three fields, instead of requiring CMAKE_CXX_COMPILER in
    # CMakeCache.txt.
    #
    # UNVERIFIED: the exact `set(NAME "value")` line syntax assumed by the
    # regexes below is CMake's own long-stable, standard
    # CMakeCXXCompiler.cmake.in template output - not something this task
    # authored - but it was not confirmed against a real generated file in
    # this session (no execution channel).
    $compilerCmakePath = Join-Path $BuildDir (Join-Path 'CMakeFiles' (Join-Path $ExpectedCMakeVersion 'CMakeCXXCompiler.cmake'))
    Assert-FileExists $compilerCmakePath "CMake-generated CMakeCXXCompiler.cmake (configure must have run before this check; expected under CMakeFiles/$ExpectedCMakeVersion/, matching this Runbook's own exact required CMake version)"
    $compilerCmakeContent = Get-Content $compilerCmakePath -Raw

    if ($compilerCmakeContent -notmatch 'set\(CMAKE_CXX_COMPILER "([^"]*)"\)') {
        throw 'CMakeCXXCompiler.cmake does not record a CMAKE_CXX_COMPILER value'
    }
    $resolvedCompiler = $Matches[1].Trim()
    Assert-FileExists $resolvedCompiler 'resolved cl.exe (path recorded in CMakeCXXCompiler.cmake)'

    if ($compilerCmakeContent -notmatch 'set\(CMAKE_CXX_COMPILER_ID "([^"]*)"\)') {
        throw 'CMakeCXXCompiler.cmake does not record a CMAKE_CXX_COMPILER_ID value'
    }
    $resolvedCompilerId = $Matches[1].Trim()
    if ($resolvedCompilerId -ne 'MSVC') {
        throw "CMakeCXXCompiler.cmake records CMAKE_CXX_COMPILER_ID='$resolvedCompilerId', expected exactly 'MSVC'"
    }

    if ($compilerCmakeContent -notmatch 'set\(CMAKE_CXX_COMPILER_VERSION "([^"]*)"\)') {
        throw 'CMakeCXXCompiler.cmake does not record a CMAKE_CXX_COMPILER_VERSION value'
    }
    $resolvedCompilerVersion = $Matches[1].Trim()
    if ($resolvedCompilerVersion -ne $ExpectedMsvcCompilerVersion) {
        throw "CMakeCXXCompiler.cmake records CMAKE_CXX_COMPILER_VERSION='$resolvedCompilerVersion', expected exactly '$ExpectedMsvcCompilerVersion'"
    }

    # AA Source Review Round 4 MINOR hardening: the prior version of this
    # check only verified the resolved path CONTAINS the expected VCTools
    # version as a substring ([regex]::Escape($ExpectedVcToolsVersion)) -
    # which would also accept, e.g., a stray second VS installation whose
    # unrelated path segments happened to embed that same version string,
    # or a differing host/target architecture pair (x86 hosting x64, etc.)
    # under the correct VCTools version. This now resolves the actual VS
    # installation root independently (same vswhere pattern as the item 5
    # checks above - not read from that scriptblock's own $installPath,
    # which is local to its own scope) and compares the FULL normalized
    # cl.exe path exactly against the one specific accepted
    # Hostx64/x64 toolset path, not a substring match.
    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    $vswhere = $vswhereCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $vswhere) {
        throw 'vswhere.exe was not found - cannot resolve the Visual Studio installation root to validate the exact expected cl.exe path'
    }
    $installPath = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if (-not $installPath) {
        throw 'vswhere reported no Visual Studio installation with the v143 (VC.Tools.x86.x64) component - cannot validate the exact expected cl.exe path'
    }
    $expectedCompiler = Join-Path $installPath (Join-Path 'VC\Tools\MSVC' (Join-Path $ExpectedVcToolsVersion 'bin\Hostx64\x64\cl.exe'))
    Assert-FileExists $expectedCompiler 'expected exact cl.exe (VC\Tools\MSVC\<version>\bin\Hostx64\x64\cl.exe)'

    $normalizedResolved = [System.IO.Path]::GetFullPath($resolvedCompiler)
    $normalizedExpected = [System.IO.Path]::GetFullPath($expectedCompiler)
    # Windows paths are case-insensitive; an ordinal case-insensitive
    # comparison of the two normalized full paths is the exact match this
    # check requires - not a substring/contains check.
    if (-not [string]::Equals($normalizedResolved, $normalizedExpected, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "resolved compiler path '$normalizedResolved' does not exactly match the expected cl.exe path '$normalizedExpected' (VC\Tools\MSVC\$ExpectedVcToolsVersion\bin\Hostx64\x64\cl.exe under the resolved VS installation root '$installPath')"
    }
}

# ==============================================================================
# Brief section 24, item 6 (M15 addendum): fail closed on the fresh CMake
# cache's exact CMAKE_TOOLCHAIN_FILE and VCPKG_TARGET_TRIPLET, not merely
# effective compiler identity - AA Source Review Round 3 finding M15: a
# partially different toolchain (e.g. a stray, non-accepted vcpkg
# installation resolved instead of $AcceptedVcpkgRoot, or a non-
# x64-windows triplet) must not be allowed to PASS.
# ==============================================================================
Invoke-Check "fresh CMake cache's CMAKE_TOOLCHAIN_FILE and VCPKG_TARGET_TRIPLET are exactly the accepted vcpkg toolchain / x64-windows" '6' {
    Assert-FileExists (Join-Path $BuildDir 'CMakeCache.txt') 'CMakeCache.txt (configure must have run before this check)'
    $cache = Get-Content (Join-Path $BuildDir 'CMakeCache.txt') -Raw

    if ($cache -notmatch 'CMAKE_TOOLCHAIN_FILE:(?:FILEPATH|STRING|UNINITIALIZED)=(.*)') {
        throw 'CMakeCache.txt does not record a CMAKE_TOOLCHAIN_FILE'
    }
    $resolvedToolchainFile = (($Matches[1].Trim()) -replace '/', '\').ToLowerInvariant().TrimEnd('\')
    $expectedToolchainFile = ((Join-Path $AcceptedVcpkgRoot 'scripts\buildsystems\vcpkg.cmake')).ToLowerInvariant().TrimEnd('\')
    if ($resolvedToolchainFile -ne $expectedToolchainFile) {
        throw "CMakeCache.txt records CMAKE_TOOLCHAIN_FILE='$($Matches[1].Trim())', expected exactly '$(Join-Path $AcceptedVcpkgRoot 'scripts\buildsystems\vcpkg.cmake')' (the accepted bundled `$AcceptedVcpkgRoot's own vcpkg.cmake) - a partially different vcpkg toolchain must not PASS"
    }

    if ($cache -notmatch 'VCPKG_TARGET_TRIPLET:(?:STRING|UNINITIALIZED)=(.*)') {
        throw 'CMakeCache.txt does not record a VCPKG_TARGET_TRIPLET'
    }
    $resolvedTriplet = $Matches[1].Trim()
    if ($resolvedTriplet -ne 'x64-windows') {
        throw "CMakeCache.txt records VCPKG_TARGET_TRIPLET='$resolvedTriplet', expected exactly 'x64-windows'"
    }
}

# ==============================================================================
# Brief section 24, item 10: frozen vcpkg baseline unchanged.
#
# AA Source Review Round 2 finding B02: this project pins its vcpkg
# baseline via vcpkg-configuration.json's default-registry.baseline field -
# vcpkg.json itself has no builtin-baseline field in this repo (confirmed by
# reading both files directly from the task worktree). Round 1's version of
# this check read a field that does not exist here, so it would always have
# thrown "vcpkg.json has no builtin-baseline field" and never actually
# verified anything about the real baseline.
# ==============================================================================
Invoke-Check "vcpkg-configuration.json default-registry.baseline matches the frozen baseline ($VcpkgBaseline)" '10' {
    $vcpkgJsonPath = Join-Path $RepoRoot 'vcpkg.json'
    Assert-FileExists $vcpkgJsonPath 'vcpkg.json'
    $vcpkgJson = Get-Content $vcpkgJsonPath -Raw | ConvertFrom-Json
    if ($vcpkgJson.PSObject.Properties.Name -contains 'builtin-baseline') {
        # Defensive: this check's whole design rests on vcpkg.json NOT
        # carrying its own builtin-baseline (only vcpkg-configuration.json
        # does, in this repo, today). If that has changed, the two files
        # could now disagree and silently picking one over the other would
        # be worse than failing loudly here.
        throw "vcpkg.json now has a 'builtin-baseline' field ('$($vcpkgJson.'builtin-baseline')') - this check's design assumption (the baseline is pinned exclusively via vcpkg-configuration.json's default-registry.baseline) no longer holds; reconcile both files by hand before trusting this check again, do not silently prefer either value."
    }

    $vcpkgConfigPath = Join-Path $RepoRoot 'vcpkg-configuration.json'
    Assert-FileExists $vcpkgConfigPath 'vcpkg-configuration.json'
    $vcpkgConfig = Get-Content $vcpkgConfigPath -Raw | ConvertFrom-Json
    if (-not $vcpkgConfig.'default-registry' -or -not $vcpkgConfig.'default-registry'.baseline) {
        throw 'vcpkg-configuration.json has no default-registry.baseline field'
    }
    $resolvedBaseline = $vcpkgConfig.'default-registry'.baseline
    if ($resolvedBaseline -ne $VcpkgBaseline) {
        throw "vcpkg-configuration.json default-registry.baseline is '$resolvedBaseline', expected the frozen baseline '$VcpkgBaseline'"
    }
}

# ==============================================================================
# Brief section 24, items 11-14: resolved qtbase exactly 6.11.1#1, resolved
# bgfx exactly 1.129.8940-496#1, bgfx multithreaded feature absent, Qt
# Widgets present.
# ==============================================================================
# AA Source Review Round 2 finding M12: Round 1 only checked that the
# version *substring* (e.g. "6.11.1") appeared somewhere in the vcpkg list
# line - that would also silently accept a stale/mismatched portversion
# (e.g. a rebuild that resolved "6.11.1#2" instead of the frozen "6.11.1#1"
# would still contain the substring "6.11.1" and pass). This now tokenizes
# the vcpkg list line on whitespace and requires an exact match on the
# "<version>#<portversion>" token Brief section 2 actually pins.
# AA Runbook D Attempt 1 correction (RD1-04): items 11+12/13/14 all
# previously resolved vcpkg.exe via $env:VCPKG_ROOT / a PATH fallback and
# invoked `vcpkg list --x-install-root=<explicit path>` against the fresh
# build's own installed tree - the authoritative diagnostic found this
# resolved a vcpkg.exe (or an install-root argument form that vcpkg.exe)
# not actually reading the fresh build's tree, reporting "No packages are
# installed". Item 11+12 (package/portversion identity) keeps using
# `vcpkg.exe list`, but now resolves it explicitly and only from
# $AcceptedVcpkgRoot (the same bundled root this script's B03/item-5 checks
# already treat as authoritative) rather than an ambient PATH/env vcpkg,
# and fails closed if the explicit install root directory itself does not
# exist yet. Items 13/14 (per-FEATURE presence/absence) no longer use
# `vcpkg list` at all - its default one-line-per-package output does not
# print feature names, so those checks could never have matched their
# target text regardless of install-root resolution. They now parse
# <install-root>/vcpkg/status directly (Get-VcpkgStatusStanzas /
# Test-VcpkgPackageInstalled above), which also lets item 13 fail closed on
# bgfx's required "tools" feature - a check this Runbook did not
# previously have - in addition to bgfx's forbidden "multithreaded"
# feature, and explicitly fail (rather than vacuously pass) if the target
# package itself is not found installed at all.
Invoke-Check "resolved qtbase is exactly $ExpectedQtBasePortVersion and resolved bgfx is exactly $ExpectedBgfxPortVersion (bundled vcpkg.exe list, explicit install root)" '11+12' {
    $vcpkgExePath = Join-Path $AcceptedVcpkgRoot 'vcpkg.exe'
    Assert-FileExists $vcpkgExePath 'bundled vcpkg.exe (AcceptedVcpkgRoot)'
    $vcpkgInstalledRoot = Join-Path $BuildDir 'vcpkg_installed'
    if (-not (Test-Path $vcpkgInstalledRoot)) {
        throw "explicit vcpkg install root '$vcpkgInstalledRoot' does not exist - configure/build must have run before this check"
    }
    Push-Location $BuildDir
    try {
        $listOutput = & $vcpkgExePath list --x-install-root=$vcpkgInstalledRoot
    } finally {
        Pop-Location
    }
    $qtLine = $listOutput | Where-Object { $_ -match '^qtbase[:\[]' }
    $bgfxLine = $listOutput | Where-Object { $_ -match '^bgfx[:\[]' }
    if (-not $qtLine) { throw 'vcpkg list reported no resolved qtbase package' }
    if (-not $bgfxLine) { throw 'vcpkg list reported no resolved bgfx package' }
    $qtTokens = @(($qtLine -join ' ') -split '\s+' | Where-Object { $_ -ne '' })
    $bgfxTokens = @(($bgfxLine -join ' ') -split '\s+' | Where-Object { $_ -ne '' })
    if ($qtTokens -notcontains $ExpectedQtBasePortVersion) {
        throw "resolved qtbase line '$qtLine' does not contain the exact expected version#portversion token '$ExpectedQtBasePortVersion'"
    }
    if ($bgfxTokens -notcontains $ExpectedBgfxPortVersion) {
        throw "resolved bgfx line '$bgfxLine' does not contain the exact expected version#portversion token '$ExpectedBgfxPortVersion'"
    }
}
Invoke-Check "bgfx 'tools' feature is present and 'multithreaded' feature is absent from the resolved feature set (vcpkg_installed/vcpkg/status, fail-closed)" '13' {
    $statusPath = Join-Path $BuildDir (Join-Path 'vcpkg_installed' (Join-Path 'vcpkg' 'status'))
    $stanzas = Get-VcpkgStatusStanzas $statusPath
    if (-not (Test-VcpkgPackageInstalled -Stanzas $stanzas -PackageName 'bgfx' -Triplet $Triplet)) {
        throw "vcpkg_installed/vcpkg/status records no installed 'bgfx' package (triplet '$Triplet') at all - this must fail, not be treated as vacuous evidence that its 'multithreaded' feature happens to be absent."
    }
    if (-not (Test-VcpkgPackageInstalled -Stanzas $stanzas -PackageName 'bgfx' -Triplet $Triplet -Feature 'tools')) {
        throw "vcpkg_installed/vcpkg/status does not record bgfx's required 'tools' feature as installed (triplet '$Triplet') - without it, shaderc (RD1-01) would not have been available for the build that just ran."
    }
    if (Test-VcpkgPackageInstalled -Stanzas $stanzas -PackageName 'bgfx' -Triplet $Triplet -Feature 'multithreaded') {
        throw "vcpkg_installed/vcpkg/status records bgfx's 'multithreaded' feature as installed (triplet '$Triplet'), which Brief section 2/12 forbids for P0-T003."
    }
}
Invoke-Check "Qt Widgets is present in the resolved qtbase feature set (vcpkg_installed/vcpkg/status, fail-closed)" '14' {
    $statusPath = Join-Path $BuildDir (Join-Path 'vcpkg_installed' (Join-Path 'vcpkg' 'status'))
    $stanzas = Get-VcpkgStatusStanzas $statusPath
    if (-not (Test-VcpkgPackageInstalled -Stanzas $stanzas -PackageName 'qtbase' -Triplet $Triplet)) {
        throw "vcpkg_installed/vcpkg/status records no installed 'qtbase' package (triplet '$Triplet') at all."
    }
    if (-not (Test-VcpkgPackageInstalled -Stanzas $stanzas -PackageName 'qtbase' -Triplet $Triplet -Feature 'widgets')) {
        throw "vcpkg_installed/vcpkg/status does not record qtbase's required 'widgets' feature as installed (triplet '$Triplet')."
    }
}

# ==============================================================================
# Brief section 24, item 15: format job.
# ==============================================================================
Invoke-Check 'clang-format reports zero formatting diffs across the P0-T003 first-party sources' '15' {
    $clangFormat = Get-Command clang-format -ErrorAction SilentlyContinue
    if (-not $clangFormat) { throw 'clang-format was not found on PATH' }
    $sourceFiles = Get-ChildItem -Path (Join-Path $RepoRoot 'src\viewport'), (Join-Path $RepoRoot 'src\desktop') `
        -Recurse -Include '*.hpp', '*.cpp' -File
    $dirty = @()
    foreach ($file in $sourceFiles) {
        $diffOutput = & $clangFormat.Source '--dry-run' '--Werror' $file.FullName 2>&1
        if ($LASTEXITCODE -ne 0) {
            $dirty += $file.FullName
        }
    }
    if ($dirty.Count -gt 0) {
        throw "clang-format --dry-run --Werror reported diffs for: $($dirty -join ', ')"
    }
}

# ==============================================================================
# Brief section 24, item 16: configure/build/full CTest job.
#
# AA Source Review Round 2 finding M12: Round 1's version of this item only
# checked that a build directory / CMakeCache.txt / artifacts already
# existed somewhere - it never actually drove a configure or build itself,
# so a stale build directory left over from an earlier, unrelated run (or
# built against different sources entirely) could pass every check below
# without this run ever having compiled anything.
#
# The actual "wipe $BuildDir and drive a real 'cmake --preset
# $ConfigurePreset' / 'cmake --build --preset $ConfigurePreset'" check for
# this item now runs EARLIER in this script - immediately after item 5
# (the VS Build Tools environment check) and before item 6 (effective
# compiler identity) - not here in Brief-item numeric order. Item 6 reads
# CMakeCache.txt and items 11+12/13/14 read $BuildDir's resolved
# vcpkg_installed tree; both need this run's OWN fresh configure to have
# already happened, not a leftover CMakeCache.txt/vcpkg_installed from
# whatever was in $BuildDir before this script started (which is exactly
# what item 16's fresh-build fix above is for). Search this file for
# "Build directory was wiped and freshly configured+built" for that check;
# it is still tagged Brief item '16' in the final tally despite running
# earlier in the script, since Invoke-Check's Item tag is independent of
# call order. The artifact-existence checks below only need the BUILD half
# to be done, so they stay here in their natural item-16 position -
# Assert-FileFresh still correctly compares against
# $script:PreBuildTimestampUtc, set once by that earlier check.
# ==============================================================================
Invoke-Check 'bim_viewport static library was built (fresh this run)' '16' {
    $found = Find-BuiltFile @(
        (Join-Path $BuildDir 'src\viewport\bim_viewport.lib'),
        (Join-Path $BuildDir 'src\viewport\Release\bim_viewport.lib'),
        (Join-Path $BuildDir 'src\viewport\Debug\bim_viewport.lib')
    )
    if (-not $found) { throw 'bim_viewport.lib not found under the build tree (checked common single-/multi-config layouts)' }
    Assert-FileFresh -Path $found -What 'bim_viewport.lib'
}
Invoke-Check 'bim_viewport_bgfx static library was built (fresh this run)' '16' {
    $found = Find-BuiltFile @(
        (Join-Path $BuildDir 'src\viewport\bgfx\bim_viewport_bgfx.lib'),
        (Join-Path $BuildDir 'src\viewport\bgfx\Release\bim_viewport_bgfx.lib'),
        (Join-Path $BuildDir 'src\viewport\bgfx\Debug\bim_viewport_bgfx.lib')
    )
    if (-not $found) { throw 'bim_viewport_bgfx.lib not found under the build tree' }
    Assert-FileFresh -Path $found -What 'bim_viewport_bgfx.lib'
}
Invoke-Check 'bim_desktop_spike.exe was built (fresh this run)' '16' {
    $exe = Find-SpikeExe -InBuildDir $BuildDir
    if (-not $exe) { throw 'bim_desktop_spike.exe not found under the build tree' }
    Assert-FileFresh -Path $exe -What 'bim_desktop_spike.exe'
}
Invoke-Check 'Compiled dx11 shader binaries exist and are fresh this run (vs_p0_t003.bin, fs_p0_t003.bin)' '16' {
    Assert-FileFresh (Join-Path $BuildDir 'shaders\dx11\vs_p0_t003.bin') 'compiled vertex shader'
    Assert-FileFresh (Join-Path $BuildDir 'shaders\dx11\fs_p0_t003.bin') 'compiled fragment shader'
}
Invoke-Check 'Shader binaries were staged next to bim_desktop_spike.exe (fresh this run)' '16' {
    $exe = Find-SpikeExe -InBuildDir $BuildDir
    if (-not $exe) { throw 'cannot check staged shaders: bim_desktop_spike.exe was not found (see the prior check)' }
    $shaderDir = Join-Path (Split-Path $exe -Parent) 'shaders\dx11'
    Assert-FileFresh (Join-Path $shaderDir 'vs_p0_t003.bin') 'staged vertex shader'
    Assert-FileFresh (Join-Path $shaderDir 'fs_p0_t003.bin') 'staged fragment shader'
}
Invoke-Check 'Full CTest run (every registered test, not exact-name-filtered) exits 0' '16' {
    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }
    Push-Location $BuildDir
    try {
        Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('--output-on-failure') | Out-Null
    } finally {
        Pop-Location
    }
}

# ==============================================================================
# Brief section 24, item 17: static-analysis job.
# ==============================================================================
Invoke-Check 'clang-tidy reports zero diagnostics across the P0-T003 first-party sources' '17' {
    $clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
    if (-not $clangTidy) { throw 'clang-tidy was not found on PATH' }
    $compileCommands = Join-Path $BuildDir 'compile_commands.json'
    Assert-FileExists $compileCommands 'compile_commands.json (configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)'

    # AA RD1.7 (Full Runbook D Attempt 2, Check 24 - HARNESS/COMPDB ONLY,
    # clang-cl analysis compatibility): the authoritative diagnostic proved
    # that with exact clang-tidy 19.1.5 the raw compile_commands.json
    # contains exactly one /Zc:preprocessor token (the MSVC-only option
    # src/viewport/bgfx/CMakeLists.txt applies to bim_viewport_bgfx for
    # renderer.cpp - RD1.3-01, required by bx/platform.h under MSVC and
    # proven by the passing production build), and that renderer.cpp's
    # clang-tidy run fails with exit=1 on that token ALONE
    # ('argument unused during compilation: /Zc:preprocessor'
    # [clang-diagnostic-unused-command-line-argument]) - clang-cl accepts the
    # MSVC build's flags but has no use for that one. Removing exactly that
    # token from renderer.cpp's command, with identical source bytes, gives
    # exit=0 and no project diagnostic. So this check analyses against a
    # TEMPORARY, analysis-only copy of the compilation database in which
    # that single token is removed from renderer.cpp's command only. The
    # authoritative raw compile_commands.json is never modified (hash
    # asserted unchanged below); the production MSVC build keeps
    # /Zc:preprocessor; renderer.cpp and src/viewport/bgfx/CMakeLists.txt
    # are untouched; no NOLINT is added. Every assumption is asserted and
    # fails closed: exactly one token in the raw DB, exactly one entry
    # carrying it, that entry is renderer.cpp, exactly one removal, zero
    # tokens left in the temporary DB.
    $rawText = Get-Content -LiteralPath $compileCommands -Raw
    $rawHashBefore = (Get-FileHash -LiteralPath $compileCommands -Algorithm SHA256).Hash
    $token = '/Zc:preprocessor'
    $rawTokenMatches = [regex]::Matches($rawText, [regex]::Escape($token))
    if ($rawTokenMatches.Count -ne 1) {
        throw "raw compile_commands.json contains $($rawTokenMatches.Count) '$token' token(s); this check's clang-cl analysis-compatibility rewrite is defined only for exactly one (bim_viewport_bgfx / renderer.cpp, RD1.3-01). Fail-closed: not guessing which occurrence(s) to remove."
    }
    # AA RD1.8 Amendment A3 (Full Runbook D Attempt 3, Check 24 -
    # RUNBOOK_CHECK24_HARNESS_OR_COMPDB_WIRING_DEFECT, PROVEN): under
    # Windows PowerShell 5.1, @($rawText | ConvertFrom-Json) is not
    # guaranteed to normalize the parsed top-level JSON array into the
    # individual compile-entry objects directly - Attempt 3's own
    # authoritative logs showed the resulting collection observed zero
    # compile entries carrying the token even though the fresh raw compdb
    # genuinely has 42 entries and exactly one /Zc:preprocessor token
    # (owner: renderer.cpp), independently reproduced against a temporary
    # analysis-only compdb (11/11 first-party TUs exit 0, zero project
    # diagnostics). This is a Runbook parsing/wiring defect only - the
    # production compile_commands.json, CMake, and renderer.cpp all
    # require no correction. Rather than trust the implicit pipe-based
    # conversion, ConvertFrom-Json is now called directly (not piped, so
    # no pipeline-driven reshaping of its output can occur) and its result
    # is always explicitly rebuilt, one item at a time, into a flat
    # one-dimensional array of entry objects - regardless of whether the
    # parsed root came back as an array, a single object (a documented
    # ConvertFrom-Json single-element-array collapse), or anything else
    # enumerable.
    $parsedRoot = ConvertFrom-Json -InputObject $rawText
    $rawEntriesList = New-Object System.Collections.Generic.List[object]
    if ($null -ne $parsedRoot) {
        if (($parsedRoot -is [System.Collections.IEnumerable]) -and -not ($parsedRoot -is [string])) {
            foreach ($item in $parsedRoot) { [void]$rawEntriesList.Add($item) }
        } else {
            [void]$rawEntriesList.Add($parsedRoot)
        }
    }
    $rawEntries = @($rawEntriesList.ToArray())
    if ($rawEntries.Count -eq 0) {
        throw "raw compile_commands.json normalized to zero parsed compile entries (expected > 0 - this raw compdb is asserted above to contain exactly one '$token' token, so it cannot be empty); this is the RD1.8 A3-diagnosed Runbook compdb normalization defect surfacing again, or the raw compdb is genuinely empty/malformed. Fail-closed: not proceeding with an empty entry set."
    }
    $entriesWithToken = @($rawEntries | Where-Object {
        (($_.PSObject.Properties.Name -contains 'command') -and ($_.command -like "*$token*")) -or
        (($_.PSObject.Properties.Name -contains 'arguments') -and (@($_.arguments) -contains $token))
    })
    if ($entriesWithToken.Count -ne 1) {
        throw "raw compile_commands.json has $($entriesWithToken.Count) compile entries carrying '$token' (expected exactly one) out of $($rawEntries.Count) normalized parsed entries. Fail-closed."
    }
    $expectedFileSuffix = 'src\viewport\bgfx\src\renderer.cpp'
    $tokenEntryFile = [string]$entriesWithToken[0].file
    if (-not (($tokenEntryFile -replace '/', '\') -like "*\$expectedFileSuffix")) {
        throw "the only compile entry carrying '$token' is for '$tokenEntryFile', not '...\$expectedFileSuffix' as the RD1.7 classification requires. Fail-closed: not removing the token from any other translation unit."
    }
    # Textual removal of that single token (leading whitespace included for
    # the 'command' string form; leading comma for the 'arguments' array
    # form) so every other byte of the database is preserved verbatim.
    $removalPattern = '(?:\s+' + [regex]::Escape($token) + '(?=[\s"])|,\s*"' + [regex]::Escape($token) + '")'
    $removalMatches = [regex]::Matches($rawText, $removalPattern)
    if ($removalMatches.Count -ne 1) {
        throw "expected exactly one removable '$token' occurrence in renderer.cpp's compile command, found $($removalMatches.Count) (unexpected quoting/spelling). Fail-closed."
    }
    $analysisText = ([regex]$removalPattern).Replace($rawText, '', 1)
    if (([regex]::Matches($analysisText, [regex]::Escape($token))).Count -ne 0) {
        throw "temporary analysis compile_commands.json still contains '$token' after the single removal. Fail-closed."
    }
    $analysisDir = Join-Path $BuildDir 'evidence\P0-T003\runbook-d\clang-tidy-analysis-compdb'
    New-Item -ItemType Directory -Force -Path $analysisDir | Out-Null
    $analysisCompileCommands = Join-Path $analysisDir 'compile_commands.json'
    [System.IO.File]::WriteAllText($analysisCompileCommands, $analysisText, [System.Text.UTF8Encoding]::new($false))
    Write-Host "  clang-cl analysis compdb (temporary, analysis-only): $analysisCompileCommands"
    Write-Host "  removed exactly 1 '$token' token from renderer.cpp's command only; raw compile_commands.json untouched"

    $sourceFiles = Get-ChildItem -Path (Join-Path $RepoRoot 'src\viewport'), (Join-Path $RepoRoot 'src\desktop') `
        -Recurse -Include '*.cpp' -File
    $failed = @()
    foreach ($file in $sourceFiles) {
        # AA RD1.8 Amendment A4 (RUNBOOK_CHECK24_NATIVE_STDERR_ERRORACTIONPREFERENCE_DEFECT,
        # PROVEN): clang-tidy's own native stderr ("N warnings generated.")
        # is entirely normal, clean-run output, not evidence of a real
        # diagnostic - but the prior direct
        # `& $clangTidy.Source ... 2>&1 | Out-Null` call let this Runbook's
        # $ErrorActionPreference = 'Stop' promote every native stderr line
        # into a *terminating* PowerShell exception (proven on Attempt 4:
        # "exception: 63246 warnings generated." etc. for all 11 TUs),
        # aborting this loop before $LASTEXITCODE was ever meaningfully
        # consulted - a Runbook-only native-process handling defect, not a
        # real clang-tidy failure (independently reproduced: with native
        # stderr handled as non-terminating, the exact same 11 TUs are
        # 11/11 exit 0, zero project diagnostics; A3's compdb normalization
        # is unaffected and remains correct on this fresh database). Rather
        # than invent a new wrapper, this now reuses this project's own
        # existing native-command handling pattern -
        # scripts/ci/_common.ps1's Invoke-Native, already dot-sourced by
        # this script (see the top of the file) and already used for every
        # other native invocation here (cmake, ctest, the PowerShell child
        # scripts, evidence-mode) - which was already hardened for exactly
        # this class of defect. Invoke-Native scopes
        # $ErrorActionPreference = 'Continue' only for the duration of the
        # native call (restored via finally, so this Runbook's global
        # 'Stop' policy is untouched everywhere else), captures stdout and
        # stderr as text (neither discarded - both are printed, same as
        # every other check in this script), and reads $LASTEXITCODE right
        # after as the sole authority for success/failure. -AllowFailure is
        # passed so a single failing TU returns the result object instead
        # of throwing and aborting this foreach loop - preserving the
        # pre-A4 code's exact per-file $failed accumulation shape. A
        # non-zero native exit code - this repo's .clang-tidy
        # WarningsAsErrors policy is what turns a real project diagnostic
        # into a non-zero clang-tidy exit, unchanged and not newly parsed
        # here - still fails this file exactly as before; stderr text alone
        # with a zero exit code no longer does.
        $tidyResult = Invoke-Native -Exe $clangTidy.Source -CmdArgs @('-p', $analysisDir, $file.FullName) -AllowFailure
        if ($tidyResult.ExitCode -ne 0) {
            $failed += $file.FullName
        }
    }
    $rawHashAfter = (Get-FileHash -LiteralPath $compileCommands -Algorithm SHA256).Hash
    if ($rawHashAfter -ne $rawHashBefore) {
        throw "authoritative raw compile_commands.json changed during this check (sha256 $rawHashBefore -> $rawHashAfter); it must never be modified"
    }
    if ($failed.Count -gt 0) {
        throw "clang-tidy reported diagnostics for: $($failed -join ', ')"
    }
}

# ==============================================================================
# Brief section 24, item 18: architecture job with every required exact
# architecture test.
# ==============================================================================
foreach ($t in $P0T003Tests | Where-Object { $_ -like 'arch_*' }) {
    Invoke-Check "CTest '$t' is registered and passes (exact-name)" '18' {
        Invoke-ExactCTest -TestName $t -InBuildDir $BuildDir
    }
}
Invoke-Check 'scripts/ci/architecture.ps1 job (all required architecture tests, P0-T001+P0-T002+P0-T003) passes' '18' {
    $script = Join-Path $RepoRoot 'scripts\ci\architecture.ps1'
    $psCmd = Get-Command powershell -ErrorAction SilentlyContinue
    if (-not $psCmd) { $psCmd = Get-Command pwsh -ErrorAction SilentlyContinue }
    if (-not $psCmd) { throw 'neither powershell nor pwsh was found on PATH' }
    Invoke-Native -Exe $psCmd.Source -CmdArgs @('-File', $script, '-BuildDir', $BuildDir) | Out-Null
}

# ==============================================================================
# Brief section 24, item 19: license-inventory job.
# ==============================================================================
Invoke-Check 'scripts/ci/license-inventory.ps1 job (all 7 direct dependencies captured) passes' '19' {
    $script = Join-Path $RepoRoot 'scripts\ci\license-inventory.ps1'
    $psCmd = Get-Command powershell -ErrorAction SilentlyContinue
    if (-not $psCmd) { $psCmd = Get-Command pwsh -ErrorAction SilentlyContinue }
    if (-not $psCmd) { throw 'neither powershell nor pwsh was found on PATH' }
    Invoke-Native -Exe $psCmd.Source -CmdArgs @('-File', $script, '-BuildDir', $BuildDir, '-Triplet', $Triplet) | Out-Null
}

# ==============================================================================
# Brief section 24, item 20: viewport-spike evidence job (now the JSON
# -based scripts/ci/viewport-spike.ps1, which itself covers Brief section
# 16's >= 20 process-level repeatability cycles and section 23's final
# live evidence mode - AA Source Review Round 1 finding M05/M06).
# ==============================================================================
Invoke-Check 'scripts/ci/viewport-spike.ps1 job (required tests, 20+ process-level repeatability cycles, final live evidence) passes' '20' {
    $script = Join-Path $RepoRoot 'scripts\ci\viewport-spike.ps1'
    $psCmd = Get-Command powershell -ErrorAction SilentlyContinue
    if (-not $psCmd) { $psCmd = Get-Command pwsh -ErrorAction SilentlyContinue }
    if (-not $psCmd) { throw 'neither powershell nor pwsh was found on PATH' }
    # AA RD1.7 (Attempt 2 Check 35): viewport-spike.ps1 resolves the same
    # process-local Qt plugin environment itself for its own child
    # processes; wrapping here additionally fails this check closed (with
    # the Runbook's own message) before the child script is even launched
    # if qwindows.dll is missing from this build's vcpkg_installed tree.
    Invoke-WithQtPluginEnvironment -InBuildDir $BuildDir -ForTriplet $Triplet -Body {
        Invoke-Native -Exe $psCmd.Source -CmdArgs @('-File', $script, '-BuildDir', $BuildDir) | Out-Null
    }
}

# ==============================================================================
# Brief section 24, item 21: all P0-T001 regression tests exact-name
# anchored.
# ==============================================================================
foreach ($t in $P0T001Tests) {
    Invoke-Check "P0-T001 regression CTest '$t' is registered and passes (exact-name)" '21' {
        Invoke-ExactCTest -TestName $t -InBuildDir $BuildDir
    }
}

# ==============================================================================
# Brief section 24, item 22: all P0-T002 tests exact-name anchored.
# ==============================================================================
foreach ($t in $P0T002Tests) {
    Invoke-Check "P0-T002 regression CTest '$t' is registered and passes (exact-name)" '22' {
        Invoke-ExactCTest -TestName $t -InBuildDir $BuildDir
    }
}

# ==============================================================================
# Brief section 24, item 23: all P0-T003 tests exact-name anchored.
# ==============================================================================
foreach ($t in $P0T003Tests | Where-Object { $_ -notlike 'arch_*' }) {
    Invoke-Check "P0-T003 CTest '$t' is registered and passes (exact-name)" '23' {
        Invoke-ExactCTest -TestName $t -InBuildDir $BuildDir
    }
}
# (P0-T003's arch_* tests were already exact-name-anchored under item 18
# above - Brief section 24 items 18 and 23 overlap by construction for the
# architecture tests specifically, since "the required exact architecture
# test[s]" and "all P0-T003 tests" both include them; not duplicated here.)

# ==============================================================================
# Brief section 24, item 24: evidence JSON schema and backend identity.
# ==============================================================================
Invoke-Check 'bim_desktop_spike.exe --evidence-mode (headless) writes valid JSON with the required top-level fields' '24' {
    $exe = Find-SpikeExe -InBuildDir $BuildDir
    if (-not $exe) { throw 'bim_desktop_spike.exe not found' }
    $jsonPath = Join-Path $BuildDir 'evidence\P0-T003\runbook-d\headless-schema-check.json'
    New-Item -ItemType Directory -Force -Path (Split-Path $jsonPath -Parent) | Out-Null
    # AA RD1.7 (Attempt 2 Check 60): the headless evidence process is a Qt
    # application and needs the platform plugin to start at all - launched
    # with the process-local Qt plugin environment (fail-closed if
    # qwindows.dll is missing), restored right after.
    Invoke-WithQtPluginEnvironment -InBuildDir $BuildDir -ForTriplet $Triplet -Body {
        Invoke-Native -Exe $exe -CmdArgs @('--evidence-mode', $jsonPath) | Out-Null
    }
    Assert-FileExists $jsonPath 'evidence JSON (headless)'
    $evidence = Get-Content $jsonPath -Raw | ConvertFrom-Json
    $requiredFields = @(
        'task_id', 'gate_id', 'brief_id', 'qt_version', 'bgfx_version_frozen_baseline',
        'camera', 'lifecycle', 'backend', 'window', 'scenes', 'resize_result',
        'surface_recreation_result', 'lifecycle_tail', 'ray_result', 'repeatability',
        'overall_passed'
    )
    $missing = @($requiredFields | Where-Object { -not ($evidence.PSObject.Properties.Name -contains $_) })
    if ($missing.Count -gt 0) {
        throw "evidence JSON is missing required top-level field(s): $($missing -join ', ')"
    }
    if ($evidence.backend.selected_renderer_backend -ne 'Noop') {
        throw "headless evidence JSON reports backend '$($evidence.backend.selected_renderer_backend)', expected 'Noop'"
    }
    if ($evidence.scenes.Count -ne 5) {
        throw "evidence JSON scenes array has $($evidence.scenes.Count) entries, expected exactly 5 (V01-V05)"
    }
}
# The live-mode JSON's schema and explicit Direct3D11 backend identity are
# validated by scripts/ci/viewport-spike.ps1's own final-live-evidence step
# (item 20 above) - not duplicated here to avoid running the live path
# (which requires an attached display) twice per Runbook D invocation.

# ==============================================================================
# Brief section 24, item 25: final task/main Git invariants and candidate
# identity.
# ==============================================================================
Invoke-Check 'Git index is empty (Implementation Authorization section 11: index must remain empty at handover)' '25' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $staged = & $gitCmd.Source diff --cached --name-only
    } finally {
        Pop-Location
    }
    $stagedList = @($staged | Where-Object { $_ -and $_.Trim() -ne '' })
    if ($stagedList.Count -gt 0) {
        throw "Git index is not empty - staged path(s): $($stagedList -join ', ')"
    }
}
Invoke-Check 'main branch is still exactly the frozen baseline after all other checks ran (no check mutated it)' '25' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $mainRef = $null
        foreach ($candidate in @('main', 'origin/main', 'master', 'origin/master')) {
            $rev = & $gitCmd.Source rev-parse --verify --quiet $candidate 2>$null
            if ($LASTEXITCODE -eq 0 -and $rev) { $mainRef = $rev.Trim(); break }
        }
    } finally {
        Pop-Location
    }
    if ($mainRef -ne $MainBaseline) {
        throw "main ref is now '$mainRef', expected the frozen main baseline '$MainBaseline' - something modified main during this run"
    }
}
Invoke-Check 'candidate identity: current branch/HEAD are still exactly the expected task branch/HEAD' '25' {
    $gitCmd = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitCmd) { throw 'git was not found on PATH - cannot verify' }
    Push-Location $RepoRoot
    try {
        $branch = (& $gitCmd.Source rev-parse --abbrev-ref HEAD).Trim()
        $head = (& $gitCmd.Source rev-parse HEAD).Trim()
    } finally {
        Pop-Location
    }
    if ($branch -ne $TaskBranch -or $head -ne $ExpectedPreVerificationHead) {
        throw "candidate identity drifted during this run: branch='$branch' (expected '$TaskBranch'), HEAD='$head' (expected '$ExpectedPreVerificationHead')"
    }
}

# ------------------------------------------------------------------------------
# Verdict
# ------------------------------------------------------------------------------
Write-CiSection 'Verification-RunbookD-v1.0.ps1 - final verdict'

# AA RD1.8 Amendment A3 (SECOND PROVEN RUNBOOK DEFECT - final tally/verdict
# contradiction): Attempt 3's authoritative logs proved the prior tally
# logic here was defective - `$failed = $script:Checks | Where-Object {...}`
# returns a *scalar* PSCustomObject, not a one-element array, whenever
# exactly one check fails (a documented PowerShell Where-Object behavior:
# zero matches -> $null, exactly one match -> that single object, more
# than one -> an array). A scalar/$null has no .Count property, so
# `$failed.Count` silently evaluated to $null; `$total - $null` coerces
# the $null to 0 in arithmetic, so the printed PASS count came out as the
# full total and the printed FAIL count came out blank; `$null -gt 0` is
# `$false`, so the exit-1 branch was never taken and exit 0 / "= PASS" was
# reached even with a real [FAIL] row already printed above (Attempt 3:
# a printed [FAIL] Check 24 row alongside "Total checks: 63 PASS: 63" and
# "VERIFICATION RUNBOOK D = PASS", exit code 0). Every collection below is
# therefore always forced into a real array with @(...) before its .Count
# is read, so a single-match (or zero-match) Where-Object result can never
# silently collapse to a non-array scalar or $null again. Each check's
# authoritative status is also read here as an explicit boolean (`$true`
# only for the literal string 'PASS'), and the printed per-check rows, the
# printed counts, the exit code, and the global verdict are all derived
# from that one same boolean array - none of them independently
# recomputed or allowed to disagree with each other.
$allChecks = @($script:Checks)
$checkIsPass = @($allChecks | ForEach-Object { [bool]($_.Status -eq 'PASS') })

$totalCount = $allChecks.Count
$passCount = @($checkIsPass | Where-Object { $_ -eq $true }).Count
$failCount = @($checkIsPass | Where-Object { $_ -eq $false }).Count

for ($i = 0; $i -lt $allChecks.Count; $i++) {
    $c = $allChecks[$i]
    $isPass = $checkIsPass[$i]
    $rowStatus = if ($isPass) { 'PASS' } else { 'FAIL' }
    $color = if ($isPass) { 'Green' } else { 'Red' }
    Write-Host ("  [{0}] Check {1,3} (Brief 24.{2,-4}) - {3}" -f $rowStatus, $c.Number, $c.Item, $c.Name) -ForegroundColor $color
}
Write-Host ''
Write-Host "Total checks: $totalCount  PASS: $passCount  FAIL: $failCount"

$coveredItems = ($allChecks | ForEach-Object { $_.Item } | Sort-Object -Unique)
Write-Host "Brief section 24 items covered by at least one check above: $($coveredItems -join ', ')"
Write-Host 'Cross-check this list against Brief section 24''s literal 25-item enumeration by hand - item "11+12" above intentionally covers two Brief items with one combined check, so this list has 24 distinct entries (1-10, "11+12", 13-25) covering all 25 Brief items, not 25 entries.'

# AA RD1.8 Amendment A3: global PASS is allowed ONLY when every one of
# these four invariants holds over the exact same boolean state computed
# above; any inconsistency (including an impossible tally that the prior
# logic could previously let through silently) fails closed with a
# non-zero exit, never a silent PASS. This does not hard-code Check 24 (or
# any other individual check) as PASS, and does not bypass any failed
# check - $checkIsPass is read straight from this run's own per-check
# Status values with no special-casing by check number or name.
$invariantsHold = (
    ($totalCount -eq $script:ExpectedTotalCheckCount) -and
    ($passCount -eq $totalCount) -and
    ($failCount -eq 0) -and
    (($passCount + $failCount) -eq $totalCount)
)

if (-not $invariantsHold) {
    Write-Host ''
    Write-Host "VERIFICATION RUNBOOK D = FAIL (total=$totalCount expected=$($script:ExpectedTotalCheckCount) pass=$passCount fail=$failCount)" -ForegroundColor Red
    exit 1
}

Write-Host ''
Write-Host 'VERIFICATION RUNBOOK D = PASS' -ForegroundColor Green
exit 0
