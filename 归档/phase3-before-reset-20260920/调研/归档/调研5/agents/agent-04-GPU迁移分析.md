# Agent 4 调研报告：GPU（CUDA / Triton / PyTorch）实现迁移分析（AddRmsNormBias）

- 任务：纯调研（本机 macOS 无 NPU，不做任何编译、精度或性能验证）
- 目标题目：AddRmsNormBias（2026 CANN 挑战赛西南赛区，CANN 9.0.0，SoC `dav-2201`，vector 核函数直调单文件 `kernel.asc`）
- 报告日期：2026-09-12
- 证据分级：A=官方文档/官方仓库页面；B=官方源码/官方样例/官方教程（本报告以源码级为主）；C=社区文章/论坛/个人仓库；D=仅搜索摘要未核验
- 边界声明：本文所有"迁移结论"均为 GPU 源码静态分析推论，**未经过任何 CANN 编译或 NPU 验证**；Ascend 侧 API 事实引用前序 Agent（2/3/7）已核对的结论，不重复调研。

---

## 1 调研范围与平台覆盖

| 平台/仓库 | 覆盖方式 | 源码级文件 | 状态 |
|---|---|---|---|
| PyTorch aten（CUDA/ROCm） | 源码全文读取 | `aten/src/ATen/native/cuda/layer_norm_kernel.cu`（2105 行）+ `block_reduce.cuh` | ✅ verified |
| vLLM | 源码全文读取 | `csrc/libtorch_stable/layernorm_kernels.cu`（373 行）+ `quantization/vectorization_utils.cuh` + `type_convert.cuh` | ✅ verified |
| NVIDIA TransformerEngine | 源码全文读取 | `transformer_engine/common/normalization/rmsnorm/` 四个文件 + `common/utils.cuh`（Stats/Reducer）+ `normalization/kernel_traits.h` | ✅ verified |
| flashinfer | 源码全文读取 | `include/flashinfer/norm.cuh`（RMSNormKernel + FusedAddRMSNormKernel） | ✅ verified |
| Triton 官方教程 | 源码全文读取 | `python/tutorials/05-layer-norm.py`（381 行） | ✅ verified |
| liger-kernel | 源码全文读取 | `src/liger_kernel/ops/rms_norm.py`（693 行） | ✅ verified |
| unsloth | 源码读取（Liger 引用的固定 commit） | `unsloth/kernels/rms_layernorm.py` | ✅ verified |
| NVIDIA Developer Forums / GPU MODE / HF Forums / ROCm 社区 | 搜索级覆盖（经 vLLM issue/PR、PyTorch PR 间接获得） | 无独立源码 | ⚠️ partial（见 §6） |

说明：任务书要求 ≥4 个源码级参考实现，本报告实际交付 **7 个**（PyTorch aten、vLLM、TransformerEngine、flashinfer、Triton 教程、Liger、unsloth），全部逐行读取原文件，非博客转述。所有源码均为 2026-09-11/12 抓取的 main 分支 HEAD（commit 见 §7 来源登记表）。

语义对标：本题计算 `y = x + residual`；`rms = sqrt(mean(y², dim=-1) + eps)`；`output = y/rms*gamma + bias`。与 vLLM `fused_add_rms_norm`、flashinfer `FusedAddRMSNorm` 的前向语义（先加残差再归一化）**同族**，但有两处差异需注意（详见 §2.2、§2.4 源码卡片中的语义辨析框）：
1. vLLM/flashinfer 把 `y` 原地写回 residual、把归一化结果覆写 input（推理引擎省显存）；本题 `output` 是独立输出张量，`y` 只需留在片上中间存储，**不需要双写回**。
2. flashinfer 的 `weight_bias` 是**乘性**偏置（`(weight_bias + w)` 参与乘法，为 Gemma 的 1+γ 设计）；本题 `bias` 是**加性**项（`y/rms*gamma + bias`），不能照搬该参数的用法。

---

## 2 参考实现源码卡片

### 2.1 PyTorch aten：`RmsNormKernelImpl`（两代路径：向量化单核 + 通用两核两遍）

仓库：`pytorch/pytorch`，文件：`aten/src/ATen/native/cuda/layer_norm_kernel.cu`，main @ `8671f09631c5`（2026-09-11）。

入口分发（L1234-1254）：`RmsNormKernelImpl` 只是 `LayerNormKernelImplInternal<T, T_ACC, /*rms_norm=*/true>` 的薄封装，复用 LayerNorm 全部机制；`T_ACC = acc_type<scalar_t, true>`，即 half/bf16 输入的累加类型一律 **FP32**。

```cpp
// L1181-1184：向量化快路径的条件——dtype ∈ {float, half, bf16}、N ≤ 2^24、
// N % vec_size == 0（vec_size=4）、且 X/Y/gamma/beta 指针全部 16B 对齐
if ((std::is_same_v<T, float> || std::is_same_v<T, at::Half> || std::is_same_v<T, at::BFloat16>) &&
N <= static_cast<int64_t>(1ULL << std::numeric_limits<float>::digits) && N % num_vec_elems == 0 &&
can_vec_X && can_vec_Y && can_vec_gamma && can_vec_beta) {
  launch_vectorized_layer_norm_kernel<T, T_ACC, rms_norm>(...);
} else { /* 两核两遍慢路径 */ }
```

关键点 1——**RMSNorm 下 Welford 退化为平方和累加**（L172-170 附近）：

```cpp
// cuWelfordOnlineSum 的 rms_norm 分支：不维护 mean/count，只累加 sigma2 += val*val
} else{
  return {0.f, curr_sum.sigma2 + val * val, 0};
}
```

关键点 2——**warp shuffle + 共享内存两级归约**（compute_stats，L220-264）：

```cpp
for (int i = thrx; i < n_vec_to_read; i += numx) {     // 向量化读取（4 元素/线程/步）
  vec_t data = X_vec[i];
  #pragma unroll
  for (int ii=0; ii < vec_size; ii++)
    wd = cuWelfordOnlineSum<acc_t, rms_norm>(static_cast<acc_t>(data.val[ii]), wd);
}
// intra-warp reduction：蝴蝶降档 shuffle
for (int offset = (C10_WARP_SIZE >> 1); offset > 0; offset >>= 1) {
  WelfordDataLN wdB{WARP_SHFL_DOWN(wd.mean, offset), WARP_SHFL_DOWN(wd.sigma2, offset),
                    WARP_SHFL_DOWN(wd.count, offset)};
  wd = cuWelfordCombine<rms_norm>(wd, wdB);
}
// inter-warp：blockDim.y 个 warp 经共享内存合并（偶数轮写入/奇数轮合并 + __syncthreads）
```

关键点 3——**单核内两遍扫描**：第一遍 compute_stats 得 rstd，第二遍**不重读全局内存判断直接重读 X_vec**（L299-312，注意：PyTorch 快路径第二遍是重新 load `X_vec[i]`，寄存器只保留统计量）：

