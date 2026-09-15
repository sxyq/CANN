# Agent 4 报告：GPU/CUDA/Triton/PyTorch RMSNorm 实现 → Ascend C 可迁移性分析

> 调研日期：2026-09-11。任务：为 CANN 挑战赛初赛 AddRmsNormBias（昇腾 CANN 9.0.0，Ascend C vector Kernel，Direct Invocation）做"GPU 实现 → Ascend C 可迁移性"分析。
> 本地无昇腾 NPU，本报告所有结论均标注"未在真实 NPU 验证"；任何实现都不得据此声称 NPU 编译/精度/性能通过。
> 证据等级：A=官方仓库源码；B=官方样例/教程；C=社区文章。
> 源代码快照存放于 `/tmp/gpu_research/`（临时目录，仅本轮分析用；对应仓库版本与 URL 见第 6 节来源清单）。

---

## 1. 实现清单（6 个真实实现，均含链接与关键代码行）

### 1.1 PyTorch：RowwiseMomentsCUDAKernel（两遍）+ vectorized_layer_norm_kernel（融合版）

来源：`aten/src/ATen/native/cuda/layer_norm_kernel.cu`（main 分支）
URL：https://raw.githubusercontent.com/pytorch/pytorch/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu

- `RowwiseMomentsCUDAKernel`（L56-103）：两遍方案的第一遍。每行一个 block，`for (j = threadIdx.x; j < N; j += blockDim.x)` 线程步进累积 **Welford** 在线统计；`cuda_utils::BlockReduce` 做块内归约（L84-88）；`rms_norm` 模板参数为 true 时输出 `rstd = rsqrt(m2 + m1*m1 + eps)`（L96，m1 恒 0，即 `rstd = rsqrt(m2 + eps)`）。第二遍 `LayerNormForwardCUDAKernel`（L106+）重读 X 归一化。
- `vectorized_layer_norm_kernel`（L364-374）+ `compute_stats`（L206-268）：**融合版**（单 kernel 内先算 stats 再归一化，两遍读 X）。`compute_stats` 用 `WARP_SHFL_DOWN`（L229）做 warp 内归约、shared memory `meansigmabuf` 做跨 warp 归约（L248-251）——这是标准的两级归约：**warp shuffle（寄存器间）→ shared memory（block 内）**。
- 数值策略：累加类型 `T_ACC` 默认 float（L62-65 WelfordType 用 T_ACC；launch 时 fp16/bf16 输入一律 T_ACC=float），输出 `static_cast<T_ACC>` 再隐式转回 T（L308-312）。
- 尾块：本 kernel 假设 `N % vec_size == 0`（L298 注释 "No tail, N is guaranteed to be multiple of vec size"）；非对齐场景由 host 层选择非向量化路径或另行处理。
- 关键行摘录（L90-97）：
```cpp
if (threadIdx.x == 0) {
    auto [m2, m1] = welford_op.project(val);
    if constexpr (!rms_norm){ mean[i] = m1; rstd[i] = rsqrt(m2 + eps); }
    else { rstd[i] = c10::cuda::compat::rsqrt(m2 + m1 * m1 + eps); }
}
```

### 1.2 vLLM：rms_norm_kernel（两遍）+ fused_add_rms_norm_kernel（residual 融合、in-place 存 y）

来源：`csrc/libtorch_stable/layernorm_kernels.cu`（main 分支）
URL：https://raw.githubusercontent.com/vllm-project/vllm/main/csrc/libtorch_stable/layernorm_kernels.cu

