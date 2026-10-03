# MAIN-2 Local Control Dashboard

UPDATED_UTC: 2026-10-03T07:14:12Z
SCOPE: MAIN-2 local control only
CANONICAL_SHARED_LEDGER: UNMODIFIED

## Event receipt — HBH-09 V002 Window D measurement — 2026-10-03T07:14Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=ec65b3b9
ROUTE_MEASUREMENT_COMMIT=e30ac1d6
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
OFFICIAL_CHAMPION=R31B V011 / 45.16
DEVICE=4; SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
SAME_BINARY=PASS; MAD_MEDIAN=3.7951%; BLOCK_DRIFT=0.3795%
PAIRED=PASS_RUNNER_RC0; BLOCKS=8; RAW_PAIRED_SAMPLES=496
PARENT_PAIRED_MAD_GT_10_PERCENT=6_OF_8_BLOCKS
VALID_LOCAL_SCORE=NO; LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO; DIRECT_ONLINE_SUBMISSION=0
CANONICAL_SHARED_LEDGER_CHANGES=0
```

The paired block deltas varied from `-14.346%` to `+26.991%`, with substantial
Parent jitter in 6/8 blocks and a direction reversal. All 558 samples and
pre/post device snapshots are committed; no sample was removed. This remains
measurement evidence only, not a valid Local score or a Candidate verdict.

```text
V002_NEXT=FRESH_TEMPORAL_WINDOW_RESOURCE_SCAN; SAME_SOURCE_AND_RUNNER
PUSH_STATE=PUSH_PENDING; PUSH_IS_NOT_AN_EXPERIMENT_GATE
```

## Event receipt — HBH-09 V002 Window D protocol predeclaration — 2026-10-03T07:10Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=ef55896c
ROUTE_PROTOCOL_COMMIT=ef0618a0
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
DEVICE=4; PRIMARY_SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
```

Window D repeats the primary BF16 D6144 rows=80 on d4 after Window A's d3
paired Parent jitter and mixed directions failed to support a score. The d4
07:10 probe showed HBM usage 94%, AICore 0%, and no shared lease. Next:
refresh all-device/process/lease snapshots; run same-binary first, with
immediate 8-block paired P/C only if qualification passes.

## Event receipt — HBH-09 V002 Window C qualification result — 2026-10-03T07:05Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=2159ba25
ROUTE_QUALIFICATION_COMMIT=ce582cd7
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
DEVICE=5; PRIMARY_SHAPE=BF16_D6144_ROWS40
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LOCAL_FORMAL_LEASE=ACQUIRED
SAME_BINARY=MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE
MAD_MEDIAN=37.1654%; BLOCK_DRIFT=57.1654%; RAW_SAMPLES=62
PARENT_CANDIDATE_TIMING=NOT_RUN
VALID_LOCAL_SCORE=NO; LOCAL_SCORE=NONE
ONLINE_READY=NO; DIRECT_ONLINE_SUBMISSION=0
CANONICAL_SHARED_LEDGER_CHANGES=0
```

The d5 live snapshot showed about 1479 MB free HBM, AICore 5%, no active
shared d5 lease, and high but recorded host/process load. The qualification
failed its same-binary noise gate, so no paired timing ran. Its session log's
final `LOCAL_FORMAL_LEASE=CONFLICT` line is an orchestration-wrapper
misclassification: the same log first records lease acquisition, and the
qualification log/raw file prove the runner executed. The original log is
preserved; the correction is documented in the Route evidence.

```text
V002_NEXT=FRESH_TEMPORAL_WINDOW_RESOURCE_SCAN; TRY_ANOTHER_PRIMARY
NO_NEXT_PERFORMANCE_REVISION=YES
PUSH_STATE=PUSH_PENDING; PUSH_IS_NOT_AN_EXPERIMENT_GATE
```

## Event receipt — HBH-09 V002 Window C protocol predeclaration — 2026-10-03T07:01Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=382fbf00
ROUTE_PROTOCOL_COMMIT=10d6065e
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
DEVICE=5; PRIMARY_SHAPE=BF16_D6144_ROWS40
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
```

Window C is predeclared from the primary d5 BF16 D6144 rows=40 target. Window
A's Parent paired MAD/median was below 5% in all four blocks; its Candidate
delta was still within the same-binary noise floor. The current d5 scan showed
approximately 1479 MB free HBM, AICore 0%, and no active shared d5 lease.
Next: refresh admission data, then same-binary qualification followed
immediately by 8-block paired P/C only on PASS.

## Event receipt — HBH-09 V002 Window B measurement — 2026-10-03T06:54Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=314afd8f
ROUTE_MEASUREMENT_COMMIT=a53c5a98
CURRENT_REVISION=HOTLOOP-BRANCH-HOIST-CHAMPION-X / HBH-09 V002
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
OFFICIAL_CHAMPION=R31B V011 / 45.16
DEVICE=4; SHAPE=FP32_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
BUILD=PASS; CORRECTNESS=PASS_12_OF_12
SAME_BINARY=PASS; MAD_MEDIAN=4.0715%; BLOCK_DRIFT=0.1986%
PAIRED=PASS_RUNNER_RC0; BLOCKS=8; RAW_PAIRED_SAMPLES=496
VALID_LOCAL_SCORE=NO; LOCAL_SCORE=NONE
ONLINE_READY=NO; DIRECT_ONLINE_SUBMISSION=0
CANONICAL_SHARED_LEDGER_CHANGES=0
```

The Parent same-binary qualification passed, and paired P/C ran immediately on
the same card under one lease. However, Parent paired MAD/median exceeded 10%
in 7/8 blocks; paired block directions and latency levels varied materially.
All 558 same-binary/paired raw samples and pre/post resource snapshots are
committed. No sample was removed. This is not a valid score and does not close
V002 or authorize a next performance Revision.

```text
V002_NEXT=NEW_TEMPORAL_WINDOW_RESOURCE_SCAN; SAME_SOURCE_AND_RUNNER
NO_NEXT_PERFORMANCE_REVISION=YES
PUSH_STATE=PUSH_PENDING; PUSH_IS_NOT_AN_EXPERIMENT_GATE
```

## Event receipt — HBH-09 V002 runtime loader support recovery — 2026-10-03T06:51Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=4e336469
ROUTE_ENV_RECOVERY_COMMIT=7d19917d
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
KERNEL_SOURCE_CHANGED=0
RUNNER_BINARY_CHANGED=0
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LDD_RC=0; UNRESOLVED_LIBRARY_COUNT=0
DIRECT_ONLINE_SUBMISSION=0
```

The support-only per-command CANN loader-path correction is recorded and
committed. It resolves `libruntime.so` and all other runner dependencies
without rebuilding or changing the runner. The failed launch remains a
startup diagnostic, not a qualification failure. Next: retry the exact d4
FP32 D6144 rows=80 same-binary probe, then immediately run paired P/C only on
qualification PASS.

