# AddRmsNormBias 技术方案调研报告（调研2）

> 调研日期：2026-09-11
> 题目：`AddRmsNormBias`｜2026 CANN 挑战赛·西南赛区（初赛）｜题目 ID `6a9a9a99bf41025d6013eb85`｜平台编号 `1742`
> 目标 CANN `9.0.0`｜Kernel 类型 `vector`
> 执行方式：10 个子代理并行调研（Agent01–10），主代理合并、去重、定级与复核
> 来源总表：`sources.md`（编号 S001–S194）｜覆盖计划：`平台与研究覆盖计划.md`｜子代理报告：`agents/agent01…agent10-*.md`
>
> ## 全局声明
>
> 1. 本机为 **macOS，无 CANN 工具链、无昇腾 NPU**。本轮**只**做了资料核对、源码阅读、CPU 参考计算与静态分析。
> 2. 本轮**没有**在真实 CANN/NPU 上编译、运行或验证任何代码。凡涉及 NPU 侧编译、精度、性能的结论，一律标注「**未在真实 CANN/NPU 验证**」。
> 3. 本轮**没有**执行 CANNJudge 上传，**没有**消耗任何提交次数。
> 4. 证据等级：**A** 官方题面/官方 API/官方仓库源码；**B** 官方样例/官方测试/官方培训/官方 PR/Issue；**C** 社区文章/论坛/个人仓库；**D** 仅搜索摘要；**L** 本机实测或本项目本地记录。

---

## 1. 调研范围与平台覆盖情况

### 1.1 覆盖的平台

| 代理 | 平台 | 覆盖状态 |
| --- | --- | --- |
| 01 | CANNJudge 公开 API、GitCode 赛事页 | ✅ 题面 API JSON 与赛事 API JSON 抓取成功；提交页需登录，未能覆盖 |
| 02 | hiascend.com / hiascend.cn / asc.gitcode.com（CANN 9.0.0 API 参考） | ✅ 12 个关键 API 页面按 9.0.0 版本核对，2 个新页面存档到 `临时/api-pages/` |
| 03 | gitcode.com/cann/{ops-transformer, ops-nn, cann-samples, cann-learning-hub, asc-devkit} | ✅ 5 个官方仓库全部浅克隆成功，精读约 16 个源码文件 |
| 04 | NVIDIA / Triton / PyTorch / ROCm / GPU MODE / Hugging Face | ✅ 23 条来源，12 行迁移差异表 |
| 05 | LLVM / MLIR / IREE / TVM / TorchInductor / TileLang / PyPTO / msopgen | ✅ 13 个工具/项目评估 |
| 06 | NumPy / PyTorch / MindSpore / 论文 | ✅ 12 条来源 + 7 组本机实测实验 |
| 07 | 昇腾硬件/Profiling 文档 / NVIDIA / ROCm / HPC | ✅ 20 条来源（官方 A 级 10 条） |
| 08 | Linux DO / V2EX / Stack Overflow / ServerFault / Phoronix 等 | ⚠️ 官方环境文档覆盖完整（27 条）；**Linux DO 与 V2EX 均为「无命中」**（已给出检索式与结果数） |
| 09 | GitHub Issue/PR、GitCode/Gitee、Kaggle、GPU MODE、历史算子竞赛 | ✅ 20 条失败案例，其中 12 条有可点击真实记录 |
| 10 | 无独立外网检索（审阅 01–09 产物 + 本机复核） | ✅ 见 `agents/agent10-synthesis.md` |

### 1.2 覆盖缺口（明确未覆盖）

| 缺口 | 原因 | 影响 |
| --- | --- | --- |
| CANNJudge 提交页表单字段名 | 需登录 | 上传格式与字段无法最终确认 |
| 15 个测试点的具体 shape/dtype/epsilon | 平台不开放（`visible_testcase_count: 0`） | 无法针对性调优，必须泛化 |
| 判题机具体 SoC 型号 | 题面 API `soc_version: null`、`ddk_version: null` | 无法确定 DataCopyPad 支持矩阵与 `-c ai_core-<soc>` 取值 |
| 决赛规则 | 未发布 | 只能按初赛准备 |
| 真实 NPU 上的编译/精度/性能 | 本机无 NPU | 本报告全部 NPU 侧结论均为待验证 |

---

## 2. 题目约束（A 级为主）

### 2.1 计算语义

```text
Step 1  y_i      = x_i + residual_i
Step 2  rms      = sqrt( mean(y^2, dim=-1) + epsilon )
        z_i      = y_i / rms * gamma_i
Step 3  output_i = z_i + bias_i
```

- epsilon 在 **`mean(y²)` 之后、开方之前**（S145，PyTorch `nn.RMSNorm` 官方文档同构）。
- bias 在归一化**之后**（不是 `RMSNorm(x, gamma, beta)` 的 beta 位置）。
- **注意**：官方仓库中所有「非量化」`AddRmsNorm` 都是 `output = y/rms*gamma`，**不含 bias**；只有量化版 `AddRmsNormQuant` 含 `+beta`（S030/S031/S032，A/B 级）。因此官方源码只能借算法与分块结构，**不能直接照抄语义**。

### 2.2 接口约束（S001，A 级）

| 项 | 值 |
| --- | --- |
| 输入顺序 | `x` → `residual` → `gamma` → `bias` |
| 属性 | `epsilon`（float，默认 `1e-5`） |
| 输出 | `output`，shape 与 dtype 同 `x` |
| x / residual | `(..., D)`，rank ∈ {2D, 3D, 4D} |
| gamma / bias | `(D,)` |
| dtype | `float16` / `bfloat16` / `float32` |
| D | `[64, 32768]`，**可能不是 32 的倍数** |
| batch | `[1, 8192]`；seq_len `[1, 32768]` |

### 2.3 判分约束（S001/S002/S003，A 级）

| 项 | 值 |
| --- | --- |
| 测试点 | **15 个，全部通过才计分** |
| 每点 iterations | **5** |
| 精度 | fp32：相对 <1e-4 且绝对 <1e-4；fp16/bf16：相对 <1e-3 且绝对 <1e-3 |
| 得分 | `100 / (1 + log₁.₅(t/T))`，`T` 为该点全局最优耗时，总分 = 15 点均值 |
| 计分维度 | **性能是唯一计分维度**（正确性为门槛） |
| 提交额度 | 每日 **50 次**（S003，A 级） |
| 成绩取法 | **取最后一次提交成绩**（S038/S003） |
| 晋级 | 初赛前 32 名进决赛 |
| 违规 | Host 代算、空 kernel 等取消成绩 |

### 2.4 本地判定口径（B 级，与判题端未必一致）

模板 `verify_result.py`（S182）的判定是：`np.isclose(rtol, atol, equal_nan=True)` 逐元素比较，再叠加「**失配元素比例 ≤ tol**」的容忍，模板默认 `(fp16, rtol=1e-3, atol=1e-3, tol=1e-3)`。

**这是全案最大的未知量之一**：题面只写了误差阈值，**没有写是否允许部分元素失配**。若判题端与模板一致（允许 ≤0.1% 失配），则 bf16 的固有量化翻转不构成问题；若判题端要求**逐元素**达标，则 bf16 存在结构性风险（见 §6）。