```cpp
// L296-312（rms_norm 分支）
T_ACC rstd_val = c10::cuda::compat::rsqrt(wd.sigma2 + eps);
for (int i = thrx; i < n_vec_to_read; i += numx) {
  vec_t data = X_vec[i];
  vec_t out;
  ...
  out.val[ii] = static_cast<T_ACC>(gamma_vec[i].val[ii]) * (rstd_val * static_cast<T_ACC>(data.val[ii]));
```

关键点 4——**网格配置**（L1102-1105）：`threads(32, 16)` = 512 线程/块（2D block：x=lane，y=warp），`blocks(M)` 每行一块。慢路径（`RowwiseMomentsCUDAKernel` + `LayerNormForwardCUDAKernel`）是**两个核**分别算统计和归一化；ROCm 分支额外做 grid-stride 行循环规避 32 位网格上限（L1187-1198）。

配套文件 `aten/src/ATen/native/cuda/block_reduce.cuh`（同 commit）：`WarpReduceSum`（`WARP_SHFL_DOWN` 蝴蝶）→ `BlockReduceSum`（warp 间经 `shared[wid]` + 首个 warp 二次 shuffle），以及泛型 `BlockReduce(val, op, identity, shared)`——这就是任务书要求覆盖的"warp shuffle / block / shared memory 两级归约"的标准实现。

---

### 2.2 vLLM：`rms_norm_kernel` 与 `fused_add_rms_norm_kernel`（语义最接近本题）

仓库：`vllm-project/vllm`，文件：`csrc/libtorch_stable/layernorm_kernels.cu`（**注意：旧路径 `csrc/layernorm_kernels.cu` 已迁移**，issue #43390 仍按旧路径引用行号），main @ `0c1e89ceb92b`（2026-09-11）。

融合核（FP16/BF16 特化版，L106-174）——**与本题 y=x+residual 语义一致的标杆实现**：

```cpp
template <typename scalar_t, int width, bool HasWeight>
__global__ std::enable_if_t<(width > 0) && _typeConvert<scalar_t>::exists>
fused_add_rms_norm_kernel(scalar_t* __restrict__ input, const int64_t input_stride,
    scalar_t* __restrict__ residual, const scalar_t* __restrict__ weight,
    const float epsilon, const int num_tokens, const int hidden_size,
    const int64_t residual_stride) {
  const int vec_hidden_size = hidden_size / width;
  __shared__ float s_variance;
  float variance = 0.0f;
  ...
  for (int idx = threadIdx.x; idx < vec_hidden_size; idx += blockDim.x) {
    _f16Vec<scalar_t, width> temp = input_v[strided_id];   // 向量读 x
    temp += residual_v[id];                                 // y = x + residual（packed half2 运算）
    variance += temp.sum_squares();                         // FP32 累加平方和
    residual_v[id] = temp;                                  // y 原地写回 residual
  }
  using BlockReduce = cub::BlockReduce<float, 1024>;
  __shared__ typename BlockReduce::TempStorage reduceStore;
  variance = BlockReduce(reduceStore).Reduce(variance, CubAddOp{}, blockDim.x);
  if (threadIdx.x == 0) { s_variance = rsqrtf(variance / hidden_size + epsilon); }
  __syncthreads();
  for (int idx = threadIdx.x; idx < vec_hidden_size; idx += blockDim.x) {
    _f16Vec<scalar_t, width> res = residual_v[id];          // 第二遍重读 y
    _f16Vec<scalar_t, width> w = weight_v[idx];
    float x = Converter::convert(res.data[j]);              // 标量提升到 FP32 计算
    float wf = Converter::convert(w.data[j]);
    out.data[j] = Converter::convert(x * s_variance * wf);  // 覆写 input 为输出
  }
}
```

要点：
- **归约与 epilogue 全 FP32**：`variance`/`s_variance` 是 `float`；packed 运算只用于搬运和加法，乘 gamma 前逐元素 `convert` 回 float。
- **generic 版**（L179-217，width=0 或类型不支持 packed）：纯标量路径，`float x = (float)z; variance += x*x;`——FP32 累加纪律与向量版一致。
- **网格/块配置启发式**（host 侧 L329-372）：

```cpp
dim3 grid(num_tokens);                    // 一行一块
const int max_block_size =
    batch_invariant_launch ? 1024 : ((num_tokens < 256) ? 1024 : 256);  // 小 batch 大块、大 batch 小块提占用
dim3 block(std::min(hidden_size, max_block_size));
constexpr int vector_width = 8;           // half/bf16 ×8 = 16B
constexpr int req_alignment_bytes = vector_width * 2;
bool offsets_are_multiple_of_vector_width =
    hidden_size % vector_width == 0 && input_stride % vector_width == 0 && ...;
if (ptrs_are_aligned && offsets_are_multiple_of_vector_width && !batch_invariant_launch)
  LAUNCH_FUSED_ADD_RMS_NORM(8, true);     // 对齐→向量化核
else
  LAUNCH_FUSED_ADD_RMS_NORM(0, true);     // 否则→标量核
```

- `rms_norm_kernel`（非融合版，L14-100）用 `vectorize_read_with_alignment<VEC_SIZE>` 做归约遍历、第二遍显式 `reinterpret_cast` 向量化读 x/w 写 out；`VEC_SIZE = gcd(16/sizeof(T), hidden_size)`。
- 配套 `quantization/vectorization_utils.cuh` 的**前缀-主体-尾块三段式**（L142-184）：地址不对齐时先标量处理 `prefix_elems`，中段 `num_vec = len / VEC_SIZE` 向量化，`tail_start = num_vec * VEC_SIZE` 之后标量收尾。这是 GPU 侧"D 非 32 倍数"的标准答案（逐元素粒度）。
- 已知缺陷（issue #43390，C 级）：`int idx` 循环变量乘 `vec_hidden_size` 在 hidden×batch 巨大时 32 位溢出——对 Ascend 侧的警示见 §6。

> **语义辨析**：vLLM 融合核第二遍把 `y` 从 residual 重读（因为第一遍只把它留在 GM）；且输出覆写 input、y 覆写 residual。本题 y 无需写回 GM（留在 UB 即可），output 独立——迁移时取其"FP32 累加 + rsqrtf + 二遍归一化"骨架，舍弃其原地双写回。

### 2.3 NVIDIA TransformerEngine：tuned（编译期平铺）+ general（运行期列数）双路径

仓库：`NVIDIA/TransformerEngine`，文件：`transformer_engine/common/normalization/rmsnorm/rmsnorm_fwd_kernels.cuh`、`rmsnorm_fwd_cuda_kernel.cu`、`rmsnorm_api.cpp`，main @ `224f6ecf5e8d`（2026-09-11）。

