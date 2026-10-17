# CROSSROW-FULL-PIPELINE-CHAMPION-X V006

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V006
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Interleaving retained-y FP16 pass-1 units by tile across the rows in each
batch may expose more cross-row overlap than completing all tiles of one row
before starting the next row.

## Scope

- Change one schedule variable: pass-1 unit traversal order.
- Map units as tile-major across `batchRow` while preserving each unit's row,
  tile, output, and reduction destinations.
- Keep every event and wait, barrier, tile width, retained-y layout, buffer
  size, arithmetic order, output-store sequence, and all non-FP16 paths
  unchanged.
- Parent source is V001 Candidate; Candidate differs by this traversal order
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
