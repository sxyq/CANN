# SYNC-BARRIER-ELISION-X V014

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V014
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the main tiled output pass, remove only the `PIPE_V` barrier after `Muls(valueTile, valueTile, invRms, valid)` and before FP16 `FromFloat(outputLocal, valueTile, valid)`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from exact R31B-V011. No V001-V013 candidate changes are inherited.
- DUPLICATION_AUDIT: V006 tests the post-`Muls` barrier in `ProcessNarrowMidOverlap`; this is a distinct main tiled output-pass site, absent from V001-V013 diffs.
- COMPILE_TARGET: `sync_barrier_elision_v014`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `e93a15346ec3ff3af3b13550ba44bfdef3eb28a6ed990da72f6b96abbf0e0bb6`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded in the compile log.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `-19.992359431%` pooled 62-pair device-event geomean; run scores `-15.537234451%` and `-24.212491648%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 6.84/7.92 us; median-derived throughput 2.395322/2.068687 G elements/s. Event CV 1.287360–1.536144, maxima 96.82/208.90 us; device 3 AICore increased from 13% to 27%. All samples retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V014 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
