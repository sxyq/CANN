# Main-2 Next Route Planning Pack

Date: 2026-10-05

This is an evidence and recommendation pack. It does not select, park, close,
merge, replace or create a route. `MAIN_SELECTED=NONE`.

## Canonical boundary

- Official anchor: R31B V011, Official 45.16.
- Local scorer: `MAIN2-CANONICAL-V1`.
- Suite: `CANONICAL_LOCAL_SUITE_V1`; Core C12/C13/C14/C16, diagnostics C01/C08/C11.
- Formula: `100 * geometric_mean(fresh_V011_core_time / candidate_core_time)`.
- Fresh anchor: source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, 21/21 valid runs.
- Census: 26 routes, 43 implemented revisions, 38 candidate rows with finite canonical scores plus the anchor, 6 candidate rows explicitly unscored.
- No new Kernel Revision and no Online submission were created in this round.

## Local top 10

| Rank | Route | Revision | Local score | Delta vs V011 | Fragile | Reliable gain | Core quality |
|---:|---|---|---:|---:|---|---|---|
| 1 | ASYNC-OVERLAP-CHAMPION-X | V001 | 100.725142 | +0.725142 | YES | NO | 1 GOOD / 0 FAIR / 3 POOR |
| 2 | R31B | V011 | 100.000000 | 0.000000 | NO | BASELINE | 3 GOOD / 0 FAIR / 1 POOR |
| 3 | ASYNC-OVERLAP-CHAMPION-X | V003 | 99.759996 | -0.240004 | NO | NO | 3 GOOD / 0 FAIR / 1 POOR |
| 4 | EPILOGUE-FUSE-X | V001 | 99.626812 | -0.373188 | NO | NO | 3 GOOD / 0 FAIR / 1 POOR |
| 5 | SCHED-CHAMPION-X | V002 | 99.335036 | -0.664964 | NO | NO | 1 GOOD / 0 FAIR / 3 POOR |
| 6 | HOTLOOP-ADDR-HOIST-CHAMPION-X | V002 | 99.256484 | -0.743516 | NO | NO | 2 GOOD / 0 FAIR / 2 POOR |
| 7 | REDUCE-HIER-X | V002 | 99.011633 | -0.988367 | NO | NO | 3 GOOD / 0 FAIR / 1 POOR |
| 8 | EPILOGUE-FUSE-X | V002 | 98.684973 | -1.315027 | NO | NO | 3 GOOD / 0 FAIR / 1 POOR |
| 9 | ASYNC-OVERLAP-CHAMPION-X | V002 | 98.523402 | -1.476598 | NO | NO | 2 GOOD / 0 FAIR / 2 POOR |
| 10 | STORE-EPILOGUE-X | V001 | 98.089048 | -1.910952 | NO | NO | 2 GOOD / 0 FAIR / 2 POOR |

The numerical local champion is `ASYNC-OVERLAP-CHAMPION-X/V001`, but it is a
provisional numerical leader only. Its Core ratios are C12=1.030395,
C13=0.955769, C14=0.976117, C16=1.010624; three Core cases are POOR and the
paired-device drift contradicts a reliable gain. It must not become a Parent
or an Online recommendation.

## Mechanism and route findings

- ASYNC has one real local leader, followed by neutral/regressed siblings. The
  source review confirms V001 is an inter-pass parameter prologue change, but
  its scope is narrow and its signal is fragile. Do not recreate it under
  INTERPASS without a materially different boundary.
- EPILOGUE arithmetic has the cleanest source-level OFAT among the top group,
  but the fixed Core vector contains a C12 regression and no reliable gain.
  Further scale-fold/VMLA variants are not justified by this scoreboard alone.
- SCHED V002 is close to the anchor numerically, but its source includes both
  row-group ownership and a BF16 store-lifetime synchronization change. The
  score is not a causal proof of occupancy benefit; an unchanged or near-zero
  row-group effect on the canonical shapes is plausible.
- ADDR V002 has a real rolling row-base change, but C12 and C14 regress while
  C13/C16 are near neutral. The historical ADDR H3 Official 42.72 remains a
  false-positive warning; no address route is recommended as an immediate
  performance parent.
- REDUCE, COEFF/parameter prefetch, and STORE variants provide negative or
  neutral evidence in this unified score. Their broad forms are exhausted for
  the current evidence set.
- Research-only routes with no implementation remain research-only: INTERPASS,
  CROSSROW, PARAM, ROW-OCCUPANCY, TILECOUNT and UB-CHAMPION. Their existence
  does not constitute a route decision.

## Duplicate and exhaustion review

Already materially explored or rejected for this Planning gate:

- Generic reduction topology: REDUCE-HIER and REDUCE-INVSCALE evidence does not
  support a large-D reduction bottleneck.
- Broad gamma/bias locality and parameter prefetch: COEFF variants and related
  R014/BATCH evidence do not support unrestricted residency. Only a narrowly
  gated lifetime question remains open.
- Same-row MTE2/V/MTE3 overlap and inter-pass parameter tile-0 prefetch:
  ASYNC V001/V002/V003/V004 already cover adjacent stage and handoff surfaces.
