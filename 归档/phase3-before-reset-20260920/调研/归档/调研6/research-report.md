# AddRmsNormBias 深度技术方案调研报告（调研2）

> **调研日期**：2026-09-12
> **组织方式**：主代理本地核对 + 10 个平台/主题互斥的子代理并行调研 + Agent 10 独立审阅裁决 + 主代理合并复核
> **产物目录**：`调研/调研2/`（本报告 + `sources.md` + `方案矩阵.md` + `精度测试矩阵.md` + `validation-host-notes.md` + `平台与研究覆盖计划.md` + `agents/agent01…agent10-*.md` + `数值参考脚本/`）
> **来源总量**：235 条（S001–S291 区间），见 `sources.md`
>
> **边界声明（必须首先阅读）**：本机为 macOS，**无 CANN 工具链、无昇腾 NPU、无 Docker**。本轮只做了资料核对、模板与源码审阅、CPU 参考数值计算和静态分析。
> **本轮未执行 CANNJudge 上传、未消耗任何提交次数、未修改 `源码/` 与 `提交/` 下任何文件、未创建第二套工程。**
> 本报告**不声称**任何 NPU 编译通过、精度通过或性能达标。凡涉及 NPU 的结论一律标注为「未在真实 NPU 验证」或「推算」。

---

## 1. 调研范围与平台覆盖

### 1.1 十代理分工与实际覆盖

| # | 主题 | 平台 | 报告 | 来源数 | 平台可达性 |
| --- | --- | --- | --- | --- | --- |
| 1 | 题面、规则与提交接口 | CANNJudge、GitCode 赛事页、题目/赛事公开 API | `agent01-problem-and-submit.md` | 15 | **可达**（题面页 + 3 个 API 全通）；测试点详情 403 |
| 2 | 官方 Ascend C API | hiascend.com/.cn、asc.gitcode.com、本地缓存 `临时/api-pages/` | `agent02-ascendc-api.md` | 30 | 部分页面 JS 渲染，用本地缓存补足；18 项 API 全覆盖 |
| 3 | 官方开源仓库 | GitCode `cann/*`、GitHub `Ascend/*` | `agent03-official-repos.md` | 25 | **GitHub 侧 404，GitCode 侧可达**；已克隆并逐行读源码 |
| 4 | GPU / CUDA / Triton / PyTorch | NVIDIA 论坛、CUDA Samples、PyTorch、Triton、GPU MODE、ROCm | `agent04-gpu-migration.md` | 15 | 可达；4 份真实源码片段已核验 |
| 5 | 编译器 / IR / 算子生成 | LLVM/MLIR Discourse、TVM、OpenXLA、IREE、TorchInductor、TileLang、PyPTO、msopgen | `agent05-compiler-codegen.md` | 30 | 可达；10 个工具逐项评估 |
| 6 | 数值精度与验证 | PyTorch Issue/PR、NumPy、MindSpore、论文 + **本机 CPU 实测** | `agent06-numerics.md` | 16 | 可达；已跑 425 行脚本，有运行日志 |
| 7 | 性能 / UB / 硬件架构 | Ascend 硬件与 Profiling 文档、社区性能案例、Nsight/ROCm/HPC | `agent07-perf-ub.md` | 23 | 可达；硬件参数多为 B 级转引 |
| 8 | Linux / 真机工程环境 | Linux DO、V2EX、Stack Overflow、Ask Ubuntu、Server Fault、HPCwire、Phoronix | `agent08-env-linux.md` | 30 | **Linux DO 未命中（受限）；V2EX 仅 1 条真实命中**（已如实记录，未编造） |
| 9 | 竞赛经验与失败案例 | GitHub Issue/PR、GitCode、Kaggle/AIcrowd/Codeforces、CANN 社区 | `agent09-failure-cases.md` | 30 | 可达；18 个真实案例 |
| 10 | 证据审阅与方案合并 | 不新增外部平台，只审阅 A1–A9 + 本地一手材料 | `agent10-synthesis.md` | 21 | — |

### 1.2 覆盖情况自评

- **已充分覆盖**：题面与接口、Ascend C 官方 API、官方开源实现、GPU 迁移对照、自动生成路线评估、数值精度（CPU 实证）、UB 与带宽推算、真机环境准备、竞赛失败案例。
- **部分覆盖**：官方 API 的少量页面因 JS 渲染只拿到表格或标题，已用本地缓存交叉验证并标注 `partial`。
- **未覆盖 / 受限**：Linux DO 站内搜索 `fetch failed` 且搜索引擎未索引到昇腾相关帖；V2EX 仅 1 条命中（未达"各 3 条"预期，如实说明）；判题端测试点配置、判题 SoC、判题端判定容差平台未开放。

### 1.3 本轮相对上一轮（2026-09-11）的新增事实

1. **官方 fp32 精度阈值是 1e-4，比本地模板的 1e-3 更严**（题面 §五，A 级）→ 本地自测必须按 1e-4 检验 fp32。
2. **每日 50 次提交且取最后一次成绩**（GitCode 规则页 + 平台 `ranking_submission_mode=latest`，两处对"最优/最新"表述矛盾，以平台 API 为准）。
3. **`T` 不是固定基线**：题目 API `use_baseline=False` → `T` 为实时最优，登顶即 100 分。
4. **官方开源仓的真实位置在 GitCode 不在 GitHub**（`Ascend/ops-transformer`、`Ascend/ops-nn`、`Ascend/cann-samples` 在 GitHub 全部 404）。
5. **官方 AddRmsNorm 源码存在但都无 bias 融合**，且**强依赖 `AddRMSNormTilingData` 结构体**——与本题"无 tiling 结构体"直接冲突。
6. **公开统计更新**：通过率 61%、通过 240 人、尝试 391 人（2026-09-12）。
7. **CPU 实证**：`Divs` 与 golden 逐位一致；`Muls(1/rms)` 在 bf16 有 1–28 元素越界；fp16 平方在 `|y|≥256` 溢出；低精度中间累加被证否（bf16 50%+ 失配）。
8. **独立复核确认**：没有任何自动生成工具能产出判题形态的单文件 `kernel.asc`（10 个工具逐项评估，B 类为空）。

