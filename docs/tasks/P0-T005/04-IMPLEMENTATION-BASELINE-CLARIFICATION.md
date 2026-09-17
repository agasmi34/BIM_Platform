# P0-T005 — Implementation Baseline Clarification

**Record ID:** BIM-AA-P0-T005-BASELINE-CLARIFICATION v1.0
**Date:** 2026-09-17
**Status:** AUTHORITATIVE CLARIFICATION
**Task:** P0-T005 — IFC Spike
**Architecture Authority:** Product Authority + ChatGPT

---

## 1. Conflict being corrected

Two governing documents used the phrase "authorized task baseline" for
different lifecycle points:

- `docs/tasks/P0-T005/03-IMPLEMENTATION-AUTHORIZATION.md` named
  `d6d84632cf467ce56f535c525db6e0d884c07207` /
  `70721dbd111efcf692fb81fdb0d5b9b862520ce2`.
- `P0-T005-Claude-Implementation-Execution-Packet-v1.0.md` named
  `3eb6fed9e1334aa5d596afdc0680e3561cd29c11` /
  `6f52d12f6c580deb7ccaf7e41bb11386a8861b82`.

The first pair is the **pre-authorization parent baseline** from which the
authorization governance commit was created.

The second pair is the **authorization materialization commit/tree**, i.e. the
first repository state that actually contains the issued authorization.

Therefore the Authorization document's wording "Claude must begin from" the
pre-authorization parent is stale/incorrect as an execution-baseline rule.

---

## 2. Authoritative interpretation

For provenance:

```text
Pre-authorization parent commit:
d6d84632cf467ce56f535c525db6e0d884c07207

Pre-authorization parent tree:
70721dbd111efcf692fb81fdb0d5b9b862520ce2
```

Authorization was materialized as:

```text
Authorization materialization commit:
3eb6fed9e1334aa5d596afdc0680e3561cd29c11

Authorization materialization tree:
6f52d12f6c580deb7ccaf7e41bb11386a8861b82
```

The exact implementation execution baseline must be a repository state that
contains the authorization and this clarification.

Accordingly, after this clarification is materialized, Architecture Authority
will issue **Claude Implementation Execution Packet v1.1** naming the exact
clarification-materialization commit/tree.

That v1.1 packet will be the sole exact execution-baseline authority.

---

## 3. Supersession rule

This record supersedes only the conflicting baseline-start wording in:

```text
docs/tasks/P0-T005/03-IMPLEMENTATION-AUTHORIZATION.md
section 2
```

It does not revoke or change the authorization decision itself.

`P0-T005-Claude-Implementation-Execution-Packet-v1.0.md` is withdrawn for
execution and will be replaced by v1.1 after this clarification commit is
known.

All other frozen architecture, ACR, dependency, scope, and implementation
constraints remain unchanged.

---

## 4. Execution hold

Until v1.1 is issued:

```text
Implementation Authorization = ISSUED
Claude implementation        = PAUSED FOR BASELINE CLARIFICATION
Candidate staging/commit     = NOT AUTHORIZED
Kimi                         = NOT AUTHORIZED
Main integration             = NOT AUTHORIZED
Push                         = NOT AUTHORIZED
```

No production implementation edit is authorized during this hold.

---

## 5. Main baseline remains unchanged

```text
main HEAD:
2c2b89f73651f7d5981d42546cbc0e321d6bb055

main tree:
c2f6f8e463248f98aafe8109f71d85cf9003d5a9
```

---

## 6. Next gate

Materialize this clarification as a docs-only governance commit, then report
the resulting exact commit/tree to Architecture Authority.

Architecture Authority will then issue:

```text
P0-T005 Claude Implementation Execution Packet v1.1
```

with the exact post-clarification implementation start baseline.
