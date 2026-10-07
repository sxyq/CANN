# SYNC-BARRIER-ELISION-X V024

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V024
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only `SetFlag<HardEvent::MTE2_V>(paramReady)` immediately after the gamma/bias loads in the non-resident-parameter path.
- SINGLE_CHANGE_BOUNDARY: One event-set deletion from the exact R31B-V011 parent. No V001-V023 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V023 actual source diffs contain no deletion of this `paramReady` SetFlag. V020 deletes the distinct matching `paramReady` WaitFlag; V019 deletes `inputReady` WaitFlag; V022 deletes `inputReady` SetFlag; V023 deletes `inputRelease` SetFlag.
- COMPILE_TARGET: `sync_barrier_elision_v024`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `9c3290b9206dbe96966f6fc861cb5c163f92b2be3255f584f66134d61c8e0b10`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; configure and build return codes were 0.
- CORRECTNESS: FAIL. FP16 rows=8 width=128 passed exact bitwise comparison; the next candidate case (width=256, based on runner order) ended with ACL status `507034` / vector-core timeout (`retCode=0x30`). Runner returned 2 at `2026-10-07T15:00:19Z`.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V024 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw runner/plog logs under `logs/`.
