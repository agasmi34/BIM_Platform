# BIM Platform - P0-T008 Topological Reference Spike Architecture Gate

**Gate:** BIM-AG-P0-T008 v1.0
**Task:** P0-T008 - Topological Reference Spike
**Gate status:** FROZEN / APPROVED
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude - NOT AUTHORIZED
**Independent Reviewer:** Kimi - NOT AUTHORIZED
**Architecture Change Record:** NONE

## 1. Authority and lifecycle state

P0-T008 is the Phase-0 persistent-reference architecture spike required by AG-018 before Phase-1 BIM feature implementation may begin.

This Gate does not authorize implementation.

At materialization time:

```text
Phase A bootstrap                = CLOSED / PASS
Phase B1 authority extraction    = CLOSED / PASS
Phase B2 architecture contract   = AA FROZEN
Phase B3 schema discovery        = CLOSED / PASS
Architecture Gate               = FROZEN / APPROVED
Product Authority approval      = APPROVED
Implementation Brief            = NOT RELEASED
Implementation Authorization    = NOT ISSUED
Claude                           = NOT AUTHORIZED
Kimi                             = NOT AUTHORIZED
Candidate commit                = NOT AUTHORIZED
Main integration                = NOT AUTHORIZED
Push                             = NOT AUTHORIZED
ACR                              = NONE
```

## 2. Objective

Prove that a project-owned semantic face reference can survive controlled geometry regeneration when the intended semantics remain valid, without depending on raw OCCT topology identity.

The proof must also demonstrate explicit failure when continuity is missing or ambiguous.

Returning an arbitrary face is not acceptable.

## 3. Existing authority and inherited boundary

P0-T001 and P0-T002 already establish that public BIM identity must not use raw OCCT `TopoDS_*` identity.

P0-T002 further establishes that OCCT Generated/Modified/IsDeleted history and transient input-face ordinals are diagnostic evidence, not persistent BIM identity.

P0-T008 owns the persistent-reference architecture that was explicitly deferred by P0-T002.

AG-018 remains locked: Phase 1 waits for the persistent-reference and dependency-graph spikes.

## 4. Repository and toolchain baseline

The Gate is based on:

- HEAD `385e07b2a7306664d1063281ab150657dadcb154`;
- TREE `fe52cdb590b4aea0137d4561ea847d59a4987e99`;
- branch `task/P0-T008-topological-reference-spike`;
- worktree `D:\Projects\BIM-Platform-WT-P0-T008`.

The locked Windows x64, C++20, VS2022, Ninja, CMake, vcpkg and OCCT 8.0.1 baseline remains unchanged.

P0-T008 requires no dependency-version change.

## 5. Frozen scope

P0-T008 shall prove only the minimum persistent-reference architecture required for a semantic face reference.

In scope:

- project-owned reference value semantics;
- project-owned narrow owner identity/token for the spike;
- semantic face role/path;
- neutral face observations required for resolution;
- deterministic reference resolution;
- same-spec regeneration;
- controlled parameter/dimension changes;
- translation/coordinate changes;
- repeat regeneration;
- selected Boolean-cut continuity;
- explicit split/ambiguity behavior;
- explicit deleted/missing behavior;
- invalid-owner/reference behavior;
- architecture enforcement and evidence.

Persistent Edge and Vertex references are not required.

## 6. Explicit non-goals

P0-T008 does not authorize:

- production Wall, Door, Window, Slab or Room implementation;
- final platform-wide BIM UUID architecture;
- dependency-graph behavior;
- dirty-node propagation or recompute scheduling;
- project-schema persistence changes;
- undo/redo changes;
- IFC reference integration;
- DWG reference integration;
- RVT/BimRv reference integration;
- UI or viewport feature work;
- collaboration/cloud behavior;
- a general CAD topological naming framework;
- new third-party dependencies.

Dependency-graph behavior remains P0-T009.

## 7. Persistent-reference ownership law

Persistent semantic references belong above the geometry kernel.

Semantic naming and resolution are owned by `bim_model`.

Kernel-specific geometric/topological extraction is owned by `bim_geometry_occt`.

The public bridge is `bim_geometry_api`, which must remain project-owned and OCCT-free.

Intended dependency direction:

```text
bim_model
    -> bim_geometry_api
    -> bim_foundation

bim_geometry_occt
    -> bim_geometry_api
    -> bim_foundation
    -> OCCT
```

`bim_model` must not link OCCT directly.

## 8. Durable identity prohibition

A persistent semantic reference MUST NOT use any of the following as its durable identity:

- `TopoDS_Shape`, `TopoDS_Face` or `TopoDS_Edge` identity;
- OCCT object address;
- OCCT handle identity;
- `TopExp` traversal position;
- raw face/edge enumeration index;
- P0-T002 `input_face_ordinal`;
- raw ordering of Generated/Modified results;
- face-count or edge-count values.

These values may be diagnostic evidence but cannot define durable identity.

## 9. Conceptual persistent-reference contract

A P0-T008 face reference conceptually contains:

```text
owner identity/token
semantic role/path
topology kind = Face
optional project-owned disambiguation evidence
```

Exact C++ type and enum names belong to a later Implementation Brief.

The P0-T008 owner token is intentionally narrow and does not finalize the platform-wide BIM UUID contract.

## 10. Semantic face-role corpus

The deterministic extrusion corpus must support semantic roles equivalent to:

```text
start cap
end cap
U-min side
U-max side
V-min side
V-max side
```

The contract describes semantics, not kernel face numbering.

Exact C++ identifiers remain an implementation-brief decision.

## 11. Resolution semantics

Resolution must be fail-closed and machine-classifiable.

Conceptual outcomes include:

```text
Resolved
Missing
Ambiguous
InvalidReference / InvalidOwner
```

A resolver must never silently choose a candidate when semantic continuity is ambiguous.

Enumeration order is not an ambiguity resolver.

## 12. Neutral geometry observation boundary

`bim_geometry_api` may expose only project-owned neutral observations needed by the semantic resolver.

Such observations may include neutral geometric facts such as orientation, plane/frame relationship, bounding information, centroid-like measurements, area-like measurements or other first-party values justified by the spike.

No OCCT type, handle or object identity may cross this boundary.

The Gate does not freeze the exact observation struct; that belongs to the Implementation Brief after approval.

## 13. OCCT history role

OCCT Generated, Modified and IsDeleted information may be used as supporting evidence or candidate-mapping assistance.

It must not become the persistent reference itself.

A reference must remain meaningful because of project-owned semantic rules, not because one OCCT object happens to map to another OCCT object during one run.

## 14. Mandatory proof corpus

The later implementation proof must include at least the following cases.

### A. Same-spec regeneration

A semantic reference resolves to the same semantic face after rebuilding equivalent geometry.

### B. Dimension change

The semantic reference survives when dimensions change but the intended role still exists.

### C. Translation / coordinate change

The semantic role remains stable independent of absolute location and raw topology enumeration.

### D. Repeat regeneration

Repeated runs produce the same resolution classification.

### E. Boolean cut

Unaffected semantic faces remain resolvable where semantic continuity still exists.

### F. Split-face ambiguity

If one previous semantic face becomes multiple equally plausible faces and the semantic contract cannot uniquely identify one, the result is Ambiguous.

The resolver must not choose by enumeration order.

### G. Deleted face

If the intended semantic face no longer exists, the result is Missing.

### H. Invalid owner/reference

An invalid owner or malformed reference produces explicit project-owned failure classification.

## 15. Persistence boundary

P0-T008 does not reopen P0-T004.

No native database/schema change is required.

The spike proves semantic continuity across controlled regeneration in memory.

Later serialization/persistence of the approved reference contract requires its own authority if not already covered by a future gate.

## 16. Dependency-graph boundary

P0-T008 does not implement:

- semantic DAG nodes;
- dependency edges;
- dirty propagation;
- recompute scheduling;
- dependency invalidation;
- graph cycle handling.

Those belong to P0-T009.

## 17. BIM-feature boundary

The proof corpus may use neutral wall-like or extrusion-like shapes.

It must not introduce production BIM Wall, Door, Window, Slab, Room, hosted element, family or parameter behavior.

## 18. Interoperability boundary

P0-T008 contains no IFC, DWG or RVT/BimRv persistent-reference integration.

Vendor handles and IDs from those formats are not promoted into the P0-T008 persistent reference.

## 19. Architecture enforcement requirements

