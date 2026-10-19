# 题目分析：AddRmsNormBias

> 依据官网题面原文（2026-09-10 抓取）整理。与用户提供的初版理解有两处差异，已用官网原文核对并以官网为准。

## 1. 数学公式

```
Step 1（残差加法）:
    y_i = x_i + residual_i                       ∀ i ∈ [0, D)

Step 2（RMS 归一化，沿最后一维）:
    rms = sqrt( (1/D) * Σ_{i=0..D-1} y_i^2 + epsilon )
    z_i = y_i / rms * gamma_i                    ∀ i ∈ [0, D)

Step 3（逐通道偏置加法）:
    output_i = z_i + bias_i                      ∀ i ∈ [0, D)
```

- 与用户初版题意的差异点：官方归一化的分母是 **mean(y^2)**（除以 D，而不是别的定义），且 `epsilon` 加在 `mean(y^2)` 之后、开方之前，与 PyTorch `rms_norm` 一致。
- 偏置加法在归一化**之后**（`z + bias`），不在归一化内部。

## 2. 输入输出关系

| 参数 | 方向 | 形状 | dtype（与 x 一致） | 说明 |
| --- | --- | --- | --- | --- |
| x | 输入 | (..., D) 2D/3D/4D | fp16 / bf16 / fp32 | 主输入 |
| residual | 输入 | 与 x 完全一致 | 同 x | 残差 |
| gamma | 输入 | (D,) | 同 x | 缩放系数 |
| bias | 输入 | (D,) | 同 x | 逐通道偏置 |
| epsilon | 属性 | — | float | 默认 1e-5 |
| output | 输出 | 与 x 相同 | 同 x | 结果 |

约束：`output.shape == x.shape`；`residual.shape == x.shape`；`gamma.shape == bias.shape == (D,)`。

## 3. 数据类型

- 支持：float16、bfloat16、float32，输出与输入同型。
- 计算精度策略：**归约与中间累加必须使用 FP32**（官方训练营"黄金法则"），f16/bf16 输入先 Cast 到 FP32 再平方求和，最后输出时 cast 回原类型（仅一次量化）。
- 判定阈值：fp32 → 相对/绝对 <1e-4；f16/bf16 → 相对/绝对 <1e-3。
- 判定实现（2026-09-11 核对）：本地模板 `verify_result.py` 为 `np.isclose(rtol=0.001, atol=0.001, equal_nan=True)` 逐元素 + 失配容忍 0.1%（判题端是否一致未证实，见 11 节）。
- **Cast 舍入（2026-09-11 核对，A 级）**：fp16/bf16→fp32 用 `CAST_NONE`（精确）；输出 fp32→half/bf16 用 `CAST_RINT`（=RNE，与 numpy 一致；A2 上 fp32→bf16 无 CAST_NONE 可用）。**A2 的 Add/Mul 不支持 bfloat16_t，必须在 FP32 域计算**。
- NaN 输入 → 对应位置输出 NaN，不能崩溃；Inf 输入 → 按数学公式得 Inf/NaN（Inf/Inf=NaN），不能崩溃；禁止 clamp/分支。

## 4. 张量维度

- 支持 2D（batch, D）、3D（batch, seq, D）、4D（batch, seq, heads, D）。
- batch ∈ [1, 8192]，seq_len ∈ [1, 32768]，**D ∈ [64, 32768]**（官网约束；用户初版写“D<=32768”，一致；官网下限 64）。
- 实现按“沿最后一维 D 归约，其余维度积 outer = batch*seq*heads 作为行数”统一展平处理。

## 5. 广播关系

- gamma、bias 为 (D,)，沿最后一维广播到每个样本；每行（一个 outer 行）共享同一份 gamma/bias。
- residual 与 x 形状完全一致，无广播（逐元素相加）。本题无跨维广播需求，实现上直接逐元素搬运对齐即可。

## 6. epsilon 处理

- 属性类型 float，默认 1e-5；取值范围通常在 1e-5 ~ 1e-6。
- 顺序必须是 `sqrt(mean(y^2) + eps)` —— eps 在开方**之前**、加在均值上（不是开方之后）。
- 防止 rms=0 除零；实现按 float 计算并参与 FP32 计算链，避免低精度截断。

## 7. D 非 32 倍数 / 非对齐边界处理

