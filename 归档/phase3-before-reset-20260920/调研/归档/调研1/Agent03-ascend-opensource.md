# Agent 3 调研报告：昇腾官方开源仓库源码调研（AddRmsNormBias 参考实现）

> 调研日期：2026-09-11　|　调研 Agent：Agent 3（官方开源仓库）
> 任务：为 CANN 挑战赛初赛题目 AddRmsNormBias（`y = x + residual; rms = sqrt(mean(y², dim=-1) + eps); out = y/rms*gamma + bias`，CANN 9.0.0，vector Kernel，Direct Invocation）调研官方开源实现。
> **本机为 macOS，无 CANN/NPU。本轮所有结论均未在真实 NPU 编译/精度/性能验证。** 以下每条关键结论均标注验证状态。
> 调研边界：不抓 hiascend API 文档（Agent 2 负责）；不研究 GPU/CUDA（Agent 4 负责）；不研究竞赛规则。

---

## 一、仓库清单

| # | 仓库 | URL | commit | 本地路径 | 读取状态 |
|---|------|-----|--------|----------|----------|
| 1 | cann-samples（官方性能实战样例） | gitcode.com/cann/cann-samples；GitHub 镜像 github.com/Ascend/cann-samples | `23c981c0918e3183958e94e58ef6989d44983230`（2026 修复 StreamK 示例） | /tmp/cann-samples.vc2a1d | ✅ 成功（上一轮快照，7 个 `.asc` 已读 4 个关键文件） |
| 2 | ops-transformer（官方算子库，MC2/MoE 组合算子） | gitcode.com/cann/ops-transformer；GitHub 镜像 ai2open/cann-ops-transformer | `e7019c299cfc02293b184dcf0ca270b08740cdc1`（2026 增加 MsaIndexScore 950 实现） | /tmp/ops-transformer.eKIcVc | ✅ 成功（上一轮快照） |
| 3 | ops-nn（官方神经网络算子库，aclnnAddRmsNorm / aclnnRmsNorm / AddRmsNormQuant 系列） | gitcode.com/cann/ops-nn（gitee 与 GitHub Ascend/ops-nn 均需登录/404，gitcode 可 clone） | `9b594837`（master，2026-09 修复 logit 算子 A5 精度） | /tmp/ops-nn.tmp | ✅ 成功（本轮 gitcode clone，约 340MB，完整 checkout） |
| 4 | cann-learning-hub（官方学习仓库） | GitHub 镜像 github.com/hicann/cann-learning-hub（Ascend org 下不存在；gitee gitcode 需登录） | master（部分文件） | /tmp/learning-hub-partial/qwen_ops/（GitHub raw 拉取 5 个文件）；clone 中途放弃 | ⚠️ 部分（关键文件已通过 GitHub API/raw 拉取） |

**核心结论先行**：官方已有与本题公式**几乎一致**的实现——`ops-nn` 的 **AddRmsNormQuantV2**（公式 `y_i = x_i * gamma_i / Rms(x) + bias`，x = x1+x2），只差"输出类型"（官方输出 int8 量化，本题输出原类型）。其 kernel 中 bias 融合、ReduceSum、尾块处理、Tiling 结构全部可直接借鉴。

---

## 二、关键实现分析

### 2.1 ops-nn `norm/add_rms_norm`（官方标准 AddRmsNorm 算子，A 级）

**文件**：/tmp/ops-nn.tmp/norm/add_rms_norm/

- **README.md:19-27** 公式：`x_i = x1_i + x2_i`；`RmsNorm(x) = x_i/Rms(x) * g_i`，`Rms(x) = sqrt(1/n * Σx² + eps)`。
  - **与本题差异**：官方 AddRmsNorm **无 bias**，且输出 3 个张量：`y`（归一化结果）、`rstd`（Rms）、`x`（Add 结果）。
