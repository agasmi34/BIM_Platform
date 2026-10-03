# BIM Platform — P1-T001 Task Record

**Task:** P1-T001 — Core BIM Identity & Domain Model  
**Task branch:** `task/P1-T001-core-bim-identity-domain-model`  
**Worktree:** `D:\Projects\BIM-Platform-WT-P1-T001`  
**Phase:** Phase 1  
**Architecture Authority:** Product Authority + ChatGPT  
**Product Authority contract approval:** APPROVED  
**Implementation status:** NOT AUTHORIZED  
**ACR:** NONE  

## 1. Authoritative baseline

    HEAD = 1ec4958560835c241dfda5013ffaf7c556528d79
    TREE = 49f8e64465c29c72833ffdf10065dfec8646a861

This is the accepted, integrated, pushed and cleaned P0-T010 Phase-1 Architecture Gate baseline.

## 2. Objective

Introduce the first production BIM identity and domain contracts required by VS1 — Level-Constrained Straight Wall.

P1-T001 owns:

- durable `ElementId`;
- `Level`;
- model-owned `Point2D`;
- `StraightWall`;
- production `WallFaceRole`;
- production `WallFaceReference`;
- structural model validation;
- ElementId generation and canonical textual conversion.

## 3. Explicitly out of scope

P1-T001 does not own:

- `bim_document`;
- document runtime state;
- ElementId <-> DependencyGraph::NodeId association;
- dependency graph registration or recompute;
- persistence/schema v2;
- commands;
- query surfaces;
- transactions/journal integration;
- geometry generation;
- tessellation;
- viewport integration;
- desktop/UI;
- IFC/DWG/RVT implementation;
- non-VS1 BIM elements.

## 4. Governance lineage

- P1-T001 A1-R0 — Phase-1 Task Intake / Baseline Freeze: PASS / CLOSED.
- P1-T001 A2-R0 — Domain Contract Extraction: PASS / CLOSED.
- P1-T001 A3 — Architecture Authority Domain Contract Resolution: APPROVED / CLOSED.
- Product Authority approved the A3 domain contract.
- Implementation Brief v1.0 has been issued by Architecture Authority.
- P1-T001 production implementation is not authorized by this task record.

## 5. P0-T008 transition law

P0-T008 remains historical spike evidence.

The existing:

- `FeatureOwnerId`;
- `FaceRole`;
- `PersistentFaceReference`;
- `FeatureFrame`;
- `ResolveFaceReference(...)`;

remain unchanged unless a later explicit Architecture Authority decision says otherwise.

Production Phase-1 persistent references use durable `ElementId`.

## 6. Current lifecycle

    Architecture intake       = CLOSED / PASS
    Contract extraction       = CLOSED / PASS
    Domain contract resolution= APPROVED / CLOSED
    Product Authority approval= APPROVED
    Implementation Brief      = ISSUED v1.0
    Task branch               = CREATED by controlled bootstrap
    Task worktree             = CREATED by controlled bootstrap
    Implementation auth       = NOT AUTHORIZED
    Claude                    = NOT AUTHORIZED
    Kimi                      = NOT AUTHORIZED
    Commit                    = NOT AUTHORIZED
    Push                      = NOT AUTHORIZED
    ACR                       = NONE

No production implementation is authorized by this record.
