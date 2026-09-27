# BIM Platform - P0-T007 RVT / BimRv Evaluation Closure Gate

**Gate:** BIM-AG-P0-T007 v1.0
**Task:** P0-T007 - RVT / BimRv Evaluation
**Architecture Authority:** Product Authority + ChatGPT
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## 1. Authority and lifecycle state

P0-T007 is a Phase-0 evaluation task. Its authorized outcome is a read-only
capability matrix and a controlled technical conclusion.

This Gate does not authorize implementation.

At materialization time:

- evaluation evidence = CLOSED / PASS;
- product integration = NOT AUTHORIZED;
- RVT writer = PROHIBITED;
- Claude = NOT AUTHORIZED;
- Kimi = NOT AUTHORIZED;
- candidate commit = NOT AUTHORIZED;
- main integration = NOT AUTHORIZED;
- push = NOT AUTHORIZED;
- ACR = NONE.

## 2. Objective

Determine whether the controlled ODA BimRv evaluation package provides a
credible read-only technical path for RVT data relevant to future BIM Platform
architecture, without adding BimRv to the product, linking proprietary ODA
artifacts into the repository, or implementing an RVT writer.

## 3. Frozen architectural direction

P0-T007 freezes the following Phase-0 direction:

1. RVT is evaluation-only.
2. Read capability may be evidenced.
3. Writer behavior is outside scope and prohibited.
4. Product integration is not authorized.
5. ODA BimRv remains external to Git.
6. No public BIM Platform RVT API is created.
7. No CMake/product dependency is added.
8. Evaluation results inform later architecture; they do not themselves
   authorize that later architecture.

This preserves AG-012.

## 4. Repository and toolchain baseline

Evaluation documentation is based on:

- parent HEAD:
  `1c6dc07d93fb93b70cb6a422025267557b24f1e4`;
- parent TREE:
  `342fb966d1ae78e062650c9768d4ca3368649f77`;
- task branch:
  `task/P0-T007-rvt-bimrv-evaluation`;
- isolated worktree:
  `D:\Projects\BIM-Platform-WT-P0-T007`.

The locked project toolchain remains unchanged by this task.

## 5. Frozen BimRv dependency identity

The controlled external evaluation used:

- ODA BimRv 27.07 / 27.7 family;
- `vc16_amd64dll` package surface;
- isolated external toolkit root:
  `D:\Dependencies\ODA\evaluation\P0-T007-B5-A4-2AAAFC1F\run1\ODAToolkit`.

Direct BimRv SDK header closure and required runtime dependency closure were
established before runtime testing.

The evaluation package is not a repository dependency.

## 6. Licensing and proprietary-material boundary

ODA BimRv is proprietary/commercial vendor material used under the controlled
evaluation/trial context available to the Windows Execution Operator.

P0-T007 establishes no commercial deployment, redistribution, production
license, sublicensing, or packaging right.

The following remain external:

- vendor headers;
- libraries;
- runtime modules;
- archives;
- activation material;
- activation tokens or secrets.

No such material may be committed by P0-T007.

Because BimRv is not linked into the repository by this evaluation task, it is
not added to the repository's direct build-dependency table.

## 7. Controlled RVT fixture

The runtime fixture is:

- `rac_basic_sample_project.rvt`;
- size `17125376` bytes;
- SHA256
  `A1D3D0775AE0937E3F4C8FB97015B6E0884ACEFAE52435D9CACD27D008F5FEFB`.

Static metadata established:

- Autodesk Revit 2017;
- build `20160130_1515(x64)`.

Runtime tests used isolated working copies. The canonical fixture and working
copies were hash-checked before and after controlled reads.

## 8. Read capability contract

The accepted Phase-0 read-evaluation contract is limited to evidence that BimRv
can:

- load the controlled RVT database;
- load nested databases/elements as required by the vendor runtime;
- resolve a selected model element;
- vectorize supported model content;
- expose geometry through vendor read/vectorization surfaces.

No guarantee is made for all RVT files or all Revit versions.

## 9. Controlled runtime evidence

The first controlled `OdaBimApp` open:

- successfully opened the Revit 2017 fixture;
- returned normally;
- left the working RVT hash unchanged;
- left the canonical RVT hash unchanged;
- created no additional file in the isolated runtime directory.

`OdBmVectorizeEx`:

- executed against an isolated fixture copy;
- returned exit code 0;
- left the working and canonical RVT files unchanged;
- created no additional runtime file.

