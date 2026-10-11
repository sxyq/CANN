# N04 V011 Handoff

V011 is a single-factor residual-stream policy trial based on V010. In the wide low-precision path, `residualGm_` changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_LAST`; `xGm_` remains `CACHE_MODE_PERSISTENT` and `outputGm_` remains `CACHE_MODE_DISABLE`. Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency remain unchanged. Candidate source commit: `6eef84ad`.

The existing server3 paired target built with CANN `8.5.0.alpha002`; V011/V011 Correctness passed all four shapes on d7. Paired Local completed 21 pairs per shape. FP16 D=12288 and BF16 D=32768 were negative on every pair; FP16 D=32768 was negative on 19/21 pairs; BF16 D=12288 had large two-sided outliers and is inconclusive. Exact statistics, source identities, device snapshots, and raw output are in `RESULTS.md` and adjacent files.
