# RISK-REGISTER

Real risks identified during P0-T001, stated plainly. This register does
not silently downgrade or resolve a risk by recording it; each row's
"status" reflects what is actually known as of this addendum.

| ID | Risk | Impact if realized | Status | Mitigation / next action |
|---|---|---|---|---|
| R-001 | No execution result received so far (bootstrap v1.2 defect, v1.3 success) has been a raw, verbatim console transcript — all have been narrated summaries | A real failure, or a partial/different actual state, could be misreported without Claude or the record being able to tell | **OPEN** | Every historical record in this addendum that depends on such a report says so explicitly. Verification Runbook B writes a full `Start-Transcript` log to `docs/evidence/P0-T001/`; that file, returned in full, is the first genuinely raw evidence this task will have produced. |
| R-002 | OCCT 8.0.1 (released 2026-07-30) postdates Claude's reliable knowledge cutoff; the vcpkg baseline SHA could not be recalled and had to be researched live | A wrong/fabricated SHA would silently fail to resolve OCCT 8.0.1, or resolve a different version | **MITIGATED, not yet field-verified** | SHA `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843` was verified via the GitHub commits API and the commit's own diff (not guessed). Still requires a real `vcpkg install`/CMake configure on Windows to become AC-007 evidence. |
| R-003 | `bim_persistence`'s dual-path SQLite CMake resolution (`SQLite::SQLite3` vs `unofficial::sqlite3::sqlite3`) has only been exercised against a Linux system SQLite3 in the cloud sandbox, not against the actual vcpkg `sqlite3` port on Windows | The `find_package` branch that actually fires on the real vcpkg triplet is unconfirmed; a `FATAL_ERROR` here would be a real (not architectural) blocker | **OPEN** | Verification Runbook B's `configure-build-test` job will surface this immediately if it occurs. |
| R-004 | This session has no command-execution tool for the Windows target (confirmed twice, including again this session) | Any "execution" evidence necessarily comes from a human operator, introducing the transcription/narration gap in R-001 | **STANDING / BY DESIGN** (ACR-P0T001-001 resolution, Option B) | Not something Claude can mitigate from within this session; inherent to the current tooling. |
| R-005 | `.clang-format`/`.clang-tidy` availability on the operator's machine is unverified | `scripts/ci/format.ps1` is designed to record this as an explicit operational blocker (Implementation Brief Phase K) rather than silently pass, but that design itself is untested against the real machine | **OPEN** | Resolved by Runbook B's `format` job output. |
| R-006 | `arch_repository_boundaries`'s checker is a blunt substring/token scanner, not a real C++ parser | Could produce a false negative (miss a violation phrased unusually) or a false positive (a comment happens to contain a forbidden token) | **KNOWN LIMITATION, ONE FALSE-POSITIVE ALREADY CAUGHT AND FIXED** | During authoring, a comment in `src/foundation/CMakeLists.txt` originally contained the literal text `target_link_libraries()`, which would have self-triggered rule R5; the comment was reworded before transfer. The checker's R5 rule also explicitly skips comment-only lines as a second layer of defense. No further false positives found in the self-verification pass, but the tool remains a heuristic, not a guarantee. |
| R-007 | Architecture Addendum A1 adds a `docs/` substructure not present in the canonical repository tree locked by Architecture Gate section 6 / Implementation Brief section 5 | Repository tree drift from the approved gate document, without a formally versioned gate amendment | **OPEN / LOW SEVERITY** | Recorded in `DECISION-LEDGER.md` under "Addenda issued since gate approval." Architecture Authority stated no new ADR is required; Claude has applied the addendum as instructed but flags the gap for the record rather than silently normalizing it. |

## Risks retired since earlier task phases

None. No risk row above has been marked resolved; several are contingent on
Verification Runbook B's actual execution, which has not happened yet.

## P0-T006 DWG / ODA controlled risks - 2026-09-22

| ID | Risk | Impact | State | Control |
|---|---|---|---|---|
| P0T6-R01 | ODA Drawings SDK is proprietary and external to Git | Build/test cannot assume SDK availability | CONTROLLED / OPEN | `BIM_ENABLE_DWG=OFF` by default; explicit `BIM_ODA_DRAWINGS_ROOT` required |
| P0T6-R02 | Trial evidence does not prove commercial redistribution rights | Commercial deployment could violate licensing if assumed | OPEN / BUSINESS-LEGAL GATE | No redistribution claim; production licensing established separately |
| P0T6-R03 | ODA write may materialize defaults and change handles/object population | Binary/object identity comparisons can produce false fidelity failures | MITIGATED | Semantic fidelity contract; handles/object count non-authoritative |
| P0T6-R04 | ODA handles are unstable across write/reopen | Persistent identity corruption if reused as BIM IDs | MITIGATED | Handles explicitly prohibited as persistent platform identity |
| P0T6-R05 | External SDK/runtime packaging may differ between machines | Configure/runtime failure | CONTROLLED | Explicit root, accepted version, clear failure, no global fallback |

## P0-T007 RVT / BimRv controlled residual risks - 2026-09-26

| ID | Risk | Impact | State | Control |
|---|---|---|---|---|
| P0T7-R01 | ODA BimRv is proprietary/trial evaluation material; production redistribution/commercial rights were not established | Evaluation evidence cannot be treated as deployment authority | CONTROLLED / OPEN | Keep all ODA material external to Git; make no redistribution or production-license claim |
| P0T7-R02 | BimRv 27.7 package-specific maximum supported Revit version was not established | Unsupported version-range claims could be made | CONTROLLED / OPEN | Record runtime proof only for controlled Revit 2017 fixture; do not infer an upper bound |
| P0T7-R03 | RFA and RTE were not runtime-tested | Format-wide runtime claims would exceed evidence | CONTROLLED / OPEN | Classify RFA/RTE as documented but not runtime-proven |
| P0T7-R04 | Detailed geometry runtime proof covers one selected `SWall` in one public RVT fixture | Evidence does not prove arbitrary-element/model completeness | CONTROLLED / OPEN | Preserve explicit evidence boundary in Gate and capability matrix |

## P0-T008 Topological Reference controlled risks - 2026-09-27

| ID | Risk | Impact | State | Control |
|---|---|---|---|---|
| P0T8-R01 | Raw OCCT subshape identity changes during regeneration | Persistent references could silently retarget | CONTROLLED / OPEN | Durable identity is semantic and project-owned; raw TopoDS identity is prohibited |
| P0T8-R02 | Face enumeration order changes while geometry remains semantically equivalent | Ordinal-based references could target the wrong face | CONTROLLED / OPEN | Enumeration order is explicitly non-authoritative |
| P0T8-R03 | One semantic face becomes multiple plausible faces after an operation | Resolver could choose an arbitrary face | CONTROLLED / OPEN | Required outcome is Ambiguous unless uniqueness is proven |
| P0T8-R04 | Referenced semantic face disappears | Stale reference could be misinterpreted | CONTROLLED / OPEN | Required outcome is Missing; no silent fallback |
| P0T8-R05 | P0-T008 grows into dependency-graph or BIM-feature implementation | Phase-0 architecture boundary would be bypassed | CONTROLLED | DAG work remains P0-T009; Phase-1 features remain blocked |
| P0T8-R06 | Spike owner token is mistaken for final platform-wide BIM UUID architecture | Temporary proof contract could ossify prematurely | CONTROLLED / OPEN | Owner identity is narrow P0-T008 proof scope only unless later AA decision expands it |
