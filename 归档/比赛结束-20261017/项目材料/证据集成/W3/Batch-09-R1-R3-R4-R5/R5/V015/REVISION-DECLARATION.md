# CROSSROW-FULL-PIPELINE-CHAMPION-X V015

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V015
DIRECT_PARENT: V012
STATUS: DECLARED

## Single hypothesis

Releasing the FP16 pass-2 parameter buffer after the final retained-row
vector-to-MTE2 sync and before that row's MTE3 store may overlap the next
parameter prefetch with the final output store of the current tile.

## Scope

- Change one FP16 pass-2 event placement: move the current parameter tile's
  `V_MTE2` release to the final retained-row store boundary.
- Keep the parameter-ready events, reuse waits, MTE3-to-V wait, input pipeline,
  barriers, row/tile traversal, batch size, tile width, retained-y layout,
  arithmetic, output order, and all non-FP16 paths unchanged.
- Parent source is the V012 Candidate; V015 Candidate differs by this release
  placement only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
