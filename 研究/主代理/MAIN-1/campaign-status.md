# MAIN-1 LONG-RUN CHAMPION-EXPLOIT CAMPAIGN STATUS

- Campaign: LONG-RUN / MULTI-AGENT / CHAMPION-EXPLOIT
- Planning approval: 本轮正式批准进入
- Date: 2026-09-29
- Main-1 HEAD (campaign start): `dac227c6`
- Main-1 worktree: `/Users/sunyiyang/Desktop/Project/cann-main1`
- Main-1 branch: `main1/champion-exploit`

## Approved 5 Active Lanes

| Lane | Route | Branch | Worktree | Official Best | Agent |
|---|---|---|---|---|---|
| M1-1 | R31B | `m1/r31b-exploit` | `../cann-m1-r31b` | V011 = 45.16 | general-1 |
| M1-2 | STORE-EPILOGUE-X | `m1/store-epilogue` | `../cann-m1-store` | V002 = 45.07 | general-2 |
| M1-3 | R31A | `m1/r31a-exploit` | `../cann-m1-r31a` | V016 = 45.00 | general-3 |
| M1-4 | SHAPE-TILING-CHAMPION-X | `m1/shape-tiling-champion` | `../cann-m1-tiling` | 无（新） | general-4 |
| M1-5 | EPILOGUE-ARITH-CHAMPION-X | `m1/epilogue-arith-champion` | `../cann-m1-epi-arith` | 无（新） | general-5 |

## Seed Sources

| Route | Seed | SOURCE_SHA256 |
|---|---|---|
| R31B / SHAPE-TILING / EPILOGUE-ARITH | `线上结果/R31B/V011/submission.asc` | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| STORE-EPILOGUE-X | `本地实验/STORE-EPILOGUE-X/V002/submission.asc` | `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839` |
| R31A | `线上结果/R31A/V016/submission.asc` | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |

## Lane Boundaries

| Lane | 允许 | 禁止 |
|---|---|---|
| M1-1 R31B | Champion 路径单变量微架构 | 一次改 tile+reduction+store+dtype+scheduling |
| M1-2 STORE | store/writeback/output transaction | reduction/dtype/scheduling/multirow DMA/tiling |
| M1-3 R31A | affine/Rsqrt/路径选择/局部数据流 | 复制 R31B 整套实现 |
| M1-4 TILING | shape-conditioned tiling / tile-UB 权衡 | reduction/store/dtype/scheduling（须先去重） |
| M1-5 EPI-ARITH | RMS 后 store 前算术链 | store transaction/DMA/scheduling/tiling/dtype/reduction |

## Budget

- SOFT BUDGET = 6 closed performance Revisions per lane
- HARD MAX = 10 per lane, then return to Planning
- 3 consecutive no-gain OFAT → `LANE_NEEDS_PLANNING_REVIEW` (not self-PARK)

## Git / Worktree Status (campaign start)

- Main-1 worktree clean, remote == local at `dac227c6`
- 5 child worktrees created from `dac227c6`, all pushed with `-u`
- Main-2 has separate worktrees (`cann-m2-*`); 不干扰

## Progress Log

| Date | Event |
|---|---|
| 2026-09-29 | Campaign start; canonical merged (`347e26f4` → `dac227c6`); 5 worktrees created; 5 agents spawned |

## Progress Log (continued)

| Date | Event |
|---|---|
| 2026-09-29 | SHAPE-TILING V001: BUILD PASS; found wide-FP32 invRms nondeterminism (~0.8%); OFAT safety PASS; timing gated |
| 2026-09-29 | STORE-EPILOGUE: Track-B next-hypotheses.md produced (store gap analysis) |
| 2026-09-29 | R31A: same-binary + paired measurements running on D24576/D32768 |
| 2026-09-29 | Main-1 review sent to SHAPE-TILING: allow same-binary P/C with COMMON_MODE_WITH_PARENT documentation |
| 2026-09-29 | R31B / EPILOGUE-ARITH: still in Track-B research |

## Cross-cutting Finding (needs Planning awareness)

