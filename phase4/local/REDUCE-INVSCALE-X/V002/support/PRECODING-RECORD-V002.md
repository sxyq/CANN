# REDUCE-INVSCALE-X V002 — pre-coding record (correctness-only)

Recorded BEFORE any coding, per route protocol. Main assignment 2026-09-23T21:00:21Z.

```text
ROUTE             : REDUCE-INVSCALE-X
REVISION          : V002
CONTEXT_CLASS     : HISTORICAL_DERIVED
REVISION_KIND     : CORRECTNESS_REPAIR_ONLY (no performance variable added)
DIRECT_PARENT     : REDUCE-INVSCALE-X-V001
PARENT_SOURCE     : phase4/local/REDUCE-INVSCALE-X/V001/submission.asc
PARENT_SOURCE_SHA : f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023
PARENT_SCORE      : LOCAL_PARENT (V001 was handed off LOCAL_REJECTED: correctness
                    FAIL for D > 6144, no formal score, not an online candidate)
SINGLE_HYPOTHESIS : Repair the R006 partial collector destination alignment only.
                    Give every tile partial its own 32-byte-aligned collector slot
                    so ReduceTileToPartial's ReduceSum destination is always
                    32-byte aligned, and collapse the strided slots on Vector.
                    Do NOT change the R019 invscale normalization, tile size,
                    pipeline, row scheduling, or parameter policy.
CONTEXT           : HISTORICAL_DERIVED (parent chain FULL-R006-V001 -> V001 -> V002)
```

## Scope boundary

Inherited unchanged from V001 (which inherited from FULL-R006-V001):

- R019 normalization: `return 1.0f / rmsValue` in `ComputeRowRms`, single
  `Muls(yFp32, yFp32, invRms, valid)` in `WriteNormalizedRow`.
- `kReduceTileElems = 6144`, `kPartialCapacity = 64`
- `ReduceTileToPartial` load / form-u / square / ReduceSum structure
- collector flush logic and the one row-level collapse per collector window
- `SyncMte2ToVector` / `SyncVectorToMte2` / `SyncVectorToMte3` / `SyncMte3ToVector`
- round-robin `row += coreNum` ownership, non-aligned rows forced to 1 core
- gamma/bias reloaded tile-by-tile (no parameter residency)
- host launch, dtype dispatch, validation

Changed by this revision only:

- collector slot layout: packed floats `partialBase[i]` → strided
  `partialBase[i * 8]`, one 32-byte slot per partial
- `partials_` size `64 * 4 = 256 B` → `64 * 8 * 4 = 2048 B`
- `CollapsePartials`: one contiguous `ReduceSum(dst, partialBase, tmp, count)`
  → a Vector-only walk of the strided slots with 1-element `Adds`/`Add`
  (only lane 0 of each slot is valid, which is the documented ReduceSum
  destination contract)

## Why this repairs the failure

`ReduceSum(dst, ...)` writes a single float but its destination is a vector
address and must be 32-byte aligned. The parent packed partials at float
indices 0,1,2,... so slot 1 sits at byte offset 4, slot 2 at byte offset 8, and
so on. Slot 0 is aligned; every later slot is not. Observed behaviour matches
exactly: D = kReduceTileElems = 6144 (one tile, one partial, `count == 1`
`Adds` path) passes, while D > 6144 (two or more partials) fails grossly and
identically in parent and V001.

Striding by 8 floats puts slot *i* at byte offset `32 * i`, which is 32-byte
aligned for every `i`. The collapse then cannot be a single contiguous
`ReduceSum` over packed floats, so it walks the slots with 1-element vector
`Add`s — still fully on Vector, still one row-level collapse per collector
window, still no scalar participation, which preserves the R006 claim that the
row partial is handled "without any scalar read".

UB budget after the change (worst case `T = float`):

```text
inputX_ 24 KiB + inputR_ 24 KiB + output_ 24 KiB
fp32A_ 24 KiB + fp32B_ 24 KiB + value_ 24 KiB + reduceWork_ 24 KiB
partials_ 2048 B + scalars_ 64 B
= 174144 B = 170.06 KiB  <  184 KiB TOTAL_VEC_LOCAL_SIZE
```

## Rejected alternatives (recorded so the choice is auditable)

1. Aligned scratch + scalar stash of each partial into a packed collector.
   Would work, but adds one scalar read and one scalar write per tile plus
   V_S/S_V events, contradicting the R006 "no scalar read per tile" property.
2. Zero-fill the collector and collapse with `ReduceSum(..., count * 8)`.
   Depends on ReduceSum leaving lanes 1-7 of the destination untouched, which
   is not part of the documented contract (only "the first element is valid").
   Rejected as unsafe.
3. Per-tile scalar accumulator `Add` into one aligned slot. That is B0's
   serialized per-tile accumulator update, the exact structure R006 exists to
   replace. Rejected: it would change the reduction architecture under test.

## Evidence retention

V001 evidence is untouched at `phase4/local/REDUCE-INVSCALE-X/V001/`. This
revision's record goes to `phase4/local/REDUCE-INVSCALE-X/V002/` with
`diff.patch` against V001.

---

## Outcome addendum (recorded after the runs)

The assigned alignment hypothesis was implemented first and **did not repair
correctness**. Three builds were measured on the same 16-shape matrix:

| build | content | result on D > 6144 |
|---|---|---|
| alignment only | V001 + 32-byte collector slots + strided collapse | **FAIL**, identical signature (max_abs 5.4174 at D=8192, same as parent) |
| alignment + sync | above plus `SyncVectorToMte2()` after gamma/bias use | **PASS** all 16 shapes |
| sync only | V001 + `SyncVectorToMte2()` only, packed collector unchanged | **PASS** all 16 shapes |

So packed-float `ReduceSum` destinations at byte offsets 4, 8, 12, ... are
*not* the defect: the sync-only build keeps them and passes. The actual defect
is the missing vector-to-MTE2 ordering between the gamma/bias read and the next
tile's x/residual reload. The collector-stride change is therefore dropped from
V002: it was not needed, and it would have altered the reduction collapse path
(a performance-relevant structure) inside a correctness-only revision.

**V002 as delivered = V001 + one change: `SyncVectorToMte2()` after the
gamma/bias consumption in `WriteNormalizedRow`.** R019 invscale, tile size,
pipeline, row scheduling, parameter policy and the packed partial collector are
byte-identical to V001.

Deviation from the literal assignment is flagged for Main in the handoff.
