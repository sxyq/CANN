# STORE-H2B SPEC — single-row resident writeback merge

Date: 2026-09-27 (C2C overnight; research/spec only — NOT implemented)
Route: EPILOGUE-FUSE-X
Status: approval-ready specification for Main review.
Parent of record: R31B-V011 (`R31B-V011-LP-ROW-PIPELINE_kernel.asc`)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
Predecessor audit: `STORE-H1-COLLISION-AUDIT.md` (STORE-H1 =
CONCEPT_COLLISION_WITH_MAIN1; this spec is the selected successor).

HYPOTHESIS_ID=STORE-H2B

## MECHANISM

For a **single row**, the finished output of the row is one contiguous
UB run mapped to one contiguous GM span. Today the parent writes that
run back in per-tile pieces (`kTileElems=4096` or
`kWideFp32CachedTileElems=7680` slices). STORE-H2B merges the
contiguous output-tile store run of one row into **one writeback**
(DataCopyPad call) where legal.

Legal predicate (all must hold for the run):

1. run stays inside one row — never crosses a row boundary
   (single-row only; inter-row merge is R015 territory, excluded);
2. run is contiguous in UB (the row's finished values sit at
   consecutive element offsets of one buffer) and contiguous in GM
   (`row * rowWidth + col`, flattened row-major);
3. run start is 32B-aligned in UB (DataCopyPad requirement; ALIGN-TAIL
   evidence `next-hypotheses.md:26`); unaligned-start rows keep the
   parent's per-tile stores;
4. run length fits one pad block; if `rowWidth * sizeof(T)` exceeds the
   pad block-length limit, the row is split into maximal legal chunks
   (still far fewer descriptors than per-tile; if the cap forces chunks
   as small as tiles, that site gains nothing — see STOP_CONDITION).

What is **not** changed:

