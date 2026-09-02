# BIM Platform — P0-T001 Architecture Gate Package

**Document ID:** BIM-AG-P0-T001  
**Version:** 1.0  
**Date:** 2026-08-26  
**Parent authority:** BIM Platform — Master Engineering Constitution v0.2  
**Gate status:** READY FOR PRODUCT AUTHORITY APPROVAL  
**Implementation status:** NOT RELEASED  

> This package freezes the repository, toolchain, module boundaries, build/test rules, source-control workflow, and acceptance evidence for P0-T001. Claude MUST NOT write production code for P0-T001 until Product Authority explicitly approves this gate and Architecture Authority releases the Implementation Brief.

## 1. Authority and operating model

| Role | Authority | P0-T001 responsibility |
|---|---|---|
| Product Authority | Project Owner | Approves scope, cost-sensitive dependencies, release of the gate, and final acceptance. |
| Architecture Authority | Project Owner + ChatGPT | Owns architecture, module boundaries, dependency policy, ADRs, task briefs, acceptance criteria, and closure. |
| Implementation Engineer | Claude | Implements only the released task brief. May not change architecture or expand scope. |
| Independent Reviewer | Kimi | Reviews diff, tests, architecture compliance, licensing and evidence independently. Does not silently fix Claude's work during review. |
| CI / Repository | Objective authority | Build/test/format/license evidence. A failing mandatory gate cannot be overruled by an AI opinion. |

### 1.1 Mandatory escalation
If Claude discovers an architectural ambiguity or a required change to a locked decision, Claude MUST stop that part of implementation and raise an **ACR — Architecture Clarification Request**. Claude must not invent a local workaround that changes a public contract.

## 2. Gate objective
P0-T001 establishes a reproducible, auditable, Windows-first C++ repository that can host the BIM kernel without forcing premature decisions about UI, cloud, IFC, DWG, RVT, or product branding.

### 2.1 In scope
- Repository scaffold and canonical directory layout.
- CMake project and presets.
- vcpkg manifest-mode dependency management.
- First-party module targets and architecture-boundary stubs.
- OCCT 8.0.1 toolchain availability check, but no BIM geometry feature implementation.
- SQLite dependency availability check, but no project schema implementation.
- Unit-test framework and one smoke test per foundational target where meaningful.
- Formatting/lint baseline.
- Git/branch/worktree conventions.
- CI skeleton for configure/build/test/format/architecture checks.
- License inventory mechanism.
- Evidence collection and closure format.

### 2.2 Explicitly out of scope
- Wall, Door, Slab or other BIM feature code.
- Native project schema.
- Parametric graph implementation.
- Topological naming implementation.
- Qt desktop UI.
- Renderer or bgfx integration.
- IfcOpenShell integration.
- ODA integration.
- RVT/RFA integration.
- AI runtime integration.
- Cloud/collaboration services.
- Product branding/name.

## 3. Canonical project identity

| Item | Locked value |
|---|---|
| Repository technical name | `bim-platform` |
| Root CMake project name | `BIMPlatform` |
| C++ root namespace | `bim` |
| First shipping OS target | Windows x64 |
| Source language baseline | C++20 |
| Scripting/AI language | Python, introduced only when a task requires it |
| Build system | CMake |
| C++ dependency manager | vcpkg manifest mode |
| Test runner | CTest |
| Unit test framework | Catch2 v3 |
| Source encoding | UTF-8 without BOM unless a tool requires otherwise |
| Repository line endings | LF, enforced by `.gitattributes` |

**Branding rule:** The technical repository name and `bim` namespace are internal engineering identifiers and MUST NOT be treated as the future commercial product name.

## 4. Windows toolchain baseline

### 4.1 Locked for P0-T001
- **Visual Studio 2022 17.14 Current Channel**, MSVC v143 toolset. The current supported 17.14 line remains under Microsoft support through the Visual Studio 2022 lifecycle.
- **CMake 4.4.2** as the P0 reference/CI version.
- **Ninja** as the preferred CMake generator for scripted/CI builds; Visual Studio generator remains supported for local IDE workflows.
- **Git** current maintained release; exact local patch level is evidence, not an architectural contract.
- **C++ standard:** C++20. C++23 language/library features are prohibited in Phase 0 unless an ADR explicitly approves them.

### 4.2 Compiler policy
First-party targets MUST use:
- `/W4`
- `/permissive-`
- `/Zc:__cplusplus`
- `/EHsc`
- Unicode definitions (`UNICODE`, `_UNICODE`)
- `NOMINMAX`

