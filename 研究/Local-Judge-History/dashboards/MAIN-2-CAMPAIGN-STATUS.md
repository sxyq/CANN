# MAIN-2 Long-Run Orthogonal Explore Campaign

- 授权来源：C2C CONTROL / Planning Review Layer（本轮消息）
- 阶段：LONG-RUN / MULTI-AGENT / ORTHOGONAL-EXPLORE
- 记录日期：2026-09-29
- 记录者：MAIN-2

## 0. Canonical 同步

| 项 | 值 |
|---|---|
| 动作 | `git fetch origin` + `git merge origin/main` + `push` |
| MERGE_BASE | `141a6549` |
| MAIN2_HEAD | `7fd752f75fb38c3f95dd29374ea79cb8375e03c3` |
| REMOTE_HEAD | 同上（local == remote） |
| 冲突 | 无 Candidate/evidence 冲突；origin/main 仅改 `调度/主代理分工.md` |
| 禁止项遵守 | 未 rebase / 未 reset / 未 force push |

## 1. Active Lane 与 worktree

| Lane | Route | Branch | Worktree | Child Agent | Official Best | 状态 |
|---|---|---|---|---|---|---|
| M2-1 | REDUCE-HIER-X | `m2/reduce-hier` | `../cann-m2-reduce` | general-1 | V003=44.24 | research phase |
| M2-2 | VECTOR-MATH-X | `m2/vector-math` | `../cann-m2-vector` | general-2 | V001=44.22 | research phase |
| M2-3 | COEFF-LOCALITY-X | `m2/coeff-locality` | `../cann-m2-coeff` | general-3 | V001=44.16 | research phase |
| M2-4 | ASYNC-OVERLAP-CHAMPION-X | `m2/async-overlap-champion` | `../cann-m2-async` | general-4 | 无 | new route research |
| M2-5 | MULTIROW-DMA-CHAMPION-X | `m2/multirow-dma-champion` | `../cann-m2-multirow` | general-5 | 无 | new route research |

五条 branch 均已 `push -u origin`。每个 child 独立 worktree / branch / context，不共享可写 Candidate tree。

## 2. Evidence-only（非 Active）

| Route | 本轮用途 |
|---|---|
| DTYPE-SPECIAL-X | 只读证据 / reference |
| SCHED-CHAMPION-X | 调度证据 / donor reference |
| UB-LIVENESS-X | correctness-gap 证据；禁止 MAIN-2 建 V004 |

## 3. 预算与闭环规则

- 每 Lane soft = 6 Revision，有新信息/新收益可到 10。
- 连续 3 个独立 Revision 无收益且无新信息 → `LANE_NEEDS_PLANNING_REVIEW`（不是 PARK）。
- 当前 Revision 未闭环，不得创建下一 Revision。
- MAIN-2 可在 5 条 lane 内批准下一 OFAT hypothesis，但不写 Kernel。
- 不开第 6 条路线。
- PLANNING_DECISIONS_CHANGED = 0（本轮不改 Planning 生命周期决定）。
- NEW_ROUTES_OUTSIDE_APPROVED_5 = 0。

## 4. Online 条件式批准

- CONDITIONAL ONLINE APPROVAL：单 Revision 显著稳定超噪，或同一 Local Best chain 连续 2–3 次 LOCAL_ACCEPTED。
- 触发后 MAIN-2 做 Main Review → 统一 Judge Owner 提交。
- Route child 不提交 Online。

## 5. Git 纪律

- 一个技术改动 / server build / correctness / performance / Online / rollback 各自独立 commit+push。
- 禁止把代码修改、server result、rollback、下一技术假设塞进同一巨大 commit。
- LOCAL_REJECTED：保留失败源码与 evidence → commit/push → 显式 revert 到 LOCAL_BEST → push → 再开下一 Revision。
- 禁止 reset / 删 commit / 覆盖历史。
- Main-2 branch 只存控制、全局记录、evidence reference、Online package；child Kernel 留在各自 branch。

## 6. 共享总账更新权

仅 MAIN-2 单线程更新（且只改自己 Route 行）：

- `技术路线/全版本记录.tsv`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `调度/本地线上校准.tsv`
- `研究/主代理/MAIN-2/本地评分器校准.md`

## 7. 进度日志

### 2026-09-29 · campaign start

- canonical merge 完成，HEAD `7fd752f7`。
- 5 worktree 创建并 push。
- 5 child 已 spawn（general-1..5），第一阶段任务均为只读研究 + hypothesis package handoff，等待 MAIN-2 批准后才写 Kernel。