- **参数（README.md:46-96）**：x1/x2/gamma 支持 FLOAT32/FLOAT16/BFLOAT16，ND 格式；epsilon 属性默认 1e-6。
- **op_host/add_rms_norm_tiling.h:22-43**：`AddRMSNormTilingData` 字段：`num_row, num_col, block_factor, row_factor, ub_factor, epsilon, avg_factor, num_col_align, last_block_factor, row_loop, last_block_row_loop, row_tail, last_block_row_tail, mul_loop_fp32, mul_tail_fp32, dst_rep_stride_fp32, mul_loop_fp16, mul_tail_fp16, dst_rep_stride_fp16, is_performance`（`TILING_DATA_FIELD_DEF` 宏，REGISTER_TILING_DATA_CLASS 注册）。
- **op_host/add_rms_norm_tiling.cpp:352-360**：`CalculateBlockParameters`：`blockFactor = ceil(numRow/numCore)`（块因子=每核行数），`useCoreNum = ceil(numRow/blockFactor)`，`latsBlockFactor = numRow - blockFactor*(useCoreNum-1)`。
- **op_host/add_rms_norm_tiling.cpp:370-424 `DetermineModeParameters`** 模式选择（**本题 Tiling 直接照搬此逻辑**）：
  - `numCol > ubFactor(12288 B16 / 10240 B32)` → **SPLIT_D**（D 维分块）：ubFactor = `ceil(numCol / (ceil(numCol/CUTD) * dataPerBlock)) * dataPerBlock`（dataPerBlock = 16 for B16/BF16，8 for FP32）。
  - `blockFactor == 1`（且非 310P）→ **SINGLE_N**（单核单行，大 UB 常驻）。
  - `numColAlign <= 2000`（且非 310P）→ **MERGE_N**（多行合并 reduce）：`rowFactor = ubSize/(numColAlign*weight + 260)`，`ubFactor = rowFactor*numColAlign`。
  - FP16 且 `numCol == numColAlign` → **MULTI_N**：`rowFactor = (ubSize - 1024 - 256 - numColAlign*2)/(numColAlign*16 + 64)`。
  - 其余 → **NORMAL**（rowFactor=64，ubFactor=12288）。
  - tilingKey = `dtypeKey*10 + modeKey + norm_key`（dtypeKey：FP16=1、FP32=2、BF16=3；modeKey：NORMAL=0、SPLIT_D=1、MERGE_N=2、SINGLE_N=3、MULTI_N=4）。
- **op_kernel/add_rms_norm.cpp:30-127** 入口：按 tilingKey 分发到 `KernelAddRmsNorm`/`SplitD`/`MergeN`/`SingleN`/`MultiN`，模板参数 MODE=1（ADD）/2（PRE）/3（POST）控制 rstd/x 输出开关。
- **op_kernel/add_rms_norm.h（NORMAL 版，346 行）** 单行整行处理流程：
  - `CopyIn`（L108-153）：x1、x2 各搬一行；half 路径 `Add(xLocal, x1, x2, numCol)` 在 **half 域相加** → `Cast(x1_fp32, xLocal, CAST_NONE)` 存 FP32（L123-128）；bf16 路径先各自 Cast 到 FP32 再 Add（L129-138）；x1+x2 输出到 `xGm`。
  - `Compute`（L253-298，half 版）：`Mul(sqx, x_fp32, x_fp32)` → `Muls(sqx, avgFactor)` → `ReduceSumCustom(sqx, sqx, reduce_buf, numCol)` → `Adds(eps, 1)` → `Sqrt(1)` → `Div(1/sqrt)` → `GetValue(0)` 取标量 rstd（V_S/S_V 事件同步）→ `Muls(x_fp32, rstdValue)` → `Cast(yLocal, x_fp32, CAST_NONE)` → `Mul(yLocal, gammaLocal, yLocal)`（**gamma 在 half 域乘**）。
  - **可迁移点**：① 第一遍 Add 后把 x1+x2 以 FP32 保留在 `xFp32Buf`，第二遍直接用，不重新搬 GM；② rstd 用 `Div(Duplicate(1), sqrt)` 得倒数再标量乘，避免逐元素除法；③ `GetValue(0)` 前后用 `SetFlag/WaitFlag<HardEvent::V_S>` 与 `S_V` 同步。
  - **差异**：无 bias；gamma 相乘在 half 域（half 输入时），本题若追求精度可在 FP32 域乘 gamma 再加 bias。
- **op_kernel/add_rms_norm_split_d.h（D 维分块版，406 行）**——**大 D（D>12288）时本题首选**：
  - `Process`（L75-85）：`i_o_max = ceil(rowWork/rowFactor)`（行分块）× `j_max = ceil(numCol/ubFactor)`（D 维分块，`col_tail` 处理尾块）。
  - 第一遍 `ComputeFormer`（L199-209）：每 j 块 `CopyInAndAdd`（x1+x2 输出 xGm + `xFp32Buf` 保留 FP32 结果，L150-197）→ `ComputeSum`（`Mul` 平方 → `Muls(avgFactor)` → `ReduceSumFP32ToBlock(sumLocal[i*8], ...)`，每行部分和放 8 个 float）→ 块末 `BlockReduceSumFP32` + `Add(rstdLocal, rstdLocal, sumLocal)` 跨块累加。
  - `ComputeRstd`（L194-204）：+eps → Sqrt → Div(1, rstd)。
  - 第二遍 `ComputeLatter`（L206-217）：每 j 块 `CopyInGamma`（gamma 按 `gammaGm[j*ubFactor]` 分块搬）→ **`CopyInX` 从 `xGm` 重读 x1+x2**（因为 FP32 缓冲只有 ubFactor 大小，放不下整行）→ `ComputeY`（`GetValue(i_i)` 取该行 rstd → `Muls` → Cast → `Mul(gamma)`）→ `CopyOutY` 写回 normOut。
  - **可迁移点**：D 分块 + 每块部分和 + 跨块 BlockReduceSum 累加的两遍扫描结构；gamma/bias 均可按 j 块搬运。
  - **差异**：无 bias；第二遍重读 xGm（本题同样适用，因 UB 放不下整行 FP32）。