Warnings are errors in CI for first-party code. Third-party headers MUST be isolated so third-party warnings do not fail first-party CI.

Exceptions and RTTI remain enabled because current CAD dependencies and error boundaries require them. Disabling either requires an ADR.

## 5. Dependency baseline and freeze policy

### 5.1 P0-T001 dependencies
| Dependency | P0-T001 decision | Integration route |
|---|---|---|
| Open CASCADE Technology | **LOCK: 8.0.1** | vcpkg manifest; dynamic Windows linkage preferred during P0 unless packaging tests prove otherwise |
| SQLite | **LOCK: vcpkg baseline version** | vcpkg manifest |
| Catch2 | **LOCK: major version 3** | vcpkg manifest |
| fmt | **LOCK: current vcpkg baseline** | vcpkg manifest |
| spdlog | **LOCK: current vcpkg baseline** | vcpkg manifest |

The exact vcpkg `builtin-baseline` commit MUST be frozen in `vcpkg-configuration.json` during P0-T001 and captured in evidence. It MUST be a commit that resolves OCCT 8.0.1. Once the task is accepted, dependency baseline changes require a dependency-update task or ADR; Claude may not advance the baseline ad hoc.

### 5.2 Deliberately deferred dependencies
- **Qt:** not linked in P0-T001. Qt 6.11.2 is the current stable release on 2026-08-26, while Qt 6.12 is still beta and is scheduled as a commercial LTS line. P0-T003 will compare the stable 6.12 release after publication against the 6.11 baseline and lock the desktop UI version.
- **IfcOpenShell:** P0-T005.
- **ODA Drawings SDK:** P0-T006 licensing/evaluation gate.
- **ODA BimRv:** P0-T007 evaluation only; no product integration authorization.
- **bgfx / viewport abstraction:** P0-T003 benchmark/evaluation.

### 5.3 Dependency rules
- Production third-party C++ dependencies MUST enter through vcpkg unless an ADR approves another mechanism.
- `FetchContent` MUST NOT silently pull production dependencies at configure time.
- Git submodules are prohibited by default.
- Vendored source is prohibited unless licensing, patch ownership, and update responsibility are explicitly approved.
- Every shipped dependency MUST have a recorded license entry under `third_party/licenses/` or an automatically generated equivalent inventory.

## 6. Canonical repository layout

```text
bim-platform/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ vcpkg.json
├─ vcpkg-configuration.json
├─ .gitattributes
├─ .gitignore
├─ .clang-format
├─ .clang-tidy
├─ README.md
├─ LICENSES.md
├─ cmake/
│  ├─ BimCompilerWarnings.cmake
│  ├─ BimOptions.cmake
│  └─ BimSanitizers.cmake
├─ docs/
│  ├─ constitution/
│  ├─ adr/
│  ├─ gates/
│  ├─ tasks/
│  ├─ reviews/
│  └─ evidence/
├─ src/
│  ├─ foundation/
│  ├─ model/
│  ├─ dependency_graph/
│  ├─ transactions/
│  ├─ commands/
│  ├─ query/
│  ├─ geometry/
│  │  ├─ api/
│  │  └─ occt/
│  ├─ persistence/
│  ├─ interop/
│  │  ├─ ifc/
│  │  ├─ dwg/
│  │  └─ rvt/
│  ├─ documentation/
│  ├─ viewport/
│  └─ desktop/
├─ tests/
│  ├─ unit/
│  ├─ integration/
│  ├─ architecture/
│  └─ fixtures/
├─ tools/
├─ scripts/
└─ third_party/
   └─ licenses/
```

Empty future modules MUST contain a short `README.md` boundary statement instead of fake implementation code.

## 7. Canonical CMake targets

| Directory | Target | Type at P0-T001 |
|---|---|---|
| `src/foundation` | `bim_foundation` | STATIC library |
| `src/model` | `bim_model` | STATIC library |
| `src/dependency_graph` | `bim_dependency_graph` | STATIC library |
| `src/transactions` | `bim_transactions` | STATIC library |
| `src/commands` | `bim_commands` | STATIC library |
| `src/query` | `bim_query` | STATIC library |
| `src/geometry/api` | `bim_geometry_api` | INTERFACE/STATIC as required by first real types |
| `src/geometry/occt` | `bim_geometry_occt` | STATIC library linking OCCT |
| `src/persistence` | `bim_persistence` | STATIC library linking SQLite |
| future interop modules | `bim_interop_*` | NOT built until their gate task |
| future desktop | `bim_desktop` | NOT built until UI gate |

Target aliases SHOULD use `bim::foundation`, `bim::model`, etc. Public include paths MUST use `include/bim/...` layout where a public header exists.