- `rms_norm_kernel`（L15-100）：**两遍**。grid = num_tokens（每行一个 block），block ≤ 1024；第一遍 `vllm::vectorize_read_with_alignment<VEC_SIZE>` 向量化读入并把 `x*x` 累进 `float variance`（L54-66）；`cub::BlockReduce<float,1024>` 块内归约（L68-70）；`s_variance = rsqrtf(variance/hidden_size + epsilon)` 写 shared（L72-74）；`__syncthreads()` 后第二遍重读 input 乘 `s_variance` 乘 weight（L77-99）。
- `fused_add_rms_norm_kernel`（fp16/bf16 特化版 L107-174；通用版 L179-217）：**与本题语义最接近**（input + residual → RMSNorm）。
  - 第一遍循环（L133-140）：`temp = input_v[strided_id]; temp += residual_v[id]; variance += temp.sum_squares(); residual_v[id] = temp;` —— **关键技巧：把 y=x+residual 就地写回 residual buffer**，第二遍（L151-173）直接从 residual_v 读 y 做 `x * s_variance * w`，**避免重读 input**。
  - 向量化：`_f16Vec<scalar_t, width>` 类型双关（width=8，即 16B 对齐的 8×fp16/bf16 打包读），host 端校验指针 16B 对齐与 hidden_size % 8 == 0 才走 vec8 路径（L344-372）。
  - 尾块：**该 kernel 不支持尾块**——要求 `hidden_size % width == 0`；D 非 8 倍数走 width=0 的标量路径（L361）。
  - 数值：`float x = Converter::convert(res.data[j])`，乘除在 fp32，输出 `Converter::convert(...)` 回原 dtype（L160-163）。
  - 归约顺序注意（L333-336 注释）：batch-invariant 模式下 block 固定 1024 以保持跨 batch 位级一致（同一个 token 的归约宽度不同会导致浮点求和顺序不同）。

### 1.3 flash-attention：ln_fwd_kernel（CUDA，多行/多 CTA 特化，residual 融合，存 x 中间 buffer）

来源：`csrc/layer_norm/ln_fwd_kernels.cuh`（main 分支）
URL：https://raw.githubusercontent.com/Dao-AILab/flash-attention/main/csrc/layer_norm/ln_fwd_kernels.cuh

- 模板特化矩阵：按 hidden size（256/512/…/8192）、`BYTES_PER_LDG`（16B 向量化）、WARPS_N/WARPS_M 组合出数十个特化 kernel（`ln_fwd_*.cu` 逐个实例化），`ln_api.cpp` 按 N 查表选择——**编译期特化路线**。
- `ln_fwd_kernel`（L19-193）核心流程：
  - `save_x = has_residual || ...`（L50）：**只要存在 residual，第一遍就把 y=x+residual 写进 params.x 中间 buffer**（L137 `x.store_to(params.x, idx_x)`），并在寄存器保留 xf 副本。
  - 归约：每线程算 `xf[]` 后 `stats.template compute<Is_even_cols>`（L155-157），内部同样走 warp 归约 + smem（见 ln_utils.cuh 的 Stats 类，本仓库未展开）。
  - `rs = rsqrtf(m2 * inverse_cols + epsilon + (rms? 0 : mu*mu))`（L166）——RMS 分支直接把 mu² 项置 0。
  - 第二遍从 params.x buffer 读 y（而非重读 x+residual），乘 `rs * gamma + beta` 写 z（L176-189）。
- 多 CTA 行协作：`CTAS_PER_ROW > 1` 时用 `cudaLaunchCooperativeKernel` + grid sync 跨 CTA 归约（L260-267），行内均摊；默认 CTAS_PER_ROW=1。
- persistent 风格：`ctas_per_col` 由 occupancy 计算（L236-238），`for(row = r; row < rows; row += ctas_per_col * ROWS_PER_CTA)` grid-stride 多行循环（L102）。
- 尾块：`Is_even_cols` 编译期分支（cols == HIDDEN_SIZE 时）vs 运行期 `num_valid_ldgs` 判定（L82）——**掩码式尾块**，不补 0，无效 lane 不参与归约（`valid_elts_in_warp_fn` L148-154 精确统计有效元素数）。

### 1.4 flash-attention：Triton 单遍 _layer_norm_fwd_1pass_kernel（含 residual + RMS 融合）

来源：`flash_attn/ops/triton/layer_norm.py`（main 分支）
URL：https://raw.githubusercontent.com/Dao-AILab/flash-attention/main/flash_attn/ops/triton/layer_norm.py

