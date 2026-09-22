# P0-T006 - DWG / ODA Spike Implementation Brief

**Brief ID:** BIM-TASK-P0-T006-CLAUDE v1.0
**Status:** RELEASED - IMPLEMENTATION NOT AUTHORIZED
**Release date:** 2026-09-22
**Task:** P0-T006 - DWG / ODA Evaluation
**Architecture Gate:** BIM-AG-P0-T006 v1.0 - FROZEN / APPROVED
**Architecture Change Record:** NONE
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi - NOT YET AUTHORIZED
**Architecture Authority:** Product Authority + ChatGPT

---

## 1. Authority and execution state

This brief freezes the implementation contract for P0-T006.

Release of this brief does not authorize production implementation.

Claude must not modify production source until Architecture Authority issues a
separate Implementation Authorization against the governance commit/tree that
first contains this released brief.

Current lifecycle:

```text
P0-T006 evaluation                 = CLOSED / PASS
B10 semantic fidelity             = CLOSED / PASS
Architecture Gate                 = FROZEN / APPROVED
Implementation Brief              = RELEASED
Implementation                    = NOT AUTHORIZED
Independent Review                = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

---

## 2. Objective

Implement the smallest production-quality Phase-0 DWG interoperability spike
that proves:

```text
neutral first-party DWG probe API
        ->
private ODA Drawings adapter
        ->
read controlled AC1018 DWG
        ->
explicit same-version vAC18 write
        ->
reopen AC1018
        ->
verify controlled semantic invariants
```

It is not the final DWG import/model architecture.

---

## 3. Frozen external dependency identity

```text
ODA Drawings / DWG-DXF native C++
ODA version               = 27.7.0.0
platform                  = Windows x64
vendor delivery           = vc16_amd64dll
vendor toolset generation = v142-compatible
evaluation package SHA256 =
7460ae11e05a2627dc7b13f078a875fe32e20e86ab7eb05d798ded4325554ac3
```

First-party code remains C++20 and VS2022/v143.

ODA remains external to Git.

No activation token or license secret may be logged or committed.

---

## 4. Dependency delivery rule

Root CMake shall define `BIM_ENABLE_DWG` with default `OFF`.

When enabled, configuration requires `BIM_ODA_DRAWINGS_ROOT` pointing to the
externally installed accepted ODA Drawings Toolkit.

Resolution shall use only that supplied root.

No arbitrary global fallback search is permitted.

Only the minimum Drawings read/write dependency closure is authorized.

Production P0-T006 shall not link TF/revision-control functionality.

---

## 5. First-party module ownership

```text
Production target:
bim_dwg

Alias:
bim::dwg

Public boundary:
src/interop/dwg/include/bim/dwg/probe.hpp

Public-facing implementation:
src/interop/dwg/src/dwg_probe.cpp

Private ODA adapter:
src/interop/dwg/src/detail/oda_adapter.hpp
src/interop/dwg/src/detail/oda_adapter.cpp
```

Public dependency is `bim::foundation`.

ODA Drawings is private.

No other first-party dependency is authorized.

---

## 6. Public contract

Implement the following vendor-neutral Phase-0 contract semantically.

```cpp
namespace bim::dwg {

struct DwgRoundTripEvidence {
    std::string source_version;
    std::string reopened_version;

    bool read_success = false;
    bool write_success = false;
    bool reopen_success = false;

    bool source_version_preserved = false;

    bool probe_mtext_found_before = false;
    bool probe_mtext_found_after = false;

    bool probe_mtext_text_preserved = false;
    bool probe_mtext_position_preserved = false;
    bool probe_mtext_orientation_preserved = false;
    bool probe_mtext_dimensions_preserved = false;
    bool probe_mtext_style_preserved = false;

