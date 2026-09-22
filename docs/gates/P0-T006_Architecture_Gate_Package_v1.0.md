# BIM Platform - P0-T006 DWG / ODA Architecture Gate Package

**Gate ID:** BIM-AG-P0-T006 v1.0
**Status:** FROZEN / APPROVED
**Decision date:** 2026-09-22
**Task:** P0-T006 - DWG / ODA Evaluation
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi - NOT YET AUTHORIZED
**Architecture Change Record:** NONE

---

## 1. Authority and lifecycle state

This Architecture Gate freezes the P0-T006 technical direction after the
controlled ODA Drawings evaluation and semantic-fidelity investigation.

Current state:

```text
P0-T006 discovery/evaluation          = CLOSED / PASS
ODA acquisition/activation evidence   = CLOSED / PASS
VS2022/v143 compatibility             = PROVEN
DWG read                              = PROVEN
DWG write                             = PROVEN
AC1018 same-version write             = PROVEN
B10 semantic-fidelity investigation   = CLOSED / PASS
Architecture Gate                     = FROZEN / APPROVED
Implementation Brief                  = RELEASED WITH THIS GOVERNANCE DELTA
Implementation                        = NOT AUTHORIZED
Independent Review                    = NOT AUTHORIZED
Main integration                      = NOT AUTHORIZED
Push                                  = NOT AUTHORIZED
ACR                                   = NONE
```

Architecture Gate approval does not authorize implementation.

A separate Implementation Authorization must be issued only after the
governance commit that materializes this Gate and the Implementation Brief is
known.

---

## 2. Objective

P0-T006 shall implement the smallest production-quality Phase-0 DWG
interoperability spike proving:

```text
vendor-neutral first-party DWG probe contract
        ->
private ODA Drawings adapter
        ->
controlled DWG open
        ->
explicit same-version AC1018 write
        ->
reopen
        ->
neutral semantic-invariant verification
```

This is an interoperability spike, not a final CAD/BIM model-import subsystem.

---

## 3. Frozen architectural direction

AG-011 remains authoritative:

```text
NO IN-HOUSE DWG PARSER
```

ODA Drawings is the accepted DWG implementation direction for P0-T006.

The first-party implementation boundary is:

```text
src/interop/dwg/**
```

Only that boundary may directly include ODA headers, use ODA C++ types, call
ODA APIs, or link ODA Drawings libraries.

No ODA type may leak into project-owned public contracts outside that private
boundary.

---

## 4. Repository and toolchain baseline

Pre-governance-release parent:

```text
Task branch:
task/P0-T006-dwg-oda-spike

Task HEAD:
fe2dfa813070df0875eaff93ad40cd2af2ec1dad

Task tree:
611728d8acc14dfd7b46187b1a1d82f3d1925196

Canonical main HEAD:
fe2dfa813070df0875eaff93ad40cd2af2ec1dad
```

Frozen project toolchain:

```text
Visual Studio 2022 Build Tools
MSVC toolset 14.44.35207
cl 19.44.35228
link 14.44.35228
Windows SDK 10.0.26100.0
CMake 4.4.2
Ninja 1.12.1
first-party language level C++20
Windows x64
```

Frozen vcpkg baseline:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

P0-T006 does not change the vcpkg baseline.

---

## 5. Frozen ODA dependency identity

Accepted evaluation dependency:

```text
Product family:
ODA Drawings / DWG-DXF native C++

ODA version:
27.7.0.0

Platform:
Windows x64

Vendor delivery:
vc16_amd64dll

Vendor toolset generation:
v142-compatible delivery

Evaluation package SHA256:
7460ae11e05a2627dc7b13f078a875fe32e20e86ab7eb05d798ded4325554ac3
```

VS2022/v143 compilation and linking against this vendor delivery were proven
during P0-T006 evaluation.

The ODA SDK is proprietary and shall remain outside Git.

No SDK headers, libraries, DLLs, archives, activation material, activation
tokens or license secrets may be committed.

---

## 6. Licensing boundary

The technical evaluation used an ODA trial/evaluation delivery.

This Gate does not claim that trial terms authorize commercial redistribution.

