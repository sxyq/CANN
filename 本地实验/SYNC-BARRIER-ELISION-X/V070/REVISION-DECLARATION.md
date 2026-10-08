# V070 Revision Declaration

- Route: `SYNC-BARRIER-ELISION-X`
- Direct parent: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- V069 is not the parent; its Local Best remained exact `R31B-V011`.
- Active path: `ProcessSmallLowPrecisionContiguousBatched`, used by the BF16 `128x256` local case.
- Single change: remove the one `SyncMTE3ToV()` immediately after `Store(outputGm_, batchOffset, outputLocal, totalElems)` in that function.
- Hypothesis: for the active small contiguous path, the output drain may be unnecessary before the next independent batch or kernel completion; removing one MTE3-to-V dependency may reduce synchronization overhead.
- Correctness risk: the next batch may reuse the output staging buffer before the previous asynchronous store completes. Correctness is the safety gate.
- Scope exclusions: no math, dtype policy, dispatch, UB layout, cross-row pipeline, address arithmetic, or other route changes.
- Required loop: `RULE_REFRESH -> ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT`.
- Local score remains local-only and is not comparable to Official `45.16`.
