# P0-T004 - Task Record

## Identity

Task:

P0-T004 - Persistence Spike

Architecture Gate:

BIM-AG-P0-T004 v1.0

Date:

2026-09-14

Authoritative parent:

5bac905e29c1390c90cc6807a5d92ab217764396

Authoritative parent tree:

3121a74e2e5edf960adfe5f36d0684541c805a1a

Branch:

task/P0-T004-persistence-spike

Worktree:

D:\Projects\BIM-Platform-WT-P0-T004

## Authority

Product Authority:

APPROVED

Architecture Authority:

APPROVED

Architecture Gate:

APPROVED + LOCKED

Discovery:

PASS

Discovery classification:

READY_FOR_ARCHITECTURE_GATE_DRAFT

ACR:

NONE

## Purpose

Prove the first durable persistence slice of the BIM Platform without
prematurely defining the complete BIM project-file format.

The task must prove:

- SQLite remains private to bim_persistence;
- schema versioning uses PRAGMA user_version;
- an empty database bootstraps atomically to schema version 1;
- unsupported or unrecognized database states fail closed;
- bim_transactions owns a neutral SQLite-independent journal contract;
- journal transactions persist atomically through bim_persistence;
- committed journal data survives close/reopen;
- uncommitted journal data is absent after close/reopen;
- duplicate transaction IDs fail atomically;
- binary payloads round-trip exactly;
- persistent journal ordering is deterministic;
- architecture enforcement proves SQLite ownership and public-header neutrality.

## Locked non-goals

P0-T004 does not authorize:

- BIM domain tables;
- Wall/Slab/Door/Window persistence;
- OCCT shape serialization;
- command execution or product undo/redo;
- collaboration, cloud synchronization or multi-user locking;
- final project-file extension or commercial format identity;
- schema version 2+;
- ORM introduction;
- SQLite replacement;
- vcpkg baseline movement.

## Current state

ARCHITECTURE GATE APPROVED + LOCKED

Implementation:

NOT AUTHORIZED

Authorized next activity:

Prepare and review the P0-T004 Implementation Brief, then issue a separate
Architecture Authority implementation authorization.

No production source implementation may start from this record alone.
