# CROSSROW-EPI-PROLOGUE-X V002

AGENT_ID: W5-R01 CROSSROW-EPI-PROLOGUE-X
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V002
STATUS: DECLARED

## Direct parent and source

DIRECT_PARENT: W5-R01 CROSSROW-EPI-PROLOGUE-X V001
DIRECT_PARENT_GIT_COMMIT: 463b239f521d5f8db5928128aa501793c4f3daa4
DIRECT_PARENT_SOURCE_SHA256: 3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
DIRECT_PARENT_SOURCE_PATH: ../V001/Candidate.asc

## Selected hypothesis

HYPOTHESIS_ID: H1_MID_EPILOGUE_ISSUE
HYPOTHESIS: In the generic cached-row path, issue the existing next-row first-tile x/residual MTE2 prologue after the current row's first epilogue tile completes and before the second tile begins. This should avoid front-loading the DMA before the epilogue while retaining vector/MTE3 work to cover the copy.

## Why not duplicate

W3 CROSSROW-FULL-PIPELINE V012 changes the FP16 pass-2 parameter double-buffer phase; this revision moves only the existing cross-row x/residual prologue within the generic cached-row path. Main-1 CASE47 is a narrow `ProcessNarrowMidOverlap` scalar-handoff direction, CASE14 is same-row D-slice/intra-row overlap, and Tiny is fixed-cost/dispatch overhead. V002 changes none of those axes.

## Dispatch and lifetime boundary

- Target probe: FP16 `rows=16,width=6144,blocks=8`; effective blocks=8 and localRows=2, so the generic cached-row `Process()` path has two 4096-element tiles.
- `M=1` negative control: `rows=1,blocks=1` gives localRows=1; the row-successor guard is false, so V002 must issue no prologue and must not change the single-row buffer path.
- The existing pass-1 `SyncVToMTE2()` remains before the prologue; the next row retains its existing `SyncMTE2ToV()` consumer boundary.
- The prologue writes only `xBuf_` and `residualBuf_`; current-row `valueFp32Buf_` remains the epilogue source and its lifetime is unchanged.

## Scope

- One Kernel performance change: move the existing guarded two-load block from the V001 pre-epilogue position to the boundary after epilogue tile zero and before tile one.
- Parent is the V001 Candidate source; Candidate is this single placement change.
- Reuse the existing V001 paired runner executable and CMake entry; no new runner implementation or execution chain.
- No changes to ABI, arithmetic, output stores, queue allocation, shape arguments, warmup policy, event timing, or PC/CP order.

## Acceptance sequence

EDIT -> BUILD -> CORRECTNESS (target plus M=1 control) -> LOCAL (target only) -> RESULT -> COMMIT

LOCAL is conditional on build/correctness and is not promotable if target-device pollution remains.
