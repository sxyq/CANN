# Agent 07 调研报告：性能、UB 与硬件架构

> 主题：AddRmsNormBias（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，vector 算子，昇腾 NPU）
> 工作语言：简体中文
> 调查人：agent07
> 访问日期：2026-09-11（所有来源 URL 的访问日期均为本日）
> 重要约束：本机 macOS，**无 CANN、无昇腾 NPU**，无法实测任何耗时。下文所有“耗时/带宽上限”均为**理论上界或社区实测量级**，绝不可当作判题机耗时。

---

## 1. 结论摘要（≤12 条）

1. **UB 容量（关键事实）**：Atlas A2 训练系列（Ascend 910B1/B2）UB = **192 KB**；910B3/B4 同样 192 KB；310B/310P 为 256 KB。该数字源自 CANN 硬件目标定义文件 `NPUTargetSpec.td`（达芬奇架构 `dav-c220`），并被多份独立来源交叉印证。**等级 A/B**（见硬件事实表）。
2. **核数**：910B2 = 24 个 AI Core，每个 AI Core 含 2 个 AIV（Vector Core）→ **48 个向量核**；910B3/B4 = 20 AI Core / 40 向量核。本算子为 `__vector__` 类型，应当用 **GetCoreNumAiv()（=48）** 作为并行核数，否则会浪费一半算力。
3. **向量吞吐（官方口径）**：Vector 单元每次 repeat 读 **256 字节**；故 fp16/bf16 每 repeat 处理 **128 元素**，fp32 每 repeat 处理 **64 元素**（mask 上限分别为 128 / 64）。来源：昇腾社区《通用参数说明》。**等级 A**。
4. **候选实现 UB 占用**：fp16/bf16 单核峰值 **108 KB（110592 B）**、fp32 峰值 **76 KB（77824 B）**，均远低于 192 KB 上限，UB 不是候选实现的瓶颈。
5. **访存次数（核心结论）**：候选“两遍扫描”相对“单遍暂存”**每行的 GM 流量多 2·D·S 字节**（即多读一次 x 与一次 residual）。整体上 GM 流量比 = **7 : 5 = 1.4×**。fp16 @ D=32768 时每行多 128 KB；fp32 @ D=32768 时每行多 256 KB。
6. **单遍暂存在 UB 内可行**：把 `value = x + residual`（fp32）整行驻留 UB，fp16/bf16 需 128 KB、fp32 需 128 KB（D=32768），均可放入 192 KB。单遍可比两遍少 ~28.6% 的 GM 流量。
7. **ReduceSum 含标量同步**：`ReduceSum` 内部存在 scalar 的 SET_FLAG/WAIT_FLAG（社区实测确认会阻塞流水）；候选实现在 ReduceSum 后还额外做 `V_S→WaitFlag→GetValue(0)→S_V`，对“小 outer 大 D”形态（每行多次 tile）会放大标量停顿。
8. **小 D 形态应消除 GetValue**：当 D 很小（如 64），可把 rms 的计算保留为 UB 内 1 元素张量，用 Vector 的 `Sqrt` + `Reciprocal` 得到 `1/rms` 再做 `Muls` 广播，**完全避免标量读回**，对小 D 形态收益最大。
9. **多核负载均衡**：候选用 `each=outer/blocks; extra=outer%blocks; first=block*each+(block<extra?block:extra); count=each+(block<extra?1:0)`，是正确的“前 extra 个核多 1 行”均衡法；但前提是 `blocks` 取对——必须取向量核数（48），否则半数核空闲。
10. **DataCopyPad 相对 DataCopy 有开销**：在 D 满足 32 字节对齐（fp16 的 D 为 16 的倍数、fp32 为 8 的倍数）时应改用 `DataCopy` 走对齐高效 DMA 路径；`DataCopyPad` 仅在非对齐/需 padding 时使用。该结论主要来自社区经验（等级 C），但属合理优化方向。
11. **性能测量**：上板用 `msprof op`（配 `--aic-metrics=Occupancy,Memory,MemoryUB,PipeUtilization,ResourceConflictRatio`）；单算子耗时用 `msopst run ... -soc Ascend910Bx` 且 `msopst.ini` 中 `performance_mode=True`，看 `op_summary_0_1.csv`。无 NPU 时仅能用 `msprof op simulator`（结果仅供参考）。
12. **未证实项**：GM→UB 的单核实测 DMA 带宽、ReduceSum 内部标量同步的精确延迟、DataCopyPad 相对 DataCopy 的量化开销——均无官方/实测数字，列入“未验证清单”。