---

## 3. Ascend C 官方 API（A 级，未在真实 CANN/NPU 验证）

### 3.1 争议点裁决（Agent02 结论，详见 `agents/agent02-ascendc-api.md`）

| # | 争议 | 裁决 | 等级 |
| --- | --- | --- | --- |
| 1 | `DataCopyPadExtParams<T>` 字段顺序 | **`{isPad, leftPadding, rightPadding, paddingValue}`**（S010）。项目旧笔记写的顺序是**错的** | A |
| 2 | UB→GM 搬出时超出有效长度的 padding 是否污染相邻内存 | **不污染**：框架在写入 GM 时自动丢弃 dummy（S010） | A |
| 3 | `ReduceSum` 的 `count` 上限 | 官方仅约束「≤UB 限制」，**不存在 255 / 4096 / 16320 这类单一硬上限**；社区数字是不同实现路径的口径 | A + C |
| 4 | Atlas A2 上 `Add`/`Mul` 是否支持 `bfloat16_t` | **不支持**，bf16 必须转 FP32 计算（S013） | A |
| 5 | 向量 `Sqrt`/`Rsqrt` 精度 | 存在 1ULP 与「快速求逆」等精度模式；用标量 `sqrtf` 可规避模式不确定性 | A |
| 6 | 入口限定符 `__vector__` vs `__aicore__` | 纯 Vector 算子两者**均合法**；`__vector__` 限定仅在 Vector 核执行，直调推荐（S022，**注意该页为 8.5.0alpha002**） | A（限修饰符表）/ 版本差异已标注 |

### 3.2 对本题最关键的 API 约束

| API | 关键约束 | 对本题的影响 |
| --- | --- | --- |
| `DataCopy` | GM/Local 地址与长度需 32 B 对齐 | 对齐场景用它，比 `DataCopyPad` 快（C 级） |
| `DataCopyPad` | GM 地址**无对齐约束**；Local 起始需 32 B 对齐；搬入可补零；搬出按实际字节写 | 尾块唯一安全路径 |
| `DataCopyExtParams` | `blockLen` 单位为**字节**；`srcStride`（GM 侧）单位为**字节**，`dstStride`（UB 侧）单位为 **dataBlock(32 B)** —— 单位不同是高频错误（S167，C 级） | 参数填写必须按单位逐项核对 |
| `ReduceSum`（基础 count 版） | 源 32 B 对齐、目的按 dtype 对齐；源/目的/tmp 三者地址不可重叠；半精度输入下**只支持特定产品** | 主线归约入口 |
| `ReduceSum`（高阶 `Pattern::Reduce`） | A2/A3 仅支持二维 shape，`srcInnerPad` 仅支持 `true`；临时空间用 host 侧 `GetReduceSumMaxMinTmpSize` 查询（S012/S021） | 备选路径 |
| `Cast` | fp16/bf16→fp32 用 `CAST_NONE`（精确）；fp32→fp16/bf16 用 `CAST_RINT`（就近舍入，RNE） | V002 写法正确 |
| `Sqrt` / `Rsqrt` | 存在多精度模式 | 优先标量 `sqrtf` 或明确指定精度模式 |
| `GetValue` | 标量读回需配 `SetFlag/WaitFlag<HardEvent::V_S>`，读后用 `S_V` 回同步 | V002 写法正确 |
| `TPipe` / `TQue` / `TBuf` | 官方最佳实践：**`TPipe` 应在 kernel 入口创建**，不要作为类成员在对象内构造（S080，A 级） | V002 把 `TPipe tpipe` 放在类内——**与官方最佳实践不符，需评估** |

> **重要提醒（S080）**：官方明确「避免 `TPipe` 在对象内创建和初始化」，因为其构造会设置全局指针、污染类对象内存、抑制编译器标量折叠。V002 的 `TPipe tpipe;` 是**类成员**，而模板 `main.asc`/`kernel.asc` 的推荐形态也是在核函数外使用。这条要交给真机与 Agent10 复核——它也可能是 V001 报错的背景之一。

### 3.3 `workLocal` 尺寸核算（主代理独立核算）

V002 的 `work_` 为 `WORK_LEN = 1024` 个 float。按社区推导的公式（firstMaxRepeat = count / elementsPerRepeat，float 时 elementsPerRepeat = 64；所需元素数 = `RoundUp(firstMaxRepeat, 32/typeSize) × (32/typeSize)`，float 时 `32/4 = 8`）：

```text
fp32 路径：tile_len_ = 2048 → count 最大 2048 → firstMaxRepeat = 2048/64 = 32
          所需 = RoundUp(32, 8) × 8 = 32 × 8 = 256 个元素
fp16/bf16 路径：count 最大 4096（FP32 域归约）→ firstMaxRepeat = 4096/64 = 64
          所需 = RoundUp(64, 8) × 8 = 64 × 8 = 512 个元素
```

→ **1024 个元素足够**（余量 2×）。但该公式**未见官方文档给出**（Agent02 确认为证据缺口），需真机确认。

---

## 4. 官方开源实现对比（A/B 级，仅静态阅读，未编译）

### 4.1 官方实现对照

| 仓库 | 文件 | 语义 | 归约方式 | D 切分与尾块 | 多核 | 与本题差异 |
| --- | --- | --- | --- | --- | --- | --- |
| ops-transformer (mc2) | `add_rms_norm*.h`（single_n / normal / split_d / multi_n / merge_n）、`common_add_rms_norm_tiling.cpp` | `y/rms*gamma`，**无 bias** | 自写 `ReduceSumCustom` → 底层 `WholeReduceSum`/`BlockReduceSum` | `jMax=ceil(numCol/ubFactor)`，`colTail` 处理非 32 倍数；两遍 former/latter | 按行分 `blockFactor`，尾核吃余数 | 无 bias；**SPLIT_D 仅 fp16/bf16** |
| ops-nn | `fused_add_rms_norm*.h` | residual + gamma，**无 bias** | 同上族 | 有 | 有 | 无 bias；**含 fp32 分支**（比 ops-transformer 更适合参考） |
| ops-nn | `add_rms_norm_quant*` | **含 `+beta`**，但输出 int8 | 同上族 | 有 | 有 | **唯一含 bias 的官方实现**，可借 epilogue 结构，量化部分不可迁移 |
| cann-samples | `rms_norm_quant_story/src/0_naive.asc` 等 | RMSNorm+量化 | **基础 API `ReduceSum(dst, src, work, count)`** | 有 | 有（7 步优化阶梯示例） | 与判题形态最接近的官方单文件 `.asc` 样例 |
| cann-learning-hub | `rmsnorm_baseline_kernel.h` | RMSNorm | FP32 标量骨架 + Newton rsqrt | — | — | 教学 baseline |
| asc-devkit | ReduceSum 接口定义 | — | 基础 + 高阶两套 | — | — | API 事实来源 |

### 4.2 可迁移点（≥3）

