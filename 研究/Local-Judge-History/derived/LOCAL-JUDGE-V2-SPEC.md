# Local Judge V2 Specification

## Status

`LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`; Online hidden cases are only partially reproducible.
- Unified Suite V1 cases: 16
- Retained Suite V2 cases: 1
- Fresh R31B V011 valid cases: 15
- LOO candidate rows: 12
- MAE: 7.106473; RMSE: 9.283397
- Spearman: 0.321678; Kendall: 0.121212

## Input Contract

- Model case IDs: C13
- Raw local log-ratio Spearman: -0.482517; Kendall: -0.272727
- Champion decision accuracy: 11/12
The scorer consumes the same suite, device-event protocol, warmup, sample policy, source identity, correctness status, and load evidence for every version. It aggregates the median of three independent run medians and never uses a best-of-run sample.

## Surrogate Model

Per-version features are local candidate/anchor ratios from the retained V2 case set. The primary LOO prediction is a simple linear mapping from the retained-case mean log ratio to Official score, trained without the held-out version. Formula-scaled, nearest-neighbor, and monotonic alternatives are retained in the cross-validation table.
The V2 case selection is exploratory over the full historical calibration set; it is not nested inside each held-out fold. This prevents claiming final predictive readiness from this small sample.

The exact Official formula remains: s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)

## Gate

LOCAL_JUDGE_READY=NO
Online eligibility remains NO until the validation error, false-positive behavior, correctness, source identity, and Planning/Judge-owner decision all pass.

## Failure Handling

A failed build, correctness-invalid case, missing run, or source mismatch is recorded as NA/invalid. High jitter remains an explicit quality flag and is excluded from the retained-case decision when its historical median CV is too high; no old Parent timing is copied into a fresh feature vector.
