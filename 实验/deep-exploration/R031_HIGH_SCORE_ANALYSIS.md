# R031 HIGH SCORE ANALYSIS

## 范围与结论

本报告只分析 `R031-V001-FULL-MULTIMODE-REWRITE` 的源码快照、结果 JSON、题面整理和必要的 Ascend C API 资料。源码快照为
`实验/online/upstream-2026-09-18/sources/R031-V001-FULL-MULTIMODE-REWRITE_kernel.asc`，结果为
`实验/online/upstream-2026-09-18/results/R031-V001_RESULT.json`。源码 SHA-256 为
`e22188efbb916803b34a427cb353a29fc83494a7b22dec249981ae1acd0155ba`。

R031 的高分来自一个运行时多模式分派器：先把所有前导维度展平成行，再按 dtype、D、每个 Vector Core 的行数和 D 是否超过 8192 选择路径。窄行路径重点省参数重复搬运和行级 V/S 往返；宽行路径重点省 GM 重读、复用 gamma/bias、分批行处理和 MTE2/V/MTE3 双缓冲。源码与结果能证明这些机制存在，不能单独证明每个模式对 38.36 的增益，因为结果 JSON 没有公开 15 个 testcase 的 shape、rank、dtype、epsilon 或 availableCoreNum。

R031 结果字段确认：`status=Pass`、`correctness=15/15`、`official_score=38.36`，15 项 `case_times_us` 和 15 项 `tbest_times_us_latest` 均存在；`candidate_total_us=27332.17`，`fixed_baseline_improvement_percent=80.33432589255682`。Official Score 只作为平台返回事实使用，不把总耗时换算为分数。

## A. 架构总览

### A.1 Host 入口与数据形状

入口是 Direct Invocation 形式的 `extern "C" run_kernel`，源码在 2884–2957 行。入口先验证地址、TensorGroupInfo、shape、dtype 和 gamma/bias 的一维形状，再把 `x` 的前导维度乘成 `rowCount`，把最后一维保存为 `rowWidth`；对应题面语义是沿最后一维 D 做 RMS 归约，前导维度展平成 outer 行。入口的展开与溢出判断见 2916–2935 行，题面支持的 2D/3D/4D 与 outer 语义见 `文档/problem-add-rms-norm-bias.md:44-48`。

`availableCoreNum` 先转成正数，随后限制到 `rowCount`，再限制到 `UINT32_MAX`，最终作为 `blockCount` 启动 Vector Kernel，见 2937–2946 行。dtype 0、1、2 分别实例化 `float`、`half`、`bfloat16_t`，见 2948–2957 行；其它 dtype 没有启动分支。

### A.2 Device 组织

每个 block 对应一个 Vector Core，按 `baseRows=rowCount/blockCount` 和 `extraRows=rowCount%blockCount` 均匀获得连续行区间，窄行与宽行函数都使用同一分配公式。窄行分配见 181–186 行，宽行 FP32 分配见 1655–1672 行，宽行低精度分配见 2689–2697 行。源码注释明确每行只归属一个 Vector Core，避免跨核 RMS 工作区和跨核同步，见 31–38 行。

### A.3 数值数据流

所有路径都遵循：

```text
x/residual 搬入
  -> y = x + residual
  -> y^2 的 FP32 ReduceSum
  -> squareSum / D + epsilon
  -> 向量 Sqrt，再取 reciprocal
  -> y * invRms * gamma + bias
  -> 目标 dtype 输出
```

窄行通用路径的第一遍、行级归约和第二遍位于 299–371 行；宽行 FP32 的归约与输出分成 1688–1749 行的两遍；宽行低精度的 invRms 计算和输出分别由 2146–2312、2669–2768 行覆盖。`Load`/`Store` 都以有效元素数乘 `sizeof(T)` 形成 `DataCopyExtParams.blockLen`，再用 `DataCopyPad`，见 2771–2787 行。低精度输入向 FP32 使用 `CAST_NONE`，最终回到目标 dtype 使用 `CAST_ROUND`，见 2789–2809 行。

### A.4 两个总分支

- **Narrow**：`D <= 8192`。使用 4096 元素基础 tile；FP32/BF16 还配置 FP32 中间缓冲，能在 D 适合时把 y 留在 UB，避免输出遍重新读 x/residual，见 126–165 行。
- **Wide**：`D > 8192`。初始化阶段直接选择 FP32 宽行子路径或低精度宽行子路径，见 58–123 行。宽行路径把输入、参数、输出拆成更小的 tile，使用行批处理、参数复用或输入/输出事件流水。

