# CROSSROW-FULL-PIPELINE-CHAMPION-X V008

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V008
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Applying retained-y rows in reverse order inside each FP16 parameter tile may
change the cross-row output pipeline balance and reduce contention between
adjacent row stores, while leaving the parameter stream and all dependencies
unchanged.

## Scope

- Change one schedule variable: pass-2 `batchRow` traversal direction.
- Keep parameter prefetch, input traversal, event allocation and waits,
  barriers, tile width, retained-y layout, buffer sizes, arithmetic order, and
  all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this row-order change
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
