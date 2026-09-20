# Agent 07 调研报告：AddRmsNormBias 的 UB/Tiling/硬件架构性能分析

> 调研日期：2026-09-11（Agent 7，性能/UB/硬件架构专项）
> 平台前提：本地 macOS，无 CANN、无 NPU。**本报告所有性能数字均来自官方文档/官方样例/社区实测转述，一律未在判题机真实 NPU 上验证**；凡标注"未在真实 NPU 验证"的结论，只能作为真机验证的预判与测量锚点，不得声称性能达标。
> 证据等级：A=官方文档/官方仓库原文；B=官方样例/官方培训/官方技术文章转述；C=社区二手。
> 相关边界：精度分析由 Agent 6 负责；竞赛规则/上传格式由 Agent 1 负责；CANN 安装环境由 Agent 8 负责。本报告不做这三块内容。

---

## 0. 结论速览（TL;DR）

1. **AddRmsNormBias 是确定的访存密集型（memory-bound）算子**：fp32 下算术强度 ≈0.5 flop/Byte 量级，远低于 A2 系 Vector 单元与 GM 带宽之比对应的转折点。**瓶颈在 MTE2/MTE3（GM↔UB 搬运），不在计算**。优化主轴是：减少 GM 访问次数（两遍→单遍/驻留）、大块搬运、双缓冲流水、避免尾块非对齐。
2. **UB 预算按 192KB/AI Core（Atlas A2/A3 系）规划**；A2 训练系实测可调度 48 个 AIV（vector）核，AIC（cube）24 核（官方样例 B 级）；910B 系核数社区说法 20~48 不一，代码一律用 `GetBlockNum()` 运行时查询。
3. **ReduceSum 官方实测**（A 级 asc.gitcode.com）：归约类指令延迟约为 Add 的 2~5 倍；30000 元素 float 全归约，二分累加（Add+ReduceRepeat）172 cycles vs ReduceRepeat 单指令 242 cycles，**ReduceSum 接口通常最慢**。GetValue/SetValue 标量操作"吞吐极低，是性能黑洞"（C 级实战）；每行一次 GetValue 在本算子中被放大为"每行打断流水一次"。
4. **DataCopyPad 非对齐有实打实代价**（官方样例 B 级，A2 系实测）：整矩阵仅尾块 1 行不齐时，GM→UB 端到端劣化约 **-21.6%**，GM→L1 劣化约 **-47.5%**。A2/A3 建议主搬运维度连续字节 512B 对齐。
5. **性能测量路线**：msKPP（写码前建模）→ msOpST `performance_mode=True`（单算子 op_summary）→ msprof op（PipeUtilization/Memory/Arithmetic 细分）→ msopprof（Roofline/流水图）。三者都需真机。

---

## 1. 硬件规格调查（各 SoC）

### 1.1 关键规格表

| 规格项 | Atlas A2 训练系列（910B 系，推测判题机） | Atlas 800I A2 推理（910B2/B4） | Ascend 910B 芯片（学术/社区口径） | 证据等级 |
| --- | --- | --- | --- | --- |
| UB 每 AI Core | **192KB**（16 Bank Group × 3 Bank × 4KB；每 Bank 128 行 × 32B） | 同左（同代架构） | 整片 UB 3.75MB（arXiv 论文）；192KB/核 × ~20 核 ≈ 3.84MB，量级自洽 | B（asc-devkit 官方样例）+ C（CSDN 实战文 "ascend910b 仅 192KB"）+ C（arXiv 2607.20120） |
| L2 Cache | **192MB**（A2/A3 系列，官方样例实测口径） | 同左 | — | B |
| HBM 带宽（单芯片） | **1.6 TB/s**（Atlas 800T A2 官方彩页：8×64GB、内存带宽 1.6TB/s → 单芯片 1.6TB/s） | ~1.6TB/s（同 910B2） | 1.6 TB/s（arXiv）；另有社区口径 ~1.5TB/s、~400GB/s（后者存疑，见未确认清单） | A（官方彩页）/ C（arXiv、社区） |
| L2 带宽 | ~7.86 TB/s（910B/C 论文口径，未细分读写） | 同左 | 7.86 TB/s | C（arXiv） |
| GM 峰值带宽（搬运估算用） | ~1.8 TB/s（asc-devkit 样例用此估算 A2 系 MTE2 理论耗时） | 同左 | — | B |
| AI Core 数量 | AIV（vector）实测可调度 **48** 核；AIC（cube）24 核（官方样例 Block Num 口径） | 未公布；社区口径 20/24/25/32/40 不一 | 20（由 UB 总量反推）/24/25/32 均有说法 | B（样例 Block Num）+ C（社区，互相矛盾） |
| AI Core 频率 | 1.65 GHz（msprof 样例 Current Freq: 1650，Atlas A2 训练系） | 同左 | — | B |
| FP16 算力（单芯片） | 3.0 PFLOPS/8 卡 = 375 TFLOPS（官方彩页口径） | 313 TFLOPS（910B3）~376 TFLOPS（910B2） | 320~376 TFLOPS 社区说法不一 | A/B/C |

