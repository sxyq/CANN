# CROSSROW-FULL-PIPELINE-CHAMPION-X V011

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V011
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Starting the FP16 retained-y pass-1 double buffer on the second slot instead
of the first may change the MTE2/V scheduling phase and improve overlap with
the following tile, while preserving the same alternating pipeline.

## Scope

- Change one pipeline variable: FP16 pass-1 double-buffer phase.
- Keep the same two buffers, event allocation, event-to-slot mapping,
  `nextA` alternation, waits, barriers, DMA order, traversal order, parameter
  prefetch, batch size, tile width, retained-y layout, arithmetic order, and
  all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by the initial FP16
  phase expression only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
