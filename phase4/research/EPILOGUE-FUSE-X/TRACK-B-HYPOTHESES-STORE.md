# TRACK-B HYPOTHESES — OUTPUT-STORE / EPILOGUE-STORE PATH

Date: 2026-09-27
Route: EPILOGUE-FUSE-X (research note only — no implementation in this tree)
Parent: FROZEN R31B-V011 (`R31B-V011-LP-ROW-PIPELINE_kernel.asc`,
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3)
OFFICIAL_ANCHOR=45.16

Scope: the UB→GM output side only — store descriptor form, store
granularity, convert-staging placement, and store-side event overhead.
This note supersedes nothing: the earlier epilogue arithmetic hypotheses
(SCALE-FOLD, CONVERT-ONCE, VMLA) in `TRACK-B-HYPOTHESES.md` were about
the post-`invRms` V-pipe chain and are closed by `BOTTLENECK-NOTE.md`
(3 V-passes is not the binding constraint on 5–12 µs kernels).

Context: INTEGRATION-X V001 scored 41.93 on Official (below the 45.16
anchor); SCHED row-group ownership is banned from the Champion path.
The remaining weak Official cases on R31B-V011 are concentrated in
mid/large workloads (per-case: a ~16.5 ms case against a 3.75 ms best,
and several 50–600 µs cases at 2.5–4× best). Output-store pressure grows
with D and row count, so the store side is the remaining unexplored
surface that scales into those cases.

## Store-side map of the frozen parent (R31B-V011)

| site | store form | granularity | per-store event cost |
|---|---|---|---|
| `Store` helper `:3377-3383` | `DataCopyPad`, 1 block, runtime `blockLen` | any count | — (shared by all callers) |
| `Process()` 2nd pass FP32 `:466-473` | pad from `valueTile` | per tile | `SyncVToMTE3` + drain; drain deferred to row end when `cacheRow` (V006 rule) |
| `Process()` 2nd pass FP16 `:475-481` | pad from `outputBuf_` after `FromFloat` | per tile | full `SyncVToMTE3`+`SyncMTE3ToV` every tile |
| `Process()` 2nd pass BF16 `:483-490` | `FromFloat` into `xBuf_`/`gammaBuf_` alias, then pad | per tile | full round trip every tile; store source aliases input staging (`xBuf_`) |
| `ProcessNarrowMidOverlap` `:586-610` | same 3-way split | one row | one round trip per row |
| `ProcessFp32FullRowOutputPipelined` `:1218` | pad from `valueTile` | per 4096-tile | per tile |
| `ProcessFp16/Bf16FullRowOutputPipelined` `:978-982`, `:1089-1100` | `FromFloat` + pad | per tile | per tile |
| `ProcessSmallFp32Batched` `:1542-1557` | pad from `valueRow` | **per row** (2-deep ring) | per row |
| `ProcessSmallLowPrecisionContiguousBatched` `:1751-1773` | one pad of `totalElems` | **per batch** | one round trip per batch |
| `ProcessWideFp32FullCacheRows` output pass `:2208-2235` | pad from resident y row | per (row, tile) | 2-deep ring (V006) |
| `ProcessWideFp16CachedRows` `:2788-2797` | `FromFloat` + pad | per tile | per tile |

Three structural observations:

1. **Every store is `DataCopyPad`.** The helper never uses the aligned
   bulk copy even when `count * sizeof(T)` is a 32-byte multiple and both
   addresses are 32-byte aligned — the common case for interior
   `kTileElems=4096` tiles (`:1242`).
2. **Granularity is inconsistent across paths with identical memory
   layouts.** `ProcessSmallLowPrecisionContiguousBatched` stores a whole
   batch in one descriptor (`:1773`); `ProcessSmallFp32Batched` stores
   the same layout row by row (`:1553`). Wide paths store tile by tile
   even when the full y row is already resident in UB (`:2224`).
3. **FP16/BF16 arms drain MTE3 to V after every tile store** (`:481`,
   `:490`) while the FP32 arm already defers that drain to row end
   (`:471-473`, the V006 rule). The per-tile drain exists because the
   convert staging buffer is overwritten by the next tile's `FromFloat`.

