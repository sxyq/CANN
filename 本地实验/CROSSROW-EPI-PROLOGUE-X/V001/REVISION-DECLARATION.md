# CROSSROW-EPI-PROLOGUE-X V001

AGENT_ID: W5-R01 CROSSROW-EPI-PROLOGUE-X
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STATUS: DECLARED

## Direct parent and source

DIRECT_PARENT: W3 CROSSROW-FULL-PIPELINE-CHAMPION-X V012
DIRECT_PARENT_SOURCE_SHA256: 6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794
DIRECT_PARENT_GIT_COMMIT: b68fcebe6492e2c34c7fcc03c7506a74a0ad1a3b
DIRECT_PARENT_SOURCE_PATH: local evidence object for CROSSROW-FULL-PIPELINE-CHAMPION-X V012 Candidate.asc

## Orthogonality check

HYPOTHESIS: In the generic cached-row path, move the existing next-row first-tile x/residual MTE2 prologue from immediately after the current row's pass-1 tile loop to the window after the current row's invRms scalar tail and immediately before its epilogue. The earlier issue point lets MTE2 overlap the invRms tail; the new issue point tests overlap with the current-row epilogue while keeping the same bytes and row order.

WHY_NOT_DUPLICATE: W3 Crossrow V012 changed the FP16 pass-2 parameter double-buffer phase; its generic cached-row prefetch remains the parent baseline and the V001 change moves only that existing next-row prologue issue point. Main-1 CASE47 has no selected performance hypothesis and its recorded H1 is a narrow localRows==2 scalar handoff, not this generic cached-row cross-row schedule. Main-1 CASE14 is recorded as same-row D-slice overlap/duplicate axes with missing exact path evidence. Main-1 Tiny has no selected hypothesis and unknown case shape/dispatch mapping. This revision changes no parameter phase, epilogue arithmetic, reduction, ownership, dispatch, or queue depth.

## Scope

- Candidate-only kernel change in `Process()`'s `cacheRow` generic path: move the existing two input loads for the next row's first tile across the invRms tail.
- Existing paired runner test plumbing only: widen its old V012 `D > 8192` argument guard to admit the selected generic cached-row probe (`rows=16,width=6144,blocks=8,fp16`); allocation, kernel calls, correctness comparison, timing, and output format remain unchanged.
- Keep the `cacheRow` predicate, row-boundary guard, DMA byte counts, source offsets, input buffers, and all existing dependency events unchanged.
- Keep `valueFp32Buf_` as the current row's retained y storage and epilogue source; the prologue continues to use only `xBuf_` and `residualBuf_`.
- Keep all arithmetic, `PipeBarrier` calls, output stores, UB allocations, mode dispatch, wide paths, narrow paths, and dtype paths unchanged.
- Reuse the V012 paired Parent/Candidate CMake and runner; no second runner or execution path.

## Acceptance sequence

EDIT -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT

Targeted first check: generic cached-row FP16 with at least two rows per effective block, then the existing harness correctness path. Local is conditional on compile and correctness passing and `FREE_HBM >= 100 MB`.
