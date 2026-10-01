# HOTLOOP-BRANCH-HOIST-CHAMPION-X - Track-B hypotheses V003

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
- SERVER3_OR_ONLINE_EXECUTED: `NO`
- DIRECT_PARENT: `R31B V011`
- PARENT_OFFICIAL: `45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SOURCE: `线上结果/R31B/V011/submission.asc`

This record refines the remaining branch-control search around exactly one axis:
remove or hoist repeated control decisions while preserving the V011 donor,
dispatch domain, runtime tile geometry, operation order, data addresses, event
protocol, reductions, arithmetic, and buffers. No source transformation is
selected by this record.

The codegen statements below are static feasibility evidence from the committed
source. No compiler, build, disassembly, NPU, timing, server3, or Online result
is claimed. A future Track-A implementation must prove the stated emitted-CFG
gate before it can be treated as an informative hypothesis.

## Source and shape evidence

### Generic path

The V011 source defines `kMidRowMinWidth=128`, `kTileElems=4096`, and
`kCacheElems=8192` (source lines 1241-1242 and 1288). `widePath_` is enabled
only for `rowWidth > 8192` (lines 56-61), so `D=8192` is not a wide-path probe.
For `D=6144`, the existing mid-size return is bypassed and the generic
`Process()` body is active. The source computes `cacheRow` and the
loop-invariant `cacheParams` at lines 242-247:

```text
cacheRow    = (D <= 8192)
cacheParams = cacheRow && (localRows > 1)
```

The output loop then repeats mode-dependent selections at lines 383, 389,
393, 416, 430, 442, 464, 477, and 485. This is direct source evidence for a
generic branch-control surface. It is not evidence that the compiler emitted
those branches.

Shape controls:

| Probe | Existing path fact | Branch state exercised |
|---|---|---|
| FP32/FP16/BF16 `D=6144`, `localRows=1` | Generic path, two 4096-element tiles | `cacheParams=false`; BF16 `cacheParamFp32=false` |
| FP32/FP16/BF16 `D=6144`, `localRows>=2` | Generic path, same runtime tile loop | `cacheParams=true`; BF16 also `cacheParamFp32=true` |
| `D=8192`, `localRows=1` | Generic streamed control only | Does not prove cached-mode removal |
| `D=8192`, `localRows>1` | Existing dtype-specific full-row return may win first | Not a valid generic branch probe |

No host shape gate or new dispatch bucket is allowed to manufacture these
states. The runner must report the actual `M`, `D`, `blockCount`, and observed
`localRows` if a hypothesis is ever selected.

### Active wide low-precision path

`ProcessWideLowPrecision()` computes runtime `tileWidth` from
`ChooseWideFullYRows()` (lines 3089-3094 and 1293-1329). The source computes
`tileCount` at lines 3091-3093, decodes `u` into `batchRow` and `tile` at
3125-3128, selects pass-1 slots from `u & 1` at 3134-3139, and guards the
next-unit load at 3146-3174. Pass 2 selects parameter slots from `tile & 1`
at 3267-3276 and guards the next parameter load at 3278-3297.

Static budget arithmetic gives useful shape evidence, but not a replacement
for runtime logging:

- FP16 `D=16384` remains a wide path and fits a two-row complete-y batch with
  the initial 4096-element tile; it is a useful multi-unit control.
- FP16 `D=32768` reaches the one-row wide path with the initial 4096-element
  tile under the V011 budget.
- BF16 `D=32768` uses `sizeof(float)` for retained `y` in the chooser and
  therefore shrinks the one-row tile to 2560 elements under the 176 KiB
  budget. It is not a 4096-element control; the runtime result must be
  recorded.
- A tail diagnostic must use an existing accepted width whose remainder is
  nonzero for the observed runtime `tileWidth`. Do not change `tileWidth` or
  add a shape bucket to manufacture a tail.

The V011 wide invariants are exact: `units=batchRows*tileCount`,
`col=tile*tileWidth`, `rowOffset=(batchBegin+batchRow)*rowWidth`, all `valid`
and `nValid` byte counts, `rd0/rd1`, `rel0/rel1`, `prd0/prd1`, `prel0/prel1`,
their waits and releases, `ReduceSum` destinations, and output store offsets.

## Refined hypothesis pool

All hypotheses below are `NEEDS_MORE_EVIDENCE`. They are mutually exclusive
OFAT concepts even where they touch neighboring source lines. Each has exactly
one `BRANCH_KIND`; changing another axis is a rejection, not a refinement.

### HBH-09 - Generic reuse-fence guard factoring

- `ID=HBH-09`
- `BRANCH_KIND=GENERIC_REUSE_FENCE_GUARD`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

Refine HBH-06 to the two repeated generic reuse-fence decisions only: the
`!cacheRow || !cacheParams` tests before `SyncVToMTE2()` and at the output
reuse sites. A valid implementation may select one runtime-counted fence mode
for the row or use equivalent control factoring, but it must not rewrite
coefficient operand selection, `cacheParamFp32`, vector operations, stores, or
any fence itself.

Invariant evidence:

- `cacheRow` and `cacheParams` remain the existing runtime booleans from
  lines 242-247; no host dispatch or dtype-by-D specialization is introduced.
- The mode-specific count and relative order of `SyncVToMTE2`,
  `SyncVToMTE3`, and `SyncMTE3ToV` remain unchanged, including the FP32
  deferred final-row MTE3 drain at lines 467-473.
- `rowOffset`, `col`, `valid`, all GM ranges, output staging, and all vector
  arithmetic remain unchanged.

Static codegen gate:

- FP32, FP16, and BF16 emitted CFGs must show that the repeated reuse-fence
  predicates are absent at the claimed sites.
- A boolean temporary with the same conditional branches is `CODEGEN_NOOP`.
- Any missing or moved fence is `INFEASIBLE_FOR_THIS_ROUTE`; any code-size,
  register, spill, or UB increase is evidence against the hypothesis.

Shape evidence and minimal probe: `D=6144` FP32 and BF16 with observed
`localRows=1` and `localRows>=2`; FP16 is a control instantiation. `D=8192`
with `localRows=1` is streamed-only and cannot prove the cached case. The
future probe must first compare source/executable identity, emitted CFG, and
fence trace; no timing is authorized by this record.

Duplicate boundary: this is not parameter residency, address formation,
selective dispatch, reduction, or tile-count specialization. Splitting the
coefficient bodies, adding a shape/dtype gate, changing a fence, or changing
the generic loop trip count changes another axis and is rejected.

### HBH-10 - Wide pass-1 slot-selection state

- `ID=HBH-10`
- `BRANCH_KIND=WIDE_PASS1_SLOT_SELECT`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

Refine HBH-07 to pass 1 only. Replace the repeated `useA`-based selection of
the existing two local x/residual slots and their `rd`/`rel` event IDs with a
runtime current/next slot state. This is a representation of the same
two-slot state, not hand-written alternating loop bodies and not a rolling GM
address cursor.

Invariant evidence:

- `u`, `batchRow`, `tile`, `col`, `rowOffset`, `nRow`, `nTile`, `nCol`,
  `nRowOffset`, `valid`, and `nValid` remain parent-equivalent.
- The `u==0` initial load, the `u+1<units` next-load guard, both x/residual
  loads, the `SetFlag`/`WaitFlag` pairs, and the final pending-slot waits stay
  in the same order.
- `reduceLocal[batchRow * reduceStride + tile]`, all `ReduceSum` calls, and
  the FP16/BF16 arithmetic paths are untouched.

Static codegen gate:

- The emitted pass-1 CFG must show removal of the repeated parity ternaries
  at lines 3134-3139 without adding indirect local-tensor or event lookup.
- It must retain the same DMA/event trace and register/UB footprint. If the
  compiler lowers the state back to parity selection, classify `CODEGEN_NOOP`.
- Alternating manually duplicated bodies, loop peeling, or fixed-trip forms
  are `DUPLICATE_REJECTED` under `TILECOUNT-STATIC-UNROLL`.

Shape evidence and minimal probe: FP16 `D=16384`, FP16 `D=32768`, BF16
`D=32768`, plus one runtime-tail width. Record `wideFullYRows_`, runtime
`tileWidth`, `tileCount`, `batchRows`, and `units`; do not infer them from D.
Pass-2, host dispatch, and all addresses are outside this hypothesis.

### HBH-11 - Wide pass-2 parameter-slot selection state

- `ID=HBH-11`
- `BRANCH_KIND=WIDE_PASS2_PARAM_SLOT_SELECT`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

Refine HBH-08 to pass 2 only. Represent the existing two gamma/bias staging
slots and `prd`/`prel` events with a runtime current/next state rather than
reselecting them from `tile & 1` at every tile. The initial tile load and the
existing next-tile guard remain intact.

Invariant evidence:

- `tile`, `col`, `nCol`, `valid`, `nValid`, all gamma/bias GM offsets, and all
  output store offsets remain identical.
- `prd0/prd1`, `prel0/prel1`, `pRelA/pRelB`, all waits/releases, and the
  per-row `Muls`/`Mul`/`Add`/`FromFloat` order remain identical.
- `SyncVToMTE2`, `SyncVToMTE3`, `SyncMTE3ToV`, and the store order at lines
  3338-3344 remain unchanged. No V017-style MTE3 wait movement is allowed.

Static codegen gate:

- The emitted pass-2 CFG must remove the repeated parity selection at
  lines 3272-3276 without introducing indirect tensor/event selection,
  additional waits, or event movement.
- A source cursor that emits the same parity branch is `CODEGEN_NOOP`; a
  cursor that changes event lifetime is `INFEASIBLE_FOR_THIS_ROUTE`.
- Fixed tile bodies, parity-specialized unrolling, and first/last peeling are
  `DUPLICATE_REJECTED` under `TILECOUNT-STATIC-UNROLL`.

Shape evidence and minimal probe: FP16 `D=16384` as the primary multi-tile
control, FP16 `D=32768`, BF16 `D=32768` with its adaptive tile, and one
runtime-tail width. Include both `batchRows=1` and a valid `batchRows>1`
case where the unchanged chooser permits it. Pass 1, reduction, and host
dispatch are outside this hypothesis.

### HBH-12 - Wide pass-1 terminal lookahead guard

- `ID=HBH-12`
- `BRANCH_KIND=WIDE_PASS1_LOOKAHEAD_TERMINAL_GUARD`
- `MATURITY=NEEDS_MORE_EVIDENCE`
- `PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

