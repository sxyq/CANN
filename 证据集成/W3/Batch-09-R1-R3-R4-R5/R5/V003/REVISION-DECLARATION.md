# CROSSROW-FULL-PIPELINE-CHAMPION-X V003

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V003
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Adding one MTE3-to-V completion wait after the first parameter tile of the
FP16 retained-y pass-2 may drain the initial output-store burst early and
improve the overlap shape of later row/tile work, while the V001 batch-end
completion wait remains the final ordering boundary.

## Scope

- Add one completion wait at the end of the first parameter tile group only.
- Keep V001's batch-end completion wait and every other event placement.
- Keep row ownership, tile width, retained-y layout, event allocation, buffer
  sizes, parameter prefetch, arithmetic order, and non-FP16 paths unchanged.
- Parent source is V001 Candidate; Candidate differs by this one wait only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