- `_layer_norm_fwd_1pass_kernel`（L174-287）：**单遍**。`BLOCK_N = next_power_of_2(N)`（调用侧确定），整行一次 `tl.load` 进寄存器/共享内存：
  - `x = tl.load(X + cols, mask=cols < N, other=0.0).to(tl.float32)`（L228）——**mask + other=0 补零**。
  - residual 融合：`residual = tl.load(RESIDUAL + cols, mask=cols < N, other=0.0); x += residual`（L254-256）。
  - RMS 分支：`xbar = tl.where(cols < N, x, 0.0); var = tl.sum(xbar * xbar, axis=0) / N`（L265-266）——补零后归约，除以真实 N。
  - `rstd = 1 / tl.sqrt(var + eps)`（L267）后**直接用寄存器中的 x** 归一化输出（L276-279）——**无第二遍读回**，代价是 BLOCK_N 大时寄存器/共享压力（文件头注释 L6：hidden dim 到 8k 可接受，更大时 register spilling）。
  - bias 支持：`y = x_hat * w + b`（L277），bias 与 weight 同形状 [N]，广播语义是**逐列乘法/加法**。
- 说明：这是 flash-attention 训练路径（dropout/prenorm/并行 norm 等开关），本题只需子集（residual + rms + gamma + bias）。

### 1.5 Triton 官方教程 05-layer-norm：_layer_norm_fwd_fused（mask 尾块教学模板）

来源：`python/tutorials/05-layer-norm.py`（main 分支）
URL：https://raw.githubusercontent.com/triton-lang/triton/main/python/tutorials/05-layer-norm.py

- `_layer_norm_fwd_fused`（L48-95）：每行一个 program（`row = tl.program_id(0)`），`BLOCK_SIZE` 分块循环：
  - 归约：`_mean += tl.load(X + cols, mask=cols < N, other=0.).to(tl.float32)` → `mean = tl.sum(_mean, axis=0) / N`（L68-72）；方差同理（L74-80）。
  - **尾块 = mask + other=0 补零**，且 mask 在第二遍 `tl.where(cols < N, x - mean, 0.)`（L78）再次清零防止无效元素污染方差。
  - 输出写回 `tl.store(Y + cols, y, mask=mask)`（L95）。
- 教学要点：Triton 无显式 warp 概念，`tl.sum(axis=0)` 由编译器映射到块内归约；尾块处理完全靠 mask 谓词 + other 填零，且**归约后必须除以真实 N 而非块大小**。

### 1.6 llama.cpp：rms_norm_f32（两遍 + do_add/do_multiply 融合）

来源：`ggml/src/ggml-cuda/norm.cu`（master 分支）
URL：https://raw.githubusercontent.com/ggml-org/llama.cpp/master/ggml/src/ggml-cuda/norm.cu

- `rms_norm_f32`（L77-155）：**两遍**。`for (col = tid; col < ncols; col += block_size)` 线程步进累加 `tmp += xi * xi`（L131-134）；`block_reduce<SUM, block_size>`（L138，内部 warp shuffle + smem，见 ggml-cuda 公共头）；`mean = tmp / ncols; scale = rsqrtf(mean + eps)`（L140-141）；第二遍 `dst[col] = scale * x[col] * mul[mul_col] + add[add_col]`（L147）——**把 mul（gamma）与 add（bias/residual 级联）编译期融合进同一 kernel**，grid 用 blockIdx.x/y/z 映射 4D 形状（L101-112）。
- 这是**最简单的两遍模板**（无 Welford、无向量化、纯 f32），适合对照"最小实现"在 Ascend C 中的对应物。

---

## 2. 迁移差异表（GPU 机制 / Ascend C 对应机制 / 可迁移 / 不可迁移 / 风险）