1. **D 切分 + 列尾块的标准做法**：`jMax = ceil(D / ubFactor)`，最后一块按 `colTail` 只算有效元素——与本题 D 非 32 倍数的需求一致。
2. **多核按行切 + 尾核吃余数**：官方 `blockFactor` 分配法与 V002 的 `each/extra` 分工一致。
3. **FP32 域归约**：官方内核统一在 FP32 累加平方和，与本题「黄金法则」一致。
4. **`WholeReduceSum` + `BlockReduceSum` 组合**比裸 `ReduceSum` 更快的官方口径（S054，A 级）——性能路线的直接依据。
5. **cann-samples 的单文件 `.asc` 形态**与判题的 `kernel.asc` 直调形态最接近，是最有价值的形态参考。

### 4.3 不可迁移点（≥3）

1. **形态差异**：官方是 msopgen/Aclnn 完整工程（`op_host` / `op_kernel` / `op_api` / `op_graph` / CMake），而判题是**单文件直调 + `run_kernel`**。不能照抄编译。
2. **语义差异**：官方非量化 AddRmsNorm **均无 bias**；本题必须自己实现 `+ bias` 的 epilogue。
3. **dtype 覆盖差异**：ops-transformer 的 `SPLIT_D` 路径仅 fp16/bf16，而本题**必须支持 fp32**且 fp32 是精度最严的一档（1e-4）。
4. **上下文差异**：官方实现多嵌在 MoE / MatMul-AllReduce 融合上下文（含通信、量化），本题是独立的 4 输入 1 输出小算子。
5. **tiling 依赖差异**：官方 tiling 由 host 侧下发且与 aclnn 接口耦合，判题是 host 侧 `run_kernel` 直接用 `TensorGroupInfo` 读 shape。

---

## 5. GPU / NPU 迁移分析（C 级为主，仅作研究参考）

### 5.1 迁移差异表（Agent04 提供，12 行）

| # | GPU 机制 | Ascend C 对应（方向性） | 可迁移部分 | 不可直接迁移部分 | 风险 |
| --- | --- | --- | --- | --- | --- |
| 1 | warp shuffle 蝶形归约 | 无 warp 概念；改用核内归约原语（`WholeReduceSum` / `BlockReduceSum`） | 「先局部后全局、树形合并」思想 | `__shfl_xor_sync`、lane/warp 语义 | 直接照搬会数据错乱 |
| 2 | block reduction（shared 暂存） | UB 暂存中间归约结果 | 两阶段树形归约思想 | CUB `BlockReduce`、显式 `__shared__` 管理 | 同步点语义不同 |
| 3 | `__syncthreads()` | `SetFlag`/`WaitFlag`、`PipeBarrier` | 「归约前后需同步」的动机 | 线程块屏障语义 | 误用导致数据竞争 |
| 4 | shared memory 复用整行 | UB 片上缓存 | 「一行放片上算完再写回」 | shared 生命周期与 bank 冲突 | UB 预算不同，需重算 tiling |
| 5 | 寄存器累加器 | 寄存器 + UB 向量累加器 | 「累加器 FP32 驻留、减少回写」 | 寄存器分配/spill 模型 | UB 溢出导致性能骤降 |
| 6 | 向量化加载 `float4`/`half8` | 128-bit 向量搬运 + 向量指令 | 合并访存、一次搬 128 bit | 指针强转与对齐判断 | **非对齐 / D 非 16 倍数时尾块问题** |
| 7 | FP32 累加器 | **同样用 FP32 累加** | **几乎完全可迁移** | 无（数学层） | 低 |
| 8 | grid-stride 行划分 | 多核 + tiling，每核若干行 | 「按行并行、每核一段」 | grid/block 组织与 stride 语义 | 核数/负载需按 Ascend 重算 |
| 9 | 尾块 mask/predication | mask + `DataCopyPad` | 边界判断思路 | GPU 的向量化 tail 写法 | 未对齐尾块易越界写 |
| 10 | occupancy 调优 | 无 occupancy；对应核内流水、UB 容量、多核利用率 | 仅「提高并行度隐藏访存延迟」思路 | occupancy 公式与查询 API | **高：GPU 直觉在 Ascend 无依据** |
| 11 | warp divergence | 无 warp | 仅「用 mask 替代分支」思想 | divergence 定义与代价模型 | **高：在 Ascend 谈此概念无意义** |
| 12 | L2 命中率调优 | 无 L2 概念（L1/UB + HBM） | 仅「访存局部性」思想 | L2 作为可显式利用的缓存层次 | **高：无对应物** |

### 5.2 明确判定为「不可用于本题提交」的 GPU 内容

所有 CUDA / Triton / HIP 源码、NVIDIA SOL-ExecBench 的 Python 参考、GPU speedup 基准数字、occupancy / L2 / warp divergence 调优结论，以及 **框架层算子 `torch_npu` / `npu_add_rms_norm`（属框架算子，非手写 Kernel，违反题意，S113）**。

### 5.3 跨平台唯一站得住的四条杠杆

1. **FP32 累加**（数学层，可直接迁移）；
2. **算子融合减少片外往返**（`x + residual` 与 RMSNorm 融合，动机一致）；
3. **合并/向量化访存**（需按 Ascend 的 32 B / 256 B 粒度重新设计）；
4. **按行并行 + 尾块掩码**（结构一致）。

---

## 6. 数值精度方案

### 6.1 精度判定的两个口径（必须先分清）

| 口径 | 含义 | bf16 能否达标 |
| --- | --- | --- |
| 与**同 dtype golden** 比对 | 判题端用同 dtype 的参考实现，比较两者输出 | **可以**（靠量化一致性） |
| 与**高精度真值（FP64）** 比对 | 与数学真值比较 | **结构性不可能** |

本机实测（`数值参考脚本/`，S148，**CPU/numpy 口径**）：

- golden 自身 vs FP64 真值的最大相对偏差：fp16 最大 **2.87e-3**（D=32768）、bf16 约 **3.9e-3**（≈ BF16 ulp 2^-8，S143 一致）。
- 所以 **bf16 的 1e-3 阈值只能在与同 dtype golden 比对的口径下成立**——这是判题口径必须确认的第一前提。

### 6.2 主代理独立实测的关键结论

| 实验 | 结论 | 数字 |
| --- | --- | --- |
| V002 算法结构 vs 官方 golden（21 组） | **完全等价** | 失配率全部 **0.0000%**（含 D=67/129/4097/32768） |
| 分块累加（tile=4096）vs 整体累加 | **不是误差来源** | 平方和相对差 ≤ **8.9e-8**，rms 相对差 ≤ 7.3e-8 |
| `Muls(y, 1/rms)` vs golden 的 `y / rms` | **是主要差异来源** | fp16：≤3/131072 元素不等；bf16 D=32768：3 个元素相差 1~3 ulp，最大相对 **7.41e-3**、绝对 **9.77e-4**；fp32：约 19% 元素差 1 ulp，但绝对 ≤4.8e-7 |
| 多种子扫描（40 种子 × D × 量级 1/10/100） | 在 isclose 口径下安全 | fp16 最大相对 ≤9.75e-4；bf16 最大相对 ≤7.58e-3；**失配率 >0.1% 的种子数 = 0 / 40** |
| fp16 平方和溢出边界 | **低精度累加被否决** | `|y| ≥ 256` 时 fp16 域 `y*y` = inf；FP32 域无此问题 |

### 6.3 Agent06 的独立实测（互相印证）