### 2026-09-28/29 · first implementation wave

| Lane | Revs closed | 最佳 | 状态 |
|---|---|---|---|
| M2-1 REDUCE-HIER-X | V004 LOCAL_REJECTED, V005 LOCAL_REJECTED | Official 44.24 (V003) | **LANE_NEEDS_PLANNING_REVIEW**（五变体全伪） |
| M2-2 VECTOR-MATH-X | V001–V003 + S3 probe 已收口 | Official 44.22 (V001) | **LANE_NEEDS_PLANNING_REVIEW**（SEQ-FUSE-2 轴关闭） |
| M2-3 COEFF-LOCALITY-X | V003 LOCAL_REJECTED, V004 NEEDS_ONE_MORE_LOCAL | Official 44.16 (V001) | **LANE_NEEDS_PLANNING_REVIEW**（四维探尽） |
| M2-4 ASYNC-OVERLAP-CHAMPION-X | V001 LOCAL_ACCEPTED→Official 44.17 REJECT; V002–V004 mixed | LOCAL_BEST=V001 | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-5 MULTIROW-DMA-CHAMPION-X | V001 LOCAL_REJECTED, V002 NEEDS_ONE_MORE_LOCAL 终局 | none | **LANE_NEEDS_PLANNING_REVIEW**（等 C2 裁定） |

Online：ASYNC V001 以及 Main-1 的 R31B V017、R31A V028、STORE V003、EPI V002 均已完成正式提交；五条均 15/15 PASS，但均低于各自提交参照。

关键发现：
- 归约轴五变体全伪；父版 ReduceSum 接近最优。
- param MTE2 时序在常量 tileElems 下证伪。
- 命令计数与指令形态均非 DMA 主导成本。
- NarrowMid 调度类改动天花板（每行 squareSum 读取硬下限）。
- **测量装置 candidate 侧偏置 ~2.8%**；p10 在带载窗口对小信号乐观偏置。
- vcadd mode=0 输出粒度为每 64 FP32 一个和。
- DataCopyParams.blockLen 为 32B 单位。

Planning 待裁：C2 三选项、UB-GAP-CLUE、四条 lane 生命周期、W3 vs STORE-EPILOGUE 划界、null-binary 对照授权、Official case shape map。

---

## 8. 最终回执（2026-09-28/29 campaign wave-1）

### HEAD

| 项 | 值 |
|---|---|
| MAIN2_HEAD | `2da5620a88ab0827b8d2be839f0c60264a1d92bf` |
| REMOTE_HEAD | 同上（local == remote） |

### 五条 Lane

| Lane | closed | accepted | rejected | corr-fail | server best | online | official best | next | lane state |
|---|---|---|---|---|---|---|---|---|---|
| M2-1 REDUCE-HIER-X | 5 (V001–V005) | 0 | 2 (V002,V004,V005) | 0 | Official 44.24 V003 | 0 new | 44.24 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-2 VECTOR-MATH-X | 3 (V001–V003)+S3 | 0 (V001 batched hist.) | 0 | 0 | Official 44.22 V001 | 0 new | 44.22 | Planning | **LANE_NEEDS_PLANNING_REVIEW**（SEQ-FUSE-2 轴关闭） |
| M2-3 COEFF-LOCALITY-X | 4 (V001–V004) | 0 | 2 (V001,V003) | 0 | Official 44.16 V001 | 0 new | 44.16 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-4 ASYNC-OVERLAP-CHAMPION-X | 4 (V001–V004) | 1 (V001) | 0 | 0 | LOCAL_BEST V001 | 1 (44.17 REJECT) | 44.17 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-5 MULTIROW-DMA-CHAMPION-X | 2 (V001–V002) | 0 | 1 (V001) | 0 | none | 0 | none | Planning（C2） | **LANE_NEEDS_PLANNING_REVIEW** |

### SERVER

| 项 | 值 |
|---|---|
| max compile concurrency | 5 lanes 并行（每 lane 串行） |
| max jobs/card | ≤1 性能任务/卡（正式测时） |
| HBM block | 编译按 FREE_HBM≥100MB；未因 AICore/VLLM/d7 停编译 |
| performance runs | REDUCE V004/V005、VECTOR V003+S3、COEFF V003/V004、ASYNC V001–V004、MULTIROW V001/V002 均完成配对或记 BLOCKED |

