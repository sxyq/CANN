# MAIN-2 Next Track-B Planning Pack — 2026-10-04

```text
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
MODE=READ_ONLY_STATIC_RESEARCH
MAIN_SELECTED=NONE
REVISION_CREATED=NO
KERNEL_CHANGED=NO
BUILD/CORRECTNESS/TIMING/NPU=NOT_RUN
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

The source hash was checked against the local V011 submission and sidecar.
These are planning hypotheses only. No candidate was selected, and no new
Route or Revision is proposed.

## Candidate 1 — FP16/BF16 full-row parameter DMA command coalescing

```text
HYPOTHESIS_ID=MAIN2-TB-PARAM-DMA-COALESCE-D8192
EXACT_FUNCTIONS=ProcessFp16FullRowOutputPipelined; ProcessBf16FullRowOutputPipelined
EXACT_CODE_SITE=submission.asc:887-896 and :1011-1020 (gamma/bias tile-load loops)
TARGET_SHAPE=[M,8192], localRows > 1, existing full-row output path
TARGET_DTYPE=FP16, BF16
```

- **MECHANISM / BOTTLENECK:** each full-row path currently loads two 4096-element
  gamma slices and two bias slices before processing the row batch. The probe
  asks whether two contiguous transfers (one gamma, one bias, 8192 elements
  each) lower MTE2 command/setup cost without changing bytes or residency.
- **ONE_FACTOR_DIFF:** change only the copy granularity for these full-row
  parameter loads. Preserve destination layout, total bytes, row sharing,
  compute order, waits, events, and stores.
- **EXPECTED_EFFECT / THROUGHPUT_EXPECTATION:** at most a small startup/command
  overhead reduction for multi-row D=8192 blocks; no expected change to
  arithmetic or per-row data volume. Zero gain is plausible if CCEC/hardware
  already coalesces the transfers or DMA issue is hidden.
- **WHY_NOT_DUPLICATE / RELATED_OLD_ROUTE:** COEFF-LOCALITY H3 concerns the
  generic single-row D=6144/8192 cache policy and moves loads out of the
  output-pass tile loop. This candidate retains the existing full-row
  per-core residency and changes only transfer granularity in different
  specialized functions. It is adjacent to COEFF/R014 parameter-DMA work and
  that pack labels full-row residency optimal; therefore novelty is
  `NEEDS_MAIN_REVIEW`, not assumed.
- **MINIMAL_EXPERIMENT:** first verify DataCopyPad length/alignment support and
  inspect exact V011 codegen for the four existing transfers. A source-only
  prototype must prove identical local contents and total bytes. If the parent
  already emits two full-row transfers or the candidate adds staging/spills,
  classify `CODEGEN_NOOP`/`INFEASIBLE`; only a Planning-selected Revision may
  proceed to correctness and timing.
- **RISK:** DMA length/alignment or path-dispatch mistake could corrupt
  coefficients; timing ceiling is low and may be below noise.

```text
DUPLICATE_STATUS=RELATED_TO_COEFF_LOCALITY; PLANNING_NOVELTY_REVIEW_REQUIRED
MATURITY=NEEDS_CODEGEN_AND_OWNERSHIP_REVIEW
```

## Candidate 2 — exact-preserving wide UB-fit search simplification

```text
HYPOTHESIS_ID=MAIN2-TB-WIDE-UB-FIT-SEARCH
EXACT_FUNCTION=ChooseWideFullYRows
EXACT_CODE_SITE=submission.asc:1293-1328; called from Init at :71, :82, :99
TARGET_SHAPE=all existing wide-path probes where ChooseWideFullYRows runs
TARGET_DTYPE=FP32, FP16, BF16
```

- **MECHANISM / BOTTLENECK:** `ChooseWideFullYRows` performs a bounded descending
  row-fit search and may then decrement `tileElems` in 512-element steps to
  select the UB-safe `(wideFullYRows_, tileElems)` pair during per-block Init.
  The hypothesis is to simplify only the calculation that selects that pair.
- **ONE_FACTOR_DIFF:** replace the search representation with an
  exact-equivalent bounded formula/table; every accepted `(rowWidth, yTypeBytes,
  ioTiles, ioTypeBytes, workTiles)` input must return the identical rows and
  tile size. No allocation, tile geometry, or work loop may change.
- **EXPECTED_EFFECT / THROUGHPUT_EXPECTATION:** possibly reduce fixed per-block
  Init scalar/branch work. This is setup-only and likely a very small effect;
  no throughput gain is assumed unless exact-parent codegen shows residual
  search instructions on a meaningful workload.
- **WHY_NOT_DUPLICATE / RELATED_OLD_ROUTE:** TILECOUNT H1-H3 target source-loop
  expansion in the FP32 compute/output passes and failed for lack of exact
  parent codegen evidence. This candidate targets only the UB-fit selector in
  Init and must return identical allocation geometry. It is not a tile-size
  search or a new UB plan; R005/UB work are related context, not proof this
  selector is costly.
- **MINIMAL_EXPERIMENT:** exhaustive host-side equivalence over the valid
  width/type/buffer-argument domain, then exact V011 codegen inspection for the
  bounded search. No kernel edit is allowed unless Planning selects it. If
  codegen already folds the search or there is no measurable fixed-cost
  opportunity, classify `CODEGEN_NOOP` and stop.
- **RISK:** any off-by-one changes row residency or tile geometry, risking UB
  overflow or a different kernel path. Full equivalence is mandatory.

```text
DUPLICATE_STATUS=DISTINCT_FROM_TCSU_BODY_UNROLL; RELATED_TO_UB/TILING
MATURITY=NEEDS_EXACT_CODEGEN_EVIDENCE
```

## Candidate 3 — conditionally reserve the narrow-mid parameter-ready event

```text
HYPOTHESIS_ID=MAIN2-TB-NARROWMID-UNUSED-PARAM-EVENT
EXACT_FUNCTION=ProcessNarrowMidOverlap
EXACT_CODE_SITE=submission.asc:499-518 and :577-620
TARGET_SHAPE=[M,D], 128 < D <= 4096, localRows > 1
TARGET_DTYPE=FP16, BF16 (FP32 is a separate control)
```

- **MECHANISM / BOTTLENECK:** `paramReady` is allocated before
  `residentParams = (localRows > 1)`, while its SetFlag/WaitFlag uses are only
  in the `!residentParams` arm. For multi-row blocks this event ID is not used.
- **ONE_FACTOR_DIFF:** allocate/release `paramReady` only for the single-row
  path that sets and waits on it. Keep every existing event direction, flag,
  wait, DMA issue order, arithmetic operation, and store unchanged.
- **EXPECTED_EFFECT / THROUGHPUT_EXPECTATION:** at most remove an unused event
  reservation from multi-row narrow-mid blocks; likely zero if AllocEventID is
  compile-time bookkeeping or no runtime instruction. No timing gain is
  claimed without codegen evidence.
- **WHY_NOT_DUPLICATE / RELATED_OLD_ROUTE:** ASYNC-TRIPLE and prior sync-topology
  work change overlap/dependency placement. This item does not move or remove
  a SetFlag/WaitFlag and does not change pipeline depth; it concerns only an
  event ID whose uses are unreachable when parameters are resident. Because
  it touches event resource ownership, it still requires Main review against
  synchronization-route boundaries.
- **MINIMAL_EXPERIMENT:** prove by control-flow audit that every path allocating
  the event has exactly the matching set/wait/release and the multi-row path
  has none. Inspect generated code for a real runtime allocation/reservation
  cost. If no emitted-code difference exists, classify `CODEGEN_NOOP`; never
  remove any wait/flag to force a result.
- **RISK:** event allocator semantics may be compile-time or have hidden
  lifecycle requirements; an incorrect conditional release can cause
  correctness or synchronization failure.

```text
DUPLICATE_STATUS=NOT_THE_ASYNC_OVERLAP_MECHANISM; EVENT-OWNERSHIP_REVIEW_REQUIRED
MATURITY=NEEDS_EVENT-API_AND_CODEGEN_EVIDENCE
```

## Planning handoff

```text
HYPOTHESIS_COUNT=3
MAIN_SELECTED=NONE
IMPLEMENTATION_AUTHORIZED=NO
ONLINE_DECISION_CHANGED=NO
PLANNING_DECISION_CHANGED=0
```

Candidate 1 is adjacent to the old coefficient-locality family and requires a
novelty decision. Candidate 2 has the clearest boundary from TILECOUNT because
it preserves all geometry and touches only Init's capacity-selection
calculation, but its expected performance ceiling is small. Candidate 3 may be
a compiler no-op and needs event-resource review. Planning may select one,
request more static evidence, or reject all three. This pack selects nothing.
