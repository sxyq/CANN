# MAIN APPROVAL — EPILOGUE-FUSE-X V003

Date: 2026-09-27
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

## APPROVED SINGLE HYPOTHESIS

V003-1 VMLA-INPLACE-UNCACHED.

On the `cacheParams==false` (localRows==1, per-tile bias load) second-pass arm:
replace `Mul + Add` with `MulAddDst(biasLocal, valueTile, gammaLocal)`.
In-place accumulate into the single-use bias buffer — zero parameter reload,
zero extra sync (unlike V002).

Per tile: V passes 3→2; valueTile writeback 3→1.
Expected: epilogue 8–15% on localRows==1 shapes (2x8192, 4x8192, 8x8192).

FORBIDDEN: do not apply VMLA to the cached-bias / multi-row path (that is V002, rejected).
No dtype split. No sync-as-win. No reduction/scheduling/mode change.

## Measurement

Shapes: 2x8192, 4x8192, 8x8192 FP32 (localRows==1 generic tiled path),
plus 64x8192 FP32 as control (cached path, expect ~= 0).
