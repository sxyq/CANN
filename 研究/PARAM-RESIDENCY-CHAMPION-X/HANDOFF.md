# PARAM-RESIDENCY-CHAMPION-X Track-B Handoff

ROUTE=PARAM-RESIDENCY-CHAMPION-X
ROUTE_BOUNDARY=Only gamma/bias residency across rows or row-groups. Exclude row-to-core mapping, buffer lifetime changes unrelated to parameter residency, and interpass/crossrow scheduling.
MAIN_SELECTED=NONE
POOL_STATUS=ROUTE_HYPOTHESIS_POOL_EXHAUSTED
VALID_HYPOTHESIS_COUNT=1 (provisional; cache-policy support is unconfirmed)

## Route Scope

DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL=45.16, 15/15

ALLOWED_CHANGES=Track-B research only. A future one-factor source change may affect only the supported cache policy of gamma/bias GM reads, and only after Planning selects it. Keep row ownership, batch geometry, buffer allocation and lifetime, arithmetic, and interpass order unchanged.
FORBIDDEN_CHANGES=No Candidate or Revision edits; no row-to-core mapping; no unrelated buffer lifetime changes; no interpass/crossrow scheduling; no edits to kernel, .asc/.cpp/.h/.hpp, CMake, runner, wrapper, shared schedule, shared score/history tables, or another route.

## Parent Evidence

The committed V011 sidecar and the computed digest of `线上结果/R31B/V011/submission.asc` both equal PARENT_SOURCE_SHA above. `result.json` binds that source to submission `6ab2c10c0304f72a56a0c5cb`, Pass, 15/15, Official 45.16. V011 already loads full gamma/bias once per core for eligible generic multi-row rows; its wide FP32 full-y path reloads per parameter tile per batch, while wide low-precision has two-slot per-tile parameter prefetch.

The likely remaining residency window is therefore limited to D>8192 cases where a core executes more than one wide row batch. The visible COEFF H2 evidence says its tested launch shapes had `localRows=1`; its wide FP32 stripe also needed 32 KiB for one gamma+bias tile while only about 16 KiB remained at the tested tile width.

## Hypothesis Pool

### PARAM-L2-ROWGROUP-RETENTION-01

HYPOTHESIS_ID=PARAM-L2-ROWGROUP-RETENTION-01
VALIDITY=PROVISIONAL; requires confirmation that the target CANN/MTE2 path exposes a cache-retention policy for gamma/bias GM reads.
MECHANISM=If supported, apply one hardware-supported cache-retention policy only to repeated gamma/bias GM reads so their lines can remain resident between existing row-batch iterations. Do not copy them into a new UB stripe or change the loop order.
BOTTLENECK=Repeated wide-path MTE2 reads of row-invariant gamma/bias when `localRows > batchLimit`; the hypothesis is relevant only if those reads miss the useful device cache.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=D>8192, primarily D=16384 or 32768, with rowCount large enough that the existing launch gives `localRows > batchLimit`; include one case with a single batch as a neutral control.
TARGET_DTYPES=FP32, FP16, BF16, limited to types for which the documented cache policy applies to the existing MTE2 load.
WHY_IT_MAY_HELP=Would test cross-row-group residency without consuming the UB bytes that prevented the COEFF H2 stripe. It may reduce lower-memory traffic while leaving row work and ownership unchanged.
WHY_IT_MAY_FAIL=The vector MTE2 path may not accept such a policy; a hint may be ignored; gamma/bias lines may already hit cache; input/output traffic may evict them; and the `Load` commands remain, so this cannot help if descriptor or MTE2 issue cost dominates.
UB_IMPACT=No new UB allocation or tile-width reduction.
DMA_IMPACT=No transfer-command reduction. Only source-memory traffic may fall if the policy is supported and lines are reused.
SYNC_IMPACT=No intended event or ordering change.
PRECISION_RISK=No arithmetic change expected. Address alignment, cache-policy legality, and unchanged synchronization still require future correctness evidence.
DUPLICATE_CHECK=Distinct storage level from R014 and COEFF H2's UB-resident vectors/stripes; no committed evidence found for an L2 retention policy on these gamma/bias MTE2 reads. Keep provisional until the exact toolchain/device behavior is documented.
RELATED_OLD_ROUTES=R014; FULL-R014; FULL-R030; R31B V011; R31A V026; MIX-R014-R002-R019; MIX-A; BATCH-RESIDENT-X; COEFF-LOCALITY-X; EPI-X-FRESH; ASYNC-OVERLAP-CHAMPION-X.
MINIMAL_EXPERIMENT=First, read-only confirmation from committed SDK/device documentation that the exact GlobalTensor-to-UB MTE2 operation supports cache retention with the required scope. If confirmed and later selected by Planning, make one cache-policy-only change to gamma/bias loads in the existing wide path; leave every buffer, tile, row assignment, loop, and arithmetic operation unchanged. Then run the project's ordinary correctness and shape-qualified paired local protocol. No part of this experiment was run in this handoff.