### 1.2 对本题的含义（均未在真实 NPU 验证）

- **UB 按 192KB/核规划**是安全的（跨 A2 训练/推理系列一致，且有 910B 整片 3.75MB 反推佐证量级）。若判题机恰好是 910B 系（Atlas 800I A2 推理），该预算同样成立。
- **核数不确定不影响代码正确性**：多核切分用 `GetBlockNum()`/`GetBlockIdx()`，性能预估按 24~48 核区间做上下界。
- **带宽预算**：搬运理论耗时 = 字节数 / 理论带宽（官方口径）；A2 系 GM 估算 1.8TB/s、HBM 1.6TB/s。48 核均分单核约 33~37 GB/s 的 HBM 份额（均匀假设），这是判断"多少搬运量会在多少时间内完成"的粗锚点。

---

## 2. 分主题分析

### 2.1 UB 容量与分块预算算式（主题 1）

**UB 预算算式（统一公式）**：

```text
UB 总占用 = Σ(元素数 × 类型字节)
  = 输入双缓冲 (BUFFER_NUM × tileD × inTypeSize)
  + y/中间 fp32 缓冲 (tileD × 4B)
  + ReduceSum workLocal (fp32 官方公式：≥ RoundUp(firstMaxRepeat, 8) × 8 元素 × 4B；32/typeSize=8，见 2.4)
  + gamma/bias 驻留 (D × 2B [half] 或 D × 4B [fp32]；仅当 D 足够小)
  + 标量/地址/索引开销（预留 ~2~4KB）
  ≤ 192KB × (1 - 安全余量 ≈ 0.85~0.9)   // 建议 ≤ 165~172KB
```

其中 `firstMaxRepeat = ceil(tileD / elementsPerRepeat)`，fp32 时 `elementsPerRepeat=64`、`elementsPerBlock=8`（32B/4B）；`RoundUp(a,b) = ceil(a/b)*b`。

**gamma/bias 驻留可行性**（决定 v2 优化是否成立）：

| 输入 dtype | D 上限（gamma+bias 整体驻留 UB 且不挤爆双缓冲） | 说明 |
| --- | --- | --- |
| half | D ≤ ~8192（2×16KB=32KB 驻留，配合 2×8192×2×2B=64KB 输入双缓冲 + 32KB y + ~2KB workLocal ≈ 130KB） | D>8192 时 gamma/bias 需按 tile 切片搬运，不能整体驻留 |
| fp32 | D ≤ ~4096（2×16KB=32KB 驻留，同上账目） | 同上 |

结论：**"gamma/bias 驻留"只在 D 较小形态成立**；大 D（如 [1,32768]）下 gamma/bias 必须跟随 tile 切片搬运（每 tile 搬对应 slice，成本低但增加指令数）。报告后文 tiling 表按此分形态给出。

### 2.2 tile 大小、指令 repeat 效率与搬运粒度（主题 2）

**Vector 指令理论并行度（官方 SIMD API 文档，A 级，Atlas A2/A3 系列）**：

| 指令 | fp32（element/cycle） | half（element/cycle） |
| --- | --- | --- |
| Add / Sub / Mul / Abs / Max / Min / Adds / Muls | 64 | 128 |
| Rsqrt | 64 | 128 |
| Sqrt / Div / Exp / Ln | 32 | 32（half Sqrt=32） |
| DataCopy(UB→UB) | — | 256 B/cycle |

- 单核 fp32 向量 Add 吞吐 ≈ 64 elem/cycle × 1.65GHz ≈ **105.6 Gelem/s ≈ 422 GB/s**（读+写）。Rsqrt fp32 同 64 elem/cycle；Sqrt 慢一倍（32）。→ 计算侧远快于搬运侧，佐证 memory-bound。
- **一次 DataCopy 的最佳字节数**（官方样例实测，A2 训练系，GM→UB DataCopyPad，half，12288×12288，48 核）：

| 单次搬运量 | Task Duration | 相对基线 | 结论 |
| --- | --- | --- | --- |
| 128B（[1,64]） | 564.84μs | 基线 | 指令发射开销主导，MTE2 占比 98.9% |
| 8192B（[64,64]） | 233.54μs | +148% | 增大分块大幅降开销 |
| 131072B（[64,1024]） | 215.82μs | +170% | 大分块最优；实测 202.7μs vs 理论 167.8μs，高约 20.8% |

- 结论：**单次搬运 8KB~128KB 区间效率接近；低于 1KB 效率骤降**。A2/A3 建议主搬运维度连续字节 **512B 对齐**（half 数据口径）。
- 256B/32B 对齐：Vector 指令以 256B 块（repeat 粒度）为天然单位；DMA 基础 DataCopy 需 32B 对齐；DataCopyPad 允许非对齐但性能有代价（见 2.8）。
- tile 选择的实用指导：tile 元素数取"整 repeat × 幂 2"（fp32: 64×k；half: 128×k），字节数 ≥ 4KB，优先 ≥ 32KB（双缓冲时每 buffer 减半）。

### 2.3 GM↔UB 搬运吞吐 vs 计算吞吐（roofline，主题 3）

