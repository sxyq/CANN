# Revision 声明 — SHAPE-TILING-CHAMPION-X V002

（改代码之前记录；本文件在代码修改前写入。）
来源：MAIN-1 SELECTION 2026-09-29，选定假设 H2。

```text
ROUTE=SHAPE-TILING-CHAMPION-X
REVISION=V002
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=Official 45.16
SINGLE_HYPOTHESIS=FP32 D=32768 宽行 seed tile 4096→6112，使 UB 预算把字节花在更少的 tile 往返（8→6）；不改其他 (dtype, D) 格子。
CONTEXT_CLASS=CHAMPION_SEEDED_TILING
WHY_NOT_DUPLICATE=对照 DEDUP-MATRIX 与 V001：V001 是 D≤16384 档（NEEDS_ONE_MORE_LOCAL，不叠加）；本改动是 D=32768 专属档。WIDE-X-FRESH4 是弱父全局 2048→4096；R31B V016 只动 FP16/BF16 seed；R31A 是 7680 CachedRows 另一函数路径；FULL-R029/MODE-X 是五模式 taxonomy。四者都没做 FP32 D=32768 的 tile 档。
EXPECTED_SHAPES=主探针 FP32 D=32768（rows=2 与 rows=8）；空白对照 FP32 D=8192 / D=12288 / D=16384、FP16 D=32768、BF16 D=32768
EXPECTED_RISK=正确性风险低（数值顺序与归约语义不变，只改 tile 切分）；性能方向不确定（tile 往返 8→6 的收益 vs 单次 DMA 变长）
```

## MINIMAL_OFAT_DIFF

只改 `Init` 中 FP32 宽行分支的 seed tile 取值：`rowWidth >= 32768` 时 seed=6112，否则 seed=4096（即 `kWideFullYTileElems`）。
不改 `ChooseWideFullYRows` 算法、不改任何 `Process*` 函数、不改 reduction / store 事务 / dtype 数值路径 / 行调度。
不叠加 V001 的 D≤16384 档（V001 是 `NEEDS_ONE_MORE_LOCAL`，不是 LOCAL_BEST）。

## 为什么取 6112 而不是 6144

`kWideFullYBudgetBytes = 176 * 1024 = 180224`。
FP32 D=32768：`yBytesPerRow = 32768 * 4 = 131072`，`ioTiles=2, ioTypeBytes=4, workTiles=0`，
`reduceBytes(N=1) = kWideFullYReduceStride * 4 = 64`。

`need(tile) = 131072 + 2 * tile * 4 + 64 = 131136 + 8 * tile`

| tile | need (B) | 预算 180224 | 结论 | tile 数 ceil(32768/tile) |
|---|---|---|---|---|
| 4096（现状） | 163904 | 余 16320 | 在预算内 | 8 |
| **6112（选定）** | **180032** | **余 192** | **在预算内** | **6** |
| 6136 | 180224 | 余 0 | 贴边 | 6 |
| 6144 | 180288 | **超 64** | **不进预算** | — |
| 5632 | 176192 | 余 4032 | 在预算内（6144 会被步进到这里） | 6 |

`ChooseWideFullYRows` 的 N=1 回退循环每次 `tileElems -= 512`：若 seed 取 6144，第一次判定超预算后落到 **5632**，
实际档位不是 6144。取 **6112**（源码已有常量 `kWideFp32BatchTileElems = 6112` 同值）既进预算又保留 192B 余量，
tile 往返 8→6。6136 虽同为 6 往返但贴死预算，不如 6112 稳。

## 正确性门槛

宽 FP32 invRms 非确定性已知（`研究/SHAPE-TILING-CHAMPION-X/WIDE-FP32-NONDETERMINISM.md`），
按 MAIN-1 先例记 `COMMON_MODE_WITH_PARENT`，记录但不阻断。
仍须：编译 → 父/子对照正确性 → same-binary → 交错 P/C（带空白对照定噪声带）。
变更域 paired median 超出空白对照噪声带才算信号。