| # | GPU 机制 | Ascend C 对应机制 | 可迁移部分 | 不可直接迁移部分 | 风险 |
| --- | --- | --- | --- | --- | --- |
| 1 | **warp shuffle / block reduce**（`__shfl_down_sync`、`cub::BlockReduce`、`block_reduce`）：32 线程寄存器交换归约 + smem 跨 warp 归约 | 向量单元 `ReduceSum`（fp16 每 repeat 128 元素、fp32 64，mask 模式限有效数）+ `workLocal` 工作区二叉树归约；跨块用标量累加（`GetValue`） | "分块部分和 → 分层归约 → 单值"的两级结构；**一律 FP32 累加**（GPU 与昇腾训练营黄金法则一致） | shuffle 依赖 32 线程寄存器通信，Ascend C 无线程概念，归约全部走 SIMD 指令 | ReduceSum 的 `workLocal` 需按 API 手册公式预留；`GetValue(0)` 标量读有同步开销；二叉树归约与 GPU 归约求和顺序不同 → 位级结果不同，只能比误差预算 |
| 2 | **atomicAdd / lock（Triton bwd 的 `tl.atomic_cas`/`atomic_xchg`、flash-attention 跨 CTA 归约）** | 昇腾有 atomic 指令但支持面窄、性能受限；**本题行切分后各行独立，无跨核归约需求** | 无（规避） | 跨 CTA/跨核共享累加 buffer + 自旋锁的模式 | 若照搬"块间原子累加"会退化为串行写热点；正确做法是把归约限制在行内，多核按行切分 |
| 3 | **动态 shared memory**（`extern __shared__` + `cudaFuncSetAttribute(MaxDynamicSharedMemorySize)`，运行时按 N 调整） | UB 由 `TPipe::InitBuffer` 显式分配固定 buffer（VECIN/VECOUT/VECCALC），无硬件自动管理，容量约 192-256KB 级（待 Agent 7 确认） | 容量预算思想：UB 预算 = smem 预算，按 D 与双缓冲份数规划；**大 N 必须分块**（GPU 侧同样有 48KB/block 上限） | 运行时按 N 自由伸缩 smem | UB 分配超限直接编译/运行失败；多 buffer 叠加（双缓冲×数据块）需精确计算，否则流水退化 |
| 4 | **线程级并行 + strip-mining**（`for (i = threadIdx.x; i < N; i += blockDim.x)` / `tid` 步进） | 单指令流：每 AI Core 一个执行流，向量指令按 repeat 处理整块（256B/次）；循环体现在"repeat 分块 + 逐块搬运" | 外层分块循环结构（把 N 切成若干 256B 块）与 grid-stride 外层行循环 | thread 索引 → repeat 索引的映射；无"多线程隐藏访存延迟"，靠双缓冲 + 指令级并行 | repeat 次数/边界计算错误（尤其尾块）会覆盖相邻行或漏算；块内顺序累加顺序固定，无法像 GPU 那样调线程数 |
| 5 | **向量化加载 float4 / `_f16Vec<8>`**（128-bit，16B 对齐，vLLM 要求指针%16==0 才走 vec8） | `DataCopy`/`DataCopyPad` 256B 块搬运；32B 对齐约束，非对齐走 DataCopyPad | 按 dtype 选搬运宽度、对齐时走整块快速路径的**分级策略** | 16B 粒度 vs 昇腾 32B/256B 粒度；"指针对齐检查选路径"的运行时 dispatch 需换成 tiling 阶段静态判定 | 行首地址 32B 未对齐（如行间 stride 非对齐）时 DataCopyPad 的开销与补 0 语义必须全程处理 |
| 6 | **mask 尾块**（`tl.load(mask=cols<N, other=0)`、predicated 访问，**不触碰越界内存**；flash-attention `Is_even_cols` 编译期分支 + 运行期有效数统计） | `DataCopyPad` 搬运非对齐尾块并**真实补 0 写入 UB**；归约侧用 mask 连续模式 `ReduceSum` 限定有效 repeat 元素数 | "尾块填零后归约、除法用真实 D"的数值策略完全等价（Triton other=0 与 DataCopyPad 补 0 同构）；**编译期 even/odd 双分支**思路可迁移到 tiling | GPU mask 是谓词（不写内存、不占带宽）；DataCopyPad 补 0 真实占用 UB 与搬入带宽 | 补 0 后若归约未限定有效元素，0² 虽无害，但**搬出尾块若不限长会覆盖相邻行**；补 0 空间在 UB 里必须预留 |
| 7 | **cooperative groups / `cudaLaunchCooperativeKernel`**（flash-attention `CTAS_PER_ROW>1` 跨 CTA 归约 + grid sync） | 昇腾多核无 grid-sync 等价物；核间通信受限（atomic 限制多） | 无 | 跨 CTA barrier 协作归约、同一行多核分摊 | **必须避免**：本题行切分（每行归约不跨核）即可，多核按 `GetBlockIdx/GetBlockNum` 切行 |
| 8 | **persistent kernel / grid-stride 循环**（flash-attention 按 occupancy 定 `ctas_per_col`，一个 block 循环处理多行；Triton 每 program 一行） | 每核 `for` 循环处理多行，`GetBlockIdx` 定行区间，尾核处理余数行 | 多行循环 + 行余数分摊模式（对应 Ascend C 多核均分行） | occupancy 自动推导 CTA 数（GPU 硬件调度）→ 昇腾核数是编译/启动时定死的 | 行数 < 核数时核间负载不均；行数余数分配边界算错会漏行/重算 |
| 9 | **双缓冲流水**（GPU：`cp.async`/`cudaMemcpyAsync` 异步拷贝 + 软件流水，与计算重叠） | `TQue`（TPosition::VECIN/VECOUT）`EnQue/DeQue` 队列双缓冲，CopyIn 与 Compute 自动重叠 | **分块 tiling + 双 buffer 交替**是本题性能主线，直接对应昇腾官方双缓冲模式 | GPU 流水靠流/硬件异步；昇腾靠队列显式管理，buffer 个数与队列深度需手动配 | buffer 份数不足退化为串行；D 小（如 67）时整行一块，双缓冲收益有限，需评估 67/129 这类小 D 的性能 |
| 10 | **in-place 存 y**（vLLM 把 y=x+residual 写回 residual buffer，第二遍从 residual 读，省一次 GM 读） | UB 中间 LocalTensor 存 y（**不能写回 GM 只读输入**——直调模板下 input/residual 是只读 GM 句柄） | "第一遍算 y 并存下来，第二遍直接用"的**单遍存 y** 思路（对应 flash-attention Triton 1pass 的寄存器存 x，昇腾换成 UB 存 y） | 就地覆盖输入 tensor | UB 容量限制 D 上限（fp32 4B×D；D=1000 需 4KB，D=32768 需 128KB+，可能超预算 → 必须退化为两遍分块）；y 在 UB 占一份、平方和归约还需额外空间 |
| 11 | **Welford 在线统计**（PyTorch 单遍同时算 mean/var，数值稳定） | 昇腾教学实现为 `Cast→Mul→ReduceSum`（平方和直接累加） | RMS 分支不需要 mean，Welford 的数值优势对 RMS 无关紧要；**RMS 只需 sum(y²)** | Welford 需要多状态（mean/var/count）在线更新，Ascend C SIMD 归约下收益有限 | 平方和溢出：fp16 输入平方先转 fp32 再累加是**必须**（GPU 与昇腾一致）；Kahan 补偿仅当判题误差预算吃紧时考虑 |
| 12 | **BF16/FP16 转换**（GPU：输入 `static_cast<float>` 精确扩展，输出 `static_cast<bf16>`/`__float2bfloat16` 按 round-to-nearest-even） | 昇腾 `Cast` 指令：bf16/fp16→fp32 无损（CAST_NONE），fp32→bf16/fp16 用 `CAST_RINT` 等模式 | **fp32 中间累加 + 末尾单次 cast 回原 dtype**的策略可迁移（所有 6 个 GPU 实现一致） | RNE 与 CAST_RINT 的逐位等价性**未在 NPU 验证**，不得声称一致 | bf16 尾数 8 位，输出相对误差下限 ~2^-8≈3.9e-3；判题按 PyTorch 同 dtype 基准对比（见 research-report.md 第 4 节），对齐基准的舍入行为比追求绝对误差更重要 |