## B. Mode 表

下表使用报告内的 mode 名称。`localRows` 是某个 block 实际获得的行数；`avgRows` 是整数除法 `rowCount/blockCount`。分派顺序很重要：前面的精确 D 模式会先于后面的通用模式命中。

| Mode | dtype | 条件 | 入口函数 | 主要目标 |
|---|---|---|---|---|
| N-F32-ROW8192 | FP32 | `D=8192 && localRows>1` | `ProcessFp32FullRowOutputPipelined` | 8192 行缓存、参数复用、输出双缓冲 |
| N-F32-CONTIG | FP32 | `D<=2048 && D%8=0 && localRows>1` | `ProcessSmallFp32ContiguousBatched` | 连续多行一次搬入/搬出 |
| N-F32-BATCH | FP32 | `D<=4096 && D%8=0 && localRows>1`，且未命中 N-F32-CONTIG | `ProcessSmallFp32Batched` | 多行共用参数和 V/S 批处理 |
| N-F16-ROW4096 | FP16 | `D=4096 && localRows>1` | `ProcessFp16FullTileBatchedOutputPipelined` | 两行交替输出、FP16 参数常驻 |
| N-F16-ROW8192 | FP16 | `D=8192 && localRows>1` | `ProcessFp16FullRowOutputPipelined` | 8192 元素 y 缓存和输出流水 |
| N-BF16-ROW4096 | BF16 | `D=4096 && localRows>1` | `ProcessBf16FullTileBatchedOutputPipelined` | FP32 y/归约、BF16 参数先提升 |
| N-BF16-ROW8192 | BF16 | `D=8192 && localRows>1` | `ProcessBf16FullRowOutputPipelined` | FP32 参数缓存和两 tile 输出 |
| N-LP-CONTIG | FP16/BF16 | `D<=2048 && D%16=0 && localRows>1`，且未命中精确 4096/8192 | `ProcessSmallLowPrecisionContiguousBatched` | 16-bit 连续行批处理 |
| N-GENERIC-PARAM | 任一支持 dtype | `D<=8192`，上述模式均未命中，`localRows>1` | `Process` 通用段 | 预载 gamma/bias，逐行两遍扫描 |
| N-GENERIC-SINGLE | 任一支持 dtype | `D<=8192`，上述模式均未命中，`localRows=1` | `Process` 通用段 | 不做跨行参数驻留，保留 y 行缓存 |
| W-F32-BATCH | FP32 | `D>8192 && avgRows>=2` | `ProcessWideFp32Batched` | 8 行批、6112 FP32 tile、参数/输入/输出双缓冲 |
| W-F32-CACHED | FP32 | `D>8192 && avgRows<2 && D<=32768` | `ProcessWideFp32CachedRows` | 7680 tile，整行 y 留在 UB |
| W-F32-STREAM | FP32 | `D>8192 && avgRows<2 && D>32768` | `ProcessWideFp32` | 12288 tile 流式处理；按题面 D 上限在正式输入中不可达 |
| W-F16-CACHED | FP16 | `D>8192 && rowCount<=blockCount` | `ProcessWideFp16CachedRows` | 每核一行，原始 y 暂存后再输出 |
| W-BF16-CACHED | BF16 | `D>8192 && rowCount<=blockCount` | `ProcessWideBf16CachedRows` | 每核一行，先存紧凑 half 暂存再回 FP32 |
| W-F16-BATCH | FP16 | `D>8192 && rowCount>blockCount && avgRows>=2` | `ProcessWideFp16BatchedOutputPipelined` | 8 行批、4096 输出 tile、输入/输出事件流水 |
| W-LP-GENERIC | FP16/BF16 | 其它 `D>8192` 低精度情形 | `ProcessWideLowPrecision` 通用段 | 8 行批，按 8192 tile 复用 gamma/bias |

## C. Dispatch 条件

### C.1 dtype、D、row count、rank

