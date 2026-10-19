# CROSSROW-FULL-PIPELINE-CHAMPION-X V014

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V014
DIRECT_PARENT: V012
STATUS: DECLARED

## Single hypothesis

Issuing the next FP16 retained-y pass-1 x/residual prefetch before waiting
for the current unit's MTE2/V ready event may overlap MTE2 with that ready
wait across rows and tiles.

## Scope

- Change one input-prefetch order variable: move the next-unit prefetch block
  before the current-unit MTE2/V ready wait.
- Keep the same input buffers, ready and release events, waits, barriers,
  row/tile traversal, parameter pipeline, batch size, tile width, retained-y
  layout, arithmetic, and output order unchanged.
- Parent source is the V012 Candidate; V014 Candidate differs by this order
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
