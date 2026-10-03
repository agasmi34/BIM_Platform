# BIM Platform — P1-T001 Implementation Brief

**Document ID:** BIM-TASK-P1-T001-CLAUDE  
**Version:** 1.0  
**Task:** P1-T001 — Core BIM Identity & Domain Model  
**Architecture Authority:** Product Authority + ChatGPT  
**Product Authority approval:** APPROVED  
**Implementation authorization:** NOT AUTHORIZED BY THIS BRIEF  
**ACR:** NONE  

## 1. Authoritative Baseline

Implementation, when separately authorized, MUST start from:

    HEAD = 1ec4958560835c241dfda5013ffaf7c556528d79
    TREE = 49f8e64465c29c72833ffdf10065dfec8646a861

P0-T010 is APPROVED / CLOSED.

No dependency-baseline modification is authorized.

## 2. Objective

P1-T001 introduces the first production BIM domain types required by VS1 — Level-Constrained Straight Wall.

The task owns only:

- durable `ElementId`;
- `Level`;
- `Point2D`;
- `StraightWall`;
- production wall semantic face-role/reference contract;
- structural model validation;
- ElementId generation and canonical textual conversion.

It MUST NOT implement document runtime, dependency-graph integration, persistence, commands, queries, desktop/UI, geometry generation, tessellation or viewport integration.

## 3. Module Ownership