### GIT

| 项 | 值 |
|---|---|
| main2 commit count | 22（自 BASE 141a6549） |
| lane push count | reduce 19 / vector 10 / coeff 11 / async 27 / multirow 21 |
| revert count | 2 显式回退（REDUCE V004、V005 → FROZEN）；MULTIROW V001 回退 |

### CALIBRATION

| 项 | 值 |
|---|---|
| new online | 5（ASYNC V001 + Main-1 四条；均 15/15 PASS） |
| false positive | 5（五个重点样本均为 Local 正向、Official 下降） |
| false negative | 0 新增（COEFF V001 历史 FN 保留） |
| evaluator proposal | SERVER_EVALUATOR_CANDIDATE_V1 草案（SHADOW_ONLY，未启用） |

### 约束遵守

```text
PLANNING_DECISIONS_CHANGED = 0
NEW_ROUTES_OUTSIDE_APPROVED_5 = 0
```

### 关键可复用事实

1. 归约轴五变体全伪；父版 `ReduceSum` 接近最优。
2. V/S handoff 成本 0.049 µs/row；纯 V 链更贵 +0.012。
3. param MTE2 时序非 large-D 瓶颈（常量 tileElems 证伪）。
4. DMA 命令计数与指令形态均非主导；流水重叠才是。
5. NarrowMid 每行 squareSum 读取是硬下限。
6. **测量装置 candidate 侧偏置 ~2.8%**；p10 带载乐观偏置。
7. S1 形状加长（9–15µs 带）可绕开短 kernel same-binary 门槛。
8. vcadd mode=0 每 64 FP32 一和；DataCopyParams.blockLen=32B。
9. FullCache invRms 占用 xBuf_ 限制 prologue 窗口。
10. UB-GAP-CLUE：FROZEN parent FP32 wide 非确定（bad 随 run 变）。

### STOP 条件

五条 lane 全部 `等待 Planning`，wave-1 收口。等待 Planning 裁定生命周期、C2 取舍、UB-GAP、W3 划界、null-binary 授权与 case map。

---

## Judge V4 / performance restart gate — 2026-10-05

