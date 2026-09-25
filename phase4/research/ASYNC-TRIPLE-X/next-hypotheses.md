# ASYNC-TRIPLE-X — Next-Hypothesis Research (TRACK-B)

> **MAIN-2 APPROVALS 2026-09-25** — Probe shape change APPROVED (measurement design only, not a Revision): old primary probe `8×1024` (width=1024, tileCount=1, Candidate≡SEED instruction stream) is retired as primary performance probe; new probe MUST use width>1024 with tileCount≥2, preferably tileCount≥3 — candidates from 4096 / 6144 / 8192 (e.g. 8×4096 tileCount=4, or rows×8192 tileCount=8). APPROVED NEXT backlog: H1 inter-pass prologue prefetch — implementable only after Main disposes the current triple-overlap Candidate; no queue-depth or extra-buffer changes alongside it.

Route: ASYNC-TRIPLE-X · Worktree `cann-next6/ASYNC-TRIPLE-X` · Branch `exp/next6-async-triple-x` · OWNER=MAIN-2
Mode: Long-Horizon Parallel Exploration (execution-contract section R). TRACK-A candidate untouched this turn.

## CURRENT_CANDIDATE

```text
REVISION            V001
SOURCE_SHA256       2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c (FROZEN)
DIRECT_PARENT       ASYNC-TRIPLE-X-SEED (R013-derived two-stage pipeline), SHA f20da79c…
PARENT_SOURCE_SHA   f20da79c7086483c7cf0bddad630bdea92a219992a5f061c9fd3a6b8edbc3572
DONOR               R013 DOUBLE-BUFFER PIPELINE (MTE2 prefetch N+1 + Vector N; MTE3 not a stage)
SINGLE_HYPOTHESIS   Only the output-phase MTE3 schedule: store of tile N-1 issued before
                    Vector compute of tile N, MTE2 prefetch of N+1 stays issue-ahead
                    => true triple overlap MTE2(N+1) / V(N) / MTE3(N-1).
CONFIRMED PATH      MTE2 N+1 / Vector N / MTE3 N-1 — PASS (L006 PATH_CONFIRMATION)
CORRECTNESS         PASS (max_abs <= 3.79e-07 standard 8x1024; <= 3.28e-07 wide 2x8192)
DECISION            NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED (WINDOW_UNQUALIFIED 2/2)
                    IDLE — no device runs, no kernel edits, no V002 this turn.
```

Kernel shape facts (read-only from `submission.asc`):
- Two passes per row: pass-1 accumulates sum-of-squares (MTE2 prefetch N+1 / AccumulateTile N), then `FinishRms()` scalar, then pass-2 re-reads x+residual+gamma+bias and does MTE2(N+1) / MTE3(N-1) / V(N).
- Queues are `TQue<VECIN/VECOUT, 2>` (depth 2) driven by EnQue/DeQue only. No explicit `SetFlag`/`WaitFlag` in this kernel.
- `kTileLength = 1024` fixed. Row-parallel core mapping `row += blockNum`.
- Probe shapes: standard rows=8 width=1024 (blocks=8) and wide rows=2 width=8192 (blocks=2).

## CURRENT_BLOCKER

- Timing blocked, not architecture: WINDOW_UNQUALIFIED 2/2 (d6+d4 FAIL); protocol says stop timing today; SHA frozen; no V002 without Main `NEXT_HYPOTHESIS`/`PROMOTEE`.
- Signal on record is LOAD_CONTAMINATED: seed CV 35.6%, median delta -21.5 us inside a 75.9 us noise margin. 5/6 pairs favor V001 but not separable from noise.
- **Shape inertness finding (this turn):** at width=1024 and `kTileLength=1024`, tileCount=1. In pass-2 the guards `tile+1<tileCount` and `tile>=1` are both false, so V001 executes the *same* instruction stream as SEED (prefetch-tile0 -> ComputeOutputTile(0) -> CopyOut(0)). On the standard probe shape the triple-overlap mechanism is therefore **structurally inert**; any measured delta there is pure noise. Triple overlap only engages when width>1024 (tileCount>=2). This must shape all future P/C (see RECOMMENDED_NEXT).

## BOTTLENECK_MODEL

Where a multi-tile row actually spends time, and which units are on the critical path:

```text
per row = pass1 [prologue fill -> steady (MTE2 N+1 / V N) -> epilogue] 
             -> FinishRms (vector sqrt/div + scalar GetValue: FULL pipe drain)
          -> pass2 [prologue fill tile0 -> steady (MTE2 N+1 / MTE3 N-1 / V N) -> last-tile flush]
```

1. **Inter-pass drain (once per row):** `FinishRms()` blocks on `GetValue(0)`; Vector and both DMA units drain. Pass-2 tile-0 MTE2 is issued only *after* this drain returns.
2. **First-tile fill (twice per row):** pass-1 and pass-2 each open with a cold prologue where tile 0 is prefetched then computed with no store/prefetch partner yet.
3. **Last-tile flush (once per row):** the final `CopyOut` is issued after the pass-2 loop with nothing after it in that row. On both probe shapes blocks==rows, so each core owns exactly **one** row — the tail store has no cross-row partner and is fully exposed.
4. **MTE2 / MTE3 HBM contention (pass-2 steady state):** pass-2 issues a 4-copy load (x+res+gamma+bias) and a store in the same iteration; both hit HBM concurrently. Pass-2 also *re-reads* the whole row (2x MTE2 traffic overall).
5. **Scalar issue pacing:** the scalar thread serializes issue and blocks on `xQueue_.DeQue()` (wait MTE2(N)) inside `ComputeOutputTile`, and on `outputQueue_.DeQue()` (wait V(N-1)) inside `CopyOut`. Whether the output-ring `FreeTensor` after the async `DataCopyPad` also couples MTE3 completion into the scalar path decides if store latency is hidden or exposed.
6. **large-D vs small-D:** benefit grows with tileCount. tileCount<=1 -> inert; tileCount=2 -> one overlap event; tileCount>=4 -> steady state dominates and scheduling matters.

