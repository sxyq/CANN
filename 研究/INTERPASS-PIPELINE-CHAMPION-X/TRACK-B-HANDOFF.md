# INTERPASS-PIPELINE-CHAMPION-X — Track-B Handoff

DATE: 2026-09-30
ROUTE: INTERPASS-PIPELINE-CHAMPION-X
BRANCH: w2/m2/interpass
BOOTSTRAP: COMPLETE
BASE_HEAD: ed860e392d7604694ac6664da60aff1fc1f4c04f
DIRECT_PARENT: R31B V011 Official 45.16
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED: NONE
HYPOTHESIS_POOL_STATUS: ROUTE_HYPOTHESIS_POOL_EXHAUSTED

The parent source digest was recomputed from `线上结果/R31B/V011/submission.asc` and agrees with its committed sidecar and `source-meta.json`. The research stayed within committed project material in this route worktree. No other route worktree or private context was accessed.

## ROUTE_BOUNDARY

TRACK-B only. Consider stage overlap contained within one logical row and its two-pass computation. Row-to-row pipelining, multi-row DMA, shape dispatch changes, implementation choice, and lifecycle decisions are outside this handoff. The screened ideas below are distinct mechanisms, but committed evidence shows each repeats an existing mechanism. No independent candidate remains.

## ALLOWED_CHANGES

Only this route's research/handoff file on `w2/m2/interpass`. `MAIN_SELECTED=NONE`; no hypothesis is selected for implementation.

## FORBIDDEN_CHANGES

- Kernel/Candidate source, `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, wrapper, or Revision changes.
- Build, correctness, timing, profiling, server3, or Online activity.
- Shared ledger/scheduler changes, route lifecycle changes, or new routes.
- Reading another route's private context or writable worktree.

## EVIDENCE_PATHS

- Bootstrap rules: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`.
- Bootstrap overview and calibration: `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`; `调度/当前任务.tsv`; `调度/线上候选.tsv`; `调度/本地线上校准.tsv`.
- Parent source and official evidence: `线上结果/R31B/V011/submission.asc`; `线上结果/R31B/V011/submission.sha256`; `线上结果/R31B/V011/source-meta.json`; `线上结果/R31B/V011/result.json`; `线上结果/R31B/V011/diff.patch`.
- R013/FULL evidence: `研究/ASYNC-TRIPLE-X/ROUTE-BRIEF.md`; `归档/phase3-before-reset-20260920/实验/server/FULL-R013-DOUBLE-BUFFER-PIPELINE/V001/结果.md`; `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`.
- R31A/R31B evidence: `本地实验/R31A/V020/local-result.json`; `本地实验/R31A/V020/diff.patch`; `本地实验/R31A/V021/local-result.json`; `线上结果/R31B/V017/result.json`; `线上结果/R31B/V017/diff.patch`; `归档/历史工作区/R31B/R31B-V012-FP32-ROW-PIPELINE_kernel.asc`; `技术路线/全版本记录.tsv`.
- MIX evidence: `线上结果/MIX-A/V003/diff.patch`; `本地实验/MIX-A/V007/local-result.json`; `本地实验/MIX-A/V007/diff.patch`; `技术路线/全版本记录.tsv`.
- Interpass/overlap research: `研究/ASYNC-TRIPLE-X/next-hypotheses.md`; `本地实验/ASYNC-TRIPLE-X/V001/submission.asc`; `本地实验/ASYNC-TRIPLE-X/V001/local-result.json`; `本地实验/ASYNC-TRIPLE-X/V001/diff.patch`; `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/submission.asc`; `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/result.json`; `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/diff.patch`; `技术路线/全版本记录.tsv`.
- Coefficient-locality evidence: `研究/COEFF-LOCALITY-X/TRACK-B-BRIEF.md`; `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md`; `研究/COEFF-LOCALITY-X/MAIN-APPROVAL-V001.md`; `研究/COEFF-LOCALITY-X/MAIN-APPROVAL-V002.md`; `本地实验/COEFF-LOCALITY-X/V001/local-result.json`; `本地实验/COEFF-LOCALITY-X/V002/local-result.json`; `技术路线/全版本记录.tsv` entries for V003/V004.
- Wave-2 route summaries: `技术路线/全版本记录.tsv` entries for `ASYNC-OVERLAP-CHAMPION-X` V002-V004, `MULTIROW-DMA-CHAMPION-X` V001-V002, and `COEFF-LOCALITY-X` V003-V004; `调度/当前任务.tsv`.

