# DECISION-LEDGER

Mirrors and extends the locked-decisions register in
`docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 24, plus the
release clarifications (IC-001..IC-004) from
`docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md` section 4, plus
decisions made during this task's execution. This file does not itself lock
or unlock anything — it records what is locked, by which document, and any
addenda issued since.

## Architecture Gate locked decisions (AG-001..AG-018)

Reproduced by reference, not by value: see
`docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 24 for the
authoritative table (AG-001 repository name, AG-002 Windows x64 first
target, AG-003 C++20 baseline, AG-004 CMake+vcpkg manifest, AG-005 OCCT
8.0.1 kernel baseline, AG-006 no raw OCCT in model public API, AG-007
SQLite adapter without native schema, AG-008 Qt deliberately not linked in
P0-T001, AG-009 Qt lock in P0-T003, AG-010 no FreeCAD fork, AG-011 no
in-house DWG parser, AG-012 RVT read-evaluation only, AG-013 one
task = one worktree, AG-014 Claude implements / Kimi reviews, AG-015 no
architecture change without ACR, AG-016 Product Owner + ChatGPT are
Architecture Authority, AG-017 main receives only accepted controlled
integration, AG-018 Phase 1 waits for P0-T008/009/010). None of these were
changed by Phase C implementation work.

## Implementation Brief release clarifications (IC-001..IC-004)

- **IC-001** — Empty repository bootstrap exception (one empty commit only,
  `chore: initialize repository`, if and only if STATE B). Exercised via
  Bootstrap Runbook A v1.3 (reported).
- **IC-002** — vcpkg baseline syntax: use current vcpkg-supported syntax
  rather than an obsolete field. Applied: `builtin-baseline` placed in
  `vcpkg-configuration.json` (`default-registry.baseline`), not duplicated
  in `vcpkg.json`.
- **IC-003** — CI provider neutrality: no hosted CI provider existed, so
  `scripts/ci/` contains only provider-neutral scripts; hosted-runner
  integration recorded as operationally deferred (see `scripts/ci/README.md`).
- **IC-004** — No fake public APIs for empty modules: applied via private
  compilation anchors for `model`, `dependency_graph`, `transactions`,
  `commands`, `query`.

## Decisions made during P0-T001 execution

| ID | Decision | Made by | Basis |
|---|---|---|---|
| D-001 | Execution model: Claude authors files/scripts; a human Windows Execution Operator executes them; raw output is authoritative | Architecture Authority (ACR-P0T001-001 resolution) | Necessitated by Claude having no command-execution tool for the Windows target in this session (a fact, not a preference). |
| D-002 | vcpkg registry baseline frozen at commit `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843` (resolves `opencascade` to exactly `8.0.1`, port-version 0) | Claude, per IC-002/AG-005 | Resolved via live research (GitHub commits API + commit diff), not fabricated. See `vcpkg-configuration.json` and `LICENSES.md` for the full citation. |
| D-003 | Test registration uses plain `add_test()` per Catch2 executable rather than `catch_discover_tests()`/`include(Catch)` | Claude | Avoids an untested dependency on locating Catch2's bundled CMake module inside the vcpkg install tree; still satisfies "registered in CTest" (Architecture Gate section 12.1). |
| D-004 | Architecture-boundary checker implemented as a dependency-free `cmake -P` script (`tools/architecture_checker.cmake`) rather than Python | Claude, per Implementation Brief Phase J | Explicit brief instruction: "Do not add Python only for this checker in P0-T001." |
| D-005 | Enabled clang-tidy findings from the curated `.clang-tidy` `Checks` list are CI-gating: `WarningsAsErrors` changed from `''` (none) to `'*'` (all enabled checks) | Architecture Authority, per "P0-T001 Architecture Review — Verification Runbook B v1.3" finding BLOCKER 2 | Explicit disposition: the prior `WarningsAsErrors: ''` allowed `scripts/ci/static-analysis.ps1` to report `STATIC ANALYSIS PASSED` (clang-tidy exit 0) even with live findings, which Architecture Authority judged misleading. The curated `Checks` list itself is unchanged — only whether a finding is allowed to coexist with a zero exit code changed. `static-analysis.ps1` remains report-only (`-fix` is not and has never been used). No ADR required per the instruction. |

## Addenda issued since gate approval