## Duplicate Audit

| Evidence line | Similar mechanism | Actual difference | Committed evidence | Would a separate experiment add value? |
|---|---|---|---|---|
| R001-R029 / R014 | Full gamma/bias vectors are loaded into UB once per core and reused across rows. | R014 targets widths up to 4096 and uses existing x/residual UB space; the provisional H-L2 targets repeated wide-path reads without a UB copy. | `技术路线/技术路线总表.md`; `归档/phase3-before-reset-20260920/提交/外部轨道-ChatGPT编译并修复/源码/R014-V001/结果.md`; same directory `R014-V002/结果.md` and `R014-V001/kernel.txt`. | Repeating the UB-cache experiment is DUPLICATE_REJECTED. A different cache level could add evidence for D>UB only if MTE2 support is confirmed. |
| FULL-R014 | Parameter-residency architecture, with full parameter staging across rows. | The upstream FULL route is a separate implementation and its retained result is Runtime Error; its mechanism still duplicates UB residency. | `归档/phase3-before-reset-20260920/实验/online-independent/FULL-R014-PARAMETER-RESIDENCY-ARCH/I001/6aaed3c3b0477ec41e17d6cc/kernel.txt` and `result.json`; `技术路线/技术路线总表.md`. | Reimplementing UB residency is DUPLICATE_REJECTED. A documented cache-policy path could avoid its capacity cost, but needs API evidence first. |
| FULL-R030 / R030 | Wide-row gamma/bias are held in UB and reused across a core's rows. | The wide FP16/BF16 submission returned Runtime Error at case 5 (4/15); it also retains y and changes two-pass dataflow. | `归档/phase3-before-reset-20260920/实验/online/FULL-R030-WIDE-PARAM-REUSE/V001/6aad8c4db0477ec41e864499/result.md`; sibling `6aae421ab0477ec41ecde49f/result.md`; corresponding `kernel.txt`; `技术路线/技术路线总表.md`. | Another UB wide-cache variant is DUPLICATE_REJECTED. An L2-level test could isolate a different storage level and avoid adding UB, conditional on API support. |
| MIX / H002 / MIX-A | R014 parameter caching is inherited or combined with other arithmetic/dataflow changes; A001's FastKernel also loads parameters once per core. | The mixes include R002/R019 or dispatch/dataflow changes, so they do not isolate cache policy. H002's single-point profile was about 5-6% and not a Champion result; MIX-A V001 failed 14/15. | `归档/phase3-before-reset-20260920/提交/混合方案/H002-性能组合/MIX-R014-R002-R019-V001/结果.md`; `技术路线/技术路线总表.md`; `技术路线/全版本记录.tsv`. | Repeating their parameter-copy strategy is DUPLICATE_REJECTED. A cache-level-only change could isolate a different cause, if supported. |
| R31B V011 | Generic paths already cache gamma/bias once per core when `localRows > 1`; wide low-precision already prefetches parameter tiles. | H-L2 is limited to repeated existing wide batches and does not add a staging pair or reorder loads. | `线上结果/R31B/V011/submission.asc:245`; `:2082`; `:2191`; `:3076`; `:3261`; `:3295`; `线上结果/R31B/V011/submission.sha256`; `线上结果/R31B/V011/result.json`. | Repeating the existing generic or two-slot UB mechanism is DUPLICATE_REJECTED. A cache-policy-only test could add evidence only when `localRows > batchLimit` and misses are shown. |
| R31A V026 / Wave-1 | Pass-2 parameter staging liveness uses release plus prefetch and has a recorded local improvement. | Its stated change is staging liveness within the pass; it does not establish cache retention across row-groups. The local record reports D32768 incremental -1.69%, 7/7 dual-device. | `线上结果/R31A/V026/submission.asc`; `技术路线/全版本记录.tsv` (R31A V024-V028 entries). | Not an exact duplicate. Independent value is limited to separating cross-batch cache retention from intra-pass staging; V026 is evidence only, not this route's parent. |
| R31B V017 / R31A V028 | Historical donor results may look attractive beside a new parent. | V017 changes MTE3 wait timing; V028 removes a batch-affine flag wait. Their local gains did not beat their respective Official anchors. Neither changes gamma/bias residency. | `技术路线/全版本记录.tsv`; `调度/线上候选.tsv`; `调度/本地线上校准.tsv`. | No parameter-residency experiment is justified by these mechanisms. Both remain historical evidence only and are not the direct parent. |
| BATCH-RESIDENT-X | R014-style parameters are reused with contiguous multi-row batching; its charter explicitly includes parameter staging and row-batch parameter reuse. | V001's change is multi-row DMA on A001-V017; it does not test a new parameter cache level, and its retained local result is unqualified/inconsistent. | `研究/BATCH-RESIDENT-X/next-hypotheses.md`; `本地实验/BATCH-RESIDENT-X/V001/local-result.json`; `本地实验/BATCH-RESIDENT-X/V001/MAIN-REVIEW.md`; `本地实验/BATCH-RESIDENT-X/harness/TRACK-A-RESULTS-20260925.md`; `调度/当前任务.tsv`. | New row-group reuse or row ownership is DUPLICATE_REJECTED and outside this boundary. H-L2 could add value without touching its batch ownership only if cache support and a real repeated-batch shape are confirmed. |
| COEFF-LOCALITY-X | H1/H2/H3 and V003/V004 study two-slot prefetch, cross-batch UB stripe residency, split-phase parameter loads, and full-row preload. | V001 was slower on its primary wide FP32 shape; V002 had no useful UB budget and no repeated batches in tested shapes; V003 was rejected; V004's small residual signal could not be separated from measurement bias. | `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md`; `研究/COEFF-LOCALITY-X/MAIN-APPROVAL-V001.md`; `研究/COEFF-LOCALITY-X/MAIN-APPROVAL-V002.md`; `本地实验/COEFF-LOCALITY-X/V001/support/results-timing-v001-20260927/SUMMARY.md`; `本地实验/COEFF-LOCALITY-X/V002/NO_UB_BUDGET.md`; V003/V004 rows in `技术路线/全版本记录.tsv`; `调度/当前任务.tsv`. | UB stripe, prefetch, and full-row preload candidates are DUPLICATE_REJECTED. The L2 storage level differs, but remains provisional until the exact MTE2 policy is verified. |
| EPI-X-FRESH / Wave-2 | Loads each gamma/bias chunk once and reuses it across an eight-row batch. | It also changes fused epilogue/dataflow and fails correctness (6490/8 mismatches); its source is a separate two-pass implementation. | `本地实验/EPI-X-FRESH/CURRENT/submission.asc:66`; `:95`; `本地实验/EPI-X-FRESH/CURRENT/local-result.json`; `技术路线/全版本记录.tsv`. | Eight-row UB reuse is DUPLICATE_REJECTED. The only distinct question here is cache-level retention without that batch/dataflow rewrite. |
| ASYNC-OVERLAP-CHAMPION-X / Wave-2 | Prefetches gamma/bias earlier in the pass. | It changes load timing to overlap with invRms work, not residency across row-groups; Official 44.17 was below the 45.16 anchor. | `技术路线/全版本记录.tsv`; `调度/当前任务.tsv`; `调度/本地线上校准.tsv`. | Not a candidate within this boundary; interpass scheduling is excluded. |
| MULTIROW-DMA-CHAMPION-X / Wave-2 | Multi-row transfer can reduce input DMA commands. | It modifies x/residual transfer geometry, not gamma/bias residency; V001 was rejected and V002 ended inside noise. | `本地实验/MULTIROW-DMA-CHAMPION-X/V001/`; `本地实验/MULTIROW-DMA-CHAMPION-X/V002/`; `技术路线/全版本记录.tsv`. | Not a candidate within this boundary; input DMA changes are excluded. |