    bool overall_passed = false;
};

[[nodiscard]]
bim::foundation::Status OpenAndValidateDwgFile(
    const std::filesystem::path& path);

[[nodiscard]]
bim::foundation::Status RunDwgRoundTripProbe(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path,
    DwgRoundTripEvidence& out_evidence);

} // namespace bim::dwg
```

The header may include only project-owned and standard-library types.

No ODA type/header is permitted publicly.

---

## 7. Adapter behavior

The private adapter owns:

```text
ODA services/runtime initialization
DWG read
source-version inspection
AC1018 -> vAC18 mapping
DWG write
reopen
controlled MText semantic extraction/comparison
ODA exception translation
ODA lifecycle cleanup
```

No ODA type may escape the private implementation.

---

## 8. Version rule

Controlled P0-T006 v1.0 path:

```text
AC1018 input
    ->
OdDb::vAC18 write
    ->
AC1018 reopen
```

Do not use `OdDb::kDHL_CURRENT` as implicit source-version preservation.

Unsupported/unmapped write versions return a first-party error.

---

## 9. Controlled fixture

```text
logical name:
OdWriteEx XRef.dwg

bytes:
31437

SHA256:
033D87D748533DEB4F37C10A755AC9861416FD1AA7494DD3CB86184E7222EB94

