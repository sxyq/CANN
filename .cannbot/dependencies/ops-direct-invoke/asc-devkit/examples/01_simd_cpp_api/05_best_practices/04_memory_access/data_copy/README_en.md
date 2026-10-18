# DataCopy Best Practice Example

## Overview

This example demonstrates data transfer practices from Global Memory to UB and from Global Memory to L1. The example does not include computation logic and focuses on observing MTE2 transfer behavior, as well as the impact of block granularity, unaligned data transfer, L2Cache reuse, and same-address access conflict avoidance on data transfer performance. It compares the performance of DataCopy/DataCopyPad across different transfer modes.

## Supported Products and CANN Versions

| Product | CANN Version |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |
| Atlas A3 Training Series Products/Atlas A3 Inference Series Products | >= CANN 9.0.0 |
| Atlas A2 Training Series Products/Atlas A2 Inference Series Products | >= CANN 9.0.0 |

## Directory Structure

```
├── data_copy
│   ├── scripts
│   │   └── gen_data.py        // Input data generation script
│   ├── CMakeLists.txt         // Build project file
│   ├── data_copy.asc          // Ascend C example entry and Kernel invocation
│   ├── data_copy_l1.h         // GM to L1 transfer implementation
│   ├── data_copy_ub.h         // GM to UB transfer implementation
│   ├── data_utils.h           // Data read/write functions
│   └── README.md              // Example documentation
```

## Example Description

The input for this example is a half-type 2D matrix in ND format. The aligned scenario input shape is [12288, 12288], and the unaligned scenario input shape is [12288, 12287]. The destination storage location is selected through the build option `COPY_DST`, and different transfer scenarios are selected through `SCENARIO_NUM`.

- `COPY_DST=UB`: Uses AIV cores to perform GM to UB transfer. Kernel name is `kernel_data_copy_pad_gm2ub`
- `COPY_DST=L1`: Uses AIC cores to perform GM to L1 transfer. Kernel name is `kernel_data_copy_gm2l1`

## Example Implementation and Performance Analysis

For table presentation convenience, the following text refers to Atlas A2 Training Series Products/Atlas A2 Inference Series Products and Atlas A3 Training Series Products/Atlas A3 Inference Series Products collectively as Atlas A2/A3 Series, and Ascend 950PR/Ascend 950DT as Ascend 950 Series.

This chapter addresses the most common issues in data transfer optimization: it first explains performance metric meanings, then compares block granularity, unaligned data, L2Cache reuse, and multi-core same-address access conflicts. Each optimization point includes implementation method, comparison method, performance data, and conclusions, making it easy to correlate code behavior with performance changes.

### Core Feature Overview

| Optimization Point | Primary Observation Target | Comparison Method |
| --- | --- | --- |
| Block Granularity | Impact of single DataCopy transfer size on MTE2 efficiency | Keep matrix size constant, adjust TILE_M/TILE_N |
| Unaligned Data Transfer | Impact of non-divisible shapes on transfer overhead | Keep block size constant, change N=12288 to N=12287 |
| L2Cache Reuse | Cache benefit when repeatedly accessing the same GM data | Compare full-block repeated transfer vs. N-direction sliced repeated transfer |
| Same-Address Conflict Avoidance | Conflict impact when multiple cores access the same GM address range simultaneously | Compare all cores accessing in the same order vs. staggered access by core |

### Performance Metric Description

The standard MTE2 performance table primarily observes transfer duration and transfer instruction ratio. Field meanings are as follows.

"Performance improvement" in the table is calculated as `baseline duration / current duration - 1`; positive "performance change" indicates performance improvement, and negative indicates performance degradation.

| Field Name | Field Meaning |
| --- | --- |
| Task Duration(μs) | Total execution time of the entire task. Operator execution time is determined by this parameter. |
| *_total_cycles | Total cycle count for Task execution. |
| *_mte2_time(μs) | MTE2 type instruction (DDR -> AI Core transfer instructions) duration, in μs. |
| *_mte2_ratio | Ratio of MTE2 type instruction (DDR -> AI Core transfer instructions) cycle count to total cycle count. |

The L2Cache performance table additionally shows cache hit, miss, and eviction related counts beyond MTE2 duration. Field meanings are as follows.

