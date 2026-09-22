# P0-T006 - Task Record

**Task:** P0-T006 - DWG / ODA Evaluation

**State:** ARCHITECTURE GATE FROZEN / IMPLEMENTATION BRIEF RELEASED /
IMPLEMENTATION NOT AUTHORIZED

**Architecture Gate:** BIM-AG-P0-T006 v1.0

**Gate decision date:** 2026-09-22

**Architecture Authority:** Product Authority + ChatGPT

**Implementation Engineer:** Claude

**Independent Reviewer:** Kimi

**Architecture Change Record:** NONE

## Repository identity

Pre-governance-release parent:

```text
Base branch:
main

Main HEAD:
fe2dfa813070df0875eaff93ad40cd2af2ec1dad

Task branch:
task/P0-T006-dwg-oda-spike

Task HEAD:
fe2dfa813070df0875eaff93ad40cd2af2ec1dad

Task tree:
611728d8acc14dfd7b46187b1a1d82f3d1925196

Task worktree:
D:\Projects\BIM-Platform-WT-P0-T006
```

The exact Implementation Brief release baseline is intentionally not
self-referenced.

It is the governance-only commit/tree that first materializes the P0-T006
Gate, Task Record and Implementation Brief.

Architecture Authority will bind that exact post-C2 commit/tree in the
separate Implementation Authorization.

## Objective

Prove a vendor-neutral first-party DWG interoperability boundary using the
externally supplied proprietary ODA Drawings SDK, including controlled AC1018
same-version round trip and semantic-fidelity verification, without building
an in-house DWG parser or leaking ODA types outside the adapter.

## Frozen evaluation disposition

```text
ODA Drawings 27.7.0.0              = ACCEPTED TECHNICAL DIRECTION
Windows x64                        = ACCEPTED
VS2022/v143 compatibility          = PROVEN
DWG read                           = PROVEN
DWG write                          = PROVEN
AC1018 same-version write          = PROVEN
B10 semantic investigation         = CLOSED / PASS
Native object deletion observed    = NO
User-visible MText loss observed   = NO
ODA defect established             = NO
ACR                                = NONE
```

## Current authority state

```text
Architecture Gate             = FROZEN / APPROVED
Implementation Brief          = RELEASED
Implementation Authorization  = NOT ISSUED
Claude implementation         = NOT AUTHORIZED
Candidate staging             = NOT AUTHORIZED
Candidate commit              = NOT AUTHORIZED
Kimi review                   = NOT AUTHORIZED
Main integration              = NOT AUTHORIZED
Push                          = NOT AUTHORIZED
```

## Next gate

Materialize this governance delta as a docs-only commit after Architecture
Authority inspection.

Then issue a separate P0-T006 Implementation Authorization bound to that
exact resulting commit/tree.