| 条件维度 | 源码规则 | 影响 |
|---|---|---|
| dtype | 0→FP32，1→FP16，2→BF16；入口 2948–2957 行 | 决定算子模板、是否可用原生 16-bit Add/Mul，以及参数提升路径 |
| D | `D<=8192` 走 Narrow，`D>8192` 走 Wide，初始化 58–123 行 | 决定是否把整行或大 tile 放入 UB |
| row count | `rowCount` 是所有前导维度乘积，2916–2929 行 | 决定 block 数量和每个 block 的 `localRows` |
| rank | 源码只要求 `numDims>=1`，没有 2D/3D/4D 专用分支，2903–2907 行 | rank 本身不选 mode；只有展平后的 `rowCount` 参与分派。题面正式范围仍是 2D/3D/4D |
| availableCoreNum | `blockCount=min(max(availableCoreNum,1),rowCount)`，2937–2945 行 | 可改变 `localRows`，从而改变批处理、参数缓存和宽行流水模式 |

### C.2 alignment、row bytes、wide/narrow

`rowBytes = D * sizeof(T)` 不是单独的分派变量。显式 alignment 条件只有：FP32 小行 `D%8=0`，低精度小行 `D%16=0`，以及 4096/8192 元素精确 tile；这些条件分别见 196–207、212–240 行。它们都使行字节数为 32B 的整数倍。其它 D 仍使用 `DataCopyPad`，按有效字节数搬运，尾块由 `TileLength`、`WideTileLength` 等函数裁剪，见 1140–1177、2771–2787 行。

因此：

- **aligned fast mode**：FP32 的 8 元素对齐、FP16/BF16 的 16 元素对齐、4096/8192 精确 tile；可把多行视为连续块，减少 DMA 次数。
- **narrow generic mode**：D 不满足上述条件，仍在 `D<=8192` 范围内逐 tile 搬运，尾块使用实际有效 count。
- **wide mode**：D 大于 8192，不再以 rowBytes 是否 32B 对齐选择模式；使用 8192、7680、6112、4096 或 12288 元素 tile，末 tile 由 valid count 处理。

### C.3 row scheduling

所有 block 都按连续行区间工作，`beginRow` 和 `localRows` 的计算保持行不重叠。窄行特殊模式要求 `localRows>1`，宽行 FP32/FP16 批流水主要要求 `avgRows>=2`；低精度宽行 cached 模式使用 `rowCount<=blockCount`，在入口已把 blockCount 限制到不超过 rowCount 的前提下，实际含义接近每核一行。这个规则不等同于“每次一定有相同的 localRows”：余数行会让前若干 block 多一行。

## D. 15 testcase 到 mode 的映射

### D.1 证据边界

平台快照只提供 15 项 timing，没有公开 shape、rank、dtype、epsilon 或 availableCoreNum；题面整理也明确记录这些 15 个 testcase 配置未开放，见 `文档/problem-add-rms-norm-bias.md:75-84`。项目实验纪律禁止从 case 编号猜隐藏 shape。因此下面的 `mode` 列采用“精确未知 + 可执行解析规则”，不是把时间大小冒充输入元数据。

### D.2 逐点 timing 与可解析 mode

| testcase | R031 `case_times_us` | `tbest_times_us_latest` | R031/TBest | timing 分组 | mode |
|---:|---:|---:|---:|---|---|
| T01 | 4.66 | 1.47 | 3.170x | Q1 | 精确未知；按 C 节 resolver |
| T02 | 3.80 | 2.16 | 1.759x | Q1 | 精确未知；按 C 节 resolver |
| T03 | 5.93 | 2.48 | 2.391x | Q1 | 精确未知；按 C 节 resolver |
| T04 | 19.20 | 6.66 | 2.883x | Q1 | 精确未知；按 C 节 resolver |
| T05 | 10.90 | 5.21 | 2.092x | Q1 | 精确未知；按 C 节 resolver |
| T06 | 32.45 | 11.45 | 2.834x | Q2 | 精确未知；按 C 节 resolver |
| T07 | 61.15 | 14.05 | 4.352x | Q2 | 精确未知；按 C 节 resolver |
| T08 | 70.18 | 30.64 | 2.290x | Q2 | 精确未知；按 C 节 resolver |
| T09 | 73.16 | 50.61 | 1.446x | Q2 | 精确未知；按 C 节 resolver |
| T10 | 75.02 | 47.35 | 1.584x | Q2 | 精确未知；按 C 节 resolver |
| T11 | 161.52 | 67.54 | 2.391x | Q3 | 精确未知；按 C 节 resolver |
| T12 | 98.67 | 76.55 | 1.289x | Q2 | 精确未知；按 C 节 resolver |
| T13 | 565.53 | 307.60 | 1.839x | Q3 | 精确未知；按 C 节 resolver |
| T14 | 16500.00 | 3750.00 | 4.400x | Q4 | 精确未知；按 C 节 resolver |
| T15 | 9650.00 | 8510.00 | 1.134x | Q4 | 精确未知；按 C 节 resolver |

