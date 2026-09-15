# Agent 04：GPU/CUDA/Triton/PyTorch RMSNorm 实现及 Ascend C 迁移映射

> 日期：2026-09-11
> 题目：AddRmsNormBias（CANN 9.0.0 / vector kernel / 直调模板）
> 性质：GPU 侧调研 + 迁移参考。**不产出 Ascend C 可提交代码，不声称 NPU 验证。**
> 语义锁定：`y = x + residual`；`rms = sqrt(mean(y^2) + eps)`；`out = y / rms * gamma + bias`。

---

## 0. 边界声明

- 本文所有 GPU 实现只作算法与工程结构参考，禁止把任何 GPU 代码标为「Ascend C 可用」。
- Ascend C 侧对应机制按项目约定写：GM/UB 两级存储、`DataCopy`/`DataCopyPad`、`TQue`/`TBuf`、`ReduceSum`、`Cast` FP32、多核 `GetBlockIdx`/`blockIdx`。
- CUDA shared memory / warp shuffle / `__syncthreads` / float4 向量化 load 在 Ascend C 无直接对应物。
- 本机无 CANN、无 Ascend C 编译器、无昇腾 NPU。

---

## 1. 代表实现摘要

### 1.1 NVIDIA Apex（CUDA，layer_norm_cuda_kernel.cu）

来源：`NVIDIA/apex` `csrc/layer_norm_cuda_kernel.cu`，已读原文。

| 维度 | 做法 |
| --- | --- |
| 并行模型 | 一个 block 处理一行：`blockDim = (32, 4, 1)`，`blockIdx.y` 网格遍历行；`gridDim.y = min(n1, maxGridSize[1])` |
| 归约 | 每线程本地累加平方和（RMS 路径：`sigma2 += curr*curr`），intra-warp 用 `WARP_SHFL` 树规约，inter-warp 用 shared memory 写半数再 `__syncthreads` 合并 |
| 累加精度 | `accscalar_t = at::acc_type<scalar_t, true>`：fp16/bf16 → float 累加 |
| 尾块 | 循环 `for (l = 4*thrx; l+3 < n2; l += 4*numx)` + 尾标量循环；不依赖 N 对齐 |
| epilogue | `c_invvar = rsqrt(sigma2 + epsilon)`；`out = gamma * (c_invvar * curr)`（rms_only 路径） |
| 输出 | 直接写回 `invvar` 供 backward 使用；forward 不缓存中间 y |

要点：Apex 用 Welford 在线求和仅是为了和 LayerNorm 共用代码；纯 RMS 路径退化为简单平方累加。**没有 residual add**，也没有 bias epilogue。

### 1.2 PyTorch 原生 CUDA（layer_norm_kernel.cu）

来源：`pytorch/pytorch` `aten/src/ATen/native/cuda/layer_norm_kernel.cu`，已读原文。

| 维度 | 做法 |
| --- | --- |
| 快路径条件 | `N % vec_size == 0`（vec_size=4）且指针 16B 对齐且 `N <= 2^24`，走 `vectorized_layer_norm_kernel` |
| 向量化 | `aligned_vector<T, 4>` 重解释，`float4`/`half4` 整读整写 |
| 归约 | Welford + warp shuffle + shared memory inter-warp；RMS 路径 `cuWelfordOnlineSum` 只累加 `val*val` |
| 慢路径 | 拆成两个 kernel：`RowwiseMomentsCUDAKernel`（先算 mean/rstd 写 GM）+ `LayerNormForwardCUDAKernel`（再读回做归一化） |
| 累加精度 | 全程 `T_ACC`（fp16/bf16 → float） |
| 尾块 | 快路径要求 N 是 4 的倍数（否则退回慢路径标量循环）；慢路径任意 N |
| epilogue | `rstd = rsqrt(m2 + eps)`；RMS：`Y = rstd * X * gamma` |
| residual | **无**。residual 在 PyTorch 图上是独立 Add |

要点：PyTorch 的「向量化快路径 + 标量慢路径」双轨制，是 Ascend 侧「对齐 tile + DataCopyPad 尾块」的直接类比。慢路径两遍 GM 往返对应本项目 v1 两遍扫描路线。

### 1.3 Triton 官方 LayerNorm 教程（05-layer-norm）

来源：`triton-lang.org` Tutorial 05，已读原文。

