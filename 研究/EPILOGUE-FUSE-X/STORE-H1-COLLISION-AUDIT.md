# STORE-H1 COLLISION AUDIT

Date: 2026-09-27 (C2C overnight, research + audit only)
Route: EPILOGUE-FUSE-X
Subject: STORE-H1 ALIGNED-BULK-STORE from `TRACK-B-HYPOTHESES-STORE.md`
Parent reviewed: FROZEN R31B-V011 (`R31B-V011-LP-ROW-PIPELINE_kernel.asc`,
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3)
No implementation, no worktree, no MAIN-1 file touched.

## VERDICT

**CONCEPT_COLLISION_WITH_MAIN1**

STORE-H1 is not an output-store / epilogue-store organization mechanism.
It is a **generic alignment route**: the mechanism variable is copy-
instruction selection by 32B alignment (bulk `DataCopy` vs `DataCopyPad`
tail) inside the shared copy helpers. That identity is already reserved
by old ALIGN-TAIL-X (including the copy-direction asymmetry sub-case)
and appears as a listed candidate in the Track-B of four MAIN-1 routes
— MODE-X-R015C being the trigger named in the audit brief.

Consequence: STORE-H1 is withdrawn from the store-track recommendation
line. The next orthogonal store hypothesis is selected in the final
section.

## KEY QUESTION — answered

> Is STORE-H1 primarily output-store/epilogue-store organization, or is
> it actually wide-D/tail architecture, multi-row DMA, mode redesign, or
> generic alignment route?

It is a **generic alignment route**. Three discriminators:

