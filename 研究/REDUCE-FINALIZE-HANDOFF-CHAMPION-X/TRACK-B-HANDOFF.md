# TRACK-B HANDOFF — REDUCE-FINALIZE-HANDOFF-CHAMPION-X

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize
BRANCH=w2/m2/reduce-finalize
START_HEAD=ed860e392d7604694ac6664da60aff1fc1f4c04f
TRACK=TRACK-B
MODE=RESEARCH_ONLY
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
ROUTE_HYPOTHESIS_POOL_STATE=ROUTE_HYPOTHESIS_POOL_EXHAUSTED
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
REMOTE_FRESHNESS=UNKNOWN_FOR_BRANCH_W2_M2_REDUCE_FINALIZE
```

## 1. Scope and evidence boundary

This is a read-only research handoff. No kernel/source/build/revision,
server, correctness, timing, profiling, or Online work was performed. The
only proposed axis is the already-produced reduction partials → final scalar
reduction → scalar extraction → invRms → first consumer edge.

The active low-precision wide path dispatches from `Process()` to
`ProcessWideLowPrecision` for non-FP32 types when `widePath_` is true
(`线上结果/R31B/V011/submission.asc:58-99,160-169,3076-3077`). The earlier
`ComputeWideLowPrecisionInvRmsPipelined` helper is not the active route for
this path and is not the evidence basis for this handoff.

In the active `ProcessWideLowPrecision` path:

1. Each (row, tile) unit emits the existing `ReduceSum` at `3188-3190`
   (FP16) or `3201-3203` (BF16), then sets its `V_MTE2` release flag at
   `3205`.
2. Ping-pong reuse waits at `3162-3169` protect subsequent input-buffer
   reloads; they are explicitly out of scope for relocation.
3. Conditional terminal waits are at `3212-3217`; event IDs are released at
   `3218-3221`.
4. Per-row final reduction, first `GetValue`, `meanSquare`, `Duplicate`/`Sqrt`,
   second `GetValue`, and reciprocal are at `3224-3241`.
5. The existing `SyncVToMTE2()` is at `3243`, before pass 2. First scalar
   `Muls` consumers are at `3320` (FP16) and `3330` (BF16).

This leaves one narrow *research question*: can the two conditional terminal
waits remain present but move after the complete per-row finalizer loop, with
their IDs kept live until both waits finish, then be released before the
existing `SyncVToMTE2()` and pass-2 buffer reuse? The finalizer uses
`reduceFp32Buf_`, `xFp32Buf_`, and `residualFp32Buf_`, rather than the
`xBuf_`/`residualBuf_` input slots whose release events are pending. This is
only a static independence observation, not a safety proof; event lifetime,
aliasing, and cross-pipe ordering require review before any separately
approved experiment. The existing `SyncVToMTE2()` may also subsume the waits,
making relocation neutral.

Future target classes, if Planning separately authorizes an experiment, are
FP16/BF16 rows dispatched to this wide path (`rowWidth > kCacheElems`) with
`tileCount > 1`, such as D=16384/32768 where present in the champion shape
set. Observe one-row and row-batched forms separately; use the smallest
reachable multi-tile wide shape as a short-window/control class. These are
probe classes, not asserted Official case mappings or measurements.

## 2. Old mechanism versus new difference

`OLD_MECHANISM`: in active `ProcessWideLowPrecision`, the parent waits for
outstanding `rel0`/`rel1` terminal `V_MTE2` events and releases their IDs
before starting the per-row final reduction and invRms scalar/vector tail.

`NEW_DIFFERENCE`: keep the same event IDs, `SetFlag` calls, wait count, input
buffer ownership, `ReduceSum` calls, partial count, precision, tile geometry,
pass count, and row mapping; extend the event-ID scope across the finalizer,
then move only the two conditional terminal waits (and their ID releases) to
after the finalizer loop but before `SyncVToMTE2()` and pass-2 loads. The
expected variable is wait placement at the finalizer edge, not reduction
topology or scalar-sync deletion.

## 3. Candidate screening

The table includes every plausible class requested by the route brief. Only
RFH-1 survives as independent. Rejected entries are recorded to prevent a
future agent from relabeling an old mechanism as new.

### RFH-1 — DEFER-TERMINAL-INPUT-RELEASE-WAITS

`STATUS=INDEPENDENT_RESEARCH_LEAD`

- **OLD_MECHANISM:** the active pass-1 scope waits on outstanding `rel0` /
  `rel1` `V_MTE2` release flags and releases those IDs before the final
  partial reduction and invRms scalar/vector tail.
- **NEW_DIFFERENCE:** retain the same flags, waits, IDs, reductions, and
  scalar operations, but keep the IDs live and move only those terminal waits
  and releases to after the finalizer and before pass-2 buffer reuse.
- **Mechanism:** in active `ProcessWideLowPrecision`, preserve both terminal
  `V_MTE2` waits and their IDs, but run the existing per-row final `ReduceSum`
  and invRms tail before those waits. Extend the enclosing scope so the IDs
  remain live, then wait and release them before existing
  `SyncVToMTE2()`/pass-2 loads. Do not alter in-loop reuse waits or move any
  `SetFlag`.
- **Bottleneck:** two terminal V→MTE2 waits currently precede the finalizer
  loop. If a release handshake is still outstanding, moving the waits could
  overlap its completion with the independent final reduction and scalar/
  vector tail; if already complete or dominated by the later
  `SyncVToMTE2()`, the change is likely neutral.
- **Parent/source SHA:** R31B V011, Official 45.16,
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** FP16/BF16 shapes dispatched to active wide path
  with `tileCount > 1`, including D=16384/32768 where present; measure
  one-row and row-batched forms separately. Small paths that do not dispatch
  to `ProcessWideLowPrecision` are external negative controls, not the same
  code-path control.
- **Why it may help:** removes a serialized terminal wait from the critical
  path without deleting synchronization. The final reduction does not read
  `firstX`/`secondX` or their residual input slots, and the invRms scratch is
  distinct from those input slots in the parent function.
- **Why it may fail:** the event may already be complete, the existing
  `SyncVToMTE2()` may dominate it, or the compiler/runtime may serialize the
  work anyway. The scalar tail may be too short. IDs must not be released or
  reused before their waits, and all shapes must prove that no path reloads
  `xBuf_`/`residualBuf_` before the deferred waits.
- **UB/DMA/sync/precision:** UB sizes and aliases unchanged; DMA addresses,
  byte counts, tile count and queue depth unchanged; synchronization is
  preserved, only its terminal edge moves. FP32 accumulation and invRms
  arithmetic are unchanged. The correctness risk is event/ID lifetime and
  buffer reuse ordering, not numerical precision.
- **Duplicate check:** distinct from R028 scalar-sync deletion because no
  `SyncVToS`, `SyncSToV`, or `GetValue` is removed. Distinct from ASYNC V004
  prologue prefetch because no parameter load is moved. Distinct from R31B
  V017 because this is the pass-1 input-release edge, not MTE3 store drain.
- **Related old routes:** R31B V006/V009 queue-depth changes are untouched;
  MIX-A V003/V007 changed other V_MTE2 synchronization instances; R31A V021
  deferred an MTE3→V store wait, not this V→MTE2 release wait.
- **Minimal experiment:** after separate Planning approval, make one OFAT
  source diff in `ProcessWideLowPrecision`: widen scope for the four event
  IDs, relocate only the terminal conditional waits and matching releases,
  and keep them before existing `SyncVToMTE2()`. First complete static
  event/alias/lifetime review; then correctness and paired parent/candidate
  timing on target classes. No experiment is authorized by this handoff.
- **Focus axis:** `handoff edge`.
- **Expected evidence:** source diff showing only terminal wait relocation;
  event-ID/flag audit; byte-identical reduction and math region; correctness
  equality; same-binary qualification; paired device-event deltas separated
  by multi-tile versus control paths. A null or control-identical result is
  useful evidence that this edge is not exposed.

### RFH-2 — SCALAR-FIRST-INVRMS-FINALIZER

`STATUS=DUPLICATE_REJECTED`

- **OLD_MECHANISM:** the active path uses the existing scalar extraction and
  V/S synchronization around `meanSquare`, `Sqrt`, and the second scalar
  extraction before storing `invRmsValues`.
- **NEW_DIFFERENCE screened:** calculate the tail directly from the extracted
  scalar, removing the vector materialization/return. That is a scalar-sync
  and invRms arithmetic change, not a terminal input-release wait placement.
- **Mechanism:** calculate mean-square, square root, and scalar reciprocal
  directly from the existing extracted square sum, avoiding the vector
  `Duplicate`/`Sqrt`/second `GetValue` materialization.
- **Bottleneck:** the second V→S/S→V scalar/vector handoff and scalar tail.
- **Parent/source SHA:** R31B V011, Official 45.16,
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** active wide FP16/BF16 rows with `tileCount > 1`,
  where the finalizer at lines 3224-3241 runs; any shape is only a path probe,
  not an asserted Official case mapping. FP32 and non-wide paths are not
  retained as targets for this handoff.
- **Why it may help/fail:** it could remove a short vector/scalar round trip,
  but it changes the scalar/vector synchronization and potentially the
  numerical instruction path; the parent may already have the lowest-cost
  supported sequence.
- **UB/DMA/sync/precision impacts:** no DMA change, but it deletes/reorders
  scalar/vector synchronization and changes the precision/operation path,
  violating this route's frozen precision and handoff-only boundary.
- **Duplicate check:** this is R028's scalar-sync reduction and R019's
  invRms arithmetic family; VECTOR-MATH-X V001/V003 already moved or fused
  denominator-tail work. It is not a new handoff edge. **Reject.**
- **Related old routes:** R028 scalar-sync reduction and R019 invRms
  arithmetic cover this class; VECTOR-MATH-X V001/V003 already moved or
  fused denominator-tail work. **DUPLICATE_REJECTED.**
- **Minimal experiment:** none under this route. An owning scalar-math route
  would need a scalar-only single-change candidate and a precision check
  before timing.
- **Focus axis:** `handoff edge` (screened; not retained).
- **Expected evidence:** scalar instruction/sync diff, ULP/error versus the
  parent, and shape-qualified device-event evidence; none is claimed here.

### RFH-3 — FIRST-CONSUMER-NO-SCALAR-VECTOR-HANDOFF

`STATUS=DUPLICATE_REJECTED`

- **OLD_MECHANISM:** the active path stores each row's scalar `invRmsValues`
  result and the first output tile consumes it through scalar `Muls` at line
  3320 (FP16) or 3330 (BF16).
- **NEW_DIFFERENCE screened:** keep `invRms` in a vector and broadcast it to
  the first value tile, deleting or bypassing the scalar/vector handoff. This
  changes the scalar-sync/materialization mechanism and belongs to the
  VECTOR-MATH/R028 family, not this final wait edge.
- **Mechanism:** retain invRms in a one-element vector and broadcast it into
  the first value tile, replacing the scalar `Muls` consumer and its scalar
  return path.
- **Bottleneck:** the invRms-to-first-consumer V/S handoff.
- **Parent/source SHA:** R31B V011, Official 45.16,
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** the first consumer in active wide FP16/BF16
  multi-tile paths, especially D=16384/32768 where present; Official mapping
  is not inferred from these probe classes.
- **Why it may help/fail:** it could eliminate a round trip, but needs a
  vector broadcast/materialization and has already shown alignment/primitive
  feasibility and precision sensitivity in the vector-math experiments.
- **UB/DMA/sync/precision impacts:** no DMA or row mapping change, but it
  changes scalar/vector synchronization, vector materialization, and possibly
  precision/UB lifetime.
- **Duplicate check:** VECTOR-MATH-X VM-H3/V002 and SEQ-FUSE-2 V003 cover the
  vector denominator/broadcast consumer; R028 covers scalar handoff removal.
  This is explicitly a covered scalar/vector handoff mechanism. **Reject.**
- **Related old routes:** VECTOR-MATH-X V002/V003 and R028; R31A V024's
  invRms scale placement is an adjacent consumer-side mechanism.
- **Minimal experiment:** none under this route; if re-opened by Planning,
  the owning VECTOR-MATH route would need one broadcast-form-only diff.
- **Focus axis:** `handoff edge` (screened; not retained).
- **Expected evidence:** handoff count and UB lifetime comparison, numerical
  equality, then shape-qualified timing; no result is claimed here.

### RFH-4 — HOIST-RETAINED-ROW-INVRMS-SCALE

`STATUS=DUPLICATE_REJECTED`

- **Mechanism:** on the active BF16 wide path, where
  `ProcessWideLowPrecision` retains the complete `y` row in FP32, hoist the
  existing `Muls(y, y, invRms)` from each affine tile to one full-row scale
  after that row's invRms becomes available. FP16 is not a safe equivalent
  without a wider full-row representation or a changed rounding path.
- **Bottleneck:** repeated scalar-to-vector scale materialization and its
  per-tile vector/barrier issue in the first-consumer region.
- **Parent/source SHA:** R31B V011, Official 45.16,
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** BF16 wide multi-tile rows where the complete FP32
  `y` row is retained; no FP16 variant is retained by this screening.
- **Why it may help/fail:** a full-row Muls might reduce tile-loop issue and
  barrier overhead, but does not reduce elementwise work; UB bandwidth or
  long-vector limits may erase the benefit.
- **UB/DMA/sync/precision impacts:** BF16's retained `y` is already FP32, so
  the row storage could be reused without new DMA/allocation while preserving
  `(y * invRms) * gamma + bias`; FP16 would need full-row FP32 storage or a
  changed rounding path. Reduction and scalar extraction are unchanged.
- **Duplicate check / related old routes:** R31A V024 already hoisted the
  same invRms row-scale Muls out of a cached-row affine tile loop and kept the
  same expression order. A dtype-specific backing-store port is not a new
  mechanism. VECTOR-MATH-X and R028 cover adjacent scalar-to-vector consumer
  reduction. **DUPLICATE_REJECTED.**
- **Minimal experiment:** none under this route: R31A V024 already isolates
  the one-change row-scale placement; this BF16 form is a dtype-port of that
  mechanism, while the FP16 form would violate the frozen precision/storage
  constraints.
- **Focus axis:** `handoff edge` (screened; not retained).
- **Expected evidence:** source-level Muls/barrier count, exact output and
  operation-order comparison, then shape-qualified timing; no result is
  claimed here.

## 4. Mandatory duplicate audit

| Audit object | Old mechanism | New proposed difference | Result |
|---|---|---|---|
| R007 | Native `ReduceSum` primitive/topology | RFH-1 leaves every `ReduceSum`, width, partial count and final collapse unchanged | PASS: independent edge |
| R011 | Manual vector reduction/tree replacement | RFH-1 does not replace or reorganize reduction | PASS: independent edge |
| R028 | Scalar synchronization/GetValue reduction, including batched scalars | RFH-1 retains all scalar sync/GetValue operations; moves only terminal V→MTE2 waits | PASS for RFH-1; RFH-2/RFH-4 rejected |
| R029 | Legacy five-mode dispatch/taxonomy; FULL-R029 wide cached-row specialization | RFH-1 changes no dispatch predicate, mode selection, cache/residency policy, tile policy, or scalar handoff | PASS for RFH-1; same-family variants rejected |
| REDUCE-HIER-X | Eager fold, manual tree, short-span, binned and WholeReduceSum reductions | RFH-1 preserves reduction tree, topology and partial count | PASS for RFH-1 |
| VECTOR-MATH-X | Vector denominator, broadcast, and SEQ-FUSE-2 tail placement | RFH-1 preserves invRms arithmetic and first consumer | PASS for RFH-1; RFH-3 rejected |
| ASYNC-OVERLAP V001–V004 / ASYNC-TRIPLE | V001 prologue param prefetch; V002 W2 next-row input issue reorder with deferred MTE3→V wait; V003 W4 GetValue-tail convergence; V004 W1 FullCache prologue prefetch; ASYNC-TRIPLE studies MTE2/V/MTE3 scheduling | RFH-1 is a terminal pass-1 V→MTE2 input-release wait edge; no prefetch, scalar-sync, queue, MTE3, or buffer-depth change | PASS: distinct event/edge, but preserve the explicit boundary |
| HOTLOOP-ADDR-HOIST-CHAMPION-X (current ADDR lane) | Row-base hoist, rolling input-address cursor, or stateful row/tile index counters | RFH-1 changes no GM address expression, row/tile decoder, cursor, or index state; only terminal event-wait placement is screened | PASS: orthogonal |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X (current BRANCH lane) | Hoist coefficient-mode, row-prefetch-terminal, or per-tile valid-length branch decisions | RFH-1 changes no branch predicate, valid-length computation, prefetch, or mode body | PASS: orthogonal |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X (current TILECOUNT lane) | Fixed N=2 loop unrolling/static offset and tile-counter specialization | RFH-1 preserves runtime `tileCount`, loop structure, offsets, and parity; no unroll or tile-count change | PASS: orthogonal |
| MULTIROW-DMA-CHAMPION-X (current M2-5 lane) | Stride/multi-row DMA shape, burst grouping, and related UB/DMA ownership | RFH-1 preserves DMA count/bytes, input slots, tile geometry, row mapping, and two-buffer protocol | PASS: orthogonal |
| R31B V006/V009 | MTE3/MTE2 queue depth changes | RFH-1 keeps queue depth and event count | PASS |
| R31B V017 | Deferred MTE3→V store drain | RFH-1 moves V→MTE2 input release waits after finalizer; different event direction and buffer | PASS |
| R31A V021 | Deferred MTE3 completion wait until staging reuse | RFH-1 is not a store wait and does not alter output staging | PASS |
| R31A V024/V028 | InvRms scale placement / affine barriers | RFH-1 changes neither affine arithmetic nor output barriers | PASS |
| MIX-A V003/V007 | Dispatch restriction and a different V_MTE2 removal | RFH-1 keeps dispatch and all waits; only terminal wait placement is studied | PASS |
| FULL/R029 and wide cached routes | Wide/cache/tile/residency policy | RFH-1 changes none of tile, cache, UB sizing, or row policy | PASS |

The mandatory audit therefore finds one new hypothesis and three rejected
scalar-tail variants. Any variant that changes the reduction tree/topology,
partial count, or scalar-sync mechanism is `DUPLICATE_REJECTED` for this
route, even if it is phrased as an earlier invRms calculation or a first
consumer optimization.

The R001–R029 idea-pool index was audited as a complete route matrix. The
relevant historical families are R001 (two-pass structure), R006/R007/R011
(reduction organization), R013 (pipeline), R019/R020 (invRms math), R028
(scalar sync), and R029 (dispatch/cache taxonomy); the remaining entries
cover different copy, dtype, scheduling, tiling, or process axes. The retained
FULL-R007, FULL-R011, and FULL-R028 result records are runtime-error results,
not positive evidence for retrying their covered mechanisms. This historical
failure status does not make a new variant of those same axes independent.

The current Wave-2 ADDR, BRANCH, and TILECOUNT handoffs were also audited
from their route branches. They remain independent lanes because RFH-1 does
not alter address/index expressions, branch predicates, valid/tail-length
code, static tile-count specialization, or loop offsets. Conversely, any RFH
restatement that changes one of those surfaces is a duplicate of the named
lane and must be `DUPLICATE_REJECTED`, not relabeled as a finalizer-edge
hypothesis.

## 5. Route boundary and handoff to Planning

```text
IN_SCOPE=research-only analysis of the active ProcessWideLowPrecision
         partial-reduction -> final scalar -> invRms -> first-consumer edge
