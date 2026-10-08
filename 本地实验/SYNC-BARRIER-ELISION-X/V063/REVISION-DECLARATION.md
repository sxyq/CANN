# SYNC-BARRIER-ELISION-X V063

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V063`
- DIRECT_PARENT: exact `R31B-V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `57f9b176c1c444ada9f58ffd8741ca3655ac47cc9c25518b921115908265c013`
- SINGLE_HYPOTHESIS: test whether the V-to-S handoff before reading batched FP32 reduction results is required after the preceding V-to-MTE2 handoff.
- SINGLE_CHANGE_BOUNDARY: delete only `SyncVToS()` after `SyncVToMTE2()` and before the `meanSquares` scalar extraction loop in `ProcessSmallLowPrecisionContiguousBatched`. Preserve every other barrier, event, operation, and path.
- DUPLICATION_AUDIT: `NO_MATCH_IN_SCOPED_DIFFS_FOR_EXACT_CALLSITE`; prior V029/V031/V036 V-to-S deletions are in other functions, and V055 removes a distinct pre-SyncVToS pipeline barrier. See `RULE_REFRESH_RECEIPT.md`.
- FOCUS_AXIS: batched reduction-result V-to-S event handoff.
- FOCUS_VALUE: one `SyncVToS()` before scalar `meanSquares` reads.
- CURRENT_LOCAL_BEST: exact `R31B-V011`; no rejected Candidate is inherited.
- COMPILE: `PASS` for kernel and correctness targets after the recorded host-include fix; failed attempts remain in `logs/`.
- CORRECTNESS: `PASS`, 7/7 FP16 cases bitwise equal.
- LOCAL: numeric 62-pair result recorded in `RESULT.md`; `LOCAL_REJECTED_NOISY`, not promoted. Exact Local Best remains `R31B-V011`.
