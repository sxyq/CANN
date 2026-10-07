# SYNC-BARRIER-ELISION-X V016

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V016
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the main tiled `Process()` second pass, remove only the `PIPE_V` barrier immediately after FP16 `FromFloat(outputLocal, valueTile, valid)` and before the gamma `Mul`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V015 candidate changes are inherited.
- DUPLICATION_AUDIT: V003 and V009 remove a conversion-to-gamma-Mul barrier in `ProcessNarrowMidOverlap`, not this main tiled `Process()` site. V014 tests the barrier before `FromFloat`; V015 tests the later gamma-Mul-to-bias-Add barrier. V001-V015 evidence contains no deletion at this exact site.
- COMPILE_TARGET: `sync_barrier_elision_v016`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `77ebff3586ee28911f3b3932d228f2cdb01f2281427b92f30a5e4204c8d47981`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `+15.261919003%` pooled 62-pair device-event geomean; per-run scores `+14.835632711%` and `+15.689787729%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 7.01/6.72 us; median-derived throughput 2.337233/2.438095 G elements/s. Event CV 1.704175/1.875794, maxima 199.34/176.68 us; device 3 AICore was 32% before and 31% after. All 62 raw pairs retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V016 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
