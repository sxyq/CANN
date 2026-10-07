# SYNC-BARRIER-ELISION-X V019

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V019
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the `WaitFlag<HardEvent::MTE2_V>(inputReady)` after loading the FP16 input row and before vector consumption.
- SINGLE_CHANGE_BOUNDARY: One event-wait deletion from the exact R31B-V011 parent. No V001-V018 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V018 actual source diffs contain no deletion of the `inputReady` wait. V006 tests the distinct `paramReady` wait after scalar math, not the input-ready wait. The 8x2048 Local shape dispatches to `ProcessNarrowMidOverlap` (`kMidRowMinWidth=128`, `kTileElems=4096`).
- COMPILE_TARGET: `sync_barrier_elision_v019`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: Not run because Correctness failed; retain LOCAL_SCORE=NONE.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `c7785014791d09697219ffb6c691a84619df7a0ea98356b9d99c10cf12ab8a93`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; exact candidate SHA recorded.
- CORRECTNESS: FAIL on device 3. FP16 rows=8 width=128 passed with zero mismatches; width=256 had 2048 bit mismatches and runner returned 3. Per workflow, width testing stopped at the first failure.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V019 is rejected and not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
