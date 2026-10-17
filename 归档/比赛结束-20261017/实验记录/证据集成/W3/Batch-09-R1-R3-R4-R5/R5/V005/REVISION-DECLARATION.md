# CROSSROW-FULL-PIPELINE-CHAMPION-X V005

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V005
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Removing the FP16 pass-1 `PIPE_V` barrier between the retained-y copy
operation and the independent FP32 conversion may let the vector pipeline
overlap those operations while preserving the value and reduction order.

## Scope

- Remove one barrier between `Muls(yTile, xLocal, 1)` and
  `ToFloat(valueFp32, xLocal)` in the FP16 pass-1 path.
- Keep every event and wait, row traversal, tile width, retained-y layout,
  buffer size, arithmetic operation, output-store sequence, and all non-FP16
  paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this one barrier
  placement only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
