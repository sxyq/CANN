# HOTLOOP-BRANCH-HOIST-CHAMPION-X — bounded BRANCH planning pack

## Frozen scope and closure state

- DATE: 2026-10-02
- ROUTE: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- TRACK: `B` / parent-only research
- FOCUS_AXIS: `BRANCH_KIND`
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE: `线上结果/R31B/V011/submission.asc`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- MAIN_SELECTED: `NONE`
- REVISION_CREATED: `NO`
- CANDIDATE_CREATED: `NO`
- FORMAL_TIMING: `NO`
- CORRECTNESS: `NO`
- ONLINE: `NO`
- BRANCH_PLANNING_CANDIDATE_1: `HBH-10`
- BRANCH_PLANNING_CANDIDATE_2: `HBH-09`
- COMPILER_ELIMINATED: `NONE_PROVEN`

The V011 donor/path, host dtype dispatch, wide-path threshold, adaptive tile
geometry, addresses, DMA ownership, event lifetime, arithmetic, reductions,
buffers, and synchronization are frozen. The two ordered planning slots are
not a selection or permission to implement. HBH-11 remains a source-level
survivor but is not assigned a third planning slot. Any future emitted-code
result showing no claimed control removal is `CODEGEN_NOOP` and must be
rejected; no score or timing conclusion is made here.

## Parent dispatch facts

`run_kernel` launches only `float`, `half`, or `bfloat16_t` for dtype 0/1/2
(source lines 3543–3551). `Init()` sets `widePath_` only when
`rowWidth > 8192` (lines 56–113), and `Process()` sends non-float wide inputs
to `ProcessWideLowPrecision()` (lines 159–169). The generic body is reached
only after the existing special returns (lines 177–241); no new dispatch gate
is allowed in this pack.

## Parent-only compiler/static disposition

The exact parent bytes compile with the installed Ascend compiler; the
compiler/device-binary limitation is recorded in
`PARENT-COMPILER-STATIC-EVIDENCE-20261002.md`. No device IR or usable AICore
disassembly correlated to these source lines was available. Therefore the
runtime branch statuses below are deliberately `UNVERIFIED`, not claims that
the compiler retained or removed an instruction. The `if constexpr` dtype
arms are compile-time facts and are not surviving runtime hypotheses.

## Ordered planning slots

### BRANCH_PLANNING_CANDIDATE_1 — HBH-10

`ID=HBH-10`; `BRANCH_KIND=WIDE_PASS1_SLOT_SELECT`; `STATUS=STATIC_SURVIVOR`;
`COMPILER_STATUS=UNVERIFIED`.

- Exact function and condition: `AddRmsNormBiasKernel<T>::ProcessWideLowPrecision()`
  (lines 3076–3365), pass-1 unit loop (3125–3211). The current-slot control
  is `const bool useA = (u & 1) == 0` (3134), followed by the x/r local-slot
  and `rd`/`rel` selections (3135–3139). The lookahead state is
  `nextA = !useA` with x/r/`nrd` selection (3155–3160), plus the existing
  release checks and the final `if (useA)` ownership update (3161–3170,
  3205–3210).
- Invariant proof: `units=batchRows*tileCount`; `u`, its `batchRow/tile` decode,
  `col`, `rowOffset`, `nRow/nTile`, `nCol/nRowOffset`, `valid/nValid`, all GM
  addresses, the A/B local offsets, `rd0/rd1`, `rel0/rel1`, waits, loads,
  flags, `ReduceSum` destinations, arithmetic, final pending waits, and event
  releases must remain identical and ordered. A runtime state representation
  is admissible only if it does not introduce indirect tensor/event lookup,
  spills, or a new event schedule.
- Loop level and executions: one flattened pass-1 `u` iteration per unit,
  inside each `batchBegin` batch. For `K=tileCount`, `B=batchRows`,
  `U=B*K`: current parity/slot selection executes `U` times; lookahead slot
  selection executes `U-1` times. Per batch row, current selection is `K`; the
  next selection is `K` for every non-final batch row and `K-1` for the final
  batch row. The initial `u==0` load is one per batch and is not this
  hypothesis.
