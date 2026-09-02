# 05 — Bootstrap Runbook A (approved v1.3, verbatim)

## Revision history

| Version | Change | Trigger |
|---|---|---|
| v1.0 | First draft: `--global`-scoped identity check; try/catch around native `git` calls for failure detection; basic STATE B classification (existence + `.git` + `rev-parse --verify HEAD`) | Initial authoring |
| v1.1 | Identity check switched to `git config --get` (no forced scope, catches system/global/includeIf); `$LASTEXITCODE`-based `Invoke-Native`/`Invoke-Git` helpers replace try/catch for native exit codes; STATE B classification adds a check for stray content beyond `.git`; unborn-HEAD normalization to `main` applied unconditionally (Step 5b); safety wording corrected to acknowledge partial state is possible after a failure | Architecture review round 1 |
| v1.2 | STATE B classification hardened: `git rev-list --all --count == 0` and `git for-each-ref` emptiness added (a non-resolving HEAD alone does not prove zero commits/refs elsewhere); `Assert-True` introduced and applied to 9 concrete post-condition invariants (previously some were printed but not enforced); `.git` confirmed to be a genuine directory via `Get-Item -Force` / `.PSIsContainer` (not a gitlink-style file) | Architecture review round 2 |
| **v1.3 (approved, reproduced in full below)** | Fixes a real Windows PowerShell 5.1 execution defect: native stderr captured via `2>&1` under `$ErrorActionPreference = 'Stop'` could be promoted to a terminating `ErrorRecord` before `$LASTEXITCODE` could be checked, defeating an intentionally-permitted `-AllowFailure` probe (`rev-parse --verify HEAD` on an unborn branch) and aborting mid-Step-6. Fixed by scoping `$ErrorActionPreference = 'Continue'` around only the native invocation inside `Invoke-Native`, restored via `finally`; `$LASTEXITCODE` is the sole authority. Also added `--quiet` to HEAD probes and hardened `Get-DiagnosticState` so each diagnostic probe is independently try/catch-wrapped. Verified pure-ASCII. | Reported execution defect against v1.2 on the real Windows target (reported via chat, narrated — see `docs/project-control/TASK-LEDGER.md` row 8; not a raw transcript) |

## Reported outcome of v1.3 execution

Reported (not independently verified by Claude — see
`06-BOOTSTRAP-EVIDENCE.md`): bootstrap commit `chore: initialize
repository`, SHA `4b339248dd8b050e7b603ef0b5707440e582c315`, on `main`;
task branch `task/P0-T001-repo-toolchain-scaffold` created from that commit;
isolated worktree `D:\Projects\BIM-Platform-WT-P0-T001` created; all 9+
enforced `Assert-True` invariants reported as having passed.

## Verbatim script content (v1.3, as delivered and, per report, executed)

