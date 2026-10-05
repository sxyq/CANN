# C13 Forensic

- Case: FP16 rows=128 D=16384; path=WIDE_MULTIROW_FULL.
- V2 selected C13 after evaluating all 12 Official candidate labels; therefore that selection is leakage for V2 LOO metrics.
- Full-label exploratory association: Spearman(log local ratio, Official)=-0.482517; Kendall=-0.272727; n=12. This is descriptive only, not held-out validation.
- Run stability from the available 3-block measurements: CV=0.171872; MAD/median=0.226126; paired direction majority=0.666667. Per-run load sensitivity is unavailable.
- Route-averaged correlation (one point per Route)=-0.445455 across 11 Routes. This limits repeated-Store weighting but is still exploratory.
- Leave-one-Route-out C13 correlations: ASYNC-OVERLAP-CHAMPION-X=-0.536364; COEFF-LOCALITY-X=-0.536364; DTYPE-SPECIAL-X=-0.545455; HOTLOOP-ADDR-HOIST-CHAMPION-X=-0.627273; INTEGRATION-X=-0.472727; REDUCE-HIER-X=-0.445455; REDUCE-INVSCALE-X=-0.327273; SCHED-CHAMPION-X=-0.472727; SCHED-ROWGROUP-X=-0.463636; STORE-EPILOGUE-X=-0.369697; VECTOR-MATH-X=-0.409091.
- Direction changes after removing any one Route: NO.
- Interpretation: C13 is one FP16 128x16384 wide multirow path. Even if its historical association is numerically strongest, a single dtype/path cannot establish generalization; several cases have substantial timing dispersion, and the 12 labels contain no score above the 45.16 Champion, so champion sensitivity is untestable.
- Decision: do not treat C13 as a standalone Local Judge or Online gate feature.