---

## 2. 题目约束（合并自 Agent 1 + 本地模板核对）

### 2.1 语义（A 级，golden 唯一权威 = `scripts/AddRmsNormBias.py`）

```text
y       = x + residual
rms     = sqrt( mean(y^2, axis=-1) + epsilon )     # eps 在 mean 之后、开方之前
z       = y / rms * gamma                          # 注意是「除法」不是「乘 1/rms」
output  = z + bias                                 # bias 在归一化之后
# 全程 FP32 计算，最后一次性 cast 回原 dtype
```

### 2.2 接口与约束表

| 项 | 值 | 证据 |
| --- | --- | --- |
| 输入 x / residual | `(..., D)`，同 shape 同 dtype | 题面 §3.3（A） |
| gamma / bias | `(D,)`，与 x 同 dtype | 题面 §3.3（A） |
| output | 与 x 同 shape 同 dtype | 题面 §3.6（A） |
| dtype | fp16 / bf16 / fp32（题面 §3.4 仅此三者；"int32 完全精确"为模板残留） | 题面（A）+ 模板 dtype 枚举 0/1/2 |
| rank | 2D(batch,D) / 3D(batch,seq,D) / 4D(batch,seq,heads,D) | 题面 §3.4（A） |
| 范围 | batch∈[1,8192]、seq∈[1,32768]、**D∈[64,32768]** | 题面 §3.4（A） |
| 非对齐 | D 可能不是 32 的整数倍 | 题面 §3.4（A） |
| epsilon | float，默认 `1e-5`，由判题端作为 `run_kernel` 末位参数传入 | 题面 §3.5 + `main.asc`（A） |
| 特殊值 | NaN/Inf 输入须按公式传播且不崩溃；同输入多次执行结果必须完全一致 | 题面 §3.7 / §四（A） |
| 判题形态 | Direct Invocation 直调单文件 `kernel.asc`；`extern "C" run_kernel(...)` + `<<<blockNum,nullptr,stream>>>` | 模板（A） |
| 元信息 | `TensorGroupInfo`/`TensorInfo` **运行时**传入；**无 tiling 结构体、无 Host 侧 shape 常量** | 模板（A） |
| 核数 | `availableCoreNum`（`main.asc` 用 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 查询后传入） | 模板（A） |
| 编译架构 | 模板默认 `SOC_ARCH="dav-2201"`，可被 `NPU_ARCH` 覆盖；**判题真实 SoC 未在任一公开接口出现** | 模板（A）+ API（未确认） |
| kernel 类型 | `kernel_pattern=vector`；CANN `9.0.0` | 题目 API（A） |
| 超时 | `run.sh` 中 `timeout 120` 秒 | 模板（A） |

### 2.3 评分与规则（A 级）

- **15 个测试点，全部通过才计分**（题面 §六 + API `testcases` 数组恰 15 项）。
- 单点得分 `100/(1+log_1.5(t/T))`，`T` = 该点**实时最优**（`use_baseline=False`）；总分 = 15 点均值；同分按提交时间早者优先。
- 每点 `iterations=5`；5 次如何聚合成 `t`（最好/平均/中位数）**平台未说明**。
- 精度阈值：fp32 相对/绝对 `<1e-4`；fp16/bf16 `<1e-3`。**是否允许"失配比例 tol"平台未开放**（本地 `verify_result.py` 有 `tol=1e-3`，判题端未知）。
- **每天最多 50 次提交，取最后一次成绩**。
- 违规：核心计算必须在 NPU 以 Ascend C 完成；Host CPU 代算、空 kernel 占位 → 取消该次成绩。
- 赛程：初赛 2026-10-17 18:00 截止（17:30 封榜），前 32 强晋级决赛。

### 2.4 对实现的直接约束

1. **shape/rank/dtype 只能运行时分支** —— 不能依赖任何编译期常量或 Host 侧 tiling。
2. **fp32 必须按 1e-4 自测**，本地模板的 1e-3 会漏判。
3. **性能是唯一计分维度，但 15 点全过是准入门槛** → 正确性绝对优先。
4. **取最后一次成绩** → 不能把未验证的大改直接提交；应先验证通道，再增量加复杂度。

---

## 3. Ascend C 官方 API（CANN 9.0.0）

完整 18 项 API 签名表见 `agents/agent02-ascendc-api.md` §2。此处只列与本题强相关的结论与裁决。

### 3.1 关键 API 与本题的对应

| API | 9.0.0 要点 | 本题用途 |
| --- | --- | --- |
| `GlobalTensor::SetGlobalBuffer` | 绑定 GM 地址与长度 | 运行时读 shape 后绑定 x/residual/gamma/bias/output |
| `LocalTensor::GetValue/SetValue` | 单点读写 UB 元素 | 读回归约结果标量 |
| `DataCopy` | 有 32B datablock 对齐约束 | 对齐场景的主搬运 |
| `DataCopyPad` | GM→UB 带 `DataCopyPadExtParams`；**UB→GM 重载无 padParams** | 非对齐 D 的尾块搬运 |
| `ReduceSum(dst, src, sharedTmpBuffer, count)` | A2 仅 **half/float**（无 bf16）；`count` 为 `int32_t`，**无官方硬上限** | 沿 D 求 `sum(y²)` |
| `Cast(dst, src, roundMode, count)` | fp32→bf16 支持 `CAST_RINT/FLOOR/CEIL/ROUND/TRUNC`；**bf16/half→fp32 无损无舍入** | 输入提升到 FP32、输出降回原类型 |
| `Add` / `Mul` / `Muls` / `Div`/`Divs` | **A2 上均不支持 `bfloat16_t`**（仅 half/float/int） | FP32 域的加减乘除 |
| `Sqrt` / `Rsqrt` | A2：half/float | 求 rms |
| `TPipe::InitBuffer` / `TQue` / `TBuf` | UB 分配与队列 | 三段式流水 |
| `PipeBarrier<pipe_t>` + `HardEvent` | `PIPE_V/MTE2/MTE3/S`；`V_S`/`S_V`/`MTE2_V` | 归约后的标量同步 |
| `GetBlockIdx` / `GetBlockNum` | 多核分片 | 按行分核 |
| `__global__ __vector__` vs `__aicore__` | 模板用 `__vector__`；A2 耦合架构下两者均合法（`__vector__` 不生效，等价 `__aicore__`） | 入口限定符 |