```powershell
<#
==============================================================================
 P0-T001 - Bootstrap Runbook A - v1.3
 Classification : Operator-executed / Claude-authored runbook
 Revision       : v1.3 - fixes an execution defect reported against v1.2 on
                  real Windows PowerShell 5.1.

 Reported defect: Invoke-Native ran native commands (via 2>&1 redirection)
 while $ErrorActionPreference = 'Stop' was in force at global scope. On
 Windows PowerShell 5.1, native stderr captured this way can be surfaced
 as a PowerShell ErrorRecord that becomes a TERMINATING error under
 $ErrorActionPreference = 'Stop', before the helper's own $LASTEXITCODE
 check ever runs - defeating -AllowFailure for an intentionally-permitted
 non-zero probe such as 'git rev-parse --verify HEAD' on an unborn branch
 (which prints "fatal: Needed a single revision" to stderr and exits 1).
 This caused the v1.2 run to stop at Step 6 with an unhandled exception
 instead of reaching the -AllowFailure branch, leaving: an initialized,
 empty, ref-free, commit-free repository at D:\Projects\BIM-Platform,
 unborn HEAD already normalized to refs/heads/main, and no worktree.

 Fix: Invoke-Native now scopes $ErrorActionPreference = 'Continue' ONLY
 around the native invocation itself, restores the previous value in a
 finally block, and decides success/failure EXCLUSIVELY from
 $LASTEXITCODE - never from try/catch around the native call. This is
 the same mechanism used everywhere in this script; nothing else in the
 execution model changed.

 Repository path: D:\Projects\BIM-Platform
 Worktree path  : D:\Projects\BIM-Platform-WT-P0-T001
 Branch         : task/P0-T001-repo-toolchain-scaffold

 This revision is written to SAFELY RESUME from the exact controlled
 state reported after the v1.2 failure (existing .git; no user content;
 zero commits; zero refs; unborn HEAD -> refs/heads/main; no task
 worktree). It does this the same way v1.2 already did: Step 3's STATE B
 classification treats an initialized-but-empty, content-free,
 commit-free, ref-free repository as a valid reuse case and Step 5
 skips 'git init' accordingly; Step 5b only rewrites the unborn ref if
 it is not already refs/heads/main, so a repo already on 'main' is a
 no-op there. Nothing in this revision deletes, recreates, or resets
 the repository - v1.3 only fixes how native command results are
 observed.

 All v1.2 invariants are unchanged and still enforced:
   - '.git' must be a genuine directory (not a file/other indirection).
   - No content beyond '.git' when reusing an existing repository.
   - Zero reachable commits (git rev-list --all --count == 0) and zero
     refs under refs/heads, refs/tags, refs/remotes, in addition to an
     unresolved HEAD, before a repository is accepted as STATE B.
   - Effective Git identity validated via 'git config --get user.name'
     / 'user.email' (no scope forced), checked both before any repo
     exists and again from inside the repository after init.
   - Unborn HEAD normalized to refs/heads/main strictly before any
     commit is made.
   - Exactly one empty bootstrap commit; asserted afterward: reachable
     commit count == 1, commit subject exactly matches (case-sensitive),
     the commit's tree is genuinely empty (git ls-tree -r --name-only
     HEAD is blank), branch is exactly 'main', working tree is clean.
   - Isolated sibling worktree created on the task branch; asserted
     afterward: worktree branch exactly matches, worktree HEAD equals
     main's HEAD, worktree status is clean, and finally that main's
     branch/HEAD/status are unchanged by the worktree/branch operations.
   - Every asserted post-condition uses Assert-True (Fail on mismatch);
     nothing is "print only".

 New in v1.3:
   - Invoke-Native: native invocation wrapped with a locally-scoped
     $ErrorActionPreference = 'Continue', restored via finally; success/
     failure decided exclusively by $LASTEXITCODE.
   - HEAD-existence probes now use 'rev-parse --verify --quiet HEAD'
     (suppresses the expected "fatal: needed a single revision" stderr
     text for the known-non-error unborn-branch case, on top of the
     Invoke-Native fix - belt and suspenders, not a substitute for it).
   - Get-DiagnosticState rewritten to route every probe through
     Invoke-Git -AllowFailure, each wrapped in its own try/catch, so one
     failing diagnostic probe cannot abort the rest of the diagnostic
     dump.
   - Script content kept strictly ASCII (no em dashes, no curly quotes,
     no non-breaking spaces) for Windows PowerShell 5.1 safety.

 Safety (unchanged from v1.2):
   - Every precondition is checked BEFORE any filesystem or Git
     mutation, so a precondition failure is guaranteed non-mutating.
   - Once mutation begins, a failure does NOT roll back or delete
     anything. The script stops immediately, prints a best-effort
     diagnostic dump, and requires manual inspection before re-running.
   - Exits non-zero on any abort; exits 0 only on full, verified success.

 Usage (regular PowerShell, no admin rights required):
   PS> cd <anywhere>
   PS> .\Bootstrap-RunbookA-v1.3.ps1

 Return the ENTIRE console transcript verbatim, including any diagnostic
 dump, to the Implementation Engineer.
==============================================================================
#>

$ErrorActionPreference = 'Stop'

$RepoPath     = 'D:\Projects\BIM-Platform'
$ProjectsRoot = 'D:\Projects'
$WorktreePath = 'D:\Projects\BIM-Platform-WT-P0-T001'
$TaskBranch   = 'task/P0-T001-repo-toolchain-scaffold'
$BootstrapMsg = 'chore: initialize repository'

$script:MutationStarted = $false

function Write-Section {
    param([string]$Title)
    Write-Host ''
    Write-Host "=== $Title ===" -ForegroundColor Cyan
}

function Invoke-Native {
    param(
        [Parameter(Mandatory)][string]$Exe,
        [Parameter(Mandatory)][string[]]$CmdArgs,
        [switch]$AllowFailure
    )

    $stdoutLines = [System.Collections.Generic.List[string]]::new()
    $stderrLines = [System.Collections.Generic.List[string]]::new()

    # Fix for the v1.2 execution defect: native stderr captured via 2>&1
    # can be promoted to a TERMINATING error under a 'Stop' preference on
    # Windows PowerShell 5.1, before $LASTEXITCODE can be consulted. Scope
    # 'Continue' around only the native call itself, and restore the
    # caller's preference immediately afterward regardless of outcome.
    $previousEap = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Exe @CmdArgs 2>&1 | ForEach-Object {
            if ($_ -is [System.Management.Automation.ErrorRecord]) {
                $stderrLines.Add($_.ToString())
            } else {
                $stdoutLines.Add([string]$_)
            }
        }
        $code = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousEap
    }

    $result = [PSCustomObject]@{
        ExitCode = $code
        Stdout   = ($stdoutLines -join "`n")
        Stderr   = ($stderrLines -join "`n")
    }

    # Success/failure is decided EXCLUSIVELY from $LASTEXITCODE, never from
    # a try/catch around the native invocation.
    if ($code -ne 0 -and -not $AllowFailure) {
        Fail "Command failed (exit $code): $Exe $($CmdArgs -join ' ')`nSTDOUT:`n$($result.Stdout)`nSTDERR:`n$($result.Stderr)"
    }

    return $result
}

