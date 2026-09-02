# BIM Platform - P0-T001 Implementation Brief - Claude

**Document ID:** BIM-TASK-P0-T001-CLAUDE  
**Version:** 1.0  
**Release date:** 2026-08-27  
**Task:** P0-T001 - Repository & Toolchain Scaffold  
**Parent gate:** BIM-AG-P0-T001 v1.0  
**Parent constitution:** BIM Platform - Master Engineering Constitution v0.2  
**Product Authority decision:** APPROVED  
**Implementation state:** RELEASED FOR IMPLEMENTATION  
**Implementation Engineer:** Claude  
**Independent Reviewer:** Kimi  
**Architecture Authority:** Product Owner + ChatGPT

> This brief is the only implementation authority for P0-T001. Claude may execute it, but may not expand scope, alter locked architecture, or reinterpret a conflict silently. If this brief conflicts with the approved Architecture Gate, the Architecture Gate wins and Claude must raise an ACR.

---

## 1. Mission

Establish a clean, reproducible, auditable Windows-first C++ repository for the BIM Platform. This task proves the engineering toolchain and module boundaries only. It does not implement BIM behavior.

The finished task must demonstrate that:

1. the repository can be configured and built from the command line;
2. the approved target/module graph exists;
3. OCCT 8.0.1 is reachable behind a project-owned geometry adapter boundary;
4. SQLite is reachable behind a project-owned persistence boundary;
5. Catch2/CTest tests run successfully;
6. architecture boundary rules can be checked mechanically;
7. dependencies and licenses are attributable and reproducible;
8. the task can be handed to an independent reviewer with complete evidence;
9. the `main` worktree remains untouched by implementation work.

---

## 2. Authority model for this task

### 2.1 Claude may

- inspect the machine, repository, Git state, compiler, CMake, Ninja and vcpkg;
- initialize the repository only under the bootstrap rules in section 6;
- create the approved branch/worktree;
- create files required by the approved repository scaffold;
- select technically correct CMake/vcpkg syntax that satisfies the locked decisions;
- add tests strictly required by this brief;
- make local implementation decisions that do not alter public architecture or scope;
- report environmental blockers and ACRs.

### 2.2 Claude may not

- change any `AG-*` locked decision;
- add Qt, IfcOpenShell, ODA, bgfx, Blender, FreeCAD code, Python runtime, cloud code or AI runtime;
- implement Wall, Door, Slab, Room, Family, BIM parameters, persistent naming, project schema, dependency graph behavior, collaboration, DWG, RVT or IFC behavior;
- expose raw OCCT types from model/public BIM APIs;
- write SQLite from the commands module;
- alter `main` with implementation changes;
- upgrade the vcpkg baseline after a successful frozen baseline is established without an ACR;
- add production dependencies not listed in this brief;
- use FetchContent, Git submodules or vendored third-party source;
- disable tests, warnings or architecture checks to make CI pass;
- modify the approved gate document to make the implementation appear compliant.

### 2.3 Mandatory stop conditions

Claude MUST stop the affected part and issue an **ACR - Architecture Clarification Request** if any of the following occurs:

- OCCT 8.0.1 cannot be resolved by the selected vcpkg baseline;
- a locked target/module name cannot be implemented as specified;
- satisfying one acceptance criterion requires violating another;
- the repository already contains conflicting architecture or user work;
- the repository path/branch state is ambiguous and proceeding could overwrite work;
- the only available fix requires adding a non-approved dependency;
- compiler/toolchain constraints conflict with the approved baseline;
- a public third-party type appears necessary outside an approved adapter boundary;
- the implementation would require a change to the approved module dependency law.

An ACR must contain: observed fact, evidence, affected locked decision/criterion, options, recommended option, and exact work paused.

---

## 3. Locked baseline

Claude MUST implement against the following baseline:

| Area | Locked P0-T001 value |
|---|---|
| Repository technical name | `bim-platform` |
| Root CMake project | `BIMPlatform` |
| C++ namespace | `bim` |
| First platform | Windows x64 |
| C++ language level | C++20 |
| Compiler | MSVC v143 / Visual Studio 2022 17.14 line |
| CMake reference | 4.4.2 |
| Preferred scripted generator | Ninja |
| Dependency manager | vcpkg manifest mode |
| Test runner | CTest |
| Unit framework | Catch2 v3 |
| Geometry kernel | Open CASCADE Technology 8.0.1 |
| Persistence dependency | SQLite via vcpkg |
| Utility dependencies | fmt, spdlog via vcpkg |
| Source encoding | UTF-8 without BOM unless a tool requires otherwise |
| Repository line endings | LF via `.gitattributes` |

