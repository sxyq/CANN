# Agent03 调研报告：官方 Ascend 开源仓库中的 RMSNorm / AddRmsNorm 实现

> 调研主题：官方仓库里与 `RMSNorm` / `AddRmsNorm` / `AddRmsNormQuant` 真实源码相关的算法、分块、归约、尾块、多核、构建与测试做法。
> 目标 CANN 版本：9.0.0；Kernel 类型：vector；竞赛算子：`AddRmsNormBias`（`y = x + residual`；`rms = sqrt(mean(y^2)+eps)`；`out = y/rms*gamma + bias`）。
> 工作机：macOS，无 CANN 工具链、无 NPU，仅本地读源码（5 个仓库已浅克隆到 `/tmp/`）。
> 证据等级：A=直接读官方源码并定位行号；B=README/接口文档级；C=个人 fork（本报告无 C 级来源，均为官方仓库）。

---

## 1. 结论摘要（≤10 条）

1. **官方 AddRmsNorm（非量化版）全部不含 bias 项**。ops-transformer 的 `matmul_all_reduce_add_rms_norm`、ops-nn 的 `fused_add_rms_norm`、`inplace_add_rms_norm` 的语义均为 `output = y / rms * gamma`（或 `x1*scale + x2` 后再 `* gamma`），**没有 `+ bias`**。（证据 A）
2. **唯一含 `+ beta`（bias）项的官方实现是量化版 `AddRmsNormQuant` / `AddRmsNormQuantV2`**，但其输出是 int8（量化），不是 fp16/bf16/fp32。语义 `y_i = x_i/Rms(x)*g_i + beta` 与竞赛完全一致，只是多了量化支路。（证据 A，ops-nn/norm/add_rms_norm_quant/README.md:18-28）
3. **竞赛候选实现 V002 已正确实现 bias**：`ApplyAffine` 中 `Mul(value,gamma)` 后 `Add(value,bias)`（kernel.asc:117-146）。它比所有非量化官方 AddRmsNorm 更接近竞赛语义；与官方 `AddRmsNormQuant` 的归一化+gamma+bias 路径等价，只是不带量化。（证据 A）
4. **归约方式分三类**：① 基础 API `ReduceSum(dst,src,work,count)`（候选 V002 与 cann-samples story 用此，asc-devkit `kernel_operator_vec_reduce_intf.h` 定义）；② 官方 AddRmsNorm 内核自写 `ReduceSumCustom` → 底层 `WholeReduceSum` / `BlockReduceSum`（跨 block 归约）；③ 高阶 `Pattern::Reduce`（asc-devkit `adv_api/reduce/reduce.h`，本报告未在 RMSNorm 源码中看到实际使用）。（证据 A）
5. **D 维度（列）分块是官方标配**：当 `numCol > ubFactor` 时切 D（`SPLIT_D`），采用「先按列块累加 partial sum → 再归约为每行 rstd → 再按列块做归一化」的两遍策略；尾块 `colTail = numCol - (jMax-1)*ubFactor` 自然处理 D 非整尾块。（证据 A，add_rms_norm_quant_split_d.h:110-138）
6. **多核按行切分**：`blockFactor = ceil(numRow / numCore)`，每核处理 `blockFactor` 行，最后一块 `rowWork = numRow - (numBlocks-1)*blockFactor` 吃掉余数；部分实现还用 `rowFactor` 在核内把多行批处理进一次 UB 循环（MERGE_N / MULTI_N / UB 多行）。（证据 A）
7. **fp32 输入支持不一致**：ops-nn 的 `fused_add_rms_norm_single_n.h` 明确有 fp16/**fp32**/bf16 三分支；但 ops-transformer 的 mc2 各 kernel（single_n/multi_n/split_d/normal）及 SPLIT_D 仅 fp16/bf16 重载，**fp32 在 SPLIT_D 路径下无对应实现**。（证据 A）
8. **cann-samples 的 `rms_norm_quant_story` 是最贴近「判题直调单文件」形态的高价值参考**：7 个递进 `.asc` 文件（0_naive→6_binary_sum），用基础 `ReduceSum`、讲述多核/寄存器数据流(VF RegAPI)/Double Buffer/UB 多行/二分累加优化，但其数学是 `RmsNorm(x)*gamma` 再量化，**无 residual add、无 bias**。（证据 A/B）
9. **官方仓库形态与判题直调单文件形态差异巨大**：官方是 msopgen/Aclnn 完整算子工程（op_host 算 tiling/infershape、op_kernel 算内核、op_api 封装 aclnn、op_graph 构图、examples/tests 各一套、每个算子一个 CMakeLists）；判题只吃一个 `kernel.asc` + `run_kernel` 入口，tiling 由 CANNJudge 隐式传入（outer/dim/epsilon）。**不能直接照抄编译**。（证据 A，见第 6 节）
10. **epsilon 为属性而非 tensor**：所有官方实现都把 epsilon 当 tiling/属性常量（默认 1e-6，部分校验 `(0,1)`）。竞赛 V002 也以 `float epsilon` 参数传入，一致。（证据 A）

