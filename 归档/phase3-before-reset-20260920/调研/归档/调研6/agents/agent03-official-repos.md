# Agent 3 调研报告：官方 Ascend 开源仓库中的 RMSNorm / AddRmsNorm 实现

> 调研范围：官方 Ascend 开源仓库中 RMSNorm / AddRmsNorm / RMSNormQuant / LayerNorm 系列的真实 Ascend C 源码。
> 调研日期：2026-09-12（访问日期统一记为 2026-09-12）。
> 本机环境：macOS，无 CANN 工具链、无 NPU。**本报告所有结论均来自对仓库源码的逐行阅读，未在任何 NPU 上编译/运行**（不声称编译或运行过任何 Ascend C 代码）。
> 题面约束（来自 Agent 1）：本题为 `AddRmsNormBias`，`y = x + residual`；`rms = sqrt(mean(y^2, dim=-1) + eps)`；`output = y / rms * gamma + bias`；dtype ∈ {fp16, bf16, fp32}；D ∈ [64, 32768]，可能不是 32 的倍数；判题为 **Direct Invocation 直调单文件 `kernel.asc`**，`extern "C" void run_kernel(...)` + `<<<blockNum, nullptr, stream>>>` 启动 `__global__ __vector__` 核函数，**没有 tiling 结构体、没有 Host 侧 shape 常量**。

---

## 1. 执行摘要（最重要的 8 条发现）

1. **四个目标仓库全部存活于 GitCode（`gitcode.com/cann/*`），而不是 GitHub 的 `Ascend/*`。** 用 `git ls-remote`/`raw.gitcode.com` 实测确认：`cann/ops-transformer`、`cann/ops-nn`、`cann/cann-samples`、`cann/cann-learning-hub` 均存在；而 GitHub 上 `Ascend/ops-transformer`、`Ascend/ops-nn`、`Ascend/cann-samples`、`Ascend/cann-learning-hub` 全部 404。GitCode 的 GitLab v4 API 被禁用，改用 git 协议克隆 + `raw.gitcode.com/{owner}/{repo}/raw/{branch}/{path}` 取文件。
2. **在官方仓库里找到了真正的 AddRmsNorm（残差融合版）Ascend C 源码**，集中在两处：(a) `cann/ops-transformer` 的 `mc2/matmul_all_reduce_add_rms_norm/op_kernel/`；(b) `cann/ops-nn` 的 `norm/add_rms_norm/op_kernel/`。两者都实现了 `y = x + residual` 后再做 RMSNorm 的融合算子，与本题语义的「残差相加」部分**一致**。
3. **所有官方 AddRmsNorm 都只有 `y/rms * gamma`，没有 `+ bias`。** 我逐文件 grep 确认：`add_rms_norm` 的 `bias` 字样全是 `gm_bias`（GM 偏移变量），并非输出偏置。本题的 `AddRmsNormBias`（`... * gamma + bias`）在官方仓库里**不存在**，需要我们自己把 bias 融合进输出阶段（在乘 gamma 之后再加 bias）。
4. **RMSNormQuant（量化版）与本题不等价，已显式甄别。** `ops-nn/norm/add_rms_norm_quant` 与 `cann-samples/.../rms_norm_quant_story` 带 `quant_scale/quant_offset`、输出 int8。**它末尾的 `Adds(offset)` 是量化偏移，不是本题的 bias**，绝不能当成等价实现。
5. **官方实现普遍依赖一个 `AddRMSNormTilingData` 结构体**（含 `num_row/num_col/block_factor/row_factor/ub_factor/epsilon/avg_factor`）从 Host 侧 tiling 传入。这与本题「没有 tiling 结构体」直接冲突——我们必须**在核函数内部用传入的 shape 与 `GetBlockNum()` 运行时推导**出 `blockFactor/rowFactor/ubFactor`。
6. **多核切分方式高度统一：按「行」切分（`GetBlockIdx()` 分块连续行），尾核拿余数行。** `blockFactor = ceil(numRow / blockNum)`，尾核 `rowWork = numRow - (blockNum-1)*blockFactor`。这是可迁移的核心范式。
7. **大 D 与非 32 对齐 D 的两种成熟处理范式已找到**：(a) ops-transformer 的 `split_d` 两遍法（先把 D 切成 `ubFactor` 块累加平方和，再统一开方后第二遍乘回）；(b) cann-samples `4_double_buffer.asc` 的 `__simd_vf__` 按 64 元素分块 + `UpdateMask` 掩码尾块法（天然处理非对齐 D）。两种都可直接借鉴。
8. **cann-samples 的 `*.asc` 样例与本题判题形态几乎一致**：`__global__ __aicore__ __vector__ void rms_norm_quant(...)` + `<<<blockNum, 0, stream>>>` 直调单文件。这是离「本题直调单文件 `kernel.asc`」最接近的真实样例，但其数值链是量化版；结构/多核/双缓冲写法可直接迁移，量化相关行需删除或替换。

---

## 2. 仓库矩阵表