- **op_kernel/add_rms_norm_base.h:48-93**：`ReduceSumFP32ToBlock`（Add 折叠 + `BlockReduceSum`，count 需 < 255 repeat ≈ 16320 个 FP32）与 `BlockReduceSumFP32`（对 8 倍数输入）。**DataCopyCustom（L95-132）**：非 32B 对齐时 GM→UB 直接 `AlignUp` 多搬（padding 被 mask 忽略）；UB→GM 时先搬对齐部分，再用 `GetValue/SetValue` 把尾部元素搬到 buffer 头部后整块写出（**DataCopyPad 之外的官方尾块处理方案，910 系列可免 DataCopyPad**）。

### 2.2 ops-nn `norm/rms_norm`（官方标准 RmsNorm 算子，A 级）

**文件**：/tmp/ops-nn.tmp/norm/rms_norm/

- **op_kernel/rms_norm_base.h:137-171 `ReduceSumFP32`**：`Duplicate(work, 0, 64)` → `Add(work, src, work, mask=64, repeatTimes, BinaryRepeatParams{src0RepStride=8, src1RepStride=0, dstRepStride=0})` 将每 64 个元素折叠为部分和 → `AscendCUtils::SetMask<float>(64)` → `WholeReduceSum(dst, work, ...)` 归约 64 个部分和 → 标量。**这是官方标准 ReduceSum 替代方案**（对长度 > 255 repeat 的场景：首轮 Add 折叠即可支持任意 count，尾数用 tailCount 单独 Add）。
- **op_kernel/rms_norm_whole_reduce_sum.h（910 平台版，368 行）**：
  - 多行并行：`once_num = rowFactor * numColAlign` 一次处理多行，`num_col_align = ceil(numCol/16)*16`（DataCopy 按对齐列搬）。
  - `ComputeRstd`（L139-182）：`ReduceSumHalfInterval`（**二分折叠归约**：`findPowerTwo(count)` 后每次 `Add(src, src, src[bodyCount], bodyCount)` 对半折叠，直到 ≤64 元素，再 `WholeReduceSum`；就地修改 src）→ 对每行 `GetValue(i*8)` 标量 rstd → `Muls` 逐行归一化。
  - **可迁移点**：`ReduceSumHalfInterval` 比 ReduceSumFP32 少一次 Duplicate/work 缓冲，适合整行在 UB 的场景；rstd 多行批量 Muls/Adds/Sqrt/Div 用 `repeat` 参数（255 上限分段）。
- **op_host/rms_norm_tiling.cpp:199-265 `CalMixDtypeTiling`（混合精度 x FP16/γ FP32）**：UB 预算公式 `oneRowBufSize = B16*2*numColAlign + FP32*1*numColAlign（x+y+gamma）+ FP32*2*numColAlign（tmp）+ rstd + reduce`；模式选择顺序 **MergeN（小 D）→ Normal → SingleRow → SplitD**。
- **op_host/rms_norm_tiling.cpp:434-460（同源非混合精度）**：`ubFactor = UB_FACTOR_B16(12288)/B32(10240)`；SPLIT_D 时 `colTileNum=ceil(numCol/ubFactor)`，`ubFactor = ceil(numCol/(colTileNum*16))*16`，且循环微调保证 `numCol % ubFactor == 0` 或 `余数 >= 16`（**尾块不小于 16 元素 = 32B 对齐，避免跨行覆盖**——本题"D 非 32 倍数不覆盖相邻行"约束的官方解法）。
- **与本题差异**：RmsNorm 无 Add 无 bias，仅参考其归约与 Tiling 框架。

### 2.3 ops-nn `norm/add_rms_norm_quant` + `add_rms_norm_quant_v2`（**本题公式最接近，A 级）**

**文件**：/tmp/ops-nn.tmp/norm/add_rms_norm_quant/、/tmp/ops-nn.tmp/norm/add_rms_norm_quant_v2/