---

## 2. 仓库清单

| 仓库 | 地址 | 分支 / commit | 是否官方 | 克隆 | 目录结构要点（与 RMSNorm 相关） |
|---|---|---|---|---|---|
| ops-transformer | gitcode.com/cann/ops-transformer | master @ d967eeb08ceef7c394d0d5e27767e5b08b77aa58 | 官方 | 成功 | `mc2/matmul_all_reduce_add_rms_norm/`（**融合 MatMul+AllReduce+AddRmsNorm**）；`experimental/attention/.../rms_norm.h`（注意力内 RMSNorm）；含 op_host/op_kernel/op_api/op_graph/examples/tests |
| ops-nn | gitcode.com/cann/ops-nn | master @ 1c891ca0bbc8852a3ef80fbf7d067c8ce034e592 | 官方 | 成功 | `norm/rms_norm/`（独立 RMSNorm 基础）、`norm/fused_add_rms_norm/`（**AddRmsNorm 融合版**）、`norm/inplace_add_rms_norm/`、`norm/add_rms_norm_quant/`（**量化版，含 beta**）、`norm/add_rms_norm_dynamic_quant/`；`torch_extension/.../add_rms_norm_dynamic_quant` |
| cann-samples | gitcode.com/cann/cann-samples | master @ 23c981c0918e3183958e94e58ef6989d44983230 | 官方 | 成功 | `Samples/2_Performance/rms_norm_quant_story/`（**7 步优化单文件 .asc** + Story.md）、`kv_rms_norm_rope_cache_story/` |
| cann-learning-hub | gitcode.com/cann/cann-learning-hub | master @ 9f8e8e2b9b2ffc79846a2cdf29b4261e296cadb2 | 官方 | 成功 | `contrib/tutorials/qwen_ops/01_rmsnorm_baseline/`（RMSNorm 教学 baseline 单算子工程）、`blogs/operator/ascend950_rmsnormquant_optimization/`、`tutorials/.../04_fused_operators/` |
| asc-devkit | gitcode.com/cann/asc-devkit | master @ d6ea6db110f5924a49cbbb941abef44722e3b72c | 官方 | 成功 | `include/basic_api/kernel_operator_vec_reduce_intf.h`（基础 `ReduceSum` 签名）、`include/adv_api/reduce/reduce.h`（高阶 `Pattern::Reduce`）、`kernel_tiling.h` 等框架头 |

> 访问说明：5 个仓库均浅克隆成功（无官方仓库访问失败）。未使用任何个人 fork，故无 C 级来源。本机无法编译，所有结论均来自源码/README 静态阅读。

---

## 3. 官方实现对照表

