# SYNC-BARRIER-ELISION-X V026

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V026
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the final `WaitFlag<HardEvent::V_MTE2>(inputRelease)` in the post-loop drain before `SyncMTE3ToV()` and event-ID release.
- SINGLE_CHANGE_BOUNDARY: One event-wait deletion from the exact R31B-V011 parent. No V001-V025 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V025 actual source diffs contain no deletion of this final post-loop drain wait. V021 deletes the per-row `inputRelease` WaitFlag before buffer reuse; V023 deletes the producer `inputRelease` SetFlag. This revision changes only the distinct post-loop drain WaitFlag.
- COMPILE_TARGET: `sync_barrier_elision_v026`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `a54e86bf395ad4a580c12a1441479fdca0a7916c06c8d97ff5e737e543c17ffc`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; configure and build return codes were 0.
- CORRECTNESS: `CORRECTNESS_RUN_FAILED_PARENT_VECTOR_CORE_TIMEOUT`. FP16 rows=8 widths 128, 256, 1024, and 2048 each completed with zero bit mismatches. The next operation failed in `sync_parent` before the width=4096 comparison, with ACL status `507034` / vector-core timeout (`retCode=0x30`). Runner returned 2 at `2026-10-07T15:48:36Z`; this does not provide a Candidate comparison result for width 4096.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because the Correctness runner did not complete.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V026 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw runner/plog logs under `logs/`.
