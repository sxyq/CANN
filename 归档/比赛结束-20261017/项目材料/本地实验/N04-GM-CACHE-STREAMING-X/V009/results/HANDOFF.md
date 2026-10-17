# N04 V009 Handoff

V009 is a single-factor input-policy trial based on V008. In the wide low-precision path, `xGm_` changes from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`; `outputGm_` remains `CACHE_MODE_DISABLE`. Arithmetic, DMA descriptors, prefetch, parameter residency, and all other behavior remain unchanged. Candidate source commit: `61054394`.

The existing server3 paired target built with CANN `8.5.0.alpha002`. V011/V009 Correctness passed all four cases on d7. Paired Local completed 21 pairs per case: both D=32768 wide cases favored V009 on every pair; the tail medians were also negative, with more variation in FP16. Exact medians, paired deltas, MAD, load snapshots, source identities, and raw output are in `RESULTS.md` and the adjacent evidence files. The first incomplete pre-build metadata capture and the subsequent complete capture are both retained.