---

## 2. 硬件参数事实表

> 证据等级：A=昇腾社区/官方文档口径；B=CANN 源码级硬件定义或硬件校验工具编码值；C=社区实测/经验/第三方整理（含学术论文），存在不确定；D=未证实/推测。
> “官方文档口径”与“社区实测数字”已明确区分；社区绝对耗时不得当作判题机耗时。

| 参数 | 值 | 适用产品 | 官方/权威出处 | 访问日期 | 等级 |
|---|---|---|---|---|---|
| Unified Buffer 容量 | 192 KB (196608 B) | Ascend 910B1 / 910B2 / 910B3 / 910B4（Atlas A2 训练系列） | CANN 硬件目标定义 `NPUTargetSpec.td`（架构代号 `dav-c220`）；另见 ascend-rs 编译校验报错 “Ascend910B3 UB limit of 196608 bytes”、arXiv 论文(ENEC) “e.g., 192KB” | 2026-09-11 | **B**（源码级/工具校验，非华为单页 datasheet；华为未公开完整 910B datasheet，aiwiki 已注明） |
| Unified Buffer 容量 | 256 KB (262144 B) | Ascend 310B 系列、Ascend 310P | ascend-rs 报错 “256KB for 310P”；CANN `NPUTargetSpec.td` 310B UB=256KB | 2026-09-11 | B |
| AI Core 数 | 24（910B1/910B2）；20（910B3/910B4） | Ascend 910B 系列 | CANN `NPUTargetSpec.td`（Copied from cannbot-skills 整理） | 2026-09-11 | B |
| Vector Core 数（AIV） | 48（910B1/910B2）；40（910B3/B4） | Ascend 910B 系列 | `VectorCoreCount = 2 × AiCoreCount`，`NPUTargetSpec.td`；arXiv 2505.15112 确认“each AI Core contains 1 AIC + 2 AIV” | 2026-09-11 | B |
| Cube Core 数（AIC） | 24 / 20 | Ascend 910B 系列 | 同上 | 2026-09-11 | B |
| L1 Buffer | 512 KB | 910B 系列 | `NPUTargetSpec.td` | 2026-09-11 | B |
| L0A/L0B | 64 KB / 64 KB | 910B 系列 | 同上 | 2026-09-11 | B |
| L0C | 128 KB | 910B 系列 | 同上 | 2026-09-11 | B |
| Vector 每 repeat 处理字节 | 256 B | 全系（达芬奇 Vector 单元） | 昇腾社区《通用参数说明》：向量单元每次迭代读取连续 256 字节 | 2026-09-11 | **A** |
| fp16/bf16 每 repeat 元素数 | 128 | 全系 | 256/2=128，mask∈[1,128] | 2026-09-11 | **A** |
| fp32 每 repeat 元素数 | 64 | 全系 | 256/4=64，mask∈[1,64] | 2026-09-11 | **A** |
| 聚合 HBM 带宽（上界） | 约 1600 GB/s（910B，HBM2e 4×16GB）；初代 910 为 1228 GB/s | 910B / 910 | 社区整表（华为 2025 昇腾生态大会等）与 CSET 对技术文档整理；**非单核 DMA 实测** | 2026-09-11 | C（聚合上界，非核内 DMA 实测） |
| 单核 GM→UB（MTE2）带宽 | 无官方/实测数字 | 910B | — | — | **D（未证实）** |
| 多核同步函数 | GetBlockIdx / GetBlockNum | 全系 | 昇腾社区《GetBlockNum》API 文档（CANN 9.0.x / 8.0.RC3） | 2026-09-11 | A |
| 向量核数查询 | GetCoreNumAiv() | 全系 | 昇腾社区 API（多核执行文档提及） | 2026-09-11 | A |