| Field Name | Field Meaning |
| --- | --- |
| Task Duration(μs) | Total execution time of the entire task. Operator execution time is determined by this parameter. |
| *_total_cycles | Total cycle count for Task execution. |
| *_time(μs) | Theoretical execution time of the Task on the corresponding AI Core, in μs. |
| *_write_cache_hit | Number of write Cache hits. |
| *_write_cache_miss_allocate | Number of cache reallocations after write Cache misses. |
| *_r*_read_cache_hit | Number of read r* channel Cache hits. r0/r1 are the two hardware read/write channels. When analyzing total read hits, accumulate both channels. |
| *_r*_read_cache_miss_allocate | Number of reallocations after read r* channel Cache misses. r0/r1 are the two hardware read/write channels. When analyzing total read misses, accumulate both channels. |
| *_read_local_l2_hit | Number of read Cache hits. |
| *_read_local_l2_miss | Number of read Cache misses. |
| *_read_local_l2_victim | Number of read Cache misses that triggered data eviction from Cache. |

### Optimization Point 1: Impact of Block Granularity on Transfer Efficiency

**Implementation**: Refer to the GM to UB `DataCopyPad` transfer in `data_copy_ub.h` and the GM to L1 `DataCopy` transfer in `data_copy_l1.h`. Different `TILE_M/TILE_N` combinations are switched through compile-time scenario parameters. The input matrix size remains unchanged; only `TILE_M/TILE_N` is modified. Smaller blocks increase the number of transfer instructions, while larger blocks reduce instruction dispatch count but require more on-chip temporary space.

First, examine the shape of a single DataCopy transfer: the UB path transfers data directly into UB following the ND layout, while the L1 path completes the ND to NZ layout conversion when transferring into L1. Both paths use different parameters, but both are determined by `TILE_M/TILE_N` for the 2D block size per transfer.

| Path | Parameter Field | Value in This Example | Meaning |
| --- | --- | --- | --- |
| GM to UB | DataCopyParams.blockCount | tileM | Number of rows per transfer |
| DataCopyParams.blockLen | curCols * sizeof(half) | Continuous bytes transferred per row |  |
| DataCopyParams.srcStride | (n - curCols) * sizeof(half) | Bytes skipped between adjacent rows on the source side |  |
| DataCopyParams.dstStride | 0 | Stored continuously in UB, no extra row skipping |  |
| GM to L1 | Nd2NzParams.nValue | tileM | Number of rows in the ND source matrix for this transfer |
| Nd2NzParams.dValue | curCols or tileN | Number of columns in the ND source matrix for this transfer |  |
| Nd2NzParams.srcDValue | n | Row width of the original matrix in GM |  |
| Nd2NzParams.dstNzC0Stride | AlignUp(tileM, 16) | C0 direction stride of the NZ layout in L1 |  |

In this example, the single DataCopy source data volume is calculated as `tileM * curCols * sizeof(half)`:

| Path | Scenario | Tile | Single Transfer Volume |
| --- | --- | --- | --- |
| GM to UB | Scenario 1 | [1,64] | 128B |
| Scenario 2 | [64,64] | 8192B |  |
| Scenario 3 | [64,1024] | 131072B |  |
| GM to L1 | Scenario 1 | [64,64] | 8192B |
| Scenario 2 | [64,256] | 32768B |  |

As blocks become larger, each instruction transfers more data, and the loop count and instruction dispatch count decrease accordingly.

**Core Distribution and Data Loading Pattern**:

```text
Input matrix GM: [M, N]

M direction split by core:
┌────────────── M ──────────────┐
│ core0: singleCoreM rows       │
│ core1: singleCoreM rows       │
│ ...                           │  NUM_BLOCKS cores in parallel
│ last core: singleCoreM rows   │
└───────────────────────────────┘

Single core transfers by tile:
singleCoreM rows
┌──── tileN ────┬──── tileN ────┬──── tileN ────┐
│ tileM rows    │ tileM rows    │ tileM rows    │
├───────────────┼───────────────┼───────────────┤
│ tileM rows    │ tileM rows    │ tileM rows    │
└───────────────┴───────────────┴───────────────┘

GM -> UB: DataCopyPad, executed by AIV cores
GM -> L1: DataCopy(ND2NZ), executed by AIC cores
```

**MTE2 Bandwidth Theoretical Analysis**:

The baseline scenarios in this group only perform a single GM read, without considering L2Cache reuse benefits. The input matrix is `M=12288, N=12288`, data type is half, and the total read data volume is:

$$Total Read Data = M \times N \times sizeof(half) = 12288 \times 12288 \times 2B = 301989888B \approx 301.99MB$$

