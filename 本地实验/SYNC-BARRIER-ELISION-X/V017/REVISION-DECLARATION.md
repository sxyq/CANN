# SYNC-BARRIER-ELISION-X V017

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V017
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the generic tiled `Process()` second pass, remove only the common `PIPE_V` barrier after the dtype-specific epilogue; for FP16 this is immediately after the final bias `Add` and before `SyncVToMTE3()`/Store.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V016 candidate changes are inherited.
- DUPLICATION_AUDIT: V001 removes a barrier after output Add in `ProcessNarrowMidOverlap`, a distinct narrow/mid path. V002/V015 target the output Mul-to-Add barrier, not the post-Add barrier in generic tiled `Process()`. V001-V016 diffs contain no deletion at this exact generic-path site.
- COMPILE_TARGET: `sync_barrier_elision_v017`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `2de913203223f6d4c5aeaae9abbd7bfb355f710ee6f73d75dc970d3a01fa64ce`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `-3.134245841%` pooled 62-pair device-event geomean; per-run scores `-1.222162972%` and `-5.009315743%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 6.69/7.31 us; median-derived throughput 2.449028/2.241313 G elements/s. Event CV 2.154876/1.967805, maxima 277.28/251.44 us; device 3 AICore was 6% before and 30% after. All 62 raw pairs retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V017 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
