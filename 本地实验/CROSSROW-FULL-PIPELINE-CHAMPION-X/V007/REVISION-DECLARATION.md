# CROSSROW-FULL-PIPELINE-CHAMPION-X V007

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V007
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Issuing the next FP16 gamma/bias tile load before waiting for the current
parameter-ready event may overlap parameter prefetch with the current tile's
ready wait, while the existing double-buffer reuse waits remain unchanged.

## Scope

- Move one operation group: the next gamma/bias prefetch block moves before
  the current tile's `MTE2_V` wait.
- Keep parameter buffer reuse waits, event allocation, output-store events,
  row traversal, tile width, retained-y layout, buffer sizes, arithmetic order,
  and all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this prefetch order
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