**Explicitly EXCLUDED as the next hypothesis (evidence, not assumption):** "add one more buffer" / raise queue depth.
- R31B V006 "MTE3 queue depth 2" -> 43.91, **T14 16670 still flat**.
- R31B V009 "MTE2 queue depth 2" -> 43.81, **T14 16424 still flat; "MTE2 queue not T14 driver"**.
- idea-pool R013: "R31 tried MTE2 depth and MTE3 depth separately; both flat on T14."
- Current design already runs depth-2 rings on all four queues and already forms MTE2/V/MTE3 triple overlap. UB headroom exists (~88 KB used of ~192 KB on 910B) but depth was historically flat, so a buffer add is a duplicate of a PROVEN-flat mechanism. Every hypothesis below locates a **different** mechanism (sync placement / issue order / loop structure).

---

## HYPOTHESIS-1 — Inter-pass prologue prefetch overlap (first-tile fill + issue placement)

```text
MECHANISM
  Reorder the pass-2 prologue so the tile-0 MTE2 prefetch (`CopyInOutputData(row,0,...)`)
  is ISSUED BEFORE `FinishRms()` runs, instead of after it. Pass-2 tile-0 read does not
  depend on `inverseRms` (only its ComputeOutputTile does), so MTE2 for pass-2 tile 0 can
  run concurrently with the FinishRms vector sqrt/div + scalar GetValue drain. The loop's
  first `DeQue(x[0])` then waits on an already-in-flight load instead of starting it cold.

BOTTLENECK
  Inter-pass drain + first-tile fill. Today FinishRms fully drains the pipe, returns the
  scalar, and only then does pass-2 begin filling — so MTE2 tile-0 latency and the
  scalar drain are serialized instead of overlapped.

EXPECTED_SHAPES
  Helps every multi-tile shape (tileCount>=1, because pass-2 always has a prologue).
  Largest relative gain on small/medium tileCount where the prologue is a bigger fraction;
  still positive on wide (tileCount=8) because the drain is once per row. No effect on
  pass-1 (its tile-0 prefetch is already first). Works even at tileCount=1, unlike V001.

WHY_IT_MAY_HELP
  Removes one serialized (scalar-drain + MTE2-fill) segment per row. On single-row-per-core
  probe shapes there is no cross-row cover, so this drain is otherwise fully exposed.

WHY_IT_MAY_FAIL
  FinishRms is only ~8-element vector work + one GetValue — may be shorter than the tile-0
  4-copy load, so only the drain portion (not the load) is hidden. Compiler/runtime may
  already hoist the prefetch. If MTE2 tile-0 is not on the critical path, delta ~ 0.

ASCEND_FEASIBILITY
  High. Pure scalar issue reorder inside ProcessRow; uses the existing TQue EnQue/DeQue.
  No new API, no event semantics change, no ABI change. Single conceptual edit (OFAT).

UB/CORE/DMA_IMPACT
  UB: none (no new/ resized buffers). Core: unchanged row-parallel mapping. DMA: same total
  bytes; only the start time of the pass-2 tile-0 MTE2 load moves earlier.

SYNC_IMPACT
  Relies on the existing DeQue(x[0]) to hold compute until the earlier-issued load lands. Must keep
  FinishRms's scalar `GetValue` result out of the prefetch path (it already is). Verify no
  read-after-write hazard on the param/x queues across the reorder.

PRECISION_RISK
  None. Same reads, same order of arithmetic, same inverseRms. Byte-identical output.

DUPLICATE_CHECK
  vs R013 donor: donor has no MTE3 stage and no such reorder. vs V001: V001 only added the
  MTE3 stage; it did not move the prologue. vs idea-pool R001/R002/R019: reduction/invRms
  structure untouched. vs active routes (SCHED row-group, ALIGN tail, REDUCE invscale,
  BATCH residency, UB-LIVENESS, DTYPE-SPECIAL): none owns the pass-boundary issue order.
  NOT a buffer/queue-depth change.

MINIMAL_OFAT_DIFF
  Move the single `CopyInOutputData(row, 0, ...)` call from after `FinishRms()` to before
  it; nothing else changes. One conceptual change => SINGLE_CHANGE_AUDIT eligible.

EXPECTED_LOCAL_PROBES
  P/C vs frozen V001 (and vs SEED) on a MULTI-TILE shape only: width>=2048 (tileCount>=2),
  ideally width=8192 (tileCount=8), rows>blocks avoided so the drain is not cross-row
  covered. Device-event primary, warmup>=10, interleaved, in-process. Expect a small
  positive shift (low-us) on wide; confirm not inside same-binary noise floor.
```

**Classification: READY_FOR_MAIN_REVIEW** — clean single-edit mechanism, feasible, no precision risk; needs a multi-tile measurement window to confirm.

---

## HYPOTHESIS-2 — Output-ring store-latency decoupling (store latency hiding + event placement)