可复用的精确 resolver 是：先得到 `(dtype,D,rowCount,availableCoreNum)`；计算 `blockCount`、`localRows` 和 `avgRows`；按 B 表从上到下命中第一个条件。没有这四类输入，就不能把 T01–T15 绑定到具体 mode。这个限制同时意味着不能从 T14/T15 的大耗时直接断言它们一定命中了某个 wide mode。

## E. 每种 mode 的机制

### E.1 Narrow family

| Mode | tile / UB | reduction | DMA / buffering | core mapping / row scheduling | parameter / normalization / tail |
|---|---|---|---|---|---|
| N-F32-ROW8192 | x/residual 4096 元素，y/value 8192 元素，gamma/bias 各 8192 FP32；输出不另设 FP32 buffer | 每个 4096 tile 平方后 ReduceSum，最后再对 tile sum ReduceSum；V↔S 读 squareSum 和 invRms，见 993–1110 | 预载两组 gamma/bias；y 留在 value UB；两个输出事件让 MTE3 与后续行交错 | 每 core 处理多行；每行 2 个 4096 tile，下一行首 tile 可预取 | meanSquare=`squareSum/D+epsilon`，向量 Sqrt；FP32 直接 Mul/Add；尾块只在通用窄行路径出现，精确 8192 无尾块 |
| N-F32-CONTIG | 4096 元素输入和 8192 元素 value cache；最多 8 行或受 `4096/D` 限制 | 一次批搬入后按每行起点 ReduceSum；每行一次 V/S 往返 | x/residual 与 output 对每个 row batch 各一次连续 DataCopyPad；gamma/bias 预载 | 每 core 多行，按 `batchLimit` 行分组 | D<=2048 且 D%8=0；`Mul/Add` 对小宽度使用 batch repeat；无非对齐尾块 |
| N-F32-BATCH | value cache 按 `8192/D` 容纳最多 4 行；D=4096 转入 full-tile 版本 | 每行独立 ReduceSum，批量读取多个 scalar、批量做一元素 Sqrt | x/residual 分行搬入，输出按 batch 交替事件；参数一次加载 | 每 core 多行，最多 4 行一组 | D<=4096、D%8=0；FP32 中间；对 D=4096 为两 tile/行 |
| N-F16-ROW4096 | x/residual 4096 half、value 8192 FP32、output 8192 half、gamma/bias 8192 half | FP16 Add 后提升到 FP32，平方与 ReduceSum 在 FP32 | 两个 V→MTE3/MTE3→V event ID，输出 row 交替；gamma/bias 每 core 预载 | 每 core 多行，最多 2 行一组 | `Muls` 在 FP32，随后一次 Cast 回 half，再 native Mul/Add；D=4096 整 tile |
| N-F16-ROW8192 | value 8192 FP32、output 8192 half、x/residual 4096 half，参数 8192 half | 两个 4096 tile 的 FP32 partial sum 再归约 | y 留在 value；输出两个 tile 交替 | 每 core 多行，逐行处理 | 一次量化回 FP16 后再乘 gamma 和加 bias；无尾块 |
| N-BF16-ROW4096 | 与窄行 FP32 中间工作集近似同级，另有 gammaFp32/biasFp32 | 输入先 Cast FP32，FP32 Add/Mul/ReduceSum；参数也先提升 | gamma/bias 4096 staging 后转入 FP32；输出用 gamma buffer 重用 | 每 core 多行，最多 2 行输出批 | FP32 域完成归一化、乘法和偏置；最后一次 BF16 Cast；整 tile |
| N-BF16-ROW8192 | 8192 FP32 value、两组 FP32 参数、4096 source staging；输出两 tile | 每个 4096 tile FP32 归约，再合并 | gamma/bias FP32 预载；两个输出 tile 交替 | 每 core 多行，逐行输出 | `Muls`/Mul/Add 均 FP32；最后 Cast BF16；无尾块 |
| N-LP-CONTIG | 4096 元素总输入工作区，按 `4096/D` 与最多 8 行装入；gamma/bias 一次载入 | 每行在拼接批次的对应 offset 上 ReduceSum；每批集中 V/S 与 Sqrt | 一次连续 x/residual 搬入，一次连续 output 搬出；FP16 output 或 BF16 gamma buffer 重用 | 每 core 多行，8 行上限 | D<=2048、D%16=0；FP16 先 FP32 归一化后回 half；BF16 还需 FP32 参数；无非对齐尾块 |
| N-GENERIC-PARAM | 基础 4096 tile；value FP32 cache 8192；FP32/BF16 常见总工作集约 176 KB，FP16 约 136–144 KB，均为源码配置量估算 | 每 tile 一个 partial，再做一次 tile-level ReduceSum；每行两次 V/S 读取 scalar | D<=8192 时 y 留在 value，gamma/bias 在 `localRows>1` 时整行预载；输出逐 tile | 每 core 多行，按 `baseRows/extraRows` 分配 | FP32/FP16 参数保持源 dtype；BF16 参数转 FP32；任意 D 的最后 tile 使用 valid count |
| N-GENERIC-SINGLE | 同上，`localRows=1` 时不做参数驻留 | 同上，行级 ReduceSum、向量 Sqrt、scalar reciprocal | y 仍可留在 value，gamma/bias 在第二遍按 tile 重载 | 每 core 一行；不付跨行缓存收益 | 适合非对齐 D 和单行 outer；尾块由 DataCopyPad 精确字节数处理 |