function Invoke-Git {
    param([Parameter(Mandatory)][string[]]$GitArgs, [switch]$AllowFailure)
    return Invoke-Native -Exe 'git' -CmdArgs $GitArgs -AllowFailure:$AllowFailure
}

function Get-DiagnosticState {
    Write-Host ''
    Write-Host '--- DIAGNOSTIC STATE AT FAILURE (best-effort; each probe is independent) ---' -ForegroundColor Yellow

    try { Write-Host "RepoPath exists      : $(Test-Path $RepoPath)" } catch { Write-Host "(RepoPath probe failed: $($_.Exception.Message))" }
    try { Write-Host "WorktreePath exists  : $(Test-Path $WorktreePath)" } catch { Write-Host "(WorktreePath probe failed: $($_.Exception.Message))" }

    $gitDirPresent = $false
    try { $gitDirPresent = Test-Path (Join-Path $RepoPath '.git') } catch { Write-Host "(.git probe failed: $($_.Exception.Message))" }

    if ($gitDirPresent) {
        $probes = @(
            @{ Label = 'status --short --branch';        Args = @('-C', $RepoPath, 'status', '--short', '--branch') },
            @{ Label = 'branch -a';                       Args = @('-C', $RepoPath, 'branch', '-a') },
            @{ Label = 'rev-list --all --count';          Args = @('-C', $RepoPath, 'rev-list', '--all', '--count') },
            @{ Label = 'for-each-ref';                    Args = @('-C', $RepoPath, 'for-each-ref') },
            @{ Label = 'worktree list';                   Args = @('-C', $RepoPath, 'worktree', 'list') },
            @{ Label = 'rev-parse --verify --quiet HEAD'; Args = @('-C', $RepoPath, 'rev-parse', '--verify', '--quiet', 'HEAD') }
        )
        foreach ($p in $probes) {
            Write-Host "-- git $($p.Args -join ' ') --"
            try {
                $r = Invoke-Git -GitArgs $p.Args -AllowFailure
                if (-not [string]::IsNullOrWhiteSpace($r.Stdout)) { Write-Host $r.Stdout }
                if ($r.ExitCode -ne 0) {
                    Write-Host "(exit code $($r.ExitCode))"
                    if (-not [string]::IsNullOrWhiteSpace($r.Stderr)) { Write-Host $r.Stderr }
                }
            } catch {
                Write-Host "(probe '$($p.Label)' failed unexpectedly: $($_.Exception.Message))"
            }
        }
    } else {
        Write-Host '(no .git directory present under RepoPath - nothing further to inspect)'
    }

    Write-Host '--- END DIAGNOSTIC STATE ---'
    Write-Host ''
}

