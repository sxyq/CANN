# HOTLOOP-ADDR-HOIST-CHAMPION-X — Next Track-B Planning Pack

## Scope and evidence boundary

```text
DATE_UTC=2026-10-04
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
TRACK=TRACK-B-ONLY
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
MAIN_SELECTED=NONE
REVISION_CREATED=NO
IMPLEMENTATION=NO
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
NPU=NOT_USED
```

The parent file SHA was recalculated and matches both its sidecar and the ADDR
V001/V002 parent records. The audited implementation is the exact V011
`ProcessWideLowPrecision` function, `线上结果/R31B/V011/submission.asc:3076-3365`.
The retained compiler record establishes a successful parent Release build,
but no emitted V011 instruction listing or parent-only profile was found.
Consequently all expected savings below are conditional source-level
hypotheses, not codegen or performance findings. Official case-to-shape
mapping is unavailable; listed dimensions are path probes only.

Read-only evidence used:

- Project entry, current project rules, and `.agents/skills/cann-mainline/SKILL.md`.
- `研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md`,
  `ADDR-PLANNING-PACK-20261002.md`,
  `TRACK-B-CROSS-BRANCH-CODEGEN-AUDIT-20261001.md`, and Track-B V004/V005.
- Exact parent source/sidecar and retained ADDR V001/V002 declarations,
  source identities, and score/correctness summaries.

No other Route's private context was read. This document proposes research
only; it does not select a hypothesis or authorize a Revision.

## Common frozen boundary

All candidates retain V011 dispatch, `tileWidth`, `tileCount`, traversal and
loop order, DMA counts/bytes, local-buffer ownership, reduction/math order,
events, waits, barriers, and stores. No row scheduling, tile specialization,
arithmetic, synchronization, allocation, aliasing, or lifetime change is
proposed. Any candidate whose generated code does not show a real reduction
without adverse live-range or branch cost is a codegen no-op/regression, not a
reason to proceed to implementation.

## Hypotheses

### ADDR-H09 — shared pass-1 ping-pong local-slot offset

```text
MECHANISM=Reuse one current-slot element offset for the x and residual local views
BOTTLENECK=Duplicate scalar slot selection/address materialization for paired VECCALC LocalTensor views
EXACT_FILE=线上结果/R31B/V011/submission.asc
EXACT_FUNCTION=ProcessWideLowPrecision
EXACT_CODE_SITE=3134-3137 (current xLocal/residualLocal); next-pair analogue 3155-3159 is out of first experiment
SINGLE_VARIABLE=Current pass-1 slot-address representation only
TARGET_DTYPES=FP16, BF16
TARGET_SHAPES=Wide-LP path probes D=32768; BF16 D=32768/40960 reduced-tile paths; include D=12288 as a smaller-tile-count control
```

**Proposed mechanism.** Preserve `useA`, event IDs, and all slot ownership;
derive one current slot element offset (`0` or `tileWidth`) and use it for
both `xBase` and `rBase` LocalTensor views. Do not change the next-prefetch
selection in the first OFAT form.

**Why it may help.** Both current views use the same predicate and the same
two-slot displacement. A shared scalar offset could avoid duplicated
conditional view/address materialization in the flattened pass-1 hot loop.

**Why it may fail.** CCEC may already common the two selectors. Dynamic
LocalTensor indexing can instead add an address operation or lengthen the
offset live range; the compiler may emit the same sequence as the existing
conditional expressions.

**Duplicate audit.** No exact match in ADDR H1-H8: H1/H2/H4/H5 address GM
inputs, H3 changes `(batchRow,tile)` decoding, H6 changes pass-2 column
formation, H7 shares a GM input offset, and H8 targets output-GM row bases.
This changes only the paired local ping-pong slot expression. It is adjacent
to the historical async/ping-pong mechanism but does not change pipeline
depth, parity, event order, or overlap; classify as non-duplicate only while
that boundary remains frozen.

**Minimal experiment (future, only after Planning selection).** First compare
the exact V011 compiler representation for the paired `xLocal` and
`residualLocal` selections with a one-change prototype. Verify both view
bases are identical to V011 for every `useA` value and that no extra branch,
register spill, event, or slot transition appears. No source/build/NPU action
is authorized by this pack.

**Risk.** Address/slot risk MEDIUM: an offset error can select the other live
buffer. Numeric precision risk LOW. UB footprint, DMA, synchronization, and
buffer lifetime are unchanged by intent. `CODEGEN_GATE=REQUIRED`.

### ADDR-H10 — batch-scope retained-y LocalTensor base handle

```text
MECHANISM=Materialize the FP16 retained-y LocalTensor view once per batch and reuse its base handle
BOTTLENECK=Repeated gammaBuf_.Get<half>() view acquisition in the pass-1 unit loop and pass-2 row loop
EXACT_FILE=线上结果/R31B/V011/submission.asc
EXACT_FUNCTION=ProcessWideLowPrecision
EXACT_CODE_SITE=3177-3179 and 3315-3317; allocation/role at 89-92
SINGLE_VARIABLE=Frequency/scope of the retained-y gammaBuf_ LocalTensor view acquisition
TARGET_DTYPES=FP16 only (BF16 already obtains valueFp32Buf_ outside the unit loop)
TARGET_SHAPES=Wide-LP FP16 probes D=12288/16384/32768; report actual batchRows and tileCount
```