- **add_rms_norm_quant_v2/README.md**：公式 `y_i = 1/Rms(x) * x_i * gamma_i + bias`（**与本题完全一致**），新增可选输出 `res_out`（量化前的归一化结果，不含 bias）。V2 相对 V1 仅增加 `bias` 输入。
- **add_rms_norm_quant_v2/op_kernel/add_rms_norm_quant_v2.cpp**：复用 V1 的 `KernelAddRmsNormQuant`/`SplitD`/`SingleN` 模板，模板参数 `<TX, TScale, TOffset, RN, A, PT>`（RN=是否输出 res_out；A=是否输出 x；PT=per-tensor 量化），多传 `bias` GM 指针。
- **add_rms_norm_quant/op_kernel/add_rms_norm_quant_kernel.h（NORMAL 版，396 行）**：
  - `CopyIn`（L138-192）：同 add_rms_norm.h（half 域 Add → Cast FP32 保留），`hasBeta` 时 bf16 路径多做一次"x→bf16→fp32"往返（L171-175）。
  - `Compute`（L215-251）：rstd 计算与 2.1 完全一致（FP32 全程 + GetValue(0) 标量）。
  - `ComputePart2`（L253-291）：**bias 融合（L282-289）**：
    ```cpp
    if (hasBeta) {
        PipeBarrier<PIPE_MTE2>();
        LocalTensor<float> betaFp32 = sqxBuf.Get<float>();
        Cast(betaFp32, betaLocal, RoundMode::CAST_NONE, numCol);   // bias(T) → FP32
        PipeBarrier<PIPE_V>();
        Add(xFp32Local, xFp32Local, betaFp32, numCol);              // FP32 域加 bias
        PipeBarrier<PIPE_V>();
    }
    ```
    —— **归一化+gamma 之后、量化之前，在 FP32 域加 bias**。本题去掉 doQuant 改为 Cast 回目标类型输出即可。
  - gamma 相乘：half 路径 `Cast(xFp16Cast, xFp32, CAST_NONE)` → `Mul(xFp16Cast, gammaLocal, xFp16Cast)`（half 域）→ `Cast(xFp32, xFp16Cast, CAST_NONE)` 回 FP32（L256-263）；bf16 路径 gamma Cast 到 FP32 在 FP32 域乘（L264-268）。**注意官方 half 路径 gamma 在 half 域乘、bias 在 FP32 域加**。
  - 可选输出 `res_out`（RN，L270-281）：归一化结果（不含 bias）输出。
- **add_rms_norm_quant/op_kernel/add_rms_norm_quant_split_d.h**：D 分块版，`ComputeY`（L300-346）中 bias 同样在 FP32 域 `Add(xFp32Local, xFp32Local, sqx)`（L342-346），且 **bias 按 j 块搬运**（`betaGm[jIdx*ubFactor]`，L281）。
- **add_rms_norm_quant/op_host/add_rms_norm_quant_tiling.h:37-38**：tiling 含 `hasBeta` 字段；**add_rms_norm_quant_tiling.cpp:124-126**：`hasBeta = betaExist ? 1 : 0`（可选输入存在性判断）；L158-169：可选参数（beta/scales/zero_points）按元素数扣减 ubFactor（`optionalUbNum_0 = hasBeta*2 + hasZeroPoints1*4 + 4` 等）；L210-226：有可选参数时 NORMAL/SINGLE_N/SPLIT_D 的 ubFactor 重新按 `(ubSize - 静态开销)/(静态块数 + 可选参数元素数)` 计算。
- **差异**：输出为 int8（量化路径），无 rstd 输出；本题输出原类型且无量化。**量化相关（doScales/doZeroPoints/RoundFloat2Int8）不可迁移**，其余全部可迁移。
- **未验证**：本机无法编译；`hasBeta` 的 bf16 往返转换（L171-175）意图是保证 x 输出（xGm）为 bf16 舍入后的 x1+x2，本题无 x 输出时不需要。

### 2.4 ops-transformer `mc2/matmul_all_reduce_add_rms_norm`（MoE 组合算子，A 级）

**文件**：/tmp/ops-transformer.eKIcVc/mc2/matmul_all_reduce_add_rms_norm/

