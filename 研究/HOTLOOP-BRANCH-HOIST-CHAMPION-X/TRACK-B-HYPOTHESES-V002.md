# HOTLOOP-BRANCH-HOIST-CHAMPION-X - Track-B hypotheses V002

## Status and frozen boundary

- DATE: 2026-10-01
- ROUTE: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- TRACK: `B` / research-only
- FOCUS_AXIS: `BRANCH_KIND`
- MAIN_SELECTED: `NONE`
- ROUTE_HYPOTHESIS_POOL_EXHAUSTED: `NO`
- REVISION_CREATED: `NO`
- BUILD_OR_CODEGEN_EXECUTED: `NO`
- CORRECTNESS_OR_TIMING_EXECUTED: `NO`
- ONLINE_EXECUTED: `NO`

The direct parent and donor are frozen to `R31B V011`:

- `DIRECT_PARENT=R31B V011`
- `PARENT_OFFICIAL=45.16`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `DONOR_PATH=线上结果/R31B/V011/submission.asc`
- `DONOR_PATH_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

This round keeps the V011 host and device boundary: `run_kernel` validates metadata,
dispatches one `T` instantiation per input dtype, and passes flattened `rowCount`,
`rowWidth`, `blockCount`, `invRowWidth`, and `epsilon` to the unchanged kernel. The
existing `widePath_` test, `ChooseWideFullYRows()` result, adaptive `tileWidth`, UB
buffers, DMA ownership, event allocation, arithmetic/reduction order, and output
store protocol are frozen. No proposal adds a shape bucket, changes the selected
V011 path, or introduces a new donor.

The previous V002 audit already covered HBH-01 (generic coefficient-mode guard),
HBH-02 (generic next-row terminal prefetch guard), and HBH-03 (wide runtime
tail-length calculation). This note advances the scan to the remaining control
surfaces; it does not rename those three hypotheses. HBH-04/HBH-05 remain rejected:
first-tile peeling, fixed-trip unrolling, static offset propagation, and alternating
hand-written bodies belong to `TILECOUNT-STATIC-UNROLL`.

## Evidence rule for this round

All three entries are `NEEDS_MORE_EVIDENCE`. No emitted instruction, register count,
latency, Local verdict, or Official gain is known. A source-level transformation is
informative only if the stated codegen gate proves that a repeated branch or dynamic
selection actually disappeared while the V011 event and data invariants remain.
Failure of that gate is a no-op or a rejected implementation, not a performance
result.

## HBH-06 - Generic reuse-fence control factoring

- `ID=HBH-06`
- `BRANCH_KIND=GENERIC_REUSE_FENCE_GUARD`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

### Mechanism and invariant

In the active generic `Process()` output loop, V011 evaluates the same row-mode
predicate `!cacheRow || !cacheParams` before `SyncVToMTE2()` and again at the
store-side reuse point (the generic output body around source lines 393 and
464/476/485). Factor only this *reuse-fence decision* to row scope, or use two
runtime-counted fence-mode loop bodies selected once per row. The coefficient load,
coefficient source selection, tensor aliases, arithmetic, store, and all fence calls
must stay otherwise identical. The exact number and relative order of
`SyncVToMTE2()`, `SyncVToMTE3()`, and `SyncMTE3ToV()` for each mode are invariant.

`cacheRow` is constant for the launch and `cacheParams` is constant for the core's
row group. The experiment may exploit that loop invariant, but may not infer a new
host-selected path from it. This is deliberately narrower than HBH-01: HBH-01
changes where coefficient-mode loads and cached/streamed parameter operands are
selected; HBH-06 changes only the repeated reuse-fence control and must not touch
those operand branches.

### Shape and dtype domain

- Generic path only: `4097 <= D <= 8192`; use `D=6144` as the primary probe.
- FP32 and BF16 are required codegen instantiations; FP16 is a control instantiation.
- Use one-row and multi-row core assignments so `cacheParams=false` and
  `cacheParams=true` are both represented. `D=8192, localRows=1` is only a
  streamed-mode control and does not prove the cached mode.
- No wide path, mid-size special path, or existing dtype-specific early return may
  be bypassed to manufacture a case.

### Why it may help / why it may fail

The fence decision is loop-invariant while the output tile loop is runtime-counted;
removing its repeated branch could reduce scalar/control instructions in the generic
FP32 path. The compiler may already common the predicate, or loop-body duplication
may increase code size and register pressure. The fence itself is not removable:
an apparent speedup caused by a missing wait is an invalid candidate.

### ASCEND feasibility and proof gates

- `UB_CORE_DMA_IMPACT`: no new UB allocation, buffer alias, DMA, or core ownership;
  same x/residual/parameter/output byte ranges and same `TileLength()` values.
- `SYNC_IMPACT`: zero protocol changes. The gate must preserve the mode-specific
  fence count and ordering, including the FP32 deferred MTE3 drain.
- `PRECISION_RISK`: none intended; no cast, vector op, reduction, rounding, or
  store type may change.
- `CODEGEN_PROOF_GATE`: inspect FP32, FP16, and BF16 emitted CFG/instructions. The
  repeated `!cacheRow || !cacheParams` decision must be absent from the claimed
  fence sites. If only a boolean temporary is introduced and the same branches
  remain, classify as `CODEGEN_NOOP`. Also record code size and register/UB usage.
- `HOST_PROOF_GATE`: leave `run_kernel` and dtype dispatch unchanged. The host-side
  probe must log dtype, flattened `M`, `D`, and `availableCoreNum`/`blockCount`, then
  prove the expected `localRows` mode on device for the one-row and multi-row
  cases. No new D or dtype dispatch bucket is admissible.

### Duplicate disposition and minimal experiment

- `SELECTIVE-FASTPATH`: `DUPLICATE=NO`. No new call target, shape gate, dtype gate,
  or operation subset is selected; the same generic `Process()` remains active.
- `ADDR`: `DUPLICATE=NO`. `rowOffset`, `col`, all GM offsets, and output addresses
  are byte-for-byte invariant.
- `TILECOUNT-STATIC-UNROLL`: `DUPLICATE=NO` for this bounded form. The tile loop,
  runtime trip count, terminal-tile predicate, and offsets remain dynamic. Any
  loop peel, fixed tile count, or static offset propagation is rejected and moved to
  that owner.
- `MINIMAL_OFAT_DIFF`: one generic `Process()` control-only diff at the two
  reuse-fence sites; no parameter-body rewrite, no host edit, no helper-path edit.
- `MINIMAL_EXPERIMENT`: after Main selection, compile exact V011-derived source for
  `D=6144` FP32/BF16 with `localRows=1` and `localRows=2+`; first pass the CFG,
  fence-count, source/executable identity, and targeted correctness gates. Only
  after those gates may a same-binary-qualified interleaved Parent/Candidate probe
  be considered. No timing is authorized by this research note.

## HBH-07 - Wide pass-1 two-slot state cursor

- `ID=HBH-07`
- `BRANCH_KIND=WIDE_PASS1_SLOT_STATE`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

### Mechanism and invariant

In the active FP16/BF16 `ProcessWideLowPrecision()` pass-1 stream, V011 derives
`useA` from `(u & 1)` and repeatedly selects the A/B local buffers and event IDs
for the current and next input units. Test a runtime two-slot cursor/state object
that carries the current and next `x/r` local handles plus `rd/rel` event handles,
swapping those handles after each unit. This is a representation change for the
existing two-slot state, not two hand-written loop bodies.

The following are invariants: `units`, `batchRow`, `tile`, `rowOffset`, `col`,
`nRow`, `nCol`, and every GM address are unchanged; initial slot A is unchanged;
the same loads, `SetFlag`, waits, vector operations, release flags, and event ID
release occur in the same order; and `valid`/tail bytes are unchanged. The
`u==0` first load and `u+1<units` next-load guards remain present. Pass 2 and the
host are not changed by this hypothesis.

### Shape and dtype domain

- Active wide low-precision path only: `D > 8192`, dtype FP16 or BF16.
- Probe `D=16384` as a full-tile control and `D=32768` as an adaptive-tile case;
  also use one width with `D % runtime_tileWidth != 0` for a tail diagnostic.
- The runtime `wideFullYTileElems_` and `tileCount` must be recorded after the
  existing `ChooseWideFullYRows()` logic. Never assume that BF16 `D=32768` uses a
  4096-element tile.
- The probe must include a unit stream with at least two units. It must not alter
  `wideFullYRows_` or manufacture a new batch shape.

### Why it may help / why it may fail

The current parity-derived ternaries can create repeated slot-selection control in
the largest active loop. A compiler may already lower them to the same fixed A/B
selection, or a cursor may introduce indirect local-tensor/event selection, spills,
or extra moves. No gain is expected until emitted code proves fewer control
instructions without a new indirect address path.

### ASCEND feasibility and proof gates

- `UB_CORE_DMA_IMPACT`: no buffer size, UB lifetime, DMA source/destination, or core
  mapping change. The two local slots and their ownership are fixed.
- `SYNC_IMPACT`: preserve `rd0/rd1`, `rel0/rel1`, `relA/relB`, all MTE2/V and V/MTE2
  waits, and the final pending-slot waits exactly. No wait may be removed or moved.
- `PRECISION_RISK`: none intended; FP16 native add, BF16 widening, FP32 square,
  reduction, and retained-y store representation are frozen.
- `CODEGEN_PROOF_GATE`: emitted code must show no residual `u & 1` compare or
  parity ternary and no slower indirect local-tensor/event lookup. Verify identical
  ordered DMA/event operations, code size, register use, and UB use. If the cursor
  lowers back to parity or adds dynamic selection, classify `CODEGEN_NOOP` or
  `INFEASIBLE`, with no performance claim.
- `HOST_PROOF_GATE`: `run_kernel` must still dispatch only dtype 1/2 to the same
  template and the device must still enter `widePath_` solely from the existing
  `D > 8192` test. Record the runtime tile width/count and source/executable SHA;
  no host-side shape specialization is allowed.

### Duplicate disposition and minimal experiment

- `SELECTIVE-FASTPATH`: `DUPLICATE=NO`. Every active wide FP16/BF16 input still runs
  the same runtime loop; there is no new D bucket or alternate operation path.
- `ADDR`: `DUPLICATE=NO`. The cursor may select local slots only; all GM address
  expressions and flattened index decoding are frozen. Any rolling GM cursor is an
  ADDR candidate and is rejected here.
- `TILECOUNT-STATIC-UNROLL`: `DUPLICATE=NO` only for a true runtime cursor. The
  loop trip count, `tileCount`, `tileWidth`, offsets, and first/last guards remain
  dynamic. Alternating hand-written bodies, loop peeling, fixed trip counts, or
  propagated tile offsets are `DUPLICATE_REJECTED` under that owner.
- `MINIMAL_OFAT_DIFF`: pass-1 slot selection only; do not touch pass-2 parameter
  slots, tail arithmetic, addresses, event placement, or host dispatch.
- `MINIMAL_EXPERIMENT`: after Main selection, use exact V011-derived FP16
  `D=16384` and BF16 `D=32768`, plus one runtime-tail width. First compare emitted
  pass-1 CFG and event/load trace, then run targeted full/tail correctness. Timing
  is gated on those proofs and on same-binary qualification; no timing result is
  asserted here.

## HBH-08 - Wide pass-2 parameter-slot state cursor

- `ID=HBH-08`
- `BRANCH_KIND=WIDE_PASS2_PARAM_SLOT_STATE`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

### Mechanism and invariant

In the active wide pass-2 parameter loop, V011 derives `useA` from the runtime
`tile` parity and selects `gBase`/`bBase` plus `prd0`/`prd1` and `prel0`/`prel1`.
Test a separate two-slot parameter cursor: retain the initial gamma/bias load,
keep the `tile + 1 < tileCount` prefetch guard, and advance a current/next
parameter-slot state after each tile. This hypothesis does not modify the pass-1
input pipeline; it is a distinct event lifetime and a distinct loop surface from
HBH-07.

The invariant is the exact gamma/bias byte range and `nCol`/`nValid` for every
prefetch, the same `prd` wait and `prel` release ownership, the same per-row
Muls/Mul/Add/FromFloat sequence, and the same `SyncVToMTE2`, `SyncVToMTE3`,
`SyncMTE3ToV`, and output store order. Preserve even the existing dtype-specific
conversion/barrier sequence; removing a seemingly redundant operation is outside
this branch-only hypothesis.

### Shape and dtype domain

- Active wide low-precision path only: FP16/BF16 with `D > 8192`.
- Use `D=16384` with at least four runtime parameter tiles as the primary control,
  `D=32768` to exercise adaptive tile width, and one width with a runtime tail.
- Include `batchRows=1` and, where the unchanged UB budget permits, `batchRows>1`.
  Record `wideFullYRows_`, runtime `tileWidth`, and `tileCount`; do not use a
  compile-time D bucket to infer them.

### Why it may help / why it may fail

The parameter loop repeatedly selects the same two local slots and event handles;
stateful handles might remove parity selection from the apply/prefetch loop. The
compiler may already fold the parity, or dynamic handle state may cost more than
the removed branch. The per-row store synchronization dominates many cases, so no
latency direction is claimed without codegen and qualified timing evidence.

### ASCEND feasibility and proof gates

- `UB_CORE_DMA_IMPACT`: no parameter buffer, y-row, output buffer, or UB lifetime
  change; gamma/bias DMA ownership and byte counts remain V011's.
- `SYNC_IMPACT`: `prd0/prd1`, `prel0/prel1`, `pRelA/pRelB`, initial prefetch,
  next-tile issue, final pending waits, and event release order must be identical.
- `PRECISION_RISK`: none intended; no dtype conversion, rounding, arithmetic, or
  output-store change is admissible.
- `CODEGEN_PROOF_GATE`: prove removal of the repeated tile-parity selection without
  residual branch, indirect local-tensor select, extra event moves, or register/
  code-size regression. A source cursor with unchanged emitted control is a no-op.
- `HOST_PROOF_GATE`: preserve the one-time host dtype dispatch and existing
  `widePath_`/adaptive-tiling device decisions. Verify the exact source SHA and
  runtime `tileWidth`/`tileCount` on FP16/BF16; no host dispatch or selective path
  may be added.

### Duplicate disposition and minimal experiment

- `SELECTIVE-FASTPATH`: `DUPLICATE=NO`. The same wide path, tile loop, and parameter
  operations run for all eligible wide FP16/BF16 shapes; only local slot state is
  represented differently.
- `ADDR`: `DUPLICATE=NO`. `col`, `nCol`, all gamma/bias GM offsets, output offsets,
  and flattened row state remain unchanged. A rolling address cursor is explicitly
  out of scope.
- `TILECOUNT-STATIC-UNROLL`: `DUPLICATE=NO` only while `tileCount` and the runtime
  loop remain unchanged. Fixed tile bodies, first/last peeling, parity-specialized
  unrolling, or static offset propagation are rejected as that route's mechanism.
- `MINIMAL_OFAT_DIFF`: pass-2 parameter-slot selection only; pass-1 input slots,
  tail calculation, math, sync placement, and host code remain untouched.
- `MINIMAL_EXPERIMENT`: after Main selection, compile the exact V011-derived FP16
  `D=16384` primary probe and BF16 `D=32768` adaptive probe, then one tail width.
  Compare emitted CFG, gamma/bias load bytes and event trace before targeted
  correctness. Only a codegen-positive, identity-matched, same-binary-qualified
  candidate may proceed to interleaved timing; this note contains no performance
  result.

## Pool decision and handoff

The next-round pool is three distinct control-flow probes: generic reuse-fence
factoring (HBH-06), wide pass-1 input-slot cursor (HBH-07), and wide pass-2
parameter-slot cursor (HBH-08). They share the frozen V011 donor but touch different
branch state and event lifetimes. None is selected:

```text
MAIN_SELECTED=NONE
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
```

Planning/Review must select at most one before any Track-A source change. Until
then, no Revision, build, codegen, correctness, timing, profiling, server3,
Online, shared-ledger, or control-worktree action is authorized by this note.