---

## STORE-H1 — ALIGNED-BULK-STORE: bulk `DataCopy` for aligned full blocks, `DataCopyPad` for the tail only

**MECHANISM**
Split the `Store` helper into two execution paths on one condition:
if `count * sizeof(T) % 32 == 0` and `(offset * sizeof(T)) % 32 == 0`,
issue the aligned bulk copy (`AscendC::DataCopy` / contiguous block form);
otherwise keep `DataCopyPad`. Interior tiles of any row whose width and
offset are element-aligned (all `kTileElems` tiles except a ragged last
tile) take the bulk path with a constant block length; the ragged tail
tile — and only the tail — keeps the pad path. The tail is isolated, not
specialized: one helper, one branch on an alignment predicate, no dtype
or width-mode split.

**BOTTLENECK EVIDENCE**
`Store` `:3377-3383` issues `DataCopyPad` unconditionally with
`DataCopyExtParams(1, count*sizeof(T), 0, 0, 0)`. Pad copies pay the
pad-logic path on the MTE3 side even when the block is an exact aligned
multiple. Full tiles are the dominant store count on mid/large rows:
D=8192 → 2 stores/row, D=32768 → 8 stores/row (`tileCount =
rowWidth/kTileElems`), all identical 16 KiB blocks when
`rowWidth % 4096 == 0`. Every path in the store map above funnels
through this one helper, so the primitive-level fix covers mid and large
shapes without touching any call site.

**EXPECTED AFFECTED SHAPES**
- large: every multi-tile wide path (8x8192, 1x32768, 8x32768 classes) —
  2–8 identical full tiles per row, all bulk-eligible.
- medium: multi-tile tiled rows (4096 < D ≤ 8192 generic/overlap paths).
- small: single-tile rows mostly unaffected (one store/row, often the
  tail-sized pad path anyway); no regression expected.

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only: the `Store` helper
(`:3377-3383`), optionally with a second inline `StoreBulk` helper used
by the same callers. No call-site edits, no Init change, no host /
CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
No dtype split (the predicate is type-width arithmetic, identical code
for every `T`), no mode redesign, no multi-row DMA, no rows-per-block,
no wide-D specialist, no row ownership change, no event/sync change.
The DMA geometry is unchanged — the same bytes move in the same single
block; only the copy instruction variant differs.

**WHY_NOT_DUPLICATE_EPILOGUE/VECTOR-MATH**
- EPILOGUE-FUSE-X arithmetic hypotheses: post-`invRms` V-pipe chain;
  this starts after the affine, at the MTE3 issue.
- VECTOR-MATH-X: denominator/`invRms` production; untouched.
- ALIGN-TAIL-X (donor history): bulk/tail copy specialization on the
  input side. This is the output side, a different helper, and the
  predicate is re-derived for stores. If Main considers bulk/tail
  policy single-owned, this hypothesis is the output-side instance of
  the same policy and should be labeled as such rather than rediscovered.
- ASYNC-TRIPLE-X: store event schedule unchanged.

**CORRECTNESS RISK**
Low. The only hazard is the alignment predicate itself — a store that
takes the bulk path with a non-aligned offset or length will fault or
silently corrupt. Mitigation: predicate must use byte counts
(`count * sizeof(T)` and `offset * sizeof(T)`), not element counts;
FP16/BF16 element alignment is not byte alignment. The tail path is
byte-identical to today's behavior. NPU matrix on all 3 dtypes before
any timing.

**MEASUREMENT PLAN**
1. Same-binary qualification of parent and candidate on probe shapes.
2. One revision, one diff: only the helper branch.
3. Interleaved P/C pairs: one large multi-tile (8x8192), one medium
   multi-tile (2x4096), one small control (33x100). warmup=45,
   samples=41, 6 pairs, device-event primary.
4. Falsifier: if 8x8192 shows no direction-consistent drop while
   single-tile is flat, pad-vs-bulk is not the cost — move to STORE-H2.

---