### E.2 Wide family

| Mode | tile / UB | reduction | DMA / buffering | core mapping / row scheduling | parameter / normalization / tail |
|---|---|---|---|---|---|
| W-F32-BATCH | 6112 FP32 tile；8 组工作 buffer 加 8 个 partial scalar，源码宣称适配 192 KB UB；按常量计算约 195,616 B，需目标工具链确认 | `ComputeWideFp32InvRmsPipelined` 用双输入 pair 逐 tile归约，再合并 partial | gamma/bias、输入 pair、value、输出 pair 均交替使用 event ID；8 行批在输出阶段复用参数 tile | 每 core 至少 2 行；每批最多 8 行；按 6112 tile 扫 D | FP32 全链；epsilon 加在均值后；D 尾块 valid count |
| W-F32-CACHED | x/residual 各 7680 FP32，value 保存整行 y，最多 D=32768；上界约 192,544 B（含 partial），需工具链确认 | 7680 tile partial，再做 tile-level ReduceSum | 第一遍只读 x/residual 一次；第二遍直接读 value，gamma/bias tile 另从 x/residual buffer 搬入 | 每 core 少于 2 行，通常一行；无跨行参数复用 | 省第二遍 x/residual GM 读；尾 tile 用 7680 valid |
| W-F32-STREAM | 12288 FP32 x/residual/value；约 147,488 B | 12288 tile partial，再合并 | 第一遍与第二遍都重新读 x/residual，x/residual buffer 第二遍重用为 gamma/bias | 每 core 少于 2 行；按行顺序扫描 | 只有 D>32768 才会到达；在题面 D<=32768 时是保留分支 |
| W-F16-CACHED | x/residual 7680 half、output 保存整行、value/xFp32/residualFp32 各 7680 FP32；D=32768 时约 188,448 B | native FP16 Add 后转 FP32，FP32 square/ReduceSum；第二遍从 output 暂存恢复 y | 第一遍 output buffer 保存 y；第二遍加载 gamma/bias，output buffer 原位完成归一化和输出 | 每核一行；逐 7680 tile | Muls 在 FP32，Cast 回 half 后 native Mul/Add；尾 tile valid |
| W-BF16-CACHED | 与 W-F16-CACHED 同一大小级别；output buffer 用 half 视图暂存 BF16 y | 输入和参数均转 FP32，FP32 ReduceSum；缓存 y 时先压成 half，再恢复 FP32 | 用 output buffer 保存紧凑 y，第二遍载 gamma/bias 并转 FP32 | 每核一行 | 该缓存会引入中间 half 暂存，线上 15/15 只能证明该候选通过，不能证明其数值代价为零；尾 tile valid |
| W-F16-BATCH | 4096 half 输出 tile，两个 4096 输入/输出槽；约 180,256 B 级别 | invRms 先按 8192 tile 用双输入 pair 求出，输出阶段按 4096 tile 处理 | 输入 ready/release 与输出 ready/release 各两组 event ID；每个 channel tile 复用 gamma/bias | 每 core 多行，8 行一批；行内先求 invRms，再按 tile 输出 | FP16 Add 后升 FP32，归一化后回 half，再 native Mul/Add；尾 tile valid |
| W-LP-GENERIC | 8192 source/output tile；低精度输入、输出、参数和三个 FP32 工作 tile，约 180,256 B 级别 | 每行先通过 `ComputeWideLowPrecisionInvRmsPipelined` 求 invRms；之后按 8192 tile 做输出 | gamma/bias tile 对 batchRows 复用；计算 invRms 时以 gamma/bias buffer 充当第二输入 pair | 每 core 可一行或多行；外层每批最多 8 行 | FP16 采用 native Add；BF16 全部 Add/Mul 在 FP32；尾块由 `WideTileLength` 处理 |