---

## 3. 对本题可借鉴的具体算法

### 3.1 方案 A（首选，小 D）：单遍存 y —— flash-attention Triton 1pass / vLLM in-place 的昇腾化

- 思路来源：flash-attention `_layer_norm_fwd_1pass_kernel`（L228-279）把整行 x（含 residual）留在寄存器，算完 rstd 后直接归一化；vLLM `fused_add_rms_norm_kernel`（L133-173）把 y 写回 buffer 供第二遍用。
- 昇腾对应：`D × 4B(fp32) ≤ UB 预算` 时——
  1. CopyIn：DataCopyPad 搬入 x、residual 块（含 gamma、bias）；
  2. 计算：Cast fp32 → Add（y=x+residual）→ **y 存 UB 中间 buffer** → Mul（y²）→ ReduceSum → 标量 `rstd = rsqrt(sum/D + eps)`；
  3. 输出：UB 中 y × rstd × gamma + bias → Cast 回 dtype → CopyOut（尾块 DataCopyPad）。
- 收益：**省掉第二遍 GM 读 x+residual**（内存带宽瓶颈下接近 2 倍读量节省）；与 vLLM 注释"memory-latency bound"结论一致（vLLM L330-332）。
- 边界：D=1000 时 y 占 4KB，UB 无压力；D=32768 时 128KB 已超典型 UB 预算 → 退化为方案 B。
- 注意：不能用 vLLM 的 in-place 写回 GM 输入（直调模板输入只读）；中间 y 只能放 UB。