1. **Mechanism identity.** The proposed change is one predicate —
   `count * sizeof(T) % 32 == 0 && (offset * sizeof(T)) % 32 == 0` —
   selecting `DataCopy` over `DataCopyPad` at the copy helper. Nothing
   in that statement is about epilogue dataflow, store scheduling, or
   output-buffer lifetime. The same predicate applied at `Load` is the
   input-side twin; applying it only at `Store` does not change its
   identity, it only flips the copy direction — and copy-direction
   asymmetry is itself inside ALIGN-TAIL-X's reserved scope
   (`ALIGN-TAIL-X/next-hypotheses.md:3`: "H3 copy-direction asymmetry
   remains in research").
2. **Not any of the other named identities.** It changes no tile width
   (not wide-D architecture), issues no strided multi-row descriptor
   (not multi-row DMA), alters no row→block mapping (not mode redesign).
   The disqualifying identity in the question — generic alignment — is
   the one it has.
3. **The program already treats aligned DataCopy as contested.**
   `main1-route-selection.md:37` (R31A row): "跨路线的 Rsqrt、FMA、
   对齐 DataCopy 后续想法需要和 DTYPE/MIX 去重" — aligned DataCopy is
   named as a cross-route concept needing de-duplication.

## Evidence quotes — frozen R31B-V011 copy helpers

The shared helpers (frozen seed `:3367-3383`) — every load and every
store funnels through them, unconditionally on the pad path:

```
:3367  __aicore__ inline void Load(const AscendC::LocalTensor<T>& dst,
:3368                              const AscendC::GlobalTensor<T>& src,
:3369                              uint64_t offset, int32_t count)
:3370  {
:3371      const AscendC::DataCopyExtParams copyParams(
:3372          1, static_cast<uint32_t>(count * static_cast<int32_t>(sizeof(T))), 0, 0, 0);
:3373      const AscendC::DataCopyPadExtParams<T> padParams;
:3374      AscendC::DataCopyPad(dst, src[offset], copyParams, padParams);
:3375  }

:3377  __aicore__ inline void Store(const AscendC::GlobalTensor<T>& dst, uint64_t offset,
:3378                               const AscendC::LocalTensor<T>& src, int32_t count)
:3379  {
:3380      const AscendC::DataCopyExtParams copyParams(
:3381          1, static_cast<uint32_t>(count * static_cast<int32_t>(sizeof(T))), 0, 0, 0);
:3382      AscendC::DataCopyPad(dst[offset], src, copyParams);
:3383  }
```

Note the symmetry: `:3372` and `:3381` build the *same* ext-params
shape. An alignment-selected bulk path belongs to both or to neither —
scoping it to `Store` is the copy-direction asymmetry sub-case, not a
different mechanism.

Output store call sites (22 total). Non-wide:
`:468 :480 :489` (`Process` second pass), `:587 :597 :610`
(`ProcessNarrowMidOverlap`), `:731 :853` (small lowp batched),
`:982 :1100 :1218` (full-row pipelined), `:1452 :1553 :1659 :1773
:1800` (small FP32 batched variants). Wide-path sites:
`:1922` (`ProcessWideFp32CachedRows`), `:2224`
(`ProcessWideFp32FullCacheRows`), `:2374` (`ProcessWideFp32PanelResident`),
`:2516` (`ProcessWideFp32Batched`), `:2796` (`ProcessWideFp16CachedRows`),
`:2889` (`ProcessWideBf16CachedRows`), `:3041`
(`ProcessWideFp16BatchedOutputPipelined`), `:3341`
(`ProcessWideLowPrecision`).

## Wide-path question — answered

> Does the Store helper change touch wide-path store sites in a way that
> IS wide redesign?

**It touches them; it is not a wide redesign.** The helper is shared, so
a helper-level predicate silently changes store instruction selection at
all eight wide-path sites listed above. But no wide-path structure
changes: no tile width (`kWideFp32CachedTileElems` etc. untouched), no
wide row/batch geometry, no panel layout, no mode. What the blast radius
proves is the opposite of the claim: the mechanism is cross-path copy
geometry, which is why it is not filed as epilogue-store
organization. WIDE-X-FRESH4's declared mechanism is a "wide tile-width
方案" (`main1-route-selection.md:61`) — STORE-H1 does not touch that
axis — but the wide-path blast radius means any successor that edits
wide store loops needs WIDE-X-FRESH4 clearance (see STORE-H2 note).

## Comparison table

| Route | Their declared mechanism | Overlap with STORE-H1 | Class |
|---|---|---|---|
| MAIN-1 WIDE-X-FRESH4 | wide tile-width scheme; Track-B notes aligned-copy is "邻接 R31/DTYPE/MIX" (`main1-route-selection.md:61`) | helper change reaches 8 wide store sites (`:1922`–`:3341`) but changes no wide structure | blast-radius adjacency, not concept ownership |
| MAIN-1 MODE-X-R015C | r4 = one row one block, segment/copy path, 64 KiB staging; Track-B ≥7 items incl. "**aligned DataCopy**" (`:75`); note: "aligned copy 与 stride descriptor 候选需分别区分" (`:73`) | same concept on their candidate list | **CONCEPT (trigger)** |
| MAIN-1 MIX-A | V007 = one `SyncVToMTE2` removal; Track-B priority incl. "**对齐 narrow-mid DataCopy**" (`:51`) | same concept, narrow-mid slice | CONCEPT |
| MAIN-1 DTYPE-SPECIAL-X | FP32 copy elision / dtype arith; Track-B incl. "**完整输出 tile 的 direct copy**" (`:82` area) | STORE-H1's core case is exactly full-tile direct copy | CONCEPT |
| MAIN-1 R31A (V021 line) | wide FP32 cached-row output-wait move; Track-B incl. "**对齐 tile 的 DataCopy**" (`:39`) | same concept, tiled slice | CONCEPT (reinforcing) |
| VECTOR-MATH (current) | denominator/`invRms` production chain | none — math path, untouched by copy selection | clear |
| old ALIGN-TAIL-X | V001 single hypothesis verbatim: "explicit tail specialization sends aligned bulk to direct DataCopy and non-aligned remainder to a minimal tail path, cutting DataCopyPad/tail overhead" (`ROUTE-BRIEF.md:11`); reserved scope: "ALIGN keeps DataCopy / DataCopyPad / aligned bulk / tail / copy-direction asymmetry only" (`next-hypotheses.md:3`); split fires at "per-tile (4096-elem) **Load/Store**" (`next-hypotheses.md:10`) | **identity match**, both directions; Store-only = their H3 sub-case | CONCEPT (owner of record) |
| old EPILOGUE-FUSE-X | post-`invRms` epilogue arithmetic (SCALE-FOLD / CONVERT-ONCE / VMLA); `BOTTLENECK-NOTE.md` closes the V-pass line | none — those are V-pipe chain revisions, not copy geometry | clear |
| idea-pool R009/R010 | R009 "DataCopyPad 尾块" marked `covered` (I001 V004 proven, `idea-pool-29-routes.md:21`); R010 MANUAL TAIL | tail-block handling already has a proven historical instance | historical precedent, reinforces |

Trigger rationale: the audit brief marks collision if implementation
materially overlaps MAIN-1 WIDE **or MODE**. MODE-X-R015C's Track-B
lists "aligned DataCopy" as a planned candidate with its own OFAT tests
— implementing STORE-H1 now consumes that concept under a different
route name without their lane's evidence. MIX-A, DTYPE-SPECIAL-X and
R31A carry the same item in their own lists, and the program note
already calls for cross-route de-dup on 对齐 DataCopy. Material overlap
is established.

## Disposition

- STORE-H1: **withdrawn** from recommendation. No
  STORE-EPILOGUE-X V001 declaration draft is issued (that draft is
  conditional on a PASS verdict).
- The store-track document's ranking is amended by this audit: H1 is
  CONCEPT-busy; the recommendation slot passes to the next orthogonal
  hypothesis.

## Successor selection — STORE-H2 instance (b) only

**STORE-H2-WIDE-ROW-WRITEBACK**: where a finished output row already
occupies one contiguous UB run equal to its GM span (the resident y rows
of the wide full-y paths), issue one write-back for the whole row
instead of `tileCount` per-tile stores. Single-row, single run, no
stride descriptor, no multi-row geometry, no alignment routing — the
parent's pad instruction form is retained; only the call count changes.

Why this slice and not STORE-H2 as originally written:

- Instance (a) — batched multi-row one-descriptor store — is dropped:
  it approaches R015 true multi-row DMA (excluded axis, `idea-pool-29-
  routes.md:21`-adjacent R015 row: "多行合并 DMA", MID-X primary) and
  MODE-X-R015C's "segment copy descriptor 批处理" (`main1-route-selection.md:75`).
- Instance (b) is one row's already-resident contiguous run. Its
  variables are write-back count and descriptor count, not copy
  geometry, not row/block mapping, not store-event order.
- STORE-H3 is not the successor: deferred MTE3 drain collides with
  MIX-A's Track-B "去掉单行退出前无消费者的 MTE3-to-V wait"
  (`main1-route-selection.md:51`) — the same sync edge — plus the
  ASYNC-TRIPLE-X store-schedule border.
- STORE-H4 remains the fallback if Main judges instance (b)'s wide-path
  touch material against WIDE-X-FRESH4; its only border is UB-LIVENESS
  (buffer lifetime), not MAIN-1.

### Successor declaration draft (STORE-EPILOGUE-X V001, NOT implemented, pending current route record)

```
ROUTE=STORE-EPILOGUE-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
SINGLE_HYPOTHESIS=Wide-row write-back coalescing: resident y row is one
  contiguous UB run; store it in one call instead of tileCount per-tile
  calls (ProcessWideFp32FullCacheRows output pass, frozen :2208-2235
  loop). Call count only; copy primitive, event order, tile width,
  batch geometry, dtype arms untouched.
INSTANCE_SCOPE=instance (b) only; STORE-H2 instance (a) dropped
  (multi-row adjacency).
FILES_TO_TOUCH=ProcessWideFp32FullCacheRows output pass only (frozen
  :2224 site and its loop bounds); no helper, no Init, no host.
EXPECTED_SHAPES=large wide rows (8x8192, 1x32768 classes); small ≈ 0.
CORRECTNESS_RISK=low; one row's run length is rowWidth, already
  contiguous in UB and GM.
MEASUREMENT_PLAN=same-binary both sides first; interleaved P/C pairs on
  8x8192 + 1x32768; 33x100 control; warmup=45, samples=41, 6 pairs.
CLEARANCE_REQUIRED=WIDE-X-FRESH4 lane boundary (wide-path store-loop
  touch, mechanism is not tile-width) — Main confirmation before coding.
WHY_NOT_DUPLICATE=ALIGN-TAIL owns copy geometry (primitive selection);
  this changes neither DataCopy/Pad choice nor tail policy. MODE owns
  row→block mapping and segment-copy descriptors; this issues no
  segment, no stride, no multi-row block. EPILOGUE arithmetic and
  VECTOR-MATH denominator untouched.
```

Next: VECTOR-MATH V002 disposition, current route record, WIDE-X-FRESH4
clearance. Not implemented.