- bf16 与同 dtype golden 比对：D=1024 失配 **0.012%**、D=32768 失配 **0.009%**，均低于 0.1% 容忍线 → 在模板口径下可通过。
- bf16 与 FP64 真值比对：最差单元素相对误差可达 **16%** → 结构性不可达标。
- D=32768 分块 vs 顺序归约：平方和相对误差 **2.8e-7 ~ 7.9e-6**（差约 28×）→ 可忽略。
- **epsilon 位置写错的代价**：`mean(y²)=1e-3` 时 rms 偏 −0.46%；`mean(y²)=1e-8` 时 rms 偏 **−96.5%**，输出差约 28 倍 → 可用于反向校验实现是否正确。

### 6.4 精度方案结论

| 方案 | 判定 | 依据 |
| --- | --- | --- |
| **FP32 全中间计算（S3）** | ✅ **必须采用** | fp16 平方和溢出（|y|≥256 → inf）；官方仓库一致 FP32 累加；三方独立来源一致 |
| 低精度中间计算（S4） | ❌ **否决** | 上述溢出边界实测 |
| 输出前一次性 cast（`CAST_RINT`） | ✅ 采用 | 与 golden 的 numpy `astype` 同链；A2 上 fp32→bf16 无 `CAST_NONE` |
| **归一化改「先除」（`Divs(y, rms)`）** | ✅ **建议采用** | 消除与 golden 的舍入路径差异，**零额外成本**，能把 bf16 的 1~3 ulp 差异收敛到 0（待 Agent10 独立复核） |
| 标量 `sqrtf` | ✅ 采用 | 规避向量 Sqrt/Rsqrt 的精度模式不确定性 |
| bf16 严格逐元素 1e-3 | ⚠️ **有风险** | 受 BF16 自身 ulp（2^-8 ≈ 3.9e-3）限制，只能依赖判题的失配容忍 |

---

## 7. UB 分块与性能方案

### 7.1 硬件参数（S050–S053、S023，B/A 级）

| 参数 | 值 | 等级 |
| --- | --- | --- |
| UB 容量（Ascend 910B1~B4 / Atlas A2 训练系列） | **192 KB**（196608 B） | B（CANN 源码级 + 工具校验，**华为未发布完整 910B datasheet**，S062） |
| AI Core 数（910B1/B2） | 24（910B3/B4 为 20） | B |
| **AIV 向量核数** | **48**（= 2 × AI Core） | B |
| 向量单元每 repeat 读取 | **256 B** | A |
| fp16/bf16 每 repeat 元素数 | **128** | A |
| fp32 每 repeat 元素数 | **64** | A |
| 聚合 HBM 带宽（910B） | ≈ 1600 GB/s（**上界，非核内 DMA 实测**） | C |

### 7.2 V002 的 UB 占用核算（估算）

以 fp16/bf16（`tile_len_ = 4096`，`BUFFER_NUM = 1`）为例：

| 缓冲 | 元素数 | 字节 |
| --- | --- | --- |
| `x_queue_` | 4096 × 2 B | 8192 |
| `residual_queue_` | 4096 × 2 B | 8192 |
| `gamma_queue_` | 4096 × 2 B | 8192 |
| `bias_queue_` | 4096 × 2 B | 8192 |
| `output_queue_` | 4096 × 2 B | 8192 |
| `x_float_` | 4096 × 4 B | 16384 |
| `residual_float_` | 4096 × 4 B | 16384 |
| `value_float_` | 4096 × 4 B | 16384 |
| `work_` | 1024 × 4 B | 4096 |
| `sum_` | 4096 × 4 B | 16384 |
| **合计** | | **≈ 108 KB** |

fp32 路径（`tile_len_ = 2048`）合计 ≈ **76 KB**。两者均 < 192 KB，**UB 不是当前候选的瓶颈**。

### 7.3 访存次数对比（两遍 vs 单遍）

V002 是两遍扫描：Pass1 读 x、residual；Pass2 再读 x、residual、gamma、bias；写 output。

| 形态 | 每行 GM 流量（`s` = 元素字节） | 相对最小基线 |
| --- | --- | --- |
| 理论最小（读 x、residual，写 output） | `3·D·s`（gamma/bias 可忽略） | 1.0× |
| **两遍扫描（V002）** | 读 2+4 = `6·D·s` + 写 `1·D·s` = `7·D·s` | **2.33×** |
| **单遍暂存** | 读 `2·D·s` + 写 `1·D·s` = `5·D·s` | 1.67× |

→ 单遍相对两遍**减少约 28.6% 的 GM 流量**（Agent07 口径为 7:5 = 1.4×；两个口径的差异在于是否把 gamma/bias 计入，结论方向一致）。
单遍暂存的 UB 需求：整行 `value` 以 FP32 驻留，D=32768 需 **128 KB**，仍在 192 KB 内（**未在真机确认**）。

### 7.4 三种硬件形态的推荐 tiling（Agent07 结论，未在真机验证）

| 形态 | 特征 | 推荐 tiling 思路 | 主要风险 |
| --- | --- | --- | --- |
| **A：小 outer、大 D**（outer ≤ 核数，D=32768） | 并行度不足，单行耗时长 | 单遍驻留整行 value（FP32 128 KB）、`BUFFER_NUM=2` 双缓冲、**必须用满 48 个向量核** | 核数不足时半数向量核空闲；UB 峰值逼近上限 |
| **B：大 outer、小 D**（outer=8192，D=64） | 每行一次标量同步被放大 8192 倍 | **把 `1/rms` 留在 UB，用向量 `Sqrt`+`Reciprocal` 广播，彻底去掉 `GetValue` 标量读回** | 向量 Sqrt 精度模式不确定；核间负载均衡 |
| **C：中间形态**（outer ≈ 核数若干倍，D=1024/4096） | 均衡 | 单遍驻留（UB 占用小）+ 双缓冲 + 满核；归约用 `BlockReduceSum`+`WholeReduceSum` 组合 | 需真机测双缓冲收益 |

### 7.5 V002 的性能改进点（按预期收益排序）

| 优先级 | 改进 | 依据 | 预期 |
| --- | --- | --- | --- |
| P1 | **确认核数用满**：`blocks` 应等于向量核数（48），而非 `availableCoreNum` 的语义不明值 | S052（B）、Agent07 | 形态 A 可能直接翻倍 |
| P1 | **消除 `GetValue` 标量同步**：归约结果留 UB，用向量 `Sqrt`+`Reciprocal` 广播 | Agent07 §8 | 形态 B 收益最大 |
| P2 | **单遍暂存**（D ≤ 4096 时） | §7.3 计算 | 减少 ~28.6% GM 流量 |
| P2 | **`BUFFER_NUM = 2` 双缓冲** | Agent07 附注 | 重叠 MTE 与 V |
| P3 | **对齐时用 `DataCopy` 替代 `DataCopyPad`** | S064（C，无官方数字） | 需真机量化 |
| P3 | **fp32 分支的冗余 UB→UB 拷贝**：V002 在 fp32 路径下用 `DataCopy` 把 `value_float_` 拷到 `output_queue_`，再搬出；可直接从计算缓冲搬出 | 静态阅读 `提交/V002/kernel.asc:148-154、244-249` | 省一次全 tile 拷贝 |
| P3 | **gamma/bias 片上常驻**：V002 每行每 tile 都从 GM 重读 gamma/bias | 静态阅读 `提交/V002/kernel.asc:227-230` | outer 大时收益明显 |
| P3 | **归约指令换成 `BlockReduceSum`+`WholeReduceSum`** | S054（A） | 官方口径更快 |