```text
MECHANISM
  Replace the pass-2 output ring's implicit TQue coupling with explicit inter-pipe events
  so an MTE3 store of tile N-1 completes fully asynchronously and only the *specific* UB
  slot's next reuse waits on it. Concretely: keep depth-2 output slots, but sync with
  `SetFlag/WaitFlag<HardEvent::V_MTE3>` (Vector done -> MTE3 may read) and
  `WaitFlag<MTE3 done>` per-slot before that slot is re-allocated for the next
  ComputeOutputTile, instead of letting `outputQueue_.DeQue/FreeTensor` serialize store
  completion into the scalar issue path. Goal: hide MTE3 store latency behind V(N).

BOTTLENECK
  Store latency hiding. If the TQue output-ring FreeTensor after the async DataCopyPad
  forces the scalar (or the next AllocTensor) to wait for the in-flight store to drain,
  then every steady-state iteration leaks MTE3 latency into the critical path even though
  the store is nominally "issued before compute."

EXPECTED_SHAPES
  Only multi-tile (tileCount>=2) engages pass-2 steady state; effect grows with tileCount.
  Largest on wide/large-D where stores are long and frequent (tileCount=8+). Zero on
  tileCount=1 (inert path).

WHY_IT_MAY_HELP
  If store completion currently blocks slot recycling, explicit per-slot events let V(N)
  and MTE3(N-1) overlap cleanly, converting an exposed store into a hidden one without
  adding a buffer.

WHY_IT_MAY_FAIL
  AscendC TQue may already keep the store async and only waits on the *next* V write, in
  which case there is no exposed latency and delta ~ 0 (this is the main unknown -> hence
  NEEDS_MORE_EVIDENCE). Event overhead (extra flags per tile) could offset any gain on
  short tiles.

ASCEND_FEASIBILITY
  Medium-high. `SetFlag/WaitFlag<HardEvent::V_MTE3>` is already proven in this codebase:
  MODE-X submission uses `SetFlag<HardEvent::V_MTE3>`, EPI-X uses `MTE2_V / V_MTE3 /
  MTE3_V / V_S / S_V`, MIX-A V003 used a "V_MTE2 release". So the event API and pattern are
  established on Ascend 910B. Requires converting one queue to manual slot bookkeeping.

UB/CORE/DMA_IMPACT
  UB: unchanged (still depth-2 output slots; ~8 KB). Core: unchanged. DMA: identical store
  bytes; only their completion visibility changes.

SYNC_IMPACT
  Highest of all hypotheses. Replaces implicit TQue output sync with explicit flags; must
  get event-id direction right (V_MTE3 before store; store-done before slot reuse) or risk
  a read/write race on the reused slot. Correctness proof required before any timing.

PRECISION_RISK
  None if sync is correct (same data, same order). Incorrect event direction -> potential
  race producing wrong output (a correctness, not precision, risk).

DUPLICATE_CHECK
  vs "add one more buffer": NOT a depth increase — same 2 slots, different sync placement
  (this is the point of the hard rule). vs R31B V006/V009 queue-depth-2 (flat): different
  mechanism (events, not depth). vs UB-LIVENESS-X (UB lifetime/alias): that route owns UB
  liveness/aliasing generally; this is a specific output-ring *event* schedule — verify at
  Main review that UB-LIVENESS is not claiming inter-pipe event placement. vs MIX-A V003
  (V_MTE2 release): that released MTE2, this manages MTE3 store completion.

MINIMAL_OFAT_DIFF
  Convert only the output queue to explicit V_MTE3/store-done events; leave x/residual/
  param queues on TQue. Single conceptual change (sync placement).

EXPECTED_LOCAL_PROBES
  Multi-tile width>=2048, device-event primary. First run a same-binary correctness precheck
  (byte compare vs V001) because a sync-direction bug would corrupt output; only then P/C.
  If store latency is already hidden, delta ~0 -> LOCAL_REJECTED / NEEDS_MORE_EVIDENCE.
```

**Classification: NEEDS_MORE_EVIDENCE** — hinges on whether the current TQue output ring actually exposes MTE3 completion to the scalar path. Resolve by an AscendC queue-semantics check + a same-binary correctness precheck before spending a device window.

---

## HYPOTHESIS-3 — Pass-2 MTE2/MTE3 issue-order arbitration (HBM contention)

```text
MECHANISM
  In the pass-2 steady state each iteration currently issues the 4-copy MTE2 load of tile
  N+1 (x, residual, gamma, bias) FIRST, then the MTE3 store of tile N-1. Flip the issue
  order so the shorter MTE3 store of tile N-1 is issued BEFORE the heavier 4-copy MTE2 load
  of tile N+1, giving the store a clean HBM window ahead of the load burst. Single reorder
  of two issue calls inside the pass-2 loop.

BOTTLENECK
  MTE2/MTE3 HBM contention. Pass-2 drives a 4-tile-element load burst and a store in the
  same iteration against the same HBM; the load currently wins the issue order every
  iteration, so the store queues behind it and its completion (which limits the next slot
  reuse and the row tail) is lengthened.

EXPECTED_SHAPES
  Only multi-tile (tileCount>=2). Effect grows with tileCount and with tile bytes (large-D
  wide tiles) where each iteration's load/store collision is biggest. ~0 at tileCount=1
  (inert) and small on narrow tiles.

WHY_IT_MAY_HELP
  Issue order can bias which unit's command reaches HBM arbitration first. Issuing the
  single store before the 4-copy load may shorten store completion, tightening both the
  steady-state slot recycle and the exposed last-tile flush.

WHY_IT_MAY_FAIL
  MTE2 and MTE3 are separate hardware units and HBM arbitration may be largely independent
  of scalar issue order — then delta ~ 0. Delaying the load could also starve the Vector
  unit (which is fed by the load), trading store time for compute starvation = net loss.

ASCEND_FEASIBILITY
  Trivial: swap two existing calls (`CopyOut(prev)` before `CopyInOutputData(next)`).
  Uses existing TQue. No API, no event, no UB change.

UB/CORE/DMA_IMPACT
  UB: none. Core: unchanged. DMA: identical bytes, only the command issue sequence changes;
  no change to arbitration guarantees.

SYNC_IMPACT
  Must preserve correctness: `CopyOut(N-1)` DeQue waits on V(N-1) (already done) and
  `CopyInOutputData(N+1)` needs a free slot (freed at end of ComputeOutputTile(N-1)).
  Reordering is safe because both prerequisites are met by iteration N-1. Confirm no
  dependency of the store on the load (there is none — different addresses).

PRECISION_RISK
  None (same data, same arithmetic, only issue timing changes).

DUPLICATE_CHECK
  vs H2 (event placement): H2 changes the *sync primitive*, H3 changes only the *order* of
  two issues on the existing TQue — distinct. vs R013 / V001: neither reorders MTE2 vs MTE3.
  vs idea-pool R015 multi-row DMA / R016 core mapping: untouched. vs active routes: none
  owns intra-iteration MTE2-vs-MTE3 ordering. NOT a buffer/depth change.

MINIMAL_OFAT_DIFF
  Move the `if (tile>=1) CopyOut(...)` block above the `if (tile+1<tileCount)
  CopyInOutputData(...)` block in the pass-2 loop; nothing else. One conceptual change.

EXPECTED_LOCAL_PROBES
  Multi-tile width>=2048 (prefer 8192), device-event primary, interleaved vs frozen V001.
  Because the effect may be small or reversed, run same-binary floor first; require the
  delta to exceed the same-binary MAD/median before crediting the mechanism.
```