| 仓库 / 文件（行号） | 算法 | 分块 | 归约方式 | 尾块处理 | 多核策略 | 可迁移点 | 差异点（vs 竞赛 AddRmsNormBias） | 证据 |
|---|---|---|---|---|---|---|---|---|
| ops-transformer `mc2/.../op_kernel/add_rms_norm_single_n.h:68-139` | `y=x+residual; out=y/rms*gamma`（**无 bias**） | 整行进 UB（numCol 必须 ≤ ubFactor） | `ReduceSumCustom`→`WholeReduceSum`（mararn_rms_norm_base.h:40-74） | 整行一次，无列尾块 | `blockFactor=1` 时 SINGLE_N，每核 1 行 | 整行单次归约、rstd 取标量、事件同步 V_S/S_V 模板 | 无 bias；仅 fp16/bf16（ProcessFp16/ProcessBf16，无 fp32）；融合在 MatMul+AllReduce 大算子内，依赖 HCCl | A |
| ops-transformer `mc2/.../op_kernel/add_rms_norm.h:90-266`（NORMAL） | 同上，无 bias | 整行 | `ReduceSumCustom` | 整行 | `blockFactor`×`rowFactor` 行批 | 与 single_n 同；多行批 `rowFactor` | 无 bias；仅 fp16/bf16；gamma 与 y 均为 `(D,)` | A |
| ops-transformer `mc2/.../op_kernel/add_rms_norm_split_d.h:88-117` | 同上，无 bias | **切 D**：`jMax=ceil(numCol/ubFactor)`，两遍（former 累加 / latter 归一） | `ReduceSumFP32ToBlock`+`BlockReduceSumFP32`（跨 block 累加） | `colTail=numCol-(jMax-1)*ubFactor`（add_rms_norm_split_d.h:93） | 行切 + 行内 `rowFactor` 批 | **D 大时分块+尾块**，两遍归约，天然支持 D 非 32 倍数 | 无 bias；仅 fp16/bf16（`ComputeY` 仅 half/bf16 重载）；`rstdLocal.GetValue` 取标量 | A |
| ops-transformer `mc2/.../op_kernel/add_rms_norm_multi_n.h:91-204` | 同上，无 bias | 行内多行批（`rowFactor`），整行 D | `ReduceSumCustom` 每行 + `Gather` 展开 rstd | 整行 | `blockFactor`×`rowFactor` | 多行批处理、rstd 用 `NUM_PER_BLK_FP32` 排布 | 无 bias；仅 fp16（模板半实例化）；输出需 `yGm` | A |
| ops-transformer `mc2/.../op_kernel/add_rms_norm_merge_n.h:80-258` | 同上，无 bias | 小 D（≤2000）多行合并进 UB，`BroadCast` gamma/rstd | `ReduceSumMultiN` | `isNumColAlign_` 分支 + `DataCopyCustom(...,calc_row_num,numCol_)` 非对齐块拷贝 | 行切 + `rowFactor` 批 | **非 32 对齐 D 的块拷贝处理**（`isNumColAlign_` 路径） | 无 bias；依赖 `adv_api` 的 `BroadCast`/`ascend_dequant`；仅 fp16/bf16 | A |
| ops-transformer `mc2/.../op_host/op_tiling/arch22/common_add_rms_norm_tiling.cpp:198-301` | tiling 选择 | 模式：NORMAL/SPLIT_D/MERGE_N/SINGLE_N/MULTI_N（`numCol>ubFactor`→SPLIT_D；`numColAlign≤2000`→MERGE_N） | — | ubFactor 对齐到 16 倍数 | `numCore=GetCoreNumAiv()`，`blockFactor=ceil(numRow/numCore)` | tiling 决策树、UB 因子常量（fp16=12288 / fp32=10240） | epsilon 校验 `(0,1)`；依赖 `PlatformAscendC` 取核数/UB 大小 | A |
| ops-nn `experimental/norm/fused_add_rms_norm/op_kernel/fused_add_rms_norm_single_n.h:57-320` | `x=x1*scale+x2; out=x/rms*gamma`（**无 bias**，`scale` 默认 1.0） | 整行 | `BuildRstd`→`ReduceSumCustom`→底层 `WholeReduceSum` | 整行 | SINGLE_N：`blockFactor=1`，每核 1 行 | **含 fp32 分支**（ProcessFp32:151-210）；残差结果 `x` 落 GM（=竞赛的 y 输出） | 无 bias；多输出 `rstd`/`x`；需 `rstdGm` | A |
| ops-nn `experimental/norm/fused_add_rms_norm/op_kernel/fused_add_rms_norm.h:51-206` | 同上，无 bias | 整行 + 行内 `rowFactor` 批 | `BuildRstd` | 整行 | `blockFactor`×`rowFactor` | 与 single_n 同；含 fp32 | 无 bias；有 `scale` 属性（竞赛无 scale，固定为 1） | A |
| ops-nn `experimental/norm/fused_add_rms_norm/op_kernel/fused_add_rms_norm_common.h:90-107` | `BuildRstd` | — | `Mul→Muls(avgFactor)→ReduceSumCustom→Adds(eps)→Sqrt→Div(1/..)` | — | `rowWork = (blockIdx<numBlocks-1)?blockFactor : numRow-(numBlocks-1)*blockFactor` | **多核尾块余数处理模板**（`FUSED_ADD_RMS_NORM_INIT_ROW_COMMON`） | — | A |
| ops-nn `norm/inplace_add_rms_norm/`（README + op_kernel） | `x1=x1+x2; out=RmsNorm(x1)*gamma`（**in-place，无 bias**） | 整行 | 同 fused 体系 | 整行 | 同体系 | in-place 写法、输出 `x2`（=残差和） | 无 bias；in-place 修改输入 x1；额外 `rstd` 输出 | A/B |
| **ops-nn `norm/add_rms_norm_quant/`**（README.md:18-28；split_d.h:342-348） | `x=x1+x2; out=x/rms*gamma + beta; 再量化` | **切 D**（`jMax`/`colTail`） | `ReduceSumCustom`/`WholeReduceSum`+`BlockReduceSum` | `colTail=numCol-(jMax-1)*ubFactor`（split_d.h:113） | 行切 + `rowFactor` 批 | **唯一含 bias（beta）的官方实现**；beta 在 `*gamma` 之后以 fp32 `Add` 施加（split_d.h:342-348）；D 尾块 | 输出 int8（量化），有 scales/zero_points/divMode 一大堆量化参数；bf16 下 beta 需 x→bf16→fp32 回环（split_d.h:181-185） | A/B |
| cann-samples `Samples/2_Performance/rms_norm_quant_story/src/0_naive.asc:168-204` | `out=RmsNorm(x)*gamma*scale+offset`（量化，**无 residual、无 bias**） | 整行（Step5 才多行批） | **基础 `ReduceSum(reduce,x,work,r)`**（0_naive.asc:182） | 整行 | Step2 多核：`blockFactor/blockTail`，`<<<64,0,stream>>>` | **基础 ReduceSum 直调用法、DataCopyPad 直调、单文件可编译形态** | 无 residual、无 bias；量化到 int8 | A/B |
| cann-samples `rms_norm_quant_story/Story.md`（Step0-6） | 同上 | Step5 多行批 `ubFactor` | Step6 二分累加（pairwise）+ 迭代求 rsqrt | — | Step2 64 核按行均分 | 性能优化方法论（多核/RegAPI/DoubleBuffer/UB 多行/二分累加） | 纯方法论，数学无 bias | B |
| cann-learning-hub `contrib/tutorials/qwen_ops/01_rmsnorm_baseline/op_kernel/rmsnorm_baseline_kernel.h:61-91` | `out=RmsNorm(x)*weight`（**教学 baseline，无 residual、无 bias**） | 无（逐元素 GM GetValue/SetValue，单核标量） | 标量 for 循环累加 | 无 | `coreNum/rowsPerCore` 按行切（教学最简版） | 最简教学骨架、rstd 用 `RmsNormInvSqrtApprox`（Newton 迭代） | fp32 only；无 tiling、无向量化；非生产实现 | A/B |
| asc-devkit `include/basic_api/kernel_operator_vec_reduce_intf.h` | 框架 API | — | 基础 `ReduceSum(dst,src,sharedTmpBuffer,count)`（`int32 count` 重载） | — | — | **候选 V002 用的就是此 API** | 仅接口定义，无算子语义 | A |
| asc-devkit `include/adv_api/reduce/reduce.h` | 框架 API | — | 高阶 `ReduceSum<pattern>`（模板化 `Pattern::Reduce`） | — | — | 高阶归约入口（本报告 RMSNorm 源码未直接使用） | — | A |