### 3.2 方案 B（大 D）：两遍分块 + 块间标量累加 —— PyTorch / llama.cpp / flash-attention CUDA 的昇腾化

- 思路来源：PyTorch `RowwiseMomentsCUDAKernel`（L80-88）、llama.cpp `rms_norm_f32`（L131-138）的两遍结构；flash-attention `save_x`（L50/L137）的"第一遍顺带存 y"。
- 昇腾对应：每核按行切分；行内第一遍逐块 `CopyIn → Cast → Add → Mul → ReduceSum`，块间标量累加得 `sum_y2`，行末 `rstd = rsqrt(sum_y2/D + eps)`；第二遍重读 x、residual 归一化输出。
- 优化点（GPU 侧启示）：第一遍的 y 若能留一部分在 UB（如分块小、能留最近一块），第二遍首块可复用——这是"伪单遍"折中；flash-attention 用 GM workspace 存 x（L137），昇腾若判题允许额外 workspace 同样可行，**但首选 UB 方案，不依赖 workspace 权限**。

### 3.3 双缓冲 tiling —— GPU cp.async 流水 / Triton 分块循环的昇腾化

- 思路来源：Triton 教程分块循环（L68-95）+ 昇腾 TQue 官方双缓冲（research-report.md 第 2 节）。
- 昇腾对应：行内 D 大时分段（每段 ≤ UB 预算/双缓冲份数），`TQue VECIN` 双 buffer 交替：CopyIn(块 k+1) 与 Compute(块 k) 重叠；CopyOut 走 VECOUT 队列，同样双缓冲。
- 适用性：D 大（1000/32768）时收益明显；D=67/129 时整行一块、双缓冲流水意义小，按"每核多行并行"策略（方案 A/B 的行循环天然提供并行度）。

### 3.4 尾块处理 —— Triton mask+other=0 与 DataCopyPad 补 0 的同构

- 数值上完全同构：Triton `tl.load(mask=cols<N, other=0.0)`（L228/L255）对应 DataCopyPad 补 0；Triton `var = tl.sum(xbar*xbar)/N` 用真实 N 除（L266）对应昇腾 `rstd = sqrt(sum/D + eps)` 用 D 除（补 0 的 0² 不改变和）。
- 额外注意（昇腾特有）：搬出尾块必须 `DataCopyPad` 限定长度，否则覆盖相邻行（GPU 端 store 有 mask 不会越界，昇腾没有谓词 store）。

### 3.5 编译期 even/odd 双分支 —— flash-attention `Is_even_cols` 的昇腾化

- flash-attention 对 `cols == HIDDEN_SIZE` 编译期特化、去掉尾块判定（L228）；昇腾 tiling 阶段同样可在 D % 256B == 0 时走无尾块路径（纯 DataCopy + 无 mask ReduceSum），否则走 DataCopyPad + mask 归约路径——两条 kernel 分支用 tiling 参数区分，避免运行期逐块分支开销。

---