version:
AC1018
```

The fixture remains external.

Validation receives it through `BIM_P0_T006_DWG_FIXTURE`.

Authoritative validation verifies its SHA256 before and after tests.

---

## 10. Controlled semantic invariants

Locate the known MText semantically.

Its plain text includes `External Reference`.

Verify preservation of:

```text
MText text
MText position
MText normal/orientation
MText X direction
MText text height
MText width / box dimensions
MText attachment/alignment semantics
resolved text-style semantics
```

Fixture-local numeric tolerance:

```text
absolute tolerance = 1e-9
```

Binary equality, handle equality and native object-count equality are not
acceptance invariants.

---

## 11. Required failure behavior

At minimum:

```text
missing input
non-DWG/unreadable input
input path == output path
unsafe overwrite request
unsupported source write version
ODA initialization failure
ODA runtime/module resolution failure
write failure
reopen failure
controlled semantic mismatch
```

All ODA failures translate to `bim::foundation::Status`.

---

## 12. CMake integration

Root:

```text
option(BIM_ENABLE_DWG ... OFF)
```

Only when enabled:

```text
add_subdirectory(src/interop/dwg)
```

`src/interop/dwg/CMakeLists.txt` shall:

1. require `BIM_ODA_DRAWINGS_ROOT`;
2. resolve headers/libraries only beneath it;
3. fail clearly on missing required dependency closure;
4. create `bim_dwg`;
5. expose only first-party public includes;
6. link ODA privately;
7. keep first-party C++20;
8. follow project warning/sanitizer convention;
9. stage runtime DLLs only into build output when required;
10. never copy ODA runtime into the source tree.

Default build with DWG disabled remains unaffected.

---

## 13. Required tests

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

Tests use the production `bim::dwg` target.

---

## 14. Architecture enforcement

Extend the architecture checker without weakening R1-R15.

### R16 - ODA_DRAWINGS_ONLY_DWG_OWNER

ODA production usage is allowed only beneath `src/interop/dwg/**`.

Positive and negative fixtures are required.

### R17 - DWG_PUBLIC_NEUTRAL

Public DWG headers must contain no ODA dependency/type leakage.

Positive and negative fixtures are required.

Use comment-insensitive scanning consistent with existing checker practice.

---

## 15. Deterministic evidence contract

Logical evidence includes:

```text
dependency_family                 = ODA Drawings
dependency_version                = 27.7.0.0
fixture_expected_sha256           =
033D87D748533DEB4F37C10A755AC9861416FD1AA7494DD3CB86184E7222EB94

source_version                    = AC1018
reopened_version                  = AC1018

read_success
write_success
reopen_success
source_version_preserved

probe_mtext_found_before
probe_mtext_found_after
probe_mtext_text_preserved
probe_mtext_position_preserved
probe_mtext_orientation_preserved
probe_mtext_dimensions_preserved
probe_mtext_style_preserved

overall_passed
```

Evidence must not contain secrets, unstable timestamps, random values or
absolute proprietary SDK paths.

---

## 16. Validation script

Add:

```text
scripts/ci/dwg-spike.ps1
```

It shall verify baseline, fixture SHA, external ODA root, configure/build,
targeted tests, architecture tests, deterministic evidence, post-test fixture
hash and working-tree status.

It never stages, commits, integrates or pushes.

---

## 17. Source and license hygiene

Expected:

```text
ODA headers committed          = 0
ODA libraries committed        = 0
ODA DLLs committed             = 0
ODA archives committed         = 0
activation material committed  = 0
activation tokens committed    = 0
vcpkg baseline change          = NONE
```

`LICENSES.md` may record factual dependency identity and unresolved commercial
redistribution authorization.

Do not copy proprietary license text without permission.

---

## 18. Expected implementation footprint

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

Anything else is a STOP condition.

---

## 19. Forbidden implementation shortcuts

```text
in-house DWG parser
ODA types in public headers
ODA types in model/foundation/core contracts
persistent identity based on DWG handles
binary equality as sole fidelity criterion
object-count equality as sole fidelity criterion
implicit kDHL_CURRENT preservation
in-place fixture writes
global ODA fallback discovery
ODA SDK/runtime vendoring
activation-token commit
vcpkg baseline advancement
new unrelated dependency
R1-R15 weakening
model-contract expansion
RVT/BimRv/Civil/Mechanical/Map/Web expansion
commercial-deployment claims based on trial evidence
```

---

## 20. Implementation execution discipline

When separately authorized:

1. Claude works only in `D:\Projects\BIM-Platform-WT-P0-T006`.
2. Claude starts from the exact post-C2 governance baseline named by the
   separate Authorization.
3. Claude verifies branch/HEAD/tree/clean state first.
4. Claude implements minimum delta only.
5. Claude preserves P0-T001 through P0-T005 architecture.
6. Claude does not change toolchain/vcpkg baseline.
7. Claude does not stage or commit.
8. Contract-changing failures return to Architecture Authority.
9. Ordinary in-scope defects may be corrected.
10. Claude must not invoke Kimi.

---

## 21. Implementation acceptance gates

Before independent review:

```text
authorized baseline lock PASS
candidate path-set freeze PASS
default BIM_ENABLE_DWG=OFF regression PASS
BIM_ENABLE_DWG=ON configure PASS
fresh P0-T006 build PASS
required unit/integration tests PASS
R16 positive + negative PASS
R17 positive + negative PASS
fixture pre-hash PASS
AC1018 read PASS
explicit same-version write PASS
AC1018 reopen PASS
controlled semantic invariants PASS
fixture post-hash unchanged PASS
source/vendor hygiene PASS
license-inventory statement PASS
format/lint/static-analysis PASS
main unchanged + clean PASS
task candidate path-set controlled PASS
```

Kimi remains separately gated.

---

## 22. Stop conditions

STOP for Architecture Authority classification if:

```text
accepted ODA 27.7 SDK cannot be resolved
VS2022/v143 compatibility no longer reproduces
additional ODA product/module becomes necessary
TF/revision-control becomes necessary
ODA public type becomes necessary
model-contract expansion becomes necessary
new third-party dependency becomes necessary
vcpkg baseline change becomes necessary
AC1018 same-version preservation fails
controlled semantic invariant is lost
input must be modified in place
ODA source/SDK patch becomes necessary
architecture checker must be weakened
commercial redistribution assumption becomes necessary
authorized footprint is insufficient
```

Do not self-authorize an architectural workaround.

---

## 23. Current gate

```text
Architecture Gate            = FROZEN / APPROVED
Implementation Brief         = RELEASED
Implementation Authorization = NOT ISSUED

Claude                       = NOT AUTHORIZED
Kimi                         = NOT AUTHORIZED

Candidate staging            = NOT AUTHORIZED
Candidate commit             = NOT AUTHORIZED
Main integration             = NOT AUTHORIZED
Push                         = NOT AUTHORIZED

ACR                          = NONE
```

Next:

```text
Architecture Authority inspection
    ->
governance-only materialization commit
    ->
exact release HEAD/TREE capture
    ->
separate P0-T006 Implementation Authorization
```