| 仓库 | 相关路径 | commit/版本 | 是否含 AddRmsNorm/RMSNorm | 是否含 bias 融合 | 是否量化 | 有源码可摘录? | 证据等级 |
|---|---|---|---|---|---|---|---|
| `gitcode.com/cann/ops-transformer` | `mc2/matmul_all_reduce_add_rms_norm/op_kernel/`（`add_rms_norm*.h`、`reduce_common.h`、`rms_norm_base.h`、`add_rms_norm_kernel.h`、`*_tiling_data.h`） | 9.0.0 分支 `efca5d19e52b6c6fa3da44d962e92d91357fe983` | 含 AddRmsNorm（残差融合） | **否**（仅 gamma） | 否（但该目录下有 `*_quant` 变体） | 是（逐行） | A/B |
| `gitcode.com/cann/ops-nn` | `norm/add_rms_norm/op_kernel/`（`add_rms_norm.h`、`*_single_n.h`、`*_split_d.h`、`*_multi_n.h`、`*_merge_n.h`、`add_rms_norm.cpp`）；另有 `norm/rms_norm`（纯 RMSNorm）、`norm/add_rms_norm_quant`（量化） | 9.0.0 分支 `fcebf031d193d641d2d1472a539bcc387b1e5f09` | 含独立 AddRmsNorm（残差融合）+ 纯 RMSNorm | **否**（仅 gamma） | 否（另有 `*_quant` 量化变体） | 是（逐行） | A/B |
| `gitcode.com/cann/cann-samples` | `Samples/2_Performance/rms_norm_quant_story/src/*.asc`（`0_naive`~`6_binary_sum`）；`Samples/2_Performance/simd_vf_story/reduce/src/reduce_*.asc` | master `23c981c0918e3183958e94e58ef6989d44983230` | 含 RMSNormQuant（**无残差**）与 ReduceSum 原语样例 | **否** | **是**（quant，int8 输出） | 是（逐行 `.asc`） | A/B |
| `gitcode.com/cann/cann-learning-hub` | 未深挖；仅确认可达 | master `ff0e08e27655888c1bc8f6befd1956f39ca79346` | 未核实（旧名 `Ascend/samples`） | 未核实 | 未核实 | 仅可达，未深挖 | A |
| GitHub `Ascend/samples` | 仓库存在（旧名 cann-learning-hub） | master | 未深挖 | 未深挖 | 未深挖 | README 级 | A |
| GitHub `Ascend/ops-transformer`、`Ascend/ops-nn`、`Ascend/cann-samples`、`Ascend/cann-learning-hub` | — | — | — | — | — | **不存在（404）** | A（否定） |

> 证据等级：A = 官方仓库/官方 API；B = 官方样例·源码·测试·原作者资料；C = 社区文章/论坛；D = 仅搜索摘要/未核验。本报告最低到 B。

---

## 3. 逐个实现深度分析

### 3.1 `cann/ops-transformer`：`matmul_all_reduce_add_rms_norm`（残差融合 AddRmsNorm，4 策略）

**仓库/路径/版本**：`gitcode.com/cann/ops-transformer` @ 9.0.0 (`efca5d19`)。
**目录树相关路径**：
```
mc2/matmul_all_reduce_add_rms_norm/
├── op_kernel/
│   ├── add_rms_norm.h            // KernelAddRmsNorm（D 整体进 UB 的基线版）
│   ├── add_rms_norm_split_d.h    // KernelAddRmsNormSplitD（大 D 切 D 两遍）
│   ├── add_rms_norm_single_n.h   // KernelAddRmsNormSingleN（一行一核，整 D 进 UB）
│   ├── add_rms_norm_multi_n.h    // 多行合并版
│   ├── add_rms_norm_merge_n.h    // 多行合并版
│   ├── rms_norm_base.h           // ReduceSumCustom / DataCopyCustom / 常量
│   ├── reduce_common.h           // WholeReduceSum / 级联加 ReduceSumHalfInterval
│   ├── add_rms_norm_kernel.h     // 分发器（按 keyTile 选策略）
│   └── *_tiling_data.h           // AddRMSNormTilingData 结构体
```
> 注：`mc2/3rd/add_rms_norm`、`mc2/3rd/rms_norm` 只是薄封装/基类的引用，无独立算子体。

#### 3.1.1 关键源码摘录（基线 `add_rms_norm.h` 的核）

**多核切分（`Init` 内，`add_rms_norm.h:39-43`）**：
```cpp
if (GetBlockIdx() < numBlocks - 1) {
    this->rowWork = blockFactor;                 // 非尾核：整块 blockFactor 行
} else if (GetBlockIdx() == numBlocks - 1) {
    this->rowWork = numRow - (numBlocks - 1) * blockFactor;  // 尾核：余数行
}
// GM 起始按核偏移到连续行块
normOutGm.SetGlobalBuffer((__gm__ T*)normOut + GetBlockIdx() * blockFactor * numCol, rowWork * numCol);
```

**核内分行（`Process` 内，`add_rms_norm.h:95-101`）**——`rowFactor` 行一批，`i_o_max = CeilDiv(rowWork, rowFactor)`，末批 `row_tail`：
```cpp
uint32_t i_o_max = CeilDiv(rowWork, rowFactor);
uint32_t row_tail = rowWork - (i_o_max - 1) * rowFactor;
for (uint32_t i_o = 0; i_o < i_o_max - 1; i_o++) SubProcess(i_o, rowFactor, gammaLocal);
SubProcess(i_o_max - 1, row_tail, gammaLocal);
```

**残差相加（`CopyIn`，`add_rms_norm.h:121-153`）**——fp16 直接 `Add`，bf16 先 cast 到 fp32 再相加再 cast 回：
```cpp
// fp16 路径
Add(xLocal, x1Local, x2Local, numCol);          // x1=normOut, x2=residual
PipeBarrier<PIPE_V>();
Cast(x1_fp32, xLocal, RoundMode::CAST_NONE, numCol);   // 升 fp32 供后续归约
// bf16 路径：Cast(x1_fp32,x1Local); Cast(x2_fp32,x2Local); Add(x1_fp32,...); Cast(xLocal,x1_fp32,CAST_RINT)
DataCopyCustom<T>(yGm[gm_bias], x_out, numCol);  // 把 y=x+residual 先存到 yGm
```

