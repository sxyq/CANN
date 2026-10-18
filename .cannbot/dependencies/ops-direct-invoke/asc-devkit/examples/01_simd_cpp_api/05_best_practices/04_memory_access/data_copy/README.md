# DataCopy 最佳实践样例

## 概述

本样例展示 Global Memory 到 UB、Global Memory 到 L1 的数据搬运实践。样例不包含计算逻辑，重点用于观察 MTE2 搬运行为，以及分块粒度、非对齐数据搬运、L2Cache 复用、同地址访问冲突规避对数据搬运性能的影响，对比 DataCopy/DataCopyPad 在不同搬运模式下的性能表现。

## 本样例支持的产品及CANN软件版本

| 产品 | CANN软件版本 |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 | >= CANN 9.0.0 |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 | >= CANN 9.0.0 |

## 目录结构介绍

```
├── data_copy
│   ├── scripts
│   │   └── gen_data.py        // 输入数据生成脚本
│   ├── CMakeLists.txt         // 编译工程文件
│   ├── data_copy.asc          // Ascend C 样例入口与 Kernel 调用
│   ├── data_copy_l1.h         // GM 到 L1 搬运实现
│   ├── data_copy_ub.h         // GM 到 UB 搬运实现
│   ├── data_utils.h           // 数据读入写出函数
│   └── README.md              // 样例说明文档
```

## 样例描述

本样例输入为 half 类型二维矩阵，格式为 ND。对齐场景输入shape为[12288, 12288], 非对齐场景输入shape为[12288, 12287]。通过编译选项 `COPY_DST` 选择目标存储位置，通过 `SCENARIO_NUM` 选择不同搬运场景。

- `COPY_DST=UB`：使用 AIV 核执行 GM 到 UB 搬运，Kernel 名为 `kernel_data_copy_pad_gm2ub`
- `COPY_DST=L1`：使用 AIC 核执行 GM 到 L1 搬运，Kernel 名为 `kernel_data_copy_gm2l1`

## 样例实现与性能分析

为便于表格展示，下文将 Atlas A2 训练系列产品/Atlas A2 推理系列产品、Atlas A3 训练系列产品/Atlas A3 推理系列产品简称为 Atlas A2/A3 系列，将 Ascend 950PR/Ascend 950DT 简称为 Ascend 950 系列。

本章围绕数据搬运优化中最常见的几个问题展开：先说明性能指标含义，再分别比较分块粒度、非对齐数据、L2Cache 复用和多核同地址访问冲突。每个优化点都包含实现方式、对比方式、性能数据和结论，便于将代码行为和性能变化对应起来。

### 核心特性总览

| 优化点 | 主要观察对象 | 对比方法 |
| --- | --- | --- |
| 分块粒度 | 单次 DataCopy 搬运大小对 MTE2 效率的影响 | 保持矩阵规模不变，调整 TILE_M/TILE_N |
| 非对齐数据搬运 | 非整除 shape 对搬运开销的影响 | 保持分块大小不变，将 N=12288 改为 N=12287 |
| L2Cache 复用 | 重复访问相同 GM 数据时的 Cache 收益 | 比较整块重复搬运和 N 方向分片后重复搬运 |
| 同地址冲突规避 | 多核同时访问相同 GM 地址段的冲突影响 | 比较所有核同序访问和按核错开访问 |

### 性能指标说明

普通 MTE2 性能表主要观察搬运耗时和搬运指令占比，字段含义如下。

表格中的“性能提升”按 `基线耗时 / 当前耗时 - 1` 计算；“性能变化”为正表示性能提升，为负表示性能劣化。

| 字段名 | 字段含义 |
| --- | --- |
| Task Duration(μs) | 整个任务执行的总时间，算子执行时间以该参数为准。 |
| *_total_cycles | Task 执行总 cycle 数。 |
| *_mte2_time(μs) | MTE2 类型指令（DDR -> AI Core 搬运类指令）耗时，单位为 μs。 |
| *_mte2_ratio | MTE2 类型指令（DDR -> AI Core 搬运类指令）的 cycle 数在 total cycle 数中的占比。 |

L2Cache 性能表在 MTE2 耗时之外，还会展示 Cache 命中、缺失和换出相关计数，字段含义如下。

