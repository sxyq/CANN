# HOTLOOP-INDEX-ARITH-X V001

ROUTE=HOTLOOP-INDEX-ARITH-X
REVISION=V001
DIRECT_PARENT=R31B V011
PARENT_SOURCE_PATH=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=HOTLOOP_INDEX_ARITHMETIC
SINGLE_HYPOTHESIS=Replace flattened row/tile index division and multiplication with loop-carried induction state in the V011 low-precision wide-row pass-1 loop.
WHY_NOT_DUPLICATE=This route changes only repeated integer index generation inside ProcessWideLowPrecision; it does not change core mapping, tile sizes, pipeline depth, row scheduling, UB layout, or synchronization.
ALLOWED_AXIS=address arithmetic; offset increment; index reuse; loop induction
ONLINE=FORBIDDEN
SINGLE_CHANGE_AUDIT=PASS

RULE_RECALL=Route, context, branch, and worktree are isolated to this Studio; DIRECT_PARENT is exact R31B V011; one conceptual change only; execute ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT; compile/correctness may proceed with the permitted resource rules, but Local requires qualified same-binary parent/candidate measurement; retain raw evidence and failures; Local and Official remain separate; stage explicit Route-local paths only; never modify shared files, other Routes, rules, Dashboard, or submit Online; after Local Result classify briefly and begin the next edit within 120 seconds.