**RMSNorm 数值链（`Compute`，`add_rms_norm.h:163-219`，fp16 重载节选）**：
```cpp
Mul(sqx, x_fp32, x_fp32, numCol);               // y^2
Muls(sqx, sqx, avgFactor, numCol);              // * (1/numCol)  -> mean
ReduceSumCustom(sqx, sqx, reduce_buf_local, numCol);  // sum over D
Adds(sqx, sqx, epsilon, 1);                     // + eps
Sqrt(sqx, sqx, 1);
Duplicate(reduce_buf_local, (float)1.0, 1);
Div(sqx, reduce_buf_local, sqx, 1);             // 1/rms
// V_S / S_V 同步后把标量 rstd 读回
float rstd_value = sqx.GetValue(0);             // 关键：标量 1/rms 读回
Muls(x_fp32, x_fp32, rstd_value, numCol);       // y * (1/rms)
Cast(yLocal, x_fp32, RoundMode::CAST_NONE, numCol);
Mul(yLocal, gammaLocal, yLocal, numCol);        // * gamma  -> 输出（无 +bias）
outQueueY.EnQue<half>(yLocal);
```

#### 3.1.2 大 D 处理：`add_rms_norm_split_d.h`（两遍法，尾块友好）
`split_d` 在 `Init` 里 UB 分配：`inQueueX` 给 `2 * ubFactor * sizeof(T)`（同时放 x1、x2），`rstdBuf(rowFactor*f32)`、`xFp32Buf/sqxBuf(ubFactor f32)`、`sumBuf(rowFactor*8 f32)`、`reduceFp32Buf(64 f32)`。
`Process` 把 D 切成 `j_max = CeilDiv(numCol, ubFactor)` 块，`col_tail = numCol - (j_max-1)*ubFactor` 处理末块；**先 Former 遍**逐 D 块算 `x^2` 的部分和（经 `ReduceSumFP32ToBlock`→`BlockReduceSum` 累加到每行 rstd），**再 `ComputeRstd`**（`+eps→sqrt→1/`），**后 Latter 遍**逐 D 块 `Muls(x, rstd)→Cast→Mul(gamma)` 写回。这样 D 可以任意大（含非对齐尾块）。

#### 3.1.3 分发器（`add_rms_norm_kernel.h`）——4 策略怎么选
`keyTile` 决定实现：`10=ADD_RMS_NORM(half)`、`30=BF16`、`11=SPLIT_D`、`31=SPLIT_D_BF16`、`12=MERGE_N`、`32=MERGE_N_BF16`、`13=SINGLE_N`、`33=SINGLE_N_BF16`、`14=MULTI_N`。`numBlocks` 来自 tiling key（`ARNNumBlocksTile`/`ARNNumBlocksTail`）。即**策略选择完全由 Host tiling 决定**。

#### 3.1.4 归约原语（`reduce_common.h` / `rms_norm_base.h`）
- `ReduceSumFP32`：用 `Add` 按 `NUM_PER_REP_FP32=64` 分 repeat 累加，再 `WholeReduceSum<float,false>` 收口。
- `ReduceSumHalfInterval`：对 `count>64` 用 `findPowerTwo` 做对数级级联 `Add` 再 `WholeReduceSum`，**专门处理大 D（可达 32768）归约**。
- `DataCopyCustom`（`rms_norm_base.h:132-169`）：dav-2201（`__CCE_AICORE__==220`）走 `DataCopyPad`（`blockLen = count*sizeof(T)`，需 32B 对齐）；非 220 路径对「非 32 字节整数倍」的尾块用 `GetValue/SetValue` 手工补齐——**这是处理非对齐 D 尾部的一个具体写法**。

#### 3.1.5 分块策略 / 多核 / 数值链 / 与本题差异
- **分块**：行向 `blockFactor` 行/核；核内 `rowFactor` 行/批；D 向 `ubFactor` 元素/块（`split_d` 才切 D）。UB 用 `SINGLE_BUFFER_NUM=1`（单缓冲，非双缓冲），fp16/bf16 额外开 `xFp32Buf` 放 fp32。
- **多核**：按行切，`GetBlockIdx()`，尾核拿余数行。✅ 可迁移。
- **数值链**：fp16/bf16 升 fp32 → 平方 → `*1/D` → ReduceSum → `+eps` → `sqrt` → `1/` → `GetValue(0)` 读回标量 → `*y` → `*gamma` → cast 回。✅ 可迁移（去掉/替换量化即可）。
- **与本题差异**：① **无 bias**（缺 ` + bias`，需自己加）；② **有 tiling 结构体** `AddRMSNormTilingData`（本题没有，需运行时推导）；③ 它是 `matmul+all_reduce+add_rms_norm` 融合算子的一部分，`ComputeProcess` 的 `addRmsNormCount/rcvCnt` 是融合场景参数（纯 AddRmsNorm 时只需跑一次）；④ `.cpp` 入口的 `biasGM` 是 **matmul 的 bias，不是 RMSNorm 输出的 bias**，勿混淆；⑤ 该 header 的 `Compute` 预置了 `xFp32Buf`（fp16/bf16 分支），**未显式覆盖 fp32 分支**（融合场景下 x 已为 fp32）；纯 fp32 直调用需自行补 cast 进 `xFp32Buf`。

---

### 3.2 `cann/ops-nn`：`norm/add_rms_norm`（独立 AddRmsNorm 算子，最干净）

**仓库/路径/版本**：`gitcode.com/cann/ops-nn` @ 9.0.0 (`fcebf031`)。这是**最贴近「单算子 AddRmsNorm」**的官方实现（不是融合算子）。
**目录树相关路径**：
```
norm/add_rms_norm/
├── op_kernel/
│   ├── add_rms_norm.h          // KernelAddRmsNorm<T, MODE>
│   ├── add_rms_norm_base.h
│   ├── add_rms_norm_single_n.h
│   ├── add_rms_norm_split_d.h
│   ├── add_rms_norm_multi_n.h
│   ├── add_rms_norm_merge_n.h
│   └── add_rms_norm.cpp        // extern "C" __global__ 核函数入口
├── op_host/  (tiling / infershape / def / op_api)
└── tests/    (ut / st / examples)
norm/rms_norm/            // 纯 RMSNorm（无 residual）——参考对照
norm/add_rms_norm_quant/  // 量化版（≠本题，见 §3.2.6）
```