Rough estimation of MTE2 theoretical duration based on GM peak bandwidth:

$$MTE2 Theoretical Duration = \frac{301.99MB}{GM Peak Bandwidth}$$

For Atlas A2/A3 Series, with GM bandwidth approximately 1.8TB/s, the theoretical duration is approximately 167.77μs. In the large block scenario, GM to UB `aiv_mte2_time` is 202.657μs, and GM to L1 `aic_mte2_time` is 214.519μs, which are approximately 20.8% and 27.9% higher than theoretical values respectively.

For Ascend 950 Series, with GM bandwidth approximately 1.6TB/s, the theoretical duration is approximately 188.74μs. In the large block scenario, GM to UB `aiv_mte2_time` is 185.66μs, and GM to L1 `aic_mte2_time` is 187.19μs, which are close to theoretical estimates. This estimation is only used to judge transfer efficiency magnitude. Actual performance is affected by instruction dispatch, address continuity, DataCopyPad/ND2NZ processing, and other factors.

**GM to UB Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | MTE2 Performance Improvement vs Baseline | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 1 | Tensor=[12288,12288]Tile=[1,64]DataCopyParams={blockCount=1, blockLen=128B, srcStride=24448B}Block Num=48 | 564.84 | 49222116 | 548.161 | 0.989 | Baseline | Small block transfer |
| 2 | Tensor=[12288,12288]Tile=[64,64]DataCopyParams={blockCount=64, blockLen=128B, srcStride=24448B}Block Num=48 | 233.54 | 20167322 | 220.772 | 0.972 | +148.3% | Medium block transfer |  |
| 3 | Tensor=[12288,12288]Tile=[64,1024]DataCopyParams={blockCount=64, blockLen=2048B, srcStride=22528B}Block Num=48 | 215.82 | 18563430 | 202.657 | 0.969 | +170.5% | Large block transfer |  |
| Ascend 950 Series | 1 | Tensor=[12288,12288]Tile=[1,64]DataCopyParams={blockCount=1, blockLen=128B, srcStride=24448B}Block Num=64 | 884.06 | 91585425 | 881.13 | 1 | Baseline | Small block transfer |
| 2 | Tensor=[12288,12288]Tile=[64,64]DataCopyParams={blockCount=64, blockLen=128B, srcStride=24448B}Block Num=64 | 208.64 | 31489587 | 205.99 | 0.99 | +327.8% | Medium block transfer |  |
| 3 | Tensor=[12288,12288]Tile=[64,1024]DataCopyParams={blockCount=64, blockLen=2048B, srcStride=22528B}Block Num=64 | 188.52 | 19710997 | 185.66 | 0.99 | +374.6% | Large block transfer |  |

**GM to L1 Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | MTE2 Performance Improvement vs Baseline | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 1 | Tensor=[12288,12288]Tile=[64,64]Nd2NzParams={nValue=64, dValue=64, srcDValue=12288}Block Num=24 | 300.7 | 12870910 | 283.508 | 0.978 | Baseline | Medium block transfer |
| 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=24 | 230.24 | 9795512 | 214.519 | 0.972 | +32.2% | Large block transfer |  |
| Ascend 950 Series | 1 | Tensor=[12288,12288]Tile=[64,64]Nd2NzParams={nValue=64, dValue=64, srcDValue=12288}Block Num=32 | 245.1 | 12480180 | 241.56 | 0.99 | Baseline | Medium block transfer |
| 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=32 | 190.52 | 9853588 | 187.19 | 0.99 | +29.0% | Large block transfer |  |

**Optimization Effect Analysis**:
- Increasing block size significantly reduces transfer instruction dispatch overhead. In GM to UB scenarios, large blocks vs. small blocks improve end-to-end performance by approximately 161.7% on Atlas A2/A3 Series and approximately 369.0% on Ascend 950 Series.
- In GM to L1 scenarios, after increasing `TILE_N`, end-to-end performance improves by approximately 29% or more on both Atlas A2/A3 Series and Ascend 950 Series, indicating that MTE2 transfer efficiency is higher after reducing overhead from excessively small transfer granularity.
- When configuring in practice, it is recommended to prioritize increasing the single transfer size within the limits of on-chip space, while keeping blocks within UB/L1 available cache. The larger the single transfer byte count and the fewer `mLoopCount/nLoopCount`, the lower the DataCopy instruction count and loop control overhead.

