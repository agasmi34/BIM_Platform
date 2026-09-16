# BIM Platform — P0-T005 IFC Spike Architecture Gate Package

**Document ID:** BIM-AG-P0-T005
**Version:** 1.0
**Date:** 2026-09-16
**Gate status:** APPROVED
**Product Authority:** APPROVED
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi
**Task:** P0-T005 — IFC Spike
**Baseline repository:** `D:\Projects\BIM-Platform`
**Baseline branch:** `main`
**Baseline HEAD:** `2c2b89f73651f7d5981d42546cbc0e321d6bb055`
**Baseline tree:** `c2f6f8e463248f98aafe8109f71d85cf9003d5a9`

---

## 1. Discovery verdict

P0-T005 Discovery v1 completed read-only and passed.

Repository evidence shows:

- `src/interop/ifc/README.md` is the only IFC-named path.
- The repository currently contains no IFC implementation.
- IfcOpenShell is not currently a direct dependency.
- P0-T001 explicitly reserves IfcOpenShell integration for P0-T005.
- P0-T001 defines P0-T005 as **“IFC Spike — IfcOpenShell boundary and round-trip proof.”**
- `foundation` must not depend on IfcOpenShell.
- `model` public headers must not expose IfcOpenShell/vendor types.
- `src/interop/ifc` is not yet included in the root build graph.
- The current frozen vcpkg baseline is `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843`.
- Advancing the vcpkg baseline is prohibited without an ACR.
- Existing architecture enforcement is R1–R13.

Discovery classification:

```text
READY_FOR_ARCHITECTURE_GATE_DRAFT
```

Discovery source modifications:

```text
NONE
```

Discovery ACR:

```text
NONE
```

---

## 2. Architecture goal

P0-T005 shall prove that the BIM Platform can integrate an IFC implementation behind a project-owned, vendor-neutral C++ boundary and can perform a controlled IFC round trip without contaminating the model, foundation, persistence, UI, geometry, or other module contracts with IfcOpenShell types.

This is an interoperability **spike**, not the final production IFC subsystem.

---

## 3. Locked scope

P0-T005 SHALL implement only the minimum needed to prove:

1. a dedicated IFC adapter module can be built on Windows x64;
2. IfcOpenShell is owned exclusively by that IFC adapter boundary;
3. public first-party IFC headers remain vendor-neutral;
4. a small project-owned neutral dataset can be exported to an IFC file;
5. the exported IFC can be reopened/imported;
6. selected neutral invariants survive the export/import round trip;
7. malformed/unsupported input fails through first-party status/error semantics rather than leaking third-party exceptions/types;
8. architecture rules mechanically prevent IfcOpenShell ownership leakage;
9. the proof is deterministic, repeatable, and leaves no generated IFC artifacts in the source tree.

---

## 4. Explicit non-goals

P0-T005 SHALL NOT implement or decide:

- the final BIM domain model;
- Wall/Door/Slab/Room/Family production entities;
- full model-to-IFC mapping;
- full IFC-to-model mapping;
- IFC editing workflows;
- IFC federation;
- clash detection;
- BCF;
- IDS;
- MVD/view-definition support;
- IFC validation/certification;
- georeferencing architecture;
- full property-set framework;
- material/style mapping;
- full geometry conversion between OCCT and IFC;
- topological naming/reference semantics;
- dependency-graph semantics;
- IFC persistence inside SQLite;
- DWG or RVT interoperability;
- desktop/UI workflows;
- Python runtime or IfcOpenShell Python bindings;
- cloud services;
- byte-for-byte IFC file reproducibility.

These remain outside the P0-T005 spike.

---

## 5. Core architecture decisions

### AG-P0T005-001 — Sole IfcOpenShell owner

`src/interop/ifc/**` SHALL be the only first-party production module permitted to include, reference, instantiate, or link IfcOpenShell C++ APIs.

No other production module may directly consume IfcOpenShell.

### AG-P0T005-002 — Vendor-neutral public surface

Public headers exposed by the IFC module SHALL contain only:

- project-owned first-party types;
- C++ standard-library types;
- approved first-party `foundation` status/result types if required.

Public IFC headers SHALL NOT expose:

- IfcOpenShell classes;
- IfcOpenShell namespaces;
- IfcOpenShell headers;
- Python types/runtime;
- OCCT types;
- SQLite types;
- Qt types;
- bgfx types;
- ODA types.

### AG-P0T005-003 — No model contract expansion in this spike

P0-T005 SHALL NOT create the final model/IFC semantic mapping.

The spike SHALL use a small **P0-only project-owned neutral IFC probe contract** local to the IFC module.

`bim_model` remains unchanged unless Architecture Authority separately approves an architecture amendment.

### AG-P0T005-004 — Dependency direction

The intended target shall be:

```text
bim_ifc
alias: bim::ifc
path: src/interop/ifc
```

Allowed direct dependencies:

```text
bim::foundation
IfcOpenShell C++ dependency
C++ standard library
```

Disallowed direct dependencies unless a later AA amendment explicitly authorizes them:

```text
bim::model
bim::transactions
bim::commands
bim::query
bim::persistence
bim::geometry_api
bim::geometry_occt
bim::viewport
bim::viewport_bgfx
desktop / Qt
SQLite
ODA
Python runtime
```

Rationale: the task proves the IFC library boundary and round-trip mechanism without prematurely coupling IFC to the still-minimal production BIM model.

### AG-P0T005-005 — No third-party exception/type leakage

IfcOpenShell errors/exceptions SHALL be translated at the adapter boundary into first-party status/error results.

Third-party exception types SHALL NOT cross a public first-party API.

### AG-P0T005-006 — C++ integration only

P0-T005 SHALL use the C++ integration path.

IfcOpenShell Python bindings and a Python runtime are explicitly prohibited in P0-T005.

---

## 6. Dependency resolution gate

IfcOpenShell is not currently in `vcpkg.json`.

P0-T005 implementation SHALL begin with a dependency-resolution preflight before any adapter implementation.

### Locked dependency policy

1. Keep the existing vcpkg registry baseline:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

2. Verify whether that exact baseline can resolve a usable IfcOpenShell C++ package/target.

3. If it can:
   - record the exact resolved package version;
   - record the exact CMake package name;
   - record the exact exported target names;
   - record required features/transitive runtime requirements;
   - add only the minimum direct dependency required.

4. If it cannot:
   - STOP;
   - do not use `FetchContent`;
   - do not vendor binaries/source manually;
   - do not add an ad-hoc package registry;
   - do not silently advance the vcpkg baseline;
   - return to Architecture Authority.

A baseline advance or alternate third-party delivery mechanism requires a separate dependency decision/ACR.

At draft time, the exact IfcOpenShell version and exported CMake target names remain **UNRESOLVED BY REPOSITORY EVIDENCE** and must not be guessed.

---

## 7. IFC schema decision

The repository does not currently specify an IFC schema version.

### AA proposed Phase-0 decision — requires Product Authority approval with this gate

For P0-T005 only:

```text
Primary spike schema: IFC4
```

The spike SHALL NOT claim general support for IFC2X3, IFC4X3, or every IfcOpenShell-supported schema.

If the resolved dependency at the frozen baseline cannot provide the required IFC4 read/write path, STOP and return to AA rather than silently changing the schema target.

This schema choice is a Phase-0 spike constraint, not a long-term product limitation.

---

## 8. Neutral P0 IFC probe contract

The public spike contract SHALL be intentionally narrow and clearly marked as P0 diagnostic/probe surface rather than the final product IFC API.

Conceptually, the neutral input/output shall carry only enough data to prove the round trip, such as:

```text
project name
one physical/proxy element identity
element GlobalId
element name
element object/type label
small deterministic set of scalar properties
```

The final exact DTO names and field spelling belong in the Implementation Brief, but the following are locked:

- all fields must be first-party/stdlib types;
- no IfcOpenShell handle/entity type in the public contract;
- no final BIM-model semantics may be implied;
- no generic arbitrary-IFC object graph may be exposed publicly.

---

## 9. Round-trip proof

### AG-P0T005-007 — Required round-trip shape

The authoritative proof SHALL perform:

```text
neutral P0 seed
    -> export through bim::ifc
    -> temporary .ifc file
    -> reopen/import through bim::ifc
    -> neutral P0 result
    -> invariant comparison
```

### AG-P0T005-008 — Required invariants

At minimum, the proof SHALL demonstrate preservation of:

1. IFC schema identity = configured P0-T005 schema;
2. project identity/name selected by the probe;
3. exactly the expected probe element is found after reopen;
4. element `GlobalId` survives round trip;
5. element name survives round trip;
6. element object/type label survives round trip;
7. all configured scalar probe properties survive with exact logical values;
8. export succeeds to a new file;
9. reopened import succeeds from that file.

### AG-P0T005-009 — What round trip does not mean

The acceptance proof SHALL NOT require:

- byte-identical STEP text;
- identical entity numbers;
- identical serialization order;
- stable whitespace;
- stable header timestamps;
- a complete production semantic mapping.

---

## 10. File and temporary-artifact policy

All IFC test/evidence files SHALL be created under unique temporary/output directories outside the source tree.

Required hygiene:

- no `.ifc` files committed as generated runtime output;
- no stale generated output may cause a false PASS;
- every required test creates or receives a unique test path;
- cleanup is deterministic where the process owns the path;
- source-tree artifact scan must confirm no generated IFC files remain.

A small hand-authored IFC fixture may be committed only if the Implementation Brief explicitly identifies it as controlled test input and records its purpose and hash.

---

## 11. Failure semantics

The adapter SHALL fail cleanly for:

- missing input file;
- unreadable file;
- malformed IFC content;
- unsupported schema relative to the P0-T005 locked scope;
- export path/open failure;
- dependency/library parse or serialization failure.

The failure contract SHALL:

- return first-party status/error information;
- not crash;
- not leak a third-party exception across a public boundary;
- not partially claim a successful round trip.

---

## 12. Architecture enforcement extensions

P0-T005 SHALL extend the architecture checker beyond R1–R13.

### Proposed R14 — IFC_OPEN_SHELL_ONLY_IFC_OWNER

Goal:

```text
IfcOpenShell C++ API/vendor tokens may appear only under src/interop/ifc/**
```

The rule shall follow the existing sole-owner pattern used for OCCT, Qt, bgfx, and SQLite.

It SHALL be comment-aware and SHALL use identifier/boundary-aware matching where required to avoid the false-positive class already observed in older ownership rules.

A dedicated negative fixture SHALL prove a real IfcOpenShell ownership violation outside `src/interop/ifc/**` is rejected.

### Proposed R15 — IFC_PUBLIC_NEUTRAL

Goal:

```text
src/interop/ifc/include/** exposes no IfcOpenShell/vendor type/header
```

A dedicated negative fixture SHALL prove a vendor type/header leak is rejected.

Exact vendor token lists SHALL be finalized only after the dependency-resolution preflight reveals the actual resolved C++ package/header/namespace shape. They SHALL then be frozen in the Implementation Brief before source implementation.

Existing R1/R2 protection for model/foundation remains in force.

---

## 13. Build-system requirements

If dependency resolution passes, P0-T005 may add:

```text
add_subdirectory(src/interop/ifc)
```

to the root build composition.

The IFC target SHALL:

- be a first-party C++20 target;
- apply repository warning policy;
- apply repository sanitizer policy where supported;
- privately link the resolved IfcOpenShell C++ target;
- expose only vendor-neutral include paths;
- not globally modify compiler flags;
- not globally add third-party include directories.

No dependency version/baseline change may be hidden inside CMake logic.

---

## 14. Required tests

The eventual Implementation Brief SHALL require at least:

```text
unit_ifc_probe_contract
integration_ifc_round_trip
integration_ifc_missing_file
integration_ifc_malformed_input
integration_ifc_unsupported_schema
integration_ifc_evidence
arch_ifc_openshell_only_ifc_owner
arch_p0_t005_ifc_owner_fixture_rejected
arch_ifc_public_neutral
arch_p0_t005_ifc_public_fixture_rejected
```

Names may receive a minimum wording adjustment in the Brief if CMake conventions require it, but test coverage may not be weakened.

---

## 15. Evidence executable

P0-T005 SHALL provide a deterministic evidence executable/job producing machine-readable JSON.

Required logical evidence fields:

```text
schema_expected
schema_reopened
export_succeeded
reopen_succeeded
project_identity_preserved
element_found
global_id_preserved
name_preserved
object_type_preserved
properties_preserved
missing_file_rejected
malformed_input_rejected
unsupported_schema_rejected
overall_passed
```

The exact JSON spelling/order shall be frozen in the Implementation Brief.

Evidence must be generated from a fresh temporary path and run at least twice to prove deterministic logical output. Fields affected by temporary path or timestamps must not be emitted as unstable evidence values.

---

## 16. Dependency/license evidence

Because P0-T005 introduces a new third-party dependency if resolution succeeds, the task SHALL include dependency/license evidence.

The task shall:

- record exact direct dependency identity/version;
- capture applicable license inventory;
- confirm the new dependency is present in the project dependency manifest;
- not mutate or normalize unrelated third-party license files;
- classify any license-inventory mutation before accepting it as task scope.

---

## 17. Required validation layers

Before candidate freeze, the accepted implementation must pass:

1. format;
2. fresh configure with the authoritative Windows CI preset;
3. full build;
4. full CTest;
5. static analysis;
6. complete architecture job including new R14/R15 tests;
7. IFC-specific tests;
8. deterministic evidence run;
9. source-tree generated-IFC hygiene check;
10. dependency/license inventory applicable to the new dependency;
11. exact candidate-path/hash lock.

Any failure is classified by Architecture Authority before implementation changes are returned to Claude.

---

## 18. Expected implementation footprint

The eventual implementation is expected to touch only a controlled subset similar to:

```text
vcpkg.json                         [only if frozen baseline resolves IfcOpenShell]
CMakeLists.txt
src/interop/ifc/README.md          [replace/update placeholder]
src/interop/ifc/CMakeLists.txt
src/interop/ifc/include/bim/ifc/**
src/interop/ifc/src/**
tests/unit/**
tests/integration/**
tests/architecture/**
tests/fixtures/p0_t005_*/**
tools/architecture_checker.cmake
scripts/ci/**                      [only if an IFC-specific evidence/hygiene job is required]
```

No modification to `src/model/**`, `src/persistence/**`, `src/geometry/**`, `src/viewport/**`, or `src/desktop/**` is authorized by this gate unless AA later approves a minimum architecture amendment.

---

## 19. Acceptance criteria

### Architecture

**AC-001** `src/interop/ifc/**` is the sole first-party IfcOpenShell owner.
**AC-002** public IFC headers expose no IfcOpenShell types/headers.
**AC-003** model public headers remain IfcOpenShell-free.
**AC-004** foundation remains dependency-free from IfcOpenShell/project modules.
**AC-005** no new coupling from IFC to persistence/transactions/commands/query/viewport/desktop.
**AC-006** no Python runtime is introduced.
**AC-007** no vcpkg baseline change occurs without separate ACR.

### Dependency

**AC-008** exact IfcOpenShell package/version/targets are proven from the frozen baseline before adapter implementation.
**AC-009** if frozen-baseline resolution fails, implementation stops rather than using an unapproved alternate delivery path.

### Functional spike

**AC-010** neutral P0 seed exports successfully to IFC.
**AC-011** exported IFC reopens successfully.
**AC-012** configured schema is verified after reopen.
**AC-013** project identity/name invariant passes.
**AC-014** probe element is found after reopen.
**AC-015** GlobalId invariant passes.
**AC-016** name invariant passes.
**AC-017** object/type invariant passes.
**AC-018** scalar property invariants pass.