### 3.2 五个"卡脖子"问题的裁决（Agent 2 裁定，Agent 10 复核）

| # | 问题 | 裁决 | 等级 |
| --- | --- | --- | --- |
| 1 | `DataCopyPadExtParams<T>` 字段序 | **`{isPad, leftPadding, rightPadding, paddingValue}`**。项目旧笔记的 `{isPad, paddingValue, leftPadding, rightPadding}` 是错的。**始终按成员名赋值**以免疫版本歧义 | A |
| 2 | UB→GM 搬出是否污染相邻 GM | **不污染**。UB→GM 重载无 padParams，框架写 GM 时丢弃对齐 dummy。前提：用真实有效 `blockLen`（可非对齐），**不要把 UB 手动圆整到 32B 后整体搬出** | A |
| 3 | `ReduceSum` count 硬上限 | **官方未给出 255/4096/16320 这类硬上限**，仅约束"不超过 UB 大小限制"。`GetReduceSumMaxMinTmpSize` 为 9.0.0 host 接口且 max==min | A |
| 4 | A2 上 `Add`/`Mul`/`Muls`/`Div` 是否支持 bf16 | **不支持**（9.0.0-beta.2 明文：`Add` 不支持 `bfloat16_t` 源操作数）。bf16 必须 Cast→fp32→Cast | A（beta.2 明文）；正式版 Mul/Div 产品表为同族推断（B） |
| 5 | `Cast` fp32→bf16 的 roundMode | 支持 `CAST_RINT/FLOOR/CEIL/ROUND/TRUNC`；**bf16/half→fp32 无损** | A |

### 3.3 版本敏感点

- 用 8.x 文档支撑 9.0.0 结论的风险已被显式标注（Agent 2 §6）。凡影响实现的 API（尤其是 bf16 算术支持），**真机第一验证批必须复核正式版产品支持表**。
- 硬件数字（UB=192KB / 48 AIV）来自规格表转引（B 级），**不是 9.0.0 官方接口文档**。

---

## 4. 开源实现对比

完整分析见 `agents/agent03-official-repos.md`（含逐行源码摘录）。

### 4.1 仓库矩阵

| 仓库 | 关键路径 | commit/版本 | 是否 AddRmsNorm | 是否有 bias 融合 | 是否量化 |
| --- | --- | --- | --- | --- | --- |
| `gitcode.com/cann/ops-transformer` | `mc2/matmul_all_reduce_add_rms_norm/op_kernel/`（`add_rms_norm.h`、`_split_d.h`、`_single_n.h`、`_multi_n.h`、`_merge_n.h`、`rms_norm_base.h`、`reduce_common.h`） | 9.0.0 分支 `efca5d19` | **是**（残差融合） | **否** | 否（同目录有 `*_quant` 变体） |
| `gitcode.com/cann/ops-nn` | `norm/add_rms_norm/op_kernel/`（`add_rms_norm.h`、`_single_n.h`、`_split_d.h`、`_multi_n.h`、`_merge_n.h`）；另有纯 `rms_norm`、`add_rms_norm_quant` | 9.0.0 分支 `fcebf031` | **是** | **否** | 否（另有量化变体） |
| `gitcode.com/cann/cann-samples` | `Samples/2_Performance/rms_norm_quant_story/src/*.asc`（`0_naive`~`6_binary_sum`）、`simd_vf_story/reduce/src/reduce_*.asc` | master `23c981c0` | RMSNormQuant（**无残差**）+ ReduceSum 原语 | **否** | **是**（int8 输出） |
| GitHub `Ascend/ops-transformer` 等 | — | — | **不存在（404）** | — | — |

### 4.2 可直接借鉴的共性做法

1. **多核按「行」切分，尾核拿余数行**：`blockFactor = ceil(numRow / blockNum)`；尾核 `rowWork = numRow - (blockNum-1)*blockFactor`。这是最值得照搬的范式。
2. **大 D 两种成熟范式**：(a) `split_d` 两遍法（把 D 切成 `ubFactor` 块累加平方和，再统一开方后第二遍乘回）；(b) `__simd_vf__` 按 64 元素分块 + `UpdateMask` 掩码尾块法（天然处理非 32 倍数）。
3. **cann-samples 的 `*.asc` 直调形态与判题形态几乎一致**：`__global__ __aicore__ __vector__ void ...(...)` + `<<<blockNum, 0, stream>>>`。这是最接近本题的真实样例。

### 4.3 与本题的差异（不能直接照搬的地方）

| 差异 | 说明 | 处置 |
| --- | --- | --- |
| **无 bias 融合** | 官方 AddRmsNorm 只有 `y/rms*gamma`，`bias` 字样全是 `gm_bias`（GM 偏移变量） | **必须自己把 `+ bias` 融合进输出阶段** |
| **强依赖 `AddRMSNormTilingData`** | 官方从 Host tiling 传入 `num_row/num_col/block_factor/row_factor/ub_factor/epsilon/avg_factor` | 本题无 tiling 结构体 → **必须在核内用运行时 shape + 核数推导** |
| **RMSNormQuant 不等价** | 带 `quant_scale/quant_offset`、输出 int8；末尾的 `Adds(offset)` 是**量化偏移不是 bias** | 明确排除，不得当等价实现 |
| 形态差异 | 官方是完整算子工程（op_host + op_kernel） | 判题是直调单文件 |
| dtype 三态 | 官方样例多为 half/float 单路径 | 本题需运行时三分支 |

---

## 5. GPU / NPU 迁移分析

完整 15 行迁移对照表见 `agents/agent04-gpu-migration.md` §3。

### 5.1 可迁移的"思想"（不是代码）

