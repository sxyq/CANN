# Local Judge V4 — Ranking Validation

Mode: CALIBRATED_SURROGATE. Models A/B/C use only core Suite V2 cases C13/C14/C16/C12: arithmetic mean, geometric mean, and fresh-V011-noise-weighted geometric mean of their local latency ratios. The three diagnostic-only cases never enter the models. Fixed model features avoid label-based feature selection in the V4 outer folds.

## Suite and near-Champion definition

- Official candidate labels: 12 revisions across 11 Routes; Champion anchor R31B/V011 remains 45.16 and is not a candidate fit label.
- Near-Champion band is the top quartile of the 12 Official candidate scores: 3 versions; cutoff 44.24. The cutoff is derived from the observed score distribution.

## Nested version-level LOOCV — Ensemble mean of A/B/C raw predictions

- n=12; MAE=6.462037; RMSE=8.837852; Spearman=0.678322; Kendall=0.454545.
- Pairwise order accuracy=0.727273 (66 pairs). Near-Champion pairwise=0.333333 (3 pairs).
- TOP1 hit=0; TOP3 precision/recall=0.666667/0.666667; TOP5 precision/recall=0.8/0.8.
- Raw false positives=2; false-positive rate=0.166667; actual scores above Champion=0.

## Leave-one-Route-out — Ensemble

- n=12; MAE=5.281435; RMSE=7.198079; Spearman=0.734266; Kendall=0.545455.
- Pairwise order accuracy=0.772727 (66 pairs). Near-Champion pairwise=0.333333 (3 pairs).
- TOP1 hit=0; TOP3 precision/recall=0.666667/0.666667; TOP5 precision/recall=0.8/0.8.

## Ranking-level assessment

- Judge level assessment=LEVEL_1_LIMITED_RANKING_SIGNAL. The evaluator does not automatically assign a level; Main must weigh overall, route-held-out, TOP-K, and near-Champion evidence.
- Level 2 authorizes only limited local exploration, not automatic Online submission. Level 3 requires reliable near-Champion ordering; inspect its pair count and accuracy rather than overall rank alone.
- No actual candidate label exceeds 45.16, so positive-class sensitivity remains unknown. Conservative V3 margins are not used as ranking scores.

- MODEL_A_MEAN_RATIO LOO: MAE=6.472534, Spearman=0.678322, pairwise=0.727273, near=0.333333, TOP3 recall=0.666667.
- MODEL_A_MEAN_RATIO Route-out: MAE=5.252305, Spearman=0.699301, pairwise=0.757576, near=0.333333, TOP3 recall=0.666667.
- MODEL_B_GEOMEAN_RATIO LOO: MAE=6.532014, Spearman=0.65035, pairwise=0.712121, near=0.333333, TOP3 recall=0.333333.
- MODEL_B_GEOMEAN_RATIO Route-out: MAE=5.360681, Spearman=0.706294, pairwise=0.757576, near=0.333333, TOP3 recall=0.333333.
- MODEL_C_BASELINE_STABILITY_WEIGHTED LOO: MAE=6.489025, Spearman=0.65035, pairwise=0.712121, near=0.333333, TOP3 recall=0.333333.
- MODEL_C_BASELINE_STABILITY_WEIGHTED Route-out: MAE=5.319687, Spearman=0.734266, pairwise=0.772727, near=0.333333, TOP3 recall=0.666667.
- ENSEMBLE LOO: MAE=6.462037, Spearman=0.678322, pairwise=0.727273, near=0.333333, TOP3 recall=0.666667.
- ENSEMBLE Route-out: MAE=5.281435, Spearman=0.734266, pairwise=0.772727, near=0.333333, TOP3 recall=0.666667.