- **算术强度**：fp32 输入，两遍扫描下每元素访存 = 读 x + 读 residual（两遍各一次：2×(4+4)B）+ 写 output 4B = **20B/元素**；计算 = Add + Mul + 归约(~log2D 分摊) + Mul(rms) + Mul(gamma) + Add(bias) ≈ 5~8 flop/元素 → **AI ≈ 0.25~0.4 flop/B**。
- A2 系 roofline 转折点（粗算）：Vector 算力约 422 GB/s（fp32 读写口径）vs GM 带宽 1.8TB/s（MTE2 峰值）→ 转折点 AI* ≈ 422/1800 ≈ 0.23 flop/B。**本题 AI 略高于转折点但同一量级，且实际搬运效率只有 60~80%**，实际表现大概率 bandwidth-bound；单遍/驻留优化后 AI 升到 ~0.5 flop/B，仍不足以转 compute-bound。**结论：两遍扫描、单遍都是访存瓶颈，优化以减搬运量为第一优先**。
- 双缓冲的意义不是提升带宽峰值，而是把"搬运-计算串行"改成重叠：官方训练营数据（C 级）串行版计算利用率 ~35% → Pipe 双缓冲 ~85%，端到端 -60%（10.2ms→4.1ms）。memory-bound 算子尤其吃这一条。
- 方法论对照（GPU MODE/Nsight/ROCm）：与 Nsight 的 Memory Throughput/Dram Throughput 指标对应 Ascend 的 `*_mte2_ratio`/`Memory.csv` 带宽利用率；与 occupancy/延迟隐藏对应 Ascend 的双缓冲+多核并行。只做方法论对照，不引入其数字。

### 2.4 ReduceSum 成本与 GetValue 标量同步（主题 4）

- **ReduceSum 三种实现的选择**（A 级官方 + B 级样例实测）：
  - 实测（float, 30000 元素）：二分累加（Add 循环 + 末段 ReduceRepeat）**172 cycles** < ReduceRepeat 单指令 **242 cycles**；官方说明"ReduceSum 接口由多种指令组合实现，通常最慢"。
  - 归约类指令延迟约为 Add 的 2~5 倍 → **能用 Add 逐步归约就不要一次 ReduceSum**。
  - A2 系语义：tensor 前 n 个 count 版本 = repeat 内二叉树 + repeat 间顺序累加；高维切分版本 = 全二叉树。精度无差异时性能优先选"高维切分/多行合并"（一次处理多行，减少循环与指令发射）。
- **workLocal 空间**（A 级手册公式，fp32）：`firstMaxRepeat = ceil(count/64)`；`workLocal ≥ RoundUp(firstMaxRepeat, 8) × 8`（元素，32/typeSize=8）。例：count=8192 → firstMaxRepeat=128 → 128 元素 = **512B**；count=32768 → 512 元素 = **2KB**；count=1024 → 16 元素 = **64B**（保守预留 128B~512B 均可，手册亦允许传入更大 workLocal）。**workLocal 与 tile 大小正相关，大 tile 要多预留**。
- **GetValue(0)（V→S 同步）**：
  - 社区实战（C 级 CANN 学习中心 as_strided）明确写："**GetValue/SetValue 是性能黑洞**，标量操作吞吐极低，优先用向量 API"。
  - 单次 GetValue 的绝对开销量级**没有公开数字**（列入未确认清单）；但放大效应可定性：每 tile 一次 GetValue = 每 tile 打断一次"搬运/计算"流水等待标量返回；官方技术文章给出同类 scalar 优化案例（TPipe 移出类，scalar_time -17%），说明 scalar 指令占比可达 ~20% 量级、值得优化。
  - **缓解手段**（按优先级）：① 减少调用次数——每核每行只取 1 次（D 大时每行合并各 tile 的部分和，行末一次 GetValue）；② 多行合并——D 小时把多行的归约结果拼成一个向量再取（批量取数）；③ 用向量化 rsqrt 替代标量 rsqrt（把 1/rms 做成向量 scale，配合向量 Muls），把 V→S→V 全部消掉。④ 若必须标量，一次 GetValue 后复用多次（行内 Pass2 全程用该标量）。
- **S→V 事件**：rsqrt 结果回灌向量（Muls 标量参数）也有同步成本，同一"每行一次"预算内。

### 2.5 多核负载均衡与切分策略（主题 5）

- **按行（outer）切分是本算子天然选择**：行间无依赖（每行独立归约），行内 D 方向切分只有"减少单核单行压力"的意义。D ≤ tile 预算（如 D ≤ 8192 half）时无需行内再切。
- 负载均衡：`rowsPerCore = ceil(outer / coreNum)`，`rowStart = blockIdx * rowsPerCore`，末核少算；行数不能整除核数时，**前 (outer mod coreNum) 个核多分 1 行**（静态均分，不用动态调度）。
- 极端小 outer（如 [1,32768]）：outer=1 < coreNum，**只有 1 核干活**。此时要么行内 D 向多核切分（多核各算一段的 y² 部分和，然后跨核归约——但跨核归约需要 GM 原子加或额外同步，复杂且不划算，除非行数极多），要么接受单核。**推荐：小 outer 大 D 形态接受单核，优化单核流水；大 outer 小 D 形态把并行度吃满。**
- 核数上限：A2 训练系 AIV 实测 48 核（样例 Block Num=48）；910B 系 20~48 说法不一。判题机按 24~48 区间预估；`blockDim` 由 Host tiling 传，Kernel 用 `GetBlockNum()` 自适应，不写死。
- 多核扩展性测量：同一算子分别以 blockDim=1/2/4/.../max 跑，观察耗时-核数曲线，斜率趋平处即并行度上限（可能被 HBM 带宽总量卡住——本题大概率卡带宽，多核扩展收益会有限，真机验证）。

