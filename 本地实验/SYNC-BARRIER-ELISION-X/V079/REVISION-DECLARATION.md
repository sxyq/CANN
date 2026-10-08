# SYNC-BARRIER-ELISION-X V079

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V079`
- DIRECT_PARENT: exact `R31B-V011`
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged)
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `f605ee1765eef7818485101d3d6621ed2d4ff35ae3758cf033bee8d7dcfe1030`
- SINGLE_CHANGE: Remove only the active `ProcessSmallLowPrecisionContiguousBatched` BF16-path `PipeBarrier<PIPE_V>` between `Add(valueLocal, xFp32, residualFp32, valid)` and `Mul(xFp32, valueLocal, valueLocal, valid)`.
- CHANGE_CLASS: one active-path synchronization/dependency-boundary deletion.
- DISTINCT_FROM: V069 `WaitFlag<MTE3_V>` deletion and V078's different conversion-to-Add barrier deletion; V079 is the Add-to-Mul barrier site.
- CORRECTNESS_RISK: `Mul` could consume an incomplete Add result if the dependency is required.
- SCOPE_EXCLUSIONS: no math, dtype policy, dispatch, tiling, buffer allocation, address arithmetic, store, event allocation, or other synchronization change.
- COMPILE_TARGET: `sync_barrier_elision_v079`
- CORRECTNESS: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.
- LOCAL_CASE: BF16 `128x256`, device 3, 60 warmups, 31 interleaved Parent/Candidate pairs; device-event timing with wall-clock diagnostics.
- LOCAL_SCORE_TYPE: `SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`
- LOCAL_RUN_SCORES: `-1.197606%` initial raw run, `-4.390243%` corrected-label rerun.
- LOCAL_SCORE: `-4.390243%` (latest corrected-label run, paired-median scorer).
- LOCAL_DELTA: `-4.390243%`; `LOCAL_REJECTED_NOISY`; no promotion.
- MEASUREMENT_CONTEXT: device 3 had approximately 62 GB free HBM and 0% AICore in the pre/post snapshots; other-device processes were left untouched.
- CONTINUITY: initial Compile/Correctness/Local sequence completed before the support-label rebuild; corrected runner rerun preserved as additional evidence.
- ONLINE: forbidden; no submission made.

V079 starts from exact V011. V069 and V078 are preserved evidence and are not its Parent.
