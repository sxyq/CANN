# V001 REVISION-DECLARATION — MULTIROW-DMA-CHAMPION-X

（修改前记录。Main-2 批准 2026-09-28：H1 STRIDE-MULTIROW-WIDE-IN。）

## 声明字段

| 字段 | 值 |
|---|---|
| ROUTE | MULTIROW-DMA-CHAMPION-X |
| REVISION | V001 |
| REVISION_KIND | PERFORMANCE_SINGLE_HYPOTHESIS |
| DIRECT_PARENT | R31B-V011（FROZEN exact Champion source） |
| PARENT_SOURCE_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_SCORE | 45.16（Official 15/15，submission_id `6ab2c10c0304f72a56a0c5cb`） |
| OFFICIAL_ANCHOR | 45.16 |
| CONTEXT_CLASS | FROZEN_STRONG_BASELINE_TRANSPLANT |
| SINGLE_HYPOTHESIS | H1 STRIDE-MULTIROW-WIDE-IN：wide 低精度 full-y 路径 pass-1 输入侧，同 tile 列窗跨多行用 stride 多行 `DataCopy`（nBursts=行数, blockLen=tileBytes, srcStride=(rowWidth-tile)*elem）替代逐行单 burst；满 32B 对齐 tile 才合并，尾 tile 与不对齐回退单 burst |
| EXPECTED_SHAPES | FP16/BF16 wide D>8192 且 localRows≥2、wideFullYRows_≥2（主靶 8×16384、16×16384、8×32768、12×12288 这组探针及其 rows≥2×cores 变体）；B=1 形状为阴性对照 |
| OFAT_AUDIT | 只改输入侧发事务形态（ProcessWideLowPrecision pass-1 的取数方式）。不改 ownership、dispatch 门槛、tile 宽度、y 驻留、gamma/bias、Store、数学序 |

## WHY_NOT_DUPLICATE

- 非 contiguous multi-row batch ownership（BATCH-RESIDENT 形态）：不动行所有权，不引入 batchRows ownership 选择器；wide 行距 rowWidth>tileWidth 恒成立，事务是真 stride 窗口（pitch≠blockLen），不是连续整行 flat 打包。
- 非 full-y multi-row residency（R31B V003 形态）：`ChooseWideFullYRows`、gammaBuf_/valueFp32Buf_ 的 y 驻留布局、wideFullYRows_ 语义全部不动。
- 非 row_copy microkernel（MODE-X-R015C 形态）：无独立搬运内核，AddRmsNormBias 逐 (行,tile) 数学序原样保留。
- 非冠军 CONTIG flat 多行：那是整行长 burst（count=B*D）；本改动是 nBursts>1 + srcStride 的列窗事务。全文件此前 0 处 nBursts>1/stride。
- 非 mode-selection：`Process()` 全部分发条件与阈值不动，只改已进入 `ProcessWideLowPrecision` 后的取数形态。
- 非 R015 失败形态：不用 pad 多 burst 整行；只用非 Pad `DataCopy` 于 32B 对齐满 tile，尾 tile/不对齐回退现有 `Load` 单 burst。

## UB 数字推演（风险前置，实现前完成）

### 推演问题

stride 多行事务需要连续接收暂存（dstStride=0）：nBursts=chunk 行 × tile。若沿用 V011 的 2-deep ping-pong（暂存 2×tile 用于两个在飞单元），多行化后需要 2×chunk tile 的 x 与 res 暂存。

### 数字（910B3 可用 UB 184KB=188416B；ChooseWideFullYRows 预算常量 176KB=180224B）

`ChooseWideFullYRows` 校验式（FP16）：`rowWidth*2*rows + 20*tileElems + 64*rows ≤ 180224`。实际分配比校验式多恰好一个 `outputBuf_`（2*tileElems 字节），靠 176KB vs 184KB 的 8KB 余量覆盖——两者当前是自洽的。

