# V071 Revision Declaration

- Route: `SYNC-BARRIER-ELISION-X`
- Direct parent: exact `R31B-V011`; V070 is not inherited because it was not promoted
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Active path: `ProcessSmallLowPrecisionContiguousBatched` -> BF16 `ApplyFp32GammaBiasBatch`
- Single change: remove only the final `PipeBarrier<PIPE_V>` after one `Add` at the helper's column-loop boundary
- Hypothesis: the next column uses a disjoint vector slice, so this local V-pipe dependency boundary may not be needed between columns; the final conversion and store barriers remain unchanged
- Correctness risk: the last parameterized Add must still be complete before `FromFloat`; the exact parent final boundary is retained as the safety gate for that transition
- Scope exclusions: no math, dtype policy, dispatch, tiling, buffer, event, store, address arithmetic, or other synchronization changes
- Duplicate audit: route-local prior declarations and diffs contain no deletion at this exact `ApplyFp32GammaBiasBatch` post-Add boundary; prior helper-adjacent and wide-branch barriers are distinct sites
- Required loop: `RULE_REFRESH -> ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT`
- Local score remains local-only and is not comparable to Official `45.16`
