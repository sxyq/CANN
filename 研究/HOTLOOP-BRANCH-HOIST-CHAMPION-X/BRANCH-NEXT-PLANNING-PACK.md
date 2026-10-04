# HOTLOOP-BRANCH-HOIST-CHAMPION-X — next planning handoff

## Scope and decision state

- DATE: 2026-10-04
- ROUTE: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- REVIEW: own-route planning pack plus V001/V002 declaration, source diff,
  correctness, and local-result evidence
- TRACK: `B` / read-only research handoff
- UNUSED_SOURCE_LEVEL_SURVIVOR: `HBH-11`
- SURVIVOR_STATUS: `CONDITIONAL_SURVIVOR_FOR_PLANNING_REVIEW`
- MAIN_SELECTED: `NONE`
- REVISION_CREATED: `NO`
- IMPLEMENTATION_AUTHORIZED: `NO`
- DIRECT_PARENT_IF_SELECTED: `R31B V011`
- PARENT_OFFICIAL: `45.16`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

This file does not select HBH-11 or authorize a revision. It records the one
unused survivor already named in `BRANCH-PLANNING-PACK-20261002.md`; no new
hypothesis is invented. Planning must decide whether its pass-2 site is
conceptually distinct enough from the failed V001 cursor experiment to merit
selection.

## V001/V002 evidence reviewed

| Revision | Hypothesis and parent | Existing evidence | Consequence for this handoff |
|---|---|---|---|
| V001 | HBH-10; direct parent `R31B V011` | Final verdict `LOCAL_REJECTED`, BF16 D32768. Two valid windows had mixed paired deltas; window 2 did not reproduce a stable gain. Candidate source SHA `4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b`. | Closed; do not use V001 as parent. The tested pass-1 rolling slot-handle/event cursor did not establish a gain. |
| V002 | HBH-09; direct parent `R31B V011` | Build PASS; correctness PASS (12 parent/candidate cases, D6144 FP32/BF16/FP16, localRows 1/2). Three-run engineering score: `-0.048802%`, `POOR`, mixed directions; prior formal paired verdict is `MEASUREMENT_BLOCKED`. Candidate source SHA `1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65`. | Complete but not accepted; do not use V002 as parent or claim a stable gain. |

The recorded V001/V002 lineages are siblings from the official-backed V011
source. Any future HBH-11 revision, if Planning selects it, must also be a
sibling from the exact V011 source—not a continuation of V001 or V002.

Reviewed route records: `BRANCH-PLANNING-PACK-20261002.md`,
`PARENT-COMPILER-STATIC-EVIDENCE-20261002.md`, V001 `REVISION-DECLARATION.md`,
`diff.patch`, and `LOCAL-VERDICT-V001-FINAL.md`; V002
`REVISION-DECLARATION.md`, `diff.patch`, `correctness-result.json`,
`local-result.json`, and `LOCAL-SCORE-AUDIT-20261003.md`.

## Candidate survivor: HBH-11

- `ID`: `HBH-11`
- `BRANCH_KIND`: `WIDE_PASS2_PARAM_SLOT_SELECT`
- `SINGLE_HYPOTHESIS`: replace only pass-2 runtime parity-based current/next
  gamma/bias and event-slot selection with an equivalent rolling two-slot
  state representation.
- `EXACT_FUNCTION`: `AddRmsNormBiasKernel<T>::ProcessWideLowPrecision()`.
- `EXACT_CODE_SITE`: parent `submission.asc` pass-2 tile loop, lines 3267–3352:
  - current slot predicate and gamma/bias/`prd`/`prel` selections: 3272–3276;
  - next-slot selection and existing prefetch guard/load/flag sites: 3278–3297;
  - current-slot release ownership update: 3346–3351.
- `BOTTLENECK`: repeated runtime parity/slot-selection control once per
  parameter tile. For `K=tileCount`, current selection/release executes `K`
  times per batch and next-slot selection executes `K-1` times; selected
  parameters are consumed by the existing inner `batchRow` loop.
- `TARGET_DTYPES`: FP16 and BF16 only, through the existing non-float wide
  path (`rowWidth > 8192`).
- `TARGET_SHAPES`: use existing wide-path probes such as D=16384 and D=32768;
  include a tail-bearing accepted-domain shape only after recording the
  unchanged runtime `tileWidth`, `tileCount`, and `batchRows`. These are probes,
  not new dispatch buckets. FP32, generic/specialized paths, and new host
  dispatch are out of scope.
- `WHY_IT_MAY_HELP`: a rolling state may avoid repeated parity-derived
  tensor/event handle selection in the pass-2 parameter pipeline.