**UB 192 KB 官方出处的说明（硬性要求回应）**：
- 未检索到华为以“datasheet 单页”形式写明 “Atlas A2 UB=192KB” 的网页（aiwiki 明确说“华为从未发布完整 910B datasheet，市面数字多为分析估算”）。
- 但 **CANN 软件自身** 在 `NPUTargetSpec.td` 中把 910B 系列的 UB 定义为 192 KB（架构 `dav-c220`），且 ascend-rs 的 `ascend_compile` 在编译期据此校验 `InitBuffer` 上限并报错 “UB limit of 196608 bytes”。这属于**源码级/工具校验级**证据（等级 B），可信度高于普通社区猜测，但仍非“华为市场文档”。本报告据此采用 192 KB，同时标注其等级为 B。

---

## 3. UB 预算分析（候选实现 `提交/V002/kernel.asc`）

### 3.1 候选 UB 分配逐项（来自 `Init()`，BUFFER_NUM=1）

InitBuffer 一次性预留以下缓冲（UB 峰值 = 所有预留之和，因 BUFFER_NUM=1 无双缓冲）：

| 缓冲 | 类型/位置 | fp16/bf16（TILE_HALF=4096, S=2） | fp32（TILE_FLOAT=2048, S=4） |
|---|---|---|---|
| x_queue_ (VECIN) | GM 镜像 x | 4096×2 = 8192 B | 2048×4 = 8192 B |
| residual_queue_ (VECIN) | GM 镜像 residual | 4096×2 = 8192 B | 2048×4 = 8192 B |
| gamma_queue_ (VECIN) | GM 镜像 gamma | 4096×2 = 8192 B | 2048×4 = 8192 B |
| bias_queue_ (VECIN) | GM 镜像 bias | 4096×2 = 8192 B | 2048×4 = 8192 B |
| output_queue_ (VECOUT) | GM 镜像 output | 4096×2 = 8192 B | 2048×4 = 8192 B |
| x_float_ (VECCALC) | fp32 中转 | 4096×4 = 16384 B | 2048×4 = 8192 B |
| residual_float_ (VECCALC) | fp32 中转 | 4096×4 = 16384 B | 2048×4 = 8192 B |
| value_float_ (VECCALC) | 计算结果 | 4096×4 = 16384 B | 2048×4 = 8192 B |
| work_ (VECCALC) | ReduceSum 工作区 | 1024×4 = 4096 B | 1024×4 = 4096 B |
| sum_ (VECCALC) | ReduceSum 结果 | 4096×4 = 16384 B | 2048×4 = 8192 B |
| **合计** | | **110592 B ≈ 108 KB** | **77824 B ≈ 76 KB** |

- **结论**：fp16/bf16 峰值 108 KB、fp32 峰值 76 KB，均 < 192 KB（UB 上限）。UB 不是候选实现的瓶颈，余量分别约 84 KB / 116 KB。
- 工程记录中“fp16/bf16 单遍暂存峰值约 80–112 KB”的假设与上面 108 KB 自洽（候选虽是两遍，但缓冲一次性全预留，故峰值同量级）。**该假设成立**，且与 192 KB 上限不冲突。
- 候选用 BUFFER_NUM=1（无双缓冲），因此 MTE 搬运与 Vector 计算**无法重叠**，是性能隐患（见第 5、6 节）。

### 3.2 单遍暂存的 UB 可行性

若改为单遍并整行驻留 `value`（fp32）：
- fp16/bf16 输入：value = D×4 字节。D=32768 → 128 KB；加每 tile 的 gamma/bias/output（各 4096×2=8 KB）→ 峰值 ≈ 128+24 = 152 KB < 192 KB。可行。
- fp32 输入：value = D×4 = 128 KB（D=32768）；gamma/bias 按 tile 流式读（各 8 KB）→ 峰值 ≈ 152 KB < 192 KB。可行。
- bf16 同 fp16。
- **结论**：对题目所有 D∈[64,32768]，单遍整行驻留 value 均在 192 KB 内可行；因此对“访存次数”优化无需牺牲 UB。

---

## 4. 访存次数对比：两遍扫描 vs 单遍暂存

设 dtype 字节数 S（fp16/bf16=2，fp32=4），行长度 D，单核处理 `count` 行。

### 4.1 每行 GM 流量公式（忽略 32B padding 对读的影响——DataCopyPad 仅零填充 UB 目的，源端仍只读 len×S）

