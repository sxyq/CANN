# N05 V022 Handoff

V022 source commit is `a05054d177c849c4132e0ffd6671298e26453298`, based on V021 source commit `bca94254e8cbfe639c593366a4637addfcd5cb3e`. The pass-two retained-y row/tile UB address is now derived from batchRow and col rather than advanced through a row cursor.

The paired experiment used Parent V021 SHA256 `035c5e689e0d0a91865b726eb3be7dda747064f471f577b3605dfea95fdd95b4` and Candidate V022 SHA256 `24fdf8e689783a23dd2d68cc26edf35185e5419ed5f85b353036351ddec74ef0`; local and server copies match. CANN 8.5 compile passed, Correctness passed all four cases on d2, and one paired Local completed with 21 pairs per case.

The paired-delta medians are -0.540, +0.060, +0.720, and +0.300 us. All ranges span zero; `LOCAL_SCORE=NONE`. All raw samples and device snapshots are retained.

Continue the Scalar address-generation study with a distinct single-factor change. Keep math, tile order, DMA/event order, and Vector scheduling unchanged. Reuse the existing server3 paired CMake target and d2 runner.