## Event receipt — HBH-09 V002 Window B runner startup diagnostic — 2026-10-03T06:45Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=294b931e
ROUTE_PROTOCOL_COMMIT=80280eec
ROUTE_DIAGNOSTIC_COMMIT=9d8e1e42
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
DEVICE=4; PRIMARY_SHAPE=FP32_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LOCAL_FORMAL_LEASE=ACQUIRED_AND_RELEASED
SAME_BINARY=NOT_STARTED; RUNNER_RC=127
PARENT_CANDIDATE_TIMING=NOT_RUN
LOCAL_SCORE=NONE
ONLINE_READY=NO; DIRECT_ONLINE_SUBMISSION=0
CANONICAL_SHARED_LEDGER_CHANGES=0
```

Window B admission found no active d4 shared lease. The pre-run snapshot had
HBM `61977/65536 MB` (about 3559 MB free), AICore 1%, host load
`61.80/62.66/63.35`, and resident user/VLLM processes; none was stopped or
migrated. The exact existing performance executable then failed before
entering its runner because `libruntime.so` was absent from this command's
loader path. This is an invocation-environment startup failure, not a
same-binary qualification result and not a performance measurement. The
lease was released; no raw timing samples were produced.

```text
V002_NEXT=DIAGNOSE_CANN_RUNTIME_ENV; RETRY_EXACT_D4_FP32_D6144_ROWS80
PUSH_STATE=PUSH_PENDING; PUSH_IS_NOT_AN_EXPERIMENT_GATE
```

## Event receipt — HBH-09 V002 paired recovery, Window A — 2026-10-03T06:22Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=7de98c71
ROUTE_PAIRED_COMMIT=c6d6c34e
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CURRENT_REVISION=HOTLOOP-BRANCH-HOIST-CHAMPION-X / HBH-09 V002
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
ROUTE_BRANCH_HEAD=c6d6c34e
BUILD=PASS; CORRECTNESS=PASS_12_OF_12
EVENT_DRIVEN_PIPELINE=ENABLED
PLANNING_DECISIONS_CHANGED=0
CANONICAL_SHARED_LEDGER_CHANGES=0
DIRECT_ONLINE_SUBMISSION=0
ONLINE_READY=NO
```

The three primary-shape same-binary PASS probes from Window A immediately
entered paired Parent/Candidate timing on their qualified devices. All raw
samples are retained. None establishes a valid Local score: paired direction
is mixed, and the paired Parent jitter is substantially above the earlier
same-binary noise estimate.

| Device / primary shape | Parent median us | Candidate median us | Delta | Same-binary noise | Block directions |
|---|---:|---:|---:|---:|---|
| d0 FP32 D6144 rows=40 | 17.650 | 17.560 | -0.510% | 6.123% | mixed; paired Parent MAD/median 10.71% |
| d3 BF16 D6144 rows=80 | 20.290 | 19.720 | -2.809% | 2.037% | mixed; paired Parent MAD/median 8.43% |
| d5 BF16 D6144 rows=40 | 8.830 | 8.880 | +0.566% | 2.945% | mixed |

```text
V002_WINDOW_A_PAIRED_PROTOCOL=45_WARMUPS;31_DEVICE_EVENT_SAMPLES;4_ALTERNATING_BLOCKS
V002_LOCAL_SCORE=NONE
V002_VERDICT=NOT_COMPLETE; NO_ACCEPTED_OR_REJECTED_SCORE_VERDICT
V002_NEXT=NEW_RESOURCE_SCAN_AND_TEMPORALLY_DISTINCT_PRIMARY_QUALIFICATION
PUSH_STATE=PUSH_PENDING; PUSH_IS_NOT_AN_EXPERIMENT_GATE
```

## Event receipt — HBH-09 V002 multi-device qualification recovery, Window A — 2026-10-03T06:17Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=85e49000
ROUTE_QUALIFICATION_COMMIT=c0dd0e9a
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
EVENT_DRIVEN_PIPELINE=ENABLED
PLANNING_DECISIONS_CHANGED=0
CANONICAL_SHARED_LEDGER_CHANGES=0
DIRECT_ONLINE_SUBMISSION=0
ONLINE_READY=NO
```

The Oct-03 resource scan covered d0–d7. All eight cards had >100 MB free HBM
and there was no shared active lease at scan time; host load was
`33.52/52.21/56.72` with 34 logged-in users. Existing processes were not
stopped or migrated. A d6 shared lease appeared before admission, so that
comparison was skipped. Seven exact-source same-binary comparisons ran on
separate devices in one short global window:

| Device | Primary probe | Qualification | MAD/median | Block drift |
|---:|---|---|---:|---:|
| d0 | FP32 D6144 rows=40 | PASS | 0.061225 | 0.002268 |
| d1 | BF16 D6144 rows=40 | BLOCKED_FOR_SHAPE | 0.296791 | 0.030749 |
| d2 | FP32 D6144 rows=80 | BLOCKED_FOR_SHAPE | 0.351852 | 0.000000 |
| d3 | BF16 D6144 rows=80 | PASS | 0.020370 | 0.001852 |
| d4 | FP32 D6144 rows=40 | NEEDS_VALIDATION | 0.170940 | 0.170940 |
| d5 | BF16 D6144 rows=40 | PASS | 0.029446 | 0.030624 |
| d6 | FP32 D6144 rows=80 | NOT_RUN — SHARED_LEASE_CONFLICT | — | — |
| d7 | BF16 D6144 rows=80 | BLOCKED_FOR_SHAPE | 0.410798 | 0.458920 |

The three primary PASS pairs (d0 FP32 rows=40, d3 BF16 rows=80, d5 BF16
rows=40) immediately enter interleaved Parent/Candidate timing on those same
devices. No Local score is claimed yet; no control dtype substitutes for a
primary shape.

```text
V002_PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
V002_CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
V002_QUALIFICATION_WINDOW_A=7_RUNS;3_PRIMARY_PASS;1_LEASE_SKIPPED
V002_NEXT=d0_FP32_R40+d3_BF16_R80+d5_BF16_R40_PAIRED_FORMAL
V002_LOCAL_SCORE=NONE_PENDING_PAIRED_WINDOWS
PUSH_STATE=PUSH_PENDING
```

## Event receipt — HBH-09 V002 final Local closure — 2026-10-02T20:54Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=7dbfe5fa
ROUTE_BRANCH_HEAD=c2cafa83
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
EVENT_DRIVEN_PIPELINE=ENABLED
PLANNING_DECISIONS_CHANGED=0
CANONICAL_SHARED_LEDGER_CHANGES=0
DIRECT_ONLINE_SUBMISSION=0
ONLINE_QUEUE=EMPTY
```

`HOTLOOP-BRANCH-HOIST-CHAMPION-X` HBH-09 sibling V002 has reached its final
bounded local verdict. The Candidate was built from the exact R31B V011
parent, not from rejected HBH-10 V001. Build and correctness passed, but no
performance conclusion is admissible: FP32/BF16 primary shapes did not pass
the same-binary qualification gate, and the only authorized FP16 rows=80
control paired run failed the Parent noise gate. V002 is therefore
`MEASUREMENT_BLOCKED`, with no Local score, no Local Best change, and no Online
eligibility.

```text
BRANCH_HBH09_REVISION=V002
BRANCH_HBH09_HYPOTHESIS=HBH-09_GENERIC_REUSE_FENCE_GUARD_FACTORING
BRANCH_HBH09_PARENT=R31B V011
BRANCH_HBH09_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BRANCH_HBH09_CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
BRANCH_HBH09_SOURCE_COMMIT=28b7b3bd
BRANCH_HBH09_BUILD=PASS; BUILD_COMMIT=5ee6673c; BOUNDED_RETRY=004
BRANCH_HBH09_CORRECTNESS=PASS; CASES=12/12; CORRECTNESS_COMMIT=6c7ac354
BRANCH_HBH09_SAME_BINARY_COMMIT=065c8c21
BRANCH_HBH09_PRIMARY_GATE=FP32/BF16_BLOCKED_OR_NEEDS_VALIDATION
BRANCH_HBH09_CONTROL_GATE=FP16_ROWS80_PASS
BRANCH_HBH09_PAIRED_COMMIT=d49d55e3
BRANCH_HBH09_PAIRED_RUNNER=PASS; RUNNER_RC=0; DEVICE=7
BRANCH_HBH09_PARENT_MAD_OVER_MEDIAN=0.179208,0.299904,0.257561,0.222965
BRANCH_HBH09_NOISE_THRESHOLD=0.10
BRANCH_HBH09_FINAL_VERDICT=MEASUREMENT_BLOCKED
BRANCH_HBH09_FINAL_COMMIT=c2cafa83
BRANCH_HBH09_LOCAL_SCORE=NONE
BRANCH_HBH09_LOCAL_BEST_CHANGED=0
BRANCH_HBH09_OFFICIAL_SCORE=NONE
BRANCH_HBH09_ONLINE=NOT_PERMITTED
BRANCH_HBH09_NEXT=PLANNING_REVIEW; NO_RETEST_NO_SECOND_MECHANISM
SERVER_DEVICE=7; HBM_PAGES_PRE_POST=24790/24792; AICORE_PRE_POST=0/0
SERVER_LEASE=LOCAL_FLOCK_RELEASED; SHARED_TABLE_MODIFIED=0
CONTROL_HEAD_AFTER_RECEIPT=2c523497
PUSH_STATE=PUSH_PENDING; LAST_PUSH_ATTEMPT=20S_TIMEOUT_EXIT124
```