`OdBmGetGeomEx`:

- resolved the selected `SWall` after the documented handle-representation
  correction;
- returned exit code 0;
- emitted 1057 output lines;
- emitted shell/vertex/face/edge/texture/polyline geometry;
- reported no "no entity with such handle" diagnostic in the successful run;
- left both RVT hashes unchanged;
- created no additional runtime file.

## 10. Identifier-semantics finding

`OdaBimApp` displayed the selected element handle as decimal `198694`.

Static vendor source evidence established that the command-line
`OdDbHandle` string representation is hexadecimal.

The exact conversion was:

- displayed decimal value: `198694`;
- hexadecimal handle string: `30826`.

The initial unsuccessful lookup was classified as an input-representation
mismatch, not a BimRv, asset, repository, or architecture defect.

## 11. Capability classification rule

P0-T007 uses the following evidence classes:

- `RUNTIME PROVEN` - directly exercised against the controlled fixture;
- `STATICALLY ESTABLISHED` - implementation surface established from vendor
  source/headers but not runtime-validated for this fixture;
- `OFFICIALLY DOCUMENTED` - vendor documentation evidence only;
- `NOT ESTABLISHED` - evidence insufficient;
- `OUT OF SCOPE` - explicitly excluded from P0-T007.

The detailed matrix is frozen in `02-CAPABILITY-MATRIX.md`.

## 12. Version and format boundary

The evaluation establishes actual runtime evidence for the controlled Revit
2017 RVT fixture.

Prior official-documentation review established a broader BimRv read surface
for RVT/RFA/RTE and Revit-family versions, but P0-T007 does not convert that
documentation into package-specific runtime proof.

Specifically:

- RFA runtime read = NOT TESTED;
- RTE runtime read = NOT TESTED;
- BimRv 27.7 package-specific maximum Revit version = NOT ESTABLISHED.

No unsupported upper-bound claim may be inferred.

## 13. Repository boundary

`src/interop/rvt/` remains a boundary placeholder.

P0-T007 shall not add:

- `src/interop/rvt/CMakeLists.txt`;
- BimRv source adapters;
- public RVT APIs;
- runtime wrappers;
- tests that require proprietary BimRv;
- vendor binaries;
- vendor libraries;
- vendor headers;
- activation material.

The root build remains unchanged.

## 14. Explicit non-goals

P0-T007 does not authorize:

- RVT writing;
- RVT save/save-as behavior;
- RVT conversion;
- RFA writing;
- RTE writing;
- product adapter implementation;
- redistribution packaging;
- commercial-deployment claims;
- repository dependency integration;
- future Phase-1 architectural commitments.

## 15. Controlled residual risks

Residual limitations are recorded in the project risk register:

1. proprietary/trial licensing and redistribution rights are not established;
2. package-specific maximum supported Revit version is not established;
3. RFA and RTE were not runtime-tested;
4. detailed runtime geometry evidence is bounded to one selected element in one
   controlled Revit 2017 sample.

These limitations do not convert into defects and do not expand task scope.

## 16. Required closure evidence

Evaluation closure requires evidence of:

- exact repository baseline;
- external SDK/runtime closure;
- controlled public fixture identity;
- Revit fixture version/build identity;
- successful read-only RVT open;
- unchanged input hashes;
- successful vectorization path;
- successful selected-element geometry extraction;
- no repository implementation;
- no proprietary ODA material committed;
- capability matrix with explicit evidence boundaries;
- ACR = NONE unless an architecture contract changes.

These conditions are satisfied for the evaluation closure package subject to
Architecture Authority review of this documentation materialization.

## 17. Independent review gate

Kimi remains NOT AUTHORIZED during this materialization checkpoint.

Independent review may only be authorized by Architecture Authority after:

1. the uncommitted documentation path set is verified;
2. documentation content is accepted by Architecture Authority;
3. any candidate-commit gate required by the task lifecycle is separately
   authorized and verified.

Independent review does not imply product-integration authority.

## 18. Final gate decision

~~~text
P0-T007 technical evaluation      = CLOSED / PASS
Read-only capability matrix       = ESTABLISHED
RVT writer                        = PROHIBITED
Product integration               = NOT AUTHORIZED
Repository RVT implementation     = NONE
Claude                            = NOT AUTHORIZED
Kimi                              = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
Architecture Change Record        = NONE

~~~

This Gate freezes evaluation conclusions only. It does not release an
Implementation Brief and does not authorize implementation.
