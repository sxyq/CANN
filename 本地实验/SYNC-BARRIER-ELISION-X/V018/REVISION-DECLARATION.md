# SYNC-BARRIER-ELISION-X V018

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V018
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the FP16-path `PIPE_V` barrier after the one-element `Sqrt(xFp32, xFp32, 1)` and before `SyncVToS()` reads that value for `invRms`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V017 candidate changes are inherited.
- DUPLICATION_AUDIT: V010's declaration names this narrow/mid point, but its actual diff hunk is at source line 361 in generic `Process()`; `ProcessNarrowMidOverlap` starts at line 499. V001-V017 actual deletion diffs contain no deletion at this narrow/mid Sqrt-to-`SyncVToS()` site. The 8x2048 Local shape dispatches to `ProcessNarrowMidOverlap` (`kMidRowMinWidth=128`, `kTileElems=4096`).
- COMPILE_TARGET: `sync_barrier_elision_v018`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `9d63b230bedbdfb205bf725b89f8590b7f176047c623e3bf934ff5e1c9f8ace0`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `-21.285317064%` pooled 62-pair device-event geomean; per-run scores `-17.648576483%` and `-24.761454689%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 6.97/7.54 us; median-derived throughput 2.350646/2.172944 G elements/s. Event CV 1.971141/1.896632, maxima 187.76/256.32 us; device 3 AICore was 31% before and 31% after. All 62 raw pairs retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V018 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
