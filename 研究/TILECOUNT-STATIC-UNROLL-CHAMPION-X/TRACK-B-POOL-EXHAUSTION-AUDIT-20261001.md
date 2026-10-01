# TCSU Track-B Pool Exhaustion Audit

```text
DATE_UTC=2026-10-01
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
TRACK=TRACK-B-ONLY
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
PROFILING=NOT_RUN
ONLINE=NOT_RUN
POOL_RESULT=EXHAUSTED_WITH_THREE_ADMISSIBLE_HYPOTHESES
```

This is a route-local, read-only pool audit. It records the result of auditing
the exact parent path, R005, R029, SHAPE-TILING-CHAMPION-X, R022/codegen, and
the historical unroll references. It does not select a hypothesis or make a
route lifecycle decision. No `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, or
host wrapper was edited. No Revision, build, server3, NPU, timing, profiling,
or Online action was performed.

## 1. Exact parent boundary

The retained parent source is `线上结果/R31B/V011/submission.asc`; its SHA-256
matches both the parent metadata and the route handoff. The only admissible
static tile-count instance is:

| Item | Static evidence | Required invariant |
|---|---|---|
| Path | `Process()` selects `ProcessFp32FullRowOutputPipelined` only for `rowWidth == kCacheElems && localRows > 1` (`submission.asc:177-180`). | Keep the same FP32 path predicate. |
| Geometry | `kTileElems=4096` (`:1242`), `kCacheElems=8192` (`:1288`). | `N = kCacheElems / kTileElems = 2`; no tile or cache-width change. |
| Launch witness | `[128,8192]`, `availableCoreNum=32` gives `blockCount=32`, `localRows=4` after the existing wrapper clamp. | Keep block ownership and runtime row loop unchanged. |
| Parameter loop | `:1129-1132`, `col={0,4096}`; two gamma and two bias loads. | Same four parameter DMA operations, order, and destinations. |
| First-pass loop | `:1164-1180`, `col={0,4096}`; existing first-tile prefetch condition remains row-dependent. | Same x/residual load opportunities, reductions, and MTE2/V fences. |
| Output loop | `:1203-1225`, `col={0,4096}`; `useFirst` selects the two existing event pairs. | Same two stores, offsets, event IDs, waits, releases, and arithmetic order. |

The source proves a two-iteration source trip count only. It does not prove
that the compiler emitted a loop backedge. The retained parent package has no
assembly, disassembly, codegen dump, register report, or compile log for this
function. Therefore all three candidates remain codegen-gated.

## 2. Geometry, UB, DMA, and sync invariants

The FP32 cache-row path's source allocation is fixed by
`submission.asc:116-149`: `xBuf_` and `residualBuf_` are one `kTileElems`
float tile each; `gammaBuf_` and `biasBuf_` are one `kCacheElems` vector each;
`xFp32Buf_` and `residualFp32Buf_` are one `kTileElems` float tile each;
`valueFp32Buf_` is one `kCacheElems` float row; and `reduceFp32Buf_` is one
`kTileElems` float tile. Static unrolling is admissible only if this allocation
and every buffer lifetime remain byte-for-byte equivalent.

The following contract applies to every candidate:

| Axis | Parent contract | Disallowed change |
|---|---|---|
| Tile geometry | `4096` element tiles inside an `8192` element cached row; offsets are `0` and `4096`. | Any `kTileElems`, `kCacheElems`, tile-count, row-width bucket, or shape dispatch change. |
| UB | Existing buffers and ownership/lifetimes above. | Larger tile, extra buffer, changed alias, changed residency, or changed lifetime. |
| Core ownership | Existing `blockCount`, `beginRow`, `localRows`, and row loop. | Row unrolling, row regrouping, block-factor or launch changes. |
| MTE2 | Existing parameter and x/residual loads, including the next-row prefetch. | Fewer/more DMAs, batching, prefetch movement, or overlap redesign. |
| V/MTE2 | Two first-pass tile synchronization pairs in the existing order. | Removing, moving, or adding fences/barriers. |
| V/MTE3/MTE3/V | Two output event pairs, two stores, then existing drain and release sequence. | Store merging, wait movement, event reuse, or drain changes. |
| Arithmetic | Existing `Muls -> Mul -> Add` output order and reduction order. | Fusion, reassociation, reduction topology, dtype, or precision-order change. |

Consequently, the only remaining performance lever is removal or constant
folding of control/index work in one already-fixed two-trip source loop. A
measured change without a parent/candidate codegen delta cannot be attributed
to TCSU.

## 3. Admissible hypothesis pool

These are the complete admissible candidates. They are unranked and none is
selected.

| ID | Single mechanism | Expected bottleneck | Minimal OFAT scope | Main risk / gate |
|---|---|---|---|---|
| TCSU-H1 | Explicitly expand only the parameter-preload loop. | Per-block loop backedge and tile-offset calculation before rows. | Two gamma and two bias loads at offsets `0/4096`; preserve order and all allocation. | One-time setup cost is small; reject if parent already has straight-line code. |
| TCSU-H2 | Explicitly expand only the first-pass tile loop. | Per-row tile counter, branch, and offset/index work around input, square, and partial reduction. | Keep the first-tile prefetch predicate, two tile bodies, two `ReduceSum` calls, both sync pairs, and next-row prefetch unchanged. | DMA/vector/sync cost may dominate; register pressure or pre-existing compiler unroll can erase the mechanism. |
| TCSU-H3 | Explicitly expand only the output-pass tile loop. | Per-row loop control and constant `useFirst` event selection. | Keep both tile stores, the original event IDs/order, drain flags, and `Muls -> Mul -> Add` order. | Store/event work dominates; code size or pre-existing compiler unroll may erase the mechanism. |

For each candidate, the mandatory first evidence is exact-parent codegen showing
a relevant backedge or dynamic index work, followed by candidate codegen showing
only its fixed offsets/control removal. Source constants alone are insufficient.

## 4. Historical and cross-route audit

| Audited source | What it actually changes or proves | TCSU disposition |
|---|---|---|
| R005 / FULL-R005-LARGE-TILE | `kWorkTileElems=8192`; it uses one wide work tile, a different UB lifetime plan, and fewer tile transitions. The retained kernel explicitly allocates 8192-element native/FP32 buffers. | `DUPLICATE_REJECTED`: geometry and UB change. It is not source unrolling of the parent's 4096-element N=2 loop. |
| R029 / FULL-R029 | Five runtime modes (`SPLIT_D`, `SINGLE_N`, `MERGE_N`, `MULTI_N`, `NORMAL`) and wide cached-row variants. The retained L005 source changes wide segment/cache behavior and allocation. | `DUPLICATE_REJECTED`: mode/shape/width/residency axis. It does not prove parent compiler emission. |
| SHAPE-TILING-CHAMPION-X | The formal Main-1 records define a separate shape-conditioned tile/UB tradeoff route. Its recorded V001/V002 work changes tile tiers and the route is explicitly research-only pending Planning review. | `DUPLICATE_REJECTED`: shape-conditioned geometry/UB axis; no shape bucket or tile tier is allowed in TCSU. |
| R022 / FULL-R022-AUTO-CODEGEN | The generator substitutes schedule/source parameters into a template (`tile_elems=4096` in its spec); it does not inspect or prove the compiled instruction stream of R31B V011. | `REFERENCE_ONLY`: generator/source emission is a different mechanism and would add a schedule parameter. |
| Historical Ascend C `simd_vf_story/reduce` variants | Official sample references include `baseline/pragma/unroll/binary` variants, but they are standalone reduction examples, not this parent path or its DMA/event protocol. | `REFERENCE_ONLY`: useful vocabulary, no AddRmsNormBias parent codegen evidence. |
| Historical CUDA/GPU `#pragma unroll` examples | They show source/compiler unrolling in another programming model, with register/shared-memory and warp behavior not equivalent to this Ascend C path. | `REFERENCE_ONLY`: cannot be used to claim Ascend compiler emission or a safe candidate. |
| Generic compiler/IR/codegen survey | The retained compiler survey classifies generators/runtimes as unable to emit the required single-file direct-invoke artifact; it contains no exact parent assembly. | `EVIDENCE_LIMIT`: distinguishes generic compiler knowledge from this kernel's emitted code. |