### 2.6 shape 形态分析（主题 6）

三形态的 tiling 详见第 3 节表。此处给定性：

| 形态 | 并行度 | 单核 UB 压力 | 归约/GetValue 频率 | 主导瓶颈 |
| --- | --- | --- | --- | --- |
| [1,32768]（小 outer 大 D） | 低（单核） | 高（单行 128KB fp32） | 每行 4~8 次 tile 归约 + 1 次 GetValue | 单核串行 + 搬运带宽；双缓冲收益最大 |
| [8192,64]（大 outer 小 D） | 高（48 核吃满） | 极低（行仅 256B fp32） | 每行 1 次 → **8192 次 GetValue/算子** | 标量同步 + 每行小搬运开销；必须行间合并 |
| [512,1024]（中等） | 中（48 核×~11 行） | 低~中（行 4KB fp32） | 每行 1 次（512 次/算子，可接受） | 平衡；双缓冲 + 行内两遍即可 |

### 2.7 两遍扫描 vs 单遍保存中间结果（主题 7）

- **两遍扫描**：GM 读 2×(x+residual) + 写 output。访存 = 2×2×D×ts + D×ts（ts=2 或 4）。优点：UB 只放一行/tile，任何 D 都能跑；缺点：x/residual 读两遍，访存量翻倍。
- **单遍（保存 y 或 y²）**：Pass1 边算边把 y（fp32）存 GM 或 UB，Pass2 只读 y。
  - 存 UB：仅当整行 y 能驻留（y fp32 = D×4B ≤ ~64KB → D ≤ 16K，与双缓冲预算冲突，实际 D ≤ 4~8K 才现实）；收益：x/residual 只读一遍。
  - 存 GM：额外 GM 写 y + 读 y（2×D×4B），净收益 = 少读一遍 x+residual（2×D×ts）vs 多 8D 字节 → 仅当 ts=4（fp32 输入）时临界持平；**fp16/bf16 输入（ts=2）存 GM 净亏**（8D > 4D）。
- **推荐**：v1~v4 全程两遍扫描；"y 驻留 UB"只在 D≤4096 fp32 / D≤8192 half 的中等形态下作为 v5 级可选优化（减少一次 GM 读），并用真机对比验证。**不要引入 GM 中间缓冲**（净收益不成立）。
- 补充：gamma/bias 每行都要用（D 个元素/行），若每次搬会放大 2×D×ts×outer 的访存；所以**只要 D 允许就整体驻留**（见 2.1），不允许就按 tile 切片，而不是按行重复搬。

### 2.8 DataCopyPad 性能代价（主题 8）

官方样例（B 级，A2 训练系，half 12288×12287，仅尾列不齐）：

| 路径 | 对齐基线 | 非对齐（仅尾块 2046B vs 2048B） | 劣化 |
| --- | --- | --- | --- |
| GM→UB（DataCopyPad，AIV） | 215.82μs | 275.3μs | **-21.6%** |
| GM→L1（AIC） | 230.24μs | 438.76μs | **-47.5%** |

- 启示：**尾块非对齐不是免费的**。本题 D 非 32 倍数时每行都有尾块 → 尾块非对齐的代价会随行数放大（每行 1 次非对齐搬运）。缓解：① 尾块尽量凑 512B 连续对齐（对齐到 32B 至少）；② 把尾块并入前一块做"对齐搬运+掩码计算"（搬整块，用 mask 只算有效元素，搬出用 DataCopyPad 或 UnPad）；③ 若判题机 DataCopyPad 不可用（910B 系不支持）则需 GatherMask/原子累加降级方案（已记录于主调研报告，不展开）。
- 注意：DataCopyPad 官方支持矩阵明确**Atlas A2 训练/800I A2 推理支持**（无 mode 模板版本）；Atlas 推理系列（910B 等）不支持——**判题机若为 910B 裸芯片系需确认实际产品形态**（800I A2 是支持的）。这是真机第一验证项。

### 2.9 性能测量方法（主题 9，详见第 5 节）

msKPP（建模）→ msOpST（单算子 ST+性能）→ msprof op / msOpProf（上板细分）→ msopprof（Roofline/流水图）→ npu-smi（设备状态）。多核扩展性用同一算子不同 blockDim 的耗时曲线。

---

## 3. 不同 shape 形态下的推荐 tiling 思路表（核心交付）

