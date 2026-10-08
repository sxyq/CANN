# SYNC-BARRIER-ELISION-X V078

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V078`
- DIRECT_PARENT: exact `R31B-V011`
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged)
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_CHANGE: Remove only the BF16-path `PipeBarrier<PIPE_V>` between `ToFloat(xFp32, xLocal, valid)` / `ToFloat(residualFp32, residualLocal, valid)` and `Add(valueLocal, xFp32, residualFp32, valid)` in `ProcessSmallLowPrecisionContiguousBatched`.
- CHANGE_CLASS: one active-path synchronization/dependency-boundary deletion.
- DISTINCT_FROM: V069 active-path `WaitFlag<MTE3_V>` deletion and V077 BF16 Add-to-Mul `PipeBarrier<PIPE_V>` deletion.
- CORRECTNESS_RISK: `Add` could consume incomplete conversion results if the dependency is required.
- SCOPE_EXCLUSIONS: no math, dtype policy, dispatch, tiling, buffer allocation, address arithmetic, store, event allocation, or other synchronization change.
- COMPILE_TARGET: `sync_barrier_elision_v078`
- CORRECTNESS_CASES: 9 cases (8 FP16 and 1 BF16), exact Parent/Candidate bit comparison.
- LOCAL_CASE: BF16 `128x256`, device 3, 60 warmups, 3 independent runs of 31 interleaved Parent/Candidate pairs; device-event timing with wall-clock diagnostics.
- LOCAL_SCORE_TYPE: `SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`
- LOCAL_RUN_SCORES: `+8.574740%`, `+2.386115%`, `+2.826088%`
- LOCAL_SCORE: `+2.826088%` (latest complete run, paired-median scorer)
- LOCAL_DELTA: `+2.826088%`; `LOCAL_REJECTED_NOISY`; direction is not repeatable across independent runs.
- ONLINE: forbidden; no submission made.

V078 starts from exact V011; V077 is preserved as a negative noisy result and is not its Parent.
