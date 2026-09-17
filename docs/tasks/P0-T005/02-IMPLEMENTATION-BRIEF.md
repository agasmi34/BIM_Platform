# P0-T005 — IFC Spike Implementation Brief

**Brief ID:** BIM-TASK-P0-T005-CLAUDE v1.0
**Status:** RELEASED — IMPLEMENTATION NOT AUTHORIZED
**Release date:** 2026-09-17
**Task:** P0-T005 — IFC Spike
**Architecture Gate:** BIM-AG-P0-T005 v1.0 — APPROVED
**Architecture Change Record:** ACR-P0-T005-001 — APPROVED
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi — NOT YET AUTHORIZED
**Architecture Authority:** Product Authority + ChatGPT

---

## 1. Authority and execution state

This brief freezes the implementation contract for P0-T005.

Release of this brief does **not** authorize production implementation.

Claude must not modify production code until a separate Architecture Authority
Implementation Authorization is issued against the committed brief-release
baseline.

Current lifecycle state:

```text
P0-T005 Discovery                    = CLOSED / PASS
Architecture Gate                    = APPROVED
ACR-P0-T005-001                      = APPROVED
Dependency Resolution Phase A        = CLOSED / EXPECTED STOP
Dependency Resolution Phase B        = CLOSED / PASS
Dependency Resolution Phase C        = CLOSED / PASS
Implementation Brief                 = RELEASED
Implementation                       = NOT AUTHORIZED
Independent Review                   = NOT AUTHORIZED
Main integration                     = NOT AUTHORIZED
Push                                 = NOT AUTHORIZED
```

---

## 2. Objective

Implement the smallest production-quality Phase-0 IFC interoperability spike
that proves:

```text
neutral first-party IFC probe contract
        ->
private IfcOpenShell C++ adapter
        ->
IFC4 export
        ->
filesystem round trip
        ->
IFC4 reopen/import
        ->
semantic invariant verification
```

The spike proves the dependency boundary and controlled IFC4 round trip. It is
not the final IFC subsystem, geometry pipeline, authoring model, or import/export
product UX.

---

## 3. Frozen dependency identity

IfcOpenShell is consumed under ACR-P0-T005-001 from exact pinned upstream source.

```text
Repository:
https://github.com/IfcOpenShell/IfcOpenShell.git

Ref:
refs/tags/ifcconvert-0.8.5

Commit:
16723d11cab9bc8a13b4e025a00d39445ccc462e

Tree:
3b3e7bd633c14333a07f6c0e498bbf8483d23ad6

Version:
0.8.5
```

The BIM Platform vcpkg baseline remains exactly:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

No baseline advancement is permitted.

---

## 4. Proven dependency build contract

The accepted Windows dependency proof established:

```text
IfcOpenShell dependency language level = C++17
BIM Platform / consumer language level = C++20

Schema                             = IFC4 only
BUILD_SHARED_LIBS                  = OFF for Phase-0 proof
BUILD_IFCGEOM                      = OFF
BUILD_IFCPYTHON                    = OFF
BUILD_CONVERT                      = OFF
BUILD_GEOMSERVER                   = OFF
WITH_OPENCASCADE                   = OFF
WITH_CGAL                          = OFF
IFCXML_SUPPORT                     = OFF
WITH_ROCKSDB                       = OFF
COLLADA_SUPPORT                    = OFF
GLTF_SUPPORT                       = OFF
HDF5_SUPPORT                       = OFF
WITH_PROJ                          = OFF
USD_SUPPORT                        = OFF
WITH_RELATIONSHIP_VALIDATION       = OFF
WITH_ZSTD                          = OFF
USE_MMAP                           = OFF
USE_CCACHE                         = OFF
SCHEMA_VERSIONS                    = 4
VERSION_OVERRIDE                   = ON
```

The installed package exports:

```text
IfcOpenShell::IfcParse
```

The accepted Phase-C proof successfully built `IfcParse`, installed the package,
compiled a separate C++20 consumer, and completed IFC4 parse/write/reopen.

This C++17 dependency build does not lower the BIM Platform language level. All
first-party BIM Platform targets remain C++20.

The Phase-0 static IfcParse build is a verified technical integration shape, not
a final commercial static-vs-shared distribution decision.

---

## 5. Dependency delivery rule