**候选两遍扫描**：
- 第 1 遍 ReduceRow：读 x（D·S）+ 读 residual（D·S）= 2·D·S
- 第 2 遍 NormalizeRow：读 x（D·S）+ residual（D·S）+ gamma（D·S）+ bias（D·S）+ 写 output（D·S）= 5·D·S
- **合计 = 7·D·S / 行**

**单遍暂存（推荐）**：
- 读 x（D·S）+ residual（D·S）+ gamma（D·S）+ bias（D·S）+ 写 output（D·S）= 5·D·S / 行
- reduce 读 UB 内 value（无 GM）；normalize 读 UB 内 value（无 GM）
- **合计 = 5·D·S / 行**

### 4.2 字节数与相对比例（按 dtype）

| dtype | D | 两遍/行 | 单遍/行 | 多读（=重读 x+residual） | 比例 |
|---|---|---|---|---|---|
| fp16/bf16 | 32768 | 458752 B（448 KB） | 327680 B（320 KB） | 131072 B（128 KB） | 1.400× |
| fp32 | 32768 | 917504 B（896 KB） | 655360 B（640 KB） | 262144 B（256 KB） | 1.400× |
| fp16/bf16 | 4096 | 57344 B | 40960 B | 16384 B | 1.400× |
| fp16/bf16 | 64 | 896 B | 640 B | 256 B | 1.400× |
| fp32 | 64 | 1792 B | 1280 B | 512 B | 1.400× |

**结论**：两遍扫描相对单遍的 GM 流量恒为 **1.4×**，额外部分正是“重读一次 x 与一次 residual”。整体访存量随 outer 线性放大；性能评分只依赖耗时，故减少 ~28.6% 的 GM 读对大 outer 形态尤其关键。

### 4.3 关于 DataCopyPad 的额外开销（叠加项）

- 候选对**所有** CopyIn/CopyOut 用 `DataCopyPad`。社区经验（arXiv AscendOptimizer 经验库）指出：在对齐满足时用 `DataCopy` 比 `DataCopyPad` 更快，因为后者有“非连续/非对齐额外处理”。
- 当 D 满足 32B 对齐（fp16：D%16==0；fp32：D%8==0）时，可切换为 `DataCopy`；不满足（如 D=32768→32768%16==0 对齐；但 D 可能为 64~32768 任意值且“可能不是 32 倍数”）时保留 `DataCopyPad` 保证正确。
- 量化开销：无官方数字，等级 C。

---

## 5. 三种硬件形态的推荐 tiling

> 通用前提（三形态共用）：
> 1. 并行核数取 **GetCoreNumAiv()（910B2=48）**，而非 AI Core 数（24）；否则向量核半数空闲，对“小 outer”形态致命。
> 2. 优先**单遍暂存**（驻留 value），将 GM 流量从 7·D·S 降到 5·D·S（见第 4 节）。
> 3. 开启 **BUFFER_NUM=2 双缓冲**，让 MTE2（搬入）/V（计算）/MTE3（搬出）流水重叠，隐藏搬运延迟。
> 4. 多核均衡用候选的“前 extra 个核多 1 行”法（`count=each+(block<extra?1:0)`）。

### 形态 A：小 outer、大 D（outer ≤ 向量核数，D = 32768）

- **理由**：outer ≤ 48（甚至 ≤24），行级并行充足，每个核处理 ≤1~数行；瓶颈在**单行内 GM 流量大**与**标量同步/流水断流**。单遍驻留 value 可将每行 GM 流量从 448KB（fp16）降到 320KB；双缓冲隐藏 MTE 延迟；UB 余量充足（108KB→可到 152KB）。
- **tile 建议**：沿 D 切 tile=4096（fp16/bf16）/2048（fp32），与候选一致即可（UB 足够）。若采用单遍驻留整行 value（128KB）+ 双缓冲队列（x/residual 不再需要，gamma/bias/output 各 8KB），tile 仍可保持 4096。
- **风险**：
  - (a) 若 host 以 AI Core 数（24）启动 blockDim，48 向量核中半数空闲 → outer≤24 时实际只用 ≤24 核，浪费一半；若 outer 介于 24~48，部分行串行排队。**必须核对 availableCoreNum = GetCoreNumAiv()**。
  - (b) 单行 D=32768 在 tile=4096 下分 8 个 tile，候选每 tile 做一次 `ReduceSum + V_S/WaitFlag + GetValue + S_V`，共 ~8 次标量同步/行；若维持两遍则每行 ~16 次。单行内标量停顿占比高，需尽量减少（单遍 + 消除 GetValue，见形态 B 技巧）。
  - (c) D 非 32B 对齐时 DataCopyPad 仍有开销（等级 C）。
