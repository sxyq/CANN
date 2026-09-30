# EPI-ARITH-CHAMPION-W2-X Track-B Handoff

- BRANCH / WORKTREE: `w2/m1/epi-arith` / `/Users/sunyiyang/.codex/worktrees/w2-m1-epi/cann`
- DIRECT_PARENT: R31B V011, Official 45.16, source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- STATUS: Track-B only; no implementation direction selected; no Revision created
- CHILD_RECOMMENDED_HYPOTHESIS: retain H3 for Main review only; selection remains with Main

## ROUTE_BOUNDARY

Arithmetic after `invRms` and before Store: norm, gamma, bias, AXPY, Muls/Mul/Add, and ordering of independent arithmetic operations. The Wave-2 peer labels below are taken from the task request; peer-private contexts and other worktrees were not read.

## ALLOWED_CHANGES

One-factor changes confined to the post-`invRms` arithmetic expression or issue order. Preserve each element's arithmetic order unless a separately stated precision experiment is authorized. Keep row ownership, dispatch, buffers, and Store behavior unchanged.

## FORBIDDEN_CHANGES

Store transaction or source, DMA, dispatch/fast-path selection, tiling, row-to-core assignment, reduction organization, dtype specialization, build files, and any implementation or measurement before Main selection.

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
- TARGET_SHAPES: FP32, `rowWidth > 8192`, with `rowCount >= 2 * availableCoreNum` and actual `batchRows >= 2`; example `80x16384_fp32` only when 40 cores are available. `1x32768_fp32` is an inactive control.
- TARGET_DTYPES: FP32 only; no dtype branch.
- WHY_IT_MAY_HELP: Fewer waits between independent rows while retaining the shared gamma/bias loads.
- WHY_IT_MAY_FAIL: The device may hide these waits; Store or another stage may dominate. R31A V028 also reduced affine barriers, so the remaining opportunity may be small.
- UB_IMPACT: None; existing value/gamma/bias buffers only.
- DMA_IMPACT: None; Load count and order unchanged.
- SYNC_IMPACT: Mul/Add barriers change from `2 * batchRows` to 2 per tile; Muls and Store synchronization stay unchanged.
- PRECISION_RISK: Low; each element executes the same operations in the same order. Precision validation must precede timing.
- DUPLICATE_CHECK: Not the V001 row-wide Muls hoist. R31A V028 removes terminal barriers immediately before events while retaining row-wise arithmetic; H3 groups Mul and Add across resident rows and retains one barrier after each operation group. Possible Wave-2 SYNC overlap remains open.
- RELATED_OLD_ROUTES: R001-R029 / FULL-R, R31A V024/V028, R31B V011, MIX-A V003, Wave-1 EPI V001/V002 and STORE V003.
- PROPOSED_ONE_FACTOR_DIFF: Only regroup the second-pass Mul/Add loops at `线上结果/R31B/V011/submission.asc:2200-2207`; do not change Muls, Load, Store, dispatch, buffers, or row ownership.
- MINIMAL_EXPERIMENT: After Main selects H3 and authorizes implementation, first run full precision validation. Then use paired, interleaved V011/candidate timing on one confirmed `batchRows>=2` wide-FP32 shape and the same width with `batchRows=1` as control. Stop if dispatch misses, precision fails, or gain stays within that shape's measured noise.
- UNCERTAINTY: Active shape depends on launch core count and row distribution; no timing evidence exists for this exact grouping.

## Route Comparisons