> **范围澄清**：TE 的 RMSNorm **前向核不带 residual add**（`rmsnorm_fwd` 签名只有 x/gamma/epsilon/z/rsigma）；residual 融合在 TE 的 LayerNorm 前向（`ln_fwd_kernels.cuh`）与 RMSNorm **反向**（`nvte_rmsnorm_bwd_add`，`rmsnorm_api.cpp` L176-245）里。因此 TE 对本题的价值是**平铺与归约组织**，residual 融合语义以 vLLM/flashinfer 为准。任务书所述 "Transformer Engine fused_add_rmsnorm" 在当前 main 分支的前向路径上不存在，此为已核实的事实修正（B 级）。

tuned 核（编译期已知 HIDDEN_SIZE，L25-141）——**整行驻留寄存器 + gamma 预载**：

```cpp
compute_t *rs_ptr = static_cast<compute_t *>(params.rs);
Wvec gamma[LDGS];                          // gamma 预载到寄存器，跨行复用
index_t idx = c;
#pragma unroll
for (int it = 0; it < LDGS; it++) { gamma[it].load_from(params.gamma, idx); idx += VEC_COLS_PER_LDG; }
constexpr compute_t rn = 1.f / compute_t(Ktraits::COLS);
for (int row = r; row < params.rows; row += params.ctas_per_col * ROWS_PER_CTA) {   // 行跨步
  Ivec x[LDGS];
  compute_t xf[LDGS * NUM_ELTS];           // 整行元素驻留寄存器（单遍数据不复读）
  ...
  stats_t s = stats.compute(xf, rn);       // Welford 统计（块内+跨CTA）
  compute_t mu  = Get<0>::of<stats_t, compute_t>(s);
  compute_t m2  = Get<1>::of<stats_t, compute_t>(s);
  compute_t rs  = rsqrtf(rn * m2 + mu * mu + params.epsilon);   // RMSNorm: rstd = rsqrt(E[x²]+eps)
  ...
  compute_t y_ij = rs * (xf[it * NUM_ELTS + jt]);    // 直接用寄存器里的 x，不重读
  compute_t g_ij = gamma[it].data.elt[jt];
  compute_t temp_output = g_ij * y_ij;
```

general 核（运行期 cols，L161-317）——**列不对齐/超宽行的兜底**：`load_from_elts(ptr, idx, valid_elts)` 带边界读，`col + jt < params.cols` 逐元素守卫，跨 CTA 用 `DynamicReducer::allreduce`（workspace + barrier）。

launch 侧（`rmsnorm_fwd_cuda_kernel.cu` L23-58）：

```cpp
NVTE_CHECK_CUDA(cudaOccupancyMaxActiveBlocksPerMultiprocessor(
    &ctas_per_sm, kernel, Kernel_traits::THREADS_PER_CTA, Kernel_traits::SMEM_BYTES_FWD));
launch_params.params.ctas_per_col = multiprocessorCount * ctas_per_sm / ctas_per_row;
...
if (ctas_per_row == 1) {
  kernel<<<ctas_per_col, THREADS_PER_CTA, SMEM_BYTES_FWD, stream>>>(params);
} else {
  dim3 grid(ctas_per_row * ctas_per_col);
  cudaLaunchCooperativeKernel(...);       // 单行拆多 CTA 须协同启动
}
```

`common/utils.cuh`（同 commit）的 `Stats<T, CTAS_PER_ROW, ...>`（L699-765）：块内 Welford（`warp_chan_upd_dynamic`，Chan 并行更新公式）→ 各 CTA 写 `workspace[bidn_]` → `InterCTASync` 自旋（`red.release.gpu.global.add` + `ld.global.acquire`，双 barrier 缓冲 `w0_/w1_` 防摇摆）→ 单 warp `warp_chan_upd_dynamic` 终归约。**这是"一行拆多 CTA"的完整参考，但其协同启动模型在 Ascend vector 直调下无对应物**（§3、§5）。

### 2.4 flashinfer：`RMSNormKernel` / `FusedAddRMSNormKernel`（shared memory 缓存 y，免二次读 GM）

仓库：`flashinfer-ai/flashinfer`，文件：`include/flashinfer/norm.cuh`，main @ `c05407ceffb7`（2026-09-11）。

融合核（L414-504）——**与本题最贴切的"y 驻留片上"实现**：

```cpp
__global__ void FusedAddRMSNormKernel(T* input, T* residual, T* weight, const uint32_t d,
    const uint32_t stride_input, const uint32_t stride_residual, float weight_bias, float eps) {
  ...
  extern __shared__ float smem[];
  float* smem_x = smem + ceil_div(num_warps, 4) * 4;    // y 的 FP32 缓存区
  float sum_sq = 0.f;
  for (uint32_t i = 0; i < rounds; i++) {
    vec_t<T, VEC_SIZE> input_vec, residual_vec;  vec_t<float, VEC_SIZE> x_vec;
    if ((i * num_threads + thread_id) * VEC_SIZE < d) {   // 整 vec 粒度边界守卫
      input_vec.load(...); residual_vec.load(...);
    }
    #pragma unroll
    for (uint32_t j = 0; j < VEC_SIZE; j++) {
      float x = float(input_vec[j]);
      x += float(residual_vec[j]);       // FP32 中做 y = x + residual
      sum_sq += x * x;
      residual_vec[j] = (T)x;            // y 写回 residual（转回 T）
      x_vec[j] = x;                      // FP32 的 y 驻留 smem
    }
    if (...) { residual_vec.store(...); x_vec.store(smem_x + ...); }
  }
  // 两级归约：warp 内 shfl_xor 蝴蝶 + 首 warp 跨 warp smem 归约
  for (uint32_t offset = warp_size / 2; offset > 0; offset /= 2)
    sum_sq += math::shfl_xor_sync(sum_sq, offset);
  smem[ty] = sum_sq;  __syncthreads();
  if (ty == 0) { /* 首 warp 归约 smem[tx] → smem[0] */ }
  __syncthreads();
  float rms_rcp = math::rsqrt(smem[0] / float(d) + eps);
  for (uint32_t i = 0; i < rounds; i++) {                 // 第二遍从 smem_x 读，不回 GM
    ...
    x_vec.load(smem_x + ...);  weight_vec.load(weight + ...);
    input_vec[j] = x_vec[j] * rms_rcp * (weight_bias + float(weight_vec[j]));
  }
}
```

host 侧（L507-537）：`vec_size = gcd(16 / sizeof(T), d)`；`block = min(1024, d / vec_size)`；`smem_size = (ceil_div(num_warps,4)*4 + d) * sizeof(float)` 并用 `cudaFuncSetAttribute` 突破 48KB 默认动态共享内存；支持 Hopper+ PDL（`griddepcontrol.wait/launch_dependents`）。

