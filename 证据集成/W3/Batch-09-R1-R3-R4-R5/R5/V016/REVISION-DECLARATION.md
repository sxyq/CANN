# CROSSROW-FULL-PIPELINE-CHAMPION-X V016

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V016
DIRECT_PARENT: V012
STATUS: DECLARED

## Single hypothesis

Issuing the FP16 bias load before the gamma load for each retained-y pass-2
parameter tile may change MTE2 request scheduling and improve overlap with
the adjacent row and tile work.

## Scope

- Change one FP16 pass-2 parameter DMA order variable: bias-first for the
  initial tile and every prefetched next tile.
- Keep parameter-ready and release events, waits, barriers, input pipeline,
  row/tile traversal, batch size, tile width, retained-y layout, arithmetic,
  output order, and all non-FP16 paths unchanged.
- Parent source is the V012 Candidate; V016 Candidate differs by this load
  order only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
