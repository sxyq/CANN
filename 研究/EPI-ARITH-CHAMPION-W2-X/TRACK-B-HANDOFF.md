# EPI-ARITH-CHAMPION-W2-X Track-B Handoff

- BRANCH / WORKTREE: `w2/m1/epi-arith` / `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/epi-arith`
- DIRECT_PARENT: R31B V011, Official 45.16, source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- STATUS: H3 selected by Planning and confirmed by Main-1; V001 declaration recorded; Candidate awaits its first source commit
- MAIN_SELECTED: `YES`
- SELECTED_HYPOTHESIS: `H3 EAW2-H3-ROW-OP-GROUP`
- SELECTION_EVIDENCE: `研究/主代理/MAIN-1-W2/campaign-status.md` at commit `2a27be0bbaa81b7f67777f7d8e99277cbe23946a`, section `Current lane instructions and Main review`
- CHILD_RECOMMENDED_HYPOTHESIS: H3, as selected by Planning and confirmed by Main-1
- REVISION_DECLARATION: `研究/EPI-ARITH-CHAMPION-W2-X/V001-REVISION-DECLARATION.md`

## ROUTE_BOUNDARY

Arithmetic after `invRms` and before Store: norm, gamma, bias, AXPY, Muls/Mul/Add, and ordering of independent arithmetic operations. Wave-2 peer boundaries below are sourced from Main-1's committed review; no peer worktree or private context was read.

## ALLOWED_CHANGES

One-factor changes confined to the post-`invRms` arithmetic expression or issue order. Preserve each element's arithmetic order unless a separately stated precision experiment is authorized. Keep row ownership, dispatch, buffers, and Store behavior unchanged.

## FORBIDDEN_CHANGES

Store transaction or source, DMA, dispatch/fast-path selection, tiling, row-to-core assignment, reduction organization, dtype specialization, build files, shared records, other routes, Online activity, and compile/correctness/performance work before Main assigns a device and job.

## Duplicate Rejections

| Candidate | Status and similar mechanism | Actual difference | Replacement search | Evidence paths |
|---|---|---|---|---|
| H1 NORM-HOIST | `DUPLICATE_REJECTED`. Move per-tile invRms Muls to one row-wide Muls before the parameter loop. | None at mechanism level: Wave-1 EPI V001 is the same transform on the same R31B V011 source. R31A V024 applies the same hoist in its CachedRows function. | Per-tile Muls barrier grouping was considered, but it belongs to H3's arithmetic scheduling axis; no second independent norm candidate found. | `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V001.md`; `线上结果/EPILOGUE-ARITH-CHAMPION-X/V001/submission.asc`; `线上结果/R31A/V024/diff.patch`, `source-meta.json` |
| H2 GAMMA-FIRST-AXPY | `DUPLICATE_REJECTED`. `Mul(y,gamma)` followed by `Axpy(bias, value, invRms)`. | None at mechanism level: Wave-1 EPI V002 already uses this exact gamma-first order. It passed 15/15 Official but scored 44.96, below 45.16; its local signal did not transfer. | Keeping the original norm-first order while replacing Mul/Add with AXPY is a gamma/bias fused operation and falls into the VMLA mechanism below. No distinct replacement found. | `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/diff.patch`, `local-result.json`, `result.json`; `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V002.md` |
| H4 VMLA | `DUPLICATE_REJECTED`. Accumulate `norm*gamma` into the bias tile with `MulAddDst`. | The proposed target would be the wide full-y function; Wave-1 EPILOGUE-FUSE V003 targeted uncached arms. The arithmetic/dataflow mechanism is still the same; writing from bias also changes the Store source, which is forbidden here. | Folding invRms into gamma is H5/SCALE-FOLD, not a new VMLA replacement. No non-repeated fusion found. | `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-V003.md`, `MAIN-APPROVAL-V003.md`, `BOTTLENECK-NOTE.md` |
| H5 SCALE-FOLD | `DUPLICATE_REJECTED`. Scale gamma by invRms, then multiply the value tile. | A single-row wide-FP32 target would differ by path and shape from Wave-1's general proposal, but not by expression transform; product association and rounding also change. Prior evidence was within about ±5% noise. | Combining gamma and bias with VMLA returns to H4. No exact-rounding, fewer-pass replacement found with the current operation set. | `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES.md`, `BOTTLENECK-NOTE.md`, `MAIN-APPROVAL-V002.md` |