#### 3.2.1 关键源码摘录（`add_rms_norm.h`）

**多核切分（用 `GetBlockNum()`，而非传入 numBlocks，`add_rms_norm.h:41-51`）**：
```cpp
blockIdx_ = GetBlockIdx();
if (blockIdx_ < GetBlockNum() - 1) this->rowWork = this->blockFactor;
else if (blockIdx_ == GetBlockNum() - 1) this->rowWork = this->numRow - (GetBlockNum()-1)*this->blockFactor;
x1Gm.SetGlobalBuffer((__gm__ T*)x1 + blockIdx_ * this->blockFactor * this->numCol, this->rowWork * this->numCol);
```

**残差相加 + 写回中间 y（`CopyIn`，`add_rms_norm.h:107-152`）**：fp16/bf16 升 fp32 相加、或 fp32 直接 `Add`；并在 `MODE==ADD_RMS_NORM_MODE||PRE_RMS_NORM_MODE` 时把 `y=x+residual` 写入 `xGm`（可选输出中间结果）：
```cpp
// fp32 分支（确认支持 fp32！）
Add(x1Local, x1Local, x2Local, numCol);
Adds(xLocal, x1Local, (float)0, numCol);
// fp16 分支：Add -> Cast(x1_fp32,...,CAST_NONE)
// bf16 分支：Cast 双升 fp32 -> Add -> Cast 回
if constexpr (MODE == ADD_RMS_NORM_MODE || MODE == PRE_RMS_NORM_MODE)
    DataCopyCustom<T>(xGm[gm_bias], x_out, numCol);   // 存 y = x+residual
```

**RMSNorm 数值链（`Compute`，`add_rms_norm.h:161-198`，fp32/half 重载节选）**：
```cpp
Mul(sqx, xLocal, xLocal, numCol);          // y^2  (xLocal 已为 fp32)
Muls(sqx, sqx, avgFactor, numCol);        // *1/D
ReduceSumCustom(sqx, sqx, reduce_buf_local, numCol);
Adds(sqx, sqx, epsilon, 1); Sqrt(sqx, sqx, 1);
Duplicate(reduce_buf_local, ONE, 1); Div(sqx, reduce_buf_local, sqx, 1);  // 1/rms
float rstdValue = sqx.GetValue(0);        // 标量读回
rstdLocal.SetValue(inner_progress, rstdValue);
Muls(yLocal, xLocal, rstdValue, numCol);  // * (1/rms)
Mul(yLocal, gammaLocal, yLocal, numCol);  // * gamma  -> 输出（同样无 +bias）
```
bf16 重载（`add_rms_norm.h:200-251`）多一次 `Cast(yLocal, x_fp32, CAST_RINT)` 与 `Cast(x_fp32, yLocal, CAST_NONE)` 的舍入往返，再 `Cast(sqx, gammaLocal)` → `Mul` → `Cast(..., CAST_RINT)`。

#### 3.2.2 分块 / 多核 / 数值链 / 与本题差异
- **与 ops-transformer 几乎同构**：行向 `blockFactor`、核内 `rowFactor`、D 向 `ubFactor`；`split_d/single_n/multi_n/merge_n` 四种策略同款。但它是**纯算子**，没有 matmul 融合噪声，更适合参考。
- **明确支持 fp32**（有 `else` 分支直接 `Add`），比 ops-transformer 的该 header 更完整。✅ 对本题 dtype∈{fp16,bf16,fp32} 友好。
- **差异同 §3.1.5①②③**：无 bias；依赖 `AddRMSNormTilingData` 结构体（见 `add_rms_norm_tiling.h`）；多核/数值链可迁移。
- **额外可迁移点**：它把 `rstd` 和中间 `x`（=y）作为可选 GM 输出（`ADD_RMS_NORM_MODE`），说明官方用「先算 rstd 存 GM、再乘 gamma」的两段式；若我们想省一次读写，可把 rstd 留在 UB 标量（如 §3.1 的 `GetValue(0)` 路线）即可。

#### 3.2.3 纯 RMSNorm 对照（`norm/rms_norm`）
`norm/rms_norm` 是**无残差**的 RMSNorm（只有 `x/rms*gamma`），可用于对照「残差融合」到底改了哪几行（即 `CopyIn` 里的 `Add(x, residual)` 与写入 `xGm`）。本题需要的是带 residual 的版本，故以 `add_rms_norm` 为主。

#### 3.2.4 量化版甄别（`norm/add_rms_norm_quant`）——**与本题不等价**
`norm/add_rms_norm_quant` / `add_rms_norm_quant_v2` / `multi_add_rms_norm_dynamic_quant` 带 `quant_scale/quant_offset`，输出 int8/int32。**其末尾的 `+offset` 是量化偏移，不是本题 bias**，语义不等价，仅作「量化形态」参考，**不可作为本题等价实现**。

---

### 3.3 `cann/cann-samples`：`rms_norm_quant_story`（直调 `.asc` 样例，形态最像本题）

**仓库/路径/版本**：`gitcode.com/cann/cann-samples` @ master (`23c981c`)。
**目录树**：`Samples/2_Performance/rms_norm_quant_story/src/*.asc`（`0_naive.asc`→`6_binary_sum.asc`），以及 `Story.md` 教程。另：`Samples/2_Performance/simd_vf_story/reduce/src/reduce_*.asc`（ReduceSum 原语）。

#### 3.3.1 关键源码摘录（`2_multi_core.asc`）

