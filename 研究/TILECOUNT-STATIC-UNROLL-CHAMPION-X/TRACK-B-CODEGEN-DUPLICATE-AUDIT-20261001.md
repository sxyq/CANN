# TCSU Track-B Codegen and Duplicate Audit

```text
DATE_UTC=2026-10-01
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
MAIN=MAIN-2
TRACK=TRACK-B-ONLY
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
PROFILING=NOT_RUN
ONLINE=NOT_RUN
```

This is a read-only source, codegen-gate, and cross-route duplicate audit. The
reviewed parent evidence is `线上结果/R31B/V011/submission.asc`, its SHA
sidecar, `source-meta.json`, `result.json`, and the existing TCSU handoff.
The ADDR and BRANCH comparisons use their committed Track-B handoffs; REDUCE
uses the REDUCE-HIER-X route brief and approval records. No compiler,
assembler, server3, NPU, runner, timing, profiling, or Online command was
run. No source/build/wrapper file, Revision, shared ledger, or control file
was changed.

## Parent path and fixed-N proof

The parent source has the following static facts:

| Fact | Evidence | Result |
|---|---|---|
| Exact FP32 path | `Process()` first checks `widePath_` (`submission.asc:163-170`); `Init()` sets it only when `rowWidth > kCacheElems` (`:60-61`). The candidate domain uses `rowWidth=8192`, so it reaches the non-wide dispatch. | PASS for the stated domain |
| Path predicate | FP32 dispatch calls `ProcessFp32FullRowOutputPipelined()` only for `rowWidth == kCacheElems && localRows > 1` (`:177-180`). | PASS for the stated domain |
| Tile constants | `kTileElems=4096` (`:1242`) and `kCacheElems=8192` (`:1288`) are compile-time constants. | `8192 / 4096 = N=2` |
| Launch instance | For `[128,8192]`, `availableCoreNum=32`, the wrapper clamps `blockCount` to 32 (`:3532-3540`); `baseRows=4`, `extraRows=0`, hence every block has `localRows=4`. | PASS; `localRows > 1` |
| H1 loop | Parameter preload at `:1129-1132` takes `col={0,4096}` and issues two gamma plus two bias loads. | Exactly 2 iterations |
| H2 loop | First pass at `:1164-1180` takes `col={0,4096}`; it retains the row-dependent first-tile load condition and one `ReduceSum` per tile. | Exactly 2 iterations |
| H3 loop | Output pass at `:1203-1225` takes `col={0,4096}`; `useFirst` selects the two existing event pairs and stores at the two tile offsets. | Exactly 2 iterations |

The row loop (`:1145-1226`) and `localRows` remain runtime-dependent. The
proof is therefore only for the three inner tile loops, and does not justify
unrolling rows, changing launch ownership, or specializing another path.

## Parent-codegen gate

The source constants prove the source-level trip count, not the generated
instruction stream. The retained `线上结果/R31B/V011/` evidence contains no
parent assembly, disassembly, compiler codegen dump, register report, or
compile log for this function. The parent source itself is not evidence that
the compiler emitted a loop backedge.

| Candidate | Required parent evidence | Gate conclusion |
|---|---|---|
| TCSU-H1 parameter preload | Exact-parent codegen must show whether the two gamma/bias tile pairs still have a loop backedge or equivalent dynamic offset/index work. Candidate codegen must show offsets 0 and 4096 with the same four loads and order. | `CODEGEN_GATE=UNVERIFIED`; do not implement or attribute a gain until this evidence exists. |
| TCSU-H2 first pass | Exact-parent codegen must show whether the two tile bodies retain loop control/address calculation. Candidate must preserve the first-row/preloaded-tile condition, two input-load opportunities, two `ReduceSum` calls, both `SyncMTE2ToV`/`SyncVToMTE2` boundaries, and order. | `CODEGEN_GATE=UNVERIFIED`; do not implement or attribute a gain until this evidence exists. |
| TCSU-H3 output pass | Exact-parent codegen must show whether the two output bodies retain a backedge or runtime tile/event selection. Candidate must preserve two stores at offsets 0/4096 and the existing V-MTE3/MTE3-V event set/order. | `CODEGEN_GATE=UNVERIFIED`; do not implement or attribute a gain until this evidence exists. |

`UNVERIFIED` is not evidence that the parent is unrolled, and it is not a
performance failure. It means the mandatory first gate cannot be closed in
this Track-B task because build and codegen execution are forbidden. A future
selected experiment must use identical compiler options for parent and child,
then reject the hypothesis as a no-op if the parent already has equivalent
straight-line code or if the child produces no relevant codegen delta.