- **公式（docs/aclnnMatmulAllReduceAddRmsNorm.md + op_kernel/mm_allreduce_add_rms_norm_910_general.h:86-87）**：`x1 = matmul(a,b)+bias_matmul`；`y = x1 + residual`；`normOut = rmsnorm(x1+residual)*gamma`。**AddRmsNorm 部分本身无 bias**（bias 是 matmul 的 bias，经 `PreProcForBiasOnVector` 在 vector 侧预处理）。
- **op_kernel/add_rms_norm_kernel.h:25-33**：5 种 tilingKey：`ADD_RMS_NORM(10)`、`SPLIT_D(11)`、`MERGE_N(12)`、`SINGLE_N(13)`、`MULTI_N(14)`（+BF16 变体 30/31/32/33）。**与 ops-nn 完全同构**（同一套代码族）。
- **op_kernel/add_rms_norm.h（NORMAL 版，302 行）**：与 ops-nn 版一致（half 域 Add → Cast FP32 → rstd 标量 → Muls → Cast 回 half → Mul(gamma)），差异是 gamma 用 `inQueueGamma` 队列常驻。
- **op_kernel/add_rms_norm_multi_n.h（274 行）**：多行并行版：`Add(xLocal, x1, x2, calc_row_num*numColAlign)`（**按对齐列数算，padding 为 0 无影响**，L143）；`ComputeRstd` 每行 `ReduceSumCustom(rstdLocal[i*8], sqx[i*numColAlign], reduce_buf, numCol)`（L180-182）；`ComputeY` 用 `Gather(rstdLocal, rstdLocal, offsetLocal)` 把每行 rstd 展开为 repeat 向量后 `Mul` 带 `{src1RepStride=0}` 参数（L211-226，**一行乘同一标量的 repeat 技巧**）。
- **op_kernel/add_rms_norm_merge_n.h（438 行）**：小 D 版，`BroadCastGamma`（L172-183，`adv_api/pad/broadcast.h` 的硬件广播指令把 gamma 从 1×D 广播到 rowFactor×D）与 `BroadCast<float, DIM_NUM, 1>(sqx, rstdLocal, {calc_row_num, numColAlign}, {calc_row_num, 1})`（L220，rstd 从 N×1 广播到 N×D）；reduce 用 `ReduceSumMultiN`（3rd/norm_common/op_kernel/reduce_common.h:95-105，要求 D < 255*8=2040）。
- **op_kernel/add_rms_norm_split_d.h（327 行）**：与 ops-nn split_d 同构。
- **op_host/op_tiling/arch22/common_add_rms_norm_tiling.h:45-50 + tiling_data.h:28-36**：`AddRMSNormTilingData{num_row, num_col, block_factor, row_factor, ub_factor, epsilon, avg_factor}`（**精简 7 字段版**）；`AddRMSNormTilingeKeyData{ARNKeyTile, ARNKeyTail, ARNNumBlocksTile, ARNNumBlocksTail}`（tile/尾块分开的 key 与核数）。
- **op_host/op_tiling/arch22/common_add_rms_norm_tiling.cpp:198-254 `SetAddRmsNormTilingData`**：模式选择与 ops-nn 相同（SPLIT_D 阈值 ubFactor=12288/10240，MERGE_N 阈值 2000，rowFactor=64），UB_FACTOR 常量一致（L23-26）。
- **差异**：整体是多卡（HCCL allreduce + 跨核同步 `SyncAll` + `rcvCntGM` 流水协调，add_rms_norm_kernel.h:56-95）与 matmul 融合场景；**通信算子、Hccl 句柄、KERNEL_TYPE_MIX_AIC_1_2 等不可迁移**；其纯 vector 的 AddRmsNorm 内核部分可迁移（与 ops-nn 同源）。
- **inplace_matmul_all_reduce_add_rms_norm / moe_distribute_combine_add_rms_norm**：前者是 matmul+allreduce+addrmsnorm 的 inplace 变体（复用同一内核，仅地址复用）；后者含 MoE 路由/combine 逻辑（arch22/moe_distribute_combine_add_rms_norm_a3.cpp 仅 50 行入口，核心在 a2a 通信）。两者核心 AddRmsNorm 计算与 2.1 相同，**通信与路由部分不可迁移**。

### 2.5 cann-samples `Samples/2_Performance/rms_norm_quant_story`（性能优化实战样例，B 级官方样例）

**文件**：/tmp/cann-samples.vc2a1d/Samples/2_Performance/rms_norm_quant_story/

- **Story.md:5-8**：Ascend 950PR（64 Vector Core 仿真），`x[4096, 8192] fp16 → y int8`，**7693us → 49.0us（157×）**。优化步骤：0 公式直译 → 1 gamma 预加载 → 2 多核并行 → 3 寄存器数据流（VF RegAPI）→ 4 双缓冲 → 5 UB 高利用率 → 6 二分累加 + 迭代 rsqrt。
- **src/0_naive.asc（单核基线，L127-213）**：整行一个 tile（r=8192）；**`ReduceSum(reduceLocal, rmsLocal, xInLocalTensor.ReinterpretCast<float>(), r)` 直接用输入队列 tensor 的 ReinterpretCast 作 workLocal（L183）**——无需独立 work buffer；`Duplicate(rmsLocal, reduceLocal, r)` 标量广播（L184）。
- **src/2_multi_core.asc（L324-334 calcTiling）**：`blockFactor = ceil(a/coreNum)`，`blockNum = ceil(a/blockFactor)`，`blockTail = a - blockFactor*(blockNum-1)`，每核 `SetGlobalBuffer(x + blockIdx*blockFactor*r)`（L129-133）——按行多核切分。
- **src/6_binary_sum.asc（L143-222）**：`ComputeSquareReduceSum` 二分累加：`calcBinaryAddPoint`（L567-581）取 ≥r/2 的 2 的幂为折叠点；`flodAddLoops/flodAddTailLoops/binaryAddLastLoops` 两轮折叠后每行部分和存 `rmsAddr + loopA`；**Reg::ReduceSum 在寄存器累加**（vregReduceSum += vregXQuared 每 VL 一次）；`ComputeRstdVf`（L225-282）用 **Newton 迭代求 rsqrt**（`y*(1.5 - 0.5*x*y*y)` 两轮）+ `CompareScalar/Select` 处理 `var==0 → +inf`、`var==+inf → 0` 特殊值；L215 注释：**`UpdateMask` 会破坏 count 标量，循环后必须重新赋值**（Reg API 坑）。
- **src/5_ub_utilization.asc:428-454 `calcMaxUbFactor`**：`maxUbFactor = (ubSize - fixedSize)/linearCoef`，其中 fixedSize = `rAlign*(sizeof(dataType)+sizeof(float)) + 32`（gamma 输入+FP32 缓冲），linearCoef = `rAlign*(sizeof(dataType)*BUF_NUM + sizeof(outputType)*BUF_NUM) + sizeof(float)`（x 队列、y 队列、rms 缓冲）——**UB 利用率最大化的 ubFactor 解析公式**。
- **src/6_binary_sum.asc:583-641 calcTiling**：数据搬运用 `DataCopyPad blockCount=ubFactor, blockLen=r*sizeof(T), dstStride=(rAlign-r)*sizeof(T)/32`（多行 + 32B 对齐 pad，L411-418）；tiling 用 `GetCoreMemSize(UB)` + `GetCoreNumAiv()`。
- **差异**：RmsNormQuant 无 Add 无 bias，输出 int8；但**性能方法论（多核、双缓冲、VF RegAPI、二分累加、UB 利用率公式、rsqrt 迭代）全部适用于本题性能优化阶段**。VF RegAPI 依赖 `basic_api/reg_compute/kernel_reg_compute_utils.h`（CANN 9.0 新 API），正确性风险较高，建议先实现标准 API 版本再考虑。
- **CMakeLists.txt:12-49**：`cann_sample_check_arch(dav-3510)`；`.asc` 直接 `add_executable` + `--npu-arch` 编译选项；构建 `cmake -S . -B build -DNPU_ARCH=dav-3510 && cmake --build build --target rms_norm_quant_story`；仿真 `cannsim record <bin> -s Ascend950 --gen-report`。

