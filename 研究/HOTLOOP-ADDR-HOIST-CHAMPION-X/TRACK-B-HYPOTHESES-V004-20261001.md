# HOTLOOP-ADDR-HOIST-CHAMPION-X - Track-B Hypotheses V004

## Status and evidence boundary

```text
DATE_UTC=2026-10-01
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
TRACK=TRACK-B-ONLY
MAIN=MAIN-2
MAIN_SELECTED=NONE
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL=45.16
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CODEGEN=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
PROFILING=NOT_RUN
ONLINE=NOT_RUN
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
```

This is a new address-expression round. It does not re-submit H1 row-base
hoist, H2 rolling input cursor, or H3 row/tile counters from the prior
handoff. The three hypotheses below isolate three different address surfaces
in the exact V011 `ProcessWideLowPrecision` body:

1. the next-unit pass-1 prefetch offset;
2. the current-unit pass-1 column offset;
3. the pass-2 parameter and output column offset.

No compiler, assembler, NPU, server3, build, correctness, timing, profiling,
or Online command was run. The only executable check in this round was an
ephemeral host integer model whose output was not written to the repository.
The model proves address algebra for the stated cases; it is not an NPU
correctness result and does not imply a performance result.

## Exact parent facts

The frozen parent has these relevant boundaries:

- `ProcessWideLowPrecision` is source lines 3076-3365. It is reached for
  low-precision dtypes only when `rowWidth > kCacheElems`, where
  `kCacheElems=8192` (lines 60 and 163-168). `D=8192` is therefore not an
  ADDR wide-LP target; `D=12288/16384/32768` are path probes.
- FP32 uses `ProcessWideFp32`, not this low-precision function. The target
  dtype set for this round is FP16/BF16.
- The pass-1 stream decodes `u` into `batchRow` and `tile` (3125-3128),
  computes `col` and `rowOffset` (3128-3130), then issues the current
  x/residual loads (3140-3143). The next-unit prefetch independently derives
  `nRow`, `nTile`, `nCol`, and `nRowOffset` (3146-3174).
- Pass 1 writes retained y/reduction slots using `batchRow`, `tile`, and
  `col` (3176-3203). Those logical indices and reduction offsets are frozen
  by all three candidates.
- Pass 2 has a separate `tile` loop (3267-3352). It computes `col` and
  `nCol`, loads gamma/bias from GM, and stores each batch row at
  `(batchBegin + batchRow) * rowWidth + col` (3267-3344).
- `Load` accepts a `uint64_t` element offset and invokes `src[offset]`
  (3367-3375). No new GlobalTensor or DataCopy API is needed for this round.
- `tileWidth=wideFullYTileElems_` starts at 4096 but may be reduced by
  `ChooseWideFullYRows` for an N=1 UB fallback. A fixed 4096 assumption is
  invalid.

The Official testcase-to-shape map remains missing. The dimensions below are
kernel-path probes, not claims about Judge testcase IDs.

## Runtime probe matrix

The following values were recomputed from the exact `ChooseWideFullYRows`
helper: 176 KiB budget, max eight rows, 4096 initial tile, and 512-element
fallback decrement. `tail` is the final tile length in elements.

| dtype | D | tileWidth | batchLimit | tileCount | tail |
|---|---:|---:|---:|---:|---:|
| FP16 | 12288 | 4096 | 3 | 3 | 4096 |
| FP16 | 16384 | 4096 | 2 | 4 | 4096 |
| FP16 | 32768 | 4096 | 1 | 8 | 4096 |
| BF16 | 12288 | 4096 | 2 | 3 | 4096 |
| BF16 | 16384 | 4096 | 1 | 4 | 4096 |
| BF16 | 32768 | 2560 | 1 | 13 | 2048 |

The matrix intentionally includes the reduced-tile BF16 case. Each future
proof must also cover `batchRows < batchLimit` for a partial final batch; it
must not only test a full batch.

## ADDR-H4 - next-prefetch offset delta reuse

### Mechanism

In pass 1, keep the current `batchRow`, `tile`, `col`, `rowOffset`, `valid`,
and all current loads exactly as V011. Keep the parent `nRow`, `nTile`, `nCol`,
and `nValid` derivation for the first OFAT form. Replace only the GM offset
passed to the two next-unit loads:

```text
currentOffset = rowOffset + col
nextOffset = currentOffset +
    (same-row ? tileWidth : rowWidth - col)
```