### 7.6 性能测量方法（真机）

```bash
# 单算子耗时
msopst run -i <case>.json -soc Ascend910Bx -out ./out     # msopst.ini 中 performance_mode=True
# 生成 op_summary_0_1.csv

# 算子级 profiling（核内流水/占用率/UB 冲突）
msprof op --aic-metrics=Occupancy,Memory,MemoryUB,PipeUtilization,ResourceConflictRatio \
          --application="./add_rms_norm_bias_custom"
```

---

## 8. 编译与真机环境方案

### 8.1 关键环境事实（A 级）

| 项 | 事实 | 来源 |
| --- | --- | --- |
| CANN 9.0.0 安装路径 | **`/usr/local/Ascend/cann/`**（不是 8.x 的 `ascend-toolkit/`）；模板 `run.sh` 的回显提示是 8.x 旧路径，**易误导** | S070 |
| 模板默认 SoC | `NPU_ARCH` 未设时用 `dav-2201` | S180 |
| `dav-2201` 对应产品 | Atlas A2 训练/推理（910B1~B4、910B2C）与 Atlas A3（910_93）；950/A5 系用 `dav-3510` | S074（A） |
| 判题机 SoC | **平台未指定**（题面 API `soc_version: null`） | S001 |
| `ASCEND_HOME_PATH` | 指错版本 → `find_package(ASC)` 失败或 `kernel_operator.h` 不匹配 → 直接 Compile Error | S073 |
| `ASCEND_OPP_PATH` | **禁止手动 export**（会漏变量，触发 561107） | S071/S072 |
| 构建入口 | 模板用 `find_package(ASC REQUIRED)` + `--npu-arch=${SOC_ARCH}`；本地跑法是 `./run.sh` | S180/S073 |

### 8.2 V001 报错的性质

V001 平台返回 15/15 `Compile Error`，错误集中在 `TPipe pipe_` 成员声明与 `InitBuffer` 调用（`unknown type name 'pipe_'`、`cannot use dot operator on a type`），**全部停在 `make` 阶段**。

两种解释存在冲突，均已登记：

- **H1（项目原记录）**：`pipe_` 与保留宏/标识符冲突 → 改名 `tpipe` 修复。
- **H2（Agent08 推测）**：`TPipe` 在 CANN 9.0.0 的作用域可见性问题。

**并且必须强调一个事实**：V002 的平台失败已被诊断为**上传内容截断**（平台收到的文件第 1 行为 `return false;`），所以 **V002 的代码从未被平台真正编译过**。「改名 `pipe_` → `tpipe` 修好了编译」**目前没有任何证据支持**。

### 8.3 与官方最佳实践的偏差（需要真机复核）

官方文档明确「**避免 `TPipe` 在对象内创建和初始化**」（S080，A 级），理由是构造会设置全局指针、污染类对象内存、抑制编译器标量折叠。V002 把 `TPipe tpipe;` 作为 **kernel 类成员**，与该建议不符——这与 H2 的方向一致，值得在真机上优先验证。

### 8.4 真机从零跑通的最短路径

```bash
# 1. 设备与环境
npu-smi info
source /usr/local/Ascend/cann/set_env.sh          # 注意 9.0.0 路径，非 ascend-toolkit
echo "$ASCEND_HOME_PATH"                           # 必须指向 9.0.0

# 2. 用完整文件替换模板 kernel.asc（务必全选替换，核对首末行与行数）
cp /Users/sunyiyang/Desktop/Project/cann/提交/V002/kernel.asc \
   /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc
wc -l /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc   # 期望 405

# 3. 编译 + 跑默认用例（FP16 [1,64]）
cd /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template && ./run.sh

# 4. SoC 与判题机不一致时覆盖
#    cmake -DNPU_ARCH=dav-XXXX ..
```

---

## 9. 竞赛失败案例（20 条，12 条有真实记录）

### 9.1 按错误类型的规律

| 类型 | 规律 | 代表案例 |
| --- | --- | --- |
| **编译** | 几乎全部集中在**入口/类型/宏命名/参数个数**，而非算法 | V001 `pipe_`（L3）；官方 FAQ 五大高频案例（S079）；ascend-rs 三关静态校验（S008/S051） |
| **精度** | **归约未用 FP32 累加是第一杀手**；中间过程提前降精度次之 | FP16 归约溢出（S161）；rms_norm 中间转 bf16（S162）；bf16 累加致训练发散（S163） |
| **越界** | 尾块越过本段边界、`ALIGN_UP` 读越界、`DataCopyPad` 参数单位混淆 | S165、S166、S167；非对齐尾块用 `DataCopy` 直接 Core Dump（S168） |
| **性能** | 「单 shape 通过 ≠ 全点通过」；为提速牺牲泛化的 reward hacking | robust-kbench（S170） |
| **流程** | 上传内容异常、提交次数管理、成绩取法 | **V002 上传截断（L4）**；每日 50 次与取最后一次（S038） |

### 9.2 本项目最可能踩的坑 Top 5（配本机可执行检查项）

| # | 风险 | 本机检查项 |
| --- | --- | --- |
| 1 | **上传内容异常**（编辑器未全选替换、只贴片段） | 提交前核对：首行是 `#include <cmath>`、末行是 `}`、`wc -l` ≈ 405、含 `extern "C" void run_kernel` |
| 2 | 宏/命名冲突导致编译失败 | `grep -n "TPipe pipe_" 提交/V00N/kernel.asc` 应无命中 |
| 3 | **归约未用 FP32** | `grep -n "ReduceSum\|GetValue" 提交/V00N/kernel.asc`，人工确认归约输入是 FP32 缓冲 |
| 4 | 尾块越界写相邻行 | 人工核对 `DataCopyPad` 搬出入参：搬入 `rightPadding` 只向右补零、搬出 `blockLen = 有效元素数 × sizeof(T)` |
| 5 | `DataCopyPad` 参数单位混淆 | 逐字段核对：`blockLen` = 字节；`srcStride` = 字节；`dstStride` = dataBlock(32 B)（**三处单位不同**） |

### 9.3 证据审阅结论（Agent10，完整版见 `agents/agent10-synthesis.md`）

**11 项矛盾裁决结果**：8 项定案，3 项未定案。

