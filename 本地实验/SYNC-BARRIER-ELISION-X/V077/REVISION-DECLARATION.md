# SYNC-BARRIER-ELISION-X V077

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V077`
- DIRECT_PARENT: exact `R31B-V011`
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged)
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_CHANGE: Remove only the BF16-path `PipeBarrier<PIPE_V>` immediately after `Add(valueTile, xFp32, residualFp32, valid)` and before `Mul(residualFp32, valueTile, valueTile, valid)` in `ProcessSmallLowPrecisionContiguousBatched`.
- CHANGE_CLASS: one active-path synchronization/dependency-boundary deletion.
- DUPLICATE_AUDIT: distinct from the V069 active-path `WaitFlag<MTE3_V>`, V075 parameter-staging `SyncVToMTE2()`, and V076 post-`ReduceSum` `SyncVToMTE2()` removals.
- CORRECTNESS_RISK: `Mul` could consume an incomplete vector result if this dependency is required.
- SCOPE_EXCLUSIONS: no math, dtype policy, dispatch, tiling, buffer allocation, address arithmetic, store, event allocation, or other synchronization change.
- COMPILE_TARGET: `sync_barrier_elision_v077`
- CORRECTNESS_CASES: 9 cases (8 FP16 and 1 BF16), exact Parent/Candidate bit comparison.
- LOCAL_CASE: BF16 `128x256`, device 3, 60 warmups, 3 independent runs of 31 interleaved Parent/Candidate pairs; device-event timing with wall-clock diagnostics.
- LOCAL_SCORE_TYPE: `SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`
- LOCAL_SCORE: `-0.670241%` (latest complete run, paired-median scorer)
- LOCAL_DELTA: `-0.670241%` relative to the exact Parent in that run
- DIAGNOSTIC_RUN_SCORES: `+10.233918%`, `0.000000%`, `-0.670241%`
- POOLED_DIAGNOSTIC: `+2.591284%` over all 93 raw pairs; not used for promotion because run directions disagree.
- QUALITY: `POOR/NOISY`
- VERDICT: `LOCAL_REJECTED_NOISY`
- ONLINE: forbidden; no submission made.

All raw samples remain in `logs/local-run1.log`, `logs/local-run2.log`, `logs/local-run3.log`, and `logs/local.log`. V077 is not promoted; the next Revision must start from exact R31B-V011.