| 字段名 | 字段含义 |
| --- | --- |
| Task Duration(μs) | 整个任务执行的总时间，算子执行时间以该参数为准。 |
| *_total_cycles | Task 执行总 cycle 数。 |
| *_time(μs) | Task 在对应 AI Core 上的理论执行时间，单位为 μs。 |
| *_write_cache_hit | 写 Cache 命中的次数。 |
| *_write_cache_miss_allocate | 写 Cache 缺失后重新分配缓存的次数。 |
| *_r*_read_cache_hit | 读 r* 通道 Cache 命中次数，r0/r1 为硬件读写的两个通道，分析总读命中次数时需将两个通道累加。 |
| *_r*_read_cache_miss_allocate | 读 r* 通道 Cache 缺失后重新分配的次数，r0/r1 为硬件读写的两个通道，分析总读缺失次数时需将两个通道累加。 |
| *_read_local_l2_hit | 读 Cache 命中的次数。 |
| *_read_local_l2_miss | 读 Cache 缺失次数。 |
| *_read_local_l2_victim | 读 Cache 未命中并触发 Cache 中数据被换出的次数。 |

### 优化点1：分块粒度对搬运效率的影响

**实现方式**：参考 `data_copy_ub.h` 中 GM 到 UB 的 `DataCopyPad` 搬运，以及 `data_copy_l1.h` 中 GM 到 L1 的 `DataCopy` 搬运。通过编译期场景参数切换不同 `TILE_M/TILE_N` 组合。保持输入矩阵规模不变，仅改变 `TILE_M/TILE_N`。小分块会增加搬运指令数量，大分块减少指令发射次数，但需要更大的片上临时空间。

先看单次 DataCopy 搬运的形状：UB 路径按 ND 布局直接搬入 UB，L1 路径在搬入 L1 时完成 ND 到 NZ 的布局转换。两条路径使用的参数不同，但都由 `TILE_M/TILE_N` 决定单次搬运的二维块大小。

| 路径 | 参数字段 | 本样例取值 | 含义 |
| --- | --- | --- | --- |
| GM 到 UB | DataCopyParams.blockCount | tileM | 一次搬运的行数 |
| DataCopyParams.blockLen | curCols * sizeof(half) | 每行连续搬运的字节数 |  |
| DataCopyParams.srcStride | (n - curCols) * sizeof(half) | 源端相邻两行之间跳过的字节数 |  |
| DataCopyParams.dstStride | 0 | UB 中连续存放，不额外跳行 |  |
| GM 到 L1 | Nd2NzParams.nValue | tileM | ND 源矩阵中本次搬运的行数 |
| Nd2NzParams.dValue | curCols 或 tileN | ND 源矩阵中本次搬运的列数 |  |
| Nd2NzParams.srcDValue | n | GM 中原矩阵的行宽 |  |
| Nd2NzParams.dstNzC0Stride | AlignUp(tileM, 16) | L1 中 NZ 布局的 C0 方向跨度 |  |

本样例中，单次 DataCopy 源数据量均按 `tileM * curCols * sizeof(half)` 计算：

| 路径 | 场景 | Tile | 单次搬运量 |
| --- | --- | --- | --- |
| GM 到 UB | 场景1 | [1,64] | 128B |
| 场景2 | [64,64] | 8192B |  |
| 场景3 | [64,1024] | 131072B |  |
| GM 到 L1 | 场景1 | [64,64] | 8192B |
| 场景2 | [64,256] | 32768B |  |

分块变大后，每次指令实际搬运的数据更多，循环次数和指令发射次数同步减少。

**分核与数据载入模式**：

```text
输入矩阵 GM: [M, N]

M 方向按核切分：
┌────────────── M ──────────────┐
│ core0: singleCoreM 行         │
│ core1: singleCoreM 行         │
│ ...                           │  NUM_BLOCKS 个核并行
│ last core: singleCoreM 行     │
└───────────────────────────────┘

单核内部按 tile 搬运：
singleCoreM 行
┌──── tileN ────┬──── tileN ────┬──── tileN ────┐
│ tileM 行      │ tileM 行      │ tileM 行      │
├───────────────┼───────────────┼───────────────┤
│ tileM 行      │ tileM 行      │ tileM 行      │
└───────────────┴───────────────┴───────────────┘

GM -> UB: DataCopyPad，AIV 核执行
GM -> L1: DataCopy(ND2NZ)，AIC 核执行
```

**MTE2 带宽理论分析**：

本组基础场景只做单次 GM 读入，不考虑 L2Cache 复用收益。输入矩阵为 `M=12288, N=12288`，数据类型为 half，读入数据总量为：

$$读入数据总量 = M \times N \times sizeof(half) = 12288 \times 12288 \times 2B = 301989888B \approx 301.99MB$$

按 GM 峰值带宽粗略估算 MTE2 理论耗时：