| 维度 | 做法 |
| --- | --- |
| 并行模型 | `grid = (M,)`，`row = tl.program_id(0)`，一行一个 program |
| 归约 | `tl.sum(x * x, axis=0)` 单指令整行归约（BLOCK_SIZE 内） |
| tile | `BLOCK_SIZE = min(MAX_FUSED_SIZE, next_power_of_2(N))`；`MAX_FUSED_SIZE = 65536 / element_size`（fp16 → 32768） |
| 尾块 | `mask = cols < N`，`tl.load(..., other=0.)` 补零后参与归约，天然正确 |
| 累加精度 | `_mean`/`_var` 用 `tl.float32`，load 后 `.to(tl.float32)` |
| 教程限制 | 明确 `if N > BLOCK_SIZE: raise RuntimeError("feature dim >= 64KB")` |
| num_warps | `min(max(BLOCK_SIZE // 256, 1), 8)` 启发式 |

要点：Triton 教程是**两遍扫描**（先 mean，再 var，再归一化），因为 LayerNorm 需要 mean。RMSNorm 可压成一遍归约。教程 BLOCK_SIZE 上限 64KB 寄存器/共享预算，与 Ascend UB 容量约束同构。

### 1.4 Liger-Kernel `rms_norm.py` / `fused_add_rms_norm.py`（Triton）

来源：`linkedin/Liger-Kernel` `src/liger_kernel/ops/rms_norm.py` 与 `fused_add_rms_norm.py`，已读原文。

| 维度 | 做法 |
| --- | --- |
| 融合 residual | `S_row = X_row + R_row` → `tl.store(S_ptr)`（更新 residual）→ `mean_square = tl.sum(S_row * S_row, axis=0) / n_cols` → `rstd = rsqrt(...)` → `Y = S_row * rstd * (offset + W_row)` |
| 语义差异 | Liger 的 `offset` 是加在 **weight** 上：`(x/rms) * (W + offset)`。本题 bias 是加在 **归一化之后**：`(x/rms)*gamma + bias`。**不可直接套用** |
| casting_mode | `llama`：只 rstd 用 fp32，乘 weight 回原 dtype；`gemma`：全程 fp32 再 cast 回；`none`：全程原 dtype。本题应取 gemma 等价策略（全程 FP32 中间计算） |
| 尾块 | `mask = col_offsets < n_cols`，`other=0` 补零 |
| 块变体 | `_block_rms_norm_forward_kernel`：一次处理 `BLOCK_ROW=16` 行，`tl.sum(..., axis=1)` 沿行归约，适合 M 极大时提高单核吞吐 |
| 启发式 | `BLOCK_SIZE, num_warps = calculate_settings(n_cols)`；当 `n_rows < 4096*8` 或 `BLOCK_SIZE > 256` 时退回一行一 program |
| backward | dW 用 `sm_count` 个 partial buffer + `.sum(0)`，与 forward 无关 |

要点：Liger 是与本题最接近的 GPU 实现（residual add + RMSNorm 融合），但 epilogue 语义不同（weight-offset vs post-bias）。它展示了「单 program 读 x 和 residual → 融合 add → 归约 → epilogue → 写 y」的完整数据流，可直接映射为 Ascend 单遍路线的数据依赖图。

### 1.5 flash-attention `ln_api.cpp`（CUDA，DropoutAddLayerNorm）

来源：`Dao-AILab/flash-attention` `csrc/layer_norm/ln_api.cpp`，已读原文。

| 维度 | 做法 |
| --- | --- |
| 融合范围 | Dropout + residual add + LayerNorm/RMSNorm（`is_rms_norm` 开关）一次 kernel |
| 类型组合 | compute 始终 fp32；input/residual/weight/output 可独立选 fp16/bf16/fp32；有 residual 可为 fp32 而 input 为 fp16 |
| 形状约束 | `hidden_size % 8 == 0 && hidden_size <= 8192`（编译期按 hidden_size 取模实例化） |
| residual 处理 | residual 指针可选；有 residual 时 kernel 内完成 `x = x0 + residual` 并可写回 |
| beta | LayerNorm 路径有 beta（归一化后加）；RMSNorm 路径 `is_rms_norm=true` 时跳过 mean/beta |
| hidden 对齐取整 | `round_multiple(hidden_size, 256/512/1024)`，按 hidden 分档选择 launcher |

