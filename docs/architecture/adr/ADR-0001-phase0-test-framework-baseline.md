# ADR-0001 — Phase 0 C++ Test Framework Baseline

> **Authoritative location.** Per Master Engineering Constitution v0.2 and
> "P0-T001 Architecture Record Placement Correction" (Architecture
> Authority, this session), `docs/architecture/adr/` is the authoritative
> location for Architecture Decision Records; this file is the
> authoritative copy of ADR-0001. `docs/adr/` remains in this repository as
> the scaffold directory required by the P0-T001 Implementation Brief, but
> no longer holds a copy of this record — see `docs/adr/README.md`. This
> relocation is a placement correction only; no technical decision content
> below was altered.

**Status:** ACCEPTED — scoped to Phase 0 only.
**Date:** 2026-08-30 (date of Architecture Authority's disposition issuing this
ADR; see "Authority and evidentiary basis" below).
**Deciding authority:** Architecture Authority.
**Affected task:** P0-T001 (and, by scope, all other Phase 0 tasks).

## Context

Architecture Authority identified a conflict between two governing documents:

- `BIM Platform - Master Engineering Constitution v0.2`, decision `D-028`,
  which states GoogleTest is the LOCKED C++ test framework.
- The approved `P0-T001 Architecture Gate Package v1.0` (section on required
  toolchain: "Catch2 | LOCK: major version 3") and the approved
  `P0-T001 Implementation Brief Claude v1.0` (unit framework: Catch2 v3),
  both of which this task was executed against from the start.

**Important scoping note on this ADR's own evidentiary basis:** the Master
Engineering Constitution v0.2 itself has never been supplied to this
repository (see `docs/constitution/README.md` and `README.md`, both of
which record this as an open gap predating this ADR). Claude has not read
`D-028`'s literal text and is not independently confirming its content or
its conflict with Catch2 — this section records the conflict exactly as
Architecture Authority identified and reported it via chat instruction, not
as something Claude verified against the Constitution document itself.
Resolving exactly this kind of conflict between a parent governing document
and an approved, already-executing task package is squarely within
Architecture Authority's role under the authority model (Architecture Gate
section 1 / Implementation Brief section 2), which is why this ADR is
issued on Architecture Authority's decision alone, without requiring Claude
to have independently reconciled the two documents first.

## Decision

- **Catch2 v3 + CTest is the approved testing baseline for all Phase 0
  tasks**, including P0-T001.
- This decision **explicitly supersedes `D-028` for Phase 0 only**. It does
  not amend, rewrite, or otherwise alter the Master Engineering Constitution
  v0.2 itself.
- The **Phase 1+ long-term C++ test framework choice is PROVISIONAL** and
  will be re-evaluated before Phase 1 implementation begins. This ADR does
  not resolve that question and should not be read as pre-deciding it.
- **The existing P0-T001 Catch2 implementation is not to be replaced with
  GoogleTest.** No code change follows from this ADR: P0-T001's `tests/`
  tree, `vcpkg.json` dependency, and `CMakeLists.txt` wiring already use
  Catch2 v3 exclusively (Architecture Gate lock; Implementation Brief;
  `DECISION-LEDGER.md` row D-003), so this ADR ratifies the status quo
  implementation rather than requiring any change to it.

## Consequences

- No source, build, or test files in the P0-T001 scaffold require any
  change as a result of this ADR.
- `docs/project-control/DECISION-LEDGER.md` is updated (see below) to
  record `D-028`'s Phase-0 supersession as a locked decision, so a future
  reader of the ledger sees the resolution without needing to separately
  read this ADR first.
- The Phase 1+ test framework question is now explicitly tracked as open
  (PROVISIONAL) rather than silently assumed to be Catch2 by default;
  whoever plans Phase 1 work needs to either re-affirm Catch2 or resolve
  the question with its own ADR before Phase 1 implementation starts.
- The Master Engineering Constitution v0.2 document itself is unchanged.
  This ADR is additive and Phase-0-scoped; it is not a substitute for
  eventually reconciling or updating the Constitution's own `D-028` text,
  which remains outside this repository and outside this task's authority
  to edit.

## Authority and evidentiary basis

Issued via Architecture Authority's chat instruction "ARCHITECTURE AUTHORITY
DISPOSITION," item 2, this session. Claude authored this ADR file and the
corresponding `DECISION-LEDGER.md`/`CHANGE-LOG.md` entries directly (not
reported by another party), but the underlying resolution — that `D-028`
and the Gate/Brief conflict, and that Catch2 wins for Phase 0 — is
Architecture Authority's decision, recorded here at its instruction rather
than independently derived or verified by Claude against the Constitution's
own text.

## Record history

- Authored and first placed at `docs/adr/ADR-0001-phase0-test-framework-baseline.md`
  (Architecture Authority disposition, this session).
- Relocated to this path (`docs/architecture/adr/`) per "P0-T001
  Architecture Record Placement Correction" (Architecture Authority, this
  session), which identified `docs/architecture/adr/` as the location
  defined by Master Engineering Constitution v0.2. No content in this
  record was altered by the relocation.