> 对照结论：**竞赛 `AddRmsNormBias` 的「residual add + gamma + bias + 非量化 fp16/bf16/fp32 输出」组合，官方仓库里没有 100% 直接对应的实现**。最接近的拼图：residual add+gamma 取自 `fused_add_rms_norm`/mc2 `AddRmsNorm`，bias 项取自量化版 `AddRmsNormQuant`（去掉量化支路），D 分块/尾块/多核取自各类 `*_split_d`。

---

## 4. 关键源码摘录（带文件路径与行号）

### 4.1 官方 AddRmsNorm 内核（无 bias）——核心归约 + 归一化（ops-nn fused_add_rms_norm_common.h:90-107）
```cpp
__aicore__ inline void BuildRstd(LocalTensor<float> sqx, LocalTensor<float> xLocal,
    LocalTensor<float> reduceLocal, float avgFactor, float epsilon, uint32_t count) {
    Mul(sqx, xLocal, xLocal, count);          // y^2
    Muls(sqx, sqx, avgFactor, count);         // * (1/n)
    ReduceSumCustom(sqx, sqx, reduceLocal, count);  // sum -> sqx[0]
    Adds(sqx, sqx, epsilon, 1);               // + eps
    Sqrt(sqx, sqx, 1);
    Duplicate(reduceLocal, ONE, 1);
    Div(sqx, reduceLocal, sqx, 1);            // 1/rms  (rstd)
}
```
> 注意：**这里只有 `y/rms * gamma`，没有 `+ bias`**。`ReduceSumCustom` 在 `fused_add_rms_norm_base.h:50-78` 实现为 `Add` 累加到 work，再 `BlockReduceSum`/`WholeReduceSum`，与候选的直接 `ReduceSum` 基础 API 不同。