要点：flash-attn 把 residual add 放进 kernel 主循环，与本题语义完全一致。它的 beta（归一化后加）与本题 bias 位置相同，但 flash-attn 的 RMSNorm 路径通常不带 beta——本题是「RMSNorm + post-bias」的特殊组合，GPU 生态无一字不差的现成实现，必须自己拼。

### 1.6 共性算法骨架（跨实现提炼）

```
// 伪代码：GPU 生态 RMSNorm + residual 的共同骨架
for each row i:                          // program_id / blockIdx
    acc = 0                              // FP32
    for j in tile(row_i):                // strided loop 或 单 tile
        y = x[j] + residual[j]           // 原 dtype 或 FP32
        acc += float(y) * float(y)       // FP32 平方累加
    // 归约：warp shuffle / tl.sum / ReduceSum
    ms = acc / D
    rstd = rsqrt(ms + eps)               // 或 1/sqrt(ms+eps)
    for j in tile(row_i):
        y = x[j] + residual[j]
        out[j] = float(y) * rstd * float(gamma[j]) + float(bias[j])
        // 或 cast 回原 dtype 后写
```

单遍 vs 两遍：单遍需在 UB/寄存器暂存整行 y（Liger、apex forward）；两遍则 GM 往返两次（PyTorch 慢路径、Triton 教程）。D 超出片上容量时只能两遍。

---

## 2. 算法差异（GPU 实现之间）

| 差异点 | Apex | PyTorch | Triton 教程 | Liger | flash-attn |
| --- | --- | --- | --- | --- | --- |
| residual add | 无 | 无（图上独立 Add） | 无 | 有，kernel 内融合 | 有，kernel 内融合 |
| post-bias | 无（LayerNorm beta 另论） | 无 | 有（LayerNorm beta） | 无（用 weight-offset） | LayerNorm 路径有 beta；RMS 路径无 |
| 归约指令 | warp shuffle + shmem | warp shuffle + shmem | `tl.sum` | `tl.sum` | 手写 CTA 内归约 |
| 尾块策略 | 标量尾循环 | 快路径要求 N%4==0，否则慢路径 | mask + other=0 | mask + other=0 | 要求 hidden%8==0 |
| D 上限 | 无硬限（循环） | 无硬限 | 64KB/element | BLOCK_SIZE 启发式 | 8192（编译期） |
| 累加精度 | fp16/bf16→fp32 | fp16/bf16→fp32 | 显式 fp32 | 可选（llama/gemma/none） | 始终 fp32 |
| 单遍/两遍 | 单遍（不存 y） | 快路径单遍；慢路径两遍 | 两遍（LN 需要 mean） | 单遍（存 y 与 residual） | 单遍 |
| bias/offset 语义 | — | — | post-add | weight-offset | post-add（仅 LN） |

与本题语义最近的是 **Liger 融合骨架 + flash-attn post-bias 位置** 的组合：residual add 融合、FP32 归约、`out = y*rms^{-1}*gamma + bias`。

---

## 3. 差异映射表（GPU → Ascend C）

