# V074 Revision Declaration

- Route: `SYNC-BARRIER-ELISION-X`
- Direct parent: exact `R31B-V011`; V069 through V073 were not promoted
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Active path: `ProcessSmallLowPrecisionContiguousBatched` -> per-row `Muls(invRms)` -> `ApplyFp32GammaBiasBatch`
- Single change: remove only the `PipeBarrier<PIPE_V>` immediately after `Muls(valueRow, valueRow, invRmsValues[batchRow], width)`
- Hypothesis: the following batched gamma/bias vector operations can consume the completed invRms-scaled row without this extra V-pipe boundary, allowing dependency overlap
- Correctness risk: gamma/bias multiplication may observe incomplete invRms scaling; correctness is the immediate safety gate
- Scope exclusions: no math, dtype policy, dispatch, tiling, buffer, address arithmetic, event, store, or other synchronization changes
- Duplicate audit: V069 removed an active `MTE3_V` wait; V070 removed `SyncMTE3ToV`; V071 removed the `ApplyFp32GammaBiasBatch` post-Add barrier; V072 removed the post-FromFloat barrier; V073 removed the post-Mul barrier. V074 targets only the preceding post-Muls boundary.
- Required loop: `RULE_REFRESH -> ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT`
- Local score remains local-only and is not comparable to Official `45.16`