### Optimization Point 2: Impact of Unaligned Data Transfer

**Implementation**: The unaligned data scenario reuses the same transfer flow as the aligned scenario, only changing the matrix `N` dimension so that the last transfer block cannot cover the full column width.

When unaligned, the last N-direction tile has `curCols` less than `TILE_N`. In the UB path, `DataCopyParams.blockLen` changes from the full `TILE_N * sizeof(half)` to `curCols * sizeof(half)`; in the L1 path, `Nd2NzParams.dValue` changes from the full `TILE_N` to `curCols`.

In the unaligned scenario of this example, the full tile transfer volume for GM to UB is `64 * 1024 * 2B = 131072B`, and the tail block is `64 * 1023 * 2B = 130944B`; the full tile transfer volume for GM to L1 is `64 * 256 * 2B = 32768B`, and the tail block is `64 * 255 * 2B = 32640B`. The unaligned scenario does not significantly increase total data volume; rather, the tail block requires additional boundary processing, causing transfer efficiency degradation.

**Comparison Method**: Keep block size constant, change `N` from `12288` to `12287`, and observe the impact of unaligned data processing on end-to-end duration and MTE2 duration.

**GM to UB Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | Performance Change vs Aligned Scenario | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 3 | Tensor=[12288,12288]Tile=[64,1024]blockLen=2048BsrcStride=22528BBlock Num=48 | 215.82 | 18563430 | 202.657 | 0.969 | Baseline | Aligned large block |
| 4 | Tensor=[12288,12287]Tile=[64,1024]full/tail curCols=1024,1023blockLen=2048B,2046BsrcStride=22526B,22528BBlock Num=48 | 275.3 | 23648813 | 259.047 | 0.973 | -21.6% | Unaligned data large block |  |
| Ascend 950 Series | 3 | Tensor=[12288,12288]Tile=[64,1024]blockLen=2048BsrcStride=22528BBlock Num=64 | 188.52 | 19710997 | 185.66 | 0.99 | Baseline | Aligned large block |
| 4 | Tensor=[12288,12287]Tile=[64,1024]full/tail curCols=1024,1023blockLen=2048B,2046BsrcStride=22526B,22528BBlock Num=64 | 192.07 | 19739629 | 189.16 | 0.99 | -1.8% | Unaligned data large block |  |

**GM to L1 Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | Performance Change vs Aligned Scenario | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=24 | 230.24 | 9795512 | 214.519 | 0.972 | Baseline | Aligned large block |
| 3 | Tensor=[12288,12287]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256(full),255(tail), srcDValue=12287}Block Num=24 | 438.76 | 19020328 | 422.515 | 0.986 | -47.5% | Unaligned data large block |  |
| Ascend 950 Series | 2 | Tensor=[12288,12288]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256, srcDValue=12288}Block Num=32 | 190.52 | 9853588 | 187.19 | 0.99 | Baseline | Aligned large block |
| 3 | Tensor=[12288,12287]Tile=[64,256]Nd2NzParams={nValue=64, dValue=256(full),255(tail), srcDValue=12287}Block Num=32 | 206.61 | 10589826 | 202.97 | 0.99 | -7.8% | Unaligned data large block |  |

**Optimization Effect Analysis**:
- Unaligned data introduces additional boundary processing, with more significant impact on Atlas A2/A3 Series, especially in the GM to L1 scenario where end-to-end performance degrades by approximately 47.5%.
- On Ascend 950 Series, the impact of unaligned data is relatively smaller but still introduces additional overhead. When selecting blocks, prioritize alignment of the primary transfer dimension.
- It is recommended to use aligned matrices when designing matrix shapes and splitting strategies. Based on the half data type in this example, Atlas A2/A3 Series recommend that the continuous byte count corresponding to the primary transfer dimension satisfies 512B alignment; Ascend 950 Series recommend 128B alignment.

### Optimization Point 3: Repeated Transfer and L2Cache Reuse

**Implementation**: The repeated transfer scenario performs multiple rounds of GM reads on the same data scale, using `msopprof --ai-core=on --aic-metrics=L2Cache` to collect L2Cache read hit and miss allocate data.

For full-block repeated transfer, `RepeatCopy(0, n, 4)` means `nStart=0`, `nCount=N`, `repeatTimes=4`, with each round reading the same data block along the full N direction. A2/A3 has an L2Cache size of 192MB, and 950PR has an L2Cache size of 128MB. In this example, a single `M * N` matrix is approximately 301.99MB. During full-block repeated transfer, the single-round working set exceeds L2Cache capacity, making write eviction likely and resulting in low L2Cache hit rates.