## 8. Module dependency law

### 8.1 Allowed dependency direction

```text
foundation
  ↑
  ├──────── geometry_api
  ├──────── dependency_graph
  └──────── model
               ↑
        ┌──────┼───────────┐
   transactions          query
        ↑
     commands

geometry_occt ──> geometry_api + foundation + OCCT
persistence   ──> model + transactions + foundation + SQLite
interop_*     ──> public model/query/command contracts only
viewport      ──> model/query + geometry_api (never geometry_occt internals)
desktop       ──> application-facing public APIs; no raw DB or raw OCCT dependency
```

### 8.2 Hard prohibitions
- `foundation` MUST NOT depend on any project module, Qt, OCCT, SQLite, ODA, IfcOpenShell, or Python.
- `model` MUST NOT include OCCT headers, Qt headers, SQLite headers, ODA headers, or GUI types.
- Public BIM identity MUST NOT use raw OCCT `TopoDS_*` handles.
- `commands` MUST NOT write SQLite directly.
- `query` MUST NOT mutate the model.
- `desktop` MUST NOT reach around the command/query APIs to modify internal state.
- Interop modules MUST NOT become native model owners.
- No circular target dependencies are permitted.

Architecture tests MUST eventually enforce these prohibitions mechanically; P0-T001 establishes the scaffolding and at least one boundary-check mechanism.

## 9. Public API and header policy
- Public headers: `src/<module>/include/bim/<module>/...`.
- Private implementation: `src/<module>/src/...`.
- Tests MUST consume public headers unless the test is explicitly a white-box unit test.
- No public header may expose third-party implementation types unless the module is an explicit adapter boundary and Architecture Authority approves it.
- PImpl or narrow adapter interfaces SHOULD be used where third-party ABI exposure would create lock-in.

## 10. CMake and preset contract
Required presets:
- `win-msvc-debug`
- `win-msvc-relwithdebinfo`
- `win-msvc-release`
- `ci-win-msvc`

All scripted builds MUST be possible from a clean clone using documented commands without opening Visual Studio manually.

The root `CMakeLists.txt` MUST NOT contain module implementation logic. It owns project policy, options and `add_subdirectory` composition only.

## 11. Build output and configuration policy
- Out-of-source builds only.
- `build/` and `.cache/` are ignored.
- Debug, RelWithDebInfo and Release are supported.
- CI primary verification is RelWithDebInfo plus unit tests; Debug may be used for developer diagnostics.
- No generated binary artifact is committed to Git.
- Runtime DLL staging, when required by OCCT, must be scripted rather than performed manually.

## 12. Testing baseline

### 12.1 P0-T001 mandatory tests
1. Foundation smoke test.
2. Model can link against foundation without third-party leakage.
3. Geometry OCCT adapter can construct one trivial OCCT primitive internally and return a project-owned neutral result/status without exposing a `TopoDS_*` type through the public interface.
4. Persistence target links SQLite and can open an in-memory DB in a test; no project schema yet.
5. Architecture boundary check detects at least one prohibited include/dependency class through a script or CMake rule.

### 12.2 Test naming
- Unit: `unit_<module>_<behavior>`
- Integration: `integration_<scope>_<behavior>`
- Architecture: `arch_<rule>`

All tests MUST be registered in CTest and runnable by the CI preset.

## 13. Formatting and static-analysis policy
- `.clang-format` is repository-authoritative.
- `.clang-tidy` baseline is repository-authoritative.
- Format checking is mandatory in CI.
- Static analysis starts with a curated rule set; it MUST NOT be configured so noisily that developers routinely ignore it.
- Suppressions require an inline reason or central suppression entry.
- Generated and third-party sources are excluded from first-party style rules.

## 14. Logging and error contract for Phase 0
- Use project-owned status/error types at module boundaries.
- Third-party exceptions/errors MUST be translated at adapter boundaries where practical.
- `spdlog` may be used behind a project-owned logging facade; core public APIs MUST NOT require callers to depend on spdlog types.
- Secrets, license keys and local absolute paths MUST NOT be logged by default.

## 15. Git, branch and worktree law

### 15.1 Main branch
- `main` is integration authority.
- Direct implementation commits to `main` are prohibited.
- Accepted tasks integrate only after reviewer and Architecture Authority closure.

### 15.2 P0-T001 branch
Canonical branch:
`task/P0-T001-repo-toolchain-scaffold`

Canonical worktree naming pattern:
`../bim-wt-P0-T001`

Physical parent path is environment-specific and is not part of repository architecture.