| GPU 机制 | Ascend C 对应机制 | 可迁移部分 | 不可直接迁移部分 | 风险 |
| --- | --- | --- | --- | --- |
| `__shared__` shared memory | UB（Unified Buffer），`TBuf`/`TQue` 管理 | 「片上暂存中间 y / 部分和」的算法意图 | 容量、bank、同步语义完全不同；UB 约 192KB 量级需自行规划 | UB 规划不当导致 tile 过小或溢出 |
| `WARP_SHFL` / warp shuffle 归约 | `ReduceSum` API；或手工向量累加 + `GetValue` | 「分段累加 → 最终归一」的两段式结构 | 无 lane/warp 概念；ReduceSum 有 count 上限（争议：4096 / 16320） | ReduceSum workLocal 不足或 count 超限 |
| `__syncthreads()` | 标量同步 / 流水线阶段由 Ascend C 编排器处理 | 「归约完成后再 epilogue」的阶段划分 | 不需要手写 barrier；但 `GetValue` 触发的 V_S 同步有性能代价 | 大行数时 GetValue 频繁导致性能退化 |
| `tl.sum(x*x, axis=0)` | `ReduceSum`（FP32） | 「整行平方和一次归约」的语义 | Triton 的 axis 归约隐含跨线程；Ascend 是向量内归约，需显式分块 | 分块边界漏加 |
| `blockIdx.x` = 行号 | `GetBlockIdx()` / 直调模板 `blockIdx` | 「按行分核」的并行划分 | GPU 一 block 可多线程协作一行；Ascend 一核通常独占若干整行 | 多核按行均分时尾核负载不均 |
| `dim3 blocks(M)` grid | `availableCoreNum` + 行块切分 | 「行数 > 核数时网格扩展」的思路 | Ascend 核数固定（A2 约 20~40）；需 `blockIdx` 映射行区间 | 行基址 32 位溢出（必须 `uint64_t`） |
| `aligned_vector<float,4>` / float4 向量化 load | `DataCopy`（32B 对齐）/ `DataCopyPad`（任意长度） | 「按固定宽度整块搬运」的带宽思路 | GPU 用指针重解释；Ascend 用 DMA 引擎，有对齐/长度约束 | DataCopy 要求 32B 对齐；非对齐必须 Pad |
| 尾块 `mask + other=0` | `DataCopyPad` 搬入 padding=0；向量计算连续 mask | 「补零不污染归约」的正确性论证 | GPU 是寄存器级 mask；Ascend 是 DMA padding + 指令 mask 两层 | 搬出方向 padding 可能覆盖相邻行（文档矛盾，真机必验） |
| fp16/bf16→fp32 累加（`acc_type`） | `Cast` 到 `float` 再计算；输出前 `Cast` 回原 dtype | 「归约全程 FP32、末尾一次 cast」的黄金法则 | A2 的 Add/Mul **不支持 bfloat16_t**，必须在 FP32 域完成所有加乘 | bf16 路径若漏 Cast 直接编译失败或静默错值 |
| `rsqrt(ms + eps)` | `Sqrt` 后再 `Div`；或 `Rsqrt`（若头文件提供） | 「eps 加在开方前」的顺序 | Ascend 向量 Sqrt 为 0 ulp；标量 `sqrtf` 精度可能不足 bf16 | 标量 sqrtf 在大 D bf16 下误差放大 |
| `num_warps` / `num_stages` 启发式 | UB 分块大小 + `TQue` 深度（双缓冲） | 「按 D 选 tile」的分档思想 | GPU 的 warp/stage 模型无对应；Ascend 用 MTE/Vector 流水 | 直接照搬 tile 数值会打爆 UB |
| 单遍暂存整行 y | UB 中保留 y 的 FP32 副本 | 「省一次 GM 读」的带宽收益 | UB 容量限制：D≤4096（fp32 y 约 16KB/行）可行；D=32768 必须两遍 | 单遍在大 D 时 UB 溢出 |
| residual add 融合（Liger/flash-attn） | Kernel 内 `Add`（FP32 域） | 「x 与 residual 一次读入后融合」完全可迁移 | — | 低。这是本题必须路径 |
| post-bias epilogue | 归一化后 `Add(bias)`，bias 常驻 UB | 「gamma 与 bias 只搬一次」 | GPU 的 offset 加在 weight 上，语义不同 | 误把 bias 加成 `gamma+bias` 会全错 |
| rstd 缓存到 GM（backward 用） | 不需要 | — | 本题无 backward | 无 |
| hidden_size 编译期实例化（flash-attn） | 运行时 D + tiling 分支 | 「按 D 分档选策略」 | flash-attn 限 8192；本题 D 到 32768 | 硬套 8192 上限会漏大 D |
| `tl.load(..., other=0)` | `DataCopyPad` paddingValue=0 | 「补零参与归约安全」的论证 | — | paddingValue 传参位置错填（V002 已踩） |

---

## 4. 十四条方案路线中受 GPU 启发的子集评价

覆盖计划 §2 的 14 条中，与 GPU 证据直接相关的子集：

