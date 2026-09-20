# scripts/dependencies/ifcopenshell/

Controlled Windows PowerShell dependency bootstrap for the P0-T005 IFC
Spike (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0 section 5;
Execution Packet v1.1 section 11; ACR-P0-T005-001; corrected per
BIM-AA-P0-T005-CORR-001 v1.0 - see "External support dependency
provisioning" below - BIM-AA-P0-T005-CORR-002 v1.0 - see "License
evidence verification" below - and BIM-AA-P0-T005-CORR-003 v1.0 - see
"Fresh external BinaryDir / InstallPrefix" below).

```powershell
.\bootstrap-ifcopenshell.ps1 `
    -VcpkgToolchainFile <path-to-this-repo's-vcpkg>\scripts\buildsystems\vcpkg.cmake
```

Clones the exact pinned upstream commit (`ifcconvert-0.8.5`,
`16723d11cab9bc8a13b4e025a00d39445ccc462e`) into a short external build root
(default `C:\bimdeps\ifc`, outside this repository), provisions the frozen
external Boost/eigen3 support dependency closure IfcOpenShell's C++ core
needs to compile against (see below), configures a static, C++17,
IFC4-only, core-parse-only (`IfcParse`) build with
Python/IfcGeom/OpenCascade/CGAL all `OFF`, builds and installs it to an
external prefix, and proves the installed `IfcOpenShell::IfcParse` CMake
package exists. It also verifies the pinned commit's actual `COPYING` /
`COPYING.LESSER` files against the pinned identities in the Implementation
Brief - and, per CORR-002, verifies (never writes) the two frozen candidate
license files already present in `third_party/licenses/` (see "License
evidence verification" below).

Safe to re-run: an existing clone already at the pinned commit is reused
unless `-Force` is passed; a clone at any OTHER commit is a hard failure
(never silently rebuilt against a mismatched identity). The external
support dependency install (below) is separately, naturally idempotent -
`vcpkg install` skips packages that are already installed and unchanged,
so re-running the script does not require deleting or forcing anything
there. The external build directory and install prefix, by contrast, are
always freshly deleted and recreated on every run (see "Fresh external
BinaryDir / InstallPrefix" below) - this is deliberate, not a mistake, and
happens regardless of `-Force`.

Never vendors IfcOpenShell source or binaries into this repository, never
uses `FetchContent`/a Git submodule, and never falls back to a global
system-wide IfcOpenShell install.

## External support dependency provisioning (CORR-001)

AA's Full Candidate Validation v1.2 ran the original version of this script
against the real pinned commit and found that binding IfcOpenShell's CMake
configure directly to this repository's own vcpkg toolchain file
(`-DCMAKE_TOOLCHAIN_FILE=<this repo>\vcpkg\scripts\buildsystems\vcpkg.cmake`
alone) does **not** provision Boost for IfcOpenShell's separate, external
configure:

```text
Could NOT find Boost
(missing: Boost_INCLUDE_DIR system program_options regex thread date_time iostreams)
```

The root cause: vcpkg's manifest discovery resolves relative to the CMake
source directory being configured (IfcOpenShell's own `<repo>\cmake`, which
has no `vcpkg.json` of its own), not relative to the toolchain file's
location - so no manifest was ever found and nothing was ever installed for
that configure.

The corrected script now provisions its own **runtime-generated, external**
vcpkg manifest before configuring IfcOpenShell:

```text
C:\bimdeps\ifc\support-manifest\vcpkg.json             (generated; 17-package closure only)
C:\bimdeps\ifc\support-manifest\vcpkg-configuration.json (generated; frozen baseline)
C:\bimdeps\ifc\vcpkg_installed\                          (external install root)
```

This manifest is **never** this repository's own `vcpkg.json` (which would
pull in unrelated BIM Platform dependencies such as OCCT/Qt/bgfx/SQLite,
and is explicitly prohibited by CORR-001). It contains exactly the 17
packages Dependency Resolution Phase C already proved (Implementation
Brief section 19) - `boost-system`, `boost-program-options`, `boost-regex`,
`boost-thread`, `boost-date-time`, `boost-iostreams`, `boost-uuid`,
`boost-logic`, `boost-scope-exit`, `boost-multi-index`,
`boost-circular-buffer`, `boost-filesystem`, `boost-locale`, `boost-math`,
`boost-property-tree`, `boost-variant`, `eigen3` - frozen at the same
project vcpkg baseline as always, unchanged:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

The script locates `vcpkg.exe` next to the given `-VcpkgToolchainFile`
(the conventional `<vcpkg root>\scripts\buildsystems\vcpkg.cmake` layout),
runs `vcpkg install` against the generated manifest non-interactively and
deterministically into the external install root, and only then configures
IfcOpenShell, explicitly binding it to that external manifest/install root
via `-DVCPKG_MANIFEST_DIR`, `-DVCPKG_INSTALLED_DIR`,
`-DVCPKG_TARGET_TRIPLET=x64-windows` and `-DVCPKG_MANIFEST_INSTALL=OFF`
(the last one so CMake configure only *consumes* the already-installed
closure and never triggers a second, implicit vcpkg install of its own).
New `-SupportManifestDir`, `-SupportInstalledDir`, and `-Triplet`
parameters expose these paths; see `bootstrap-ifcopenshell.ps1`'s own
`.PARAMETER` doc comments.

## License evidence verification (CORR-002)

AA's own validation run of the CORR-001 script surfaced a repository
mutation boundary defect: the script's earlier license-capture step wrote
`third_party/licenses/ifcopenshell.COPYING.txt` and
`ifcopenshell.COPYING.LESSER.txt` into the tracked repository during what
was meant to be a repository-source/index **read-only** validation run
(AA License Materialization Triage v1). The license bytes themselves were
correct - the defect was the write, not the content. Those two files are
now accepted, frozen candidate artifacts at their pinned hashes and are
never touched by this script again.

The corrected script is **verify-only** for license evidence. It:

1. reads the pinned upstream `COPYING` and `COPYING.LESSER` from the
   external clone and verifies their SHA256 against the frozen expected
   hashes (Brief section 16) - read-only, never copied anywhere;
2. separately verifies the two existing repository candidate license
   files (`third_party/licenses/ifcopenshell.COPYING.txt` and
   `ifcopenshell.COPYING.LESSER.txt`) exist and carry those same frozen
   hashes;
3. fails actionably - without creating, overwriting, or otherwise
   touching either file - if either the upstream or the repository copy
   is absent or hash-mismatched.

The script writes **no repository file** as part of license verification.
Its only other repository-external I/O (the bootstrap receipt, the
CORR-001 external support-manifest/install root) already lived outside
this repository and remains unchanged by CORR-002.

## Fresh external BinaryDir / InstallPrefix (CORR-003)

AA's Full Candidate Validation v1.3 proved CORR-001 and CORR-002 work
end-to-end - the exact pinned clone was reused, both upstream and
repository license hashes verified, the external support manifest was
generated, and the 17-package Boost/eigen3 closure installed - then hit a
new failure only at IfcOpenShell's own configure step:

```text
vcpkg manifest mode was enabled for a build directory
where it was initially disabled.