| 定案（8 项） | 结论 |
| --- | --- |
| `DataCopyPadExtParams` 字段顺序 | `{isPad, leftPadding, rightPadding, paddingValue}`（A）；V002 逐字段赋值天然免疫 |
| V001 根因 | `pipe_` 命名冲突（`unknown type name` 表明编译器把 `pipe_` 当类型名）；H2 的作用域推测未被证实，但改名同时覆盖两种可能 |
| `ReduceSum` count 上限 / `workLocal` | 官方仅约束 ≤UB；按社区公式 `WORK_LEN=1024` 足够（count=4096 → 需 512） |
| 入口限定符 | `__vector__` 与 `__aicore__` 均合法，V002 写法正确 |
| 是否需要 `GetReduceRepeatSumSpr` | **不需要**，基础 `ReduceSum + GetValue(0)` 足够 |
| BF16 精度口径与「先除」修正 | 主代理发现可靠；改 `Divs(y, rms)` 后与 golden **逐位一致**，零成本 |
| fp32 支持 | 官方 SPLIT_D 确实缺 fp32 分支，但**自研 fp32 无技术障碍**（V002 已覆盖） |
| 并行核数 | `ACL_DEV_ATTR_VECTOR_CORE_NUM` 与 `GetCoreNumAiv()` **一致**，V002 用 `availableCoreNum` 合理，且有 `min(核数, outer)` 钳制 |
| `DataCopyExtParams` 单位 | V002 用单块 + 全 0 stride，**对该单位歧义免疫**，参数正确 |

| 未定案（3 项，均 P0） | 为什么定不了 |
| --- | --- |
| UB→GM padding 是否真丢弃 | 官方 A 级口径称丢弃，但属**静默数据损坏型**风险，必须真机构造相邻缓冲验证 |
| V002「改名修复编译」是否成立 | V002 的 405 行**从未被真正编译过**（平台收到的是截断文件），无编译证据 |
| 判题 SoC 型号 | 题面 API `soc_version: null`，平台未指定；影响 `Add/Mul` bf16 矩阵、`ReduceSum` 行为、UB 容量、核数、`DataCopyPad` 支持范围 |

**误用清单（社区推断被当成官方结论）**：①「padding 写回污染相邻内存」——实为官方称丢弃；②「`ReduceSum` count 上限 = 4096/255/16320」——实为社区估算；③「`dav-2201` = 平台约束」——实为模板编译默认；④「`CAST_RND` 是 9.0.0 枚举」——实为旧别名（应为 `CAST_ROUND`/`CAST_RINT`）；⑤「rsqrt 误差 2^-20」——实为社区估计；⑥「`TPipe` 在 9.0.0 有作用域问题」——推测，未被证实。

**跨型号误用**：310P/310B 的 UB = 256 KB 不能用于 A2（192 KB）；950/A5 需 `dav-3510`，按 `dav-2201` 编译会指令集不兼容。

---

## 10. 统一方案评估表

> 完整矩阵与逐格证据见 `方案矩阵.md`（含推荐级别裁决理由、不推荐做法、待真机确认做法）。下表为同一矩阵的正文版，编号 S1–S14 与 `平台与研究覆盖计划.md` 一致。

| 方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **S1 两遍扫描** | Pass1 求平方和；Pass2 重读 x/res 做归一化 + γ + β | 7·D·S / 行（A，Agent07§4） | 低（≈108 KB fp16，A/B） | 低（由 S3 保障） | 低（由 S9 覆盖） | 中（比单遍慢 1.4×） | 低 | 低 | **可作为第二路线** |
| **S2 单遍暂存** | 一次读 x/res，把 y 驻留 UB 再归一化 | 5·D·S / 行（A，Agent07§4） | 中（整行 value 128 KB @D=32768，B） | 低 | 低 | 高（−28.6% GM 流量） | 中（UB 规划） | 低 | **首选** |
| **S3 FP32 全中间** | 全程 FP32，末次 cast 输出 | 同 S1/S2 | 同 S1/S2 | 极低（消除 fp16 溢出 / bf16 坍塌，A，Agent06Q5） | 低 | — | 低 | 低 | **首选** |
| **S4 低精度中间** | 16-bit 域累加（仅对照，不可提交） | — | — | **极高**（\|y\|≥256 时 fp16 平方直接 Inf 污染整行，A） | — | — | — | 低 | **不建议** |
| **S5 大 tile** | tile 尽量大以减少同步次数 | 同 S1/S2 | 高（UB 压力大，B） | 低 | 低 | 高（少同步） | 低 | 低 | **可作为第二路线** |
| **S6 小 tile** | tile 小，UB 占用低 | 同 S1/S2 | 低 | 低 | 低 | 低（同步被放大，C） | 低 | 低 | **仅作研究参考** |
| **S7 按行分配 AI Core** | 以行为单位分核，核内串行多行 | — | — | 低 | 低 | 高（标准分核） | 低 | 低 | **首选** |
| **S8 按 tile 分配 AI Core** | 跨核协作同一行，需跨核归约 | — | — | 中（跨核归约顺序差异，A） | 低 | 中（复杂度抵消收益） | **高** | 中 | **不建议** |
| **S9 DataCopyPad 尾块** | 用 DataCopyPad 处理非 32 B 对齐尾块 | — | — | 低 | **中（dummy 丢弃未真机验证，A 官方口径 vs C 社区相反）** | — | 低 | 低 | **首选** |
| **S10 手工尾块** | 对齐块 + 掩码 / Duplicate / GatherMask | — | — | 低 | 中（掩码易算错，C） | — | 中 | 低 | **可作为第二路线** |
| **S11 ReduceSum 方案** | 官方 `ReduceSum`（基础 count 版）+ `GetValue(0)` | — | 中（`work=1024` 经核算足够，D 级推导） | 低（顺序差 ≤1e-5，A） | 低 | 中（含标量同步，C） | 低 | 低 | **首选** |
| **S12 手工向量归约** | `WholeReduceSum` / `BlockReduceSum` 组合 | — | 中 | 中（顺序差 1~3 ulp，A） | 中 | 高（可去标量同步） | **高** | 中 | **可作为第二路线** |
| **S13 纯 Ascend C 实现** | 全部逻辑手写于 `kernel.asc`，`run_kernel` 入口 | — | — | 低 | 低 | — | 中 | 低（**唯一合规形态**） | **首选** |
| **S14 GPU/Triton 迁移** | 借鉴 GPU 算法结构 / 移植 CUDA·Triton | — | — | — | — | — | — | **极高（不可编译）** | **仅作研究参考** |

**推荐级别分布**：首选 = S2 / S3 / S7 / S9 / S11 / S13；可作为第二路线 = S1 / S5 / S10 / S12；仅作研究参考 = S6 / S14；不建议 = S4 / S8。

**需要显式说明的一处分歧（主代理与 Agent10）**：主代理初稿把 **S1 两遍扫描列为「首选」（作为首版正确性基线）**，Agent10 把 **S1 降为「第二路线」、把 S2 列为「首选」**。两者的分歧不在事实，而在「推荐级别」这个词的含义：

- 若「推荐级别」= **方案的长期最优选择**（性能优先）→ 采纳 Agent10 的 S2 首选；
- 若「推荐级别」= **风险最低的落地顺序**（先过 15 点门槛）→ S1 应先实施。

本文档的处理：**矩阵沿用 Agent10 的评级**（性能视角），但**第 11 章的路由顺序按风险排序**——首版用 S1 骨架先拿「15 点全过」，随即把 S2 作为第二候选。两个视角在文档内同时保留，未消解。

