# PROJECT-STATUS

**Scope:** BIM Platform engineering repository

**System of record:** Git

**Current active task:** P0-T009 - Dependency Graph Spike

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

## Current - P0-T007 RVT / BimRv Evaluation Closure (2026-09-26)

~~~text
P0-T007 - RVT / BimRv Evaluation

Technical evaluation        = CLOSED / PASS
Read capability matrix      = ESTABLISHED
Controlled RVT open         = PASS
Vectorization runtime       = PASS
Geometry extraction         = PASS
RVT writer                  = PROHIBITED
Product integration         = NOT AUTHORIZED
Repository RVT code         = NONE
ACR                         = NONE

Current lifecycle gate:
documentation materialization -> Architecture Authority diff/path-set review

Claude                      = NOT AUTHORIZED
Kimi                        = NOT AUTHORIZED
Candidate commit            = NOT AUTHORIZED
Main integration            = NOT AUTHORIZED
Push                        = NOT AUTHORIZED
~~~

## Current - P0-T008 Topological Reference Spike Architecture Gate (2026-09-27)

```text
P0-T008 - Topological Reference Spike

Phase A bootstrap                 = CLOSED / PASS
Phase B1 authority extraction     = CLOSED / PASS
Phase B2 architecture contract    = AA FROZEN
Phase B3 schema discovery         = CLOSED / PASS
Architecture Gate                 = FROZEN / APPROVED
Product Authority approval       = APPROVED

Primary objective:
semantic reference / persistent naming proof

Persistent face references        = IN SCOPE
Edge/vertex persistent refs       = OUT OF SCOPE
Raw OCCT topology identity        = PROHIBITED
Enumeration-index identity        = PROHIBITED
Dependency graph                  = P0-T009 / OUT OF SCOPE
Phase-1 BIM features              = BLOCKED
Implementation Brief              = NOT RELEASED
Implementation Authorization      = NOT ISSUED
Claude                            = NOT AUTHORIZED
Kimi                              = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

Next lifecycle gate:

```text
Architecture Authority inspection of approved seven-path governance delta
    ->
governance-only candidate commit authorization
    ->
candidate commit only if separately authorized
```

## Current - P0-T008 Implementation Brief Approved (2026-09-28)

```text
Architecture Gate                 = FROZEN / APPROVED
Governance baseline HEAD          = df9536d20495d97034d3aef5b4e81adb54bfa520
Implementation Brief              = P0-T008-IB v1.0 / FROZEN / APPROVED
Product Authority approval        = APPROVED
Implementation Authorization      = NOT ISSUED
Claude                            = NOT AUTHORIZED
Kimi                              = NOT AUTHORIZED
Source implementation             = NONE
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

Next lifecycle gate:

```text
Architecture Authority inspection of approved brief materialization
    ->
governance-only brief commit authorization
    ->
exact brief baseline capture
    ->
separate Implementation Authorization
```

## Current - P0-T008 Implementation Authorization Issued (2026-09-28)

```text
Architecture Gate                 = FROZEN / APPROVED
Implementation Brief              = FROZEN / APPROVED
Approved brief baseline HEAD       = 31a5b9c6ba1c679ab34858089af14607d78ecdd9
Implementation Authorization      = P0-T008-IA v1.0 / ISSUED
Execution baseline                = FINAL AUTHORIZATION GOVERNANCE COMMIT / TO BE CAPTURED
Execution activation              = PENDING CORRECTED AUTHORIZATION BASELINE CAPTURE
Claude                            = NOT YET AUTHORIZED TO EXECUTE
Kimi                              = NOT AUTHORIZED
Source implementation             = NONE
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

Next lifecycle gate:

```text
AA inspection of authorization materialization
    ->
governance-only authorization commit
    ->
exact authorization baseline capture
    ->
AA execution activation / Claude handover
```

## Current - P0-T009 Dependency Graph Spike Architecture Gate (2026-09-29)

```text
P0-T009 - Dependency Graph Spike

Bootstrap                     = PASS / CLOSED
Architecture baseline capture = PASS / CLOSED
BIM-AG-P0-T009 v1.0          = FROZEN / APPROVED
Product Authority             = APPROVED

Graph owner                   = bim_dependency_graph
Production dependency         = bim::foundation only
Direction                     = upstream -> downstream
Dirty propagation             = affected downstream only
Planning                      = deterministic topological
Cycle policy                  = reject before mutation
Computed refresh              = atomic
Recompute clean-state commit  = atomic / fail closed
P0-T008 semantics             = preserved
Persistence                   = out of scope
Production BIM features       = out of scope
Parallel scheduler            = out of scope
Third-party dependency delta  = none
ACR                           = NONE

Implementation Brief          = NOT RELEASED
Implementation Authorization  = NOT ISSUED
Claude                        = NOT AUTHORIZED
Kimi                          = NOT AUTHORIZED
Commit                        = NOT AUTHORIZED
Push                          = NOT AUTHORIZED
```

## Current - P0-T009 Implementation Brief Approved (2026-09-29)

```text
P0-T009 - Dependency Graph Spike

Architecture Gate              = FROZEN / APPROVED
P0-T009-IB v1.0               = FROZEN / APPROVED
Product Authority              = APPROVED

Public primitive               = bim::dependency_graph
Production dependency          = bim::foundation only
Initial candidate manifest     = 8 implementation/test paths
NodeId                         = project-owned opaque uint64 contract
Dependency provenance          = ExplicitSemantic / Computed
Dirty propagation              = downstream / deterministic
Planning                       = topological / deterministic
Cycle mutation                 = reject before commit
Computed replacement           = atomic
Recompute Clean commit         = atomic
P0-T008 contracts              = unchanged
Persistence                    = out of scope
New third-party dependency     = none
ACR                            = NONE

Implementation Authorization   = NOT ISSUED
Implementation                 = NOT AUTHORIZED
Claude                         = NOT AUTHORIZED
Kimi                           = NOT AUTHORIZED
Candidate commit               = NOT AUTHORIZED
Main integration               = NOT AUTHORIZED
Push                           = NOT AUTHORIZED

Next lifecycle gate:
Implementation Brief governance audit
    ->
Implementation Brief governance commit
    ->
separate Implementation Authorization
    ->
separate execution activation
```

## Current - P0-T009 Implementation Authorization Issued (2026-09-29)

```text
P0-T009 - Dependency Graph Spike

Architecture Gate              = FROZEN / APPROVED
P0-T009-IB v1.0               = FROZEN / APPROVED
BIM-AUTH-P0-T009 v1.0         = ISSUED / FROZEN
Product Authority              = APPROVED

Initial execution manifest     = 8 paths / FROZEN
Production dependency          = bim::foundation only
New third-party dependency     = none
P0-T008 contracts              = unchanged
ACR                            = NONE

Execution Activation           = NOT ISSUED
Implementation                 = NOT AUTHORIZED TO START
Claude Execution               = NOT ACTIVE
Kimi                           = NOT AUTHORIZED
Candidate commit               = NOT AUTHORIZED
Main integration               = NOT AUTHORIZED
Push                           = NOT AUTHORIZED

Next lifecycle gate:
Authorization governance audit
    ->
Authorization governance commit
    ->
Activation baseline lock
    ->
explicit Architecture Authority Execution Activation
    ->
Claude implementation may begin
```
