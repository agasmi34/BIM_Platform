# ADR-0003 - Phase-1 C++ Test Framework Continuity

**Status:** ACCEPTED
**Date:** 2026-10-01
**Authority:** Product Authority + Architecture Authority
**Scope:** Phase-1 VS1 program

## Context

ADR-0001 approved Catch2 v3 + CTest for Phase 0.

ADR-0001 intentionally left the Phase-1+ choice PROVISIONAL.

P0-T010 is the planned point to resolve that deferred decision.

## Decision

For the VS1 Phase-1 program:

    Catch2 v3 + CTest = APPROVED

The existing test framework remains the repository baseline.

No migration to GoogleTest or another framework is authorized merely
because Phase 1 begins.

## Rationale

- the stack is already integrated with CMake/CTest;
- accepted Phase-0 tests already use it;
- no VS1 capability gap requires replacement;
- migration would add unrelated churn before the first production BIM slice;
- continuity keeps Phase-1 work focused on BIM architecture.

## Consequences

- existing tests remain valid;
- new VS1 unit/integration tests use the same baseline;
- P0-T010 changes no test dependency;
- this ADR itself authorizes no source/CMake/test modification;
- future migration requires its own ADR.

## Relationship to ADR-0001

ADR-0001 remains authoritative for Phase 0.

ADR-0003 resolves its provisional Phase-1 question for VS1 without
rewriting Phase-0 history.

## ACR

    ACR = NONE
