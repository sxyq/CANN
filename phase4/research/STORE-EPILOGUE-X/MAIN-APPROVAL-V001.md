# MAIN APPROVAL — STORE-EPILOGUE-X V001

Date: 2026-09-27 (C2C overnight)
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
VECTOR_MATH_V002_DISPOSITION=MEASUREMENT_BLOCKED (not a parent)

## APPROVED SINGLE HYPOTHESIS

STORE-H2B — single-row resident writeback merge.

For one row, merge a contiguous run of UB output tiles into a single
DataCopyPad writeback instead of per-tile stores.

BOUNDARY (hard):
- Intra-row only. Crossing row boundary is multi-row DMA (R015) — FORBIDDEN.
- Do NOT change the Store helper's DataCopy/DataCopyPad choice (ALIGN-TAIL axis).
- Preserve all store-ring / event discipline at each site (ASYNC-TRIPLE axis).
- Site :1218 ring-overlap may be skipped if it cannot be changed without
  touching sync discipline.

Preferred sites (frozen seed):
- :2224 wide FP32 (8 tiles/row)
- :2796 / :2889 wide FP16/BF16
- :1922
- :468-489
Parent already has single-block 32 KiB writeback precedent at :1452 / :1659.

FORBIDDEN: input DMA redesign, wide-D mode, row ownership, multi-row scheduling,
dtype specialization, reduction rewrite, sync/fence removal as the win claim,
SCHED row grouping.

## After implementation

exact source SHA → server3 SHA → compile (numeric RC) → link (numeric RC)
→ executable SHA → full correctness (FP16/BF16/FP32, small/medium/large).
Do not wait for performance window.

Then same-binary + P/C on small/medium/large with special attention to
the medium band that killed INTEGRATION-X. No Online this cycle.
