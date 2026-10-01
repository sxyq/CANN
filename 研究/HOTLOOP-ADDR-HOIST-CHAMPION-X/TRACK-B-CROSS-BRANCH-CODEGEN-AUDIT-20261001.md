# HOTLOOP-ADDR-HOIST-CHAMPION-X - Track-B Cross-Route Audit

## Audit metadata

```text
DATE_UTC=2026-10-01
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
TRACK=TRACK-B-ONLY
MAIN=MAIN-2
MAIN_SELECTED=NONE
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BRANCH_REFERENCE_COMMIT=01641fed8a1096d000de3a0bc1b3db13cab53f84
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
PROFILING=NOT_RUN
ONLINE=NOT_RUN
```

This is a static duplicate and codegen-feasibility audit. No compiler,
assembler, NPU, server3, or profiling command was run. No `.asc`, `.cpp`,
`.h`, `.hpp`, CMake, runner, host wrapper, Revision directory, shared ledger,
or control file was changed.

## Evidence boundary

- The frozen parent is the committed `线上结果/R31B/V011/submission.asc`.
  `ProcessWideLowPrecision` is at source lines 3076-3365. Its active pass-1
  stream decodes `u` into `batchRow` and `tile`, computes `col` and
  `rowOffset`, and independently computes `nRow`, `nTile`, `nCol`, and
  `nRowOffset` for the prefetch.
- The parent source computes `tileCount` at lines 3091-3093 and keeps
  `tileWidth` in the runtime member `wideFullYTileElems_`. The member starts
  at 4096, but `ChooseWideFullYRows` can reduce it in the N=1 UB fallback;
  therefore no audit item may assume a fixed 4096-element tile for every
  target shape.
- The parent `Load` helper accepts a `uint64_t` element offset and passes it
  to `src[offset]` at lines 3367-3375. H1 and H2 can therefore stay within the
  existing load API if their offsets remain element offsets and their counts
  are unchanged.
- The compared branch route is the committed
  `研究/HOTLOOP-BRANCH-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md` at
  `01641fed8a1096d000de3a0bc1b3db13cab53f84`. Its surviving hypotheses are
  HBH-01 coefficient-mode guard, HBH-02 generic next-row prefetch terminal
  guard, and HBH-03 final-tile valid-length hoist.

The V011 Official shape-to-case map remains unavailable in committed evidence.
Any future probe must record the actual dispatch, dtype, `tileWidth`,
`tileCount`, and concrete shape rather than infer a Judge testcase identity.

## Duplicate audit against HOTLOOP-BRANCH-HOIST

| ADDR item | HBH-01 coefficient mode | HBH-02 terminal-row prefetch | HBH-03 final-tile length |
|---|---|---|---|
| ADDR-H1 row-base hoist | `OVERLAP=NO`. H1 changes pass-1 row address formation; HBH-01 changes generic output coefficient-source selection. | `MECHANISM_DUPLICATE=NO`. H1 has row-base rollover bookkeeping in the wide-LP stream; HBH-02 moves a generic-path terminal-row predicate. | `CODE_SURFACE=ADJACENT; MECHANISM_DUPLICATE=NO`. H1 preserves `valid`; HBH-03 preserves row/index/address expressions and only hoists valid-length arithmetic. |
| ADDR-H2 rolling input cursor | `OVERLAP=NO`. H2 changes only pass-1 x/residual element offsets. | `MECHANISM_DUPLICATE=NO`. H2's row-tail cursor correction is not the generic next-row prefetch predicate. | `CODE_SURFACE=ADJACENT; MECHANISM_DUPLICATE=NO`. H2 must preserve valid lengths and DMA counts; HBH-03 does not change input offsets. |
| ADDR-H3 stateful row/tile counters | `OVERLAP=NO`. H3 changes flattened index decoding, not coefficient mode. | `MECHANISM_DUPLICATE=NO`. H3's counter rollover is local to the wide-LP flattened stream, not the generic row prefetch. | `CODE_SURFACE=ADJACENT; MECHANISM_DUPLICATE=NO`. HBH-03 explicitly retains `u`, `batchRow`, `tile`, and the flattened loop; H3 is the separate index-decoder change. |

The shared `ProcessWideLowPrecision` source region is not sufficient to call
these mechanisms duplicates. HBH-03 keeps the address and index expressions
fixed, while H1/H2/H3 change those expressions or their state and keep the
per-unit `valid` result fixed. H1/H2/H3 also do not touch HBH-01's generic
coefficient mode or HBH-02's generic next-row prefetch guard.

The following remains explicitly outside this route: fixed-trip tile-count
specialization, static unrolling, hand-written parity bodies, static offset
propagation, a new dtype-by-D generated body, or a changed dispatch domain.
Those are the branch route's rejected/owned classes, and turning H3 into one
of them would make the proposed change a duplicate rather than an ADDR
index-strength-reduction experiment.

## Static codegen-feasibility audit

No generated instruction stream is available because this task forbids build
and codegen execution. The labels below mean that the source transform is
expressible under the frozen API and dataflow; they do not claim that the
compiler emits fewer instructions.

### ADDR-H1 - row-base hoist

