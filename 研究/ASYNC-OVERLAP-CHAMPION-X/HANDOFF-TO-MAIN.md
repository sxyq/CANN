# HANDOFF-TO-MAIN — ASYNC-OVERLAP-CHAMPION-X 第一阶段

收件：Main-2  
状态：研究交付完毕，**STOP**。在 Main-2 批准前不写 Kernel、不创建 V001、不跑 server3、不提交 Online。

## 1. 本轮完成

- 按启动清单完成必读（AGENTS.md、cann-mainline Skill、实验总则、执行约定、本地性能测试规范、Git工作流程、服务器实验规范入口、R31B-V011 exact source 与 champions.tsv、main2-r2-route-registry 的 Frozen strong / Official score structure / SCHED 段、ASYNC-TRIPLE-X ROUTE-BRIEF 与 next-hypotheses、全版本记录相关行、idea-pool R013 行、MAIN-2 初始化报告 3.6 / 5.1）。
- 建立 `研究/ASYNC-OVERLAP-CHAMPION-X/`。
- 写 `ROUTE-DECLARATION.md`（含逐条 WHY_NOT_DUPLICATE）。
- 写 `TRACK-B-HYPOTHESES.md`（4 个候选，全部限定 pipeline scheduling / sync placement / issue order / loop structure）。
- 未改 Kernel、未建 V001、未动共享调度、未碰其他 lane。

## 2. 去重确认（一句话版）

| 对照 | 结论 |
|---|---|
| R013 DOUBLE-BUFFER | 只有两段 overlap，Official 18.76；不重复，且禁止再把「加 buffer/两段」当 V001 |
| ASYNC-TRIPLE-X V001 | 「只加 MTE3 stage」已做（路径 PASS，smoke 507035 未解，tileCount=1 惰性）；禁止复用该步 |
| MIX-A V003/V007 | 同步增删先例在 FastKernel 路径；sync 删除类假设需降级（见 H3） |
| R31B V006/V009 | MTE3/MTE2 queue depth 2 在 T14 proven-flat；禁止再加深 queue |
| R31B V011 seed | 已有 depth-2 rings 与多处 triple overlap；研究空间收敛为边界/同步/发射/循环结构 |

## 3. 推荐 V001（待 Main-2 批准）

```text
ROUTE               ASYNC-OVERLAP-CHAMPION-X
REVISION            V001（尚未创建）
DIRECT_PARENT       R31B-V011
PARENT_SHA          a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CONTEXT_CLASS       CHAMPION_PIPELINE_SCHEDULE
SINGLE_HYPOTHESIS   H1 Inter-pass prologue param prefetch：
                    在 ProcessWideLowPrecision 中，把 pass-2 tile-0 的 gamma/bias
                    MTE2 从 invRms 标量回路之后（L3260–3265）提前到回路之前
                    （pass-1 释放 xBuf_/residualBuf_ 之后），让参数 DMA 与 invRms
                    回路重叠；pass-2 入口只 WaitFlag 已在飞的 load。
WHY_NOT_DUPLICATE   见 ROUTE-DECLARATION.md 逐条对照；本假设不加 buffer、不加深
                    queue、不加 MTE3 stage、不是 MIX-A 式 sync 增删，且从未在
                    R31B-V011 上实现过。
EXPECTED_SHAPES     所有 wide full-y 形状（rowWidth > kCacheElems），含 tileCount=1
                    （prologue 类不依赖 tileCount）；优先 large-R wide FP16/BF16。
OFAT_DIFF           移动一处 Load+SetFlag 的发射位置；必要时把 L3243 SyncVToMTE2
                    收窄为该 Load 的事件顺序（仍属同一假设）。
```

## 4. 备选顺序（Main-2 若不选 H1）

1. H2 LP pass-2 store 解耦（先做 y 行区域生命周期依赖审计；不新增 UB）。
2. H4 pass-2 稳态内参数 prefetch 与 store 发射对调。
3. H3 冗余 SyncVToMTE2 下沉（不推荐做 V001，MIX-A V007 同轴观感）。

## 5. 请求 Main-2 的决定

- [ ] 批准 H1 为 V001 SINGLE_HYPOTHESIS，或指定另一条。
- [ ] 确认 V001 只落 `ProcessWideLowPrecision` 一条路径（FullCache 同类改动留给后续 Revision）。
- [ ] 批准后的测量形状倾向（large-R wide LP；是否保留 tileCount=1 作为 prologue 对照）。

## 6. 风险与限制

- `技术路线/冠军/R31B-V011/` 下 diff.patch / source-meta.json / submission.asc / submission.sha256 为断链 symlink（指向不存在的 `../../online/`）；exact source 已用 `线上结果/R31B/V011/submission.asc` 复核 SHA = a8c19a19…。这是快照布局问题，不影响 seed 身份。
- ASYNC-TRIPLE-X 历史测量窗口与 correctness smoke 问题不构成本路线结论；本路线任何本地结论必须按 `本地性能测试规范.md` 重新做。
- H1 收益预期是「每 batch 消掉一段串行边界」，量级可能落在噪声范围内；below floor 记 unmeasurable，不记 refuted。