| ID | Title | Issued by | Channel | Requires new ADR? | Status |
|---|---|---|---|---|---|
| A1 | Project record & traceability system (`docs/project-control/`, `docs/tasks/P0-T001/`) | Architecture Authority | Chat instruction (not a separately versioned, uploaded gate-amendment document) | Architecture Authority stated this is "a governance/traceability addendum, not a change to the approved technical architecture" and does not require a new ADR. Claude has not independently contested this characterization but notes it for the record: A1 does add a substructure to `docs/` beyond the canonical tree in Architecture Gate section 6 / Implementation Brief section 5, so a future ADR capturing it formally (rather than only a chat instruction) would strengthen traceability. | Applied to the isolated worktree only; not committed. |

No `AG-*` locked decision was changed by A1 or by any Phase C implementation
work. If a future instruction requires changing an `AG-*` decision, Claude
raises an ACR per Implementation Brief section 2.3 rather than applying it
silently.

## ADRs issued

| ID | Title | Decision | Supersedes | Scope | Status |
|---|---|---|---|---|---|
| ADR-0001 | Phase 0 C++ Test Framework Baseline | Catch2 v3 + CTest is the approved testing baseline for all Phase 0 tasks. | `D-028` (Master Engineering Constitution v0.2 — GoogleTest LOCKED) | **Phase 0 only.** Phase 1+ framework choice is explicitly PROVISIONAL, to be re-evaluated before Phase 1 implementation. | ACCEPTED. See `docs/architecture/adr/ADR-0001-phase0-test-framework-baseline.md` (authoritative location per Master Engineering Constitution v0.2 — relocated from `docs/adr/` by "P0-T001 Architecture Record Placement Correction"; content unchanged by the move) for full context, including the evidentiary-basis note (Claude has not read the Master Engineering Constitution v0.2's own text — see below). No code change required: P0-T001 already implements Catch2 v3 exclusively (row D-003 above). |

**Non-destructive supersession note:** `D-028` is superseded for Phase 0
only, by Architecture Authority's own decision (ADR-0001) — this ledger and
the ADR record that supersession; neither this repository nor this task
edits, holds, or rewrites the Master Engineering Constitution v0.2 itself,
which has not been supplied to this repository (`docs/constitution/README.md`,
`README.md`). Claude's evidentiary basis for the conflict this ADR resolves
is Architecture Authority's chat instruction, not independent verification
against the Constitution's literal `D-028` text.

P0-T002 architecture decisions
ADR-0002 — Phase 0 Task Sequence Reconciliation

Status: ACCEPTED
Date: 2026-09-02
Authority: Architecture Authority / Product Authority approval

The operative Phase 0 sequence now defines P0-T002 as the OCCT Geometry Spike.

The former standalone Dependency & License Baseline task is treated as
materially absorbed by P0-T001.

This decision changes task sequencing only. It does not change dependency
versions, the vcpkg baseline, P0-T001 evidence or the Master Engineering
Constitution itself.

Authoritative record:

docs/architecture/adr/ADR-0002-phase0-task-sequence-reconciliation.md

BIM-AG-P0-T002 v1.0

Product Authority approved the P0-T002 OCCT Geometry Spike Architecture Gate
on 2026-09-02.

Implementation remains explicitly blocked until Architecture Authority releases
the P0-T002 Claude Implementation Brief.

## 2026-09-03 - P0-T002 Implementation Brief Amendment 01

Architecture Authority accepted the read-only Phase A + B Contract Design
Check with no ACR required.

Decisions:

- AA-C01 explicitly authorizes `tools/architecture_checker.cmake` and
  `scripts/ci/architecture.ps1`.
- AA-C02 locks rectangular profile axes to orthogonality within caller
  angular tolerance.
- AA-C03 separates Cut volumetric-intersection qualification from Fuse
  proximity qualification.
- AA-C04 prohibits positive common-volume qualification as a Fuse
  prerequisite because J04 is a valid face-contact union case.
- AA-C05 accepts `SolidResult`, `MetricsResult` and the opaque
  `SolidHandle` design.
- AA-C06 adds the callable neutral history-diagnostics contract.
- AA-C07 requires an explicit JSON path for the evidence CTest.
- AA-C08 keeps the geometry API contract unit test independent of the OCCT
  adapter.
- AA-C09 defines rule-selectable architecture enforcement.
- AA-C10 requires the architecture CI job to execute all four architecture
  tests.

Amendment document:

`docs/tasks/P0-T002/03-IMPLEMENTATION-BRIEF-AMENDMENT-01.md`

Amendment SHA256:

`544EFB2664FAE9B37F4F94F3EB0536F38DC2B4FBA141F55A3DF8F3C07CE53CDE`

Phase C-M remains NOT YET AUTHORIZED.