C++23 features are prohibited in P0-T001.

---

## 4. Release clarifications issued by Architecture Authority

These clarifications are part of this brief and do not modify the approved architecture.

### IC-001 - Empty repository bootstrap

If and only if the designated `bim-platform` repository has no commits and no user content, Claude is authorized to create one empty bootstrap commit on `main`:

`chore: initialize repository`

This is a repository bootstrap exception, not an implementation commit. All P0-T001 implementation changes must occur in the task worktree/branch.

If there is existing uncommitted user content, Claude MUST NOT bootstrap over it.

### IC-002 - vcpkg baseline syntax

Use the current vcpkg-supported manifest/configuration syntax to freeze the exact registry baseline commit. The architectural requirement is an exact immutable baseline SHA plus reproducible resolution of OCCT 8.0.1. Do not force an obsolete JSON field merely to match wording in a prior document.

### IC-003 - CI provider neutrality

P0-T001 must create provider-neutral CI entrypoints under `scripts/ci/` that implement the four required logical jobs. If the repository already has an established hosted CI provider, wire these entrypoints into it. If no hosted provider is established, do not invent an external account/service; record hosted-runner integration as operationally deferred. This does not waive local CI-equivalent acceptance evidence.

### IC-004 - No fake APIs for empty modules

Foundational targets that need no public contract yet may contain a private compilation anchor only. Do not invent public domain APIs merely to make a static library non-empty. Public probe APIs are permitted only at explicit adapter/test boundaries described below.

---

## 5. Required canonical repository tree

Claude must create the following structure. Future modules may contain only a boundary `README.md` if they are not built yet.

```text
bim-platform/
|-- CMakeLists.txt
|-- CMakePresets.json
|-- vcpkg.json
|-- vcpkg-configuration.json
|-- .gitattributes
|-- .gitignore
|-- .clang-format
|-- .clang-tidy
|-- README.md
|-- LICENSES.md
|-- cmake/
|   |-- BimCompilerWarnings.cmake
|   |-- BimOptions.cmake
|   `-- BimSanitizers.cmake
|-- docs/
|   |-- constitution/
|   |-- adr/
|   |-- gates/
|   |-- tasks/
|   |-- reviews/
|   `-- evidence/
|-- src/
|   |-- foundation/
|   |-- model/
|   |-- dependency_graph/
|   |-- transactions/
|   |-- commands/
|   |-- query/
|   |-- geometry/
|   |   |-- api/
|   |   `-- occt/
|   |-- persistence/
|   |-- interop/
|   |   |-- ifc/
|   |   |-- dwg/
|   |   `-- rvt/
|   |-- documentation/
|   |-- viewport/
|   `-- desktop/
|-- tests/
|   |-- unit/
|   |-- integration/
|   |-- architecture/
|   `-- fixtures/
|-- tools/
|-- scripts/
|   `-- ci/
`-- third_party/
    `-- licenses/