> **语义辨析**：flashinfer 的 `weight_bias` 出现在乘法里（`x * rms_rcp * (weight_bias + w)`），是为 Gemma 的 `(1+γ)` 设计的**乘性**偏置；本题 `bias` 是 `y/rms*gamma + bias` 的**加性**项。迁移该核时 epilogue 必须改为 `x_vec[j]*rms_rcp*w + b`，gamma 与 bias 各自独立加载。

非融合 `RMSNormKernel`（L63-137）的第二遍则**重读全局内存** input（与 PyTorch 快路径同策略），对照可见"y 留 smem"是融合版独有的带宽优化——代价是 smem 占 `d×4B`（d=32768 → 128KB，仅 H100 级 smem 才放得下）。

### 2.5 liger-kernel：`_rms_norm_forward_kernel`（Triton，一行一 program，BLOCK_SIZE 整行）

仓库：`linkedin/Liger-Kernel`，文件：`src/liger_kernel/ops/rms_norm.py`（693 行），main @ `95b01e94027c`（2026-09-10）。文件头注明代码源自 unsloth（`rms_layernorm.py` L22，commit `fd753fed`）并经修改。

```python
@triton.jit
def _rms_norm_forward_kernel(..., n_cols, eps, offset, casting_mode: tl.constexpr,
                             elementwise_affine: tl.constexpr, BLOCK_SIZE: tl.constexpr):
    row_idx = tl.program_id(0).to(tl.int64)
    col_offsets = tl.arange(0, BLOCK_SIZE)
    mask = col_offsets < n_cols                      # 尾块掩码，越界补 0
    ...
    X_row = tl.load(x_base + col_offsets, mask=mask, other=0)
    if casting_mode == _CASTING_MODE_LLAMA:          # Llama：统计在 fp32
        X_row = X_row.to(tl.float32)
    ...
    eps = eps.to(tl.float32)                         # 防止 Inductor 传 fp64 标量把整个计算提升到 f64
    mean_square = tl.sum(X_row * X_row, axis=0) / n_cols
    rstd = rsqrt(mean_square + eps)
    tl.store(rstd_base, rstd)                        # rstd 缓存供反向复用
    X_row = X_row * rstd
    if casting_mode == _CASTING_MODE_LLAMA:
        X_row = X_row.to(X_row_dtype)                # 权重乘回原 dtype（复刻 HF）
    if elementwise_affine:
        Y_row = X_row * (offset + W_row)
    tl.store(y_base + col_offsets, Y_row, mask=mask)
```

要点：`BLOCK_SIZE = triton.next_power_of_2(n_cols)`（`calculate_settings`），**整行一块、无循环**——依赖 BLOCK_SIZE ≤ Triton/PTX 寄存器与 smem 上限；尾块靠 `mask + other=0`（补零对 `sum(x²)` 恰好无贡献）；`_CASTING_MODE_LLAMA` 的"归一化结果转回原 dtype 再乘 weight"是**精度复刻 HF 而非性能选择**，与本题"归一化及中间累加 FP32"的题面要求不同，不可照搬（§5）。另有 NPU 适配痕迹（`get_npu_core_count`、大 GRF 模式，2026-01 合入的 PR #1000 把指针原地 += 改为 base+index，供 Torch NPU 后端）——说明该核已被移植到昇腾 Triton 类后端，可作旁证（C 级）。

### 2.6 Triton 官方教程 05-layer-norm：分块循环两遍扫描（大 D 的通用解）

仓库：`triton-lang/triton`，文件：`python/tutorials/05-layer-norm.py`，main @ `7fe90e2150e6`（2026-09-11）。

```python
@triton.jit
def _layer_norm_fwd_fused(X, Y, W, B, Mean, Rstd, stride, N, eps, BLOCK_SIZE: tl.constexpr):
    row = tl.program_id(0)
    ...
    _mean = tl.zeros([BLOCK_SIZE], dtype=tl.float32)
    for off in range(0, N, BLOCK_SIZE):              # 第一遍：分块累加
        cols = off + tl.arange(0, BLOCK_SIZE)
        a = tl.load(X + cols, mask=cols < N, other=0.).to(tl.float32)
        _mean += a
    mean = tl.sum(_mean, axis=0) / N
    _var = tl.zeros([BLOCK_SIZE], dtype=tl.float32)
    for off in range(0, N, BLOCK_SIZE):              # 第二遍：分块累加平方
        cols = off + tl.arange(0, BLOCK_SIZE)
        x = tl.load(X + cols, mask=cols < N, other=0.).to(tl.float32)
        x = tl.where(cols < N, x - mean, 0.)
        _var += x * x
    var = tl.sum(_var, axis=0) / N
    rstd = 1 / tl.sqrt(var + eps)
    for off in range(0, N, BLOCK_SIZE):              # 第三遍：读 x/w/b 写 y
        ... x_hat = (x - mean) * rstd;  y = x_hat * w + b
```

要点：当 D 大于单块可驻留容量时，用 `for off in range(0, N, BLOCK_SIZE)` 分块**多遍扫描**（LayerNorm 要三遍；RMSNorm 化简后两遍）；掩码补零 + `tl.where` 显式置零防止越界值污染方差。这对 Ascend 侧"D=32768 时 y 的 FP32 中间值（128KB）+ 各输入缓冲无法同时驻留 192KB UB"的场景是直接可借鉴的组织方式（配合 DataCopyPad 分块搬入）。

### 2.7 unsloth：`_rms_layernorm_forward`（Liger 的上游，最精简形态）

仓库：`unslothai/unsloth`，文件：`unsloth/kernels/rms_layernorm.py`，commit `fd753fed99ed5f10ef8a9b7139588d9de9ddecfb`（Liger 头部指定的精确 commit）：

```python
X_row = tl.load(X + col_offsets, mask = mask, other = 0).to(tl.float32)
W_row = tl.load(W + col_offsets, mask = mask, other = 0)
row_var = tl.sum(X_row * X_row, axis = 0) / n_cols
inv_var = tl.math.rsqrt(row_var + eps)
tl.store(r, inv_var)
normed = X_row * inv_var
normed = normed.to(W_row.dtype)     # "Exact copy from HF"：乘 weight 前转回原 dtype
output = normed * W_row
tl.store(Y + col_offsets, output, mask = mask)
```

要点：单 program 整行、`rsqrt`、masked load——Liger 的直系源头；同样有"转回原 dtype 乘 weight"的 HF 精度复刻行为（与本题 FP32 要求冲突，见 §5）。

---

## 3 统一迁移差异表（GPU 机制 → Ascend C）

