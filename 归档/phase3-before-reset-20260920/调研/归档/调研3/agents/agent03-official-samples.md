# Agent 3 调研报告：Ascend 官方开源仓库中的 RMSNorm / AddRmsNorm / 融合实现

调研日期：2026-09-12  
调研范围：GitHub Ascend 组织、GitCode Ascend 主站、Gitee 历史入口  
工作方式：GitHub REST API 目录树 + raw.githubusercontent 源码直读；WebSearch 不可用，GitCode 部分页面 403/404

---

## 一、仓库命中表

| # | 仓库 / 路径 | URL | 是否含 residual+bias 融合 | 计算精度策略 | Tiling 方式 | 证据状态 |
|---|---|---|---|---|---|---|
| 1 | Ascend/samples `…/6_ascendc_custom_op/kernel_invocation/Add_tile/` | https://github.com/Ascend/samples/tree/master/cplusplus/level1_single_api/4_op_dev/6_ascendc_custom_op/kernel_invocation/Add_tile | 否（纯向量加） | FP16 直算 | 多核按元素均分 + 每核 tileNum×双缓冲 | verified（读了 kernel、CMakeLists、tiling 头） |
| 2 | Ascend/samples 同目录 `Add/`、`MatMul/`、`TopK/`、`kernel_template/` | https://github.com/Ascend/samples/tree/master/cplusplus/level1_single_api/4_op_dev/6_ascendc_custom_op/kernel_invocation | 否 | — | — | verified（目录列表） |
| 3 | Ascend/op-plugin `op_plugin/ops/aclops/AddRmsNormKernelNpu.cpp` | https://github.com/Ascend/op-plugin/blob/master/op_plugin/ops/aclops/AddRmsNormKernelNpu.cpp | residual 融合，**无 bias** | rstd=FP32，y/x 同输入 dtype | 无（ACL 封装，tiling 在闭源 CANN 内） | verified（读了全文） |
| 4 | Ascend/op-plugin `op_plugin/ops/opapi/AddRmsNormV2KernelNpuOpApi.cpp` | https://github.com/Ascend/op-plugin/blob/master/op_plugin/ops/opapi/AddRmsNormV2KernelNpuOpApi.cpp | residual 融合，**无 bias** | y 固定 FP32（inplace 变体） | 无 | verified（读了全文） |
| 5 | Ascend/op-plugin `docs/zh/custom_APIs/torch_npu/torch_npu-npu_add_rms_norm.md` | https://github.com/Ascend/op-plugin/blob/master/docs/zh/custom_APIs/torch_npu/torch_npu-npu_add_rms_norm.md | 同上；官方公式与产品支持表 | 明确 rstd=FP32 | — | verified（读了全文） |
| 6 | Ascend/mojo_opset `mojo_opset/backends/ttx/kernels/npu/fused_add_rms_norm.py` | https://github.com/Ascend/mojo_opset/blob/master/mojo_opset/backends/ttx/kernels/npu/fused_add_rms_norm.py | residual 融合，**无 bias**（可选 weight offset） | 方差/归约 FP32 | Triton：列 BLOCK_SIZE_N（>4096 取 2048）+ 行 BLOCK_SIZE_M autotune 1–16 | verified（读了全文） |
| 7 | Ascend/mojo_opset `mojo_opset/backends/ttx/kernels/npu/rmsnorm.py`、`layernorm.py` | https://github.com/Ascend/mojo_opset/tree/master/mojo_opset/backends/ttx/kernels/npu | 无 residual | FP32 归约 | 同上 Triton 风格 | verified（路径+大小） |
| 8 | Ascend/cann_op_contrib `community/ops/` | https://github.com/Ascend/cann_op_contrib/tree/init_framework/community/ops | 否；仅 add/sub/tanh | — | — | verified（目录树+README） |
| 9 | Ascend/canncamp `operator_camp/sample/` | https://github.com/Ascend/canncamp/tree/master/operator_camp/sample | 否；仅 RoiExtractor | — | — | verified（目录树） |
| 10 | Ascend/MindIE-Turbo | https://github.com/Ascend/MindIE-Turbo | 否；纯 Python vLLM 补丁器 | — | — | verified（完整树无 kernel 源码） |
| 11 | Ascend/samples `operator/op_implementation/` | https://github.com/Ascend/samples/tree/master/operator/op_implementation | 否；仅 `.keep` 占位 | — | — | verified（空目录） |

### 明确未找到 / 不存在