### 2.6 cann-learning-hub `contrib/tutorials/qwen_ops/01_rmsnorm_baseline`（官方训练营 RMSNorm 课程，B 级官方教程源码）

**文件**：/tmp/learning-hub-partial/qwen_ops/（GitHub raw：github.com/hicann/cann-learning-hub，master）

- **rmsnorm_baseline_kernel.cpp（34 行）+ rmsnorm_baseline_kernel.h（105 行）**：
  - **Direct Invocation 模板范例**：`rmsnorm_baseline_kernel_do(uint32_t blockDim, void* stream, uint8_t* input, ..., uint8_t* tiling)`（extern "C"，kernel 用 `<<<blockDim, nullptr, stream>>>` 启动）——与本题 Direct Invocation 直调模板一致。
  - **Tiling 传递方式**：tiling 作为 `__gm__ uint32_t/float` 数组直接 reinterpret 读取（rows/hidden/coreNum/rowsPerCore/eps/invHidden，L11-18）——**无 TILING_DATA 宏，最简单的 Direct Invocation 数据传递方式**。
  - 计算：`RmsNormInvSqrtApprox`（rmsnorm_baseline_kernel.h:9-32）标量 Newton 迭代 rsqrt（归一化到 [0.5,2] + 6 次迭代）；主流程用 `GetValue/SetValue` 逐元素标量读写（L74-90）——**教学基线，性能极差，仅作语义/模板参考**。
- **build.sh（58 行）**：`detect_soc`（npu-smi 探测 SoC → ascend910b4 默认）→ `cmake -DSOC_VERSION=... -DASCEND_CANN_PACKAGE_PATH=...` → 产出 `out/bin/rmsnorm_baseline_standalone` 与 `out/lib/librmsnorm_torch_register.so`（torch extension 注册）。**这是官方训练营给出的 CANN 9.0 Direct Invocation 工程构建入口样板**。
- **差异**：无 Add 无 bias，逐元素标量；仅模板结构可参考，算法不可参考。
- **未验证**：notebook（01.01_rmsnorm_baseline.ipynb）与测试脚本未拉取（clone 超时，仅 raw 拉取 5 个源码文件）。

---

## 三、构建/测试入口汇总

| 仓库 | 构建入口 | 测试入口 |
|------|----------|----------|
| ops-nn | 顶层 `CMakeLists.txt`（opbuild.cmake 机制，可选 `WITH_CANN_PKG/SOURCE`）；算子目录 `norm/add_rms_norm/CMakeLists.txt`、`op_host/CMakeLists.txt` | `norm/add_rms_norm/tests/ut/op_host/`（test_add_rms_norm_tiling.cpp 1362 行、test_AddRmsNorm_infershape.cpp）、`tests/ut/op_kernel/`、`tests/ut/op_api/`；`examples/test_aclnn_add_rms_norm.cpp`（aclnn 两段式调用示例：GetWorkspaceSize + Execute，ND 格式） |
| ops-transformer | `bash build.sh --pkg --soc=ascend910b --ops=...`；`bash build.sh --ophost_test --opapi_test` | `mc2/matmul_all_reduce_add_rms_norm/tests/ut/{op_host,op_kernel,op_api}/`（test_matmul_all_reduce_add_rms_norm_tiling.cpp、test_add_rms_norm.h、test_aclnn_*.cpp） |
| cann-samples | `cmake -S . -B build -DNPU_ARCH=dav-3510 && cmake --build build --target rms_norm_quant_story` | 自带 main() 生成数据（gen_data.py）并对比 golden，可独立编译运行；`cannsim` 仿真（Ascend950） |
| cann-learning-hub | `scripts/build.sh`（cmake -DSOC_VERSION=ascend910b4 -DASCEND_CANN_PACKAGE_PATH） | `verify_kernel_launch/rmsnorm_baseline_standalone.cpp`（standalone 验证）+ `tests/compare_qwen_native.py`（与 Qwen 原生对比）+ torch extension |

