# MAIN APPROVAL — REDUCE-HIER-X V003

Date: 2026-09-27
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
V002_DISPOSITION=LOCAL_REJECTED (H2 pairwise tree +6.6% on 1x32768)

## APPROVED SINGLE HYPOTHESIS

H3 — Short-span ReduceSum + single barrier, keep one API reduce per tile.

Change only HOW the per-tile square-sum is reduced:
- Keep one `AscendC::ReduceSum` per tile (do not replace with Add tree).
- Feed ReduceSum a SHORT span: first fold the tile's square vector down to
  `kReduceSpan` (e.g. 64 or 128) elements with ONE pairwise Add level group
  (batched Adds with a single PipeBarrier, not per-level barriers),
  then ReduceSum that short span.
- Goal: cut ReduceSum's internal work (it is cheaper on short spans) while
  paying at most ONE extra PipeBarrier per tile instead of 12.

Alternative if short-span still loses: keep full-span ReduceSum but issue the
per-tile ReduceSum results into an 8-lane packed accumulator and do a single
end-of-row ReduceSum (this is V001 H1 — already measured -3.8%). Do not
re-test V001 as V003.

FORBIDDEN: no scheduling, dtype, DMA, wide, epilogue, gamma-cache, sync-removal.

## Measurement

Shapes: 1x32768 (cleanest), 1x16384, 8x8192, 1x4096 control.
Compare vs frozen parent (not vs rejected V002).
