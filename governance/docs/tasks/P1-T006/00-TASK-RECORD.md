# P1-T006 Task Record

Task: P1-T006 — Desktop VS1 End-to-End Acceptance

Status: ARCHITECTURE / FOOTPRINT FROZEN — IMPLEMENTATION NOT YET AUTHORIZED

Architecture Gate: BIM-AG-P1-T006

Baseline commit:
09b6c1d4706b295041093e0c5c151983c7f2f57d

Baseline tree:
9d58caf1ef1c7036ca27f8e0addb9c5155c47b08

Task branch:
task/P1-T006-desktop-vs1-e2e-acceptance

Task worktree:
D:\Projects\BIM-Platform-WT-P1-T006

A1:
PASS / CLOSED

A2:
APPROVED / FROZEN

A3:
PASS / CLOSED

Frozen implementation footprint:
8 modified + 9 new + 0 deleted = 17 implementation paths.

Footprint manifest SHA256:
1CD748B871D404931B8C9D0DA1B50DDB6853AC28F4BFEC779A0251FE90216747

Objective:
Complete the Phase-1 VS1 end-to-end vertical slice through the production desktop boundary:
user workflow -> commands/query/document -> derived wall geometry -> presentation mesh -> viewport.

P1-T006 also closes the previously deferred production WallFaceReference resolution requirement using durable ElementId + WallFaceRole and neutral geometry observations.

Hard constraints:
- production mutations go through bim::commands;
- query remains read-only;
- Save/Open remain document lifecycle operations;
- desktop rendering uses bim::presentation -> RenderMeshData -> viewport;
- no desktop-to-OCCT rendering path;
- no direct UI model mutation;
- no durable NodeId, GPU handle, kernel handle or transient face index;
- P0-T008 historical reference implementation remains unchanged;
- no new dependency or package;
- no persistence schema change;
- no threading architecture change.

Expected final test registration:
- full CTest: 90
- architecture CTest: 44
- focused P1-T006 runtime: 2
- focused P1-T006 architecture: 6

ACR:
NONE