**关键常量（910B 平台，多仓库一致）**：UB_FACTOR_B16=12288、UB_FACTOR_B32=10240、SPLIT_D 裁剪值 B16_CUTD=12096/B32_CUTD=9696、SMALL_REDUCE_NUM=2000、NUM_PER_REP_FP32=64、NUM_PER_BLK_FP32=8、block=32B、dataPerBlock(B16/BF16)=16、dataPerBlock(FP32)=8。

---

## 四、迁移建议小结（面向 AddRmsNormBias 实现）

### 可直接借鉴（推荐采用）

1. **Tiling 模式选择**（ops-nn add_rms_norm_tiling.cpp:370-424 原样逻辑）：numCol>12288 → SPLIT_D；blockFactor==1 → SINGLE_N；numColAlign≤2000 → MERGE_N；numCol==numColAlign 且 FP16 → MULTI_N；否则 NORMAL。tiling_data_t 字段用 ops-nn 的 20 字段版或 mc2 的 7 字段精简版均可（Direct Invocation 自行定义 struct 即可）。
2. **bias 融合**（add_rms_norm_quant_kernel.h:282-289）：`Cast(beta, CAST_NONE)` 到 FP32 后 `Add` 于 FP32 域，在 gamma 乘之后、Cast 回目标类型之前。**与首版 kernel.asc 的 ApplyAffine 一致**，官方实现验证了该融合位置。
3. **FP32 归约链路**（add_rms_norm.h Compute / rms_norm_base.h ReduceSumFP32）：`Mul 平方 → Muls(1/N) → Add 折叠部分和 + WholeReduceSum（或 BlockReduceSum）→ Adds(eps) → Sqrt → Div(1) → GetValue(0)`。注意官方 `Muls(avgFactor)` 在 reduce **之前**做（缩小动态范围），与首版 reduce 后再除 N 不同，可对照精度。
4. **两遍扫描 + D 分块**（add_rms_norm_split_d.h）：大 D 时第一遍算部分和（每 j 块 ReduceSumFP32ToBlock 到每行 8 个 float，跨块 BlockReduceSumFP32 累加），第二遍从 `xGm` 重读 x1+x2 计算输出；gamma/bias 按 j 块搬运。**尾块 `numCol % ubFactor` 处理直接解决了"D 非 32 倍数"约束**（官方保证余数 ≥16 元素或整除）。
5. **尾块搬运**：DataCopyPad（220/3003/3113 arch）或 DataCopyCustom 的 GetValue/SetValue 重排（910 系列）；或 cann-samples 的多行 `DataCopyPad blockCount=ubFactor, dstStride=(rAlign-r)*32/32`。
6. **多核切分**：按行 `blockFactor=ceil(numRow/numCore)`、尾核 blockTail，与首版 kernel.asc 相同；GetBlockNum 上限 `min(numCore, numRow)`。
7. **Direct Invocation 模板与构建**：learning-hub 训练营的 `kernel_do` + tiling 数组 + cmake(-DSOC_VERSION -DASCEND_CANN_PACKAGE_PATH) 为官方样板；cann-samples 的独立 main + gen_data.py 数据生成 + 精度对比为本地验证样板。
8. **性能优化路径**（后续阶段）：gamma 预加载 → 多核 → 双缓冲 → UB 利用率公式（calcMaxUbFactor）→ 多行批处理（ubFactor 行×对齐列）→ VF RegAPI（UpdateMask 循环后重赋值坑）→ 二分累加 + 迭代 rsqrt。

### 不可迁移（明确排除）

- **通信/多卡**：matmul_all_reduce_add_rms_norm 的 Hccl allreduce、`SyncAll`、`rcvCntGM` 流水协调、KERNEL_TYPE_MIX_AIC_1_2（单算子单卡场景不需要）。
- **量化路径**：AddRmsNormQuant 的 doScales/doZeroPoints/RoundFloat2Int8/SetDeqScale（输出非 int8）。
- **MoE 路由/combine、matmul 内核**（moe_distribute_combine_*、inplace_matmul_all_reduce_* 的通信与路由逻辑）。
- **教学标量实现**（learning-hub baseline 的 GetValue/SetValue 逐元素循环，仅模板可参考）。
- **Reg API 版本**（arch35/add_rms_norm_regbase*、rms_norm_regbase*、6_binary_sum 的 `__simd_vf__`）：CANN 9.0 新寄存器 API，可作为性能冲刺选项，但正确性风险高，不应作为首发实现。