The paired raw samples, logs, and pre/post load snapshots are retained in the
Route worktree. The two generated build directories remain untracked local
artifacts and were not deleted or included in the evidence commits.

## Latest closure receipt

The task-scoped Track-B reconciliation for the five originally approved M2
routes is recorded in
`研究/主代理/MAIN-2-W2/TRACK-B-HANDOFF-CLOSURE-20261001.md`. It confirms
five local committed handoffs, `MAIN_SELECTED=NONE` throughout, no new
implementation or experiment, and unresolved remote write synchronization.
The later portfolio notes in this dashboard remain historical control records;
this receipt does not make a lifecycle decision or change the shared ledgers.

## Latest long-run checkpoint

BOOTSTRAP_RECEIPT: `BOOTSTRAP-RECEIPT-20261001.md`
CANONICAL_HEAD: `02482b46c2ee1fdd5bab1f88a474c7e70426f661`
OVERALL_OFFICIAL_CHAMPION: `R31B V011 / 45.16`

| Route | Latest local fact | Revision / selection | Worktree |
|---|---|---|---|
| UB-LIFETIME-SAFE-CHAMPION-X | V001 build/link/identity PASS; correctness FAILED (7/8, FP32-wide-16384); evidence `c44e1a89`; restore trail `6b6972a0`; status update `12cfb16` | V001 failed and restored to legal parent evidence; no V002 | CLEAN |
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V005 committed `ee5f3b73`; four refined items; duplicate audit and shape qualification complete | `MAIN_SELECTED=NONE`; Track-B only | CLEAN |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | HBH-09 V002 final `MEASUREMENT_BLOCKED`; closure `c2cafa83`; source `1aa6d8ec…` | no Local score; no Online; Planning/Review | Route worktree has only untracked build artifacts |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X | pool audit committed `526c181a`; TCSU-H1..H3 only | `MAIN_SELECTED=NONE`; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` | CLEAN |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X | boundary audit committed `6ac8774b`; RFH-1 only | `MAIN_SELECTED=NONE`; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` | CLEAN |

All route-local contexts are closed. No research lane created a Revision or
changed kernel/build/runner files. No formal timing, Online, or Judge action
is eligible from this checkpoint.

## Resource and online snapshot

```text
SNAPSHOT_UTC=2026-10-01T22:12:02Z
HOST=hwnput3
FREE_HBM_MB=0:5313,1:5263,2:5319,3:5318,4:6346,5:5356,6:5357,7:12508
AICORE_PERCENT=0:18,1:20,2:33,3:33,4:0,5:0,6:0,7:1
EXISTING_USERS=VLLM_WORKERS_ON_0_TO_6; PYTHON3_ON_7 (PID 439848)
PROCESS_ACTIONS=NONE; no user process stopped, paused, migrated, or preempted
SERVER3_ALIAS=cann-server3; DNS_UNRESOLVED_FROM_THIS_RUNTIME
SERVER_JOBS_STARTED_BY_THIS_CHECKPOINT=0
FORMAL_PERFORMANCE_RUNS=0
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
ONLINE_SUBMISSIONS=0
```

## Event receipt — BRANCH V001 final closure / ADDR known-good closure — 2026-10-02T19:40Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=76d51ecb
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
EVENT_DRIVEN_PIPELINE=ENABLED
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
CANONICAL_SHARED_LEDGER_CHANGES=0
DIRECT_ONLINE_SUBMISSION=0
ONLINE_QUEUE=EMPTY
```

BRANCH HBH-10 V001 is now closed with final `LOCAL_REJECTED` after two valid
formal windows on BF16-D32768. The second window deltas were
`-0.071280%, 0.000000%, +0.278936%, +0.722542%`; combined with window 1 they
remain mixed and below the observed same-binary noise scale. No Local or
Official score is claimed. Evidence commit `b0d33c09` is followed by the
explicit restore commit `c24dbb2a`; the route source is byte-identical to
R31B V011 (`a8c19a...`). HBH-09 is now the only authorized sibling and its
fresh Route Agent has started; no V002 result exists yet.

ADDR H3 V001 used a support-only runner rebuilt from the historical R31B V016
paired runner. The exact Parent failed at the first D40960 warmup synchronize
with `507035`; prior isolated Parent/Candidate d4/d7 runs recorded the same
blocker for both binaries. Final classification is
`MEASUREMENT_BLOCKED / SHARED_RUNTIME_BLOCKER`; Formal Local, ADDR-H1, V002,
and Direct Online are forbidden. Closure commit is `53db8038`.

```text
BRANCH_V001_FINAL_VERDICT=LOCAL_REJECTED
BRANCH_V001_FORMAL_WINDOWS=2
BRANCH_V001_LOCAL_SCORE=NONE
BRANCH_V001_RESTORE_COMMIT=c24dbb2a
BRANCH_V001_RESTORED_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BRANCH_HBH09_AGENT=01a0fe20-e90a-7922-a74e-d414ff7a491c
BRANCH_HBH09_V002=IN_PROGRESS_NO_RESULT
ADDR_V001_BUILD=PASS
ADDR_V001_KNOWN_GOOD_BUILD=PASS
ADDR_V001_FINAL_CORRECTNESS=MEASUREMENT_BLOCKED
ADDR_V001_CLASSIFICATION=SHARED_RUNTIME_BLOCKER
ADDR_V001_FORMAL_LOCAL=NOT_ELIGIBLE
ADDR_V001_H1=FORBIDDEN
ADDR_V001_V002=FORBIDDEN
UB=FORENSIC_CLOSED_PARENT_INSTABILITY
TILECOUNT=PARKED
REDUCE=PARKED
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```

## Event receipt — ADDR BF16-D40960 final shared-path classification — 2026-10-02T18:42Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=e80943be
ADDR_ROUTE_HEAD=e70e04287506aff71e59afbefa80c9aa7e7b3f24
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
EVENT_DRIVEN_PIPELINE=ENABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
ONLINE_QUEUE=EMPTY
```

ADDR V001 H3 completed the bounded BF16-D40960 diagnostic. The exact R31B
V011 Parent and H3 Candidate binaries both reached the first launch and then
failed at `aclrtSynchronizeStream=507035`, independently on d4 and d7. The
same failure across both binaries and devices classifies the event as
`SHARED_PATH_SIDE_EFFECT`; a Candidate-specific bug is not established.
The route evidence records correctness as `INCOMPLETE`, so Formal Local
timing is not eligible and no score is produced.

```text
ADDR_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
ADDR_CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
ADDR_RUNNER_SHA256=1df859b0d8e7161ac5d95962a97b06776011153e92f82c9d7f12e7c853c38b19
ADDR_BUILD=PASS
ADDR_CORRECTNESS=INCOMPLETE
ADDR_CLASSIFICATION=SHARED_PATH_SIDE_EFFECT
ADDR_CANDIDATE_BUG=NOT_ESTABLISHED
ADDR_FORMAL_LOCAL=NOT_ELIGIBLE
ADDR_TIMING=FORBIDDEN
ADDR_V002=FORBIDDEN
BRANCH_FORMAL_PERFORMANCE_RUNS=1_SHAPE
BRANCH_LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL
BRANCH_LOCAL_SCORE=NONE
BRANCH_ONLINE_QUEUE=EMPTY
UB_CLASS=PARENT_INSTABILITY
UB_V002=FORBIDDEN
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```