## 4. 绝不能直接照搬的 GPU 技巧及原因

1. **线程/块网格映射**（`grid=num_tokens, block=1024`，vLLM L256-260；Triton `program_id(0)`）：昇腾无线程与 CTA 概念，并行单元是 AI Core；行切分用 `GetBlockIdx/GetBlockNum`，且核数（个位数到数十）远小于 GPU block 数（数千），映射策略完全不同。
2. **warp shuffle / cub::BlockReduce**（PyTorch L229、vLLM L68、llama.cpp L138）：寄存器级线程归约在昇腾无对应物；必须改用向量 `ReduceSum` + workLocal。GPU 归约顺序（shuffle 树）与昇腾二叉树归约的求和顺序不同 → **位级结果不可移植**，只能保证同误差预算。
3. **atomicAdd / 自旋锁归约**（Triton tutorial L177-194；flash-attention 多 CTA 归约）：昇腾 atomic 支持受限；本题按行切分天然避免跨核归约，不需要也不应该用。
4. **cooperative groups / grid sync**（flash-attention L260-267）：昇腾无 grid 级同步，同一行分给多核需要跨核通信 → 直接否掉该路线。
5. **动态 shared memory 按运行时 N 伸缩**（flash-attention L254-256）：昇腾 UB 必须 tiling 期静态规划；"运行时才知道 N"在 Direct Invocation 模板下由 tiling 函数先算好再进 kernel，不能 kernel 内动态分配。
6. **mask 谓词的"内存安全"假设**：GPU 上 mask 加载/存储不触碰越界内存、不产生带宽；昇腾 DataCopyPad 补 0 会真实写 UB 且占搬入带宽，**搬出必须限长**。把 GPU 的 mask 想当然成"零成本"会低估尾块开销。
7. **float4/_f16Vec<8> 16B 对齐快速路径**（vLLM L344-372）：昇腾对齐粒度是 32B（DataCopy）/256B（块），且非对齐行首只能 DataCopyPad；GPU 的"16B 对齐即可向量化"前提不成立。
8. **Welford 在线统计**（PyTorch L206-268）：RMS 分支不需要均值，Welford 三状态更新在昇腾 SIMD 归约下是纯负担；直接用 `Cast→Mul→ReduceSum`。
9. **Triton autotune（num_warps 等）**（flash-attention Triton L163-166）：昇腾无 warp 概念，性能调参是 repeat/双缓冲份数/tiling 块大小，机制完全不同。

**总判断**：GPU 实现的**算法结构与数值策略**（两遍/单遍存 y、FP32 累加、尾块补零+真实 D 除法、行切分、双缓冲 tiling、编译期 even/odd 分支）可以直接迁移到 Ascend C，且这些策略与昇腾官方训练营/ops-transformer 的推荐一致，可作交叉印证；但**执行机制层**（SIMT 线程模型、寄存器/共享内存、warp 归约、grid 同步、atomic）**绝不能照搬**。任何"把某 GPU kernel 直接翻译成 Ascend C 即可用"的判断都应否定；正确路径是：以 GPU 实现为算法参考，按昇腾的搬运/归约/UB 模型重写，再在真实 NPU 上验证（本轮全部未验证）。

---

## 5. BF16 专项结论

- GPU 侧统一模式（PyTorch/vLLM/flash-attention/llama.cpp/Triton）：输入 fp16/bf16 → `float`（精确扩展）→ fp32 平方与累加 → 输出 `static_cast<dtype>`（CUDA 默认 RNE 舍入）。与昇腾"归约前必须转 FP32"黄金法则完全一致，**该策略可迁移**。
- fp16 平方溢出风险：`65504²` 即溢出，fp32 累加是硬性要求（GPU 与昇腾同理）。
- 输出舍入：CUDA `__float2bfloat16` 为 RNE；昇腾 `CAST_RINT` 的逐位等价性**未在 NPU 验证**——若判题基准为 PyTorch bf16 结果，应优先让输出舍入行为对齐基准 dtype，真机对比误差分布后再决定是否用 `CAST_RINT`（与 research-report.md 第 4 节结论一致）。
- bf16 精度天花板：8 位尾数 → 输出相对误差下限 ~2^-8≈3.9e-3，任何实现都无法低于该上限；判题误差语义（相对/绝对、与同 dtype 基准比）需在真机确认。