$$MTE2理论耗时 = \frac{301.99MB}{GM 峰值带宽}$$

Atlas A2/A3 系列按 GM 带宽约 1.8TB/s 估算，理论耗时约 167.77μs。大分块场景下，GM 到 UB 的 `aiv_mte2_time` 为 202.657μs，GM 到 L1 的 `aic_mte2_time` 为 214.519μs，实测相对理论值分别高约 20.8% 和 27.9%。

Ascend 950 系列按 GM 带宽约 1.6TB/s 估算，理论耗时约 188.74μs。大分块场景下，GM 到 UB 的 `aiv_mte2_time` 为 185.66μs，GM 到 L1 的 `aic_mte2_time` 为 187.19μs，实测与理论估算基本接近。该估算只用于判断搬运效率量级，实际性能会受到指令发射、地址连续性、DataCopyPad/ND2NZ 处理等因素影响。

**GM 到 UB 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | 相对基线MTE2性能提升 | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 1 | Tensor=[12288,12288]Tile=[1,64]DataCopyParams={blockCount=1, blockLen=128B, srcStride=24448B}Block Num=48 | 564.84 | 49222116 | 548.161 | 0.989 | 基线 | 小分块搬运 |
| 2 | Tensor=[12288,12288]Tile=[64,64]DataCopyParams={blockCount=64, blockLen=128B, srcStride=24448B}Block Num=48 | 233.54 | 20167322 | 220.772 | 0.972 | +148.3% | 中等分块搬运 |  |
| 3 | Tensor=[12288,12288]Tile=[64,1024]DataCopyParams={blockCount=64, blockLen=2048B, srcStride=22528B}Block Num=48 | 215.82 | 18563430 | 202.657 | 0.969 | +170.5% | 大分块搬运 |  |
| Ascend 950 系列 | 1 | Tensor=[12288,12288]Tile=[1,64]DataCopyParams={blockCount=1, blockLen=128B, srcStride=24448B}Block Num=64 | 884.06 | 91585425 | 881.13 | 1 | 基线 | 小分块搬运 |
| 2 | Tensor=[12288,12288]Tile=[64,64]DataCopyParams={blockCount=64, blockLen=128B, srcStride=24448B}Block Num=64 | 208.64 | 31489587 | 205.99 | 0.99 | +327.8% | 中等分块搬运 |  |
| 3 | Tensor=[12288,12288]Tile=[64,1024]DataCopyParams={blockCount=64, blockLen=2048B, srcStride=22528B}Block Num=64 | 188.52 | 19710997 | 185.66 | 0.99 | +374.6% | 大分块搬运 |  |

**GM 到 L1 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | 相对基线MTE2性能提升 | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 1 | Tensor=[12288,12288]Tile=[64,64]Nd2NzParams={nValue=64, dValue=64, srcDValue=12288}Block Num=24 | 300.7 | 12870910 | 283.508 | 0.978 | 基线 | 中等分块搬运 |
| 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=24 | 230.24 | 9795512 | 214.519 | 0.972 | +32.2% | 大分块搬运 |  |
| Ascend 950 系列 | 1 | Tensor=[12288,12288]Tile=[64,64]Nd2NzParams={nValue=64, dValue=64, srcDValue=12288}Block Num=32 | 245.1 | 12480180 | 241.56 | 0.99 | 基线 | 中等分块搬运 |
| 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=32 | 190.52 | 9853588 | 187.19 | 0.99 | +29.0% | 大分块搬运 |  |

**优化效果分析**：
- 增大分块可以显著降低搬运指令发射开销。GM 到 UB 场景中，大分块相对小分块在 Atlas A2/A3 系列上端到端性能提升约 161.7%，在 Ascend 950 系列上提升约 369.0%。
- GM 到 L1 场景中，增大 `TILE_N` 后，Atlas A2/A3 系列、Ascend 950 系列的端到端性能均提升约 29% 以上，说明减少过小搬运粒度带来的循环与指令开销后，MTE2 搬运效率更高。
- 实际配置时建议在片上空间允许的前提下优先增大单次搬运大小，同时保持分块不超过 UB/L1 可用缓存。单次搬运字节数越大、`mLoopCount/nLoopCount` 越少，DataCopy 指令数量和循环控制开销越低。

### 优化点2：非对齐数据搬运影响

**实现方式**：非对齐数据场景复用与对齐场景相同的搬运流程，仅改变矩阵 `N` 维，使最后一个搬运块无法按完整列宽覆盖。