The active-branch aggregate push and its bounded `ls-remote` verification each
timed out (`exit 124`). Remote synchronization is therefore unconfirmed; no
force push or alternate remote was used.

## Planning gate

```text
UB_REVIEWABLE=YES; V001_CORRECTNESS_FAILED_AND_RESTORE_TRAIL_RECORDED
ADDR_TRACK_B_REVIEWABLE=YES
BRANCH_TRACK_B_REVIEWABLE=YES
TILECOUNT_TRACK_B_REVIEWABLE=YES; POOL_EXHAUSTED
REDUCE_TRACK_B_REVIEWABLE=YES; POOL_EXHAUSTED
MAIN_SELECTED_FOR_RESEARCH=0
REVISION_CREATED_BY_THIS_CHECKPOINT=0
KERNEL_FILES_CHANGED_BY_MAIN=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_CHANGED=0
READY_FOR_PLANNING_REVIEW=YES_LOCAL_ONLY
```

## Remote synchronization audit

AUDIT_UTC: 2026-10-01 (current continuation)

```text
FETCH_ORIGIN_MAIN=PASS; origin/main=02482b46c2ee1fdd5bab1f88a474c7e70426f661
REMOTE_HOTLOOP_ADDR=ABSENT
REMOTE_HOTLOOP_BRANCH=ABSENT
REMOTE_TILECOUNT=ABSENT
REMOTE_REDUCE_FINALIZE=ABSENT
REMOTE_UB=a9c9affcb49e8591f803ab409aafa6192a34caca
ACTIVE_PUSH_RESULT=FAILED; exit=128; could not read Username for https://github.com
UB_REMOTE_RELATION=NON_FAST_FORWARD_DIVERGENCE; common_base=ed860e39
FORCE_PUSH=NOT_USED
OVERWRITE=NOT_USED
ALTERNATE_REMOTE=NOT_USED
```

The remote UB branch is an existing older evidence line and is not overwritten.
The four absent active branches remain locally committed and ready for Planning;
remote publication requires restored GitHub write credentials. This is a remote
delivery blocker, not a reason to select a new hypothesis or alter route code.

## Final continuation audit

AUDIT_UTC: 2026-10-01T22:50:18Z

```text
READ_ONLY_LS_REMOTE=TIMEOUT_20S
FETCH_ORIGIN_MAIN=INTERRUPTED_AFTER_30S; no ref update observed
CONTROL_PUSH=TIMEOUT_30S; remote synchronization unconfirmed
FORCE_PUSH=NOT_USED
REMOTE_REF_CHANGES=NONE_CONFIRMED
```

The local canonical and control evidence remain intact. The timeout is carried
as a remote delivery blocker; it does not authorize a new Revision, route
selection, or lifecycle change.

This file is the explicit Dashboard for the current MAIN-2 long-run because
the repository has no pre-existing Dashboard path. It is not a replacement
for `调度/当前任务.tsv`, `技术路线/路线成绩表.tsv`, or any other canonical
shared ledger.

## Anchors

```text
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN=02482b46c2ee1fdd5bab1f88a474c7e70426f661
CANONICAL_STATUS=CLEAN
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

`origin/main` was fetched and canonical `main` was fast-forwarded. The
remote commit only registers MAIN-1 Wave-2 schedule rows; no Candidate source
or score changed.

## Route lanes and final gates

| Lane | Route | Branch / worktree | State | Current gate |
|---|---|---|---|---|
| M2-1 | UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` / `/home/data4t2/lelinfeng/cann-w2-m2-ub` | Track-A result recorded; context closed | V001 `CORRECTNESS_FAILED`; restore trail recorded; no V002 |
| M2-2 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `w2/m2/hotloop-addr` / `worktrees/w2/m2/hotloop-addr` | Track-B handoff closed; context closed | committed research only; `MAIN_SELECTED=NONE` |
| M2-3 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `w2/m2/hotloop-branch` / `worktrees/w2/m2/hotloop-branch` | Track-B handoff closed; context closed | committed research only; `MAIN_SELECTED=NONE` |
| M2-4 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `w2/m2/tilecount-unroll` / `worktrees/w2/m2/tilecount-unroll` | Track-B handoff closed; context closed | pool exhausted; `MAIN_SELECTED=NONE`; no implementation |
| M2-5 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `w2/m2/reduce-finalize` / `worktrees/w2/m2/reduce-finalize` | Track-B handoff closed; context closed | pool exhausted; `MAIN_SELECTED=NONE`; Planning review pending |

The older INTERPASS, CROSSROW, PARAM-RESIDENCY, and ROW-OCCUPANCY routes
remain Planning-owned read-only evidence and are not active in this dashboard.

## Current candidate and queue

```text
UB_V001=BUILD_PASS; LINK_PASS; EXECUTABLE_IDENTITY_PASS; CORRECTNESS=CORRECTNESS_FAILED (7/8; fp32-wide-16384)
UB_SOURCE_SHA256=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
UB_CORRECTNESS_EVIDENCE_COMMIT=c44e1a8933e11660ce78842231ffa2ef1069294a
UB_RESTORE_TRAIL_COMMIT=6b6972a0c16b05aa9345511bae9d262e22ae5693
UB_PERFORMANCE=NOT_ELIGIBLE
UB_ONLINE=NOT_AUTHORIZED
HBH09_V002=BUILD_PASS; CORRECTNESS_PASS_12_OF_12; FORMAL_LOCAL=MEASUREMENT_BLOCKED; FINAL=c2cafa83
HBH09_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
HBH09_LOCAL_SCORE=NONE; HBH09_ONLINE=NOT_PERMITTED
TRACK_B_MAIN_SELECTED_COUNT=0
NEW_REVISION_CREATED_BY_MAIN=0
LOCAL_BEST_CHANGES=0
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
ONLINE_SUBMISSIONS=0
LAST_OFFICIAL_RESULT=NONE_THIS_LONGRUN
```

All Track-B lanes remain handoff-only and created no Candidate. UB V001 failed
the exact-source correctness gate, so it cannot enter same-binary timing or
Online; no V002 is authorized from this checkpoint.

## Latest host resource snapshot

```text
LOCAL_HOSTNAME=hwnput3
SSH_ALIAS=cann-server3 (DNS_UNRESOLVED_FROM_THIS_RUNTIME)
NPU_COUNT=8
SNAPSHOT_UTC=2026-10-01T22:12:02Z
FREE_HBM_MB_BY_DEVICE=0:5313,1:5263,2:5319,3:5318,4:6346,5:5356,6:5357,7:12508
AICORE_PERCENT_BY_DEVICE=0:18,1:20,2:33,3:33,4:0,5:0,6:0,7:1
CORRECTNESS_DEVICE_PLAN=NONE; V001 correctness result already recorded
FORMAL_PERFORMANCE_LEASES_CREATED=0
```

HBM and load values are observational. Existing vLLM and other processes are
not stopped, paused, migrated, or treated as experiment failures. No new device
job is authorized from this checkpoint.

## Agent events

```text
UB_AGENT=01a0f97d-9396-7b00-81cf-c68a64db2f32 (fresh; closed after V001 evidence and restore)
ADDR_AGENT=01a0f97d-97e8-72f1-9fed-d8246e743cb7 (fresh; closed after Track-B handoff)
BRANCH_AGENT=01a0f97f-014e-7b71-85e7-9b8c1dccc2df (fresh; closed after Track-B handoff)
TILECOUNT_AGENT=01a0f97f-fe62-7f72-b64b-dd3a00b48fb4 (fresh; closed after pool audit)
REDUCE_AGENT=01a0f97f-93bf-74c2-afa9-d788d434d39d (fresh; closed after boundary audit)
ALL_WAVE2_CONTEXTS_CLOSED=YES
429_OCCURRED=NO
CONTEXT_SHARING=NO
```

Each fresh context owned one route and one worktree. All five bounded tasks
ended in committed route-local evidence or an explicit pool-exhausted finding;
no context remains active.

## Event log