function Fail {
    param([string]$Message)
    Write-Host ''
    Write-Host "BOOTSTRAP ABORTED: $Message" -ForegroundColor Red
    if ($script:MutationStarted) {
        Write-Host 'Mutation had already begun. State was NOT rolled back or deleted - inspect and resolve manually before re-running.' -ForegroundColor Red
        Get-DiagnosticState
    } else {
        Write-Host 'No mutation had occurred yet. Nothing on disk or in Git was changed by this run.' -ForegroundColor Red
    }
    exit 1
}

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { Fail $Message }
}

function Get-ReachableCommitCount {
    param([Parameter(Mandatory)][string]$Path)
    $res = Invoke-Git -GitArgs @('-C', $Path, 'rev-list', '--all', '--count')
    $countStr = $res.Stdout.Trim()
    $count = 0
    if (-not [int]::TryParse($countStr, [ref]$count)) {
        Fail "Could not parse commit count from 'git rev-list --all --count' output: '$countStr'"
    }
    return $count
}

function Get-AllRefs {
    param([Parameter(Mandatory)][string]$Path)
    $res = Invoke-Git -GitArgs @('-C', $Path, 'for-each-ref', '--format=%(refname)', 'refs/heads', 'refs/tags', 'refs/remotes')
    return $res.Stdout
}