非对齐时，最后一个 N 方向 tile 的 `curCols` 小于 `TILE_N`。UB 路径中，`DataCopyParams.blockLen` 会从完整的 `TILE_N * sizeof(half)` 变成 `curCols * sizeof(half)`；L1 路径中，`Nd2NzParams.dValue` 会从完整的 `TILE_N` 变成 `curCols`。

本样例非对齐场景中，GM 到 UB 的完整 tile 搬运量为 `64 * 1024 * 2B = 131072B`，尾块为 `64 * 1023 * 2B = 130944B`；GM 到 L1 的完整 tile 搬运量为 `64 * 256 * 2B = 32768B`，尾块为 `64 * 255 * 2B = 32640B`。非对齐场景并不是总数据量明显增加，而是尾块需要额外边界处理，导致搬运效率下降。

**对比方式**：保持分块大小不变，将 `N` 从 `12288` 改为 `12287`，观察非对齐数据处理对端到端耗时和 MTE2 耗时的影响。

**GM 到 UB 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | 相对对齐场景性能变化 | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 3 | Tensor=[12288,12288]Tile=[64,1024]blockLen=2048BsrcStride=22528BBlock Num=48 | 215.82 | 18563430 | 202.657 | 0.969 | 基线 | 对齐大分块 |
| 4 | Tensor=[12288,12287]Tile=[64,1024]full/tail curCols=1024,1023blockLen=2048B,2046BsrcStride=22526B,22528BBlock Num=48 | 275.3 | 23648813 | 259.047 | 0.973 | -21.6% | 非对齐数据大分块 |  |
| Ascend 950 系列 | 3 | Tensor=[12288,12288]Tile=[64,1024]blockLen=2048BsrcStride=22528BBlock Num=64 | 188.52 | 19710997 | 185.66 | 0.99 | 基线 | 对齐大分块 |
| 4 | Tensor=[12288,12287]Tile=[64,1024]full/tail curCols=1024,1023blockLen=2048B,2046BsrcStride=22526B,22528BBlock Num=64 | 192.07 | 19739629 | 189.16 | 0.99 | -1.8% | 非对齐数据大分块 |  |

**GM 到 L1 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | 相对对齐场景性能变化 | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=24 | 230.24 | 9795512 | 214.519 | 0.972 | 基线 | 对齐大分块 |
| 3 | Tensor=[12288,12287]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256(full),255(tail), srcDValue=12287}Block Num=24 | 438.76 | 19020328 | 422.515 | 0.986 | -47.5% | 非对齐数据大分块 |  |
| Ascend 950 系列 | 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=32 | 190.52 | 9853588 | 187.19 | 0.99 | 基线 | 对齐大分块 |
| 3 | Tensor=[12288,12287]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256(full),255(tail), srcDValue=12287}Block Num=32 | 206.61 | 10589826 | 202.97 | 0.99 | -7.8% | 非对齐数据大分块 |  |

**优化效果分析**：
- 非对齐数据会引入额外边界处理，在 Atlas A2/A3 系列上影响更明显，尤其 GM 到 L1 场景端到端性能劣化约 47.5%。
- 在 Ascend 950 系列上，非对齐数据影响相对较小，但仍会带来额外开销。选择分块时应优先保证主搬运维度对齐。
- 建议在设计矩阵 shape 和切分策略时尽量采用对齐矩阵。按本样例 half 数据类型计算，Atlas A2/A3 系列建议主搬运维度对应的连续字节数满足 512B 对齐；Ascend 950 系列建议满足 128B 对齐。

### 优化点3：重复搬运与 L2Cache 复用

**实现方式**：重复搬运场景在同一数据规模下执行多轮 GM 读取，使用 `msopprof --ai-core=on --aic-metrics=L2Cache` 采集 L2Cache 读命中和 miss allocate 数据。

整块重复搬运时，`RepeatCopy(0, n, 4)` 表示 `nStart=0`、`nCount=N`、`repeatTimes=4`，每轮都按完整 N 方向读取同一块数据。A2/A3 的 L2Cache 大小为 192MB，950PR 的 L2Cache 大小为 128MB。本样例单个 `M * N` 矩阵大小约为 301.99MB，整块重复搬运时单轮工作集大于 L2Cache 容量，容易发生写替换，导致 L2Cache 命中率较低。

分片重复搬运时，先令 `quarterN=N/4`，再对每个 `splitIdx` 调用 `RepeatCopy(splitIdx * quarterN, quarterN, 4)`，即每次只在 N 方向的 1/4 分片内连续重复 4 次。两种写法搬运总数据量相同，但分片后每个分片约为 75.50MB，单次工作集更小，更容易在连续重复访问时保留在 L2Cache 中。

