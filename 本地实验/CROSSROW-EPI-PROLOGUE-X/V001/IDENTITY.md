# Identity Result

ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STAGE: IDENTITY

DIRECT_PARENT: W3 CROSSROW-FULL-PIPELINE-CHAMPION-X V012
DIRECT_PARENT_GIT_COMMIT: b68fcebe6492e2c34c7fcc03c7506a74a0ad1a3b
DIRECT_PARENT_SOURCE_SHA256: 6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794
PARENT_ASC_SHA256: 6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794
CANDIDATE_ASC_SHA256: 3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
RUNNER_SHA256: 6096c0bd0fa6f26384a2967adb7222d8d4712a781b855cc4579152c3e3fd502f

## Candidate delta

`Candidate.asc` differs from `Parent.asc` only by moving the existing cached-row next-row first-tile `xBuf_`/`residualBuf_` load block from the post-pass-1, pre-invRms position to immediately after `SyncSToV()` for the current row and before the epilogue. The `cacheRow` guard, row offset, tile length, input buffers, retained `valueFp32Buf_` y storage, arithmetic, events, and UB allocations are unchanged. The exact source diff is the two-way `Parent.asc`/`Candidate.asc` comparison at the V001 source boundary.

The only non-kernel change is the existing paired runner's width guard, widened to admit the selected generic cached-row probe; its allocation, calls, comparison, timing, and output logic remain unchanged.