### 15.3 Commit rule
The accepted implementation should normally collapse to a small coherent commit set. Preferred final task commit subject:
`P0-T001: scaffold repository and toolchain`

No unrelated formatting or opportunistic refactors.

## 16. Task-state machine

```text
DRAFT
  ↓
ARCHITECTURE GATE READY
  ↓ Product Authority approval
RELEASED FOR IMPLEMENTATION
  ↓ Claude
IMPLEMENTED / EVIDENCE READY
  ↓ Kimi
REVIEWED
  ├─ CHANGES REQUIRED → Claude
  └─ PASS → Architecture Authority
ARCHITECTURE CLOSURE
  ↓
ACCEPTED
  ↓
CONTROLLED INTEGRATION
  ↓
CLOSED
```

Claude MUST NOT skip states. Kimi review does not equal acceptance. Only Architecture Authority can issue Architecture Closure, and Product Authority retains final product veto.

## 17. Claude implementation constraints for P0-T001
When released, Claude MUST:
1. Inspect repository and environment first; never assume state.
2. Record preflight evidence before edits.
3. Work only in the P0-T001 worktree/branch.
4. Implement exactly the released scaffold and target graph.
5. Not add Qt, IfcOpenShell, ODA, bgfx, Python runtime, cloud code, or BIM features.
6. Not rename modules or targets without ACR approval.
7. Run format, configure, build and CTest before handover.
8. Produce a machine-readable and human-readable evidence summary.
9. Leave the main working tree untouched.
10. Stop and raise an ACR if a locked requirement cannot be satisfied.

## 18. Kimi independent-review contract
Kimi MUST review:
- Scope compliance.
- Full diff, not only Claude's summary.
- Target dependency graph.
- Public-header third-party leakage.
- Reproducible configure/build/test evidence.
- vcpkg baseline freeze.
- License inventory.
- Repository cleanliness.
- No prohibited dependency or feature creep.

Finding severity:
- **BLOCKER:** cannot accept; correctness, architecture, licensing, security, reproducibility or scope breach.
- **MAJOR:** must be fixed before acceptance unless Architecture Authority explicitly waives with written rationale.
- **MINOR:** non-blocking improvement; may be deferred to a recorded follow-up.

Kimi MUST NOT alter Claude's branch during the review. Review independence must remain auditable.

## 19. Evidence bundle required from Claude
The handover MUST contain:

### 19.1 Authority/preflight
- Repository absolute path.
- Branch.
- HEAD SHA.
- `git status --short`.
- Worktree list.
- CMake version.
- Compiler version.
- vcpkg baseline SHA.
- Resolved dependency versions.

### 19.2 Build evidence
- Clean configure command and exit status.
- Build command and exit status.
- CTest command, count and result.
- Format/static check result.
- Architecture-boundary check result.

### 19.3 Change evidence
- `git diff --stat`.
- Changed-file list.
- Final repository tree to the depth specified by the task brief.
- Any deviations: MUST be zero unless approved through ACR.

### 19.4 Post-state
- Final HEAD.
- Final branch.
- Final `git status --short`.
- Main worktree status proving it remained untouched.

## 20. CI contract
P0-T001 creates a CI skeleton with these logical jobs:
1. **format** — formatting conformance.
2. **configure-build-test** — Windows MSVC configure/build/CTest.
3. **architecture** — dependency/include boundary rules.
4. **license-inventory** — verifies third-party license metadata is present.

The exact hosted runner image is operational configuration and must be pinned in the CI file at implementation time after verifying availability. The tool versions printed by CI become part of the evidence. Changing the runner image later requires a maintenance task because it can change compiler/SDK behavior.

## 21. Security and secret policy
- No credentials, ODA keys, Qt commercial credentials, tokens or local secrets in Git.
- `.env` is not a supported production configuration mechanism for desktop core secrets.
- CI secrets live only in the CI secret store when such secrets become necessary.
- Binary artifacts from unknown sources are prohibited.
- Dependency origin and license must be attributable.

## 22. P0-T001 acceptance criteria
P0-T001 is acceptable only if ALL criteria pass:

