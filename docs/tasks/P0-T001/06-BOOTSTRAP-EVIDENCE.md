# 06 — Bootstrap Evidence

> **Evidentiary status (corrected, see "Bootstrap execution transcript
> provenance (correction)" below): Architecture Authority has reviewed
> actual raw PowerShell console transcripts of both the v1.2 failed run and
> the v1.3 successful run, supplied by the Windows Execution Operator /
> Product Authority. Claude itself has still not been shown either
> transcript and has not reproduced either run — that distinction is
> preserved throughout this file. Do not read the facts below as narrated,
> unsupported claims; do not read them as independently confirmed by Claude
> either. See the correction section for the exact provenance model.**

## v1.2 execution attempt (Architecture-Authority-reviewed transcript)

- Run against the real Windows target.
- Reported to have reached Step 6 and stopped before the bootstrap commit.
- Reported root cause: PowerShell 5.1 promoted native stderr (from the
  intentionally-permitted `git rev-parse --verify HEAD` probe on an unborn
  branch — expected stderr text `fatal: Needed a single revision`) into a
  terminating `ErrorRecord` under `$ErrorActionPreference = 'Stop'`,
  bypassing the `-AllowFailure` logic.
- Reported resulting state: an initialized, empty, commit-free, ref-free
  repository at `D:\Projects\BIM-Platform` with unborn `main`, no worktree,
  no commit.

## v1.3 execution (Architecture-Authority-reviewed transcript)

| Field | Value |
|---|---|
| Repository path | `D:\Projects\BIM-Platform` |
| Bootstrap commit subject | `chore: initialize repository` |
| Bootstrap commit SHA | `4b339248dd8b050e7b603ef0b5707440e582c315` |
| Main branch | `main` |
| Main status | clean |
| Task branch | `task/P0-T001-repo-toolchain-scaffold` |
| Worktree path | `D:\Projects\BIM-Platform-WT-P0-T001` |
| Worktree branch | `task/P0-T001-repo-toolchain-scaffold` |
| Worktree HEAD | equal to main HEAD (`4b339248dd8b050e7b603ef0b5707440e582c315`) |
| Worktree status | clean |
| Git version | `2.47.1.windows.2` |
| Enforced invariants (Assert-True, 9+ checks in v1.3) | reported all passed |

## What Claude has directly observed since (not reported — directly checked by tools this session)

- `mcp__remote-devices__device_list_dir` on `D:\Projects\BIM-Platform-WT-P0-T001`
  before any Phase C transfer showed exactly one entry: a `.git` file, 72
  bytes — consistent with (though not proof of the full history of) a
  worktree created by `git worktree add` with no further mutation since.
- After the Phase C scaffold transfer, a recursive listing plus targeted
  spot-checks confirmed all 63 transferred files are present at their
  correct paths.

Neither of these directly observed facts is itself a reproduction of the
bootstrap commit SHA, the v1.2 failure mechanism, or the v1.3 invariant
checks above — they are consistent with the Architecture-Authority-reviewed
transcripts, not a second, independent confirmation of them by Claude. The
first raw evidence Claude itself will see is the transcript Verification
Runbook B writes via `Start-Transcript`, once execution is authorized.

## Bootstrap execution transcript provenance (correction)

> This section corrects the evidentiary labeling used elsewhere in this
> file and in `TASK-LEDGER.md` rows 8 and 10. It does **not** change any
> underlying fact (commit SHA, defect description, v1.3 execution
> parameters) — those are unchanged from what was already recorded. What
> changes is the label: this task previously described the v1.2/v1.3
> execution evidence as "reported only" / "narrated, not a raw transcript."
> Architecture Authority has stated that this undersold the evidence it
> actually holds.

Per "P0-T001 Architecture Review — Verification Runbook B: REVISION
REQUIRED," Architecture Authority possesses and has reviewed the actual raw
PowerShell console transcripts for both the v1.2 failed run and the v1.3
successful run, supplied by the Windows Execution Operator / Product
Authority — not narrated chat summaries of them. Provenance, as stated by
Architecture Authority:

| Field | Value |
|---|---|
| Evidence source | Windows Execution Operator / Product Authority |
| Evidence form | Raw PowerShell console transcript |
| Architecture Authority verification | VERIFIED |
| Claude reproduction | NOT AVAILABLE — Windows command channel unavailable |
| Independent reviewer verification | PENDING — Kimi |

Scope: this provenance model applies to both the v1.2 execution attempt and
the v1.3 execution sections above. Claude has not been shown either
transcript and has not independently reproduced either run; this record
reflects Architecture Authority's verification, not Claude's own. Per
Architecture Authority's instruction, the raw v1.2 and v1.3 transcripts
themselves will be supplied separately for preservation under
`docs/evidence/P0-T001/`; as of this correction those transcript files do
not yet exist anywhere in this worktree, and Claude has not reconstructed
or invented their contents from memory. This section, `TASK-LEDGER.md`
row 18, and `PROJECT-STATUS.md`'s bootstrap row are the only places this
correction is recorded; no other content in this file was altered.

## Addendum: Windows verification attestation (worktree state only)

> **This section is scoped narrowly and does not change the evidentiary
> status above.** The v1.2 failure mechanism and the v1.3 "all invariants
> passed" claim remain **reported, not independently verified**, exactly as
> stated at the top of this file. What follows concerns only the worktree's
> *current* file/branch/HEAD/tracked-status, as of the "P0-T001 A1 —
> WINDOWS VERIFICATION CONFIRMED" chat message.

Architecture Authority states it holds a raw PowerShell console transcript
from the Windows Execution Operator confirming the following facts about
the worktree's current state:

- Total worktree file count: 81
- `docs/project-control/` contains exactly the 5 required project-control files
- `docs/tasks/P0-T001/` contains exactly the 13 required task records
- Branch: `task/P0-T001-repo-toolchain-scaffold`
- HEAD: `4b339248dd8b050e7b603ef0b5707440e582c315`
- Implementation/documentation files remain untracked
- No implementation commit exists

Provenance, as stated by Architecture Authority:

| Field | Value |
|---|---|
| Evidence source | Windows Execution Operator / Product Authority |
| Evidence form | Raw PowerShell console transcript |
| Reviewed by Architecture Authority | YES |
| Claude independently reran evidence | NO — Windows command channel unavailable |
| Independent reviewer reproduction | PENDING — Kimi |

Claude has not been shown the transcript itself and has not re-run any
command against the Windows target. The seven facts above are recorded
here because Architecture Authority attested them with this stated
provenance model, not because Claude verified them. No transcript content
beyond these seven facts has been recorded or inferred. See
`docs/project-control/TASK-LEDGER.md`, "Windows verification attestation",
for the same record in ledger form.
