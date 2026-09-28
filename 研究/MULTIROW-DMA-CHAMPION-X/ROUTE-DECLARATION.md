# ROUTE-DECLARATION — MULTIROW-DMA-CHAMPION-X

## 身份

| 字段 | 值 |
|---|---|
| ROUTE | MULTIROW-DMA-CHAMPION-X |
| LANE | M2-5 |
| WORKTREE | `/Users/sunyiyang/Desktop/Project/cann-m2-multirow` |
| BRANCH | `m2/multirow-dma-champion` |
| DIRECT_PARENT | R31B-V011（exact Champion source，固定种子） |
| PARENT_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_SCORE | Official 45.16（15/15，submission_id `6ab2c10c0304f72a56a0c5cb`） |
| CONTEXT_CLASS | FROZEN_STRONG_BASELINE_TRANSPLANT（在固定冠军源码上只改 DMA 事务形态） |
| SEED_SOURCE | `线上结果/R31B/V011/submission.asc`（本 worktree 实测 sha256 与 submission.sha256 一致） |
| IMPLEMENTATION_APPROVED | YES（Planning 正式批准；本轮仍只做第一阶段研究，不写 Kernel） |
| PHASE-1 STATUS | TRACK_B_ONLY；未创建 V001；等待 Main-2 批准后才动手 |

种子身份核对（2026-09-29，本 worktree）：`sha256sum 线上结果/R31B/V011/submission.asc` = `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，与 `线上结果/R31B/V011/submission.sha256`、`技术路线/冠军/R31B-V011/RESULT_REF.txt`、main2-r2-route-registry 的 FROZEN_BASELINE_SHA 三处一致。

## 研究范围

研究：stride DMA / multi-row DMA / row batching / DMA transaction reduction / 连续行搬运——全部限定为 **DMA 事务与搬运形态**（MTE2 侧为主）。

禁止同时做（本路线任何 V001 都不得混入）：

- row scheduling（行调度 / rows-per-block / mode selection）
- tiling（tile 切分策略本身）
- parameter locality（gamma/bias 驻留、面板复用）
- store merge（输出侧合并写）

禁止碰的其他路线实现：DTYPE / SCHED-CHAMPION / UB-LIVENESS。禁止开第 6 条路线。OFAT：一个 Revision 一个概念变化。

## WHY_NOT_DUPLICATE（逐条对照）

### 1. vs R015（FULL-R015-MULTI-ROW-DMA）

| 项 | R015 | 本路线 |
|---|---|---|
| 机制 | block 拥有 8 连续行 + `DataCopyPad` nBursts=rowsThisBlock，整行合并搬运 | 不改行所有权；只改同 tile 列窗在多行间的发数形态 |
| 结果 | Official RUNTIME_ERROR 1/15，无有效分数 | 未实现 |
| 形态差异 | 连续整行 batch ownership + pad 多 burst（整行 blockLen，D 非 32B 对齐时高风险） | stride 窗口形态：`nBursts=B, blockLen=tileBytes, srcStride=(rowWidth-tile)*elem`，要求 blockLen/stride 32B 对齐，尾 tile 回退单 burst |
| 结论 | 不重复。R015 的运行时失败点（整行 pad 多 burst）被本路线的对齐约束与回退规则显式绕开 | |

### 2. vs BATCH-RESIDENT-X V001

| 项 | BATCH-RESIDENT-X | 本路线 |
|---|---|---|
| 机制 | R014 参数驻留后 contiguous multi-row batch DMA ownership（kMaxBatchRows=4，`DataCopy` nBursts=B 整行） | 不引入 batchRows ownership 选择器；不改所有权；只做 tile 列窗的 stride 多行发数 |
| 父与结论 | 父 A001-V017 36.41 弱基线；window 不合格，未形成架构结论 | 父为 45.16 冠军 |
| 形态差异 | 整行、连续、ownership 打包（BATCH-RESIDENT 形态，已排除） | 行窗、strided、pitch≠blockLen |
| 结论 | 不重复。且冠军自身已有 flat 连续多行（见 DMA 盘点），再做 contiguous batch 既重复历史也重复冠军 | |

### 3. vs MODE-X-R015C

| 项 | MODE-X-R015C | 本路线 |
|---|---|---|
| 机制 | row_copy microkernel（拷贝微内核）；r4 map one row per block | 不引入任何 copy-only 微内核；AddRmsNormBias 语义与数学序全部保留 |
| 结果 | r3 仅 row-copy 3/3（非目标算子）；r4 AddRmsNormBias 语义 3/3 FAIL | 未实现 |
| 结论 | 不重复。本路线的改动只发生在现有 `Load` 发事务的方式上，不是独立搬运内核 | |

### 4. vs R31B V003（Champion lineage 已试）

| 项 | R31B V003 | 本路线 |
|---|---|---|
| 机制 | UB-budgeted full-y multi-row residency and parameter reuse | 不改 y 驻留生命周期、不改参数复用 |
| 结果 | Official 43.68（低于 V002 43.78，REJECT）；但 V006 = pure V003 + MTE3 queue depth → 43.91 PROMOTE，full-y 多行驻留成为冠军 wide 路径核心（V011 源码注释即写明 "V003: every wide shape keeps complete y rows in UB"） | full-y 结构字节级保留 |
| 形态差异 | UB 形态（y 完整多行驻留） | DMA 发事务形态（x/residual 的 tile 列窗如何发） |
| 结论 | 不重复。V003 排除项的含义是「不得再把 full-y residency 当作 V001 的变化点」，不是「冠军里没有它」——它已在冠军里，本路线原样不动 | |

### 5. 排除形态重申（不得作为 V001）

- contiguous multi-row batch ownership（BATCH-RESIDENT 形态）
- full-y multi-row residency（R31B V003 形态）
- row_copy microkernel（MODE-X-R015C 形态）

### 6. 独立方向的成立条件（去重结论要求，本轮已核实）

- 目标形态：stride DataCopy 形态的 multi-row / row-batching（同 tile 列窗跨多行，pitch≠blockLen）。
- 未被 V003 覆盖：V003 是 y 的 UB 驻留形态；冠军中 x/residual 的发事务仍是逐 (行, tile) 单 burst（见 `DMA-INVENTORY-R31B-V011.md`），没有 nBursts>1、没有非零 stride。
- 未被 mode-selection 偷走：V001 不改任何路径分发门槛（widePath_ / CONTIG / BATCH / mid / generic 的阈值与条件全部字节级不动），只改已选中路径内部的输入发事务方式。
- 未被冠军 flat 多行覆盖：冠军已有的多行是「连续整行 flat 单 burst」（CONTIG/BATCH 形态），与 stride 窗口形态不同。

## 第一阶段产出

1. `DMA-INVENTORY-R31B-V011.md` — 固定冠军源码 DMA/DataCopy 调用点只读盘点
2. `TRACK-B-HYPOTHESES.md` — 4 个 V001 候选（全部限定 DMA 事务/搬运形态）+ 推荐 SINGLE_HYPOTHESIS
3. `HANDOFF-TO-MAIN.md` — 交 Main-2 审阅

第一阶段硬约束遵守情况：未改 Kernel、未创建 V001、未跑 server3、未提交 Online、未改共享总账、未开新路线。
