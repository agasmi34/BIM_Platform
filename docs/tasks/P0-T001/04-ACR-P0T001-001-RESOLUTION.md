# 04 — ACR-P0T001-001 Resolution

> Same provenance note as `03-ACR-P0T001-001.md`: this is a factual summary
> written from this session's record, not a byte-verbatim reproduction of
> the original resolution message.

## Decision

**Option B approved:** controlled operator execution. Claude authors
PowerShell runbooks/scripts; the Windows Execution Operator (Product
Authority, acting in that capacity) executes them; raw console output and
exit codes remain authoritative evidence; Kimi's role remains
review-only (must not implement or repair).

## Constraints established by the resolution

- Bootstrap Runbook A only for the initial bootstrap step; it must be safe
  to stop on error, must not install any dependency, and must not create
  any implementation/scaffold file (bootstrap is Git-repository setup only).
- Repository path: `D:\Projects\BIM-Platform`.
- Task worktree path: `D:\Projects\BIM-Platform-WT-P0-T001`.
- All other conditions of the ACR resolution remain those of the standard
  execution model already established (Implementation Brief section 2,
  Architecture Gate sections 1 and 17).

## Effect

Work paused under ACR-P0T001-001 resumed under this resolution: Bootstrap
Runbook A authoring began (see `05-BOOTSTRAP-RUNBOOK-A-v1.3.md` for its
full revision history).