**直调单文件 + 多核（与本题判题形态一致！`2_multi_core.asc:236-243, 442-443`）**：
```cpp
template <typename DATA_TYPE, ...>
__global__ __aicore__ __vector__ void rms_norm_quant(
    __gm__ DATA_TYPE *x, __gm__ DATA_TYPE *gamma,
    __gm__ SCALE_TYPE *scale, __gm__ OFFSET_TYPE *offset,
    __gm__ OUTPUT_DTYPE *y, RmsnormQuantTilingData tiling) { ... }
// 启动：<<<blockNum, 0, stream>>>，blockNum 由 calcTiling 用 GetCoreNumAiv() 算得
rms_norm_quant<...><<<blockNum, 0, stream>>>(xDevice, gammaDevice, scaleDevice, offsetDevice, yDevice, tilingData);
```
> 注意：它把 `RmsnormQuantTilingData tiling` 作为**结构体按值传参**给核函数——这正是「官方有 tiling 结构体、本题没有」的活样本。本题需改成「裸参数 + 核内推导」。

**结构体与多核尾核（`2_multi_core.asc:64-70, 121-127`）**：
```cpp
struct RmsnormQuantTilingData { int64_t a, r, blockFactor, blockTail; float epsilon; };
// Init 中：
if (blockIdx_ == GetBlockNum() - 1) curblockFactor_ = tilingData_->blockTail;
else curblockFactor_ = tilingData_->blockFactor;
xGm_.SetGlobalBuffer(x + blockIdx_ * tilingData_->blockFactor * tilingData_->r, ...);
```

**RMSNorm 数值链（`Compute`，`2_multi_core.asc:182-214`，注意是量化版）**：
```cpp
Cast(xLocalTensor, xInLocalTensor, CAST_NONE, r);
Mul(rmsLocalTensor, xLocalTensor, xLocalTensor, r);          // x^2
AscendC::ReduceSum(reduceLocalTensor, rmsLocalTensor, ..., r); // 高层 ReduceSum
Duplicate(rmsLocalTensor, reduceLocalTensor, r);
Muls(rmsLocalTensor, rmsLocalTensor, rInv_, r);              // *1/r
Adds(rmsLocalTensor, rmsLocalTensor, epsilon, r); Sqrt(...);
Div(xLocalTensor, xLocalTensor, rmsLocalTensor, r);          // x/rms
Mul(rmsLocalTensor, xLocalTensor, gammaLocalTensor, r);      // *gamma
Muls(rmsLocalTensor, rmsLocalTensor, scale_, r);             // *quant_scale  <-- 量化
Adds(rmsLocalTensor, rmsLocalTensor, offset_, r);            // +quant_offset <-- 量化（非本题 bias）
Cast(... -> half -> int8);                                   // 量化输出
```

#### 3.3.2 双缓冲 + VF 处理非对齐 D（`4_double_buffer.asc`）
`BUF_NUM = 2`（双缓冲）；`ComputeRmsVf` 用 `__simd_vf__` + `RegTensor`，以 `VL_B32_SIZE = 256/sizeof(float) = 64` 为块：
```cpp
uint16_t vfLoopRNum = CeilDiv(r, VL_B32_SIZE);
for (i=0; i<vfLoopRNum; i++) {
    preg = AscendC::Reg::UpdateMask<float>(rLocal);   // 每块的掩码；末块自动只剩余数元素
    Reg::DataCopy<DATA_TYPE, DIST_UNPACK_B16>(vregXIn, xInAddr + i*VL_B32_SIZE);
    Reg::Cast<float,DATA_TYPE>(vregX, vregXIn, preg);
    Reg::Mul(vregXQuared, vregX, vregX, preg);
    Reg::Add(vregReduceSum, vregReduceSum, vregXQuared, pregAll);  // 跨块累加平方和
}
Reg::ReduceSum(vregReduceSum, vregReduceSum, preg);     // 收口
Reg::Muls/Adds/Sqrt(...);                              // 1/rms
```
**这是处理「D 不是 32 倍数、甚至任意值」最干净的模式**：按 64 元素分块、用 `UpdateMask` 让末块只覆盖剩余元素，无需显式 `split_d` 两遍，也规避了 `DataCopyPad` 的 32B 对齐限制。强烈建议本题 D 向分块借鉴此写法。

#### 3.3.3 分块 / 多核 / 数值链 / 与本题差异
- **形态**：`__global__ __aicore__ __vector__` + `<<<blockNum,0,stream>>>` 直调单文件 `.asc`——**与本题判题形态几乎一致**（硬性差异：本题核名 `run_kernel`、参数裸传、无 tiling 结构体、非量化）。✅ 形态可迁移。
- **多核**：按行切 + 尾核拿 `blockTail`，同前。✅ 可迁移。
- **数值链**：与官方一致（升 fp32→平方→`*1/r`→ReduceSum→`+eps`→`sqrt`→`1/`→`*gamma`），但**多了 `Muls(scale)` + `Adds(offset)` 两步量化，且输出 int8**。**这两步是量化、不是 bias**，必须删除/替换；同时它**没有残差相加**（输入只有 `x`，无 `residual`），所以它是 RMSNormQuant 而非 AddRmsNorm。
- **可迁移**：结构骨架、多核、`AlignBytes`/`DataCopyPad`/`ReduceSum` 调用方式、`calcTiling` 思路；**不可直接迁移**：量化四行、无 residual、`RmsnormQuantTilingData` 结构体、int8 输出。

---

### 3.4 `cann/cann-samples`：`simd_vf_story/reduce`（ReduceSum 原语样例）