---

## 6. 本轮新增来源清单

| # | 名称 | URL / 仓库 | 版本 | 访问日期 | 用途 | 证据等级 | 是否真实 NPU 验证 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | PyTorch layer_norm_kernel.cu（RowwiseMoments/vectorized kernel） | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu （raw 同上路径） | main（2026-09-11 抓取） | 2026-09-11 | 两遍 Welford、warp shuffle+smem 归约、rms_norm 分支、fp32 累加 | A（官方仓库源码） | 未验证 |
| 2 | vLLM layernorm_kernels.cu（rms_norm_kernel / fused_add_rms_norm_kernel） | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/layernorm_kernels.cu | main（2026-09-11 抓取） | 2026-09-11 | residual 融合 RMSNorm、in-place 存 y、vec8 向量化、batch-invariant 归约顺序 | A（官方仓库源码） | 未验证 |
| 3 | flash-attention csrc/layer_norm/ln_fwd_kernels.cuh（ln_fwd_kernel） | https://github.com/Dao-AILab/flash-attention/blob/main/csrc/layer_norm/ln_fwd_kernels.cuh | main（2026-09-11 抓取） | 2026-09-11 | 特化矩阵、save_x 中间 buffer、跨 CTA 协作、persistent grid-stride | A（官方仓库源码） | 未验证 |
| 4 | flash-attention flash_attn/ops/triton/layer_norm.py（_layer_norm_fwd_1pass_kernel） | https://github.com/Dao-AILab/flash-attention/blob/main/flash_attn/ops/triton/layer_norm.py | main（2026-09-11 抓取） | 2026-09-11 | 单遍存 x、residual+RMS+bias 融合、mask+other=0 尾块 | A（官方仓库源码） | 未验证 |
| 5 | Triton 官方教程 05-layer-norm | https://github.com/triton-lang/triton/blob/main/python/tutorials/05-layer-norm.py | main（2026-09-11 抓取） | 2026-09-11 | tl.sum 块内归约、mask 尾块、真实 N 除法、atomic 反向参考 | B（官方教程） | 未验证 |
| 6 | llama.cpp ggml/src/ggml-cuda/norm.cu（rms_norm_f32） | https://github.com/ggml-org/llama.cpp/blob/master/ggml/src/ggml-cuda/norm.cu | master（2026-09-11 抓取） | 2026-09-11 | 最小两遍模板、block_reduce、mul/add 融合、4D grid 映射 | A（官方仓库源码） | 未验证 |
| 7 | NVIDIA Apex layer_norm | https://github.com/NVIDIA/apex/tree/master/csrc/layer_norm | master（未下载，仅对照） | 2026-09-11 | 与 PyTorch 同源（Welford+两遍），未深入 | A（未读取正文） | 未验证 |
| 8 | Unsloth kernels/utils.py | https://github.com/unslothai/unsloth/blob/main/unsloth/kernels/utils.py | main（2026-09-11 抓取） | 2026-09-11 | 检索后确认**不含** rmsnorm（仅 gemv 等），未采用 | A（已检索，未采用） | 未验证 |

> 所有源码均为 raw.githubusercontent.com 抓取，快照在 `/tmp/gpu_research/`。已登记至 `sources.md`（追加编号 54-61）。全部条目：未在真实 NPU 验证。

---

## 7. 遗留事项（需其他 Agent / 真机确认）

1. UB 实际容量与双缓冲份数预算（Agent 7 负责）→ 决定方案 A（单遍存 y）的 D 上限。
2. `CAST_RINT` 与 CUDA RNE 的逐位等价性（Agent 2/真机）。
3. 判题误差判定语义（相对/绝对、与同 dtype PyTorch 基准比）→ 决定 bf16 舍入对齐策略。
4. Direct Invocation 模板下 GM 输入是否只读、能否申请额外 workspace（影响"存 y 到 GM"路线，当前首选 UB 方案不依赖）。