| 目标 | 结果 |
|---|---|
| github.com/Ascend/cann-samples | 404。README 中自称 cann-samples，实际仓库名为 `Ascend/samples` |
| github.com/Ascend/ops-transformer | 404 |
| github.com/Ascend/ops-nn | 组织仓库列表中无此名 |
| gitcode.com/Ascend/cann-samples | 404 |
| 官方开源的 **AddRmsNorm / RMSNorm 的 Ascend C kernel 源码** | **未找到**。op-plugin 只是 ACL/OpAPI 适配层；CANN 内置算子的 kernel 实现闭源 |
| 官方开源的 **AddRmsNormBias**（带 bias 加法） | **未找到**。op-plugin 文档与实现均无 bias 输入 |

---

## 二、最相关实现剖析

### 2.1 op-plugin `AddRmsNorm`（官方算子语义锚点）

`AddRmsNormKernelNpu.cpp` 全文核心：

```cpp
cmd.Name("AddRmsNorm")
    .Input(x1, "x1").Input(x2, "x2").Input(gamma, "gamma")
    .Output(y, "y").Output(rstd, "rstd").Output(x, "x")
    .Attr("epsilon", static_cast<float>(epsilon))
    .Run();
```

文档（`torch_npu-npu_add_rms_norm.md`）确认的语义：

- 公式：`x_i = x1_i + x2_i`；`RMSNorm(x) = x / RMS(x) * gamma`，`RMS = sqrt(mean(x^2) + eps)`
- **没有 bias**
- 返回三元组：`(y, rstd, x)`，其中 `x` 是 residual add 的结果，`rstd = 1/RMS` 且为 FP32
- 输入支持 1–8 维、fp32/fp16/bf16；`gamma` dtype 与 `x1` 一致；`gamma` shape 对应 `x1` 的后若干维
- `rstd` shape = `x1` 前若干维（归一化维置 1）
- 支持产品：Atlas A2/A3 训练推理、Ascend 950DT；Atlas 推理系列不支持 bf16

`AddRmsNormV2KernelNpuOpApi.cpp` 是 inplace 变体，调用 `aclnnInplaceAddRmsNorm`，返回的 `y` 固定 FP32，把 add 结果写回 `x1`。这是图内融合用的接口，不是独立 kernel。

**与本题关系**：官方算子只做到 residual add + RMSNorm + gamma，本题多一个 bias。bias 必须在自定义 kernel 内补上。

### 2.2 mojo_opset `fused_add_rms_norm`（算法结构最接近的开源实现）

这是 Ascend 官方 GitHub 组织下、面向 NPU 的 Triton（TTX 后端）实现，算法与本题高度同构。

两遍扫描结构（forward kernel）：

1. **Pass 1**：沿列分块循环  
   `S = X + R` → 写回 S → `var_acc += sum(S_f32 * S_f32)`  
   然后 `rstd = rsqrt(var/n_cols + eps)`，写出 RSTD（FP32）
2. **Pass 2**：再次沿列分块  
   读 S 与 weight W → `(S * rstd) * (W + offset)` → 写 Y

关键工程点：

- 列分块：`n_cols > 4096` 时 `BLOCK_SIZE_N = 2048`，否则按向量对齐粒度取
- 行分块：`BLOCK_SIZE_M ∈ {1,2,4,8,12,16}` autotune，多行进一个 program
- grid 大小 = NPU `num_vectorcore`（多核并行按行任务步进）
- 方差与 rstd 始终 FP32；LLAMA casting 模式下归一化后先转回原 dtype 再乘 weight
- 返回 `(Y, S)`（pre 模式）或 `(Y, Y)`（post 模式）
- 注释注明原始参考 LinkedIn Liger-Kernel，再做 NPU 适配

**与本题关系**：算法骨架（两遍、FP32 归约、列分块、多核行切分）可直接对照；但语言是 Triton 不是 Ascend C，且无 bias。README 的 Future Work 写明「Ascend NPU's official implementation using Ascend C language」尚未提供。

### 2.3 samples `Add_tile`（官方 Ascend C 工程模板）

唯一在官方仓库中读到的完整 Ascend C 自定义算子样例（向量加）。

目录：

```text
Add_tile/
├── CMakeLists.txt
├── add_custom.cpp        # kernel
├── add_custom_tiling.h   # tiling 结构体
├── add_custom.py         # 生成输入
├── data_utils.h
├── main.cpp              # host 调用
├── run.sh
└── input/
```

kernel 要点（旧式 tik2 API）：

- `TPipe` + `TQue<VECIN/VECOUT, BUFFER_NUM>` 双缓冲
- `DataCopy` 搬入 → `Add` 指令计算 → `DataCopy` 搬出
- 多核：`blockLength = totalLength / blockDim`，每核 `tileNum` 个 tile
- 仅 FP16；无归约、无 DataCopyPad、无 FP32 提升