| UTC | Event | Evidence / consequence |
|---|---|---|
| 2026-10-01T21:09Z | bounded `git fetch origin main` succeeded | remote `origin/main` advanced to `02482b46` |
| 2026-10-01T21:10Z | canonical fast-forward | local `main == origin/main`, clean |
| 2026-10-01T21:17Z | UB harness audit | `support/correctness/correctness_runner.asc` referenced missing `../submission.asc`; Candidate source itself unchanged |
| 2026-10-01T21:18Z | UB correctness context created | support-only binding fix and exact-source correctness are the next gate |
| 2026-10-01T21:19Z | ADDR/BRANCH contexts created | Track-B only; no Revision or device work |
| 2026-10-01T21:38Z | UB exact-source correctness completed | 7/8 passed; `fp32-wide-16384` failed; performance and Online remain ineligible |
| 2026-10-01T22:20Z | Track-B reconciliation closed | five handoffs committed locally; all contexts closed; `MAIN_SELECTED=NONE` |
| 2026-10-01T22:34Z | Remote branch audit completed | four active branches absent remotely; normal push blocked by GitHub HTTPS credentials; no force push |

## Blockers and invariants

```text
DASHBOARD_PATH=研究/主代理/MAIN-2-W2/DASHBOARD.md
SERVER3_REMOTE_ACCESS=BLOCKED_BY_DNS_ALIAS
LOCAL_SERVER_HOST=AVAILABLE_FOR_NONFORMAL_CHECKS
GITHUB_WRITE=BLOCKED; prior HTTPS auth failure and latest bounded push timeout
CANONICAL_SHARED_LEDGER_CHANGES=0
KERNEL_FILES_CHANGED_BY_CURRENT_LONGRUN=0
PERFORMANCE_RUNS=0
FORCE_PUSH=0; RESET=0; CLEAN=0
```

## Post-Track-B event-driven run (2026-10-02)

This is the current control checkpoint for the Planning-approved lifecycle.
The older route tables above are retained as historical evidence; this section
is the active view for the current forensic/research run.

```text
CONTROL_HEAD_AT_RUN_START=17425aeca0c34cc34f2f94183e0686416ad2037c
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
EVENT_DRIVEN_PIPELINE=ENABLED
BATCH_GATE=DISABLED
GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
MAIN_SELECTED_COUNT=0
NEW_REVISION_COUNT=0
ONLINE_QUEUE=EMPTY
ONLINE_TICK=SKIPPED; REASON=NO_ELIGIBLE_CANDIDATE (initial check)
ONLINE_SUBMISSIONS=0
PLANNING_DECISIONS_CHANGED=0
```

| Route | Planning lifecycle | Context / worktree | Current event |
|---|---|---|---|
| UB-LIFETIME-SAFE-CHAMPION-X | KEEP / DIAGNOSTIC | `01a0fa2f-2b44-7be0-b76c-8a2dadf07d08` (closed) / `/home/data4t2/lelinfeng/cann-w2-m2-ub` | forensic V001 closed; `UNRESOLVED`; no V002 |
| HOTLOOP-ADDR-HOIST-CHAMPION-X | KEEP / RESEARCH | `01a0fa2f-2e7f-7461-85e0-0a1d446944c8` (closed) / `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr` | planning pack committed; context closed |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | KEEP / RESEARCH | replacement `01a0fa4b-0161-7351-b95f-a8b1df5482a5` (closed) / `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-branch` | planning pack committed; context closed |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X | PARK | no Agent | no hypothesis or Revision allowed |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X | PARK | no Agent | no hypothesis or Revision allowed |

```text
UB_V001=CORRECTNESS_FAILED; no V002; no performance; no Online
ADDR_MAIN_SELECTED=NONE; no Candidate; planning pack committed
BRANCH_MAIN_SELECTED=NONE; no Candidate; planning pack committed
TILECOUNT=PARKED; REDUCE=PARKED
FORMAL_PERFORMANCE_RUNS=0
KERNEL_FILES_CHANGED_BY_MAIN=0
CANONICAL_SHARED_LEDGER_CHANGES=0
```

The event rule is: an eligible local commit starts its own Build immediately;
Build PASS starts Correctness immediately; Correctness PASS starts the next
authorized stage immediately. A route does not wait for sibling routes,
aggregate push, or a complete Dashboard refresh. This run has no eligible
Candidate for formal timing: UB V001 is correctness-failed and ADDR/BRANCH
are research-only.

## Current host snapshot for event scheduling

```text
SNAPSHOT_UTC=2026-10-02T01:57:17Z
HOST=hwnput3
SERVER3_ALIAS=cann-server3
SERVER3_DNS=UNRESOLVED
FREE_HBM_MB_BY_DEVICE=0:5312,1:5262,2:5319,3:5318,4:6346,5:5336,6:5337,7:40748
AICORE_PERCENT_BY_DEVICE=0:0,1:0,2:0,3:0,4:0,5:0,6:0,7:8
EXISTING_USERS=VLLM_WORKERS_ON_0_TO_6; VLLM/RAY/PYTHON_ON_7
UB_FORENSIC_REBUILD=3_ATTEMPTS (1_PASS; 2_TOOLCHAIN_ENV_FAILURES)
UB_FORENSIC_CORRECTNESS=2_NONFORMAL_RUNS_ON_DEVICE7
BRANCH_PARENT_COMPILE=1_PASS; STATIC_CODEGEN_PROBE_ONLY
ADDR_COMPILE_OR_PROFILE=0
NEW_NPU_JOBS_STILL_RUNNING=0
FORMAL_LEASES=0
FORMAL_PERFORMANCE_RUNS=0
HBM_BLOCKS=NONE_AT_SNAPSHOT
PROCESS_ACTIONS=NONE
```

The V001 reruns were non-formal correctness diagnostics and are preserved in
the UB route evidence; they do not make V001 performance-eligible. Existing
vLLM/Ray/Python processes were observed only and left untouched.

GitHub publication remains `PUSH_PENDING` because HTTPS remote access is
unconfirmed; this does not gate the three local research contexts. No force
push, reset, clean, alternate remote, or remote branch overwrite is allowed.

### Event receipt: ADDR planning pack

```text
EVENT_UTC=2026-10-02T01:27:56Z
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
LOCAL_COMMIT=cf148d5f8103c931cb253d2f36f0d7c59068fe23
EVIDENCE=研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/ADDR-PLANNING-PACK-20261002.md
ADDR_PLANNING_CANDIDATE_1=ADDR-H3
ADDR_PLANNING_CANDIDATE_2=ADDR-H1
REJECTED=DUPLICATE_OR_NO_EFFECT_REJECTED (H2,H4-H8,Q9,Q10)
MAIN_SELECTED=NONE
REVISION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
ONLINE=NOT_RUN
PUSH=PUSH_PENDING; remote publication is not a local-stage gate
```

ADDR's first event is a committed research handoff, not a Candidate score;
the Online queue remains empty and no sibling route was held for this receipt.

### Event receipt: UB V001 forensic closure

```text
EVENT_UTC=2026-10-02T01:27:25Z onward
ROUTE=UB-LIFETIME-SAFE-CHAMPION-X
REVISION=V001
STATIC_AUDIT_COMMIT=1922013f
EXECUTION_COMMIT=6d620289
EVIDENCE=研究/UB-LIFETIME-SAFE-CHAMPION-X/V001-FORENSIC-DIAGNOSTIC.md
FORENSIC=CLOSED
UB_V001_CLASS=UNRESOLVED
PARENT_SAME_BINARY=MISSING_PARENT_RUN
V001_REBUILT_EXECUTABLE_SHA256=c7b8fc47ee0e54f509499fd5829dcc5744eefcd9e57d4d323a1f51dd57f30849
V001_FP32_WIDE_16384_RUNS=FAIL; failures=16342,max_abs=3.32307e+38; failures=16347,max_abs=inf
V001_AGGREGATE=NONDETERMINISTIC
LIFETIME_ORDER=PASS_STATIC; last_y_read_3318 < output_write_3322 < store_wait_3345
PERFORMANCE=NOT_ELIGIBLE
ONLINE=NOT_AUTHORIZED
LANE_STATE=LANE_NEEDS_PLANNING_REVIEW
PUSH=PUSH_PENDING
```

