# SYNC-BARRIER-ELISION-X V047

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V047
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessSmallLowPrecisionContiguousBatched`, remove only the `SyncMTE2ToV()` immediately after the batched x/residual loads.
- SINGLE_CHANGE_BOUNDARY: One MTE2-to-V synchronization-call deletion from the exact R31B-V011 parent. No V046 or other Route candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V046 route-local diff patches contain no deletion at this exact operation point. V034/V044 target generic `Process()`, V045 targets resident parameter loads in `ProcessNarrowMidOverlap`, and V046 removes a distinct V-to-MTE2 handoff after reductions.
- COMPILE_TARGET: `sync_barrier_elision_v047`
- CORRECTNESS_CASES: FP16 Parent/Candidate bitwise comparison across the seven cases configured in the V047 runner.
- LOCAL_CASE: FP16 128x128; device-event timing, same-binary qualification, and two 31-pair interleaved Parent/Candidate blocks; retain all samples and load/jitter context. Await Main's exclusive device assignment after Correctness.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `d91dc605796bdb3e1594126847302ae958857f933392c9b417961e435deb6173`
- COMPILE: PASS; target `sync_barrier_elision_v047`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`. The first attempt failed before source compilation because `ASCEND_HOME_PATH` was unset; the retry sourced the installed toolkit `aarch64-linux/script/set_env.sh` and passed. Both logs are retained.
- CORRECTNESS: FAIL; FP16 cases 8x128, 8x256, 8x1024, 8x2048, 8x4096, and 16x2064 had zero bit mismatches. The final 128x128 case had 8,187 mismatches; runner exit code 3.
- LOCAL: NOT_RUN because Correctness failed; `LOCAL_SCORE=NONE`.
- CURRENT_LOCAL_BEST: R31B-V011 (unchanged); V047 is not promoted.
- ONLINE: NOT_SUBMITTED.
- RESULT: Correctness failure retained; no Local lease requested.
- EVIDENCE: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