## Cross-route duplicate boundary

The routes share the frozen R31B V011 parent, but they own different
mechanisms. The boundary is the changed expression/control/topology, not the
shared function name or the fact that a loop is present.

| Route | Owned mechanism | TCSU disposition |
|---|---|---|
| `HOTLOOP-ADDR-HOIST-CHAMPION-X` | Dynamic FP16/BF16 `ProcessWideLowPrecision` address/index strength reduction: row-base hoist, rolling input cursor, or stateful `(batchRow,tile)` decode. It preserves runtime `tileWidth`/`tileCount`, the flattened loop, valid lengths, and DMA/event order. | TCSU-H1/H2/H3 are on the separate FP32 `ProcessFp32FullRowOutputPipelined` path and replace only its proven N=2 tile-loop control with fixed offsets. `CROSS_ROUTE_DUPLICATE=NO` under that path boundary. Applying TCSU to the runtime wide-LP loop, or changing TCSU into a rolling cursor/index decoder, is `DUPLICATE_REJECTED` / ADDR-owned. |
| `HOTLOOP-BRANCH-HOIST-CHAMPION-X` | Dynamic control placement: generic coefficient-mode guard, generic terminal-row prefetch guard, and runtime final-tile valid-length hoist. Its rejected HBH-04/HBH-05 explicitly send first-tile peeling, fixed tile-count/parity specialization, loop-branch removal, and static offset propagation to TCSU. | TCSU owns the fixed-trip N=2 specialization and is not a duplicate of HBH-01..03, which target different dynamic guards/paths. `CROSS_ROUTE_DUPLICATE=NO` with an explicit ownership handoff. Any TCSU diff that also hoists row/tail guards, changes `valid`, splits rows, or changes generic control is `DUPLICATE_REJECTED` / OFAT failure. |
| `REDUCE-HIER-X` / REDUCE family | RMS square-sum reduction topology, partial-sum lifetime, `ReduceSum` span/primitive, merge tree, or V/S handoff count. REDUCE records forbid unrelated scheduling, DMA, wide-path, epilogue, and gamma-cache changes. | TCSU-H2 may textually contain `ReduceSum` inside the loop, but it must keep the same per-tile calls, reduction destination/order, final reduction, and all V/S and MTE synchronization. TCSU changes only loop control for N=2. `CROSS_ROUTE_DUPLICATE=NO`; replacing `ReduceSum`, changing span/merge topology, removing barriers, or changing reduction workspace is `DUPLICATE_REJECTED` / REDUCE-owned. |

Additional hard boundary: changing `kTileElems`, `kCacheElems`, tile count,
UB allocation, dispatch predicates, DMA counts/bytes, event topology, or
arithmetic/precision order is not static unrolling. It is outside TCSU and
must not be smuggled into a selected one-loop experiment.

## Candidate verdicts

| Candidate | Source fixed-N proof | Parent codegen gate | Duplicate audit | Track-B disposition |
|---|---|---|---|---|
| TCSU-H1 | `PASS` for FP32 D=8192 path; two parameter-load iterations | `UNVERIFIED` | `NO` versus ADDR/BRANCH/REDUCE under the stated path and OFAT boundary | `NEEDS_MORE_EVIDENCE`; no implementation |
| TCSU-H2 | `PASS` for FP32 D=8192 path; two first-pass tile iterations | `UNVERIFIED` | `NO` if reductions/syncs remain identical; otherwise REDUCE duplicate | `NEEDS_MORE_EVIDENCE`; no implementation |
| TCSU-H3 | `PASS` for FP32 D=8192 path; two output iterations | `UNVERIFIED` | `NO` if event/store order remains identical; otherwise branch/schedule overlap | `NEEDS_MORE_EVIDENCE`; no implementation |

```text
FIXED_N_SOURCE_PROOF=PASS (N=2 only; exact FP32 D=8192 path)
PARENT_CODEGEN_GATE=UNVERIFIED_FOR_H1_H2_H3
CROSS_ROUTE_DUPLICATE=NONE_WITH_EXPLICIT_SCOPE_GUARDS
MAIN_SELECTED=NONE
REVISION_CREATED=NO
```

No route lifecycle decision is made here. Planning/Review must select at most
one hypothesis before any Track-A work, and that selection still cannot bypass
the parent-codegen gate.
