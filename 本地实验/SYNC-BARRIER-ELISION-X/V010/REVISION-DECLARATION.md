# SYNC-BARRIER-ELISION-X V010

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V010
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the FP16-path `PIPE_V` barrier after the one-element `Sqrt(xFp32, xFp32, 1)` and before `SyncVToS()` reads that element for `invRms`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V009 changes are inherited.
- DISTINCTION: This is separate from all barrier locations tested by SYNC-BARRIER-ELISION-X V001-V009.
- COMPILE_TARGET: `sync_barrier_elision_v010`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using the existing route runner.
- CURRENT_LOCAL_BEST: R31B-V011
- COMPILE: PASS; target `sync_barrier_elision_v010`; exact candidate SHA verified before and after compilation.
- CORRECTNESS: PASS; five FP16 widths (128, 256, 1024, 2048, 4096), bitwise equal to the exact parent with zero mismatches.
- LOCAL: `LOCAL_REJECTED_NOISY`; 62 interleaved pairs on device 3, pooled raw-pair geometric-mean score `-14.393133069%`; per-run scores `-0.962944886%` and `-26.002084197%`.
- LOCAL_QUALITY: Both run candidate medians were slower (6.66 vs 6.14 us; 5.06 vs 4.88 us). Device-event CV ranged from 1.80506 to 2.56072, with large spikes; no samples were removed.
- CURRENT_LOCAL_BEST: R31B-V011 (unchanged); V010 is not promoted.
- ONLINE: PAUSED; no submission will be made.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, and the raw logs under `logs/`. The pooled metric is route-local and is not comparable to any Official score. The runner's per-run `LOCAL_RESULT` score fields do not match the geometric means recalculated from its raw pairs; the recorded score follows the all-sample raw-pair method and the discrepancy is retained in `local-result.json`.
