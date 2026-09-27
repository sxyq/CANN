# REVISION DECLARATION — STORE-EPILOGUE-X V002

ROUTE=STORE-EPILOGUE-X
REVISION=V002
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS_ID=STORE-H2B-GATED
SINGLE_HYPOTHESIS=Single-row resident writeback merge WITH tileCount>=4
  gate: merge one row's contiguous finished output run into one
  DataCopyPad writeback only when the row has >= 4 output tiles and the
  UB run start is 32B-aligned; tileCount<4 keeps the parent per-tile
  store path.
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
CHANGE_SITE=ProcessWideFp32FullCacheRows output pass ONLY (frozen
  :2208-2234 store block). Generic Process second pass reverted to
  parent byte-identical — its tileCount is capped at 2
  (rowWidth <= kCacheElems = 8192 / kTileElems = 4096), so the gate is
  never open there and the parent per-tile path is the correct form.
BOUNDARY=Intra-row only; Store helper untouched (no DataCopy/Pad choice
  change); 2-deep MTE3 ring discipline preserved verbatim at the touched
  site (fallback per-tile block is the parent block); no dtype split; no
  reduction/scheduling/SCHED/sync change.
WHY_NOT_DUPLICATE_MAIN1=Same as V001: WIDE-X-FRESH4 no tile-width
  change; MODE-X-R015C no row-block/segment/inter-row batching; MIX-A no
  wait placement change; DTYPE-SPECIAL-X no copy-primitive/arithmetic
  change; ALIGN-TAIL-X no DataCopy/Pad selection.
WHY_NOT_DUPLICATE_MAIN2=Same as V001: VECTOR-MATH denominator untouched;
  EPILOGUE V001-3 arithmetic closed; STORE-H2(a) inter-row dropped;
  ASYNC-TRIPLE-X rings kept verbatim.
V001_TO_V002_DELTA=one predicate (tileCount>=4) + generic-site revert.
  V001 SOURCE_SHA 06564134e4930eb3e2ca30297b9bda548135d1aa8a1f43238b75dafc8a8c09f9.
SINGLE_CHANGE_AUDIT=PENDING (one variable: the tileCount gate on the
  existing merge)
MAIN_APPROVAL=YES (MAIN-APPROVAL-V002.md)
CORRECTNESS_FIX=NONE REQUIRED (gate-off branches are parent code;
  gate-on writeback is bit-identical values to V001's merge)
SOURCE_SHA=PENDING
COMPILE_RC=PENDING
LINK_RC=PENDING
CORRECTNESS=PENDING
LOCAL_VERDICT=PENDING
MEASUREMENT=PENDING