The forensic closure does not authorize a V002, a source fix, timing, or
Online. The parent comparison and per-element mismatch diagnostics remain
explicitly missing from the existing source-bound harness; no stronger class
than `UNRESOLVED` is inferred.

### Event receipt: BRANCH planning pack and final gate

```text
EVENT_UTC=2026-10-02T01:52:51Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
LOCAL_COMMIT=7ffd28bcf01a0ea7420a283043bcfaea5af146d8
EVIDENCE=研究/HOTLOOP-BRANCH-HOIST-CHAMPION-X/BRANCH-PLANNING-PACK-20261002.md
COMPILER_EVIDENCE=研究/HOTLOOP-BRANCH-HOIST-CHAMPION-X/PARENT-COMPILER-STATIC-EVIDENCE-20261002.md
BRANCH_PLANNING_CANDIDATE_1=HBH-10
BRANCH_PLANNING_CANDIDATE_2=HBH-09
UNSLOTTED_SURVIVOR=HBH-11
REJECTED=HBH-12; REASON=REJECTED_NON_INDEPENDENT
COMPILER_ELIMINATION=NONE_PROVEN; RUNTIME_BRANCH_STATUS=UNVERIFIED
MAIN_SELECTED=NONE
REVISION=NONE
BUILD=NOT_RUN_FOR_CANDIDATE
CORRECTNESS=NOT_RUN_FOR_CANDIDATE
TIMING=NOT_RUN
ONLINE=NOT_RUN
PUSH=PUSH_PENDING; remote publication is not a local-stage gate
```

## Final event-driven Planning gate

```text
UB_FORENSIC=CLOSED; UB_V001_CLASS=UNRESOLVED; LANE_NEEDS_PLANNING_REVIEW
ADDR=RESEARCH_CLOSED; PLANNING_PACK=COMMITTED
BRANCH=RESEARCH_CLOSED; PLANNING_PACK=COMMITTED
TILECOUNT=PARKED; NO_AGENT; NO_REVISION
REDUCE=PARKED; NO_AGENT; NO_REVISION
READY_FOR_PLANNING_REVIEW=YES
MAIN_SELECTED_COUNT=0
NEW_REVISION_COUNT=0
ONLINE_SUBMISSIONS=0
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
FORENSIC_AGENT=01a0fa2f-2b44-7be0-b76c-8a2dadf07d08 CLOSED
ADDR_AGENT=01a0fa2f-2e7f-7461-85e0-0a1d446944c8 CLOSED
BRANCH_AGENT_REPLACED=01a0fa2f-3610-7d03-a158-a196219dcfff SHUTDOWN_NO_HANDOFF
BRANCH_AGENT=01a0fa4b-0161-7351-b95f-a8b1df5482a5 CLOSED
ONLINE_TICK_2026-10-02T01:37Z=RETROSPECTIVE_SKIP; REASON=NO_ELIGIBLE_CANDIDATE (ADDR/BRANCH research-only; UB V001 correctness-failed/diagnostic)
ONLINE_TICK_2026-10-02T01:57Z=SKIPPED; REASON=NO_ELIGIBLE_CANDIDATE
```

The three required independent contexts have produced committed route-local
evidence. This Main stops at Planning/Review: it does not select HBH-10,
HBH-09, ADDR-H3, or ADDR-H1; it does not open UB V002, ADDR V001, or BRANCH
V001. No formal Local score exists in this run, so the Online queue remains
empty and no score commit is fabricated.

```text
CONTROL_HEAD_AT_PLANNING_GATE=bf96bff5a235b59b698d35ba20e6768a90166537
UB_ROUTE_HEAD=6d620289c2764e502ed4795f679caa5110896b3e
ADDR_ROUTE_HEAD=cf148d5f8103c931cb253d2f36f0d7c59068fe23
BRANCH_ROUTE_HEAD=7ffd28bcf01a0ea7420a283043bcfaea5af146d8
CONTROL_PUSH=TIMEOUT_30S; PUSH_PENDING
LAST_PUSH_WINDOW_UTC=2026-10-02T01:56:00Z
```

```text
FINAL_LOCAL_CONTROL_HEAD_BEFORE_THIS_RECORD=0805d80cdba49265257d141ae5bc58dae22b14e3
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN_OBSERVED=15e7d8dc0a9269fdcdb09af0d1b9e4660c070778
ORIGIN_MAIN_DELTA=MAIN-1 dashboard/index.html only; no source, score, or champion change
CANONICAL_MAIN_MODIFIED=NO (the Planning directive forbids it)
REMOTE_PUSH_STATE=UNCONFIRMED_TIMEOUT; newest control commits remain local/PUSH_PENDING
CONTROL_HEAD_AFTER_FINAL_AUDIT=db176dca
```

## Event-driven implementation wave startup — 2026-10-02T14:48Z

```text
CONTROL_HEAD_AT_WAVE_START=50ea8cf1a4a8f1e014bfe8a003c56ff161424afd
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN=15e7d8dc0a9269fdcdb09af0d1b9e4660c070778
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
EVENT_DRIVEN_PIPELINE=ENABLED
BATCH_GATE=DISABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

| Lane | Fresh Agent | Branch / worktree | Current event | Parent |
|---|---|---|---|---|
| ADDR | `01a0fd14-133f-72f2-a53e-ceaf4b669876` | `w2/m2/hotloop-addr` / `worktrees/w2/m2/hotloop-addr` | `ADDR-H3` authorized; V001 declaration gate in progress | `R31B V011`, source SHA `a8c19a…15e3` |
| BRANCH | `01a0fd14-1f3d-7dd2-b955-0bcdbb7c1ac1` | `w2/m2/hotloop-branch` / `worktrees/w2/m2/hotloop-branch` | `HBH-10` authorized; V001 declaration gate in progress | `R31B V011`, source SHA `a8c19a…15e3` |
| UB forensic | `01a0fd14-1611-7153-83fc-f5a92fb52786` | `w2/m2/ub-lifetime-safe` / `cann-w2-m2-ub` | `UB-DIAG-2` started; no Candidate source change permitted | preserved V001 / exact R31B V011 |

```text
LOCAL_STAGE_GATE=local_commit_then_build_then_correctness_then_formal_local
GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
ONLINE_QUEUE=EMPTY_AT_START
FORMAL_PERFORMANCE_RUNS=0_AT_START
SERVER_HOST=hwnput3
SERVER3_ALIAS=cann-server3; DNS_UNRESOLVED_FROM_THIS_RUNTIME
FREE_HBM_MB=0:5312,1:4044,2:1440,3:4076,4:5121,5:1486,6:1487,7:40758
HOST_AVAILABLE_DISK=629G
HOST_MEMORY_AVAILABLE=736GiB
NPU_JOBS_STARTED_BY_MAIN=0
EXISTING_VLLM_AND_OTHER_PROCESSES=OBSERVED_ONLY; NOT_STOPPED_OR_MOVED
PUSH_STATE=PUSH_PENDING; no bounded push attempted in this startup event
```

This is a startup receipt only. It records no Build, Correctness, Local score,
Online-ready package, or Official result until the corresponding route-local
evidence is committed.

## Event receipt — declarations and diagnostic start — 2026-10-02T15:26Z

```text
ADDR_V001=DECLARED; HYPOTHESIS=ADDR-H3; DECLARATION_COMMIT=3353747f2b1ca7a4cf2c3caa026d2d3f91f0cc10
ADDR_SOURCE=WORKING_TREE_UNCOMMITTED; BUILD=NOT_STARTED; CORRECTNESS=NOT_STARTED; FORMAL_LOCAL=NOT_STARTED
BRANCH_V001=DECLARED; HYPOTHESIS=HBH-10; DECLARATION_COMMIT=2fa435dad89a0231b536ed662c089ef828023cb8
BRANCH_SOURCE=NOT_YET_WRITTEN; BUILD=NOT_STARTED; CORRECTNESS=NOT_STARTED; FORMAL_LOCAL=NOT_STARTED
UB_DIAG_2=HARNESS_CREATED_IN_WORKTREE; CANDIDATE_SOURCE=UNCHANGED; V002=FORBIDDEN
FORMAL_PERFORMANCE_RUNS=0
ONLINE_QUEUE=EMPTY
LOCAL_SCORE=NONE
```

The ADDR and BRANCH declarations are separate local commits and precede their
Candidate source edits. UB-DIAG-2 currently has only route-local support files
in its worktree; no diagnostic result or classification is claimed yet.

```text
PUSH_WINDOW_2026-10-02T15:27Z=TIMEOUT_20S
PUSH_COMMAND=git push origin w2/main2/control
PUSH_PENDING=YES
REMOTE_SYNC=UNCONFIRMED
FORCE_PUSH=0
RETRY_LOOP=0
LOCAL_EXPERIMENT_GATE=NO
```

## Event-driven implementation checkpoint — 2026-10-02T16:32Z

This checkpoint supersedes the stale startup-only stage fields above for the
three active contexts. It is a local control record; it does not alter the
canonical shared ledgers or the Official champion.

```text
CONTROL_HEAD_BEFORE_CHECKPOINT=68d02d92
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN=15e7d8dc0a9269fdcdb09af0d1b9e4660c070778
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
EVENT_DRIVEN_PIPELINE=ENABLED
BATCH_GATE=DISABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

