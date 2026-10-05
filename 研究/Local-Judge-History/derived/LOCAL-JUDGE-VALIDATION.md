# Local Judge V1 Validation

## Result

`LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.
The public API exposes testcase IDs and tbest values, but not the exact 15-case shape/dtype/input mapping. A complete Local→Online held-out prediction cannot be claimed from the current evidence.

- Official result files scanned: 117
- Passing Official rows eligible for formula replay: 64
- Local→Online rows with comparable full-suite features: 0
- Historical explicit false positives in the calibration ledger: 7
- Historical explicit false negatives in the calibration ledger: 1
- Formula replay MAE: 0.002818203 points (diagnostic only)
- Formula replay RMSE: 0.003451365 points (diagnostic only)
- Formula replay rank correlation: 0.999954 (same-payload replay; not predictive validation)
- Local→Online MAE/RMSE/rank correlation: NOT_COMPUTABLE
- Champion decision accuracy / held-out false-positive rate: NOT_COMPUTABLE for a Local model; exact suite features are missing.

## Why This Is Not a Successful Predictor Yet

The existing local observations are single-shape or route-specific paired probes. They do not provide 15 local timings with a known case mapping. Fitting a model from the Official per-case payload would leak the label and would not measure Local→Online prediction. Therefore all local calibration rows are descriptive-only and no train/validation fit is reported.

## ADDR H3

- Old local engineering delta: -24.193320%
- Old local quality: POOR; historical Candidate faster 1/3; extreme sample dominates.
- V1 predicted Official score: UNAVAILABLE (case coverage gate; no exact 15-case local vector).
- V1 gate result: ONLINE_ELIGIBLE=NO; a missing prediction cannot become a false positive.
- Actual Official score: 42.72 (15/15 PASS).
- Per-case ADDR H3 timing/score details: MISSING in the retained result package; no case-level gain/loss is synthesized.
- Conclusion: V1 does not yet numerically predict ADDR H3; it correctly blocks the old single-shape signal from being used as an Online gate.

## Formula Replay Scope

The replay uses s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i) with the best_time values stored in each historical result payload. It checks arithmetic consistency only. It does not validate shape coverage, local noise handling, throughput consistency, or candidate ranking.

## Required Next Calibration Evidence

1. Recover exact 15-case shape/dtype/input definitions or obtain a sanctioned local runner that emits the same ordered cases.
2. Run fresh R31B V011 and each candidate on that exact suite with the same binary, device class, warmup, sample policy, and throughput capture.
3. Reserve at least two Official-labelled revisions as held-out validation before fitting any local-to-online mapping.
4. Until then, keep all new performance revisions and Online submissions frozen at Planning review.
