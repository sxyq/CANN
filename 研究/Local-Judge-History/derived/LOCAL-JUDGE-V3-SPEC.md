# Local Judge V3 Specification

Mode: CALIBRATED_SURROGATE. Online hidden-case reproduction remains PARTIAL; this tool predicts an Official-score range proxy and must not be described as an exact Online reproduction.

## Inputs and invariants

- Training labels are the 12 Official-tested candidate revisions in LOCAL-JUDGE-CALIBRATION-DATASET-V2.tsv. R31B/V011 is the fixed local and Official Champion anchor (45.16), not a candidate training target.
- Local features are the same-suite candidate-to-fresh-V011 latency ratios for the 16 V1 cases. Missing features remain missing; the fitter never fills them from another shape or old parent logs.
- Per-run stability comes only from the completed V011 and candidates-blocks measurement roots. The interrupted candidates run is excluded. Per-run device-load sensitivity is unavailable.

## Nested procedure

For each outer held-out version or Route, exclude its complete group. Within the remaining training groups, grouped leave-one-Route-out predictions compare TOP-1..TOP-5 and the stability-filtered ensemble, then select one of five simple models. TOP-1 is measured as a baseline but is ineligible for the selected V3 pipeline: every selected model must use at least 3 cases. The selected pipeline is fitted only on the outer training set.

The safety margin is the largest positive overprediction among nested, route-held-out predictions generated entirely inside the outer training set: max(0, predicted - actual). Conservative score = predicted score - this margin. This is empirical worst validated overprediction, not a statistical guarantee.

Models: mean-ratio linear baseline, geometric-mean-ratio linear baseline, stability-weighted geometric baseline, fixed-ridge regression (lambda=1), and monotone isotonic mapping when data support it. Configuration selection orders false-positive count first, then MAE, rank correlation, and smaller feature count.

## Gate

ONLINE_ELIGIBLE requires correctness PASS, at least 3 stable selected cases, Conservative Score > 45.16, and at least one above-Champion Official training label. Since the current 12 historical candidate labels contain zero above-Champion outcomes, sensitivity is unvalidated and V3 currently returns NO for submission eligibility.

## Commands

- node 工具/main2-local-judge-v3.mjs validate-all
- node 工具/main2-local-judge-v3.mjs validate-loocv
- node 工具/main2-local-judge-v3.mjs validate-route-out
- node 工具/main2-local-judge-v3.mjs audit-false-positive
- node 工具/main2-local-judge-v3.mjs predict --input candidate-ratios.tsv

LOCAL-JUDGE-MODEL-COMPARISON.tsv records TOP-1..TOP-5 and stability-filtered baseline comparisons for inner configuration selection only; it is not an outer validation result.

Suite V2 currently has 7 provisional cases. This is an evaluation/benchmark proposal only; it does not establish predictive readiness.
