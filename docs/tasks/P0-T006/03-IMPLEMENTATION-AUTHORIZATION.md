# P0-T006 - Implementation Authorization

**Authorization ID:** BIM-AUTH-P0-T006-CLAUDE v1.0

**Task:** P0-T006 - DWG / ODA Evaluation

**Architecture Gate:** BIM-AG-P0-T006 v1.0 - FROZEN / APPROVED

**Implementation Brief:** BIM-TASK-P0-T006-CLAUDE v1.0 - RELEASED

**Implementation Engineer:** Claude

**Architecture Authority:** Product Authority + ChatGPT

**Independent Reviewer:** Kimi - NOT AUTHORIZED

**Architecture Change Record:** NONE

**Authorization date:** 2026-09-22

---

## 1. Authorization semantics

Architecture Authority approves P0-T006 implementation under the frozen Gate
and Implementation Brief.

This document becomes effective only after it is materialized in a committed,
clean P0-T006 task state whose parent is the exact Implementation Brief release
baseline defined below.

C3A authoring by itself does not authorize Claude execution.

Claude execution begins only after Architecture Authority verifies the
post-materialization authorization commit and publishes its exact execution
start HEAD/TREE.

---

## 2. Authorized implementation-content baseline

The frozen implementation-content baseline is:

```text
branch:
task/P0-T006-dwg-oda-spike

C2B release HEAD:
6afb2462a3dddc1c4f0e7780f857acae5733d3fc

C2B release TREE:
682e1e48ea56a755cab9904b4d6bebf400e3a8dd

C2B parent:
fe2dfa813070df0875eaff93ad40cd2af2ec1dad
```

Frozen governance identities:

```text
Architecture Gate SHA256:
27E5B5EBBA2EE170E7964D500DC57866687BA69C2DC327EF847A79DA913EF64B

Implementation Brief SHA256:
2587E2C4383E820AC9838F49C1CF1A87135EDABE4DB7495A1E8A475032E5BDA6
```

No production implementation existed in this baseline.

---

## 3. Authorization materialization rule

The commit that first materializes this Authorization must:

1. have parent
   `6afb2462a3dddc1c4f0e7780f857acae5733d3fc`;
2. contain governance/documentation changes only;
3. contain no production source implementation;
4. leave canonical `main` unchanged;
5. leave the task worktree clean after commit;
6. contain this Authorization document.

After materialization, Architecture Authority records the resulting commit/tree
as the exact Claude execution-start state.

Claude shall begin from that post-authorization committed state, not from an
uncommitted worktree and not directly from the C2B parent.

---

## 4. Authorized objective

Claude is authorized to implement the minimum production-quality Phase-0 DWG
interoperability spike defined by the frozen Implementation Brief:

```text
vendor-neutral bim::dwg API
        ->
private ODA Drawings adapter
        ->
controlled AC1018 read
        ->
explicit OdDb::vAC18 write
        ->
AC1018 reopen
        ->
controlled semantic-fidelity verification
```

AG-011 remains binding:

```text
NO IN-HOUSE DWG PARSER
```

---

## 5. Authorized implementation footprint

Claude may modify only:

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

Claude must return to Architecture Authority before making such an edit.

---

## 6. External dependency boundary

Authorized dependency:

```text
ODA Drawings / DWG-DXF native C++
version: 27.7.0.0
platform: Windows x64
vendor delivery: vc16_amd64dll
```

The ODA SDK/runtime remains external to Git.

Claude may consume the operator-provided SDK through:

```text
BIM_ODA_DRAWINGS_ROOT
```

Claude shall not:

- vendor ODA headers, libraries or DLLs;
- copy activation material;
- log activation tokens or secrets;
- introduce automatic ODA download/activation;
- change the vcpkg baseline;
- add another DWG implementation dependency.

---

## 7. Controlled fixture boundary

The controlled external fixture remains outside Git.

Authoritative identity:

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

Claude/test code may receive the path through:

```text
BIM_P0_T006_DWG_FIXTURE
```

The input fixture shall never be modified in place.

---

## 8. Frozen architecture constraints

Claude must preserve all of the following:

