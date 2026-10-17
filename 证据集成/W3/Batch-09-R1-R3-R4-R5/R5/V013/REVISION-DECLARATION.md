# CROSSROW-FULL-PIPELINE-CHAMPION-X V013

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V013
DIRECT_PARENT: V012
STATUS: DECLARED

## Single hypothesis

Finishing the retained-y pass-1 row reductions in reverse row order may
change cross-row V/S scheduling and expose overlap before the retained-y
pass-2 store stream, while preserving the computed value for every row.

## Scope

- Change one completion-order variable: reverse the FP16 retained-y pass-1
  `ReduceSum` and `invRms` row loop.
- Keep the input and parameter pipelines, event allocation, waits, barriers,
  tile traversal, batch size, tile width, retained-y layout, arithmetic, and
  output order unchanged.
- Parent source is the V012 Candidate; V013 Candidate differs by this loop
  direction only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