### 4.2 多核切分 + 尾块余数（ops-nn fused_add_rms_norm_common.h:33-48）
```cpp
this->blockIdx_ = GetBlockIdx();
this->rowWork = (this->blockIdx_ < GetBlockNum() - 1) ?
    this->blockFactor :
    this->numRow - (GetBlockNum() - 1) * this->blockFactor;
```
> 与候选 V002 `Process()` 中 `first/count` 的「余数前移」策略等价（候选用 `extra=outer%blocks`，前 `extra` 个核多分 1 行）。

### 4.3 D 切分 + 列尾块（ops-nn add_rms_norm_quant_split_d.h:110-138）
```cpp
uint32_t jMax = CeilDiv(numCol, ubFactor);
uint32_t colTail = numCol - (jMax - 1) * ubFactor;   // 尾块自然处理 D 非整
for (j=0; j<jMax-1; j++) ComputeFormer(... ubFactor);
ComputeFormer(... colTail);                            // 尾块
ComputeRstd(rstdLocal, calcRowNum);
for (j=0; j<jMax-1; j++) ComputeLatter(... ubFactor);
ComputeLatter(... colTail);
```
> 两遍：former 用 `ReduceSumFP32ToBlock`+`BlockReduceSumFP32` 把各列块的 `y^2` 累加到每行 partial sum；latter 用 `rstd` 做归一化。`colTail` 同时解决「D 非 32 倍数」问题。

### 4.4 **bias（beta）项施加位置**（ops-nn add_rms_norm_quant_split_d.h:342-348）
```cpp
if (hasBeta) {
    PipeBarrier<PIPE_MTE2>();
    Cast(sqx, betaLocal, RoundMode::CAST_NONE, num);   // beta -> fp32
    PipeBarrier<PIPE_V>();
    Add(xFp32Local, xFp32Local, sqx, num);            // out = y/rms*gamma + beta
    PipeBarrier<PIPE_V>();
}
```
> **这是全仓库里唯一明确实现 `output = y/rms*gamma + bias` 的位置**，且与竞赛语义完全一致（beta 在 `*gamma` 之后以 fp32 加）。竞赛候选 V002 的 `ApplyAffine`（`Mul`+`Add`）与之等价，仅候选未量化。

### 4.5 候选实现 V002 的 bias 路径（/Users/sunyiyang/Desktop/Project/cann/提交/V002/kernel.asc:117-146）
```cpp
__aicore__ inline void ApplyAffine(LocalTensor<float>& value, const LocalTensor<float>& gamma,
                                   const LocalTensor<float>& bias, uint32_t len) {
    Mul(value, value, gamma, len);   // y/rms * gamma
    Add(value, value, bias, len);    // + bias
}
```
> 结论：候选已正确实现 bias（比所有非量化官方 AddRmsNorm 更贴合赛题）。