For a same-row successor, `currentOffset + tileWidth` equals
`nRowOffset + nCol`. At the last tile of a row, `currentOffset +
rowWidth - col` equals the next row base, and the parent `nCol` remains zero.
The candidate must not replace the parent `nValid` calculation with a new
tail or dispatch rule. A compiler may remove the now-dead `nRowOffset` work;
that is the intended address-composition effect, not a new logical-index
algorithm.

### Scope, target, and possible failure

- **Bottleneck:** duplicate next-unit row-base plus column address
  composition in the 2-deep MTE2 prefetch.
- **Target:** FP16/BF16 wide-LP probes in the matrix above, with emphasis on
  multi-tile `tileCount=3/4/8` and the BF16 `tileCount=13` tail case.
- **May help:** the next load reuses the already formed current address and
  avoids an independent next-row multiply/add sequence.
- **May fail:** the compiler may already CSE the two address forms; the
  row-boundary select can become a real branch; or the extra `currentOffset`
  live value can increase scalar/register pressure. The V/S/reduction or
  MTE2/MTE3 schedule may dominate the scalar address cost.
- **Address risk:** high at the last tile. A wrong `rowWidth-col` correction
  aliases the next row while leaving vector arithmetic apparently valid.
- **Precision risk:** low. No arithmetic order, cast, or reduction input is
  changed.

### Feasibility and minimal proof

`STATIC_SOURCE_FEASIBLE=YES`: the formula uses only existing `uint64_t`
element offsets and the existing `Load` helper. `CODEGEN_GATE=UNVERIFIED`:
the first evidence must show that the emitted next-load address sequence
actually loses the independent row-base composition and does not add a costly
branch. No performance benefit is asserted before that evidence.

The minimum host/address proof is exhaustive over every pass-1 unit for each
matrix row, `batchRows=1..batchLimit`, and representative `batchBegin` values
including a nonzero origin. Assert:

```text
parent_next = (batchBegin + nRow) * rowWidth + nCol
candidate_next = currentOffset + (tile + 1 < tileCount ?
                                  tileWidth : rowWidth - col)
parent_next == candidate_next
```

The proof must separately report the final tile and the partial final batch.
It must also assert unchanged `nValid`, DMA count, and DMA byte count from the
parent expression. This is an address proof only; it does not authorize a
Revision or correctness run.

### Duplicate disposition

`NOT_DUPLICATE_WITH_H1/H2/H3`, with a shared next-unit code surface only:

- H1 changes current and prefetched row-base state; H4 changes only the next
  prefetch offset and leaves the current row address in parent form.
- H2 uses one cursor for current and next x/residual addresses; H4 has no
  persistent input cursor and leaves the current address unchanged.
- H3 changes current and next logical index decoding; H4 retains the current
  `batchRow/tile` decode and parent `nValid` derivation.

Against branch hypotheses, H4 does not move the generic terminal-row
prefetch guard or coefficient mode. Its row-boundary select is part of the
address delta; if codegen turns it into a new hot branch, the candidate is
`CODEGEN_REJECTED`, not a branch-route candidate. It does not specialize
`tileCount` and does not touch reduction.

## ADDR-H5 - pass-1 current-column cursor

### Mechanism

In the flattened pass-1 loop, keep the parent `batchRow`, `tile`, `rowOffset`,
all next-prefetch address expressions, and all reduction slot indices. Replace
only the current `col = tile * tileWidth` value with a `uint64_t colCursor`
initialized to zero at each `batchBegin`:

```text
col = colCursor
after the unit: colCursor = (tile + 1 == tileCount) ? 0
                                                     : col + tileWidth
```

The first OFAT form leaves the parent `nCol = nTile * tileWidth` untouched.
Thus H5 isolates the current-unit column multiply used by current x/residual
loads, retained-y indexing, and current valid calculation. It does not become
H2 by feeding the next prefetch from the cursor.

### Scope, target, and possible failure

- **Bottleneck:** current-unit `tile * tileWidth` column formation inside the
  flattened pass-1 stream.
- **Target:** the full matrix, especially `tileCount=8` and the reduced
  `tileCount=13` BF16 case; include `batchRows=1` and multi-row batches where
  the source allows them.
- **May help:** a stateful column value can replace one dynamic multiply per
  current unit while retaining the existing row-base expression.
