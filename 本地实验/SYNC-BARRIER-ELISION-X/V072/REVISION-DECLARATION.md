# V072 Revision Declaration

- Route: `SYNC-BARRIER-ELISION-X`
- Direct parent: exact `R31B-V011`; V071 was not promoted
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Active path: `ProcessSmallLowPrecisionContiguousBatched` -> BF16 output conversion and MTE3 handoff
- Single change: remove only the `PipeBarrier<PIPE_V>` immediately after `FromFloat(outputLocal, valueLocal, totalElems)` and before `SyncVToMTE3()` in the BF16 branch
- Hypothesis: the explicit V-to-MTE3 dependency boundary already provided by `SyncVToMTE3()` makes this adjacent V-pipe barrier redundant for the completed conversion; correctness is the safety gate
- Correctness risk: MTE3 may observe outputLocal before `FromFloat` completes if the event handoff does not cover this dependency
- Scope exclusions: no math, dtype policy, dispatch, tiling, buffer, address arithmetic, event allocation, store, or other synchronization changes
- Duplicate audit: V071 removed a different post-Add barrier in `ApplyFp32GammaBiasBatch`; V072 targets only the post-FromFloat boundary
- Required loop: `RULE_REFRESH -> ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT`
- Local score remains local-only and is not comparable to Official `45.16`