### 4.6 基础 ReduceSum 用法（cann-samples rms_norm_quant_story/src/0_naive.asc:181-184）
```cpp
Mul(rmsLocalTensor, xLocalTensor, xLocalTensor, r);
ReduceSum(reduceLocalTensor, rmsLocalTensor, xInLocalTensor.ReinterpretCast<float>(), r);
Duplicate(rmsLocalTensor, reduceLocalTensor, r);
Muls(rmsLocalTensor, rmsLocalTensor, rInv_, r);
Adds(rmsLocalTensor, rmsLocalTensor, epsilon, r);
Sqrt(rmsLocalTensor, rmsLocalTensor, r);
Div(xLocalTensor, xLocalTensor, rmsLocalTensor, r);
```
> 与候选 V002 `CopyAndReduce` 中 `ReduceSum(sum, value, work, calc_len)` 完全一致——均为基础 API，且 `work` 缓冲区语义相同。

### 4.7 epsilon 属性校验（ops-transformer common_add_rms_norm_tiling.cpp:191-194）
```cpp
OP_TILING_CHECK((epsilon != nullptr && (*epsilon <= 0 || *epsilon >= 1)),
    ... "epsilon", "(0, 1)", return ge::GRAPH_FAILED);
```
> 竞赛以 `float epsilon` 运行时参数传入，无此范围校验（判题约定 eps 合理）。

---

## 5. 可迁移点（≥3）与不可迁移点（≥3）

### 可迁移点
1. **D 维度切分 + 列尾块（`colTail`）模板**：`jMax=ceil(numCol/ubFactor)`、`colTail=numCol-(jMax-1)*ubFactor`、两遍（former 累加 / latter 归一化）可直接用于竞赛大 D（[64,32768] 且可能非 32 倍数）。候选 V002 已用 `tile_len` 切 D，可对照官方把「两遍」改成「保持 y 在 UB 内一次过」以减少一次 GM 回读。（来源：add_rms_norm_quant_split_d.h:110-138；add_rms_norm_split_d.h:88-117）
2. **多核按行切分 + 余数归尾核**：`blockFactor=ceil(numRow/numCore)` + 最后一块吃余数，或直接用候选的 `extra` 前移策略；核内 `rowFactor` 多行批处理可提升 UB 利用率。（来源：fused_add_rms_norm_common.h:33-48；common_add_rms_norm_tiling.cpp:282-287；cann-samples Story Step2/Step5）
3. **`output = y/rms*gamma + bias` 的归一化+仿射顺序与 fp32 中间量**：先 `y^2*avgFactor` 归约 → `+eps` → `Sqrt` → `1/rms` → `*y` → `*gamma` → `+beta`，全程 fp32 中间缓冲，beta 在最后以 fp32 `Add`。候选 V002 已遵此顺序，可直接对齐。（来源：add_rms_norm_quant_split_d.h:342-348；BuildRstd）
4. **rstd 取标量 / 事件同步 V_S↔S_V 模板**：把归约结果从 VEC 搬到 Scalar 读标量再搬回，是官方稳定写法，候选的 `event_vs/event_sv` 同步可对照。（来源：mararn_rms_norm_base.h:121-127）
5. **性能优化方法论（判题直调形态最相关）**：多核并行（Step2，~60x）、Double Buffer 流水重叠（Step4，1.55x）、UB 多行批（Step5）、二分累加+迭代 rsqrt 精度（Step6）。这些与「是否融合/量化」无关，可直接套用到候选。（来源：cann-samples rms_norm_quant_story/Story.md）

