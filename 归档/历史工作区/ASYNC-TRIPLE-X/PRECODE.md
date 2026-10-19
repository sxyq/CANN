# ASYNC-TRIPLE-X Pre-Code Record

```text
ROUTE: ASYNC-TRIPLE-X
REVISION: V001
DIRECT_PARENT: route seed / R013-derived baseline constructed in this worktree (ASYNC-TRIPLE-X SEED)
PARENT_SOURCE_SHA: f20da79c7086483c7cf0bddad630bdea92a219992a5f061c9fd3a6b8edbc3572 (ASYNC-TRIPLE-X-SEED.asc)
PARENT_SCORE: n/a (fresh route seed; no Official Score)
SINGLE_HYPOTHESIS: With reduction, parameter policy, dtype strategy, and core mapping unchanged from the R013-derived SEED, restructure the output-phase loop so MTE3 store of tile N-1 is issued before Vector compute of tile N while MTE2 prefetch of tile N+1 remains issue-ahead — forming true triple overlap MTE2(N+1) / V(N) / MTE3(N-1).
CONTEXT_CLASS: HISTORICAL_DERIVED
DONOR: R013 DOUBLE-BUFFER PIPELINE (MTE2 prefetch N+1 + Vector compute N; MTE3 store not scheduled as a third stage)
DONOR_SOURCE: phase4/archive/historical-branches-20260924/independent__full-r013-double-buffer-pipeline-i001/.../kernel.txt
DONOR_SOURCE_SHA256: ba1167079cf54277506e4b4fa192a91a036a6b8d992650e8532fb182c555bda0
CANONICAL_SEED_COMMIT: ae46d7ca9b2ca2d60a27d0fdc430801c13268f0b
```

## Scope lock (V001)

ONLY change: output-phase MTE3 schedule (defer store of N-1 to overlap compute of N).

DO NOT change:

- reduction (two-pass ReduceSum + FinishRms)
- parameter policy (gamma/bias loaded with each output tile via paramQueue)
- dtype strategy (FP16/BF16/FP32 template split, FP32 middle)
- core mapping (row-stride `row += blockNum` over `outer`)
- tile length, UB budget framing, ABI `run_kernel`

## WHY_NOT_DUPLICATE

| Compared with | Why not duplicate |
|---|---|
| R001 two-pass sum | R001 is reduction structure; SEED already two-pass; V001 does not touch reduction. |
| R002 resident-y | V001 does not retain y/u across GM or change read count. |
| R003 pure Ascend C direct | Infra/compile path; not a pipeline schedule. |
| R004 low-precision middle | Blocked/historical; V001 keeps FP32 middle. |
| R005 large tile | Tile length unchanged (1024). |
| R006 chunked reduce | Reduce path untouched. |
| R007 ReduceSum | Same ReduceSum call as SEED; no multi-row ReduceSum. |
| R008 tile-across-cores | Row-parallel mapping unchanged; no D-slice. |
| R009 DataCopyPad tail | CopyIn/CopyOut helpers unchanged. |
| R010 manual tail/epilogue | No software mask epilogue added. |
| R011 manual vector reduce | No vector reduction tree. |
| R012 32B row alignment | Alignment helpers unchanged. |
| **R013 double-buffer** | **Donor only.** R013 issue-ahead is MTE2+Vector; MTE3 runs after compute in the same iteration and is not scheduled as stage N-1. V001’s single change is the third-stage MTE3 schedule. Full MTE2/V/MTE3 triple remains unexplored (idea-pool R013 wide triple). |
| R014 GammaBias residency | Param queue pattern unchanged. |
| R015 multi-row DMA | No stride multi-row DataCopy. |
| R016 AI Core per row | Core mapping unchanged. |
| R017 FP32 full middle | Unchanged. |
| R018 CAST_RINT | Unchanged. |
| R019 invRms | Unchanged FinishRms. |
| R020 sqrt | Unchanged Sqrt. |
| R021–R028 | Infra / process / measurement / SPR / GPU notes / taxonomy — not this schedule change. |
| R029 five-mode dispatch | No mode taxonomy. |
| R030 / R031 | R031 is R31A origin (multimode champion family). V001 does not use multimode dispatch, mode buckets, or R31 special functions; SEED is R013-shaped, not R31-shaped. |
| MIX-A | MIX is R31×A001 hybrid with MULTI_CHANGE parents; V001 is single-schedule change on R013-derived SEED, no FastKernel/Resident split, no mode gates. |
| First six-lane (R31A/R31B/MIX-A/WIDE-X-FRESH4/MODE-X-R015C/EXT-ASCEND-X) | R31A/B exploit multimode online; WIDE-X-FRESH4 is Fresh Blind wide-D; MODE-X is R015C multi-row DMA; EXT-ASCEND-X forbids triple pipeline in its V001. None isolates full triple overlap on an R013-derived two-stage pipeline. |
| Other next6 routes (CORE-SCHED-X, UB-LAYOUT-X, DTYPE-SPECIAL-X, PARAM-REUSE-X, NEW-EXTERNAL-DERIVED-X) | Different single hypotheses (core schedule, UB layout, dtype specialization, param reuse, external-derived). ASYNC-TRIPLE-X owns only triple-overlap schedule. |

**Conclusion:** Full true triple overlap (MTE2 prefetch N+1, Vector compute N, MTE3 store N-1) is still unexplored per idea-pool R013 item 3. This route is not a duplicate of R001–R029, R030/R031, MIX, the first six-lane lineup, or the remaining next6 candidates.


## Baseline correctness note (applies to SEED and V001 equally)

R013 donor declared `TQue<...,1>` while calling `InitBuffer(..., kDoubleBuffer=2)`.
With depth 1, issue-ahead `EnQue` of tile N+1 before `DeQue` of tile N deadlocks on
multi-tile rows (observed hang at width=2048). SEED and V001 both use `TQue<...,2>`
to match the double-buffer allocation. This is a correctness alignment of the donor's
stated double-buffer intent, identical on both sides of the V001 comparison; the
V001 single change remains only the MTE3 schedule.


## Baseline correctness note 2 (SEED and V001 equally)

R013 donor `ToFp32`/`FromFp32` for float used `blockLen = count * sizeof(float)`.
Local-to-local `DataCopy` `blockLen` is in 32B blocks (A001 uses `(count+7)/8`).
The byte-count form over-copies for larger counts and corrupts UB neighbors
(observed wrong invRms for width >= 384). Both SEED and V001 use
`blockLen = (count + 7) / 8`. This is a correctness alignment of the donor's
float path; the V001 single change remains only the MTE3 schedule.