Refine the active-wide terminal-control search to the pass-1 `u+1 < units`
decision at line 3146 only. Investigate a dynamic terminal-row/terminal-unit
control form that makes the next-unit address/load block explicit while
preserving the current-unit body. This is not permission to remove the guard,
preload a different unit, or change the pipeline schedule.

Invariant evidence:

- Exactly `units-1` next-unit load pairs occur, with the same
  `nRow`/`nTile` decode, `nCol`, `nRowOffset`, `nValid`, slot, event, and
  address values.
- The terminal unit performs no next-unit load or flag, and the current unit
  still waits on the same `rd` before vector work.
- `ReduceSum` placement, `relA/relB` ownership, final waits, event release,
  and all arithmetic remain byte/order equivalent.

Static codegen gate:

- The only acceptable codegen change is removal or hoisting of the terminal
  decision while preserving the ordered load/event trace.
- A split that fixes `units`, `tileCount`, or offsets at compile time is
  `DUPLICATE_REJECTED`; a split that moves the next load or changes overlap is
  `INFEASIBLE_FOR_THIS_ROUTE`.
- Because the branch is close to loop-shape ownership, an unchanged emitted
  branch is `CODEGEN_NOOP` and does not justify a Revision.

Shape evidence and minimal probe: FP16 `D=16384` with at least two units,
FP16 `D=32768`, BF16 `D=32768`, and one tail-bearing runtime width. Record
`batchRows`, `tileCount`, `units`, and the terminal unit explicitly. A
single-unit control is useful only as a non-trigger control, not as evidence
that the guard was removed.

