# MAIN APPROVAL — COEFF-LOCALITY-X V001

Date: 2026-09-27
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

## APPROVED SINGLE HYPOTHESIS

H1 — Param double-buffer prefetch in FP32 wide output pass (V011 sibling pattern).

In `ProcessWideFp32FullCacheRows` pass 2, replace serialized
`Load(gamma/bias) → SyncMTE2ToV → apply` with a 2-deep param MTE2 pipeline:
while tile t is applied, tile t+1 gamma/bias are in flight into a second staging pair.
Copy the pattern already in `ProcessWideLowPrecision` (prd0/prd1).

Change is load timing and staging residency only. Arithmetic unchanged.
Existing SyncMTE2ToV before each apply stays.

Scope note: this is coefficient-load locality inside an existing wide FP32 function,
NOT a wide-path redesign and NOT a multi-row batch DMA mode. The donor is the
in-kernel lowp sibling, not another route.

FORBIDDEN: no multi-row DMA, no rows/block, no dtype split, no reduction,
no scheduling, no sync-removal as the win claim, no wide architecture redesign.

## Measurement

Primary: 1x32768 FP32, 1x16384 FP32, 8x32768 FP32 (est -8% to -25%).
Control: 1x4096 FP32 (expect ~= 0).
Falsification: if 1x32768 clean |Δ|<=2%, param MTE2 is not the bottleneck.