- **真机验证方法**：`msprof op --aic-metrics=Occupancy,MemoryUB,PipeUtilization` 看单核 UB 占用与 V/MTE 流水占比；`ResourceConflictRatio` 看 SET_FLAG/WAIT_FLAG 占比；`msopst` 取 `op_summary_0_1.csv` 的 aicore 耗时。重点对比“两遍 vs 单遍”耗时差。

### 形态 B：大 outer、小 D（outer = 8192，D = 64）

- **理由**：每行极小（64 元素），outer 巨大 → 行级并行极佳（8192/48≈170 行/核，余数 32，均衡良好）；瓶颈在**每行固定开销**（标量同步、pipeline 启动）被放大 8192 倍，而非 tile 大小或 UB。每核仅 ~170 行，每行一次 tile（D<4096）。
- **tile 建议**：D=64 单 tile；**关键不是 tile 而是消除每行的标量读回**。做法：用 Vector 归约把整行和收敛到 UB 内 1 元素张量，随后 `Sqrt`(向量)→`Reciprocal`(向量) 得到 `1/rms`（保持在 UB），再 `Muls(value, value, recip)` 广播，**完全避免 `GetValue` 标量读回与 V_S/S_V 同步**。ReduceSum 内部标量同步仍可能存在，但去掉了候选显式叠加的那一次。
- **风险**：
  - (a) 小 D 下 Vector 单元利用率低（64 fp16 = 半 block，mask=64），存在算力浪费，但 GM 流量极小（单行仅 ~640B~896B），瓶颈在开销而非算力，优化方向是“减同步、减启动”而非“增大 tile”。
  - (b) 若仍用两遍 + GetValue，8192 行 × ~每行 1 次 GetValue（单 tile）≈ 8192 次标量停顿/核，绝对耗时小但占比高，可能成为相对主要耗时。
  - (c) 平衡：8192%48=32，前 32 核 171 行、后 16 核 170 行，差 <1%，可接受。
- **真机验证方法**：`msprof op --aic-metrics=ResourceConflictRatio,PipeUtilization` 对比“用 GetValue” vs “用 Vector Recip 广播”的 SET_FLAG/WAIT_FLAG 次数与流水断流；`msopst` 测绝对 aicore 周期。

### 形态 C：中间形态（outer ≈ 核数若干倍，D = 1024 / 4096）

- **理由**：行并行与单核内计算量均适中。D=1024 单 tile（<4096）；D=4096 单 tile（fp16）或 2 tile（fp32）。每行 GM 流量中等，标量同步每行 1 次（单 tile），被较多行摊销，瓶颈在“GM 流量 + 双缓冲是否开启”。
- **tile 建议**：保持 tile=4096/2048；**单遍驻留 value**（D=1024→4KB，D=4096→16KB，极小）；**BUFFER_NUM=2 双缓冲**隐藏 MTE 延迟；并行核数取 48。
- **风险**：
  - (a) 若 outer 仅为核数 2~4 倍（如 outer=100），余数均衡仍是候选法正确，但单核行数少、双缓冲收益取决于 tile 数（D=1024 仅 1 tile → 双缓冲仅能重叠“读下一行”与“算本行”，仍有效）。
  - (b) D=4096 fp32 时每行 2 tile，标量同步 2 次/行，需确认 ReduceSum 路径（BlockReduceSum+WholeReduceSum 组合比单纯 WholeReduceSum 快，官方最佳实践实测 8.44us vs 13us @256 float）。
- **真机验证方法**：`msprof op --aic-metrics=Occupancy,PipeUtilization,MemoryUB`；`msopst` 取 `op_summary_0_1.csv` 对比双缓冲开/关。

---

## 6. 同步与标量读回的成本分析

### 6.1 候选的同步路径（ReduceRow 每段 tile）