### 与首版 kernel.asc 的对照结论

首版（按行切分 + 两遍扫描 + DataCopyPad 尾块 + ReduceSum(count) + GetValue 标量 rsqrt）与官方 NORMAL/SPLIT_D 结构一致，方向正确。可改进点：
- ReduceSum 的 workLocal 可复用输入 tensor 的 `ReinterpretCast<float>`（0_naive.asc:183），减少 4KB 独立 buffer；或改用官方 Add 折叠 + WholeReduceSum（对超长 D 更稳）。
- 官方在 reduce 前先 `Muls(avgFactor)`，首版是 reduce 后除 N——精度行为可能不同，需在真机对照。
- 官方 half 路径 gamma 在 half 域乘、bias 在 FP32 域加；首版全部 FP32 域——精度更高但多一次 Cast，可在精度验证后决定。
- 大 D（>12288）时首版两遍扫描每次只处理一个 tile_len 块，官方 SPLIT_D 用"每块部分和 + 跨块 BlockReduceSum"在块间只保留每行 8 个 float，中间态更省 UB。

---

## 五、本轮新增来源清单

| URL | 标题 | 仓库/版本 | 访问日期 | 用途 | 证据等级 | 状态 |
|-----|------|-----------|----------|------|----------|------|
| https://gitcode.com/cann/ops-nn | ops-nn（CANN 神经网络算子库） | gitcode cann/ops-nn，master `9b594837` | 2026-09-11 | add_rms_norm / rms_norm / add_rms_norm_quant(_v2) 源码 | A（官方仓库源码原文） | verified（本地 /tmp/ops-nn.tmp 完整 checkout） |
| https://gitcode.com/cann/ops-transformer | ops-transformer（MC2/MoE 组合算子库） | gitcode cann/ops-transformer，master `e7019c29` | 2026-09-11 | matmul_all_reduce_add_rms_norm 5 变体 + Host tiling | A（官方仓库源码原文） | verified（本地 /tmp/ops-transformer.eKIcVc） |
| https://gitcode.com/cann/cann-samples | cann-samples（算子性能实战样例） | gitcode cann/cann-samples，master `23c981c0` | 2026-09-11 | rms_norm_quant_story 7 步优化（0/2/5/6 已读） | A（官方仓库源码原文）/ B（官方样例） | verified（本地 /tmp/cann-samples.vc2a1d） |
| https://github.com/hicann/cann-learning-hub | cann-learning-hub（官方学习仓库，GitHub 镜像） | hicann/cann-learning-hub，master | 2026-09-11 | qwen_ops/01_rmsnorm_baseline 训练营（Direct Invocation 模板 + 构建） | B（官方教程源码） | partial（raw 拉取 5 个文件；clone 超时，notebook/测试未取） |
| https://github.com/hicann/cann-learning-hub/blob/master/blogs/operator/ascend950_rmsnormquant_optimization | ascend950_rmsnormquant_optimization 博客 | 同上（GitHub API 目录树确认，正文未拉取） | 2026-09-11 | RmsNormQuant 优化方法（与 cann-samples Story 同源） | B | partial（仅目录确认） |

**证据等级说明**：A=官方仓库源码原文（已读源码文件）；B=官方样例/测试/教程源码；C=社区转述（本轮未采用 C 级来源作结论依据；WebSearch 命中的 CSDN 训练营文章仅作背景，未引用其代码）。

---

## 六、验证状态声明

- **本轮所有"可迁移/可借鉴"结论均基于源码静态分析，未在真实 NPU 上编译、运行、精度或性能验证**（本机 macOS，无 CANN/NPU）。
- 依赖项提示：ops-nn/ops-transformer 内核依赖 `kernel_operator.h`、`basic_api/kernel_basic_intf.h`（CANN 9.0）、`platform_ascendc`、`register/tilingdata_base.h` 等 CANN 9.0 头文件；cann-samples 依赖 `platform/platform_ascendc.h` 与 `basic_api/reg_compute/kernel_reg_compute_utils.h`（VF RegAPI）。迁移时需在真实 CANN 9.0 环境验证接口签名（如 `WholeReduceSum`、`BlockReduceSum`、`BinaryRepeatParams`、`DataCopyPadExtParams`、`BroadCast` 指令可用性）。
- 待后续 Agent 在真机验证项：① SPLIT_D 尾块余数 ≥16 元素规则的 910B 行为；② half 域 Add 与 FP32 域 Add 的精度差异（官方 half 路径用 half 域 Add，首版用 FP32 域）；③ `Muls(avgFactor)` 前置 vs 后置除 N 的精度差异；④ bf16 路径 gamma 的 FP32 域乘法 vs half 域的 CAST_NONE/CAST_RINT 舍入差异。