## SCREENED HYPOTHESES

These five mechanisms differ from one another, but each is marked `DUPLICATE_REJECTED` after comparison with committed evidence. They are recorded to make the rejection auditable; they are not a surviving 3-5 independent hypothesis pool.

### IPX-S01 — Pass-1 input prefetch

- `HYPOTHESIS_ID`: IPX-S01
- `MECHANISM`: In the same row's pass-1 tile loop, issue MTE2 loads for tile N+1 while Vector adds, squares, and reduces tile N, using two existing staging slots.
- `BOTTLENECK`: Serialized MTE2-to-Vector gaps between row-local reduction tiles.
- `DIRECT_PARENT`: R31B V011 Official 45.16.
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `TARGET_SHAPES`: Single-row-per-core, multi-tile wide rows, e.g. D=16384-32768; no cross-row overlap.
- `TARGET_DTYPES`: FP32, FP16, BF16.
- `WHY_IT_MAY_HELP`: The next tile's input DMA could run during current-tile Vector work.
- `WHY_IT_MAY_FAIL`: The parent already uses this MTE2/Vector pairing on its low-precision wide-row path; the FP32 analogue was separately tested and did not improve the Official score. Event/refill mistakes can corrupt staging.
- `UB_IMPACT`: A two-slot design needs staging space; no new slot or buffer is proposed. Parent low-precision path already uses two slots.
- `DMA_IMPACT`: Same bytes; only per-row issue timing changes.
- `SYNC_IMPACT`: Requires MTE2-ready and Vector-release ordering per slot; a premature refill races the reduction.
- `PRECISION_RISK`: Arithmetic order can remain unchanged; incorrect slot ownership can cause wrong output.
- `DUPLICATE_CHECK`: `DUPLICATE_REJECTED`. Similar mechanism: R31B V011 low-precision row pipeline and R31B V012 FP32 wide input pipeline. Actual difference: V020 used a separate FP32 CachedRows route and failed with native status 507035 on D=32768. The stage pair is still MTE2(N+1)/Vector(N); another test of that pair has no independent value without a new, evidenced code path. Evidence: `线上结果/R31B/V011/submission.asc`; `归档/历史工作区/R31B/R31B-V012-FP32-ROW-PIPELINE_kernel.asc`; `本地实验/R31A/V020/local-result.json`; `技术路线/全版本记录.tsv`.
- `RELATED_OLD_ROUTES`: R013/FULL-R013; R31A V010/V020; R31B V011/V012.
- `MINIMAL_EXPERIMENT`: None; this mechanism is already represented in the parent and historical revisions. No run requested or authorized.

### IPX-S02 — Inter-pass tile-0 prefetch

- `HYPOTHESIS_ID`: IPX-S02
- `MECHANISM`: Start the same row's pass-2 tile-0 parameter MTE2 load before the final inverse-RMS scalar handoff completes.
- `BOTTLENECK`: The boundary between reduction completion and pass-2 prologue, where a cold tile-0 load may serialize behind Vector/Scalar work.
- `DIRECT_PARENT`: R31B V011 Official 45.16.
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `TARGET_SHAPES`: Multi-tile rows, especially D=8192-32768; one logical row per core for attribution.
- `TARGET_DTYPES`: FP16/BF16 wide rows and FP32 FullCache rows.
- `WHY_IT_MAY_HELP`: It could hide part of the fixed per-row scalar tail under a pass-2 parameter transfer.
- `WHY_IT_MAY_FAIL`: The overlap window may be shorter than the transfer, the load may contend with reduction scratch, or the scalar tail may already be hidden. Single-tile rows have no useful tile pipeline.
- `UB_IMPACT`: No intended capacity change; the selected staging pair must be idle and not used as reduction scratch.
- `DMA_IMPACT`: Same parameter bytes, issued earlier.
- `SYNC_IMPACT`: Must not expose parameters to Vector until MTE2 completion and must preserve the scalar dependency for inverse RMS.
- `PRECISION_RISK`: None if only load timing moves and arithmetic order is unchanged; premature use can produce wrong output.
- `DUPLICATE_CHECK`: `DUPLICATE_REJECTED`. Similar mechanisms: ASYNC-OVERLAP-CHAMPION-X V001 moved pass-2 gamma/bias tile-0 load above the inverse-RMS scalar loop; V004 tested the FullCache variant. ASYNC-TRIPLE-X H1 moved its pass-2 `CopyInOutputData` tile-0 load before `FinishRms`. Actual difference: the latter also loads x/residual while the Champion parent retains y and reloads only parameters. Both target the same pass-boundary MTE2-versus-Vector/Scalar window, so changing the loaded operands does not justify an independent test. Evidence: `研究/ASYNC-TRIPLE-X/next-hypotheses.md`; `本地实验/ASYNC-TRIPLE-X/V001/submission.asc`; `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/submission.asc`; `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/result.json`; `技术路线/全版本记录.tsv`.
- `RELATED_OLD_ROUTES`: ASYNC-TRIPLE-X H1; ASYNC-OVERLAP-CHAMPION-X V001/V004; COEFF-LOCALITY-X V001.
- `MINIMAL_EXPERIMENT`: None; prior committed work already isolates the same boundary. Do not repeat without new evidence that the target code path differs materially.