## STORE-H2 — CONTIGUOUS-STORE-COALESCING: one descriptor per contiguous UB run instead of per tile or per row

**MECHANISM**
Where finished output already occupies one contiguous UB run whose
length equals the GM span it maps to, issue a single store for the whole
run and drop the per-tile / per-row store loop. Two instances, same
variable (store descriptor granularity):
(a) batched small/mid paths — store `batchRows * width` elements once
    per batch, as `ProcessSmallLowPrecisionContiguousBatched` already
    does (`:1773`), instead of the per-row loop in
    `ProcessSmallFp32Batched` (`:1542-1557`);
(b) wide full-y paths — store the complete resident y row once
    (`rowWidth` elements) instead of `tileCount` tile stores
    (`:2224`, `:2788-2797`).
The 2-deep store ring stays; it just has fewer entries to manage.

**BOTTLENECK EVIDENCE**
The parent already demonstrates both forms working: `:1773` (one batch
store) and the V006 ring. The FP32 batched path stores the identical
layout row-by-row (`:1553`) — 4 descriptors + 4 event round trips where
1 suffices. Wide paths hold the full row in UB (`valueFp32Buf_` /
`gammaBuf_` for half y) yet still emit `tileCount` pad stores per row;
at D=32768 that is 8 descriptors and up to 8 V→MTE3 handshakes per row
on top of STORE-H1's primitive cost.

**EXPECTED AFFECTED SHAPES**
- large: wide full-y paths (8x8192 via the 8192 full-row path, 1x32768,
  8x32768) — descriptor count drops by `tileCount`.
