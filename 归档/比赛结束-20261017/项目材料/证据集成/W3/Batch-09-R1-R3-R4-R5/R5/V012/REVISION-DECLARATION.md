# CROSSROW-FULL-PIPELINE-CHAMPION-X V012

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V012
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Starting the FP16 gamma/bias double-buffer on the second parameter slot may
change the MTE2/V phase of the retained-y pass-2 and improve overlap with the
first output tile, while preserving the same alternating parameter pipeline.

## Scope

- Change one pipeline variable: FP16 pass-2 parameter double-buffer phase.
- Map the initial gamma/bias load and ready event to the second slot, then
  continue the existing alternating slot sequence.
- Keep the parameter prefetch position, reuse waits, output-store events,
  input phase, barriers, traversal order, batch size, tile width, retained-y
  layout, arithmetic order, and all non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this parameter phase
  expression only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
