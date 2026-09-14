# TASK-LEDGER

Chronological ledger of tasks and task-relevant events for the BIM Platform
engineering repository. Each entry states what happened, who reported it,
and whether Claude directly observed the evidence or is recording a
claim/report from another party. See PROJECT-STATUS.md for the "not the
system of record" caveat that applies to every row below.

## P0-T001 — Repository & Toolchain Scaffold

| Seq | Event | Reported by | Claude's evidentiary basis |
|---|---|---|---|
| 1 | Architecture Gate `BIM-AG-P0-T001 v1.0` issued | Product Owner + ChatGPT (Architecture Authority) | Document supplied in chat; verbatim copy at `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md`, byte-identical to the upload (checked with `diff`). |
| 2 | Implementation Brief `BIM-TASK-P0-T001-CLAUDE v1.0` released | Product Authority approval + Architecture Authority | Document supplied in chat; verbatim copy at `docs/tasks/P0-T001_Implementation_Brief_Claude_v1.0.md`, byte-identical to the upload. |
| 3 | ACR-P0T001-001 raised: this session has no command-execution access to the target Windows machine (no such tool was available; only file list/stage/commit tools exist) | Claude | Directly observed — Claude verified tool availability itself before raising the ACR. See `03-ACR-P0T001-001.md`. |
| 4 | ACR-P0T001-001 resolved: Option B (controlled Windows Execution Operator execution) approved; repository/worktree paths set to `D:\Projects\BIM-Platform` / `D:\Projects\BIM-Platform-WT-P0-T001` | Architecture Authority (chat) | Reported via chat instruction; recorded as the operative decision. See `04-ACR-P0T001-001-RESOLUTION.md`. |
| 5 | Bootstrap Runbook A v1.0 authored and delivered | Claude | Directly authored; delivered via file transfer. |
| 6 | Architecture review round 1 on v1.0/identity/state-detection issues; v1.1 produced with `$LASTEXITCODE`-based `Invoke-Native`/`Invoke-Git`, unborn-HEAD normalization | Architecture Authority (review) / Claude (fix) | Review comments reported in chat; fixes directly authored by Claude. |
| 7 | Architecture review round 2 on v1.1 (STATE B classification depth, unenforced post-conditions); v1.2 produced with `rev-list --all --count`/`for-each-ref` checks and 9 `Assert-True` invariants | Architecture Authority (review) / Claude (fix) | Same basis as row 6. |
| 8 | v1.2 executed on the real Windows target; **execution defect**: PowerShell 5.1 promoted native stderr (from an intentionally-permitted `rev-parse --verify HEAD` probe) into a terminating `ErrorRecord` under `$ErrorActionPreference='Stop'`, aborting mid-Step-6 before the bootstrap commit | Windows Execution Operator (Architecture-Authority-reviewed raw transcript — see row 18 correction) | Claude diagnosed the described symptom as a real, independently-plausible PowerShell behavior and fixed it. Claude itself has not seen the transcript; Architecture Authority has, and states it is a raw PowerShell console transcript, not a narrated summary. **Corrected by row 18** — do not read this row's older "reported only" phrasing as implying Architecture Authority lacked execution evidence. |
| 9 | v1.3 produced: `$ErrorActionPreference` scoped to `'Continue'` only around the native call inside `Invoke-Native`, restored via `finally`; `$LASTEXITCODE` sole authority; `--quiet` added to HEAD probes; verified pure-ASCII | Claude | Directly authored and self-checked (ASCII decode check run in the cloud sandbox). |
| 10 | v1.3 executed successfully: one bootstrap commit `chore: initialize repository` on `main`, SHA `4b339248dd8b050e7b603ef0b5707440e582c315`; task branch `task/P0-T001-repo-toolchain-scaffold` created; isolated worktree `D:\Projects\BIM-Platform-WT-P0-T001` created; git version reported as `2.47.1.windows.2` | Windows Execution Operator / Architecture Authority (Architecture-Authority-reviewed raw transcript — see row 18 correction) | See `06-BOOTSTRAP-EVIDENCE.md` for the full record. **Corrected by row 18** — do not read this row's older "reported only" phrasing as implying Architecture Authority lacked execution evidence. |
| 11 | Phase B accepted; Phase C (scaffold) authorized to resume | Architecture Authority (chat) | Reported/instructed via chat. |
| 12 | vcpkg registry baseline for OCCT 8.0.1 researched (not recalled from training data, since the release postdates Claude's knowledge cutoff) and resolved to commit `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843` | Claude | Directly observed — resolved via the GitHub commits API and the commit's own diff, both fetched live by Claude in this session. |
| 13 | Phase C scaffold (63 files) authored, self-verified where possible without Windows/MSVC/vcpkg (architecture checker run for real; CMake graph configure-tested up to the expected OCCT `find_package` boundary; SQLite probe compiled, linked, and run for real against sandbox system SQLite), and transferred into `D:\Projects\BIM-Platform-WT-P0-T001` | Claude | Directly observed — all verification and the transfer's `written`/`rejected` result were produced by tools Claude called directly in this session. |
| 14 | Verification Runbook B authored and delivered; not yet executed | Claude | Directly authored. Execution status: PENDING. |
| 15 | Architecture Addendum A1 (project-record & traceability system) issued, including a claim that Architecture Authority "independently verified" the 63-file transfer/branch/HEAD on the Windows machine | Architecture Authority (chat) | The addendum instruction itself is a valid chat instruction. **The "independently verified" claim is a report Claude cannot corroborate** — the figures cited (63 files, branch name, HEAD SHA) match what Claude itself had already stated in the prior turn, so this message does not constitute new external confirmation from Claude's point of view. Recorded here as a claim, not as independent verification. |
| 16 | `docs/project-control/` and `docs/tasks/P0-T001/` authored per Addendum A1 and transferred into the worktree only; no commit made | Claude | Directly observed by Claude (this session's own actions). |
| 17 | "P0-T001 A1 — WINDOWS VERIFICATION CONFIRMED" issued: Architecture Authority states it now holds a raw PowerShell console transcript from the Windows Execution Operator, and gives Claude a specific provenance model for it (below) rather than a bare restatement of figures | Architecture Authority (chat) | The chat instruction itself is directly observed. **The transcript's contents are not** — Claude has not been shown the transcript. See "Windows verification attestation" below for the exact facts attested and their scope. |
| 18 | "P0-T001 Architecture Review — Verification Runbook B: REVISION REQUIRED" issued: Architecture Authority states it possesses and has reviewed the actual raw PowerShell console transcripts of the v1.2 and v1.3 bootstrap runs (rows 8 and 10), supplied by the Windows Execution Operator / Product Authority — correcting this ledger's prior "reported only, narrated" labeling of those two rows, which Architecture Authority states understated the evidence it held. Runbook B also sent back for revision (BLOCKER/MAJOR findings); Verification Runbook B v1.1 authored in response. | Architecture Authority (chat) | The chat instruction and the correction it directs are directly observed and applied. **The transcripts themselves are still not seen by Claude** — see "Bootstrap execution transcript provenance (correction)" in `06-BOOTSTRAP-EVIDENCE.md` for the exact provenance model, which is deliberately not collapsed into "Claude verified." The raw v1.2/v1.3 transcript files do not yet exist in this worktree; Architecture Authority states they will be supplied separately for preservation under `docs/evidence/P0-T001/`. |

## Windows verification attestation

This section records the attestation from row 17 above, using the exact
provenance model Architecture Authority specified. It is an **attestation**,
not Claude's own verification: Claude has not seen the underlying raw
transcript and has not re-run any command against the Windows target.

**Facts attested** (stated by Architecture Authority; not independently
rerun by Claude):

- Total worktree file count: 81
- `docs/project-control/` contains exactly the 5 required project-control files
- `docs/tasks/P0-T001/` contains exactly the 13 required task records
- Branch: `task/P0-T001-repo-toolchain-scaffold`
- HEAD: `4b339248dd8b050e7b603ef0b5707440e582c315`
- Implementation/documentation files remain untracked
- No implementation commit exists

**Provenance model** (as specified by Architecture Authority):

| Field | Value |
|---|---|
| Evidence source | Windows Execution Operator / Product Authority |
| Evidence form | Raw PowerShell console transcript |
| Reviewed by Architecture Authority | YES |
| Claude independently reran evidence | NO — Windows command channel unavailable |
| Independent reviewer reproduction | PENDING — Kimi |

**Scope note:** These attested facts concern the worktree's *current*
file/branch/HEAD/tracked-status only — the same facts Claude had already
computed from its own transfer records (81 files = 63 Phase C + 18
Addendum A1 files delivered by Claude itself in this session). This
attestation does **not** extend to, and does not upgrade the evidentiary
status of, the separate Bootstrap Runbook A v1.2/v1.3 execution narrative
in row 8-10 above and in `06-BOOTSTRAP-EVIDENCE.md`, which remains
reported-only pending its own raw transcript. No transcript content beyond
the seven attested facts listed above has been recorded anywhere in this
repository; none has been invented.

## Next expected entries (not yet occurred)

- Verification Runbook B executed by the Windows Execution Operator; raw transcript returned.
- Phase O scope audit performed.
- Phase P implementation commit (`P0-T001: scaffold repository and toolchain`) made by the operator.
- `docs/evidence/P0-T001/CLAUDE_HANDOVER.md` / `.json` produced by Claude.
- Kimi independent review.
- Architecture Disposition and, if applicable, Architecture Closure.

P0-T001 final completion

The historical P0-T001 task subsequently completed the events that older
snapshots in this ledger previously listed as pending.

Final authoritative lineage:

EventResult
Phase P implementation commit37b9ab44adb6edf72e172dd6d3382464d7357a61
Kimi independent reviewPASS — zero unresolved BLOCKER/MAJOR
AC-013PASS
AC-014PASS
Architecture Authority closure8c84152c82a4c76e5c5c1d01c4cbd97f94784942
AC-001 through AC-015PASS
Controlled fast-forward integrationPASS
Integration record commit8c1c38990f75d5b0122e90d85bb8757e83a553a1
Final task stateACCEPTED + CLOSED + INTEGRATED

The original historical rows above remain preserved rather than rewritten;
this section records the later completed outcome.

P0-T002 — OCCT Geometry Spike
SeqEventAuthority / sourceState
1P0-T002 architecture preflight against integrated mainArchitecture Authority + Windows Execution OperatorPASS
2Approved base fixed at 8c1c38990f75d5b0122e90d85bb8757e83a553a1Architecture AuthorityLOCKED
3ADR-0002 Phase 0 Task Sequence ReconciliationArchitecture AuthorityACCEPTED
4BIM-AG-P0-T002 v1.0Product AuthorityAPPROVED
5Branch task/P0-T002-occt-geometry-spike createdWindows Execution OperatorPASS
6Worktree D:\Projects\BIM-Platform-WT-P0-T002 createdWindows Execution OperatorPASS
7P0-T002 implementationArchitecture AuthorityNOT RELEASED

No production-code implementation has occurred at this point.

## P0-T002 implementation release

| Event | Result |
|---|---|
| Architecture materialization commit | `ee0f47ceb5d29f995635594cbb33ee3dd82f6e96` |
| Architecture tree | `8ba4c7dfe4d626ced52821b753fe3d4af285af2a` |
| Canonical Gate SHA256 | `6F3157A3A19115D54FAAA93B13AAE22D13FA87299538A698D99DB2A1FEE66949` |
| ADR-0002 SHA256 | `1E23045B1A855AAD34F8D124629836CA050E71EBCD0C40BE29DF3F080BA91694` |
| Claude Implementation Brief | `BIM-TASK-P0-T002-CLAUDE v1.0` |
| Implementation Brief SHA256 | `7958C0BE210E4EBFFD137EA102069B6332CE4BD26BA1D825FE8310CFB9DE0D64` |
| Implementation state | RELEASED FOR IMPLEMENTATION |

Architecture Authority has released implementation only within the locked
P0-T002 OCCT Geometry Spike scope.

This release does not assert implementation completion, verification PASS,
independent-review PASS, acceptance, closure or integration.

## P0-T002 Phase A+B acceptance and Amendment 01

| Event | Result |
|---|---|
| Parent release commit | `751c5832eb8ed759d13b035d011999e8e5c7becc` |
| Parent Implementation Brief | `BIM-TASK-P0-T002-CLAUDE v1.0` |
| Parent Brief SHA256 | `7958C0BE210E4EBFFD137EA102069B6332CE4BD26BA1D825FE8310CFB9DE0D64` |
| Phase A + B | ACCEPTED |
| ACR | NOT REQUIRED |
| Amendment | `BIM-TASK-P0-T002-CLAUDE-A01 v1.0` |
| Amendment SHA256 | `544EFB2664FAE9B37F4F94F3EB0536F38DC2B4FBA141F55A3DF8F3C07CE53CDE` |
| Phase C-M | NOT YET AUTHORIZED |

The exact authorized implementation manifest is contained in Amendment 01.

No production implementation is part of this record.

## P0-T002 Phase C-M implementation authorization

| Event | Result |
|---|---|
| Amendment parent | 6081a30e03fb11d35d4920c3f7d35c253595270c |
| Phase A+B | ACCEPTED |
| OCCT 8.0.1 API verification | ACCEPTED |
| ACR | NONE |
| Implementation Authorization | BIM-TASK-P0-T002-IMPLEMENTATION-AUTH v1.0 |
| Authorization SHA256 | 2D987978D21B9343B90A61B6BF8B94678773FF6CBD63626FAC3C9BC3596C001E |
| Phase C-M | AUTHORIZED |

Claude implementation is limited to the controlled writable transport
snapshot.

No live-Git implementation, Windows verification, independent review,
acceptance or integration is asserted by this authorization.

## P0-T003 - Desktop + Viewport Spike

Architecture Gate:

BIM-AG-P0-T003 v1.0

Status:

ARCHITECTURE GATE APPROVED + LOCKED

Authoritative parent:

3f2230be5fcd796c370f485975547112ad52d2e3

Branch:

task/P0-T003-desktop-viewport-spike

Worktree:

D:\Projects\BIM-Platform-WT-P0-T003

Technology direction:

Qt 6 + Qt Widgets desktop shell

bgfx viewport-abstraction candidate

D3D11 authoritative Windows Phase-0 backend

Independent architecture consultation:

PASS

BLOCKER = 0

MAJOR = 0

Implementation:

NOT AUTHORIZED

Next:

Phase A read-only dependency/capability probe, then Phase B Contract Design
Check.

## 2026-09-08 - P0-T003 Implementation Brief Release

Brief:

BIM-TASK-P0-T003-CLAUDE v1.0

Architecture Gate:

BIM-AG-P0-T003 v1.0

Gate commit:

cf7a971903d8103143b2b94e94a4d185d86ce42b

Phase A:

ACCEPTED

Phase B:

ACCEPTED

ACR:

NONE

Exact implementation footprint:

59 exact implementation/verification/evidence paths. No path is conditional.

Status:

BRIEF RELEASED + LOCKED

Production implementation:

NOT AUTHORIZED

Next:

controlled C0 VCPKG_ROOT/toolchain execution preflight and explicit
implementation authorization.

## 2026-09-08 - P0-T003 Production Implementation Authorization

Authorization:

BIM-TASK-P0-T003-IMPLEMENTATION-AUTH v1.0

Implementation Brief:

BIM-TASK-P0-T003-CLAUDE v1.0

C0 toolchain execution preflight:

PASS

C0 ACR:

NONE

Implementation Engineer:

Claude

Authorized worktree:

D:\Projects\BIM-Platform-WT-P0-T003

Authorized footprint:

59 paths exactly

Main:

MUST REMAIN UNTOUCHED

Git staging/commit by Claude:

FORBIDDEN

Status:

PRODUCTION IMPLEMENTATION AUTHORIZED

Next:

Claude implements the locked P0-T003 candidate and returns the required
handover to Architecture Authority.

## 2026-09-14 - P0-T003 Acceptance, Integration and Closure

| Event | Result |
|---|---|
| Implementation commit | `5bac905e29c1390c90cc6807a5d92ab217764396` |
| Integrated tree | `3121a74e2e5edf960adfe5f36d0684541c805a1a` |
| Integration method | FAST-FORWARD |
| Full Verification Runbook D | CLOSED PASS |
| Authoritative Runbook D attempt | ATTEMPT 5 |
| Kimi BLOCKER | 0 |
| Kimi MAJOR | 0 |
| Kimi MINOR | 1 |
| Kimi NOTE | 3 |
| Task worktree | REMOVED |
| Local task branch | DELETED |
| ACR | NONE |

P0-T003 is accepted, closed and integrated locally on `main`.

The four non-blocking independent-review findings remain deferred follow-up
items and are not silently folded into P0-T004.

## P0-T004 - Persistence Spike

Architecture Gate:

BIM-AG-P0-T004 v1.0

Status:

ARCHITECTURE GATE APPROVED + LOCKED

Date:

2026-09-14

Authoritative parent:

5bac905e29c1390c90cc6807a5d92ab217764396

Authoritative parent tree:

3121a74e2e5edf960adfe5f36d0684541c805a1a

Branch:

task/P0-T004-persistence-spike

Worktree:

D:\Projects\BIM-Platform-WT-P0-T004

Discovery:

PASS

Discovery classification:

READY_FOR_ARCHITECTURE_GATE_DRAFT

Architecture direction:

- SQLite remains private to `bim_persistence`;
- schema version authority is `PRAGMA user_version`;
- schema version 1 contains only the Phase-0 journal persistence needed by
  this spike;
- `bim_transactions` owns the neutral journal contract;
- persistence proves atomic append, close/reopen durability, rollback,
  duplicate-ID rejection, binary payload integrity and deterministic order;
- R12 enforces SQLite sole ownership;
- R13 enforces persistence public-header neutrality;
- frozen vcpkg baseline remains unchanged.

ACR:

NONE

Implementation:

NOT AUTHORIZED

Next:

Prepare the P0-T004 Implementation Brief, perform Architecture Authority
review, and issue a separate implementation authorization before any
production source implementation.
