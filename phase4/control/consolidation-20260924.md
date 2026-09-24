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
| BATCH-RESIDENT-X | V001 | PASS | PASS batch path; col-split parent-inherited | LOAD_CONTAMINATED inconsistent | NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | V001 | PASS | PASS | LOAD_CONTAMINATED; aligned control also moved | NEEDS_ONE_MORE_LOCAL |
| REDUCE-INVSCALE-X | V001 | PASS | FAIL D>6144 parent partial-align | PARTIAL_CONTAMINATION | LOCAL_REJECTED; assign V002 correctness-only |
| ALIGN-TAIL-X | V001 | PASS | PASS pair_max_abs=0 | LOAD_CONTAMINATED | NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | V001 r2 | PASS | PASS | set1/set2 disagree; aligned control opposite | NEEDS_ONE_MORE_LOCAL; require VLLM stop |
| ASYNC-TRIPLE-X | V001 | - | - | worker general-1 UnknownError | replacement worker same route |

### NEXT6 batch review after first full handoffs

| route | rev | correctness | local | decision |
|---|---|---|---|---|
| ASYNC-TRIPLE-X | V001 | PASS | contaminated noise>signal | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| BATCH-RESIDENT-X | V001 | PASS batch path | no clean window | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| SCHED-ROWGROUP-X | V001 | PASS | set1/set2 disagree; no VLLM-free | NEEDS_ONE_MORE_LOCAL + PROBE PARKED |
| REDUCE-INVSCALE-X | V002 | PASS D<=32768 | repair cost below noise | correctness ACCEPTED; R019 perf deferred; PROBES PARKED; align hypothesis refuted; sync-only approved |
| ALIGN-TAIL-X | V001 | PASS | no clean window 81min | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| UB-LIVENESS-X | V001 | FAIL | N/A | LOCAL_REJECTED; assign V002 correctness-only |

Shared blocker: persistent root/zhangkaijie VLLM + HBM saturation; agents cannot create clean window alone.
| UB-LIVENESS-X | V002 | partial improve still FAIL | raw invalid | LOCAL_REJECTED; assign V003 pass1 acc fix |
| UB-LIVENESS-X | V003 | PASS bad=0 full battery | alias probes inconclusive | ONLINE_CANDIDATE frozen to pool; Judge Owner only |

## Server3 8-NPU window survey (2026-09-24)

| dev | free HBM MB | used/65536 | AICore | resident | role |
|---:|---:|---|---:|---|---|
| 0 | 5580 | 59956 | 31% | VLLMWorker_TP | busy compute |
| 1 | 5530 | 60006 | 32% | VLLMWorker_TP | busy compute |
| 2 | 5587 | 59949 | 32% | VLLMWorker_TP | busy compute |
| 3 | 5585 | 59951 | 32% | VLLMWorker_TP | busy compute |
| 4 | **6349** | 59187 | **0%** | VLLMEngineCor 55664MB | **PRIMARY run window** |
| 5 | 5658 | 59878 | **0%** | VLLMWorker_TP | SECONDARY run window |
| 6 | 5661 | 59875 | **0%** | VLLMWorker_TP | SECONDARY run window |
| 7 | 9 | 65527 | 17% | python 61828MB | AVOID |

