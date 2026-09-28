# HANDOFF-TO-MAIN — MULTIROW-DMA-CHAMPION-X 第一阶段

ROUTE: MULTIROW-DMA-CHAMPION-X（LANE M2-5）
WORKTREE: `/Users/sunyiyang/Desktop/Project/cann-m2-multirow`
BRANCH: `m2/multirow-dma-champion`
DIRECT_PARENT: R31B-V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` / Official 45.16
PHASE: 第一阶段（TRACK_B_ONLY）— 完成，STOP，等 Main-2 批准

## 交付物

| 文件 | 内容 |
|---|---|
| `研究/MULTIROW-DMA-CHAMPION-X/ROUTE-DECLARATION.md` | 路线身份、种子 SHA 核对、范围、WHY_NOT_DUPLICATE（R015 / BATCH-RESIDENT-X / MODE-X-R015C / R31B V003 逐条） |
| `研究/MULTIROW-DMA-CHAMPION-X/DMA-INVENTORY-R31B-V011.md` | 固定冠军源码 DMA/DataCopy 调用点只读盘点 |
| `研究/MULTIROW-DMA-CHAMPION-X/TRACK-B-HYPOTHESES.md` | H1–H4 候选 + 推荐 V001 SINGLE_HYPOTHESIS |
| `研究/MULTIROW-DMA-CHAMPION-X/HANDOFF-TO-MAIN.md` | 本文件 |

## 去重确认（一句话版）

四种历史机制都不是本路线 V001：R015 = 整行 ownership + pad 多 burst（Official RE）；BATCH-RESIDENT-X = contiguous batch ownership（弱父，window 不合格）；MODE-X-R015C = row_copy 微内核（语义 FAIL）；R31B V003 = full-y 多行驻留（已在冠军内，原样保留）。独立空间 = 同 tile 列窗跨多行的 stride 多行事务（nBursts=B，pitch≠blockLen），冠军 0 处使用。

## DMA 盘点一句话版

全文件仅 `Load`/`Store` 两个单 burst `DataCopyPad` 发事务点；已有 flat 连续多行（CONTIG/BATCH）；wide full-y 与 generic 多 tile 的输入仍是逐 (行,tile) 单 burst——stride 合并空间在这两处。

## 推荐 V001（待批）

H1 STRIDE-MULTIROW-WIDE-IN：wide 低精度 full-y 路径输入侧，同 tile 列窗跨 B 行一条 stride 多行 `DataCopy`（满 tile 才合并，尾 tile 回退）；dispatch、所有权、y 驻留、Store、gamma/bias、tile 宽度全部不动。主靶 FP16/BF16 wide（B≥2 家族）。

备选：H3 ALIGNED-DATACOPY-FORM（Pad→DataCopy 对齐替换，diff 更小、信息增量弱）。
后续桶：H2 generic 多 tile 同机制；H4 FP32 wide 变体（先推演 B≥2 存在性）。

## 请 Main-2 决定

1. 是否批准 H1 为 V001 SINGLE_HYPOTHESIS（或改批 H3）。
2. 批准后的 V001 流程：Revision 声明 → 本 worktree 改 Kernel → server3 exact-source 编译 → NPU correctness → same-binary → 交错 P/C → handoff 回 Main。
3. 风险前置确认：实现前的 UB 数字推演若显示必须改 `ChooseWideFullYRows` 语义，我停下报事实，不自行扩成 full-y 改动。

## 本轮边界遵守

- 未改 Kernel、未创建 V001、未跑 server3、未提交 Online。
- 未改共享总账（调度/、技术路线/*.tsv）；只写本路线 `研究/MULTIROW-DMA-CHAMPION-X/`。
- 未开第 6 条路线；未碰 DTYPE / SCHED-CHAMPION / UB-LIVENESS 实现。
- 未发现 UB correctness gap 新线索（现有已知项见 main2-r2-route-registry：宽 FP32 本地 golden 与 Official 容差不一致属 pre-existing，未动）。