- **May fail:** the compiler may already strength-reduce the multiply; the
  reset/select at each row boundary may cost more than it saves; and one live
  64-bit cursor may increase register pressure. The current column is also
  used for retained-y indexing, so an off-by-tail error can corrupt pass 2.
- **Address risk:** high at row rollover and reduced-tile tail.
- **Precision risk:** low if the address trace is exact; no vector arithmetic
  is intentionally changed.

### Feasibility and minimal proof

`STATIC_SOURCE_FEASIBLE=YES`: the source already uses a `uint64_t col` for
the same local/GM offsets, and `tileWidth` is runtime `int32_t`. The host
proof must assert, for every unit and partial batch:

```text
colCursor_at_u == tile(u) * tileWidth
rowOffset + colCursor_at_u == parent_current_offset
batchRow/tile/reduceLocal[batchRow * reduceStride + tile] unchanged
```

`CODEGEN_GATE=UNVERIFIED`: inspect whether the current-unit multiply is gone,
whether the reset is predicated rather than a new loop branch, and whether
register/spill counts stay neutral. If the emitted sequence keeps the
multiply or adds a branch with no scalar reduction, classify H5 as a no-op or
regression risk and do not create a Revision.

### Duplicate disposition

`NOT_DUPLICATE_WITH_H1/H2/H3`, but not combinable with them in one OFAT:

- H1 removes/reuses the row component; H5 only changes the column component
  and recomputes `rowOffset` in the parent form.
- H2 replaces the complete current/next input address sequence; H5 changes
  only current pass-1 `col` and deliberately leaves next prefetch address
  formation unchanged.
- H3 changes the logical `(batchRow,tile)` decoder; H5 retains both values
  for valid, retained-y, reduction, parity, and prefetch logic.

H5 is adjacent to HBH-03's valid-length code surface because `valid` consumes
`col`, but it does not hoist or change the final-tile predicate or byte count.
It is not a BRANCH hypothesis unless an implementation changes loop shape or
adds a hand-written branch body. It does not specialize tile count and does
not alter reduction.

## ADDR-H6 - pass-2 column cursor

### Mechanism

Leave pass 1 byte-for-byte at the source level. In the pass-2 `for (tile ...)`
loop, initialize a `uint64_t colCursor=0` and use it as the current `col`.
Advance it by `tileWidth` after each non-terminal tile. Keep the parent
`nCol = col + tileWidth` expression for the next gamma/bias prefetch and keep
the existing `tile` value for parity and loop predicates.

This changes only the column operand used by pass-2 gamma/bias GM loads,
retained-y/value indexing, and output stores. It does not change pass-1
addresses, pass-1 reductions, parameter DMA counts, or the per-row store
order.

### Scope, target, and possible failure

- **Bottleneck:** repeated pass-2 `tile * tileWidth` column composition and
  its reuse in parameter-load/store offsets.
- **Target:** FP16/BF16 matrix cases with `tileCount=3/4/8/13`; the 13-tile
  BF16 case is the useful dynamic-tail stress point, while 3/4/8 are controls.
- **May help:** one incremental column state can feed all current parameter
  and store offsets in the tile loop and avoid the current multiply.
- **May fail:** pass 2 may be dominated by vector apply, MTE3 store waits, or
  parameter DMA; the compiler may already lower the affine loop to increments;
  an extra live cursor can raise register pressure. The candidate may be
  structurally neutral even if the source looks simpler.
- **Address risk:** medium/high because both gamma/bias and output stores use
  the value, although the row base remains parent-derived.
- **Precision risk:** low; arithmetic and rounding order are frozen.

### Feasibility and minimal proof

`STATIC_SOURCE_FEASIBLE=YES`: the cursor has the same element-offset type as
the existing `col`, and no new API is required. The host proof must compare
for every tile and every `batchRow` in the selected batch:

```text
colCursor == tile * tileWidth
(batchBegin + batchRow) * rowWidth + colCursor
    == parent pass-2 store offset
gamma/bias current and next offsets == parent col/nCol offsets
```

`CODEGEN_GATE=UNVERIFIED`: the emitted pass-2 address sequence must show a
real multiply reduction with no new loop split, static unroll, or register
spill. No performance gain is inferred from source-level algebra.

### Duplicate disposition