| 路线 | GPU 证据来源 | 算法类比 | Ascend 落点 | 推荐级别 |
| --- | --- | --- | --- | --- |
| 1. 两遍扫描 | PyTorch 慢路径；Triton LN 教程 | 先归约写中间，再读回归一化 | Pass1 `ReduceSum` 平方和；Pass2 归一化+gamma+bias 写回 | **首选（正确性基线）** |
| 2. 单遍暂存中间 y | Liger fused_add_rms_norm；apex forward | UB 内保留 y，省一次 GM 读 | D≤4096 时激活；D>4096 回退路线 1 | **可作第二路线** |
| 3. FP32 全中间计算 | flash-attn `ctype=fp32`；Liger gemma 模式；apex `acc_type` | 归约与累加全程 FP32 | Cast→FP32→算→Cast 回 | **首选（强制）** |
| 4. 低精度中间计算 | Liger `casting_mode=none` | 全程原 dtype，省带宽但增误差 | 仅作对照实验，不得进提交 | **不建议** |
| 5. 大 tile | Triton `BLOCK_SIZE = next_power_of_2(N)`，上限 64KB | 单 program 一次吃满一行 | UB 容量内尽量大 tile；受 ReduceSum count 上限约束 | **可作第二路线** |
| 6. 小 tile | Triton 按 64KB 上限拆；Liger BLOCK_SIZE 启发式 | 分块归约 + 块间合并 | D>4096 时分块 ≤4096 | **首选（大 D 必须）** |
| 7. 按行分配 AI Core | Apex `gridDim.y=n1`；Triton `grid=(M,)`；Liger `program_id=row` | 一行一核（或一核多行） | `GetBlockIdx` 行区间切分 | **首选** |
| 8. 按 tile 分配 AI Core | 无直接 GPU 对应（GPU 不按列切行） | — | 不适用（本题归约在行内，跨核切列需额外通信） | **不建议** |
| 9. DataCopyPad 尾块 | Triton/Liger `mask + other=0` | 补零不污染归约 | 搬入 Pad 补 0；搬出 Pad 指定 valid_len | **首选（非 32 倍数 D）** |
| 10. 手工尾块 | PyTorch 标量尾循环；apex 尾标量 | 整块 + 标量尾 | 对齐部分 `DataCopy`，尾部标量/窄 mask | **可作第二路线** |
| 11. ReduceSum | `tl.sum`；warp shuffle + shmem | 整行一次归约 | `ReduceSum` + `GetValue(0)` | **首选** |
| 12. 手工向量归约 | apex 线程内累加后 shmem 合并 | 分段累加 + 标量合并 | UB 内 Mul→分段 ReduceSum→标量加 | **可作第二路线** |
| 13. 纯 Ascend C | — | — | 不依赖 GPU 代码 | **首选（工程主线）** |
| 14. CUDA/Triton 迁移参考 | 本文全部内容 | 数据流与精度策略可迁；指令/并行模型不可迁 | 只借算法结构，不抄实现 | **仅作研究参考** |

### 受 GPU 启发的关键判断

1. **residual add 必须融合进 kernel**：Liger 与 flash-attn 均在 kernel 内完成 add，不先写回 GM。本题同样应 `y = x + residual` 在 UB 内完成后再归约。
2. **post-bias 不能照搬 Liger 的 weight-offset**：Liger 是 `(x/rms)*(W+offset)`，本题是 `(x/rms)*gamma + bias`。flash-attn 的 LayerNorm beta 位置才对，但 flash-attn 的 RMSNorm 路径通常不带 beta。需自行拼接。
3. **FP32 中间计算是跨实现共识**：apex、PyTorch、flash-attn、Liger(gemma) 均如此。bf16 路径尤其强制。
4. **单遍 vs 两遍的切换点在片上容量**：GPU 以 64KB/element 或寄存器压力为界；Ascend 以 UB 容量为界（D≤4096 单遍可行，与本项目文档 v2 路线一致）。
5. **尾块补零安全**：Triton/Liger 的 `other=0` 论证与 `DataCopyPad` paddingValue=0 同构——补零不改变平方和。但**搬出方向**不能套用（可能覆盖相邻行）。

---

## 5. 已确认 / 未找到 / 无法确认

### 5.1 已确认（源码原文核对）

- Apex：RMS 路径为平方在线累加 + warp shuffle + shared memory 合并；`acc_type` FP32；无 residual、无 post-bias。
- PyTorch：快路径要求 `N % 4 == 0` 且 16B 对齐；慢路径两遍 GM 往返；RMS 与 LN 共用 Welford 骨架。
- Triton 教程：`BLOCK_SIZE = min(65536/elem_size, next_power_of_2(N))`；`N > BLOCK_SIZE` 直接 raise；mask + other=0 补零。
- Liger `fused_add_rms_norm`：kernel 内 `S = X + R` → store residual → `tl.sum(S*S)/n` → `rsqrt` → `Y = S * rstd * (offset + W)`；offset 在 weight 上。
- Liger casting_mode：llama（只 rstd fp32）/ gemma（全 fp32）/ none（全原 dtype）。
- flash-attn：`ctype` 始终 fp32；residual 可选且可为 fp32；`hidden_size % 8 == 0 && <= 8192`；`is_rms_norm` 开关；LayerNorm beta 在归一化后加。
- 与本题语义的差异：GPU 生态无「RMSNorm + residual + post-bias」一字不差的现成实现；最接近的是 Liger 融合骨架 + flash-attn beta 位置的拼接。