```
MakeValue → Mul(平方) → ReduceSum → PipeBarrier<PIPE_V>
→ SetFlag<V_S> → WaitFlag<V_S> → sum_.GetValue(0)   // 标量读回
→ SetFlag<S_V> → WaitFlag<S_V> → PipeBarrier<PIPE_V>
```
- `GetValue` 走 **PIPE_S（标量流水）**；WaitFlag(V_S) 由 Scalar 执行、见到标志为 0 则阻塞后续指令（昇腾社区《同步控制简介》）。
- 含义：Vector 算完 ReduceSum 结果在 UB，Scalar 必须等 V→S 同步后才能把该标量取走用于累加 `total`。这一步**强制 Vector 与 Scalar 会合**，打断 Vector/MTE 流水。

### 6.2 ReduceSum 内部标量同步（社区实测，等级 C 但高可信）

- 社区 Ascend C 开发文章实测：`ReduceSum` API 内部即含 scalar 的 SET_FLAG/WAIT_FLAG，“会阻塞流水”；改用 `BlockReduceSum`/`WholeReduceSum` 组合或手写二分 Add 树可缓解。
- 官方《归约指令最佳实践》确认：单次 `BlockReduceSum` 比 `WholeReduceSum` 执行更快；256 个 float 用“1×BlockReduceSum + 1×WholeReduceSum”= 8.44us，优于 2×WholeReduceSum（13us）与 3×BlockReduceSum（13.94us）。来源：昇腾社区最佳实践文档（等级 A 为官方口径，但 8.44us 为社区实测量级）。

### 6.3 放大效应（按形态）

- **小 outer 大 D（形态 A）**：D=32768、tile=4096 → 8 tile/行。候选两遍 → 每行 8 次 ReduceSum（各含内部标量同步）+ 8 次显式 GetValue = 显著串行化。单行计算量小，标量停顿占比被放大。→ 应单遍 + 减少 tile 数（增大 tile 到 UB 允许上限，如 fp16 用整行 32768 单 tile 做归约，但受 mask/repeat 限制需分 repeat，归约本身仍多级）。
- **大 outer 小 D（形态 B）**：每行 1 tile，但行数 8192 → 标量停顿被“行数”放大（~8192 次/核）。→ 应消除 GetValue（Vector Recip 广播）。
- **中间形态（形态 C）**：每行 1~2 tile，行数中等，标量停顿被行数摊销，相对可控。

### 6.4 建议

- 归约用 `BlockReduceSum`+`WholeReduceSum` 组合（官方推荐）替代裸 `ReduceSum`。
- 尽量把 rms/recip 留在 UB 内做 Vector 广播，避免 `GetValue` 标量读回（尤其形态 B）。
- 仅在对正确性必需时插入 `SetFlag/WaitFlag`；候选的 V_S/S_V 在“单遍驻留 value”后可省（value 已在 UB，无需标量读回累加）。

---

## 7. 性能测量方法（命令级）

### 7.1 上板调优（msprof op）——需昇腾 NPU 环境

```bash
# 单算子上板采集（blockdim 由算子自身决定，或 app 自带参数）
msprof op --application=./add_rms_norm_bias_npu \
  --aic-metrics=Occupancy,Memory,MemoryUB,PipeUtilization,ResourceConflictRatio \
  --output=./prof_out

# 说明：
#  Occupancy     —— 核间负载均衡图（Atlas A2 支持）；若最慢/最快核差距>10% 即负载不均
#  MemoryUB      —— UB 访存/容量热点
#  PipeUtilization —— 各流水(V/MTE2/MTE3)耗时占比，定位瓶颈在搬运还是计算
#  ResourceConflictRatio —— 展示 SET_FLAG/WAIT_FLAG 占比（Atlas A2 支持），定位标量同步开销
#  Roofline       —— 需 simulator 配合，且要求源码含 assert；仅 Atlas A2 支持
```
- 可视化：将生成的 `trace.json` 拖入 Chrome `chrome://tracing`，或导入 MindStudio Insight 看通算流水图、指令流水图、Roofline。
- 来源：昇腾社区《工具使用》《工具概述》（msProf），访问 2026-09-11，等级 A。

### 7.2 仿真调优（无 NPU 时唯一可用，但仅供参考）

