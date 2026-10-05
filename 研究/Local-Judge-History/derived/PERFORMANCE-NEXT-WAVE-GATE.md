# Main-2 Judge V4 and Performance Restart Gate

Date: 2026-10-05

```text
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
KERNEL_SOURCE_CHANGED=NO
ONLINE_SUBMISSIONS=0
PLANNING_DECISIONS_CHANGED=0
MAIN_SELECTED=NONE
```

## Candidate calibration execution

The five information candidates were built from exact source identities and
the same V011 parent. All five builds passed, and all five candidates passed
the seven-case unified correctness suite (C13, C14, C16, C12 plus diagnostic
C01, C11, C08). No candidate source was edited.

| Candidate | Build | Correctness | Formal local vector | Status |
|---|---|---|---|---|
| HOTLOOP-ADDR-HOIST-CHAMPION-X/V002 | PASS | 7/7 PASS | 0/4 core cases | measurement blocked |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X/V001 | PASS | 7/7 PASS | 0/4 core cases | measurement blocked |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X/V001 | PASS | 7/7 PASS | 0/4 core cases | measurement blocked |
| SCHED-CHAMPION-X/V001 | PASS | 7/7 PASS | 0/4 core cases | measurement blocked |
| STORE-EPILOGUE-X/V001 | PASS | 7/7 PASS | 0/4 core cases | measurement blocked |

The formal Parent same-binary qualification failed its bounded two-attempt
window on d4 for all seven ADDR cases. One safe-device retry on d5 also failed
the same qualification. No Parent/Candidate timing pair was accepted, so no
candidate receives a Local score or information-gain rank. This is an honest
apparatus/protocol block, not a performance rejection.

## Judge V4 historical validation

V4 uses fixed core features C13/C14/C16/C12. Diagnostic cases never enter the
models. Historical labels are the 12 Official-tested revisions; R31B/V011 is
the 45.16 anchor and is not fit as a candidate label.

| Split | MAE | RMSE | Spearman | Kendall | Pairwise | TOP-3 recall | Near-Champion pairwise |
|---|---:|---:|---:|---:|---:|---:|---:|
| Nested version LOO | 6.462037 | 8.837852 | 0.678322 | 0.454545 | 0.727273 (66) | 0.666667 | 0.333333 (3) |
| Leave-one-Route-out | 5.281435 | 7.198079 | 0.734266 | 0.545455 | 0.772727 (66) | 0.666667 | 0.333333 (3) |

The historical data supports a limited broad ranking signal, but the
near-Champion sample is too small and too inaccurate for reliable screening
near 45.16. The appropriate current assessment is `JUDGE_LEVEL=1`: useful for
rejecting obviously poor candidates and research prioritization only. It is
not an Online gate and not a near-Champion selector.

## Performance restart gate

Planning may consider at most two new OFAT revisions after an explicit
selection. Main-2 has not selected either revision. The current ranking for
Planning review is conditional:

1. `H1 MAIN2-TB-PARAM-DMA-COALESCE-D8192-FP16` remains TOP-1. It reduces the
   FP16 full-row parameter load calls from four to two without changing bytes,
   but is adjacent to COEFF-LOCALITY/R014 and needs a novelty decision plus
   exact codegen evidence.
2. `H3 MAIN2-TB-NARROWMID-UNUSED-PARAM-EVENT` remains TOP-2. The API audit
   confirms real event-pool allocation/release bookkeeping, but reachability,
   lifecycle proof, and generated codegen remain required.

H2 remains below them because its selector runs only during initialization and
has no demonstrated emitted cost. `MAIN_SELECTED=NONE` and
`IMPLEMENTATION_AUTHORIZED=NO` remain in force. No V001 or new performance
revision was created in this window.

## Server and Git

Formal timing used one device at a time. d4 and d5 were admitted because free
HBM was above the 100 MB hard threshold; no external process was stopped or
migrated. All leases were released. The initial build environment and runner
compatibility issues were corrected only in the V4 tool: host C++ include
paths and Ascend runtime library initialization. Candidate source bytes were
unchanged.

Remote synchronization completed after the bounded retry. The calibration
evidence head `baade9a48cc0aacded17043492f2fb8d21c4f0b0` was verified, and the
follow-up status head `bdd23bffb235347873d171c64fb747035799dd5` was also pushed
and verified.

```text
PERFORMANCE_WAVE_READY=NO
ONLINE_READY=NONE
REMOTE_SHA_VERIFIED=YES
LAST_PUSH_ACKNOWLEDGED_EVIDENCE_HEAD=1825d393
FINAL_STATUS_ONLY_COMMIT=97571aeb
FINAL_STATUS_PUSH=PUSH_PENDING_NETWORK
READY_FOR_PLANNING_REVIEW=YES
```

## Engineering calibration vector completion

```text
CALIBRATION_TIMING_MODE=ENGINEERING_3RUN
ENGINEERING_VECTOR_COMPLETE=5/5
CORE_CELLS_COMPLETE=20/20
DIAGNOSTIC_CELLS_COMPLETE=15/15
RUNS=105/105_VALID
DEVICE_PRIMARY=4
DEVICE_FALLBACK=7_NOT_USED
FORMAL_QUALIFICATION_IS_GATE=NO
```

All five candidates used the same V011 parent, runner, Suite V2, warmup 45,
31 device-event samples per run, and interleaved order `P-C/C-P/P-C`. Every
cell has three valid runs. The old formal qualification results remain in the
old evidence files and are retained as quality context; they did not block
these engineering vectors.

| Quality | Cells |
|---|---:|
| GOOD | 12 |
| FAIR | 8 |
| POOR | 15 |
| INVALID | 0 |

Engineering vectors are calibration inputs, not Formal Local Scores and not
`LOCAL_ACCEPTED`. The 15 POOR cells remain in the dataset; no fastest-run or
directional sample was removed. Throughput is recorded for diagnosis and is
not inserted into the Official formula.

## Information-gain label recommendations

These are recommendations for Planning to decide whether to acquire new
Official labels. Main-2 did not submit anything Online.

| Priority | Version | Predicted score signal | Model spread | Feature distance | Quality |
|---:|---|---:|---:|---:|---|
| 1 | HOTLOOP-BRANCH-HOIST-CHAMPION-X/V001 | 35.686 | 1.258 | 1.821 | 3 GOOD / 1 FAIR / 3 POOR |
| 2 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X/V001 | 39.333 | 1.129 | 1.572 | 2 GOOD / 1 FAIR / 4 POOR |
| 3 | HOTLOOP-ADDR-HOIST-CHAMPION-X/V002 | 42.853 | 0.330 | 1.143 | 3 GOOD / 1 FAIR / 3 POOR |

The displayed predictions are `LEVEL_1_RANKING_SIGNAL_NOT_OFFICIAL_SCORE`.
The fixed historical V4 uncertainty is 15.101 points; none of these rows is
an automatic Online gate or Champion claim. SCHED and STORE are complete but
outside the information-gain TOP-3 under the current route/diversity ranking.

Evidence: `CALIBRATION-CANDIDATES-VECTORS-V2.tsv`,
`ONLINE-CALIBRATION-CANDIDATES-V2.tsv`, and
`calibration-v4/runs/20261005-v4-engineering/`.