```text
technical adapter implementation = allowed after separate authorization
technical local validation       = allowed after separate authorization
commercial deployment            = NOT established by this Gate
runtime redistribution           = NOT established by this Gate
production licensing             = separate business/legal prerequisite
```

---

## 7. First-party module ownership

Production target:

```text
bim_dwg
alias: bim::dwg
```

Public contract:

```text
src/interop/dwg/include/bim/dwg/**
```

Private ODA implementation:

```text
src/interop/dwg/src/detail/**
```

Expected dependency direction:

```text
bim::dwg
    ->
bim::foundation
    +
ODA Drawings private dependency
```

No dependency from `bim::dwg` to model, transactions, persistence, IFC,
viewport, Qt, bgfx or OCCT is authorized.

---

## 8. Build integration contract

The root project shall add:

```text
BIM_ENABLE_DWG
```

with default `OFF`.

When enabled, the operator shall provide:

```text
BIM_ODA_DRAWINGS_ROOT
```

The DWG module must fail clearly when the required SDK structure or minimum
Drawings read/write libraries cannot be resolved.

No arbitrary global ODA installation fallback is permitted.

Only the minimum Drawings read/write runtime closure is authorized.

TF/revision-control, Civil, Mechanical, Map, BimRv, RVT, Web and unrelated ODA
modules are outside P0-T006 production scope.

---

## 9. Public contract rule

All public `bim::dwg` types must be project-owned and vendor-neutral.

Public headers must not expose ODA headers or ODA types such as `OdDb*`,
`OdRx*`, `OdGe*`, `OdString`, `OdError`, ODA smart pointers,
`OdDbObjectId` or `OdDbHandle`.

The Phase-0 surface is a diagnostic/probe contract only.

---

## 10. Version-preservation rule

Binary equality is not a fidelity contract.

ODA object handles are not stable application identities.

Native object-count equality is not a fidelity contract.

Accepted controlled path:

```text
source format = AC1018
write mapping = ODA vAC18
reopened      = AC1018
```

The production implementation must not use `OdDb::kDHL_CURRENT` as an
implicit preserve-source-version policy.

Unsupported/unmapped versions must fail explicitly.

---

## 11. Controlled fidelity fixture

Authoritative external fixture:

```text
Logical name:
OdWriteEx XRef.dwg

Bytes:
31437

SHA256:
033D87D748533DEB4F37C10A755AC9861416FD1AA7494DD3CB86184E7222EB94

DWG version:
AC1018
```

The fixture remains outside Git.

Validation receives the fixture through:

```text
BIM_P0_T006_DWG_FIXTURE
```

The authoritative Windows validation harness must verify its hash before and
after execution.

---

## 12. Accepted B10 fidelity classification

Controlled AC1018 investigation established:

```text
native objects before       = 101
native objects after        = 111
native object deletions     = 0
native class removals       = 0
```

Materialized additions:

```text
AcDbDictionary        +2
AcDbDictionaryVar     +6
AcDbRegAppTableRecord +1
AcDbSun               +1
```

The inspected user-visible AcDbMText payload remained field-identical.

Viewport visual-style handle churn preserved the inspected semantic
projection.

Accepted contract:

```text
user-content preservation        = REQUIRED
semantic fidelity                = REQUIRED
binary equality                  = NOT REQUIRED
handle stability                 = NOT REQUIRED
internal object-count equality   = NOT REQUIRED
```

No universal lossless-DWG claim is made.

---

## 13. Controlled semantic probe

The fixture contains user-visible MText whose plain text includes:

```text
External Reference
```

Locate it semantically rather than through a persistent handle.

Verify preservation of text, position, normal/orientation, X direction, text
height, width/box dimensions, attachment/alignment and resolved text-style
semantics.

Fixture-local numeric tolerance:

```text
absolute tolerance = 1e-9
```

This is not a platform-wide tolerance decision.

---

## 14. Failure behavior

The first-party boundary must translate ODA failures to project-owned status.

At minimum fail safely for:

```text
missing input
non-DWG / unreadable input
input path == output path
unsafe overwrite request
ODA initialization failure
required runtime/module unavailable
unsupported/unmapped write version
write failure
reopen failure
controlled semantic invariant failure
```

No failure may modify the input fixture in place.

---

## 15. Architecture enforcement

Extend the existing checker without weakening R1-R15.