### IPX-S03 — Same-row MTE2/Vector/MTE3 triple issue

- `HYPOTHESIS_ID`: IPX-S03
- `MECHANISM`: In pass 2, issue MTE2 for tile N+1, Vector work for tile N, and MTE3 store for tile N-1 in one same-row steady-state iteration.
- `BOTTLENECK`: Pass-2 stage serialization and exposed output-store latency on multi-tile rows.
- `DIRECT_PARENT`: R31B V011 Official 45.16.
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `TARGET_SHAPES`: At least three tiles per row; wide rows D>=8192, one row per core for attribution.
- `TARGET_DTYPES`: FP32, FP16, BF16, subject to the path's tile count.
- `WHY_IT_MAY_HELP`: It can overlap a previous output transfer and a next-tile parameter transfer with current-tile Vector work.
- `WHY_IT_MAY_FAIL`: MTE2/MTE3 may contend for HBM; event overhead or staging-slot reuse can erase the overlap or introduce a race. Short rows have too few steady-state iterations.
- `UB_IMPACT`: Keep existing depth and capacity; no added slot. Reuse of output and parameter slots needs explicit ownership.
- `DMA_IMPACT`: Same total bytes, different issue timing; concurrent MTE2/MTE3 may worsen effective bandwidth.
- `SYNC_IMPACT`: Highest risk: Vector-to-MTE3 readiness, MTE3 completion before slot reuse, and MTE2-to-Vector readiness must all be paired correctly.
- `PRECISION_RISK`: Arithmetic can stay identical; a missing completion edge can expose partially written output or overwritten input.
- `DUPLICATE_CHECK`: `DUPLICATE_REJECTED`. Similar mechanism: ASYNC-TRIPLE-X V001 directly tested true MTE2/V/MTE3 overlap; its H3 separately considered MTE2/MTE3 issue ordering. R31B V017 deferred pass-2 MTE3 completion. Actual difference: those experiments use different source paths and staging operands, but the same three hardware stages and row-local store-latency window. Evidence: `本地实验/ASYNC-TRIPLE-X/V001/diff.patch`; `研究/ASYNC-TRIPLE-X/next-hypotheses.md`; `线上结果/R31B/V017/diff.patch`; `线上结果/R31B/V017/result.json`; `技术路线/全版本记录.tsv`.
- `RELATED_OLD_ROUTES`: R31A V014/V015; R31B V006/V009/V017; ASYNC-TRIPLE-X V001; FULL-R013.
- `MINIMAL_EXPERIMENT`: None; the same three-stage row-local pipeline has a committed route and result. No additional device run proposed.

### IPX-S04 — Defer output-store completion wait

- `HYPOTHESIS_ID`: IPX-S04
- `MECHANISM`: Issue each same-row output store and defer its MTE3-to-Vector completion wait until immediately before that local output slot is reused or the row ends.
- `BOTTLENECK`: A per-tile MTE3 completion wait on the row's output critical path.
- `DIRECT_PARENT`: R31B V011 Official 45.16.
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `TARGET_SHAPES`: Multi-tile wide rows, notably D=24576-32768; single-row-per-core preferred for attribution.
- `TARGET_DTYPES`: FP32 and low-precision wide paths.
- `WHY_IT_MAY_HELP`: If the store remains asynchronous and its source slot is not reused, later same-row work may cover MTE3 latency.
- `WHY_IT_MAY_FAIL`: The queue may already hide completion, or delayed waits may only move the same drain to the row tail. Reusing a slot early risks a race.
- `UB_IMPACT`: No capacity change; output slot lifetime extends through store completion.
- `DMA_IMPACT`: Identical store bytes and count; only completion visibility moves.
- `SYNC_IMPACT`: Per-slot MTE3 completion must precede any overwrite; a row-tail wait alone is unsafe when a slot is reused earlier.
- `PRECISION_RISK`: No arithmetic change; synchronization error can corrupt output.
- `DUPLICATE_CHECK`: `DUPLICATE_REJECTED`. Similar mechanisms: R31A V021 deferred the FP32 wide per-tile MTE3 wait to the row boundary; R31B V017 deferred a pass-2 store drain and reported local gains but Official 44.68, below the parent score. Actual difference: buffer layout and dtype path vary, while the moved MTE3 completion edge is the same. The existing evidence is enough to reject a mechanism-only rerun. Evidence: `本地实验/R31A/V021/local-result.json`; `线上结果/R31B/V017/diff.patch`; `线上结果/R31B/V017/result.json`; `技术路线/全版本记录.tsv`.
- `RELATED_OLD_ROUTES`: R31A V014/V015/V021; R31B V017; ASYNC-TRIPLE-X H2.
- `MINIMAL_EXPERIMENT`: None; the wait-placement concept is already tested. No new run proposed.

