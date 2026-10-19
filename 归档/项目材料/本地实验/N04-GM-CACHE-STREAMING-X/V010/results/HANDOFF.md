# N04 V010 Handoff

V010 is a single-factor residual-stream policy trial based on V009. In the wide low-precision path, `residualGm_` receives `CACHE_MODE_DISABLE`; `xGm_` remains `CACHE_MODE_PERSISTENT` and `outputGm_` remains `CACHE_MODE_DISABLE`. Arithmetic, DMA descriptors, prefetch, parameter residency, and other behavior remain unchanged.

The existing server3 paired target built with CANN `8.5.0.alpha002`; V011/V010 Correctness passed all four shapes on d7. Paired Local completed 21 pairs per shape. FP16 D=32768 had 21/21 negative deltas (median `-57.340 us`, MAD `6.540 us`); tail cases and BF16 D=32768 had large outliers. Overall `LOCAL_SCORE=NONE`, with the FP16 wide result retained as a shape-specific signal only. Full distributions, load snapshots, source identities, and raw output are in `RESULTS.md` and adjacent evidence files. No sample was omitted.
