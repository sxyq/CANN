# CROSSROW-FULL-PIPELINE-CHAMPION-X V002

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V002
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

In the retained-y FP16 wide-row pass-2, moving the MTE3-to-V completion wait
from the end of the entire retained batch to the end of each parameter tile
group will expose more overlap between the next row's pass-1 work and the
current row's output store while preserving output ordering.

## Scope

- Change one event/barrier placement only: the FP16 pass-2 completion wait.
- Keep row ownership, tile width, retained-y layout, event count, buffer sizes,
  parameter prefetch, arithmetic order, and all non-FP16 paths unchanged.
- Parent source is V001 Candidate; V002 Candidate must differ by this one
  placement only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use the existing paired runner and the same wide FP16
proxy shape as V001: rows=16, width=16384, blocks=8.
