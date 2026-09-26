# Worktree Consolidation (2026-09-26)

Asset recovery, canonical merge, and non-MAIN-1 worktree cleanup.

No kernel source change, no new performance Revision, no server3 run, no
profiling, no CANNJudge submission, no new Route, no new worktree was created.
No MAIN-1 worktree was touched. No remote branch was deleted. No history was
rewritten and no force push was used.

## CANONICAL

- branch: `exp/independent-breadth`
- worktree: `/Users/sunyiyang/Desktop/Project/cann`
- HEAD before: `b8ee4857ee62c1279871cf4ec09654796aad8d7c`
- remote: `origin/exp/independent-breadth`

## WORKTREE INVENTORY (before)

15 worktrees.

| Path | Branch | HEAD | OWNER_CLASS | KEEP_OR_CLEAN |
|---|---|---|---|---|
| `cann` | `exp/independent-breadth` | `b8ee485` | CANONICAL | KEEP |
| `cann-sixlane/R31B` | `exec/sixlane-20260923-r31b` | `99e878f` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/R31A` | `exec/sixlane-20260923-r31a` | `67d2dc0` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/MIX-A` | `exec/sixlane-20260923-mix-a` | `67aa684` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/WIDE-X-FRESH4` | `exec/sixlane-20260923-wide-x-fresh4` | `d23dcce` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/MODE-X-R015C` | `exec/sixlane-20260923-mode-x-r015c` | `e5106f0` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/DTYPE-SPECIAL-X` | `exec/sixlane-20260924-dtype-special-x` | `7858613` | MAIN1_PROTECTED | KEEP |
| `cann-sixlane/ASYNC-TRIPLE-X` | `exec/sixlane-20260924-async-triple-x` | `1632518` | HISTORICAL_TO_CONSOLIDATE | CLEAN after merge |
| `cann-sixlane/EXT-ASCEND-X` | `exec/sixlane-20260923-ext-ascend-x` | `a758f65` | HISTORICAL_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/ASYNC-TRIPLE-X` | `exp/next6-async-triple-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/BATCH-RESIDENT-X` | `exp/next6-batch-resident-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/SCHED-ROWGROUP-X` | `exp/next6-sched-rowgroup-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/REDUCE-INVSCALE-X` | `exp/next6-reduce-invscale-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/ALIGN-TAIL-X` | `exp/next6-align-tail-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |
| `cann-next6/UB-LIVENESS-X` | `exp/next6-ub-liveness-x` | `ae46d7c` | MAIN2_TO_CONSOLIDATE | CLEAN after merge |

No worktree was classified `UNKNOWN_REVIEW_REQUIRED`. Every non-MAIN-1 worktree
was dirty-free after its route assets were committed to its own branch.

## ROUTE BRANCH COMMIT + PUSH (step 6)

Each of the six MAIN-2 worktrees held untracked evidence
(`ROUTE-BRIEF.md`, `phase4/local/<ROUTE>/**`, `phase4/workspaces/<ROUTE>/**`).
Each was committed **only** to its own route branch, then pushed.

| Route | Branch | PRE_COMMIT_HEAD | POST_COMMIT_HEAD | REMOTE_HEAD | files | DIRTY_AFTER_COMMIT |
|---|---|---|---|---|---:|---|
| ALIGN-TAIL-X | `exp/next6-align-tail-x` | `ae46d7c` | `ce4abd8` | `ce4abd8` | 890 | NO |
| ASYNC-TRIPLE-X | `exp/next6-async-triple-x` | `ae46d7c` | `d5ab9a4` | `d5ab9a4` | 245 | NO |
| BATCH-RESIDENT-X | `exp/next6-batch-resident-x` | `ae46d7c` | `e95301e` | `e95301e` | 296 | NO |
| REDUCE-INVSCALE-X | `exp/next6-reduce-invscale-x` | `ae46d7c` | `d855cfc` | `d855cfc` | 499 | NO |
| SCHED-ROWGROUP-X | `exp/next6-sched-rowgroup-x` | `ae46d7c` | `b246c45` | `b246c45` | 1681 | NO |
| UB-LIVENESS-X | `exp/next6-ub-liveness-x` | `ae46d7c` | `ccd968e` | `ccd968e` | 127 | NO |

Local branch == remote branch (`ahead=0 behind=0`) for all six. These six
branches had **no remote before this pass**; they now exist on `origin`.

The two sixlane historical branches were already committed and already in sync
(`exec/sixlane-20260924-async-triple-x` = `1632518`,
`exec/sixlane-20260923-ext-ascend-x` = `a758f65`, both `ahead=0 behind=0`);
nothing was left uncommitted in those two worktrees.

No disposable/temporary file list was needed: every untracked file in the nine
consolidated worktrees was evidence and was committed. Nothing was deleted with
`git clean`, `git reset --hard`, or `git worktree remove --force`.

## CANONICAL MERGE (step 7)

Files were copied from the route worktrees into the canonical tree at the
existing layout, then verified byte-for-byte against the route branch blobs
(`MISMATCH_COUNT=0`).

| Source | Destination | files | verify |
|---|---|---:|---|
| `cann-next6/<ROUTE>/phase4/local/<ROUTE>/**` | `phase4/local/<ROUTE>/**` | 4738 total across all destinations | SHA256 identical |
| `cann-next6/<ROUTE>/phase4/workspaces/<ROUTE>/**` | `phase4/workspaces/<ROUTE>/**` | (included above) | SHA256 identical |
| `cann-next6/<ROUTE>/ROUTE-BRIEF.md` | `phase4/research/<ROUTE>/ROUTE-BRIEF.md` | 6 | SHA256 identical |
| `cann-sixlane/EXT-ASCEND-X/.../V001/**` | `phase4/local/EXT-ASCEND-X/V001/**` | 41 | git blob identical |
| `cann-sixlane/ASYNC-TRIPLE-X/.../V001/**` (local) | `phase4/archive/active-sixlane-20260924/ASYNC-TRIPLE-X/V001/**` | 38 | git blob identical |
| `cann-sixlane/ASYNC-TRIPLE-X/.../V001/**` (workspace) | `phase4/archive/active-sixlane-20260924/ASYNC-TRIPLE-X/V001-workspaces/**` | 18 | git blob identical |

`ROUTE-BRIEF.md` lives at the repository root in each route worktree, so six
copies cannot all occupy that path in canonical. They were placed at
`phase4/research/<ROUTE>/ROUTE-BRIEF.md` instead; the route branches keep them
at the root, which is the layout the routes were authored in.

### Path collision: two different ASYNC-TRIPLE-X V001 records

Both the Main-1 first-round copy and the Main-2 round-two route own a record
called `phase4/local/ASYNC-TRIPLE-X/V001/`, and they are different candidates:

| | Main-1 copy | Main-2 route |
|---|---|---|
| branch | `exec/sixlane-20260924-async-triple-x` | `exp/next6-async-triple-x` |
| candidate SHA256 | `61223a486cca4c54e099f760e9f48a4cd2fbedb2645967b936fe80d2785e1e1a` | `2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c` |
| correctness | `UNRESOLVED_SHARED_507035`, never assessed | PASS, `max_abs<=3.79e-07` |
| timing | `NOT_RUN` | measurement blocked after probes |
| status | `COLLISION_FROZEN` / `HISTORICAL_ONLY` | `ACTIVE_EXPLORE` |

`phase4/local/ASYNC-TRIPLE-X/V001/` and `phase4/workspaces/ASYNC-TRIPLE-X/`
therefore stay with the Main-2 route. The Main-1 copy was retained whole under
`phase4/archive/active-sixlane-20260924/ASYNC-TRIPLE-X/` with a README, and
`main1-revision-ledger.tsv` now points there. Neither record was deleted,
merged into the other, or rewritten.

### EXT-ASCEND-X source-meta was incomplete in canonical

canonical's `phase4/local/EXT-ASCEND-X/V001/source-meta.json` held a truncated
subset (0 lines unique to canonical, branch version strictly larger). The
branch version from `exec/sixlane-20260923-ext-ascend-x` replaced it, together
with the other 40 files, so the full `FIX_1..FIX_4` correctness record,
`EVIDENCE_FREEZE`, and `PARK_EXPLORE_SLOT` recommendation are now in canonical.

## ROUTE DISPOSITIONS (step 9)

Disposition is read off the recorded evidence; no new Main decision, no new
experiment, no new submission.

| Route | final Revision | Local verdict | Online status | Official result | Disposition | Evidence path (canonical) |
|---|---|---|---|---|---|---|
| SCHED-ROWGROUP-X | V001 | `ONLINE_CANDIDATE`, same-binary PASS, P/C 33x100 median −51.38% | **SUBMITTED** | **15/15, 22.27** (`6ab6c41b694b590c3ce2ef67`) vs parent 17.14 | **KEEP** (official PASS above parent) | `phase4/online/SCHED-ROWGROUP-X/V001/`, `phase4/local/SCHED-ROWGROUP-X/V001/`, `phase4/workspaces/SCHED-ROWGROUP-X/` |
| UB-LIVENESS-X | V003 (V001/V002 `LOCAL_REJECTED`) | `ONLINE_CANDIDATE`, correctness `bad=0` full battery | `NOT_FORMALLY_SUBMITTED` (`JUDGE_READY`) | none — no `result.json` exists | **JUDGE_PENDING** | `phase4/local/UB-LIVENESS-X/V003/`, `phase4/control/judge-handoff-ub-v003.md` |
| ALIGN-TAIL-X | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`; warmup=45 same-binary PASS, 4 pairs mixed | `NOT_SUBMITTED` | none | **KEEP** | `phase4/local/ALIGN-TAIL-X/V001/`, `phase4/workspaces/ALIGN-TAIL-X/` |
| BATCH-RESIDENT-X | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`; window quals 2/2 FAIL, warmup=45 cells polluted | `NOT_SUBMITTED` | none | **KEEP** | `phase4/local/BATCH-RESIDENT-X/V001/`, `phase4/workspaces/BATCH-RESIDENT-X/` |
| REDUCE-INVSCALE-X | V002 (V001 `LOCAL_REJECTED`) | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`; warmup=45 same-binary FAIL | `NOT_SUBMITTED` | none | **KEEP** | `phase4/local/REDUCE-INVSCALE-X/V001/`, `.../V002/`, `phase4/workspaces/REDUCE-INVSCALE-X/` |
| ASYNC-TRIPLE-X (MAIN-2) | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`; window quals 2/2 FAIL | `NOT_SUBMITTED` | none | **KEEP** | `phase4/local/ASYNC-TRIPLE-X/V001/`, `phase4/workspaces/ASYNC-TRIPLE-X/` |
| ASYNC-TRIPLE-X (MAIN-1 copy) | V001 | correctness `UNRESOLVED_SHARED_507035`, timing `NOT_RUN` | `NOT_SUBMITTED` | none | **HISTORICAL_ONLY** | `phase4/archive/active-sixlane-20260924/ASYNC-TRIPLE-X/V001/` |
| EXT-ASCEND-X | V001 | `LOCAL_REJECTED`, FP32 27/27 FAIL after `FIX_1..FIX_4` | `NOT_SUBMITTED` | none | **PARK** (`PARK_EXPLORE_SLOT`) | `phase4/local/EXT-ASCEND-X/V001/` |

No route was `REJECT`ed or deleted at route level; every failure/contamination
record (`MEASUREMENT_BLOCKED`, `WINDOW_UNQUALIFIED`, `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`,
`LOCAL_REJECTED`) was merged into canonical together with the reason.

## SCHED-ROWGROUP-X OFFICIAL RESULT RECONCILED (step 8)

Exact repository evidence in `phase4/online/SCHED-ROWGROUP-X/V001/result.json`:

- `passCount` 15 / `testcaseCount` 15, `status` `pass`
- `officialScore` **22.27**, `maxOutputErrorPercent` 0
- `identityStatus` `LOCAL_SHA_EQ_REMOTE_SHA`, `formalResultEligible` true
- `submissionId` `6ab6c41b694b590c3ce2ef67`
- source SHA256 `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c`
  (matches `submission.sha256` and the pinned pool SHA)

Control was synchronized to that evidence: `scheduler.tsv` `online_status`
changed from `NOT_SUBMITTED`, `online-candidate-pool.tsv` row moved from
`JUDGE_READY=YES; READY_FOR_FORMAL_SUBMISSION` to `ONLINE_RESULT`.
**This revision was not resubmitted and no new submission was made.**

## CONTROL FILES UPDATED (step 10)

| File | Change |
|---|---|
| `phase4/control/scheduler.tsv` | workspace column for the six MAIN-2 rows and `EXT-ASCEND-X` now points at canonical paths; `SCHED-ROWGROUP-X` `online_status` now carries the 15/15 22.27 result and its superseded `WINDOW_UNQUALIFIED` compile note is corrected; every touched row records `WORKTREE_CONSOLIDATED 2026-09-26` |
| `phase4/control/online-candidate-pool.tsv` | `SCHED-ROWGROUP-X` → `ONLINE_RESULT` with submission id, SHAs, and canonical evidence paths (the original exact-source detail was kept, not overwritten); `UB-LIVENESS-X` records that no `result.json` exists → `NOT_FORMALLY_SUBMITTED`; `NEXT6-POLICY` records the consolidation |
| `phase4/control/main1-revision-ledger.tsv` | `EVIDENCE_PATH` for Main-1 `ASYNC-TRIPLE-X/V001` and `EXT-ASCEND-X/V001` repointed from the two removed worktrees |
| `phase4/control/judge-handoff-ub-v003.md` | UB package path now canonical |
| `phase4/control/local-timing-protocol.md` | four reference-harness / evidence paths now canonical |
| `phase4/control/next-round-plan.md` | NEXT6 worktree column now canonical path + removal note |
| `phase4/control/WORKTREE_DELETION_PLAN.tsv` | pre-delete safety audit (new) |
| `phase4/control/worktree-consolidation-20260926.md` | this document (new) |

`phase4/control/server3-device-leases.tsv` needed no edit: every lease line is
already `RELEASED` or `FREE`; there is no open lease to close.

Left untouched on purpose: historical session logs
(`consolidation-20260924.md`, `handoff-*.md`,
`measurement-layer-b1b2-research-20260925.md`, `research/*/next-hypotheses.md`)
still quote the worktree paths that existed when they were written. They are
records of those sessions, not live pointers; canonical equivalents are listed
above and in `phase4/README.md`.

## DELETION (step 14)

See `phase4/control/WORKTREE_DELETION_PLAN.tsv`. Removal used plain
`git worktree remove <path>` (never `--force`). The MAIN-1 six and the
canonical worktree are never candidates for removal.

## BRANCH POLICY (step 15)

- route branches: kept, including the six newly pushed `exp/next6-*` branches
- remote branches: **none deleted**
- no history rewrite, no force push
- whether to drop remote historical branches later is left to Main Review