For sliced repeated transfer, first set `quarterN=N/4`, then for each `splitIdx` call `RepeatCopy(splitIdx * quarterN, quarterN, 4)`, meaning each time only 4 consecutive repetitions occur within 1/4 of the N-direction slice. Both approaches transfer the same total data volume, but after slicing, each slice is approximately 75.50MB with a smaller single-round working set, making it more likely to remain in L2Cache during consecutive repeated accesses.

**Comparison Method**: Compare transferring the full matrix 4 times consecutively along the same path vs. slicing the N direction into 4 parts and transferring each part 4 times consecutively. The latter repeats access within each slice consecutively, making it easier to observe L2Cache reuse benefits.

Scenario 5/6 (GM to UB) and Scenario 4/5 (GM to L1) both use `Tile=[64,1024]`. The source data volume for a single `DataCopyPad` or `DataCopy (Nd2NzParams)` transfer is:

$$64 \times 1024 \times sizeof(half) = 64 \times 1024 \times 2B = 131072B = 0.131072MB$$

During full-block repeated transfer, the per-round working set is `301.99MB`, with total read of `1207.96MB` over 4 consecutive rounds; after slicing the N direction into 4 parts, each slice working set is `75.50MB`, with total read data volume still `1207.96MB`, but each slice is more easily retained and reused by L2Cache.

**L2Cache Reuse Pattern**:

```text
Scenario 5(UB) / Scenario 4(L1): Full matrix consecutive repeated transfer

GM matrix: [M, N]
┌─────────────────────────────── N ───────────────────────────────┐
│                    All columns transferred at once                │
└──────────────────────────────────────────────────────────────────┘

Start all cores, transfer the full matrix along the same path:
Round 1: All cores read the full matrix from GM -> UB or L1
Round 2: All cores read the full matrix again -> UB or L1
Round 3: All cores read the full matrix again -> UB or L1
Round 4: All cores read the full matrix again -> UB or L1
Note: Each round working set is the full matrix, making it difficult for L2Cache to fully retain the previous round data

Scenario 6(UB) / Scenario 5(L1): N direction sliced into 4 parts, each part transferred consecutively

GM matrix: [M, N]
┌──────── N/4 ────────┬──────── N/4 ────────┬──────── N/4 ────────┬──────── N/4 ────────┐
│      Slice 0        │      Slice 1        │      Slice 2        │      Slice 3         │
└─────────────────────┴─────────────────────┴─────────────────────┴─────────────────────┘

Start all cores, transfer Slice 0 data 4 consecutive rounds, then process the next slice:
Slice 0: Round 1 reads from GM, Rounds 2-4 preferentially read from L2Cache
Slice 1: Round 1 reads from GM, Rounds 2-4 preferentially read from L2Cache
Slice 2: Round 1 reads from GM, Rounds 2-4 preferentially read from L2Cache
Slice 3: Round 1 reads from GM, Rounds 2-4 preferentially read from L2Cache
Note: Single slice working set is smaller, making it easier to retain in L2Cache during consecutive repeated access
```

**L2Cache Theoretical Performance Analysis**:

The input matrix for this group of scenarios is `M=12288, N=12288`, data type is half, and the single full read data volume is:

$$Single Read Data = M \times N \times sizeof(half) = 12288 \times 12288 \times 2B = 301989888B \approx 301.99MB$$

When the full matrix is transferred 4 times consecutively, the total read data volume is:

$$Full Block Repeated Read Data = 301989888B \times 4 = 1207959552B \approx 1207.96MB$$

This access pattern has a single working set of 301.99MB, which is difficult to fully retain in L2Cache, so it can be approximately estimated as primarily reading from GM:

$$Full Block Repeated Theoretical Duration = \frac{1207.96MB}{GM Bandwidth}$$

After slicing the N direction into 4 parts, the data volume per slice is:

$$Slice Data Volume = 301989888B \div 4 = 75497472B \approx 75.50MB$$

When each slice is transferred 4 times consecutively, ideally the first time reads from GM and the subsequent 3 times read from L2Cache:

$$GM Read Data Volume = 75497472B \times 4 = 301989888B \approx 301.99MB$$

$$L2Cache Read Data Volume = 75497472B \times 3 \times 4 = 905969664B \approx 905.97MB$$

