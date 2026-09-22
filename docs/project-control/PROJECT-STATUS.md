# PROJECT-STATUS

**Scope:** BIM Platform engineering repository

**System of record:** Git

**Current active task:** P0-T005 — IFC Spike

## P0-T001

ACCEPTED + CLOSED + INTEGRATED

Main baseline:

8c1c38990f75d5b0122e90d85bb8757e83a553a1

## P0-T002

ACCEPTED + CLOSED + INTEGRATED

Implementation commit:

2dca329ecc7ebeaeee27dba85017db7aad3842ba

Closure commit:

5e266a2629c2f9502505dd36bfe6986e5caec0cc

Final integration-record commit / previous authoritative main baseline:

3f2230be5fcd796c370f485975547112ad52d2e3

Full Authoritative Runbook C:

PASS

D2-A/B/C/D:

ISSUED

Kimi implementation review:

PASS

AC021:

PASS

AC022:

PASS

ACR:

NONE

## P0-T003

ACCEPTED + CLOSED + INTEGRATED

Task:

Desktop + Viewport Spike

Architecture Gate:

BIM-AG-P0-T003 v1.0

Implementation commit / integrated main baseline:

5bac905e29c1390c90cc6807a5d92ab217764396

Integrated tree:

3121a74e2e5edf960adfe5f36d0684541c805a1a

Integration method:

FAST-FORWARD

Full Verification Runbook D:

CLOSED PASS

Authoritative Runbook D attempt:

ATTEMPT 5

Independent implementation review:

APPROVED FOR ARCHITECTURE AUTHORITY INTEGRATION DECISION

BLOCKER:

0

MAJOR:

0

MINOR:

1

NOTE:

3

Local task worktree:

REMOVED

Local task branch:

DELETED

ACR:

NONE

## P0-T004

Task:

Persistence Spike

Architecture Gate:

BIM-AG-P0-T004 v1.0

Status:

APPROVED + LOCKED

Authoritative parent:

5bac905e29c1390c90cc6807a5d92ab217764396

Authoritative parent tree:

3121a74e2e5edf960adfe5f36d0684541c805a1a

Branch:

task/P0-T004-persistence-spike

Worktree:

D:\Projects\BIM-Platform-WT-P0-T004

Discovery:

PASS

Discovery classification:

READY_FOR_ARCHITECTURE_GATE_DRAFT

Locked direction:

SQLite schema version 1 + neutral transaction journal proof behind
bim_persistence.

SQLite ownership:

src/persistence/** only

Schema version authority:

PRAGMA user_version

Current ACR:

NONE

Implementation Brief:

APPROVED + RELEASED

Implementation Authorization:

BIM-AUTH-P0-T004 v1.0

Implementation:

AUTHORIZED

Implementation commit:

NOT AUTHORIZED

Main integration:

NOT AUTHORIZED

## Next state transition

Claude begins P0-T004 Phase A on the authorized task branch/worktree and
proceeds through the released brief using minimum necessary delta.

After implementation verification and candidate freeze, control returns to
Architecture Authority before independent review or any implementation commit.

## P0-T005 — IFC Spike

Architecture Gate:

`BIM-AG-P0-T005 v1.0 = APPROVED`

Approved base:

`main@2c2b89f73651f7d5981d42546cbc0e321d6bb055`

Task branch:

`task/P0-T005-ifc-spike`

Task worktree:

`D:\Projects\BIM-Platform-WT-P0-T005`

Next gate:

`IfcOpenShell dependency-resolution preflight against frozen vcpkg baseline`

Implementation Brief:

`NOT RELEASED`

Implementation:

`NOT AUTHORIZED`

Current ACR:

`NONE`

## P0-T005 dependency status

```text
ACR-P0-T005-001               = APPROVED
IfcOpenShell vcpkg resolution = FAILED — PORT ABSENT
vcpkg baseline                = UNCHANGED
Approved dependency route     = PINNED UPSTREAM SOURCE / EXTERNAL PREFIX
Release line                  = 0.8.5
Exact commit                  = PENDING PHASE B
Implementation Brief          = NOT RELEASED
Implementation                = NOT AUTHORIZED
```

## P0-T005 implementation gate

```text
Architecture Gate             = APPROVED
ACR-P0-T005-001               = APPROVED
Dependency Resolution Phase C = CLOSED / PASS
Implementation Brief          = RELEASED
Implementation                = NOT AUTHORIZED
Independent Review            = NOT AUTHORIZED
Main integration              = NOT AUTHORIZED
Push                          = NOT AUTHORIZED
```

Next gate: separate Architecture Authority Implementation Authorization.

## P0-T005 implementation execution

```text
Architecture Gate             = APPROVED
ACR-P0-T005-001               = APPROVED
Dependency Resolution Phase C = CLOSED / PASS
Implementation Brief          = RELEASED
Implementation Authorization  = ISSUED
Claude implementation         = AUTHORIZED
Candidate staging/commit      = NOT AUTHORIZED
Independent Review            = NOT AUTHORIZED
Main integration              = NOT AUTHORIZED
Push                          = NOT AUTHORIZED
```

Current execution owner: Claude, minimum-delta implementation only.

## P0-T005 baseline clarification hold

```text
Implementation Authorization = ISSUED
Baseline conflict             = CLARIFIED
Execution Packet v1.0         = WITHDRAWN FOR EXECUTION
Claude implementation        = PAUSED
Execution Packet v1.1         = PENDING
Candidate staging/commit     = NOT AUTHORIZED
Independent Review           = NOT AUTHORIZED
Main integration             = NOT AUTHORIZED
Push                         = NOT AUTHORIZED
```

## Current - P0-T006 DWG / ODA Evaluation (2026-09-22)

Current active Phase-0 task:

```text
P0-T006 - DWG / ODA Evaluation
```

Lifecycle:

```text
Discovery / evaluation            = CLOSED / PASS
B10 semantic fidelity             = CLOSED / PASS
Architecture Gate                 = FROZEN / APPROVED
Implementation Brief              = RELEASED
Implementation Authorization      = NOT ISSUED
Claude implementation             = NOT AUTHORIZED
Candidate staging                 = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Independent Review                = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

Frozen architecture:

```text
ODA Drawings direction            = ACCEPTED
No in-house DWG parser            = LOCKED
ODA sole owner                    = src/interop/dwg/**
Public API                        = vendor-neutral
ODA dependency                    = external / proprietary
DWG build                         = opt-in / default OFF
Controlled round trip             = AC1018 -> vAC18 -> AC1018
Fidelity basis                    = semantic, not binary/handle equality
```

Next gate:

```text
Architecture Authority inspection
    ->
governance-only materialization commit
    ->
exact release HEAD/TREE capture
    ->
separate P0-T006 Implementation Authorization
```

## Current - P0-T006 Implementation Authorization (2026-09-22)

```text
Architecture Gate                 = FROZEN / APPROVED
Implementation Brief              = RELEASED / MATERIALIZED

C2B HEAD                          = 6afb2462a3dddc1c4f0e7780f857acae5733d3fc
C2B TREE                          = 682e1e48ea56a755cab9904b4d6bebf400e3a8dd

Implementation Authorization      = APPROVED
C3 materialization                = PENDING
Claude execution                  = NOT YET ACTIVE

Kimi                              = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED

ACR                               = NONE
```

Next:

```text
C3 governance-only materialization commit
    ->
record exact clean post-C3 HEAD/TREE
    ->
activate Claude implementation from that state
```
