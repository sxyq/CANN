# ROUTE-DECLARATION — ASYNC-OVERLAP-CHAMPION-X

## 声明字段

```text
ROUTE                ASYNC-OVERLAP-CHAMPION-X
LANE                 M2-4
REVISION             （未创建；本文件是 V001 前的路线声明）
DIRECT_PARENT        R31B-V011
PARENT_SHA           a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH   线上结果/R31B/V011/submission.asc
                     （champions.tsv: EXACT_GIT_BLOB / TRANSFORMED_VERIFIED；
                       技术路线/冠军/R31B-V011/ 下 4 个 symlink 指向 ../../online/ 为断链，
                       exact bytes 以 线上结果/R31B/V011/submission.asc 为准，本地 SHA 已复核）
PARENT_SCORE         Official 45.16（15/15，submission_id 6ab2c10c0304f72a56a0c5cb）
CONTEXT_CLASS        CHAMPION_PIPELINE_SCHEDULE （以 FROZEN Champion seed 做调度类研究）
WORKTREE             /Users/sunyiyang/Desktop/Project/cann-m2-async
BRANCH               m2/async-overlap-champion
IMPLEMENTATION_APPROVED   YES（Planning 本轮正式批准；仍须 Main-2 批准具体 V001 假设后才写 Kernel）
SINGLE_HYPOTHESIS    见 TRACK-B-HYPOTHESES.md 推荐项（H1 inter-pass prologue param prefetch）
```

## 路线目标

在 FROZEN Champion R31B-V011 上研究 MTE2 load / Vector / MTE3 store 的真正 overlap
改善，范围限定为 **pipeline scheduling / sync placement / issue order / loop structure**。
每个 Revision 只允许一个 pipeline mechanism（OFAT）。

Official score 结构（registry「Official score structure」段，R31B-V011=45.16）：
`s_i = 100 / (1 + log_1.5(time_i / best_time_i))`，均值 45.16。要过 50 需把
time/best 平均比压到 1.5 以下。缺口集中在 idx 14 / 7 / 1 / 6 / 4 / 8 / 3；
其中 idx 14（16486.8 vs 3750.1，ratio 4.40）绝对缺口最大，是 wide/large-D
路径的主要压力点。本路线优先覆盖 wide 路径的 pipeline 调度。

## WHY_NOT_DUPLICATE（逐条对照）

### 1. vs R013 DOUBLE-BUFFER

| 项 | 内容 |
|---|---|
| 历史机制 | MTE2 prefetch N+1 + Vector compute N 两段重叠 |
| 历史结果 | Official 18.76 PASS（弱父版时代） |
| 为何不重复 | R013 只有两段，没有 MTE3 stage，也没有 pass 边界 issue 重排。本路线不加 buffer、不加深 queue，只做 scheduling / sync / issue order / loop structure。 |
| 明确排除 | 不得把「再加一层 buffer / 再做一遍两段 double-buffer」当作本路线 V001。 |

### 2. vs ASYNC-TRIPLE-X V001

| 项 | 内容 |
|---|---|
| 历史机制 | 在 R013 种子上只加 MTE3 store stage，形成 MTE2(N+1)/V(N)/MTE3(N-1) |
| 历史结果 | 路径确认 PASS；correctness smoke 507035 未解；tileCount=1 结构惰性；窗口 2/2 不合格；`HISTORICAL_ONLY` / PARK |
| 为何不重复 | 「只加 MTE3 stage」这一步已做过且被明确排除。本路线 seed 是 R31B-V011（已含 depth-2 rings 与多处 MTE2/V/MTE3 重叠），不是 R013 种子；研究对象是 Champion 热路径上的 sync placement / issue order / loop structure，不是再加一级 stage。 |
| 历史可复用但不重复的部分 | ASYNC-TRIPLE-X next-hypotheses 中的 H1（inter-pass prologue prefetch）、H2（per-slot store flags）、H3（MTE2/MTE3 issue order）是**机制分类**，从未在 R31B-V011 上实现或测过。本路线把这些分类落到 Champion 的真实函数边界（见 Track-B 中的源码行号），属于新的 DIRECT_PARENT 实验。 |
| 明确排除 | 不得把「只再加 MTE3 stage」或「把 ASYNC-TRIPLE-X V001 原样移植」当作本路线 V001。 |