This is not supported. Please delete the build directory
and reconfigure.
```

The root cause: the external CMake binary directory
(`C:\bimdeps\ifc\build`) still held CMake/vcpkg cache state from an
earlier, pre-CORR-001 configure (when manifest mode was not bound), and
reusing that binary directory across a manifest-mode transition is not
supported by vcpkg/CMake.

The corrected script now refreshes two - and only two - of its external
paths immediately before every IfcOpenShell configure:

```text
BinaryDir      (default C:\bimdeps\ifc\build)    - deleted if present, then recreated
InstallPrefix  (default C:\bimdeps\ifc\install)  - deleted if present, then recreated
```

`SourceDir` (the exact pinned clone), `SupportManifestDir`, and
`SupportInstalledDir` (the CORR-001 external support manifest and its
restored/installed Boost/eigen3 closure) are never touched by this
refresh - discarding those would be expensive and unnecessary; the
manifest-mode transition problem is specific to the CMake binary
directory and the install prefix.

Every deletion is guarded by an explicit, fail-closed safety check
(`Assert-SafeToDeleteExternalPath`) before anything is removed. A target
is only ever deleted if it:

- resolves beneath the configured `BuildRoot`;
- is not `BuildRoot` itself;
- is not `SourceDir`;
- is not `SupportManifestDir`;
- is not `SupportInstalledDir`;
- is not inside `D:\Projects\BIM-Platform`;
- is not inside `D:\Projects\BIM-Platform-WT-P0-T005`.

Any violation fails the script closed with an actionable message, rather
than deleting anything. This is a deliberately narrow rule - never a broad
deletion rooted at `C:\bimdeps`, `%LOCALAPPDATA%`, the repository, or the
source clone.

After a successful run, configure the BIM Platform build with:

```powershell
cmake --preset <preset> -DBIM_ENABLE_IFC=ON -DIfcOpenShell_DIR=<install-prefix>\lib\cmake\IfcOpenShell
```

See `bootstrap-ifcopenshell.ps1`'s own `.NOTES` block for this script's
disclosed execution-channel risk: it was authored without a live run
against the pinned commit in this session (no execution channel to clone
real upstream Git repositories or drive a real MSVC toolchain here), and
the CORR-001 support-manifest provisioning, the CORR-002 verify-only
license logic, and the CORR-003 fresh-BinaryDir/InstallPrefix refresh (and
its deletion-safety guard) above were likewise authored without a live
execution channel against a real vcpkg installation, a real CMake
cache-mode transition, or a real repository working tree in this session.
Its first real run - including all three corrections - is on the Windows
Execution Operator's machine; Architecture Authority has not yet rerun the
authoritative Windows validation against the corrected script.
