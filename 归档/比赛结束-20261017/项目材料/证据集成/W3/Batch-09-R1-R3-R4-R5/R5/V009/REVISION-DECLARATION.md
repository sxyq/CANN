# CROSSROW-FULL-PIPELINE-CHAMPION-X V009

ROUTE: CROSSROW-FULL-PIPELINE-CHAMPION-X
REVISION: V009
DIRECT_PARENT: V001
STATUS: DECLARED

## Single hypothesis

Reducing the FP16 retained-row batch size from the computed two-row batch to
one row may reduce live cross-row UB pressure and improve per-row pipeline
stability, at the cost of less row reuse per batch.

## Scope

- Change one batching variable: FP16 `batchLimit` is one retained row.
- Keep the computed two-row UB capacity and all buffers allocated as in V001;
  only the processing batch cap changes.
- Keep every event and wait, barrier, traversal order, parameter prefetch,
  tile width, retained-y layout, arithmetic order, and all non-FP16 paths
  unchanged.
- Parent source is V001 Candidate; Candidate differs by this batch-size cap
  only.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Correctness and Local use rows=16, width=16384, blocks=8, dtype=fp16.
