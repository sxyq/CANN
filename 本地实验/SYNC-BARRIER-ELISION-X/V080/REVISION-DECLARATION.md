# SYNC-BARRIER-ELISION-X V080

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V080`
- DIRECT_PARENT: exact `R31B-V011`
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged)
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `acff22e50cc4aa2ddea833e6ef8da6625800740ac71cc77841e63cb11c6d6581`
- SINGLE_CHANGE: Remove only the active main-row BF16/non-half `PipeBarrier<PIPE_V>` immediately after `Muls(valueLocal, valueLocal, invRms, valid)` in the single-row path.
- CHANGE_CLASS: one active-path synchronization/dependency-boundary deletion.
- DISTINCT_FROM: V069 `WaitFlag<MTE3_V>` deletion, V074 helper-path post-`Muls` barrier deletion, V078 conversion-to-Add barrier deletion, and V079 batched Add-to-Mul barrier deletion.
- CORRECTNESS_RISK: the subsequent dtype-specific affine path could consume an incomplete Muls result if the dependency is required.
- SCOPE_EXCLUSIONS: no math, dtype policy, dispatch, tiling, buffer allocation, address arithmetic, store, event allocation, or other synchronization change.
- COMPILE_TARGET: `sync_barrier_elision_v080`
- CORRECTNESS: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.
- LOCAL: three complete 31-pair runs; all raw samples, throughput and load context retained in `local.log`.
- LOCAL_SCORE_TYPE: `SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`
- ONLINE: forbidden; no submission made.

V080 starts from exact V011. V069 and V079 are preserved evidence and are not its Parent.