### Failure behavior

**AC-019** missing file rejects cleanly.
**AC-020** malformed IFC rejects cleanly.
**AC-021** unsupported schema rejects cleanly.
**AC-022** third-party exceptions/types do not cross the public boundary.

### Enforcement/evidence

**AC-023** R14 real-tree pass + negative fixture rejection.
**AC-024** R15 real-tree pass + negative fixture rejection.
**AC-025** complete existing architecture suite still passes.
**AC-026** deterministic JSON evidence reports `overall_passed=true`.
**AC-027** generated IFC files do not remain in the source tree.
**AC-028** full Windows CI configure/build/CTest passes.
**AC-029** static analysis passes.
**AC-030** exact candidate path/hash freeze passes.

---

## 20. Stop conditions

Implementation SHALL stop and return to Architecture Authority if any of the following occurs:

- IfcOpenShell cannot be resolved through the frozen dependency policy;
- resolution requires a vcpkg baseline advance;
- resolution requires Python;
- resolution requires manual vendoring/FetchContent/ad-hoc registry;
- public IFC surface cannot be kept vendor-neutral;
- required IFC4 read/write path is unavailable;
- adapter requires model contract changes to satisfy the spike;
- adapter requires OCCT coupling for the locked proof;
- architecture enforcement requires weakening an existing rule;
- implementation expands into DWG/RVT/persistence/UI/topological naming;
- a required dependency/license condition is unclear.

---

## 21. Governance lifecycle

After Product Authority approval of this gate:

1. Architecture Authority materializes the controlled gate/task records.
2. Dependency-resolution evidence is obtained against the exact baseline.
3. Architecture Authority freezes any dependency-specific package/target facts.
4. Architecture Authority issues the Implementation Brief.
5. Product/Architecture Authority separately issues Implementation Authorization.
6. Claude implements minimum delta in an isolated task worktree.
7. Authoritative validation executes.
8. Candidate is frozen.
9. Architecture Authority reviews.
10. Kimi receives a read-only independent-review packet only after explicit authorization.
11. Implementation commit and main integration require separate AA authorizations.
12. Push remains separately authorized.

---

## 22. Approved Phase-0 decisions

Product Authority approval on 2026-09-16 locks these Phase-0 decisions:

```text
1. P0-T005 is an IfcOpenShell C++ boundary + round-trip spike only.
2. src/interop/ifc is the sole IfcOpenShell owner.
3. Public IFC contract is project-owned/vendor-neutral.
4. P0-T005 does not modify the production model contract.
5. IFC module direct first-party dependency is foundation only.
6. Python is prohibited.
7. Primary spike schema is IFC4.
8. Round trip is neutral seed -> IFC -> reopen -> neutral invariant comparison.
9. R14/R15 architecture rules are required.
10. Frozen vcpkg baseline must not move automatically.
11. Failure to resolve IfcOpenShell at the frozen baseline is a STOP/AA decision.
12. ACR = NONE at Architecture Gate draft time.
```

---

## 23. Current status

```text
Discovery                    = CLOSED / PASS
Architecture Gate Draft      = SUPERSEDED BY APPROVED v1.0
Architecture Gate Approved   = YES
Implementation Brief         = NOT AUTHORIZED
Implementation               = NOT AUTHORIZED
Branch/worktree materialized = NO
ACR                          = NONE
```

---

## 24. Product Authority approval

Product Authority approved `BIM-AG-P0-T005 v1.0` on `2026-09-16`.

This approval locks the architecture decisions in this package.

It does **not** authorize production implementation.

The next authorized lifecycle action is controlled governance materialization,
followed by read-only dependency-resolution evidence against the frozen vcpkg
baseline.

```text
Architecture Gate       = APPROVED
Governance materialize  = AUTHORIZED
Dependency resolution   = NEXT AFTER MATERIALIZATION
Implementation Brief    = NOT RELEASED
Implementation          = NOT AUTHORIZED
ACR                     = NONE
```