1. ODA ownership only under `src/interop/dwg/**`.
2. Public `bim::dwg` types remain ODA-neutral.
3. `BIM_ENABLE_DWG` defaults OFF.
4. ODA resolution uses only explicit `BIM_ODA_DRAWINGS_ROOT`.
5. No arbitrary machine-wide ODA fallback search.
6. First-party production code remains C++20.
7. Project toolchain/vcpkg baseline remains frozen.
8. AC1018 source preservation uses explicit `OdDb::vAC18`.
9. `OdDb::kDHL_CURRENT` is not a preservation policy.
10. DWG handles are not persistent platform identity.
11. Binary equality is not the fidelity contract.
12. Object-count equality is not the fidelity contract.
13. R1-R15 architecture enforcement must not be weakened.
14. R16 and R17 must be added as specified by the Brief.
15. RVT/BimRv/Civil/Mechanical/Map/Web remain out of scope.

---

## 9. Controlled semantic acceptance

The implementation shall locate the controlled user-visible MText semantically,
not by persistent DWG handle.

Known plain-text content includes:

```text
External Reference
```

Before vs reopened output shall preserve:

```text
text
position
normal/orientation
X direction
text height
width / box dimensions
attachment/alignment semantics
resolved text-style semantics
```

Fixture-local numeric tolerance:

```text
absolute tolerance = 1e-9
```

No broader geometry-tolerance policy is authorized.

---

## 10. Required tests

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

All functional tests must exercise the production `bim::dwg` target.

No duplicate test-only DWG implementation is permitted.

---

## 11. Claude execution discipline

When Architecture Authority publishes the post-C3 execution-start HEAD/TREE,
Claude shall:

1. work only in `D:\Projects\BIM-Platform-WT-P0-T006`;
2. verify branch, HEAD, TREE and clean worktree before editing;
3. verify canonical `main` remains unchanged and clean;
4. implement minimum delta only;
5. remain inside the authorized footprint;
6. preserve the frozen Architecture Gate and Brief;
7. not modify governance documents unless explicitly re-authorized;
8. not stage files;
9. not commit files;
10. not merge;
11. not push;
12. not invoke Kimi;
13. return implementation/evidence to Architecture Authority.

---

## 12. Failure classification rule

An implementation/build/test failure does not automatically imply an
architecture or ODA defect.

Claude may correct an ordinary in-scope coding defect.

Claude must STOP and return to Architecture Authority if resolution appears to
require:

```text
architecture change
dependency-family expansion
additional ODA product family/module
TF/revision-control production dependency
public ODA type exposure
model-contract expansion
new third-party dependency
vcpkg baseline change
toolchain-baseline change
ODA SDK/source patch
architecture-checker weakening
in-place fixture modification
commercial/runtime redistribution assumption
authorized-footprint expansion
```

Architecture Authority classifies such failures before further changes.

---

## 13. Required implementation handback

Claude shall return:

```text
final branch
starting HEAD/TREE
final uncommitted path-set
implementation summary
exact files added/modified
ODA libraries/runtime modules actually required
configure command
build command
test commands
test results
architecture-rule results
fixture pre/post SHA256
source/reopened DWG version evidence
semantic-fidelity evidence
git diff --check result
git status result
main repository status
known limitations
STOP conditions encountered, if any
```

Claude must not create the candidate commit.

---

## 14. Independent review gate

Kimi remains NOT AUTHORIZED during implementation.

Independent review becomes eligible only after:

1. Claude handback;
2. Architecture Authority inspection;
3. authoritative Windows validation;
4. candidate path-set freeze;
5. candidate defect/harness classification;
6. explicit Architecture Authority authorization for independent review.

---

## 15. Commit and integration authority

This Authorization does not authorize Claude to:

```text
git add
git commit
git merge
git rebase
git push
modify main
authorize Kimi
integrate candidate
```

Candidate commit, independent review, integration and push remain separate
Architecture Authority gates.

---

## 16. Authorization decision

```text
Architecture Gate            = FROZEN / APPROVED
Implementation Brief         = RELEASED / MATERIALIZED

Implementation Authorization = APPROVED
Effectiveness                 = AFTER C3 GOVERNANCE MATERIALIZATION
                               AND AA POST-COMMIT START LOCK

Claude                       = NOT YET EXECUTING
Kimi                         = NOT AUTHORIZED

Candidate commit             = NOT AUTHORIZED
Main integration             = NOT AUTHORIZED
Push                         = NOT AUTHORIZED

ACR                          = NONE
```

The next lifecycle step is the governance-only C3 materialization commit.

After that commit is validated, Architecture Authority may publish the exact
Claude execution-start HEAD/TREE and activate implementation execution.