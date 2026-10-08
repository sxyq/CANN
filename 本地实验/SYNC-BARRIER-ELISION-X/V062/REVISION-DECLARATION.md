# SYNC-BARRIER-ELISION-X V062

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V062`
- DIRECT_PARENT: exact `R31B-V011` (not V061)
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `25addc5b93d805ba0c4c1e0b531ddd2d81b89e20d9f85981664773e3a9568582`.
- SINGLE_HYPOTHESIS: test whether the output-ready V-to-MTE3 wait is redundant in the low-precision contiguous batched output store path.
- SINGLE_CHANGE_BOUNDARY: delete only `AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(outputReady)` immediately before the batched `Store`. Preserve the preceding barriers and SetFlag, the Store, the following MTE3-to-V SetFlag, release-state updates, and all other operations.
- DUPLICATION_AUDIT: `NO_MATCH_IN_SCOPED_DECLARATIONS` for this exact outputReady wait deletion in the Route's revision declarations; the search found distinct post-store drains and the V061 outputRelease wait, not this point. This narrow declaration check is not represented as a complete diff-level proof.
- FOCUS_AXIS: FP16 batched output-ready V-to-MTE3 event wait.
- FOCUS_VALUE: one `WaitFlag<V_MTE3>(outputReady)` before `Store`.
- CURRENT_LOCAL_BEST: exact `R31B-V011`; V061 is not inherited.
- COMPILE: PASS for `sync_barrier_elision_v062` and `sync_barrier_elision_correctness`; build artifacts and hashes are recorded in `RESULT.md` (the full compiler transcript was not retained under this revision's `logs/`).
- CORRECTNESS: PASS, 7/7 FP16 cases bitwise equal.
- LOCAL: 62 interleaved pairs at FP16 128x128; paired-median score `-0.429185%`; separate median-latency ratio `+3.249540%`; `LOCAL_REJECTED_NOISY` due to block disagreement and high jitter.
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged).
