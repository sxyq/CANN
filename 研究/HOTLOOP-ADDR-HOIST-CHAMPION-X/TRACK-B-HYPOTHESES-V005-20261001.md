# HOTLOOP-ADDR-HOIST-CHAMPION-X - Track-B Hypotheses V005

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

This round continues address-expression research from the committed route
handoff and V004 evidence. H1-H3 (V003) and H4-H6 (V004) are prior ideas and
are not counted again. The two hypotheses below isolate address surfaces that
remain in the exact V011 `ProcessWideLowPrecision` body; the two qualification
facts are negative-control and shape-boundary evidence, not selected work.

No `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, host wrapper, Revision, shared
ledger, server3, compiler, assembler, NPU, correctness, timing, profiling, or
Online command was run. A temporary host-only integer model was used for the
shape table and address equalities; its output was not written to the
repository and is not a correctness or performance result.

## Parent surfaces and prior-handoff audit

The frozen V011 source has the following relevant expressions:

- Pass 1 current input loads use `rowOffset + col` twice at source lines
  3140-3143. The same expression is repeated for x and residual, without a
  named offset value.
- Pass 1 next input prefetch uses `nRowOffset + nCol` twice at lines
  3146-3174. H4 already studies replacing this next-unit expression by a
  current-offset delta; V005 does not restate H4.
- Pass 2 computes `col = tile * tileWidth` at line 3268 and uses it for
  retained-y/local indexing and the store. H6 already studies a pass-2
  column cursor; V005 does not restate H6.
- Pass 2 computes `nCol = col + tileWidth` at lines 3291-3296. This is already
  affine and is the basis for the negative codegen fact Q9 below.
- The output store repeats `(batchBegin + batchRow) * rowWidth + col` at
  lines 3341-3343 for every tile and batch row. This is a separate output
  row-base surface from H1's pass-1 row-base state.
- `tileWidth` is runtime state and starts at 4096, but
  `ChooseWideFullYRows` can reduce it to 2560 or 2048 for a one-row fallback.
  No proposal in this file assumes a fixed tile width.

The route's prior evidence establishes:

| Prior evidence | What remains frozen | V005 consequence |
|---|---|---|
| V003 H1-H3 | pass-1 row-base, input cursor, and flattened index decode were separate axes | V005 H7 is only pairwise expression reuse; H8 is pass-2 output-store state |
| V004 H4-H6 | next pass-1 delta, pass-1 current-column cursor, and pass-2 current-column cursor were separate axes | H7 does not create a cursor; H8 does not change `col`; Q9 rejects a redundant next-parameter cursor |
| Cross-branch audit | branch guards, final-tile valid length, static tile specialization, and reduction are outside this route | all items preserve predicates, counts, bytes, event order, and reduction indices |
| V011 exact source | wide low-precision pipeline is the parent, not a claim that all Official cases use it | shape facts record path qualification instead of inferring Judge case IDs |

## ADDR-H7 - paired current-input offset CSE

### Focus axis and mechanism

`FOCUS_AXIS=pass-1 same-unit GM offset expression reuse`.

Name one `uint64_t currentInputOffset = rowOffset + col` immediately before
the current x/residual loads and pass that same value to both `Load` calls.
This is deliberately narrower than H2: it does not create a persistent
cursor, change row/tile state, or alter the next-unit prefetch. The first
version must leave `nRow`, `nTile`, `nCol`, `nRowOffset`, `nValid`, and both
prefetch calls in their V011 form.

### Why it may help and why it may fail

- It may expose one producer of the identical current GM offset to the two
  adjacent MTE2 address operands and remove a duplicated scalar add in the
  generated sequence.
- The compiler may already common-subexpression-eliminate the repeated
  expression. A named source value can also add a move or extend its live
  range, so source simplification alone is not evidence.
- The change cannot improve DMA geometry or overlap by itself. Wave-1 reports
  that DMA command shape is not the dominant issue and that pipeline overlap
  matters; therefore a codegen gate is mandatory before any Track-A request.

### Qualification and minimal proof

```text
STATIC_SOURCE_FEASIBLE=YES
CODEGEN_GATE=STRICT
ADDRESS_RISK=LOW
PRECISION_RISK=LOW
ASCEND_FEASIBILITY=READY_FOR_MAIN_REVIEW
```

The codegen gate must show either one address producer feeding both current
loads already present in V011, or a real reduction in scalar address
instructions with no extra branch, byte conversion, or harmful live-range
growth. If V011 already emits one producer, classify H7 as `CODEGEN_NOOP` and
do not create a Revision.

The host proof must enumerate every pass-1 unit for `batchRows=1..batchLimit`,
including a partial final batch and a nonzero `batchBegin`, and assert:

```text
currentInputOffset == rowOffset + col
x_offset == currentInputOffset
residual_offset == currentInputOffset
valid, DMA count, DMA bytes, and event sequence == V011
```

The proof is an address/dataflow check only. It does not authorize source
implementation, build, correctness, or timing.

### Duplicate disposition

`NOT_DUPLICATE_WITH_H1-H6` at the mechanism level. H1 changes row-base state;
H2 changes the sequence of current and next input offsets; H4 changes only the
next prefetch offset; and H5 changes current-column state. H7 only names the
already identical current offset for the two current loads. It does not touch
the branch, tile-count, reduction, or parameter axes.

Maturity: `NEEDS_MORE_EVIDENCE` because the expected compiler transform is
likely already implicit in the parent.

## ADDR-H8 - pass-2 output-store row-base table

### Focus axis and mechanism

`FOCUS_AXIS=pass-2 output GM row-base reuse`.

At the start of each wide-LP batch, form a small scalar table for the active
rows:

```text
storeRowBase[batchRow] =
    (batchBegin + static_cast<uint64_t>(batchRow)) * rowWidth
