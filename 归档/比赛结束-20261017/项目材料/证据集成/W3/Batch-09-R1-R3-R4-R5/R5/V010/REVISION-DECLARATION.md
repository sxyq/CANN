# CROSSROW-FULL-PIPELINE-CHAMPION-X V010

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V010
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Issuing the residual tile DMA before the x tile DMA in the FP16 retained-y
pass-1 may improve MTE2 scheduling overlap across adjacent rows while leaving
the same input buffers, ready events, waits, and arithmetic dependencies.

## Scope

- Change one input DMA ordering variable: residual-first for FP16 pass-1.
- Apply the same order to the initial unit and each prefetched next unit.
- Keep event allocation and signaling, waits, barriers, traversal order,
  parameter prefetch, batch size, tile width, retained-y layout, buffer sizes,
  arithmetic order, and all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this input load order
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
