# MAIN-1 Wave-2 算术路线研究交接

- ROUTE: `EPI-ARITH-CHAMPION-W2-X`
- BRANCH / WORKTREE: `w2/m1/epi-arith` / `/Users/sunyiyang/.codex/worktrees/w2-m1-epi/cann`
- TRACK: `B only`; `MAIN_SELECTED=NO`; no Revision created
- DIRECT_PARENT: `R31B V011`, Official `45.16`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- DATE: `2026-09-30`

## Evidence

- Parent identity and score: `线上结果/R31B/V011/source-meta.json`, `result.json`; affine code at `submission.asc:2194-2207`.
- H1 NORM-HOIST already became EPILOGUE-ARITH V001: `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V001.md`.
- H2 GAMMA-FIRST-AXPY already became V002: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/diff.patch`, `local-result.json`; Official `44.96`, 15/15, below parent: `result.json`, `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V002.md`.
- Cross-route NORM-HOIST: `线上结果/R31A/V024/diff.patch`, `source-meta.json`.
- Prior affine fusion and SCALE-FOLD outcomes: `研究/EPILOGUE-FUSE-X/MAIN-APPROVAL-V003.md`, `TRACK-B-HYPOTHESES-V003.md`, `BOTTLENECK-NOTE.md`, `TRACK-B-HYPOTHESES.md`.

V001/V002 are historical evidence, not new candidates. Any selected direction must pass precision validation before local timing. No build, precision run, timing, or server3 access occurred in this research turn.

## H3: Group Independent Row Arithmetic

- HYPOTHESIS_ID: `EAW2-H3-ROW-OP-GROUP`
- MECHANISM: Per parameter tile, issue `Mul` for all rows, one `PIPE_V` barrier, then `Add` for all rows and one barrier. Per-element order remains `Muls -> Mul -> Add`.
- BOTTLENECK: With `batchRows=2`, the current Mul/Add loops use four barriers per tile; grouping uses two. Vector op count and elements stay unchanged.
- DIRECT_PARENT: `R31B V011`, SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- TARGET_SHAPES: FP32, `rowWidth > 8192`, and `rowCount >= 2 * availableCoreNum` so a core can receive at least two rows; e.g. `80x16384_fp32` when 40 cores are available. Confirm launch dispatch and `batchRows` first. `1x32768_fp32` is an inactive control.
- TARGET_DTYPES: `FP32` only; no dtype branch added.
- WHY_IT_MAY_HELP: Fewer waits between independent rows while retaining shared parameter loads.
- WHY_IT_MAY_FAIL: Wait cost may be hidden; Store or other work may dominate, leaving any gain inside measurement noise.
- UB_IMPACT: None; existing value/gamma/bias buffers only.
- DMA_IMPACT: None; Load order and count unchanged.
- SYNC_IMPACT: Mul/Add barriers fall from `2 * batchRows` to 2 per tile; Muls and Store synchronization stay unchanged.
- PRECISION_RISK: Low; each element performs the same operations in the same order. Validate all shapes against V011 first.
- DUPLICATE_CHECK: Distinct from V001's row-wide Muls and V002's AXPY. The adjacent negative evidence is instruction reshuffling in `研究/EPILOGUE-FUSE-X/BOTTLENECK-NOTE.md`; it cautions that the gain may be too small, not that this exact grouping was tested.
- RELATED_OLD_ROUTES: EPILOGUE-ARITH V001/V002, R31A V024, EPILOGUE-FUSE-X.
- PROPOSED_ONE_FACTOR_DIFF: Only regroup the two second-pass loops at `线上结果/R31B/V011/submission.asc:2200-2207`; leave Muls, Load, Store, dispatch, and buffers unchanged.
- MINIMAL_EXPERIMENT: After Main selection, run full precision validation first. Then paired, interleaved V011/candidate timing on one active wide-FP32 shape with `batchRows>=2`; use the same width with `batchRows=1` as control. Stop if dispatch misses, precision fails, or gain stays inside that shape's noise.
- UNCERTAINTY: The active shape depends on `availableCoreNum`, row distribution, and `ChooseWideFullYRows`; no hardware timing evidence exists for this grouping.

## H4: Fuse Gamma Multiply and Bias Add

- HYPOTHESIS_ID: `EAW2-H4-VMLA-ONE-PASS`
- MECHANISM: On a single-row tile, keep norm Muls and replace `Mul(valueRow, gammaLocal)` plus `Add(valueRow, biasLocal)` with `MulAddDst(biasLocal, valueRow, gammaLocal)`.
- BOTTLENECK: Potentially removes one vector issue and value-tile writeback.
- DIRECT_PARENT: `R31B V011`, SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- TARGET_SHAPES: Single-row wide FP32, such as `1x16384_fp32` and `1x32768_fp32`.
- TARGET_DTYPES: `FP32`.
- WHY_IT_MAY_HELP: Bias is single-use in the tile, so it can act as the fused accumulator without a reload.
- WHY_IT_MAY_FAIL: Output then resides in `biasLocal`, changing the Store source; copying back to preserve it can erase the pass reduction. Fused rounding differs from separate Mul/Add.
- UB_IMPACT: No new allocation; overwrites the existing bias tile.
- DMA_IMPACT: Input DMA unchanged; Store source operand changes in the direct form.
- SYNC_IMPACT: No added barrier in the direct form; copy-back would add work and synchronization.
- PRECISION_RISK: Medium; FP32 fused rounding is not guaranteed to match two separate roundings.
- DUPLICATE_CHECK: Same dataflow as `VMLA-INPLACE-UNCACHED` in `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-V003.md` and `MAIN-APPROVAL-V003.md`; it also changes the Store source, outside this task's allowed scope.
- RELATED_OLD_ROUTES: EPILOGUE-FUSE-X V002/V003; EPILOGUE-ARITH V002 uses a different `Mul + Axpy` order.
- PROPOSED_ONE_FACTOR_DIFF: Replace only Mul/Add with `MulAddDst` into bias; Store source must change, so this diff is not eligible here.
- MINIMAL_EXPERIMENT: No Revision recommended. If separately authorized, precision validation must precede active-shape timing; this task does not run it.
- UNCERTAINTY: V003 notes describe both direct Store from bias and copy-back variants; without its raw measurement package, do not merge their outcomes.

## H5: Fold invRms into the Gamma Tile

- HYPOTHESIS_ID: `EAW2-H5-SCALE-GAMMA-TILE`
- MECHANISM: For `batchRows=1`, scale `gammaLocal` by `invRms`, then multiply `valueRow` by that tile; keep bias Add unchanged.
- BOTTLENECK: Removes one Muls write from the value tile, but keeps the total vector-op count and moves the work to gamma.
- DIRECT_PARENT: `R31B V011`, SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- TARGET_SHAPES: Single-row wide FP32, such as `1x16384_fp32` and `1x32768_fp32`; not shared-panel batches with distinct per-row `invRms`.
- TARGET_DTYPES: `FP32`.
- WHY_IT_MAY_HELP: May shorten the value-tile dependency path if its writes are limiting.
- WHY_IT_MAY_FAIL: Same V-op count; both tiles use the V pipe. `(y*invRms)*gamma` becomes `y*(invRms*gamma)`, changing intermediate rounding. Multi-row batches need separate scaled gamma tiles.
- UB_IMPACT: None for one row using `gammaLocal` in place; multi-row form would need extra storage or repeated work.
- DMA_IMPACT: None for one row; parameter reload is excluded.
- SYNC_IMPACT: Barrier count unchanged; target buffer changes.
- PRECISION_RISK: Medium. The proposed form does not meet a no-extra-risk claim; verify all shapes before timing.
- DUPLICATE_CHECK: Same mechanism as SCALE-FOLD in `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES.md`; `BOTTLENECK-NOTE.md` records its result inside roughly ±5% noise. Do not resubmit it as a new route direction.
- RELATED_OLD_ROUTES: EPILOGUE-FUSE-X SCALE-FOLD; EPILOGUE-ARITH V001/V002.
- PROPOSED_ONE_FACTOR_DIFF: Replace only `Muls(valueRow, valueRow, invRms)` with `Muls(gammaLocal, gammaLocal, invRms)`; retain Mul/Add/Store order.
- MINIMAL_EXPERIMENT: Not recommended for a new Revision. If Main requests a boundary probe, validate precision first, then pair V011/candidate on `1x32768_fp32`; stop if the result is within shape noise.
- UNCERTAINTY: The old noise figure is from another route/window and is not a noise estimate for this worktree's future run.

## Main Recommendation

Only H3 is recommended for selection. It preserves per-element arithmetic and has no direct duplicate, but its expected gain is unproven and depends on a real `batchRows>=2` case. H4 and H5 are distinct mechanisms but duplicate prior routes and are not recommended. Wait for `MAIN_SELECTED=YES` before implementation.