- Output store aggregation and store scheduling: STORE-EPILOGUE variants and
  ASYNC/store research cover the broad store axis.
- Row-group ownership: SCHED-ROWGROUP and SCHED-CHAMPION cover the 32B/group
  family; any new occupancy proposal must keep row-unit ownership and change
  only launch-width policy.
- Address arithmetic: ADDR siblings cover row-base/cursor/index families; any
  new address proposal needs codegen evidence, not another algebraic rewrite.

## Recommendations for Planning

### TOP-1: bounded occupancy launch-width probe

- Route / hypothesis: `ROW-OCCUPANCY-CHAMPION-X` / `ROW-OCC-H1-BOUNDED-LOGICAL-BLOCKS`.
- Exact code location: host `run_kernel` block-count choice in the V011 runner;
  device `Process` and contiguous `baseRows/extraRows` ownership remain fixed.
- Direct parent: R31B V011, source SHA above.
- Why now: it is the cleanest untested structural axis after row-group
  experiments, with a one-expression OFAT and no kernel arithmetic/DMA/store
  change.
- Relation to current local champion: orthogonal to ASYNC V001; it tests launch
  packet granularity rather than pipeline overlap.
- Expected Core impact: only multi-row regimes where rowCount exceeds physical
  vector-core count; no effect expected on one-row controls.
- Duplicate risk: medium; must remain distinct from SCHED row-group/core
  ownership and must not edit device ownership.
- Correctness risk: low-to-medium, limited to row coverage and block-count
  arithmetic, but all dtypes and paths need correctness.
- Implementation cost: low; host-side formula plus launch-count evidence.
- Expected local upside: low-to-medium and uncertain; intended value is a
  bounded falsification test, not an assumed gain.
- Why better than alternatives: lower code blast radius and clearer causal
  interpretation than another ASYNC/ADDR/REDUCE sibling.

### TOP-2: gamma-only bounded two-row lifetime

- Route / hypothesis: `PARAM-RESIDENCY-CHAMPION-X` / `H1-GAMMA-TWO-ROW-LIFETIME`.
- Exact code location: V011 `ProcessWideFp32FullCacheRows`, gamma staging and
  consumer lifetime; bias remains per-row. Keep existing tile and event forms.
- Direct parent: R31B V011, source SHA above.
- Why now: broad parameter locality is already negative, but the FP32-only
  two-row consumer-lifetime question is narrower and was not isolated by COEFF
  V001-V004.
- Relation to current local champion: independent of ASYNC V001's issue-point
  move; it reduces one repeated gamma consumer load only if the parent really
  reloads it.
- Expected Core impact: FP32 diagnostic path first; it may not affect the four
  Core score cases, so a diagnostic result must not be promoted to a global
  score.
- Duplicate risk: high unless gamma-only, exactly two rows, fixed tile and no
  prefetch timing change are enforced. Broad residency is duplicate/rejected.
- Correctness risk: medium-high due to stale gamma or buffer lifetime races.
- Implementation cost: medium; requires exact live-range/UB proof and distinct
  gamma/bias correctness inputs.
- Expected local upside: low-to-medium, proportional to repeated FP32 gamma
  traffic; may be zero if the parent already retains it.
- Why better than alternatives: it extracts one falsifiable parameter-lifetime
  question from an otherwise exhausted parameter-DMA family.

### TOP-3: cross-row Affine-to-ReducePrep wait placement

- Route / hypothesis: `CROSSROW-PIPELINE-CHAMPION-X` /
  `CROSSROW-H5-AFFINE-REDUCEPREP-RELAX`.
- Exact code location: the existing generic/narrow path readiness wait between
  row N affine completion and row N+1 reduction preparation; move one wait only.
- Direct parent: R31B V011, source SHA above.
- Why now: it tests a remaining stage boundary without changing tile, DMA
  command shape, row ownership, store policy or arithmetic.
- Relation to current local champion: orthogonal to ASYNC V001's same-row
  inter-pass parameter prefetch; it is explicitly cross-row and must not reuse
  the ASYNC source diff.
- Expected Core impact: only shapes that naturally produce localRows>=2; one-row
  controls should be unchanged.
- Duplicate risk: medium-high. Interpass H1/H2/H3, ASYNC V002/V004 and any
  store/load overlap must be excluded in the declaration and review.
- Correctness risk: high synchronization/lifetime risk despite unchanged math.
- Implementation cost: medium-high; requires per-dtype live-buffer proof and
  exact event trace.
- Expected local upside: low-to-medium with high information value; a neutral
  result would close a specific cross-row stall hypothesis.
- Why better than alternatives: it probes a structural boundary not represented
  by the current top scores and separates stage admission from DMA aggregation.

## Required gate for any future selection

Planning must select at most one hypothesis. Before implementation, require an
exact source diff, direct parent SHA, duplicate audit, live-buffer/event proof,
and a target shape that naturally reaches the intended path. A numerical score
above 100 on this campaign is not sufficient for `LOCAL_ACCEPTED` or Online.

`MAIN_SELECTED=NONE`
