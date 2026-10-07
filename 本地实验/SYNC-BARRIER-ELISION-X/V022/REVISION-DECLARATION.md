# SYNC-BARRIER-ELISION-X V022

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V022
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the `SetFlag<HardEvent::MTE2_V>(inputReady)` after loading the FP16 input row.
- SINGLE_CHANGE_BOUNDARY: One event-set deletion from the exact R31B-V011 parent. No V001-V021 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V021 actual source diffs contain no deletion of the `inputReady` SetFlag. V019 deletes its paired WaitFlag, which is a distinct synchronization operation at a different instruction point.
- COMPILE_TARGET: `sync_barrier_elision_v022`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: Not run because Correctness failed; retain LOCAL_SCORE=NONE.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `eb5859059b5e345e159e878707df69d498f23c5ce14b3688e01ccf46d3f6be65`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: FAIL on device 3 at FP16 rows=8 width=128 with 768 bit mismatches; runner returned 3 and stopped at first failure.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V022 is rejected and not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