> 假设：A2 系，UB 192KB，AIV 核 24~48；ts = 输入类型字节（half/bf16=2，fp32=4）。数值未在真实 NPU 验证。
> 预算算式：`输入双缓冲 + y(fp32) + workLocal + gamma/bias 驻留 ≤ 165KB`。

### 形态 A：[1, 32768]，fp32（ts=4，小 outer 大 D）

| 项 | 数值/建议 | 理由 |
| --- | --- | --- |
| UB 预算算式 | 方案①（保守，独立缓冲）：x 双缓冲 2×32KB + residual 双缓冲 2×32KB + y fp32 32KB + workLocal ~1KB ≈ **161KB**（贴预算上限，可用）；方案②（推荐）：y 经 TQueBind VECOUT 复用已释放的输入缓冲 → **~129KB**，留足余量。gamma/bias **不能整体驻留**（32768×4×2=256KB），按 tile 切片搬 | 单行 128KB，必须 tile；gamma/bias 按 tile 切片搬 |
| tile 大小 | tileD = **8192 元素（32KB）**，双缓冲；行内 4 个 tile。若真机显示 UB 吃紧，降 tileD=4096（16KB，总占用 ~80KB，仍处大块高效区） | 单次搬运 32KB 处在大块高效区（2.2）；8192/64=128 repeat 整数，指令整齐；4096/64=64 repeat 同样整齐 |
| 双缓冲 | **要**（BUFFER_NUM=2） | 单核串行形态，流水重叠收益最大；无其他并行手段 |
| ReduceSum/GetValue 频率 | 每 tile 1 次 ReduceSum（4 次/行）→ 部分和向量合并 → **行末 1 次 GetValue** | 避免每 tile GetValue；D=32768 行末一次即可 |
| 多核切分 | outer=1 → **单核执行**；不做行内跨核归约 | 跨核归约代价（GM 原子加/同步）> 收益；除非 outer 稍大（如 [4,32768]）再按行均分 |
| 预估瓶颈 | 单核 MTE2 带宽 + 串行流水 | 整行搬运 2 遍 × 256KB + 输出 128KB ≈ 640KB/行，1.6TB/s/48 核≈33GB/s → 单核纯搬运 ~19μs 量级（不含流水损失，粗锚点） |
| 备选 | 若真机显示 scalar bound（GetValue 太贵）：改向量化 rsqrt + Muls | 2.4 缓解手段③ |

### 形态 B：[8192, 64]，half（ts=2，大 outer 小 D）

| 项 | 数值/建议 | 理由 |
| --- | --- | --- |
| UB 预算算式 | 一次搬 K 行：K×64×2B×2（x、res 双缓冲）+ y fp32 K×64×4 + workLocal(按 K×64 归约) + gamma/bias 驻留 64×2×2=256B（可驻留） | 单行仅 128B，**绝不能逐行搬**（2.2 实测 128B 级搬运效率最低） |
| tile 大小 | **K=32 行合并为一个 tile（4096B/缓冲）**；K 取 16~64 扫描 | 行内 128B + 非对齐尾块风险最小化；合并后 ≥4KB 才进入高效区 |
| 双缓冲 | 要（BUFFER_NUM=2） | 多核吃满时靠流水隐藏搬运 |
| ReduceSum/GetValue 频率 | **行间合并归约：用高维切分 ReduceSum 一次处理 K 行（dst 为 K 元素）**；每核每批 1 次取数；**GetValue 从 8192 次降到 ~256 次（8192/32）** | 2.4 缓解手段②；这是本形态最大优化点 |
| 多核切分 | outer=8192 行均分 48 核 ≈ 170 行/核；每核 6~11 批 | 行数充足，核数吃满 |
| 预估瓶颈 | 若不做合并：**GetValue 标量同步**（8192 次/算子，每行打断流水）；合并后：MTE2 小搬运指令发射开销 | 本形态把"减少标量同步"列为第一优先级 |
| 备选 | D=64 时一行 <1 repeat（fp32 64 元素=1 repeat）无尾块对齐问题（64×2B=128B 是 32B 倍数） | 尾块只出现在 D 非 32 倍数时 |

### 形态 C：[512, 1024]，fp32（ts=4，中等）

| 项 | 数值/建议 | 理由 |
| --- | --- | --- |
| UB 预算算式 | 行内 tileD=1024（4KB）；双缓冲 x、res 2×2×4KB=16KB + y 4KB + workLocal（1024→firstMaxRepeat=16→16 元素=64B，保守预留 512B）+ gamma/bias 驻留 1024×4×2=8KB → 合计 ~29KB，**余量巨大** | 全形态最宽裕；可考虑 y 驻留单遍优化 |
| tile 大小 | tileD=1024（整行）；如需更大吞吐可一次搬 2~4 行（8~16KB/缓冲） | 4KB 已在高效区边缘，合并 2~4 行更稳 |
| 双缓冲 | 要（BUFFER_NUM=2） | 标准流水；余量允许 |
| ReduceSum/GetValue 频率 | 每行 1 次 ReduceSum（count 版本 1024 元素）+ 每行 1 次 GetValue（512 次/算子） | 频率中等；若 scalar bound 明显再行间合并（2~4 行一批） |
| 多核切分 | 512 行均分 48 核 ≈ 11 行/核（或按 32 核 ≈ 16 行/核） | outer 与核数匹配良好 |
| 预估瓶颈 | 搬运带宽为主，scalar 次之 | 平衡形态，先跑 v1 再按 profiling 数据定优化优先级 |
| 备选 | v5 级：y 驻留 UB 单遍（D=1024 fp32，y 仅 4KB，可行） | 2.7 中"单遍存 UB"的适用场景 |