IfcOpenShell source must not be vendored into the BIM Platform repository.

No `FetchContent` of IfcOpenShell is permitted.

No Git submodule is permitted.

No committed IfcOpenShell binaries are permitted.

Provide a controlled PowerShell dependency bootstrap that:

1. resolves the exact repository/ref/commit above;
2. rejects a mismatched commit;
3. builds outside the repository in a short external path;
4. configures the exact core-only C++17 options above;
5. installs to an external prefix;
6. proves the installed `IfcOpenShell::IfcParse` package;
7. does not change the project vcpkg baseline;
8. is safe to re-run against the same exact dependency identity.

The script must use a short external build root on Windows. Do not reproduce the
previous long-path build layout.

The project CMake integration shall consume the installed package from an
explicit operator/CI-provided external prefix or `IfcOpenShell_DIR`. It must
fail clearly if the approved external dependency is unavailable.

It must not silently search arbitrary global installations as a fallback.

---

## 6. vcpkg support dependency closure

At the frozen vcpkg baseline, the external IfcParse build/consumer proof required
the following direct support set:

```text
boost-system
boost-program-options
boost-regex
boost-thread
boost-date-time
boost-iostreams
boost-uuid
boost-logic
boost-scope-exit
boost-multi-index
boost-circular-buffer
boost-filesystem
boost-locale
boost-math
boost-property-tree
boost-variant
eigen3
```

These may be added to the BIM Platform `vcpkg.json` at the existing frozen
baseline if required by the implementation and installed package contract.

Do not add IfcOpenShell itself to `vcpkg.json`.

Do not change `vcpkg-configuration.json` baseline.

Do not add dependencies that are not demonstrated necessary by the accepted
dependency proof or the implementation's actual compile/link contract.

---

## 7. First-party module ownership

The sole first-party owner of IfcOpenShell APIs is:

```text
src/interop/ifc/**
```

No IfcOpenShell header, namespace, type, exception, handle, ownership model, or
generated schema type may appear outside that owner in production source.

The IFC module target shall be:

```text
bim_ifc
alias: bim::ifc
```

Direct first-party dependency:

```text
bim::foundation
```

Direct third-party dependency:

```text
IfcOpenShell::IfcParse
```

No direct dependency is authorized from `bim::ifc` to:

```text
model
transactions
commands
query
persistence
geometry
viewport
desktop
Qt
bgfx
SQLite
ODA
Python
```

No OCCT coupling is authorized in P0-T005.

---

## 8. Public contract

Public IFC headers must be first-party and vendor-neutral.

Use a small P0-T005 probe contract local to the IFC module. Do not expand the
canonical model contract.

The public contract must represent, at minimum:

- target schema identifier;
- project identity/name;
- one expected element identity;
- element GlobalId;
- element name;
- element object-type label;
- a small deterministic scalar-property set;
- export/reopen outcome;
- preservation result;
- first-party status/error information.

The public API must not expose:

- IfcOpenShell types;
- generated IFC schema classes;
- Boost types;
- filesystem implementation details;
- third-party exceptions.

Third-party errors must be translated at the adapter boundary into project-owned
status/error semantics.

---

## 9. Controlled round-trip scenario

The positive integration proof shall create a deterministic, geometry-free IFC4
seed containing:

1. one IFC project with deterministic project identity/name;
2. one simple non-geometric IFC element suitable for the spike;
3. deterministic element GlobalId derived from a fixed canonical UUID or another
   deterministic, standards-valid mechanism;
4. deterministic element name;
5. deterministic object-type label;
6. deterministic scalar property values.

The scenario must then:

```text
construct seed
-> export IFC4 to controlled temporary .ifc
-> close writer state
-> reopen the exported file
-> locate expected project
-> locate expected element
-> read neutral values
-> compare all required invariants
```

No geometry conversion is required.

No byte-identical STEP serialization is required.

---

## 10. Required semantic invariants

The round trip passes only if all of the following are proven after reopen:

```text
schema_expected       = IFC4
schema_reopened       = IFC4
export_success        = true
reopen_success        = true
project_found         = true
project_identity      = preserved
project_name          = preserved
element_found         = true
element_global_id     = preserved
element_name          = preserved
element_object_type   = preserved
scalar_properties     = preserved
overall               = pass
```

