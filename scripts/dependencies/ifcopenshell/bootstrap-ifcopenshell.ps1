#Requires -Version 5.1
<#
.SYNOPSIS
    Controlled, pinned-source dependency bootstrap for IfcOpenShell
    (P0-T005 - IFC Spike). Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0
    section 5 / Execution Packet v1.1 section 11; ACR-P0-T005-001.

.DESCRIPTION
    Clones the EXACT pinned upstream IfcOpenShell commit outside this
    repository, provisions the frozen external Boost/eigen3 support
    dependency closure it needs to compile against (CORR-001 - see
    "External support dependency manifest" below), configures it as a
    static, C++17, IFC4-only, core-parse-only ("IfcParse") build with
    geometry/Python/OpenCascade/CGAL all OFF, builds and installs it to an
    external prefix, and proves the installed IfcOpenShell::IfcParse CMake
    package exists. Verifies (never materializes/writes) the pinned
    IfcOpenShell license evidence against the frozen candidate license
    files already present in third_party/licenses/ (CORR-002 - see
    "License evidence verification" below). Refreshes its own external
    BinaryDir and InstallPrefix before every configure, guarded by explicit
    deletion-safety checks (CORR-003 - see "Fresh external BinaryDir /
    InstallPrefix" below). Never touches the BIM Platform source tree with
    vendored upstream source or binaries beyond that license verification;
    never uses FetchContent or a Git submodule; never falls back to a
    global system install.

    Safe to re-run: if the pinned commit is already cloned, the clone-reuse
    step is skipped unless -Force is passed. BinaryDir/InstallPrefix are
    ALWAYS freshly deleted-and-recreated on every run regardless of -Force
    (CORR-003 - see below); SourceDir, SupportManifestDir, and
    SupportInstalledDir are never touched by that refresh. A commit-identity
    mismatch at any point is a hard failure, not a silent continue.

    External support dependency manifest (CORR-001 - BIM-AA-P0-T005-CORR-001
    v1.0): AA's own Windows validation (Full Candidate Validation v1.2)
    showed that binding IfcOpenShell's CMake configure directly to this
    repository's own vcpkg toolchain file does NOT provision Boost for
    IfcOpenShell's separate/external configure - vcpkg's manifest discovery
    resolves relative to the CMake source directory being configured
    (IfcOpenShell's own <repo>\cmake, which has no vcpkg.json of its own),
    not relative to the toolchain file's location, so no manifest was ever
    found and nothing was installed ("Could NOT find Boost ..."). This
    script now generates its OWN runtime, external vcpkg manifest - under
    <BuildRoot>\support-manifest, outside this repository's tracked source
    and containing ONLY the frozen Phase C boost-*/eigen3 closure (never
    this repository's own vcpkg.json, which would pull unrelated BIM
    Platform dependencies such as OCCT/Qt/bgfx/SQLite) - installs it
    non-interactively to an external install root
    (<BuildRoot>\vcpkg_installed) via an explicit `vcpkg install`
    invocation using the SAME vcpkg install this repository's toolchain
    file belongs to, and then explicitly binds IfcOpenShell's CMake
    configure to that external manifest/install root via
    VCPKG_MANIFEST_DIR / VCPKG_INSTALLED_DIR / VCPKG_TARGET_TRIPLET (with
    VCPKG_MANIFEST_INSTALL=OFF, since installation already happened as its
    own explicit, deterministic step above - CMake configure only consumes
    the already-installed packages, it never triggers a second implicit
    install). The frozen vcpkg baseline commit
    (f89a4a1da4e3176a8d1a14c1825b9b2f98e48843) is unchanged and is pinned
    in the generated vcpkg-configuration.json exactly as it already is for
    this repository's own manifest.

    License evidence verification (CORR-002 - BIM-AA-P0-T005-CORR-002 v1.0):
    an earlier version of this script copied the pinned upstream COPYING /
    COPYING.LESSER files into third_party/licenses/ during its run - AA's
    own validation flagged this as a repository mutation boundary defect,
    since that validation run was meant to be repository-source/index
    read-only. third_party/licenses/ifcopenshell.COPYING.txt and
    ifcopenshell.COPYING.LESSER.txt are now accepted, frozen candidate
    artifacts at their pinned hashes and this script never creates,
    overwrites, or otherwise touches them again. Instead it only verifies:
    the pinned upstream files (read-only, from the external clone) against
    the frozen expected hashes, and separately the two existing repository
    candidate files against those same frozen hashes - failing actionably,
    without writing anything, if either is absent or mismatched.

    Fresh external BinaryDir / InstallPrefix (CORR-003 -
    BIM-AA-P0-T005-CORR-003 v1.0): AA's Full Candidate Validation v1.3
    proved CORR-001/CORR-002 work end-to-end (exact clone reused, license
    hashes verified, support manifest generated and installed), then hit
    "vcpkg manifest mode was enabled for a build directory where it was
    initially disabled. This is not supported. Please delete the build
    directory and reconfigure." - because the external BinaryDir
    (<BuildRoot>\build) still carried CMake/vcpkg cache state from an
    earlier, pre-CORR-001 configure (when manifest mode was not bound), and
    reusing it across that configuration-mode transition is not supported.
    This script now deletes-if-present and recreates BOTH BinaryDir and
    InstallPrefix immediately before every configure, guarded by an
    explicit deletion-safety check (Assert-SafeToDeleteExternalPath) that
    fails closed unless the target resolves beneath BuildRoot and is none
    of: BuildRoot itself, SourceDir, SupportManifestDir,
    SupportInstalledDir, or anything inside the BIM Platform repository
    (D:\Projects\BIM-Platform or D:\Projects\BIM-Platform-WT-P0-T005). The
    exact pinned SourceDir and the CORR-001 external support
    manifest/install are never deleted by this refresh - only the
    bootstrap's own disposable CMake binary directory and install prefix.

.PARAMETER BuildRoot
    Short external build root, OUTSIDE this repository (brief section 5
    item 3: "use a short external path" - avoids the long-path build
    failures the prior %LOCALAPPDATA%\P0T5C\... evidence path already
    worked around once; see ACR/Brief provenance notes). Default:
    C:\bimdeps\ifc (a short, drive-root-adjacent path deliberately chosen
    to stay well under Windows MAX_PATH even for IfcOpenShell's own deeply
    nested generated-schema source paths).

.PARAMETER InstallPrefix
    External install prefix for the built IfcOpenShell package. Default:
    <BuildRoot>\install. Pass this same path (plus \lib\cmake\IfcOpenShell)
    as -DIfcOpenShell_DIR=... when configuring the BIM Platform build with
    -DBIM_ENABLE_IFC=ON.

.PARAMETER VcpkgToolchainFile
    Path to this repository's own vcpkg.cmake toolchain file. Used to (a)
    locate the vcpkg installation (vcpkg.exe) this script uses to
    explicitly install the external Boost/eigen3 support closure per
    CORR-001, and (b) as the CMAKE_TOOLCHAIN_FILE for IfcOpenShell's own
    configure, so CMake's vcpkg integration is active - never to resolve
    IfcOpenShell itself, which is not vcpkg-managed (ACR-P0-T005-001
    section 1), and never bound to this repository's own vcpkg.json
    manifest (see CORR-001 note in .DESCRIPTION). Required.

.PARAMETER SupportManifestDir
    Runtime-generated, external (outside this repository's tracked source)
    vcpkg manifest directory containing ONLY the frozen P0-T005
    boost-*/eigen3 support closure (CORR-001). Default:
    <BuildRoot>\support-manifest. Regenerated deterministically on every
    run (not hand-maintained; safe to delete/regenerate).

.PARAMETER SupportInstalledDir
    External vcpkg install root the support closure above is installed
    into, separate from this repository's own vcpkg_installed tree
    (CORR-001). Default: <BuildRoot>\vcpkg_installed.

.PARAMETER Triplet
    vcpkg triplet used for both the external support closure install and
    IfcOpenShell's own CMake configure. Default: x64-windows.

.PARAMETER Configuration
    CMake build configuration for the IfcOpenShell dependency build.
    Default: RelWithDebInfo (brief section 4's "Debug/Release/RelWithDebInfo
    implications on Windows x64" - RelWithDebInfo is the default so a debug
    BIM Platform build still gets usable symbols without paying full Debug
    build cost for a third-party dependency built once and reused).

.PARAMETER Force
    Re-clone the pinned SourceDir even if it already appears present and
    matching. Does not force-reinstall the external support closure
    (CORR-001) - `vcpkg install` is already idempotent against unchanged
    manifest content and is always re-invoked (cheaply, as a no-op when
    nothing changed) on every run. Does not affect BinaryDir/InstallPrefix
    refresh (CORR-003) either - those are always freshly deleted and
    recreated on every run, with or without -Force.

.EXAMPLE
    .\bootstrap-ifcopenshell.ps1 -VcpkgToolchainFile D:\Projects\BIM-Platform-WT-P0-T005\vcpkg\scripts\buildsystems\vcpkg.cmake

.NOTES
    RISK DISCLOSURE: this script was authored without a live execution
    channel against the pinned commit in this session (no device_bash on
    the Windows worktree; this cloud sandbox has no internet path to clone
    real upstream Git repositories or a real Visual Studio/MSVC toolchain
    to build against). The git/cmake/msbuild invocations, option names, and
    IfcOpenShell's own repository layout (its top-level CMake entry point
    lives under <repo>\cmake\, not the repo root - a real, but
    not-independently-reverified-this-session, detail of the upstream
    repository structure) are authored at best-effort confidence from the
    accepted Dependency Resolution Phase C proof (Implementation Brief
    section 19) and IfcOpenShell's well-known public build conventions. The
    Windows Execution Operator's first real run of this script is its first
    actual execution - treat a failure here as an ordinary dependency
    -bootstrap defect to fix minimum-delta (Brief section 20 item 6),
    unless it matches one of Brief section 22's stop conditions.

    CORR-001 RISK DISCLOSURE (additional, same session constraints as above -
    still no execution channel against a real vcpkg installation or the
    pinned commit): the derivation of vcpkg.exe's path from
    VcpkgToolchainFile (assuming the conventional
    <vcpkg root>\scripts\buildsystems\vcpkg.cmake layout) and the
    `vcpkg install --x-manifest-root=... --x-install-root=...` invocation
    shape, and the VCPKG_MANIFEST_DIR/VCPKG_INSTALLED_DIR/
    VCPKG_TARGET_TRIPLET/VCPKG_MANIFEST_MODE/VCPKG_MANIFEST_INSTALL CMake
    cache variable names, are authored at best-effort confidence from
    vcpkg's public documentation and were not independently re-verified
    against a real vcpkg binary in this session. Its first real run is on
    the Windows Execution Operator's machine - treat a failure at this step
    the same way: an ordinary dependency-bootstrap defect to fix
    minimum-delta, not a reason to fall back to a global Boost install or to
    point at this repository's own vcpkg.json/vcpkg_installed.

    CORR-002 RISK DISCLOSURE: the verify-only license logic (reading the two
    upstream files from the external clone and the two existing repository
    candidate files, hashing both, never writing either) was authored
    without a live execution channel against a real pinned checkout or a
    real repository working tree in this session. Its first real run is on
    the Windows Execution Operator's machine. If it fails because the
    repository candidate license files are absent or mismatched, treat that
    as evidence to investigate (e.g. the accepted candidate footprint was
    not actually present on disk) - never as a reason to have this script
    recreate or overwrite them; that is exactly the repository-mutation
    behavior CORR-002 removed.

    CORR-003 RISK DISCLOSURE: the BinaryDir/InstallPrefix fresh-refresh
    logic and its Assert-SafeToDeleteExternalPath deletion-safety guard
    were authored without a live execution channel against a real external
    build root, a real vcpkg/CMake cache-mode transition, or a real
    repository working tree in this session - same session constraints as
    above. Its first real run is on the Windows Execution Operator's
    machine. The guard's explicit repository-path checks
    (D:\Projects\BIM-Platform, D:\Projects\BIM-Platform-WT-P0-T005) are the
    literal paths CORR-003 names; if a real environment uses different
    paths for these, treat a guard rejection as a signal to re-derive the
    guard list minimum-delta from the real environment - never as a reason
    to remove or loosen the guard to make a deletion "succeed".
#>
[CmdletBinding()]
param(
    [string]$BuildRoot = 'C:\bimdeps\ifc',
    [string]$InstallPrefix = "$BuildRoot\install",
    [Parameter(Mandatory = $true)]
    [string]$VcpkgToolchainFile,
    [string]$SupportManifestDir = "$BuildRoot\support-manifest",
    [string]$SupportInstalledDir = "$BuildRoot\vcpkg_installed",
    [string]$Triplet = 'x64-windows',
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Configuration = 'RelWithDebInfo',
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# ------------------------------------------------------------------------------
# Frozen dependency identity (Brief section 3/4; Packet v1.1 section 4).
# Never advance these without a separate Architecture Authority decision.
# ------------------------------------------------------------------------------
$IfcOpenShellRepo = 'https://github.com/IfcOpenShell/IfcOpenShell.git'
$IfcOpenShellRef = 'refs/tags/ifcconvert-0.8.5'
$IfcOpenShellTag = 'ifcconvert-0.8.5'
$ExpectedCommit = '16723d11cab9bc8a13b4e025a00d39445ccc462e'
$ExpectedTree = '3b3e7bd633c14333a07f6c0e498bbf8483d23ad6' # provenance only; not independently re-derivable by this script (no tree-hash tool used here - git itself can report it, see the tree-check step below)

# Pinned upstream license identities the candidate must eventually capture
# under third_party/licenses/** (Brief section 16) - verified against the
# ACTUAL cloned files below, never assumed.
$ExpectedCopyingSha256 = '3237699d9e6781c85877365abd9493ca132a0fd31f43e1a3278b8e75f3654af4'
$ExpectedCopyingLesserSha256 = '7d3a95e5e06978064ed3f8e2b7c8f845e7fd8a405294727cc708f94cb83b8059'

# ------------------------------------------------------------------------------
# CORR-001: frozen EXTERNAL support dependency closure (Boost/eigen3) that
# IfcOpenShell's C++ core needs to compile against. This is the exact same
# closure/baseline Dependency Resolution Phase C already proved (Implementation
# Brief section 19) - never advance the baseline or the package list here
# without a separate Architecture Authority decision.
# ------------------------------------------------------------------------------
$SupportDependencyBaseline = 'f89a4a1da4e3176a8d1a14c1825b9b2f98e48843'
$SupportDependencies = @(
    'boost-system',
    'boost-program-options',
    'boost-regex',
    'boost-thread',
    'boost-date-time',
    'boost-iostreams',
    'boost-uuid',
    'boost-logic',
    'boost-scope-exit',
    'boost-multi-index',
    'boost-circular-buffer',
    'boost-filesystem',
    'boost-locale',
    'boost-math',
    'boost-property-tree',
    'boost-variant',
    'eigen3'
)

function Write-Step {
    param([string]$Message)
    Write-Host "[bootstrap-ifcopenshell] $Message" -ForegroundColor Cyan
}

function Write-FailActionable {
    param([string]$Message)
    Write-Error "[bootstrap-ifcopenshell] STOP: $Message"
    exit 1
}

function Assert-CommandAvailable {
    param([string]$Name, [string]$Hint)
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        Write-FailActionable "'$Name' was not found on PATH. $Hint"
    }
}

# CORR-003 (BIM-AA-P0-T005-CORR-003 v1.0) deletion safety guard: fails closed
# unless the target resolves beneath BuildRoot and is none of the paths this
# script must never delete (BuildRoot itself, SourceDir, SupportManifestDir,
# SupportInstalledDir, or anything inside the BIM Platform repository).
# Deliberately narrow - never a broad deletion rooted at C:\bimdeps,
# %LOCALAPPDATA%, the repository, or the source clone.
function Assert-SafeToDeleteExternalPath {
    param([string]$Path, [string]$Label)

    $resolved = [System.IO.Path]::GetFullPath($Path)
    $resolvedBuildRoot = [System.IO.Path]::GetFullPath($BuildRoot)
    $resolvedSourceDir = [System.IO.Path]::GetFullPath($SourceDir)
    $resolvedSupportManifestDir = [System.IO.Path]::GetFullPath($SupportManifestDir)
    $resolvedSupportInstalledDir = [System.IO.Path]::GetFullPath($SupportInstalledDir)
    $buildRootPrefix = $resolvedBuildRoot.TrimEnd('\') + '\'

    # Repository guards named explicitly by CORR-003 section 4 - this script
    # must never delete anything inside either of these, regardless of what
    # BuildRoot/BinaryDir/InstallPrefix are passed as.
    $repoGuards = @('D:\Projects\BIM-Platform', 'D:\Projects\BIM-Platform-WT-P0-T005')

    if (-not $resolved.StartsWith($buildRootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it does not resolve beneath the configured BuildRoot ('$resolvedBuildRoot'). This script only ever recursively deletes bootstrap-owned descendants of BuildRoot."
    }
    if ($resolved -ieq $resolvedBuildRoot) {
        Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it IS BuildRoot itself, not a bootstrap-owned descendant of it."
    }
    if ($resolved -ieq $resolvedSourceDir) {
        Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it is the pinned exact-commit SourceDir, which this script preserves/reuses and never deletes as part of a binary-dir/install-prefix refresh."
    }
    if ($resolved -ieq $resolvedSupportManifestDir) {
        Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it is SupportManifestDir (CORR-001), which this script preserves/regenerates in place and never deletes wholesale."
    }
    if ($resolved -ieq $resolvedSupportInstalledDir) {
        Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it is SupportInstalledDir (CORR-001's restored/installed support dependency closure), which this script preserves/reuses and never deletes."
    }
    foreach ($guard in $repoGuards) {
        $guardPrefix = $guard.TrimEnd('\') + '\'
        if (($resolved -ieq $guard) -or $resolved.StartsWith($guardPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            Write-FailActionable "CORR-003 safety guard: refusing to delete $Label ('$resolved') - it is inside (or equal to) '$guard'. This script must never delete anything inside the BIM Platform repository."
        }
    }
}

# ------------------------------------------------------------------------------
# 0. Preconditions
# ------------------------------------------------------------------------------
Assert-CommandAvailable -Name 'git' -Hint 'Install Git for Windows and ensure it is on PATH.'
Assert-CommandAvailable -Name 'cmake' -Hint 'Install CMake (matching the repository CI reference version) and ensure it is on PATH.'

if (-not (Test-Path -LiteralPath $VcpkgToolchainFile -PathType Leaf)) {
    Write-FailActionable "VcpkgToolchainFile '$VcpkgToolchainFile' does not exist. Pass the path to this repository's own vcpkg.cmake toolchain (its vcpkg installation is what this script uses to install the CORR-001 external boost-*/eigen3 support closure, and it also supplies the CMake vcpkg integration used to configure IfcOpenShell itself)."
}

# The vcpkg toolchain file conventionally lives at
# <vcpkg root>\scripts\buildsystems\vcpkg.cmake - derive the vcpkg root and
# locate vcpkg.exe there (RISK DISCLOSURE: this layout assumption is not
# independently re-verified in this session; if a real vcpkg install uses a
# non-standard toolchain file location, correct this derivation minimum-delta
# rather than working around it).
$VcpkgRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $VcpkgToolchainFile))
$VcpkgExe = Join-Path $VcpkgRoot 'vcpkg.exe'
if (-not (Test-Path -LiteralPath $VcpkgExe -PathType Leaf)) {
    Write-FailActionable "Could not locate vcpkg.exe at the derived path '$VcpkgExe' (derived from VcpkgToolchainFile assuming the conventional <vcpkg root>\scripts\buildsystems\vcpkg.cmake layout). CORR-001's external support dependency install requires the actual vcpkg executable - inspect the real vcpkg installation and correct this derivation minimum-delta if its layout differs."
}

New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null
$SourceDir = Join-Path $BuildRoot 'src'
$BinaryDir = Join-Path $BuildRoot 'build'

Write-Step "BuildRoot            = $BuildRoot"
Write-Step "SourceDir            = $SourceDir"
Write-Step "BinaryDir            = $BinaryDir"
Write-Step "InstallPrefix        = $InstallPrefix"
Write-Step "SupportManifestDir   = $SupportManifestDir"
Write-Step "SupportInstalledDir  = $SupportInstalledDir"
Write-Step "Triplet              = $Triplet"
Write-Step "VcpkgExe             = $VcpkgExe"
Write-Step "Configuration        = $Configuration"
Write-Step "Pinned commit        = $ExpectedCommit ($IfcOpenShellRef)"

# ------------------------------------------------------------------------------
# 1. Resolve exact pinned source (clone once; safe to re-run; reject mismatch)
# ------------------------------------------------------------------------------
$needClone = $true
if ((Test-Path -LiteralPath (Join-Path $SourceDir '.git')) -and -not $Force) {
    Push-Location $SourceDir
    try {
        $currentCommit = (git rev-parse HEAD).Trim()
    } finally {
        Pop-Location
    }
    if ($currentCommit -eq $ExpectedCommit) {
        Write-Step "Existing clone at $SourceDir already matches the pinned commit - skipping re-clone (pass -Force to redo it anyway)."
        $needClone = $false
    } else {
        Write-FailActionable "Existing clone at $SourceDir is at commit '$currentCommit', which does NOT match the pinned commit '$ExpectedCommit'. Re-run with -Force to discard and re-clone the pinned commit, or investigate before proceeding - do not silently build a moving/unpinned checkout."
    }
}

if ($needClone) {
    if (Test-Path -LiteralPath $SourceDir) {
        Write-Step "Removing existing (stale/-Force'd) source directory before re-clone: $SourceDir"
        Remove-Item -LiteralPath $SourceDir -Recurse -Force
    }
    Write-Step "Cloning $IfcOpenShellRepo @ $IfcOpenShellTag (shallow, single tag) into $SourceDir ..."
    git clone --branch $IfcOpenShellTag --depth 1 $IfcOpenShellRepo $SourceDir
    if ($LASTEXITCODE -ne 0) {
        Write-FailActionable "git clone failed (exit $LASTEXITCODE)."
    }

    Push-Location $SourceDir
    try {
        $clonedCommit = (git rev-parse HEAD).Trim()
    } finally {
        Pop-Location
    }
    if ($clonedCommit -ne $ExpectedCommit) {
        Write-FailActionable "Cloned tag '$IfcOpenShellTag' resolved to commit '$clonedCommit', which does NOT match the pinned/expected commit '$ExpectedCommit'. This means upstream moved the tag, or the pinned identity in the Implementation Brief is stale - STOP and return to Architecture Authority (Brief section 22: 'exact pinned IfcOpenShell commit cannot be reproduced'). Do not proceed with this checkout."
    }
    Write-Step "Verified cloned commit matches pinned identity exactly: $clonedCommit"
}

# ------------------------------------------------------------------------------
# 2. License evidence verification (Brief section 16; corrected by CORR-002
#    section 4 - VERIFY-ONLY). The two frozen candidate license files
#    (third_party/licenses/ifcopenshell.COPYING.txt and
#    ifcopenshell.COPYING.LESSER.txt) are now an accepted, frozen part of the
#    candidate footprint (AA License Materialization Triage v1 /
#    CORR-002 section 1) - this script must never create, overwrite,
#    normalize, or otherwise mutate them, or any other repository file,
#    during what is meant to be a repository-source/index read-only
#    validation run (CORR-002 section 2/6). It only reads the pinned
#    upstream files (from the external clone) and the existing repository
#    candidate files, and verifies both against the same frozen hashes -
#    it writes NO repository file.
# ------------------------------------------------------------------------------
$licensesRepoDir = Join-Path (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))) 'third_party\licenses'
# ($PSScriptRoot = scripts\dependencies\ifcopenshell; three levels up = repo root)

function Assert-UpstreamLicenseHash {
    param([string]$SourceFileName, [string]$ExpectedSha256)
    $src = Join-Path $SourceDir $SourceFileName
    if (-not (Test-Path -LiteralPath $src -PathType Leaf)) {
        Write-FailActionable "Expected upstream license file '$SourceFileName' was not found in the pinned checkout ($src). License provenance is uncertain - STOP per Brief section 22."
    }
    $actualHash = (Get-FileHash -LiteralPath $src -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualHash -ne $ExpectedSha256) {
        Write-FailActionable "Upstream '$SourceFileName' SHA256 ($actualHash) does NOT match the pinned identity ($ExpectedSha256) recorded in Implementation Brief section 16. Do not proceed - license provenance is uncertain (Brief section 22)."
    }
    Write-Step "Verified upstream $SourceFileName matches pinned identity (SHA256 $actualHash) - read-only, not copied into the repository."
}

function Assert-RepositoryLicenseHash {
    param([string]$DestFileName, [string]$ExpectedSha256)
    $dest = Join-Path $licensesRepoDir $DestFileName
    if (-not (Test-Path -LiteralPath $dest -PathType Leaf)) {
        Write-FailActionable "Expected frozen candidate license file '$dest' does not exist in the repository. CORR-002 requires this file to already be present as an accepted candidate artifact (AA License Materialization Triage v1) - this script no longer materializes it. Restore it from the accepted candidate footprint before re-running; do not let this script create it."
    }
    $actualHash = (Get-FileHash -LiteralPath $dest -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualHash -ne $ExpectedSha256) {
        Write-FailActionable "Repository candidate license file '$dest' SHA256 ($actualHash) does NOT match the frozen pinned identity ($ExpectedSha256). This file is frozen per CORR-002 section 3 - this script will never overwrite it. Investigate and restore it from the accepted candidate footprint instead of re-running this script to 'fix' it."
    }
    Write-Step "Verified repository candidate license file matches frozen identity: $dest (SHA256 $actualHash)"
}

Assert-UpstreamLicenseHash -SourceFileName 'COPYING' -ExpectedSha256 $ExpectedCopyingSha256
Assert-UpstreamLicenseHash -SourceFileName 'COPYING.LESSER' -ExpectedSha256 $ExpectedCopyingLesserSha256
Assert-RepositoryLicenseHash -DestFileName 'ifcopenshell.COPYING.txt' -ExpectedSha256 $ExpectedCopyingSha256
Assert-RepositoryLicenseHash -DestFileName 'ifcopenshell.COPYING.LESSER.txt' -ExpectedSha256 $ExpectedCopyingLesserSha256

# ------------------------------------------------------------------------------
# 2b. CORR-001 - external support dependency manifest (Boost/eigen3).
#
# Root cause of AA's Full Candidate Validation v1.2 configure failure
# ("Could NOT find Boost ..."): binding IfcOpenShell's configure directly to
# this repository's own vcpkg toolchain file does not provision Boost,
# because vcpkg's manifest discovery resolves relative to the CMake source
# directory being configured (IfcOpenShell's own <repo>\cmake, which has no
# vcpkg.json), not relative to the toolchain file's location - so no
# manifest was ever found and nothing was installed.
#
# Fix: generate our OWN runtime, external vcpkg manifest (outside this
# repository's tracked source, never this repository's own vcpkg.json - that
# would pull unrelated BIM Platform dependencies such as OCCT/Qt/bgfx/
# SQLite), containing ONLY the frozen boost-*/eigen3 closure at the frozen
# baseline, install it explicitly and deterministically to an external
# install root using the SAME vcpkg install this repository's own toolchain
# file belongs to, then bind IfcOpenShell's configure to that external
# manifest/install root explicitly (section 3 below) instead of relying on
# ambient discovery.
# ------------------------------------------------------------------------------
Write-Step "Provisioning external support dependency closure (CORR-001) at $SupportManifestDir ..."
New-Item -ItemType Directory -Force -Path $SupportManifestDir | Out-Null
New-Item -ItemType Directory -Force -Path $SupportInstalledDir | Out-Null

$supportManifest = [ordered]@{
    name            = 'p0-t005-ifcopenshell-support'
    'version-string' = '0.0.0'
    description     = 'CORR-001: external, minimum, frozen Boost/eigen3 closure for the P0-T005 IfcOpenShell dependency build only. Runtime-generated - not hand-maintained, not part of the tracked repository, never the BIM Platform vcpkg.json.'
    dependencies    = $SupportDependencies
}
$supportManifestPath = Join-Path $SupportManifestDir 'vcpkg.json'
($supportManifest | ConvertTo-Json -Depth 4) | Set-Content -LiteralPath $supportManifestPath -Encoding utf8

$supportConfiguration = [ordered]@{
    'default-registry' = [ordered]@{
        kind     = 'builtin'
        baseline = $SupportDependencyBaseline
    }
}
$supportConfigurationPath = Join-Path $SupportManifestDir 'vcpkg-configuration.json'
($supportConfiguration | ConvertTo-Json -Depth 4) | Set-Content -LiteralPath $supportConfigurationPath -Encoding utf8

Write-Step "Wrote external support manifest  : $supportManifestPath ($($SupportDependencies.Count) packages)"
Write-Step "Wrote external support config    : $supportConfigurationPath (baseline $SupportDependencyBaseline)"

Write-Step "Installing external support closure (non-interactive, deterministic) ..."
$vcpkgInstallArgs = @(
    'install',
    "--x-manifest-root=$SupportManifestDir",
    "--x-install-root=$SupportInstalledDir",
    "--triplet=$Triplet",
    '--feature-flags=manifests',
    '--no-print-usage'
)
& $VcpkgExe @vcpkgInstallArgs
if ($LASTEXITCODE -ne 0) {
    Write-FailActionable "vcpkg install of the external support dependency closure failed (exit $LASTEXITCODE). This is the exact frozen boost-*/eigen3 set Dependency Resolution Phase C already proved at baseline $SupportDependencyBaseline - do not substitute a different package set or baseline to work around a failure here; investigate and fix minimum-delta, or STOP per Brief section 22 if the closure itself cannot be reproduced."
}
Write-Step "External support dependency closure installed to: $SupportInstalledDir"

# ------------------------------------------------------------------------------
# 2c. CORR-003 - fresh external BinaryDir/InstallPrefix before every configure.
#
# AA Full Candidate Validation v1.3 proved CORR-001/CORR-002 work (exact
# clone reused, license hashes verified, support manifest generated, support
# closure installed) but then hit:
#   "vcpkg manifest mode was enabled for a build directory where it was
#    initially disabled. This is not supported. Please delete the build
#    directory and reconfigure."
# Root cause: the external BinaryDir (<BuildRoot>\build) can carry stale
# CMake/vcpkg cache state from an earlier configure (e.g. pre-CORR-001, when
# manifest mode was not bound) - reusing it across a configuration-mode
# transition is not supported by vcpkg/CMake.
#
# Fix: for this Phase-0 spike, BinaryDir and InstallPrefix are refreshed
# (deleted-if-present, then recreated) before every configure, while
# SourceDir (the exact pinned clone), SupportManifestDir, and
# SupportInstalledDir (both CORR-001) are left completely untouched - they
# are expensive/valid state this correction must not discard.
# ------------------------------------------------------------------------------
Write-Step "Refreshing bootstrap-owned external BinaryDir and InstallPrefix (CORR-003) ..."
Assert-SafeToDeleteExternalPath -Path $BinaryDir -Label 'BinaryDir'
Assert-SafeToDeleteExternalPath -Path $InstallPrefix -Label 'InstallPrefix'

if (Test-Path -LiteralPath $BinaryDir) {
    Write-Step "Removing existing external BinaryDir (stale CMake/vcpkg cache-mode state - CORR-003): $BinaryDir"
    Remove-Item -LiteralPath $BinaryDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $BinaryDir | Out-Null

if (Test-Path -LiteralPath $InstallPrefix) {
    Write-Step "Removing existing external InstallPrefix (fresh install target - CORR-003): $InstallPrefix"
    Remove-Item -LiteralPath $InstallPrefix -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $InstallPrefix | Out-Null

Write-Step "BinaryDir and InstallPrefix refreshed. SourceDir, SupportManifestDir, and SupportInstalledDir were NOT touched by this step."

# ------------------------------------------------------------------------------
# 3. Configure - static, C++17, IFC4-only, core-parse-only (Brief section 4)
# ------------------------------------------------------------------------------
# IfcOpenShell's own top-level CMake entry point lives under <repo>\cmake\,
# not the repository root (see this script's own RISK DISCLOSURE above).
$cmakeSourceDir = Join-Path $SourceDir 'cmake'
if (-not (Test-Path -LiteralPath $cmakeSourceDir -PathType Container)) {
    Write-FailActionable "Expected CMake entry point '$cmakeSourceDir' does not exist in the pinned checkout. This is exactly the kind of upstream-layout assumption this script's RISK DISCLOSURE flags as unverified in this session - inspect the actual checkout and correct this script's -S path minimum-delta rather than working around it with an architecture change."
}

# BinaryDir was already freshly created in section 2c above (CORR-003) -
# nothing further to create here.

$configureArgs = @(
    '-S', $cmakeSourceDir,
    '-B', $BinaryDir,
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchainFile",
    # CORR-001: explicitly bind to the external support manifest/install
    # root provisioned in section 2b above, instead of relying on vcpkg's
    # ambient manifest discovery (which never found a manifest here - see
    # section 2b's root-cause note - and which, if it ever did discover one,
    # must never be this repository's own vcpkg.json/vcpkg_installed).
    "-DVCPKG_MANIFEST_DIR=$SupportManifestDir",
    "-DVCPKG_INSTALLED_DIR=$SupportInstalledDir",
    "-DVCPKG_TARGET_TRIPLET=$Triplet",
    '-DVCPKG_MANIFEST_MODE=ON',
    # The external support closure was already installed explicitly and
    # deterministically in section 2b; configure must only CONSUME it, never
    # trigger a second implicit vcpkg install of its own.
    '-DVCPKG_MANIFEST_INSTALL=OFF',
    "-DCMAKE_INSTALL_PREFIX=$InstallPrefix",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    '-DCMAKE_CXX_STANDARD=17',
    '-DCMAKE_CXX_STANDARD_REQUIRED=ON',
    '-DBUILD_SHARED_LIBS=OFF',
    '-DSCHEMA_VERSIONS=4',
    '-DVERSION_OVERRIDE=ON',
    '-DBUILD_IFCGEOM=OFF',
    '-DBUILD_IFCPYTHON=OFF',
    '-DBUILD_CONVERT=OFF',
    '-DBUILD_GEOMSERVER=OFF',
    '-DWITH_OPENCASCADE=OFF',
    '-DWITH_CGAL=OFF',
    '-DIFCXML_SUPPORT=OFF',
    '-DWITH_ROCKSDB=OFF',
    '-DCOLLADA_SUPPORT=OFF',
    '-DGLTF_SUPPORT=OFF',
    '-DHDF5_SUPPORT=OFF',
    '-DWITH_PROJ=OFF',
    '-DUSD_SUPPORT=OFF',
    '-DWITH_RELATIONSHIP_VALIDATION=OFF',
    '-DWITH_ZSTD=OFF',
    '-DUSE_MMAP=OFF',
    '-DUSE_CCACHE=OFF'
)

Write-Step "Configuring IfcOpenShell (this is a one-time dependency build, never rebuilt as part of ordinary BIM Platform target compilation - Brief section 14) ..."
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    Write-FailActionable "cmake configure failed (exit $LASTEXITCODE). If this is again a 'Could NOT find Boost' failure, verify the external support closure actually installed at '$SupportInstalledDir' for triplet '$Triplet' (section 2b) before assuming a new defect. If this is a 'vcpkg manifest mode was enabled for a build directory where it was initially disabled' failure, BinaryDir should already have been freshly deleted/recreated this run (section 2c, CORR-003) - investigate why stale state survived rather than reintroducing a stale-reuse defect. If this failure indicates a required option is unavailable at the pinned revision, or that Python/IfcGeom/OpenCascade/CGAL cannot actually be disabled, STOP per Brief section 22 rather than relaxing a REQUIRED/OFF flag to work around it."
}

# ------------------------------------------------------------------------------
# 4. Build + install to the external prefix
# ------------------------------------------------------------------------------
Write-Step "Building + installing IfcOpenShell::IfcParse to $InstallPrefix ..."
& cmake --build $BinaryDir --config $Configuration --target install
if ($LASTEXITCODE -ne 0) {
    Write-FailActionable "cmake --build --target install failed (exit $LASTEXITCODE)."
}

# ------------------------------------------------------------------------------
# 5. Prove the installed package actually exists (Brief section 5 item 6)
# ------------------------------------------------------------------------------
$packageConfigCandidates = @(
    (Join-Path $InstallPrefix 'lib\cmake\IfcOpenShell\IfcOpenShellConfig.cmake'),
    (Join-Path $InstallPrefix 'lib\cmake\ifcopenshell\ifcopenshellConfig.cmake'),
    (Join-Path $InstallPrefix 'cmake\IfcOpenShellConfig.cmake')
)
$foundPackageConfig = $packageConfigCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
$headerCandidate = Join-Path $InstallPrefix 'include\ifcparse\IfcFile.h'

if (-not $foundPackageConfig) {
    Write-FailActionable "No installed IfcOpenShell CMake package config was found under any of: $($packageConfigCandidates -join ', '). The install may have used a different package-config path than this script assumes (RISK DISCLOSURE above) - inspect $InstallPrefix, correct this script's candidate list minimum-delta, and re-run rather than proceeding without proof the package exists (Brief section 5 item 6: 'prove the installed IfcOpenShell::IfcParse package')."
}
if (-not (Test-Path -LiteralPath $headerCandidate -PathType Leaf)) {
    Write-FailActionable "Installed IfcOpenShell::IfcParse public header was not found at the expected path '$headerCandidate'."
}

Write-Step "Proved installed package config: $foundPackageConfig"
Write-Step "Proved installed public header : $headerCandidate"

# ------------------------------------------------------------------------------
# 6. Deterministic bootstrap receipt (evidence, never written into the BIM
#    Platform source tree - lives only under BuildRoot, outside the
#    repository, matching this script's own "leave BIM source tree free of
#    vendored upstream source/binaries" requirement).
# ------------------------------------------------------------------------------
$receipt = [ordered]@{
    task                          = 'P0-T005'
    corrections_applied           = @(
        'BIM-AA-P0-T005-CORR-001 v1.0',
        'BIM-AA-P0-T005-CORR-002 v1.0',
        'BIM-AA-P0-T005-CORR-003 v1.0'
    )
    repository                    = $IfcOpenShellRepo
    ref                           = $IfcOpenShellRef
    expected_commit               = $ExpectedCommit
    expected_tree                 = $ExpectedTree
    resolved_commit               = $ExpectedCommit
    build_root                    = $BuildRoot
    install_prefix                = $InstallPrefix
    configuration                 = $Configuration
    schema                        = 'IFC4'
    build_ifcgeom                 = $false
    build_ifcpython               = $false
    with_opencascade              = $false
    with_cgal                     = $false
    package_config_found          = [string]$foundPackageConfig
    header_found                  = [string]$headerCandidate
    copying_sha256                = $ExpectedCopyingSha256
    copying_lesser_sha256         = $ExpectedCopyingLesserSha256
    support_manifest_dir          = $SupportManifestDir
    support_installed_dir         = $SupportInstalledDir
    support_dependency_baseline   = $SupportDependencyBaseline
    support_dependencies          = $SupportDependencies
    support_triplet               = $Triplet
}
$receiptPath = Join-Path $BuildRoot 'bootstrap-receipt.json'
($receipt | ConvertTo-Json -Depth 4) | Set-Content -LiteralPath $receiptPath -Encoding utf8
Write-Step "Wrote bootstrap receipt: $receiptPath"

Write-Host ''
Write-Host '[bootstrap-ifcopenshell] BOOTSTRAP COMPLETE' -ForegroundColor Green
Write-Host "  Configure the BIM Platform build with:" -ForegroundColor Green
Write-Host "    -DBIM_ENABLE_IFC=ON -DIfcOpenShell_DIR=`"$(Split-Path -Parent $foundPackageConfig)`"" -ForegroundColor Green