**Classification: NEEDS_MORE_EVIDENCE** — feasibility trivial but the physical effect (does scalar issue order actually steer HBM arbitration on 910B?) is unproven. Cheap to test; may be flat like the historical depth changes.

---

# OPTIONAL-HYPOTHESIS-4 — Inter-row pipeline continuity (cover inter-pass drain + tail)

```text
MECHANISM
  Today Process() runs rows strictly sequentially: row R does pass1 -> FinishRms drain ->
  pass2 -> last-tile flush, then row R+1 starts cold. On shapes where a core owns several
  rows (outer > blockNum), restructure so row R's pass-2 (MTE2+MTE3+V) overlaps row R+1's
  pass-1 prologue fill and accumulate — using row R's FinishRms drain and row R's tail
  flush as the window to issue row R+1's first loads. This amortizes the per-row inter-pass
  drain, first-tile fill, and last-tile flush across rows instead of paying them per row.

BOTTLENECK
  Per-row prologue/epilogue + inter-pass scalar drain, currently paid once per row with no
  cross-row cover when the core owns few rows; fully exposed when a core owns exactly 1 row.

EXPECTED_SHAPES
  Gains only when outer > blockNum (core owns >=2 rows). Both current probe shapes have
  outer==blockNum (8/8 and 2/2) => 1 row/core => NO gain on the probe shapes; gains appear
  on large-outer testcases. On large-D where each row is long, per-row drain is a smaller
  fraction, so the biggest relative win is many short rows per core.

WHY_IT_MAY_HELP
  Converts three per-row serial segments (drain, first fill, tail flush) into overlapped
  work, keeping MTE2/V/MTE3 busy across row boundaries.

WHY_IT_MAY_FAIL
  (a) Queue conflict: row R pass-2 and row R+1 pass-1 both use the shared xQueue/
  residualQueue depth-2 rings; interleaving two rows' tiles into one depth-2 ring needs
  per-row tags or a 3rd slot — the latter is a buffer add (BANNED), the former is complex.
  (b) Per-row state (sumSquares, inverseRms, gm pointers) must be held for in-flight rows.
  (c) On 1-row/core shapes (both probes) it does nothing.

ASCEND_FEASIBILITY
  Medium. Needs an outer-loop restructure and careful queue hand-off; no new API but real
  correctness surface. The shared depth-2 ring is the main obstacle.

UB/CORE/DMA_IMPACT
  UB: adds a few scalar slots for in-flight rows (negligible) but the queue-sharing problem
  may push toward a 3rd slot -> collides with the hard rule. Core: same mapping. DMA: same
  total bytes.

SYNC_IMPACT
  High: cross-row DeQue/EnQue ordering must never let row R+1's MTE2 overwrite a buffer row
  R's compute still reads. Needs explicit reasoning or events.

PRECISION_RISK
  None (each row's math unchanged); correctness risk from cross-row buffer aliasing.

DUPLICATE_CHECK
  vs SCHED-ROWGROUP-X (row-group ownership): that route assigns rows to cores / 32B groups;
  this is about *pipelining across* rows within a core's sequential loop — related but not
  the same; MUST confirm at Main review there is no overlap. vs BATCH-RESIDENT-X (contiguous
  multi-row batch DMA): that batches DMA across rows for residency; this overlaps passes
  across rows — different. NOT a buffer add (and if it needs one, it becomes INFEASIBLE).

MINIMAL_OFAT_DIFF
  Not minimal — this is a loop-structure redesign. Would be its own revision with a single
  conceptual change (inter-row overlap), no other mechanism mixed.

EXPECTED_LOCAL_PROBES
  Needs a shape with outer>blockNum (e.g. rows>=16, blocks=8). Correctness precheck first
  (byte-compare), then P/C. Risk of partial/complex change -> likely NEEDS_ONE_MORE_LOCAL.
```

**Classification: NEEDS_MORE_EVIDENCE** — real mechanism covering three themes at once, but the shared depth-2 queue makes it complex and it may drift toward a buffer add (banned) or collide with SCHED/BATCH route ownership. Park until queue-sharing strategy is designed without extra buffers.

---