- 约束：D 可能不是 32 的整数倍（题面示例 D=192、576；更一般地 D 可为 64..32768 任意值）。
- 昇腾 DMA（MTE2/MTE3）对 `DataCopy` 有 32B 对齐/长度约束；本项目使用 **DataCopyPad** 处理：
  - 搬入：GM→UB 任意字节长度，不足部分自动补 0（正好满足归约需要：补 0 不影响平方和）。
  - **【字段顺序｜调研2 已定案，2026-09-11】**：`DataCopyPadExtParams<T>` 的声明顺序为 **`{isPad, leftPadding, rightPadding, paddingValue}`**，依据是 CANN 9.0.0 官方 API 页（`hiascend.com/.../CANNCommunityEdition/900/.../atlasascendc_api_07_0265.html`，A 级）。旧笔记写的 `isPad, paddingValue, leftPadding, rightPadding` **是错的**，该「证据冲突」已消解。当前 `提交/混合方案/H001-正确性优先/V002/kernel.asc` 使用**逐字段赋值**（`pad.isPad/…`），对声明顺序不敏感，写法正确。**规范照旧：逐字段赋值或 Designated Initializers；禁止裸聚合初始化。** 来源与裁决过程见 `../调研/归档/调研2/Agent02_官方Ascend_C_API.md`、`../调研/归档/调研2/sources.md`（S010）。
  - 搬出：UB→GM 目的地址无对齐约束（手册明确 Global 地址无对齐约束），DataCopyPad 支持非对齐搬出，入参必须指定实际有效字节数 `valid_len * sizeof(T)`。
  - **【并发总线踩踏防范】多核 32B Cache Line 撕裂**：当 $D \times \text{sizeof}(T)$ 非 32 字节整数倍时，全局内存中行与行连续紧凑排布，前一行的尾块与后一行的头块落入**同一个 32 字节物理缓存行**！若多核按行均分并发写回，无锁 DMA 将触发总线踩踏冲突。**多核切分必须以使得 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$ 的最小行数整数倍 $k$ 为粒度分配**。
  - **【大张量 64 位寻址防溢出】**：在大 Batch 或 3D/4D 场景下，全局元素总数极易突破 $2^{32}-1$（约 42.9 亿元素）。**行基址计算严禁使用 `uint32_t`，必须强制声明为 `uint64_t base = static_cast<uint64_t>(row) * dim_;`**。
  - A2 支持情况：Atlas A2 训练系列/Atlas 800I A2 推理产品支持 DataCopyPad（无 mode 参数版本）；Atlas 200/500 A2 不支持，需 fallback（GatherMask/atomic），当前按 A2 主路径实现。
  - **写方向语义｜调研2 已裁决，2026-09-11**：官方 A 级文档（CANN 9.0.0 搬出接口页）明确 **UB→GM 搬出时 UB 侧补的 dummy 在映射到 GM 时被丢弃，不写相邻内存**；社区相反实测仅为 C 级、未找到可靠实证。裁决结论：**采官方口径，但风险等级仍为「高（静默数据损坏型）」**——因为它不崩溃、难发现，且依赖判题机 CANN 版本与 SoC 行为与文档一致。**真机第一验证项保留**：构造 D 非 32B 对齐尾块 + 紧邻有效数据的缓冲，用非对齐 `DataCopyPad` 搬出后读相邻内存是否被改写（D=67/129/1000）。若被证伪 → 切换到备选路线「手工尾块」（掩码/Duplicate/GatherMask）。
  - **ReduceSum count 上限｜调研2 已裁决，2026-09-11**：官方仅约束「临时空间 ≤ UB 限制」，**不存在 255 / 4096 / 16320 这类单一硬上限**——那些数字分别是不同实现路径（repeat 上限 / 保守安全值 / 255×64 fp32 最大 count）的口径。`workLocal` 尺度按社区推导公式核算：fp32 路径 count 最大 2048 → 需 256 个元素；fp16/bf16 路径（FP32 域归约）count 最大 4096 → 需 512 个元素；当前 `WORK_LEN = 1024` **足够（余量 2×）**。工程保守处置照旧：**分块 ≤ 4096**。官方未给出该公式，真机仍需复核。
  - 向量计算尾块：Vector 指令的 mask 连续模式（fp16 ∈[1,128]、fp32 ∈[1,64] 每 repeat）可用于限定有效元素，冗余数据已清零则不影响结果。

## 8. 15 个测试点要求与得分规则