| 形状 | wideFullYRows_ | 现状实际占用 | 2-deep×B 需要 | 结论 |
|---|---|---|---|---|
| FP16 D=16384, B=2 | 2 | ≈155776B | +32768B → ≈188544B | **超出可用 UB 128B，放不下** |
| FP16 D=12288, B=3 | 3 | ≈164032B | +32768B（x/res 各+2 tile）→ ≈196800B | **放不下** |
| BF16 D=12288, B=2 | 2 | ≈164032B | +32768B → ≈196800B | **放不下** |
| FP16 D=32768 | 1 | — | — | B=1，机制不启动（阴性对照） |

### 结论与处置（走假设已写明的「缩 B」分支，不改 ChooseWideFullYRows）

- **不改** `ChooseWideFullYRows` 语义、不改任何 `InitBuffer` 尺寸。
- stride 事务的 nBursts **缩到现有暂存容量**：`chunk = min(batchRows, kStrideChunkRows=2)`（xBuf_/residualBuf_ 现为 2 tile，正好一次接收 2 行 × 1 tile）。batchRows≤2 时一条事务覆盖整个 batch（即 nBursts=B，与批准文本一致）；batchRows=3 时 2+1 两条。
- 后果（写入 SYNC_IMPACT/风险）：多行路径每个 (tile, chunk) 串行 MTE2→V，V011 的 (行,tile) 流 2-deep 预取在该路径内不再跨单元重叠（暂存被 B 行窗口占满）。batchRows==1 或回退路径保持 V011 原样。
- 若 NPU 实测显示该形态退步，归因于「事务削减 < 流水损失」，是机制结论，不是构建问题。

## UB / CORE / DMA / SYNC / PRECISION IMPACT

| 项 | 影响 |
|---|---|
| UB | 0 字节变化（复用现有 xBuf_/residualBuf_ 2-tile 暂存；chunk≤2 行） |
| CORE | 0（blockDim、blockIdx/baseRows/extraRows 所有权不动） |
| DMA | 满 tile 输入事务数：batchRows×tileCount×2 条 → ceil(batchRows/2)×tileCount×2 条（B=2 时减半）；尾 tile/不对齐回退不变；Store 与 gamma/bias 不动 |
| SYNC | 事件粒度变粗；多行路径每 chunk 用 SyncMTE2ToV/SyncVToMTE2；无新增 PipeBarrier；batchRows==1 路径保持 V011 2-deep 事件结构 |
| PRECISION_RISK | 低。逐 (行,tile) 的 Add→Muls→ToFloat→Mul→ReduceSum（FP16）/ ToFloat→Add→Mul→ReduceSum（BF16）指令序与写槽（y 行内 tile、reduceLocal[batchRow*stride+tile]）不变；取数批大小变化。仍需全 golden 矩阵 |

## MINIMAL_OFAT_DIFF

1. 新增助 `LoadStridedRows`（非 Pad `DataCopy`，nBursts/blockLen/srcStride，仅 32B 对齐调用方使用）。
2. `ProcessWideLowPrecision` pass-1：`batchRows>=2` 且存在满对齐 tile 时走 tile-outer/chunk 循环（stride 合并 + 尾部单 burst 回退）；否则原有 V011 unit 循环字节级保留。
3. 不动：`Process()` 分发、`ChooseWideFullYRows`、Init/InitBuffer、pass-2、Store、`ProcessWideFp32*`、generic/mid/CONTIG/BATCH、host `run_kernel`。

## 风险注记（NPU 实测前保留）

R015 用 `DataCopyPad` nBursts 多 burst 整行曾 Official RUNTIME_ERROR。本实现避开其触发面（非 Pad、仅 32B 对齐满 tile、srcStride 显式、尾部回退单 burst），但 stride 多行事务的驱动行为在本平台未实测过——correctness 失败或 runtime error 时按 evidence 上报，不静默改机制。