```bash
msprof op simulator --soc-version=Ascend910B2 \
  --application=./add_rms_norm_bias_npu blockdim 1 \
  --aic-metrics=PipeUtilization \
  --output=./sim_out
# 注意：需 export LD_LIBRARY_PATH=${INSTALL_DIR}/tools/simulator/Ascend910B2/lib:$LD_LIBRARY_PATH
# 官方明确：“仿真结果仅供参考，算子真实运行情况以用户实际仿真数据为准”
```

### 7.3 单算子耗时（msopst）

```bash
# 1) 由 Host 侧 .cpp 生成 ST 用例定义
msopst create -i add_rms_norm_bias.cpp -out ./st

# 2) 编辑 msopst.ini：performance_mode = True（默认 False，仅精度）
#    vim ${INSTALL_DIR}/python/site-packages/bin/msopst.ini

# 3) 执行（指定 SoC 与设备）
msopst run -i ./st/AddRmsNormBias_case_*.json \
  -soc Ascend910B2 -out ./out -d 0

# 4) 性能结果：运行成功后生成
#    ./out/run/out/prof/JOBxxx/summary/op_summary_0_1.csv
#    含 aicore 执行周期/耗时，用于横向对比不同 tiling
```
- 来源：昇腾社区《生成/执行测试用例》、MindStudio Ops System Test 快速入门，访问 2026-09-11，等级 A。
- 注意：`msopst.ini` 默认 FP16 精度模式；换精度需改 `--precision_mode`。`err_thr` 控制精度阈值。

### 7.4 负载均衡快速判据（形态 A/B/C 通用）

- 用 `Occupancy` 指标：各物理核“耗时/吞吐量/Cache 命中率”对比，最大最小差 >10% 即不均。候选的均衡分配法正确，不均通常源于 `blocks` 取错（见 5.A 风险 a）。

---

## 8. 未验证清单

| 项 | 状态 | 说明 |
|---|---|---|
| GM→UB（MTE2）单核实测带宽 | **D 未证实** | 仅有聚合 HBM 上界（~1600 GB/s，910B），非核内 DMA 实测；不可用于耗时估算 |
| ReduceSum 内部标量同步精确延迟 | **C 社区实测** | 社区确认“阻塞流水”，但无纳秒级官方数字 |
| DataCopyPad 相对 DataCopy 量化开销 | **C 社区经验** | arXiv 经验库称对齐时用 DataCopy 更快，无官方百分比 |
| 910B 是否公开 datasheet 写明 UB=192KB | **B 源码级** | 华为未发完整 datasheet；数字来自 CANN `NPUTargetSpec.td` 与编译校验 |
| GetCoreNumAiv() 在 CANN 9.0.0 对 910B2 的返回值 | **B 预期=48** | 架构 2×AIV/AI Core；待真机 `npu-smi`/API 确认 |
| 双缓冲（BUFFER_NUM=2）对 AddRmsNormBias 的实际加速比 | **未实测** | 逻辑上隐藏 MTE 延迟，但比例依赖 tile 数与 D |
| 单遍驻留 value 的 UB 峰值（含双缓冲） | **B 计算可行** | 见 3.2，D≤32768 均 <192KB；双缓冲队列另计余量 |

---

## 9. 来源表