**路径**：`Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ra_baseline.asc`（另有 `ar/ra`、`baseline/pragma/unroll/binary` 共 16 个变体）。
**关键摘录（`reduce_sum_ra_baseline.asc:25-45, 111-116`）**：
```cpp
__simd_vf__ inline void ReduceSumRaVf(__ubuf__ float* xAddr, __ubuf__ float* zAddr, uint32_t d0, uint32_t d1, uint32_t d1Pad) {
    Reg::RegTensor<float> accReg, inReg;
    uint32_t remainCols = d1;                       // 剩余列数
    const uint16_t colChunks = (d1 + VL_B32 - 1) / VL_B32;
    for (uint16_t c = 0; c < colChunks; c++) {
        Reg::MaskReg mask = Reg::UpdateMask<float>(remainCols);  // 本块有效列（末块=余数）
        Reg::Duplicate(accReg, 0.0f);
        for (uint16_t i = 0; i < d0; i++) {         // 跨行累加
            Reg::LoadAlign<float, DIST_NORM>(inReg, xAddr + i*d1Pad + c*VL_B32);
            Reg::Add(accReg, accReg, inReg, mask);
        }
        Reg::StoreAlign<float, DIST_NORM>(zAddr + c*VL_B32, accReg, mask);
    }
}
__global__ __aicore__ __vector__ void ReduceSumKernel(GM_ADDR x, GM_ADDR z, uint32_t d0, uint32_t d1) { ... }
ReduceSumKernel<<<1, nullptr, stream>>>(dX, dZ, D0, D1);  // 注释标注 D1=250 且 %8≠0，专门覆盖尾块
```
**与本题关系**：这是**归约原语**的标准教学样例，演示了「沿最后一维归约 + 非对齐尾块（`UpdateMask` 掩码）+ `DataCopyPad`/`DataCopyExtParams`」的完整写法。本题沿最后一维 D 归约可直接套用：把 `d0` 换成 1（单行）、`d1` 换成 D，或复用 §3.3.2 的单行 VF 归约。证据等级 B（官方样例源码）。

---

### 3.5 `cann/cann-learning-hub` / GitHub `Ascend/samples`（可达，未深挖）

- `gitcode.com/cann/cann-learning-hub` @ master (`ff0e08e2…`) 经 `git ls-remote` 确认存在、可达（与 GitHub `Ascend/samples` 同源，后者是旧名、在 GitHub 上确实存在）。因 GitCode GitLab v4 API 被禁用、且本轮已从甲、乙两份 ops 仓库 + cann-samples 拿到足量逐行源码，本仓库**仅确认可达、未做深读**，列为「未深挖」。若后续需要 cannon-learning-hub 内的单算子样例（如经典 ReduceSum/DataCopyPad 教学样例），可再用 `git clone --filter=blob:none` 拉取 `samples/operator/` 子树。

---

## 4. 官方实现的共性做法（提炼）

1. **多核 = 按行切分（GetBlockIdx 分连续行块），尾核拿余数行。** 四套实现（ops-transformer / ops-nn / cann-samples 两份）全部如此：`blockFactor = ceil(numRow / blockNum)`，非尾核 `rowWork=blockFactor`，尾核 `rowWork = numRow - (blockNum-1)*blockFactor`。这是本题**最值得直接照搬**的范式。
2. **核内再分行：`rowFactor` 行/批，`i_o_max=CeilDiv(rowWork, rowFactor)`，末批 `row_tail` 处理行向尾块。**
3. **残差融合 = CopyIn 阶段做 `Add(x, residual)`**（fp16 直接 Add；bf16 先双升 fp32 相加再 cast 回；fp32 直接 Add）。ops-transformer/ops-nn 的 `add_rms_norm` 已融合 residual，与本题 `y=x+residual` 一致。
4. **统一数值链（fp32 为主战场）**：`cast fp32 → x^2 → *(1/D) → ReduceSum → +eps → sqrt → 1/ → *x → *gamma`。rstd 是标量，经 `GetValue(0)`（或 VF 的 `ReduceSum` 收口）读回后乘以整行。
5. **归约两种成熟实现**：(a) 高层 `AscendC::ReduceSum`（样例用）；(b) 自定义 `WholeReduceSum`/`BlockReduceSum` + 掩码（ops 仓库用，支持到 255×64=16320；更大 D 用级联加 `ReduceSumHalfInterval`）。
6. **大 D / 非对齐 D 两种尾块处理**：(a) `split_d` 两遍（D 切 `ubFactor` 块，先累加平方和再统一开方后第二遍乘回）；(b) VF 按 64 元素分块 + `UpdateMask` 掩码（cann-samples `4_double_buffer.asc`/`reduce_*.asc`）——后者对非 32 倍数 D 最友好。
7. **UB 分配惯例**：输入/gamma/输出各一个 `TQue`（深度 `SINGLE_BUFFER_NUM=1` 或双缓冲 `BUF_NUM=2`）；fp16/bf16 额外开一块 fp32 UB（`xFp32Buf`）放升精度后的数据；归约临时缓冲 `reduceFp32Buf` 仅 `NUM_PER_REP_FP32=64` 个 float。
8. **tiling 全部在 Host 侧算好，结构体传入核函数**（`AddRMSNormTilingData{num_row,num_col,block_factor,row_factor,ub_factor,epsilon,avg_factor}`）。这是与本题「无 tiling 结构体」的根本冲突点。

---

## 5. 本题不能直接照搬的地方（必须运行时化 / 自己融合）