**Wide-FP32 invRms nondeterminism in Champion `ProcessWideFp32FullCacheRows`:**
- invRms varies ~0.8% across runs of same binary/input
- Suspected: `kWideFullYReduceStride=16` tail partial slots unread/uninitialized (tileCount typically 3/4/8)
- FP16/BF16 deterministic; BF16 quantization hides the drift
- Official Judge scores this exact source 15/15, so official cases don't expose it
- Impact: any local P/C on wide-FP32 path is noise-dominated; correctness vs golden is not adjudicable
- Evidence: `cann-m1-tiling/研究/SHAPE-TILING-CHAMPION-X/handoff-2026-09-28.md`, `local-result.json`
| 2026-09-29 | STORE-EPILOGUE: V002 closed LOCAL_BEST; Track-B 5 hypotheses; Main selected H1 for V003 |
| 2026-09-29 | R31A: V021 solidified NEEDS_ONE_MORE_LOCAL; path-dispatch finding (D24576→batch); Main selected H3 for V022 |
| 2026-09-29 | SHAPE-TILING: V001 NEEDS_ONE_MORE_LOCAL (in noise band); Main selected H2 for V002 |
| 2026-09-29 | Wide-FP32 invRms nondeterminism forwarded to R31B lane |
| 2026-09-29 | SHAPE-TILING: V002 NEEDS_ONE_MORE_LOCAL (tile rounds not bottleneck at rows=2); directed multi-row re-measure before V003 |
| 2026-09-29 | Tiling multiscale: tile axis falsified (magnitude check); case14 gap = per-tile sync/pipeline depth; forwarded to R31B & EPILOGUE-ARITH |
| 2026-09-29 | R31A V022 LOCAL_REJECTED (H3 falsified +4.33% on D24576); Main selected H1 for V023 on D32768 |
| 2026-09-29 | Tiling: segmented timing + msprof confirm pipeline-overlap is case14 gap (1.23/4); LANE_NEEDS_PLANNING_REVIEW; forwarded to R31B & EPILOGUE-ARITH |
| 2026-09-29 | R31A V023 NEEDS_ONE_MORE_LOCAL; **Rsqrt is ~2^-10 approx on 910B3** (max_abs 9.94e-4); H2 selected for V024 |
| 2026-09-29 | EPILOGUE-ARITH V001 first signal: 1x32768 -1.51% 6/6 (noise band ±3-5%); clean remeasure directed |
| 2026-09-29 | **R31A V024 LOCAL_ACCEPTED -3.66% 8/8 dual-device**; LOCAL_BEST V016→V024; ONLINE_RECOMMENDATION WORTHY (trigger A); package ready for Judge Owner |
| 2026-09-29 | **R31B V016 LOCAL_ACCEPTED fp16-wide-d32768 -6.5~-7.0%**; LOCAL_BEST=V016; ONLINE_RECOMMENDATION WORTHY (trigger A); H1 selected for V017 |
| 2026-09-29 | Two Online candidates ready: R31A V024 + R31B V016 |
| 2026-09-29 | **R31A V025 LOCAL_ACCEPTED** incremental -4.12%; chain V016→V024→V025 ~-6.8%; trigger B met; ONLINE WORTHY V025 |
| 2026-09-29 | Methodology: OFAT parent module must be DIRECT_PARENT (not default V016) |
| 2026-09-29 | EPILOGUE-ARITH V001: 24/31 favor C p=0.0017 but noise band 3-5%; Main rules Option A (tighter window d6+gap=5s) |
| 2026-09-29 | R31A Track-B v2: 4 hypotheses (H5-H8); Main selected H5 for V026 (pass-2 param staging) |
| 2026-09-29 | **R31A V026 LOCAL_ACCEPTED** -1.69% incr; chain -8.3%; trigger B (3 consecutive); ONLINE WORTHY primary V026 |
| 2026-09-29 | **EPILOGUE-ARITH V001 LOCAL_ACCEPTED** NORM-HOIST -2.55%; acceptance band = shape's own noise; ONLINE WORTHY (priority 3) |
| 2026-09-29 | **R31B V017 LOCAL_ACCEPTED** bf16-wide -14.3% + fp16 -6.7~8.5%; trigger B; ONLINE WORTHY primary candidate |
| 2026-09-29 | R31A Track-B v3: H9 barrier-redundancy selected for V028 |
| 2026-09-29 | **R31A V028 LOCAL_ACCEPTED** batch -1.09%; barrier non-load-bearing finding; V028 superset of V026; ONLINE WORTHY final |
| 2026-09-29 | R31A lane closing: LANE_NEEDS_PLANNING_REVIEW after V028 |
| 2026-09-29 | R31A final handoff + LANE_NEEDS_PLANNING_REVIEW; two lanes now await Planning (R31A, TILING) |
| 2026-09-29 | R31B hypothesis pool updated with barrier calibration; H2 selected for V018 |
| 2026-09-29 | Three active lane agents cancelled; replacement workers spawned (general-6/7/8) inheriting R31B V018, STORE V003, EPI V002 |
| 2026-09-29 | R31B V018 NEEDS_ONE_MORE_LOCAL (H2 refuted: pass-1 not V-bound); H3 selected for V019 |
| 2026-09-29 | EPI V002 EA-H2 strong signal -6.64% 6/6 (differential -19pp); same-binary blocked; clean remeasure directed; precision regression on golden-error shapes not blocking |
| 2026-09-29 | R31B V019 NEEDS_ONE_MORE_LOCAL (H3 refuted); V-side deletions 2x null; bottleneck=MTE2/reduce; H5 selected for V020 |