| # | GPU 机制（证据来源） | Ascend C 对应机制 | 可迁移部分 | 不可直接迁移部分 | 风险 |
|---|---|---|---|---|---|
| 1 | **warp shuffle 归约**：`__shfl_down/_xor` 蝴蝶 + 首 warp 二次归约（PyTorch `block_reduce.cuh`；flashinfer L96-110；TE `warp_chan_upd_dynamic`） | `WholeReduceSum`（块内）/ `ReduceSum`（LocalTensor 最后一维）；API 细节前序 Agent 2 已核 | "线程先局部累加、再整块归约一次"的两级结构思想完全可迁移；归约输入须 FP32 | shuffle 指令本身无 Ascend C 编程接口，须整体替换为 ReduceSum/WholeReduceSum 调用；ReduceSum 不支持 bfloat16_t（前序已确认）→ 归约前必须 Cast | 归约 API 的累加顺序/精度与 GPU 不同属正常现象，但需过题面误差门限；`ReduceSum` 的 mask/尾块语义须按 Agent 2 核对的签名确认 |
| 2 | **shared memory**：`__shared__` 数组存 warp 部分和/整行 y（vLLM `s_variance`；flashinfer `smem_x`；PyTorch `s_data`） | **UB（Unified Buffer，A2 192KB）+ LocalTensor**（容量/bank 数据见 Agent 7 报告 §2.1） | "中间统计量/y 留在片上、不回 GM"的带宽策略可迁移：y 的 FP32 副本驻留 UB，二遍直接复用（flashinfer smem_x 思想） | UB 是 Tensor 语义（DataCopy 搬运 + API 计算），不能像 smem 一样按字节 reinterpret 拼浮点缓冲；bank 冲突规则不同（48 bank/16 BG vs smem 32 bank），GPU 侧 padding 技巧数字不可照搬 | D=32768 时 y 的 FP32 副本 128KB，加 x/residual/gamma/bias 缓冲超 UB 预算 → 必须分块（§4 结论 C3） |
| 3 | **coalesced/vectorized load**：`aligned_vector<T,4>`、`vec_t`、`_f16Vec<T,8>`（16B 向量）（PyTorch L39-49；vLLM L116-131；flashinfer `vec_t`） | `DataCopy`/`DataCopyPad`（GM→UB 整块搬运，32B 对齐粒度，Agent 2 已核） | "按最大对齐宽度整块搬入、一次搬运多个元素"的带宽利用思想可迁移；16B/32B 对齐检查逻辑（`can_vectorize`/指针取模）可对应到 32B 对齐约束 | GPU 是"线程视角的向量指令、地址由编译器保证合并"；Ascend 是"块视角的批量搬运 API"，没有 per-thread 向量访存，无法把 `reinterpret_cast<_f16Vec*>` 逐指针翻译 | 尾部不足 32B 时的搬运粒度是主要风险点（见第 5 行） |
| 4 | **寄存器驻留**：整行元素/ gamma 预载寄存器数组（TE tuned 核 `compute_t xf[LDGS*NUM_ELTS]`、`Wvec gamma[LDGS]`） | LocalTensor 常驻 UB + gamma/bias 分块复用（ops-nn add_rms_norm 已有五种 tiling 模式，Agent 3 已核） | "gamma/bias 跨行复用、只搬一次"的策略可迁移（gamma/bias 长度 D ≤ 32768，fp32 最多 128KB，可整驻或按块驻留）；"整行数据驻留到二遍"（对应 UB 驻留） | 无用户可见寄存器编程模型，`xf[]` 数组→LocalTensor；编译器寄存器分配压力问题（GPU 上 HIDDEN_SIZE 过大时 tuned 核不适用）在 Ascend 侧转化为 UB 容量问题，约束不同 | 单行 fp32 驻留 128KB 上限时 UB 余量不足，需验证分块两遍方案的精度（累加顺序改变） |
| 5 | **mask 尾块**：Triton `mask=cols<N, other=0`（liger/unsloth/教程）；vLLm `vectorize_read_with_alignment` 前缀+主体+尾段三段式（L142-184）；flashinfer 整 vec 边界 `if ((i*num_threads+tid)*VEC_SIZE < d)`；TE `load_from_elts(..., valid)` | `DataCopyPad`（GM→UB 时自动 pad 到 32B 边界，Agent 2 已核）+ 写回时按行真实长度搬运 | "越界位置补零、零对 sum(x²) 无贡献"的数学安全性可迁移（DataCopyPad 默认补 0 与 GPU other=0 等价）；"主循环只走整块 + 尾块单独处理"的三段式结构可迁移 | Ascend 无逐元素 mask load；DataCopyPad 是搬运级 padding，**写回 GM 时若 pad 长度大于真实 D 会越界覆盖下一行**——GPU 的 masked store 天然安全，Ascend 必须显式区分"搬入 pad"与"搬出真实长度" | 本题 D 非 32 倍数是明确测试点；输出越界覆盖相邻行 = 判题直接失败，属最高优先级验证项 |
| 6 | **blockDim 电网格**：`grid=num_tokens` 一行一块（vLLM/flashinfer/PyTorch）；TE `ctas_per_col×ROWS_PER_CTA` 行跨步 `row += ctas_per_col*ROWS_PER_CTA`；ROCm 分支 grid-stride（PyTorch L1187-1198） | `<<<blockNum, nullptr, stream>>>` + `GetBlockIdx()/GetBlockNum()` 行分配或行跨步循环（题面直调格式） | "blockIdx = 行号"直接映射 `GetBlockIdx()`；outer>核数时 TE/ROCm 的行跨步循环模式可整体迁移；vLLM "小 batch 大块、大 batch 小块" 的占用启发式可参考 Agent 7 的核数数据转化为 blockNum 选择 | GPU 网格是硬件调度自由度（可 65535×65535），Ascend blockNum 上限与核数强相关（Agent 7 §2.2）；cudaOccupancyMaxActiveBlocksPerMultiprocessor 无对应 API | **整数溢出**：vLLM #43390（`int idx` × `vec_hidden_size` 溢出）前车之鉴——Ascend 上 `GetBlockIdx()*D` 若用 int32，outer×D > 2³¹ 即溢出（D=32768、outer≥65536 触发），须用 int64 或核算范围 |
| 7 | **cp.async 双缓冲**：显式异步搬运 + 搬运/计算重叠 | `TQue` 双缓冲（`TPipe`/EnQue/DeQue，`SetBufPosition`/double buffer，Agent 2/7 已核） | "多行/多块间搬入与计算流水重叠"的思想可迁移（TQue 声明 `bufferNum=2` 即得双缓冲） | **本次调研的 7 个 RMSNorm 实现均未在前向使用 cp.async**（数据量小、单核内两遍结构，靠 ILP/occupancy 隐藏延迟已够）——GPU 侧 norm 前向的常态就是同步 load，双缓冲收益主要来自多行循环场景 | 直调单核函数模型下 TQue 用法与工程样例绑定（Agent 3 已核 ops-nn 用法），照搬 GPU 的"async copy + commit group"心智模型可能写出不必要的复杂流水；以 ops-nn 现成 tiling 为准 |
| 8 | **__syncthreads**：块内栅栏（vLLM L75/149；flashinfer L101-111；PyTorch inter-warp 合并） | `PipeBarrier<PIPE_V>()`（vector 核内同步）；HardEvent 用于向量/标量不同流水同步（Agent 2 已核签名） | "统计量算完→栅栏→全体广播 rstd→二遍计算"的控制流可迁移；rstd 广播（GPU 上 smem[0]/s_variance 全线程可见）对应 UB 上标量/单元素 LocalTensor 或寄存器标量复用 | GPU 的 __syncthreads 语义粒度是"块内所有线程"；Ascend vector 核一个 block 对应一份标量代码路径，同步点语义更接近"流水线间依赖"，两者不是一一映射 | 归约 API（WholeReduceSum 等）内部是否已含同步语义需按 Agent 2 核对结果使用，避免冗余/缺失 barrier 造成读旧值 |
| 9 | **Welford/单遍 vs 两遍扫描**：PyTorch/TE 用 Welford 在线算法（RMSNorm 分支退化为 `sigma2 += val*val`，即**单遍平方和**）；vLLM/flashinfer 直接 `sum_sq += x*x` 单遍 + 二遍归一化；Triton 教程分块多遍 | 两遍结构：第一遍 y=x+residual + FP32 平方和（y 驻留 UB），`Rsqrt`/`Sqrt`+`Muls`，第二遍 epilogue（Mul/Add） | **核心结论：RMSNorm 不需要 Welford**（无 mean，PyTorch 源码 rms_norm 分支已退化）；"单遍累加平方 + 二遍 epilogue"是最简且全实现一致的结构，可直接映射为 Ascend C 的两段计算 | Welford 三元组（mean/M2/count）合并逻辑（PyTorch `cuWelfordCombine`、TE `warp_chan_upd_dynamic`/Chan 公式）对本题是**多余机制**，不应迁移；TE 跨 CTA Welford 合并（workspace+barrier）亦不可迁移（见 §5） | 二遍数据来源的选择（y 驻留 UB 复用 vs 重读 GM/重搬）直接决定带宽占用与 UB 预算，是本题实现的主要设计自由度（§4 C3） |