```

### 5.1 Documentation placement

If the Product Authority provides the approved Markdown files, copy them without rewriting into:

- `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md`
- `docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md`

Do not fabricate or summarize a missing constitution file. A missing parent artifact should be referenced by document ID in repository README/documentation until the canonical source is provided.

---

## 6. Execution protocol

Claude must execute the task in the exact phase order below. Do not begin editing before Phase A evidence is captured.

### Phase A - Read-only preflight

Capture, without editing repository files:

1. current working directory;
2. candidate repository path;
3. `git --version`;
4. repository existence and `git rev-parse --show-toplevel` if applicable;
5. branch and HEAD if applicable;
6. `git status --short`;
7. `git worktree list`;
8. configured remotes (`git remote -v`);
9. CMake version;
10. Ninja version;
11. Visual Studio/MSVC discovery and `cl` version from an x64 developer environment;
12. vcpkg executable/root and current registry HEAD if available;
13. PowerShell version;
14. existing environment variables relevant to `VCPKG_ROOT` and compiler setup, without printing secrets.

Do not print access tokens, license keys or secret environment values.

#### Phase A decision

Classify the repository into exactly one state:

- **STATE A:** repository exists, has commits, is clean and compatible with the gate;
- **STATE B:** repository does not exist or exists empty with no commits and no user content;
- **STATE C:** repository has conflicting/uncommitted/user content or ambiguous state.

STATE C requires an ACR before any edit.

### Phase B - Repository bootstrap and isolated worktree

#### If STATE A

- verify `main` exists;
- record main HEAD SHA;
- do not edit main;
- create branch `task/P0-T001-repo-toolchain-scaffold` from approved main HEAD;
- create worktree `../bim-wt-P0-T001` or the equivalent sibling path resolved from the actual repo parent.

#### If STATE B

- create/initialize `bim-platform`;
- set default branch to `main`;
- verify Git author identity exists; do not invent an identity;
- create one empty bootstrap commit only: `chore: initialize repository`;
- create branch `task/P0-T001-repo-toolchain-scaffold` from that commit;
- create isolated sibling worktree `../bim-wt-P0-T001`;
- all subsequent edits occur only in the worktree.

Immediately after worktree creation, record:

- main path, branch, HEAD, status;
- task worktree path, branch, HEAD, status;
- `git worktree list`.

### Phase C - Scaffold files and repository policy

Create the canonical tree and repository policy files.

#### `.gitattributes`

Must enforce LF for source/text/config files and prevent accidental normalization of binary assets. At minimum cover common C/C++/CMake/JSON/YAML/Markdown/PowerShell/Python text extensions.

#### `.gitignore`

Must ignore at minimum:

- `build/` and CMake output;
- vcpkg installed/build cache folders if created under the repo;
- `.cache/`;
- Visual Studio user state (`.vs/`, `*.user`, etc.);
- IDE temporary files;
- local environment files and secrets;
- generated test output/log directories.

Do not ignore evidence documents that are intentionally committed.

#### `README.md`

Must state:

- repository is an engineering repository, not the commercial product name;
- Windows x64 is the first target;
- P0-T001 is scaffold only;
- command/query, model/geometry and persistence boundaries are intentional;
- no BIM features are implemented yet;
- canonical configure/build/test commands;
- link/reference to controlled architecture documents.

### Phase D - vcpkg manifest and dependency freeze

Direct dependencies for P0-T001 only:

- `opencascade` resolving version 8.0.1;
- `sqlite3`;
- `catch2` major 3;
- `fmt`;
- `spdlog`.

Requirements:

1. use manifest mode;
2. freeze an exact vcpkg registry baseline SHA;
3. if required, use a supported override/version constraint so OCCT resolves exactly 8.0.1;
4. do not activate optional OCCT features not needed for the smoke probe;
5. do not add Qt through OCCT features;
6. record the final dependency graph and resolved versions;
7. do not update the baseline after successful configure except through ACR.

If vcpkg's current port metadata gives OCCT 8.0.1 as the baseline version, prefer the simplest manifest consistent with reproducibility.

### Phase E - Root CMake policy

The root `CMakeLists.txt` must contain project policy and `add_subdirectory` composition only. It must not contain module implementation logic.

Required policy:

- `cmake_minimum_required` compatible with reference CMake 4.4.2 while not needlessly excluding earlier maintained versions;
- `project(BIMPlatform LANGUAGES CXX)`;
- C++20 required, extensions off;
- out-of-source build guard;
- `CTest`/testing enablement;
- central options/warnings modules;
- module subdirectories in dependency-safe order;
- first-party warning policy applied via a dedicated CMake target/helper rather than copy-pasted target flags.

### Phase F - CMake presets

Create command-line presets for:

- `win-msvc-debug`
- `win-msvc-relwithdebinfo`
- `win-msvc-release`
- `ci-win-msvc`

Also create matching build/test presets where useful so a clean scripted sequence is obvious.

Presets must:

- use Ninja for scripted builds;
- use the vcpkg toolchain through `VCPKG_ROOT` or another non-hardcoded environment-aware mechanism;
- use out-of-source build directories;
- avoid machine-specific absolute paths;
- preserve x64/MSVC expectations;
- make `ci-win-msvc` suitable for RelWithDebInfo verification.

### Phase G - First-party targets

Create these targets with aliases:

| Target | Alias | Required dependencies |
|---|---|---|
| `bim_foundation` | `bim::foundation` | none |
| `bim_model` | `bim::model` | foundation |
| `bim_dependency_graph` | `bim::dependency_graph` | foundation |
| `bim_transactions` | `bim::transactions` | model + foundation |
| `bim_commands` | `bim::commands` | transactions (and model only if necessary) |
| `bim_query` | `bim::query` | model + foundation |
| `bim_geometry_api` | `bim::geometry_api` | foundation |
| `bim_geometry_occt` | `bim::geometry_occt` | geometry_api + foundation + OCCT |
| `bim_persistence` | `bim::persistence` | model + transactions + foundation + SQLite |

The following future modules are scaffold-only and MUST NOT be built in P0-T001:

- interop/ifc;
- interop/dwg;
- interop/rvt;
- documentation;
- viewport;
- desktop.

Each future module gets a short boundary README, not fake implementation code.

### Phase H - Public/private header discipline

Where a public header is required, use:

`src/<module>/include/bim/<module>/...`

Private code belongs under:

`src/<module>/src/...`

Rules:

- public headers must not expose OCCT, Qt, SQLite, ODA or IfcOpenShell types outside explicit adapter boundaries;
- `foundation` includes no project module or third-party application dependency;
- tests consume public headers unless intentionally testing a private helper;
- do not introduce a monolithic `Element` or BIM object hierarchy in this task.

### Phase I - Minimal compile anchors and probe contracts

Do not invent domain behavior.

#### Foundation

A small, genuinely reusable project-owned status/result utility is permitted if needed by adapter probes. Keep it minimal and dependency-free.

#### Model, dependency graph, transactions, commands, query

If no public API is required yet, use private compilation anchors only. Do not create fake BIM entities or placeholder public classes.

#### Geometry API / OCCT adapter

Implement a tiny toolchain probe whose public surface is project-owned and neutral. The probe must:

- cause `bim_geometry_occt` to construct a trivial OCCT solid internally (for example a rectangular box);
- compute one deterministic neutral value such as volume or validity;
- return only project-owned primitive/status data;
- expose zero `TopoDS_*`, `gp_*`, `BRep*`, OCCT smart handles or OCCT headers in `bim_geometry_api` or model public headers.

This is not the future geometry engine API; name/document it as a Phase 0 diagnostic/probe to prevent accidental API ossification.

#### Persistence

Implement only an in-memory SQLite probe that:

- opens `:memory:`;
- verifies basic connection/statement execution;
- closes cleanly;
- returns project-owned status data;
- creates no BIM/project schema and no on-disk project file.

SQLite types must not leak into model/command public APIs.

### Phase J - Mandatory tests

Register all tests with CTest.

Required tests:

1. `unit_foundation_smoke`
2. `unit_model_links_foundation`
3. `integration_geometry_occt_probe`
4. `integration_persistence_sqlite_memory`
5. `arch_repository_boundaries`
6. `arch_checker_detects_violation`

`arch_checker_detects_violation` must deliberately run the architecture checker against a controlled bad fixture and be configured so the test passes only when the checker correctly rejects that fixture.

The architecture checker must at minimum enforce:

- no OCCT/Qt/SQLite/ODA/IfcOpenShell includes/types in model public headers;
- no project/third-party dependency in foundation;
- no direct SQLite include/use in commands;
- no raw OCCT include in viewport/desktop public code if those directories contain only future READMEs;
- no forbidden CMake target linkage that can be checked reliably at this stage.

Prefer a CMake script (`cmake -P`) or another dependency-free repository mechanism. Do not add Python only for this checker in P0-T001.

### Phase K - Compiler warnings, format and static analysis

Create `BimCompilerWarnings.cmake` and apply first-party policy:

- `/W4`
- `/permissive-`
- `/Zc:__cplusplus`
- `/EHsc`
- `UNICODE`
- `_UNICODE`
- `NOMINMAX`

CI-equivalent builds treat first-party warnings as errors. Third-party headers must not cause first-party warning failures.

Create repository-authoritative `.clang-format` and `.clang-tidy`.

Requirements:

- formatting check is deterministic and scriptable;
- static analysis uses a curated, low-noise baseline;
- generated/third-party sources are excluded;
- no blanket `NOLINT`/warning-disable strategy.

If clang-format/clang-tidy executables are unavailable locally, do not silently waive the check. Record the environment gap and either install only with explicit user authorization or raise an operational blocker. Do not change system software silently.

### Phase L - Provider-neutral CI entrypoints

Create:

- `scripts/ci/format.ps1`
- `scripts/ci/configure-build-test.ps1`
- `scripts/ci/architecture.ps1`
- `scripts/ci/license-inventory.ps1`
- `scripts/ci/run-all.ps1`

Each script must return a non-zero exit code on failure.

Logical jobs:

1. format;
2. configure-build-test;
3. architecture;
4. license-inventory.

The scripts must not contain user-specific absolute paths. They may require documented environment variables such as `VCPKG_ROOT`.

If an established hosted CI provider exists, create the thin provider workflow that invokes these scripts. If no provider exists, stop at provider-neutral scripts and state this fact in evidence.

### Phase M - License inventory

Create `LICENSES.md` and `third_party/licenses/` inventory for every direct P0 dependency.

At minimum record:

- dependency name;
- resolved version;
- source/homepage;
- license/SPDX identifier where available;
- vcpkg baseline SHA;
- path/source of the captured copyright/license notice.

Prefer copying the installed vcpkg package copyright/notice files from the resolved build into clearly named inventory files rather than manually retyping license text.

The license job must fail if any direct P0 dependency lacks an inventory entry.

### Phase N - Verification from clean state

Perform at least one clean verification using the `ci-win-msvc` path.

The evidence must include exact commands and exit codes for:

1. configure;
2. build;
3. CTest;
4. format check;
5. static-analysis check, if required by the repository baseline;
6. architecture checks;
7. license inventory check.

Do not use a previously dirty build directory as final evidence. Delete/recreate the final evidence build directory or use a new one.

### Phase O - Scope audit before commit

Before committing, verify all of the following:

- no Qt dependency;
- no IfcOpenShell dependency;
- no ODA code/dependency;
- no bgfx;
- no Python runtime;
- no DWG/RVT/IFC implementation;
- no BIM entities/features;
- no project database schema;
- no topological naming implementation;
- no dependency graph behavior beyond target scaffold;
- no user-specific absolute paths;
- no secrets;
- no binaries/build output committed.

### Phase P - Commit

Preferred final implementation commit subject:

`P0-T001: scaffold repository and toolchain`

Keep the task commit set small and coherent. Do not rewrite unrelated history.

After commit, task worktree must be clean.

If Git identity is unavailable, do not fabricate it. Stop before commit and report the operational blocker.

### Phase Q - Handover evidence

Produce both:

1. human-readable Markdown handover;
2. machine-readable JSON summary.

Recommended locations:

- `docs/evidence/P0-T001/CLAUDE_HANDOVER.md`
- `docs/evidence/P0-T001/CLAUDE_HANDOVER.json`

The handover is part of the task evidence and should be committed if it contains no machine-specific secret data. Absolute paths may appear in evidence if needed, but no credentials/tokens may appear.

---

## 7. Required CMake architecture

Expected dependency graph:

```text
foundation
  ^
  |-- geometry_api
  |-- dependency_graph
  `-- model
        ^
        |-- transactions
        |      ^
        |      `-- commands
        `-- query

geometry_occt --> geometry_api + foundation + OCCT
persistence   --> model + transactions + foundation + SQLite
```