### R16 - ODA_DRAWINGS_ONLY_DWG_OWNER

ODA Drawings API/header/type usage is permitted only beneath
`src/interop/dwg/**`.

A controlled negative fixture must prove rejection elsewhere.

### R17 - DWG_PUBLIC_NEUTRAL

Public headers beneath `src/interop/dwg/include/bim/dwg/**` must remain
ODA-neutral.

A controlled negative fixture must prove leakage rejection.

Use the repository's accepted comment-insensitive scanning convention.

---

## 16. Required implementation tests

At minimum:

```text
unit_dwg_probe_contract
integration_dwg_missing_file
integration_dwg_same_path_rejected
integration_dwg_vendor_fixture_read
integration_dwg_same_version_round_trip
integration_dwg_evidence

arch_p0_t006_oda_owner
arch_p0_t006_oda_owner_fixture_rejected
arch_p0_t006_dwg_public_neutral
arch_p0_t006_dwg_public_fixture_rejected
```

---

## 17. Required evidence

Final controlled evidence must establish:

```text
fixture identity precheck PASS
ODA SDK identity/configuration PASS
fresh configure PASS
fresh build PASS
full applicable CTest PASS
R16 positive + negative PASS
R17 positive + negative PASS
AC1018 source version PASS
explicit vAC18 write mapping PASS
AC1018 reopen PASS
controlled MText semantic invariants PASS
input fixture SHA256 unchanged PASS
ODA/vendor files committed = NONE
main unchanged + clean PASS
candidate path-set controlled PASS
```

---

## 18. Source and dependency hygiene

P0-T006 shall not:

```text
change vcpkg baseline
vendor ODA source
vendor ODA binaries
commit trial archive
commit activation material
commit ODA runtime DLLs
introduce an in-house DWG parser
add unrelated dependencies
expand canonical BIM model contracts
depend on ODA handles as persistent identity
```

---

## 19. Expected implementation footprint

Implementation Authorization may permit minimum-delta changes only in:

```text
CMakeLists.txt
src/interop/dwg/**
tests/unit/CMakeLists.txt
tests/unit/unit_dwg_probe_contract.cpp
tests/integration/CMakeLists.txt
tests/integration/*dwg*
tests/architecture/CMakeLists.txt
tests/fixtures/bad_architecture/**
tools/architecture_checker.cmake
scripts/ci/dwg-spike.ps1
LICENSES.md
```

Any required edit outside this footprint is a STOP condition.

---

## 20. Explicit non-goals

P0-T006 does not implement general BIM-model import from DWG, DWG-to-IFC
conversion, CAD editing UI, DWG rendering, RVT/BimRv/Civil/Mechanical/Map/Web
integration, a custom DWG parser, handle-based persistent application identity
or commercial deployment packaging.

---

## 21. Architecture risks

Known controlled risks:

1. ODA is proprietary and external to Git.
2. Trial evidence does not establish commercial/runtime redistribution rights.
3. Default CI may not possess ODA, therefore DWG build remains explicit opt-in.
4. ODA may materialize defaults and change internal handles/object population.
5. DWG handles cannot be platform persistent identity.

---

## 22. Implementation governance

Implementation proceeds only after:

1. this Gate and the Implementation Brief are inspected;
2. both are materialized in a governance-only commit;
3. Architecture Authority records that exact commit/tree;
4. a separate Implementation Authorization is issued against that state.

Claude must not begin production edits earlier.

Kimi remains unauthorized until candidate validation.

---

## 23. Final gate decision

```text
P0-T006 Architecture Gate       = FROZEN / APPROVED
ODA Drawings direction          = ACCEPTED
AG-011 no in-house parser       = RETAINED
B10 fidelity investigation      = CLOSED / PASS
ACR                             = NONE

Implementation Brief            = RELEASED WITH GOVERNANCE DELTA
Implementation                  = NOT AUTHORIZED
Claude                          = NOT AUTHORIZED
Kimi                            = NOT AUTHORIZED
Candidate commit                = NOT AUTHORIZED
Main integration                = NOT AUTHORIZED
Push                            = NOT AUTHORIZED
```

Next lifecycle gate is governance materialization, followed by a separately
issued Implementation Authorization bound to that exact commit/tree.