A later Implementation Brief must require mechanical proof that:

```text
model public API exposes no OCCT types
geometry public API exposes no OCCT types
bim_model does not link OCCT directly
persistent-reference public types expose no raw kernel handle
persistent-reference public types expose no transient face ordinal
semantic resolver remains project-owned
```

Negative fixtures should be used where appropriate.

## 20. Expected future implementation footprint

If Product Authority approves this Gate, the later Implementation Brief may authorize minimum-delta changes primarily within:

```text
src/model/**
src/geometry/api/**
src/geometry/occt/**
tests/unit/**
tests/integration/**
tests/architecture/**
tests/fixtures/**
docs/tasks/P0-T008/**
docs/evidence/P0-T008/**
docs/project-control/**
```

This is footprint guidance, not implementation authorization.

Changes to persistence, commands, transactions, query, viewport, desktop, interop adapters, dependencies or unrelated modules require Architecture Authority review and may require an ACR.

## 21. Acceptance criteria

P0-T008 may close only if all required evidence establishes that:

1. a project-owned semantic face reference exists without raw OCCT identity;
2. same-semantics regeneration resolves deterministically;
3. permitted geometric changes preserve references where semantics survive;
4. enumeration order is not part of identity;
5. ambiguous continuity returns Ambiguous;
6. deleted continuity returns Missing;
7. invalid references fail explicitly;
8. OCCT history remains supporting evidence only;
9. public APIs remain vendor-neutral;
10. model does not link OCCT directly;
11. architecture enforcement passes;
12. existing regression tests remain green;
13. dependency graph and Phase-1 BIM features remain absent;
14. no unauthorized dependency change occurs;
15. independent review has no unresolved BLOCKER or MAJOR finding;
16. Architecture Authority issues written closure.

A system that always returns some face does not satisfy this Gate.

## 22. Required evidence

Required later evidence includes:

- exact implementation baseline;
- exact candidate path set;
- clean locked toolchain build;
- full CTest regression;
- persistent-reference unit tests;
- OCCT-backed controlled regeneration tests;
- ambiguity and missing-reference proof;
- repeatability evidence;
- architecture enforcement;
- no raw OCCT leak;
- no dependency-graph implementation;
- no Phase-1 BIM feature implementation;
- repository cleanliness;
- independent review.

## 23. Stop conditions

Implementation must stop and return to Architecture Authority if it requires:

- raw OCCT identity in the durable public reference;
- persistent Edge/Vertex scope;
- a final platform-wide UUID architecture;
- persistence-schema changes;
- dependency-graph implementation;
- production BIM feature implementation;
- a new third-party dependency;
- interop-specific identity integration;
- module dependency reversal;
- an architecture boundary not frozen by this Gate.

Such a conflict must not be solved silently.

## 24. Architecture Change Record status

```text
ACR = NONE
```

This Gate is the planned P0-T008 architecture decision required by AG-018. It does not presently modify a previously locked dependency or product architecture contract.

A future conflict with an existing locked decision requires a separate ACR.

## 25. Governance lifecycle

The required lifecycle is:

```text
Architecture Gate draft materialization
    ->
Architecture Authority content/path review
    ->
Product Authority approval
    ->
governance commit
    ->
exact Gate HEAD/TREE capture
    ->
separate Implementation Brief
    ->
separate Implementation Authorization
    ->
Claude implementation
    ->
candidate validation
    ->
independent Kimi review
    ->
controlled integration
```

No later step is authorized by an earlier step.

## 26. Gate disposition at draft materialization

```text
P0-T008 Architecture Gate        = DRAFT
AA architecture contract         = FROZEN
Product Authority approval       = PENDING
Implementation Brief             = NOT RELEASED
Implementation Authorization     = NOT ISSUED
Claude                            = NOT AUTHORIZED
Kimi                              = NOT AUTHORIZED
Candidate commit                  = NOT AUTHORIZED
Main integration                  = NOT AUTHORIZED
Push                              = NOT AUTHORIZED
ACR                               = NONE
```

Product Authority approved this Gate on 2026-09-27. The approved Gate is not yet a committed governance baseline. Implementation remains unauthorized until a separate Implementation Brief is released and a separate Implementation Authorization is issued against a controlled baseline.
