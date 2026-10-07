# P1-T003 Implementation Brief

ID: P1-T003-IB
Architecture: BIM-AG-P1-T003
Status: FROZEN / NOT IMPLEMENTATION AUTHORIZATION

## 1. Exact implementation footprint

Exactly 18 implementation paths are authorized for a later implementation gate.

### Modified existing paths

1. src/document/include/bim/document/document.hpp
2. src/document/src/document.cpp
3. src/commands/CMakeLists.txt
4. src/query/CMakeLists.txt
5. tests/unit/CMakeLists.txt
6. tests/integration/CMakeLists.txt
7. tests/architecture/CMakeLists.txt
8. tools/architecture_checker.cmake

### Deleted obsolete anchor paths

9. src/commands/src/commands_anchor.cpp
10. src/query/src/query_anchor.cpp

### New paths

11. src/commands/include/bim/commands/commands.hpp
12. src/commands/src/commands.cpp
13. src/query/include/bim/query/query.hpp
14. src/query/src/query.cpp
15. tests/unit/unit_p1_t003_commands_query.cpp
16. tests/integration/integration_p1_t003_commands_query_vertical_slice.cpp
17. tests/fixtures/p1_t003_bad_commands_public_boundary/commands/include/bim/commands/bad_commands.hpp
18. tests/fixtures/p1_t003_bad_query_mutation/query/include/bim/query/bad_query.hpp

No root CMakeLists.txt modification is required or authorized.

No src/document/CMakeLists.txt modification is required or authorized.

No src/dependency_graph/**, src/transactions/**, src/model/**,
src/persistence/**, src/geometry/** or src/desktop/** modification is authorized.

## 2. Commands target

Replace the P0 compilation anchor with production commands.cpp.

bim_commands links to bim::document and exposes its own public include
directory.

It does not link bim::transactions, bim::persistence or
bim::dependency_graph.

## 3. Query target

Replace the P0 compilation anchor with production query.cpp.

bim_query exposes its own public include directory and depends on
bim::document + bim::model.

It remains read-only.

## 4. Command API

Namespace: bim::commands

Provide:

- CommandResultCode
- CommandResult
- CreateLevel
- ChangeLevelElevation
- CreateStraightWall
- ChangeStraightWallGeometry
- DeleteElement

CreateLevel accepts Document& and const Level&.

ChangeLevelElevation accepts Document&, ElementId and elevation.

CreateStraightWall accepts Document& and const StraightWall&.

ChangeStraightWallGeometry accepts Document&, wall ElementId, start, end,
thickness, base_offset and height. It reads the current wall and preserves the
existing level_id.

DeleteElement accepts Document& and ElementId.

Every command is noexcept and translates DocumentResult into CommandResult.

## 5. Query API

Namespace: bim::query

Provide:

- FindLevel(const Document&, ElementId)
- FindStraightWall(const Document&, ElementId)
- ListLevels(const Document&)
- ListStraightWalls(const Document&)

Find operations return optional copies.

List operations return vector copies ordered by ElementId ascending.

No mutable Document reference is allowed.

## 6. Document deletion

Add ElementHasDependents.

Add DeleteElement.

For a wall:
- remove wall authoritative state;
- remove wall geometry;
- remove its forward/reverse runtime mapping;
- rebuild graph using survivor runtime identities.

For an empty Level:
- remove Level authoritative state;
- remove its forward/reverse runtime mapping;
- rebuild graph.

For a Level with any wall:
- return ElementHasDependents;
- zero mutation.

Rebuilding the graph must not allocate new runtime node ids.

next_node_value remains unchanged.

## 7. Document enumeration

Add ListLevels and ListStraightWalls.

State containers are already keyed by ElementId, but the contract is semantic:
returned values must be in ElementId ascending order regardless of internal
container choices.

Reads expose committed state only.

## 8. R19

Add selector COMMANDS_PUBLIC_BOUNDARY.

The real src tree must pass.

The controlled bad commands fixture must fail.

Do not modify R1-R18 semantics/token sets.

## 9. R20

Add selector QUERY_READ_ONLY_BOUNDARY.

The real src tree must pass.

The controlled bad query fixture must fail.

The rule must distinguish the allowed const bim::document::Document& from a
mutable bim::document::Document&.

Do not weaken this requirement by simply forbidding every Document& substring.

## 10. Tests

Add exactly six CTest registrations:

1. unit_p1_t003_commands_query
2. integration_p1_t003_commands_query_vertical_slice
3. arch_commands_public_boundary
4. arch_p1_t003_commands_public_fixture_rejected
5. arch_query_read_only_boundary
6. arch_p1_t003_query_mutation_fixture_rejected

The two fixture tests use WILL_FAIL TRUE.

Baseline authoritative full CTest count is 62.
Expected full count after P1-T003 is 68.

Baseline architecture count is 26.
Expected architecture count is 30.

Focused P1-T003 runtime count is 2.

Required regression groups:
- P1-T002: 2
- P1-T001: 3
- P0-T009: 2

Do not invent or freeze a build-target count.

## 11. Required behavioral proof

Cover at minimum:

- CreateLevel success and invalid rejection;
- CreateStraightWall success and missing-Level rejection;
- ChangeLevelElevation dependent wall recompute;
- ChangeStraightWallGeometry success;
- invalid wall geometry zero mutation;
- no wall rehost;
- delete StraightWall success;
- delete empty Level success;
- reject deletion of Level with dependents;
- failed deletion has zero observable mutation;
- subsequent runtime allocation does not reuse the deleted runtime identity by
  implementation/source invariant; no public NodeId test hook may be added;
- survivor mappings are preserved by graph rebuild implementation;
- query lookup;
- deterministic Level enumeration;
- deterministic StraightWall enumeration;
- failed commands do not change query results;
- query observes only committed state.

NodeId is deliberately private. Do not add a public/debug NodeId accessor merely
to make a black-box test possible. Non-reuse and survivor stability are also
verified by Architecture Authority source review of the staged implementation.

## 12. Validation

Use the frozen native Windows toolchain/preset process.

BIM_ENABLE_DWG remains OFF for the authoritative standard validation.

Required:
- configure PASS;
- build PASS;
- exact CTest registration check;
- focused P1-T003 2/2;
- P1-T002 regression 2/2;
- P1-T001 regression 3/3;
- P0-T009 regression 2/2;
- architecture 30/30;
- full 68/68.

No expensive rerun is required after later governance-only operations when
candidate bytes remain unchanged.

## 13. No authorization

This brief defines implementation requirements.
It does not authorize implementation, staging, commit, merge, push or cleanup.
