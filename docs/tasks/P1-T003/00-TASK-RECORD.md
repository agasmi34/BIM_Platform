# P1-T003 - Commands & Query Vertical Slice

## Status

Architecture resolved and frozen.
Implementation is NOT authorized by this document.

## Baseline

- Commit: 7baca779f084fdcf22e8d3442332a4fd39341e2a
- Tree: 8ed6c0efbbd9bd451ad889dca99c053c341efd33
- Branch: task/P1-T003-commands-query-vertical-slice
- Worktree: D:\Projects\BIM-Platform-WT-P1-T003

## Authority chain

P0-T010 froze P1-T003 as the Commands & Query Vertical Slice.

P1-T003 owns exactly five VS1 mutation semantics:

1. CreateLevel
2. ChangeLevelElevation
3. CreateStraightWall
4. ChangeStraightWallGeometry
5. DeleteElement

It also owns read-only Level and StraightWall lookup/enumeration.

## Closed gates before materialization

- A1-R0: baseline / architecture discovery PASS
- A1-R1: isolated worktree bootstrap PASS
- F-P1T003-A1-01: CLOSED / HARNESS ONLY
- A2-R0: architecture contract resolution PASS
- A3: architecture APPROVED / FROZEN / CLOSED
- A4-R0: footprint / enforcement discovery PASS
- A4-R1: exact implementation footprint FROZEN

## Architecture summary

- commands -> document
- query -> document + model
- query is read-only
- no command dependency on transactions, persistence or dependency_graph
- no persistence/schema-v2 work
- no durable journal work
- no undo/redo
- no rehost
- no cascade deletion
- no dependency_graph public API extension

DeleteElement permits:
- StraightWall deletion;
- empty Level deletion.

DeleteElement rejects a Level with one or more dependent walls with
ElementHasDependents and zero mutation.

Deletion rebuilds the private dependency graph from surviving document state.
Surviving runtime node identities remain unchanged. A deleted runtime node
identity is never reused and the allocator is never decremented.

## Governance

Architecture conflict: NONE.
ACR: NONE.

No implementation, staging, commit, integration, push or cleanup is authorized
until a later explicit Architecture Authority gate.
