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
- CORRECTNESS: FAIL. FP16 rows=8 width=128 passed exact bitwise comparison; the next candidate case (width=256, based on runner order) ended with ACL status `507034` / vector-core timeout (`retCode=0x30`). Runner returned 2 at `2026-10-07T14:25:40Z`.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V023 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw runner/plog logs under `logs/`. The original correctness log was retained and naturally completed; its interim pending observation remains recorded in `correctness-result.json`.