- Compiler result: `u` is runtime and parity changes every iteration; no
  source-level hoist is valid. The parent compiler probe proves compilation,
  but not device CFG/ISA removal, so this is `UNVERIFIED`, not
  `COMPILER_ELIMINATED`. If emitted code still parity-selects the same slots,
  classify `CODEGEN_NOOP`.
- Tail semantics: current `valid` and lookahead `nValid` remain the exact
  `min(rowWidth-offset,tileWidth)` values. The final partial tile still loads,
  computes, reduces, and releases the same slot; the terminal unit still has
  no lookahead load/flag. Runtime `tileWidth` and `tileCount` remain adaptive.
- Dispatch coverage: FP16/BF16 only, `rowWidth > 8192`, existing
  `ProcessWideLowPrecision()` path. FP32 wide, generic/specialized paths, and
  host dispatch are outside scope. No fixed D or tile-count bucket is valid.
- Duplicate audit: `ADDR=NO` (all index/address expressions frozen);
  `UB=NO` (same two slots and allocation); `SELECTIVE-FASTPATH=NO` (same
  call path); `TILECOUNT-STATIC-UNROLL=NO` only for a genuinely runtime state
  cursor. Alternating hand-written bodies, peeling, fixed trips, or constant
  offsets are `DUPLICATE_REJECTED`. Combining HBH-10 with HBH-11 is a second
  mechanism and is rejected.

### BRANCH_PLANNING_CANDIDATE_2 — HBH-09

`ID=HBH-09`; `BRANCH_KIND=GENERIC_REUSE_FENCE_GUARD`; `STATUS=STATIC_SURVIVOR`;
`COMPILER_STATUS=UNVERIFIED`.

- Exact function and condition: `AddRmsNormBiasKernel<T>::Process()` generic
  output loop (lines 159–494, output tiles 369–492). The first guard is
  `if (!cacheRow || !cacheParams)` before `SyncMTE2ToV()` (393–395). The
  second same predicate is at the dtype-selected output reuse site: FP32
  (464–466), FP16 (475–477), or BF16 (482–487). `cacheRow` and `cacheParams`
  are computed once before the row loop (242–247).
- Invariant proof: in a selected runtime mode, preserve the exact fence count
  and order, all operands, `valid`, `rowOffset+col`, coefficient source
  selection, vector operations, stores, `SyncVToMTE3`, `SyncMTE3ToV`, and the
  FP32 final-tile drain at 472–473. Only the repeated decision placement may
  change; no fence may disappear or move.
- Loop level and executions: the output tile loop is `col=0; col<rowWidth;
  col+=kTileElems` (370), with `K=ceil(rowWidth/4096)` in the row body
  (292–294). The two reuse-fence decisions execute `2*K` times per owned row
  (one at 393 and one dtype-selected site). The predicate is false in the
  cached multi-row mode and true in the streamed one-row mode, but that value
  is not proven compile-time for the device body.
- Compiler result: generic-path `cacheRow/cacheParams` are runtime values in
  the source even though they are loop invariant; the parent compile and
  host-side optimization record do not prove device loop unswitching. Status
  is `UNVERIFIED`; a boolean temporary retaining the same two decisions is
  `CODEGEN_NOOP`. The `if constexpr` selection among FP32/FP16/BF16 sites is
  already compile-time and is not a candidate.
- Tail semantics: `valid=TileLength(rowWidth-col)` remains unchanged, as does
  the last-tile FP32 `SyncMTE3ToV` condition. No row/tile count, coefficient
  residency, or parameter conversion may change.
- Dispatch coverage: only shapes reaching generic `Process()` are covered;
  e.g. `D=6144` for all three accepted dtypes, with `localRows=1` and
  `localRows>1`. `D>8192` is wide; exact D=4096/8192 or other existing
  specialized returns are not generic probes. No new shape/dtype dispatch is
  allowed.