**对比方式**：比较整块矩阵沿相同路径连续搬 4 次，与 N 方向切 4 份后每份连续搬 4 次。后者在每个分片内连续重复访问，更容易观察 L2Cache 复用收益。

场景5 / 6（GM 到 UB）和场景4 / 5（GM 到 L1）均使用 `Tile=[64,1024]`。单次 `DataCopyPad` 或 `DataCopy（Nd2NzParams）` 搬运的源数据量为：

$$64 \times 1024 \times sizeof(half) = 64 \times 1024 \times 2B = 131072B = 0.131072MB$$

整块重复搬运时，每轮工作集为 `301.99MB`，连续 4 次总读入 `1207.96MB`；N 方向切 4 份后，每个分片工作集为 `75.50MB`，总读入数据量仍为 `1207.96MB`，但每个分片更容易被 L2Cache 保留并复用。

**L2Cache 复用模式**：

```text
场景5(UB) / 场景4(L1)：整块矩阵连续重复搬运

GM 矩阵: [M, N]
┌─────────────────────────────── N ───────────────────────────────┐
│                         全部列一次搬完                           │
└──────────────────────────────────────────────────────────────────┘

启动全部 core，按相同路径搬运完整矩阵:
第 1 轮: 全部 core 从 GM 读取完整矩阵 -> UB 或 L1
第 2 轮: 全部 core 再次读取完整矩阵 -> UB 或 L1
第 3 轮: 全部 core 再次读取完整矩阵 -> UB 或 L1
第 4 轮: 全部 core 再次读取完整矩阵 -> UB 或 L1
说明: 每轮工作集都是整块矩阵，L2Cache 难以完整保留上一轮数据

场景6(UB) / 场景5(L1)：N 方向切 4 份后，每份连续重复搬运

GM 矩阵: [M, N]
┌──────── N/4 ────────┬──────── N/4 ────────┬──────── N/4 ────────┬──────── N/4 ────────┐
│       分片0         │       分片1         │       分片2         │       分片3          │
└─────────────────────┴─────────────────────┴─────────────────────┴─────────────────────┘

启动全部 core，先把分片0的数据连续搬运 4 轮，再处理下一个分片:
分片0: 第 1 轮从 GM 读取，第 2~4 轮优先从 L2Cache 读取
分片1: 第 1 轮从 GM 读取，第 2~4 轮优先从 L2Cache 读取
分片2: 第 1 轮从 GM 读取，第 2~4 轮优先从 L2Cache 读取
分片3: 第 1 轮从 GM 读取，第 2~4 轮优先从 L2Cache 读取
说明: 单个分片工作集更小，连续重复访问时更容易保留在 L2Cache 中
```

**L2Cache 理论性能分析**：

本组场景输入矩阵为 `M=12288, N=12288`，数据类型为 half，单次完整读入数据量为：

$$单次读入数据量 = M \times N \times sizeof(half) = 12288 \times 12288 \times 2B = 301989888B \approx 301.99MB$$

整块矩阵连续搬 4 次时，总读入数据量为：

$$整块重复读入数据量 = 301989888B \times 4 = 1207959552B \approx 1207.96MB$$

该访问方式单次工作集为 301.99MB，难以完整保留在 L2Cache 中，因此可近似按主要从 GM 读取估算：

$$整块重复理论耗时 = \frac{1207.96MB}{GM 带宽}$$

N 方向切 4 份后，每份数据量为：

$$分片数据量 = 301989888B \div 4 = 75497472B \approx 75.50MB$$

每份连续搬 4 次时，理想情况下第一次从 GM 读入，后 3 次从 L2Cache 读取：

$$GM读入数据量 = 75497472B \times 4 = 301989888B \approx 301.99MB$$

$$L2Cache读入数据量 = 75497472B \times 3 \times 4 = 905969664B \approx 905.97MB$$

$$分片重复理论耗时 = \frac{301.99MB}{GM 带宽} + \frac{905.97MB}{L2Cache 带宽}$$

Atlas A2/A3 系列按 GM 带宽约 1.8TB/s、L2Cache 峰值带宽约 5.2TB/s 估算；Ascend 950 系列按 GM 带宽约 1.6TB/s、L2Cache 峰值带宽约 5.2TB/s 估算。

$$Atlas A2/A3 系列整块重复理论耗时 = \frac{1207.96MB}{1.8TB/s} = 671.09\mu s$$