- medium: batched rows 1024 < D ≤ 4096 with `localRows > 1`.
- small: 33x100-class only when a core owns multiple rows (then
  coalescing needs the rows contiguous in UB — true for
  `ProcessSmallFp32Batched`'s `valueLocal[batchRow*width]` layout).

**FILES/FUNCTIONS TO TOUCH**
`ProcessSmallFp32Batched` (`:1542-1557`), `ProcessWideFp32FullCacheRows`
output pass (`:2208-2235`), `ProcessWideFp16CachedRows` (`:2788-2797`),
and the full-row pipelined output loops (`:1218`, `:978-982`,
`:1089-1100`). Store helper unchanged. No host / CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
Descriptor count only. No DMA shape invention (one block of the same
total bytes that were already moving), no rows/block, no mode, no dtype
split (the coalesced store is typed by `T` exactly as today's), no
ownership change.

**WHY_NOT_DUPLICATE_EPILOGUE/VECTOR-MATH**
- EPILOGUE arithmetic hypotheses: untouched V-pipe chain; the affine
  writes the same buffer, only the write-back issue changes.
- VECTOR-MATH-X: denominator stage, untouched.
- BATCH-RESIDENT-X: multi-row *input* batch DMA and batch ownership;
  this changes only the output descriptor of already-finished rows.
- COEFF-LOCALITY-X: parameter load side; untouched.

**CORRECTNESS RISK**
Low–medium. The coalesced run is legal only when UB rows are contiguous
*and* the GM rows are contiguous with no gap — true for all flattened
row-major output here (`row * rowWidth + col`), but the check must be
explicit per site. The wide half-y path stores from `gammaBuf_`-backed
y rows; coalescing must not overrun the resident row length. Tail rows
(when `rowWidth % kTileElems != 0`) keep a per-row length; the coalesced
store's block length becomes `rowWidth * sizeof(T)`, which is still a
legal pad block.

**MEASUREMENT PLAN**
Same protocol as STORE-H1. Probe shapes: 8x256 + 32x256 (batched
instance), 8x8192 + 1x32768 (wide instance), 33x100 control. Falsifier:
if batched shapes move but wide does not (or vice versa), split the two
instances into separate revisions — they may have different ceilings.

---

## STORE-H3 — DEFERRED-STORE-DRAIN: extend the V006 deferred MTE3 drain to the FP16/BF16 tile stores with 2-deep convert staging

**MECHANISM**
The FP32 arm already defers `SyncMTE3ToV` to the end of the row
(`:471-473`); the FP16/BF16 arms drain after every tile (`:481`, `:490`)
because the single convert staging slot (`outputBuf_` / `xBuf_` alias)
is overwritten by the next `FromFloat`. Give the convert staging a
2-deep footprint (alternate slots per tile) and move the drain to
row end, matching the FP32 rule. The V→MTE3 ordering before each store
is unchanged; the MTE3→V wait is *deferred*, not removed — the same
pattern the parent already uses and documents as the V006 rule.

**BOTTLENECK EVIDENCE**
`:475-481` (FP16) and `:483-490` (BF16) in `Process()` execute
`SyncVToMTE3; Store; SyncMTE3ToV` per tile — a forced MTE3 completion
wait on the V-pipe critical path for every tile of every row, while the
FP32 arm of the same loop does it once per row. At D=8192 FP16 that is
2 forced round trips per row where FP32 pays 1. The wide FP16 path
(`:2788-2797`) has the same per-tile drain.

**EXPECTED AFFECTED SHAPES**
- large: wide FP16 rows (1x16384 FP16, 8x8192-equivalent half paths).
- medium: FP16/BF16 multi-tile rows in `Process()` and the full-row
  pipelined paths.
- small: single-tile rows unchanged (one store either way).

**FILES/FUNCTIONS TO TOUCH**
`Init` (sizing `outputBuf_` to 2 slots where it is currently 1, wide
FP16 branch only if the narrow one is left alone), `Process()` FP16/
BF16 second pass (`:475-490`), `ProcessNarrowMidOverlap` BF16 arm
(`:606-610`), `ProcessWideFp16CachedRows` (`:2788-2797`), BF16 full-row
(`:978-982`). No host / CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
Not sync removal: every event pair that exists today still exists; the
wait moves later, which is the parent's own FP32 design. No mode, no
rows/block, no dtype split (both lowp arms take the same rule), no
ownership change.

**WHY_NOT_DUPLICATE_EPILOGUE/VECTOR-MATH**
- EPILOGUE arithmetic: no change to `Muls/Mul/Add/Cast` counts or order.
- VECTOR-MATH-X: denominator untouched.
- ASYNC-TRIPLE-X (border): they own the store event schedule / MTE3
  ring as a mechanism. This hypothesis *reuses the parent's existing*
  V006 2-deep pattern and extends it to two more arms — Main must decide
  whether that is a collision or a legitimate transplant of an in-kernel
  precedent. Flag before approval.

**CORRECTNESS RISK**
Medium. Store-source liveness is the hazard: the 2-deep convert staging
must not be overwritten while its store is in flight, and the BF16 arm's
current store source is `xBuf_` — the next row's input staging — which
is exactly what the deferred drain currently protects
(`:616` waits before releasing `inputRelease`). Reusing `xBuf_` with a
deferred drain is unsafe; the dedicated 2-deep staging is the fix, but
it consumes UB (2 tiles of T = 16 KiB at FP16). The budget headroom must
be checked on the wide-FP16 Init path.

**MEASUREMENT PLAN**
Protocol as above. Probe shapes: FP16 1x16384 and 8x8192-equivalent
(wide), FP16 2x4096 (mid), BF16 1x32768 (alias hazard check via
correctness first). Falsifier: if FP16 mid/wide deltas stay inside the
noise floor, the per-tile drain was not on the critical path — stop.

---

## STORE-H4 — CONVERT-STAGING-DECOUPLE: stop aliasing the BF16 store source onto the input staging buffer

**MECHANISM**
In `Process()`'s BF16 arm the store source is `outputLocal =
cacheParams ? gammaBuf_ : xBuf_` (`:383`) — i.e. the converted output
tile is written over the input staging (`xBuf_`) or the parameter cache.
That alias forces the strict store-drain-before-next-load ordering
(`:487-488` `SyncVToMTE3` before `SyncVToMTE2`) and prevents any overlap
between the output drain and the next row's first input DMA. Give BF16 a
dedicated convert staging slot (one `kTileElems` tile of `T`, the same
role `outputBuf_` already plays for FP16 at `:88`) so the store source
and the input staging have disjoint lifetimes.

**BOTTLENECK EVIDENCE**
`:383` (alias choice), `:483-490` (per-tile convert+store from the
alias), `:487-488` (forced V→MTE2 after the store drain). FP16 already
has the dedicated buffer (`:88` `InitBuffer(outputBuf_, tileElems *
sizeof(half))`); BF16 does not, and pays for it with a serialized
store→load handoff on every tile of every row.

**EXPECTED AFFECTED SHAPES**
- large: BF16 multi-tile rows (1x32768 BF16, 8x8192 BF16).
- medium: BF16 rows 4096 < D ≤ 8192 and BF16 mid-overlap rows.
- small: single-tile BF16 rows — one tile, alias cost is a single
  handoff; expect ≈0.

**FILES/FUNCTIONS TO TOUCH**
`Init` BF16 wide and narrow branches (add `outputBuf_` allocation for
BF16 where FP16 has it), `Process()` BF16 arm (`:383`, `:483-490`),
`ProcessNarrowMidOverlap` BF16 arm (`:606-610`). No host / CMake /
runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
One buffer lifetime reallocation on one dtype arm; no dtype *arithmetic*
split (the BF16 math chain is unchanged), no mode, no rows/block, no
multi-row DMA, no ownership change. UB cost is one tile of `T` — 8 KiB
at BF16 — which the non-wide Init paths have headroom for; the wide
branch must be checked against `kWideFullYBudgetBytes`.

**WHY_NOT_DUPLICATE_EPILOGUE/VECTOR-MATH**
- EPILOGUE arithmetic hypotheses: this changes where the converted tile
  lands, not the `FromFloat`-then-affine order.
- VECTOR-MATH-X: denominator untouched.
- UB-LIVENESS-X (border): buffer lifetime is their declared axis. This
  is one targeted lifetime decoupling for the store source, not a
  liveness redesign — flag before approval.
- STORE-H3 note: H3 and H4 overlap on the BF16 arm. H4 is the
  *prerequisite* subset of H3 for BF16; pick one, or run H4 first.

**CORRECTNESS RISK**
Low. Disjoint buffers cannot introduce a new race; the old alias can
only be removed, not widened. The risk is budget: if the wide BF16 Init
has no room for the extra tile, the change must be scoped to non-wide
arms and documented as such.

**MEASUREMENT PLAN**
Protocol as above. Probe shapes: BF16 1x32768 and 8x8192 (wide), BF16
2x4096 (mid), FP16 2x4096 (control — must be unchanged, FP16 already
decoupled). Falsifier: BF16 deltas ≈0 on multi-tile → the alias was not
binding; close the line.

---

## Recommendation — exactly one

**Recommend STORE-H1 (ALIGNED-BULK-STORE).**

Reasons, in order:

1. **Breadth without surgery.** One helper (`:3377-3383`) feeds every
   store site; the fix reaches large and medium shapes on all three
   dtypes without touching any path's control flow. The other three
   hypotheses each need per-path edits and per-path hazard review.
2. **Cleanest falsification story.** If pad-vs-bulk matters, the large
   multi-tile shapes move together and single-tile stays flat; if not,
   the store primitive is not the bottleneck and the whole store line
   can be deprioritized with one revision's worth of evidence.
3. **Lowest correctness surface.** An alignment predicate plus an
   unchanged tail path; no buffer lifetimes, no event schedule, no
   descriptor-count semantics. It also composes under STORE-H2 later —
   coalesced runs get the same bulk path for free.
4. STORE-H2 is the best *second* step (bigger ceiling on batched and
   wide shapes) but its two instances deserve separate measurement;
   STORE-H3 carries an ASYNC-TRIPLE-X border and a UB budget question;
   STORE-H4 is a proper subset of H3's BF16 arm.

Sequencing suggestion after H1 disposes: H2 (coalescing) → H4/H3 merged
(lowp drain) if FP16/BF16 mid shapes remain weak.

No implementation in this tree. Awaiting VECTOR-MATH-X V002 disposition
and Main approval before any candidate exists.