# OPTIONAL-HYPOTHESIS-5 — Shape-adaptive schedule selection (large-D vs small-D break-even)

```text
MECHANISM
  The triple-overlap schedule and its prologue/epilogue cost only pay off above a tileCount
  threshold. At tileCount<=1 the V001 path is structurally identical to SEED (inert); at
  tileCount=2 only one overlap event exists; steady state dominates only at tileCount>=4.
  Select the output schedule (and the H1/H3 tweaks) by a tileCount/D bucket: use the minimal
  serial path for small tileCount (avoid paying prologue+drain machinery with no partner to
  overlap) and the full triple-overlap path for large tileCount. Dispatch chosen at host
  tiling time, no runtime branches in the hot loop.

BOTTLENECK
  large-D vs small-D break-even: small-D is dominated by 2 passes + 2 fills + 2 drains + 1
  scalar sync (schedule overhead), large-D is dominated by steady-state MTE2 bandwidth
  (the 2x input re-read).

EXPECTED_SHAPES
  Small (tileCount<=2): choose serial/minimal path to skip dead overlap scaffolding.
  Large (tileCount>=4): choose triple-overlap path. The bucket boundary is the break-even
  to be measured.

WHY_IT_MAY_HELP
  Avoids carrying prologue/epilogue scaffolding where it has no partner, and concentrates
  the overlap where it pays. Makes the route's benefit shape-explicit instead of hoping one
  schedule wins across all D.

WHY_IT_MAY_FAIL
  At tileCount=1 V001 already executes the SEED-equivalent stream, so there may be no dead
  overhead to remove -> selection adds host complexity for ~0 gain on small shapes. Bucket
  boundaries are guesses until measured.

ASCEND_FEASIBILITY
  High for the dispatch (host already builds tiling; route uses `run_kernel` host shim).
  But dispatch selection logic adds a host-side mechanism.

UB/CORE/DMA_IMPACT
  UB: none (same buffers per path). Core: unchanged. DMA: none.

SYNC_IMPACT
  None within a path; two paths must each be individually correct.

PRECISION_RISK
  None — both paths compute identical math (only scheduling differs). Must keep dtype
  strategy identical across buckets.

DUPLICATE_CHECK
  IMPORTANT: idea-pool R005 "tile autotune per D bucket" is owned by MID-X/WIDE-X. A
  D-bucket schedule-selection risks duplicating that work. vs DTYPE-SPECIAL-X: dtype buckets are
  theirs, this is *schedule* buckets (different axis). vs the shape-inertness finding above:
  this hypothesis operationalizes that finding into a kernel dispatch.

MINIMAL_OFAT_DIFF
  Not minimal as a kernel change (adds dispatch). Better delivered as (a) a MEASUREMENT
  finding in RECOMMENDED_NEXT, then (b) an optional later revision only if measurements
  show a real small-vs-large schedule divergence.

EXPECTED_LOCAL_PROBES
  Bracket the break-even: measure V001 vs SEED at tileCount = 1,2,4,8,16 (width 1024..16384)
  to locate where the delta turns positive. This directly maps the large-D/small-D curve.
```

**Classification: NEEDS_MORE_EVIDENCE** (and DUPLICATE risk vs R005/MID-X/WIDE-X). Recommended first as a *measurement* across tileCount buckets rather than a kernel edit; only promote to a kernel hypothesis if the curve shows a real schedule divergence not already covered by MID-X/WIDE-X.

---

## DONOR_RECORDS (candidate donors considered this cycle)

```text
DONOR-1
  SOURCE_ROUTE     MODE-X (retired) / EPI-X (retired) / MIX-A V003 (active first-six-lane)
  MECHANISM        Explicit inter-pipe sync with SetFlag/WaitFlag<HardEvent::MTE2_V,
                   V_MTE3, MTE3_V, V_S, S_V>; MIX-A V003 "V_MTE2 release".
  OLD_CONTEXT      Those routes used events to sequence copy/compute ordering and release MTE2.
  CURRENT_CONTEXT  ASYNC-TRIPLE-X V001 uses only implicit TQue EnQue/DeQue; no explicit
                   event on the output ring.
  WHY_ORTHOGONAL   Donors decide *when a read/write may proceed*; H2 applies events to *store-
                   completion visibility / slot recycling* to hide MTE3 latency.
  WHY_NOT_DUPLICATE Donors never formed an MTE2/V/MTE3 triple-overlap output schedule; this
                   is the first application of explicit events to the output ring of a
                   triple-overlap pipeline.

DONOR-2
  SOURCE_ROUTE     R31B V006 (MTE3 queue depth 2) / R31B V009 (MTE2 queue depth 2)
  MECHANISM        Increase queue depth to widen overlap.
  OLD_CONTEXT      Both online: 43.91 and 43.81; **T14 16670 / 16424 still flat**; note
                   "MTE2 queue not T14 driver".
  CURRENT_CONTEXT  ASYNC-TRIPLE-X already at depth-2 on all queues; triple overlap confirmed.
  WHY_ORTHOGONAL   Used here as EXCLUSION evidence: pure depth increase is PROVEN-flat, so
                   every hypothesis here is deliberately a non-depth (sync/order/structure)
                   mechanism.
  WHY_NOT_DUPLICATE We do NOT propose a depth change; recording the donor to prove the hard
                   rule ("add one more buffer") is evidence-backed, not an assumption.

DONOR-3
  SOURCE_ROUTE     R31A V010 "N-MID-OVERLAP mid-size MTE2 overlap"; R31B V011 "LP row
                   pipeline" / V012 "FP32 wide row pipeline" / V013 "midwide overlap"
  MECHANISM        Overlap MTE2 with compute on mid/wide rows.
  OLD_CONTEXT      V010 -> 44.09 (T07 61->52 helped); V011 -> 45.16; but T14 stayed flat on
                   all of them.
  CURRENT_CONTEXT  ASYNC-TRIPLE-X extends overlap to a third stage (MTE3).
  WHY_ORTHOGONAL   Those are two-stage (MTE2+V) overlaps; this route's V001 added the MTE3
                   stage. H1/H3 tune the *position/order* of the third stage's issues.
  WHY_NOT_DUPLICATE Overlap-as-such is not new; the pass-boundary prologue move (H1) and
                   intra-iteration MTE2/MTE3 order (H3) are scheduling refinements the R31
                   family never isolated (they only varied depth and two-stage overlap).
```