Do not weaken an invariant merely to make the spike pass.

---

## 11. Required failure behavior

Add controlled tests proving clean first-party rejection of:

```text
missing IFC file
malformed IFC input
unsupported schema input
export/write failure where deterministically injectable
dependency/library failure where deterministically injectable
```

At minimum, the three Architecture-Gate failure cases are mandatory:

```text
integration_ifc_missing_file
integration_ifc_malformed_input
integration_ifc_unsupported_schema
```

Tests must not rely on operator machine state outside controlled fixtures.

---

## 12. Required tests

The implementation must register and pass these exact logical tests:

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

Additional narrowly scoped tests are allowed only when needed to prove the same
contract.

Do not rename the required tests.

---

## 13. Architecture enforcement

Extend the existing architecture checker without weakening any existing rule.

Add:

```text
R14 = IFC_OPEN_SHELL_ONLY_IFC_OWNER
R15 = IFC_PUBLIC_NEUTRAL
```

### R14

IfcOpenShell-specific tokens are permitted only under:

```text
src/interop/ifc/**
```

The rule must use identifier-aware/token-boundary matching sufficient to avoid
the known latent R12/R13 substring issue.

Do not repair or refactor R12/R13 as part of P0-T005.

Add a controlled negative fixture that proves an IfcOpenShell token outside the
IFC owner is rejected.

### R15

Public headers under the IFC module must remain vendor-neutral.

At minimum reject public-header exposure of:

```text
IfcOpenShell
IfcParse
IfcUtil
Ifc4
third-party IFC generated types/namespaces
```

Use a controlled negative fixture proving the checker rejects leakage.

Do not broaden R15 into unrelated repository policy.

---

## 14. CMake integration

Only after the external dependency prefix is available:

- add `src/interop/ifc` to the repository build;
- define `bim_ifc` / `bim::ifc`;
- consume `IfcOpenShell::IfcParse` privately;
- keep first-party public include directories vendor-neutral;
- keep BIM first-party targets at C++20;
- do not rebuild IfcOpenShell as part of ordinary first-party target compilation;
- fail with an actionable dependency-bootstrap message when the exact package is
  missing.

Do not use global include/link directories.

Do not copy IfcOpenShell headers into the project.

---

## 15. Evidence contract

Produce deterministic P0-T005 evidence with, at minimum, these logical fields:

```text
task
schema_expected
schema_reopened
export_success
reopen_success
project_found
project_identity_preserved
project_name_preserved
element_found
element_global_id_preserved
element_name_preserved
element_object_type_preserved
scalar_properties_preserved
missing_file_rejected
malformed_input_rejected
unsupported_schema_rejected
overall
```

Evidence must not include unstable temporary paths, wall-clock timestamps,
random values, machine-specific absolute paths, or nondeterministically ordered
collections unless explicitly normalized before serialization.

The evidence validation test must fail when any required field is absent or any
required boolean is false.

---

## 16. Source and dependency hygiene

The implementation must preserve:

```text
IfcOpenShell upstream source in repository     = NONE
IfcOpenShell committed binaries                = NONE
Python/PyPI/Conda product integration          = NONE
IfcGeom                                        = NONE
IfcOpenShell OpenCascade dependency            = NONE
CGAL                                           = NONE
project vcpkg baseline change                  = NONE
model contract expansion                       = NONE
```

Capture the applicable pinned-source license identities in the project's
third-party dependency/license evidence without rewriting unrelated license
inventory.

Pinned root license identities already proven:

```text
COPYING:
3237699d9e6781c85877365abd9493ca132a0fd31f43e1a3278b8e75f3654af4

COPYING.LESSER:
7d3a95e5e06978064ed3f8e2b7c8f845e7fd8a405294727cc708f94cb83b8059
```

Do not make a new commercial-distribution conclusion in P0-T005.

---

## 17. Expected implementation footprint

Minimum-delta changes are expected only in the following areas:

```text
vcpkg.json                                      if support deps are required
CMakeLists.txt                                  IFC module registration only
src/interop/ifc/**
tests/integration/**                            IFC spike tests/fixtures only
tests/unit/**                                   IFC probe contract only
tests/fixtures/bad_architecture/**              R14/R15 negative fixtures only
tools/architecture_checker.cmake                R14/R15 only
scripts/ci/**                                   IFC evidence/validation only
scripts/dependencies/**                         pinned IfcOpenShell bootstrap only
third_party/licenses/**                         applicable IfcOpenShell notice/evidence only
docs/tasks/P0-T005/**                           implementation lifecycle evidence only
docs/project-control/**                         controlled lifecycle updates only
```