1. **没有 bias —— 必须自己加。** 所有官方 `add_rms_norm` 都是 `output = y/rms * gamma`，**没有 `+ bias`**。RMSNormQuant 里的 `Adds(offset)` 是**量化偏移**，语义≠本题 bias，绝不能当成等价。本题需在 `* gamma` 之后再加一步 `Add(output, biasLocal)`（bias 沿最后一维广播，dtype 与 gamma/output 一致，fp16/bf16 同理在 fp32 域或原位相加）。
2. **没有 tiling 结构体 —— 必须在核内运行时推导。** 官方依赖 `AddRMSNormTilingData`；本题是直调单文件、无 Host 常量。需在 `run_kernel` 里根据 `numRow/numCol` 与 `GetBlockNum()`（`blockNum` 由 `<<<blockNum,...>>>` 给出）运行时算 `blockFactor=ceil(numRow/blockNum)`、`rowFactor`、`ubFactor`（按 UB 容量与 D 推导，或直接取 `min(D, ubCap)`）。注意 `GetBlockNum()` 在 `__global__ __vector__` 直调下返回 `<<<>>>` 的 `blockNum`。
3. **D ∈ [64,32768] 且可能非 32 倍数——要显式处理尾块。** 建议两种之一：(a) 借鉴 `split_d` 两遍（D>ubFactor 时切块、累加平方和、再乘回）；(b) 更直接地借鉴 cann-samples VF `UpdateMask` 分块（按 64 元素循环、末块掩码覆盖余数）。`DataCopyPad` 的 `blockLen` 需 32B 对齐，非对齐 D 的整行搬运要走官方 `DataCopyCustom` 的非 220 兜底（GetValue/SetValue）或 VF 掩码搬运，不能直接裸 `DataCopy`。
4. **判题形态差异（强制对齐）**：本题核名 `run_kernel`、参数裸传（`x, residual, gamma, bias, output, ...` + shape）、`<<<blockNum, nullptr, stream>>>`、单文件 `kernel.asc`；官方样例多是 `rms_norm_quant(...)` + 结构体传参 + 内嵌 `main()`。迁移时只取算子体（`class`/`__global__` 函数与 `Compute/CopyIn/CopyOut`），外壳按题面重写。
5. **量化版不可直接用**：删掉 `scale/offset` 两行与 int8 输出；保留 `cast→平方→均值→ReduceSum→+eps→sqrt→1/→*x→*gamma→+bias`。
6. **dtype 覆盖**：本题要 fp16/bf16/fp32 三态。ops-nn `add_rms_norm` 已有 fp32 分支可借鉴；bf16 走「双升 fp32 相加 + CAST_RINT 往返」路线；fp16 走「直接 Add + CAST_NONE」路线。建议算子体用模板/分支统一处理三态。
7. **无 matmul 融合、无 `addRmsNormCount/rcvCnt`**：官方 `matmul_all_reduce_add_rms_norm` 是融合算子，其 `ComputeProcess` 的循环参数属融合场景；纯 AddRmsNormBias 只需对当前核的 `rowWork` 行做一次 `Process`。

---

## 6. 未找到 / 访问受限清单

- **GitHub 上 `Ascend/ops-transformer`、`Ascend/ops-nn`、`Ascend/cann-samples`、`Ascend/cann-learning-hub` 均不存在（404）**；目标仓库实际在 `gitcode.com/cann/*`。
- **GitCode 的 GitLab v4 REST API 被禁用**（`/api/v4/projects/...` 返回 `NOT_PATH`），无法直接列目录树；改用 `git clone --depth 1 --filter=blob:none` + `raw.gitcode.com/{owner}/{repo}/raw/{branch}/{path}` 取文件。
- **`cann/cann-learning-hub`、`GitHub Ascend/samples` 仅确认可达、未深读**（本轮已从 ops-transformer、ops-nn、cann-samples 取得足量逐行源码，满足验收下限；列为未深挖而非未找到）。
- **未找到任何官方 `AddRmsNormBias`（输出带 bias）实现**——所有 `add_rms_norm` 仅含 gamma；`add_rms_norm_quant` 的 offset 是量化偏移。**结论：bias 融合需自行实现。**
- **未找到 LayerNorm 系列与本题直接相关实现**（ops 仓库以 RMSNorm 为主；LayerNorm 多为另一算子族，且本题为 RMSNorm 语义，不在本次强制检索范围）。

---

## 7. 来源清单