- `WHY_IT_MAY_FAIL`: the compiler may already lower selections efficiently;
  cursor exchanges may become equivalent moves, indirect selection, spills,
  or added register pressure. The pass-1 analogue in V001 was rejected without
  credible gain, which materially lowers confidence in this mechanism family.

### Single-variable boundary

Only represent the existing pass-2 A/B gamma/bias slots and their existing
`prd`/`prel` event IDs as current/next state. Keep unchanged:

- pass-1 and generic `Process()` code, host dispatch, dtype/path thresholds,
  and all row/tile geometry;
- `tile`, `tileCount`, `col`, `valid`, `nCol`, and `nValid` calculations,
  including the terminal no-prefetch case and partial-tile byte counts;
- gamma/bias GM addresses and bytes, load count/order, initial prefetch,
  `WaitFlag`/`SetFlag`/release ownership, and event allocation/lifetime;
- `batchRow` iteration, arithmetic, conversion, store offsets, and the order of
  `SyncVToMTE2`, `SyncVToMTE3`, and `SyncMTE3ToV`.

No loop peeling/unrolling, fixed tile counts, tile-width change, parameter
traffic reduction, buffer/UB change, synchronization movement, arithmetic
change, or combined HBH-10 pass-1 cursor is part of this hypothesis. If a
correct implementation cannot preserve these invariants while removing the
targeted parity selections, classify it `INFEASIBLE` rather than broadening the
change.

## Duplicate and novelty audit

| Comparison | Disposition | Boundary / caveat |
|---|---|---|
| V001 / HBH-10 | `DISTINCT_CODE_SITE; SAME_MECHANISM_FAMILY; PLANNING_NOVELTY_GATE_REQUIRED` | V001 carried x/residual slots and events in pass 1; HBH-11 concerns gamma/bias parameter slots in pass 2. They are separable OFAT sites, as the existing pack states, but both use parity-to-rolling-slot-state representation. V001's `LOCAL_REJECTED` result means HBH-11 must not be treated as automatically novel or promising. |
| V002 / HBH-09 | `NO_DIRECT_DUPLICATE` | HBH-09 factors generic-path reuse-fence control; HBH-11 is in the wide low-precision pass-2 parameter pipeline. |
| ADDR | `NO`, provided all indices/addresses stay identical | No address cursor, index decode, or offset change. |
| UB / REDUCE / selective fastpath | `NO`, provided buffers, reduction, and selected call path stay identical | Any such change leaves this hypothesis boundary and is not admissible. |
| TILECOUNT static-unroll | `NO` for a runtime rolling state only | Fixed-trip specialization, peeling, alternating hand-written bodies, or static offsets are duplicate/rejected forms. |
| Parameter-residency / DMA scheduling | `NO` only if traffic and issue schedule stay unchanged | HBH-11 must not reduce global-to-UB bytes, retain parameters across rows, aggregate DMA, or move prefetch/waits. |

Overall duplicate classification is **conditional**: the code site and operands
are distinct from V001, while the optimization motif is closely related.
Planning review is required to determine whether that site-level distinction
meets the campaign's independent-hypothesis bar after V001's negative result.

## Evidence gaps and bounded gates if selected

`PARENT-COMPILER-STATIC-EVIDENCE-20261002.md` records that the exact V011
parent compiled, but no usable device IR or source-correlated AICore
disassembly was available. Therefore HBH-11 remains `COMPILER_STATUS=UNVERIFIED`:
there is no evidence yet that the device binary retains the target control or
that a rolling representation removes it.

If and only if Planning selects HBH-11:

1. Declare one sibling revision from exact `R31B V011` with the parent SHA
   above; do not inherit V001 or V002.
2. Before treating a source change as a performance candidate, inspect the
   available emitted code for FP16/BF16 and require evidence that the targeted
   parity selection was actually removed without indirect lookup, spills, or
   event/data-order changes. If not demonstrated, classify `CODEGEN_NOOP` or
   `INFEASIBLE` and do not claim performance.
3. Only after authorized implementation, build, exact-source correctness for
   full and tail tiles, and identity checks may a separately authorized local
   performance stage be considered. No experiment was run for this handoff.

## Handoff

`PLANNING_ACTION_REQUIRED`: decide whether HBH-11's pass-2 site is sufficiently
independent from the rejected HBH-10 cursor motif. Until that decision and an
explicit selection, `MAIN_SELECTED=NONE`; no revision, implementation,
build/correctness/timing, NPU work, Dashboard/shared-ledger update, or Online
action is authorized by this document.
