# MAIN-2 Fresh R31B V011 Local Baseline — 2026-10-04

## Result

```text
OFFICIAL_CHAMPION=R31B V011
OFFICIAL_SCORE=45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANONICAL_LOCAL_SHAPE_SUITE=NOT_FOUND
CANONICAL_LOCAL_COMPOSITE_SCORE=NOT_FOUND
CROSS_ROUTE_SCORE_NOT_DIRECTLY_COMPARABLE=YES
FRESH_NPU_RUN_COUNT=12
DEVICE=7
LOCAL_SCORE_VERDICT=NO_NEW_CANDIDATE_VERDICT
ONLINE_ELIGIBLE=NO
DIRECT_ONLINE_SUBMISSION=0
```

The prior `MAIN2-BASELINE-CALIBRATION-20261004.md` and
`R31B-V011-LOCAL-BASELINE.tsv` reuse Parent observations extracted from older
Parent/Candidate paired runs. They remain historical evidence, but are not a
fresh Champion baseline. This report supersedes them for freshness; it does
not rewrite or discard their raw data.

## Fresh measurement protocol

- All four Parent targets used the exact R31B V011 source SHA above and device
  7 on `hwnput3` / Ascend 910B3 / CANN 8.5.0.alpha002.
- Each target had three separate Parent-only runner processes. Each executed
  45 warmups and 62 device-event samples (31 × 2 blocks); no Candidate kernel
  was invoked in these runs.
- Each process has timestamped pre/post `npu-smi info`, per-device HBM/AICore/
  AIVector readings, global NPU process inventory, host `uptime` load, runner
  identity, and raw samples in `fresh-v011-baseline-20261004/<ROUTE>/`.
- The project same-binary floor is MAD/median ≤ 0.10 and block drift ≤ 0.10.
  ADDR qualified once; BRANCH qualified once; UB did not qualify; REDUCE did
  not qualify. Failed/needs-validation windows are retained and are not used
  as evidence of a mechanism regression.
- `RUN*_THROUGHPUT = 1,000,000 / RUN*_US`. `AVG_THROUGHPUT` is the arithmetic
  mean of the three per-run throughputs; `THROUGHPUT_AT_AVG_US` is
  `1,000,000 / AVG_US`. The normalized-score table uses the latter convention
  for both sides.

## Baseline summary

| Route target | Fresh V011 run 1/2/3 (µs) | Aggregate (µs) | Throughput at aggregate (inv/s) | Quality |
|---|---:|---:|---:|---|
| ADDR BF16 [9,32768] reduced-tail | 29.340 / 31.466 / 35.593 | 32.133 mean | 31,120.655 | POOR |
| UB FP16 [1,32768] | 35.764 / 19.248 / 42.534 | 32.515 mean | 30,754.721 | POOR |
| BRANCH FP16 [80,6144] | 13.353 / 21.607 / 26.829 | 20.596 mean | 48,552.306 | POOR |
| REDUCE BF16 [1,32768] | 15.540 / 16.050 / 17.270 | 16.287 mean of run medians | 61,399.918 | POOR |

REDUCE uses the historical route metric (median of each run), not an invented
mean of raw samples. Its runner and raw format are the existing
`REDUCE-HIER-X/runner_ref.inc` event harness. The exact per-run data and source
identities are indexed in `R31B-V011-FRESH-LOCAL-BASELINE.tsv`.

## Shared load and measurement quality

Device 7 was selected after a live read showed HBM below 100%, no d7 lease,
and sufficient free HBM. During the measured sequence HBM was about 37–40%;
other Python processes remained resident. AICore/AIVector utilization changed
between snapshots (including AIVector-heavy intervals and a BRANCH run with
33% AICore), while host load averages were high. These are recorded facts;
no process was stopped, paused, migrated, or reset. Device 6's old unresolved
lease row was not used.

The same-binary results varied substantially between independent processes.
Consequently the fresh baselines are real NPU measurements but POOR-quality
calibration anchors. The normalized Candidate values reuse existing raw data
only where shape, dtype, runner logic, and event method match; they compare
different time windows and do not constitute a new interleaved Local verdict.
REDUCE additionally used device 5 for its old Candidate data, so that
comparison is only a partial device match.

The old Candidate scores were re-evaluated against the fresh baseline in
`MAIN2-FRESH-NORMALIZED-SCORES.tsv`. None is an Official score. The per-route
results do not form a cross-route leaderboard because their shapes and dtypes
differ.

## ADDR V001 Online gate

`HOTLOOP-ADDR-HOIST-CHAMPION-X V001/H3` is **not** Online eligible. The fresh
calibration is POOR, the historical paired timing made Candidate faster in
only 1/3 runs, the fresh normalized latency/throughput directions are adverse,
and BF16-D40960 remains a shared Parent/Candidate runtime blocker. The
separate identity audit confirms source identity, but its stale build/result
snapshot must not replace the later score/correctness evidence. See
`ADDR-V001-ONLINE-ELIGIBILITY-20261004.md`.

## Process and scope

One ADDR shell setup attempt failed before invoking a runner because nounset
rejected an unset `LD_LIBRARY_PATH`; it produced no NPU run and is retained in
the ADDR evidence folder. The corrected bounded retry and the other 11 runner
processes completed with raw output. The d7 lease is recorded and released in
`调度/服务器设备使用.tsv`.

No Candidate/kernel/runner source was changed, no Revision was created, no
score was promoted, no canonical performance ledger was rewritten, and no
Online submission was attempted. Official Champion remains R31B V011 / 45.16.
