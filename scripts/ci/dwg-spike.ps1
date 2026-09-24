<#
==============================================================================
 scripts/ci/dwg-spike.ps1 - P0-T006 DWG / ODA Evaluation CI job (Execution
 Packet BIM-AA-P0-T006 v1.0; AA CP2B-H3 clarification's required test list).

 Mirrors scripts/ci/architecture.ps1's exact-name-anchored ctest pattern
 (that script's own header explains why a single broad `ctest -R "^..."`
 invocation is not trustworthy proof every required test ran - the same
 reasoning applies here), following scripts/ci/viewport-spike.ps1's overall
 shape but without that script's Qt/live-surface machinery, which has no
 DWG/ODA equivalent - this spike has no interactive/live-window concept.

 Runs, as separate exact-name-anchored invocations, every test Execution
 Packet BIM-AA-P0-T006 v1.0's AA clarification response requires:
   - unit_dwg_probe_contract
   - integration_dwg_missing_file
   - integration_dwg_same_path_rejected
   - integration_dwg_vendor_fixture_read
   - integration_dwg_same_version_round_trip
   - integration_dwg_evidence
   - arch_p0_t006_oda_owner
   - arch_p0_t006_oda_owner_fixture_rejected
   - arch_p0_t006_dwg_public_neutral
   - arch_p0_t006_dwg_public_fixture_rejected

 Then:
   - re-derives the locked vendor fixture's absolute path from the build's
     own CMakeCache.txt (BIM_P0_T006_DWG_FIXTURE) unless -DwgFixture is
     passed explicitly, and hashes it (SHA256) both BEFORE and AFTER the
     test run, throwing if they differ - mechanical proof that no test in
     this job modified the original locked fixture in place (Execution
     Packet section 7 / AA's explicit "do not modify the original fixture
     in place" instruction), independent of and in addition to what any
     individual test itself asserts;
   - reads the integration_dwg_evidence test's own JSON output
     (evidence/P0-T006/p0_t006_dwg_evidence.ctest.json under the build
     directory), prints its key fields, and validates its own `overall`
     field is true;
   - never edits source/docs; never changes dependencies; never stages,
     commits, or pushes anything.

 UNVERIFIED: this script has never been executed in this session (no
 execution channel against the locked ODA Drawings SDK / Windows worktree).
 It is authored against the exact _common.ps1/Invoke-Native/Write-CiSection
 pattern already proven out by scripts/ci/architecture.ps1 and
 scripts/ci/viewport-spike.ps1, but the Windows Execution Operator's first
 real run is its first actual execution.

 Requires: an already-configured, built build directory with
 BIM_ENABLE_DWG=ON, BIM_ODA_DRAWINGS_ROOT, and BIM_P0_T006_DWG_FIXTURE all
 set at configure time (this script does not configure or build - see
 scripts\ci\configure-build-test.ps1 / the root CMakeLists.txt's own
 BIM_ENABLE_DWG comment for those variables).

 Exit code: 0 only if every required CTest test is found registered AND
 passing, the vendor fixture's SHA256 is byte-identical before and after
 the run, and the evidence JSON's own `overall` field is true.

 Usage: powershell -File scripts\ci\dwg-spike.ps1 [-BuildDir <path>] [-DwgFixture <path>]
==============================================================================
#>

param(
    [string]$BuildDir,
    [string]$DwgFixture
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\ci-win-msvc-dwg'
}

$RequiredTests = @(
    'unit_dwg_probe_contract',
    'integration_dwg_missing_file',
    'integration_dwg_same_path_rejected',
    'integration_dwg_vendor_fixture_read',
    'integration_dwg_same_version_round_trip',
    'integration_dwg_evidence',
    'arch_p0_t006_oda_owner',
    'arch_p0_t006_oda_owner_fixture_rejected',
    'arch_p0_t006_dwg_public_neutral',
    'arch_p0_t006_dwg_public_fixture_rejected'
)

# Re-derives BIM_P0_T006_DWG_FIXTURE from the build's own CMakeCache.txt
# rather than hard-coding or guessing it - the build already required this
# exact value to configure at all (tests/integration/CMakeLists.txt's own
# FATAL_ERROR guard), so the cache is the authoritative source of truth for
# "which fixture did THIS build actually compile against."
function Resolve-DwgFixturePath {
    param([Parameter(Mandatory)][string]$InBuildDir)
    $cachePath = Join-Path $InBuildDir 'CMakeCache.txt'
    if (-not (Test-Path -LiteralPath $cachePath -PathType Leaf)) {
        throw "CMakeCache.txt not found under '$InBuildDir' - is this an already-configured build directory?"
    }
    $line = Select-String -Path $cachePath -Pattern '^BIM_P0_T006_DWG_FIXTURE:.*=(.*)$' | Select-Object -First 1
    if (-not $line) {
        throw "BIM_P0_T006_DWG_FIXTURE was not found in '$cachePath' - was this build directory configured with -DBIM_ENABLE_DWG=ON -DBIM_P0_T006_DWG_FIXTURE=...?"
    }
    $value = $line.Matches[0].Groups[1].Value.Trim()
    if ([string]::IsNullOrWhiteSpace($value)) {
        throw "BIM_P0_T006_DWG_FIXTURE is present in '$cachePath' but empty."
    }
    return $value
}

try {
    Write-CiSection "dwg-spike: locate build directory ($BuildDir)"
    if (-not (Test-Path $BuildDir)) {
        throw "Build directory '$BuildDir' does not exist. Configure and build it first with -DBIM_ENABLE_DWG=ON, -DBIM_ODA_DRAWINGS_ROOT=<path>, and -DBIM_P0_T006_DWG_FIXTURE=<path>, or pass -BuildDir to point at an existing one."
    }

    if (-not $DwgFixture) {
        Write-CiSection 'dwg-spike: resolve BIM_P0_T006_DWG_FIXTURE from CMakeCache.txt'
        $DwgFixture = Resolve-DwgFixturePath -InBuildDir $BuildDir
        Write-Host "  BIM_P0_T006_DWG_FIXTURE = $DwgFixture"
    }
    if (-not (Test-Path -LiteralPath $DwgFixture -PathType Leaf)) {
        throw "Locked vendor fixture not found at '$DwgFixture'."
    }

    Write-CiSection 'dwg-spike: hash the locked vendor fixture BEFORE the test run'
    $preHash = (Get-FileHash -LiteralPath $DwgFixture -Algorithm SHA256).Hash
    Write-Host "  SHA256 (before) = $preHash"

    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }

    Push-Location $BuildDir
    try {
        foreach ($testName in $RequiredTests) {
            $exactPattern = '^' + $testName + '$'
            Write-CiSection "dwg-spike: ctest -R `"$exactPattern`" --output-on-failure --no-tests=error"
            # No -AllowFailure: a missing test (ctest exits non-zero under
            # --no-tests=error when the exact-name regex matches nothing) or
            # a failing test both throw here and are caught below - mirrors
            # scripts/ci/viewport-spike.ps1's own exact-name-anchored loop.
            Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('-R', $exactPattern, '--output-on-failure', '--no-tests=error')
        }
    } finally {
        Pop-Location
    }

    Write-CiSection 'dwg-spike: hash the locked vendor fixture AFTER the test run'
    $postHash = (Get-FileHash -LiteralPath $DwgFixture -Algorithm SHA256).Hash
    Write-Host "  SHA256 (after)  = $postHash"
    if ($postHash -ne $preHash) {
        throw "the locked vendor fixture's SHA256 changed during this test run (before=$preHash, after=$postHash) - the original fixture must never be modified in place (Execution Packet section 7)."
    }
    Write-Host '  fixture SHA256 unchanged - the original locked fixture was not modified in place.' -ForegroundColor Green

    Write-CiSection 'dwg-spike: read the integration_dwg_evidence JSON'
    $evidenceJsonPath = Join-Path $BuildDir 'evidence\P0-T006\p0_t006_dwg_evidence.ctest.json'
    if (-not (Test-Path -LiteralPath $evidenceJsonPath -PathType Leaf)) {
        throw "expected evidence JSON not found at '$evidenceJsonPath' - integration_dwg_evidence passed but did not leave the expected file?"
    }
    $evidence = Get-Content -LiteralPath $evidenceJsonPath -Raw | ConvertFrom-Json

    Write-Host ''
    Write-Host "--- $evidenceJsonPath ---"
    Write-Host "  source_version              : $($evidence.source_version)"
    Write-Host "  reopened_version            : $($evidence.reopened_version)"
    Write-Host "  read_success                : $($evidence.read_success)"
    Write-Host "  write_success               : $($evidence.write_success)"
    Write-Host "  reopen_success              : $($evidence.reopen_success)"
    Write-Host "  source_mtext_found          : $($evidence.source_mtext_found)"
    Write-Host "  reopened_mtext_found        : $($evidence.reopened_mtext_found)"
    Write-Host "  text_content_preserved      : $($evidence.text_content_preserved)"
    Write-Host "  position_preserved          : $($evidence.position_preserved)"
    Write-Host "  normal_preserved            : $($evidence.normal_preserved)"
    Write-Host "  direction_preserved         : $($evidence.direction_preserved)"
    Write-Host "  text_height_preserved       : $($evidence.text_height_preserved)"
    Write-Host "  width_preserved             : $($evidence.width_preserved)"
    Write-Host "  attachment_preserved        : $($evidence.attachment_preserved)"
    Write-Host "  text_style_preserved        : $($evidence.text_style_preserved)"
    Write-Host "  round_trip_overall_passed   : $($evidence.round_trip_overall_passed)"
    Write-Host "  missing_file_rejected       : $($evidence.missing_file_rejected)"
    Write-Host "  same_path_rejected          : $($evidence.same_path_rejected)"
    Write-Host "  overall                     : $($evidence.overall)"

    if ($evidence.source_version -ne 'AC1018' -or $evidence.reopened_version -ne 'AC1018') {
        throw "evidence JSON did not report AC1018 on both source_version and reopened_version (source='$($evidence.source_version)', reopened='$($evidence.reopened_version)')."
    }
    if (-not $evidence.overall) {
        throw "evidence JSON reported overall = false - see '$evidenceJsonPath' for the per-field breakdown."
    }

    Write-Host ''
    Write-Host "DWG SPIKE CI JOB PASSED - all required tests ($($RequiredTests -join ', ')) were found registered and passed, the locked vendor fixture's SHA256 was unchanged, and the evidence JSON reported AC1018 source/reopen identity with overall = true." -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "dwg-spike.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