try {

    # -------------------------------------------------------------------
    Write-Section 'Step 0 - Tool version'
    # -------------------------------------------------------------------
    $verRes = Invoke-Git -GitArgs @('--version')
    Write-Host $verRes.Stdout

    # -------------------------------------------------------------------
    Write-Section 'Step 1 - Git identity check (effective resolved value; verify only, no changes)'
    # -------------------------------------------------------------------
    $nameRes  = Invoke-Git -GitArgs @('config', '--get', 'user.name')  -AllowFailure
    $emailRes = Invoke-Git -GitArgs @('config', '--get', 'user.email') -AllowFailure
    $userName  = $nameRes.Stdout.Trim()
    $userEmail = $emailRes.Stdout.Trim()

    if ($nameRes.ExitCode -ne 0 -or $emailRes.ExitCode -ne 0 -or
        [string]::IsNullOrWhiteSpace($userName) -or [string]::IsNullOrWhiteSpace($userEmail)) {
        Fail @"
Effective Git identity does not resolve via 'git config --get user.name' / 'user.email'.
  user.name  exit=$($nameRes.ExitCode) value='$userName'
  user.email exit=$($emailRes.ExitCode) value='$userEmail'
This script will not invent or set an identity. Configure it yourself, e.g.:
  git config --global user.name  "Your Name"
  git config --global user.email "you@example.com"
(any scope is acceptable - global, system, or a conditional include), then re-run.
"@
    }
    Write-Host "user.name  = $userName"
    Write-Host "user.email = $userEmail"

    # -------------------------------------------------------------------
    Write-Section 'Step 2 - Target drive/path checks'
    # -------------------------------------------------------------------
    if (-not (Test-Path 'D:\')) {
        Fail "D:\ drive is not present on this machine. Cannot proceed with the approved path $RepoPath."
    }
    if (Test-Path $WorktreePath) {
        Fail "Worktree target '$WorktreePath' already exists. This script requires it to not exist yet. Resolve manually before re-running."
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 3 - Repository state classification (must be STATE B)'
    # -------------------------------------------------------------------
    $repoAlreadyInitialized = $false

    if (Test-Path $RepoPath) {
        if (-not (Get-Item $RepoPath).PSIsContainer) {
            Fail "'$RepoPath' exists and is a file, not a directory. Manual resolution required."
        }

        $gitDir = Join-Path $RepoPath '.git'

        if (Test-Path $gitDir) {

            # '.git' must be a genuine directory, not a gitlink/submodule-
            # style file or other indirection.
            $gitDirItem = Get-Item -Force -LiteralPath $gitDir
            if (-not $gitDirItem.PSIsContainer) {
                Fail "'$gitDir' exists but is not a directory (it is a $($gitDirItem.GetType().Name)). This may be a gitlink/submodule-style '.git' file or other unusual indirection. Treating as STATE C - do not proceed; raise this back as an ACR."
            }

            # No content beyond '.git' at the repository root.
            $entries = Get-ChildItem -Force -LiteralPath $RepoPath
            $others  = $entries | Where-Object { $_.Name -ne '.git' }
            if ($others.Count -gt 0) {
                Fail "'$RepoPath' contains a .git directory PLUS other entries: $($others.Name -join ', '). STATE B requires no content beyond '.git'. This is STATE C - do not proceed; raise this back as an ACR."
            }

            # HEAD must not resolve. --quiet suppresses the expected
            # "fatal: needed a single revision" stderr text for this
            # known-non-error case.
            $probe = Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', '--verify', '--quiet', 'HEAD') -AllowFailure
            if ($probe.ExitCode -eq 0) {
                Fail "'$RepoPath' already contains a Git repository WITH a resolvable HEAD ($($probe.Stdout.Trim())). This runbook only handles STATE B. This is STATE A or STATE C - do not proceed; raise this back as an ACR."
            }

            # Zero reachable commits from ANY ref, not just HEAD.
            $existingCommitCount = Get-ReachableCommitCount -Path $RepoPath
            if ($existingCommitCount -ne 0) {
                Fail "'$RepoPath' already has $existingCommitCount reachable commit(s) per 'git rev-list --all --count', even though HEAD itself doesn't resolve (commits reachable only via another branch/tag/remote-ref). STATE B requires exactly 0. This is STATE A or STATE C - do not proceed; raise this back as an ACR."
            }

            # No pre-existing refs at all.
            $existingRefs = Get-AllRefs -Path $RepoPath
            if (-not [string]::IsNullOrWhiteSpace($existingRefs)) {
                Fail "'$RepoPath' already has pre-existing refs (git for-each-ref refs/heads refs/tags refs/remotes):`n$existingRefs`nSTATE B requires none. This is STATE A or STATE C - do not proceed; raise this back as an ACR."
            }

            Write-Host "'$RepoPath' contains an initialized, ref-free, commit-free, content-free .git only. STATE B confirmed (reused) - resuming."
            $repoAlreadyInitialized = $true
        } else {
            $entries = Get-ChildItem -Force -LiteralPath $RepoPath
            if ($entries.Count -gt 0) {
                Fail "'$RepoPath' exists and is NOT empty (no .git found, but it contains: $($entries.Name -join ', ')). This is STATE C - do not proceed; raise this back as an ACR."
            }
            Write-Host "'$RepoPath' exists and is empty. STATE B confirmed."
        }
    } else {
        Write-Host "'$RepoPath' does not exist yet. STATE B confirmed (will be created)."
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 4 - Create repository directory (idempotent; does not touch existing content)'
    # -------------------------------------------------------------------
    $script:MutationStarted = $true
    New-Item -ItemType Directory -Force -Path $ProjectsRoot | Out-Null
    New-Item -ItemType Directory -Force -Path $RepoPath     | Out-Null
    Write-Host "Repository directory ready: $RepoPath"

    # -------------------------------------------------------------------
    Write-Section 'Step 5 - git init (only if not already initialized)'
    # -------------------------------------------------------------------
    if (-not $repoAlreadyInitialized) {
        Invoke-Git -GitArgs @('init', $RepoPath) | Out-Null
        Write-Host 'Initialized empty repository.'
    } else {
        Write-Host 'Reusing existing initialized-but-empty repository. Skipping git init.'
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 5b - Normalize unborn HEAD to refs/heads/main (before any commit)'
    # -------------------------------------------------------------------
    $refRes = Invoke-Git -GitArgs @('-C', $RepoPath, 'symbolic-ref', 'HEAD')
    $currentRef = $refRes.Stdout.Trim()
    Write-Host "Current unborn HEAD ref: $currentRef"

    if ($currentRef -ne 'refs/heads/main') {
        Invoke-Git -GitArgs @('-C', $RepoPath, 'symbolic-ref', 'HEAD', 'refs/heads/main') | Out-Null
        Write-Host "Normalized HEAD ref to refs/heads/main (was: $currentRef)."
    } else {
        Write-Host 'HEAD ref already refs/heads/main. No change needed.'
    }

    # -------------------------------------------------------------------
    Write-Section 'Step 5c - Re-confirm identity from inside the repository'
    # -------------------------------------------------------------------
    # Repeats Step 1 now that a local config layer exists, to also catch a
    # gitdir-conditional ('includeIf') override that only resolves once
    # inside this specific repository path.
    $inRepoNameRes  = Invoke-Git -GitArgs @('-C', $RepoPath, 'config', '--get', 'user.name')  -AllowFailure
    $inRepoEmailRes = Invoke-Git -GitArgs @('-C', $RepoPath, 'config', '--get', 'user.email') -AllowFailure
    $inRepoName  = $inRepoNameRes.Stdout.Trim()
    $inRepoEmail = $inRepoEmailRes.Stdout.Trim()

    if ($inRepoNameRes.ExitCode -ne 0 -or $inRepoEmailRes.ExitCode -ne 0 -or
        [string]::IsNullOrWhiteSpace($inRepoName) -or [string]::IsNullOrWhiteSpace($inRepoEmail)) {
        Fail "Git identity does not resolve from inside '$RepoPath' (user.name='$inRepoName' exit=$($inRepoNameRes.ExitCode), user.email='$inRepoEmail' exit=$($inRepoEmailRes.ExitCode)), even though it resolved at Step 1. No commit was made. Configure identity for this path and re-run."
    }
    Write-Host "In-repo identity confirmed: $inRepoName <$inRepoEmail>"

    # -------------------------------------------------------------------
    Write-Section 'Step 6 - Empty bootstrap commit'
    # -------------------------------------------------------------------
    $headProbe = Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', '--verify', '--quiet', 'HEAD') -AllowFailure
    if ($headProbe.ExitCode -eq 0) {
        Fail "HEAD already resolves to a commit ($($headProbe.Stdout.Trim())) even though Step 3 classified this as STATE B. Aborting rather than double-committing - manual inspection required."
    }
    $preCommitCount = Get-ReachableCommitCount -Path $RepoPath
    Assert-True ($preCommitCount -eq 0) "Reachable commit count is $preCommitCount immediately before commit, expected 0. Aborting rather than double-committing."

    Invoke-Git -GitArgs @('-C', $RepoPath, 'commit', '--allow-empty', '-m', $BootstrapMsg) | Out-Null
    Write-Host "Created bootstrap commit: '$BootstrapMsg'"

    # -------------------------------------------------------------------
    Write-Section 'Step 7 - Main branch evidence AND enforced invariants'
    # -------------------------------------------------------------------
    $mainHead   = (Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', 'HEAD')).Stdout.Trim()
    $mainBranch = (Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', '--abbrev-ref', 'HEAD')).Stdout.Trim()
    $mainStatus = (Invoke-Git -GitArgs @('-C', $RepoPath, 'status', '--short')).Stdout
    $mainCommitCount = Get-ReachableCommitCount -Path $RepoPath
    $mainSubject = (Invoke-Git -GitArgs @('-C', $RepoPath, 'log', '-1', '--format=%s')).Stdout.Trim()
    $mainTreeFiles = (Invoke-Git -GitArgs @('-C', $RepoPath, 'ls-tree', '-r', '--name-only', 'HEAD')).Stdout

    Write-Host "Main path          : $RepoPath"
    Write-Host "Main branch        : $mainBranch"
    Write-Host "Main HEAD          : $mainHead"
    Write-Host "Main commit count  : $mainCommitCount"
    Write-Host "Main commit subject: $mainSubject"
    Write-Host 'Main status        :'
    if ([string]::IsNullOrWhiteSpace($mainStatus)) { Write-Host '(clean)' } else { Write-Host $mainStatus }

    Assert-True ($mainCommitCount -eq 1) "Expected exactly 1 reachable commit on main after bootstrap, got $mainCommitCount."
    Assert-True ($mainSubject -ceq $BootstrapMsg) "Expected bootstrap commit subject exactly '$BootstrapMsg', got '$mainSubject'."
    Assert-True ([string]::IsNullOrWhiteSpace($mainTreeFiles)) "Expected the bootstrap commit's tree to be genuinely empty (no files), but 'git ls-tree -r --name-only HEAD' returned:`n$mainTreeFiles"
    Assert-True ($mainBranch -eq 'main') "Expected current branch to be 'main' but got '$mainBranch'."
    Assert-True ([string]::IsNullOrWhiteSpace($mainStatus)) "Expected main working tree to be clean after the bootstrap commit, but 'git status --short' returned:`n$mainStatus"

    # -------------------------------------------------------------------
    Write-Section 'Step 8 - Create task branch'
    # -------------------------------------------------------------------
    $branchListRes = Invoke-Git -GitArgs @('-C', $RepoPath, 'branch', '--list', $TaskBranch)
    if (-not [string]::IsNullOrWhiteSpace($branchListRes.Stdout)) {
        Fail "Branch '$TaskBranch' already exists. Aborting - ambiguous prior state."
    }
    Invoke-Git -GitArgs @('-C', $RepoPath, 'branch', $TaskBranch) | Out-Null
    Write-Host "Created branch '$TaskBranch' from $mainHead"

    # -------------------------------------------------------------------
    Write-Section 'Step 9 - Create isolated sibling worktree'
    # -------------------------------------------------------------------
    Invoke-Git -GitArgs @('-C', $RepoPath, 'worktree', 'add', $WorktreePath, $TaskBranch) | Out-Null
    Write-Host "Created worktree at '$WorktreePath' on branch '$TaskBranch'"

    # -------------------------------------------------------------------
    Write-Section 'Step 10 - Task worktree evidence AND enforced invariants'
    # -------------------------------------------------------------------
    $wtHead   = (Invoke-Git -GitArgs @('-C', $WorktreePath, 'rev-parse', 'HEAD')).Stdout.Trim()
    $wtBranch = (Invoke-Git -GitArgs @('-C', $WorktreePath, 'rev-parse', '--abbrev-ref', 'HEAD')).Stdout.Trim()
    $wtStatus = (Invoke-Git -GitArgs @('-C', $WorktreePath, 'status', '--short')).Stdout

    Write-Host "Worktree path   : $WorktreePath"
    Write-Host "Worktree branch : $wtBranch"
    Write-Host "Worktree HEAD   : $wtHead"
    Write-Host 'Worktree status :'
    if ([string]::IsNullOrWhiteSpace($wtStatus)) { Write-Host '(clean)' } else { Write-Host $wtStatus }

    Assert-True ($wtBranch -eq $TaskBranch) "Expected worktree branch exactly '$TaskBranch', got '$wtBranch'."
    Assert-True ($wtHead -eq $mainHead) "Expected worktree HEAD ($wtHead) to equal main/bootstrap HEAD ($mainHead) - they diverged unexpectedly."
    Assert-True ([string]::IsNullOrWhiteSpace($wtStatus)) "Expected worktree status clean, but 'git status --short' returned:`n$wtStatus"

    # -------------------------------------------------------------------
    Write-Section 'Step 11 - git worktree list'
    # -------------------------------------------------------------------
    $wtListRes = Invoke-Git -GitArgs @('-C', $RepoPath, 'worktree', 'list')
    Write-Host $wtListRes.Stdout

    # -------------------------------------------------------------------
    Write-Section 'Step 12 - Final main status AND enforced invariants (main untouched)'
    # -------------------------------------------------------------------
    $mainBranch2 = (Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', '--abbrev-ref', 'HEAD')).Stdout.Trim()
    $mainHead2   = (Invoke-Git -GitArgs @('-C', $RepoPath, 'rev-parse', 'HEAD')).Stdout.Trim()
    $mainStatus2 = (Invoke-Git -GitArgs @('-C', $RepoPath, 'status', '--short')).Stdout

    Write-Host "Main branch (post) : $mainBranch2"
    Write-Host "Main HEAD   (post) : $mainHead2"
    Write-Host 'Main status (post) :'
    if ([string]::IsNullOrWhiteSpace($mainStatus2)) { Write-Host '(clean)' } else { Write-Host $mainStatus2 }

    Assert-True ([string]::IsNullOrWhiteSpace($mainStatus2)) "Expected final main working tree clean, but 'git status --short' returned:`n$mainStatus2"
    Assert-True ($mainBranch2 -eq 'main') "Expected final main branch 'main', got '$mainBranch2'."
    Assert-True ($mainHead2 -eq $mainHead) "Main HEAD changed after worktree/branch operations (was $mainHead, now $mainHead2) - main must be untouched."

    # -------------------------------------------------------------------
    Write-Section 'EVIDENCE SUMMARY'
    # -------------------------------------------------------------------
    Write-Host "repo_path         = $RepoPath"
    Write-Host "worktree_path     = $WorktreePath"
    Write-Host "main_branch       = main"
    Write-Host "main_head         = $mainHead"
    Write-Host "main_commit_count = $mainCommitCount"
    Write-Host "task_branch       = $TaskBranch"
    Write-Host "worktree_head     = $wtHead"
    Write-Host "worktree_branch   = $wtBranch"
    Write-Host "git_version       = $($verRes.Stdout)"

    Write-Host ''
    Write-Host 'BOOTSTRAP RUNBOOK A (v1.3) COMPLETE - all enforced invariants passed.' -ForegroundColor Green
    exit 0

} catch {
    # Genuinely unexpected PowerShell-level exceptions only (e.g. git.exe
    # not found at all, or a bug outside Invoke-Native's scoped-EAP fix).
    Write-Host ''
    Write-Host "BOOTSTRAP ABORTED - unexpected PowerShell exception: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host $_.ScriptStackTrace -ForegroundColor DarkRed
    if ($script:MutationStarted) {
        Write-Host 'Mutation had already begun. State was NOT rolled back or deleted - inspect and resolve manually before re-running.' -ForegroundColor Red
        Get-DiagnosticState
    }
    exit 1
}
```