## EXTERNAL_IDEA_RECORDS

```text
EXTERNAL_IDEA  EXT-PIPE-NORM-1
  SOURCE       Published GPU/NPU pipelined-normalization and producer-consumer practice
               (FlashAttention-style tile staging; warp-specialized producer/consumer queues).
               PROVENANCE_CLASS = PUBLIC_ARCHITECTURE_IDEA (literature-derived; live web
               fetch unavailable this turn — not machine-fetched, no code copied).
  MECHANISM    Producer issues the next tile's global load while the consumer computes the
               current tile and a separate epilogue stage drains the previous tile's store;
               stages are decoupled by per-stage completion flags rather than by growing the
               buffer pool.
  WHY_DIFFERENT_FROM_EXISTING_ROUTES  Existing routes vary queue DEPTH (flat) or do two-
               stage overlap. This stresses STAGE-BOUNDARY decoupling (prologue prefetch,
               store-completion flags) with a fixed buffer count — matching H1/H2.
  EXPECTED_BOTTLENECK  Serialized stage boundaries: inter-pass scalar drain + first-tile
               fill + exposed store completion (exactly the BOTTLENECK_MODEL items 1-3).
  APPLICABLE_ROUTE  ASYNC-TRIPLE-X
  PROVENANCE_CLASS  PUBLIC_ARCHITECTURE_IDEA

EXTERNAL_IDEA  EXT-EVENT-DECOUPLE-2
  SOURCE       Established NPU/GPU async-copy + event-fence patterns (explicit inter-unit
               fences instead of blocking queue pops). PROVENANCE_CLASS =
               PUBLIC_ARCHITECTURE_IDEA (literature-derived; not machine-fetched).
  MECHANISM    Decouple the issuing thread from copy completion using explicit per-slot
               fences, so issue-ahead continues while earlier stores complete in the
               background; only the exact reused slot waits.
  WHY_DIFFERENT_FROM_EXISTING_ROUTES  Current kernel blocks on queue DeQue/FreeTensor in
               the scalar path; historical routes proved depth changes flat. This changes
               the *sync primitive*, not the pool size — the orthogonal axis the hard rule
               demands.
  EXPECTED_BOTTLENECK  Store latency leaking into the scalar critical path (item 5) and the
               exposed last-tile flush (item 3).
  APPLICABLE_ROUTE  ASYNC-TRIPLE-X
  PROVENANCE_CLASS  PUBLIC_ARCHITECTURE_IDEA
```

---

## RECOMMENDED_NEXT

```text
1. SHAPE FIRST (blocking for all timing): never P/C ASYNC on width=1024. At kTileLength=1024
   that is tileCount=1 where V001 == SEED structurally -> pure noise. All future ASYNC
   measurements MUST use width>1024 (tileCount>=2), prefer width=8192 (tileCount=8),
   rows>blocks avoided so the tail/drain are not cross-row covered when isolating H1.
   This is a MEASUREMENT finding Main can act on immediately; it explains the seed CV 35.6%
   and "signal inside noise" on the standard probe.

2. RUN H1 first when a device window opens (post MEASUREMENT_BLOCKED lift): smallest, cleanest
   single edit (move pass-2 tile-0 prefetch before FinishRms), zero precision risk, feasible,
   covers first-tile-fill + inter-pass-drain. Class READY_FOR_MAIN_REVIEW.

3. BEFORE spending a window on H2: resolve the TQue output-ring question (does FreeTensor/
   DeQue couple MTE3 completion into the scalar path?) via AscendC queue semantics + a
   same-binary correctness precheck. If the store is already async, H2 collapses -> mark
   LOCAL_REJECTED without a device run. Class NEEDS_MORE_EVIDENCE.

4. H3 (MTE2/MTE3 issue-order) is the cheapest experiment after H1 — a two-call swap — but
   its physical effect on 910B HBM arbitration is unproven; sequence it after H1 so we learn
   prologue vs contention independently. Class NEEDS_MORE_EVIDENCE.

5. H4 (inter-row continuity) and H5 (shape-adaptive selection) are parked: H4 risks a buffer add
   and SCHED/BATCH route overlap; H5 risks R005/MID-X/WIDE-X duplication. Revisit only if
   H1-H3 are flat and a large-outer shape opens.

SCREENED COUNT: 5 hypotheses (H1 READY_FOR_MAIN_REVIEW; H2/H3/H4/H5 NEEDS_MORE_EVIDENCE;
H5 also flagged DUPLICATE-risk). >=3 requirement met. No INFEASIBLE. No kernel edits, no
device runs, no shared-control changes this turn.
```