$$Ascend 950 系列整块重复理论耗时 = \frac{1207.96MB}{1.6TB/s} = 754.97\mu s$$

$$Atlas A2/A3 系列分片重复理论耗时 = \frac{301.99MB}{1.8TB/s} + \frac{905.97MB}{5.2TB/s} = 342.00\mu s$$

$$Ascend 950 系列分片重复理论耗时 = \frac{301.99MB}{1.6TB/s} + \frac{905.97MB}{5.2TB/s} = 362.97\mu s$$

从理论模型和实测结果看，N 方向分片后重复搬运的耗时更接近“首次 GM + 后续 L2Cache”的理想模型；整块重复搬运由于工作集更大，L2Cache 复用不足，耗时更接近多次从 GM 读取。GM 到 L1 场景包含 ND2NZ 搬运，实际耗时还会受到格式转换和 L1 写入布局影响，因此通常会比纯 GM 到 UB 搬运更高。

**GM 到 UB L2Cache 性能数据**：

Atlas A2/A3 系列和 Ascend 950 系列的 profiler 输出字段不同，命中率计算方式也不同：Atlas A2/A3 系列使用 `l2cache_hit_ratio = (r0_hit + r1_hit) / (r0_hit + r1_hit + r0_miss_allocate + r1_miss_allocate)`；Ascend 950 系列使用 `l2cache_hit_ratio = hit / (hit + miss + victim)`。

| 架构 | Scenario | 配置 | Task Duration(μs) | aiv_total_cycles | aiv_time(μs) | aiv_r0_read_cache_hit | aiv_r0_read_cache_miss_allocate | aiv_r1_read_cache_hit | aiv_r1_read_cache_miss_allocate | 说明 | L2Cache命中率 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=48 | 828.06 | 72465129 | 816.05 | 212 | 4718595 | 217 | 4718592 | 整块矩阵按相同路径连续搬 4 次 | 0.005% |
| 6 | Tensor=[12288,12288]Tile=[64,1024]Block Num=48 | 365.74 | 31484525 | 354.56 | 3539159 | 1179644 | 3539158 | 1179655 | N 方向切 4 份，每份连续搬 4 次 | 75.00% |  |

| 架构 | Scenario | 配置 | Task Duration(μs) | aiv_total_cycles | aiv_time(μs) | aiv_read_local_l2_hit | aiv_read_local_l2_miss | aiv_read_local_l2_victim | 说明 | L2Cache命中率 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Ascend 950 系列 | 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=64 | 741.58 | 77700635 | 740.75 | 31346 | 529720 | 8358412 | 整块矩阵按相同路径连续搬 4 次 | 0.35% |
| 6 | Tensor=[12288,12288]Tile=[64,1024]Block Num=64 | 354.95 | 36964347 | 354.19 | 5943026 | 528797 | 2446732 | N 方向切 4 份，每份连续搬 4 次 | 66.64% |  |

**GM 到 L1 L2Cache 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aic_total_cycles | aicore_time(μs) | aic_r0_read_cache_hit | aic_r0_read_cache_miss_allocate | aic_r1_read_cache_hit | aic_r1_read_cache_miss_allocate | 说明 | L2Cache命中率 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 4 | Tensor=[12288,12288]Tile=[64,1024]Block Num=24 | 899.34 | 38725327 | 872.19 | 305 | 4718591 | 287 | 4718601 | 整块矩阵按相同路径连续 ND2NZ 搬 4 次 | 0.006% |
| 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=24 | 414.96 | 17662155 | 397.8 | 3539061 | 1179684 | 3539133 | 1179616 | N 方向切 4 份，每份连续 ND2NZ 搬 4 次 | 75.00% |  |

| 架构 | Scenario | 配置 | Task Duration(μs) | aic_total_cycles | aicore_time(μs) | aic_read_local_l2_hit | aic_read_local_l2_miss | aic_read_local_l2_victim | 说明 | L2Cache命中率 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Ascend 950 系列 | 4 | Tensor=[12288,12288]Tile=[64,1024]Block Num=32 | 732.809 | 37950511 | 732.11 | 269472 | 2557819 | 4060060 | 整块矩阵按相同路径连续 ND2NZ 搬 4 次 | 3.91% |
| 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=32 | 390.347 | 19858177 | 389.64 | 5948502 | 1148258 | 1220548 | N 方向切 4 份，每份连续 ND2NZ 搬 4 次 | 71.52% |  |