**本轮独立复核的差异（如实记录）**：第四节「分块累加 vs 整体累加」的平方和相对差，主代理实测 ≤ **8.9e-8**，Agent10 独立复现 worst = **8.6e-6**。差异原因是 Agent10 用「朴素顺序累加」建模硬件标量归约（精度低于 numpy 的 pairwise summation）。**两者都 ≤1e-5，远低于 fp32 预算 1e-4，结论方向一致（分块累加误差可忽略）**。另两处发现（0% 失配、除法路径差异）被完整复现，且 Agent10 进一步证明改成「先除」后与 golden **逐位一致**。

---

## 11. 推荐实现路线（首版）

**目标**：先拿到「**15 点全部通过**」的门槛，再谈性能。首版不追求分数。

**首版 = 当前 V002 的算法骨架 + 三项必要条件修复**：

| # | 动作 | 理由 | 成本 |
| --- | --- | --- | --- |
| 1 | **先把 V002 完整、干净地编译一次**，确认编译通过 | V002 从未被真正编译过；这一步不通过，后面全无意义 | 1 次提交额度 |
| 2 | **归一化改「先除」**：用 `Divs(y, rms)` 替代 `Muls(y, 1/rms)` | 消除与 golden 的舍入路径差异（bf16 有 1~3 ulp 翻转）；Agent10 已独立复现「改后与 golden **逐位一致**」 | 零成本 |
| 3 | **把 `TPipe` 移到核函数入口**、类内保存指针 | 官方最佳实践（S080）；且 H2 提示 `TPipe` 作为类成员可能触发 9.0.0 的可见性问题 | 纯重构，无性能代价 |
| 4 | 保持：两遍扫描、FP32 全中间、`tile ≤ 4096`、按行均分、`DataCopyPad` 逐字段赋值、64 位行偏移、`V_S`/`S_V` 同步 | 已有 CPU 证据支持 | — |

**首版真机动作顺序**（每一步都要留证）：
1. 干净编译（记录命令与日志）；2. 跑通模板默认 FP16 `[1,64]`；3. 跑 D=67/129/1000 的多行非对齐用例，**逐字节**核对相邻行未被污染；4. 跑 15 点的等价自建用例并记录误差；5. 记录每点耗时。

**提交前红线**：`grep` 检查无 `TPipe pipe_`；无聚合初始化 `DataCopyPadExtParams`；无 `uint32_t` 行偏移；入口与模板逐字对齐。

---

## 12. 第二候选路线

| 优先级 | 路线 | 触发条件 | 预期收益 | 主要风险 |
| --- | --- | --- | --- | --- |
| **P1** | **消除标量读回**：归约结果留 UB，向量 `Sqrt`+`Reciprocal` 广播 | 首版跑通后，且 outer 较大 | 形态 B 收益最大 | 向量 Sqrt 精度模式不确定 |
| **P1** | **确认并使用满 48 个向量核** | 首版耗时明显高于预期 | 形态 A 可能翻倍 | `availableCoreNum` 语义需真机确认 |
| **P2** | **单遍暂存**（D ≤ 4096 时启用；D > 4096 回退两遍） | 首版正确且稳定 | GM 流量 −28.6% | UB 峰值 128 KB（D=32768 全行驻留），需真机确认 |
| **P2** | **`BUFFER_NUM = 2` 双缓冲** | 首版为单缓冲 | 重叠 MTE 与 V | UB 占用翻倍 |
| **P3** | 对齐 D 时 `DataCopyPad` → `DataCopy` | 有真机量化数据后 | 待量化（C 级线索） | 非对齐判断逻辑复杂度 |
| **P3** | 归约换 `BlockReduceSum` + `WholeReduceSum` | 有真机 profiling 数据后 | 官方口径更快 | 需确认 9.0.0 可用性 |
| **P3** | gamma/bias 片上常驻、省去 fp32 路径的冗余 UB→UB 拷贝 | outer 大 / fp32 路径 | 减少 GM 读与 UB 拷贝 | UB 预算 |

---

## 13. 风险清单

| # | 风险 | 级别 | 依据 | 对策 |
| --- | --- | --- | --- | --- |
| R1 | **判题端精度判定口径未知**（是否允许 ≤0.1% 失配） | **致命/未知** | 题面未写；模板 `tol=1e-3` 仅为本地口径（S182） | 首版即按「先除」写法规避 bf16 ulp 翻转；真机导出 output 与 golden 逐元素比对 |
| R2 | **V002 从未被真正编译过** | **致命** | L4（上传截断） | 第一步做干净编译 |
| R3 | `TPipe` 作为类成员与官方最佳实践不符，可能是 V001 报错背景 | 高 | S080（A）、H2 | 首版移出类 |
| R4 | 判题机 SoC 未知，`DataCopyPad` 支持矩阵/尾块语义依赖 SoC | 高 | S001（`soc_version: null`）、S025 | 准备非 A2 降级路线（S10 手工尾块 + GatherMask） |
| R5 | 入口限定符与模板不一致 | 中 | S022（**8.5.0alpha002 页**） | 与模板逐字对齐 `__global__ __vector__` |
| R6 | 多核并发写回时 32 B 缓存行撕裂（D×sizeof(T) 非 32 倍数、行间紧凑） | 中 | Agent09/S165 | 多核划分使 `k × D × sizeof(T) ≡ 0 (mod 32)` |
| R7 | 大张量 32 位行偏移溢出（元素数可超 2³²） | 中 | 工程既有约定 | 行基址与偏移强制 `uint64_t` |
| R8 | `workLocal`/`sharedTmpBuffer` 尺寸公式无官方依据 | 中 | Agent02 证据缺口 | 本机核算 1024 够用（余量 2×）；真机复核 |
| R9 | `DataCopyPad` 三处单位不同（`blockLen`/`srcStride` 字节，`dstStride` dataBlock） | 中 | S167（C） | 逐字段核对；写注释 |
| R10 | bf16 精度受自身 ulp 限制（2^-8 ≈ 3.9e-3） | 中 | S143（A） | 依赖判题口径；确保与同 dtype golden 同链 |
| R11 | 上传流程异常（编辑器未全选替换） | 中 | L4 | 提交前四项核对（首行/末行/行数/入口） |
| R12 | 直接使用 `torch_npu` 框架算子（违规） | 中 | S113 | 明令禁止；核心计算必须手写 Kernel |
| R13 | 单 shape 通过 ≠ 15 点通过 | 中 | S164、S170 | 自建覆盖矩阵（见 `精度测试矩阵.md`） |
| R14 | 提交额度与成绩取法 | 低 | S003、S038 | 每日 ≤50 次；保留稳定版本再冲性能；成绩取最后一次 |

---

## 14. 未确认事项（提交前必须闭环）