### Sources inspected (read-only)
- `phase4/control/execution-contract.md` (section R), `phase4/control/local-timing-protocol.md`
- `phase4/control/idea-pool-29-routes.md`, `phase4/control/architecture-evidence-map.md`
- `phase4/control/scheduler.tsv` (ASYNC row), `phase4/control/next-round-plan.md`
- `phase4/control/results.tsv` (R31B V006/V009, R31A V010, R31B V011-V013, MIX-A V003, A001 V011/V017)
- `cann-next6/ASYNC-TRIPLE-X/phase4/local/ASYNC-TRIPLE-X/V001/` — submission.asc, source-meta.json, diff.patch, local-result.json, MAIN-REVIEW*.md, PARKED.md
- `cann-next6/ASYNC-TRIPLE-X/phase4/workspaces/ASYNC-TRIPLE-X/PRECODE.md`, ROUTE-BRIEF.md
- Archive event/queue survey: R013 donor kernel, MODE-X/EPI-X/WIDE-X `SetFlag/WaitFlag` usage, TQue depth census (predominantly depth-1; V001/R013 at depth-2).

---

## PROBE-SHAPE DESIGN (approved change, 2026-09-25)

Measurement-design only. Source 2defc6c2 unchanged, no device runs, no new Revision, no shared-control edits. Derives the new primary/secondary performance probes under Main's approval (retire 8×1024 as primary performance probe). All line references: `cann-next6/ASYNC-TRIPLE-X/phase4/local/ASYNC-TRIPLE-X/V001/submission.asc` (V001), `.../support/ASYNC-TRIPLE-X-SEED.asc` (SEED), `cann-next6/ASYNC-TRIPLE-X/phase4/workspaces/ASYNC-TRIPLE-X/probe_main.inc` (probe host).

### Derived tileCount table

`kTileLength = 1024` fixed (V001:12); `tileCount = (width + kTileLength - 1) / kTileLength` (V001:246). A **full triple iteration** is a pass-2 loop iteration where BOTH guards hold — `tile + 1 < tileCount` (V001:270, MTE2 prefetch N+1) and `tile >= 1` (V001:278, MTE3 store N-1) alongside `ComputeOutputTile(N)` (V001:284) — i.e. tiles 1..tileCount-2 → **(tileCount − 2)** such iterations.

| width | tileCount | full triple iterations | verdict |
|---:|---:|---:|---|
| 1024 | 1 | 0 | INERT — both guards false, V001 stream ≡ SEED |
| 2048 | 2 | 0 | overlap pairs only (tile0: MTE2+V; tile1: MTE3+V); no iteration holds all three stages |
| 4096 | 4 | 2 (tiles 1–2) | legal, mechanism engaged |
| 6144 | 6 | 4 (tiles 1–4) | legal, mechanism engaged |
| 8192 | 8 | 6 (tiles 1–6) | legal, mechanism engaged, steady state dominates |

Refinement of the approval: tileCount≥2 engages pairwise overlap, but the literal triple (all three stages inside one iteration) requires **tileCount≥3** — at tileCount=2 the two overlap-capable iterations each miss one stage.

### Legality of candidate widths (rows × width × dtype)

No width- or rows-dependent constraint blocks 4096 / 6144 / 8192:

- **Host core policy**: `blockNum = min(outer, availableCoreNum)` (V001:435-436), must be ≥1 (V001:439); probe host additionally requires `1 ≤ blocks ≤ rows` (probe_main.inc:50) and passes `blocks` as availableCoreNum (probe_main.inc:115). Width never enters this path; any rows≥1 with blocks=rows is legal.
- **UB budget**: every `InitBuffer` size is a kTileLength constant (V001:82-94) — UB does not scale with width or rows at all. fp32: queues 40,960 B (x 8K + residual 8K + param 16K + output 8K, all depth-2) + fp32 work buffers 16,384 B (4×1024 floats) + reduceTmp 32,768 B + misc 96 B = **90,208 B ≈ 88.1 KB of ~192 KB**. fp16: queues halve to 20,480 B, work buffers stay fp32 → **69,728 B ≈ 68.1 KB**. Both fit at all three widths with large headroom.
- **dtype**: kernel accepts dtype ∈ {0,1,2} = fp32/fp16/bf16 (V001:417; entries V001:333-349); gamma/bias must be 1-D of the same width and dtype (V001:420). Per-tile arithmetic is width-independent — ReduceSum count ≤ kTileLength = 1024 per tile (V001:183-184); a wider row only adds tiles, it never grows a single reduce or any UB buffer. fp16 at width=8192 already has correctness evidence on record (2×8192 wide PASS, max_abs 3.28e-07, local-result.json).
- **Probe-harness caveat (measurement side, not kernel)**: probe_main.inc hardcodes dtype 0 / fp32 (L62, L99-103) with an fp32 golden tolerance (L139-171). Kernel-side fp16 is legal; an fp16 timing run needs a harness extension only.
- **GM footprint** (fp32, 5 buffers): 8×4096 → 0.66 MB, 8×6144 → 0.98 MB, 8×8192 → 1.31 MB — trivial. All three widths are exact multiples of 1024, so every tile is full and the DataCopyPad padding path (V001:123-131) never engages.

### Chosen shapes

```text
PRIMARY     rows=8  width=8192  blocks=8  fp32   tileCount=8   (6 full triple iterations)
SECONDARY   rows=8  width=4096  blocks=8  fp32   tileCount=4   (2 full triple iterations)
TERTIARY (allowed, deprioritized)  rows=8 width=6144 blocks=8 fp32  tileCount=6
```

**Rows reasoning (core occupancy):** with blocks==rows the host launches blockNum=8 cores and the kernel row-stride loop `row += blockNum` (V001:99-103) gives each core exactly **one row** (blocks==rows → 1 row/core finding). The per-row prologue fill, inter-pass FinishRms drain, and last-tile flush are then fully exposed with no cross-row cover — which (a) isolates the triple-overlap schedule itself in V001-vs-SEED pairs, and (b) keeps H1 attribution clean by construction (H4 inter-row cover excluded). rows=8 preserves the retired probe's 8-core occupancy (HBM traffic across 8 concurrent rows) rather than the 2-core wide variant.

