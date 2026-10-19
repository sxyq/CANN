# N04 V008 Handoff

V008 is a single-factor input-policy trial based on V007. In the wide low-precision path, `xGm_` changes from `CACHE_MODE_LAST` to `CACHE_MODE_NORMAL`; `outputGm_` remains `CACHE_MODE_DISABLE`. Arithmetic, DMA descriptors, prefetch, parameter residency, and all other behavior remain unchanged. Candidate source commit: `05abc48b`.

The existing paired Runner entry built successfully with CANN `8.5.0.alpha002` after restoring `ASCEND_HOME_PATH` and the route's established `CPATH`. An earlier Bisheng exit 139 and the missing-`<vector>` attempt remain in separate raw logs. V011/V008 Correctness passed all four cases on d7. Paired Local completed 21 pairs per case: both D=32768 cases showed consistent gains, FP16 D=12288 also showed a consistent gain, and BF16 D=12288 was noisy due to one large Parent outlier. Exact medians, paired deltas, MAD, load snapshots, source identities, and raw output are in `RESULTS.md` and the adjacent evidence files.

The V005/V006-labeled output preserved under V007 is unrelated to this run and is not part of V008 results. No V008 result remains pending.