1. **两遍 vs 单遍的带宽账**：GPU 侧用 `MAX_FUSED_SIZE=64KB`（SMEM）判定单遍可行性；昇腾要用 **UB=192KB** 重算，不能照搬 64KB。
2. **行级并行粒度**：把归约限制在单核内，避免跨核同步。
3. **非对齐尾块用 mask 补零**，而不是写分支。
4. **FP32 累加 + 最后一次性 cast**（PyTorch `acc_type` 的同一思想）。
5. **fused epilogue**：归一化后直接乘 gamma、加 bias，不额外落 GM。

### 5.2 不可直接迁移（**≥8 个无对应物**）

| GPU 机制 | 昇腾情况 |
| --- | --- |
| warp shuffle（`__shfl_xor_sync`） | 无 warp/线程概念；用 `ReduceSum`/`BlockReduceSum` |
| shared memory 与 bank conflict 手工优化 | 无此层；UB 由 `TPipe::InitBuffer` 静态分配 |
| blockDim / gridDim / occupancy | 无；只有 `GetBlockIdx()`/`GetBlockNum()` 与 `availableCoreNum` |
| `atomicAdd` 跨 block 归约 | 无对应物；跨核归约代价极高，应避免 |
| persistent CTA | 无对应物 |
| Triton `autotune` / `num_warps` | 无在线搜索；只能用静态经验分块 + 编译期档位 |
| `__restrict__` / PTX 内联 | 无对应物 |
| Nsight profiling | 用 `msprof op` + `OpBasicInfo.csv` / `PipeUtilization.csv` |

**判定**：GPU 实现一律标记为「迁移参考 / 仅作研究参考」，**不得**作为提交方案（对应方案 S14 = 不建议）。

---

## 6. 数值精度方案

完整实验（脚本 + 日志）见 `数值参考脚本/agent06_precision_matrix.py` 与 `.log`，矩阵定义见 `精度测试矩阵.md`。**全部为 CPU 参考，非 NPU 实测。**

### 6.1 六个数值问题的结论（CPU 实证）

| # | 问题 | 结论 |
| --- | --- | --- |
| Q1 | `Muls(y, 1/rms)` vs `Divs(y, rms)` | **`Divs` 与 golden 逐位一致（失配=0）**；`Muls` 在 fp32/fp16 失配=0，但 **bf16 有 1–28 个越界元素**（比例 ≤0.0003%，未翻车但非零风险）→ **用除法** |
| Q2 | `1/Sqrt` vs `Rsqrt` vs `Sqrt`+除法 | **`Sqrt` + 除法** 相对误差 = 0，最贴近 golden；`Rsqrt` 的 ~1 ulp 硬件舍入 CPU 无法复现 |
| Q3 | 分块累加 vs 一次累加 | 平方和相对误差 **≤3.0e-6**，远小于 1e-4 → **分块不是误差来源** |
| Q4 | fp16 平方溢出阈值 | **`|y| ≥ 256.0` 时 `y*y = inf`**（fp16 max=65504，√≈255.94）→ 平方和必须在 FP32 算 |
| Q5 | bf16 大 D 顺序 vs 分块归约 | FP32 下两种顺序失配比例 **≤0.001%**，顺序不影响精度 |
| Q6 | 全程低精度中间累加（对照） | **证否**：bf16 **50%+ 失配**（灾难性）；fp16 在 D=1024 达 **0.13% > 0.1%** 翻车 → **FP32 累加必需** |
| Q7（附加） | epsilon 内置 vs 外置 | 外置 eps 在 bf16 全零行 **0.064% 失配** → **必须内置**（`mean(y²)+eps` 后再开方） |

### 6.2 精度策略（明确到操作）

```text
入口：Cast(fp16/bf16 → fp32)            # bf16 必须转，A2 不支持 bf16 算术
残差：Add(y, x, residual)                # FP32
平方：Mul(yy, y, y)                      # FP32
归约：ReduceSum(sum, yy, work, count)    # FP32，分块 ≤ 4096 保守值
均值：Muls(mean, sum, 1/D)               # FP32
加 eps：Adds(mean, epsilon)              # 必须在开方「之前」
开方：Sqrt(rms, mean)                    # FP32
归一化：Divs(z, y, rms)                  # ← 除法，不是乘 1/rms
缩放：Mul(z, z, gamma)                   # FP32
偏置：Add(out, z, bias)                  # FP32
出口：Cast(fp32 → fp16/bf16)             # 只 cast 一次；bf16 用 CAST_RINT
```

### 6.3 局限

- 全部为 CPU 参考。硬件 `Muls`/`Divs`/`Rsqrt` 的**单次舍入**、NPU `Cast` 的**真实舍入模式**（RNE/截断/Round-to-Odd）CPU 无法复现。
- 判题端真实数据分布未知（模板只给 `uniform(-2,2)`），无法确认是否出现 `|y|≥256`。
- 判题端是否含"失配比例 tol"未开放。

---

## 7. UB 分块与性能方案

完整推算见 `agents/agent07-perf-ub.md`。**所有数字标注为「官方转引 B」或「推算」，无真机实测。**

### 7.1 硬件参数（复核）

| 参数 | 值 | SoC | 等级 | 是否官方 |
| --- | --- | --- | --- | --- |
| UB 容量 | 192 KB（196608 B） | 910B1/910B2 (dav-c220) | B | 否（规格表转引） |
| AI Core / 向量核 | 24 AIC / **48 AIV** | 910B2 | B | 否 |
| 向量 repeat 宽度 | 256 B（fp16=128 元素、fp32=64 元素） | AIV | **A** | **是** |
| GM/HBM 带宽 | 1.6 TB/s（社区 1.2–2.0 浮动） | 910B | B/C | 否 |
| DataCopy 对齐粒度 | 32 B（建议对齐 512 B 达峰值） | MTE2/MTE3 | B | 否 |

> **未定案**：模板默认 `dav-2201` 与 910B（`dav-c220`）的映射关系**未经官方确认**。若判题机是 310B（UB=256KB），下列 D 上限会放宽。

### 7.2 带宽账

- **S1 两遍扫描**：`5·N·s`（Pass1 读 x+residual = 2N·s；Pass2 读 2N·s + 写 output N·s）
- **S2 单遍保存中间**：`3·N·s`
- **节省 = 2/5 = 40% GM 流量**
- 本算子算术强度约 0.8–1.3 FLOPs/Byte，远低于 roofline 拐点（~200）→ **典型 memory-bound**