补充行（超出任务书 9 行清单、但源码中证据充分）：

| # | GPU 机制 | Ascend C 对应 | 说明 |
|---|---|---|---|
| 10 | **cub::BlockReduce**（vLLM L68-70） | WholeReduceSum/ReduceSum | CUB 是库封装，内部同样是 shuffle+smem 两级；迁移时关注语义等价（求和 + 结果可得性）而非实现 |
| 11 | **PDL（programmatic dependent launch）**：`griddepcontrol.wait/launch_dependents`（flashinfer L78-80/134-136） | 无对应 | 跨 kernel 依赖启动优化，直调单 kernel 场景无关，不迁移 |
| 12 | **packed half2 数学**：`_f16Vec::sum_squares()`、`__half22float2`（vLLM type_convert.cuh） | bf16/fp16 输入 Cast 到 FP32 后用向量 API 计算 | A2 上 Add/Mul 不支持 bfloat16_t（前序已确认）→ packed bf16 数学整体不可迁移；fp16 packed 可行但与"FP32 中间累加"题面要求冲突，统一走 FP32 |

---

## 4 对本题可迁移结论

以下结论均为**静态迁移分析**（证据等级 B：全部基于 §2 源码），未经 NPU 验证；Ascend 侧机制以 Agent 2/3/7 已核对事实为锚。

**C1. 计算骨架选 vLLM/flashinfer 融合核结构，弃 Welford。**
7 个实现中 RMSNorm 的统计全部退化为"FP32 单遍平方和 + rsqrt"（PyTorch `cuWelfordOnlineSum` rms_norm 分支 `{0, sigma2+val*val, 0}`；vLLM `variance += temp.sum_squares()`；flashinfer `sum_sq += x*x`；liger/unsloth `tl.sum(X_row*X_row)/n_cols`）。本题 Ascend C 骨架应为：搬入 x/residual → Cast FP32 → Add 得 y（UB 驻留）→ Mul(y,y) → ReduceSum（或分块累加）→ `Muls(1/D)` → Add(eps) → `Sqrt`（或 Rsqrt+Muls，Agent 7：A2 上 Rsqrt fp32 64 elem/cycle vs Sqrt 32 elem/cycle，Rsqrt 更优）→ 二遍 `y*rs*gamma + bias` → Cast 回目标 dtype → 搬出。加性 bias 直接挂在 epilogue（vLLM 结构 + 加法），不用 flashinfer 的乘性 weight_bias。

**C2. FP32 纪律与 A2 限制互相印证。**
所有 GPU 实现的统计累加与 epilogue 乘加均在 FP32（`float variance`/`T_ACC`/`vec_t<float>`），输入输出仅 half/bf16——与题面"归约及中间累加 FP32"一致；且 A2 上 Add/Mul/Sqrt/Rsqrt/ReduceSum 不支持 bfloat16_t（前序确认），fp16 虽支持但题面要求 FP32 中间值，故**统一在 Cast 到 FP32 之后做全部运算**是 GPU 证据与硬件约束的双重收敛点。gamma/bias 按其存储类型搬入后 Cast FP32 参与 epilogue（PyTorch PR #195638 的 mixed-dtype gamma 处理佐证：gamma 加载后 convert 到累加类型，B 级）。

**C3. "y 驻留片上 + 分块两遍"是 UB 预算内的主方案。**
flashinfer 用 smem 缓存 y 的 FP32 副本免除二遍读 GM（带宽最优），但 D=32768 时该副本 128KB，加 x/residual 搬入缓冲与 gamma/bias，超出 A2 UB 192KB——flashinfer 思路只能在小 D 时整体套用。大 D 时采用 Triton 教程 05 的**分块循环**组织：按块 DataCopyPad 搬入 y（已在 UB 的 FP32）分块累加平方和；块内 y 不淘汰可继续二遍 epilogue（等效单遍驻留），淘汰则二遍重搬（等效 PyTorch 快路径重读）——两种模式的 UB 预算取舍需真机验证。ops-nn add_rms_norm 的五种 tiling（Agent 3 已核）正是同一问题的官方切片，本表为其提供 GPU 侧对偶证据。

**C4. 尾块三段式 + 写回真实长度。**
vLLM `vectorize_read_with_alignment` 的"对齐前缀（标量）→ 向量主体 → 标量尾块"与 DataCopyPad 的搬运语义最接近；迁移落点：**搬入**用 DataCopyPad（越界补 0，对 sum(y²) 无污染）；**搬出**必须按每行真实 D 计算搬运长度，禁止把 pad 长度写出（覆盖相邻行 = 判题失败）。D 非 32 倍数是题面明示测试点，此为最高风险验证项。

**C5. 网格：行跨步循环为主，规避 int32 溢出。**
`GetBlockIdx()` 行分配（vLLM 一行一块）在 outer ≤ blockNum 时最优；outer 更大时用 TE/ROCm 的行跨步循环（`row += GetBlockNum()*ROWS_PER_BLOCK`）。索引一律 `GetBlockIdx() * D` 先升 int64 再运算（vLLM #43390 溢出教训，C 级佐证 + B 级代码修复模式）。

