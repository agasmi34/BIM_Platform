# bim-platform

`bim-platform` is the **internal engineering repository** for a Windows-first,
AI-native BIM kernel and platform effort. **`bim-platform` and the C++
namespace `bim` are internal engineering identifiers only and are not the
future commercial product name.**

## What this repository is right now

This repository currently contains only the output of task **P0-T001 -
Repository & Toolchain Scaffold**: a reproducible, auditable build/toolchain
baseline. It proves that the engineering toolchain and first-party module
boundaries exist and configure/build/test correctly. **It does not implement
any BIM behavior.** No Wall, Door, Slab, Room, Family, parametric graph,
topological naming, project schema, DWG/RVT/IFC, or UI code exists yet.

Governing documents for this state of the repository:

- `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` (document ID
  `BIM-AG-P0-T001`) - the approved architecture gate.
- `docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md` (document ID
  `BIM-TASK-P0-T001-CLAUDE`) - the released implementation brief this scaffold
  was built against.
- The parent constitution referenced by both documents (`BIM Platform -
  Master Engineering Constitution v0.2`) has not been supplied to this
  repository yet and is intentionally **not** fabricated or summarized here;
  it is referenced by document ID only until the Architecture Authority
  provides the canonical source.

## First target platform

Windows x64, MSVC v143 (Visual Studio 2022 17.14 line), C++20. All commands
below assume an **x64 Native Tools / Developer PowerShell for VS 2022**
environment (so `cl.exe` and the MSVC toolset are already on `PATH`) and a
`VCPKG_ROOT` environment variable pointing at a local vcpkg checkout.

## Intentional architectural boundaries

This scaffold exists specifically to prove three boundaries mechanically,
before any BIM feature code is written against them:

- **Command/query separation** - `commands` and `query` are separate,
  independently-testable targets; `query` must never mutate the model and
  `commands` must never write persistence directly.
- **Model/geometry boundary** - `bim_model`'s public headers may never expose
  raw OCCT types (`TopoDS_*`, `gp_*`, `BRep*`, OCCT smart handles). All OCCT
  usage is confined to the `bim_geometry_occt` adapter target, behind the
  project-owned `bim_geometry_api` surface.
- **Model/persistence boundary** - `bim_persistence` is the only target
  permitted to depend on SQLite. No SQLite type or header may appear in
  `bim_model`'s or `bim_commands`'s public surface.

An automated, dependency-free architecture checker
(`tools/architecture_checker.cmake`, run via `cmake -P`) enforces a minimum
set of these rules mechanically and is registered as a CTest test
(`arch_repository_boundaries`), together with a controlled negative self-test
(`arch_checker_detects_violation`) that proves the checker actually rejects a
deliberately broken fixture under `tests/fixtures/bad_architecture/`.

## Dependencies (P0-T001 only)

Managed via vcpkg manifest mode, frozen to a single immutable registry
baseline commit recorded in `vcpkg-configuration.json`:

| Dependency | Role | Route |
|---|---|---|
| Open CASCADE Technology (OCCT) 8.0.1 | Geometry kernel, behind `bim_geometry_occt` only | vcpkg |
| SQLite | Persistence, behind `bim_persistence` only | vcpkg |
| Catch2 v3 | Unit/integration test framework | vcpkg |
| fmt | Formatting utility, behind project logging facade | vcpkg |
| spdlog | Logging, behind project logging facade | vcpkg |

No Qt, IfcOpenShell, ODA, bgfx, Python runtime, or cloud/AI runtime code is
present in this repository. See `LICENSES.md` and `third_party/licenses/` for
the license inventory of the dependencies above.

## Canonical configure / build / test commands

From the repository root, in an x64 MSVC developer environment with
`VCPKG_ROOT` set:

```powershell
# Local development (Debug)
cmake --preset win-msvc-debug
cmake --build --preset win-msvc-debug
ctest --preset win-msvc-debug

# Local development (RelWithDebInfo)
cmake --preset win-msvc-relwithdebinfo
cmake --build --preset win-msvc-relwithdebinfo
ctest --preset win-msvc-relwithdebinfo

# CI-equivalent verification (RelWithDebInfo, first-party warnings as errors)
cmake --preset ci-win-msvc
cmake --build --preset ci-win-msvc
ctest --preset ci-win-msvc
```

Provider-neutral CI-equivalent scripts (format, configure-build-test,
architecture, license-inventory, and a `run-all` wrapper) live under
`scripts/ci/`. See that directory and
`docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md` section 6 (Phase L)
for details and exit-code conventions.

## Repository layout

See the canonical tree in
`docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 6. Future
modules (`src/interop/ifc`, `src/interop/dwg`, `src/interop/rvt`,
`src/documentation`, `src/viewport`, `src/desktop`) currently contain only a
short boundary `README.md` and are intentionally not built by the root
`CMakeLists.txt` in P0-T001.

## Status

This repository is at the state:
**IMPLEMENTED / EVIDENCE READY - PASS FOR INDEPENDENT REVIEW** is the maximum
claim the Implementation Engineer (Claude) may make for P0-T001, and only
after a real clean configure/build/test/architecture/license-inventory pass
has been executed and its evidence captured (see
`docs/evidence/P0-T001/CLAUDE_HANDOVER.md`). This repository does not
self-declare acceptance; acceptance requires independent review (Kimi) and
Architecture Authority closure per the task state machine in the Architecture
Gate, section 16.