### IPX-S05 — Split-phase parameter transfer

- `HYPOTHESIS_ID`: IPX-S05
- `MECHANISM`: For one row's output tile, load gamma, apply its Vector multiply, then load bias and apply Vector add, interleaving MTE2 transfers with the corresponding Vector phase without adding staging.
- `BOTTLENECK`: Serialized gamma/bias MTE2-to-Vector latency in the output pass.
- `DIRECT_PARENT`: R31B V011 Official 45.16.
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `TARGET_SHAPES`: FP32 wide multi-tile rows, especially D=16384-32768.
- `TARGET_DTYPES`: FP32.
- `WHY_IT_MAY_HELP`: It may start bias transfer while the gamma multiply runs and avoid a second staging pair.
- `WHY_IT_MAY_FAIL`: The V/MTE2 ordering may serialize on barriers, and splitting each tile may add issue overhead with no useful overlap.
- `UB_IMPACT`: No extra capacity intended; the existing parameter buffers remain in use.
- `DMA_IMPACT`: Same bytes and descriptors, with altered order.
- `SYNC_IMPACT`: Each parameter must be ready before its consuming Vector operation; the split does not permit buffer reuse before the operation completes.
- `PRECISION_RISK`: Arithmetic order is unchanged if only transfers move; wrong event placement can read incomplete coefficients.
- `DUPLICATE_CHECK`: `DUPLICATE_REJECTED`. COEFF-LOCALITY-X V003 tested this split-phase gamma/bias emission at fixed tileElems=4096 and recorded a local rejection; V001 tested a two-deep parameter prefetch, and V002 tested cross-batch stripe residency. Actual difference: V003 interleaves a single tile's separate parameter loads, while V001 prefetches the next tile. Both address the same row-local parameter MTE2 latency, so the precise issue pattern has already been measured. Evidence: `技术路线/全版本记录.tsv` V001-V004 entries; `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md`; `研究/COEFF-LOCALITY-X/MAIN-APPROVAL-V002.md`.
- `RELATED_OLD_ROUTES`: COEFF-LOCALITY-X V001-V004; ASYNC-OVERLAP-CHAMPION-X V001/V004.
- `MINIMAL_EXPERIMENT`: None; V003 already isolates the split-phase mechanism. No repeated probe proposed.

## DUPLICATE_AUDIT