### 不可迁移点
1. **量化支路（int8 输出 + scales/zero_points/divMode）**：`AddRmsNormQuant` 一大半代码是 `RoundFloat2Int8`、scales/zero_points 加载、`doScales/doZeroPoints`。竞赛输出 fp16/bf16/fp32，**必须整体删除量化路径**，只保留 normalize+gamma+beta。（来源：add_rms_norm_quant_base.h:156-233；add_rms_norm_quant_split_d.h:349-397）
2. **融合大算子上下文（MatMul + AllReduce + HCCl）**：ops-transformer 的 `matmul_all_reduce_add_rms_norm` 是 MoE 通算融合算子，依赖 `Hccl<HCCL_SERVER_TYPE_AICPU>`、`SyncAll`、`GetCurFinishedCnt` 等集合通信原语与 `op_graph` 构图；竞赛是独立小 vector 算子，**完全用不到**。（来源：add_rms_norm_kernel.h:36-95）
3. **msopgen/Aclnn 工程形态**：官方每个算子有 `op_host`（tiling/def/infershape）、`op_kernel`、`op_api`（aclnn 封装）、`op_graph`、`examples`、`tests/ut`、`CMakeLists`。判题只吃一个 `kernel.asc` + `run_kernel` 入口，tiling 由 CANNJudge 隐式给 outer/dim/epsilon。**不能把官方工程直接提交**。（来源：各仓库目录；见第 6 节）
4. **部分 kernel 缺 fp32 分支 / 缺 bias**：ops-transformer mc2 各 AddRmsNorm kernel 与 SPLIT_D 仅 fp16/bf16；非量化版均无 bias。竞赛要求 fp32 输入 + bias，需自行补 fp32 分支与 bias（候选已补）。（来源：add_rms_norm_single_n.h:44-49；add_rms_norm_split_d.h:240-290）
5. **`scale` 属性（fused_add_rms_norm 的 `x1*scale+x2`）**：竞赛是纯 `x+residual`（scale=1），官方 `scale` 默认 1.0 但属可选属性，移植时忽略即可，但注意不要把 scale 误当成 bias。（来源：fused_add_rms_norm.h:36；README.md:72-77）

---

## 6. 形态差异说明（msopgen 工程 vs 判题直调单文件）

官方仓库（ops-nn / ops-transformer）的完整算子工程结构（以 `ops-nn/norm/fused_add_rms_norm` 为例）：
```
fused_add_rms_norm/
├── op_kernel/            # 设备侧内核 .h（fused_add_rms_norm.h / *_single_n.h / *_split_d.h / *_base.h / *_common.h）
├── op_host/              # 主机侧：tiling（arch*/..._tiling.cpp）、def（..._def.cpp）、infershape、config/*.json
├── op_api/               # aclnn 封装：aclnnFusedAddRmsNorm.cpp/.h（用户调用入口）
├── op_graph/             # 图模式 IR：fused_add_rms_norm_proto.h
├── examples/             # test_aclnn_fused_add_rms_norm.cpp（CPU 侧 main + acl 初始化 + 校验）
├── tests/ut/op_kernel/   # 设备侧单测（核函数直调）
├── tests/ut/op_host/     # 主机侧 tiling/infershape 单测
├── docs/                 # aclnn 接口文档
└── CMakeLists.txt        # 编译入口（msopgen 生成）
```
- **构建**：依赖 CANN 的 `msopgen`/cmake 工具链 + `kernel_tiling.h` 自动生成的 tiling 结构体，需 `CMAKE` 配置 `NPU_ARCH`。
- **调用**：用户经 `aclnnFusedAddRmsNorm(...)`（op_api）或图 IR 调用；tiling 在 `op_host` 内于 CPU 侧计算后随 kernel 下发。

竞赛 CANNJudge 直调形态（`提交/V002/kernel.asc`）：
```
kernel.asc
└── 单个文件，包含：
    ├── 设备侧类 AddRmsNormBiasKernel（Init/Process，含 tiling 常量硬编码）
    ├── __global__ add_rms_norm_bias_kernel(...)   // 核函数入口，<<<blocks,0,stream>>>
    └── extern "C" void run_kernel(...)            // CANNJudge 直调入口
                                        // 参数：x/residual/gamma/bias/output + TensorGroupInfo + availableCoreNum + stream + epsilon
```
- **构建/调用**：由 CANNJudge 直接编译 `kernel.asc` 并调用 `run_kernel`；**无 op_host tiling、无 op_api、无 op_graph、无 CMakeLists**。
- **tiling 来源差异**：官方 tiling 在 `op_host` 用 `PlatformAscendC::GetCoreNumAiv()/GetCoreMemSize(UB)` 在 CPU 侧算出 `block_factor/row_factor/ub_factor` 后下发；判题把这些当成由框架隐式决定，候选 V002 在核内用 `tile_len` 常量（fp16=4096/fp32=2048）+ `GetBlockNum()` 自行切分，**不依赖主机侧 tiling 工程**。