`NOT_DUPLICATE_WITH_H1/H2/H3`: H1/H2/H3 are pass-1 address/index changes;
H6 leaves pass 1 unchanged and targets the independent pass-2 tile loop.
It is adjacent to HBH-03 only because both can mention the final tile; H6
preserves the parent `valid` and `nValid` expressions and does not hoist tail
length. It is not HBH-01 coefficient-mode selection or HBH-02 row-prefetch
guard. It does not change `tileCount`, loop trip count, DMA geometry, or
reduction.

## Boundary and duplicate audit

| Boundary | Frozen requirement | H4/H5/H6 status |
|---|---|---|
| BRANCH | Keep V011 dispatch, `u+1<units`, tile-loop predicates, event order, and row ownership. An address select must not become a new loop/body specialization. | H4 adds only a candidate address delta select; H5 adds only cursor rollover state; H6 adds only an affine increment. Any emitted branch/code-size cost is a codegen gate, not a claimed win. |
| TILECOUNT | Keep runtime `tileCount=(rowWidth+tileWidth-1)/tileWidth`; no fixed-trip specialization, static unroll, parity-body rewrite, or offset propagation. | All three use actual runtime `tileWidth/tileCount`; BF16 D=32768 must remain `tileCount=13`, tail=2048. Fixed-count forms are `DUPLICATE_REJECTED` under TILECOUNT-STATIC-UNROLL. |
| REDUCE | Keep `ReduceSum`, `reduceStride`, `reduceLocal[batchRow*reduceStride+tile]`, scalar handoffs, and arithmetic order. | H4/H5 preserve pass-1 reduction indices; H6 does not touch pass 1 or reduction. Any reduction-slot rewrite is `DUPLICATE_REJECTED` / REDUCE-owned. |
| DMA/event | Keep two x/residual loads per pass-1 unit, counts, byte lengths, slot parity, and all event IDs/waits. | Address operands only; H4/H5/H6 cannot change count/bytes or ping-pong ownership. |
| dtype/path | Keep exact V011 dispatch and `if constexpr` dtype bodies. | FP16/BF16 wide-LP only; no FP32 path, dtype-by-D generated body, or new dispatch domain. |

Cross-route disposition against `HOTLOOP-BRANCH-HOIST-CHAMPION-X` remains
`MECHANISM_DUPLICATE=NO` for H4/H5/H6. H4's row-boundary address correction
is only adjacent to HBH-02's generic terminal-row predicate; H5/H6 are only
adjacent to HBH-03's valid-length code surface. None selects coefficient mode,
relocates a row prefetch guard, or hoists valid length. No candidate is a
duplicate of TILECOUNT-STATIC-UNROLL or REDUCE-HIER.

## Host proof result

An ephemeral integer model of the exact `ChooseWideFullYRows` arithmetic and
the three candidate formulas was run on 2026-10-01. It tested every pass-1
unit for `batchRows=1..batchLimit`, `batchBegin=0,17`, and every pass-2 tile
and batch row in all six matrix rows above. Results:

```text
H4 next-prefetch parent/candidate offset equality: PASS
H5 pass-1 current-column cursor equality: PASS
H6 pass-2 parameter/store column cursor equality: PASS
TOTAL_HOST_PROOF_ERRORS=0
```

This proof does not validate Ascend C compilation, generated instructions,
NPU memory behavior, numerical output, or timing. It only removes the
algebraic address ambiguity before any future Track-A decision.

## Maturity, duplicate disposition, and next gate

```text
SURVIVING_INDEPENDENT_CANDIDATES=3
H4_MATURITY=READY_FOR_MAIN_REVIEW (host algebra PASS; codegen unverified)
H5_MATURITY=NEEDS_MORE_EVIDENCE (host algebra PASS; rollover codegen gate)
H6_MATURITY=NEEDS_MORE_EVIDENCE (host algebra PASS; pass-2 cost/codegen gate)
CODEGEN_PERFORMANCE_CLAIM=NONE
DUPLICATE_DISPOSITION=NO_EXACT_DUPLICATE; TILECOUNT/BRANCH/REDUCE_BOUNDARIES_FROZEN
MAIN_SELECTED=NONE
REVISION=NONE
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
NEXT_GATE=Planning/Review selects at most one H4/H5/H6; then exact-source codegen inspection is mandatory before any Revision
```

If the selected candidate fails its codegen gate, retain the failure as
research evidence and return to Planning/Review. Do not convert an unverified
instruction hypothesis into a local-performance claim, do not create a
Revision from this handoff alone, and do not change route lifecycle state.