## H3: Group Independent Row Arithmetic

- HYPOTHESIS_ID: `EAW2-H3-ROW-OP-GROUP`
- MECHANISM: For each current parameter tile, issue Mul for all resident rows, one `PIPE_V` barrier, then Add for all rows and one barrier. Per-element order remains `Muls -> Mul -> Add`.
- BOTTLENECK: With `batchRows=2`, the current Mul/Add loops use four barriers per tile; grouped issue uses two. Vector-op count and elements are unchanged.
- DIRECT_PARENT: R31B V011, source SHA above.
- TARGET_SHAPES: `rows=2, D=12288, FP32, blockCount=1`; record runtime `batchRows` before interpreting results. `D=8192` is an unchanged-path control and must not receive this arithmetic edit.
- TARGET_DTYPES: FP32 only; no dtype branch.
- WHY_IT_MAY_HELP: Fewer waits between independent rows while retaining the shared gamma/bias loads.
- WHY_IT_MAY_FAIL: The device may hide these waits; Store or another stage may dominate. R31A V028 also reduced affine barriers, so the remaining opportunity may be small.
- UB_IMPACT: None; existing value/gamma/bias buffers only.
- DMA_IMPACT: None; Load count and order unchanged.
- SYNC_IMPACT: Mul/Add barriers change from `2 * batchRows` to 2 per tile; Muls and Store synchronization stay unchanged.
- PRECISION_RISK: Low; each element executes the same operations in the same order. Precision validation must precede timing.
- DUPLICATE_CHECK: Not the V001 row-wide Muls hoist. R31A V028 removes terminal barriers immediately before events while retaining row-wise arithmetic; H3 groups Mul and Add across resident rows and retains one barrier after each operation group. Main confirms Wave-2 SYNC H2 is limited to low-precision execution, so it does not overlap this wide-FP32 loop.
- RELATED_OLD_ROUTES: R001-R029 / FULL-R, R31A V024/V028, R31B V011, MIX-A V003, Wave-1 EPI V001/V002 and STORE V003.
- PROPOSED_ONE_FACTOR_DIFF: Only regroup the second-pass Mul/Add loops in `ProcessWideFp32FullCacheRows` in the V011 parent. Preserve per-element `Muls -> Mul -> Add`; leave Muls, Load, Store, dispatch, buffers, and row ownership unchanged.
- MINIMAL_EXPERIMENT: After Main assigns the device and job, compile/link the exact source, then run precision validation before timing. Use `rows=2,D=12288,FP32,blockCount=1` as target and `D=8192` as an unchanged-path control; record runtime `batchRows`, path, and block count. Time only after correctness passes, using paired/interleaved V011 and candidate runs on the target.
- UNCERTAINTY: The target is expected to produce `batchRows=2`, but this must be observed in the assigned run. No correctness or timing evidence exists for the grouped issue order.

## Route Comparisons