| Lane | Revision / parent | Build | Correctness | Formal Local | Online | Current evidence |
|---|---|---|---|---|---|---|
| ADDR / `HOTLOOP-ADDR-HOIST-CHAMPION-X` | V001 / R31B V011 (`a8c19a…15e3`), ADDR-H3 | `BUILD_FAILED` after two preserved attempts; both fail in generated host registration with missing `<vector>` | NOT_RUN | NOT_ELIGIBLE | NOT_AUTHORIZED | route HEAD `558196b8`; `BUILD-ATTEMPT-1.md`, `BUILD-RETRY-1.md`, `local-result.json` |
| BRANCH / `HOTLOOP-BRANCH-HOIST-CHAMPION-X` | V001 / R31B V011 (`a8c19a…15e3`), HBH-10 | `PASS` after harness-only ABI fix; parent/candidate shared libraries produced | NEXT IMMEDIATELY | NOT_STARTED | NOT_AUTHORIZED | route HEAD `ee6a3532`; `support/build-fix-001.log`, `build-result.json` |
| UB / `UB-LIFETIME-SAFE-CHAMPION-X` | preserved V001 / exact R31B V011 | diagnostic harness `PASS` | FP16/BF16 cases PASS; FP32-wide-16384 parent and candidate both unstable | NOT_ELIGIBLE | NOT_AUTHORIZED | route HEAD `12e67f3b`; final class `PARENT_INSTABILITY` |

The ADDR retry is a build-chain fact, not a performance or correctness
verdict. A bounded support-only include-path Build Fix is being evaluated; no
H3 kernel change, H1 sibling, or V002 is authorized from this checkpoint.
The BRANCH Build PASS immediately opens its independent Correctness stage; it
does not wait for ADDR. UB remains closed as a diagnostic and cannot create
V002. `ONLINE_QUEUE=EMPTY`; `FORMAL_PERFORMANCE_RUNS=0`.

```text
SERVER_HOST=hwnput3
SERVER3_ALIAS=cann-server3; DNS_UNRESOLVED_FROM_THIS_RUNTIME
HBM_GATE=NO_BLOCK_RECORDED_AT_LAST_ROUTE_PREFLIGHT
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
GITHUB_PUSH=PUSH_PENDING; not a local-stage gate
```

## Event receipt — ADDR Build PASS / BRANCH timing preparation — 2026-10-02T17:00Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=c8bf65bd
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

ADDR V001 H3 has now crossed Build: the third bounded support fix propagates
the HCC C++ include environment to the generated ASCPLUGIN host compiler.
Evidence is route commit `47e01be4`, with executable SHA256
`2b2390ae67e74cc3474e9a0cf77f496628feb825bdd99891f2903157d0d506c8` and
`BUILD_RC=0`. Candidate source remains H3-only with SHA256
`26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602`.
ADDR Correctness is the immediate independent next event; no timing or V002
has started.

BRANCH V001 remains Correctness PASS at `e1346fce` and is preparing its formal
paired timing harness. No timing samples or Local score have yet been
committed. UB remains closed as `PARENT_INSTABILITY` at `12e67f3b`.

```text
EVENT_DRIVEN_PIPELINE=ENABLED
ONLINE_QUEUE=EMPTY
FORMAL_PERFORMANCE_RUNS=0_AT_RECEIPT
DIRECT_ONLINE_SUBMISSION=0
GITHUB_PUSH=PUSH_PENDING; not a local-stage gate
```

## Event receipt — BRANCH correctness / ADDR bounded build fix — 2026-10-02T16:52Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=ffc2bf90
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
DIRECT_ONLINE_SUBMISSION=0
ONLINE_QUEUE=EMPTY
```

| Route | Revision | Latest event | Evidence / next event |
|---|---|---|---|
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V001 / ADDR-H3 | `BUILD_FAILED` after bounded HCC include and environment propagation fix; generated ASCPLUGIN host unit still reports `<vector>` missing | route HEAD `8ef756a3`; `BUILD-FIX-1.md`; Correctness and timing prohibited; no V002 |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | V001 / HBH-10 | `CORRECTNESS_PASS`; 8/8 Parent/Candidate cases pass, including BF16 tail geometry | route HEAD `e1346fce`; `correctness-result.json`; Formal Local Performance starts immediately on exclusive device 7 |
| UB-LIFETIME-SAFE-CHAMPION-X | preserved V001 / diagnostic | closed `PARENT_INSTABILITY` | route HEAD `12e67f3b`; no Candidate change, V002, timing, or Online |

BRANCH correctness source identity remains exact: Parent
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, Candidate
`4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b`.
The formal stage is independent of ADDR and does not wait for its build
blocker. No Local score is recorded until the paired timing evidence is
complete; `FORMAL_PERFORMANCE_RUNS` remains `0` at receipt creation.

```text
SERVER_DEVICE_FOR_BRANCH_FORMAL=7
DEVICE_SNAPSHOT_SOURCE=V001 correctness snapshot at 2026-10-02T16:40Z
DEVICE7_HBM_USED_MB=24789/65536; FREE_HBM_APPROX_MB=40747
PROCESS_ACTIONS=NONE; existing users untouched
GITHUB_PUSH=PUSH_PENDING; not a local-stage gate
```

## Event receipt — BRANCH timing Build PASS / ADDR correctness harness Build failure — 2026-10-02T17:35Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=ee491bb1
ORIGIN_MAIN_FETCH=PASS; OBSERVED_ORIGIN_MAIN=62695ef8254c1bba61ac8534c15d23e70cf36588
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
EVENT_DRIVEN_PIPELINE=ENABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
ONLINE_QUEUE=EMPTY
```