ALLOWED_RESEARCH_VARIABLE=terminal rel0/rel1 V_MTE2 wait placement only
FROZEN=ReduceSum replacement/tree/topology/partial count; precision; tile
       width/count; UB allocation; DMA shape/count/bytes; row mapping; pass
       count; scalar GetValue/sync mechanism; first-consumer arithmetic
FORBIDDEN=kernel/source/build edits; Revision; build/correctness/timing/
          profiling/server3/Online work; shared ledger/schedule edits;
          lifecycle decisions; route selection; implementation
DECISION_OWNER=Planning/Review selects any future candidate; Main only reports
               this handoff; MAIN_SELECTED=NONE
```

The candidate pool has fewer than three independent survivors. Therefore:

```text
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES
INDEPENDENT_SURVIVORS=1
DUPLICATE_REJECTED=3
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
```

RFH-1 is a research lead only, not a recommendation to create a Revision.
Planning/Review must decide whether this single edge is worth a future
approved experiment. No route lifecycle decision is made here.

## 6. Provenance and operational note

- Direct parent fact: `技术路线/冠军/champions.tsv` and
  `线上结果/R31B/V011/{result.json,source-meta.json,submission.asc}`.
- Reduction evidence: `研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES*.md` and
  `本地实验/REDUCE-HIER-X/V001..V005/`.
- Scalar/vector evidence: `研究/VECTOR-MATH-X/TRACK-B-HYPOTHESES*.md`,
  `研究/VECTOR-MATH-X/SEQ-FUSE-2-SPEC.md`, and
  `本地实验/VECTOR-MATH-X/V001..V003/`.
- Async evidence: `研究/ASYNC-TRIPLE-X/next-hypotheses.md`,
  `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/`, and
  `技术路线/全版本记录.tsv` rows for ASYNC V001–V004.
- Historical duplicate evidence: `归档/历史控制文件/idea-pool-29-routes.md`,
  `归档/phase3-before-reset-20260920/管理/路线状态/R007.json`,
  `R011.json`, `R028.json`, `R029.json`, and the corresponding FULL result
  records.
- Current Wave-2 lane handoffs were read from the committed route refs:
  `w2/m2/hotloop-addr` at `1e605148`, `w2/m2/hotloop-branch` at
  `01641fed`, and `w2/m2/tilecount-unroll` at `429d7ef7`. They are cited for
  duplicate screening only; no other worktree or branch was edited.
- One `git fetch --prune origin` attempt completed during bootstrap (single
  attempt; no explicit timeout wrapper was recorded). It updated an unrelated
  visible remote ref (`origin/main1/champion-exploit`); no route-specific
  remote ref for `w2/m2/reduce-finalize` was confirmed. Remote freshness for
  this branch is therefore recorded as UNKNOWN.
- The task assigns this handoff as MAIN-2/M2-5, while the local campaign file
  still labels M2-5 as MULTIROW-DMA-CHAMPION-X. This discrepancy is recorded
  only; no shared schedule or lifecycle state was changed.
```
