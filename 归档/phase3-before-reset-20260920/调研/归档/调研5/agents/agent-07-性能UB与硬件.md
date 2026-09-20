# Agent 7 调研报告：性能、UB 与硬件架构（AddRmsNormBias）

- 任务：纯调研（无 NPU 环境，不做任何真实测量）
- 目标环境：CANN 9.0.0，SoC `dav-2201`（Atlas A2 / Ascend 910B 系，`__CCE_AICORE__==220`），vector 核函数直调 `kernel.asc`
- 报告日期：2026-09-12
- 证据分级：A=官方文档/官方指南；B=官方源码/官方样例（含 A2 实测数据）；C=社区实测帖/推导；D=仅摘要
- 说明：API 签名核对见 Agent 2 报告，样例工程细读见 Agent 3 报告，本文不重复；本文聚焦硬件数字、成本模型、多核策略与测量方法。所有"推断"均显式标注。

---

## 1 调研范围与覆盖

| 主题 | 覆盖情况 | 关键结论位置 |
|---|---|---|
| UB 容量与结构（A2） | ✅ 官方数字齐备（A 级） | §2.1 |
| AI Core 数量与向量吞吐 | ✅ 官方样例+文档交叉验证；判题 SKU 核数未确认 | §2.2 |
| GM↔UB 搬运成本（DataCopyPad vs DataCopy、分块、带宽） | ✅ 官方样例 A2 实测（B 级） | §2.3、§3.1 |
| ReduceSum/WholeReduceSum 成本模型 | ✅ 官方性能指南（A 级） | §3.3 |
| 多核负载均衡 | ✅ 官方样例+指南；结合 ops-nn tiling（前序已确认） | §4 |
| 两遍 vs 单遍访存对比 | ✅ 静态模型（推断，标注） | §3.2 |
| 标量同步成本 | ⚠️ 机制有 A 级证据；绝对时延未找到官方数字 | §3.4 |
| 双缓冲/多级流水 | ✅ 官方指南+样例实测 | §3.5 |
| msprof 测量方法 | ✅ 官方工具文档（A/B 级） | §6 |
| TBest 反推测试点形态 | ✅ 推断（显式标注） | §5 |

未找到官方数字的项目在 §7 汇总。

---

## 2 硬件规格数字表

### 2.1 Unified Buffer（A2 / dav-2201，A 级）

| 项目 | 数字 | 来源与等级 |
|---|---|---|
| UB 总容量 | **192KB（196608 B）** | A：官方《Vector逻辑架构》（Atlas A2/A3 段落）："UB总大小为192KB" |
| bank 结构 | **16 个 bank group × 3 bank = 48 bank**；每 bank 4KB = 128 行 × 32B/行 | A：同上；A：《避免bank冲突（NPU架构版本2201）》："192K，划分为48个bank……16个bank group，每个bank group包含3个bank" |
| bank group 排布 | bank15/bank31/bank47 同属一个 bank group（跨段同位 bank 组成一组的交错排布） | A：同上 |
| 访问吞吐 | **Vector 每拍（每指令周期）每 bank group 读写一行 32B** | A：同上 |
| 冲突类型 | 读写冲突（同 bank）；写写/读读冲突（同 bank group，含单指令 8 个 DataBlock 同 BG 排队、一拍拉长为最多 8 拍） | A：同上（含具体地址示例） |
| UB 地址对齐 | 32B（src/dst 均要求） | A：通用地址对齐约束（Agent 2 已核） |
| 官方模板 InitBuffer 上限 | 195584 B（=191KB=192KB−1KB） | B：ops-nn add_rms_norm SINGLE_N（前序 Agent 已核，实测 InitBuffer 值） |
| L1/L0A/L0B/L0C（参照） | L1 512KB、L0A/L0B 64KB、L0C 128KB（Cube 用，vector 直调基本无关） | B：cann-outreach（A2=910B2 行） |
| L2 Cache | **192MB**（A2/A3） | A/B：官方 DataCopy 样例 README |
| 950PR 对比 | UB 256KB（8 BG × 2 bank × 16KB；cann-outreach 记 248KB，疑为可用容量与总量口径差，存疑） | A：Vector逻辑架构；B：cann-outreach |

**对 AddRmsNormBias 的直接推论（推断）**：x/residual/y/gamma 四块 UB 地址若彼此间距是 12KB（=192KB/16）的整数倍，Vector 双源读会落在同一 bank group 触发读读冲突；官方推荐手段是"多申请一行 UB + 调整步长"（A 级指南原例：NZ 矩阵多申请 256B、步长 144→145，单条 Copy 从 8 拍恢复 1 拍）。

