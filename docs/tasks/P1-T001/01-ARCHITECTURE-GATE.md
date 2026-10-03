# BIM Platform — P1-T001 Core BIM Identity & Domain Model Architecture Gate

**Document ID:** BIM-AG-P1-T001  
**Version:** 1.0  
**Task:** P1-T001 — Core BIM Identity & Domain Model  
**Architecture Authority:** Product Authority + ChatGPT  
**Product Authority decision:** APPROVED  
**Architecture status:** FROZEN / APPROVED  
**Implementation status:** NOT AUTHORIZED  
**ACR:** NONE  

## 1. Baseline

    HEAD = 1ec4958560835c241dfda5013ffaf7c556528d79
    TREE = 49f8e64465c29c72833ffdf10065dfec8646a861

No dependency or toolchain change is authorized.

## 2. P1-T001 scope

P1-T001 owns only:

- `ElementId`;
- `Level`;
- `Point2D`;
- `StraightWall`;
- `WallFaceRole`;
- `WallFaceReference`;
- structural model validation;
- ElementId generation and canonical text conversion.

No graph integration, persistence, commands, queries, geometry generation or UI is authorized.

## 3. ElementId ownership

`ElementId` belongs to `bim_model` / `bim::model`.

It does not belong to:

- `bim_foundation`;
- `bim_dependency_graph`;
- persistence;
- geometry;
- viewport;
- desktop;
- external interoperability modules.

`ElementId != DependencyGraph::NodeId`.

## 4. ElementId representation

Production `ElementId` is a project-owned 128-bit value represented as sixteen ordered bytes.

Required semantics:

- 16 bytes;
- all-zero invalid;
- equality comparable;
- deterministic lexicographic ordering;
- canonical byte order independent of host endianness;
- vendor-neutral;
- kernel-neutral;
- database-neutral;
- renderer-neutral.

A recommended representation is:

    std::array<std::uint8_t, 16>

No third-party UUID dependency is authorized.

## 5. ElementId generation

Generation is first-party and uses only the locked C++ standard-library toolchain.

Generated IDs use UUID-v4-style bit layout:

- version bits = version 4;
- variant bits = binary `10`.

Generation must not derive identity from:

- wall clock;
- PID;
- memory address;
- database rowid;
- OCCT identity;
- DependencyGraph NodeId;
- GPU identity;
- IFC GlobalId;
- DWG handle;
- RVT ElementId.

Generation failure is explicit and fail-closed.

No entropy-provider exception may escape the project-owned public generation function.

## 6. ElementId textual form

Canonical output:

    xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

Rules:

- 36 characters;
- lowercase hexadecimal output;
- hyphens at 8, 13, 18 and 23;
- parser accepts upper/lower hexadecimal;
- malformed length fails;
- malformed separators fail;
- invalid hex fails;
- all-zero text fails.

Parsing a non-zero assigned ElementId must not require UUID-v4 generator bits.

## 7. Level

Minimum authoritative state:

    ElementId id
    double elevation

Rules:

- id must be valid;
- elevation must be finite;
- positive, zero and negative elevation are allowed.

No Level name/label field is required in P1-T001.

## 8. Point2D

Model-owned plan-coordinate value:

    double x
    double y

It is first-party, kernel-neutral, persistence-neutral and UI-neutral.

## 9. StraightWall

Minimum authoritative state:

    ElementId id
    ElementId level_id
    Point2D start
    Point2D end
    double thickness
    double height
    double base_offset

The ordered wall axis is:

    start -> end

Start/end ordering is semantic and must not be silently normalized or swapped.

## 10. Wall placement semantics

Conceptually:

    base_z = referenced_level.elevation + base_offset
    top_z  = base_z + height

P1-T001 freezes the meaning only.

It does not perform Level lookup or generate wall geometry.

## 11. Wall frame

The production conceptual wall frame is:

    W = +Z
    U = normalize(end - start)
    V = W × U

Therefore:

    U × V = W

The axis is the wall centerline.

Thickness semantics:

    SideA = -V * thickness / 2
    SideB = +V * thickness / 2

No kernel geometry is created in P1-T001.

## 12. Wall face roles

Exactly six production roles exist for VS1:

    Bottom
    Top
    Start
    End
    SideA
    SideB

Semantic meanings:

    Bottom = minimum W plane / wall base
    Top    = maximum W plane / wall top
    Start  = minimum U plane / start endpoint
    End    = maximum U plane / end endpoint
    SideA  = minimum V plane
    SideB  = maximum V plane

Enumeration order is never identity.

No seventh role is authorized.

## 13. Production wall face reference

Production wall semantic reference consists of:

    ElementId wall_id
    WallFaceRole role

It contains no:

- raw kernel handle;
- pointer;
- address;
- transient face ordinal;
- topology traversal index;
- geometry-enumeration position.

## 14. Structural validation

P1-T001 validation is structural only.

No hidden geometric tolerance is authorized.

### Level validity

Reject:

- invalid/all-zero id;
- NaN elevation;
- positive or negative infinity.

### StraightWall validity

Require:

- valid wall id;
- valid Level id;
- `wall.id != level_id`;
- all plan coordinates finite;
- start and end not exactly identical;
- finite thickness > 0;
- finite height > 0;
- finite base_offset.

Negative base offset is valid.

A very short but nonzero axis remains structurally valid.

Tolerance-aware geometric qualification belongs to later document/runtime/geometry work.

## 15. P0-T008 transition

P0-T008 remains unchanged historical spike evidence.

Do not:

- alias `FeatureOwnerId` to `ElementId`;
- widen `FeatureOwnerId`;
- rename historical `FaceRole` into production roles;
- rewrite P0-T008 tests to simulate Phase-1 production API.

Historical geometric correspondence:

    Bottom  <-> StartCap
    Top     <-> EndCap
    Start   <-> UMinSide
    End     <-> UMaxSide
    SideA   <-> VMinSide
    SideB   <-> VMaxSide

This is conceptual evidence only.

## 16. Dependency law

P1-T001 must not introduce dependencies on:

- `bim_dependency_graph`;
- `bim_document`;
- persistence;
- transactions;
- commands;
- query;
- desktop;
- viewport;
- OCCT;
- SQLite;
- Qt;
- bgfx;
- D3D;
- ODA;
- IfcOpenShell;
- another UUID library.

Existing approved `bim_model` dependencies may remain.

## 17. Stop conditions

Return to Architecture Authority if implementation appears to require:

- ElementId in foundation or dependency_graph;
- NodeId as durable identity;
- vendor/kernel/native identity in ElementId;
- third-party UUID dependency;
- hidden geometry tolerance;
- P0-T008 contract changes;
- geometry_api changes;
- persistence/schema changes;
- document/runtime ownership in P1-T001;
- dependency cycle;
- architecture-checker weakening;
- scope expansion beyond VS1 Level/StraightWall.

## 18. Lifecycle

    Architecture contract = APPROVED / FROZEN
    Product Authority     = APPROVED
    ACR                   = NONE

    Implementation        = NOT AUTHORIZED
    Claude                = NOT AUTHORIZED
    Kimi                  = NOT AUTHORIZED
    Commit                = NOT AUTHORIZED
    Push                  = NOT AUTHORIZED

This Architecture Gate does not itself authorize production implementation.