CMakeLists 用 `ccec -x cce` 直接编 kernel，按 `product_type × core_type` 选 `--cce-aicore-arch`（dav-c100 / dav-m200 / dav-m200-vec），并链接 `tikicpulib` 做 CPU 模拟。

**与本题关系**：工程骨架（kernel + tiling + host + CMake + run.sh）可借鉴；计算内容过于简单，且 API 风格偏 CANN 5.x/6.x，与 CANN 9.0.0 的 Ascend C 接口（`kernel_operator.h` + AscendC 命名空间）有代差。

### 2.4 其余仓库结论

- **cann_op_contrib**：`community/ops/` 仅 add/sub/tanh；README 仍是 Gitee 模板默认文本，处于 `init_framework` 分支，无 RMSNorm
- **canncamp**：`operator_camp/sample/` 仅 RoiExtractor，与归一化无关
- **MindIE-Turbo**：目录只有 `mindie_turbo/adaptor`、`utils`、`tests`，是 vLLM 的 Python 补丁层，无 kernel
- **samples/operator/op_implementation**：空目录（仅 `.keep`），说明官方把算子实现样例放在了 `6_ascendc_custom_op` 而非此处
- **samples 全树 grep**：除 `batchnorm` 验证样例和 `fused_layer_norm.py`（SentimentAnalysis 贡献样例，PyTorch 侧）外，无 RMSNorm / AddRmsNorm

---

## 三、与本题差异

| 维度 | 官方 AddRmsNorm（op-plugin） | mojo_opset Triton | samples Add_tile | 本题 AddRmsNormBias |
|---|---|---|---|---|
| residual add | 有 | 有 | 无 | 有 |
| RMSNorm | 有 | 有 | 无 | 有 |
| gamma 缩放 | 有 | 有 | 无 | 有 |
| bias 加法 | **无** | **无** | 无 | **有** |
| 输出 | y, rstd, x | Y, S | z | 仅 output |
| 归约精度 | 闭源（rstd 为 FP32） | FP32 | 无归约 | FP32（本地实现） |
| 实现语言 | 闭源 CANN kernel | Triton | Ascend C（tik2） | Ascend C（CANN 9.0） |
| 尾块处理 | 闭源 | Triton mask | 无特殊处理 | DataCopyPad（本地实现） |
| 多核切分 | 闭源 | 按行任务 grid=vectorcore | 按元素均分 | 按行均分（本地实现） |

三点结构性差异：

1. **bias 是本题独有步骤**。官方 AddRmsNorm 与 mojo_opset 都止于 `* gamma`，必须在 kernel 内追加 `+ bias`，没有现成融合算子可抄。
2. **官方开源无 Ascend C RMSNorm 源码**。CANN 包内的 AddRmsNorm kernel 闭源；samples 只有 Add/MatMul/TopK 教学样例。
3. **本地 `源码/op_kernel/add_rms_norm_bias.cpp` 的两遍扫描 + FP32 策略与 mojo_opset Triton 算法同构**，可把 Triton 版当作算法正确性对照，而不是代码迁移源。

---

## 四、迁移建议

1. **语义对齐以 op-plugin 文档为准**  
   epsilon 加在方差内部再 rsqrt；`rstd` 为 FP32；输入可 1–8 维展平为 outer 行。本题在此之上多一步 bias。

2. **算法骨架对照 mojo_opset，而不是 samples**  
   两遍扫描、FP32 方差累加、列分块、多核按行步进——本地实现已采用同一结构。Triton 的 `BLOCK_SIZE_N=2048`（大 D）与行分块 autotune 可作为 UB 分块上限的参考量级，但 Ascend C 需按 UB 容量自行计算。

3. **工程模板参考 samples Add_tile，但必须升级 API**  
   CMake + tiling 结构体 + host main + run.sh 的组织方式可借鉴；`TPipe/TQue/DataCopy` 要换成 CANN 9.0 的 `AscendC::` 接口（本地实现已用 `kernel_operator.h` + `AscendC` 命名空间）。`ccec` 编译参数与 `tikicpulib` CPU 模拟链路对真机调试有参考价值。

4. **DataCopyPad 尾块、多核行切分在官方开源样本中无直接对应**  
   samples 的 Add_tile 假设 `totalLength` 被 blockDim 整除且用对齐 `DataCopy`。本题 D 非 32 倍数时的尾块策略只能依据手册与本地实现，无法从官方开源代码验证。

5. **不要把 op-plugin 的 ACL 封装当成 kernel 实现**  
   `AddRmsNormKernelNpu.cpp` 只是把算子名和属性递给 CANN，真实 tiling 与 UB 分配在闭源 so 里。用它做语义对照可以，做实现参考不行。

