# P1-T004 - Task Record

Task: P1-T004 - Persistence Schema v2
Date opened: 2026-10-10
Architecture Authority: Abdallah Gami + ChatGPT
Implementation Engineer: Claude
Independent Reviewer: Kimi

## Baseline

Commit:
104b0c9f82cf304cdd16ee0178f0d69486cd3b3b

Tree:
5e9518ddc4470b26094189a04aa0aa46bba8b275

Branch:
task/P1-T004-persistence-schema-v2

Worktree:
D:\Projects\BIM-Platform-WT-P1-T004

## Objective

Implement Phase-1 persistence schema version 2 for the VS1 Level / StraightWall
vertical slice, including durable ElementId state, atomic schema migration,
save/reopen, runtime reconstruction inputs and durable journal coordination.

## Frozen upstream authority

P0-T010 assigns P1-T004 ownership of:

- schema v2;
- atomic 1 -> 2 migration;
- Level / StraightWall durable state;
- ElementId persistence;
- save/reopen;
- graph reconstruction inputs.

P1-T003 defers durable persistence and journal coordination to this task.

## Gate state

A1 - repository bootstrap / discovery: CLOSED / PASS
A2 - architecture contract resolution: CLOSED / PASS
A3 - architecture freeze / exact footprint discovery: CLOSED / PASS
A4 - governance materialization: IN PROGRESS

Implementation: NOT AUTHORIZED
Independent review: NOT AUTHORIZED
Staging: NOT AUTHORIZED
Commit: NOT AUTHORIZED
Push: NOT AUTHORIZED

ACR: NONE

## Exact implementation footprint

23 paths exactly:

- 13 modified existing paths;
- 10 new paths;
- 0 deleted paths.

The exact path list is frozen in 02-IMPLEMENTATION-BRIEF.md.

No file outside that implementation footprint may be modified without returning to
Architecture Authority.

The four docs/tasks/P1-T004 governance files are governance state and are not
counted inside the 23 implementation paths.
