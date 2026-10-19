# CROSSROW-FULL-PIPELINE-CHAMPION-X V004

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V004
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Moving the FP16 pass-1 input-buffer release event to immediately after the
retained-y value conversion will let the next x/residual tile load begin while
the current tile's square and reduction continue, increasing overlap in the
cross-row full pipeline.

## Scope

- Change one event placement: the `V_MTE2` release for the pass-1 input
  double-buffer.
- Keep V001 output-store waits, row traversal, tile width, retained-y layout,
  event allocation, buffer sizes, parameter prefetch, arithmetic order, and
  all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this one placement
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