If an implementation requires a production change outside these areas, STOP and
return to Architecture Authority before editing it.

Do not touch P0-T004 deferred findings.

Do not touch P0-T003 deferred findings.

---

## 18. Forbidden implementation shortcuts

The following are expressly prohibited:

```text
FetchContent(IfcOpenShell)
un-pinned clone/build
moving branch/tag consumption without commit lock
Git submodule
vendored IfcOpenShell source
committed IfcOpenShell binaries
Python bindings/runtime
IfcGeom
second OpenCascade runtime through IfcOpenShell
CGAL
project vcpkg baseline movement
global system-install dependency assumption
public third-party IFC types
model-layer IFC coupling
direct persistence writes
architecture-checker weakening
test removal or expected-failure masking
```

No warning suppression shall be added merely to hide upstream warnings.

---

## 19. Accepted dependency proof evidence

Dependency Resolution Phase C closed PASS with:

```text
Classification:
IFCOPENSHELL_IFCPARSE_WINDOWS_CORE_BUILD_SMOKE_PASS

Verdict:
PASS_FOR_ARCHITECTURE_AUTHORITY_IMPLEMENTATION_BRIEF_DECISION

External prefix:
C:\Users\abdallah\AppData\Local\P0T5C\C16-d1946765\ifcopenshell-install

Round-trip IFC SHA256:
c3f4de0988c73b463d6cc3427ee88cbe115e78e0c25a70b5b3a4348412655149

Created IFC SHA256:
38bd5a1ec03b5e1079de30168fc4bdce2cb8323649b8ab3e537a690ec29986be
```

This path is evidence from the dependency proof, not a hard-coded production
dependency path.

---

## 20. Implementation execution discipline

When implementation is separately authorized:

1. Claude works only in `D:\Projects\BIM-Platform-WT-P0-T005`.
2. Claude must begin from the exact authorized brief-release commit.
3. Claude must inspect existing conventions before creating files.
4. Claude implements minimum delta only.
5. Claude does not commit until the Architecture Authority-controlled lifecycle
   authorizes the implementation commit.
6. Build/test/harness failures return to Architecture Authority for
   classification before architectural changes.
7. Claude must not rewrite approved governance or architecture decisions.
8. Kimi is not invoked until the later independent-review gate.

---

## 21. Implementation acceptance gates

A candidate is not eligible for independent review until the Architecture
Authority has evidence for:

```text
candidate path-set freeze
format/lint PASS
fresh configure PASS
fresh build PASS
full CTest PASS
required IFC tests PASS
R14 positive + negative fixture PASS
R15 positive + negative fixture PASS
deterministic evidence PASS
dependency bootstrap identity PASS
source/vendor hygiene PASS
license evidence PASS
main unchanged + clean
task candidate clean at controlled commit
```

Static analysis must include the new first-party IFC production translation
units according to the repository's accepted static-analysis convention.

---

## 22. Stop conditions

STOP and return to Architecture Authority if any of the following occurs:

- exact pinned IfcOpenShell commit cannot be reproduced;
- dependency bootstrap requires a different baseline;
- dependency build requires Python, IfcGeom, OpenCascade, or CGAL;
- C++20 first-party consumer cannot compile against the installed boundary;
- IFC4 read/write/reopen cannot be proven;
- required semantic invariant cannot be preserved;
- an IfcOpenShell type must leak through a first-party public header;
- model contract expansion appears necessary;
- an implementation change outside the authorized footprint appears necessary;
- R14/R15 cannot be added without weakening existing rules;
- license provenance becomes uncertain;
- a new third-party dependency not already justified becomes necessary;
- a production source patch to pinned IfcOpenShell appears necessary.

Do not self-authorize an architectural workaround.

---

## 23. Current gate

```text
Implementation Brief = RELEASED
Implementation       = NOT AUTHORIZED
```

Next gate:

```text
Architecture Authority Implementation Authorization
```

Claude must wait for that separate authorization.
