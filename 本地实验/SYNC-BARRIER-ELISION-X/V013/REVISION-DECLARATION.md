# SYNC-BARRIER-ELISION-X V013

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V013
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the main tiled `Process()` first pass, remove the `PIPE_V` barrier after `Mul(xFp32, valueTile, valueTile, valid)` and before the tile `ReduceSum(reduceFp32[tileIndex], ...)`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from exact R31B-V011. No V001-V012 candidate changes are inherited.
- DUPLICATION_AUDIT: This is a different code path from V007's `Mul`-to-`ReduceSum` barrier in `ProcessNarrowMidOverlap`; V001-V012 diffs contain no deletion at this main tiled first-pass site.
- COMPILE_TARGET: `sync_barrier_elision_v013`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `548f1e1e8345c8e4e7eb970ad2765ab87047430493f091f01c33679665c77074`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded in the compile log.
- CORRECTNESS: PASS on device 3 for five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches against the exact Parent.
- LOCAL: `+12.349960946%` pooled 62-pair device-event geomean; run scores `+12.892422007%` and `+11.810106472%`; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate medians 7.56/6.59 us; median-derived throughput 2.167196/2.486191 G elements/s. Event CV ranges 1.020455–1.815562, maxima 146.46/137.42 us, and device 3 AICore was 33% at both snapshots. All samples retained.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V013 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