BRANCH V001 HBH-10 timing support build passed after the bounded host C++
include-path fix. Route commits `94f85948` and `d8d6e4ef` are the support fix
and independent Build evidence. Exact identities in the pass log remain:
Parent source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`,
Candidate source SHA256
`4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b`, and
timing executable SHA256
`dadafae6835854a799f4fc63ca1c5ae45c4495500ed18bfd9ac7f141c0ea9e61`.
The next event is same-binary qualification; no Formal Local samples or Local
verdict exist yet.

ADDR V001 H3 correctness support harness Build then failed at route commit
`2fc0ad83`: `correctness_runner.cpp:149` rejects the support-only C-style
conversion to the `__gm__ uint8_t*` kernel ABI. Candidate H3 source is
unchanged (SHA256
`26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602`), so
Correctness is `NOT_RUN`, Formal Local is `NOT_ELIGIBLE`, and the next event is
a support-only cast fix followed by an immediate rebuild. This is not an H3
failure and does not authorize ADDR-H1 or V002.

```text
BRANCH_BUILD=PASS; BRANCH_CORRECTNESS=PASS; BRANCH_TIMING_BUILD=PASS
BRANCH_SAME_BINARY=NOT_STARTED; BRANCH_FORMAL_LOCAL=NOT_STARTED
ADDR_BUILD=PASS; ADDR_CORRECTNESS_HARNESS_BUILD=FAILED; ADDR_CORRECTNESS=NOT_RUN
ADDR_FORMAL_LOCAL=NOT_ELIGIBLE
UB_CLASS=PARENT_INSTABILITY; UB_V002=FORBIDDEN
FORMAL_PERFORMANCE_RUNS=0_AT_RECEIPT
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```

## Event receipt — ADDR isolated D40960 diagnostic Build PASS — 2026-10-02T18:15Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=58f9fc41
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN_OBSERVED=62695ef8254c1bba61ac8534c15d23e70cf36588
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
EVENT_DRIVEN_PIPELINE=ENABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
```

ADDR V001 H3 added only a correctness-harness binary-selection filter so the
previous BF16-D40960 synchronization blocker can be isolated to exact Parent
and Candidate launches. The support diagnostic Build passed at route commit
`4e045d1a`; H3 Candidate source remains unchanged at SHA256
`26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602`, with
the exact R31B V011 Parent SHA256
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
Correctness is still `INCOMPLETE` pending the isolated Parent-only and
Candidate-only D40960 runs; no Formal Local timing is allowed yet. The support
filter is not a second performance mechanism and does not authorize ADDR-H1
or V002.

```text
ADDR_BUILD=PASS; ADDR_CORRECTNESS_HARNESS_DIAG_BUILD=PASS
ADDR_D40960_PARENT_RUN=NOT_RECORDED; ADDR_D40960_CANDIDATE_RUN=NOT_RECORDED
ADDR_CORRECTNESS=INCOMPLETE; ADDR_FORMAL_LOCAL=NOT_ELIGIBLE
BRANCH_LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL; BRANCH_ONLINE_QUEUE=EMPTY
UB_CLASS=PARENT_INSTABILITY; UB_V002=FORBIDDEN
FORMAL_PERFORMANCE_RUNS=1_SHAPE
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```

## Event receipt — BRANCH BF16-D32768 formal Local complete — 2026-10-02T18:01Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=cf6c527f
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN_OBSERVED=62695ef8254c1bba61ac8534c15d23e70cf36588
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
EVENT_DRIVEN_PIPELINE=ENABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
```

BRANCH V001 HBH-10 completed the only shape that passed the alternate-device
qualification: BF16-D32768 on device 4. The formal run is recorded in route
commit `c3c6b4c6`, returned `0`, and preserved 248 device-event samples (124
Parent / 124 Candidate), four interleaved blocks in both orders, raw TSV,
stdout, source identities, lease lock, and before/after load snapshots. Exact
identities are Parent source SHA256
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, Candidate
source SHA256
`4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b`, and
performance executable SHA256
`dadafae6835854a799f4fc63ca1c5ae45c4495500ed18bfd9ac7f141c0ea9e61`.

The four block median Candidate-vs-Parent deltas were `-1.267609%`,
`+0.071586%`, `-0.141244%`, and `-0.636044%`. Three blocks were negative but
the direction and magnitude were mixed; the same-binary noise floor was not
established, with resident d4 VLLM/Python processes observed and unchanged at
93% HBM. Main classification is `NEEDS_ONE_MORE_LOCAL`, not
`LOCAL_ACCEPTED` or `LOCAL_REJECTED`; no Local score is claimed and the
Online queue remains empty. The next possible action is one bounded additional
formal run only after review. HBH-09/V002 remains forbidden.

```text
BRANCH_BUILD=PASS; BRANCH_CORRECTNESS=PASS; BRANCH_FORMAL_LOCAL=COMPLETE
BRANCH_LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL; BRANCH_LOCAL_SCORE=NONE
BRANCH_ONLINE_READY=NO; BRANCH_ONLINE_QUEUE=EMPTY
BRANCH_V002=FORBIDDEN
FORMAL_PERFORMANCE_RUNS=1_SHAPE
ADDR_CORRECTNESS=INCOMPLETE_D40960_DIAGNOSTIC_PENDING
UB_CLASS=PARENT_INSTABILITY; UB_V002=FORBIDDEN
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```

## Event receipt — ADDR correctness partial pass / BRANCH qualification retry blocked — 2026-10-02T17:50Z

```text
CONTROL_HEAD_BEFORE_RECEIPT=dbf3586d
CANONICAL_MAIN_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN_OBSERVED=62695ef8254c1bba61ac8534c15d23e70cf36588
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
EVENT_DRIVEN_PIPELINE=ENABLED
DIRECT_ONLINE_SUBMISSION=FORBIDDEN
ONLINE_QUEUE=EMPTY
```

ADDR V001 H3 correctness support Build passed after the runner-only GM
address-cast fix (`72dac4a0`, Build evidence `af6b1fd2`). The first execution
was a loader-only failure because the HCC runtime selected a `libstdc++.so.6`
without `GLIBCXX_3.4.29`; this was corrected only in the execution environment
by putting system runtime libraries first. The rerun then passed Parent and
Candidate with exact output equality on eight cases: FP16 widths 12288,
16384, 24576, 32768 and BF16 widths 12288, 16384, 24576, 32768-tail.
Parent/Candidate source identities remained exact: parent SHA256
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, H3
candidate SHA256
`26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602`, runner
SHA256 `fe4607229e2a962e5d2d39dcc4b510cb13d7cf0b6ce265bf875c470e4eb3f770`.
The same run stopped at the next BF16 width 40960 with
`aclrtSynchronizeStream=507035`; therefore ADDR correctness is
`INCOMPLETE`, not PASS or Candidate failure. Timing remains prohibited. The
next event is an exact single-case/support diagnostic for D40960, with no H3
source change.

BRANCH V001 HBH-10 timing qualification retry `5848a7ed` also failed to
qualify any of the four target shapes on device 7. FP16-16384 and BF16-20000
were protocol-blocked; FP16-32768 was protocol-blocked with block drift
0.392333; BF16-32768 was `NEEDS_VALIDATION` with MAD/median 0.170151 and
drift 0.033871. No Parent/Candidate paired samples ran and no Local score is
claimed. The retry's load snapshot recorded HBM 37% and AICore 0% before the
run, with the existing d7 `python3` process retained. This is
`MEASUREMENT_BLOCKED` with retry required, not `LOCAL_REJECTED`; V002/HBH-09
remains forbidden until V001 has a clear verdict.

```text
ADDR_BUILD=PASS; ADDR_CORRECTNESS=INCOMPLETE; ADDR_FORMAL_LOCAL=NOT_ELIGIBLE
ADDR_V002=FORBIDDEN
BRANCH_BUILD=PASS; BRANCH_CORRECTNESS=PASS; BRANCH_SAME_BINARY=BLOCKED_RETRY_2
BRANCH_FORMAL_LOCAL=NOT_STARTED; BRANCH_LOCAL_SCORE=NONE
BRANCH_VERDICT=MEASUREMENT_BLOCKED; BRANCH_V002=FORBIDDEN
UB_CLASS=PARENT_INSTABILITY; UB_V002=FORBIDDEN
FORMAL_PERFORMANCE_RUNS=0_AT_RECEIPT
PUSH_STATE=PUSH_PENDING; GITHUB_PUSH_IS_LOCAL_STAGE_GATE=NO
PROCESS_ACTIONS=NONE; existing VLLM/Ray/Python processes untouched
```