### 7.3 S2 单遍可行的 D 上限（推算，UB=192KB 扣 16KB 开销）

| dtype | 严格模型（y+output+gamma+bias 四份同留 UB） | 必须回退 S1 |
| --- | --- | --- |
| fp16 / bf16 | **D ≤ 22528** | D ≥ 24576（含 32768） |
| fp32 | **D ≤ 11264** | D ≥ 12288（含 32768） |

### 7.4 四种形态的 tiling 与分核建议

| 形态 | 建议 |
| --- | --- |
| **(a) 小 outer、大 D**（outer=1~8，D=32768） | 单 tile 装不下整行 → **必须 intra-row 切 tile**；outer<核数时结构性欠利用（只用上 8/48 核），优先保证正确性走 S1 |
| **(b) 大 outer、小 D**（outer=8192，D=64~576） | **纯行并行**：8192/48 ≈ 171/170 行每核，**K 行合并 DMA** 达 512B（如 D=64 fp16 取 K=4）；双缓冲约 1.6× |
| **(c) outer ≈ 核数**（outer=48，D=1024） | 理想形态：1 行/核；**S2 单遍天然可行** |
| **(d) D 超单 tile**（D=32768，fp32 一行 128KB） | 行内切 tile + 子 tile 复用可达成 S2；**一旦一行被切给多核，跨核归约退化 → 走 S1** |

### 7.5 两个必须处理的性能/正确性细节

1. **32B cache-line 踩踏**：`D×sizeof(T)` 非 32 字节整数倍时，行与行在 GM 连续排布，前一行尾块与后一行头块落在同一 32B 物理缓存行；多核并发 DMA 写回会静默污染邻居。
   - 对策 A：按 `k·D·sizeof(T) ≡ 0 (mod 32)` 的最小 k 行作为分配粒度。
   - 对策 B（更简单）：**output 每行 stride 补齐到 32B 整数倍**（需判题端允许额外 GM，需注意 output 是平台分配的，不能改 stride → 实际只能用对策 A + 保证搬出只写有效字节）。
2. **ReduceSum 的标量同步**：内部含 scalar wait，会阻塞流水；`GetValue` 读回需 `V_S` 同步。官方最佳实践 `BlockReduceSum + WholeReduceSum` 比两次 `WholeReduceSum` 快约 40%（256 个 fp32：8.44 µs vs 13 µs）——**A2 9.0.0 是否支持该 API 未定案**。

### 7.6 性能测量方法（真机）

- `msprof op ./binary` → `OpBasicInfo.csv` 取 `Task Duration(us)`；`PipeUtilization.csv` 看 `aiv_time / mte2 / mte3` 占比。
- `iterations=5` 意味着真机要做 warmup 规避冷启动被计入。
- 本地 benchmark 可直接复用 `kernel.asc` 的直调形态自建多 shape 用例。

---

## 8. 编译与真机环境方案

完整清单与 14 行错误处置表见 `agents/agent08-env-linux.md`。

### 8.1 环境准备（执行顺序）

1. **步骤 0 硬件自检**：`lspci | grep -i d801`（卡在位）、`uname -m`、`npu-smi info`。
2. **步骤 1 驱动与固件**：**必须用官方 run 包**（`--check` 校验 → `--full --install-for-all` → reboot），**禁止 apt 安装驱动**。自检设备节点 `/dev/davinci*`、`/dev/davinci_manager`、`/dev/devmm_svm`、`/dev/hisi_hdc`。
3. **步骤 2 CANN 9.0.0 toolkit**：run 包安装 → `source ${ASCEND_HOME_PATH}/set_env.sh`（`run.sh` 第一行就是它）。
4. **步骤 3 工具链确认**：`which bisheng ccec msopgen msopst cmake`（which 不到多半是环境变量没生效）。
5. **步骤 4 多版本切换**：用 `ASCEND_HOME_PATH` 显式覆盖。
6. **步骤 5 跑通模板自带 `run.sh`**（最小前置）。

### 8.2 错误处置表（节选，完整 14 行见 Agent 8 §3）

| 症状 | 原因 | 处置 |
| --- | --- | --- |
| `find_package(ASC)` 失败 | `set_env.sh` 未 source / toolkit 不全 | 重新 source，确认 `ASCEND_HOME_PATH` |
| `kernel_operator.h` 找不到 | include 路径或 toolkit 缺失 | 检查 `target_include_directories` 与 toolkit 完整性 |
| `--npu-arch` 不匹配 | `SOC_ARCH` 与实际芯片不符 | 用 `npu-smi info` 确认芯片后设 `NPU_ARCH` |
| `unknown type name 'pipe_'`（**V001 已发生**） | 标识符与内建/宏冲突 | 重命名成员（V002 已改 `tpipe`）；**建议避免把 `TPipe` 作为 kernel 类成员** |
| `cannot use dot operator on a type`（**V001 已发生**） | 同上连锁 | 同上 |
| 平台收到文件首行 `return false;`（**V002 已发生**） | **上传内容截断/异常** | 提交前核对首行/末行/行数/md5；编辑器全选替换 |
| `aclInit`/`aclrtSetDevice` 返回非 0 | 驱动/设备/权限 | `npu-smi info` + 用户组 `HwHiAiUser` |
| `ACL_DEV_ATTR_VECTOR_CORE_NUM` 取不到 | API/版本不匹配 | 改用 `availableCoreNum` 兜底值 |
| kernel 超时（aicore timeout） | 死循环/越界/超大 shape | 缩小 shape 二分定位；`timeout 120` 是平台硬约束 |
| 链接期找不到 `tiling_api`/`register` | `target_link_libraries` 缺失 | 按模板 CMakeLists 对齐 |

### 8.3 Linux DO / V2EX 命中情况（**如实记录，未编造**）

- **Linux DO：未命中 / 检索受限**。站内搜索接口 `fetch failed`，搜索引擎未索引到昇腾相关帖；linux.do 实为 2024 年上线的以 AI 大模型讨论为主的 Discourse 社区。
- **V2EX：仅 1 条真实命中**（`t/1207741`，关于华为昇腾出货量/生态），**未达预期各 3 条**。其余命中均为 CSDN、hiascend 官方论坛、百度百科等，不计入。