## Cross-route duplicate audit

The audit uses the committed route records and V011 source visible in this
worktree. It does not read sibling private contexts or worktrees. `SELECTIVE-
FASTPATH` and `TILECOUNT-STATIC-UNROLL` are compared through their committed
handoff/audit records (`4d8e073e` and `0d062c51`).

| Comparison target | Result for HBH-09..12 | Boundary and evidence |
|---|---|---|
| `SELECTIVE-FASTPATH` | `DUPLICATE=NO` | Selective fastpath assigns a complete alternate donor/path under a new shape/dtype gate. HBH-09..12 keep the same V011 call path and runtime loop; no new target, gate, operation subset, or donor is allowed. |
| `ADDR` | `DUPLICATE=NO`; HBH-12 is address-adjacent | ADDR owns row-base/cursor/index/address expressions. HBH-10/11 only represent local slot/event selection. HBH-09 is generic fence control. HBH-12 must preserve every `nRowOffset+nCol` and `nValid`; any address rewrite becomes ADDR and is rejected. |
| `TILECOUNT-STATIC-UNROLL` | `DUPLICATE=NO` only for dynamic state; otherwise `DUPLICATE_REJECTED` | Runtime `tileCount`, `tileWidth`, loop trips, parity, offsets, and terminal guards stay dynamic. Fixed trips, loop peeling, alternating bodies, static offsets, or propagated D constants are owned by TILECOUNT. |
| `REDUCE` | `DUPLICATE=NO` | `ReduceSum`, reduce slots, `reduceStride`, scalar handoffs, and RMS arithmetic are frozen. Any reduction topology, `GetValue`, V/S handoff, or reduction destination change is REDUCE-owned. |
| `R005` | `DUPLICATE=NO` | R005/FULL-R005 changes tile size/iteration geometry. These hypotheses observe the runtime tile geometry and never change `kTileElems`, `wideFullYTileElems_`, or chooser outputs. |
| `R012` | `DUPLICATE=NO` | R012/SCHED owns 32B row grouping and row ownership. No block assignment, row group, alignment policy, or cross-row DMA change is permitted here. |
| `R028` | `DUPLICATE=NO` | R028 owns scalar synchronization/GetValue reduction. HBH-09 preserves fences; HBH-10/11 preserve all V/S and MTE event operations; none batches or removes scalar handoffs. |
| `R029` | `DUPLICATE=NO` | R029 is an outer five-mode tiling taxonomy. These hypotheses do not add dispatch modes or change V011's existing early returns/wide-path test. |
| `R31A` | `DUPLICATE=NO`, sync changes rejected | R31A V028 removes PipeBarriers on a different V026 batch path. HBH-09..12 do not remove barriers or transplant that donor; any such change is outside BRANCH_KIND. |
| `R31B` | `DUPLICATE=NO`, pipeline changes rejected | V011 is the frozen parent. V006/V009/V010/V011 changed pipeline/buffer scheduling; V017 moves MTE3 completion waits. HBH-10/11 preserve V011 event lifetime and HBH-12 preserves load order, so no pipeline donor is being reintroduced. |
| `MIX` | `DUPLICATE=NO`, sync/alternate donor changes rejected | MIX-A changes donor/path selection, localRows gates, and synchronization. No MIX donor, path gate, or `SyncVToMTE2`/MTE3 change is included in HBH-09..12. |
| `Wave-1` | `DUPLICATE=NO`; calibration context only | Wave-1 records local-positive/Official-negative cases and candidate-side measurement bias. It supplies no branch implementation to reuse. No local percentage, Official score, or calibration claim is used as hypothesis evidence. |