> 结论：官方源码**只能作为算法/分块/归约/事件同步的参考范本**，其工程骨架（op_host/op_api/op_graph/CMake）对判题不可移植；候选 V002 已是正确形态，无需套用官方工程结构。

---

## 7. 未确认事项与访问限制

- **未确认**：`AddRmsNormQuantV2` 与 `AddRmsNormQuant` 在内核层面的差异（V2 是否改了量化/归约实现）。仅从文件名与 `aclnn_add_rms_norm_dynamic_quant_v2.cpp` 存在推断 V2 存在，未读其 kernel（在 `add_rms_norm_dynamic_quant/` 下，与 `add_rms_norm_quant/` 并列；两者均含 beta 语义，差异主要在动态量化）。如需精确差异，建议后续读 `add_rms_norm_dynamic_quant/op_kernel/`。
- **未确认**：`ops-transformer/experimental/attention/.../rms_norm.h` 与 `mc2` 版算法是否一致（注意力内 RMSNorm，上下文强耦合，未深入读）。
- **未确认**：官方是否对「fp32 输入 + SPLIT_D」给出支持路径——mc2 SPLIT_D 仅见 fp16/bf16 重载，fp32 在 SPLIT_D 下疑似不支持（需编译验证，本机无法）。
- **访问限制**：本机 macOS 无 CANN 工具链/NPU，**不能编译、不能运行、不能 profiling**；所有结论基于静态源码阅读。5 个官方仓库均浅克隆成功，无官方仓库访问失败；未使用任何个人 fork（故无 C 级来源）。
- **范围外**（按指令已回避）：未搜比赛规则、未搜 Ascend C API 文档语义（属 Agent02）、未搜 GPU/CUDA、未设计数值测试矩阵、未搜 Linux 环境、未搜竞赛失败案例。

---

## 8. 来源表

| 编号 | 名称 | URL | 版本 / commit | 访问日期 | 用途 | 等级 |
|---|---|---|---|---|---|---|
| S1 | ops-transformer（mc2 AddRmsNorm） | https://gitcode.com/cann/ops-transformer | master d967eeb08ceef7c394d0d5e27767e5b08b77aa58 | 2026-09-11 | AddRmsNorm 多模式内核（含 SPLIT_D/尾块/无 bias）、tiling 决策 | A |
| S2 | ops-nn（fused_add_rms_norm） | https://gitcode.com/cann/ops-nn | master 1c891ca0bbc8852a3ef80fbf7d067c8ce034e592 | 2026-09-11 | residual+gamma 融合内核、含 fp32 分支、无 bias | A |
| S3 | ops-nn（add_rms_norm_quant，含 beta） | https://gitcode.com/cann/ops-nn | 同上 | 2026-09-11 | 唯一含 `+beta`（bias）的官方实现、D 切分+尾块、量化 | A/B |
| S4 | ops-nn（inplace_add_rms_norm） | https://gitcode.com/cann/ops-nn | 同上 | 2026-09-11 | in-place AddRmsNorm（无 bias）对照 | A/B |
| S5 | cann-samples（rms_norm_quant_story） | https://gitcode.com/cann/cann-samples | master 23c981c0918e3183958e94e58ef6989d44983230 | 2026-09-11 | 单文件 .asc 基础 ReduceSum 用法、7 步优化方法论 | A/B |
| S6 | cann-learning-hub（rmsnorm_baseline） | https://gitcode.com/cann/cann-learning-hub | master 9f8e8e2b9b2ffc79846a2cdf29b4261e296cadb2 | 2026-09-11 | RMSNorm 教学 baseline（fp32 标量骨架、Newton rsqrt） | A/B |
| S7 | asc-devkit（ReduceSum 基础/高阶 API） | https://gitcode.com/cann/asc-devkit | master d6ea6db110f5924a49cbbb941abef44722e3b72c | 2026-09-11 | 基础 `ReduceSum(dst,src,work,count)` 与高阶 `Pattern::Reduce` 接口定义 | A |
| S8 | 竞赛候选 V002 | /Users/sunyiyang/Desktop/Project/cann/提交/V002/kernel.asc | 本地 | 2026-09-11 | 对照：已实现 bias、基础 ReduceSum、D 切分 | A |

> 注：全部为官方仓库（gitcode.com/cann/*），无个人 fork，故无 C 级。
