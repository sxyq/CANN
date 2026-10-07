# SYNC-BARRIER-ELISION-X V020

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V020
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the non-resident-parameter `WaitFlag<HardEvent::MTE2_V>(paramReady)` before the output `Muls`.
- SINGLE_CHANGE_BOUNDARY: One event-wait deletion from the exact R31B-V011 parent. No V001-V019 candidate changes are inherited.
- DUPLICATION_AUDIT: V019 deletes the distinct input-row `inputReady` wait before first-pass vector consumption. V001-V019 actual diffs contain no deletion of `paramReady` wait.
- COMPILE_TARGET: `sync_barrier_elision_v020`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: Not run because Correctness failed; retain LOCAL_SCORE=NONE.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `62fc7c8f67c88a692df54c27e9f04abbc8f1a22b7bb94683c1e6c0ce7932e121`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: FAIL on device 3 at FP16 rows=8 width=128 with 256 bit mismatches; runner returned 3 and stopped at first failure.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V020 is rejected and not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