- `STATIC_SOURCE_FEASIBLE=YES`: a `uint64_t rowBase` can replace the repeated
  row multiplication for the current unit and the prefetched next unit while
  leaving `u`, `batchRow`, `tile`, `col`, `valid`, and all DMA calls intact.
- `CODEGEN_GATE=UNVERIFIED`: the expected win is removal of one or two
  row-base multiplies per unit. The emitted code may instead retain a multiply
  or trade it for a row-rollover compare, a conditional add, and one extra
  live 64-bit register. The existing runtime `u / tileCount` decode remains,
  so H1 is not an index-decoder optimization.
- `ASCEND_FEASIBILITY=READY_FOR_MAIN_REVIEW`: no new API, UB buffer, core
  mapping, DMA descriptor, event, barrier, math, or precision operation is
  needed. The required source boundary is pass 1 only; pass 2 stays unchanged.
- `PRECISION_RISK=LOW`; `ADDRESS_RISK=ROW_ROLLOVER`: numerical order is
  unchanged, but the row base must equal the original offset at every tile and
  every prefetch boundary.

### ADDR-H2 - rolling input-address cursor

- `STATIC_SOURCE_FEASIBLE=YES`: the existing `Load` helper accepts the needed
  `uint64_t` element offset, so one cursor can feed both x/residual loads for
  the current unit and the next prefetch. `tile`, `col`, `valid`, local tensor
  offsets, pass 2, and stores remain independent and unchanged.
- `CODEGEN_GATE=UNVERIFIED`: the likely removable work is the row multiply and
  row-plus-column composition on the pass-1 GM input operands. The cursor adds
  a row-boundary correction and a live 64-bit value; register pressure or the
  correction predicate can erase the gain. The correction must use the actual
  runtime `tileWidth` and `tileCount`, including the N=1 reduced-tile case.
- `ASCEND_FEASIBILITY=READY_FOR_MAIN_REVIEW`: no DMA count or byte formula
  changes. The cursor is in elements, not bytes, and both x/residual calls
  receive the same offset sequence as V011.
- `PRECISION_RISK=LOW`; `ADDRESS_RISK=HIGH`: an incorrect final-tile delta
  can alias a neighboring row without changing vector arithmetic.

### ADDR-H3 - stateful row/tile counters

- `STATIC_SOURCE_FEASIBLE=YES`: two dynamic scalar counters can replace the
  current `u / tileCount` and `u - batchRow * tileCount` expressions in pass 1;
  the `u` loop itself must remain because V011 uses its parity for the two DMA
  slots. The next-unit prefetch needs an equivalent compare-and-wrap state.
- `CODEGEN_GATE=CONDITIONAL`: `tileCount` is runtime data, so the current
  source visibly contains a variable-divisor decode at lines 3126-3127 and
  3147-3148. H3 is useful only if generated code retains material divide or
  divide-derived scalar work. If the compiler already lowers it to a cheap
  multiply/shift sequence, counter updates and rollover branches can be a
  regression or a no-op.
- `ASCEND_FEASIBILITY=NEEDS_MORE_EVIDENCE`: the source form is legal under the
  frozen dataflow, but the counter model must cover every `batchRows`,
  `tileCount`, and partial final batch without changing DMA/event order.
- `PRECISION_RISK=LOW`; `ADDRESS_RISK=HIGH`: counters do not alter arithmetic,
  but a wrap error can select a valid-looking wrong row or tile.

## Required OFAT gates if Planning selects one item

These are gates for a future selected Revision, not work performed by this
audit.

| Candidate | Single-change boundary | First codegen evidence | Host/address evidence |
|---|---|---|---|
| H1 | pass-1 row-base state only; no pass-2 or valid/DMA change | emitted scalar operations and live-register comparison must show whether row multiply disappeared | exact equality of current/prefetch GM offsets for full and partial rows |
| H2 | pass-1 x/residual GM offsets only; no logical index, output, or valid change | emitted input-address sequence and register count; confirm no new byte/stride arithmetic | exhaustive cursor trace using actual runtime `tileWidth` and `tileCount` |
| H3 | pass-1 index decode only; no static tile specialization or address cursor | prove whether variable-divisor work is removed; reject if codegen is already strength-reduced | exhaustive `(batchRow,tile)` equality, including partial final batches and prefetch units |

Only after the codegen gate passes would the selected item justify the
existing Track-B-to-Track-A sequence. Future probes should use exact V011
dispatch with FP16/BF16 wide-LP shapes around D=12288, 16384, and 32768,
recording M, dtype, actual tile values, and both one-row and multi-row cases.
No local score, Official score, or timing verdict is inferred here.

## Result

```text
CROSS_ROUTE_DUPLICATE=NONE_FOR_ADDR-H1/H2/H3
CODE_SURFACE_ADJACENCY=HBH-03_ONLY
STATIC_SOURCE_FEASIBILITY=H1 YES; H2 YES; H3 YES
CODEGEN_EVIDENCE=NOT_OBSERVED
MATURITY=H1 READY_FOR_MAIN_REVIEW; H2 READY_FOR_MAIN_REVIEW; H3 NEEDS_MORE_EVIDENCE
MAIN_SELECTED=NONE
REVISION_CREATED=NO
```

This audit adds no candidate selection and no route-lifecycle decision. The
current parent and Route handoff remain frozen until Planning/Review chooses
at most one hypothesis.
