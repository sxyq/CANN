# TRACK-B FOLLOW-UP — RFH-1 EVENT-EDGE FEASIBILITY AUDIT

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
TRACK=TRACK-B
DATE=2026-10-01
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
RFH-1=DEFER-TERMINAL-INPUT-RELEASE-WAITS
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
```

## Scope and method

This is a static follow-up to `TRACK-B-HANDOFF.md`. It compares the exact
retained R31B V011 source with the retained ASYNC V001 and R31B V017 source
blobs, and reads the committed ADDR, BRANCH, and TILECOUNT handoffs by their
recorded commits. No source, build, runner, Revision, server3, correctness,
timing, profiling, Online, or shared-control operation was performed.

## RFH-1 event-edge feasibility

The active low-precision wide path is `ProcessWideLowPrecision()` in V011
(`线上结果/R31B/V011/submission.asc:3076`). Its pass-1 event/lifetime map is:

| Surface | V011 fact | RFH-1 boundary |
|---|---|---|
| Ping-pong reuse | `rel0`/`rel1` are waited before the next load reuses the corresponding `xBase`/`rBase` slot (`:3161-3174`) | Frozen; these in-loop waits must not move |
| Event production | Each unit sets its `V_MTE2` release flag after its `ReduceSum` (`:3205-3210`) | Frozen; no `SetFlag` moves or disappears |
| Terminal waits | Remaining `relA`/`relB` flags are waited at `:3212-3217`, then all four pass-1 IDs are released at `:3218-3221` | Only these two conditional waits and the matching `rel0`/`rel1` releases are candidates for deferral |
| Finalizer | Per-row final `ReduceSum`, `GetValue`, mean-square, `Sqrt`, second `GetValue`, and reciprocal run at `:3224-3241` | Existing operations and order remain unchanged |
| Pass-2 handoff | Existing `SyncVToMTE2()` is at `:3243`; pass 2 then maps `xBuf_`/`residualBuf_` to `gBase`/`bBase` at `:3256-3257` | Deferred waits must complete before this existing handoff and before pass-2 loads |

The source allocates `xBuf_`/`residualBuf_` separately from
`xFp32Buf_`/`residualFp32Buf_`/`reduceFp32Buf_` in the wide low-precision
initialization (`:86-111`). The finalizer reads the FP32/reduction buffers,
not the pass-1 input slots whose reuse is protected by `rel0`/`rel1`. This
supports a conditional static feasibility result: the terminal input-release
waits can be considered for placement after the finalizer, provided their
event IDs remain live and the waits/releases occur before the existing
`SyncVToMTE2()` and pass-2 input-slot reuse.

This is not a safety proof. The event API's exact ordering and ID-lifetime
semantics still require a separately approved source experiment or platform
review. In particular, the finalizer must not acquire or alias the pending
input slots, and no pass-2 event ID may be allocated before the deferred
`rel0`/`rel1` waits and releases complete. The existing `SyncVToMTE2()` is
retained; RFH-1 does not delete or replace it.

## Duplicate-boundary audit

### ADDR

The committed ADDR handoff changes only address/index expression form in the
existing flattened pass-1 stream: row-base hoisting, a rolling GM input
cursor, or stateful `(batchRow,tile)` decode. V011's changed surfaces are the
`batchRow`/`tile` decode and `rowOffset + col`/next-unit offset expressions at
`:3125-3174`. RFH-1 changes none of those expressions, counters, valid-length
calculation, or GM offsets; it acts only after the pass-1 unit loop at the
terminal event edge. The shared loop boundary is adjacent code, not the same
mechanism. `DUPLICATE_CHECK=PASS`.

### BRANCH

The committed BRANCH handoff covers the generic coefficient-mode guard, the
generic next-row prefetch terminal guard, and active-wide per-unit tail-length
hoisting. RFH-1 leaves the generic path untouched (`:284-490`) and preserves
the active-wide `valid`, `u + 1 < units`, and `useA`/`nextA` control at
`:3125-3174`. RFH-1 relocates the existing `if (relA)`/`if (relB)` terminal
wait bodies; it does not hoist or rewrite their predicates, the tail branch,
or the prefetch branch. `DUPLICATE_CHECK=PASS`.

### TILECOUNT

The committed TILECOUNT handoff is gated to the FP32
`ProcessFp32FullRowOutputPipelined()` N=2 path (`:1122-1238`). RFH-1 is
restricted to the non-FP32 `ProcessWideLowPrecision()` path at `:3076`, whose
runtime `tileCount` and flattened `units = batchRows * tileCount` remain at
`:3091-3094` and `:3122-3125`. No tile width, tile count, loop bound, parity,
offset, or static unroll changes. `DUPLICATE_CHECK=PASS`.

## Historical event-edge comparisons

### ASYNC-OVERLAP V001

The exact V011-to-ASYNC V001 diff adds pass-2 parameter event allocation and
the first gamma/bias loads before the invRms finalizer (`ASYNC V001
`:3224-3245), then removes the old `SyncVToMTE2()` at that boundary. It does
not move the pass-1 `rel0`/`rel1` terminal waits or releases. RFH-1 keeps the
pass-2 parameter schedule and the existing `SyncVToMTE2()` unchanged, and
studies only the pass-1 `V_MTE2` input-release terminal edge. This is a
different event instance, pipeline stage, and synchronization direction in
the source. `DUPLICATE_CHECK=PASS`.

### R31B V017

The exact V011-to-V017 diff adds one `MTE3_V` `storeRelease` event in pass 2,
waits for it before reusing the single `outputBuf_` staging tile, sets it
after each store, and drains/releases it at the block tail (V017
`:3263-3388`). It also changes the wide tile constants, which is outside
RFH-1's frozen V011 geometry. V017 does not alter the V011 pass-1 `rel0`/
`rel1` `V_MTE2` waits at `:3215-3224`. Therefore the actual differences are:

| Dimension | RFH-1 | R31B V017 |
|---|---|---|
| Event direction | `V_MTE2` | `MTE3_V` |
| Stage | pass-1 input-slot release | pass-2 output-store completion |
| Protected storage | `xBuf_` / `residualBuf_` reuse | `outputBuf_` staging reuse |
| Change | Move existing terminal waits/releases after finalizer | Defer store drain until output staging reuse; add store event |
| Geometry | Frozen at V011 | V017 also changes wide tile constants |

`DUPLICATE_CHECK=PASS`; V017 is a distinct store-drain mechanism, not an
existing RFH-1 implementation.

## Closed conclusion

```text
STATIC_EVENT_EDGE_FEASIBILITY=CONDITIONALLY_FEASIBLE
EVENT_LIFETIME_REVIEW=REQUIRES_SEPARATE_APPROVED_EXPERIMENT_OR_PLATFORM_REVIEW
DUPLICATE_AUDIT_ADDR=PASS
DUPLICATE_AUDIT_BRANCH=PASS
DUPLICATE_AUDIT_TILECOUNT=PASS
DUPLICATE_AUDIT_ASYNC_V001=PASS
DUPLICATE_AUDIT_R31B_V017=PASS
INDEPENDENT_SURVIVOR_COUNT=1
RFH-1_STATUS=RESEARCH_LEAD_ONLY
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
```

No selection, lifecycle decision, performance expectation, or Online
recommendation is made by this audit. Planning/Review remains the owner of
any future experiment decision.
