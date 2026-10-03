# BIM Platform — P1-T001 Implementation Authorization

**Document ID:** BIM-TASK-P1-T001-AUTH  
**Version:** 1.0  
**Task:** P1-T001 — Core BIM Identity & Domain Model  
**Architecture Authority:** Product Authority + ChatGPT  
**Product Authority contract approval:** APPROVED  
**Implementation Authorization:** AUTHORIZED  
**Claude:** AUTHORIZED FOR P1-T001 IMPLEMENTATION ONLY  
**Kimi:** NOT AUTHORIZED  
**Commit:** NOT AUTHORIZED  
**Push:** NOT AUTHORIZED  
**ACR:** NONE  

## 1. Authoritative baseline

Implementation is authorized only in:

    Worktree = D:\Projects\BIM-Platform-WT-P1-T001
    Branch   = task/P1-T001-core-bim-identity-domain-model

Starting identity:

    HEAD = 1ec4958560835c241dfda5013ffaf7c556528d79
    TREE = 49f8e64465c29c72833ffdf10065dfec8646a861

Canonical `main` must not be modified by Claude.

## 2. Governing contracts

Claude MUST implement against:

    docs/tasks/P1-T001/01-ARCHITECTURE-GATE.md
    docs/tasks/P1-T001/02-IMPLEMENTATION-BRIEF.md
    docs/tasks/P1-T001/03-IMPLEMENTATION-AUTHORIZATION.md

The Architecture Gate and Implementation Brief are read-only implementation inputs.

Claude must not edit governance documents.

## 3. Frozen governance hashes

The pre-authorization governance corpus is frozen as:

    00-TASK-RECORD.md
    SHA256 = 90D3985ABB63FFE6BE5D17176374C8D5DEF3A855D445F5C16EF62DCCB1B292E8

    01-ARCHITECTURE-GATE.md
    SHA256 = 1B51478A85C45EA95C8A107A005102FDEE249919AB343AC53E4067134F853619

    02-IMPLEMENTATION-BRIEF.md
    SHA256 = 2EE494D4EF9F3A8DA7D7B372BC325BDADB565A7D4FF8A5AB9F51F7E58AD115E1

These files must remain byte-identical during Claude implementation.

## 4. Authorized production/test paths

Claude may create or modify only the following tracked implementation paths:

    src/model/include/bim/model/element_id.hpp
    src/model/include/bim/model/level.hpp
    src/model/include/bim/model/straight_wall.hpp
    src/model/include/bim/model/wall_face_reference.hpp
    src/model/include/bim/model/validation.hpp

    src/model/src/element_id.cpp
    src/model/src/validation.cpp

    src/model/CMakeLists.txt

    tests/unit/unit_model_element_id.cpp
    tests/unit/unit_model_domain_validation.cpp
    tests/unit/unit_model_wall_face_reference.cpp
    tests/unit/CMakeLists.txt

Ignored local build/test artifacts produced by the approved build system are permitted.

No other tracked repository path is authorized.

If a different tracked path appears necessary, Claude must STOP and return to Architecture Authority before changing it.

## 5. Required implementation

Implement exactly the frozen P1-T001 domain contract, including:

- project-owned 128-bit `ElementId`;
- all-zero invalid semantics;
- deterministic equality and lexicographic ordering;
- first-party standard-library UUID-v4-style generation;
- explicit generation failure;
- canonical lowercase 8-4-4-4-12 textual conversion;
- case-insensitive parser;
- parser acceptance of valid non-zero non-v4 assigned IDs;
- `Level`;
- model-owned `Point2D`;
- `StraightWall`;
- ordered start -> end semantics;
- structural wall validation;
- no hidden tolerance;
- `WallFaceRole`;
- `WallFaceReference`;
- structural wall-face-reference validation.

## 6. P0-T008 protection

Existing P0-T008 implementation and tests are frozen regression assets.