```
- [S061] 仓库 gitcode.com/cann/ops-transformer（官方镜像，含 AddRmsNorm 源码） | https://gitcode.com/cann/ops-transformer | 分支 9.0.0 commit efca5d19e52b6c6fa3da44d962e92d91357fe983 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：定位 AddRmsNorm 真实源码 | 可支持结论：官方 AddRmsNorm 存在于 mc2/matmul_all_reduce_add_rms_norm/op_kernel/
- [S062] 文件 ops-transformer mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm.h（KernelAddRmsNorm 基线实现） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：逐行摘录多核切分/残差相加/RMSNorm 数值链 | 可支持结论：多核按行、残差 Add、fp32 升精度、1/rms 经 GetValue(0) 读回、仅 *gamma 无 bias
- [S063] 文件 ops-transformer .../op_kernel/add_rms_norm_split_d.h（大 D 两遍法） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_split_d.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：大 D/非对齐 D 的 split_d 两遍处理 | 可支持结论：D 切 ubFactor 块、Former 累加平方和、Latter 乘回，支持 D 达 32768 与尾块
- [S064] 文件 ops-transformer .../op_kernel/add_rms_norm_single_n.h（一行一核整 D 进 UB） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_single_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：单行单核分块基线 | 可支持结论：SINGLE_N_BUFFER_SIZE=(192-1)*1024 字节，整 D 进 UB，D 受限时回退
- [S065] 文件 ops-transformer .../op_kernel/reduce_common.h（归约原语） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/reduce_common.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：WholeReduceSum / 级联加 ReduceSumHalfInterval | 可支持结论：大 D 归约用 findPowerTwo 级联 Add + WholeReduceSum
- [S066] 文件 ops-transformer .../op_kernel/rms_norm_base.h（ReduceSumCustom/DataCopyCustom） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/rms_norm_base.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：归约与搬运封装、非对齐尾块兜底 | 可支持结论：dav-2201 走 DataCopyPad（需 32B 对齐）；非 220 用 GetValue/SetValue 处理尾部
- [S067] 文件 ops-transformer .../op_kernel/add_rms_norm_kernel.h（分发器） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_kernel.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：keyTile 选 4 策略、numBlocks 来源 | 可支持结论：策略由 Host tiling key 决定（10/11/12/13/14 +BF16）
- [S068] 文件 ops-transformer .../op_kernel/matmul_all_reduce_add_rms_norm_tiling_data.h（AddRMSNormTilingData 结构体） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/matmul_all_reduce_add_rms_norm_tiling_data.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认 tiling 结构体字段 | 可支持结论：含 num_row/num_col/block_factor/row_factor/ub_factor/epsilon/avg_factor（本题没有，需运行时推导）
- [S069] 仓库 gitcode.com/cann/ops-nn（独立 AddRmsNorm 算子来源） | https://gitcode.com/cann/ops-nn | 分支 9.0.0 commit fcebf031d193d641d2d1472a539bcc387b1e5f09 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：独立 add_rms_norm 算子 | 可支持结论：norm/add_rms_norm 为纯净单算子实现
- [S070] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm.h（KernelAddRmsNorm<T,MODE>） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm.h | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：逐行摘录（多核 GetBlockNum/残差/数值链/fp32 分支） | 可支持结论：支持 fp32（有 else 分支直接 Add）；无 bias；多核按行+尾核余数
- [S071] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm_split_d.h | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm_split_d.h | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：独立算子的 split_d 大 D 处理 | 可支持结论：与 ops-transformer 同构的两遍法
- [S072] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm.cpp（extern "C" 核入口） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm.cpp | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认 __global__ 入口形态 | 可支持结论：标准 msopgen 单算子入口，含 tiling 结构体注册
- [S073] 仓库 gitcode.com/cann/cann-samples（直调 .asc 样例来源） | https://gitcode.com/cann/cann-samples | 分支 master commit 23c981c0918e3183958e94e58ef6989d44983230 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：直调单文件 .asc 样例 | 可支持结论：含 rms_norm_quant_story 与 reduce 原语 .asc
- [S074] 文件 cann-samples Samples/2_Performance/rms_norm_quant_story/src/2_multi_core.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/src/2_multi_core.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：直调形态 + 多核 + RMSNorm 数值链（量化） | 可支持结论：__global__ __aicore__ __vector__ + <<<blockNum,0,stream>>>；量化版（scale/offset/int8），无 residual、无 bias
- [S075] 文件 cann-samples .../rms_norm_quant_story/src/4_double_buffer.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/src/4_double_buffer.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：双缓冲 + VF 按 64 元素分块处理非对齐 D | 可支持结论：UpdateMask 掩码尾块法，天然处理 D 非 32 倍数
- [S076] 文件 cann-samples .../rms_norm_quant_story/Story.md（教学文档） | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/Story.md | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：naive→multi_core→double_buffer→binary_sum 优化脉络 | 可支持结论：官方给出的 AddRmsNorm 多核/双缓冲优化路线图
- [S077] 文件 cann-samples Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ra_baseline.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ra_baseline.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：ReduceSum 原语 + 非对齐尾块（D1=250 %8≠0） | 可支持结论：VF + UpdateMask 沿最后一维归约的标准写法，可套用于本题
- [S078] 仓库 gitcode.com/cann/cann-learning-hub（旧名 Ascend/samples） | https://gitcode.com/cann/cann-learning-hub | 分支 master commit ff0e08e27655888c1bc8f6befd1956f39ca79346 | 访问 2026-09-12 | 等级 A | 状态 partial | 用途：候选补充单算子样例库 | 可支持结论：确认可达，未深挖（本轮已从 S061-S077 取得足量逐行源码）
- [S079] 仓库 GitHub Ascend/samples（cann-learning-hub 旧名，GitHub 侧存在） | https://github.com/Ascend/samples | master | 访问 2026-09-12 | 等级 A | 状态 partial | 用途：对照 GitHub 侧可达性 | 可支持结论：GitHub 上仅 samples 存在，ops-transformer/ops-nn/cann-samples/cann-learning-hub 在 GitHub 均 404
- [S080] 目录 ops-nn norm/rms_norm（纯 RMSNorm，无 residual，对照用） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/rms_norm/op_kernel/ | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：对照「残差融合」差异 | 可支持结论：纯 RMSNorm 无 Add(residual)，本题需 residual 故主取 add_rms_norm
- [S081] 目录 ops-nn norm/add_rms_norm_quant（量化版，甄别非等价） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm_quant/op_kernel/ | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 contradicted | 用途：显式甄别「量化≠本题 bias」 | 可支持结论：带 quant_scale/quant_offset、输出 int8，末尾 offset 是量化偏移非本题 bias，不可作等价实现
- [S082] 目录 ops-transformer mc2/3rd/add_rms_norm（薄封装/引用） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/3rd/add_rms_norm/ | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认独立 add_rms_norm 引用位置 | 可支持结论：仅为 tiling 头引用，算子体在 matmul_all_reduce_add_rms_norm/op_kernel
- [S083] 文件 ops-transformer .../op_kernel/add_rms_norm_multi_n.h（多行合并策略） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_multi_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：多 N 合并分块策略参考 | 可支持结论：当行数多/UB 富余时可多行一批提升利用率
- [S084] 文件 ops-transformer .../op_kernel/add_rms_norm_merge_n.h（多行合并策略） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_merge_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：多 N 合并分块策略参考 | 可支持结论：与 multi_n 同族，提供多行合并的另一种 UB 布局
- [S085] 文件 cann-samples .../simd_vf_story/reduce/src/reduce_sum_ar_baseline.asc（axis=1 归约原语） | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ar_baseline.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：沿最后一维（axis=1）归约标准写法 | 可支持结论：与本题「沿最后一维 D 归约」直接对应，可作为归约骨架
```

---

> 备注（合规声明）：本报告全部基于对上述官方仓库源码的离线阅读，本机无 CANN/NPU，**未编译、未运行任何 Ascend C 代码**，亦未修改 `源码/`、`提交/` 下任何文件；输出仅本文件 `agent03-official-repos.md`。RMSNormQuant 与其量化偏移被显式甄别为「与本题 `AddRmsNormBias` 不等价」，未误判为可用实现。