6. **cann_op_contrib / canncamp / MindIE-Turbo 对本题无直接可用源码**，后续不必再花时间。

---

## 五、已确认 / 未找到 / 无法确认

### 已确认（有源码或文档证据）

- `Ascend/samples` 即 cann-samples；含 Ascend C 样例 Add / Add_tile / MatMul / TopK / kernel_template，不含任何 RMSNorm
- `Ascend/op-plugin` 的 AddRmsNorm：residual 融合、无 bias、返回 (y, rstd, x)、rstd 为 FP32、支持 1–8 维 fp32/fp16/bf16
- `Ascend/mojo_opset` 的 fused_add_rms_norm：Triton 实现，两遍扫描 + FP32 归约 + 列/行分块，无 bias；Ascend C 后端列为 Future Work
- `Ascend/cann_op_contrib/community/ops` 仅 add/sub/tanh
- `Ascend/canncamp/operator_camp/sample` 仅 RoiExtractor
- `Ascend/MindIE-Turbo` 无 kernel 源码
- GitHub 组织列表中无 ops-transformer / ops-nn / cann-samples 独立仓库

### 未找到

- 官方开源的 AddRmsNorm / RMSNorm **Ascend C kernel 源码**
- 任何官方实现中的 **bias 融合**（AddRmsNormBias）
- samples 中的 DataCopyPad 尾块归约样例
- gitcode 上名为 cann-samples 的仓库

### 无法确认

- GitCode 主站部分页面返回 403，未能交叉核对 GitCode 是否有 GitHub 未镜像的额外算子样例
- CANN 9.0.0 安装包内是否附带 AddRmsNorm 的 Ascend C 参考实现（本机无 CANN 环境）
- `Ascend/triton-ascend-ops`、`Ascend/MindIE-LLM` 是否在深层目录藏有融合 RMSNorm 的 Triton/图定义（本轮未展开其完整树；MindIE-Turbo 已排除）

---

## 六、来源

| # | 来源 | 类型 | 访问日期 | 证据状态 | 用途 |
|---|---|---|---|---|---|
| 1 | https://github.com/Ascend/samples （README_CN.md + 目录树 + Add_tile 源码/CMake） | B 官方仓库 | 2026-09-12 | verified | 确认 cann-samples 身份；Ascend C 样例范围；tiling/CMake 模板 |
| 2 | https://github.com/Ascend/op-plugin （AddRmsNormKernelNpu.cpp、AddRmsNormV2…OpApi.cpp、docs/zh/…npu_add_rms_norm.md） | B 官方仓库 | 2026-09-12 | verified | 官方 AddRmsNorm 语义、精度、维度约束 |
| 3 | https://github.com/Ascend/mojo_opset （fused_add_rms_norm.py 全文 + README + 目录树） | B 官方仓库 | 2026-09-12 | verified | 融合 RMSNorm 算法结构对照 |
| 4 | https://github.com/Ascend/cann_op_contrib （init_framework 分支目录树 + README） | B 官方仓库 | 2026-09-12 | verified | 确认无 RMSNorm 贡献算子 |
| 5 | https://github.com/Ascend/canncamp （operator_camp 目录树） | B 官方仓库 | 2026-09-12 | verified | 确认训练营样例无 RMSNorm |
| 6 | https://github.com/Ascend/MindIE-Turbo （完整树） | B 官方仓库 | 2026-09-12 | verified | 确认无 kernel 源码 |
| 7 | https://api.github.com/orgs/Ascend/repos （组织仓库全列表） | B 官方 API | 2026-09-12 | verified | 排除 ops-transformer / ops-nn / cann-samples 独立仓库 |
| 8 | https://gitcode.com/Ascend （组织主页） | B 官方镜像 | 2026-09-12 | partial | 确认社区主站与仓库清单；部分子页 403 |
| 9 | https://github.com/Ascend/cann-samples、https://gitcode.com/Ascend/cann-samples | — | 2026-09-12 | unavailable | 均 404，记录为不存在 |

---

## 七、对本地实现的对照结论（非修改建议，仅事实）

本地 `源码/op_kernel/add_rms_norm_bias.cpp` 采用两遍扫描 + FP32 归约 + DataCopyPad 尾块 + 多核按行切分。该结构与 mojo_opset Triton 版同构，与 op-plugin 文档语义在「residual add + RMSNorm + gamma」部分一致，且额外实现了 bias。官方开源仓库中没有可直接替换的 Ascend C 实现，也没有需要对齐的官方 bias 融合写法。