### 8.4 关键判断

**两次平台 CE 都不是算法问题**：
- V001 = 标识符/符号冲突类编译错误；
- V002 = 上传内容截断（平台收到的首行与本地快照完全不同）。
- **根因是「我们从未在本地真正编译过一次」**。拿到真机后的**第一动作**是：用**未修改的模板原样编译并运行一次**，以此区分"平台/模板/打包问题"与"我们手写 kernel 的问题"。

---

## 9. 竞赛失败案例

18 个真实案例见 `agents/agent09-failure-cases.md`。本节只列对本题最有约束力的部分。

| 编号 | 类型 | 根因 | 对本题的启示 |
| --- | --- | --- | --- |
| C1 | Compile Error | `block_idx` 等保留字/内建变量冲突 | **V001 的 `pipe_` 同款**；命名要避开内建/宏 |
| C2 | Compile Error | 算子挑战赛官方 FAQ 五类编译错误 | 按官方分类逐项排查 |
| C3 | Compile Error | **kernel 过大触发 `out of jump/jumpc imm range`**（官方指南，A 级） | **直接命中我们 2955 行 / ~20 个 Process 变体的 V002** |
| C4 | Compile Error | kernel 内误用 C++ 标准库 | 只用 Ascend C 允许的 API |
| C5 | Compile Error（本队） | V001 `pipe_` 未定义 | 已归入通道问题 |
| C6/C7 | 上传包错误 | 首行 `return false;` / 分片上传损坏 | **V002 同款**；改为提交前校验 md5+行数+首末行 |
| C8/C12 | Wrong Answer | `DataCopyPad` 非对齐写出覆盖相邻 / 尾块写 -1 | 用 DataCopyPad + 真实有效 blockLen，**禁止裸 DataCopy 写非对齐尾块** |
| C9 | Wrong Answer | float16 舍入静默错误 | FP32 中间计算 |
| C10 | Wrong Answer | 非 32B 对齐导致 UB 精度异常 | 对齐/尾块处理 |
| C11 | Wrong Answer | `ReduceSum` 溢出与对齐约束（官方 API 文档） | 平方和必须在 FP32；`srcInnerPad=true` |
| C14 | Wrong Answer | **BF16 在 A2 上 `Add`/`Mul` 不支持**（官方） | bf16 必须 Cast→fp32→Cast |
| C16/C17 | 隐藏测试点 | 边界与数值稳定性是失分主因 | 离线自造边界用例（无 NPU 也能验证逻辑等价） |
| C18 | Runtime | `GetValue` 多核 DataCache 一致性（官方 9.0.0） | 标量读回处必须有正确的 `V_S` 同步 |

### 9.1 复杂度风险评估（论证 + C3 证据）

当前 `提交/V002/kernel.asc` 有 **2955 行**、约 20 个 `Process*` 变体分支。风险：
1. **官方明确记载的 `out of jump/jumpc imm range` 编译风险**（C3，A 级）；
2. 分支越多，静态审阅覆盖不到的路径越多，走错分支的概率上升；
3. 两次 CE 都是在**零本地编译证据**下发生的 → 复杂度与"无法本地验证"叠加，风险被放大。

**建议**：首版收敛为少量参数化通用路径（按 dtype × 是否对齐 × 是否多核参数化），而不是按 shape 档位写专用分支。若体量仍大，编译选项可加 `-mllvm -cce-aicore-jump-expand=true` 兜底。

### 9.2 提交策略（每天 50 次、取最后一次）

1. **先用一个最小可编译版本（仅 FP16 单路径、无分支）验证通道**，确认上传链路与编译环境正常；
2. 再逐步加 dtype、加尾块、加多核；
3. 每次提交前做：行数/md5/首行/末行/`run_kernel` 存在性/括号平衡检查；
4. **不要**把未验证的大改直接提交（因为取最后一次成绩）。

---

## 10. 方案矩阵

完整矩阵（S1–S14，含推荐级别与证据来源）见 `方案矩阵.md`。此处给出裁决摘要：

| 推荐级别 | 方案 |
| --- | --- |
| **首选** | S1 两遍扫描、S3 FP32 全中间计算、S7 以行分配 AI Core、S9 DataCopyPad 尾块、S11 ReduceSum 归约、S13 纯 Ascend C 实现 |
| **可作为第二路线** | S2 单遍保存中间（带 D 上限护栏）、S5 大 tile、S8 以 tile 分配 AI Core（outer<核数时兜底）、S10 手工尾块（S9 备选） |
| **仅作研究参考** | S6 小 tile、S12 手工向量归约、S14 从 CUDA/Triton/PyTorch 迁移 |
| **不建议** | S4 低精度中间计算（已被 CPU 实证证否） |

### 10.1 明确列入黑名单的做法

1. 用 `Muls(y, 1/rms)` 或 `Rsqrt` 替代 `Divs` + `Sqrt`（bf16 精度风险）。
2. bf16 路径直接进入 `Add`/`Mul`/`Muls`/`Div`/`ReduceSum`（A2 不支持）。
3. 用裸 `DataCopy` 写非 32B 对齐尾块（会写 -1 污染邻居）。
4. 巨型多分支 kernel（触发 `out of jump/jumpc imm range`）。
5. 低精度（fp16/bf16）中间累加（已证否）。
6. 按旧字段序 `{isPad, paddingValue, leftPadding, rightPadding}` 初始化 `DataCopyPadExtParams`（应**按成员名赋值**）。

---

## 11. 推荐实现路线（首版 · 正确性绝对优先）

**组合**：S7（行级并行）+ S1（两遍扫描）+ S3（FP32 全中间）+ S9（DataCopyPad 尾块）+ S11（ReduceSum 归约）+ S13（纯 Ascend C 直调单文件）

### 11.1 刚性约束（逐条可执行）