```

Use `storeRowBase[batchRow] + col` only as the output-GM offset in `Store`.
Keep the parent tile-outer then batch-row-inner order, `col`, `valid`, local
retained-y offsets, all V/MTE3 barriers, and every store count unchanged.
This isolates the repeated output row multiplication. It is not H1: H1 is
pass-1 current/prefetch input address state. It is not H6: H6 is pass-2
column formation.

### Why it may help and why it may fail

- The current store expression repeats the same row multiplication for every
  tile of every active batch row. Reusing the row base could reduce scalar
  work when `tileCount` is large, especially the BF16 reduced-tile case.
- A table introduces up to eight 64-bit values and a table load on every
  store. Register pressure, local scalar spills, or an extra address load may
  exceed the removed multiply. The codegen comparison must include the
  complete store address sequence, not only source operation counts.
- Reordering the loops to avoid the table is out of scope because it could
  change store/event order and would become a different hypothesis.

### Qualification and minimal proof

```text
STATIC_SOURCE_FEASIBLE=YES
CODEGEN_GATE=STRICT
ADDRESS_RISK=MEDIUM
PRECISION_RISK=LOW
ASCEND_FEASIBILITY=CONDITIONAL
```

The first gate is static/codegen inspection: the table must not spill, change
the loop trip count, add a branch, or alter the V-MTE3/MTE3-V dependency
sequence. Reject H8 as `CODEGEN_REGRESSION_RISK` if the table load and scalar
state cost more than the removed multiply.

The host proof must enumerate every active `batchRow` and tile, including the
last tile and a partial final batch, and assert:

```text
storeRowBase[batchRow] ==
    (batchBegin + batchRow) * rowWidth