| # | 事项 | 缺什么 | 去哪补 | 怎么验 |
| --- | --- | --- | --- | --- |
| U1 | **判题端精度判定口径**（是否允许失配比例、rtol/atol/tol 实际取值） | 判题规则原文 | 登录提交页 / 平台规则页 / 结果页误差列 | 真机导出 output 与 golden 逐元素比对，统计失配率 |
| U2 | **上传表单字段名与格式** | 表单 schema | 登录 submit 页 | 与 `submission-checklist.md` 同步 |
| U3 | **判题机 SoC 型号** | SoC 名 | 登录平台 / 结果页日志 | 真机 `npu-smi info`；`msprof` 报告 |
| U4 | **15 点的具体 shape/dtype/epsilon** | 用例配置 | 平台不开放（`visible_testcase_count: 0`） | 无法获得；用覆盖矩阵逼近 |
| U5 | **V002 是否真的能编译** | 一次干净编译 | 有 CANN 9.0.0 的机器 | 跑模板 `run.sh` |
| U6 | `data_utils.h` 的模板本地用例是否可扩展为多 shape | — | 本地模板 | 改 `gen_data.py` 扩 shape，跑本地 harness |
| U7 | `TPipe` 在 9.0.0 里作为类成员是否真的报错 | 官方说明或真机 | S080 全文 + 真机 | 两种写法各编译一次对比 |
| U8 | 向量 Sqrt/Rsqrt 的精度模式在 9.0.0 的默认值 | 文档细节 | S015/S016 | 真机对比 bf16 误差 |
| U9 | `availableCoreNum`（`ACL_DEV_ATTR_VECTOR_CORE_NUM`）与 `GetCoreNumAiv()` 是否一致 | 真机值 | 真机打印 | 打印两者并比较 |
| U10 | `DataCopyPad` 相对 `DataCopy` 的量化开销 | 官方数字 | 真机 profiling | msprof 对比两种写法 |
| U11 | 决赛规则 | 官方公告 | 决赛页面 | 初赛结束后确认 |
| U12 | Linux DO / V2EX 是否有相关帖子 | 命中 | — | 本轮多轮检索**无命中**，已记录检索式 |

**与 Agent10 缺口编号的对应**：U1↔G2、U2↔（提交页）、U3↔G1、U4↔G6、U5↔（V002 编译）、U7↔（`TPipe`）、U8↔G7、U9↔（核数一致性）、U10↔（`DataCopyPad` 开销）；Agent10 另列出 **G5（UB→GM dummy 是否真丢弃，P0 静默损坏）** 与 **G3（`workLocal` 官方公式）**、**G4（`CAST_RINT` 是否 RNE）**，已并入上表 R8/R9 与 U 项。

**三项 P0 blocker（不闭环则无法保证 15 点全过）**：
1. **G1 判题 SoC 型号** —— 影响多个 API 的行为；
2. **G2 判题精度判定口径** —— 决定 bf16 是否需要「先除」修正；
3. **G5 `DataCopyPad` 搬出 dummy 是否真丢弃** —— 静默数据损坏型风险。

---

## 15. 下一阶段实验计划

### 阶段 0（无 NPU，本机可做，已完成）

- [x] 题面/规则/API/仓库/迁移/生成/精度/性能/环境/失败案例 10 路调研
- [x] 来源总表与证据分级（`sources.md`）
- [x] CPU 参考验证：算法结构等价性、分块误差、除法路径、溢出边界
- [x] 本机静态检查清单（`grep` 级）

### 阶段 1（真机第一步：先把「能跑」拿到）

1. 用完整 V002 文件替换模板 `kernel.asc`，跑 `./run.sh` → **拿到一次干净的编译结果**；
2. 若编译失败，按错误定位到具体 API（记录完整日志，不要截断）；
3. 跑通模板默认 FP16 `[1,64]`；
4. 记录 `availableCoreNum`、`GetCoreNumAiv()`、SoC 型号。

### 阶段 2（正确性与精度）

5. 用覆盖矩阵（`精度测试矩阵.md`）自建用例：dtype × rank × D 边界 × outer；
6. 重点验证 D = 67 / 129 / 1000 的**相邻行未被污染**（逐字节）；
7. 导出 output 与 golden 逐元素比对，统计**失配率**与最大绝对/相对误差；
8. 对比「先乘 1/rms」与「先除 rms」两种写法的失配元素数（预期后者为 0）；
9. 对比标量 `sqrtf` 与向量 `Sqrt` 的误差；
10. 验证 `CAST_RINT` 的舍入方向是否为 RNE。

### 阶段 3（性能）

11. 用 `msopst` 记录逐点耗时、用 `msprof op` 记录核内流水与占用率；
12. 按 §12 的 P1 → P3 顺序逐项实验，**每次只改一个变量**；
13. 每次优化后回归 15 点，确保不损伤精度。

### 阶段 4（提交）

14. 内容核对（凭据零写入、无写死、无聚合初始化、64 位偏移、无本机路径）；
15. 汇报 → **用户明确确认** → 才执行 CANNJudge 上传；
16. 记录提交编号、15 点状态、分数与耗时到 `提交/V00N/结果.md`。

---

## 16. 本轮完成情况声明

### 16.1 本轮完成了哪些技术方案调研

已完成 10 个代理的全覆盖调研（题面接口、Ascend C API、官方仓库、GPU 迁移、编译器生成、数值精度、性能 UB、Linux 环境、竞赛失败案例、证据审阅），并产出：

- 覆盖计划 1 份、综合报告 1 份（本文件）、来源总表 1 份（`sources.md`，S001–S194）、方案矩阵 1 份（`方案矩阵.md`）、精度测试矩阵 1 份、CPU 验证记录 1 份、子代理报告 10 份、可复现脚本 4 个 + 运行日志。

### 16.2 哪些方案有源码或官方 API 证据

- **S1 两遍扫描**、**S3 FP32 全中间计算**、**S7 按行分配**、**S9 DataCopyPad 尾块**、**S11 ReduceSum**、**S13 纯 Ascend C**：均有 A/B 级依据（官方 API 文档 + 官方仓库源码 + 本机 CPU 实测）。
- **S5/S6 tile 大小**、**S10 手工尾块**、**S12 手工向量归约**：有 A/B 级 API 依据，但**收益与代价需真机量化**。
- **S2 单遍暂存**：有 A/B 级 UB 容量依据与本机访存计算，**UB 峰值是否真的装得下需真机确认**。

### 16.3 哪些方案仅为迁移参考

- **S14 从 CUDA/Triton/PyTorch 迁移**：全部 GPU 实现只能借「FP32 累加、融合减少片外往返、合并访存、按行并行 + 尾块掩码」四条思想，代码与调优直觉（occupancy / L2 / warp divergence）**在 Ascend 上无对应物**。
- **S8 按 tile 分配 AI Core**：需要跨核归约，官方与社区均无同类证据，仅作研究参考。
- **所有编译器/DSL 生成方案**（msopgen / TileLang / PyPTO / IREE / TVM / TorchInductor）：**没有任何工具能产出符合判题形态的单文件 `kernel.asc`**，仅可借算法模式。

### 16.4 哪些结论仍需真实 CANN / NPU 验证

第 14 章 U1–U11 全部条目，其中**最高优先级三项**是：U1 判题精度判定口径、U2 上传表单格式、U3 判题机 SoC 型号；以及**最关键的动作性结论 U5：V002 是否真的能编译**。

### 16.5 安全与边界

- 本轮**没有**执行 CANNJudge 上传，**没有**消耗任何提交次数；
- 本轮**没有**创建第二套工程、备份工程或重复文档（新文档全部落在 `调研/调研2/`）；
- 本轮**没有**修改 `提交/V00N/`、`源码/`、`文档/` 的既有文件；
- 本轮**没有**在文档、源码或日志中写入任何凭据；
- 所有 NPU 侧结论均标注「未在真实 CANN/NPU 验证」。