### E.3 同步与事件

普通路径使用 `SetFlag/WaitFlag` 封装 MTE2→V、V→MTE2、V→MTE3、MTE3→V、V→S、S→V，见 2813–2847 行。专用流水路径额外分配 event ID，并在复用输入或输出槽前等待 release，典型实现见 1976–2145、2488–2667 行。这个结构的收益前提是搬运、Vector 计算和输出时间足够长，能覆盖 event 与 V/S 往返成本。

## F. 快速 testcase 分组及原因

以下分组只按 R031 结果 JSON 的观测时间，不代表隐藏输入分类。

| 分组 | testcase | R031 时间范围 | 观察 | 可能的机制解释 | 置信度 |
|---|---|---:|---|---|---|
| Q1 快 | T01–T05 | 3.80–19.20 us | 规模小，固定启动和同步成本占比高；T04 已明显高于其它 Q1 点 | 更可能落在窄行路径，或 outer 很小的单行路径；具体 mode 未公开 | MEDIUM：时间事实高，shape 推断低 |
| Q2 中 | T06–T10、T12 | 32.45–98.67 us | 同一数量级内 T07 相对 TBest 差距最大，T09/T10/T12 更接近 | 可能由窄行批处理、参数复用或较小宽行造成；T07 也可能触发较多 V/S 或非理想 localRows | LOW-MEDIUM |
| Q3 大 | T11、T13 | 161.52、565.53 us | T13 已进入明显大数据量区间 | 可能是宽行或多行归约；R031 有宽行分支，但没有 testcase 元数据可绑定 | LOW |
| Q4 极大 | T14–T15 | 9650–16500 us | 总耗时主要由这两项贡献；T14 相对 TBest 为 4.4x，T15 仅 1.134x | 可能是超大张量、dtype/row scheduling 差异或判题机资源波动；不能仅凭时间认定命中的 wide mode | LOW |

结果 JSON 的总量事实是候选 27332.17 us，固定参考总量 138984.15 us；15 项中最慢两项占候选总量约 95.7%，因此后续优化优先围绕大 outer/大 D 的宽行路径，但这仍是资源分配建议，不是隐藏 shape 结论。

## G. 与指定路线的机制对应

本节只写机制相关性与证据限制。没有读取这些路线的 kernel；分数和路线名称来自当前状态记录。R031 的 38.36 被项目文档定义为集成架构参考，不能向下归因给任一单路线。

| 路线 | 与 R031 的机制相关性 | 证据限制 |
|---|---|---|
| R005 | 同属大 tile 思路；R031 明确使用 4096、6112、7680、8192、12288 多档 tile，相关性是“按 D/UB 选 tile” | R005 的 25.71 只是独立路线线上结果，无法证明某个 R005 tile 被 R031 继承或贡献了增益 |
| R006 | 同属 ReduceSum/归约架构；R031 采用 tile partial + 行级合并，且把 scalar 读取压到行末 | R006 线上只有 1/15，不能把其失败原因映射到 R031，也不能把 R031 的结果归因给 R006 |
| R008 | 相关点是跨核切分边界；R031 选择整行单核所有权，不需要跨核 RMS 合并 | R008 的线上结果 21.73 只能证明另一条路线通过，未提供 R031 与跨核切分的对照 |
| R009 | 相关点是 copy-centric；R031 以 `DataCopyPad` helper 和连续 row batch 处理 DMA 颗粒度 | R009 18.61 与 R031 38.36 不是同一源文件或单点消融，不能量化 copy 贡献 |
| R010 | 相关点是手工/尾块处理的性能与安全权衡；R031 统一使用 valid count + DataCopyPad | R010 17.66 不能证明 R031 的尾块路径更快；非对齐搬出语义仍依赖官方文档与真机行为 |
| R012 | 相关点是对齐 row group；R031 通过 FP32 `D%8`、低精度 `D%16` 和连续批处理利用 32B 行字节对齐 | R012 22.96 不能证明 R031 使用了相同实现，也没有 R031 内部去掉对齐条件的对照 |
| R013 | 相关点是 double-buffer pipeline；R031 在宽行和精确 tile 模式使用 ready/release event 交错输入与输出 | R013 18.76 不能单独证明 R031 的 event 设计有效；R031 内含多种同时变化的机制 |
| R014 | 相关点是参数 residency；R031 在窄行多行和宽行 batch 中缓存 gamma/bias，按 channel tile 复用 | R014 20.09 不能量化参数驻留的独立增益，且 R031 的缓存受 localRows 与 dtype 影响 |
| R017 | 相关点是 FP32 middle；R031 的 ReduceSum、平方、Sqrt 和 BF16 参数路径都以 FP32 为中心 | R017 27.16 是单路线结果；R031 的 FP16 路径仍保留 native Add/Mul，不能把 R017 机制视为全局相同 |
| R028 | 相关点是 scalar synchronization reduction；R031 每行只在 partial 合并后通过 V/S 取 squareSum 和 invRms | R028 25.06 不能证明 scalar 读是正收益；R031 的 timing 没有去掉 V/S 往返的对照 |