Additional rules:

- no cycles;
- root target policy is centralized;
- target aliases use `bim::...`;
- public include directories are explicit;
- private includes are not propagated accidentally;
- third-party dependencies are linked at the narrowest adapter target possible.

---

## 8. Definition of implementation completion

Claude may mark the task **IMPLEMENTED / EVIDENCE READY** only when all of these are true:

- repository/worktree state is known and documented;
- all required scaffold files exist;
- vcpkg baseline is frozen;
- OCCT resolves 8.0.1;
- required targets configure/build;
- required tests are registered and pass;
- architecture checker has a positive repository test and a controlled negative self-test;
- license inventory is complete for direct P0 dependencies;
- CI-equivalent scripts pass locally;
- task branch is committed and clean;
- main worktree is unchanged by implementation;
- no approved-scope deviation exists, or every deviation has an approved ACR reference.

Claude must not claim PASS based only on code inspection.

---

## 9. Acceptance criteria mapping

Claude's handover must map each criterion to concrete evidence.

| ID | Criterion | Required evidence |
|---|---|---|
| AC-001 | Clean configure succeeds | command + exit 0 + clean build dir |
| AC-002 | MSVC build succeeds with zero first-party warnings | build log summary + exit 0 |
| AC-003 | All CTest tests pass | CTest summary |
| AC-004 | `bim_model` public headers leak no third-party types | architecture check result |
| AC-005 | OCCT 8.0.1 adapter probe works with neutral public result | resolved version + test result + header inspection |
| AC-006 | SQLite in-memory probe works only | test result + changed-file review |
| AC-007 | vcpkg manifest/baseline reproducible | manifest files + baseline SHA |
| AC-008 | required presets work | command results |
| AC-009 | formatting check passes | CI format result |
| AC-010 | architecture check passes | CI architecture result |
| AC-011 | all direct P0 dependency license entries exist | inventory result |
| AC-012 | prohibited/future features absent | scope audit |
| AC-013 | task clean, main untouched | final statuses + worktree list |
| AC-014 | no unresolved Kimi BLOCKER/MAJOR | not claimable by Claude; reserved for reviewer |
| AC-015 | Architecture Authority closure | not claimable by Claude; reserved for Architecture Authority |

