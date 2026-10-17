# N04 V015 Handoff

V015 adds one persistent L2 hint to `gammaGm_` in the wide low-precision path. It directly follows N04 V014 and leaves the x, residual, and output hints plus gamma/bias UB staging unchanged. Candidate commit is `1516f32f5`; source SHA-256 is `a480f26fa101170550d1091d34fb5c5cca09cbc247e9e2ababa8110e5368e5cb`.

Using the existing server3 CMake entry and `paired_runner_v016`, Compile passed, Champion/Candidate Correctness passed 4/4 on d7, and paired Local completed 4x21 with return code 0. Overall `LOCAL_SCORE=NONE` due to large two-sided FP16 outliers. BF16 D=12288 had 21/21 negative pairs; BF16 D=32768 had 20/21 negative pairs with one positive pair. Complete samples, statistics, and device snapshots are in `RESULTS.md` and adjacent files. These are single-run Local results, not Official results.

Next single-factor hypothesis: derive V016 from V015 and add only `biasGm_.SetL2CacheHint(CACHE_MODE_PERSISTENT)` in the same wide low-precision path. Retain the gamma persistent hint and all existing x/residual/output hints. This tests the other repeatedly read channel stream without changing parameter staging or compute behavior. Reuse the existing paired Runner, compare with R31B V011 Champion, and use d7 only.
