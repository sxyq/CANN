# MAIN-2 R2 Route Registry

Owner: MAIN-2
Canonical workspace: `/Users/sunyiyang/Desktop/Project/cann`
Repository: `sxyq/CANN`
Canonical branch: `exp/independent-breadth`
Worktree root: `/Users/sunyiyang/Desktop/Project/cann-main2-r2/`
Created: 2026-09-27

## Frozen strong baseline

| Field | Value |
|---|---|
| FROZEN_BASELINE_NAME | R31B-V011 |
| FROZEN_BASELINE_SOURCE | `phase4/workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` |
| FROZEN_BASELINE_SNAPSHOT | `phase4/online/R31B/V011/submission.asc` |
| FROZEN_BASELINE_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| OFFICIAL_ANCHOR | 45.16 (15/15) |
| OFFICIAL_SUBMISSION_ID | `6ab2c10c0304f72a56a0c5cb` |
| SOURCE_COMMIT | `43a1049a1e08e518c88e354a754fdebb85a96f99` |

Identity check 2026-09-27: workspace source bytes == online snapshot bytes == sidecar SHA. Both paths hash to the same value above.

Forbidden seeds: R31B current worktree, R31B V016, MAIN-1 uncommitted implementations, cherry-picks of MAIN-1 current experimental commits.

## SCHED exact Official (donor evidence)

| Field | Value |
|---|---|
| Route | SCHED-ROWGROUP-X V001 |
| Official | 15/15, 22.27 |
| Direct parent | R016-V001 COMPILEFIX-A, score 17.14 |
| Submission ID | `6ab6c41b694b590c3ce2ef67` |
| LOCAL_SHA == REMOTE_SHA | `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c` |
| Mechanism | 32B-safe row-group ownership + shape-aware rows/task scheduling |
| Evidence | `phase4/online/SCHED-ROWGROUP-X/V001/`, `phase4/workspaces/SCHED-ROWGROUP-X/` |

## UB V003 Judge state

| Field | Value |
|---|---|
| Route | UB-LIVENESS-X V003 |
| Status | JUDGE_READY=YES, NOT_FORMALLY_SUBMITTED |
| Frozen SHA | `2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd` |
| Judge Owner | MAIN-1 (designated 2026-09-25) |
| `phase4/online/UB-LIVENESS-X/` | absent |
| Disposition | GATED_WAIT_UB_V003_OFFICIAL — UB-CHAMPION-X not started until result returns |

## MAIN-1 current six routes (read-only for NON_OVERLAP)

Source: `phase4/control/main1-revision-ledger.tsv`, `phase4/control/scheduler.tsv`. MAIN-1 worktrees are READ/WRITE FORBIDDEN for MAIN-2.

| Route | Current revision | Latest single hypothesis | Reserved axis |
|---|---|---|---|
| R31B | V016 | FP16/BF16 wide-row full-y tile 4096→8192 | aggressive multimode / lowp wide tile |
| R31A | V021 | single fence-placement change | conservative exploit / fence placement |
| MIX-A | V007 | remove first SyncVToMTE2 before Load | hybrid architecture / synchronization-removal |
| WIDE-X-FRESH4 | V001 | wide-path tile 2048→4096 | wide-D specialist / wide-path redesign |
| MODE-X-R015C | r4 | map one row per block | multi-row DMA / rows-per-block / mode selection |
| DTYPE-SPECIAL-X | V001 | FP32 aligned local-copy helpers replace Adds(x,0) | dtype-specific specialization |

## NON_OVERLAP_MATRIX (MAIN-2 R2 vs MAIN-1)

| MAIN-2 R2 route | Core mechanism | vs R31B | vs R31A | vs MIX-A | vs WIDE | vs MODE-X | vs DTYPE | Verdict |
|---|---|---|---|---|---|---|---|---|
| SCHED-CHAMPION-X | row ownership / core assignment / 32B row-group scheduling | orthogonal (R31B=lowp wide tile) | orthogonal (R31A=fence) | orthogonal (MIX-A=sync removal) | orthogonal (wide path forbidden here) | orthogonal (MODE=multi-row DMA/rows-per-block) | orthogonal (dtype forbidden here) | PASS |
| REDUCE-HIER-X | RMS square-sum reduction topology only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal (dtype forbidden) | PASS |
| EPILOGUE-FUSE-X | invRms→norm→gamma→bias→store dataflow only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | near-border, scoped to post-RMS arithmetic/dataflow only; must not enter dtype-specific paths | PASS_WITH_SCOPE |
| COEFF-LOCALITY-X | gamma/bias load locality within a tile | orthogonal | orthogonal | orthogonal | orthogonal | near-border if multi-row mode needed — STOP if required | orthogonal | DEFERRED |
| UB-CHAMPION-X | UB lifetime / alias only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | GATED on UB V003 Official |
| VECTOR-MATH-X | invD/mean/sqrt/rsqrt math sequence, unified FP32 intermediate | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | must not specialize per dtype | READY_LATER |