- **15 个测试点 = 官方明文（2026-09-11 API 确认，A 级）**：题目 API desc 原文"共15个测试点，全部通过才计分"；排名页 15 列 + testcases 数组 15 条三方印证；无隐藏测试点证据。
- **得分公式（A 级，已实测验证）**：单点 `100/(1+log₁.₅(t/T))`，T=该点全局最优时间，总分=15 点均值，同分按提交时间；性能是唯一计分维度。每点执行/统计 iterations=5。
- **判题端精度判定细节、15 点 shape/dtype/epsilon 配置：平台不开放，未证实**。
- 本地验证计划以覆盖矩阵逼近判题组合（见 `submission-checklist.md`）：
  - dtype × {fp32, fp16, bf16}
  - rank × {2D, 3D, 4D}
  - D 边界 × {64, 96, 192, 576, 1024, 4096, 32768} ∪ 非 32 倍数 {67, 129, 1000}
  - outer × {1, 8, 8192} 与 seq 大形状抽查

## 8a. 判题接口（模板，A/B 级）

- 平台模板为 CANN 9.0 **Direct Invocation 直调工程**：`kernel.asc` 定义 `run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, …, int64_t availableCoreNum, aclrtStream stream, float epsilon)`，内部 `<<<blocks,nullptr,stream>>>` 启动 `__global__ __vector__` 核函数；`TensorInfo.dtype` 枚举 0=fp32/1=fp16/2=bf16；epsilon 由判题端传入（默认 1e-5）。
- 模板 main.asc 默认用例 FP16 [1,64]，epsilon=1e-5，`ACL_DEV_ATTR_VECTOR_CORE_NUM` 取核数。
- **入口限定符（2026-09-11 发现）**：A 级文档称纯向量算子 `__aicore__` 与 `__vector__` 均合法；C 级官方指南转述称纯向量必须 `__vector__`。当前候选为 `__aicore__`，提交前建议与模板逐字对齐 `__global__ __vector__`。

## 8b. 调研2（2026-09-12 重做）确认的新事实

> `调研/归档/调研1/` 已归档；本轮十代理调研在 `调研/归档/调研2/` 重做，以下为相对 2026-09-11 的**新增或升级**结论（A 级为主）。完整报告：`../调研/归档/调研2/research-report.md`。

1. **fp32 精度阈值是 1e-4，不是 1e-3**（题面 §五，A 级）：fp32 相对/绝对 `<1e-4`；fp16/bf16 `<1e-3`。本地 `verify_result.py` 只有 1e-3，**会漏判 fp32**。
2. **评分公式的 `T` 是实时最优，不是固定基线**（题目 API `use_baseline=False`）：`100/(1+log₁.₅(t/T))`，登顶即 100 分；总分为 15 点均值；每点 `iterations=5`（聚合方式平台未说明）。
3. **每日 50 次提交且取最后一次成绩**（GitCode 规则页 + 平台 `ranking_submission_mode=latest`；GitCode 页同段"取最优"与平台字段矛盾，以平台 API 为准）。
4. **判题 SoC 仍未确认**：题目 API 无 `soc_version` 字段；`dav-2201` 只是模板编译默认值。硬件参数（UB=192KB、48 AIV）为规格表转引（B 级），**推导出的 UB/带宽数字均为推算**。
5. **官方开源仓的真实位置是 GitCode 不是 GitHub**：`Ascend/ops-transformer`、`Ascend/ops-nn`、`Ascend/cann-samples` 在 GitHub **全部 404**；源码在 `gitcode.com/cann/*`。
6. **官方 AddRmsNorm 源码存在（有残差融合）但都无 `+ bias`**，且强依赖 `AddRMSNormTilingData` 结构体 → 本题必须自己融合 bias，并在核内运行时推导 tiling 参数。
7. **`Divs` 与 golden 逐位一致（36 组配置最大绝对差恒为 0）**；`Muls(1/rms)` 风险集中在「大 outer 小 D」，bf16 最大绝对差达 1.562e-02。**首版强制 `Divs` + `Sqrt`。**
8. **bf16 在 A2 上 `Add`/`Mul`/`Muls`/`Div`/`ReduceSum` 均不支持**（9.0.0-beta.2 明文），必须 Cast→fp32→Cast。
9. **`ReduceSum` 无官方 255/4096/16320 硬上限**，仅受 UB 约束；`GetReduceSumMaxMinTmpSize` 为 9.0.0 host 接口且 max==min。
10. **没有任何自动生成工具能产出判题形态的单文件 `kernel.asc`**（10 个工具逐项评估，B 类为空）。
11. **公开统计（2026-09-12）**：通过率 61%、通过 240 人、尝试 391 人。