1. **入口与形态**
   - 与模板逐字对齐：`extern "C" void run_kernel(...)` + `add_rms_norm_bias_custom<<<blockNum, nullptr, stream>>>(...)`，核函数用 `__global__ __vector__`。
   - **不得有 `main()`、`#pragma once`**；文件被 `#include` 进 `main.asc`。
   - `TPipe` **不作为 kernel 类成员**（规避 V001 同款符号问题）。

2. **分核（S7）**
   - `blockCount = min(availableCoreNum, rowCount)`；`rowsPerCore = ceil(rowCount / blockCount)`；尾核处理余数行。
   - **行基址与偏移一律 `uint64_t`**（`row * D` 在大 batch/3D/4D 下会超 2^32）。

3. **算法骨架（S1 + S3）**
   - Pass 1：逐 tile 读入 x/residual → `Cast` 到 FP32 → `Add` 得 y → `Mul` 得 y² → `ReduceSum` 累加平方和。
   - 行末：`Muls(1/D)` → `Adds(epsilon)` → `Sqrt` → `Divs(y, rms)` → `Mul(gamma)` → `Add(bias)` → 输出前 `Cast` 回原 dtype。
   - Pass 2：重读 x/residual 重算 y 并完成归一化与输出。
   - **分块 ≤ 4096**（保守值，规避 UB 与归约路径的不确定性）。

4. **数值（CPU 实证支撑）**
   - **强制 `Divs(y, rms)`**，禁止 `Muls(1/rms)`。
   - **强制 `Sqrt`**，禁止 `Rsqrt` 作为首版路径。
   - **epsilon 必须在 `mean(y²)` 之后、开方之前**。
   - **bf16 全程 Cast→fp32→Cast**，出口 `CAST_RINT`。
   - 输入侧 `Cast` 到 FP32 后再做平方与归约（防 `|y|≥256` 溢出）。

5. **尾块（S9）**
   - GM→UB：非对齐用 `DataCopyPad` + `DataCopyPadExtParams` **逐字段赋值**（`isPad=true, leftPadding=0, rightPadding=补齐元素数, paddingValue=0`）。
   - UB→GM：用 `DataCopyPad` 的 Local→Global 重载，**传真实有效字节数**（可非对齐），**不传 padParams**。
   - **禁止裸 `DataCopy` 写非对齐尾块**。

6. **同步**
   - `ReduceSum` 后读 `GetValue(0)` 前必须有 `PipeBarrier<PIPE_V>` + `V_S` 事件同步（或按官方示例用 `SetFlag/WaitFlag`）。
   - 避免不必要的 `PipeBarrier<PIPE_ALL>`。

7. **体量控制**
   - 收敛为参数化通用路径，**不按 shape 档位写专用分支**。
   - 目标：把 2955 行降到数百行量级，降低编译失败与分支走错风险。

### 11.2 首版验收（真机）

模板样例（FP16 `[1,64]`，eps=1e-5）必须跑通；随后按 §15 的实验矩阵扩展。

---

## 12. 第二候选路线

**组合**：S7 + **S2（单遍保存中间结果）** + S3 + S9/S10 + S11，**带 D 上限护栏**。

- **启用条件**：首版 15/15 全过之后，为性能引入。
- **护栏**：fp16/bf16 仅当 `D ≤ 22528` 走 S2；fp32 仅当 `D ≤ 11264` 走 S2；一旦 `D ≥ 24576`（fp16/bf16）/ `D ≥ 12288`（fp32）**立即回退 S1**。
- **不因性能放松**任何正确性约束（仍走 `Divs` + `Sqrt` + DataCopyPad 尾块 + bf16 Cast 中转）。
- **风险**：S2 在 D 极大时 UB 紧张，需 intra-row 切 tile。

**性能候选（第三，正确性闭环后才启用）**：S2 + S5 大 tile + S7/S8 混合分核 + 双缓冲（约 1.6×）+ K 行合并 DMA 达 512B + 评估 `BlockReduceSum + WholeReduceSum`（A2 可用性未定案）。若真机证明 `Rsqrt` 显著更快且精度仍过阈，可单独评估替换。

---

## 13. 风险清单

| 风险 | 级别 | 对策 | 证据 |
| --- | --- | --- | --- |
| **从未本地编译过**（系统性风险） | **致命** | 拿到真机后先用未修改模板原样编译一次 | 本队记录（A） |
| 巨型多分支 kernel 触发 `out of jump/jumpc imm range` | **高** | 首版收敛为参数化通用路径 | C3（A，官方） |
| 上传内容截断/异常（V002 已发生） | **高** | 提交前核对 md5/行数/首行/末行/`run_kernel` | C6/C7（A/B） |
| 多核非 32B 对齐 DMA 写回踩踏 | **高（静默损坏型）** | 按 `k·D·sizeof(T)≡0(mod 32)` 分组；只写有效字节 | Agent 7 §5.2（B/C）+ C8/C12 |
| `DataCopyPadExtParams` 字段序写错 | 高 | **始终按成员名赋值** | Agent 2 裁决（A） |
| bf16 走低精度算术（A2 不支持） | 高 | 强制 Cast→fp32→Cast | Agent 2（A）+ C14 |
| `Muls(1/rms)` 在 bf16 的舍入越界 | 中 | 改用 `Divs` | Agent 6 Q1（CPU 实证） |
| fp16 平方溢出（`|y|≥256`） | 中 | 平方与归约在 FP32 | Agent 6 Q4（CPU 实证） |
| `ReduceSum` 溢出/对齐（`srcInnerPad`） | 中 | FP32 归约 + `srcInnerPad=true` | C11（A，官方） |
| 大张量 `uint32_t` 偏移溢出 | 中 | 行基址/偏移强制 `uint64_t` | 项目既有约定 |
| `GetValue` 标量同步缺失 | 中 | 读回前 `V_S` 同步 | C18（A，官方 9.0.0） |
| 判题 SoC 与本地开发架构不一致 | 中 | 用 `npu-smi info` 确认芯片后设 `NPU_ARCH` | 未确认 |
| fp32 精度阈值 1e-4 比本地模板严 | 中 | 本地自测按 1e-4 | Agent 1（A） |
| 标量同步性能退化（大 outer 放大） | 低-中 | 批量化归约/双缓冲隐藏 | Agent 7（推算） |
| 提交取最后一次成绩，误提交未验证版本 | 中 | 先最小可编译版验证通道，再增量 | Agent 1（A） |