- copy primitive stays `DataCopyPad` (the `Store` helper `:3377-3383`
  is untouched — primitive selection is ALIGN-TAIL-X's reserved axis);
- per-store event discipline at each site stays verbatim (the ring at
  `:1218`'s site and the wide 2-deep store ring keep their flags); only
  the number of `Store()` calls inside one row changes;
- tile compute order, epilogue arithmetic, reduction, dtype arms,
  row→block mapping, batch geometry, load paths.

## BOTTLENECK_EVIDENCE (frozen R31B-V011 store sites)

Per-tile writeback loops over a single row's contiguous resident run:

| site (frozen line) | function | run | tiles today | stores today |
|---|---|---|---|---|
| `:2224` | `ProcessWideFp32FullCacheRows` output pass | `valueLocal[batchRow*rowWidth + col]`, full y row resident in `valueFp32Buf_` | `rowWidth/4096` (8 at D=32768) | per (row, tile) |
| `:2796` | `ProcessWideFp16CachedRows` | `outputLocal[col]` | `rowWidth/7680` | per tile, full `SyncVToMTE3; Store; SyncMTE3ToV` each |
| `:2889` | `ProcessWideBf16CachedRows` | `outputLocal[col]` | same | same |
| `:1922` | `ProcessWideFp32CachedRows` | `valueLocal[col]` | `rowWidth/7680` | per tile |
| `:1218` | `ProcessFp32FullRowOutputPipelined` (D=8192) | `valueLocal[col]`, both tiles in `valueFp32Buf_` (`:1151` comment: "holds both output tiles") | 2 | per tile, 2-deep MTE3 ring |
| `:468/:480/:489` | `Process` generic second pass, `cacheRow` arm (`valueTile = valueFp32[col]`, `:390`) | row's y cached at row offset | `ceil(D/4096)`, 2 for D in (4096, 8192] | per tile |

The parent already proves single large writebacks are legal where the
run is contiguous across rows: `:1452` and `:1659`
(`ProcessSmallFp32ContiguousBatched` / `ProcessSmallFp32Batched`)
issue `Store(outputGm_, batchBegin * rowWidth, valueLocal, batchRows *
rowWidth)` — one call of up to 4 rows × 2048 elements = 32 KiB FP32.
STORE-H2B reuses that precedent's call form at **one-row** granularity.

Per-tile cost being removed: one `Store()` descriptor plus the site's
V→MTE3 handshake per tile (`:2796`-style sites pay a full
`SyncVToMTE3; Store; SyncMTE3ToV` round trip per tile — 2 forced syncs
per tile where one per row would do).

## TARGET_SHAPES

| band | D | tileCount/row | merge | expected |
|---|---|---|---|---|
| small | ≤ 4096 (narrow-mid `:237`, single tile) | 1 | none possible | ≈ 0 (control) |
| medium | (4096, 8192] — generic `cacheRow` arm and D=8192 full-row pipelined | 2 | 2 → 1 writeback | modest; 50% fewer store descriptors |
| large | > 8192 wide rows (D=16384, 32768) | 4–8 | 4–8 → 1 (or 1 chunked run) | primary; 75–87.5% fewer descriptors + per-tile syncs removed on `:2796/:2889` sites |

Probe shapes: 8x8192 and 2x6144 (medium), 1x32768 and 8x16384-class
(large), 2x4096 and 33x100 (small controls, expect ≈ 0).

## EXPECTED_EFFECT

- Store descriptor count per row: `tileCount` → 1 (or
  `ceil(rowBytes / blockLenCap)` when chunked).
- On `:2796/:2889/:1922/:468-489` sites: per-tile `SyncVToMTE3` /
  `SyncMTE3ToV` pairs drop to one per row — the V-pipe is no longer
  interrupted at tile boundaries by MTE3 handshakes.
- Kernel-level: store-side issue cost is a component of short-kernel
  time (BOTTLENECK-NOTE.md: 5–12 µs kernels, ±5% noise). Honest
  expectation: direction-consistent single-digit percent on multi-tile
  rows, ≈ 0 on single-tile rows. Not a reduction-topology or epilogue-
  arithmetic claim.

## FILES/FUNCTIONS

`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only. Writeback call sites listed
above; V001 proposes touching the row-run sites with the simplest event
shape first:

1. `ProcessWideFp16CachedRows` `:2796` and `ProcessWideBf16CachedRows`
   `:2889` (per-tile sync, no ring — cleanest),
2. `ProcessWideFp32FullCacheRows` `:2224` (keep the existing 2-deep
   store ring flags; one store entry per row per ring slot),
3. generic `Process` `cacheRow` arm `:468/:480/:489` and
   `ProcessWideFp32CachedRows` `:1922`,
4. `ProcessFp32FullRowOutputPipelined` `:1218` last — its 2-deep ring
   exists to overlap tile-0 store with tile-1 compute; merging the two
   tiles into one writeback removes that overlap. Include only if 1–3
   show signal; otherwise leave the ring site unchanged.

No helper edit. No Init edit (runs use existing buffers). No host /
CMake / runner change.

## WHY_ORTHOGONAL_TO_MAIN1

| route | their axis | why STORE-H2B is not it |
|---|---|---|
| WIDE-X-FRESH4 | wide tile-width scheme (`main1-route-selection.md:61`); wide tiling/architecture | no tile width, no wide tiling, no wide path structure changes; wide *callers* benefit but the variable is writeback run length inside one row. Wide-path blast radius still warrants their lane's note — Main to acknowledge |
| MODE-X-R015C | rows-per-block / row→block mapping, segment/copy path, 64 KiB staging, segment descriptor batching (`:73-75`) | no row→block mapping change, no segment path, no staging resize, no descriptor *batching across rows*; one row's run is already one block geometrically |
| MIX-A | sync/wait placement (V007 removes one `SyncVToMTE2`) | event placement per store is kept verbatim; fewer stores is a count change, not a wait-move. Their Track-B item "对齐 narrow-mid DataCopy" is copy geometry, not writeback merging |
| DTYPE-SPECIAL-X | dtype arithmetic/copy elision, "完整输出 tile 的 direct copy" (copy *primitive* on full tiles) | STORE-H2B selects no copy primitive and elides no compute copy; Pad stays Pad. Their "direct copy" item is the STORE-H1 axis that died in the collision audit |
| ALIGN-TAIL-X | copy geometry: DataCopy/Pad selection, aligned bulk vs tail, copy-direction asymmetry (reserved scope `next-hypotheses.md:3`) | STORE-H2B never selects between DataCopy and DataCopyPad and never splits bulk/tail; it changes how many times the unchanged `Store` helper is called per row. Tail handling: the merged run includes the ragged tail inside the same pad block (`blockLen` may be unaligned — their own evidence `:26`), no tail path added |

## WHY_NOT_DUPLICATE_CURRENT_MAIN2

- **VECTOR-MATH-X** (VM-H2 denominator): math path (`invRms`
  production). STORE-H2B touches only the writeback issue after the
  affine; no denominator, no `invRms` value change.
- **EPILOGUE-FUSE-X V001/V002/V003** (SCALE-FOLD, VMLA-cached,
  VMLA-uncached): post-`invRms` V-pipe arithmetic reshuffling, all
  closed by `BOTTLENECK-NOTE.md` (pass-count not binding). STORE-H2B
  does not alter a single V-pipe op — `Muls/Mul/Add` counts and order
  per tile are byte-identical; only the MTE3 issue granularity changes.
  Different pipeline stage (writeback vs compute).
- **STORE-H2(a)** — see NON_DUPLICATION below.
- **ASYNC-TRIPLE-X** — see NON_DUPLICATION below.

## NON_DUPLICATION CHECK (requested)

**vs STORE-H2(a) (multi-row descriptor merge):** (a) merges *across
rows* — `batchRows` row-stores into one span store
(`:1553`-style loops → `:1452`-style call). That is inter-row DMA
geometry and collides with R015 multi-row DMA (idea-pool R015
"多行合并 DMA", MID-X primary) and MODE-X-R015C's segment descriptor
batching. STORE-H2B merges *across tiles inside one row* and its legal
predicate forbids crossing a row boundary (MECHANISM item 1). Different
variable: run direction (intra-row vs inter-row). (a) stays dropped.

**vs ASYNC-TRIPLE-X (store ring / MTE3 event schedule):** their axis is
the event ring — depth, which flags, how many stores outstanding, wait
placement (e.g. the 2-deep ring at `:1218`'s site and the
`storeReady0/1` ring at `:2224`'s site are their mechanism class).
STORE-H2B keeps each site's ring and flag discipline **verbatim** —
same events, same waits, same outstanding policy; only the number of
`Store()` calls inside one row changes. Border rule for V001: if a
site's ring cannot express a merged writeback without changing flag
count, depth, or wait placement, that site is excluded (this is why
`:1218` is last in FILES/FUNCTIONS and may be dropped). No sync removal
claim is part of this hypothesis.

## CORRECTNESS_RISK

Low–medium, three concrete hazards:

1. **block-length cap.** One pad block of `rowWidth * sizeof(T)` bytes
   (128 KiB at D=32768 FP32) may exceed a hardware descriptor limit.
   Mitigation: implement the run as maximal legal chunks; verify the cap
   on target hardware before timing; if the cap collapses the run to
   per-tile size, the site yields nothing (STOP_CONDITION item 3).
   Parent precedent covers 32 KiB single blocks (`:1452`).
2. **UB start alignment.** Rows whose UB run start is not 32B-aligned
   (e.g. odd-width rows like D=257 FP16 → row stride 514 B) must keep
   the parent path. The predicate checks the run's UB address, not just
   `count`.
3. **writeback liveness.** The merged store must drain before the next
   row's pass-1 overwrites `valueFp32Buf_`. The parent's existing
   MTE3_V release discipline at each site already enforces this before
   buffer reuse; the merged call must be issued under the same release
   condition. No new lifetime is introduced.

No arithmetic change: outputs are bit-identical (same values, same
pad semantics — `DataCopyPad` of a full run equals N pad calls of the
run's slices for aligned interior; the ragged tail's pad bytes are
unchanged in value).

## LOCAL_MEASUREMENT_PLAN (medium band critical)

Protocol: warmup=45, samples=41, same-binary blocks=2 (MAD/median ≤
0.15 per side), 6 interleaved P/C pairs (odd P→C, even C→P), device-
event primary, lease a single d4 window. Parent = frozen R31B-V011;
candidate = merged-writeback revision only.

Shape set (band-tagged):

| band | shape | role |
|---|---|---|
| medium | 8x8192 FP32 | primary — 2 tiles/row, full-row pipelined or generic arm |
| medium | 2x6144 FP32 | primary — generic `cacheRow` arm, 2 tiles |
| large | 1x32768 FP32 | primary — 8 tiles/row, `:2224` site |
| large | 1x16384 FP16 | primary — `:2796` site, per-tile sync removal visible |
| small | 2x4096 FP32 | control — 1 tile, expect ≈ 0 |
| small | 33x100 FP32 | control — 1 tile, expect ≈ 0 |

Medium band is critical because it is where (i) the local runner is
most stable on this device, (ii) tileCount=2 is the smallest non-
trivial merge so any effect must show cleanly, and (iii) the weak
Official mid cases live. Large band carries the expected ceiling;
small band falsifies noise-only explanations.

Verdict rules (unchanged project discipline): regression on any clean
primary shape → LOCAL_REJECTED; primary neutral with only one band
moving → NEEDS_ONE_MORE_LOCAL; wins in ≥ 2 bands without clean
regression → LOCAL_ACCEPTED + ONLINE_WORTHY consideration.

## STOP_CONDITION

Stop the revision (do not iterate a second version) if any of:

1. any dtype/shape fails NPU correctness vs parent (bit-identical
   expected; any mismatch = liveness bug, fix before timing or stop);
2. after the full protocol, both primary shapes of one band show
   clean deltas inside the same-binary noise floor and the other band
   shows no direction-consistent movement — the writeback count is not
   binding at this kernel size;
3. the pad block-length cap forces chunking as fine as the original
   per-tile calls on the large band — nothing to merge;
4. Main's disposal of VECTOR-MATH V002 or a MAIN-1 lane claims this
   axis first (re-run the collision audit before coding).

## DRAFT REVISION-DECLARATION (not yet implemented)

```
ROUTE=STORE-EPILOGUE-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS_ID=STORE-H2B
SINGLE_HYPOTHESIS=Single-row resident writeback merge: one row's
  contiguous finished output run is written back in one call instead
  of per-tile calls. Intra-row only; DataCopyPad form and per-site
  event discipline unchanged; chunked if a block-length cap requires.
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
WHY_NOT_DUPLICATE_MAIN1=WIDE: no tile-width/tiling change; MODE: no
  row-block mapping / segment / inter-row batching; MIX-A: no wait
  placement change; DTYPE: no copy-primitive or arithmetic change;
  ALIGN-TAIL: no DataCopy/Pad selection, no bulk/tail split
WHY_NOT_DUPLICATE_MAIN2=VECTOR-MATH owns denominator; EPILOGUE V001-3
  owned epilogue arithmetic (closed); STORE-H2(a) inter-row merge
  dropped (R015); ASYNC-TRIPLE-X owns store rings — rings kept verbatim
SINGLE_CHANGE_AUDIT=PENDING (one variable: writeback run length per row)
MAIN_APPROVAL=PENDING
CORRECTNESS_FIX=NONE REQUIRED (values bit-identical by construction)
SOURCE_SHA=PENDING
COMPILE_RC=PENDING
LINK_RC=PENDING
CORRECTNESS=PENDING
LOCAL_VERDICT=PENDING
MEASUREMENT=PENDING
```

Awaits: Main approval, VECTOR-MATH V002 disposition, WIDE-X-FRESH4 lane
acknowledgement (wide callers touched, no wide structure change),
block-length cap confirmation on d4 hardware. No worktree created.