### 形态间通用结论

- 三形态的**共同第一优化**：大块搬运（≥4KB，目标 32KB 级）+ 双缓冲流水 + 尾块处理（512B 对齐优先）。
- **差异点排序**：形态 A 重流水（单核）、形态 B 重标量同步削减、形态 C 最平衡、可直接作为基准形态验证 v1~v4。

---

## 4. "先正确后性能"优化阶梯（对应首版 v1 → v4）

| 版本 | 内容 | 预期收益（未真机验证） | 风险/注意 |
| --- | --- | --- | --- |
| **v1 两遍正确性优先** | 按行切分多核；行内 tile 两遍（FP32 ReduceSum + GetValue(0) + 标量 rsqrt；第二遍归一化+gamma+bias）；DataCopyPad 尾块；BUFFER_NUM=1（现首版） | 正确性基线；能跑通全部 15 点 | 串行流水利用率 ~35% 级；每行 GetValue 放大；尾块非对齐代价 21.6% 级 |
| **v2 gamma/bias 驻留** | D 允许时 gamma/bias 整体驻留 UB（仅搬一次），否则按 tile 切片；TPipe 移出类对象触发 scalar 折叠优化（官方 A 级技巧） | 减少每行 2 次搬运 + scalar 指令 ~17% 级 | 只对 D≤8192(half)/4096(fp32) 成立；需核对 2.1 账目 |
| **v3 减少标量同步 / 向量化 rsqrt** | 行内部分和向量合并、行末一次 GetValue；或全向量化：向量 Rsqrt + Muls 替代标量 rsqrt；D 小形态行间合并归约（高维切分 ReduceSum） | 消除"每 tile/每行打断流水"；scalar ratio 显著下降 | 向量化 rsqrt 需构造 scale 向量（可用 Duplicate）；行为与标量版本需一致性验证（精度归 Agent 6） |
| **v4 双缓冲流水** | BUFFER_NUM=2，CopyIn/Compute/CopyOut 重叠（TQue/TQueBind 管理） | 端到端 -50%~-60% 级（训练营 C 级参考：串行 10.2ms→4.1ms） | UB 预算翻倍输入缓冲；需重算 2.1 账目；CopyOut 也进流水（用 VECOUT TQue） |
| v5（可选，真机确认后再做） | 中等形态 y 驻留 UB 单遍；二分累加替代 ReduceSum 接口（2.4：172 vs 242 cycles） | 再减一次 GM 读；归约指令数下降 | 仅特定形态；二分累加代码复杂度上升，需数值一致性验证 |

**顺序原则**：v1 先保正确性与全形态覆盖 → 真机 profiling（第 5 节）确认瓶颈占比（MTE2 vs scalar vs vector）→ 按占比选 v2/v3/v4 优先级，不必严格按编号。例如形态 B 若 scalar bound，v3 优先于 v4。

---

## 5. 性能测量方案（真机执行）

### 5.1 开发前：msKPP 建模（可选但推荐）

- 工具：`mskpp design --op_type=...`（官方仓 Ascend/mskpp，A 级），秒级返回理论性能与 PIPE 瓶颈（Pipe_statistic.csv / Instruction_statistic.csv / trace.json）。
- 用途：写码前对比"两遍 vs 单遍""tile 大小"的理论上限，缩小真机试错面。
- 限制：是建模不是实测，最终以 msOpST/msprof 为准。

### 5.2 单算子功能+性能：msOpST

- 配置 `msopst.ini`：`performance_mode=True`，运行后读 `run/out/prof/JOBxxx/summary/op_summary_0_1.csv`（A 级官方文档）。
- 观测字段：Task Duration（端到端）、aiv_time、各 PIPE 耗时占比、Block Dim。

### 5.3 上板细分：msprof op（与 msOpProf 同源）

- 命令（B 级官方样例）：`msprof op --output=./prof ./execute_op`；参数 `--kernel-name`、`--launch-count`、`--warm-up`（防降频影响测量）、`--output`。
- 关键输出文件（官方口径）：
  - `OpBasicInfo.csv`：Task Duration、Block Dim、频率（核对是否 1650MHz，确认没有降频）。
  - `PipeUtilization.csv`：MTE2/MTE3/Vector/Scalar 各 PIPE 耗时与占比 → **判断瓶颈是搬运还是计算还是标量**（对应 2.3/2.4）。
  - `Memory.csv` / `MemoryUB.csv`：UB/L1/L2/HBM 读写带宽速率（GB/s）→ 与 1.6~1.8TB/s 理论比对，算带宽利用率。
  - `ArithmeticUtilization.csv`：Cube/Vector 指令 cycle 占比。
  - `ResourceConflictRatio.csv`：UB bank 冲突率（多 buffer 布局不合理时会高）。
  - `L2Cache.csv`：L2 命中率。