## 9. 实现风险

| 风险 | 级别 | 对策 |
| --- | --- | --- |
| `DataCopyPadExtParams` 参数倒置致死 Bug | 致命/阻断 | 严禁聚合初始化，强制使用具名初始化锁死 `rightPadding` |
| 多核非对齐 DMA 写回 32B Cache Line 踩踏 | 致命 | 多核划分基于 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$ 行块粒度对齐 |
| 大张量 32 位全局偏移溢出 | 严重 | 基址与偏移指针强制提升为 `uint64_t` |
| `GetReduceRepeatSumSpr` 可移植性（调研2） | 中 | A 级：CANN 9.0.0 专页存在此接口；与 ReduceSum 实现绑定，跨版本语义不透明。主线仍用 `ReduceSum` + V_S + `GetValue(0)` |
| FP16/FP32 混算导致归约溢出/下溢 | 高 | 一律 FP32 归约与累加，末尾一次 cast |
| ReduceSum 的 dst/work 布局错误或 workLocal 空间不足 | 高 | 按官方 workLocal 空间公式预留；dst 起始 4B 对齐、src 32B 对齐 |
| 尾块复制越界覆盖下一行 | 高 | CopyIn 用带填充 DataCopyPad；CopyOut 用非对齐 DataCopyPad，长度按实际字节 |
| GetValue/标量同步导致性能退化（大 outer 时放大） | 中 | 整行仅在行末执行一次标量提取同步；备选方案探索全向量流水线 |
| 多核负载不均（outer 不能被核数整除） | 中 | 按行切分，末尾核处理剩余行（每核行数差 ≤1） |
| SoC 判题型号与本地开发型号不一致导致指令缺失 | 高(待确认) | 目标架构按模板默认 `dav-2201` 编译；备选路线 10（手工尾块） |
| DataCopyPad 搬出写方向 padding 覆盖相邻行（矛盾 1） | 高 | 真机第一验证项：D=67/129/1000 逐字节核对；备选手工尾块 |
| ReduceSum(count) 上限（4096/16320 争议） | 高 | 分块 ≤4096；half tile 4096 降 2048 或真机确认 |
| 标量 sqrtf 精度不足（bf16） | 高 | 改向量 Sqrt（0 ulp）或正确舍入 rsqrt（误差 ≤2^-20） |
| 入口限定符 `__aicore__` vs `__vector__` | 中 | 提交前与模板逐字对齐 `__global__ __vector__` |
| bf16 大 D 顺序归约误差（D=32768 0.16% > 0.1%） | 中 | 分块 ≤4096 + 块间合并（实测 <0.01%） |
| 中间 Cast 时机与 golden 不同步（S8 案例） | 中 | 与 golden 同链：先量化输入→FP32 计算→一次 cast 输出 |
| msopgen 模板 API 版本差异（CANN 9.0.0 与 8.x 的 tiling/宏差异） | 中 | 判题为直调模板（非 msopgen 算子工程）；以 9.0.0 头文件为准 |
| int32 行（精度表）为模板遗留 | 低 | 忽略，正式输入 dtype 不含 int32 |

## 10. 性能优化路线（先正确后性能，供真机阶段实施）

1. v1（V003 基准）：按行对齐切分 + 两遍扫描（一遍归约、一遍归一化输出），闭环修复四大合规项，正确性绝对优先。
2. v2（单遍突破）：在 $D \le 4096$（覆盖绝大多数大模型隐藏维度）激活单遍暂存路线，UB 精细规划（峰值 80~112KB），直接省去 28.6% 的 GM 访存带宽；$D > 4096$ 回退两遍扫描。
3. v3（权重片上常驻）：当整张量 $D \le 4096$ 且 outer 极大时，将 $\gamma$ 和 $\text{bias}$ 仅搬入一次并常驻 UB，供同一核所有分配行重复复用。
4. v4（双缓冲流水线）：UB 分块与 TQue Ping-Pong 双缓冲重叠搬运与计算（CopyIn/Compute/CopyOut 流水）。
5. 每步在真机用 msopst / npu-smi 记录耗时与误差，防止优化损伤精度（FP32 中间计算链保持）。
