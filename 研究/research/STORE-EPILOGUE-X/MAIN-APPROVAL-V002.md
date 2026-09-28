# MAIN APPROVAL — STORE-EPILOGUE-X V002

Date: 2026-09-27 (C2C overnight)
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
V001_DISPOSITION=NEEDS_ONE_MORE_LOCAL (large −7.61% 6/0; medium slight regression)

## APPROVED SINGLE HYPOTHESIS

STORE-H2B-GATED — single-row resident writeback merge WITH tileCount>=4 gate.

Same mechanism as V001 (merge contiguous UB output-tile run into one
DataCopyPad), but apply it only when the row's tileCount >= 4.
For tileCount < 4 keep the parent per-tile store path.

Rationale: V001 timing shows merge win grows with tileCount (8→1 large win)
while tileCount=2 delayed writeback loses store/compute overlap (medium cost).

Boundary unchanged: intra-row only; no Store-helper geometry change; no
store-ring/event change; no dtype split; no reduction/scheduling/SCHED/sync-as-win.

## Measurement

1x32768 (expect keep ~-7%), 2x8192, 2x6144 (expect ~=0, no medium regression),
8x8192, 2x256 control, 1x16384 FP16 control.
