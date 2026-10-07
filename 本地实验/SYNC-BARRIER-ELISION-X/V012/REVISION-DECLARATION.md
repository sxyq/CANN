# SYNC-BARRIER-ELISION-X V012

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V012
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the main tiled path's non-cached-row second pass, remove the single `PIPE_V` barrier after widening `xLocal` and `residualLocal` to FP32 and before `Add(valueTile, xFp32, residualFp32, valid)` consumes those values.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V011 source changes are inherited.
- DUPLICATION_AUDIT: This exact conversion-to-Add barrier is absent from this Route's V001-V011 diffs. Previously tested nearby sites remain unchanged; V003/V009 and V004/V008 duplicate sites were not selected.
- COMPILE_TARGET: `sync_barrier_elision_v012`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `22016a57ecd0f487ba0b26dea3540abf15216cce523fc413bfb67dc4bdb34543`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded in the compile log.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `-18.675098888%` pooled 62-pair device-event geomean; run scores `-31.727480414%` and `-3.127355180%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 5.20/5.30 us, median-derived throughput 3.150769/3.091321 G elements/s; event CV 1.307827–2.640546 and maxima 160.30/240.36 us. All samples retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V012 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`. The runner's copied LOCAL_STATS label still reads `CANDIDATE_V011`; the log header binds the V012 source SHA, and the all-sample score is independently recalculated from the retained V012 run samples.