### 2.2 AI Core 数量与向量吞吐

| 项目 | 数字 | 来源与等级 |
|---|---|---|
| 910B1/B2 | AIC 24 / **AIV 48** | B：官方 DataCopy 样例 README（A2 上 GM→UB 用 `Block Num=48`）、Add 样例"切分48份"；C：CSDN 转述 cann/runtime platform_config（ai_core_cnt 24 / vector_core_cnt 48） |
| 910B3/B4 | AIC 20 / **AIV 40** | B：HAMi 官方 ascend-device-plugin configmap（910B3 aiCore: 20、910B4 aiCore: 20）；A：hiascend SetDim 文档离散架构示例"SetBlockDim 20 … SetDim 40 表示按 40 个 AIV 切分" |
| GetBlockNum 语义 | 返回逻辑核数=numBlocks；仅启动 AIV 时实际 AIV 数=numBlocks；AIC:AIV=1:2 时 AIC=numBlocks、AIV=2×numBlocks | A：官方 GetBlockNum 页 |
| blockDim 取值 | [1, 65535]，建议为物理核数或其倍数 | A：hiascend 核函数文档 |
| 判题环境（dav-2201）具体核数 | **未确认**——题面只给 SoC 版本，未给 SKU；910B 系 B1~B4 均映射 dav-2201 | 未找到 |
| 频率 | 910B2 1.8GHz | B：cann-outreach |
| 单 AIV 向量并行度 | half/int16：**128 element/cycle**；float/int32：**64 element/cycle**（Add/Mul/Muls/Adds/Max/Min 等）；Exp/Ln/Div/**Sqrt：32 element/cycle**；**Rsqrt：half 128 / float 64**；DataCopy(UB→UB) 256 B/cycle | A：官方《Vector指令理论性能汇总》（Atlas A2/A3 段表 6-8） |
| 整卡 Vector 算力 | fp16 ≈ 22T（910B2）；fp32 ≈ 11.06T | B：cann-outreach（22T）；C：cann-learning-hub（11.06T fp32），与"48 AIV × 1.8GHz × 128/64 elem × 2 FLOP"自洽（22.1T/11.06T，推断交叉验证）。另有 ops-transformer 迁移帖记 23.5T（C 级），疑为不同频率口径，**存疑** |

**对 AddRmsNormBias 的直接推论（推断）**：
- fp32 计算路径每元素 64/cycle，是 half 的一半——bf16 输入先转 fp32 计算时，Vector 吞吐减半，但本题全程带宽受限（见 §3.1），通常不构成瓶颈。
- `Sqrt` 只有 32 element/cycle（Add 的 1/4 吞吐），但每行仅执行一次 `sqrt(mean+eps)`，行均摊后可忽略；`Rsqrt` 吞吐 128/64 element/cycle 更快，可考虑 `rstd = alpha*Rsqt(mean+eps)`（Nova 指令带比例系数），但**Rsqrt 为硬件近似指令，fp32 1e-4 精度风险必须实测验证**（未验证，见 §7）。
- 归一化+缩放+偏置三步（Muls/Mul/Add，fp32）≈ 每元素 3×(1/64) cycle，远低于搬运每元素耗时 → 计算不是瓶颈，搬运是。

### 2.3 GM/HBM 带宽与搬运效率（官方样例 A2 实测，B 级）

来源：asc-devkit 官方 `data_copy` / `add_high_performance` 样例 README，性能数据均在 Atlas A2 训练系列上运行。

| 项目 | 数字 | 说明 |
|---|---|---|
| GM 峰值带宽（估算口径） | **≈1.8TB/s** | 官方样例与 cann-learning-hub 教程一致引用；Matmul 样例另记 HBM≈1.6TB/s、L2≈5TB/s（B 级，两口径并存，标注差异） |
| 大块搬运实测（GM→UB，48 AIV） | 301.99MB 用 `aiv_mte2_time`=202.657μs → 有效带宽 ≈**1.49TB/s**（推导，C 级），高于理论值 20.8% 以内 | 单次 DataCopyPad 131072B（Tile=[64,1024]，blockCount=64, blockLen=2048B） |
| 中块（8192B/次） | MTE2 220.772μs，较大块 **+148.3% 耗时提升**（548→221μs 量级改善） | Tile=[64,64] |
| 小块（128B/次，逐行 1 行） | MTE2 548.161μs，**占 Task 98.9%**；比大块慢 2.7 倍 | Tile=[1,64]——"大 outer 小 D 逐行搬"的极端反面教材 |
| **非对齐代价（A2）** | N=12288→12287（尾块 blockLen 2048B→2046B，仅差 2B）：GM→UB 端到端 **-21.6%**（215.82→275.30μs）；GM→L1 **-47.5%** | A2 上非对齐显著劣化；同场景 950PR 仅 -1.8%/-7.8% |
| 对齐建议 | **A2/A3 主搬运维度连续字节数建议 512B 对齐**；950 建议 128B | 官方样例结论 |
| 单核 MTE2/MTE3 带宽（推导，C 级） | Add 样例 Case1（单 AIV、8KB 块串行）：读 256MB 用 MTE2 6208μs ≈ **41GB/s**；写 128MB 用 MTE3 2613μs ≈ 49GB/s | 未找到官方"单 AIV MTE 峰值"数字，此为 B 级数据推导 |
| MTE2/MTE3 共享带宽 | "MTE2/MTE3 同时读写 GM 时，流水耗时 ≈（MTE2 量+MTE3 量）/GM 带宽" | B：cann-learning-hub 教程（08 性能优化章） |
| L2 bypass 收益 | 只读一次的输入设 `SetL2CacheHint(CACHE_MODE_DISABLE)`：Add 样例 Case4→Case5 端到端 **-28.9%**（264.02→187.68μs） | B：官方 Add 样例（8192×8192 half，48 AIV） |
| 多核同地址访问 | DataCopy 样例含"多核同时访问相同 GM 地址段冲突"场景（gamma 预载同段相关），建议按核错开访问 | B：官方样例（本章未逐数字展开，方法有效） |

---

## 3 访存与计算成本模型

### 3.1 基本流量模型（推断，基于上述 B 级带宽）

设 fp16、D=最后一维、outer=行数，每元素：

| 方案 | GM 读 | GM 写 | 合计流量/元素 |
|---|---|---|---|
| 单遍（y 留 UB） | x 2B + residual 2B | output 2B | **6B** |
| 两遍（y 走 GM 中转，ops-nn SPLIT_D 型） | x 2B + residual 2B + y 2B | y 2B + output 2B | **10B（+67%）** |
| bf16 同单遍 | 4B | 2B | 6B |
| fp32 同单遍 | 8B | 4B | 12B |

- gamma/bias 每核预载一次（D×B×2，48 核 D=32768 时 fp16 合计 6MB，192MB L2 可全容纳且各核读同段 → L2 命中，代价可忽略；D 较小时更是常数项）。
- fp16 单遍、有效带宽 1.5TB/s 时，每 μs 可处理约 0.25M 元素（48 AIV 聚合）；此斜率用于 §5 反推。
- 结论：**能单遍就单遍**；两遍仅当 D 超出 UB 可容纳（fp16 单行 y≈64KB@D=32768 其实能放下，需与双缓冲输入竞争 192KB——见 §3.5）或双缓冲+多行合并挤不下时才用，付出的代价是 +67% GM 流量。

### 3.2 尾块与非对齐

- GM→UB 方向 `DataCopyPad`（A 级，Agent 2 已核）：blockLen 非 32B 对齐时框架在 UB 侧补 dummy；A2 实测非对齐端到端 -21.6%（B 级）。
- UB→GM 方向 dummy 落 GM 时被丢弃，不踩相邻行（A 级，Agent 2 定案）。
- **策略（推断）**：D 非 512B 对齐时，主循环按 512B 对齐粒度切、尾块单独走 DataCopyPad，是 A2 上必须做的（对比直接全行 Pad 搬运的 -21.6%）。
- 950 的 Compact 模式（多块非对齐合并搬运省带宽）**A2 不支持**（A 级：该指南明确"适用于 Ascend 950PR/950DT"）——A2 上减少无效搬运只能靠 blockCount 多行合并 + 对齐切分。

### 3.3 归约成本模型（A 级，官方《选择低延迟指令，优化归约操作性能》）

| 方案 | 相对成本 |
|---|---|
| `ReduceSum` 接口 | 由多种指令组合实现，**三者中最慢**（官方原文："数据量较大、循环次数较多的场景，二分累加方案 > ReduceRepeat 单指令 > ReduceSum 接口"） |
| `ReduceRepeat`/`WholeReduceSum` 单指令 | **延迟约为 Add 指令的 2-5 倍**（官方单指令测试口径） |
| **二分累加（Add 折半折叠 + 末端 ≤256B 时 ReduceRepeat）** | 实测 float 30000 元素：**172 cycle vs ReduceRepeat 242 cycle**（-29%） |

- `asc_datablock_reduce_sum`（A 级 C API，A2 支持）：每 DataBlock（16 half/8 float）内二叉树求和，适用于"每 32B 块各自成和"的形态。
- **对 AddRmsNormBias 的推论（推断）**：sum(y²) 用 `Mul→二分 Add 折叠→末端一次 ReduceRepeat/WholeReduceSum`（官方 ops-nn 写法即"Add 折叠+WholeReduceSum 两级"的变体，前序已确认）；直接一次 `ReduceSum(count=D)` 在大 D 下明显吃亏。注意 30000 元素实测是 UB 内数据；GM→UB 搬运与归约的重叠（MTE2 与 V 并行）才是大头。

### 3.4 标量同步成本

- 机制（A 级《Scalar读写数据》+《关键特性说明》）：`GetValue/SetValue` 属 PIPE_S；**算子工程默认自动同步时无需手动 SetFlag/WaitFlag**（与本项目前序结论"直调工程默认自动同步下手动插入 PipeBarrier<PIPE_V> 冗余"一致）；手动同步示例为 S_V/S_MTE3 事件对。
- 每行取一次 rstd 标量（V_S 等待→S 侧 GetValue→S_V 通知→Muls 广播）的**绝对时延：未找到官方数字**（§7）。已知：Scalar 读 GM/UB 有 64B DataCache 行缓冲（A 级），单次 GetValue 走 UB 路径；GetBlockNum 等系统变量类 1 cycle/条（A 级指令性能表）。
- MERGE_N 多行合并一次归约、rstd 批量化后每标量成本按行数摊薄（B 级 ops-nn tiling 语义，前序已确认）；WholeReduceSum 输出直接是每行一个元素的张量，配合 Muls 逐列乘可避免逐行取标量（推断）。

### 3.5 双缓冲/多级流水（A/B 级）

| 证据 | 数字 | 等级 |
|---|---|---|
| 官方 DoubleBuffer 指南 | CopyIn/Compute/CopyOut 分属 MTE2/V/MTE3 队列天然可并行；**"数据搬运时间短、计算时间长时收益偏小；数据量小强行双缓冲可能适得其反"** | A |
| Add 样例 Case3→Case4（带宽受限型） | 双缓冲仅 **-1.7%**（268.5→264.02μs）；MTE2/MTE3 计时反增（读写共享 GM 带宽） | B |
| cann-samples rms_norm_quant_story（950PR 实测，前序已确认） | 双缓冲 **1.55x**；UB 多行 1.11x（MTE2 次数 -71%）；gamma 预载 1.13x | B |
| TQue depth=2 vs TBuf+HardEvent | 官方 Add 样例 Case4 用静态 Tensor+手工 Ping-Pong 事件对（EVENT_ID0/1）实现；TQue `InitBuffer(que,2,len)` 一行开启 | B |

- **对 AddRmsNormBias 的推论（推断）**：本题是"搬运重、计算轻"（§2.2 推论），双缓冲收益取决于能否把 MTE2 从串行变并行——大流量测试点（§5 TP9~15）预期接近 rms_norm 的 1.5x 量级而非 Add 的 1.7%；小测试点（TP1~3）任务总时长 1.5~2.5μs，双缓冲的第二块搬运启动开销可能反而劣化，**小点应单遍单缓冲、少指令**。
- UB 预算示例（fp16，D=32768，推断）：单遍需要 x+y(=x+residual 结果)+residual 三块 64KB=192KB——恰好打满，**双缓冲做不了整行**；可行解：按 D 分段两遍（归约段+缩放段，y 分段落 GM 中转或分段重算），或 fp32 中间量只存 y 的 fp16 拷贝。ops-nn 在 D>12288 直接转 SPLIT_D 两遍（B 级，前序）与该 UB 预算一致。

---

## 4 多核与分块策略

| 主题 | 结论 | 来源与等级 |
|---|---|---|
| 切分方向 | **按行（M 方向）切**保持每核 GM 地址连续，可用单条 DataCopy 大块搬运；Add 样例明确"选 M 切 48 而非 M×N 网格，核心目的是 GM 连续" | B |
| 均匀切分 | `baseCoreM=totalM/splitM`，余数分给前 `remainderM` 个核（代码级模板） | B |
| 核间均衡判定 | 各核 `aiv_time` 差异 <10% 达标、>30% 严重不均 | B：cannbot-skills（官方 skills 仓） |
| 尾核不均 | outer 非 blockDim 整数倍时用上述余数分配；ops-nn 的 `latsBlockFactor`（前块行数因子）同理 | B/A（ops-nn 前序已确认） |
| **小 outer 大 D（outer<核数）** | 行级切分出现核闲置 → ops-nn SINGLE_N：每核 1 行手工事件流水（前序已确认）。补充：此时单核 MTE2 带宽（≈41GB/s 推导）成为上限，D=32768 fp32 单行 384KB 流量 ≈9μs/核，与 TP1~TP3 的 1.5~2.5μs TBest 量级吻合度低，说明小点 outer≥核数或 D 小（推断，见 §5） | B/C |
| **大 outer 小 D（行太小）** | 逐行 DataCopy 会退化成 DataCopy 样例场景 1（128B/次，-62% 带宽）→ 必须 **blockCount 多行合并**（一条 DataCopy 搬 N 行，srcStride=行距；或 MERGE_N 多行进 UB 连续存放后归约按行分段）；实测 8KB/次块即拿回 +148% | B |
| blockDim 取值 | 判题环境 SKU 未确认（20~48 AIV）：**kernel 内用 GetBlockNum() 自适应，host 侧不写死**；直调工程可在 host 用 PlatformAscendC 的 GetCoreNumAic/GetCoreNumVector 查询后设置 | A（GetBlockNum 文档） |
| gamma/bias 多核读 | 48 核读同一 D 段 → L2 命中；官方另有"多核同地址段访问错开"缓解手段 | B |

---

## 5 结合 TBest 反推 15 个测试点形态与分形态 tiling 推荐

> **以下全部为推断**：用 fp16 单遍 6B/元素、48 AIV 有效带宽 1.5TB/s（§3.1 斜率 0.25M 元素/μs）反推；TBest 为排名页 2026-09-12 抓取值。实际 dtype 混合（fp16/bf16/fp32 各占测试点）会使同流量点时间最多差 2 倍，反推只定量级、不定精确 shape。

| # | TBest | 反推流量（fp16 单遍口径） | 可能形态 | 推荐 tiling 思路 |
|---|---|---|---|---|
| 1 | 1.47μs | ≲0.4MB | 微型：outer 小、D 小（如 [1~8, 64~1024] 级） | 单遍、少核或按行平分；无归约分段、无双缓冲；指令数最小化（固定开销主导） |
| 2 | 2.16μs | ≲0.5MB | 微型偏大 | 同上；D 对齐时纯 DataCopy |
| 3 | 2.54μs | ≈0.6MB | 微型/小型 | 同上 |
| 4 | 6.80μs | ≈1.7MB | 小型：outer≈核数×k、D 中等 | 行级均分 + 每核整行单遍 |
| 5 | 5.70μs | ≈1.4MB | 小型（可能 fp32 或 bf16 小点） | 同上 |
| 6 | 12.31μs | ≈3MB | 中小 | 行级均分；gamma 预载一次 |
| 7 | 16.70μs | ≈4MB | 中小 | 同上 |
| 8 | 31.36μs | ≈8MB | 中型 | 行级均分 + 双缓冲（收益初现） |
| 9 | 50.61μs | ≈12MB | 中型 | 双缓冲 + 512B 对齐大块 |
| 10 | 47.59μs | ≈12MB | 中型（与 9 同级不同 dtype/形态） | 同 9；若大 outer 小 D → MERGE_N |
| 11 | 133.40μs | ≈33MB | 中大；或 D=32768 行数少（两遍型） | 大 D 检查 UB 预算：放得下单遍，放不下 SPLIT_D 两遍 |
| 12 | 76.68μs | ≈19MB | 中大（小于 11，可能大 outer 小 D 多行合并型） | blockCount 多行合并 + 归约分段 |
| 13 | 411.34μs | ≈100MB | 大型（outer×D 均较大） | 全套：行均分 + 双缓冲 + L2 bypass + bank 冲突检查 |
| 14 | 3.75ms | ≈0.9GB | 巨型（outer 8192×D 32k 级组合的一部分） | 带宽受限极限优化：SPLIT_D/两遍取舍重算、每核行数均衡、SetL2CacheHint(DISABLE) 于 x/residual |
| 15 | 8.66ms | ≈2.2GB | 巨型（接近 outer 上限形态） | 同 14；确认两遍的 +67% 流量是否可换回单遍（分段 y 复用） |

**跨点通用策略（按证据强度排序）**：
1. 行级均分 + 余数分配（B）；2. gamma/bias 预载一次（B，rms_norm 1.13x）；3. 主搬运 512B 对齐、尾块 DataCopyPad（A/B，-21.6%）；4. 多行合并避免小 DMA（B，+148%）；5. 归约用 Add 折叠+末端 ReduceRepeat（A，172 vs 242 cycle）；6. 大点双缓冲（B，1.55x）+ L2 bypass 输入（B，-28.9%）；7. 小点禁双缓冲（A 指南反例）；8. UB 布局查 bank group（A）。

---

## 6 性能测量方法（真机阶段使用；本机未测）

### 6.1 msprof op（算子级）

- 拉起：`msprof op --output=<dir> --warm-up=10 --launch-count=5 ./app`（直调可执行）；`--kernel-name` 前缀匹配、`--launch-skip-before-match` 跳过前 N 个（A：hiascend msProf 工具概述；B：cannbot-skills）。
- **`--warm-up` 是必须项**：短任务达不到芯片提频最小耗时会触发降频，污染数据（B：cann-learning-hub 08 章，官方原话）。
- L2 维度采集：`msprof --ai-core=on --aic-metrics=L2Cache`（B：官方 DataCopy 样例用法）。
- 输出 8 个 CSV：`OpBasicInfo.csv`（Task Duration、Block Dim、Current/Rated Freq）、`PipeUtilization.csv`（MTE2/VEC/Scalar 各流水占比，逐核）、`ResourceConflictRatio.csv`（**bank conflict 占比**）、`Memory.csv`、`L2Cache.csv`、`ArithmeticUtilization.csv`、`MemoryUB.csv`、`MemoryL0.csv`（B：cann-learning-hub 实测目录结构）。
- 关键字段（A2 口径）：`aiv_vec_ratio / aiv_scalar_ratio / aiv_mte2_ratio / aiv_mte3_ratio / aiv_time`（B：官方 Add 样例字段表）。
- 仿真：msopprof 支持 CPU 仿真模式（msOpProf 用户指南）；bank 冲突占比可用 msOpProf 采集（A：bank 冲突指南页脚）。

### 6.2 瓶颈判定流程（B：cannbot-skills / cann-learning-hub）

1. 读 `OpBasicInfo.csv` 取 Task Duration 与 Block Dim（Block Dim 应等于可用 AIV 数）；
2. 读 `PipeUtilization.csv` 找最高占比流水；Elementwise 类预期 vec_ratio 50-80%，归约类 40-70%；
3. 理论耗时 = 流量/1.8TB/s（或计算量/11.06TOPS@fp32）；
4. 差距 <20% 视为接近硬件极限，20-50% 有优化空间，>50% 必须优化；核间 `aiv_time` 差 >30% 查负载均衡。

### 6.3 iterations=5 统计口径风险（真机验证时必须注意）

- 判题 iterations=5 的聚合方式（均值/中位数/最小值）**未在题面确认**——若为均值，首跑冷缓存与 DVFS 未提频会拖分；msprof 侧对应做法是 `--warm-up` + `--launch-count=5`（B）。
- 1.47μs 级测试点处于降频敏感区（B：warm-up 说明），本地复现必须带预热，否则本地数据与判题 TBest 不可比。
- 社区参照（C）：910B4 上端到端单 kernel launch 下限约 280μs（Mamba2 实验报告）——说明**判题 TBest 是纯 kernel 时间口径而非端到端**，本地对齐口径时用 msprof 的 Task Duration 而非 wall time。

---

## 7 风险与未确认事项

1. **判题环境 SKU/AIV 数未确认**：dav-2201 覆盖 910B1~B4（AIV 48 或 40）。核数只影响多核切分基数，用 GetBlockNum() 运行时自适应可消除该风险；host 侧也应查询而非写死。
2. **GM 带宽双口径**：1.8TB/s（样例/教程）与 1.6TB/s HBM（Matmul 样例）并存；有效带宽 1.49TB/s（大块实测推导）用于反推。§5 的流量反推误差据此约 ±15%，另受 dtype 混合影响可达 2 倍。
3. **Vector 整卡算力 22T vs 23.5T** 两说（B/C 级冲突，`contradicted`）；对本题无直接影响（计算非瓶颈）。
4. **V_S/S_V 标量事件绝对时延**：未找到官方数字（未找到官方数字）。MERGE_N 摊薄是官方写法（B），小点每行一次取标量的代价只能真机测。
5. **Rsqrt 精度**：fp32 1e-4 判据下未验证；若用需 15 点全量精度实测。
6. **Add 样例 Case 6（bank conflict 优化）的完整性能表未获取**（转载页截断，raw 404）；仅方法与样例路径（`asc-devkit/examples/.../05_best_practices`）可引用。UB 四 buffer 布局对 bank group 的实际冲突需真机用 ResourceConflictRatio.csv 验证。
7. **Compact 搬运模式 A2 不支持**（A），非对齐优化手段仅剩对齐切分+多行合并。
8. 本报告一切性能数字均来自官方样例实测/文档/社区帖，**不代表本题 AddRmsNormBias 已达任何性能水平**；§5 形态反推为推断。

---

## 8 来源登记表

| # | URL | 标题 | 版本/仓库 | 访问日期 | 用途 | 证据等级 | 状态 |
|---|---|---|---|---|---|---|---|
| S1 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/Vector逻辑架构/Vector逻辑架构.html | Vector逻辑架构（SIMD-API） | asc-devkit 文档（gitcode 镜像） | 2026-09-12 | A2 UB 192KB/16BG×3bank/行 32B；950 256KB 结构 | A | verified |
| S2 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/内存访问/避免UB的bank冲突/avoid_bank_conflict_npu_arch_2201.html | 避免bank冲突（NPU架构版本2201） | asc-devkit 指南 | 2026-09-12 | 48 bank/16BG/每拍每BG一行；冲突类型与地址优化法；msOpProf | A | verified |
| S3 | https://asc.gitcode.com/api/附录/Vector指令理论性能汇总.html | Vector指令理论性能汇总 | asc-devkit 文档 | 2026-09-12 | Add/Mul 128(half)/64(float) elem/cycle；Sqrt 32；Rsqrt 128/64；DataCopy UB→UB 256B/cycle | A | verified |
| S4 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | 选择低延迟指令，优化归约操作性能 | asc-devkit 指南 | 2026-09-12 | ReduceRepeat 延迟 2-5×Add；二分累加 172 vs 242 cycle；ReduceSum 接口最慢 | A | verified |
| S5 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/流水编排/使能DoubleBuffer.html | 开启DoubleBuffer | asc-devkit 指南 | 2026-09-12 | MTE2/V/MTE3 队列并行原理；双缓冲反例（小数据/搬运占比低时收益小或反效果） | A | verified |
| S6 | https://asc.gitcode.com/guide/技术附录/概念原理和术语/内存访问原理/Scalar读写数据.html | Scalar读写数据 | asc-devkit 指南 | 2026-09-12 | GetValue/SetValue=PIPE_S；DataCache 64B 行；自动同步语义与手动事件示例 | A | verified |
| S7 | https://asc.gitcode.com/api/SIMD-API/基础API/同步控制/核内同步/关键特性说明.html | 关键特性说明（自动同步） | asc-devkit 文档 | 2026-09-12 | 自动同步范围（SIMD 核内）；V_S 等 HardEvent 语义 | A | verified（部分正文截断，要点已获取） |
| S8 | https://asc.gitcode.com/api/SIMD-API/基础API/工具接口/系统资源与变量/GetBlockNum.html | GetBlockNum | asc-devkit 文档 | 2026-09-12 | numBlocks 语义；AIC:AIV=1:2 时 AIV=2×numBlocks | A | verified |
| S9 | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-kernel-function | 核函数（Ascend C） | HarmonyOS CANN Kit 文档 | 2026-09-12 | blockDim∈[1,65535]、逻辑核概念 | A | verified |
| S10 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlasascendc_api_07_0464.html | SetDim | CANN 8.0.RC2 商用版 | 2026-09-12 | 离散架构"20 AIC/40 AIV"示例（910B3 口径佐证） | A | verified |
| S11 | https://blog.csdn.net/gitblog_00924/article/details/157925888 | CANN/asc-devkit：DataCopy内存访问最佳实践样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | A2 实测：分块粒度 3 档（548/221/203μs）、非对齐 -21.6%/-47.5%、1.8TB/s 口径、L2 192MB、512B 对齐建议、Block Num=48 | B | verified |
| S12 | https://blog.csdn.net/gitblog_00754/article/details/157048953 | CANN/asc-devkit：Add性能调优样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | A2 实测 Case0-5：多核 306μs、大块 -12.5%、双缓冲 -1.7%、L2 bypass -28.9%；指标字段表；Case6 截断 | B | verified（Case6 数字缺失） |
| S13 | https://blog.csdn.net/gitblog_00445/article/details/151381043 | CANN/asc-devkit：Matmul最佳实践样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | HBM≈1.6TB/s、L2≈5TB/s（A2 口径）；AIC 侧指标字段 | B | verified（摘要为主，关键数字在正文可见） |
| S14 | https://blog.csdn.net/gitblog_00344/article/details/152106814 | CANN/cann-outreach：Atlas A2与A3架构对比 | CSDN 官方转载 cann-outreach | 2026-09-12 | A2=ASCEND910B/DAV_2201；910B2：24 Cube/1.8GHz/UB 192KB/Vector fp16 22T；L1/L0/UB/L2 容量表 | B | verified |
| S15 | http://raw.githubusercontent.com/Project-HAMi/ascend-device-plugin/refs/heads/main/ascend-device-configmap.yaml | HAMi ascend-device-configmap | HAMi 官方仓库 raw | 2026-09-12 | 910B2 aiCore 24、910B3/B4 aiCore 20（SKU 核数） | B | verified |
| S16 | https://bbs.huaweicloud.com/blogs/5a9291facf6e43148d16ef69a63dc97c | CANN学习资源开源仓的算子调试三msProf及仿真 | 华为云社区（cann-learning-hub 配套） | 2026-09-12 | msprof op 参数（warm-up/launch-count 1~5000）；8 个 CSV 清单；1.8TB/s 与 11.06TOPS 理论公式；MTE2/MTE3 共享带宽 | B | verified |
| S17 | https://blog.csdn.net/gitblog_00297/article/details/157004129 | CANN/cannbot-skills：上板性能采集与调优 | CSDN 官方转载 cannbot-skills | 2026-09-12 | msprof 采集命令模板；核间均衡 <10% 判定；Elementwise vec_ratio 期望；"910B: 20~40 核" | B | verified |
| S18 | https://ascendai.csdn.net/696c53eca16c6648a9832a4b.html | AscendC算子代码阅读指南 | CSDN（昇腾专区） | 2026-09-12 | ub_size=196608B=192KB；910B1~B4 核数表（引 cann/runtime platform_config）；SetFlag/WaitFlag 机制 | C | verified |
| S19 | https://github.com/hicann/cann-learning-hub/blob/master/quick_start/cann_basics/02_what_is_npu.ipynb | 02_what_is_npu.ipynb | hicann/cann-learning-hub | 2026-09-12 | 910B Vector=128 FP16 元素/周期 | C | partial（snippet 可见） |
| S20 | https://github.com/Aeolion-mu/ops/blob/main/mamba2_fusion_report.md | Mamba-2 2.7B 算子融合实验报告 | 社区仓库（北京昇腾研发部署记录） | 2026-09-12 | 910B4 kernel launch ~280μs 端到端下限（口径参照）；aclnnReduceSum 226μs 等参照 | C | verified |
| S21 | https://blog.csdn.net/jieph01/article/details/164574099 | AscendC DataCopyPad 32 字节对齐报错：尾块搬运方案 | CSDN（昇腾知识图谱检索文） | 2026-09-12 | DataCopyExtParams blockCount/blockLen/stride 单位口径；尾块拆分方案（主体 DataCopy+尾 DataCopyPad） | C | verified |
| S22 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC3/devaids/opdev/optool/atlasopdev_16_0092.html | msProf 工具概述 | CANN 8.0.RC3 官方 | 2026-09-12 | msprof op 参数官方口径（kernel-name/launch-count 等） | A | verified |
| S23 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/ascendcopapi/atlasascendc_api_07_0078.html | ReduceSum（8.0.RC3 官方 API） | CANN 商用版 | 2026-09-12 | A2 两种相加方式（前 n 个=方式二；高维切分=方式一） | A | verified（Agent 2 已核，此处仅引用） |
| S24 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlasascendc_api_07_0089.html | WholeReduce | CANN 商用版 | 2026-09-12 | WholeReduceSum 每 repeat 二叉树语义 | A | verified（Agent 2 范畴，仅引用） |

### 引用前序 Agent 结论（不重查）

- ops-nn add_rms_norm tiling 阈值与 SINGLE_N InitBuffer 195584B（Agent 3）。
- DataCopyPad GM→UB/UB→GM dummy 语义、ReduceSum count 上限（Agent 2）。
- cann-samples rms_norm_quant_story 各优化倍数（Agent 3）。

---

*本报告为静态调研，不含任何本机测量；所有"推断"结论须经真机 msprof 验证后方可用于提交决策。*