$$Slice Repeated Theoretical Duration = \frac{301.99MB}{GM Bandwidth} + \frac{905.97MB}{L2Cache Bandwidth}$$

Atlas A2/A3 Series estimates use GM bandwidth approximately 1.8TB/s and L2Cache peak bandwidth approximately 5.2TB/s; Ascend 950 Series estimates use GM bandwidth approximately 1.6TB/s and L2Cache peak bandwidth approximately 5.2TB/s.

$$Atlas A2/A3 Series Full Block Repeated Theoretical Duration = \frac{1207.96MB}{1.8TB/s} = 671.09\mu s$$

$$Ascend 950 Series Full Block Repeated Theoretical Duration = \frac{1207.96MB}{1.6TB/s} = 754.97\mu s$$

$$Atlas A2/A3 Series Slice Repeated Theoretical Duration = \frac{301.99MB}{1.8TB/s} + \frac{905.97MB}{5.2TB/s} = 342.00\mu s$$

$$Ascend 950 Series Slice Repeated Theoretical Duration = \frac{301.99MB}{1.6TB/s} + \frac{905.97MB}{5.2TB/s} = 362.97\mu s$$

From both theoretical models and measured results, the duration of N-direction sliced repeated transfer is closer to the ideal model of "first GM + subsequent L2Cache"; full-block repeated transfer has insufficient L2Cache reuse due to larger working sets, with duration closer to multiple GM reads. The GM to L1 scenario includes ND2NZ transfer, and actual duration is also affected by format conversion and L1 write layout, so it is typically higher than pure GM to UB transfer.

**GM to UB L2Cache Performance Data**:

Atlas A2/A3 Series and Ascend 950 Series have different profiler output fields and different hit rate calculation methods: Atlas A2/A3 Series uses `l2cache_hit_ratio = (r0_hit + r1_hit) / (r0_hit + r1_hit + r0_miss_allocate + r1_miss_allocate)`; Ascend 950 Series uses `l2cache_hit_ratio = hit / (hit + miss + victim)`.

| Architecture | Scenario | Configuration | Task Duration(μs) | aiv_total_cycles | aiv_time(μs) | aiv_r0_read_cache_hit | aiv_r0_read_cache_miss_allocate | aiv_r1_read_cache_hit | aiv_r1_read_cache_miss_allocate | Description | L2Cache Hit Rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=48 | 828.06 | 72465129 | 816.05 | 212 | 4718595 | 217 | 4718592 | Full matrix transferred 4 times consecutively along the same path | 0.005% |
| 6 | Tensor=[12288,12288]Tile=[64,1024]Block Num=48 | 365.74 | 31484525 | 354.56 | 3539159 | 1179644 | 3539158 | 1179655 | N direction sliced into 4 parts, each part transferred 4 times consecutively | 75.00% |  |

| Architecture | Scenario | Configuration | Task Duration(μs) | aiv_total_cycles | aiv_time(μs) | aiv_read_local_l2_hit | aiv_read_local_l2_miss | aiv_read_local_l2_victim | Description | L2Cache Hit Rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Ascend 950 Series | 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=64 | 741.58 | 77700635 | 740.75 | 31346 | 529720 | 8358412 | Full matrix transferred 4 times consecutively along the same path | 0.35% |
| 6 | Tensor=[12288,12288]Tile=[64,1024]Block Num=64 | 354.95 | 36964347 | 354.19 | 5943026 | 528797 | 2446732 | N direction sliced into 4 parts, each part transferred 4 times consecutively | 66.64% |  |

**GM to L1 L2Cache Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aic_total_cycles | aicore_time(μs) | aic_r0_read_cache_hit | aic_r0_read_cache_miss_allocate | aic_r1_read_cache_hit | aic_r1_read_cache_miss_allocate | Description | L2Cache Hit Rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 4 | Tensor=[12288,12288]Tile=[64,1024]Block Num=24 | 899.34 | 38725327 | 872.19 | 305 | 4718591 | 287 | 4718601 | Full matrix ND2NZ transferred 4 times consecutively along the same path | 0.006% |
| 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=24 | 414.96 | 17662155 | 397.8 | 3539061 | 1179684 | 3539133 | 1179616 | N direction sliced into 4 parts, each part ND2NZ transferred 4 times consecutively | 75.00% |  |

