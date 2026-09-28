# LANE-FACT-PACK-FOR-PLANNING — MULTIROW-DMA-CHAMPION-X

ROUTE: MULTIROW-DMA-CHAMPION-X（LANE M2-5）
日期: 2026-09-28
状态: **建议 LANE_NEEDS_PLANNING_REVIEW**（不是 PARK；路线生命周期决定权在 Planning）
性质: Track-B 收口事实包，只陈述事实与选项，不选路线命运

## 1. 路线与范围

- 种子：FROZEN R31B-V011（`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，Official 45.16）。
- 轴：stride DMA / multi-row DMA / row batching / DMA 事务削减 / 连续行搬运。
- 禁入：row scheduling、tiling、parameter locality、store merge；不碰 ChooseWideFullYRows 计入、DTYPE / SCHED-CHAMPION / UB-LIVENESS 实现。

## 2. 已执行实验与边界（全部有证据）

| Revision | 机制 | 结论 | 证据 |
|---|---|---|---|
| V001 | H1 stride 多行 tile 列窗（1-deep 缩 B，nBursts≤2） | **LOCAL_REJECTED**：48×16384 FP16 4/4 一致退步 +3.2%…+13.4%（median +8.4%）——命令减半抵不过失去 2-deep 流水 | `本地实验/MULTIROW-DMA-CHAMPION-X/V001/` |
| V002 | C1 对齐非 Pad DataCopy 指令形态（2-deep 不动） | **NEEDS_ONE_MORE_LOCAL 终局**：16 对 median −1.84%，与同代码对照（median −1.48%、散布 −7.1%…+5.4%）不可分——指令形态非主导成本 | `本地实验/MULTIROW-DMA-CHAMPION-X/V002/` |

成本结构结论（两实验合成）：**主导成本是 MTE2 流水重叠与 V 端指令/事件，不是命令计数，也不是单条搬运指令形态。**

## 3. 轴内剩余选项盘点（回答 Main-2：还有没有独立假设）

### 3.1 C2 — stride 合并 + 保 2-deep（唯一结构性剩余）

需要改 UB 计入（ioTiles 4→6 或等价压缩）才放得下（完整形态超 128B~8KB；3-slot 折中会使 BF16 主场 B=1）。三选项 (a)/(b)/(c) 事实包已交：`研究/MULTIROW-DMA-CHAMPION-X/C2-OPTIONS-FACT-PACKAGE.md`。**处置权在 Planning，本 agent 不选。**

### 3.2 C3 — 2-slot 部分合并（窗口首对 stride 合并）

保 2-deep、0 UB 变化、不碰 ChooseWideFullYRows。但收益上限 10–25% 命令削减，而 V001 的 50% 削减已实测净负——翻盘先验弱。Main-2 V002 裁定已标「暂不启用」。**保持休眠；除非 Planning 另行点名，不作为待批假设。**

### 3.3 平台形态探查（本轮只读，无新假设）

- `DataCopyEnhancedParams`（blockMode/deqScale/deqValue/sidStoreMode/isRelu/padMode/padValue）是量化/反量化与布局旋钮，不是 GM 读缓存模式旋钮；对本算子 ND 平铺搬运不适用。
- `ExtractCacheMode` 路径仅编译进 arch 3101/5102；dav-2201（910B3）无用户可控缓存模式接口。
- 结论：**不存在**「缓存模式 / 读取策略」类 DMA 形态假设可写。

### 3.4 流水级剩余空间 = 他路线职权（对本 lane 是重复，不得写为本 lane 假设）

主导成本落在流水重叠/事件结构，但该轴归 **ASYNC-OVERLAP-CHAMPION-X（LANE M2-4）** 所有（pipeline scheduling / sync placement / issue order / loop structure）。其 Track-B 已有四条在案：

1. inter-pass prologue 参数预取（pass-1 尾部提前发 gamma/bias MTE2）
2. LP pass-2 store 解耦（per-slot MTE3_V 延迟等待）
3. 冗余 SyncVToMTE2 下沉
4. 稳态内 MTE2/MTE3 发射对调

本 agent 不重写、不实现上述任何一条；跨 lane 重复按 NON_OVERLAP 处理。

### 3.5 结论

**除 C2（Planning 裁定中）与休眠的 C3 外，本 lane 轴内不存在「独立、不碰 ChooseWideFullYRows、非跨 lane 重复」的待批 DMA/搬运假设。** 不写 V003，不叠加新优化。

## 4. 建议

**LANE_NEEDS_PLANNING_REVIEW**（非 PARK）：

1. 轴内唯一结构性选项（C2）正等 Planning 在 (a)/(b)/(c) 中取舍——路线还有活的分岔，不满足自行终止条件。
2. 路线已产出可迁移事实：成本结构结论、同代码噪声带判定方法（median+散布双指标）、`DataCopyParams` 单位语义（32B 单位）、UB-GAP-CLUE（FP32 wide 非确定性）。
3. PARK / CLOSE / ABANDON 属 Planning 决定权；本 agent 只报事实与建议，请 Planning 在 C2 裁定的同时给出 lane 处置（继续 / 并入 / 停放 / 关闭）。

## 5. 证据索引

| 项 | 位置 |
|---|---|
| 路线声明与去重 | `研究/MULTIROW-DMA-CHAMPION-X/ROUTE-DECLARATION.md` |
| DMA 盘点 | `研究/MULTIROW-DMA-CHAMPION-X/DMA-INVENTORY-R31B-V011.md` |
| Track-B 假设池 | `研究/MULTIROW-DMA-CHAMPION-X/TRACK-B-HYPOTHESES.md` |
| V001 全套 | `本地实验/MULTIROW-DMA-CHAMPION-X/V001/` |
| V002 全套（含 API 探针、两轮 P/C） | `本地实验/MULTIROW-DMA-CHAMPION-X/V002/` |
| C2 三选项事实包 | `研究/MULTIROW-DMA-CHAMPION-X/C2-OPTIONS-FACT-PACKAGE.md` |
| UB correctness gap 线索 | `研究/MULTIROW-DMA-CHAMPION-X/UB-GAP-CLUE.md` |
| 历史 handoff | `研究/MULTIROW-DMA-CHAMPION-X/HANDOFF-TO-MAIN.md` |

## 6. 边界遵守

- 未建 V003、未改 Kernel、未叠加新优化、未提交 Online、未改共享总账。
- C2/C3 未选、未实现；跨 lane 假设未重写。
