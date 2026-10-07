# SYNC-BARRIER-ELISION-X V023

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V023
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the `SetFlag<HardEvent::V_MTE2>(inputRelease)` after FP16 row input computation and before the scalar reduction tail.
- SINGLE_CHANGE_BOUNDARY: One event-set deletion from the exact R31B-V011 parent. No V001-V022 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V022 actual source diffs contain no deletion of the `inputRelease` SetFlag. V021 removes the paired release WaitFlag, a distinct operation at the next-row buffer reuse point.
- COMPILE_TARGET: `sync_barrier_elision_v023`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `768c52fcc7ab5f86bb6a4fecabd32f8d63ebdca505f9287d5833311e50b3aa7f`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; configure and build return codes were 0.
- CORRECTNESS: `CORRECTNESS_HANG_PENDING_PROCESS_EXIT`. Runner build passed (return code 0). Process PID `2277907` was still active at `2026-10-07T14:21:58Z`, blocked in `trs_logic_cq_recv`; no correctness case result or runner return code was present. Preserve the live runner and original log.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` until Correctness passes.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V023 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`. `correctness-result.json` is a timestamped pending-process observation, not a final runner result.