| 项 | 值 |
|---|---|
| direct parent | R31B V011 (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`) |
| Official Champion | 45.16 |
| calibration candidates | 5/5 Build PASS；5/5 unified Suite V2 correctness 7/7 PASS |
| formal local vectors | 0/5；d4 bounded qualification failed，d5 safe retry also failed |
| V4 nested LOO | MAE 6.462037；Spearman 0.678322；Kendall 0.454545；pairwise 0.727273 |
| V4 Route-out | MAE 5.281435；Spearman 0.734266；Kendall 0.545455；pairwise 0.772727 |
| near-Champion pairwise | 0.333333 (3 pairs) |
| Judge level | LEVEL 1；broad rejection/research prioritization only |
| H1/H3 | conditional TOP-1/TOP-2 for Planning；`MAIN_SELECTED=NONE` |
| Online | NONE；no submission |
| remote evidence head | `baade9a48cc0aacded17043492f2fb8d21c4f0b0`；已验证 |
| remote status head | `bdd23bffb235347873d171c64fb747035799dd5`；已验证 |
| evidence push | 本轮 engineering evidence 与 gate 的 push 已被服务器确认：`1825d393` |
| final status-only commit | `97571aeb`；push 与 remote SHA query 因 GitHub 443 超时，PUSH_PENDING_NETWORK |

### Engineering calibration vector completion — 2026-10-05

| 项 | 值 |
|---|---|
| mode | `ENGINEERING_3RUN`；formal qualification 不作为 timing gate |
| vectors | 5/5 candidates；20/20 core cells；15/15 diagnostic cells |
| runs | 105/105 valid Parent/Candidate runs；P-C/C-P/P-C |
| quality | GOOD 12；FAIR 8；POOR 15；INVALID 0 |
| primary/fallback | d4 used for all；d7 未使用 |
| TOP-1 | HOTLOOP-BRANCH-HOIST-CHAMPION-X/V001 |
| TOP-2 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X/V001 |
| TOP-3 | HOTLOOP-ADDR-HOIST-CHAMPION-X/V002 |
| Online | 0；仅建议 Planning 获取 calibration labels |
| Judge | Level 1；prediction 不是 Official Score |

工程 vectors 补齐了 calibration 输入，但不改变 formal Local verdict，
不推进 LOCAL_BEST，不解除 `MAIN_SELECTED=NONE`，不创建新 Kernel Revision。

正式测时被 Parent noise-floor qualification 阻断，未生成任何候选 Local
Score；不得据此给五个候选排序或进入 Online。详见
`PERFORMANCE-NEXT-WAVE-GATE.md`。

---

## 9. Wave-2 Next Score Wave — 2026-10-03

```text
OFFICIAL_CHAMPION=R31B V011
OFFICIAL_SCORE=45.16
DIRECT_ONLINE_SUBMISSION=0
```

### Current local engineering scores

| Route | Revision | Local score | Quality / state |
|---|---|---:|---|
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V001 / H3 | -24.193320% | POOR |
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V002 / H1 | +0.954352% | POOR / LOCAL_NEUTRAL; no V003 |
| UB-LIVENESS-X | V001 | -20.878573% | POOR / engineering-reference only; no V002 |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | V002 / HBH-09 | -0.048802% | POOR / Planning review; no V003 |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X | V001 / RFH-1 | +0.553205% | POOR / LOCAL_NEUTRAL; no V002 |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X | none | NO SCORE | H1/H2/H3 final gate FAIL; MAIN_SELECTED=NONE |

### New events

- REDUCE V001 exact R31B V011 sibling baseline and RFH-1 declaration:
  `w2/m2/reduce-finalize` commit `c7bad949`; parent/submission SHA is
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
  Candidate source `83d569d2…f78338df3` is commit `15803855`. Build PASS is
  committed at `bc558fa3`; correctness PASS at `3052b573` (BF16 [1,32768],
  FP16/BF16 [1,16384], BF16 [1,4096] control; all bad=0). Device-event runner
  probes are built (`a875d789`). Three interleaved BF16 [1,32768] device-event
  pairs produced Parent average 20.486667 us and Candidate average 20.600000
  us (+0.553205%, Candidate slower); pair directions were mixed. Parent
  same-binary MAD/median=18.56%, so quality is POOR and verdict is
  LOCAL_NEUTRAL, engineering-score only; no Local Best advance. Raw/performance
  evidence commit `e9cc2dfb`, score commit `0c07dcd7`. d5 lease
  `M2-REDUCE-V001-D5-20261003T182337Z` was released (control commit
  `e81b82b1`). Direct Online is disabled; no V002.
- TILECOUNT final gate is committed in `w2/m2/tilecount-unroll` at
  `e36220ed`. Exact Parent source and compile artifact identity were found,
  but the available HiIPU device binary could not be disassembled to verify
  loop backedges/index work. All three hypotheses fail the required codegen
  gate; this is `UNVERIFIED`, not evidence that the compiler already unrolled
  them. No V001 was created.

### Git delivery

```text
PUSH=PUSH_PENDING
REASON=git ls-remote origin HEAD timed out (RC 124, 10s); no force push or retry.
LOCAL_REDUCE_HEAD=0c07dcd7
LOCAL_TILECOUNT_HEAD=e36220ed
```

```text
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

## 10. Continuous Iteration Wave — 2026-10-04

### Push-first audit

The actual local branches all have the incorrect configured upstream `origin/main`; the explicit route target refs were audited separately. Existing build outputs and other untracked evidence were retained.

This table is the pre-refresh snapshot; the MAIN-2 control HEAD shown is the parent of this dashboard update. The resulting control HEAD is the dashboard commit itself.

| Lane | Local HEAD | Configured upstream relation | Target branch | Remote HEAD | Ahead / behind target | Push state |
|---|---|---|---|---|---|
| ADDR | `fdaf5b3815f12484d0c8b7341782746f1a669c72` | `origin/main` (+45 / -47) | `w2/m2/hotloop-addr` | absent | N/A | `PUSH_PENDING_AUTH (rc=128)` |
| BRANCH | `70735a73605f3cabb41a6c0953ee0d05a1bef592` | `origin/main` (+73 / -47) | `w2/m2/hotloop-branch` | absent | N/A | `PUSH_PENDING_AUTH (rc=128)` |
| UB | `b34f94b2c214574c0237f1b677b066b484f98ec7` | `origin/main` (+28 / -47) | `w2/m2/ub-lifetime-safe` | `a9c9affcb49e8591f803ab409aafa6192a34caca` | local +28 / remote +2 | `PUSH_PENDING_DIVERGED; no force/merge` |
| REDUCE | `52bd18a86872bdbc4f1d79b71ed65240e9977234` | `origin/main` (+14 / -47) | `w2/m2/reduce-finalize` | absent | N/A | `PUSH_PENDING_AUTH (rc=128)` |
| TILECOUNT | `38ff22e593730f041e134357c6087bcce981c229` | `origin/main` (+6 / -47) | `w2/m2/tilecount-unroll` | absent | N/A | `PUSH_PENDING_AUTH (rc=128)` |
| MAIN-2 control | `b7559b3633597243032088275ce4064395db3b67` | `origin/main` (+80 / -47) | `w2/main2/control` | absent | N/A | `PUSH_PENDING_AUTH (rc=128)` |

After the research commits, one bounded non-interactive normal-push window was attempted for the five absent target branches; all returned `fatal: could not read Username for 'https://github.com': terminal prompts disabled` (RC 128). A successful follow-up `ls-remote` reconfirmed those refs are absent and UB remains at `a9c9affc`. No Git credential helper or `gh` CLI is available. SSH fallback did not pass host-key verification, and no SSH trust/configuration was changed. `PUSHED_HEAD=NONE`; no `LOCAL_HEAD == REMOTE_HEAD` claim is made. UB is not pushed because it is non-fast-forward (28 local-only / 2 remote-only); no force push or merge was attempted. Do not infer ahead/behind against `origin/main` as route synchronization.

### Current independent work

- BRANCH Track-B handoff committed as `70735a73`: HBH-11 is a conditional Planning survivor at the pass-2 gamma/bias slot-selection site; it remains `MAIN_SELECTED=NONE`, with a required novelty decision because it shares the failed HBH-10 cursor motif.
- ADDR Track-B pack committed as `fdaf5b38`: three new candidates (paired pass-1 local-slot offset, FP16 retained-y view handle, reduction-scratch row base); no selection. H11 requires REDUCE duplicate/ownership review; all require codegen proof before a performance revision.
- UB Track-B pack committed as `b34f94b2`: no 3–5 new, proven-safe hypotheses qualify; only an existing dead-allocation H3 remains for Planning review, not counted as new.
- REDUCE Track-B pack committed as `52bd18a8`: three screened hypotheses, two provisional leads and cross-route review flags; no selection or implementation.
- TILECOUNT fact pack committed as `38ff22e5`: H1/H2/H3 codegen gates failed as unverified; exhaustion is scoped to the current exact-parent N=2 expansion pool, not all tiling/codegen directions.
- All requested Track-B/fact handoffs are now locally committed; no performance hypothesis has `MAIN_SELECTED=YES`, so no implementation/build/correctness/timing event is authorized next.

### ADDR V001 package audit

`ADDR-V001-ONLINE-PACKAGE-AUDIT-20261004.md` is committed as `048f944a`. Candidate SHA and sidecar match (`26aa65a2…d6192602`), Parent SHA matches R31B V011 (`a8c19a19…879b15e3`), and build evidence is PASS. Required BF16-D40960 correctness remains `INCOMPLETE / SHARED_RUNTIME_BLOCKER` (Parent and Candidate both `507035`); local score is `-24.193320%`, `POOR`, not accepted. Therefore `ONLINE_READY_PACKAGE=NO`, `READY_FOR_EXTERNAL_JUDGE_OWNER=NO`; no direct Online submission or new qualification campaign was run. Existing stale `source-meta.json` / `local-result.json` snapshots were preserved and their mismatch with later evidence documented.

```text
OFFICIAL_CHAMPION=R31B V011 / 45.16
DIRECT_ONLINE_SUBMISSION=0
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

## 11. Baseline Calibration + Next Planning Gate — 2026-10-04

### Champion baseline and normalized score audit

- V011 local identity is verified against source and sidecar SHA `a8c19a19…879b15e3`; official correctness remains 15/15, Official 45.16.
- No canonical local shape suite/composite score was found. The baseline TSV therefore records route-specific shapes and exact runners, using the V011 Parent side from each existing three-run interleaved P/C score (2026-10-03); these matched historical runs are more comparable than a new isolated window. `FRESH_NPU_TIMING_RUNS_THIS_TURN=0`.
- The normalized table recomputes latency and reciprocal invocation-throughput deltas from each route's own same-shape Parent/Candidate averages. `CROSS_ROUTE_SCORE_NOT_DIRECTLY_COMPARABLE=YES`; no Local % is translated into Official score.
- All five scored contexts are `POOR`. ADDR V001's average throughput ratio is +31.91%, but only 1/3 runs is faster and the third Parent run is a severe load tail; required D40960 correctness is still shared-blocked at 507035. `ADDR_V001_ONLINE_ELIGIBLE=NO`; all other current Candidates are also not Online eligible.
- Historical resource audit: HBM/AICore/process snapshots are retained. Host load and AIVector were not recorded. The shared device-use ledger has a released formal lease for REDUCE V001 only; no matching lease was found for ADDR V001/V002, UB V001, or BRANCH V002. These evidence gaps remain visible and contribute to POOR quality; no fresh NPU job or new lease was created.

Artifacts (local commits):

- Baseline + method: `615b5cb0` — `R31B-V011-LOCAL-BASELINE.tsv`, `MAIN2-BASELINE-CALIBRATION-20261004.md`.
- Normalized scores: `340d9b36` — `MAIN2-BASELINE-NORMALIZED-SCORES.tsv`.
- New Track-B pack: `7a358714` — `MAIN2-NEXT-TRACK-B-PLANNING-PACK.md`; three unselected hypotheses, all gated on codegen/novelty review.

```text
TOTAL_IMPLEMENTED_REVISIONS=6
TILECOUNT_IMPLEMENTED_REVISION=NONE
MAIN_SELECTED=NONE
ONLINE_READY_CANDIDATE=NONE
DIRECT_ONLINE_SUBMISSION=0
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

### Latest Git state before this dashboard commit

| Lane | Local HEAD | Upstream | Ahead/behind upstream | Intended remote | Remote HEAD | Push |
|---|---|---|---:|---|---|---|
| ADDR | `fdaf5b38` | `origin/main` | +45 / -47 | `w2/m2/hotloop-addr` | absent | blocked: no GitHub username |
| BRANCH | `70735a73` | `origin/main` | +73 / -47 | `w2/m2/hotloop-branch` | absent | blocked: no GitHub username |
| UB | `b34f94b2` | `origin/main` | +28 / -47 | `w2/m2/ub-lifetime-safe` | `a9c9affc` | divergent: local +28 / remote +2; no force/merge |
| REDUCE | `52bd18a8` | `origin/main` | +14 / -47 | `w2/m2/reduce-finalize` | absent | blocked: no GitHub username |
| TILECOUNT | `38ff22e5` | `origin/main` | +6 / -47 | `w2/m2/tilecount-unroll` | absent | blocked: no GitHub username |
| MAIN-2 control | `245b468b` | `origin/main` | +85 / -47 | `w2/main2/control` | absent (last successful query) | blocked: no GitHub username |

`git fetch origin` timed out (RC 124). Explicit non-interactive `git push -u` was attempted once for all six branches; each failed with `could not read Username for 'https://github.com'` (RC 128). The successful remote query returned only UB's remote ref. No branch is reported synced. `origin/main` is the configured upstream on every local branch, so these counts are not route-target ahead/behind counts.

## 12 Fresh V011 Baseline Closure — 2026-10-04

The preceding calibration text above is historical and explicitly reused
Parent samples from old Candidate paired runs. It is superseded for freshness
by the independently executed evidence under
`研究/主代理/MAIN-2/fresh-v011-baseline-20261004/`.

```text
FRESH_NPU_RUN_COUNT=12
FRESH_TARGETS=ADDR,UB,BRANCH,REDUCE (3 Parent-only processes each)
DEVICE=7
RUN_PROTOCOL=45 warmups; 2x31 device-event samples; pre/post npu-smi
OFFICIAL_CHAMPION=R31B V011 / 45.16
ONLINE_ELIGIBLE=NONE
DIRECT_ONLINE_SUBMISSION=0
MAIN_SELECTED=NONE
```

Fresh route aggregates are ADDR BF16 [9,32768] `32.133 us`, UB FP16
[1,32768] `32.515 us`, BRANCH FP16 [80,6144] `20.596 us`, and REDUCE BF16
[1,32768] `16.287 us` (REDUCE uses the existing per-run median metric).
All four Parent correctness probes passed, but the same-binary floor was
POOR: ADDR and BRANCH qualified only one of three runs; UB and REDUCE had no
qualified run. These are calibration anchors only, not new Candidate verdicts.

`R31B-V011-FRESH-LOCAL-BASELINE.tsv`,
`MAIN2-FRESH-NORMALIZED-SCORES.tsv`,
`MAIN2-FRESH-BASELINE-CALIBRATION-20261004.md`, and the ADDR Online gate are
committed independently. ADDR V001 remains `ONLINE_ELIGIBLE=NO`: historical
Candidate direction was faster only 1/3, fresh calibration is POOR, and the
shared D40960 `507035` correctness blocker remains.

Exact-route upstreams were corrected locally. The five absent target refs are
still `PUSH_PENDING_AUTH` after bounded normal pushes; UB is
`DIVERGED_REMOTE` with two remote-only evidence commits and is documented in
`UB_REMOTE_RECONCILIATION.md`. No force push, merge, rebase, reset, clean, or
history rewrite was used.

## 13. ADDR V001/H3 Official closure + final Track-B audit — 2026-10-04

External Judge result supplied by the user and recorded route-locally at
`线上结果/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/`:

```text
SUBMISSION_ID=6ac20c90694b590c3cb66f2b
SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
IDENTITY=LOCAL_SHA_EQ_REMOTE_SHA; formalResultEligible=true (as supplied); local sidecar/source hash independently verified
RESULT=PASS_15_OF_15; MAX_OUTPUT_ERROR=0.000%
CALCULATED_SCORE=42.718; OFFICIAL_SCORE=42.72
OFFICIAL_ANCHOR=R31B V011 / 45.16; OFFICIAL_DELTA=-2.44 points
CLASSIFICATION=ONLINE_REJECTED; LOCAL_ONLINE_FALSE_POSITIVE
LOCAL_ENGINEERING_DELTA=-24.193320%; QUALITY=POOR; faster direction=1/3
```

The Official score is not the Local latency score; the latter remains an
engineering-only `POOR` historical signal. The local D40960 shared `507035`
diagnostic is retained, not rewritten. Calibration is added to
`调度/本地线上校准.tsv` and `本地评分器校准.md`; the permanent Online gate
now requires clean correctness, valid same-shape Champion baseline, at least
2/3 faster valid runs, average latency and throughput improving together, and
no single-run domination. This gate does not change the scoring formula.

The final three-hypothesis exact-source audit is
`MAIN2-NEXT-TRACK-B-PLANNING-PACK-V3.md`: H1 ranks first conditionally on
novelty/codegen review, H3 second, H2 third. `MAIN_SELECTED=NONE`, no Revision
or Kernel change, and no Planning decision changed. Total implemented Main-2
revisions remains 6; TILECOUNT remains without an implemented Revision.

## 14. Push status after authentication — 2026-10-04

```text
AUTH=sxyq / repo permission available / gh auth setup-git completed
PUSH_STATUS=BLOCKED_BY_GITHUB_SMART_HTTPS_NETWORK (not AUTH_BLOCKED)
API_REF_CHECK=responsive; five exact target branches remain absent
CONTROL_NORMAL_PUSH=bounded 45s attempt; RC=124 network timeout
UB_LOCAL=b34f94b2c214; UB_REMOTE=a9c9affcb49e; local +28 / remote +2
UB_SYNC_STATUS=UB_SYNC_BLOCKED_DIVERGED
FORCE_PUSH/RESET/REBASE/CLEAN=NOT_USED
```

Main-2 local work continues; retry only in the next bounded 30–60-minute
connectivity window. For UB, fetch and inspect the exact branch after the
network recovers, then leave merge/cherry-pick/new-branch choice to Planning.

## 15. Local-to-Online Calibration V2 — 2026-10-04

This section is the current calibration status. Earlier sections remain historical
campaign snapshots and are not overwritten.

```
MAIN2_HEAD=d924518c505e900ebbb9d1fd2ba3af87aaddbd97
REMOTE_HEAD=d924518c505e900ebbb9d1fd2ba3af87aaddbd97
PUSH_PENDING=NO
OFFICIAL_CHAMPION=R31B V011
OFFICIAL_SCORE=45.16
LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE
ONLINE_CASE_REPRODUCIBILITY=PARTIAL
DIRECT_ONLINE_SUBMISSION=0
NEW_PERFORMANCE_REVISION=0
```

Calibration set:

- 13 manifest versions: 12 historical Official-tested candidates plus fresh R31B V011 anchor.
- 12 historical candidates were locally runnable and benchmarked through the same 16-case suite.
- Fresh V011 anchor has 15 valid cases; C15 is excluded because Parent and Candidate share the FP32-wide correctness failure.
- V2 retained only C13 (`128x16384 FP16`, wide multi-row pipeline path); this is insufficient as a final multi-case judge.
- Completed evidence roots used by the scorer are `calibration-v2/runs/20261004-v2-v011` and `calibration-v2/runs/20261004-v2-candidates-blocks`. The interrupted `20261004-v2-candidates` root is not used.

Validation:

- MAE `7.106473`; RMSE `9.283397`.
- Predicted-score Spearman `0.321678`; Kendall `0.121212`.
- Raw local-ratio Spearman `-0.482517`; Kendall `-0.272727`.
- Champion threshold decision accuracy `11/12`; false positives `1`; false negatives `0`.
- The false positive is `VECTOR-MATH-X/V001` (predicted `49.995841`, Official `44.22`).
- ADDR H3 predicted `44.917735`, Official `42.72`; it is below the 45.16 anchor, but the margin is not a validated safety margin.

Current gate:

```
LOCAL_JUDGE_READY=NO
ONLINE_ELIGIBLE=NO
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

Readiness blockers are: one retained case, non-nested exploratory case
selection, one false positive, and high median jitter in most candidate cases.
Performance revisions and Online submissions remain frozen pending Planning /
Review calibration follow-up.

## 16. Local-to-Online Calibration V3 — 2026-10-05

This section supersedes the V2 readiness/validation figures in section 15;
section 15 is retained as the historical V2 snapshot.

V2_FEATURE_SELECTION_LEAKAGE=YES
V2_C13_SELECTION=USED_ALL_12_OFFICIAL_CANDIDATE_LABELS
V2_METRICS_STATUS=OPTIMISTIC_NON_NESTED
CALIBRATION_CANDIDATES=12 / 11 ROUTES
FRESH_R31B_V011_ANCHOR=AVAILABLE; OFFICIAL=45.16; LOCAL_VECTOR=15_OF_16
OUTER_VALIDATION=VERSION_LOO + LEAVE_ONE_ROUTE_OUT
KERNEL_CHANGE=0
NPU_RERUN=0
DIRECT_ONLINE_SUBMISSION=0
LOCAL_JUDGE_READY=NO
PERFORMANCE_REVISION_FREEZE=ACTIVE
PLANNING_DECISIONS_CHANGED=0

Strict nested results (score-error metrics use raw predicted scores; champion
decisions use the conservative score):

- Version-level nested LOOCV: MAE 6.826865, RMSE 10.376418, Spearman 0.545455, Kendall 0.333333.
- Leave-one-Route-out: MAE 6.792421, RMSE 10.372084, Spearman 0.545455, Kendall 0.333333.
- Raw-score and conservative-gate false positives: 0 in both validations; every prediction is below Champion, so this is reject-all behavior.
- All 12 candidate Official labels are below 45.16, so false-negative sensitivity and winner detection remain untestable. The displayed 100% conservative decision accuracy is vacuous specificity, not evidence of useful ranking.
- The nested empirical safety margin spans 16.55–24.606842 Official-score points in held-out folds. It rejects every candidate conservatively and is not a useful screening margin.

Hard held-out examples:

- ADDR H3: predicted 44.510, Official 42.72; conservative score 20.040; correctly rejected.
- VECTOR-MATH-X V001: predicted 43.579, Official 44.22; conservative score below Champion; correctly rejected.
- Four other historical false positives have no common 16-case vector and remain NOT_COMPUTABLE; old single-shape local results were not substituted.

MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv proposes 7 cases: 4 core-provisional cases
(C13, C14, C16, C12) and 3 diagnostic-only diversity sentinels
(C01, C11, C08). Diagnostic-only cases failed the stability screen and must
not be model features. C15 remains missing/invalid. See C13-FORENSIC.md,
LOCAL-CASE-STABILITY.tsv, LOCAL-CASE-NESTED-PREDICTIVENESS.tsv, and
LOCAL-JUDGE-MODEL-COMPARISON.tsv.

Five calibration-information candidates are listed in
ONLINE-CALIBRATION-CANDIDATES.tsv; each lacks a common-suite feature vector,
so prediction range/model disagreement are NA and none is Online-ready.

MORE_OFFICIAL_CALIBRATION_LABELS_NEEDED=YES
READY_FOR_PERFORMANCE_WAVE=NO
ONLINE_RECOMMENDATION=NONE
PUSH_PENDING=YES (normal push timed out after 25s)
REMOTE_SHA=UNVERIFIED (bounded ls-remote timed out)
FORCE_PUSH/RESET/REBASE/CLEAN=NOT_USED
