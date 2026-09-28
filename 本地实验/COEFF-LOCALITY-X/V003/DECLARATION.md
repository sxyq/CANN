# COEFF-LOCALITY-X V003 — Revision Declaration

Date: 2026-09-29
ROUTE: COEFF-LOCALITY-X
REVISION: V003
REVISION_KIND: PERFORMANCE_SINGLE_HYPOTHESIS
DIRECT_PARENT: FROZEN_R31B_V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE: 45.16
OFFICIAL_ANCHOR: 45.16
CONTEXT_CLASS: FROZEN_STRONG_BASELINE_COEFF_LOCALITY
APPROVAL: MAIN-2 APPROVAL (inbox 2026-09-28) — NH-2 approved as V003 SINGLE_HYPOTHESIS;
  Main-2 ruled NH-2 is NOT an H1 rerun (zero staging increment, tileElems pinned 4096,
  MTE2 count unchanged). H3/NH-1 deferred until after V003.

## SINGLE_HYPOTHESIS

NH-2 — wide FP32 pass 2 split-phase parameter MTE2 emission in
ProcessWideFp32FullCacheRows.

Mechanism: keep the existing two staging slots only (xBuf_ = gamma slot,
residualBuf_ = bias slot). Zero new staging buffers. tileElems stays at
kWideFullYTileElems = 4096 (Init / ChooseWideFullYRows untouched).

Emission timing change only:
- tile 0 params load at the top of the pass-2 loop (as today).
- After the last Mul consumes gamma (xBuf_ free), issue the NEXT tile's
  gamma Load into xBuf_.
- After the last Add consumes bias (residualBuf_ free), issue the NEXT
  tile's bias Load into residualBuf_.
- Each early issue is preceded by SetFlag/WaitFlag V_MTE2 on the matching
  slot (V must finish reading before MTE2 overwrites) — the same event
  idiom as ProcessWideLowPrecision's prefetch.
- Prefetched params are waited on by the EXISTING SyncMTE2ToV before the
  next apply. Arithmetic, tileElems, MTE2 transaction count (2 per tile),
  host, and the MTE3 store-drain rule are unchanged.

Goal: hide param MTE2 latency of tile t+1 behind tile t's Add (gamma) and
behind tile t's MTE3 stores + drain (bias), instead of paying it serialized
before apply at the top of each iteration.

WHY_NOT_DUPLICATE: no other MAIN-2 lane owns param MTE2 emission timing in
the wide FP32 output pass. Not reduction, not store/epilogue, not scheduling,
not multi-row DMA, not dtype split, not wide-architecture change.

WHY_THIS_IS_NOT_H1_RERUN (per Main-2 ruling): H1 added a 2-deep staging
pair (ioTiles 2→4) which forced tileElems 4096→2560 (V001 failure mode).
V003 adds ZERO staging bytes and does not enter ChooseWideFullYRows.

## FORBIDDEN (unchanged, confirmed in approval)

- no staging slot additions
- no stripe residency
- no multi-row mode / rows-per-block / distribution change
- no sync-removal as the performance variable
- no reduction / store / epilogue / scheduling changes

## OFAT SCOPE

Only: pass-2 param MTE2 emission timing in ProcessWideFp32FullCacheRows.
Sites: the pass-2 tile loop inside ProcessWideFp32FullCacheRows
(parent ~:2174-2245). Init untouched. No host / CMake / runner changes.

## MEASUREMENT PLAN

Primary large-D probes: 1x32768 FP32, 1x16384 FP32, 8x32768 FP32.
Control: 1x4096 FP32 (expect ≈ 0).
Protocol: 本地性能测试规范.md — warmup≥45, samples≥21 (41), same-binary
blocks, ≥4 interleaved P/C pairs, device events.

Falsification: if 1x32768 clean |Δ| ≤ noise with tileElems constant at 4096,
param MTE2 timing is formally not the large-D bottleneck; COEFF large-D
dimension closes and NH-1 (H3) becomes the only remaining axis item.