### Rejected Candidates

| HYPOTHESIS_ID | Screened mechanism | Status | Replacement |
|---|---|---|---|
| PARAM-UB-ONCE-PER-CORE | Retain full gamma/bias in UB across rows. | DUPLICATE_REJECTED; R014, FULL-R014 and V011 already cover it. | PARAM-L2-ROWGROUP-RETENTION-01, a different storage level. |
| PARAM-UB-STRIPE-CROSS-BATCH | Keep a K-tile gamma/bias stripe across batches. | DUPLICATE_REJECTED; COEFF H2 tested the exact residency idea and documented insufficient UB plus no reuse on measured shapes. | PARAM-L2-ROWGROUP-RETENTION-01, conditional on documented MTE2 support. |
| PARAM-WIDE-TWO-DEEP-PREFETCH | Double-buffer parameter tiles in the wide output pass. | DUPLICATE_REJECTED for this route; COEFF H1 tested it, and it changes tile timing/staging rather than cross-row residency. | PARAM-L2-ROWGROUP-RETENTION-01. |
| PARAM-ROWGROUP-REUSE | Reuse gamma/bias by extending row-batch ownership or its row-group depth. | DUPLICATE_REJECTED; overlaps BATCH-RESIDENT-X and changes batching/ownership. | PARAM-L2-ROWGROUP-RETENTION-01, with ownership fixed. |
| PARAM-GENERIC-FULLROW-PRELOAD | Preload the full parameter row for a single-row multi-tile generic core. | DUPLICATE_REJECTED; COEFF V004 covers it and its target is not reuse across rows. | No other independent replacement found. |

