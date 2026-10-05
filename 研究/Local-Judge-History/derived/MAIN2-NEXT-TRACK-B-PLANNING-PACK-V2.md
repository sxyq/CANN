# MAIN-2 Next Track-B Evidence Audit — V2 — 2026-10-04

```text
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
MODE=READ_ONLY_STATIC_AND_API_IMPLEMENTATION_AUDIT
MAIN_SELECTED=NONE
REVISION_CREATED=NO
KERNEL_CHANGED=NO
BUILD/CORRECTNESS/CANDIDATE_TIMING=NOT_RUN
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

This version audits the three existing hypotheses against the exact V011
source and the installed CANN 8.5.0.alpha002 implementation. It does not select
or authorize any Revision. The registered local-only calibration is in
`MAIN2-FRESH-BASELINE-CALIBRATION-20261004.md`; none of its low-quality
measurements changes the Official Champion.

## H1 — FP16 full-row parameter DMA command coalescing

```text
HYPOTHESIS_ID=MAIN2-TB-PARAM-DMA-COALESCE-D8192-FP16
SOURCE_ID=MAIN2-TB-PARAM-DMA-COALESCE-D8192 from V1, narrowed to one dtype
EXACT_FUNCTION=ProcessFp16FullRowOutputPipelined
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:1004-1014
DISPATCH=Process() lines 201-210; rowWidth==8192 and localRows>1
TARGET_SHAPE=[M,8192], M sufficient for localRows>1
TARGET_DTYPE=FP16 only
```

- **Current behavior:** `kTileElems=4096`, `kCacheElems=8192`. The parameter
  loop issues `Load(gamma)` and `Load(bias)` for each of two 4096-element
  slices: four `DataCopyPad` calls per core before the row loop. The FP16
  `Init` path already sizes `gammaBuf_` and `biasBuf_` to all 8192 elements.
- **Single variable:** replace only this FP16 parameter-load loop with one
  full-row `Load` for gamma and one for bias. Keep bytes, destination layout,
  load phase, math, row ownership, events, waits, and stores unchanged. That
  is 4 → 2 DMA issue calls with the same 32 KiB total gamma+bias traffic.
- **DataCopy feasibility:** `Load` uses `DataCopyExtParams` and
  `DataCopyPad` (`submission.asc:3367-3375`). Each full FP16 vector is 16,384
  bytes, fits the Ext length field, and is 32-byte aligned. No BF16 change is
  included: its current narrow-path staging buffers are only 4096 elements,
  so combining BF16 loads would also require an allocation/layout decision.
- **Hot-path frequency / effect:** one parameter prologue per active block,
  amortized over its `localRows`; no reduction in bytes or per-row work.
  Expected effect is a small reduction in exposed MTE2 issue/setup time, or
  zero if DMA issue is hidden. No percentage or throughput gain is claimed.
- **Duplicate audit:** closely related to COEFF-LOCALITY-X V004 / NH-1
  (generic single-row multi-tile full-row parameter preload) and R014
  parameter-DMA work. This hypothesis stays in the specialized FP16
  multi-row D=8192 path and does not widen `cacheParams` or move loads out of
  the generic output loop, so it is not the identical code change; however,
  both target fewer parameter DMA commands. Require a Planning novelty
  decision before implementation (`RELATED / NOVELTY REVIEW REQUIRED`).
- **Risks:** verify generated V011 path emits four distinct MTE2 issue
  sequences and that the 16 KiB single-row DataCopyPad lowers to a legal,
  non-looped operation. Official case presence for `[M,8192]` is not verified
  by the retained V011 result map.
- **Minimal experiment after Planning selection:** inspect exact-parent
  codegen, then only the FP16 loop above; verify Parent/Candidate output on
  D8192 with `localRows>1`, followed by the formal same-shape event protocol.

```text
EXPECTED_LATENCY_EFFECT=SMALL_OR_ZERO
EXPECTED_THROUGHPUT_EFFECT=SMALL_OR_ZERO; no bytes saved
DUPLICATE_STATUS=RELATED_TO_COEFF-LOCALITY-V004; PLANNING_NOVELTY_REVIEW_REQUIRED
MATURITY=NEEDS_CODEGEN_AND_NOVELTY_REVIEW
```

## H2 — Exact-preserving wide UB-fit search simplification

```text
HYPOTHESIS_ID=MAIN2-TB-WIDE-UB-FIT-SEARCH
EXACT_FUNCTION=ChooseWideFullYRows
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:1293-1329
CALL_SITE=Init lines 60-100; only rowWidth>kCacheElems (8192)
TARGET_SHAPES=existing wide path, D>8192
TARGET_DTYPES=FP32, FP16, BF16
```

- **Current behavior:** each active block calls this selector once from
  `Init`. It searches row counts 8 down to 2, then at most four 512-element
  tile decrements from 4096 toward 2048. Its inputs include the runtime row
  width and per-dtype buffer factors; its outputs determine actual UB
  allocation geometry.
- **Single variable:** alter only the selector representation, and prove the
  returned `(wideFullYRows_, tileElems)` is identical for every valid input.
  Do not change allocation sizes, tile loop, row ownership, or compute.
- **Hot-path frequency / effect:** initialization once per active block, not
  once per row or tile. The bounded scalar work may be inlined/folded by the
  compiler; a material latency or throughput effect is unlikely unless exact
  V011 codegen shows residual dynamic loop/control instructions.
- **Duplicate audit:** TCSU-H1/H2/H3 are the separate fixed-N=2 body-unroll
  pool, whose gate was `UNVERIFIED` because the exact-parent device binary
  could not establish the target loop-control cost. This selector hypothesis
  preserves all body loops and all geometry; it is related to R005/UB sizing,
  but not the TCSU transformation. Do not treat the TCSU result as proof that
  this selector is costly or redundant.
- **Risks / minimal audit:** off-by-one changes can select more rows or a
  different tile and overflow UB. Before any Revision, exhaustively compare
  the original and proposed selector over valid widths, dtypes, and buffer
  factors, then inspect exact V011 device codegen. If codegen is already
  folded or instruction savings are absent, classify `CODEGEN_NOOP`.

```text
EXPECTED_LATENCY_EFFECT=VERY_SMALL_OR_ZERO
EXPECTED_THROUGHPUT_EFFECT=VERY_SMALL_OR_ZERO
DUPLICATE_STATUS=DISTINCT_FROM_TCSU_BODY_UNROLL; RELATED_TO_UB/TILING
MATURITY=NEEDS_EXACT_CODEGEN_AND_EXHAUSTIVE_EQUIVALENCE
```

## H3 — Avoid unused `paramReady` event-pool allocation on resident rows

```text
HYPOTHESIS_ID=MAIN2-TB-NARROWMID-UNUSED-PARAM-EVENT
EXACT_FUNCTION=ProcessNarrowMidOverlap
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:499-619
DISPATCH_SITE=线上结果/R31B/V011/submission.asc:223-240
TARGET_SHAPES=[M,D], localRows>1; preferred 2048<D<4096 (e.g. D=3072)
TARGET_DTYPES=FP16, BF16
```

- **Reachability correction:** the V1 range `128<D<=4096` was too broad.
  D=4096 with `localRows>1` returns through the full-tile batched path. For
  low precision, aligned widths `D<=2048` with `D%16==0` return through the
  contiguous-batch path. A clean first probe is therefore a mid-width such
  as D=3072 and enough rows to make the actual `localRows>1`; the runner must
  verify the dispatch and row count rather than infer it from M alone.
- **Current behavior:** `inputReady`, `paramReady`, and `inputRelease` are
  allocated before `residentParams` is tested. In this function,
  `paramReady` is set/waited only under `!residentParams` (lines 538-542 and
  576-578), but its ID is released unconditionally at line 618. Thus
  multi-row resident-parameter blocks allocate/release an ID they never use.
- **API evidence:** installed CANN 8.5.0.alpha002
  `aarch64-linux/asc/impl/basic_api/kernel_tpipe_impl.h:436-459` implements
  `AllocEventID` using `sff0` to find a free event-pool bit and `sbitset1` to
  mark it; `ReleaseEventID` uses `sbitset0`. This is source-level software
  event-pool bookkeeping, not a `SetFlag`/`WaitFlag` hardware synchronization
  operation. Whether the CCEC-emitted V011 path retains or folds these
  operations is still unverified and must be checked in codegen.
- **Single variable:** allocate and release `paramReady` only when
  `!residentParams`; retain the existing conditional `SetFlag` and `WaitFlag`
  exactly. Keep `inputReady`, `inputRelease`, all DMA order, all event
  directions, math, and stores unchanged.
- **Hot-path frequency / expected effect:** at most one allocation and one
  release per active block; never per row. The multi-row path may avoid the
  event-pool free-bit scan/set/clear, with a small fixed scalar-overhead
  reduction. No throughput gain is assumed before device codegen and timing.
- **Duplicate audit:** distinct from ASYNC-OVERLAP/ASYNC-TRIPLE, which change
  stage overlap or synchronization placement; this does not add/remove/move a
  flag or wait. It does not change COEFF/R014 parameter traffic or residency.
  It remains synchronization-resource adjacent and needs a Main/API review.
- **Risk / minimal audit:** every acquired `paramReady` ID must be released
  exactly once on the matching branch; the ID must not be read when
  `residentParams` is true. Check generated code for the branch and pool
  operations, then verify exact dispatch, all dtype paths, and full
  correctness before any measurement.

```text
EXPECTED_LATENCY_EFFECT=SMALL_FIXED_PER_ACTIVE_BLOCK_OR_ZERO
EXPECTED_THROUGHPUT_EFFECT=SMALL_FIXED_PER_ACTIVE_BLOCK_OR_ZERO
DUPLICATE_STATUS=NOT_THE_ASYNC_OVERLAP_MECHANISM; EVENT-POOL RESOURCE REVIEW
MATURITY=API IMPLEMENTATION CONFIRMED; NEEDS CODEGEN/CORRECTNESS EVIDENCE
```

## Ranking for Planning review (not selection)

1. **H1 — FP16 D8192 DMA coalescing:** clearest possible reduction in issued
   copies (4 → 2), but the closest duplicate to COEFF V004; novelty decision
   is a hard gate.
2. **H3 — unused event allocation:** the exact TPipe implementation confirms
   real event-pool bookkeeping and the reachability domain is now precise;
   expected ceiling is only a few scalar operations per block.
3. **H2 — UB-fit selector:** mathematically bounded and isolated from compute,
   but initialization-only and has no exact-parent codegen evidence; likely
   below noise.

```text
MAIN_SELECTED=NONE
IMPLEMENTATION_AUTHORIZED=NO
NEW_REVISION_CREATED=NO
OFFICIAL_SCORE_CHANGED=NO
```
