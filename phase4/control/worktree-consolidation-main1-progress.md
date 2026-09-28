# Worktree Consolidation MAIN-1 Progress

Scope for this pass: MODE-X-R015C and DTYPE-SPECIAL-X only.

Canonical: `/Users/sunyiyang/Desktop/Project/cann` branch `exp/independent-breadth`

## MODE-X-R015C

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/MODE-X-R015C`
Route branch: `exec/sixlane-20260923-mode-x-r015c` (no upstream)
Worktree dirty at start: 0

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| `phase4/local/MODE-X-R015C/CURRENT/` | 0 (already identical, 10 files) | — |
| `phase4/local/MODE-X-R015C/R015C-r3/` | 16 | `5108a216` |
| `phase4/local/MODE-X-R015C/R015C-r4/` | 32 | `8ce9e2d7` |
| `phase4/workspaces/MODE-X-R015C/` | 9 added + 3 updated | `2890b5fa` |

R015C-r3 five-piece: diff.patch, local-result.json, source-meta.json, submission.asc, submission.sha256 — all present.
R015C-r4 five-piece: all present. source-meta.json records direct parent R015C-r3 with parent source SHA.

Workspace updates (canonical was older/incomplete; revision sources preserved under `local/`):

| File | Old SHA256 (canonical) | New SHA256 (worktree) |
|---|---|---|
| `op_host/row_copy_host.asc` | `c9939cb1a13c2aa883befdcdb1e13582a42f857fe68263ec5369c229d1c366a4` | `fbd3147221a36756d9fd65656635a4d5c92e8ad8be77405ed75c8de97e8726de` |
| `op_kernel/row_copy_kernel.asc` | `70ad9e17ca13f83f49e9b5dd8e7adad55f4c6f251a599fc4b87c9e935e78f04c` (CURRENT) | `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903` (r3) |
| `README.md` | `4bc31c74e6ffc9e89ed71e77567b47922ed78b39d598e7e12c2aba899bb685cc` | `24043e444f820e3a7655bf94094e3bca4ae1fe04491607fb76fb03039ccf2102` |

`phase4/online/MODE-X-R015C/` does not exist in either tree.

Ledger rows for CURRENT / r1-gap / r2 / r3 / r4 already present in `main1-revision-ledger.tsv`; no append needed.

## DTYPE-SPECIAL-X

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/DTYPE-SPECIAL-X`
Route branch: `exec/sixlane-20260924-dtype-special-x` (no upstream)
Worktree dirty at start: 1 modified + 6 untracked

### Route-branch commit first

`d90a0f83` — committed dirty `local-result.json` and 6 untracked measurement logs on the route branch (no upstream, no push).

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| V001 five-piece + docs + support scripts | 12 | `ac0eb066` |
| `V001/logs/` | 141 | `fbf983cb` |
| `V001/support/results/` deltas (R12 + admission) | 7 | `0e5c2511` |
| `phase4/workspaces/DTYPE-SPECIAL-X/V001/` | 12 | `07f05acd` |

V001 five-piece: all present. source-meta.json records direct parent `R31B-V011`, parent source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, parent score 45.16. No PARENT_UNRESOLVED.

Canonical already held 215 additional `support/results/` measurement files not in the worktree (parallel measurement runs); left untouched.

`phase4/online/DTYPE-SPECIAL-X/` does not exist in either tree.

Ledger row for V001 already present; no append needed.

## Verification

- `diff -rq` MODE local and workspaces: 0 missing, 0 differing.
- `diff -rq` DTYPE local: 0 missing from worktree, 0 differing, 215 extra in canonical (retained).
- `diff -rq` DTYPE workspaces: identical.
- SHA256 spot-check on 5 key files: all match worktree bytes.
- DTYPE `submission.sha256` references `submission.asc` hash `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f` — matches.

## Notes and residual items

- Commit `ac0eb066` accidentally included 19 already-staged `phase4/local/MIX-A/V007/` files from a parallel consolidation agent. Not amended (already pushed; amend rules not met). MIX-A content is valid evidence and is in-scope for a different consolidation pass.
- Parallel agents (R31A, R31B) commit and push on the same canonical branch. Staging must be verified with `git diff --cached` before every commit.
- Canonical working tree still carries dirty/untracked files from other routes (ALIGN-TAIL-X, BATCH-RESIDENT-X, SCHED-ROWGROUP-X, MIX-A, R31A, WIDE-X, etc.) — untouched by this pass.
- Remaining dirty in source worktrees after this pass: MODE-X-R015C 0, DTYPE-SPECIAL-X 0.