### Explicit rejected forms

- New call target, shape gate, dtype gate, or complete alternate implementation:
  `SELECTIVE-FASTPATH` duplicate.
- Fixed `D`, fixed `tileCount`, unrolled parity bodies, first/last loop peel,
  or constant offset propagation: `TILECOUNT-STATIC-UNROLL` duplicate.
- Rolling GM cursor, row-base/index decode rewrite, or changed tail byte
  count: `ADDR` duplicate.
- Changed ReduceSum, GetValue, V/S handoff, pipe barrier, DMA issue order,
  event lifetime, row ownership, or tile width: outside this route's axis.

## Review packet

| ID | One BRANCH_KIND axis | Primary source sites | Required invariant proof | Main risk |
|---|---|---|---|---|
| HBH-09 | `GENERIC_REUSE_FENCE_GUARD` | Generic lines 389-395, 464-490 | Same fence count/order and same operands | Body duplication/code size |
| HBH-10 | `WIDE_PASS1_SLOT_SELECT` | Wide lines 3134-3174 | Same x/r bytes, addresses, events, reduction slots | Indirect slot lookup/spills |
| HBH-11 | `WIDE_PASS2_SLOT_SELECT` | Wide lines 3272-3297, 3338-3351 | Same parameter bytes, stores, events, waits | Event lifetime or register cost |
| HBH-12 | `WIDE_PASS1_LOOKAHEAD_TERMINAL_GUARD` | Wide lines 3146-3174 | Exactly one next load for each non-terminal unit | Loop split becomes TILECOUNT/schedule change |

No maturity is above `NEEDS_MORE_EVIDENCE`. Before any future implementation,
Planning/Review must select at most one hypothesis and require emitted-code
inspection as a gate. No selection is made here:

```text
MAIN_SELECTED=NONE
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
REVISION_CREATED=NO
BUILD_OR_CODEGEN_EXECUTED=NO
CORRECTNESS_OR_TIMING_EXECUTED=NO
ONLINE_EXECUTED=NO
```