| # | 标题 / 出处 | URL | 访问日期 | 等级 |
|---|---|---|---|---|
| S1 | 昇腾社区《通用参数说明》（256B/repeat，mask 128/64） | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/developmentguide/opdevg/Ascendcopdevg/atlas_ascendc_10_0021.html | 2026-09-11 | A |
| S2 | 昇腾社区《工具概述 / msProf》（msprof op 参数、Occupancy、Roofline） | https://www.hiascend.com/doc_center/source/zh/mindstudio/700/ODtools/Operatordevelopmenttools/atlasopdev_16_0082.html | 2026-09-11 | A |
| S3 | 昇腾社区《工具使用 / msprof op》 | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/devaids/optool/atlasopdev_16_00851.html | 2026-09-11 | A |
| S4 | 昇腾社区《生成/执行测试用例 / msopst》 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha001/devaids/optool/atlasopdev_16_0033.html | 2026-09-11 | A |
| S5 | MindStudio Ops System Test 快速入门 | https://mindstudio-operator-tools-docs.readthedocs.io/zh-cn/latest/msopgen/source/quick_start/msopst_quick_start/ | 2026-09-11 | A |
| S6 | 昇腾社区《选择低延迟指令，优化归约操作性能》（BlockReduceSum/WholeReduceSum 实测） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850/opdevg/Ascendcopdevg/atlas_ascendc_best_practices_10_0031.html | 2026-09-11 | A |
| S7 | 昇腾社区《针对不同场景合理使用归约指令》 | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0034.html | 2026-09-11 | A |
| S8 | 昇腾社区《同步控制简介》（PIPE 类型、SetFlag/WaitFlag 语义） | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/API/ascendcopapi/atlasascendc_api_07_0179.html | 2026-09-11 | A |
| S9 | 昇腾社区《GetBlockNum》API（多核逻辑） | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/apiref/ascendcopapi/atlasascendc_api_07_0173.html | 2026-09-11 | A |
| S10 | 昇腾社区《Ascend C 算子性能优化实用技巧 02——内存优化》（UB 角色、UB 融合） | https://www.hiascend.com/developer/techArticles/20240823-1 | 2026-09-11 | A |
| S11 | CANN `NPUTargetSpec.td` 硬件规格整理（910B UB=192KB，24/48 核，310B=256KB） | https://blog.csdn.net/gitblog_01164/article/details/153903579 | 2026-09-11 | B |
| S12 | ascend-rs 编译校验报错 “UB limit of 196608 bytes (192KB) for 910B / 256KB for 310P” | https://ascend-rs.org/en/appendix/appendix-d-ecosystem.html | 2026-09-11 | B |
| S13 | arXiv ENEC 论文（UB “e.g., 192KB”，910B2=24 AIC/48 AIV） | https://arxiv-vanity.com/papers/2604.03298 | 2026-09-11 | B/C |
| S14 | arXiv 2505.15112（达芬奇架构：每 AI Core 1 AIC + 2 AIV，UB 为 AIV 工作区） | https://arxiv.org/pdf/2505.15112v1 | 2026-09-11 | B/C |
| S15 | CSET 对 910/910B HBM 带宽整理（910:1228 GB/s；910B:1600 GB/s） | https://cset.georgetown.edu/publication/pushing-the-limits-huaweis-ai-chip-tests-u-s-export-controls/ | 2026-09-11 | C |
| S16 | aiwiki（华为未发完整 910B datasheet；数字多为估算） | https://aiwiki.ai/wiki/huawei_ascend_910b | 2026-09-11 | C |
| S17 | 社区：ReduceSum 内部标量同步阻塞流水（实测经验） | https://ascendai.csdn.net/69d7935172111d255bf8761f.html | 2026-09-11 | C |
| S18 | arXiv AscendOptimizer 经验库（DataCopyPad 较 DataCopy 慢，对齐用 DataCopy） | https://arxiv-vanity.com/papers/2603.23566 | 2026-09-11 | C |
| S19 | DeepWiki 多核执行 / 同步机制（GetBlockIdx/GetBlockNum、SetFlag/WaitFlag） | https://deepwiki.com/xufan-0118/ascend-pooling/6.4-multi-core-execution 与 /6.3-synchronization | 2026-09-11 | C（代码库文档） |
| S20 | 华为 2025 昇腾生态大会整表（910B HBM 1.6TB/s，FP16 320TFLOPS） | https://blog.csdn.net/duke_zhang2024/article/details/163162620 | 2026-09-11 | C |

---

### 附：与候选实现（V002/kernel.asc）的关键对照

- 候选 BUFFER_NUM=1（无双缓冲）→ 建议改 2 以重叠 MTE/V。
- 候选两遍扫描 → 建议单遍驻留 value，GM 流量 7→5（−28.6%）。
- 候选每 tile 显式 `GetValue` 标量读回 → 建议归约结果留 UB，用 Vector `Sqrt`+`Recip` 广播消除（形态 B 尤甚）。
- 候选归约用裸 `ReduceSum` → 建议 `BlockReduceSum`+`WholeReduceSum` 组合（官方实测更快）。
- 候选 `blocks` 来自 host `availableCoreNum` → 必须确认其 = `GetCoreNumAiv()`(48)，否则形态 A 半数向量核空闲。
- 候选全用 `DataCopyPad` → D 满足 32B 对齐时切 `DataCopy` 提速（等级 C，需真机验证）。