Claude must report AC-014 and AC-015 as **PENDING REVIEW/AUTHORITY**, not PASS.

---

## 10. Required human-readable handover format

Claude must use this structure:

```text
# P0-T001 Claude Handover

## 1. Outcome
PASS FOR REVIEW / BLOCKED / PARTIAL

## 2. Authority
Task ID
Gate version
Brief version
Branch
Worktree
Start HEAD
Final HEAD

## 3. Preflight
Repository state classification
Tool versions
vcpkg baseline
Resolved direct dependency versions

## 4. Implementation summary
What was created
What was intentionally not created

## 5. Target graph
Actual first-party target dependency graph

## 6. Verification
Configure command + exit
Build command + exit
CTest command + result/count
Format result
Static analysis result
Architecture result
License inventory result

## 7. Acceptance criteria
AC-001 ... AC-015 with PASS/FAIL/PENDING and evidence pointer

## 8. Scope audit
Explicit prohibited-feature checklist

## 9. Git evidence
git diff --stat against task base
changed-file list
final git status --short
git worktree list
main worktree status

## 10. ACRs / deviations
None, or exact ACR IDs and approvals

## 11. Reviewer instructions
Exact commit/range Kimi should review
Exact commands Kimi can rerun
```

---

## 11. Required machine-readable handover fields

