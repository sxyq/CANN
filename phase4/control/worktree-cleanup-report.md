# Worktree Cleanup Report

Inventory and cleanup were performed from the local Git repository and the current phase4 control/evidence files. No kernel build, device run, benchmark, profiling, CANNJudge submission, or route redesign was performed.

## INITIAL WORKTREES

Initial count: 1

| Path | Branch | HEAD | Dirty | Modified | Untracked | Ahead/behind | Last commit | Route | Scheduler status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `/Users/sunyiyang/Desktop/Project/cann` | `exp/independent-breadth` | `beb7f4cfd774163c09ab61d49eff4faf6c649455` | yes | `phase4/workspaces/R31B/CMakeLists.txt`; `phase4/workspaces/WIDE-X/local_types.h`; `phase4/workspaces/WIDE-X/submission.asc` | `phase4/workspaces/EPI-X-FRESH/`; `phase4/workspaces/EPI-X/`; 13 MIX-A files; `phase4/workspaces/MODE-X-R015C/`; `phase4/workspaces/MODE-X/`; `phase4/workspaces/R31A/R31A-V018-submission.asc`; 4 R31B V015 files; `phase4/workspaces/WIDE-X-FRESH/`; `phase4/workspaces/WIDE-X-FRESH4/` | ahead 2, behind 0 versus `origin/exp/independent-breadth` | `phase4: preserve retired R012 source evidence` | main root containing route directories | mixed; scheduler ACTIVE routes and PARKED routes are subdirectories of this root |

The porcelain inventory contained only the main worktree. There is no `.git/worktrees` directory and no route directory with a linked-worktree `.git` file. The two commits ahead of the remote are `4b3b072` and `beb7f4c`; both are already on the target branch.

The scheduler currently marks these six routes `active`: `R31A`, `R31B`, `MIX-A`, `WIDE-X-FRESH4`, `MODE-X-R015C`, and `EPI-X-FRESH`.

## KEEP_ACTIVE

- Main worktree `/Users/sunyiyang/Desktop/Project/cann`; it is the target branch and contains the six scheduler-ACTIVE route directories.
- Retained route directories: `phase4/workspaces/R31A/`, `phase4/workspaces/R31B/`, `phase4/workspaces/MIX-A/`, `phase4/workspaces/WIDE-X-FRESH4/`, `phase4/workspaces/MODE-X-R015C/`, and `phase4/workspaces/EPI-X-FRESH/`.
- These route directories are ordinary directories inside the main worktree, not independent Git worktrees. Their Candidate contents were left unchanged.

## MERGED_THEN_DELETED

None. No secondary Git worktree existed, so there was no worktree-local commit to cherry-pick and no worktree to remove.

## ARCHIVED_THEN_DELETED

None. The archived source snapshots below were created because the source was dirty and valuable, while the original directories remain in the main worktree. No source directory was deleted.

| Route | Snapshot | Preserved material |
| --- | --- | --- |
| `EPI-X` | `phase4/archive/retired-routes/EPI-X/DIRTY-WORKTREE-20260923/` | 30 source/log files, `source.sha256`, `submission.sha256`, `diff.patch`, `source-meta.json` |
| `MODE-X` | `phase4/archive/retired-routes/MODE-X/DIRTY-WORKTREE-20260923/` | 17 source/log files, `source.sha256`, `submission.sha256`, `diff.patch`, `source-meta.json`, `HANDOFF_V001.md` |
| `WIDE-X-FRESH` | `phase4/archive/retired-routes/WIDE-X-FRESH/DIRTY-WORKTREE-20260923/` | `kernel.txt`, `source.sha256`, `diff.patch`, `source-meta.json` |
| `WIDE-X` | `phase4/archive/retired-routes/WIDE-X/DIRTY-WORKTREE-20260923/` | dirty `submission.asc` and `local_types.h`, `source.sha256`, `submission.sha256`, `diff.patch`, `source-meta.json`; existing online evidence remains at `phase4/online/WIDE-X/V001/` and `V002/` |

## DELETE_SAFE

None. `git worktree remove` was not run because Git reported no obsolete secondary worktree.

## DIRTY_UNRESOLVED