Claude MUST NOT modify:

    src/model/include/bim/model/persistent_face_reference.hpp
    src/model/include/bim/model/face_reference_resolver.hpp
    src/model/src/face_reference_resolver.cpp
    tests/unit/unit_model_face_reference_resolver.cpp
    tests/integration/integration_p0_t008_face_reference_spike.cpp

Do not alias or reinterpret:

    FeatureOwnerId
    FaceRole
    PersistentFaceReference
    FeatureFrame

as the production Phase-1 domain model.

## 7. Explicitly unauthorized scope

Claude MUST NOT implement:

- `bim_document`;
- ElementId <-> DependencyGraph::NodeId mapping;
- dependency graph registration;
- Level -> StraightWall runtime graph edges;
- dirty propagation or recompute;
- persistence/schema v2;
- save/reopen;
- commands;
- queries;
- transactions;
- geometry generation;
- production wall-face geometry resolution;
- tessellation;
- viewport integration;
- desktop/UI;
- IFC/DWG/RVT implementation;
- additional BIM element types.

## 8. Dependency/toolchain prohibition

Claude MUST NOT modify:

    vcpkg.json
    vcpkg-configuration.json
    CMakePresets.json

No new package or third-party dependency is authorized.

Do not modify `geometry_api`.

Do not weaken architecture-checker rules.

## 9. Required tests

Add and execute the focused Catch2/CTest coverage required by the Implementation Brief.

Expected focused test registrations:

    unit_model_element_id
    unit_model_domain_validation
    unit_model_wall_face_reference

Existing P0-T008 regression tests must remain unchanged and pass:

    unit_model_face_reference_resolver
    integration_p0_t008_face_reference_spike

Applicable full regression and architecture tests must also pass.

## 10. Stop / escalation conditions

Claude must STOP without workaround if implementation appears to require:

- a tracked path outside the authorized list;
- ElementId ownership outside `bim_model`;
- NodeId as durable identity;
- raw vendor/kernel/external identity in ElementId;
- third-party UUID dependency;
- hidden geometric tolerance;
- P0-T008 contract changes;
- geometry_api changes;
- persistence/schema changes;
- document/runtime ownership;
- dependency cycle;
- architecture-checker weakening;
- toolchain/dependency-baseline change;
- expansion beyond Level/StraightWall VS1 domain scope.

Report the conflict to Architecture Authority.

Do not open an ACR independently.

## 11. Git / repository prohibitions

Claude MUST NOT:

- commit;
- push;
- merge;
- rebase;
- fetch;
- reset;
- clean;
- stash;
- delete branches;
- remove worktrees;
- switch to `main`;
- edit governance documents.

All implementation changes must remain uncommitted in the isolated P1-T001 worktree for Architecture Authority review.

## 12. Required completion evidence

Claude's final implementation response must include:

1. starting branch;
2. starting HEAD;
3. starting TREE;
4. exact added/modified tracked paths;
5. confirmation that all changed tracked paths are authorized;
6. implementation summary for ElementId;
7. implementation summary for Level/StraightWall;
8. implementation summary for wall semantic face references;
9. model validation behavior;
10. exact build commands and results;
11. focused P1-T001 test results;
12. P0-T008 regression results;
13. applicable full regression result;
14. architecture-test result;
15. `git diff --check` result;
16. final `git status --short`;
17. confirmation governance files were not modified;
18. confirmation locked dependency/toolchain files were not modified;
19. stop-condition assessment;
20. ACR assessment.

## 13. Lifecycle authorization

Upon successful materialization and validation of this authorization document:

    P1-T001 implementation = AUTHORIZED
    Claude                 = AUTHORIZED FOR IMPLEMENTATION
    Kimi                   = NOT AUTHORIZED

    Commit                 = NOT AUTHORIZED
    Push                   = NOT AUTHORIZED
    Integration            = NOT AUTHORIZED
    Cleanup                = NOT AUTHORIZED

    ACR                    = NONE

Authorization is limited strictly to P1-T001 and the paths/scope above.