**Proposed mechanism.** Obtain the same `gammaBuf_.Get<half>()` view once
within the batch scope and use it for the existing pass-1 retained-y writes
and pass-2 retained-y reads. Keep every element offset and access site
unchanged. The TBuf allocation remains exactly as in V011; this is not an
allocation, reuse, or alias change.

**Why it may help.** The accessor occurs inside both tile traversals although
the underlying retained-y buffer base is stable for the batch. If accessor
lowering materializes repeated base/descriptor work, extending one handle may
remove that work.

**Why it may fail.** `Get<half>()` may inline to nothing or already be
hoisted by CCEC. Extending the handle's live range can increase register
pressure, and the repeated accessor may not lie on the critical path.

**Duplicate audit.** No exact ADDR H1/H3 match: those change global input
address state and flattened index decoding. H8 is output-GM row-base
formation; this retains the existing local element offsets and changes only
the TBuf-view materialization frequency. It does not change UB lifetime,
allocation, aliasing, or reuse points, so it is not an UB-lifetime hypothesis.
No other Route-private audit was performed.

**Minimal experiment (future, only after Planning selection).** Inspect exact
V011 lowering for the two `Get<half>()` sites, then compare a single cached
view against the original for base identity, register/live-range effect, and
any emitted accessor instructions. If parent code already materializes one
base, classify `CODEGEN_NOOP`; do not proceed to a performance Revision.

**Risk.** Buffer identity risk LOW if the same `gammaBuf_` allocation is used
through both passes; register-pressure risk MEDIUM. No data address, DMA,
sync, precision, allocation, or lifetime semantic change is intended.
`CODEGEN_GATE=STRICT`.

### ADDR-H11 — reduction-scratch row-base table for partial slots

```text
MECHANISM=Precompute per-batch-row VECCALC reduction scratch slot bases, then index each partial as rowBase + tile
BOTTLENECK=Repeated batchRow * reduceStride component in partial ReduceSum destination addressing
EXACT_FILE=线上结果/R31B/V011/submission.asc
EXACT_FUNCTION=ProcessWideLowPrecision
EXACT_CODE_SITE=3094, 3188-3203, 3227-3231; reduce buffer allocation at 96-97 and 110-111
SINGLE_VARIABLE=Address generation for reduceLocal partial/final row slots only
TARGET_DTYPES=FP16, BF16
TARGET_SHAPES=Multi-row wide-LP probes D=12288/16384 with batchRows>1 and tileCount>1; one-row cases are negative controls
```

**Proposed mechanism.** Precompute the existing `batchRow * reduceStride`
base for each active row in the current batch, then use `rowBase + tile` for
the existing per-tile partial destinations and `rowBase` for the existing
row-finalization source. Keep `reduceStride`, slot mapping, `ReduceSum`
calls, their order, and all values unchanged. No flattened index decoder is
replaced.

**Why it may help.** A row base is invariant over a row's pass-1 tiles. If
V011 emits repeated multiply/add address setup for `reduceLocal[batchRow *
reduceStride + tile]`, a small base table could remove repeated row-stride
formation while preserving the exact scratch layout.

**Why it may fail.** `reduceStride` is constant and the compiler may already
strength-reduce or hoist this expression. A table adds setup, indexed access,
and live state; for `batchRows=1` it cannot provide useful row-base reuse.

**Duplicate audit.** Within this Route, this is not H1's pass-1 GM input
row-base, H3's logical `(batchRow,tile)` decoder, or H8's output-GM store
table: it changes only VECCALC reduction scratch address formation and leaves
the reduction algorithm untouched. Historical reduction/topology work is
adjacent but not the same address transform. **Cross-route duplicate/ownership
status is UNVERIFIED by design** because this task forbids reading another
Route's private context; Planning/Main must check the active REDUCE lane before
selecting it. It is not eligible for implementation until that check passes.

**Minimal experiment (future, only after Planning selection and ownership
clearance).** Use a source-level slot trace over every active row/tile and
partial final batch to prove exact index equality. Then inspect exact-parent
codegen for the multiply/slot sequence and table register cost. A repeated
multiply must be observed and the table must not spill before any later
correctness or timing stage is considered.

**Risk.** Scratch-index risk HIGH: a wrong slot can silently mix row partials.
Numeric precision risk LOW only if the exact slot trace passes. No UB
allocation/footprint, reduction order, DMA, event, or synchronization change
is intended. `CROSS_ROUTE_REVIEW=REQUIRED`; `CODEGEN_GATE=REQUIRED`.

## Separation and handoff status

The three ideas alter distinct operands/mechanisms: paired ping-pong slot
selection, TBuf view materialization, and reduction scratch slot addressing.
They are not combined into one Revision. H11 has an explicit cross-route
ownership/duplicate hold; H09/H10 still require generated-code evidence.

```text
HYPOTHESES=3
INTER_HYPOTHESIS_DUPLICATE=NONE_IDENTIFIED
H1_H3_RESTATED=NO
EXISTING_H4_H8_RESTATED=NO
H09_MATURITY=NEEDS_CODEGEN_EVIDENCE
H10_MATURITY=NEEDS_CODEGEN_EVIDENCE
H11_MATURITY=NEEDS_CROSS_ROUTE_REVIEW_AND_CODEGEN_EVIDENCE
MAIN_SELECTED=NONE
REVISION_CREATED=NO
IMPLEMENTATION=NO
BUILD/CORRECTNESS/TIMING/NPU=NOT_RUN
```

Planning/Review may reject any or all items. This pack makes no `PARK`,
`CLOSE`, `ABANDON`, `MERGE`, `REPLACE`, route-slot, implementation, or Online
decision.