Policy update: compile/link is CPU (no HBM need) and may parallelize on server3. NPU correctness + paired probes pin to **ASCEND_DEVICE_ID=4** primary; 5/6 allowed as secondary if no concurrent next6 probe. Residual VLLM HBM documented — do not wait for full VLLM stop. Device 7 forbidden.
| ALIGN-TAIL-X | V001 dev4 | PASS pairs | DIR 2/4 noise 393pp concurrent | NEEDS_ONE_MORE_LOCAL; serialize dev4 |
| BATCH-RESIDENT-X | V001 set2 | PASS batch | DIR 3/4 median -23.9% concurrent ALIGN | NEEDS_ONE_MORE_LOCAL; serialize dev4 |
| ASYNC-TRIPLE-X | V001 set2 | PASS | DIR 3/6 noise CV41% concurrent reduce | NEEDS_ONE_MORE_LOCAL; exclusive later |
| SCHED-ROWGROUP-X | V001 set3 | PASS | DEV4 residual | NEEDS_ONE_MORE_LOCAL; exclusive later |
| REDUCE-INVSCALE-X | V002 recheck | PASS | repair-cost PARK; concurrent srx on pair1 historically | R019 DEFERRED; d4 free |
| BATCH-RESIDENT-X | set2 | PASS | DIR 3/4 median -23.9% concurrent | **EXCLUSIVE_DEV4 GRANTED for set3** |
| ALIGN-TAIL-X | L002 controlled | PASS | MODERATE DIR3/4 median+19% slower | NEEDS_ONE_MORE_LOCAL; d4 free |
| BATCH-RESIDENT-X | exclusive set3 | PASS | exclusive interleaved; DIR 3/4 pair3 +185% | NEEDS_ONE_MORE_LOCAL (prior exclusive) |
| SCHED-ROWGROUP-X | L004 controlled | PASS | LOAD_CONTAMINATED sofm0.305 DIR0.5 | NEEDS_ONE_MORE_LOCAL; d4 free |
| ASYNC-TRIPLE-X | L006 controlled | PASS path triple | LOAD_CONTAMINATED CV29% DIR3/6 | NEEDS_ONE_MORE_LOCAL; d4 free |
| REDUCE-INVSCALE-X | L005 controlled | PASS both D6144 | LOAD_CONTAMINATED jitter78% | NEEDS_ONE_MORE_LOCAL; d4 free |
| ALIGN-TAIL-X | R2 baseline | N/A candidate | parent UNSTABLE d4/d5/d6 CV0.27-0.32 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| BATCH-RESIDENT-X | R2 baseline | N/A candidate | parent UNSTABLE d4/5/6 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL; pair3 OUTLIER |
| ASYNC-TRIPLE-X | R2 baseline | N/A candidate | parent UNSTABLE d4/5/6 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| REDUCE-INVSCALE-X | R2 baseline | SHA P!=C verified | parent UNSTABLE d4/5/6 candidate0 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | R2 pairs d6 | PASS | baseline STABLE d6; DIR 4/4 -18.45%; parent sofm0.558 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL; not REJECT |
| ROUND2_SUMMARY | five routes | - | all parent/jitter gates block ONLINE judgment | MEASUREMENT_BLOCKED x5; no ONLINE from R2; UB unchanged WAIT JUDGE |
| SCHED-ROWGROUP-X | window qual | - | d6+d4 FAIL CV/maxmin 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED; STRONG_POSITIVE retained |
| ALIGN-TAIL-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| BATCH-RESIDENT-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| ASYNC-TRIPLE-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| REDUCE-INVSCALE-X | window qual | P/C SHA OK | d6+d4 FAIL 2/2 candidate0 independent | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| WINDOW_QUAL_SUMMARY | five routes | - | all 2/2 UNQUALIFIED CV<=0.15 maxmin<=1.30 | stop timing today; SERVER_RESOURCE_BLOCKED; UB JUDGE_READY |

## Harness audit + timing protocol (2026-09-24 afternoon)

No Candidate source, no new revision, no timing retry, no CANNJudge submit.

- UB V003 package re-verified: submission.asc SHA256 matches sidecar `2eb9b5d0…`, source-meta DECISION=ONLINE_CANDIDATE / AUDIT=PASS / CORRECTNESS=PASS, diff.patch 14578 B, pool row + judge-handoff present, `phase4/online/UB-LIVENESS-X/` absent. Status remains READY_FOR_FORMAL_SUBMISSION for unified Judge Owner only.
- Measurement audit across five runners: primary method is CPU wall-clock around launch+sync (SCHED/ALIGN/REDUCE/BATCH); ASYNC uses device events over a repeat batch. Warmup only 3–5; each window-qual rep is a cold process (aclInit/finalize). Alloc/H2D outside timed loop (OK). Host launch + sync wait sit inside wall-clock samples. Persistent VLLM + ~90% HBM on d4/d6 remains. Observed parent CV 0.21–0.90 and max/min up to 9.0 within same binary/device/shape.
- Reference harness designated: SCHED `support/runner_main.inc` (sample+jitter dump). Unified rules in `phase4/control/local-timing-protocol.md`.
- Same-binary noise floor: NOT_RUN today. Next clean window validates SAME vs SAME before any Candidate pair; then SCHED → ALIGN → BATCH → ASYNC → REDUCE.
- Five routes stay NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED (SERVER_RESOURCE_BLOCKED), attempts 2/2. SCHED keeps STRONG_POSITIVE_LOCAL_SIGNAL BUT LOAD_NOT_QUALIFIED.
