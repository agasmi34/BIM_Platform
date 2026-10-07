# P1-T003 Implementation Authorization

ID: BIM-AUTH-P1-T003
Status: PENDING

## Architecture status

P1-T003 architecture is APPROVED / FROZEN.

Architecture conflict: NONE.
ACR: NONE.

## Candidate scope

Implementation may eventually modify only the exact 18 paths frozen in
P1-T003-IB.

The four governance paths under docs/tasks/P1-T003 are separate governance
material and are not implementation paths.

## Current authorization

Implementation authorization: NOT GRANTED.

Claude authorization: NOT GRANTED.
Kimi authorization: NOT GRANTED.

Source modification: NOT AUTHORIZED.
Test modification: NOT AUTHORIZED.
Architecture-checker modification: NOT AUTHORIZED.

Staging: NOT AUTHORIZED.
Commit: NOT AUTHORIZED.
Integration: NOT AUTHORIZED.
Push: NOT AUTHORIZED.
Cleanup: NOT AUTHORIZED.

## Activation condition

This authorization may become active only after the Architecture Authority
verifies:

1. the four governance files exist exactly;
2. their bytes/hashes are frozen;
3. no implementation path changed during governance materialization;
4. task HEAD/TREE remain the frozen baseline;
5. canonical main remains unchanged;
6. the task worktree has only the four expected untracked governance files;
7. a separate explicit implementation authorization gate is issued.

No text in this document is self-authorizing.
