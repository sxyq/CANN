# V073 Revision Declaration

- Route: `SYNC-BARRIER-ELISION-X`
- Direct parent: exact `R31B-V011`; V071 and V072 were not promoted
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Active path: `ProcessSmallLowPrecisionContiguousBatched` -> BF16 `ApplyFp32GammaBiasBatch`
- Single change: remove only the `PipeBarrier<PIPE_V>` between the batched `Mul` and `Add` operations in `ApplyFp32GammaBiasBatch`
- Hypothesis: the batched vector operations on the same independent row slices may carry the required dependency without this explicit V-pipe boundary
- Correctness risk: `Add` may consume incomplete `Mul` results; correctness is the safety gate
- Scope exclusions: no math, dtype policy, dispatch, tiling, buffer, address arithmetic, event, store, or other synchronization changes
- Duplicate audit: V071 targeted post-Add; V072 targeted post-FromFloat; V073 targets only the preceding post-Mul boundary
- Required loop: `RULE_REFRESH -> ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT`
- Local score remains local-only and is not comparable to Official `45.16`