**优化效果分析**：
- N 方向切分后在分片内重复访问，L2Cache 复用更充分，GM 到 UB 和 GM 到 L1 的端到端性能在 Atlas A2/A3 系列、Ascend 950 系列上均有明显提升。
- Atlas A2/A3 系列需按 r0+r1 两个硬件读通道累加观察，总读 Cache 命中明显增加，miss allocate 明显降低；Ascend 950 系列对应表现为 `*_read_local_l2_hit` 增加，miss/victim 降低。
- 建议在同一批 GM 数据需要被多次读取时，优先考虑分片搬运，并在每个分片内连续完成多次访问，使单次工作集尽量落在 L2Cache 可复用范围内。

### 优化点4：多核同地址访问冲突规避

**实现方式**：same addr 场景中所有核按相同 `mBlockIdx` 顺序访问输入矩阵；offset addr 场景中每个核按 `(mBlockIdx + blockIdx) % numBlocks` 错开访问顺序。

该优化点不改变单次 DataCopy 的搬运形状，只改变不同核访问 GM 分片的顺序。每个核都会完整载入一遍输入矩阵，整体会将同一矩阵重复读取 `numBlocks` 次。same addr 模式下，所有核在同一个 `mBlockIdx` 上同步访问相同地址段；offset addr 模式下，`curMBlockIdx` 会按 `blockIdx` 在每组 `numBlocks` 个 M 块内轮转，降低同一时刻多核访问同一 GM 地址段的概率。

**关键代码**：

```cpp
constexpr uint32_t fullMBlockCount = m / singleCoreM;
constexpr uint32_t mTileCount = singleCoreM / tileM;

for (uint32_t mBlockIdx = 0; mBlockIdx < fullMBlockCount; mBlockIdx++) {
    uint32_t blockGroupStart = (mBlockIdx / numBlocks) * numBlocks;
    uint32_t curMBlockIdx = offsetAddr ? blockGroupStart + (mBlockIdx + blockIdx) % numBlocks : mBlockIdx;
    uint32_t mStart = curMBlockIdx * singleCoreM;

    for (uint32_t mTileIdx = 0; mTileIdx < mTileCount; mTileIdx++) {
        uint32_t mIdx = mStart + mTileIdx * tileM;
    }
}
```

本样例中，每次 DataCopy 在 N 方向按 `tileN` 粒度搬运；`offsetAddr=true` 时通过错开各核访问分片的顺序，让不同核在同一时刻访问不同的 GM 地址段。

**对比方式**：所有核完整载入同一输入矩阵，比较相同顺序访问与按核错开访问顺序。错开访问通过调整并行分片访问顺序，降低同一时刻多核访问同地址的冲突概率。

**GM 到 UB 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 7 | Tensor=[6144,512]Tile=[128,64]Block Num=48 | 539.42 | 42823246 | 474.717 | 0.984 | 所有核按相同顺序完整载入 |
| 8 | Tensor=[6144,512]Tile=[128,64]Block Num=48 | 328.88 | 28054500 | 307.38 | 0.973 | 所有核错开分片顺序完整载入 |  |
| Ascend 950 系列 | 7 | Tensor=[8192,512]Tile=[128,64]Block Num=64 | 342.29 | 33624098 | 339.38 | 0.99 | 所有核按相同顺序完整载入 |
| 8 | Tensor=[8192,512]Tile=[128,64]Block Num=64 | 335.64 | 35298771 | 333.08 | 0.99 | 所有核错开分片顺序完整载入 |  |

**GM 到 L1 性能数据**：

| 架构 | Scenario | 配置 | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | 说明 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 系列 | 6 | Tensor=[6144,512]Tile=[256,64]Block Num=24 | 278.56 | 10880150 | 240.198 | 0.98 | 所有核按相同顺序完整载入 |
| 7 | Tensor=[6144,512]Tile=[256,64]Block Num=24 | 221.34 | 9532153 | 209.8 | 0.977 | 所有核错开分片顺序完整载入 |  |
| Ascend 950 系列 | 6 | Tensor=[8192,512]Tile=[256,64]Block Num=32 | 369.99 | 13312673 | 366.24 | 0.99 | 所有核按相同顺序完整载入 |
| 7 | Tensor=[8192,512]Tile=[256,64]Block Num=32 | 187.35 | 9713703 | 185.03 | 0.99 | 所有核错开分片顺序完整载入 |  |

**优化效果分析**：
- offset addr 通过错开多核访问顺序，降低同一时刻访问同一 GM 地址段的冲突概率。Atlas A2/A3 系列的 UB/L1 场景和 Ascend 950 系列的 L1 场景收益更明显。
- 从端到端性能看，Atlas A2/A3 系列的 UB 场景提升约 64.0%，Ascend 950 系列的 L1 场景提升约 97.5%；Ascend 950 系列的 UB 场景收益较小。

