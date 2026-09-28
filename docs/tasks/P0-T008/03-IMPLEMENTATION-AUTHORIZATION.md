# BIM Platform - P0-T008 Implementation Authorization

**Authorization:** `P0-T008-IA v1.0`
**Task:** `P0-T008 - Topological Reference Spike`
**Decision status:** ISSUED
**Execution status:** PENDING COMMITTED AUTHORIZATION BASELINE
**Architecture Gate:** `BIM-AG-P0-T008 v1.0` - FROZEN / APPROVED
**Implementation Brief:** `P0-T008-IB v1.0` - FROZEN / APPROVED
**Approved brief baseline HEAD:** `31a5b9c6ba1c679ab34858089af14607d78ecdd9`
**Approved brief baseline TREE:** `47ed702b2ceace734d17351dfe24cdaaf54b02a0`
**Implementation Brief blob:** `30246bc9859f2f66a77815064e33b8b2af4c8e12`
**Architecture Gate blob:** `50f98bdbe59239aff52d82131c44d1d2e71a48e1`
**Task Record blob:** `75f036bfcad39deff770c1c4cce6285dd319f248`
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi
**ACR:** NONE

## 1. Authority

Architecture Authority issues `P0-T008-IA v1.0` against the exact approved brief baseline identified above.

This decision does NOT become executable merely because this file exists in the working
tree. Execution becomes active only after:

1. this authorization and its lifecycle traceability are committed in a governance-only
   commit;
2. the exact authorization commit HEAD/TREE and this document blob are captured;
3. Architecture Authority explicitly declares the authorization ACTIVE and hands the
   implementation package to Claude.

Until all three conditions are satisfied, Claude remains NOT AUTHORIZED to modify source,
CMake, tests, or dependency manifests.

## 2. Authorized implementation scope after activation

Once Architecture Authority activates this authorization, Claude MAY modify only the
minimum source, CMake, and test files required to implement `P0-T008-IB v1.0`.

The authorized logical implementation footprint is limited to:

```text
bim_model
bim_geometry_api
bim_geometry_occt
P0-T008 focused tests
minimum CMake wiring required for those tests/targets
```

All implementation MUST conform to the approved Architecture Gate and Implementation Brief.

## 3. Exact execution baseline rule

Claude MUST begin from the exact final authorization governance baseline captured after
the final authorization revision is committed and explicitly named by Architecture
Authority in the execution-activation handover.

The execution baseline is intentionally NOT self-identified by its own HEAD/TREE inside
this document because committing this document creates that baseline commit. The exact
execution baseline is therefore established by post-commit repository evidence.

Before editing, Claude MUST verify all of the following values supplied by Architecture
Authority in the activation handover:

```text
branch
authorization baseline HEAD
authorization baseline TREE
authorization blob
implementation brief blob
architecture gate blob
clean worktree state
staged count = 0
```

The authorization baseline MUST contain this authorization revision and the frozen
Implementation Brief and Architecture Gate objects.

If any supplied branch, HEAD, TREE, blob identity, clean-state, or staged-state evidence
does not match the repository, Claude MUST STOP without editing.
## 4. Frozen architecture constraints

Claude MUST preserve all approved constraints, including:

- persistent references are project-owned semantic references;
- persistent reference scope is Face only;
- the six required extrusion roles are `StartCap`, `EndCap`, `UMinSide`, `UMaxSide`,
  `VMinSide`, and `VMaxSide`;
- public `bim_model` and `bim_geometry_api` surfaces remain OCCT-free;
- `bim_model` MUST NOT directly include or link OCCT;
- raw OCCT topology identity, pointer identity, kernel handles, traversal order,
  enumeration order, and `input_face_ordinal` MUST NOT become persistent identity;
- resolution MUST explicitly distinguish `Resolved`, `Missing`, `Ambiguous`,
  `InvalidReference`, and `InvalidOwner`;
- ambiguity MUST fail closed and MUST NOT silently retarget;
- Edge/Vertex persistent naming, dependency DAG behavior, persistence/schema changes,
  production BIM features, and interoperability integration remain out of scope.

## 5. Toolchain and dependency lock

Claude MUST use the locked Windows toolchain and dependency baseline already approved for
the task.

No compiler, Windows SDK, CMake, Ninja, vcpkg baseline, dependency version, package source,
or proprietary SDK policy change is authorized.

Ninja remains the only authorized generator for this implementation.

## 6. Required proof and validation

Claude MUST implement and execute the complete proof corpus defined in
`P0-T008-IB v1.0`, including:

```text
A. Same-spec regeneration
B. Dimension change
C. Translation
D. Repeat regeneration
E. Boolean cut / unaffected Face continuity
F. Split-Face -> Ambiguous
G. Deleted Face -> Missing
H. Invalid owner/reference
```

Claude MUST also provide:

- enumeration-order independence proof;
- architecture-boundary enforcement;
- build result using the locked toolchain;
- existing relevant regression tests;
- `git diff --check`;
- exact changed-path evidence.

## 7. Failure and STOP semantics

Claude MUST STOP and return evidence to Architecture Authority if:

- the exact baseline does not match;
- a required change conflicts with the approved Gate or Brief;
- a direct `bim_model -> OCCT` dependency appears necessary;
- a public OCCT type appears necessary;
- continuity appears to require kernel identity or enumeration order;
- a global identity, tolerance, persistence, dependency-graph, or interoperability
  architecture change appears necessary;
- dependency/toolchain changes appear necessary;
- work expands outside the authorized logical footprint;
- a build/test/harness failure requires more than minimum-delta correction;
- any ACR-worthy ambiguity appears.

A STOP does not itself authorize an ACR or correction. Architecture Authority classifies
the failure first.

## 8. Git restrictions during implementation

Even after execution activation, Claude MUST NOT:

```text
git add
git commit
git merge
git rebase
git push
integrate into main
invoke Kimi
```

Claude produces an uncommitted implementation candidate plus evidence.

Staging, candidate commit, Kimi review, integration, and push each remain separate later
gates.

## 9. Evidence package required from Claude

Claude MUST return:

```text
baseline branch / HEAD / TREE / clean state
exact files changed
implementation summary
architecture-conformance notes
commands run
build result
test results
proof A-H results
enumeration-order independence result
architecture enforcement result
dependency/toolchain status
git diff --check result
known limitations
ACR status
final uncommitted worktree status
```

Claude self-review does not substitute for Architecture Authority validation.

## 10. Kimi status

Kimi is NOT AUTHORIZED during implementation.

Architecture Authority will authorize Kimi only after the implementation candidate and
evidence have passed the required implementer/AA validation gates.

## 11. Authorization disposition

```text
Authorization decision          = ISSUED
Execution activation            = PENDING COMMITTED AUTHORIZATION BASELINE
Claude                          = NOT YET AUTHORIZED TO EXECUTE
Kimi                            = NOT AUTHORIZED
Source/CMake/test modification  = NOT YET ACTIVE
Staging                         = NOT AUTHORIZED
Candidate commit                = NOT AUTHORIZED
Main integration                = NOT AUTHORIZED
Push                            = NOT AUTHORIZED
ACR                             = NONE
```

The next lifecycle steps are governance-only authorization commit, exact authorization
baseline capture, then explicit Architecture Authority activation and Claude handover.
