# TILECOUNT-STATIC-UNROLL-CHAMPION-X — Planning handoff

```text
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
IMPLEMENTATION=NO
REVISION=NONE
ONLINE=NO
POOL_STATE=THREE_ADMISSIBLE_CANDIDATES_CODEGEN_GATED
```

## Shared boundary

All candidates are restricted to the FP32
`ProcessFp32FullRowOutputPipelined()` N=2 path in the R31B V011 source
(`:1122–1238`), qualified by `rowWidth=8192 && localRows>1`.  The concrete
probe is FP32 `[128,8192]`, `availableCoreNum=32`, `blockCount=32`,
`localRows=4`.  `kTileElems=4096`, `kCacheElems=8192`, UB geometry, DMA
counts/bytes, event order, reductions, arithmetic order, and dispatch remain
frozen.

## Candidate pool for Planning selection

| ID | Mechanism / exact site | Target | Expected effect | Main risk | Duplicate status |
|---|---|---|---|---|---|
| TCSU-H1 | Unroll only the parameter-preload loop; two gamma/bias tile loads at offsets 0/4096 | FP32 `[128,8192]`, blockCount=32, localRows=4 | Remove a two-trip setup backedge and dynamic tile offset work | One-time setup is tiny; parent may already be straight-line | PASS conditionally; distinct from parameter residency/prefetch routes |
| TCSU-H2 | Unroll only the first-pass two-tile loop; preserve first-tile prefetch, both ReduceSum calls, sync pairs, and next-row prefetch | Same | Remove loop control/offset work around input and square reduction | DMA/vector/sync dominate; register pressure; reduction duplicate if changed | PASS only if reductions/syncs remain byte/order equivalent |
| TCSU-H3 | Unroll only the output two-tile loop; preserve event IDs, waits, stores, drain order and `Muls→Mul→Add` | Same | Fold constant tile identity and event selection | Store/event work dominates; code size; store-policy duplicate if changed | PASS only if store/event protocol is unchanged |

## Mandatory gate before implementation

The retained repository has no exact V011 parent disassembly/IR/register
report.  Planning must select at most one candidate, and the first Track-A
action must be an identical-options parent codegen comparison.  If the parent
already emits equivalent straight-line N=2 code, the selected candidate is a
no-op and must not become a Revision.  No Main selection is inferred from the
unranked order above.

```text
PLANNING_ACTION=SELECT_AT_MOST_ONE_TCSU-H1/H2/H3
CODEGEN_GATE=REQUIRED
NEXT_AFTER_SELECTION=parent-codegen evidence, then one OFAT Revision only
```