- 单算子带宽理论锚点：`搬运耗时 = 字节数 / 1.8TB/s`（GM 峰值），`计算耗时 = 元素数 / 105.6Gelem/s`（fp32 vector）。

### 5.4 Roofline/流水图：msopprof（SIMD 工具链）

- 上板 `msopprof ./op_binary` 与仿真 `msopprof simulator`（A 级 asc.gitcode.com）：输出计算内存热力图、Roofline 瓶颈分析图、Cache 热力图、通算流水图、指令流水图。用于确认"是否 memory-bound"与定位搬运热点。

### 5.5 设备状态：npu-smi

- `npu-smi info`（芯片型号/显存）、`npu-smi info -t usages`（使用率）、`npu-smi info -t topo`（拓扑，多卡场景确认）。用于确认判题 SoC 型号（回填未确认清单第 1 条）。

### 5.6 多核扩展性测量

- 同一算子分别跑 blockDim ∈ {1, 2, 4, 8, 16, 24, 32, 48}，记录 Task Duration 曲线：斜率趋平处 = 并行度上限（本题大概率被 HBM 总带宽封顶）。同时看是否随核数线性下降（线性=带宽未饱和，异常早平=存在串行段如 GetValue/同步）。

### 5.7 测量纪律

- 每配置多跑几次取中位数；`--warm-up` 预热防降频；固定 Host 侧任务耗时（Task Duration 含调度，只对比同模式下数值）；每轮优化后保存 profiling 目录与 git 记录（对应 AGENTS.md 的 v0.2/v0.3 证据要求）。

---

## 6. 明确未确认清单（真机/官方资料缺失项）

1. **判题机 SoC 型号**（Atlas 800T A2 训练 vs 800I A2 推理 vs 其他 910B 形态）——影响：核数（24/48?）、DataCopyPad 支持矩阵、HBM 带宽口径。当前按"800I A2 推理（910B2）支持 DataCopyPad"规划，真机 `npu-smi info` 确认。
2. **UB 每核 192KB**：A 级 asc-devkit 官方样例 + C 级实战文 + arXiv 反推一致，但未见 CANN 9.0.0 官方架构图原文；真机用 `GetRuntimeUBSize`/`GetUBSizeInBytes`（官方接口）打印确认。
3. **AI Core 核数**：社区 20/24/25/32/40/48 说法互相矛盾；官方不公布。以 `GetBlockNum()` 运行时值为准；性能预估按 24~48 区间。
4. **HBM 带宽口径**：官方彩页 1.6TB/s（Atlas 800T A2）与 arXiv 1.6TB/s 一致；个别社区文章写 ≈400GB/s（jishuzhan），疑为笔误，未采信但记录。真机以 Memory.csv 实测带宽利用率为准。
5. **GetValue(0) 单次标量同步的开销绝对值**：无公开数字；只有"标量吞吐极低""scalar 占比 ~20% 级"的定性证据。放大效应（每 tile/每行一次）为推理性结论。
6. **DataCopyPad 在 CANN 9.0.0 的支持矩阵**：8.x 手册 A2 支持；9.0.0 无变更公告，推断兼容，未验证。
7. **双缓冲收益 -60%**（10.2→4.1ms）：CANN 训练营 C 级数据，非本算子实测；本算子为 memory-bound 且两遍扫描，收益可能低于该值，真机验证。
8. **二分累加 172 vs ReduceRepeat 242 cycles**：官方 SIMD 文档口径（float 30000 元素），与 ReduceSum 接口对比在 A2 上的具体差距未给出，真机复测。
9. **512B 对齐建议**：官方样例 half 数据口径；fp32/bf16 输入的对齐推荐需真机微基准确认。
10. **频率 1.65GHz**：msprof 样例读数；判题机实际频率与是否降频未知，测量时核对。

---

## 7. 本轮新增来源清单（Agent 7）

> 完整合并清单见 `sources.md`；以下为 Agent 7 本轮新增/补充依据。全部标注"未在真实 NPU 验证"。