| Architecture | Scenario | Configuration | Task Duration(μs) | aic_total_cycles | aicore_time(μs) | aic_read_local_l2_hit | aic_read_local_l2_miss | aic_read_local_l2_victim | Description | L2Cache Hit Rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Ascend 950 Series | 4 | Tensor=[12288,12288]Tile=[64,1024]Block Num=32 | 732.809 | 37950511 | 732.11 | 269472 | 2557819 | 4060060 | Full matrix ND2NZ transferred 4 times consecutively along the same path | 3.91% |
| 5 | Tensor=[12288,12288]Tile=[64,1024]Block Num=32 | 390.347 | 19858177 | 389.64 | 5948502 | 1148258 | 1220548 | N direction sliced into 4 parts, each part ND2NZ transferred 4 times consecutively | 71.52% |  |

**Optimization Effect Analysis**:
- After N-direction slicing, repeated access within slices provides more sufficient L2Cache reuse, with significant end-to-end performance improvement on both Atlas A2/A3 Series and Ascend 950 Series for both GM to UB and GM to L1.
- Atlas A2/A3 Series requires accumulating observations across both r0+r1 hardware read channels, with total read Cache hits significantly increasing and miss allocate significantly decreasing; Ascend 950 Series shows corresponding behavior with `*_read_local_l2_hit` increasing and miss/victim decreasing.
- When the same batch of GM data needs to be read multiple times, prioritize sliced transfer and complete multiple accesses consecutively within each slice, keeping the single working set within the L2Cache reusable range.

### Optimization Point 4: Multi-Core Same-Address Access Conflict Avoidance

**Implementation**: In the same addr scenario, all cores access the input matrix in the same `mBlockIdx` order; in the offset addr scenario, each core staggers the access order by `(mBlockIdx + blockIdx) % numBlocks`.

This optimization point does not change the single DataCopy transfer shape, only changes the order in which different cores access GM slices. Each core fully loads the input matrix once, resulting in the same matrix being read `numBlocks` times overall. In the same addr pattern, all cores synchronously access the same address range on the same `mBlockIdx`; in the offset addr pattern, `curMBlockIdx` rotates by `blockIdx` within each group of `numBlocks` M blocks, reducing the probability of multiple cores accessing the same GM address range at the same time.

**Key Code**:

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

In this example, each DataCopy transfers in the N direction at `tileN` granularity; when `offsetAddr=true`, the order of access slices per core is staggered so that different cores access different GM address ranges at the same time.

**Comparison Method**: All cores fully load the same input matrix, comparing same-order access vs. core-staggered access order. Staggered access adjusts the parallel slice access order to reduce the probability of multiple cores accessing the same address at the same time.

**GM to UB Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aiv_total_cycles | aiv_mte2_time(μs) | aiv_mte2_ratio | Description |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 7 | Tensor=[6144,512]Tile=[128,64]Block Num=48 | 539.42 | 42823246 | 474.717 | 0.984 | All cores load fully in the same order |
| 8 | Tensor=[6144,512]Tile=[128,64]Block Num=48 | 328.88 | 28054500 | 307.38 | 0.973 | All cores load fully with staggered slice order |  |
| Ascend 950 Series | 7 | Tensor=[8192,512]Tile=[128,64]Block Num=64 | 342.29 | 33624098 | 339.38 | 0.99 | All cores load fully in the same order |
| 8 | Tensor=[8192,512]Tile=[128,64]Block Num=64 | 335.64 | 35298771 | 333.08 | 0.99 | All cores load fully with staggered slice order |  |

**GM to L1 Performance Data**:

| Architecture | Scenario | Configuration | Task Duration(μs) | aic_total_cycles | aic_mte2_time(μs) | aic_mte2_ratio | Description |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Atlas A2/A3 Series | 6 | Tensor=[6144,512]Tile=[256,64]Block Num=24 | 278.56 | 10880150 | 240.198 | 0.98 | All cores load fully in the same order |
| 7 | Tensor=[6144,512]Tile=[256,64]Block Num=24 | 221.34 | 9532153 | 209.8 | 0.977 | All cores load fully with staggered slice order |  |
| Ascend 950 Series | 6 | Tensor=[8192,512]Tile=[256,64]Block Num=32 | 369.99 | 13312673 | 366.24 | 0.99 | All cores load fully in the same order |
| 7 | Tensor=[8192,512]Tile=[256,64]Block Num=32 | 187.35 | 9713703 | 185.03 | 0.99 | All cores load fully with staggered slice order |  |