The only provisional replacement is PARAM-L2-ROWGROUP-RETENTION-01. Since it depends on an API/device capability not present in the committed route evidence, the valid pool remains below three and is reported as ROUTE_HYPOTHESIS_POOL_EXHAUSTED. No duplicate candidate is proposed for implementation.

## Proposed One-Factor Difference

PROPOSED_ONE_FACTOR_DIFF=If later approved, alter only the documented cache policy on gamma/bias MTE2 GM reads in the current wide path. No change to parameter buffers, tile width/count, row grouping, launch geometry, load order, event sequence, or arithmetic.

## Expected Local Probes

EXPECTED_LOCAL_PROBES=No probes run in this handoff. If the API capability is confirmed and Planning selects the hypothesis, use a shape with `D>8192` and `localRows>batchLimit` as primary; include an otherwise comparable single-batch case as neutral control; qualify same-binary stability and use paired parent/candidate device-event measurements with the project protocol. Keep FP32 and FP16/BF16 results separated. A change on the single-batch control would not support cross-row retention.

## Handoff Status

CHILD_RECOMMENDED_HYPOTHESIS=PARAM-L2-ROWGROUP-RETENTION-01, conditional on read-only API feasibility confirmation; this is not an implementation selection.
OPEN_QUESTIONS=Does the committed CANN 8.5 / DAV_2201 material document a cache-retention policy honored by vector MTE2 reads from GlobalTensor? Can an eligible benchmark shape produce `localRows>batchLimit` without changing the current row mapping? The route is absent from `调度/当前任务.tsv`; the shared schedule was not changed, so Main should reconcile that record separately if needed.
WORK_DONE=Read-only Track-B research and evidence audit. No Revision or source change, build, correctness run, timing, profile, server3 access, Online submission, or shared-record change was performed.
