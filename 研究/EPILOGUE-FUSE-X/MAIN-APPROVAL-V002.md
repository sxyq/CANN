# current route recordS — EPILOGUE-FUSE-X V002

Date: 2026-09-27
Recordr: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
V001_DISPOSITION=NEEDS_ONE_MORE_LOCAL (SCALE-FOLD within ±5% noise band)

## SELECTED SINGLE HYPOTHESIS

V002-1 VMLA FUSED AFFINE.

Replace each tile's `Mul + Add` epilogue pair with `AscendC::MulAddDst`
(`dst += src0 * src1`, i.e. vmla). Precompute per-row scale (gamma * invRms)
and bias accumulators once per row.

Arithmetic identity unchanged. FP32 intermediates unchanged. Uniform across T.
No dtype-specific path. No sync/fence change. No reduction/scheduling/mode/DMA change.

Expected: V passes 6→4 on 2-tile rows; per-tile UB writeback 3→1; est. 10–20%.

DEFERRED: V002-2 (BF16 output-domain affine) and V002-3 (row-wide issue width) are NOT in this revision.

## After implementation

server3 compile/link + correctness (FP16/BF16/FP32).
Timing shapes: 2x4096, 4x8192, 8x256 (non-wide), plus wide control ~= 0.