The historical material therefore produces no fourth admissible TCSU
mechanism. The apparent alternatives are either geometry/UB/DMA/scheduling
changes, another route's ownership, or generic evidence without exact parent
codegen.

## 5. Source unrolling versus compiler-emitted unrolling

These terms must remain separate:

1. **Source-level fixed-trip proof:** the parent source constants reduce the
   three inner loops to `col=0` and `col=4096` for the stated path.
2. **Source-level unrolling candidate:** a future one-loop OFAT edit would make
   those two bodies explicit in source while preserving the contract in Section
   2.
3. **Compiler-emitted unrolling:** the generated instruction stream may already
   contain two straight-line bodies even though the source retains a loop, or
   may retain a backedge/index calculation after a source edit. Only exact
   parent/candidate codegen can distinguish these cases.

Thus `CODEGEN_GATE=UNVERIFIED` is neither a positive nor a negative performance
result. It is the reason no candidate may be implemented or attributed yet.

## 6. Disposition

`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` means the admissible TILE_COUNT pool has
three source-level one-loop candidates and no fourth distinct candidate under
the frozen geometry/UB/DMA/sync/precision boundary. It does **not** mean the
route is parked, closed, abandoned, merged, or replaced; those decisions remain
with Planning / Review Layer.

`MAIN_SELECTED=NONE`. The next authorized action, if Planning selects one
hypothesis, starts with exact parent codegen evidence. Until then this route
remains research-only with no Revision.

## Evidence index

- Parent: `线上结果/R31B/V011/submission.asc`, `source-meta.json`, `diff.patch`, `result.json`.
- Existing route handoff: `TRACK-B-HANDOFF.md`.
- Existing codegen gate: `TRACK-B-CODEGEN-DUPLICATE-AUDIT-20261001.md`.
- R005: `归档/phase3-before-reset-20260920/提交/单方案/FULL-R005-LARGE-TILE/V001/kernel.txt`.
- R029: `归档/phase3-before-reset-20260920/提交/单方案/R029-官方Tiling五模式/README.md` and `归档/phase3-before-reset-20260920/实验/online/FULL-R029-WIDE-CACHED-ROW/V001/6aad7849b0477ec41e7fd3b1/kernel.txt`.
- R022: `归档/phase3-before-reset-20260920/实验/online/FULL-R022-AUTO-CODEGEN/V001/6aae2922b0477ec41ebe0671/generator/`.
- Historical compiler survey: `归档/phase3-before-reset-20260920/调研/归档/调研6/agents/agent05-compiler-codegen.md`.
- Historical unroll sample reference: `归档/phase3-before-reset-20260920/调研/归档/调研6/agents/agent03-official-repos.md:263-265`.
- SHAPE-TILING formal boundary: `调度/当前任务.tsv` and `研究/主代理/MAIN-1/初始化报告.md`.
