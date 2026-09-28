# DMA / DataCopy 调用点只读盘点 — FROZEN R31B-V011

对象：`线上结果/R31B/V011/submission.asc`（3554 行）
SHA：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
方式：只读；本文件是第一阶段研究证据，不含任何源码改动。

## 1. 结论摘要

1. 全文件只有 **两处** `DataCopyExtParams` 构造——通用助手 `Load`（L3367-3375）与 `Store`（L3377-3383），两者都是 `nBursts=1, blockLen=count*sizeof(T), srcStride=0, dstStride=0` 的单 burst `DataCopyPad`。
2. 全文件 **没有任何** `nBursts>1` 的事务，**没有任何** 非零 srcStride/dstStride。多行搬运只以「连续整行 flat 长 burst」存在（count = batchRows*rowWidth）。
3. wide full-y 路径与 generic 多 tile 路径的输入仍是 **逐 (行, tile) 单 burst**；同 tile 列窗跨多行可以合并为 stride 多行事务的空间 **存在且未被占用**。
4. 冠军已覆盖：aligned 小 D 连续多行 flat（CONTIG）、小 FP32 batch、full-tile/full-row batched output pipeline、wide full-y 多行 y 驻留（V003 血统）、V011 低精度 (行,tile) 流 2-deep MTE2。

## 2. 通用发事务点（全文件唯一 DMA 形态）

| 位置 | 助手 | 事务形态 |
|---|---|---|
| L3367-3375 | `Load(dst, src, offset, count)` | `DataCopyPad` + `DataCopyExtParams(1, count*sizeof(T), 0, 0, 0)` |
| L3377-3383 | `Store(dst, offset, src, count)` | `DataCopyPad` + `DataCopyExtParams(1, count*sizeof(T), 0, 0, 0)` |

所有 kernel 内 GM↔UB 搬运都经这两个助手；因此「DMA 事务形态」的改动点收敛、可审计。

## 3. 按路径盘点

### 3.1 已有多行（flat 连续形态，非 stride）

| 路径 | 位置 | 多行形态 | 门槛 |
|---|---|---|---|
| `ProcessSmallFp32ContiguousBatched` | L1576-1664 | `Load(x,res, batchOffset, totalElems=batchRows*rowWidth)` 一条 flat 长 burst 覆盖连续整行；`Store` 同样 flat 多行 | FP32，rowWidth≤2048，rowWidth%8==0，localRows>1；batchRows≤8 |
| `ProcessSmallLowPrecisionContiguousBatched` | L1665+ | 同上（`totalElems=batchRows*width` flat 入、flat 出） | FP16/BF16，rowWidth≤2048，rowWidth%16==0，localRows>1；batchRows≤8 |
| `ProcessSmallFp32Batched` | L1372-1457 | 输入仍逐行单 burst（`Load(xLocal, xGm_, rowOffset, rowWidth)`）；输出 batch flat `Store(batchBegin*rowWidth, batchRows*rowWidth)` | FP32，rowWidth≤4096，%8==0，localRows>1；batchRows≤4 |
| `ProcessFp16/Bf16FullTileBatchedOutputPipelined` | L622 / L754 | full-tile 多行 y 在 UB 中批处理；输出批写 | rowWidth==4096（kTileElems），localRows>1 |
| `ProcessFp16/Bf16FullRowOutputPipelined` | L876 / L1004 | full-row 多行 | rowWidth==8192（kCacheElems），localRows>1 |
| `ProcessFp32FullRowOutputPipelined` | L1122 | full-row 多行 | FP32 rowWidth==8192，localRows>1 |

这些是「连续整行 flat 单 burst」= BATCH-RESIDENT / R015 排除形态的同类；本路线不重做、不扩展。

### 3.2 wide full-y 路径（D>8192）——逐 (行,tile) 单 burst，stride 空间在此

| 路径 | 位置 | 输入发事务 | 说明 |
|---|---|---|---|
| `ProcessWideFp32FullCacheRows` | L2082-2247 | `for batchRow { for tile { Load(x); Load(res); ... } }`，每 (行,tile) 两条单 burst | FP32 全部 wide 形状都走这里（`ProcessWideFp32` L1847 一句分发） |
| `ProcessWideLowPrecision` | L3076+ | pass-1 以 unit=(row,tile) 流水，`units = batchRows*tileCount`；每 unit 2 条单 burst（x、res），V011 加了 2-deep MTE2 ping-pong（xA/xB 双 tile 级） | FP16/BF16 wide；V011 的 LP row pipeline 就在这里 |

