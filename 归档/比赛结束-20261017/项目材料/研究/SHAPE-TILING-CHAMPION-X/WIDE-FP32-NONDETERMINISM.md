# WIDE-FP32 invRms 非确定性 — 供 Main 汇总给 R31B lane 与 Planning

日期：2026-09-28
发现方：SHAPE-TILING-CHAMPION-X（V001 正确性对照）
对象源码：R31B V011（`线上结果/R31B/V011/submission.asc`，SHA `a8c19a19…`，Official 45.16 Champion）
本文件只描述事实与影响面，不改其他 lane 的任何文件。

## 事实

同一 binary、同一输入、两个独立进程的完整输出对比（device 4，CANN 8.5.0.alpha002，Ascend910B3）：

| 对比 | 变化元素数 | max_abs |
|---|---|---|
| FP32 D=32768，candidate 自比 | 30304 / 65536 | 0.0930 |
| FP32 D=32768，parent 自比 | 35072 / 65536 | 0.0936 |
| FP16 D=16384，candidate 自比 | 0 / 65536 | 0 |

由输出反推的 invRms（row 0，col 0，y=0.11，gamma=0.85，bias=-0.025）：

| 来源 | invRms |
|---|---|
| run 1 | 6.63797 |
| run 2 | 6.58295 |
| double 参照 | 6.49451 |

漂移形态是「整行输出按 invRms 等比偏移」，不是未写入的垃圾值；差异元素成 7 个连续块铺满输出。

## 影响面

- 触发路径：`ProcessWideFp32FullCacheRows`（FP32 且 `rowWidth > kCacheElems=8192`，即 D≥12288 的宽路径）。
- FP16 / BF16 宽路径（`ProcessWideLowPrecision`）确定性正常，逐位可复现。
- BF16 输出量化把这个量级的 invRms 漂移完全吃掉（对 double 参照 max_abs=0），所以 BF16 全过**不能**证明归约正确。
- FP16 对参照有 3–14 个元素 Inf，parent 与 candidate 完全一致，且 FP16 自身确定性正常——属于参照/容差差异，不是同一类问题。
- R31B V016 此前把「FP32 D16384 父/子共性失败」记为 inherited/harness issue。本发现把它收窄为：**不是 harness 参照错，而是宽 FP32 归约的 invRms 本身跨 run 变动约 0.8%，且与 double 参照偏差约 0.5–2%。**

## 机制猜测（未定位到具体指令）

`kWideFullYReduceStride = 16`，而 tileCount 在常见 D 上是 3 / 4 / 8。per-tile partial 写在
`reduceLocal[batchRow * 16 + tile]`，收尾 `ReduceSum(..., tileCount)` 只声明 count=tileCount。
若收尾归约按固定宽度取数、或 workspace/尾部槽位未初始化被读入，就会混进跨 run 变化的量。
`ProcessWideLowPrecision` 用同样的 16-stride 索引却确定性正常，所以更可能与 FP32 路径的
buffer 复用（xBuf_/residualBuf_ 在 pass1 当 x/残差、pass2 当 gamma/bias）或它的收尾同步有关。
尚未定位到具体指令，需要单步验证。

## 对各 lane 的含义

| lane | 含义 |
|---|---|
| R31B | V011 起宽 FP32 数值就不稳定；V016 只测 FP16/BF16 改动域所以避开了。后续任何宽 FP32 改动的正确性对照都会带这个噪声。 |
| R31A | 宽路径同源（相似度 0.947）；`kWideFp32CachedTileElems=7680` 走的是另一函数 `ProcessWideFp32CachedRows`，是否同病灶未测。 |
| SHAPE-TILING-CHAMPION-X | V001 的改动域（FP32 D=12288/16384）全落在噪声区。untouched 格子逐位一致，OFAT 数值安全已证；延迟测量仍可做，但正确性对照只能记 COMMON_MODE_WITH_PARENT。 |
| SPLIT_D / 归约类 | 在归约稳定性弄清之前，任何内核内 D-tile 两段归约的正确性都很难判定。 |

## 证据位置

- `本地实验/SHAPE-TILING-CHAMPION-X/V001/logs/build_and_test.log`（父/子对照、PARENT_VS_CANDIDATE DELTA）
- `本地实验/SHAPE-TILING-CHAMPION-X/V001/local-result.json`（determinism_probe 字段）
- server3 原始 dump：`/home/data4t2/lelinfeng/shape-tiling-champion-x/V001/support/dumps/`