---

## 14. 未确认事项（必须真机或平台开放后才能定案）

1. **判题真实 SoC 型号**（`dav-2201` 是模板默认值，不是平台约束；题目 API 无 `soc_version`）→ 决定 UB/核数参数是否适用。
2. **15 个测试点的 shape/dtype/epsilon 配置**（testcase 详情端点 403）。
3. **判题端是否含"失配比例 tol"**（本地 `verify_result.py` 有 `tol=1e-3`，判题端未开放）。
4. **5 次 iterations 如何聚合成 `t`**（最好/平均/中位数）。
5. **评分公式中的 `T`（实时最优）的初始值**。
6. **CANN 9.0.0 正式版 `Mul`/`Div` 在 Atlas A2 上对 bf16 的支持表**（当前为 beta.2 明文 + 同族推断，B 级）。
7. **`BlockReduceSum`/`WholeReduceSum` 在 A2 9.0.0 的签名与可用性**。
8. **`DataCopyPadExtParams<T>` 头文件的真实结构体声明**（未读安装包，依文档表格推断 → 故强制按成员名赋值）。
9. **硬件 `Muls`/`Divs`/`Rsqrt` 的单次舍入误差**（只能 NPU 实测）。
10. **fp32→bf16 cast 在判题 SoC 的真实舍入模式**（RNE/截断/Round-to-Odd）。
11. **判题端真实数据分布**（是否出现 `|y|≥256` 触发 fp16 平方溢出）。
12. **`availableCoreNum` 的真实取值**。
13. **S2 单遍在判题 SoC 上的真实 UB 上限**（192KB 为规格表转引，非官方接口文档）。
14. **是否有 15 个之外的隐藏测试点**（API 暴露恰好 15 个，无法 100% 排除）。
15. **上传表单的字段与大小限制**（需登录提交页，本轮未登录）。

---

## 15. 下一阶段实验计划

### 15.1 真机第一验证批（按优先级）

| 优先级 | 验证项 | 输入/操作 | 预期/判据 |
| --- | --- | --- | --- |
| **P0** | 上传通道完整性 | 提交前核 md5、行数、首行 `#include`、末行闭合、UTF-8 无 BOM | 平台回显与本地一致 |
| **P0** | 模板原样编译 + 运行 | 未修改模板跑 `./run.sh` | **Compile Pass + PASSED**（排除平台/环境问题） |
| **P0** | 最小可编译版 | 仅 FP16 单路径、无分支的 tiny kernel | Compile Pass（验证 V001/V002 类通道问题已排除） |
| P1 | 三 dtype | fp16/bf16/fp32 各 1 例（如 `[64,1024]`） | 精度过阈（fp32 按 1e-4） |
| P1 | 尾块非对齐 | fp16 `[8,70]`、`[1,30000]` | DataCopyPad 尾块正确、无 -1 污染 |
| P1 | 32B 踩踏 | fp16 `[8,70]`，`blockNum=8`，核对邻行边界字节 | 无 cache-line 污染 |
| P1 | bf16 Cast 中转 | bf16 `[64,1024]` | 精度过阈（证明未走 bf16 算术） |
| P2 | 大 D 归约 | fp32 `[1,32768]`、fp16 `[1,32768]` | 无 inf，ReduceSum(fp32) 正常 |
| P2 | S2 D 上限 | fp32 `[128,32768]`、fp16 `[1,30000]` | 验证 UB 溢出 → 回退 S1 |
| P2 | uint64 偏移 | 大 outer + D=32768 | 无 32 位溢出 |
| P3 | 性能基线 | `msprof op` 测各形态 Task Duration | 取得 aiv/mte2/mte3 占比 |
| P3 | `Rsqrt` 替换评估 | 同输入比对 Sqrt+Divs vs Rsqrt+Mul | 精度仍过阈且更快才采用 |

### 15.2 本地可继续做的（无需 NPU）

1. 用 `数值参考脚本/agent06_precision_matrix.py` 扩展矩阵（补 outer=8192 大形态、3D/4D 全覆盖、NaN/Inf 用例）。
2. 把首版 kernel 的**算法结构**（不是 Ascend C 代码）在 CPU 上再做一次等价性仿真，覆盖全部 15 点可能的 shape 组合。
3. 静态检查脚本：行数、括号平衡、首末行、`run_kernel` 存在性、禁止 `main()`/禁用 API 名单。

### 15.3 明确不做的事

- **不执行 CANNJudge 上传**（未获用户明确确认）。
- **不创建第二套工程 / 备份工程 / 重复文档**。
- **不修改 `源码/` 与 `提交/`**（本轮调研只出文档）。
- **不把 GPU 实现标为 Ascend C 可用方案**。

---

## 16. 本轮结论摘要

- **完成了什么**：10 个平台/主题互斥的子代理完成了题面与接口、官方 API、官方开源实现、GPU 迁移、编译器与代码生成、数值精度、性能与 UB、真机环境、失败案例九大方向的调研，并由 Agent 10 独立审阅裁决，形成 S1–S14 方案矩阵与三条路线。来源 235 条，全部登记在 `sources.md`。
- **哪些方案有源码或官方 API 证据**：S1/S3/S7/S9/S11/S13 均有 A 级官方 API 文档或官方仓库源码支撑（ops-transformer / ops-nn 的 AddRmsNorm 源码已逐行读取）。
- **哪些方案仅为迁移参考**：S2/S5/S8/S10 的 tile 与上限参数为**推算**（UB 与带宽数字为 B/C 级转引）；S6/S12 仅研究参考；**S14（GPU 迁移）明确不建议作为提交方案**。
- **哪些结论仍需真实 CANN/NPU 验证**：判题 SoC、15 点配置、判题端判定容差、bf16 在 A2 正式版的支持表、`BlockReduceSum` 可用性、硬件指令舍入、S2 的真实 UB 上限、以及**全部性能数字**。
- **本轮没有执行 CANNJudge 上传或提交，未消耗任何提交次数。**