- `wideFullYRows_` 由 `ChooseWideFullYRows`（L1293-1330）在 UB 预算 176KB 内取 8..1；FP16 y 存 half，B 才容易 ≥2；FP32/BF16 y 为 FP32，大 D 时 B 常为 1。
- `kWideFullYTileElems=4096`，tile 为 4096 元素；FP32 tile=16KB、FP16 tile=8KB，均为 32B 对齐。
- 输入侧完全没用 stride 事务：同一 tile 列窗在 B 行上是 B 条独立单 burst（x 与 res 各 B 条）。
- 第二遍输出：gamma/bias 按 tile 面板加载（parameter locality，范围外）+ 按 (行,tile) `Store`（store merge，范围外）。

### 3.3 generic 多 tile 路径（4096<D≤8192）——同样逐 (行,tile) 单 burst

`Process()` 的 cacheParams/generic 段（L284-495）：`for row { for col += kTileElems(4096) { Load(x); Load(res); ... } }`。cacheRow=true 时 y 全行驻留 UB、带下一首 tile 预取；每 (行,tile) 仍是单 burst。`rowWidth ∈ (4096, 8192]` 时 tileCount=2，localRows>1 时同 tile 列窗跨行的 stride 合并空间与 wide 同类。

### 3.4 mid 路径（128<D≤4096 未命中 specials）与 tiny（D≤128）

| 路径 | 位置 | 形态 |
|---|---|---|
| `ProcessNarrowMidOverlap` | L499-621 | 逐行 `Load(x)+Load(res)` 单 burst（整行）；参数驻留 localRows>1 时 gamma/bias 只载一次 |
| tiny/generic 单 tile | L250+ | 逐行逐 tile 单 burst |

mid 行在 GM 上连续，整行多行 = flat 连续形态（排除）；pad 多 burst 整行 = R015 失败形态（排除）。mid 不作为 V001 主场。

### 3.5 wide 其余函数（历史路径，未被 `ProcessWideFp32` 分发）

`ProcessWideFp32CachedRows`（L1856）、`ProcessWideFp32PanelResident`（L2248）、`ProcessWideFp32Batched`（L2383）、`ProcessWideFp16/Bf16CachedRows`（L2720/2802）、`ProcessWideFp16BatchedOutputPipelined`（L2895）等保留为历史形态；现行分发只进 FullCacheRows（FP32）与 ProcessWideLowPrecision（低精度）。V001 不碰这些死路径。

## 4. 「单行搬运 / multi-row 空间 / 已被冠军覆盖」对照

| 类别 | 位置 | 判定 |
|---|---|---|
| 单行搬运 | wide full-y 逐 (行,tile) Load/Store；generic 逐 (行,tile) Load/Store；mid 逐行 Load/Store | **stride multi-row 空间在 wide 与 generic 的输入侧** |
| multi-row 空间（未占用） | 同 tile 列窗跨 B 行：`nBursts=B, blockLen=tile*sizeof(T), srcStride=(rowWidth-tile)*sizeof(T)` | 全文件 0 处使用 |
| 已被冠军覆盖 | CONTIG/BATCH flat 连续整行；full-tile/full-row batched output；wide full-y y 多行驻留（V003 血统）；V011 2-deep MTE2 行流水 | 不得重做 |
| 范围外（禁止） | gamma/bias 面板（parameter locality）；输出 Store 合并（store merge）；行归属与块数（row scheduling）；tile 宽度选择（tiling） | V001 不碰 |

## 5. 对 V001 的直接含义

1. 改动点收敛在 `Load` 的调用形态（必要时新增一个 stride 多行助手），不动 `Store`。
2. 可安全发起的 stride 事务条件：`blockLen = tileBytes` 32B 对齐、`srcStride=(rowWidth-tile)*elem` 32B 对齐（tileWidth 整除 rowWidth 的满 tile 即满足）；尾 tile（valid<tileWidth）回退现有单 burst，避免 R015 式 pad 多 burst 运行时风险。
3. 接收端需要 `B*tile` 的连续 UB 暂存（dstStride=0）。wide 低精度路径 xBuf_/residualBuf_ 现为 2 tile（V011 ping-pong）；B=2 时一个 stride 事务正好落满一个 slot。UB 增量必须计入，且不得动 `ChooseWideFullYRows` 的 y 预算语义（否则会变成 full-y 形态改动）。
4. dispatch 门槛、所有权（blockIdx/baseRows/extraRows）、数学序、参数加载全部保持不变——满足「不被 mode-selection 偷走」与 OFAT。