- Duplicate audit: `ADDR=NO`, `UB=NO`, `SELECTIVE-FASTPATH=NO`,
  `REDUCE=NO`, and `TILECOUNT-STATIC-UNROLL=NO` while the tile loop and
  runtime offsets remain unchanged. Changing a fence, event protocol,
  coefficient operands, or loop trip count is outside this hypothesis. It is
  independent of HBH-10/11 because it is the generic path, not the wide
  low-precision slot state.

## Additional source-level survivor held outside the two slots

### HBH-11 — wide pass-2 parameter-slot selection

`ID=HBH-11`; `BRANCH_KIND=WIDE_PASS2_PARAM_SLOT_SELECT`; `STATUS=STATIC_SURVIVOR_BUT_UNSLOTTED`;
`COMPILER_STATUS=UNVERIFIED`.

- Exact function/condition: the pass-2 `for (tile=0; tile<tileCount; ++tile)`
  in `ProcessWideLowPrecision()` (3267–3352), with `useA=(tile&1)==0` and
  gamma/bias/`prd`/`prel` selections (3272–3276), the unchanged next-tile
  guard and `nextA` prefetch selections (3278–3297), and the final
  `if (useA)` release ownership (3346–3351).
- Invariant proof: preserve the initial prefetch, `prd0/prd1`, `prel0/prel1`,
  `pRelA/pRelB`, every wait/set/release, gamma/bias byte range, output offset,
  `SyncVToMTE2`, `SyncVToMTE3`, `SyncMTE3ToV`, and per-row math/store order.
  No V017-style MTE3 movement is allowed.
- Loop level and executions: parameter selection is once per parameter tile,
  not once per output row. For `K=tileCount`, current selection and
  `if(useA)` execute `K` times per batch; next-slot selection and its release
  branch execute `K-1` times. The inner `batchRow` loop performs the same
  selected parameter tile for each of `B=batchRows` rows.
- Compiler result: runtime `tile` parity is not loop invariant. No device IR
  proof of parity elimination exists; status is `UNVERIFIED`, and unchanged
  emitted parity or an indirect cursor is `CODEGEN_NOOP`/`INFEASIBLE`.
- Tail/dispatch/duplicate: current `valid` and next `nValid` remain the exact
  adaptive-tile min values, with no next prefetch after the final tile. The
  coverage is FP16/BF16 `rowWidth>8192` only. It is independent of HBH-10 at
  OFAT scope (pass 2 versus pass 1), but a combined pass-1/pass-2 cursor is
  rejected as a multi-mechanism change. ADDR, UB, selective dispatch,
  reduction, and static-unroll duplicates are all rejected under the same
  boundaries stated for HBH-10.

HBH-11 is retained as a factual survivor for Planning review but deliberately
does not create a third planning candidate.

## Explicit rejection list

- `HBH-12`: `REJECTED_NON_INDEPENDENT`. Its sole target is
  `if (u + 1 < units)` (3146), which controls the exact next-unit x/residual
  loads and MTE2/V flag. Removing that in-body terminal decision requires a
  loop split/peel, a fixed trip/offset form, or a moved prefetch schedule;
  each changes the loop/schedule ownership or violates the required ordered
  load/event invariant. A helper retaining the condition is `CODEGEN_NOOP`.
  The parent compiler did not prove it eliminated; this rejection is due to
  ownership, not a performance or timing result.
- `HBH-04/HBH-05` from earlier handoffs remain rejected fixed first-tile,
  fixed-trip, parity-unroll, and static-offset forms under
  `TILECOUNT-STATIC-UNROLL`; they are not survivors of this pack.
- No compiler-eliminated runtime condition is claimed. Any future emitted CFG
  that shows a condition already absent must be recorded as
  `COMPILER_ELIMINATED` and the corresponding planning item removed, not
  implemented as a no-op Revision.

## Safe evidence boundary

Safe parent-only evidence is limited to the exact source line references,
source hash, parent compiler/object identity, CFG/IR availability statement,
and the algebraic execution-count/tail proofs in the companion evidence file.
No Candidate, Revision, source edit, dispatch/donor/tile/DMA/UB edit, NPU run,
formal timing, score update, shared-ledger update, or Online action occurred.

