# COEFF-LOCALITY-X V004 — Revision Declaration

Date: 2026-09-29
ROUTE: COEFF-LOCALITY-X
REVISION: V004
REVISION_KIND: PERFORMANCE_SINGLE_HYPOTHESIS
DIRECT_PARENT: FROZEN_R31B_V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE: 45.16
OFFICIAL_ANCHOR: 45.16
CONTEXT_CLASS: FROZEN_STRONG_BASELINE_COEFF_LOCALITY
APPROVAL: MAIN-2 (inbox 2026-09-28) — NH-1 (=H3 精化) approved as V004.
  V003 (NH-2) confirmed LOCAL_REJECTED; param MTE2 timing closed on large-D.
  V001/V003 are NOT parents (route LOCAL_BEST = NONE).

## SINGLE_HYPOTHESIS

NH-1 — generic path full-row parameter preload for single-row multi-tile cores.

Mechanism: in `Process()`, `cacheParams = cacheRow && localRows > 1` (`:245`)
excludes single-row cores even when tileCount > 1, forcing per-tile
`Load(gamma/bias)` inside the output pass (`:390-391`, 2×tileCount serialized
MTE2 round-trips on the critical path). Extend the condition to
`cacheRow && (localRows > 1 || rowWidth > kTileElems)` so a single-row core
with D∈(4096,8192] reuses the EXISTING full-row preload block (`:249-281`):
gamma/bias load once into the already-allocated `gammaBuf_`/`biasBuf_`
(kCacheElems=8192, covers rowWidth≤8192) before the row loop. The preload
block itself is reused byte-identical (its per-tile loop already fills the
full row). Consumers (`:387` `:393` `:417` `:430` `:442` `:464`) already branch
on `cacheParams` with `[col]` indexing and need no edit.

WHY_IT_HELPS: output-pass critical path loses 2×tileCount param MTE2
round-trips and the per-tile `SyncMTE2ToV`; loads hoist before pass 1 where
they hide behind x/residual DMA + reduction work.

Scope refinement (2026-09-29, before code): the preload block (`:249-281`) is
SHARED with the localRows>1 population. Consolidating its per-tile loop to a
single full-row Load (NH-4 detail) would also change the multi-row path, i.e.
a population outside the approved scope. OFAT therefore keeps the preload
block byte-identical and the V004 diff is the `cacheParams` condition only
(one line). Descriptor consolidation is deferred, not part of V004.

UB budget / tile-shrink audit (mandatory):
- UB increment: **0**. `gammaBuf_`/`biasBuf_` are allocated unconditionally in
  generic Init (`:121-122` FP32, `:126-127` FP16, `:137-138`+`:154-155` BF16)
  sized kCacheElems/kTileElems regardless of `cacheParams`.
- tileElems: **unchanged** (generic path uses fixed kTileElems=4096; does not
  enter `ChooseWideFullYRows`).
- Extra MTE2: **none** — same bytes and same descriptor count, hoisted off
  the output-pass critical path.

## FORBIDDEN (per Main-2 approval, unchanged)

- no staging slot additions, no stripe residency
- no emission-timing changes (V003's domain — closed)
- no wide FP32 / NarrowMidOverlap / multi-row / host / reduction / store edits
- no sync-removal as the performance variable

## OFAT SCOPE

Only: `Process()` cacheParams condition (`:245`), one line. No preload-block
edits, no Init / host / CMake / runner, no other function.

## TARGET SHAPES

Mechanism fires on generic path, localRows==1, tileCount==2 → D∈(4096,8192].
Under host clamp (blockCount=min(cores,rowCount)) every measured shape has
localRows=1, so all mid-D shapes fire.

- Primary probes: 1x8192 FP32, 1x6144 FP32, 2x8192 FP32, 3x6144 FP32
- Controls (expect ≈0): 1x4096 FP32 (NarrowMidOverlap path, untouched),
  1x32768 FP32 (wide path, untouched)
- Correctness battery: full 18-shape set (FP32/FP16/BF16, single+multi-tile)

## MEASUREMENT PLAN

Protocol: 本地性能测试规范.md — warmup≥45, samples=41, same-binary blocks=2,
≥4 (6) interleaved P/C pairs, device events, d4 preferred.
Control shapes included in the same run to read the measurement-bias floor
(V003 practice, Main-2 confirmed).

Falsification: if 1x8192 clean |Δ| ≤ control bias, descriptor hoist on the
generic mid-D path is not material; combined with V003's large-D closure,
the COEFF lane then goes to LANE_NEEDS_PLANNING_REVIEW (per Main-2).
