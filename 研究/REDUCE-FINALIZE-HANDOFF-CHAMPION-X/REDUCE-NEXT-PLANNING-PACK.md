# REDUCE-FINALIZE — NEXT TRACK-B PLANNING PACK

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize
BRANCH=w2/m2/reduce-finalize
DATE=2026-10-04
TRACK=TRACK-B
MODE=READ_ONLY_STATIC_RESEARCH
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
NPU=NOT_USED
```

## Scope and evidence

This is a fresh static screen against the exact R31B V011 parent. It does not
change the historical RFH-1 audits: those found only one survivor within the
then-frozen terminal-event-wait boundary. The candidates below are different
source-level variables discovered outside that RFH-1 wait-placement change.
They remain research hypotheses, not approved route scope or implementation
instructions.

Read: project `AGENTS.md`, `cann-mainline` Skill, current project experiment,
execution, local-timing, server, and Git rules; all existing research files
under this REDUCE route; the route's V001 declaration, source diff and local
result; and the exact V011 source. The parent SHA was independently checked.
No other Route's private context was read.

Prior RFH-1 result is recorded as a local engineering score only:
`+0.553205%`, `POOR` / `LOCAL_NEUTRAL`; it did not advance Local Best. It does
not affect the parent of this research pack.

## Candidate hypotheses

### RFH-N1 — OMIT-UNUSED-INVRMS-ARRAY-ZERO-INIT

- **Mechanism / bottleneck:** remove the aggregate zero initializer from the
  per-batch scalar array, leaving the existing finalizer writes and pass-2
  reads unchanged. The candidate overhead is initialization of values that
  are overwritten before use.
- **Exact code site:** V011 `ProcessWideLowPrecision()`,
  `float invRmsValues[kWideFullYMaxRows] = {};` near line 3102; every active
  `batchRow` is assigned in the finalizer near lines 3224–3241 and consumed
  in pass 2 near lines 3320/3330.
- **Single variable:** aggregate initialization of `invRmsValues` only;
  do not alter array size, finalizer arithmetic, loop bounds, or pass-2 use.
- **Target shape/dtype:** FP16 and BF16 wide-path batches, especially
  `D=8193–16384` with `batchRows < kWideFullYMaxRows`; include one-row and
  multi-row-per-core forms only as separate probes.
- **Why it may help:** avoid scalar/register initialization instructions for
  unused array entries on each batch, shortening setup before pass 1.
- **Why it may fail:** the compiler may already prove the zero stores dead or
  scalarize the array; any saved work may be below kernel-level noise.
- **Resource / risk:** no UB allocation, DMA, reduction, sync, or precision
  change. Static safety condition: each element subsequently read must be
  assigned on every path first; the current `batchRow < batchRows` finalizer
  and pass-2 loops appear to satisfy this for each entered batch.
- **Duplicate audit:** distinct from RFH-1's event-wait placement and from
  RFH-2/3/4/5 screened mechanisms in the existing route handoff. No prior
  REDUCE-route experiment in the reviewed research changes this initializer.
  `DUPLICATE_CHECK=PASS_WITH_CODEGEN_UNVERIFIED`; a broader cross-route audit
  remains for Main review.
- **Minimal experiment:** only after Main/Planning selection, delete `{}` and
  make no other source change; verify exact-source Build and targeted
  FP16/BF16 correctness, then compare the same parent/candidate on a
  representative batch shape. Inspect generated code if available to confirm
  whether initialization instructions were present; do not infer a gain from
  source appearance alone.
- **Maturity:** `NEEDS_MORE_EVIDENCE`.

### RFH-N2 — PACK-ROW-REDUCTION-PARTIALS

- **Mechanism / bottleneck:** use the active `tileCount` as the row pitch for
  the per-row partial-sum scratch instead of the fixed 16-float pitch. Keep
  each row's partials contiguous and preserve their order and reduction tree;
  this probes inter-row scratch locality/addressing, not a new reduction.
- **Exact code site:** V011 constants
  `kWideFullYReduceStride = 16` near line 1284; scratch allocation in the
  low-precision wide branches near lines 96–110; partial writes at
  `ProcessWideLowPrecision()` lines 3188–3203 and final read at lines
  3227–3229.
- **Single variable:** partial-row pitch only. Producer and finalizer indices
  must use the same pitch; retain the existing maximum-sized allocation,
  `ReduceSum` calls/order, tile geometry, batch ownership and event schedule.
- **Target shape/dtype:** FP16 `D=12289–16384` (4 tiles) and BF16
  `D=8193–12288` (3 tiles), with `batchRows > 1` and `tileCount` materially
  below 16. Confirm the actual dispatch and `batchRows` from the exact parent
  for each probe.
- **Why it may help:** reduce the gaps between adjacent rows' active partial
  vectors and potentially improve local scratch working-set locality while
  retaining the same bytes consumed by each final `ReduceSum`.
- **Why it may fail:** each individual row's partial vector is already
  contiguous; no partial bytes or reduction operations are removed. Dynamic
  pitch arithmetic may cost more, and the accelerator may show no benefit.
- **Resource / risk:** no GM DMA, row ownership, UB lifetime, or precision
  change is intended. Check `tileCount <= kWideFullYReduceStride`, bounds for
  every producer/finalizer access, and any alignment requirement before an
  approved implementation; do not shrink the allocation in this hypothesis.
- **Duplicate audit:** independent of RFH-1's event edge. Existing REDUCE
  audits classify manual/eager/tree changes as REDUCE-HIER duplicates; this
  proposal preserves primitive, partial count and tree. Existing TILECOUNT
  research concerns FP32 unrolling, and MULTIROW-DMA concerns transfer shape
  and ownership; neither is changed here. Because this touches reduction
  scratch layout, `DUPLICATE_CHECK=NEEDS_MAIN_CROSS_ROUTE_REVIEW` rather than
  an unconditional pass.
- **Minimal experiment:** after selection, alter only the partial-row pitch
  (with producer and consumer kept consistent), leave allocation and all
  reductions untouched, statically verify bounds/alignment, then run exact
  correctness and one shape-matched local comparison for a confirmed
  multi-row FP16/BF16 case.
- **Maturity:** `NEEDS_MORE_EVIDENCE`.

### RFH-N3 — FUSE-MEANSQUARE-SCALE-AND-EPSILON

- **Mechanism / bottleneck:** express the scalar `squareSum * invRowWidth +
  epsilon` finalization as one fused multiply-add, probing whether one scalar
  instruction can be removed from the dependency chain.
- **Exact code site:** V011 `ProcessWideLowPrecision()` finalizer,
  `const float meanSquare = squareSum * invRowWidth + epsilon;` near line
  3233.
- **Single variable:** contraction of this one multiply-plus-add only; leave
  `ReduceSum`, reciprocal/sqrt sequence, synchronization, and all other
  arithmetic unchanged.
- **Target shape/dtype:** FP16/BF16 shapes dispatched to the active wide
  low-precision path, particularly multi-tile `D=16384/32768`; this scalar
  finalizer executes once per active row.
- **Why it may help:** if the compiler does not already contract the
  expression, a fused scalar operation may shorten the finalizer dependency
  chain.
- **Why it may fail:** compiler contraction may already occur; scalar FMA
  throughput may not be better; fused rounding changes numerical behavior
  and could violate the project's dtype tolerances.
- **Resource / risk:** no UB/DMA/row mapping change; precision risk is
  explicit and correctness must compare against the exact parent.
- **Duplicate audit:** independent from RFH-1's wait placement, but overlaps
  the scalar-finalizer/arithmetic boundary previously screened under RFH-2
  and the related R019 / epilogue-arithmetic families. Do not treat this as a
  clean REDUCE survivor unless Main confirms route ownership and a fresh
  duplicate audit. `DUPLICATE_CHECK=REVIEW_REQUIRED`.
- **Minimal experiment:** none until ownership/duplicate review. If approved,
  use one expression-only source delta; inspect whether the parent already
  emits FMA, then run correctness before any timing.
- **Maturity:** `DUPLICATE` pending Main's cross-route review; not selected.

## Review summary

```text
HYPOTHESIS_COUNT=3
RFH1_RENAMED=0
PROVISIONAL_NEW_LEADS=2
CROSS_ROUTE_REVIEW_REQUIRED=2
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
```

N1 and N2 are distinct static leads outside RFH-1; neither is asserted to be
performance-positive. N2 requires explicit reduction-scratch ownership
review. N3 is included as a screened, distinct mechanism for completeness,
not as a currently admissible REDUCE candidate. The 2026-10-01 historical
`INDEPENDENT_SURVIVOR_COUNT=1` remains an accurate snapshot of its earlier
boundary audit; this document asks Main/Planning to decide whether N1/N2
expand the pool. No hypothesis is selected here.

```text
KERNEL_OR_SOURCE_CHANGED=NO
REVISION_CREATED=NO
BUILD_CORRECTNESS_TIMING=NOT_RUN
SERVER_NPU=NOT_USED
DASHBOARD_SHARED_LEDGER=UNTOUCHED
PUSH=NOT_RUN
```