**AC-001** Clean clone/configure succeeds with the documented preset.  
**AC-002** MSVC build succeeds with zero first-party warnings under the CI warning policy.  
**AC-003** All CTest tests pass.  
**AC-004** `bim_model` public headers expose no OCCT/Qt/SQLite/ODA/IfcOpenShell types.  
**AC-005** `bim_geometry_occt` proves OCCT 8.0.1 is correctly linked without leaking raw OCCT topology into `bim_geometry_api`.  
**AC-006** `bim_persistence` proves SQLite linkage using an in-memory smoke test only.  
**AC-007** vcpkg manifest and `builtin-baseline` are committed and reproducible.  
**AC-008** Required CMake presets work from command line.  
**AC-009** Format check passes.  
**AC-010** Architecture-boundary check passes.  
**AC-011** Third-party license inventory contains every resolved direct P0 dependency.  
**AC-012** No Qt/IfcOpenShell/ODA/bgfx/RVT/DWG/BIM feature implementation is present.  
**AC-013** Git status is clean at handover and main working tree was not modified.  
**AC-014** Kimi returns no unresolved BLOCKER or MAJOR findings.  
**AC-015** Architecture Authority issues written closure.

## 23. Phase 0 sequence after P0-T001
The next gates are ordered as follows unless an ADR changes the sequence:

| Task | Purpose | Key decision produced |
|---|---|---|
| P0-T001 | Repository & Toolchain Scaffold | Reproducible engineering baseline |
| P0-T002 | OCCT Geometry Spike | Kernel adapter, precision/tolerance, fast-path boundary |
| P0-T003 | Desktop + Viewport Spike | Qt stable/LTS lock, bgfx vs Qt RHI decision |
| P0-T004 | Persistence Spike | SQLite schema philosophy, transaction journal proof |
| P0-T005 | IFC Spike | IfcOpenShell boundary and round-trip proof |
| P0-T006 | DWG/ODA Evaluation | Commercial/license path + DWG fidelity proof |
| P0-T007 | RVT/BimRv Evaluation | Read-only capability matrix; no product integration authorization |
| P0-T008 | Topological Reference Spike | Semantic reference/persistent naming proof |
| P0-T009 | Dependency Graph Spike | Explicit semantic DAG + computed dependency tracking proof |
| P0-T010 | Phase 1 Architecture Gate | Locks Vertical Slice implementation contracts |

No Phase 1 BIM feature implementation starts before P0-T008, P0-T009 and P0-T010 are accepted.

## 24. Locked decisions register for this gate

| ID | Decision | Status |
|---|---|---|
| AG-001 | Repository technical name is `bim-platform` | LOCKED |
| AG-002 | Windows x64 is first development/shipping target | LOCKED |
| AG-003 | C++20 is Phase 0 language baseline | LOCKED |
| AG-004 | CMake + vcpkg manifest mode is build/dependency baseline | LOCKED |
| AG-005 | OCCT 8.0.1 is geometry-kernel baseline for Phase 0 | LOCKED |
| AG-006 | Native model public APIs may not expose raw OCCT types | LOCKED |
| AG-007 | SQLite adapter exists but native project schema is deferred | LOCKED |
| AG-008 | Qt is deliberately not linked in P0-T001 | LOCKED FOR THIS TASK |
| AG-009 | Qt stable/LTS decision occurs in P0-T003 | SCHEDULED |
| AG-010 | No FreeCAD fork enters the product repository | LOCKED |
| AG-011 | No DWG parser is built in-house; ODA evaluation owns the path | LOCKED |
| AG-012 | RVT is read-evaluation only in Phase 0; no writer | LOCKED |
| AG-013 | One task = one isolated branch/worktree | LOCKED |
| AG-014 | Claude implements; Kimi independently reviews | LOCKED |
| AG-015 | Claude cannot change architecture without ACR | LOCKED |
| AG-016 | Product Owner + ChatGPT are Architecture Authority | LOCKED |
| AG-017 | Main branch receives only accepted controlled integration | LOCKED |
| AG-018 | Phase 1 waits for persistent-reference and dependency-graph spikes | LOCKED |

## 25. Product Authority approval block

**Decision required:** Approve / Reject / Request changes.

If approved, Architecture Authority will issue:
1. `P0-T001 Implementation Brief — Claude` with exact preflight and implementation steps.
2. `P0-T001 Independent Review Brief — Kimi` to be used only after Claude's evidence handover.

**Current state:** Architecture Gate complete; implementation intentionally blocked pending Product Authority approval.

---

## Appendix A — externally verified toolchain facts (2026-08-26)
- Qt 6.11.2 was released 2026-08-18; Qt 6.12 was still in beta and targeted for September 2026, with 6.12 planned as an LTS line for commercial users.
- Open CASCADE Technology 8.0.1 was released 2026-07-30 and is available as a vcpkg port.
- CMake 4.4.2 is the current stable release.
- Visual Studio 2022 17.14 is the supported Current Channel line; 17.14.39 was released 2026-08-18.

These facts justify the P0 choices but do not grant an implementation agent authority to upgrade versions independently.