### 5.2 未找到

- GPU 侧针对本题完整语义（residual + RMSNorm + post-bias）的单一开源 kernel。
- Ascend C 与 CUDA warp/shuffle 的官方逐指令对照表（不存在；映射只能按语义类比）。
- GPU MODE / NVIDIA Forums 上与本题 D 范围（64–32768）完全对齐的 RMSNorm benchmark 数据。

### 5.3 无法确认

- 本报告中的任何 GPU 算法在 Ascend C / CANN 9.0.0 上的编译可行性与性能——本机无 CANN/NPU。
- `DataCopyPad` 搬出方向是否真实丢弃 dummy（官方文档与社区实测冲突，见主线文档）——与 GPU 无关，但是迁移落地的第一验证项。
- 判题端 15 个测试点是否含大 D（32768）单遍不可行场景——决定路线 2 是否需要强制回退。

---

## 6. 来源清单

| # | 标题 / 路径 | URL | 级 | 访问日期 | 用途 | 证据状态 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | NVIDIA apex `layer_norm_cuda_kernel.cu` | https://raw.githubusercontent.com/NVIDIA/apex/master/csrc/layer_norm_cuda_kernel.cu | B | 2026-09-11 | CUDA Welford/RMS 归约、warp shuffle、shared memory 结构 | verified（原文已读） |
| 2 | Liger-Kernel `fused_add_rms_norm.py` | https://raw.githubusercontent.com/linkedin/Liger-Kernel/main/src/liger_kernel/ops/fused_add_rms_norm.py | B | 2026-09-11 | residual+RMSNorm 融合数据流、casting_mode、mask 尾块 | verified（原文已读） |
| 3 | Liger-Kernel `rms_norm.py` | https://raw.githubusercontent.com/linkedin/Liger-Kernel/main/src/liger_kernel/ops/rms_norm.py | B | 2026-09-11 | 单行/block 两种 kernel、FP32 归约、rstd 缓存 | verified（原文已读） |
| 4 | Triton Tutorial 05 Layer Normalization | https://triton-lang.org/main/getting-started/tutorials/05-layer-norm.html | B | 2026-09-11 | tl.sum、BLOCK_SIZE 启发式、64KB 上限、mask 补零 | verified（原文已读） |
| 5 | PyTorch `layer_norm_kernel.cu`（含 RmsNormKernelImpl） | https://raw.githubusercontent.com/pytorch/pytorch/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu | B | 2026-09-11 | 向量化快路径/标量慢路径、Welford、两遍 GM | verified（原文已读） |
| 6 | flash-attention `ln_api.cpp`（DropoutAddLayerNorm） | https://raw.githubusercontent.com/Dao-AILab/flash-attention/main/csrc/layer_norm/ln_api.cpp | B | 2026-09-11 | residual 融合、ctype=fp32、hidden 对齐约束、is_rms_norm | verified（原文已读） |

证据等级说明：B = 官方开源仓库源码（NVIDIA/LinkedIn/OpenAI-Triton/PyTorch/Dao-AILab）。无 C 级来源参与结论。

---

## 7. 对 Agent 10 合并的交接要点

1. GPU 证据支持「residual add 融合 + FP32 归约 + 按行分核 + 补零尾块」四条主线，与主线文档 v1/v2 路线一致。
2. GPU 证据反对两条：低精度中间计算（Liger none 模式）、按列切分 AI Core（无 GPU 对应且需跨核通信）。
3. 单遍暂存 y 的切换点（片上容量）在 GPU 侧是 64KB/element；在 Ascend 侧建议以 UB 实测容量定 D 阈值，文档已有 D≤4096 假设，需 Agent 7 用 UB 数据复核。
4. post-bias 的 epilogue 位置以 flash-attn LayerNorm beta 为准，**不要**用 Liger weight-offset。
5. 本报告未修改 `源码/`、`提交/`、`文档/` 任何文件。