Forbidden MAIN-1 axes that MAIN-2 must not create: broad R31A/R31B exploit evolution, generic multimode evolution, MIX/hybrid architecture, synchronization-removal/fence-placement, wide-D specialist, wide-path redesign, multi-row DMA, rows-per-block/mode-selection, dtype-specific specialization, independent FP16/BF16/FP32 paths.

## SCHED-CHAMPION-X non-duplication audit

| Check | Result |
|---|---|
| Frozen R31B-V011 already has 32B row-group ownership? | **NO.** `Process()` uses only `blockIdx` / `baseRows` / `extraRows` partitioning (lines 171–176). No `rowGroup = 32/gcd(rowBytes,32)` rounding, no whole-group ownership rule. |
| SCHED-ROWGROUP-X V001 has the mechanism? | YES — donor only (`WHY_NOT_DUPLICATE.md`, submission header comments). |
| Equivalent mechanism under another name in frozen champion? | NO evidence found in frozen source search (`rowGroup`, `ownership`, `rowsPerTask` rounding absent). |
| Duplicate of old MAIN-2 SCHED-ROWGROUP-X? | NO — that route is the validated donor on a weak parent (17.14). This route re-applies the same single mechanism onto frozen 45.16 baseline in an independent workspace. |
| NON_DUPLICATION_AUDIT | **PASS** |
| MAIN_APPROVAL | **YES** — first-priority route |

## Route registry (R2 batch 1)