**C6. 双缓冲按 ops-nn 现成用法，不自造流水。**
GPU 侧 7 个 norm 前向实现均未用异步搬运（第 3 表第 7 行），说明该算子的瓶颈在访存量而非搬运延迟；Ascend 侧直接采用 ops-nn add_rms_norm 的 TQue 双缓冲 tiling 模式（Agent 3 已核）即可，无需按 GPU "cp.async + commit" 心智模型重设计。

---

## 5 仅作参考 / 不可直接迁移清单

| GPU 机制 | 出处 | 不可迁移原因 | 处置 |
|---|---|---|---|
| TE `CTAS_PER_ROW>1`：单行拆多 CTA + `cudaLaunchCooperativeKernel` + `InterCTASync` 自旋 barrier（`red.release.gpu.global.add`/`ld.global.acquire` 双缓冲） | TE `utils.cuh` L395-427、`Stats` L699-765 | Ascend vector 直调单核函数模型无跨 block 协同启动与全局内存 barrier 原语对应物；且本题 D≤32768 的 FP32 平方和单核 ReduceSum 即可完成 | 仅作"超宽行需要多核协作"的问题意识参考；本题不采用 |
| Welford 三元组在线算法及 Chan 并行合并（`cuWelfordCombine`、`warp_chan_upd_dynamic`） | PyTorch L172-203；TE `utils.cuh` L653-695 | RMSNorm 无均值项，两实现源码中该路径均已退化为纯平方和；迁移 Welford 只增加计算与状态量 | 不迁移 |
| PDL `griddepcontrol.wait/launch_dependents` | flashinfer L78-136 | Hopper 跨 kernel 依赖启动特性，单 kernel 直调无关 | 不迁移 |
| packed half2/bf16 数学（`_f16Vec`、`__half22float2`、`sum_squares`） | vLLM `type_convert.cuh`、fused 核 | A2 向量 API 不支持 bfloat16_t 运算（前序确认）；fp16 packed 与题面 FP32 中间累加要求冲突 | 输入输出保持 half/bf16 搬运，运算全部 FP32 |
| flashinfer `weight_bias` 乘性偏置 `(weight_bias + w)` | flashinfer L127/494 | 本题 bias 是加性 `+ b`，语义不同 | epilogue 改为 `y*rs*gamma + bias` |
| vLLM/flashinfer 的原地双写回（residual←y，input←output） | vLLM fused 核 L139/L172；flashinfer L450-457/497 | 本题 output 为独立张量、y 为纯中间量，无需写回 y | 保留其统计与 epilogue 结构，去掉双写回 |
| liger/unsloth `_CASTING_MODE_LLAMA`：归一化结果转回原 dtype 再乘 gamma | liger L139-143；unsloth L36-38 | 该行为是**逐位复刻 HuggingFace 前向**的兼容选择（"Exact copy from HF"），与本题"归一化及中间累加 FP32"的题面要求相反 | 不迁移；本题 gamma 乘法留在 FP32 |
| vLLM batch-invariant 模式（锁 block=1024 保证跨 batch 逐位一致） | vLLM host 代码 L337-339 | 判题按测试点给误差门限，无跨 batch 逐位一致需求 | 不迁移；但记录其教训：**block/tiling 参数改变会改变浮点累加顺序**，调优与精度验证须成对进行 |
| `cudaOccupancyMaxActiveBlocksPerMultiprocessor` 动态网格 | TE launch L23-36 | 无对应 API；A2 核数为静态已知（Agent 7 §2.2） | 用固定核数×行跨步替代 |
| PyTorch 慢路径两核方案（`RowwiseMomentsCUDAKernel` + `LayerNormForwardCUDAKernel` 两次 launch） | PyTorch L1201-1207 | 两次 kernel 启动把 y 写回 GM 再读，带宽翻倍；Ascend 单核内 UB 驻留天然避免 | 仅作两遍扫描的算法参照 |

---

## 6 风险与未验证事项

**未验证（本机无 NPU，全部结论待真机确认）：**
1. §4 全部迁移结论未经 CANN 9.0.0 编译、精度或性能验证；任何"可迁移"均指结构与语义映射成立，不构成可用性证明。
2. D=32768（fp32 y 副本 128KB）下 UB 预算的具体分配（y 驻留 vs 分块重搬两种模式）未验证；UB 192KB 与 bank 冲突细节见 Agent 7 报告，本文不重复测数。
3. DataCopyPad 搬出非 32 倍 D 行时是否会越界覆盖下一行——题面明示 D 可非 32 倍数，此为**最高优先级真机验证项**（需在真机用 D%32≠0 且相邻行有哨兵值验证）。
4. ReduceSum/WholeReduceSum 与 GPU 归约的累加顺序差异对 15 个测试点误差门限的影响未验证。
5. int32 溢出风险的实际触发边界（outer×D > 2³¹）取决于判题测试点 outer 上限，题面未给 outer 范围（Agent 1 已核对题面），防御性 int64 是低成本保险。

**未找到 / 覆盖不足：**
6. NVIDIA Developer Forums、GPU MODE（YouTube/Discord）、Hugging Face Forums 未做逐帖深挖——检索到的 RMSNorm 讨论最终均指向上述仓库源码，未发现源码之外的新机制；此部分按任务书要求如实登记为 partial 覆盖，不作为结论依据。
7. ROCm 侧独立仓库（Composable Kernel、hipBLASLt 等）未单独调研；ROCm 覆盖来自 PyTorch 层面 `USE_ROCM` 分支（grid-stride 行循环、gfx90a 快速倒数精度问题 L158-161——后者对 Ascend 无直接对应，仅说明低精度倒数在累加中的风险先例）。
8. vLLM 旧路径 `csrc/layernorm_kernels.cu` 的历史版本（issue #43390 引用的 `8c8b182`）未逐 commit 追溯；本报告以 main 分支现路径为准。

**证据局限：**
9. 各仓库 main 分支 HEAD（2026-09-11/12）为快照，后续演进未跟踪；PyTorch PR #195638（mixed-dtype gamma）为 Open 状态，其 dispatch 细节可能变化。
10. liger-kernel 的 NPU 适配（PR #1000、`get_npu_core_count`）证明 Triton→昇腾后端移植可行，但该路径与 Ascend C 直调无关（编译器层话题归 Agent 5），本文仅作旁证。

---

## 7 来源登记表

访问日期均为 2026-09-12。证据等级：A=官方文档/官方仓库页面；B=官方源码（逐行读取）；C=社区/论坛/issue；D=仅搜索摘要。

