# REVISION DECLARATION — STORE-EPILOGUE-X V001

ROUTE=STORE-EPILOGUE-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS_ID=STORE-H2B
SINGLE_HYPOTHESIS=Single-row resident writeback merge: one row's
  contiguous finished output run is written back in one DataCopyPad
  call instead of per-tile calls. Intra-row only; copy primitive and
  per-site ring/event discipline unchanged; rows whose UB run start is
  not 32B-aligned keep the parent's per-tile stores.
INSTANCE_SCOPE=wide-FP32 full-cache output pass + generic FP32 cacheRow
  arm. STORE-H2(a) inter-row merge dropped (R015). Site :1218
  (ProcessFp32FullRowOutputPipelined) skipped — its ring overlaps store
  with next-row compute; merging needs the ring semantics rewritten.
  Sites :2796/:2889/:1922 (ProcessWideFp16/Bf16CachedRows,
  ProcessWideFp32CachedRows) are uncalled dead code in the frozen
  parent (V003 collapsed all wide dispatch into
  ProcessWideFp32FullCacheRows / ProcessWideLowPrecision) — no live
  edit possible there; live FP16/BF16 wide path is
  ProcessWideLowPrecision whose per-tile stores use a reused single
  staging tile (outputLocal), not a contiguous row run, so the H2B
  legal predicate does not hold there.
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
WHY_NOT_DUPLICATE_MAIN1=WIDE-X-FRESH4: no tile-width/tiling change;
  MODE-X-R015C: no row-block mapping / segment / inter-row batching;
  MIX-A: no wait placement change; DTYPE-SPECIAL-X: no copy-primitive
  or arithmetic change; ALIGN-TAIL-X: no DataCopy/Pad choice, no
  bulk/tail split (Store helper untouched).
WHY_NOT_DUPLICATE_MAIN2=VECTOR-MATH owns denominator; EPILOGUE V001-3
  owned epilogue arithmetic (closed by BOTTLENECK-NOTE); STORE-H2(a)
  inter-row merge dropped (R015); ASYNC-TRIPLE-X owns store rings —
  rings kept verbatim at every touched site.
SINGLE_CHANGE_AUDIT=PENDING (one variable: writeback run length per row)
MAIN_APPROVAL=YES (MAIN-APPROVAL-V001.md)
APPROVAL_DOC=phase4/research/STORE-EPILOGUE-X/MAIN-APPROVAL-V001.md
SPEC_DOC=phase4/research/STORE-EPILOGUE-X/STORE-H2B-SPEC.md
CORRECTNESS_FIX=NONE REQUIRED (values bit-identical by construction)
SOURCE_SHA=06564134e4930eb3e2ca30297b9bda548135d1aa8a1f43238b75dafc8a8c09f9
COMPILE_RC=0/0
LINK_RC=0/0 (se_full_link 4d57c23c...)
CORRECTNESS=PASS_VS_PARENT 26 shapes (2 pre-existing wide-FP32 golden fails both sides)
LOCAL_VERDICT=PENDING
MEASUREMENT=RUNNING (d6)
