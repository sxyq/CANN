# V028 Parent Synchronization Control

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V028`
- PURPOSE: correctness-only reproducibility diagnosis
- DEVICE: `2`
- WINDOW: `2026-10-07T02:22:32Z` to `2026-10-07T02:28:56Z`

## Source identity

- Canonical V027 Parent: SHA256 `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- Isolated diagnostic copy tested here: SHA256 `c3007e431d75f3123b8f9d45cbbabaf6440b23596bd26b665a79d12f6ce5dde8`.
- The copy differs from the canonical Parent by one inserted `SyncVToMTE2()` call before the next gamma/bias MTE2 refill, plus comments. It is not the canonical Parent or a route Candidate.

## Results

- The exact canonical Parent previously failed route-bound C15 on device 2: `rc=3`, `bad=30306`, `max_abs=1.2031`; see `../PARENT-direct-device2-20261006T2315Z/`.
- The first diagnostic-copy launch returned `127` because `libgraph.so` was missing from the runtime loader path. After loading the installed CANN environment, the direct C15 retry returned `0`.
- The subsequent 19-case matrix (C01-C16, B8193, B16384, and C15R2) returned `rc=0` and `bad=0` for every case. See `matrix-return-codes.tsv` and the per-case raw/stat/log files.

## Interpretation

This control supports the hypothesis that a V-to-MTE2 drain before staging-buffer reuse is sufficient to make this isolated Parent copy pass the exercised wide-FP32 and regression cases. It does not establish correctness of the exact canonical Parent: the tested source hash is different, and the canonical Parent failure remains unresolved.

No Local timing was run. This evidence does not authorize a performance-source edit, V029 promotion, or Online submission. Continue only with route-local correctness reproducibility until the exact Parent passes or Planning explicitly changes the comparison policy.