| Family / Wave-2 lane | Similarity | Boundary difference and evidence |
|---|---|---|
| R001-R029 and FULL-R | R002 resident-y, R014 parameter residency, R017 FP32 intermediates, and R019/R020 RMS math touch neighboring data or arithmetic. | R019/R020 change invRms production; R002/R014 change residency; R017 is the precision policy. None is H3's post-invRms Mul/Add issue grouping. R028 batches RMS scalar handoff and is before this route's boundary. `技术路线/全项目成绩与技术路线盘点.md:274-346` |
| R31A / R31B | R31A V024 duplicates H1. R31A V028 is adjacent to H3 on barrier cost. R31B V011 is the exact parent. | V028 removes two barriers immediately before event setup in `ProcessWideFp32Batched`; H3 retains the final barrier and groups Mul/Add across rows in `ProcessWideFp32FullCacheRows`. V028's Official 44.07 was below R31A V016 45.00. `线上结果/R31A/V024/diff.patch`, `线上结果/R31A/V028/diff.patch`, `source-meta.json`, `线上结果/R31B/V011/submission.asc` |
| MIX / MIX-A | MIX-A V003 combines a narrow/mid FastKernel dispatch guard (`localRows==1`) with `SyncVToMTE2`; this is relevant precedent for FASTPATH and SYNC ownership. | It changes dispatch and MTE2 reuse in `ProcessNarrowMidFast`, not the wide-FP32 post-invRms affine chain. Its audit classifies the change as multi-change. `线上结果/MIX-A/V003/diff.patch`, `source-meta.json` |
| Wave-1 EPI / STORE | EPI V001/V002 are the rejected arithmetic duplicates above. STORE V003 merges output writebacks and reports 44.38 Official. | STORE changes output writeback count; H3 leaves Store source, calls, and events unchanged. `研究/主代理/MAIN-1/campaign-status.md`, `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-STORE-V003.md`, `线上结果/STORE-EPILOGUE-X/V003/` |
| Wave-2 STORE | Same kernel and wide-row area. | Committed STORE spec owns intra-row writeback merging; H3 changes only arithmetic issue grouping and leaves output transactions intact. H4 is excluded because its direct form changes the Store source. `研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md` |
| Wave-2 FASTPATH | MIX-A V003 is a historical dispatch/FastKernel analog. | H3 does not change dispatch and only targets wide FP32. No FASTPATH-specific Wave-2 handoff is present in this branch's committed records, so exact peer boundary is unconfirmed. `线上结果/MIX-A/V003/diff.patch`; `调度/当前任务.tsv`; `调度/主代理分工.md` |
| Wave-2 SYNC | H3 changes `PIPE_V` barrier grouping; R31A V028 and MIX-A V003 show prior barrier/event work. | H3 retains the barrier after each operation group and does not alter MTE events, but may overlap a SYNC lane whose approved scope includes these `PIPE_V` barriers. No SYNC-specific Wave-2 handoff is present in this branch. `线上结果/R31A/V028/diff.patch`; `线上结果/MIX-A/V003/diff.patch` |
| Wave-2 SMALLMID | MIX-A V003 and historical mid paths use narrow/mid dispatch. | H3 is limited to `rowWidth>8192` wide FP32 and does not touch small/mid functions. The exact SMALLMID cutoff and approved changes are not present in this branch. `线上结果/MIX-A/V003/diff.patch`; `调度/当前任务.tsv` |

## EXPECTED_LOCAL_PROBES

No probe was run. After Main selection only: confirm actual launch core count and `batchRows`; precision-check all required shapes first; then paired/interleaved parent-candidate timing on an active wide-FP32 multi-row shape. Use identical `rowWidth` with `batchRows=1` as a zero-effect control and calculate noise independently for each shape.

## OPEN_QUESTIONS

- Does the Wave-2 SYNC lane own the `PIPE_V` barriers in this affine loop? Until answered, H3 is research-only.
- Which official/local shape has `rowWidth>8192` and `batchRows>=2` with the actual launch core count?
- What are the approved mechanism boundaries for Wave-2 FASTPATH and SMALLMID? Their peer-specific briefs are absent from this branch; no other worktree or private context was consulted.
- Does grouping barriers across rows add anything beyond R31A V028's terminal barrier removal?

## ROUTE_HYPOTHESIS_POOL_EXHAUSTED

ROUTE_HYPOTHESIS_POOL_EXHAUSTED: `YES`
VALID_NON_DUPLICATE_CANDIDATES: `1` (H3 retained for Main review; no implementation selection)

H1/H2/H4/H5 are `DUPLICATE_REJECTED`. Replacement searches map back to the same four transforms (row-wide norm hoist, gamma-first AXPY, VMLA, SCALE-FOLD); exact-rounding fusion was not found, and additional barrier regrouping would be a variant of H3 with unresolved SYNC overlap. Claiming three candidates would repeat known mechanisms or split one scheduling idea into labels.