| # | URL | 标题 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 |
|---|---|---|---|---|---|---|---|
| 1 | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu | PyTorch aten CUDA LayerNorm/RMSNorm kernel | pytorch/pytorch main @ `8671f09631c5`（2026-09-11） | 2026-09-12 | RmsNormKernelImpl、Welford 退化、向量化快路径、网格配置 | B | verified（全文读取） |
| 2 | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/block_reduce.cuh | PyTorch CUDA block/warp 归约工具头 | pytorch/pytorch main @ `8671f09631c5` | 2026-09-12 | warp shuffle/两级块归约标准实现 | B | verified |
| 3 | https://github.com/pytorch/pytorch/pull/195638 | [ATen] Support mixed-dtype weight in fused RMSNorm CUDA kernel（Open PR） | pytorch/pytorch PR，commit `e8a736ec16ce` | 2026-09-12 | gamma 混合精度（bf16 激活/fp32 权重）dispatch 与 FP32 累加佐证 | B（PR diff+说明） | verified |
| 4 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/layernorm_kernels.cu | vLLM RMSNorm/FusedAddRMSNorm CUDA kernel | vllm-project/vllm main @ `0c1e89ceb92b`（2026-09-11） | 2026-09-12 | fused_add_rms_norm 全文、host 启发式、cub BlockReduce | B | verified（全文读取；旧路径 csrc/layernorm_kernels.cu 已迁移） |
| 5 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/quantization/vectorization_utils.cuh | vLLM 向量化读写工具（前缀-主体-尾块） | vllm-project/vllm main @ `0c1e89ceb92b` | 2026-09-12 | 尾块三段式处理模式 | B | verified |
| 6 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/type_convert.cuh | vLLM half/bf16/float 转换与 packed 类型 | vllm-project/vllm main @ `0c1e89ceb92b` | 2026-09-12 | `_f16Vec`/packed 数学、CUDA<12 与 sm<80 的 bf16 缺失 | B | verified |
| 7 | https://github.com/vllm-project/vllm/issues/43390 | [Bug]: integer overflow in fused_add_rms_norm | vllm-project/vllm issue（2026-05-22） | 2026-09-12 | int32 索引溢出风险案例（`blockIdx.x*vec_hidden_size+idx`） | C | verified（issue 正文） |
| 8 | https://github.com/vllm-project/vllm/issues/41430 | fused_add_rms_norm 不支持 weight=None 分支 | vllm-project/vllm issue（2026-05-01） | 2026-09-12 | 融合核参数面（has_weight 分支）佐证 | C | partial |
| 9 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_fwd_kernels.cu | TE RMSNorm 前向核（tuned+general） | NVIDIA/TransformerEngine main @ `224f6ecf5e8d`（2026-09-11） | 2026-09-12 | 寄存器驻留、Ktraits 平铺、跨 CTA allreduce、尾块守卫 | B | verified（全文读取） |
| 10 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_fwd_cuda_kernel.cu | TE RMSNorm 前向 launch 配置 | 同上 | 2026-09-12 | occupancy 网格、cooperative launch、barrier/workspace 尺寸 | B | verified |
| 11 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_api.cpp | TE RMSNorm C API（fwd/bwd/bwd_add） | 同上 | 2026-09-12 | 确认前向无 residual（fused add 仅在 bwd） | B | verified |
| 12 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/utils.cuh | TE Stats/Reducer/InterCTASync（Welford+跨CTA） | 同上 | 2026-09-12 | warp_chan_upd_dynamic、InterCTASync 自旋 barrier | B | verified |
| 13 | https://docs.nvidia.com/deeplearning/transformer-engine-releases/release-1.5/user-guide/api/c/rmsnorm.html | TE rmsnorm.h C API 文档（nvte_rmsnorm_fwd 等） | NVIDIA 官方文档（release 1.5） | 2026-09-12 | API 签名与公式交叉验证 | A | verified |
| 14 | https://github.com/flashinfer-ai/flashinfer/blob/main/include/flashinfer/norm.cuh | flashinfer norm kernels（RMSNorm/FusedAddRMSNorm/Quant） | flashinfer-ai/flashinfer main @ `c05407ceffb7`（2026-09-11） | 2026-09-12 | smem_x 驻留 y、两级归约、PDL、vec 边界守卫 | B | verified（全文读取） |
| 15 | https://github.com/linkedin/Liger-Kernel/blob/main/src/liger_kernel/ops/rms_norm.py | Liger-Kernel Triton RMSNorm | linkedin/Liger-Kernel main @ `95b01e94027c`（2026-09-10） | 2026-09-12 | Triton 整行单块、mask 尾块、casting 模式 | B | verified（全文读取） |
| 16 | https://github.com/linkedin/Liger-Kernel/pull/1000 | [NPU]: avoid pointer mutation in rms_norm kernel | linkedin/Liger-Kernel PR（2026-01 合入） | 2026-09-12 | Triton→NPU 后端移植旁证 | B/C | partial |
| 17 | https://github.com/unslothai/unsloth/blob/fd753fed99ed5f10ef8a9b7139588d9de9ddecfb/unsloth/kernels/rms_layernorm.py | unsloth RMS LayerNorm Triton kernel | unslothai/unsloth @ `fd753fed99ed5f10ef8a9b7139588d9de9ddecfb`（Liger 头部指定） | 2026-09-12 | Liger 上游最简形态、HF 精度复刻行为 | B | verified |
| 18 | https://github.com/triton-lang/triton/blob/main/python/tutorials/05-layer-norm.py | Triton 官方教程 05-Layer Normalization | triton-lang/triton main @ `7fe90e2150e6`（2026-09-11） | 2026-09-12 | 分块循环两/三遍扫描（大 D 通用解）、mask 尾块 | B | verified（全文读取） |
| 19 | https://github.com/ROCm/pytorch/pull/3564 | [ROCm] Optimize AMD normalization backward kernel（tiled） | ROCm/pytorch（pytorch fork）PR | 2026-09-12 | ROCm 侧 norm 内核优化动向（M>>N 场景） | B（diff） | partial |
| 20 | https://pytorch.org/blog/towards-free-normalization-fusing-normalization-into-gemm-and-attention-kernels/ | PyTorch 博客：Towards Free Normalization（Meta，B200） | pytorch.org 官方博客（2026-07-10） | 2026-09-12 | norm 与 GEMM/attention 融合的行业动向（超出本题直调范围） | A | partial（正文已读，结论未用于本题） |

**来源使用说明**：#1-#6、#9-#12、#14-#18 为本报告 §2 源码卡片的直接依据（B 级 verified）；#7/#8/#16/#19 为佐证（C/partial）；#13/#20 为官方文档级背景（A/partial）。论坛平台（NVIDIA Forums/GPU MODE/HF Forums）未产出独立来源，如实登记为未覆盖深入（§6 第 6 条）。

---

*Agent 4 报告完。本文档仅为 GPU→Ascend C 迁移调研，不含任何算子实现代码；所有结论未在 NPU 上验证。*