**Optimization Effect Analysis**:
- Offset addr reduces the probability of multiple cores accessing the same GM address range at the same time by staggering the multi-core access order. Atlas A2/A3 Series UB/L1 scenarios and Ascend 950 Series L1 scenarios show more obvious benefits.
- From an end-to-end performance perspective, Atlas A2/A3 Series UB scenario improves by approximately 64.0%, Ascend 950 Series L1 scenario improves by approximately 97.5%; Ascend 950 Series UB scenario shows smaller benefits.

### Optimization Summary

| Optimization Method | Core Principle | Usage Recommendation |
|:---|:---|:---|
| Increase transfer block size | Reduce DataCopy instruction count and loop control overhead, improve MTE2 effective transfer efficiency | When on-chip space allows, prioritize larger `TILE_M/TILE_N` |
| Maintain primary dimension alignment | Avoid boundary processing and incomplete transfer overhead from unaligned data | When designing shapes or splitting strategies, try to make the primary transfer dimension divisible by `TILE_N`, and ensure continuous transfer byte count satisfies Atlas A2/A3 Series 512B, Ascend 950 Series 128B alignment |
| Sliced repeated access | Restrict repeated access to a smaller data range to improve L2Cache hit probability | When the same batch of GM data needs to be read multiple times, prioritize slicing first then repeating within each slice |
| Stagger multi-core access order | Reduce the probability of multiple cores accessing the same GM address range at the same time | When multiple cores read the same large data block, rotate access slice order by `blockIdx` |

## Build and Run

Run the following steps in the root directory of this example to build and run the example.

- Configure environment variables

  Configure environment variables based on the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit in the current environment.

  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example

  Run the following commands in this example directory.
  ```bash
  SCENARIO_NUM=1 ASC_ARCH=dav-2201
  COPY_DST=UB
  mkdir -p build && cd build
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCOPY_DST=$COPY_DST -DCMAKE_ASC_ARCHITECTURES=$ASC_ARCH ..;make -j;
  python3 ../scripts/gen_data.py -scenarioNum $SCENARIO_NUM -copyDst $COPY_DST -arch $ASC_ARCH
  ./demo
  ```

  To use NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Example:

  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCOPY_DST=$COPY_DST -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..
  make -j
  ```

  > **Notice:** Clear the cmake cache before switching build modes. Run `rm CMakeCache.txt` in the build directory and then re-run cmake.

- Build option description

  | Parameter | Description | Values | Default |
  |------|------|--------|--------|
  | `SCENARIO_NUM` | Scenario number | `COPY_DST=UB`: 1-8; `COPY_DST=L1`: 1-7 | 1 |
  | `COPY_DST` | Transfer destination | `UB`, `L1` | `UB` |
  | `CMAKE_ASC_RUN_MODE` | Run mode | `npu`, `sim` | `npu` |
  | `CMAKE_ASC_ARCHITECTURES` | NPU hardware architecture | `dav-2201`, `dav-3510` | `dav-2201` |

- Performance collection

  Use the `msOpProf` tool to obtain detailed performance data:

  ```bash
  msopprof ./demo
  msopprof --ai-core=on --aic-metrics=L2Cache ./demo    # Use for L2Cache related scenarios
  ```

    - Performance data description
      After the command completes, a folder named "OPPROF_{timestamp}_XXX" will be generated in the default directory. The performance data folder structure is as follows:

      ```bash
      ├──dump                       # Raw performance data, no user attention needed
      ├──ArithmeticUtilization.csv  # Cube/Vector instruction cycle ratio
      ├──L2Cache.csv                # L2 Cache hit rate, affects MTE2, suggests reasonable data transfer logic to increase hit rate
      ├──Memory.csv                 # UB, L1 and main memory read/write bandwidth rate
      ├──MemoryL0.csv               # L0A, L0B, and L0C read/write bandwidth rate
      ├──MemoryUB.csv               # Vector and Scalar to UB read/write bandwidth rate
      ├──OpBasicInfo.csv            # Operator basic information
      ├──PipeUtilization.csv        # Computation unit and transfer unit time and ratio
      ├──ResourceConflictRatio.csv  # Bank group, bank conflict and resource conflict ratio on UB in all instructions
      └──visualize_data.bin         # MindStudio Insight presentation file
      ```

  View the specific performance analysis results:

  ```bash
  # View Task Duration and various data
  cat ./OPPROF_*/PipeUtilization.csv
  ```