| Route | Branch | Worktree | Parent seed | Status | Priority |
|---|---|---|---|---|---|
| SCHED-CHAMPION-X | `exp/main2-r2-sched-champion` | `cann-main2-r2/SCHED-CHAMPION-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 1 |
| REDUCE-HIER-X | `exp/main2-r2-reduce-hier` | `cann-main2-r2/REDUCE-HIER-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 2 |
| EPILOGUE-FUSE-X | `exp/main2-r2-epilogue-fuse` | `cann-main2-r2/EPILOGUE-FUSE-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 3 |
| COEFF-LOCALITY-X | — | — | — | DEFERRED (batch 2) | 4 |
| VECTOR-MATH-X | — | — | — | DEFERRED (batch 2) | 6 |
| UB-CHAMPION-X | — | — | — | GATED on UB V003 Official | 5 |

## Old MAIN-2 route disposition (evidence donors only)

| Old route | Disposition | Use |
|---|---|---|
| SCHED-ROWGROUP-X | VALIDATED_MECHANISM donor | Official 22.27; mechanism source for SCHED-CHAMPION-X |
| UB-LIVENESS-X | JUDGE_PENDING donor | wait Official before UB-CHAMPION-X |
| ALIGN-TAIL-X | EVIDENCE_ONLY | do not recreate; wide/tail collision with MAIN-1 |
| BATCH-RESIDENT-X | EVIDENCE_ONLY / PARK | do not restore multi-row DMA |
| ASYNC-TRIPLE-X | EVIDENCE_ONLY / PARK | do not restore sync/pipeline route |
| REDUCE-INVSCALE-X | EVIDENCE DONOR ONLY | REDUCE-HIER-X is a new declaration from frozen baseline; old V002 is not parent |

## Rules in force

- One Route = one Agent = one branch = one worktree = one context.
- Track-B (3–5 hypotheses) before any kernel edit. Main approves exactly one hypothesis.
- OFAT. One revision, one conceptual change.
- Do not wait for performance window to compile/link/correctness.
- Local timing follows `phase4/control/local-timing-protocol.md` only.
- Online is scarce: only ONLINE_WORTHY, one at a time, via Judge Owner.
- MAIN-1 worktrees READ/WRITE FORBIDDEN.
- Shared control is Main-owned; Route Agents write only inside their own worktree route dirs.

## Track-B outcome and Main approval (2026-09-27)

| Route | Track-B file | Candidates | Approved V001 hypothesis | Approval doc |
|---|---|---|---|---|
| SCHED-CHAMPION-X | `cann-main2-r2/SCHED-CHAMPION-X/phase4/research/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md` | 4 | H1 GROUP-ALIGNED BALANCED OWNERSHIP | `.../MAIN-APPROVAL-V001.md` |
| REDUCE-HIER-X | `cann-main2-r2/REDUCE-HIER-X/phase4/research/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md` | 4 | H1 Eager running-fold of tile partials | `.../MAIN-APPROVAL-V001.md` |
| EPILOGUE-FUSE-X | `cann-main2-r2/EPILOGUE-FUSE-X/phase4/research/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES.md` | 4 | H1 SCALE-FOLD | `.../MAIN-APPROVAL-V001.md` |

All three H1s are single-variable, orthogonal to MAIN-1 reserved axes, and distinct from old MAIN-2 donors. Implementation of V001 is in progress on the route branches.

## Shared build/correctness findings (2026-09-27)

1. Host-compile of `.asc` on server3 needs `CPLUS_INCLUDE_PATH` / `C_INCLUDE_PATH` to include HCC 7.3.0 C++ headers, else bisheng-generated `asc_plugin_binary_register_code.c` cannot find `<vector>`.
2. Ascend vector `Add` with 1-element operands must be 32B-aligned; a 4-byte offset slot triggers ACL 507035. Use `kSmallFp32ScalarStride` (8) slot alignment.
3. Frozen parent already fails some local-runner goldens on wide FP32 D=16384/32768 (max_abs≈0.44–1.2). These are PRE_EXISTING parent/harness mismatches, not V001 regressions. Do not use those shapes as regression gates until golden is reconciled with Official tolerance.
4. Working NPU probe pattern: single `npu_correctness.asc` ASC executable target linking `ascendcl tiling_api register platform unified_dlog`, not the broken dual-compile host-runner shortcut. Reference: EPILOGUE-FUSE-X `CMakeLists.txt` + `npu_correctness.asc`.

## V001 implementation results (2026-09-27)

| Route | SOURCE_SHA | COMPILE_RC | LINK_RC | CORRECTNESS | LOCAL_VERDICT | Branch commit |
|---|---|---|---|---|---|---|
| SCHED-CHAMPION-X | `ed232872fa1837678a0a05fc16de9f06a725d61ff9d85663193fdc49e8f33678` | 0 (probe/device/submission) | 0 | 24/24 PASS (FP32/FP16/BF16 × 8 shapes) | NOT_COMPLETE / MEASUREMENT_BLOCKED | `exp/main2-r2-sched-champion` |
| REDUCE-HIER-X | `b9c618b3b53fd2988b667f0c6830fa4a7206aef0fde69f92cdb5545ba6503521` | 0 | 0 | PASS (FP32/FP16/BF16 multi-tile + single-tile) | NOT_COMPLETE / MEASUREMENT_BLOCKED | `exp/main2-r2-reduce-hier` |
| EPILOGUE-FUSE-X | `89868a52b59fcaf1d68220398698ef033827a50b1559df9fc69ffcab105dd597` | 0 | 0 | 53/54 PASS (1 pre-existing parent fail 2x16384 FP32 wide) | NOT_COMPLETE / MEASUREMENT_BLOCKED | `exp/main2-r2-epilogue-fuse` pushed |

### SCHED correctness-only repair note

Group-unit ownership exposed a latent `xBuf_` alias race in `ProcessNarrowMidOverlap` BF16: `SetFlag<V_MTE2>(inputRelease)` fired before MTE3 Store drained `xBuf_`. Fixed with `SyncMTE3ToV()` before the flag. Classified as execution-contract E correctness repair; no new performance mechanism. Frozen parent passes the same 24/24 matrix (the race does not fire when each core owns one row).

### Next

1. Qualified device window → same-binary then interleaved P/C under `local-timing-protocol.md` for each route (priority SCHED → REDUCE → EPILOGUE).
2. Batch-2 routes (COEFF-LOCALITY-X, VECTOR-MATH-X) may start research once batch-1 is stable (now true).
3. UB-CHAMPION-X remains gated on UB V003 Official.
4. No ONLINE_WORTHY until local paired deltas exist.

## Official score structure (R31B-V011, 45.16)

`s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)`. Target >50 needs mean ratio time/best < 1.5.

| idx | time_us | best_us | ratio | score | gap driver |
|---:|---:|---:|---:|---:|---|
| 14 | 16486.8 | 3750.1 | 4.40 | 21.50 | largest absolute gap |
| 7 | 52.3 | 14.0 | 3.74 | 23.51 | |
| 1 | 5.4 | 1.7 | 3.20 | 25.85 | |
| 6 | 28.5 | 10.9 | 2.62 | 29.62 | |
| 4 | 16.6 | 6.7 | 2.48 | 30.82 | |
| 8 | 69.2 | 30.2 | 2.29 | 32.80 | |
| 3 | 5.2 | 2.5 | 2.09 | 35.50 | |
| 5 | 9.9 | 5.2 | 1.91 | 38.53 | |
| 10 | 74.8 | 47.1 | 1.59 | 46.64 | |
| 2 | 3.3 | 2.2 | 1.55 | 48.19 | |
| 13 | 563.7 | 393.5 | 1.43 | 53.01 | |
| 12 | 97.7 | 76.4 | 1.28 | 62.29 | |
| 11 | 160.3 | 131.4 | 1.22 | 67.11 | |
| 15 | 9637.5 | 8321.9 | 1.16 | 73.42 | |
| 9 | 71.9 | 68.2 | 1.05 | 88.61 | already near best |

Mean 45.16. To exceed 50 the sum of per-case scores must rise by >72.6. Concentrate wins on idx 14, 7, 1, 6, 4, 8, 3.

## SCHED-CHAMPION-X V001 timing (2026-09-27, d4, warmup=45, device-event)

| shape | same-binary | clean P/C delta | note |
|---|---|---|---|
| 33x100 FP32 (rowGroup=2) | PASS both | **-8.8%** favor V001 (2/3 clean) | only qualified shape |
| 17x256 FP32 (rowGroup=1) | FAIL both | +15% noise | BLOCKED |
| 17x257 FP16 (rowGroup=16) | parent FAIL / cand PASS | **+145.9% favor P 4/4** | parallelism collapse 17→2 cores |
| 7x65 FP32 (rowGroup=8) | parent FAIL / cand PASS | mixed | BLOCKED |

Verdict: NEEDS_ONE_MORE_LOCAL. Mechanism wins when groups fill cores; collapses when rowGroup >> rowCount.

V002 approved: gated group-aligned ownership (apply only when totalGroups >= blockCount).

## REDUCE-HIER-X V001 timing (2026-09-27, d5)

| shape | same-binary | clean P/C delta | note |
|---|---|---|---|
| 8x8192 FP32 | PASS both | -3.8% (n=2 clean) | only qualified shape |
| 1x8192/4096/2048/2x256 | FAIL | — | BLOCKED (bimodal short-kernel samples under VLLM) |

Verdict: NEEDS_ONE_MORE_LOCAL. H1 fold signal is small; pursue higher-win reduction topology (manual vector tree / hierarchical) as V002.

## SCHED-CHAMPION-X V002

SOURCE_SHA `de1e93c74338802e646258ada08a8cac1031496a9985e585a5a89015b81bc990`
Gate: `totalGroups * 2 >= min(blockCount, rowCount)` — group split keeps win on 33x100, avoids 17x257 collapse.
Correctness 24/24. Timing in progress (d4).

## Cycle status 2026-09-27 late

| Route | Verdict | Signal | Next |
|---|---|---|---|
| SCHED V001 | NEEDS_ONE_MORE_LOCAL | 33x100 -8.8% (1 shape) | superseded by V002 |
| SCHED V002 | NEEDS_ONE_MORE_LOCAL | gate OK (17x257 collapse fixed); 33x100 unmeasured (noise) | remeasure 33x100 |
| REDUCE V001 | NEEDS_ONE_MORE_LOCAL | 8x8192 -3.8% | superseded by V002 H2 |
| REDUCE V002 | IMPLEMENTING | H2 manual vector tree, est -18~-35% large-D | build+correctness |
| EPILOGUE V001 | NEEDS_ONE_MORE_LOCAL | 4x8192 -5.5/-9% within ±5% band | KEEP_ACCUMULATING; H2 research |

No ONLINE_WORTHY yet. Measurement host noise (VLLM residual) is the main blocker for local verdicts.

## SCHED-CHAMPION-X V002 remeasure → LOCAL_ACCEPTED (2026-09-27)

33x100 FP32: 5/6 clean pairs, **5/5 favor V002**, median **-4.4%** (range -1.5%..-11.2%), order-robust.
17x257 FP16: +5.2% (3/4 clean) — gate holds; no V001 collapse.

Decision: **LOCAL_ACCEPTED**, LOCAL_BEST=V002 (SOURCE_SHA `de1e93c7...`).
ONLINE_WORTHY=NO — KEEP_ACCUMULATING. Small stable win; stacking more mechanisms before any Judge submission (calibration warns small local proxies can be misleading).

## REDUCE-HIER-X V002 H2 refuted (2026-09-27)

| shape | same-binary | clean Δ | verdict |
|---|---|---|---|
| 1x32768 FP32 (tileCount=8) | PASS both | **+6.6% favor P 5/0** | regression |
| 8x8192 FP32 | PASS both | +0.9% | neutral |
| 1x16384/8192/4096 | FAIL | — | BLOCKED |

LOCAL_REJECTED. Root cause: `VectorReduceTo8` pairwise `Add` tree pays `PipeBarrier<PIPE_V>` per level; barrier chain costs more than the saved `ReduceSum` V/S.
Rollback to frozen parent. Next: reduce barrier count (4-way tree) or shorten `ReduceSum` span instead of replacing it.

## EPILOGUE-FUSE-X V002 VMLA refuted (2026-09-27)

64x8192 FP32 (actual VMLA path): 6/6 favor parent, median **+3.4%** regression.
In-place accumulator + per-row bias reload + SyncVToMTE2 costs more than vmla fusion saves.
LOCAL_REJECTED. Rollback to frozen parent.

Note: ProcessFp32FullRowOutputPipelined only runs when localRows>1 (rowCount > coreCount=40).

## Cumulative local verdicts (end of this cycle)

| Route | Rev | Verdict | Best signal |
|---|---|---|---|
| SCHED-CHAMPION-X | V002 | **LOCAL_ACCEPTED** | 33x100 -4.4% 5/5 |
| REDUCE-HIER-X | V001 | NEEDS_ONE_MORE | 8x8192 -3.8% |
| REDUCE-HIER-X | V002 | LOCAL_REJECTED | 1x32768 +6.6% |
| REDUCE-HIER-X | V003 | timing | H3 short-span |
| EPILOGUE-FUSE-X | V001 | NEEDS_ONE_MORE | within ±5% |
| EPILOGUE-FUSE-X | V002 | LOCAL_REJECTED | 64x8192 +3.4% |

LOCAL_BEST chain: only SCHED V002. No ONLINE_WORTHY. Official still 45.16.

## REDUCE-HIER-X V003 + bottleneck pivot (2026-09-27)

| shape | Δ | verdict |
|---|---|---|
| 1x32768 FP32 primary | -0.5% mixed | neutral |
| 8x8192 FP32 | -3.3% | small, near noise |
| 1x16384 FP32 | -1.5% 4/0 (parent same-binary FAIL) | direction only |

**Three reduction-topology variants (V001 fold, V002 tree, V003 short-span) all failed to win on large-D. Reduction V/S is NOT the primary bottleneck on frozen R31B-V011. DMA + output pass are more likely.**

REDUCE-HIER-X: KEEP as small-win accumulator (V001 -3.8% / V003 ~neutral). No more reduction-topology revisions until a new bottleneck evidence appears.

PIVOT: start COEFF-LOCALITY-X (gamma/bias load locality) — allowed batch-2 route, orthogonal to MAIN-1, targets parameter DMA traffic which may be the real bottleneck.

## SCHED-CHAMPION-X PARK (2026-09-27)

V003 tail-group folding: LOCAL_REJECTED (33x100 +0.9%). LOCAL_BEST remains V002 (de1e93c7, 33x100 -4.4%).

Structural ceiling confirmed: `rowGroup>1` iff `width%8!=0`; batched compute requires `width%8==0`. Ownership lane cannot unlock batched compute. Param-residency channel exhausted.

**PARK ownership lane.** Retain V002 as LOCAL_BEST. No further ownership revisions.

## COEFF-LOCALITY-X Track-B + V001 approval (2026-09-27)

Load-map audit: 8 gamma/bias sites. Only `ProcessWideFp32FullCacheRows` (D>8192 FP32)
reloads params per tile per batch with no prefetch. Sibling `ProcessWideLowPrecision`
already has the V011 2-deep param MTE2 pipeline.

Approved H1: transplant that prefetch into FP32 wide output pass.
Est. -8% to -25% on D>8192 FP32 — first hypothesis sized to the Official case-14 gap.

## COEFF-LOCALITY-X V001 refuted + large-D bottleneck conclusion (2026-09-27)

| shape | Δ | note |
|---|---|---|
| 1x32768 FP32 PRIMARY | **+19.2%** favP 5/0 | regression; tileElems 4096→2560, tileCount 8→13 |
| 1x16384 FP32 (tileElems unchanged) | +1.8% noise | param prefetch alone ≈ 0 |
| 8x32768 FP32 | +12.2% favP | regression direction |

LOCAL_REJECTED. Staging UB stole tile budget.

**FOUR large-D variants have now failed (REDUCE ×3, COEFF ×1). Large-D bottleneck is NOT reduction V/S and NOT param MTE2.** Remaining suspects inside MAIN-2 scope: input x/residual DMA (ASYNC lane, parked), output store (EPILOGUE, small), compute apply itself, or mode selection (MAIN-1).

Next: COEFF H2 stripe residency with tileElems pinned at 4096; VECTOR-MATH-X (rsqrt sequence).

## COEFF-LOCALITY-X V002 NO_UB_BUDGET (2026-09-27)

tileElems=4096 leaves only ~16 KiB UB; gamma+bias tile needs 32 KiB. No stripe fits.
Also host clamps blockCount to rowCount → measured shapes have localRows=1 → no cross-batch reload to skip.

H1 REJECTED, H2 STOPPED, H3 ceiling small. **PARK COEFF-LOCALITY-X** unless H3 is explicitly requested.

## VECTOR-MATH-X V001 + INTEGRATION-X start (2026-09-27)

VECTOR-MATH V001 (vector denominator): batched 32x256 **-7.3% 3/3**, 8x256 ~-8%. KEEP_ACCUMULATING. Rsqrt rejected (fast-approx). LOCAL_BEST=V001 for batched.

INTEGRATION-X approved: merge SCHED V002 (de1e93c7) + VECTOR-MATH V001 (dbe776f9) on frozen parent. Two validated mechanisms, no third. Target multi-shape wins → ONLINE_WORTHY → Main submits.

## OFFICIAL SUBMISSION 1 — INTEGRATION-X V001 (2026-09-27)

| Field | Value |
|---|---|
| Submission ID | `6ab891a7694b590c3cd47555` |
| LOCAL_SHA == REMOTE_SHA | `52f3a329703e5ea17dda12ea275911b9354ce25d6e50fd60ed981fe075a0da12` |
| Correctness | 15/15 |
| Official Score | **41.93** |
| Official Anchor | 45.16 (R31B-V011) |
| Delta | **-3.23** |
| Decision | **REJECT** |

Per-case vs R31B-V011: cases 4/6/7 regressed sharply (16.55→31.32, 28.46→49.04, 52.34→76.45 µs).
Local 33x100 -4.4% did not transfer; mid-size Official cases paid for the group-split / vector-denominator changes.

**Champion unchanged: R31B-V011 45.16.** Calibration FALSE_POSITIVE recorded.
Evidence: `phase4/online/INTEGRATION-X/V001/`.

Online queue this cycle: 1/2 used.

## MAIN DECISION 2026-09-27 (post Official)

INTEGRATION-X V001 = REJECT (41.93 vs 45.16). Do not create V002 from it.
Do not reintroduce SCHED row-group splitting into Champion path.
Parent lineage returns to FROZEN R31B-V011 (45.16).

P0: VECTOR-MATH V002 ONLY (broadcast Mul). No SCHED/DMA/store/mode/dtype/sync combination.
P1: timing must cover small+medium+large; primary question is no medium-shape regression.
P2: Online budget remaining = 1; do not consume without Main ONLINE_WORTHY.
P3: research-only OUTPUT-STORE / EPILOGUE-STORE path (no implementation).
