<#
==============================================================================
 P0-T001 - Verification Runbook B (v1.6)
 Classification : Operator-executed / Claude-authored runbook
 Scope          : Implementation Brief Phase D (vcpkg manifest resolution
                   evidence), Phase K (static analysis), and Phase N
                   (verification from clean state).
 Repository path (main, read-only reference) : D:\Projects\BIM-Platform
 Task worktree path (this runbook operates here exclusively) :
                   D:\Projects\BIM-Platform-WT-P0-T001
 Branch         : task/P0-T001-repo-toolchain-scaffold

 v1.6 revision (Architecture Authority disposition on the FIRST REAL
 WINDOWS EXECUTION of v1.5 - "P0-T001 - Verification Runbook B v1.5 First
 Execution Disposition": VERIFICATION HARNESS DEFECT, implementation NOT
 TESTED). Real execution reached Step 2 (VSCMD_ARG_TGT_ARCH=x64,
 VSCMD_VER=17.14.39, VCToolsVersion=14.44.35207, cl.exe resolved
 correctly - all genuine evidence, not previously available to Claude, who
 has no execution channel to the Windows target) and then aborted BEFORE
 any of the five CI jobs ran, with: "Cannot bind argument to parameter
 'CmdArgs' because it is an empty array." Root cause: the runbook-local
 Invoke-Native's `[Parameter(Mandatory)][string[]]$CmdArgs` rejects an
 empty array under PowerShell's Mandatory-parameter binding rules unless
 `[AllowEmptyCollection()]` is also present; the compiler-banner probe
 (Step 2, MAJOR from v1.2) intentionally calls
 `Invoke-Native -Exe $clCmd.Source -CmdArgs @() -AllowFailure` (cl.exe with
 no arguments, by design, to capture its version banner) and was rejected
 by the binder before ever reaching the native call. This is a defect in
 the harness itself, not in anything it was verifying - no CI job, no
 CMake/vcpkg/MSVC/CTest/clang-format/clang-tidy evidence, and no
 architecture/license-inventory result exists from this run; nothing
 about the implementation was tested by this execution attempt. Minimum
 correction applied: the runbook-local Invoke-Native's $CmdArgs parameter
 is now `[Parameter(Mandatory)][AllowEmptyCollection()][string[]]$CmdArgs`.
 The compiler-banner probe call itself is UNCHANGED - same arguments, same
 -AllowFailure, same semantics; only the parameter now accepts the empty
 array it was always meant to be called with. scripts\ci\_common.ps1 is
 NOT modified - no child CI job currently calls Invoke-Native with an
 empty argument array, so broadening that copy is out of scope here. Every
 other already-accepted line of Runbook B logic (Step 0 preflight; Step 2
 tool/environment checks including the v1.4 VCToolsVersion/CMake-version
 gates; Step 3 vcpkg-configuration.json checks; Step 4's five isolated
 child-process jobs; Step 5 CMake-configuration gate; Step 6 vcpkg-list
 evidence gate; Step 7 final Git invariants; Step 8 Evidence Summary) is
 unchanged in substance by this revision.

 v1.5 revision (architecture review "P0-T001 Architecture Review -
 Verification Candidate v1.4" - ONE FINAL BLOCKER). All v1.0->v1.4
 findings are CLOSED and unregressed. This revision makes no change to
 Runbook B's own logic - it binds this candidate to the corrected
 scripts\ci\architecture.ps1 (run as job 4 of 5, Step 4, isolated in its
 own child PowerShell process exactly as before). The prior single broad
 `ctest -R "^arch_"` invocation inside architecture.ps1 did not prove both
 required tests (arch_repository_boundaries,
 arch_checker_detects_violation) were actually registered and executed;
 architecture.ps1 now runs each as its own exact-name-anchored
 `ctest -R "^<name>$" --output-on-failure --no-tests=error` invocation
 through Invoke-Native without -AllowFailure, so a missing OR a failing
 required test both fail the job - see architecture.ps1's own header
 comment for the full behavior table. Runbook B's own Step 4 job-isolation,
 Step 8 Evidence Summary, and every other already-accepted verification
 gate from v1.0->v1.4 are unchanged by this revision.

 v1.4 revision (architecture review "P0-T001 Architecture Review -
 Verification Runbook B v1.3" - REVISION REQUIRED). All findings from the
 v1.0->v1.3 reviews not listed below are unchanged and NOT regressed. Five
 fixes this revision:

   BLOCKER (CMakePresets.json) - The `$comment` field has been removed.
     `$comment` is a presets-schema-version-10 feature and is invalid at
     the declared schema version 3; this repository deliberately keeps
     version 3 for earlier-maintained-CMake compatibility rather than
     raising the schema version to keep a comment. The informative content
     that comment held (CI reference toolchain = CMake 4.4.2, Architecture
     Gate section 4.1) was not lost - it is independently documented in
     CMakeLists.txt's own header comment (a plain-text CMake file, where
     comments are valid at any version) and in the Architecture Gate
     itself. No `ci-win-msvc` preset semantics changed.

   BLOCKER (.clang-tidy) - `WarningsAsErrors` changed from `''` to `'*'`
     per Architecture Authority's explicit disposition: enabled P0-T001
     clang-tidy findings are now CI-gating. The curated `Checks` list is
     unchanged - only whether a finding is allowed to coexist with a zero
     exit code changed. scripts\ci\static-analysis.ps1 is unchanged in
     logic (still report-only, still never passes -fix) but its own header
     comment is updated to describe what its exit code now means under the
     new policy. This disposition is recorded in
     docs/project-control/DECISION-LEDGER.md and CHANGE-LOG.md.

   MAJOR (Step 2) - The developer-environment check now also captures and
     validates $env:VCToolsVersion, requiring it to belong to the VS 2022
     v143 toolset family (matched as a "14.3*"/"14.4*" MSVC tools version -
     v141/VS2017 is "14.1*", v142/VS2019 is "14.2*", v143/VS2022 is
     "14.3*"/"14.4*"), aborting otherwise - so a VS 17.14 shell
     accidentally still configured to an older v142 individual component
     cannot satisfy the v143 requirement. `cmake --version`'s own reported
     version is now parsed and required to equal exactly 4.4.2 (Architecture
     Gate's locked P0 reference/CI CMake version), aborting otherwise,
     rather than only being printed as unparsed evidence. Both verified
     values appear in the Step 8 Evidence Summary.

   MAJOR (scripts\ci\license-inventory.ps1) - Before copying this run's
     license/usage files, the script now removes any previously generated
     `<dep>.LICENSE.txt`/`<dep>.USAGE.txt` for the five direct
     dependencies (and only those - non-generated files such as
     third_party\licenses\README.md are untouched). This closes a gap
     where a stale artifact from an earlier run could satisfy this run's
     completeness check even if the current vcpkg_installed tree's copy
     silently produced nothing.

   MINOR - "VERIFICATION RUNBOOK B COMPLETE" now says ALL FIVE JOBS
     PASSED, not four; the Usage section below names this actual v1.4
     script; scripts\ci\format.ps1, configure-build-test.ps1,
     architecture.ps1, and license-inventory.ps1 header comments now say
     "logical job N of 5" (1, 2, 4, 5 respectively - static-analysis.ps1
     was already correctly "3 of 5") instead of the historical "of 4".

 Historical note: the v1.3 entry below states the repository's
 WarningsAsErrors policy was `''` at that time - this was accurate when
 v1.3 was issued and is superseded by the BLOCKER (.clang-tidy) fix above;
 it is left as-written below as a historical record of that revision, not
 edited in place.

 v1.3 revision (architecture review "P0-T001 Architecture Review - STATIC
 ANALYSIS BLOCKER"). All findings from the v1.0->v1.1 and v1.1->v1.2
 reviews are unchanged and NOT regressed (pre-mutation preflight; real
 vcpkg-configuration.json/CMakeCache.txt parsing instead of trusting
 constants; per-job isolated child processes; x64 + VS-17.14
 developer-environment verification with captured MSVC banner; required
 tool version checks without -AllowFailure; final Git invariants; enforced
 CMake-configuration gate; authoritative vcpkg-list dependency-evidence
 gate; default-registry.kind assertion). This revision closes the
 Phase K / Phase N gap the v1.2 child-script audit itself surfaced:
 clang-tidy was not executed by any CI-equivalent job.

   Fix - A fifth job, scripts\ci\static-analysis.ps1, is now run (Step 4)
     between configure-build-test and architecture, in its own isolated
     child PowerShell process exactly like the other four jobs, with an
     independently captured exit code. It requires clang-tidy to exist
     (operational blocker, exit non-zero, not silently waived - Phase K -
     if missing), runs it read-only (no -fix) against every first-party
     translation unit found in configure-build-test's compile database
     (compile_commands.json, now produced via
     CMAKE_EXPORT_COMPILE_COMMANDS=ON - see CMakePresets.json), using the
     repository .clang-tidy explicitly via --config-file, and returns
     non-zero if any invocation fails. `job_static-analysis` now appears
     in the Evidence Summary automatically (Step 8 already iterates every
     job in $results). If clang-tidy is missing or any invocation fails,
     this job's own non-zero exit code feeds into the existing
     $anyJobFailed / $anyFailed logic exactly like the other four jobs -
     Runbook B cannot report overall PASS in that case. See
     scripts\ci\static-analysis.ps1's own header comment for the precise,
     narrow scope of what "clang-tidy invocation fails" means given the
     repository's existing WarningsAsErrors: '' policy in .clang-tidy -
     this revision does not silently change that policy.

 The v1.1->v1.2 revision's four verification-integrity fixes remain in
 place, unchanged by this revision:

   BLOCKER (Step 5) - The effective CMake configuration check is now an
     ENFORCED GATE, not a warning. Whenever the configure-build-test job
     itself reports success (exit 0), this script independently requires
     CMakeCache.txt to exist AND the effective VCPKG_TARGET_TRIPLET to be
     x64-windows AND the effective CMAKE_GENERATOR to be Ninja. Any
     mismatch is added to $verificationFailures, which - together with any
     job failure - drives the final exit code (Step 8). A successful run
     is no longer possible under a triplet/generator mismatch; before this
     revision it was (only a Yellow warning was printed).

   MAJOR (Step 3) - vcpkg-configuration.json verification now also asserts
     default-registry.kind == "builtin", in addition to the existing
     default-registry.baseline assertion. IC-002 syntax is unchanged and
     not converted to the obsolete builtin-baseline field.

   MAJOR (Step 6) - `vcpkg list` evidence is no longer optional/best-effort.
     Whenever configure-build-test succeeds, this script requires the
     expected install root to exist and `vcpkg list` to succeed (both
     added to $verificationFailures on failure, not merely logged),
     records the actual resolved version of all five direct P0
     dependencies, asserts opencascade resolves to exactly 8.0.1 and
     catch2 resolves to major version 3, and asserts sqlite3/fmt/spdlog
     are present in the list (their actual resolved versions are recorded
     as evidence, not asserted against an invented exact value - none was
     given by Architecture Gate/Implementation Brief for those three).
     Resolved versions are printed in the final Evidence Summary.

   MAJOR (Step 2) - The developer-environment check now also asserts the
     active Visual Studio line is the approved 17.14 line
     ($env:VSCMD_VER -like '17.14*'), aborting (as the existing x64 check
     already does) if it is not - a different VS major/minor line is not
     silently accepted as authoritative P0-T001 evidence. The actual MSVC
     compiler version/banner is captured as evidence by invoking cl.exe
     with no arguments (which prints its version banner to stderr and
     exits non-zero by design - this is expected, not a failure, and
     -AllowFailure is used deliberately for that one call only), and the
     resolved cl.exe path is recorded alongside it.

 What this script does (unchanged from v1.1 otherwise):
   1. Verifies preconditions and the active x64 / VS-17.14 MSVC developer
      environment, printing every tool's version and the MSVC banner as
      evidence.
   2. Verifies the actual on-disk vcpkg-configuration.json registry kind
      and baseline before running any job.
   3. Runs, in isolated child processes, all five scripts\ci\*.ps1 jobs
      required by Implementation Brief Phase L / Phase K, in order:
        format.ps1 -> configure-build-test.ps1 -> static-analysis.ps1
        -> architecture.ps1 -> license-inventory.ps1
      continuing through a failed job for maximum evidence in one run.
   4. Verifies the actual effective CMake configuration after configure -
      now an enforced gate (see BLOCKER above).
   5. Verifies and records resolved direct-dependency versions via
      `vcpkg list --x-install-root=<build>\vcpkg_installed` - now an
      enforced gate (see MAJOR above) (Phase D evidence: AC-007).
   6. Prints final Git evidence for BOTH the task worktree and the main
      repository path (to prove main was not touched by this run), then a
      structured EVIDENCE SUMMARY block including resolved dependency
      versions.
   7. The ENTIRE console session is captured verbatim via Start-Transcript
      to docs\evidence\P0-T001\verification-runbook-b-transcript-<UTC
      timestamp>.txt. Per the standing evidentiary rule for this task
      ("raw outputs and exit codes remain authoritative"), return that
      transcript file's content in full, not a paraphrase.

 What this script deliberately does NOT do:
   - It does not `git add` or `git commit` anything (Implementation Brief
     Phase O "scope audit" and Phase P "commit" are separate, later,
     explicitly-authorized steps - not part of Phase N verification).
   - It does not modify D:\Projects\BIM-Platform (main) in any way.
   - It does not advance the frozen vcpkg baseline
     (f89a4a1da4e3176a8d1a14c1825b9b2f98e48843) under any circumstance; if
     resolution fails, that is a stop condition requiring an ACR, not a
     reason to edit vcpkg.json/vcpkg-configuration.json.
   - It does not silently install clang-format/clang-tidy if missing; a
     missing tool is recorded as an operational blocker (Phase K), not
     waived.
   - It does not mutate any file before the Step 0 preflight passes.
   - It does not invent exact expected versions for sqlite3/fmt/spdlog;
     it records their actual baseline-resolved versions as evidence.

 Note on the five scripts\ci\*.ps1 jobs this runbook invokes: their literal
 current contents are returned alongside this script for Architecture
 Authority's review, per the review's own request.
   - scripts\ci\static-analysis.ps1 is NEW this revision (see the v1.3
     changelog entry above) - it closes the clang-tidy gap the v1.2
     child-script audit surfaced.
   - scripts\ci\configure-build-test.ps1 was updated this revision to
     verify compile_commands.json is actually generated after configure
     (CMAKE_EXPORT_COMPILE_COMMANDS=ON, set in CMakePresets.json
     conf-common), throwing if it is missing rather than letting
     static-analysis.ps1 discover the gap later.
   - scripts\ci\format.ps1, architecture.ps1, license-inventory.ps1, and
     _common.ps1 are unchanged by this revision.

 Safety:
   - Uses the same $LASTEXITCODE-authoritative Invoke-Native pattern as
     Bootstrap Runbook A v1.3 (scripts\ci\_common.ps1) throughout - no
     native-stderr-becomes-terminating-exception pitfall.
   - Every job's exit code is captured independently, from its own child
     process (MAJOR 1, v1.1).
   - Exits 0 only if ALL FIVE jobs passed (format, configure-build-test,
     static-analysis, architecture, license-inventory) AND all final Git
     invariants held AND the CMake-configuration gate held AND the
     vcpkg-list evidence gate held. Exits non-zero (and reports exactly
     which job(s)/gate(s) failed) otherwise.

 Usage (regular PowerShell, x64 Native Tools / Developer PowerShell for
 VS 2022 17.14, no admin rights required):
   PS> cd D:\Projects\BIM-Platform-WT-P0-T001
   PS> $env:VCPKG_ROOT = 'C:\path\to\your\vcpkg'   # if not already set
   PS> .\Verification-RunbookB-v1.6.ps1

 Return the transcript file written under docs\evidence\P0-T001\ verbatim,
 in full, together with this script's own final console output.
==============================================================================
#>

$ErrorActionPreference = 'Stop'

$WorktreePath = 'D:\Projects\BIM-Platform-WT-P0-T001'
$MainRepoPath = 'D:\Projects\BIM-Platform'
$ExpectedBranch = 'task/P0-T001-repo-toolchain-scaffold'
$ExpectedBootstrapSha = '4b339248dd8b050e7b603ef0b5707440e582c315'
$Preset = 'ci-win-msvc'
$Triplet = 'x64-windows'
$ExpectedVcpkgBaseline = 'f89a4a1da4e3176a8d1a14c1825b9b2f98e48843'
$ExpectedVcpkgRegistryKind = 'builtin'
$ExpectedVsLine = '17.14'
$ExpectedVcToolsFamilyPattern = '^14\.[34]\d*\.'
$ExpectedCMakeVersion = '4.4.2'
$ExpectedOpencascadeVersion = '8.0.1'
$ExpectedCatch2Major = '3'

function Write-Section {
    param([string]$Title)
    Write-Host ''
    Write-Host "=== $Title ===" -ForegroundColor Cyan
}

function Fail {
    param([string]$Message)
    Write-Host ''
    Write-Host "RUNBOOK B ABORTED: $Message" -ForegroundColor Red
    if ($script:TranscriptStarted) { Stop-Transcript | Out-Null }
    exit 1
}

function Assert-True {
    param(
        [Parameter(Mandatory)][bool]$Condition,
        [Parameter(Mandatory)][string]$Message
    )
    if ($Condition) {
        Write-Host "  [OK] $Message"
    } else {
        throw "ASSERTION FAILED: $Message"
    }
}

function Invoke-Native {
    param(
        [Parameter(Mandatory)][string]$Exe,
        [Parameter(Mandatory)][AllowEmptyCollection()][string[]]$CmdArgs,
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
    $result = [PSCustomObject]@{ ExitCode = $code; Stdout = ($stdoutLines -join "`n"); Stderr = ($stderrLines -join "`n") }
    if ($result.Stdout) { Write-Host $result.Stdout }
    if ($result.Stderr) { Write-Host $result.Stderr }
    if ($code -ne 0 -and -not $AllowFailure) {
        throw "Command failed (exit $code): $Exe $($CmdArgs -join ' ')"
    }
    return $result
}

function Get-CMakeCacheValue {
    param(
        [Parameter(Mandatory)][string]$CachePath,
        [Parameter(Mandatory)][string]$VarName
    )
    if (-not (Test-Path $CachePath)) {
        return $null
    }
    $prefix = "$VarName" + ':'
    $lines = Get-Content -Path $CachePath
    foreach ($line in $lines) {
        $trimmed = $line.Trim()
        if ($trimmed.StartsWith('#') -or $trimmed.StartsWith('//')) { continue }
        if ($trimmed.StartsWith($prefix)) {
            $eq = $trimmed.IndexOf('=')
            if ($eq -ge 0) {
                return $trimmed.Substring($eq + 1)
            }
        }
    }
    return $null
}

function Get-VcpkgListVersion {
    param(
        [Parameter(Mandatory)][AllowEmptyCollection()][string[]]$Lines,
        [Parameter(Mandatory)][string]$PortName
    )
    foreach ($line in $Lines) {
        $trimmed = $line.Trim()
        if ([string]::IsNullOrWhiteSpace($trimmed)) { continue }
        # `vcpkg list` output format: "<port>[:<triplet>]   <version>   <description...>"
        # columns are whitespace-padded, not fixed-width - split on the
        # first run of whitespace only.
        $firstSpace = $trimmed.IndexOf(' ')
        if ($firstSpace -lt 0) { continue }
        $nameField = $trimmed.Substring(0, $firstSpace)
        $portOnly = $nameField.Split(':')[0]
        if ($portOnly -eq $PortName) {
            $rest = $trimmed.Substring($firstSpace).TrimStart()
            $secondSpace = $rest.IndexOf(' ')
            if ($secondSpace -ge 0) {
                return $rest.Substring(0, $secondSpace).Trim()
            } else {
                return $rest.Trim()
            }
        }
    }
    return $null
}

$script:TranscriptStarted = $false

# =============================================================================
# STEP 0 - PRE-MUTATION PREFLIGHT (unchanged from v1.1)
# Read-only. Runs BEFORE the evidence directory is created and BEFORE
# Start-Transcript. Deliberately outside the main try/catch below: on
# failure here, the script must exit having created nothing.
# =============================================================================
try {
    Write-Section 'Step 0 - PRE-MUTATION PREFLIGHT (read-only, before any filesystem mutation)'

    if (-not (Test-Path $WorktreePath)) {
        throw "Task worktree '$WorktreePath' does not exist. This runbook must be run against the P0-T001 isolated worktree created by Bootstrap Runbook A."
    }
    Push-Location $WorktreePath
    try {
        $preflightTaskBranch = git rev-parse --abbrev-ref HEAD
        $preflightTaskHead = git rev-parse HEAD
    } finally {
        Pop-Location
    }

    if (-not (Test-Path $MainRepoPath)) {
        throw "Main repository '$MainRepoPath' does not exist."
    }
    Push-Location $MainRepoPath
    try {
        $preflightMainBranch = git rev-parse --abbrev-ref HEAD
        $preflightMainHead = git rev-parse HEAD
        $preflightMainStatus = git status --short
    } finally {
        Pop-Location
    }

    Write-Host "task_worktree_path (preflight) = $WorktreePath"
    Write-Host "task_branch (preflight)        = $preflightTaskBranch"
    Write-Host "task_head (preflight)          = $preflightTaskHead"
    Write-Host "main_repo_path (preflight)     = $MainRepoPath"
    Write-Host "main_branch (preflight)        = $preflightMainBranch"
    Write-Host "main_head (preflight)          = $preflightMainHead"
    Write-Host ''

    Assert-True ($preflightTaskBranch -eq $ExpectedBranch) "task branch is '$ExpectedBranch' (found '$preflightTaskBranch')"
    Assert-True ($preflightTaskHead -eq $ExpectedBootstrapSha) "task HEAD is '$ExpectedBootstrapSha' (found '$preflightTaskHead')"
    Assert-True ($preflightMainBranch -eq 'main') "main branch is 'main' (found '$preflightMainBranch')"
    Assert-True ($preflightMainHead -eq $ExpectedBootstrapSha) "main HEAD is '$ExpectedBootstrapSha' (found '$preflightMainHead')"
    Assert-True ([string]::IsNullOrEmpty($preflightMainStatus)) 'main working tree is clean'

    Write-Host ''
    Write-Host 'PREFLIGHT PASSED. No filesystem mutation has occurred yet. Proceeding to create the evidence directory and start the transcript.' -ForegroundColor Green
} catch {
    Write-Host ''
    Write-Host "PREFLIGHT FAILED - RUNBOOK B ABORTED WITH ZERO MUTATION: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'No evidence directory was created and no transcript was started.' -ForegroundColor Red
    exit 1
}

# =============================================================================
# From here on the transcript is running; Fail() (which stops the
# transcript) is the correct abort path.
# =============================================================================
try {

    # -------------------------------------------------------------------
    Write-Section 'Step 1 - Start transcript and record preflight evidence'
    # -------------------------------------------------------------------
    Set-Location $WorktreePath

    $evidenceDir = Join-Path $WorktreePath 'docs\evidence\P0-T001'
    New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null
    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $transcriptPath = Join-Path $evidenceDir "verification-runbook-b-transcript-$timestamp.txt"
    Start-Transcript -Path $transcriptPath -Force | Out-Null
    $script:TranscriptStarted = $true

    Write-Host "Worktree     : $WorktreePath"
    Write-Host "Transcript   : $transcriptPath"
    Write-Host "Timestamp    : $timestamp (local); $(([DateTime]::UtcNow).ToString('u')) (UTC)"
    Write-Host ''
    Write-Host 'Preflight evidence (captured read-only before this transcript started; re-printed here for the record):'
    Write-Host "  task_branch (preflight) = $preflightTaskBranch"
    Write-Host "  task_head (preflight)   = $preflightTaskHead"
    Write-Host "  main_branch (preflight) = $preflightMainBranch"
    Write-Host "  main_head (preflight)   = $preflightMainHead"
    Write-Host '  main_status (preflight) = (clean)'

    $branch = $preflightTaskBranch
    $head = $preflightTaskHead

    # $verificationFailures accumulates every gate this revision adds
    # (CMake config gate, vcpkg-list evidence gate, final Git invariants).
    # A non-empty list, OR any job's own non-zero exit code, forces the
    # final exit code to 1 (Step 8) - collected rather than thrown so that
    # every remaining check still runs and reports for maximum evidence.
    $verificationFailures = New-Object System.Collections.Generic.List[string]

    # -------------------------------------------------------------------
    Write-Section 'Step 2 - Preconditions: required tools and approved x64 / VS 17.14 developer environment'
    # -------------------------------------------------------------------
    if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
        Fail 'VCPKG_ROOT is not set. Set it to your local vcpkg checkout, e.g.: $env:VCPKG_ROOT = "C:\tools\vcpkg"'
    }
    Write-Host "VCPKG_ROOT   : $env:VCPKG_ROOT"

    $vcpkgExe = Join-Path $env:VCPKG_ROOT 'vcpkg.exe'
    if (-not (Test-Path $vcpkgExe)) {
        Fail "vcpkg.exe not found at '$vcpkgExe'. Verify VCPKG_ROOT points at a bootstrapped vcpkg checkout (vcpkg bootstrap-vcpkg.bat has been run)."
    }

    $clCmd = Get-Command cl -ErrorAction SilentlyContinue
    if (-not $clCmd) {
        Fail 'cl.exe was not found on PATH. Run this script from an x64 Native Tools / Developer PowerShell for VS 2022.'
    }

    # MAJOR (v1.1): cl.exe merely existing on PATH is not sufficient
    # evidence of an x64 developer environment.
    $vscmdArgTgtArch = $env:VSCMD_ARG_TGT_ARCH
    $vscmdVer = $env:VSCMD_VER
    Write-Host "VSCMD_ARG_TGT_ARCH : $vscmdArgTgtArch"
    Write-Host "VSCMD_VER          : $vscmdVer"
    Write-Host "cl.exe path        : $($clCmd.Source)"
    if ($vscmdArgTgtArch -ne 'x64') {
        Fail "Active Visual Studio developer environment target architecture is '$vscmdArgTgtArch' (expected 'x64'). Run this script from the x64 Native Tools / Developer PowerShell for VS 2022, not an x86 shell."
    }

    # MAJOR (v1.2): also verify the approved VS 2022 17.14 line specifically
    # - a different VS major/minor line must not be silently accepted as
    # authoritative P0-T001 evidence.
    if ([string]::IsNullOrWhiteSpace($vscmdVer) -or -not ($vscmdVer -like "$ExpectedVsLine*")) {
        Fail "Active Visual Studio developer environment version is '$vscmdVer' (expected the approved $ExpectedVsLine line). Run this script from a Developer PowerShell for the approved VS 2022 $ExpectedVsLine line, not a different major/minor line."
    }

    # MAJOR (v1.4): a VS 17.14 shell can still be configured to an older
    # MSVC toolset (e.g. an individually-installed v142 component) - the
    # VS "line" alone does not prove the v143 toolset is active. Capture
    # and validate $env:VCToolsVersion (set by the developer-environment
    # script) against the v143 family: v141/VS2017 tools versions are
    # "14.1x.*", v142/VS2019 are "14.2x.*", v143/VS2022 are "14.3x.*" or
    # "14.4x.*". Abort if it does not match.
    $vcToolsVersion = $env:VCToolsVersion
    Write-Host "VCToolsVersion             : $vcToolsVersion"
    if ([string]::IsNullOrWhiteSpace($vcToolsVersion) -or -not ($vcToolsVersion -match $ExpectedVcToolsFamilyPattern)) {
        Fail "VCToolsVersion is '$vcToolsVersion', which does not match the required VS 2022 v143 toolset family (expected an MSVC tools version starting 14.3 or 14.4). A VS $ExpectedVsLine developer shell configured to an older v142 (or other) individual component does not satisfy the v143 requirement."
    }

    # MAJOR (v1.2): capture the actual MSVC compiler version/banner as
    # evidence. cl.exe invoked with no arguments prints its version banner
    # to stderr and exits non-zero by design (no input files) - this is
    # expected, not a failure, so -AllowFailure is used deliberately here
    # and only here.
    $clBannerResult = Invoke-Native -Exe $clCmd.Source -CmdArgs @() -AllowFailure
    $clBanner = $clBannerResult.Stderr
    if ([string]::IsNullOrWhiteSpace($clBanner)) { $clBanner = $clBannerResult.Stdout }
    Write-Host "cl.exe banner (evidence) : $clBanner"

    foreach ($tool in @('cmake', 'ninja', 'ctest', 'git')) {
        $cmd = Get-Command $tool -ErrorAction SilentlyContinue
        if (-not $cmd) {
            Fail "'$tool' was not found on PATH."
        }
    }

    # MAJOR (v1.1): required tool version checks must actually succeed -
    # no -AllowFailure on any of these five.
    Invoke-Native -Exe $vcpkgExe -CmdArgs @('version') | Out-Null
    $cmakeCmd = Get-Command cmake
    $cmakeVersionResult = Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--version')

    # MAJOR (v1.4): parse the actual reported CMake version and require it
    # to equal exactly the Architecture Gate's locked P0 reference/CI
    # version - printing it as unparsed evidence (v1.0-v1.3 behavior) is
    # not sufficient on its own.
    $cmakeVersionMatch = [regex]::Match($cmakeVersionResult.Stdout, 'cmake version (\S+)')
    $actualCMakeVersion = $null
    if ($cmakeVersionMatch.Success) {
        $actualCMakeVersion = $cmakeVersionMatch.Groups[1].Value
    }
    Write-Host "expected_cmake_version     = $ExpectedCMakeVersion"
    Write-Host "verified_effective_cmake_version = $actualCMakeVersion"
    if ([string]::IsNullOrWhiteSpace($actualCMakeVersion)) {
        Fail "Could not parse a CMake version from 'cmake --version' output. Refusing to proceed with an unverified CMake version."
    }
    if ($actualCMakeVersion -ne $ExpectedCMakeVersion) {
        Fail "Active cmake is version '$actualCMakeVersion', expected exactly '$ExpectedCMakeVersion' (Architecture Gate BIM-AG-P0-T001 section 4.1 locks this as the P0 reference/CI CMake version). Install/select CMake $ExpectedCMakeVersion, or raise an ACR to change the locked version."
    }

    $ninjaCmd = Get-Command ninja
    Invoke-Native -Exe $ninjaCmd.Source -CmdArgs @('--version') | Out-Null
    $ctestCmd = Get-Command ctest
    Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('--version') | Out-Null
    $gitCmd = Get-Command git
    Invoke-Native -Exe $gitCmd.Source -CmdArgs @('--version') | Out-Null

    $clangFormatCmd = Get-Command clang-format -ErrorAction SilentlyContinue
    if ($clangFormatCmd) {
        Invoke-Native -Exe $clangFormatCmd.Source -CmdArgs @('--version') -AllowFailure | Out-Null
    } else {
        Write-Host 'clang-format: NOT FOUND on PATH (scripts\ci\format.ps1 will record this as an operational blocker, not silently pass).' -ForegroundColor Yellow
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 3 - Verify actual vcpkg-configuration.json registry kind and baseline (MAJOR + BLOCKER carryover)'
    # -------------------------------------------------------------------
    $vcpkgConfigPath = Join-Path $WorktreePath 'vcpkg-configuration.json'
    if (-not (Test-Path $vcpkgConfigPath)) {
        Fail "vcpkg-configuration.json not found at '$vcpkgConfigPath'."
    }
    $vcpkgConfigRaw = Get-Content -Raw -Path $vcpkgConfigPath
    $vcpkgConfig = $vcpkgConfigRaw | ConvertFrom-Json

    # This repository's actual schema freezes the baseline under
    # default-registry.baseline / default-registry.kind (IC-002), not a
    # top-level "builtin-baseline" field. This reads the fields that
    # actually exist in the file, not assumed field names.
    $actualKind = $null
    $actualBaseline = $null
    if ($vcpkgConfig.PSObject.Properties.Name -contains 'default-registry') {
        $defaultRegistry = $vcpkgConfig.'default-registry'
        if ($defaultRegistry.PSObject.Properties.Name -contains 'kind') {
            $actualKind = $defaultRegistry.kind
        }
        if ($defaultRegistry.PSObject.Properties.Name -contains 'baseline') {
            $actualBaseline = $defaultRegistry.baseline
        }
    }

    Write-Host "expected_vcpkg_registry_kind           = $ExpectedVcpkgRegistryKind"
    Write-Host "verified_effective_vcpkg_registry_kind = $actualKind (parsed from vcpkg-configuration.json 'default-registry.kind')"
    Write-Host "expected_vcpkg_baseline                = $ExpectedVcpkgBaseline"
    Write-Host "verified_effective_vcpkg_baseline      = $actualBaseline (parsed from vcpkg-configuration.json 'default-registry.baseline')"

    if ([string]::IsNullOrWhiteSpace($actualKind)) {
        Fail "Could not find a 'default-registry.kind' value in vcpkg-configuration.json. Refusing to proceed with an unverified registry type."
    }
    if ($actualKind -ne $ExpectedVcpkgRegistryKind) {
        Fail "vcpkg-configuration.json 'default-registry.kind' is '$actualKind', expected '$ExpectedVcpkgRegistryKind'. Do not change this to an obsolete/different registry kind to make this check pass - raise an ACR instead."
    }
    if ([string]::IsNullOrWhiteSpace($actualBaseline)) {
        Fail "Could not find a 'default-registry.baseline' value in vcpkg-configuration.json. Refusing to proceed with an unverified baseline."
    }
    if ($actualBaseline -ne $ExpectedVcpkgBaseline) {
        Fail "vcpkg-configuration.json 'default-registry.baseline' is '$actualBaseline', expected '$ExpectedVcpkgBaseline'. This is a frozen baseline (IC-002); do not edit vcpkg-configuration.json to make this check pass - raise an ACR instead."
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 4 - Run all five CI-equivalent jobs, each isolated in its own child PowerShell process'
    # -------------------------------------------------------------------
    $jobs = @(
        @{ Name = 'format'; Script = 'scripts\ci\format.ps1' },
        @{ Name = 'configure-build-test'; Script = 'scripts\ci\configure-build-test.ps1' },
        @{ Name = 'static-analysis'; Script = 'scripts\ci\static-analysis.ps1' },
        @{ Name = 'architecture'; Script = 'scripts\ci\architecture.ps1' },
        @{ Name = 'license-inventory'; Script = 'scripts\ci\license-inventory.ps1' }
    )

    $powershellCmd = Get-Command powershell -ErrorAction SilentlyContinue
    if (-not $powershellCmd) {
        Fail 'powershell.exe was not found on PATH. Each CI job is required to run in an isolated child PowerShell process.'
    }

    $results = [ordered]@{}
    foreach ($job in $jobs) {
        Write-Section "Job: $($job.Name) ($($job.Script)) - isolated child process"
        $scriptPath = Join-Path $WorktreePath $job.Script
        if (-not (Test-Path $scriptPath)) {
            Fail "Job script not found: '$scriptPath'."
        }
        $childArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $scriptPath)
        # -AllowFailure: a throw, exit N, terminating error, or native
        # failure inside this job's own child process cannot terminate
        # this parent runbook - only that child process's own exit code,
        # captured independently here, is affected.
        $jobResult = Invoke-Native -Exe $powershellCmd.Source -CmdArgs $childArgs -AllowFailure
        $results[$job.Name] = $jobResult.ExitCode
        Write-Host ''
        if ($jobResult.ExitCode -eq 0) {
            Write-Host "Job '$($job.Name)' PASSED (child process exit 0)." -ForegroundColor Green
        } else {
            Write-Host "Job '$($job.Name)' FAILED (child process exit $($jobResult.ExitCode)). Continuing to the next job for maximum evidence - this parent process was not affected." -ForegroundColor Red
        }
    }

    $configureBuildTestExit = $results['configure-build-test']
    $buildDir = Join-Path $WorktreePath "build\$Preset"

    # -------------------------------------------------------------------
    Write-Section 'Step 5 - Verify effective CMake configuration - ENFORCED GATE (BLOCKER)'
    # -------------------------------------------------------------------
    $cmakeCachePath = Join-Path $buildDir 'CMakeCache.txt'
    $cmakeCacheExists = Test-Path $cmakeCachePath

    $verifiedTriplet = $null
    $verifiedGenerator = $null
    if ($cmakeCacheExists) {
        $verifiedTriplet = Get-CMakeCacheValue -CachePath $cmakeCachePath -VarName 'VCPKG_TARGET_TRIPLET'
        $verifiedGenerator = Get-CMakeCacheValue -CachePath $cmakeCachePath -VarName 'CMAKE_GENERATOR'
    }

    Write-Host "cmake_cache_exists                 = $cmakeCacheExists"
    Write-Host "expected_vcpkg_triplet             = $Triplet"
    Write-Host "verified_effective_vcpkg_triplet   = $verifiedTriplet"
    Write-Host "expected_cmake_generator           = Ninja"
    Write-Host "verified_effective_cmake_generator = $verifiedGenerator"

    if ($configureBuildTestExit -eq 0) {
        # BLOCKER (v1.2): for a successful configure-build-test job, the
        # effective configuration MUST match - this is an enforced gate
        # that contributes to final verification failure, not a warning.
        if (-not $cmakeCacheExists) {
            $verificationFailures.Add("configure-build-test reported success (exit 0) but CMakeCache.txt was not found at '$cmakeCachePath'")
        }
        if ($verifiedTriplet -ne $Triplet) {
            $verificationFailures.Add("configure-build-test reported success (exit 0) but effective VCPKG_TARGET_TRIPLET was '$verifiedTriplet', expected '$Triplet'")
        }
        if ($verifiedGenerator -ne 'Ninja') {
            $verificationFailures.Add("configure-build-test reported success (exit 0) but effective CMAKE_GENERATOR was '$verifiedGenerator', expected 'Ninja'")
        }
    } else {
        Write-Host "configure-build-test did not succeed (exit $configureBuildTestExit) - effective configuration reported above for evidence only; this gate applies only to a job that reported success." -ForegroundColor Yellow
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 6 - Resolved dependency versions (vcpkg list) - AUTHORITATIVE EVIDENCE GATE (MAJOR)'
    # -------------------------------------------------------------------
    $installRoot = Join-Path $buildDir 'vcpkg_installed'
    $dependencyVersions = [ordered]@{
        opencascade = $null
        catch2      = $null
        sqlite3     = $null
        fmt         = $null
        spdlog      = $null
    }

    if ($configureBuildTestExit -eq 0) {
        if (-not (Test-Path $installRoot)) {
            $verificationFailures.Add("configure-build-test reported success (exit 0) but the expected vcpkg install root '$installRoot' does not exist")
        } else {
            $vcpkgListResult = Invoke-Native -Exe $vcpkgExe -CmdArgs @('list', "--x-install-root=$installRoot") -AllowFailure
            if ($vcpkgListResult.ExitCode -ne 0) {
                $verificationFailures.Add("'vcpkg list --x-install-root=$installRoot' failed (exit $($vcpkgListResult.ExitCode)) despite configure-build-test reporting success")
            } else {
                $listLines = @()
                if ($vcpkgListResult.Stdout) { $listLines = $vcpkgListResult.Stdout -split "`n" }

                foreach ($port in @('opencascade', 'catch2', 'sqlite3', 'fmt', 'spdlog')) {
                    $dependencyVersions[$port] = Get-VcpkgListVersion -Lines $listLines -PortName $port
                }

                Write-Host ''
                Write-Host 'Resolved direct dependency versions (from vcpkg list, authoritative):'
                foreach ($port in $dependencyVersions.Keys) {
                    Write-Host ("  resolved_{0,-12}= {1}" -f $port, $dependencyVersions[$port])
                }

                if ([string]::IsNullOrWhiteSpace($dependencyVersions['opencascade'])) {
                    $verificationFailures.Add('opencascade not found in vcpkg list output')
                } elseif ($dependencyVersions['opencascade'].Split('#')[0] -ne $ExpectedOpencascadeVersion) {
                    $verificationFailures.Add("opencascade resolved to '$($dependencyVersions['opencascade'])', expected version $ExpectedOpencascadeVersion")
                }

                if ([string]::IsNullOrWhiteSpace($dependencyVersions['catch2'])) {
                    $verificationFailures.Add('catch2 not found in vcpkg list output')
                } else {
                    $catch2Major = $dependencyVersions['catch2'].Split('#')[0].Split('.')[0]
                    if ($catch2Major -ne $ExpectedCatch2Major) {
                        $verificationFailures.Add("catch2 resolved to '$($dependencyVersions['catch2'])', expected major version $ExpectedCatch2Major")
                    }
                }

                # sqlite3/fmt/spdlog: presence is required; no exact version
                # was specified by the Architecture Gate/Implementation
                # Brief for these three, so none is asserted here - only
                # their actual resolved version is recorded as evidence.
                foreach ($port in @('sqlite3', 'fmt', 'spdlog')) {
                    if ([string]::IsNullOrWhiteSpace($dependencyVersions[$port])) {
                        $verificationFailures.Add("$port not found in vcpkg list output")
                    }
                }
            }
        }
    } else {
        Write-Host "configure-build-test did not succeed (exit $configureBuildTestExit) - vcpkg list evidence is not applicable this run; this gate applies only when configure-build-test succeeds." -ForegroundColor Yellow
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 7 - Final Git invariants'
    # -------------------------------------------------------------------
    Set-Location $WorktreePath
    $finalTaskBranch = git rev-parse --abbrev-ref HEAD
    $finalTaskHead = git rev-parse HEAD
    Write-Host "Task branch (final) : $finalTaskBranch"
    Write-Host "Task HEAD (final)   : $finalTaskHead"
    Write-Host 'Task worktree status (final) - informational only, NOT required to be clean (evidence/build/license outputs are expected to leave it dirty):'
    git status --short
    Write-Host ''
    Write-Host 'git worktree list (from worktree):'
    git worktree list

    $finalMainBranch = $null
    $finalMainHead = $null
    $finalMainStatus = $null
    if (Test-Path $MainRepoPath) {
        Push-Location $MainRepoPath
        try {
            $finalMainBranch = git rev-parse --abbrev-ref HEAD
            $finalMainHead = git rev-parse HEAD
            $finalMainStatus = git status --short
            Write-Host ''
            Write-Host "Main path       : $MainRepoPath"
            Write-Host "Main branch     : $finalMainBranch"
            Write-Host "Main HEAD       : $finalMainHead"
            Write-Host 'Main status     : (must be blank - proves main was untouched by this run)'
            git status --short
        } finally {
            Pop-Location
        }
    } else {
        Write-Host "WARNING: main repository path '$MainRepoPath' was not found from this runbook's vantage point." -ForegroundColor Yellow
    }

    if ($finalTaskBranch -ne $ExpectedBranch) { $verificationFailures.Add("task branch changed to '$finalTaskBranch' (expected '$ExpectedBranch')") }
    if ($finalTaskHead -ne $ExpectedBootstrapSha) { $verificationFailures.Add("task HEAD changed to '$finalTaskHead' (expected '$ExpectedBootstrapSha')") }
    if ($finalMainBranch -ne 'main') { $verificationFailures.Add("main branch is '$finalMainBranch' (expected 'main')") }
    if ($finalMainHead -ne $ExpectedBootstrapSha) { $verificationFailures.Add("main HEAD is '$finalMainHead' (expected '$ExpectedBootstrapSha')") }
    if (-not [string]::IsNullOrEmpty($finalMainStatus)) { $verificationFailures.Add('main working tree is not clean') }

    Write-Host ''
    if ($verificationFailures.Count -gt 0) {
        Write-Host 'VERIFICATION GATE / INVARIANT VIOLATIONS SO FAR:' -ForegroundColor Red
        foreach ($f in $verificationFailures) { Write-Host "  - $f" -ForegroundColor Red }
    } else {
        Write-Host 'All Git invariants and evidence gates held so far: task branch/HEAD unchanged; main branch/HEAD/clean-status unchanged; CMake config gate; vcpkg-list evidence gate.' -ForegroundColor Green
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 8 - EVIDENCE SUMMARY'
    # -------------------------------------------------------------------
    Write-Host "preset                                = $Preset"
    Write-Host "expected_vcpkg_triplet                = $Triplet"
    Write-Host "verified_effective_vcpkg_triplet      = $verifiedTriplet"
    Write-Host "expected_cmake_generator              = Ninja"
    Write-Host "verified_effective_cmake_generator    = $verifiedGenerator"
    Write-Host "expected_vcpkg_registry_kind          = $ExpectedVcpkgRegistryKind"
    Write-Host "verified_effective_vcpkg_registry_kind= $actualKind"
    Write-Host "expected_vcpkg_baseline               = $ExpectedVcpkgBaseline"
    Write-Host "verified_effective_vcpkg_baseline     = $actualBaseline"
    Write-Host "expected_bootstrap_sha                = $ExpectedBootstrapSha"
    Write-Host "task_branch (preflight -> final)      = $branch -> $finalTaskBranch"
    Write-Host "task_head (preflight -> final)        = $head -> $finalTaskHead"
    Write-Host "expected_vs_line                      = $ExpectedVsLine"
    Write-Host "vscmd_ver (verified)                  = $vscmdVer"
    Write-Host "vscmd_arg_tgt_arch (verified)         = $vscmdArgTgtArch"
    Write-Host "expected_vctools_family_pattern       = $ExpectedVcToolsFamilyPattern (v143/VS2022)"
    Write-Host "vctools_version (verified)            = $vcToolsVersion"
    Write-Host "cl_exe_path                           = $($clCmd.Source)"
    Write-Host "cl_exe_banner                         = $clBanner"
    Write-Host "expected_cmake_version                = $ExpectedCMakeVersion"
    Write-Host "verified_effective_cmake_version      = $actualCMakeVersion"
    Write-Host ''
    Write-Host 'Resolved direct dependency versions (authoritative, from vcpkg list):'
    Write-Host "  expected_opencascade_version        = $ExpectedOpencascadeVersion"
    Write-Host "  resolved_opencascade                = $($dependencyVersions['opencascade'])"
    Write-Host "  expected_catch2_major                = $ExpectedCatch2Major"
    Write-Host "  resolved_catch2                      = $($dependencyVersions['catch2'])"
    Write-Host "  resolved_sqlite3 (no exact version required, presence only) = $($dependencyVersions['sqlite3'])"
    Write-Host "  resolved_fmt (no exact version required, presence only)     = $($dependencyVersions['fmt'])"
    Write-Host "  resolved_spdlog (no exact version required, presence only)  = $($dependencyVersions['spdlog'])"
    Write-Host ''
    foreach ($name in $results.Keys) {
        $code = $results[$name]
        Write-Host ("job_{0,-22}= exit {1}" -f $name, $code)
    }
    Write-Host "verification_gate_and_invariant_violations = $($verificationFailures.Count)"

    $anyJobFailed = @($results.Values | Where-Object { $_ -ne 0 }).Count -gt 0
    $anyVerificationFailed = $verificationFailures.Count -gt 0
    $anyFailed = $anyJobFailed -or $anyVerificationFailed

    Write-Host ''
    if ($anyFailed) {
        Write-Host 'VERIFICATION RUNBOOK B COMPLETE - ONE OR MORE JOBS OR VERIFICATION GATES FAILED. See EVIDENCE SUMMARY / violations above.' -ForegroundColor Red
        Stop-Transcript | Out-Null
        $script:TranscriptStarted = $false
        exit 1
    } else {
        Write-Host 'VERIFICATION RUNBOOK B COMPLETE - ALL FIVE JOBS PASSED AND ALL VERIFICATION GATES HELD.' -ForegroundColor Green
        Stop-Transcript | Out-Null
        $script:TranscriptStarted = $false
        exit 0
    }

} catch {
    Write-Host ''
    Write-Host "RUNBOOK B ABORTED - unhandled error: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host $_.ScriptStackTrace -ForegroundColor DarkRed
    if ($script:TranscriptStarted) { Stop-Transcript | Out-Null }
    exit 1
}