`CLAUDE_HANDOVER.json` must contain at least:

```json
{
  "task_id": "P0-T001",
  "gate_version": "1.0",
  "brief_version": "1.0",
  "outcome": "PASS_FOR_REVIEW",
  "branch": "task/P0-T001-repo-toolchain-scaffold",
  "start_head": "<sha>",
  "final_head": "<sha>",
  "vcpkg_baseline": "<sha>",
  "dependencies": {
    "opencascade": "8.0.1",
    "sqlite3": "<resolved>",
    "catch2": "<resolved-v3>",
    "fmt": "<resolved>",
    "spdlog": "<resolved>"
  },
  "verification": {
    "configure": "PASS",
    "build": "PASS",
    "ctest": "PASS",
    "format": "PASS",
    "static_analysis": "PASS_OR_DOCUMENTED_NOT_AVAILABLE",
    "architecture": "PASS",
    "license_inventory": "PASS"
  },
  "acrs": [],
  "review_range": "<base>..<final>"
}
```

Use actual values. Do not insert fake SHAs or fake PASS results.

---

## 12. Independent-review boundary

Claude does not perform Kimi's role.

Claude may self-check implementation quality, but the handover must remain factual and must not state that the task is accepted. The maximum successful Claude state is:

**IMPLEMENTED / EVIDENCE READY - PASS FOR INDEPENDENT REVIEW**

Kimi will receive a separate review brief from Architecture Authority after Claude's handover is supplied.

---

## 13. What counts as failure

Any of the following makes the implementation not ready for review:

- edits were made directly on `main` after bootstrap;
- OCCT version is not 8.0.1;
- required build/test command fails;
- first-party warnings are ignored or suppressed globally;
- raw OCCT types leak into model/public BIM contracts;
- SQLite project schema was introduced;
- forbidden dependencies/features were added;
- baseline is floating/unfrozen;
- architecture checker cannot detect its controlled bad fixture;
- task worktree is dirty at handover;
- evidence omits exact commands/results;
- user work was overwritten or discarded;
- secrets are committed or printed into evidence;
- Claude silently changes architecture instead of raising an ACR.

---

## 14. Final execution instruction to Claude

You are the **Implementation Engineer**, not the Architecture Authority.

1. Read the approved Architecture Gate and this Implementation Brief fully before editing.
2. Inspect actual repository/toolchain state first.
3. Execute P0-T001 only in the isolated task branch/worktree.
4. Do not implement future BIM functionality.
5. Do not alter architecture to solve local inconvenience.
6. Raise an ACR when a locked decision cannot be satisfied exactly.
7. Prove the implementation by running the required clean configure/build/test/architecture/license checks.
8. Commit the coherent task implementation.
9. Return the full handover and evidence requested above.
10. Stop at **PASS FOR INDEPENDENT REVIEW**. Do not merge to `main` and do not claim acceptance.

**Release status: AUTHORIZED TO IMPLEMENT P0-T001.**