## H. HIGH / MEDIUM / LOW 性能归因

### HIGH

- R031 线上通过和分数是事实：15/15、38.36，结果字段在 JSON 第 2–6 行。
- R031 确实存在按 D、dtype、localRows/avgRows 分支的多模式分派，源码 58–123、169–249、2669–2768 行直接给出条件。
- R031 确实采用整行单核所有权、FP32 归约和 DataCopyPad 有效字节搬运，源码 31–38、299–369、2771–2787 行给出证据。
- `case_times_us`、`tbest_times_us_latest`、候选总耗时与固定参考总耗时是可复核的 JSON 字段，不能据此推出具体 testcase 输入。

### MEDIUM

- 窄行多行批处理很可能改善 Q1/Q2 中的固定开销，因为源码把多个 row 的归约 scalar 和输出集中处理，具体证据在 1180–1472 行。
- 参数驻留很可能改善同一 core 处理多行的场景，因为 gamma/bias 只在初始化或 channel tile 阶段搬入，证据在 251–283、2711–2765 行。
- 宽行 FP32/FP16 的双缓冲很可能改善大 D、多行场景，因为源码确实同时维护输入 ready/release 与输出 ready/release，证据在 1976–2145、2488–2667 行；但没有单点消融。

### LOW

- 不能确认 T14/T15 分别命中 W-F32、W-F16、W-BF16 还是 W-LP-GENERIC。
- 不能确认 6112、7680、8192、12288 哪个 tile 对 Official Score 的边际贡献最大。
- 不能确认 Q2 中 T07 的 4.352x 差距来自 V/S 同步、参数搬运、row scheduling、平台噪声还是隐藏输入本身。
- 不能把 R005/R006/R008/R009/R010/R012/R013/R014/R017/R028 的独立线上分数转写为 R031 子机制因果证据。
- 宽行 UB 数值只是根据源码常量的静态算术；目标平台 UB、系统保留和编译器实际分配没有在本轮复核。

## I. 可执行消融假设

每项只改变一个机制，保留 dtype、入口、验证矩阵和其它 mode 条件；执行顺序应先覆盖 T14/T15 对应的真实 shape，再覆盖窄行。