None as a separate Git worktree. Dirty ACTIVE files remain in the main worktree by design. Dirty PARKED source was copied to the archive paths above before this report was created; the original source was not altered.

## BRANCHES_DELETED

None. No local branch was deleted. Branches not attached to a worktree were left untouched because this task had no secondary worktree deletion to trigger branch cleanup, and no `git branch -D` operation was used.

## WORKTREES_REMAINING

```text
worktree /Users/sunyiyang/Desktop/Project/cann
HEAD beb7f4cfd774163c09ab61d49eff4faf6c649455 at inventory time
branch refs/heads/exp/independent-breadth
```

The remaining Git worktree count is 1. The six scheduler-ACTIVE route directories remain under that worktree. No route was reused, renamed, or converted into another route.

## VALIDATION

- `git fetch origin` completed successfully.
- Local target branch was ahead 2 and behind 0; no mainline synchronization was required.
- The scheduler and next-round plan were read from the repository before classification.
- The six ACTIVE route directories retained identical SHA256 manifests before and after archive creation: 227 files, `ACTIVE_SOURCE_DIGESTS_UNCHANGED=YES`.
- No Active Candidate file was staged or changed by this cleanup.
- No performance experiment, server3 benchmark, CANNJudge action, profiling, new Kernel revision, or agent dispatch was performed.

## SECOND-PASS WORKTREE DELETION

This section records the second pass using the current Git state and current scheduler. It supersedes the first-pass route labels where the current scheduler differs.

### SECOND-PASS INVENTORY

- Local HEAD: `4ba313099c7a903660f212943c13958010a1eca5`
- Remote HEAD: `4ba313099c7a903660f212943c13958010a1eca5`
- Initial second-pass worktree count: 1
- `git worktree list --porcelain` returned only `/Users/sunyiyang/Desktop/Project/cann` on `exp/independent-breadth`.
- The six current ACTIVE route rows are source-seed directories inside the main worktree; scheduler records their isolated worktrees as `NOT_CREATED`.

### DELETED WORKTREES

None. No secondary Git worktree was present, so `git worktree remove` was not run.

### DELETED BRANCHES

None. No worktree was removed and no local branch met the branch cleanup condition.

### KEEP_ACTIVE

- Main worktree `/Users/sunyiyang/Desktop/Project/cann` remains retained.
- Scheduler-ACTIVE source-seed directories retained in place: `R31B`, `R31A`, `MIX-A`, `WIDE-X-FRESH4`, `MODE-X-R015C`, and `EXT-ASCEND-X` (the last is `NOT_CREATED` in the workspace tree).
- No Active Candidate file was edited, staged, or removed by this second pass.

### DIRTY_UNRESOLVED

None as a separate Git worktree. The main worktree still contains the pre-existing dirty Candidate files and untracked route source; they remain untouched. The PARKED dirty source snapshots from the first pass remain available and passed the second-pass archive validation.

### ARCHIVE VALIDATION

| Route | Source match | SHA256 manifest | Diff | Metadata |
| --- | --- | --- | --- | --- |
| `EPI-X` | YES | PASS | present | valid JSON |
| `MODE-X` | YES | PASS | present | valid JSON |
| `WIDE-X` | YES | PASS | present | valid JSON |
| `WIDE-X-FRESH` | YES | PASS | present | valid JSON |

### REMAINING WORKTREES

Final second-pass count: 1

```text
worktree /Users/sunyiyang/Desktop/Project/cann
HEAD 4ba313099c7a903660f212943c13958010a1eca5
branch refs/heads/exp/independent-breadth
```

The local and remote target branches are equal. No scheduler file, Active Candidate, Kernel source, or other policy file was changed in this second pass. No performance experiment, server3 run, CANNJudge action, or Agent dispatch was performed.

## THIRD-PASS CONSOLIDATION (2026-09-24)

See `phase4/control/consolidation-20260924.md`.

- First-round sixlane worktrees: ACTIVE_KEEP (no deletion).
- Active local evidence copied into canonical `phase4/local/**`.
- Historical non-merged branch unique trees archived under `phase4/archive/historical-branches-20260924/`.
- R31B V011 `submission.asc` restored.
- Ready for second six-lane container `cann-next6/`.