| Family / Wave-2 lane | Similarity | Boundary difference and evidence |
|---|---|---|
| R001-R029 and FULL-R | R002 resident-y, R014 parameter residency, R017 FP32 intermediates, and R019/R020 RMS math touch neighboring data or arithmetic. | R019/R020 change invRms production; R002/R014 change residency; R017 is the precision policy. None is H3's post-invRms Mul/Add issue grouping. R028 batches RMS scalar handoff and is before this route's boundary. `技术路线/全项目成绩与技术路线盘点.md:274-346` |
| R31A / R31B | R31A V024 duplicates H1. R31A V028 is adjacent to H3 on barrier cost. R31B V011 is the exact parent. | V028 removes two barriers immediately before event setup in `ProcessWideFp32Batched`; H3 retains the final barrier and groups Mul/Add across rows in `ProcessWideFp32FullCacheRows`. V028's Official 44.07 was below R31A V016 45.00. `线上结果/R31A/V024/diff.patch`, `线上结果/R31A/V028/diff.patch`, `source-meta.json`, `线上结果/R31B/V011/submission.asc` |
| MIX / MIX-A | MIX-A V003 combines a narrow/mid FastKernel dispatch guard (`localRows==1`) with `SyncVToMTE2`; this is relevant precedent for FASTPATH and SYNC ownership. | It changes dispatch and MTE2 reuse in `ProcessNarrowMidFast`, not the wide-FP32 post-invRms affine chain. Its audit classifies the change as multi-change. `线上结果/MIX-A/V003/diff.patch`, `source-meta.json` |
| Wave-1 EPI / STORE | EPI V001/V002 are the rejected arithmetic duplicates above. STORE V003 merges output writebacks and reports 44.38 Official. | STORE changes output writeback count; H3 leaves Store source, calls, and events unchanged. `研究/主代理/MAIN-1/campaign-status.md`, `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-STORE-V003.md`, `线上结果/STORE-EPILOGUE-X/V003/` |
| Wave-2 STORE | Same kernel and wide-row area. | Committed STORE spec owns intra-row writeback merging; H3 changes only arithmetic issue grouping and leaves output transactions intact. H4 is excluded because its direct form changes the Store source. `研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md` |
| Wave-2 FASTPATH | MIX-A V003 is a historical dispatch/FastKernel analog. | FASTPATH H1 qualifies a BF16 D32768 donor; H3 does not change dispatch and targets only the wide-FP32 arithmetic loop at D12288. `研究/主代理/MAIN-1-W2/campaign-status.md` at `2a27be0b`; `线上结果/MIX-A/V003/diff.patch` |
| Wave-2 SYNC | H3 changes `PIPE_V` barrier grouping; R31A V028 and MIX-A V003 show prior barrier/event work. | SYNC H2 is confined to low-precision parameter-prefetch order. H3 retains a barrier after each arithmetic group and does not change MTE events; Main confirms the execution paths do not overlap. `研究/主代理/MAIN-1-W2/campaign-status.md` at `2a27be0b`; `线上结果/R31A/V028/diff.patch`; `线上结果/MIX-A/V003/diff.patch` |
| Wave-2 SMALLMID | MIX-A V003 and historical mid paths use narrow/mid dispatch. | SMALLMID SMD-H6 reuses BF16 parameter conversion for D<=4096 conversion work; H3 is wide FP32 at D12288 and does not touch those functions. `研究/主代理/MAIN-1-W2/campaign-status.md` at `2a27be0b`; `线上结果/MIX-A/V003/diff.patch` |

## EXPECTED_LOCAL_PROBES

No probe was run. Main has selected H3. After Main assigns a device and job: record actual `blockCount`, execution path, and `batchRows` for `rows=2,D=12288,FP32`; run precision validation before any timing. Confirm D8192 stays on the unchanged path. If correctness passes, perform paired/interleaved V011-candidate timing for the target and report its noise; do not infer an arithmetic effect from the D8192 control.

## OPEN_QUESTIONS

- When will Main assign a device and job for compile, precision validation, and timing?
- Does the assigned runtime report `batchRows=2` for the target; if not, does the grouped loop still exercise more than one resident row?
- What timing noise does the target shape show under the approved paired/interleaved protocol?

## ROUTE_HYPOTHESIS_POOL_EXHAUSTED

ROUTE_HYPOTHESIS_POOL_EXHAUSTED: `YES`
VALID_NON_DUPLICATE_CANDIDATES: `1` (H3 selected; H1/H2/H4/H5 were explicitly rejected above)

H1/H2/H4/H5 are `DUPLICATE_REJECTED`. Replacement searches map back to the same four transforms (row-wide norm hoist, gamma-first AXPY, VMLA, SCALE-FOLD); exact-rounding fusion was not found, and additional barrier regrouping would be a variant of H3. Claiming three candidates would repeat known mechanisms or split one scheduling idea into labels.