| Evidence group | Similar mechanism and actual difference | Evidence and independent-test value |
|---|---|---|
| R001-R029 and FULL | R013/FULL-R013 already overlaps MTE2 input fetch with Vector tile work; it has no MTE3 in the same iteration, later covered by ASYNC-TRIPLE-X. R014 is coefficient stripe residency rather than stage overlap, covered by COEFF-LOCALITY-X. R015 multi-row DMA and R016 core mapping cross row/core boundaries, outside this route. R001/R002/R019 concern reduction structure rather than a new transfer/compute pair. | `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; FULL-R013 V001 `结果.md`; `研究/ASYNC-TRIPLE-X/ROUTE-BRIEF.md`; `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md`. A two-stage MTE2/Vector rerun adds no new test value; the other adjacent mechanisms are either already covered or outside scope. |
| R31A/R31B | V010 mid-size MTE2 overlap, V011 low-precision row pipeline, V012 FP32 wide input/parameter pipeline, V014/V015 depth-3 pipeline variants, V020 dual-slot FP32 reduction, V021 delayed MTE3 wait, and V017 delayed pass-2 store drain cover the row-local pairs and triple. V020 failed correctness at D=32768; V012 Official was 45.14; V017 Official was 44.68. | `线上结果/R31B/V011/`; `归档/历史工作区/R31B/R31B-V012-FP32-ROW-PIPELINE_kernel.asc`; `本地实验/R31A/V020/local-result.json`; `本地实验/R31A/V021/local-result.json`; `线上结果/R31B/V017/`; `技术路线/全版本记录.tsv`. No independent mechanism remains in these stage pairs. |
| MIX-A | V003's V-to-MTE2 release and V007's removal of a single-row pre-load release wait alter synchronization cost; V007 is not a new stage-overlap schedule and its shape floor was unqualified. | `本地实验/MIX-A/V007/local-result.json`; `本地实验/MIX-A/V007/diff.patch`; `技术路线/全版本记录.tsv`. Relevant sync context, not a surviving interpass idea. |
| ASYNC-TRIPLE-X | V001 implements same-row MTE2/V/MTE3 overlap; H1/H2/H3 cover pass-boundary prefetch, output-ring completion, and MTE2/MTE3 ordering. H4 is inter-row and H5 is tile-count schedule selection. | `研究/ASYNC-TRIPLE-X/next-hypotheses.md`; `本地实验/ASYNC-TRIPLE-X/V001/`. H4 violates this route's same-row boundary; H5 overlaps prior shape-bucket ownership and is not itself a new stage pair. |
| Wave-1 ASYNC-OVERLAP-CHAMPION-X | V001 overlaps pass-2 gamma/bias tile-0 MTE2 with the inverse-RMS tail on the low-precision path; V004 tests the FullCache path and documents a scratch conflict and mixed result. | `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/`; `技术路线/全版本记录.tsv`. The corresponding pass-boundary MTE2/Vector window has already been tried. |
| COEFF-LOCALITY-X | V001 parameter prefetch, V002 cross-batch stripe residency, V003 split-phase gamma/bias transfer, and V004 full-row preload cover the same-row parameter-transfer timing/locality variants. V003 was locally rejected; V004's residual was not separated from measurement bias. | `研究/COEFF-LOCALITY-X/`; `本地实验/COEFF-LOCALITY-X/V001/`; `本地实验/COEFF-LOCALITY-X/V002/`; `技术路线/全版本记录.tsv`. Repeating parameter MTE2 timing under another label has no independent value. |
| Wave-2 ASYNC / multi-row DMA | ASYNC V002 reorders next-row load and defers a store wait; V003 changes the GetValue handoff; V004 returns to the FullCache pass-boundary prologue. MULTIROW V001 changes cross-row DMA transaction shape; V002 changes aligned copy form while keeping two-deep scheduling. | `技术路线/全版本记录.tsv`; `调度/当前任务.tsv`. Cross-row mechanisms are outside scope; V004 and pass-boundary scalar work are already covered. Raw Wave-2 route packages are absent from this canonical tree and are listed below. |

The independent pool count after duplicate rejection is zero. Therefore this handoff reports `ROUTE_HYPOTHESIS_POOL_EXHAUSTED`; it does not claim three surviving hypotheses or convert a duplicate into a new proposal by changing its buffer or operand name.

## PROPOSED_ONE_FACTOR_DIFF

NONE. Every screened one-factor change is marked `DUPLICATE_REJECTED`. `MAIN_SELECTED=NONE`; no implementation decision is made.

## EXPECTED_LOCAL_PROBES

NONE for this handoff. There is no eligible new mechanism to probe. No build, correctness, timing, profiling, server3, or Online work was run or authorized.

## CHILD_RECOMMENDED_HYPOTHESIS

NONE — `ROUTE_HYPOTHESIS_POOL_EXHAUSTED`. Main review is needed before any new route scope or evidence source is added.

## OPEN_QUESTIONS

- `本地实验/ASYNC-OVERLAP-CHAMPION-X/V002/` through `V004/` raw packages are absent at this canonical HEAD; only the committed all-revision summaries are available here.
- `本地实验/MULTIROW-DMA-CHAMPION-X/V001/` and `V002/` raw packages are absent. Their committed summaries describe cross-row DMA, which is outside this route's boundary.
- `本地实验/COEFF-LOCALITY-X/V003/` and `V004/` raw packages are absent; their committed all-revision entries summarize outcomes. This limits source-level re-audit of those revisions but does not change their recorded same-row coefficient mechanisms.
- No later committed evidence was found for this route itself. The route's own research/evidence directory did not exist at bootstrap.