### 优化要点总结

| 优化手段 | 核心原理 | 使用建议 |
|:---|:---|:---|
| 增大搬运分块 | 减少 DataCopy 指令次数和循环控制开销，提高 MTE2 有效搬运效率 | 在片上空间允许时，优先选择较大的 `TILE_M/TILE_N` |
| 保持主维度对齐 | 避免非对齐数据带来的边界处理和非完整搬运开销 | 设计 shape 或切分策略时，尽量让主搬运维度被 `TILE_N` 整除，并使连续搬运字节数满足 Atlas A2/A3 系列 512B、Ascend 950 系列 128B 对齐 |
| 分片后重复访问 | 将重复访问限制在更小的数据范围内，提高 L2Cache 命中机会 | 需要多次读取同一批 GM 数据时，优先考虑先分片再在片内重复处理 |
| 错开多核访问顺序 | 降低多个核同一时刻访问相同 GM 地址段的概率 | 多核读取同一大块数据时，可按 `blockIdx` 轮转访问分片顺序 |

## 编译运行

在本样例根目录下执行如下步骤，编译并执行样例。

- 配置环境变量

  请根据当前环境上 CANN 开发套件包的[安装方式](../../../../../docs/zh/quick_start.md#prepare&install)，配置环境变量。

  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **说明：** `${install_path}` 为CANN包安装目录，未指定安装目录时默认安装至 `/usr/local/Ascend` 下。

- 样例执行

  在本样例目录下执行如下命令。
  ```bash
  SCENARIO_NUM=1 ASC_ARCH=dav-2201
  COPY_DST=UB
  mkdir -p build && cd build
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCOPY_DST=$COPY_DST -DCMAKE_ASC_ARCHITECTURES=$ASC_ARCH ..;make -j;
  python3 ../scripts/gen_data.py -scenarioNum $SCENARIO_NUM -copyDst $COPY_DST -arch $ASC_ARCH
  ./demo
  ```

  使用 NPU 仿真模式时，添加 `-DCMAKE_ASC_RUN_MODE=sim` 参数即可。

  示例：

  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCOPY_DST=$COPY_DST -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..
  make -j
  ```

  > **注意：** 切换编译模式前需清理 cmake 缓存，可在 build 目录下执行 `rm CMakeCache.txt` 后重新 cmake。

- 编译选项说明

  | 参数 | 说明 | 可选值 | 默认值 |
  |------|------|--------|--------|
  | `SCENARIO_NUM` | 场景编号 | `COPY_DST=UB`: 1-8; `COPY_DST=L1`: 1-7 | 1 |
  | `COPY_DST` | 搬运目标 | `UB`、`L1` | `UB` |
  | `CMAKE_ASC_RUN_MODE` | 运行模式 | `npu`、`sim` | `npu` |
  | `CMAKE_ASC_ARCHITECTURES` | NPU 硬件架构 | `dav-2201`、`dav-3510` | `dav-2201` |

- 性能采集

  使用 `msOpProf` 工具获取详细性能数据：

  ```bash
  msopprof ./demo
  msopprof --ai-core=on --aic-metrics=L2Cache ./demo    # L2Cache 相关场景使用
  ```

    - 性能数据说明
      命令完成后，会在默认目录下生成以"OPPROF_{timestamp}_XXX"命名的文件夹,性能数据文件夹结构示例如下：

      ```bash
      ├──dump                       # 原始的性能数据，用户无需关注
      ├──ArithmeticUtilization.csv  # cube/vector指令cycle占比
      ├──L2Cache.csv                # L2 Cache命中率，影响MTE2，建议合理规划数据搬运逻辑，增加命中率
      ├──Memory.csv                 # UB，L1和主存储器读写带宽速率
      ├──MemoryL0.csv               # L0A，L0B，和L0C读写带宽速率
      ├──MemoryUB.csv               # Vector和Scalar到UB的读写带宽速率
      ├──OpBasicInfo.csv            # 算子基础信息
      ├──PipeUtilization.csv        # 采集计算单元和搬运单元耗时和占比
      ├──ResourceConflictRatio.csv  # UB上的 bank group、bank conflict和资源冲突率在所有指令中的占比
      └──visualize_data.bin         # MindStudio Insight呈现文件
      ```

  查看具体的性能分析结果：

  ```bash
  # 查看 Task Duration 以及各项数据
  cat ./OPPROF_*/PipeUtilization.csv
  ```