| # | 名称 | URL | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- |
| A7-1 | CANN/asc-devkit Add 性能调优样例（UB 192KB 结构、A2 支持矩阵、7 步优化路径） | https://blog.csdn.net/gitblog_00754/article/details/157048953 | 2026-09-11 | UB 容量/分块/多核/双缓冲/L2 直通优化路径 | B（官方样例转述） |
| A7-2 | CANN/asc-devkit DataCopy 内存访问最佳实践样例（分块粒度/非对齐/L2Cache/冲突实测数据） | https://blog.csdn.net/gitblog_00924/article/details/157925888 | 2026-09-11 | tile 粒度、非对齐代价 21.6%/47.5%、512B 对齐、L2 192MB | B（官方样例转述） |
| A7-3 | SIMD-API Vector 指令理论性能汇总（A2/A3 各指令 element/cycle） | https://asc.gitcode.com/api/附录/Vector指令理论性能汇总.html | 2026-09-11 | 指令并行度（Add fp32=64、Rsqrt=64、Sqrt=32、DataCopy UB=256B/c） | A |
| A7-4 | asc.gitcode.com 选择低延迟指令，优化归约操作性能（二分累加 172 vs ReduceRepeat 242 cycles；归约延迟 2-5×Add） | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | 2026-09-11 | ReduceSum 成本与替代方案 | A |
| A7-5 | asc-devkit ReduceSum 归约求和 API（A2 系 count/高维切分相加方式、workLocal） | https://blog.csdn.net/gitblog_00089/article/details/151635178 | 2026-09-11 | ReduceSum 语义/参数 | B（官方仓转述） |
| A7-6 | Ascend C 算子性能优化实用技巧 05——API 使用优化（TPipe 类外化 scalar -17%、TQueBind、SetMaskCount） | https://www.hiascend.com/developer/techArticles/20241107-1 | 2026-09-11 | scalar 优化技巧 | A（官方技术文章） |
| A7-7 | CANN 学习中心：as_strided 算子实战（GetValue/SetValue 性能黑洞、ascend910b UB 192KB） | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 2026-09-11 | GetValue 标量同步代价定性 | C |
| A7-8 | CANN 训练营：双缓冲流水线实战（串行 10.2ms→Pipe 4.1ms，利用率 35%→85%） | https://blog.csdn.net/aasd23/article/details/154999873 | 2026-09-11 | 双缓冲收益参考 | C（训练营转述） |
| A7-9 | Ascend C 中的"流水线"艺术：为何计算与搬运要重叠 | https://blog.csdn.net/aasd23/article/details/154999822 | 2026-09-11 | 双缓冲原理/收益 | C |
| A7-10 | ASCEND TO SCIENCE（arXiv 2607.20120）：910A/B/C UB 总量、L2 带宽 7.86TB/s、HBM 1.6TB/s | https://arxiv.org/pdf/2607.20120 | 2026-09-11 | 芯片级带宽/UB 反推 | C（学术预印本，实测） |
| A7-11 | Atlas 800T A2 服务器官方彩页（8×昇腾910、FP16 3.0P、片上内存 1.6TB/s） | https://www.hiascend.com/hardware/ai-server?tag=900A2 | 2026-09-11 | 单芯片 HBM 带宽 1.6TB/s（A 级） | A |
| A7-12 | msOpST 生成/执行测试用例（performance_mode、op_summary_0_1.csv） | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/devaids/opdev/optool/atlasopdev_16_0033.html | 2026-09-11 | 单算子 ST+性能测量 | A |
| A7-13 | CANN 学习资源仓：算子调试三 msProf 及仿真（msprof op 参数、输出文件列表、带宽理论公式、1650MHz） | https://bbs.huaweicloud.com/blogs/5a9291facf6e43148d16ef69a63dc97c | 2026-09-11 | 上板 profiling 方法 | B |
| A7-14 | 算子开发工具指南（msKPP 建模：Pipe_statistic/Instruction_statistic/trace.json） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/devaids/opdev/optool/CANN%208.0.0%20alpha001%20算子开发工具指南%2001.pdf | 2026-09-11 | msKPP 用法 | A |
| A7-15 | asc.gitcode.com 性能调优（msOpProf 上板/仿真、Roofline 图、输出 CSV 清单） | https://asc.gitcode.com/guide/编程指南/调试调优/性能调优.html | 2026-09-11 | msopprof 用法 | A |
| A7-16 | Ascend/mskpp 官方仓库 | https://github.com/Ascend/mskpp | 2026-09-11 | msKPP 入口 | A/B |
| A7-17 | 昇腾 910B 系列四款型号对比（B1~B4 俗称、算力、服务器选型） | https://jishuzhan.net/article/2069227057103597570 | 2026-09-11 | 910B 型号谱系；注意其 400GB/s 带宽口径未采信 | C |
| A7-18 | GLM-5.2-W8A8 部署报告（Atlas 800I A2 = 8×Ascend 910B2，64GB HBM/卡） | https://firsh.me/blog/0184/attachments/glm52_w8a8_report.pdf | 2026-09-11 | 800I A2 硬件构成佐证 | C |
| A7-19 | GPU 学习笔记一：从 A100 与 910B 分析（910B 25 AI Core、2 vector+1 cube/核，单周期 128 fp16/vector core） | https://blog.csdn.net/qq_40214669/article/details/143271235 | 2026-09-11 | 核数/微架构社区口径（与官方样例 48 核口径有出入，未采信为结论） | C |
| A7-20 | 华为昇腾 910B（helix-peak：32 AI Core 口径） | https://kb.helix-peak.com/techentry/huawei-ascend-910b.html | 2026-09-11 | 核数另一社区口径（互相矛盾，仅记录） | C |

**来源冲突提示**：核数（20/24/25/32/40/48）与 HBM 带宽（1.5~1.6 vs 0.4TB/s）在社区来源间存在矛盾；本报告以 A 级官方彩页（1.6TB/s）与 B 级官方样例（UB 192KB、48 AIV 核、GM≈1.8TB/s 估算）为主口径，其余记为冲突并待真机 `npu-smi` 与 profiling 定案。
