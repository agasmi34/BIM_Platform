# PROJECT-STATUS

**Scope:** BIM Platform engineering repository, current active task P0-T001.
**Maintained by:** Implementation Engineer (Claude), on instruction from
Architecture Authority. This file records status; it does not itself confer
approval or acceptance.

> **Core governance rule (Addendum A1):** Conversation is not the system of
> record. Git is the system of record. Everything in `docs/project-control/`
> and `docs/tasks/P0-T001/` is a **record of what happened and what was
> claimed or attested**, not an independent re-verification of it. Where a
> fact was reported or attested by a party other than Claude (the Windows
> Execution Operator, the Architecture Authority) and Claude has not itself
> seen the raw evidence for it (e.g. the literal content of a console
> transcript), that is stated explicitly next to the fact, including when
> the report has been formalized into an attestation with a stated
> provenance model. An attestation is a stronger record than a bare claim,
> but it is still not the same thing as Claude having independently
> observed the underlying evidence — this file preserves that distinction
> rather than collapsing it.

## Current task state

```
P0-T001 / PHASE C IMPLEMENTATION AUTHORED / WINDOWS VERIFICATION PENDING
```

(Windows verification here refers specifically to Verification Runbook B —
format/configure-build-test/architecture/license-inventory — which has not
been executed. See "Windows verification attestation" in
`TASK-LEDGER.md` for a separate, already-attested confirmation of the
worktree's file/branch/HEAD/commit state, which is not the same thing as
Runbook B evidence.)

## What this means concretely

| Item | State |
|---|---|
| Repository bootstrap (`D:\Projects\BIM-Platform`, `main`) | Complete: one empty commit `chore: initialize repository`, SHA `4b339248dd8b050e7b603ef0b5707440e582c315` (Bootstrap Runbook A v1.3). **Architecture Authority has reviewed the actual raw PowerShell console transcripts of the v1.2 and v1.3 runs (Windows Execution Operator / Product Authority); Claude itself has not been shown either transcript and has not reproduced either run.** See `docs/tasks/P0-T001/06-BOOTSTRAP-EVIDENCE.md`, "Bootstrap execution transcript provenance (correction)," for the exact provenance model. |
| Isolated task worktree (`D:\Projects\BIM-Platform-WT-P0-T001`, branch `task/P0-T001-repo-toolchain-scaffold`) | Created (Bootstrap Runbook A v1.3), same evidentiary caveat as above for its *creation*. Its *current* state (file count, branch, HEAD, tracked/untracked status) is separately **attested** by Architecture Authority as of this addendum's follow-up — see `TASK-LEDGER.md`, "Windows verification attestation." Claude still has not seen the transcript itself. |
| Phase C scaffold (63 files: root policy, cmake/, docs/, src/ modules, tests/, tools/, scripts/ci/, third_party/licenses/) | Authored by Claude, transferred into the worktree via the file bridge, transfer confirmed by the bridge tool itself (`written`, zero `rejected`) — this part **is** directly observed by Claude, not merely reported. |
| Addendum A1 traceability system (`docs/project-control/`, `docs/tasks/P0-T001/`, 18 files) | Authored by Claude, transferred the same way, zero rejected, and directly confirmed present via a device directory listing — **directly observed by Claude**. |
| Git commit of Phase C implementation | **NOT DONE.** Implementation Brief Phase O (scope audit) and Phase P (commit) are later, separately-authorized steps. All 81 worktree files are untracked — this is both what Claude expects (it never ran any git command) and what Architecture Authority's attestation independently states. |
| `main` (`D:\Projects\BIM-Platform`) | Untouched by any Phase C or addendum work. All addendum files were applied only under `D:\Projects\BIM-Platform-WT-P0-T001`. |
| Verification Runbook B (format / configure-build-test / architecture / license-inventory) | Authored and delivered to the operator. **NOT YET EXECUTED.** Explicitly not to be executed until Architecture Authority authorizes it. |
| Independent review (Kimi) | PENDING — not started. See `docs/tasks/P0-T001/08-KIMI-REVIEW.md`. Also the party expected to reproduce/independently confirm the Windows verification attestation below. |
| Architecture Disposition | PENDING. See `docs/tasks/P0-T001/09-ARCHITECTURE-DISPOSITION.md`. |
| Closure / Integration | PENDING. See `docs/tasks/P0-T001/11-CLOSURE.md`, `12-INTEGRATION-RECORD.md`. |

## Authority model reference

See `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 1 and
`docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md` section 2 for the
full authority model (Product Authority, Architecture Authority, Implementation
Engineer, Independent Reviewer). This file is a status snapshot; the two
documents above remain the actual authority.
