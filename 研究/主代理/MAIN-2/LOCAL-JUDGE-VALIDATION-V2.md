# Main-2 Local Judge V2 Validation

Generated: 2026-10-04T20:11:59.047Z

## Boundary

- `LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.
- Online hidden case shape/dtype/input mapping remains `PARTIAL`; this is not an exact 15-case reproduction.
- Formula: s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)
- Throughput is retained as an anomaly/load feature and is not mixed into the Official score formula.

## Calibration Set

- Manifest versions: 13
- Historical candidates: 12
- Fresh anchor valid cases: 15
- Successfully benchmarked candidates with at least four valid cases: 12

## LOO Metrics

- Samples with primary prediction: 12
- MAE: 7.106473
- RMSE: 9.283397
- Model case IDs: C13
- Raw local log-ratio Spearman (lower ratio should mean higher Official score): -0.482517
- Raw local log-ratio Kendall: -0.272727
- Predicted-score Spearman: 0.321678
- Predicted-score Kendall: 0.121212
- Champion decision accuracy (threshold 45.16): 11/12 (91.667%)
- False positives (predicted > 45.16, actual < 45.16): 1
- False negatives (predicted <= 45.16, actual > 45.16): 0

## ADDR H3 Held-Out Check

- Old Local signal: -24.193320% (POOR, single-shape; not used as a vector feature).
- Predicted Official score: 44.917735
- Actual Official score: 42.72 (15/15 PASS).
- Correctly rejected below anchor: YES

## Case Predictiveness

Per-case correlation, coverage, and jitter are in `LOCAL-CASE-PREDICTIVENESS.tsv`. A case is not retained solely because it produced a local win.
- V2 model selection is exploratory over the current 13-version calibration set; it is not a nested held-out selection.

## Gate

- LOCAL_JUDGE_READY=NO
- Readiness blockers: retained case count 1 (<2), case selection is exploratory rather than nested, and false-positive count is 1.
- ONLINE_ELIGIBLE=NO until Planning/Review accepts the calibrated error and the external Judge owner approves a submission.
- A single-shape percentage cannot enter the Online gate.

## Leakage Controls

- R31B V011 is the fresh baseline anchor and is excluded from candidate training targets.
- Every candidate prediction leaves that candidate out of its linear/nearest/monotonic training set.
- Missing or correctness-invalid local cases are retained as NA, never filled from old Parent logs.
