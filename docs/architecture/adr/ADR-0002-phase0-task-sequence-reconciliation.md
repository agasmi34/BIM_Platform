ADR-0002 — Phase 0 Task Sequence Reconciliation

Status: ACCEPTED
Date: 2026-09-02
Deciding authority: Product Owner + ChatGPT, Architecture Authority
Scope: Phase 0 task sequencing only

Context

The project accumulated two historical Phase 0 task-numbering descriptions.

An older planning sequence represented Dependency & License Baseline as
P0-T002.

The accepted P0-T001 Architecture Gate subsequently defined the operative
sequence with:

P0-T002 = OCCT Geometry Spike

and continued the downstream Phase 0 tasks from that numbering.

P0-T001 also materially delivered the dependency/license-baseline capability:

vcpkg manifest mode;
frozen builtin registry baseline;
OCCT 8.0.1;
SQLite;
Catch2 v3;
fmt;
spdlog;
direct dependency license inventory;
dependency/version verification.

Leaving both sequences unresolved would create task identity ambiguity in Git,
reviews and future Architecture Gates.

Decision

The operative Phase 0 sequence is:

TaskPurpose
P0-T001Repository & Toolchain Scaffold
P0-T002OCCT Geometry Spike
P0-T003Desktop + Viewport Spike
P0-T004Persistence Spike
P0-T005IFC Spike
P0-T006DWG / ODA Evaluation
P0-T007RVT / BimRv Evaluation
P0-T008Topological Reference Spike
P0-T009Dependency Graph Spike
P0-T010Phase 1 Architecture Gate

The former standalone Dependency & License Baseline task is treated as
materially absorbed into P0-T001 rather than renumbering all accepted and
future task records again.

Constraints

This ADR:

changes Phase 0 sequencing only;
does not rewrite the Master Engineering Constitution;
does not change dependency versions;
does not relax the frozen vcpkg baseline;
does not alter P0-T001 accepted evidence;
does not authorize P0-T002 implementation by itself.

Dependency/version changes continue to require an explicit dependency update,
ACR or ADR as applicable.

Consequences

P0-T002 now unambiguously means:

OCCT Geometry Spike

P0-T003 through P0-T010 use the sequence recorded above.

All new project-control, task, review and evidence records must use this
sequence.

Relationship to P0-T001

P0-T001 is already:

ACCEPTED + CLOSED + INTEGRATED

Final P0-T001 integration-record commit on main:

8c1c38990f75d5b0122e90d85bb8757e83a553a1

This ADR does not modify P0-T001 implementation or closure history.