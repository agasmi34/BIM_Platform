# P0-T008 - Task Record

**Task:** P0-T008 - Topological Reference Spike
**Task branch:** `task/P0-T008-topological-reference-spike`
**Task worktree:** `D:\Projects\BIM-Platform-WT-P0-T008`
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude - NOT AUTHORIZED
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## Baseline

| Item | Value |
|---|---|
| Parent HEAD | `385e07b2a7306664d1063281ab150657dadcb154` |
| Parent TREE | `fe52cdb590b4aea0137d4561ea847d59a4987e99` |
| Objective | Semantic reference / persistent naming proof |
| Architecture Gate | `BIM-AG-P0-T008 v1.0` - FROZEN / APPROVED |
| Implementation Brief | NOT RELEASED |
| Implementation Authorization | NOT ISSUED |

## Scope state

P0-T008 is architecture-first.

The spike shall prove whether a project-owned semantic face reference can remain meaningful across controlled geometry regeneration without making raw OCCT topology identity part of the persistent contract.

Persistent face references are in scope.

Persistent edge and vertex references are out of scope.

Dependency-graph behavior belongs to P0-T009.

Phase-1 BIM feature implementation remains blocked.

## Locked prohibitions

A durable P0-T008 reference must not derive identity from raw `TopoDS_*` identity, an OCCT address/handle, face or edge enumeration order, `input_face_ordinal`, or transient ordering of OCCT history containers.

OCCT history may support evidence only.

No production Wall, Door, Slab, Window, Room, IFC, DWG or RVT reference integration is authorized by this task record.

## Authorization state

| Authority item | State |
|---|---|
| Architecture Gate draft materialization | AUTHORIZED |
| Product Authority Gate approval | APPROVED |
| Implementation Brief | NOT RELEASED |
| Implementation Authorization | NOT ISSUED |
| Claude execution | NOT AUTHORIZED |
| Kimi independent review | NOT AUTHORIZED |
| Git staging | NOT AUTHORIZED |
| Candidate commit | NOT AUTHORIZED |
| Main integration | NOT AUTHORIZED |
| Push | NOT AUTHORIZED |
| ACR | NONE |

Product Authority approved `BIM-AG-P0-T008 v1.0` on 2026-09-27. The next checkpoint is Architecture Authority inspection of the approved seven-path governance delta before any staging or candidate commit. No Implementation Brief or implementation authority is implied by this approval.

## Implementation Brief approval

`P0-T008-IB v1.0` is FROZEN / APPROVED against governance baseline
`df9536d20495d97034d3aef5b4e81adb54bfa520`.

Product Authority approval is APPROVED. Implementation Authorization is NOT ISSUED.
Claude and Kimi remain NOT AUTHORIZED. No source/CMake/test modification, staging,
candidate commit, main integration, or push is authorized by this brief approval alone.
