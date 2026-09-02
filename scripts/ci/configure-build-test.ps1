<#
==============================================================================
 scripts/ci/configure-build-test.ps1 - Windows MSVC configure/build/CTest job
 (Implementation Brief Phase L, logical job 2 of 5).

 Requires: an active x64 MSVC v143 developer environment (cl.exe on PATH,
 e.g. "x64 Native Tools Command Prompt for VS 2022" or "Developer PowerShell
 for VS 2022") and a VCPKG_ROOT environment variable pointing at a local
 vcpkg checkout. Uses the ci-win-msvc preset (RelWithDebInfo, first-party
 warnings as errors). Always configures into a FRESH build directory
 (Implementation Brief Phase N: "Do not use a previously dirty build
 directory as final evidence. Delete/recreate the final evidence build
 directory or use a new one.").

 v1.7 correction ("P0-T001 Architecture Authority Correction Directive",
 finding F-02 - EFFECTIVE COMPILER BASELINE ENFORCEMENT GAP; revised per
 "P0-T001 Architecture Authority Review" BLOCKER R3-B01, then BLOCKER
 R3-B02): a completed verification run proved that, under Ninja, CMake's
 own compiler auto-detection can resolve CMAKE_CXX_COMPILER to a
 Clang-based toolchain (CMAKE_CXX_COMPILER_ID = Clang,
 CMAKE_CXX_COMPILER_VERSION = 19.1.5, CMAKE_CXX_SIMULATE_ID = MSVC - i.e.
 clang-cl/clang++ simulating MSVC) even inside an active VS 2022 v143
 developer shell, because this repository's presets did not previously
 pin CMAKE_C_COMPILER/CMAKE_CXX_COMPILER to "cl" explicitly (see
 CMakePresets.json conf-common). CMakePresets.json now pins both to the
 literal command name "cl" (resolved via PATH inside the active developer
 shell - not a hard-coded absolute path), which is the primary fix.

 This script adds an independent, second-line-of-defense check. BLOCKER
 R3-B01 (Architecture Authority review of the first version of this
 correction) identified that CMake does NOT write CMAKE_CXX_COMPILER_ID,
 CMAKE_CXX_COMPILER_VERSION, or CMAKE_CXX_SIMULATE_ID into CMakeCache.txt
 - CMakeCache.txt only carries CMAKE_CXX_COMPILER:FILEPATH=<path>. Those
 three identity fields are instead written by CMake into the generated
 per-configure file CMakeFiles\<cmake-version>\CMakeCXXCompiler.cmake as
 plain `set(VAR "value")` statements. BLOCKER R3-B02 then identified that
 an unrestricted recursive search for that filename under the build
 directory is non-deterministic: nested CMakeScratch/TryCompile-style
 generated trees can in principle contain their own same-named file, and
 blindly taking the first recursive match risks picking up the wrong one.
 Corrected design: after configure succeeds and before build is allowed
 to proceed, this script discovers ONLY files of the exact form
 CMakeFiles\<single-version-directory>\CMakeCXXCompiler.cmake - it lists
 the immediate (non-recursive) subdirectories of CMakeFiles and checks
 each one's own immediate child for that exact filename; it does not
 descend below that version directory. The "<cmake-version>" segment
 (e.g. "4.4.2") itself is discovered at runtime, not hard-coded, so an
 approved future CMake point-release does not silently break this gate.
 If zero such top-level files are found, that is treated as "no
 authoritative compiler evidence" (see below); if more than one is found,
 discovery is refused as ambiguous rather than silently selecting the
 first match - both are distinct, reported outcomes, never silently
 resolved. Once exactly one such file is found, this script parses
 CMAKE_CXX_COMPILER, CMAKE_CXX_COMPILER_ID, CMAKE_CXX_COMPILER_VERSION,
 and CMAKE_CXX_SIMULATE_ID out of it with an exact-variable-name-anchored
 regex (so e.g. CMAKE_CXX_COMPILER does not accidentally match the
 CMAKE_CXX_COMPILER_ID line). It then requires BOTH, independently:
   (a) CMAKE_CXX_COMPILER_ID is exactly "MSVC" - CMAKE_CXX_SIMULATE_ID is
       captured and printed as evidence only and never participates
       positively in satisfying this check, so a Clang/clang-cl/clang++
       compiler that merely reports CMAKE_CXX_SIMULATE_ID=MSVC does NOT
       satisfy it; AND
   (b) the effective CMAKE_CXX_COMPILER path, normalized and compared
       case-insensitively, refers to the SAME executable as the already-
       validated Step-2-equivalent $clCmd.Source (Get-Command cl,
       resolved earlier in this same script) - a strong executable-path
       proof preventing a different compiler from satisfying the gate
       merely because some cl.exe happens to be found by CMake while a
       different cl.exe was the one this script itself validated.
 Both conditions must hold; either one failing throws and aborts this job
 non-zero. This closes the gap where this script previously only checked
 that a command named "cl" exists on PATH (Get-Command cl), which proves
 neither what CMake actually selected nor that they are the same binary.

 Exit code: 0 only if configure (incl. the effective-compiler-identity
 AND effective-compiler-path gates below), build, AND ctest all succeed.

 Usage: powershell -File scripts\ci\configure-build-test.ps1
==============================================================================
#>

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_common.ps1')

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$Preset = 'ci-win-msvc'
$BuildDir = Join-Path $RepoRoot "build\$Preset"
$ExpectedEffectiveCxxCompilerId = 'MSVC'

function Find-CMakeCxxCompilerInfoFile {
    # CMake writes compiler-identity metadata (CMAKE_CXX_COMPILER_ID,
    # CMAKE_CXX_COMPILER_VERSION, CMAKE_CXX_SIMULATE_ID) into the generated
    # per-configure file CMakeFiles\<cmake-version>\CMakeCXXCompiler.cmake,
    # NOT into CMakeCache.txt (BLOCKER R3-B01).
    #
    # Corrected per BLOCKER R3-B02: discovery must be DETERMINISTIC and
    # must verify only the TOP-LEVEL configure metadata - an unrestricted
    # recursive search under CMakeFiles could in principle match a
    # same-named file inside a nested CMakeScratch/TryCompile-style
    # generated tree. This function therefore lists only the IMMEDIATE
    # (non-recursive) subdirectories of CMakeFiles and checks each one's
    # own immediate child for the exact filename CMakeCXXCompiler.cmake -
    # it never descends below that single version directory. The
    # "<cmake-version>" segment (e.g. "4.4.2") is discovered at runtime,
    # not hard-coded, so an approved future CMake point-release does not
    # silently break this gate.
    #
    # Returns a status object rather than a bare path/$null, so the three
    # possible outcomes are never conflated:
    #   Status = 'NotFound'   - zero top-level matches: no authoritative
    #                           compiler evidence exists yet (e.g. configure
    #                           itself did not reach the compiler-check
    #                           stage). Path is $null.
    #   Status = 'Ambiguous'  - more than one top-level match: refused as
    #                           ambiguous rather than silently selecting
    #                           the first one. Path is $null; Candidates
    #                           lists every match found.
    #   Status = 'Found'      - exactly one top-level match. Path is its
    #                           full path; VersionDirectory is the
    #                           containing directory's own name (e.g.
    #                           "4.4.2"), for an optional cross-check
    #                           against the locked CMake version.
    param(
        [Parameter(Mandatory)][string]$BuildDirectory
    )
    $cmakeFilesDir = Join-Path $BuildDirectory 'CMakeFiles'
    if (-not (Test-Path $cmakeFilesDir)) {
        return [PSCustomObject]@{ Status = 'NotFound'; Path = $null; VersionDirectory = $null; Candidates = @() }
    }
    $versionDirs = Get-ChildItem -Path $cmakeFilesDir -Directory -ErrorAction SilentlyContinue
    $matches = @()
    foreach ($dir in $versionDirs) {
        $candidatePath = Join-Path $dir.FullName 'CMakeCXXCompiler.cmake'
        if (Test-Path -LiteralPath $candidatePath -PathType Leaf) {
            $matches += [PSCustomObject]@{ Path = $candidatePath; VersionDirectory = $dir.Name }
        }
    }
    if ($matches.Count -eq 0) {
        return [PSCustomObject]@{ Status = 'NotFound'; Path = $null; VersionDirectory = $null; Candidates = @() }
    }
    if ($matches.Count -gt 1) {
        return [PSCustomObject]@{ Status = 'Ambiguous'; Path = $null; VersionDirectory = $null; Candidates = $matches }
    }
    return [PSCustomObject]@{ Status = 'Found'; Path = $matches[0].Path; VersionDirectory = $matches[0].VersionDirectory; Candidates = $matches }
}

function Get-CMakeCxxCompilerInfoValue {
    # Parses a single `set(<VarName> "value")` statement out of a generated
    # CMakeCXXCompiler.cmake file. The variable name is matched exactly
    # (anchored on the following whitespace/paren), so e.g. VarName
    # 'CMAKE_CXX_COMPILER' does not accidentally match the
    # 'CMAKE_CXX_COMPILER_ID' or 'CMAKE_CXX_COMPILER_VERSION' lines.
    param(
        [Parameter(Mandatory)][string]$InfoFilePath,
        [Parameter(Mandatory)][string]$VarName
    )
    if (-not (Test-Path $InfoFilePath)) {
        return $null
    }
    $content = Get-Content -Raw -Path $InfoFilePath
    $pattern = '(?m)^\s*set\(' + [regex]::Escape($VarName) + '\s+"([^"]*)"\)'
    $match = [regex]::Match($content, $pattern)
    if ($match.Success) {
        return $match.Groups[1].Value
    }
    return $null
}

function Get-NormalizedExecutablePath {
    # Normalizes a Windows executable path for a case-insensitive identity
    # comparison: resolves to a full path (handling both forward- and
    # back-slash forms, since CMake-generated files use forward slashes)
    # and trims any trailing separator. Falls back to a simple lowercase
    # comparison of the original string if full-path resolution fails
    # (e.g. the path does not exist from this vantage point), rather than
    # silently treating two different-looking paths as equal.
    param(
        [string]$Path
    )
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }
    try {
        $full = [System.IO.Path]::GetFullPath($Path)
        return $full.TrimEnd('\', '/').ToLowerInvariant()
    } catch {
        return $Path.Trim().ToLowerInvariant()
    }
}

try {
    Write-CiSection 'configure-build-test: preconditions'
    Assert-CiEnvVar -Name 'VCPKG_ROOT' | Out-Null

    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCmd) { throw 'cmake was not found on PATH.' }

    $ctestCmd = Get-Command ctest -ErrorAction SilentlyContinue
    if (-not $ctestCmd) { throw 'ctest was not found on PATH.' }

    $clCmd = Get-Command cl -ErrorAction SilentlyContinue
    if (-not $clCmd) {
        throw 'cl.exe was not found on PATH. Run this script from an x64 Native Tools / Developer PowerShell for VS 2022.'
    }

    Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--version') | Out-Null

    Write-CiSection "configure-build-test: fresh build directory ($BuildDir)"
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }

    Push-Location $RepoRoot
    try {
        Write-CiSection "configure-build-test: cmake --preset $Preset"
        Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--preset', $Preset)

        Write-CiSection 'configure-build-test: verify compile_commands.json was generated'
        $compileDbPath = Join-Path $BuildDir 'compile_commands.json'
        if (-not (Test-Path $compileDbPath)) {
            throw "compile_commands.json was not generated at '$compileDbPath' after configure. CMAKE_EXPORT_COMPILE_COMMANDS=ON is required (see CMakePresets.json conf-common cacheVariables) - this is required evidence for scripts\ci\static-analysis.ps1, not merely a nice-to-have."
        }
        Write-Host "compile_commands.json present at '$compileDbPath'."

        Write-CiSection 'configure-build-test: verify effective CMake compiler identity and executable path (F-02 enforcement gate, corrected per BLOCKER R3-B01/R3-B02)'
        $cxxCompilerInfo = Find-CMakeCxxCompilerInfoFile -BuildDirectory $BuildDir
        if ($cxxCompilerInfo.Status -eq 'Ambiguous') {
            $candidateList = ($cxxCompilerInfo.Candidates | ForEach-Object { $_.Path }) -join '; '
            throw "Found more than one top-level CMakeFiles\<version>\CMakeCXXCompiler.cmake under '$BuildDir' - refusing to guess which one is authoritative (BLOCKER R3-B02: discovery must be deterministic, never select the first match). Candidates: $candidateList"
        }
        if ($cxxCompilerInfo.Status -eq 'NotFound') {
            throw "Could not locate a top-level CMakeFiles\<cmake-version>\CMakeCXXCompiler.cmake file under '$BuildDir' after a reported-successful configure. Cannot verify the effective compiler identity."
        }
        $cxxCompilerInfoFile = $cxxCompilerInfo.Path
        Write-Host "cxx_compiler_info_file (located)      = $cxxCompilerInfoFile"
        Write-Host "cxx_compiler_info_version_directory   = $($cxxCompilerInfo.VersionDirectory)"

        $effectiveCxxCompiler = Get-CMakeCxxCompilerInfoValue -InfoFilePath $cxxCompilerInfoFile -VarName 'CMAKE_CXX_COMPILER'
        $effectiveCxxCompilerId = Get-CMakeCxxCompilerInfoValue -InfoFilePath $cxxCompilerInfoFile -VarName 'CMAKE_CXX_COMPILER_ID'
        $effectiveCxxCompilerVersion = Get-CMakeCxxCompilerInfoValue -InfoFilePath $cxxCompilerInfoFile -VarName 'CMAKE_CXX_COMPILER_VERSION'
        $effectiveCxxSimulateId = Get-CMakeCxxCompilerInfoValue -InfoFilePath $cxxCompilerInfoFile -VarName 'CMAKE_CXX_SIMULATE_ID'

        $normalizedEffectiveCompiler = Get-NormalizedExecutablePath -Path $effectiveCxxCompiler
        $normalizedValidatedCl = Get-NormalizedExecutablePath -Path $clCmd.Source

        Write-Host "expected_effective_cxx_compiler_id     = $ExpectedEffectiveCxxCompilerId"
        Write-Host "verified_effective_cxx_compiler_id     = $effectiveCxxCompilerId"
        Write-Host "verified_effective_cxx_compiler        = $effectiveCxxCompiler"
        Write-Host "verified_effective_cxx_compiler_ver    = $effectiveCxxCompilerVersion"
        Write-Host "verified_effective_cxx_simulate_id     = $effectiveCxxSimulateId (informational only - a Clang/clang-cl compiler that merely simulates MSVC does NOT satisfy this gate)"
        Write-Host "validated_step2_cl_exe_path            = $($clCmd.Source)"
        Write-Host "normalized_effective_cxx_compiler_path = $normalizedEffectiveCompiler"
        Write-Host "normalized_validated_cl_exe_path       = $normalizedValidatedCl"

        if ([string]::IsNullOrWhiteSpace($effectiveCxxCompilerId)) {
            throw "Could not parse CMAKE_CXX_COMPILER_ID from '$cxxCompilerInfoFile'. Refusing to proceed with an unverified compiler identity."
        }
        if ($effectiveCxxCompilerId -ne $ExpectedEffectiveCxxCompilerId) {
            throw "Effective CMAKE_CXX_COMPILER_ID is '$effectiveCxxCompilerId' (compiler executable '$effectiveCxxCompiler', version '$effectiveCxxCompilerVersion', CMAKE_CXX_SIMULATE_ID='$effectiveCxxSimulateId'), expected exactly '$ExpectedEffectiveCxxCompilerId'. The locked P0-T001 baseline requires the real MSVC compiler (cl.exe) - a Clang/clang-cl/clang++ compiler that reports CMAKE_CXX_SIMULATE_ID=MSVC does not satisfy this requirement, even though it may otherwise build successfully. Verify CMakePresets.json's conf-common CMAKE_C_COMPILER/CMAKE_CXX_COMPILER are set to 'cl' and that no earlier-on-PATH clang-cl/clang++ is shadowing it in this developer shell."
        }
        if ([string]::IsNullOrWhiteSpace($normalizedEffectiveCompiler) -or [string]::IsNullOrWhiteSpace($normalizedValidatedCl) -or $normalizedEffectiveCompiler -ne $normalizedValidatedCl) {
            throw "Effective CMAKE_CXX_COMPILER ('$effectiveCxxCompiler') does not refer to the same executable as the validated cl.exe resolved earlier in this script ('$($clCmd.Source)'). CMAKE_CXX_COMPILER_ID reporting 'MSVC' is not sufficient on its own - a different cl.exe (or a differently-pathed compiler) must not silently satisfy this gate."
        }

        Write-CiSection "configure-build-test: cmake --build --preset $Preset"
        Invoke-Native -Exe $cmakeCmd.Source -CmdArgs @('--build', '--preset', $Preset)

        Write-CiSection "configure-build-test: ctest --preset $Preset"
        Invoke-Native -Exe $ctestCmd.Source -CmdArgs @('--preset', $Preset, '--output-on-failure')
    } finally {
        Pop-Location
    }

    Write-Host ''
    Write-Host 'CONFIGURE-BUILD-TEST PASSED' -ForegroundColor Green
    exit 0
} catch {
    Write-Host ''
    Write-Host "configure-build-test.ps1 ABORTED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
