# MAIN APPROVAL — REDUCE-HIER-X V002

Date: 2026-09-27
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

## APPROVED SINGLE HYPOTHESIS

H2 — Full manual vector reduction tree (R011, 0–1 GetValue).

Replace each tile's `AscendC::ReduceSum` with an explicit pairwise / 4-way FP32 `Add` tree
inside UB. Keep V001's eager running-fold into the row accumulator. Result: per-row
V/S handoff count drops from `tileCount + 2` to 2 (one SyncVToS before GetValue,
one SyncSToV after).

Target win: large-D multi-tile rows (D=8192..32768, tileCount=2..8), estimated -18% to -35%.

FORBIDDEN: no scheduling, no dtype split, no DMA, no wide specialization, no epilogue change,
no gamma/bias caching, no sync-removal as the performance variable.

## Implementation notes

- kTileElems = 4096 (not 1024). tileCount = ceil(D/4096) = 1/2/4/8 for D=4096/8192/16384/32768.
- Tree must produce bit-similar FP32 sums within tolerance (document max abs vs parent).
- Use 32B-aligned slots (kSmallFp32ScalarStride) for any 1-element ops.
- Cover the same sites as V001 H1: Process() first pass, ProcessWideFp32FullCacheRows,
  ProcessFp32FullRowOutputPipelined.

## After implementation

server3 compile/link + correctness (FP16/BF16/FP32, multi-tile + single-tile).
Timing when a device lease is free: shapes D=4096/8192/16384/32768 + single-tile control.
