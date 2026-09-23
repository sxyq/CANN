# Consolidation Before Second Six-Lane (2026-09-24)

No kernel source, candidate implementation, server3 run, or CANNJudge submission was performed by Main.

## CANONICAL

- Branch: `exp/independent-breadth`
- Remote: `origin/exp/independent-breadth`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann`

## WORKTREE INVENTORY

| Path | Branch | HEAD relation | Classification |
|---|---|---|---|
| `cann` | `exp/independent-breadth` | canonical | KEEP |
| `cann-sixlane/R31B` | `exec/sixlane-20260923-r31b` | same HEAD, dirty local evidence | ACTIVE_KEEP |
| `cann-sixlane/R31A` | `exec/sixlane-20260923-r31a` | same HEAD, dirty local evidence | ACTIVE_KEEP |
| `cann-sixlane/MIX-A` | `exec/sixlane-20260923-mix-a` | same HEAD, dirty V007 | ACTIVE_KEEP |
| `cann-sixlane/WIDE-X-FRESH4` | `exec/sixlane-20260923-wide-x-fresh4` | same HEAD, untracked workspace | ACTIVE_KEEP |
| `cann-sixlane/MODE-X-R015C` | `exec/sixlane-20260923-mode-x-r015c` | same HEAD, untracked workspace | ACTIVE_KEEP |
| `cann-sixlane/EXT-ASCEND-X` | `exec/sixlane-20260923-ext-ascend-x` | same HEAD, untracked local | ACTIVE_KEEP |

`cann-worktrees/` is an empty container, not a Git worktree.

No first-round worktree met DELETE_SAFE criteria (all six remain scheduler ACTIVE).

## EVIDENCE PRESERVED INTO CANONICAL

Copied from ACTIVE worktrees without mutating those worktrees:

- `phase4/local/MIX-A/V007/**`
- `phase4/local/R31A/V019/**`
- `phase4/local/R31B/V016/**`
- `phase4/local/EXT-ASCEND-X/V001/**`
- `phase4/workspaces/MIX-A/MIX-A-V007-*` and compile logs
- `phase4/workspaces/R31A/R31A-V019-*` and adapters
- snapshots under `phase4/archive/active-sixlane-20260924/`

Restored champion evidence:

- `phase4/online/R31B/V011/submission.asc` (was working-tree deleted; identical to `submission.txt`)

## HISTORICAL BRANCH CONTENT ARCHIVE

- 49 non-merged local branches considered
- 47 branches had unique paths missing from canonical and from `归档/phase3-before-reset-20260920/`
- 402 files exported to `phase4/archive/historical-branches-20260924/<branch>/`
- Summary: `phase4/archive/historical-branches-20260924/summary.json`

Those historical branch commits are still not ancestors of canonical; they are retained (not force-deleted).

## CLASSIFICATION SUMMARY

- MERGE_TO_CANONICAL_THEN_DELETE: none among first-round worktrees (zero unique commits; ACTIVE)
- ARCHIVE_TO_CANONICAL: historical branch unique trees + active worktree evidence snapshots
- DELETE_SAFE worktrees: none
- ACTIVE_KEEP: all six `cann-sixlane/*` worktrees
- DIRTY_UNRESOLVED: none blocking second round; canonical dirty PARKED candidates remain untracked by design

## SECOND SIX-LANE CREATED

Container: `/Users/sunyiyang/Desktop/Project/cann-next6/` (not a worktree)

| Route | Branch | Brief |
|---|---|---|
| ASYNC-TRIPLE-X | exp/next6-async-triple-x | ROUTE-BRIEF.md |
| BATCH-RESIDENT-X | exp/next6-batch-resident-x | ROUTE-BRIEF.md |
| SCHED-ROWGROUP-X | exp/next6-sched-rowgroup-x | ROUTE-BRIEF.md |
| REDUCE-INVSCALE-X | exp/next6-reduce-invscale-x | ROUTE-BRIEF.md |
| ALIGN-TAIL-X | exp/next6-align-tail-x | ROUTE-BRIEF.md |
| UB-LIVENESS-X | exp/next6-ub-liveness-x | ROUTE-BRIEF.md |

All six based on canonical HEAD `ae46d7c`.

## NEXT6 MAIN REVIEWS

| route | rev | audit | correctness | local | decision |
|---|---|---|---|---|---|
| ALIGN-TAIL-X | V001 | PASS | PASS pair_max_abs=0 | LOAD_CONTAMINATED 4 probes | NEEDS_ONE_MORE_LOCAL |