All P1-T001 production BIM domain contracts belong to:

    bim_model
    bim::model
    src/model/**

`ElementId` MUST NOT be placed in `bim_foundation`.

`bim_dependency_graph` MUST remain model-neutral and MUST NOT depend on `ElementId`.

No new first-party module is authorized by P1-T001.

## 4. ElementId

### 4.1 Representation

Production `ElementId` is a project-owned 128-bit value.

Required representation semantics:

- exactly 16 identity bytes;
- all-zero is invalid;
- equality comparable;
- deterministically lexicographically orderable;
- trivially/copy-friendly value semantics;
- no pointer, database rowid, kernel identity, GPU identity or external-format identifier.

Recommended public representation:

    struct ElementId {
        std::array<std::uint8_t, 16> bytes{};
        ...
    };

The implementation MUST NOT reinterpret the identifier through host-endian integer layout.

Canonical byte order is the left-to-right byte order used by textual and future persistence representations.

### 4.2 Generation

Production generation MUST be first-party.

Use only the C++ standard library already available in the locked toolchain.

Generation requirements:

- obtain random bytes using standard facilities;
- generate 128 bits;
- set UUID-v4-style version bits to version 4;
- set RFC-4122-style variant bits to binary `10`;
- never generate/use all-zero as a valid ElementId;
- do not incorporate wall clock, PID, memory address, database rowid, machine-specific vendor identity, IFC GlobalId, DWG handle or RVT ElementId;
- do not add a third-party UUID dependency.

Generation failure MUST be represented explicitly to the caller.

No exception originating from the entropy provider may escape the project-owned public generation function.

A suitable public shape is:

    std::optional<ElementId> GenerateElementId();

Exact private implementation helpers remain implementation-owned.

### 4.3 Canonical text form

Canonical output:

    xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

Requirements:

- exactly 36 characters;
- lowercase hexadecimal output;
- hyphens at positions 8, 13, 18 and 23;
- parser may accept upper- or lowercase hexadecimal;
- malformed text fails closed;
- incorrect length fails closed;
- misplaced/missing hyphens fail closed;
- non-hex characters fail closed;
- parsed all-zero ElementId fails closed.

Suggested public operations:

    bool IsValid(const ElementId&) noexcept;
    std::string ToString(const ElementId&);
    std::optional<ElementId> ParseElementId(std::string_view);

The parser MUST NOT require version-4 or variant bits for an otherwise valid non-zero assigned ElementId.

The generator produces UUID-v4-style values; the durable ElementId domain itself remains the frozen project-owned 128-bit identity space.

## 5. Level

Production `Level` minimum authoritative state:

    struct Level {
        ElementId id;
        double elevation;
    };

No production name/label field is required in P1-T001.

`elevation` is in model units.

Valid elevations may be positive, zero or negative.

Validation MUST reject:

- invalid/all-zero `id`;
- NaN elevation;
- positive infinity;
- negative infinity.

No tolerance is involved in Level structural validation.

## 6. Point2D

P1-T001 introduces a model-owned horizontal point type:

    struct Point2D {
        double x;
        double y;
    };

It represents plan coordinates only.

It MUST remain:

- project-owned;
- kernel-neutral;
- persistence-neutral;
- UI-neutral.

Do not use an external-library point type as the model contract.

## 7. StraightWall

Production minimum authoritative state:

    struct StraightWall {
        ElementId id;
        ElementId level_id;
        Point2D start;
        Point2D end;
        double thickness;
        double height;
        double base_offset;
    };

### 7.1 Ordered wall axis

The ordered wall axis is:

    start -> end

This order is semantically significant.

Implementation MUST NOT:

- sort endpoints;
- swap endpoints automatically;
- canonicalize axis direction;
- discard start/end orientation.

Swapping the two endpoints describes the opposite semantic wall direction.

### 7.2 Vertical placement

Wall base elevation is conceptually:

    base_z =
        referenced_level.elevation
        + wall.base_offset

Wall top elevation is:

    top_z =
        base_z
        + wall.height

P1-T001 does NOT look up the Level or calculate runtime geometry.

These equations freeze domain meaning only.

### 7.3 Structural validation

A `StraightWall` is structurally valid only when:

- `id` is valid;
- `level_id` is valid;
- `id != level_id`;
- start.x/start.y/end.x/end.y are finite;
- start and end are not exactly identical;
- thickness is finite and strictly greater than zero;
- height is finite and strictly greater than zero;
- base_offset is finite.

Negative `base_offset` is valid.

Positive, zero and negative plan coordinates are valid.

No hidden geometric epsilon is permitted.

Therefore a non-zero but very short wall axis is NOT rejected by P1-T001 merely because it falls below some implicit tolerance.

Tolerance-aware geometric qualification belongs to later runtime/geometry evaluation with an explicit document/project tolerance.

## 8. Wall Frame Semantics

The production conceptual wall frame is frozen as:

    W = +Z
    U = normalize(end - start)
    V = W × U

Therefore:

    U × V = W

The wall axis is the centerline.

Thickness is centered on the axis:

    SideA = -V * thickness / 2
    SideB = +V * thickness / 2

These equations freeze semantics; P1-T001 does not construct kernel geometry.

## 9. Production Wall Face Contract

Introduce a production-specific semantic enum:

    enum class WallFaceRole : std::uint8_t {
        Bottom,
        Top,
        Start,
        End,
        SideA,
        SideB,
    };

Only these six values are valid for VS1.

Provide explicit validation for an out-of-range enum value.

Introduce:

    struct WallFaceReference {
        ElementId wall_id;
        WallFaceRole role;
    };

A structurally valid wall-face reference requires:

- valid non-zero `wall_id`;
- valid `WallFaceRole`.

No raw face index, kernel handle, pointer, topology enumeration ordinal or transient geometry identifier may be stored in this reference.

## 10. Frozen Semantic Meaning of the Six Wall Faces

The six roles mean:

    Bottom = minimum W plane / wall base
    Top    = maximum W plane / wall top

    Start  = minimum U plane / start endpoint
    End    = maximum U plane / end endpoint

    SideA  = minimum V plane
    SideB  = maximum V plane

Enumeration order is never identity.

No seventh role is authorized in VS1.

## 11. P0-T008 Transition Law

Existing P0-T008 types remain a historical spike contract:

    FeatureOwnerId
    FaceRole
    PersistentFaceReference
    FeatureFrame
    ResolveFaceReference(...)

P1-T001 MUST NOT:

- rename them into production types;
- alias `FeatureOwnerId` to `ElementId`;
- widen `FeatureOwnerId` from 64 bits to 128 bits;
- replace P0-T008 `FaceRole`;
- rewrite P0-T008 proof tests to look like Phase-1 production tests;
- reinterpret P0-T008 evidence as the production data model.

The historical geometric correspondence is:

    Bottom  <-> StartCap
    Top     <-> EndCap

    Start   <-> UMinSide
    End     <-> UMaxSide

    SideA   <-> VMinSide
    SideB   <-> VMaxSide

This correspondence is conceptual evidence only.

No production API is to depend on those historical enum names.

Existing P0-T008 regression tests MUST continue to pass unchanged.

## 12. Face Resolution Scope

P1-T001 owns only:

- `WallFaceRole`;
- `WallFaceReference`;
- structural role/reference validity.

It does NOT own production wall-face resolution against generated geometry.

Production `Resolved / Missing / Ambiguous / invalid reference-or-owner` behavior is integrated in later runtime/geometry work while preserving the Phase-0 fail-closed proof.

Do not create a fake resolver in P1-T001.

## 13. Model Validation API

Validation MUST be deterministic, side-effect free and vendor-neutral.

It MUST distinguish structural invalidity without performing:

- document lookup;
- graph lookup;
- persistence access;
- geometry generation;
- tolerance-based geometric evaluation.

A project-owned validation result/code API is permitted and preferred over exceptions.

Required validation coverage includes:

### ElementId

- all-zero invalid;
- non-zero valid.

### Level

- invalid id rejected;
- non-finite elevation rejected.

### StraightWall

- invalid wall id rejected;
- invalid level id rejected;
- same wall/level ElementId rejected;
- non-finite coordinates rejected;
- exactly coincident endpoints rejected;
- non-finite thickness rejected;
- thickness <= 0 rejected;
- non-finite height rejected;
- height <= 0 rejected;
- non-finite base_offset rejected;
- negative base_offset accepted;
- short-but-nonzero axis accepted structurally.

### WallFaceReference

- zero wall id rejected;
- invalid role rejected.

## 14. Recommended Production File Layout

The implementation SHOULD use the following narrow layout unless a concrete compile/design issue requires escalation:

    src/model/include/bim/model/element_id.hpp
    src/model/include/bim/model/level.hpp
    src/model/include/bim/model/straight_wall.hpp
    src/model/include/bim/model/wall_face_reference.hpp
    src/model/include/bim/model/validation.hpp

    src/model/src/element_id.cpp
    src/model/src/validation.cpp

Existing:

    src/model/CMakeLists.txt

may be modified only as needed to register these production sources.

Existing P0-T008 source/header files are frozen regression assets for this task.

## 15. Model Dependency Law

`bim_model` may continue using its already-approved dependencies.

P1-T001 MUST NOT add:

- OCCT;
- SQLite;
- Qt;
- bgfx;
- D3D;
- ODA;
- IfcOpenShell;
- another UUID library;
- a new package;
- `bim_dependency_graph`;
- `bim_document`;
- commands/query/persistence/desktop dependencies.

The model public surface must remain first-party/vendor-neutral.

## 16. Tests

Use Catch2 v3 + CTest.

Add focused production-domain tests.

Recommended targets:

    unit_model_element_id
    unit_model_domain_validation
    unit_model_wall_face_reference

Exact test source filenames may mirror those target names.

### 16.1 ElementId mandatory proof

Prove:

- default/all-zero invalid;
- equality;
- deterministic ordering;
- generated ElementId is valid/non-zero;
- generated value carries the frozen version/variant bit pattern;
- canonical output is exactly 36 lowercase characters with correct hyphens;
- round trip ElementId -> string -> ElementId;
- uppercase canonical input accepted;
- malformed length rejected;
- malformed separators rejected;
- invalid hex rejected;
- all-zero textual value rejected;
- parsing does not reject a valid non-zero assigned ID solely because it does not contain v4 generator bits.

Do not make probabilistic uniqueness across a sample the sole proof of generator correctness.

### 16.2 Level mandatory proof

Prove:

- valid zero elevation;
- valid positive elevation;
- valid negative elevation;
- invalid ElementId rejected;
- NaN/+Inf/-Inf rejected.

### 16.3 StraightWall mandatory proof

Prove:

- valid normal wall;
- ordered start/end preserved;
- swapped endpoints remain valid and semantically distinct;
- zero wall id rejected;
- zero Level id rejected;
- wall.id == level_id rejected;
- non-finite coordinates rejected;
- identical endpoints rejected;
- near-zero but nonzero axis is not silently rejected by hidden tolerance;
- thickness <= 0 rejected;
- non-finite thickness rejected;
- height <= 0 rejected;
- non-finite height rejected;
- positive/zero/negative finite base_offset accepted;
- non-finite base_offset rejected.

### 16.4 Wall face reference mandatory proof

Prove:

- exactly six valid named roles;
- out-of-range cast rejected;
- valid wall id + valid role accepted;
- zero wall id rejected;
- equality/value semantics behave deterministically.

## 17. Regression Requirements

Existing P0-T008 tests MUST remain unchanged and PASS:

    unit_model_face_reference_resolver
    integration_p0_t008_face_reference_spike

All previously accepted regression tests applicable to the standard project configuration MUST remain green.

Architecture tests MUST remain green, including rules protecting model public headers and kernel ownership.

## 18. Explicit Non-Goals

P1-T001 MUST NOT implement:

- `bim_document`;
- document element container;
- ElementId uniqueness enforcement across a document;
- ElementId <-> DependencyGraph::NodeId mapping;
- graph node registration;
- Level -> StraightWall graph edges;
- dirty propagation;
- recompute;
- OCCT wall generation;
- wall SolidHandle lifecycle;
- production face resolution against generated geometry;
- commands;
- query surfaces;
- transactions/journal integration;
- schema v2;
- save/reopen;
- SQLite changes;
- viewport mesh generation;
- tessellation;
- desktop/UI;
- selection/snapping/grips;
- joins/openings;
- materials/layers;
- non-straight walls;
- additional BIM element classes.

## 19. Prohibited Changes

Without a new Architecture Authority decision, implementation MUST NOT modify:

    vcpkg.json
    vcpkg-configuration.json
    CMakePresets.json

It MUST NOT introduce a dependency.

It MUST NOT modify geometry API contracts to make P1-T001 easier.

It MUST NOT modify P0-T008 contracts merely to unify naming.

It MUST NOT weaken architecture-checker rules.

It MUST NOT change P0-T010 architecture decisions.

## 20. Stop / Escalate Conditions

Stop and return to Architecture Authority if implementation appears to require:

- putting ElementId into foundation or dependency_graph;
- vendor/kernel/native identifier inside ElementId;
- using NodeId as durable identity;
- a third-party UUID library;
- hidden geometry tolerance;
- changing the Level -> StraightWall dependency semantics;
- changing P0-T008 historical contracts;
- changing geometry_api;
- persistence or schema changes;
- document/runtime ownership in P1-T001;
- a first-party dependency cycle;
- architecture-checker weakening;
- scope expansion beyond Level/StraightWall;
- any locked toolchain/dependency change.

Do not silently work around a stop condition.

## 21. Required Implementation Evidence

When implementation is later authorized, Claude must report:

1. exact starting HEAD/TREE;
2. exact files added/modified;
3. explicit confirmation of no unauthorized files;
4. implementation summary by contract area;
5. build result;
6. new unit-test results;
7. P0-T008 regression results;
8. applicable full regression result;
9. architecture-test result;
10. `git diff --check`;
11. final `git status --short`;
12. any unresolved concern or stop condition;
13. ACR assessment.

Claude MUST NOT commit, push, merge, rebase, clean the worktree or modify governance documents unless separately authorized.

## 22. Acceptance Criteria

P1-T001 implementation is acceptable only if all are true:

- production 128-bit ElementId exists;
- invalid-zero semantics are enforced;
- generator and canonical text conversion satisfy this Brief;
- Level domain contract exists;
- StraightWall authoritative parameter contract exists;
- ordered axis semantics are preserved;
- structural validation is fail-closed;
- no hidden tolerance is introduced;
- six production wall face roles exist with frozen semantics;
- production wall reference uses ElementId;
- P0-T008 remains unchanged as historical proof;
- no graph/persistence/UI/runtime implementation entered scope;
- no vendor identity leaks into model public API;
- new focused tests pass;
- P0-T008 regression passes unchanged;
- architecture enforcement passes;
- no unauthorized dependency/toolchain change exists;
- repository delta is confined to authorized implementation/test/build-registration paths.

## 23. Governance State

    Product Authority contract approval = APPROVED
    Architecture Authority resolution   = COMPLETE
    Implementation Brief                = ISSUED v1.0
    ACR                                 = NONE

    P1-T001 branch                      = CREATED BY CONTROLLED BOOTSTRAP
    P1-T001 worktree                    = CREATED BY CONTROLLED BOOTSTRAP
    Claude                              = NOT AUTHORIZED
    Kimi                                = NOT AUTHORIZED
    Production implementation           = NOT AUTHORIZED

This Implementation Brief defines the implementation contract.

It does not itself authorize implementation.
