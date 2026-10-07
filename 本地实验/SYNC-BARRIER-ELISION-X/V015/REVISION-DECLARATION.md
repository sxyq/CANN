# SYNC-BARRIER-ELISION-X V015

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V015
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the main tiled `Process()` second pass, non-cached-parameter FP16 branch, remove only the `PIPE_V` barrier after `Mul(outputLocal, outputLocal, gammaLocal, valid)` and before `Add(outputLocal, outputLocal, biasLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V014 candidate changes are inherited.
- DUPLICATION_AUDIT: V002 tests a similarly shaped output `Mul`-to-`Add` barrier in `ProcessNarrowMidOverlap`, not this main tiled `Process()` non-cached-parameter branch. V001 and V003-V014 diffs/declarations contain no deletion at this exact site.
- COMPILE_TARGET: `sync_barrier_elision_v015`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain all 62 pairs and all raw samples, with no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `930056f67d6694820e4e454f573a5892f9377a53046238c81da996147be38892`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded. Initial build lacked `ASCEND_HOME_PATH`; the environment-only retry passed.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent. Runner build/runtime environment retries are retained in logs.
- LOCAL: `+2.297831838%` pooled 62-pair device-event geomean; per-run scores `-19.597769384%` and `+30.156170029%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 7.50/7.77 us; median-derived throughput 2.184533/2.108623 G elements/s. Event CV 1.583806/1.886552, maxima 174.18/239.94 us; device 3 AICore was 38% before and 31% after. All 62 raw pairs retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V015 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