### 3. vs MIX-A V003 / V007 同步增删

| 项 | 内容 |
|---|---|
| 历史机制 | V003：加 V_MTE2 release sync + localRows==1 dispatch（Official 44.69）；V007：移除 Load 前第一个 SyncVToMTE2（same-binary 未过） |
| 为何不重复 | MIX-A 的同步增删发生在 FastKernel 多行/narrow 路径，目的是正确性与 dispatch 约束，不是 pipeline overlap 的 issue 重排。本路线若触碰 sync，只允许「为 overlap 而做的 sync placement」，且必须能指出与 MIX-A V003（release 方向）/ V007（Load 前删除）不是同一处、同一目的。 |
| 明确排除 | 不得把「再加一个 V_MTE2 release」或「再删一个 Load 前 SyncVToMTE2」直接当作 V001；若某假设只做这种增删，视为与 MIX-A 同轴，需降级或改写。 |

### 4. vs 当前 R31B pipeline（V006 / V009 / V011）

| 项 | 内容 |
|---|---|
| V006 | MTE3 queue depth 2 → Official 43.91，T14 16670 仍平（proven-flat） |
| V009 | MTE2 queue depth 2 → Official 43.81，T14 16424 仍平，记录写明「MTE2 queue not T14 driver」 |
| V011 | low-precision wide-row pipeline → Official 45.16（当前 seed）。内部已有 depth-2 rings；LP 路径 pass-1 已是 2-deep MTE2 across units；FullCache pass-2 已是 per-slot MTE3_V 延迟等待 |
| 为何不重复 | 队列加深两侧都已在 T14 证平；「再加深 queue depth」是 proven-flat 机制的重复，禁止作为 V001。本路线只动 issue 先后、sync 等待点、循环结构，不改 ring 深度、不新增 UB buffer。 |
| 明确排除 | 不得把队列加深或纯 MTE3 stage 包装成新假设。 |

### 5. vs 禁碰轴

- 不开第 6 条路线。
- 不碰 DTYPE / SCHED-CHAMPION / UB-LIVENESS 实现。
- 不改共享调度总账、不改其他 lane worktree/branch。

## Champion seed 上与 overlap 相关的已读结构（只读事实）

来源：`线上结果/R31B/V011/submission.asc`（3554 行）。与本路线直接相关的两段：

1. `ProcessWideLowPrecision`（L3076–3365，V011 主推的 large-R row pipeline，FP16/BF16 wide）
   - pass-1：2-deep MTE2（rd0/rd1 + rel0/rel1）跨 (row, tile) unit 流水。
   - inter-pass：L3224–3242 逐 batchRow 的 invRms 标量回路（ReduceSum 折叠 + 两次 GetValue 往返），L3243 `SyncVToMTE2()`。
   - pass-2：L3260–3297 参数 gamma/bias 的 2-deep MTE2；但每个 batchRow 内 L3339–3344 固定
     `SyncVToMTE2(); SyncVToMTE3(); Store(...); SyncMTE3ToV();`，**store 完成被拉回标量 issue 路径**。
2. `ProcessWideFp32FullCacheRows`（L2082–2246，FP32 wide）
   - pass-1：L2116–2136 逐 tile `Load → SyncMTE2ToV → V → SyncVToMTE2`，无预取重叠（单 tile staging）。
   - inter-pass：L2139–2157 同样是 invRms 标量回路 + `SyncVToMTE2()`，随后才 Load 参数。
   - pass-2：L2164–2240 已是 per-slot `storeReady*/storeRelease*` + `storeOutstanding*` 的延迟等待，
     store 完成不进标量热路径（与 LP 路径形成对照）。

这两段对照说明：Champion 内部已经同时存在「store 解耦」与「store 耦合」两种写法，
以及「参数 Load 在 invRms 之后才发」的统一 inter-pass 边界。本路线的假设空间由此收敛。

## 阶段状态

- 第一阶段（本文件）：完成必读、建立路线目录、声明 + Track-B 假设 + 推荐 V001 + handoff。
- 不改 Kernel、不创建 V001、不跑 server3、不提交 Online。
- 等待 Main-2 批准推荐假设（或指定另一条）后，才进入 V001 实现。
