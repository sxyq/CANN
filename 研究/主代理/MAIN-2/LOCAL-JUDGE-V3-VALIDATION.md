# Local Judge V3 Validation

- V2 feature-selection leakage: YES. C13 was selected using all 12 Official candidate labels; its V2 validation metrics are OPTIMISTIC / NON-NESTED and are not accepted as formal validation.
- Calibration: 12 Official candidate revisions, 11 Routes; Champion anchor R31B/V011 Official 45.16 is excluded from candidate fit.
- Fresh V011 local vector is available for 15/16 suite cases. C15 (FP32 D32768) remains missing/invalid and is never imputed.
- No NPU re-run, Kernel change, or Online submission was performed. Interrupted candidates-run data was excluded; C15 remains missing where raw evidence is absent/invalid.

## Strict nested version-level LOOCV

- Computable rows: 12/12.
- Predicted-score MAE=6.826865; RMSE=10.376418; Spearman=0.545455; Kendall=0.333333.
- Raw-score champion false positives=0; conservative-gate false positives=0; conservative false negatives=0; conservative decision accuracy=1.
- Actual labels above Champion=0; false-negative sensitivity is therefore not empirically testable.
- Every raw held-out prediction is at or below Champion. The zero-FP count is a reject-all outcome, not evidence that V3 can identify a true winner.
- Nested overprediction-margin range across outer folds: 16.55 to 24.606842 Official points.

## Strict leave-one-Route-out

- Computable rows=12/12 across 11 Routes.
- Predicted-score MAE=6.792421; RMSE=10.372084; Spearman=0.545455; Kendall=0.333333.
- Raw-score champion false positives=0; conservative-gate false positives=0; conservative false negatives=0; conservative decision accuracy=1.
- Every raw held-out prediction is at or below Champion; false-negative sensitivity is unavailable because no actual candidate exceeds Champion.

## False-positive / hard-held-out cases

- Seven historical explicit false positives were audited. Strict same-suite held-outs are reported where a vector exists; missing vectors are NOT_COMPUTABLE, never inferred from single-shape Local deltas.
- ADDR H3: prediction=44.51; actual=42.72; conservative=20.04; correctly rejected=YES.
- VECTOR-MATH-X/V001: prediction=44.116; actual=44.22; conservative=22.836; correctly rejected=YES.
- Strict held-out means each case's Official label was excluded from case selection, weights, fitting, and margin calibration.

## C13 and proposed suite

- C13 is FP16, 128x16384, WIDE_MULTIROW_FULL; full-label association is descriptive only. Route-averaged Spearman=-0.445455; removing one Route changes association direction=NO. See C13-FORENSIC.md.
- Proposed V2 suite (7): C14[core-provisional], C13[core-provisional], C16[core-provisional], C11[diagnostic-only], C01[diagnostic-only], C12[core-provisional], C08[diagnostic-only]. Diagnostic-only cases failed stability and must not enter model features.

## Readiness

- LOCAL_JUDGE_READY=NO; READY_FOR_PERFORMANCE_WAVE=NO.
- MORE_OFFICIAL_LABELS_NEEDED=YES. Current candidate labels include no result above 45.16; route-held-out ranking and sensitivity therefore cannot validate winner selection.
- Information candidates are recorded in ONLINE-CALIBRATION-CANDIDATES.tsv with missing common-vector/model-disagreement fields explicitly NA. They are not Online-submission recommendations until the unified suite is measured and Planning approves quota.
- Gate failure is intentional: formula alignment and a computable nested pipeline do not demonstrate useful Local-to-Online predictive power.