- **PRIMARY 8×8192**: tileCount=8 → steady state dominates (6 of 8 iterations run all three stages); matches RECOMMENDED_NEXT #1 "prefer width=8192". Correctness precedent exists at 2×8192 (max_abs 3.28e-07); 8×8192 itself is not yet on record — run the probe correctness precheck (repeats=0) first when a device window opens.
- **SECONDARY 8×4096**: first bucket where the full triple repeats (2 iterations) at half the row bytes — a different kernel-length/CV regime, and it doubles as the mid point of the H5 tileCount curve (1/2/4/8).
- **6144** stays tertiary: legal, but adds little between the 4096/8192 bracket; additionally the SCHED-parent 1×6144 wide floor was NEEDS_VALIDATION (local-timing-protocol.md L155). Every shape still needs its own same-binary floor before P/C regardless.

### Triple-overlap engagement at the PRIMARY shape (source trace)

At rows=8 width=8192 → tileCount=8 (V001:246). Pass-2 loop (V001:266-285):

- **iteration tile=1**: V001:270 `if (tile + 1 < tileCount)` → `2 < 8` TRUE → V001:274 `CopyInOutputData(row, nextOffset, nextCount)` = MTE2 prefetch tile 2; V001:278 `if (tile >= 1)` TRUE → V001:282 `CopyOut(row, prevOffset, prevCount)` = MTE3 store tile 0, issued BEFORE compute; V001:284 `ComputeOutputTile(count, inverseRms)` = Vector tile 1. → **MTE2(2) / V(1) / MTE3(0) in one iteration — mechanism engaged** (comment V001:276-277 states exactly this).
- **tiles 2–6** repeat the same three-issue pattern (steady state; 6 total full triple iterations).
- **tile=7**: V001:270 → `8 < 8` FALSE (no N+1 left), V001:278 TRUE → store tile 6, compute tile 7; then the post-loop flush V001:286-291 stores tile 7.
- **Contrast at width=1024 (tileCount=1)**: V001:270 → `1 < 1` FALSE, V001:278 → `0 >= 1` FALSE → loop body is only V001:284, and epilogue V001:290 issues the same same-tile copy SEED issues at its L276 — V001 executes the SEED instruction stream and the MTE3-stage hypothesis never runs. The retired probe therefore cannot show the mechanism.
- **SEED structural contrast** (diff.patch:16-37): SEED pass-2 is prefetch-guard (SEED:269) → `ComputeOutputTile` (SEED:275) → same-tile `CopyOut` AFTER compute (SEED:276); V001 adds the `tile>=1` prev-store before compute and moves the last store to the post-loop flush. That diff is the entire SINGLE_HYPOTHESIS.

### OLD-PROBE DATASET TAG

```text
OLD-PROBE-8x1024-NON-QUALIFYING  (applied 2026-09-25)
Scope:   all 8x1024 standard-shape ASYNC-TRIPLE-X P/C datasets —
         support/results/, results-set2/, results-controlled-dev4/ standard-shape
         pairs, and the "seed CV 35.6%, median delta -21.5 us inside a 75.9 us
         margin" record.
Reason:  at kTileLength=1024, width=1024 -> tileCount=1 -> V001 instruction
         stream == SEED (both pass-2 guards false); the triple-overlap mechanism
         is structurally inert at that shape.
Status:  retained as noise-floor / harness-history reference ONLY;
         NON-QUALIFYING for any triple-overlap performance claim or H1/H2/H3
         attribution.
Note:    the wide 2x8192 datasets ARE structurally qualifying (tileCount=8,
         mechanism engaged) but remain LOAD_CONTAMINATED per CURRENT_BLOCKER —
         not usable until re-run under a qualified window.
```

### WHAT_WOULD_FALSIFY (H1, on the new probes)

H1 = move the pass-2 tile-0 `CopyInOutputData` before `FinishRms` (class READY_FOR_MAIN_REVIEW). Judge only on a shape whose same-binary floor passes the protocol first. On PRIMARY 8×8192 and SECONDARY 8×4096, device-event, warmup≥10, ≥4 interleaved pairs vs V001:

1. **Measurement validity first**: if the 8×8192 same-binary floor fails the protocol thresholds, the run says nothing about H1 — requalify the shape; do not judge a mechanism from an unqualified shape.
2. **|paired median delta| ≤ same-binary MAD/median on BOTH primary and secondary** → the serialized drain + tile-0 fill is not on the critical path (or is already overlapped by the compiler) → H1 = LOCAL_REJECTED / NEEDS_MORE_EVIDENCE.
3. **Consistently negative delta beyond the floor on both shapes** → issuing the prefetch earlier hurts (queue pressure or delayed pass-2 entry) → WHY_IT_MAY_HELP falsified.
4. **Magnitude must be roughly width-independent**: H1 saves one fixed drain + fill per row (FinishRms is 8-element vector work, V001:236-241; tile-0 load is always 1024 elements), so the ABSOLUTE delta at 4096 ≈ at 8192, with relative delta ~2× larger at 4096. An absolute delta that scales with width instead → misattribution to a steady-state effect, not the prologue → H1 attribution falsified.
5. **Gain appears only when rows > blocks** (multi-row per core) and ~0 at blocks==rows → the gain is cross-row cover (H4 territory), not the prologue → attribution falsified.
6. **Any output difference vs V001 under byte compare** → PRECISION_RISK=None claim falsified → correctness stop before any timing credit.
