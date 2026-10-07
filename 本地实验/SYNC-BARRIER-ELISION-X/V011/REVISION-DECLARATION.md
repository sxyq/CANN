# SYNC-BARRIER-ELISION-X V011

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V011
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the FP16-path `PIPE_V` barrier after `Add(valueTile, xFp32, residualFp32, valid)` in the non-cached-row path.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V010 source changes are inherited.
- DISTINCTION: This barrier is immediately before `Mul(xFp32, valueTile, valueTile, valid)` and is distinct from V007's barrier after that `Mul`, as well as every other V001-V010 tested site.
- COMPILE_TARGET: `sync_barrier_elision_v011`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using the Route runner.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: PAUSED; no submission will be made.

- COMPILE: PASS; target `sync_barrier_elision_v011`; exact candidate SHA verified against the recorded source.
- CORRECTNESS: PASS; five FP16 widths (128, 256, 1024, 2048, 4096), bitwise equal to the exact parent with zero mismatches.
- LOCAL: `LOCAL_REJECTED_NOISY`; 62 interleaved pairs on device 3, pooled raw-pair geometric-mean score `+4.219537576%`; per-run raw-pair scores `-19.677356721%` and `+35.226028044%`.
- LOCAL_QUALITY: Run direction reverses, the first candidate median is slower, and event CVs/outliers are high. No samples were removed.
- CURRENT_LOCAL_BEST: R31B-V011 (unchanged); V011 is not promoted.
- ONLINE: PAUSED; no submission will be made.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, and raw logs under `logs/`. The numeric metric is route-local and not comparable to Official scores. The runner's median-based per-run score fields are retained in the raw log; the recorded aggregate is recalculated from all 62 raw pairs.