store_offset == storeRowBase[batchRow] + col
store_offset == V011 store offset
valid, store byte count, store order, and event order == V011
```

Maturity: `NEEDS_MORE_EVIDENCE`. This item is a source-level opportunity,
not a performance claim; its table cost is unresolved without generated code.

### Duplicate disposition

`NOT_DUPLICATE_WITH_H1-H6` at the mechanism level. It is adjacent to H1 by
using a row base, but H1 targets pass-1 input and next-prefetch formation,
while H8 targets only pass-2 output-GM stores. It is adjacent to H6 through
the same pass-2 loop, but H6 changes the column operand and H8 leaves `col`
unchanged. It does not change tile geometry, row ownership, parameter DMA,
reduction, or synchronization.

## Q9 - already-affine pass-2 next-parameter offset

`FOCUS_AXIS=pass-2 next gamma/bias address qualification`.

The parent source already computes `nCol = col + tileWidth`; it does not
recompute `nCol = nTile * tileWidth` in pass 2. A proposed V005
"next-parameter cursor" would therefore be a restatement of the existing
affine add, not an independent address-hoist hypothesis. The correct outcome
is a negative qualification fact:

```text
PARENT_SOURCE_FACT=pass-2 nCol is already col + tileWidth
NEW_MULTIPLY_TO_REMOVE=NONE
CODEGEN_GATE=NO_NEW_HYPOTHESIS
DISPOSITION=DUPLICATE_REJECTED_AS_PARENT_AFFINE_FORM
```

This also narrows H6's scope: H6 may investigate the current pass-2 `col`
formation only; it may not claim a second win from next-parameter address
formation. Q9 is not selected and does not create a Revision.

## Q10 - exact shape and path qualification

`FOCUS_AXIS=wide-LP dispatch and runtime tile qualification`.

The exact V011 dispatch enters `ProcessWideLowPrecision` only when
`rowWidth > kCacheElems`, with `kCacheElems=8192`, and only for FP16/BF16.
FP32 uses `ProcessWideFp32`; D=8192 is not a target for this route. The
host-only model re-evaluated `ChooseWideFullYRows` with its exact 176 KiB
budget, eight-row maximum, 4096 initial tile, and 512-element decrement:

| dtype | D | batchLimit | tileWidth | tileCount | final tile |
|---|---:|---:|---:|---:|---:|
| FP16 | 12288 | 3 | 4096 | 3 | 4096 |
| FP16 | 16384 | 2 | 4096 | 4 | 4096 |
| FP16 | 24576 | 1 | 4096 | 6 | 4096 |
| FP16 | 32768 | 1 | 4096 | 8 | 4096 |
| BF16 | 12288 | 2 | 4096 | 3 | 4096 |
| BF16 | 16384 | 1 | 4096 | 4 | 4096 |
| BF16 | 24576 | 1 | 4096 | 6 | 4096 |
| BF16 | 32768 | 1 | 2560 | 13 | 2048 |
| BF16 | 40960 | 1 | 2048 | 20 | 2048 |

The qualification consequences are:

- H7 has the most meaningful row-rollover coverage on FP16 D=12288/16384
  and BF16 D=12288, but it must still include one-row cases.
- H8 needs high `tileCount` coverage; BF16 D=32768 and D=40960 stress the
  reduced-tile path and final-tail store address.
- A `batchRows < batchLimit` case is required even when the table is sized for
  the maximum batch. Full-batch-only algebra is insufficient.
- These are kernel-path probes, not recovered Official testcase IDs. No score
  or Official-shape mapping is inferred.

## Required duplicate audit

The audit uses the committed historical route records, the exact V011 source,
the V003/V004 route handoffs, public R31A/R31B/MIX evidence, and the committed
MAIN-2 Wave-1 receipt. `NO_EXACT_DUPLICATE` means the changed expression or
predicate is different; shared loop or source regions are recorded as
adjacency rather than treated as proof of uniqueness.

| Audited family | Recorded mechanism | H7 disposition | H8/Q9/Q10 disposition |
|---|---|---|---|
| R005 / FULL-R005 | larger tile and fewer tile iterations; tile/UB/DMA geometry changes | no tile or DMA change; no duplicate | H8 keeps runtime tile geometry; Q9 rejects only an already-affine parent form |
| R012 / FULL-R012 | 32B row-group alignment and row ownership | no alignment, padding, or ownership change | no alignment or row-group change; shape table preserves the parent tile/tail rules |
| R028 / FULL-R028 | scalar `GetValue`/sync and batched RMS scalar work | no scalar reduction, `GetValue`, event, or barrier change | no scalar/sync change; H8 preserves store-side synchronization exactly |
| R029 / FULL-R029 | dispatch taxonomy and wide cached-row/retained-y alternatives; FULL-R029 V001 runtime error | no dispatch, cached-row, or retention change | H8 does not make a cached-row path; Q10 does not add a mode or dtype-by-D body |
| R31A | V024 `invRms` arithmetic hoist out of an affine loop | address operand only; math order is frozen | H8 is output address only; no arithmetic hoist or vector operation changes |
| R31B | V011 is the frozen 2-deep MTE2/V/MTE3 parent and direct source of the loop | parent address surface only; H7 is pairwise same-unit reuse | H8 is pass-2 store row-base only; Q9 confirms next parameter offset is already affine |
| MIX-A | V003/V007 mode replacement and a `SyncVToMTE2` removal in a narrow/mid path | no mode or synchronization change; wide-LP pass 1 stays V011 | no mode gate or sync change; H8 keeps the V011 wide-LP loop and Q10 keeps dispatch fixed |
| MAIN-2 Wave-1 | reduction, V/S, parameter MTE2, DMA-shape, and async-overlap findings | H7 does not touch the falsified reduction/parameter axes; codegen proof is required because overlap dominates | H8 is store-address scalar work, not DMA shape or async scheduling; Q9 avoids claiming an existing affine add as a new axis |
| Own prior handoff | H1-H3 and H4-H6 already cover row state, input cursor, index decode, next-prefetch delta, and current columns | H7 has no cursor and only merges the two current-load operands | H8 targets output-store row base; Q9 is explicitly rejected as already present; Q10 is a qualification fact |

Audit result:

```text
DUPLICATE_AUDIT=PASS_FOR_NEW_MECHANISMS
EXACT_DUPLICATES=NONE_FOR_H7/H8
PARENT_AFFINE_NEGATIVE=Q9
BRANCH/TILECOUNT/REDUCE/DMA/EVENT_BOUNDARIES=FROZEN
SHAPE_QUALIFICATION=Q10
```

## Handoff result

```text
SURVIVING_NEW_OR_REFINED_ITEMS=4 (H7, H8, Q9, Q10)
H7_MATURITY=NEEDS_MORE_EVIDENCE (strict codegen CSE gate)
H8_MATURITY=NEEDS_MORE_EVIDENCE (store-table codegen/register gate)
Q9_MATURITY=QUALIFICATION_FACT (no new hypothesis)
Q10_MATURITY=QUALIFICATION_FACT (path/shape boundary)
MAIN_SELECTED=NONE
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
ONLINE=NOT_RUN
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
NEXT_GATE=Planning/Review may select at most one prior or new address item; any selected item requires exact-source codegen evidence before Revision
```

This handoff reports facts and proposes no route lifecycle decision. It does
not select H7 or H8, and it does not convert the host algebra or shape table
into a local or Official performance claim.