1. **H1：宽行 FP32 batch tile**。只把 `W-F32-BATCH` 的 6112 元素改为一个候选 tile，保持 8 组 buffer、行批和事件流水不变；预期可定位 UB 压力与 MTE2/V/MTE3 重叠的折中点。
2. **H2：宽行 FP32 输入双缓冲**。只关闭 `ComputeWideFp32InvRmsPipelined` 的第二输入 pair，保留同样的 tile、归约和参数复用；若大 D timing 上升，说明输入 MTE2 重叠有贡献。
3. **H3：宽行低精度行批大小**。只比较 FP16/BF16 的 8、4、2、1 行批，保持 8192 channel tile 和 FP32 归约不变；预期揭示参数复用与 V/S 标量数组压力的平衡。
4. **H4：FP32 窄行连续批处理**。只比较 `N-F32-CONTIG` 的连续一次 DMA 与逐行 DMA；固定 D、localRows 和 gamma/bias 常驻，观察小 outer 与大 outer 的差异。
5. **H5：窄行参数驻留**。只在 `N-GENERIC` 中关闭 `cacheParams`，让第二遍逐 tile 读取 gamma/bias；若多行场景恶化，说明 parameter residency 是有效因素。
6. **H6：窄行 y 缓存**。只关闭 `cacheRow` 的 value UB 保留，第二遍重读 x/residual；固定 D<=8192，用 GM 读量差异验证 y 缓存收益。
7. **H7：V/S scalar 往返**。只替换行末 `GetValue(0)` 的 invRms 获取方式，保持 ReduceSum 和 Sqrt 的数值链不变；目标是量化小 D、大 outer 的同步成本。
8. **H8：FP16/BF16 输出转换时机**。只比较当前“归一化后先回目标 dtype再做 gamma/bias”和全程 FP32 到最终输出的两种顺序；同时要求逐 dtype 精度证据，避免把速度变化误写成纯性能收益。
9. **H9：宽行 cached-row**。只比较 W-F32-CACHED/W-F16-CACHED/W-BF16-CACHED 与对应 generic streaming，保持每核一行和 D 不变；目标是测量省 GM 重读与占用整行 UB 的取舍。

## J. 给 Multimode Agent 的机制说明

### J.1 架构

把入口视为一个纯运行时 resolver：输入是 dtype、D、rowCount、availableCoreNum、rank 展平后的 shape 和 epsilon；输出是一个 mode 标签及其 tile/UB 计划。所有 mode 都共享同一条数学链，差异只在 row grouping、参数驻留、y 是否保留、tile 大小和 MTE/V/S 调度。

### J.2 判定规则

1. 先验证五个 tensor 的 shape/dtype 一致性，gamma/bias 必须是一维 D。
2. 计算 `rowCount=product(shape[:-1])`、`rowWidth=D`、`blockCount=min(max(availableCoreNum,1),rowCount)`。
3. 对 `D<=8192`，先匹配精确 D=4096/8192 的 dtype 模式，再匹配对齐小行批处理，最后落到 generic narrow。
4. 对 `D>8192`，先按 dtype 分 FP32/低精度；再按 `avgRows` 或每核一行条件选择 batch、cached-row 或 generic。
5. mode 条件必须保持互斥和有序；任何无法证明的 testcase 元数据都标为 unknown，不用 case 编号推断。

### J.3 数据流

- Copy in：按实际有效字节数搬入 x/residual，尾块只让 Vector 使用有效 count。
- Compute：x+residual；FP32 square/reduction；mean+epsilon；Sqrt；reciprocal；归一化、gamma、bias。
- Copy out：按目标 dtype 批量写回；宽行模式可让下一输入、当前 Vector、上一输出在不同 buffer 上交错。
- 复用：gamma/bias 只依赖 channel，适合在同一 core 的多行或同一 row batch 内复用；y 只在 D 和 UB 允许时保留。

### J.4 假设与证据边界

- 低延迟 testcase 可能更受启动和 V/S 固定成本影响；大 D testcase 可能更受 GM 带宽和 tile 复用影响，但两者都要以公开 shape 或实测消融确认。
- UB 计划只能先用源码常量估算，最终需目标 CANN 工具链和真机结果确认。
- DataCopyPad 的非对齐搬出、ReduceSum 临时空间、Cast 舍入和 BF16 指令支持都要以目标 CANN/SoC 文档与真机证据为准。
- 本报告不给出可复制的 kernel 函数实现；Multimode Agent 只应复用架构判定、数据流和消融假设。

## 自核索引

报告引用的主要证据：

- R031 源码：分派与 UB 58–249 行；窄行数据流 286–470 行；常量与 tile 1112–1177 行；窄行批处理 1180–1654 行；宽行 FP32 1655–2145 行；宽行低精度 2146–2768 行；搬运、Cast、同步 2771–2847 行；入口 2884–2957 行。
- R031 结果 JSON：状态、分数、正确性 2–6 行；15 项候选 timing 7–22 行；TBest timing 24–39 行；总耗时与参考字段 41–46 行。
- 题面整理：数学语义 5–20 行；dtype、rank、D 范围 35–48 行；尾块和 API 约束 61–73 行；15 testcase 元数据限制 75–84 行。
- 当前状态：R031 被定义为集成架构参考，不能作为任何独立路线的因果证据，见 `文档/当前状态.md:68-76`、`78-89`。

本轮未编译、未运行 NPU、未修改任何 kernel/CMake/compile.sh，未提交线上结果。
