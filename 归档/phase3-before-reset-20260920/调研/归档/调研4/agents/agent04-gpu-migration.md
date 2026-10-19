# Agent 04 调研报告：GPU / CUDA / Triton / PyTorch 生态的 RMSNorm 与 residual-add 融合实现，及其向 Ascend C 迁移的差异分析

> 工作语言：简体中文
> 调研范围：仅 GPU 生态（NVIDIA CUDA / Triton / PyTorch / ROCm / AMD），**不**涉及 Ascend C API、不搜索官方 Ascend 仓库、不设计数值测试矩阵、不研究比赛规则。
> 声明：本报告中所有 CUDA / Triton / ROCm 代码片段与实现**仅作研究参考**，绝不作为本题（Ascend C 手写 Kernel）的提交方案或推荐方案。

---

## 1. 结论摘要（≤10 条）

1. **GPU 上 RMSNorm 的核心瓶颈是访存带宽（memory-bound）而非算力**；算子融合（fused residual-add + RMSNorm + gamma/bias）的本质是减少 HBM 往返、合并多次读写为单次（来源：aicassindra.com 数值分析、pommedeterresautee 博客，证据等级 C/A-社区基准）。
2. **归约（last-dim reduction）在 GPU 上普遍采用「warp 内 shuffle 归约 + block 内 shared-memory 两阶段树形归约」**（来源：AkiRusProd/numpy-nn-model `rmsnorm.cu` `warp_reduce_sum`/`block_reduce_sum`，naklecha/simple-llm `kernels/norm.py`，证据等级 C）。
3. **半精度累加必须用 FP32 累加器**：bf16/fp16 输入在求 Σx² 时若用 16-bit 累加会因「大数吃小数」或溢出而失稳；这是算法层通用最佳实践，**可直接迁移到 Ascend**（来源：aicassindra.com、社区 RMSNorm kernel 一律 fp32 累加，证据等级 C；PyTorch/官方实现亦采用 `acc_type`=fp32，等级 A）。
4. **Triton 官方并无 RMSNorm 教学 kernel**；社区实现普遍采用「one program per row + `tl.sum`/`tl.arange` + mask 处理非 2 的幂尾块」的模式（来源：datawhalechina、naklecha、sparkproof 等社区实现，证据等级 C）。
5. **融合 add 的实现思路高度一致**：先在 kernel 内算 `h = x + residual`，再做 Σh² 归约，并就地写回 residual（节省一次访存）（来源：Liger-Kernel `fused_add_rms_norm.py`、vLLM `fused_add_rms_norm_kernel`、NVIDIA SOL-ExecBench #69，等级 A-社区/B）。
6. **向量化加载（float4 / half2×4 / 16 字节 LDG）是 GPU 提速关键**，依赖指针强转与对齐判断（来源：vLLM kernel、Qwen3 `rmsnorm.cu`，等级 C）。
7. **PyTorch 官方 `F.rms_norm` 在 CUDA 上的底层 kernel 使用 block 级归约 + fp32 累加 + gamma 融合**，但**传统 RMSNorm 只含 gamma、不含 bias**（来源：kiki632/Smurfs `csrc/layernorm_kernels.cu` 的 vLLM 衍生实现，含 `rms_norm_kernel` 与 `TODO: Vectorized / Warp-level` 注释，证据等级 C；PyTorch 主线归约实现等级 A）。
8. **本题 `AddRmsNormBias` 比 GPU 惯例多了一步 bias 融合**；GPU 上 RMSNorm 通常无 bias（仅 Layernorm 有 bias），因此 GPU 实现中「融合 bias」的现成参考较少，需自行在 epilogue 加 `+ bias`（来源：NVIDIA SOL-ExecBench #69 仅 weight 无 bias；Liger 亦仅 gamma，等级 B）。
9. **occupancy、warp divergence、L2 命中率这些 GPU 调优概念在 Ascend 上没有直接对应物**，把「提高 occupancy 就更快」等直觉套用到 Ascend 是**无依据**的（来源：通用架构差异，证据等级 C/推理；受禁令未查 Ascend 官方文档）。
10. **NPU 框架层（torch_npu）已提供 `npu_rms_norm` / `npu_add_rms_norm` 等融合算子**（来源：SGLang `layernorm.py` `forward_npu`，等级 C-第三方仓库）。但这是框架算子，**不等于手写 Ascend C Kernel**，且本题要求核心计算在 Kernel 内完成、禁止 Host 代算，故该算子**仅证明「融合 op 在 NPU 上可行」这一事实，绝不能当作本题实现方案**。

---

## 2. 迁移差异表（≥10 行）

> 说明：「Ascend C 对应机制」一列基于 GPU 生态的通用并行归约思想给出**方向性**对应；因受禁令未查询 Ascend C 官方 API，具体 API 名称/签名未确认，详见第 6 节「未确认事项」。

| # | GPU 机制 | Ascend C 对应机制（方向性） | 可迁移部分 | 不可直接迁移部分 | 风险 |
|---|---------|---------------------------|-----------|----------------|------|
| 1 | warp shuffle 归约（`__shfl_xor_sync` 蝶形归约） | 无 warp 概念；改用「核内/block 内归约原语（whole-block reduce）」或向量归约指令 | 归约的「先局部后全局、树形合并」思想 | `__shfl_xor_sync` 本身、lane/warp 编号语义 | 直接照搬会导致数据错乱；需改用 Ascend 归约原语 |
| 2 | block reduction（shared memory 暂存每 warp 结果再汇总） | 片上 buffer（UB/L1）暂存中间归约结果 | 两阶段树形归约思想；「warp 结果 → 全局结果」的层级 | CUB `BlockReduce`、显式 `__shared__` 数组管理 | 同步点语义不同，易写错 |
| 3 | `__syncthreads()` 线程块屏障 | `SetFlag`/`WaitFlag` 或流水屏障（非线程屏障） | 需要在归约前后插入同步点 | CUDA 的「所有线程到达某点才继续」的屏障语义 | 误用会造成死锁/数据竞争 |
| 4 | shared memory 复用（缓存整行减少 HBM 往返） | UB（Unified Buffer）/ L1 片上缓存 | 「把一行数据放进片上内存算完再写回」的核心动机 | 显式 shared 生命周期管理与 bank 冲突规避 | 片上容量预算不同，需重新评估 tiling |
| 5 | 寄存器缓存（累加器驻留寄存器） | 寄存器 + UB；向量累加器 | 「累加器用 fp32 驻留、减少回写」思想 | 寄存器分配模型、spill 行为 | UB 溢出导致性能骤降 |
| 6 | 向量化加载 `float4`/`half8`（16 字节合并 LDG） | 128-bit 向量搬运 + 向量指令（`float16x8` 等） | 合并访存、一次搬 128 bit 的思想 | 指针强转 `reinterpret_cast`、对齐判断 `ptr%16==0` | 非对齐 / D 非 16 倍数时的尾块处理 |
| 7 | 半精度累加（fp32 accumulator） | **同样用 fp32 累加**（算法层通用） | 几乎完全可迁移，强烈建议 | 无（这是数学层，非硬件特定） | 低；务必坚持 fp32 累加 |
| 8 | grid-stride 行划分（多 block 并行处理行） | 多核并行 + tiling，每核处理若干行 | 「按行（rows）并行、每核负责一段行」思想 | grid/block 的一维/二维组织、grid-stride loop 语义 | 核数/每核负载需按 Ascend 实际核数重算 |
| 9 | 尾块处理（D 非 2 的幂 / 非 32 倍数） | mask / process 参数处理尾部元素 | 高；Triton 的 `mask`、vLLM 的整除分支都可借鉴「边界判断」思路 | GPU 的向量化 tail 写法（`vec_hidden_size` 分支） | 中等；未对齐尾块易越界写 |
| 10 | occupancy 与并行度调节 | 无 occupancy 概念；对应的是核内流水（PIPE）、UB 容量、多核利用率 | 仅「提高并行度以隐藏访存延迟」的调优思路 | CUDA occupancy 公式、`maxThreadsPerBlock`/`sharedMemPerBlock` 查询 | **高**：GPU occupancy 直觉在 Ascend 无依据 |
| 11 | warp divergence | 无 warp；SIMD/向量指令宽度与分支行为不同 | 仅「尽量避免分支 / 用 mask 替代 if」的思想 | warp divergence 的定义与代价模型 | **高**：在 Ascend 谈 warp divergence 无意义 |
| 12 | L2 cache 命中率调优 | 无 L2 概念（仅有 L1/UB 与片外 HBM） | 仅「提升访存局部性、合并访存」思想 | L2 作为可显式利用的缓存层次、L2 命中率指标 | **高**：L2 命中率在 Ascend 上无对应物 |

---

## 3. 各 GPU 实现要点对照

| 实现 | 平台 | 归约方式 | 是否融合 add | 是否融合 bias/gamma | 尾块处理 | 精度策略 | 来源（等级） |
|------|------|---------|------------|------------------|---------|---------|------------|
| AkiRusProd `rmsnorm.cu` | CUDA | `warp_reduce_sum`(shuffle) + `block_reduce_sum`(shared) | 否 | 融合 **bias+gamma**（RMSNorm+bias 变体） | while 循环 strided，无专门尾块逻辑 | fp32 | 社区仓库（C） |
| Qwen3 `kernel_src/rmsnorm.cu` | CUDA(H100) | warp shuffle + shared + `__syncthreads` | 否 | 仅 gamma（bf16/half2 向量化） | `vec_hidden = D/2` 整除假设 | fp32 累加 | HuggingFace 数据集（C） |
| LeetCUDA `rms-norm/rms_norm.cu` | CUDA | 层级 block reduce + shuffle | 否 | 仅 gamma（多 vec 变体） | `f16x8` 等需整除 | f16_f16 / f16_f32 对比 | 社区教程（C） |
| naklecha/simple-llm `kernels/norm.py` | Triton | `tl.sum(x*x, axis=0)`（编译器归约） | **是**（`_fused_add_rms_norm_kernel`） | 仅 gamma | `tl.arange` mask，非 2 幂安全 | bf16 输入 + fp32 内部 | 社区（C） |
| Liger-Kernel `fused_add_rms_norm.py` | Triton | 编译器归约（per-row program） | **是** | 仅 gamma（无 bias） | `BLOCK_SIZE = next_power_of_2(D)` + mask | fp32 中间 | LinkedIn 开源（A-社区级） |
| vLLM `fused_add_rms_norm_kernel` | CUDA | CUB `BlockReduce<float,1024>` | **是**（in-place 写 residual） | 仅 weight（无 bias） | 向量化 `width=8`（16B LDG）+ 对齐判断 | fp32 `s_variance` | vLLM 衍生（C；kiki632/Smurfs 同款，C） |
| PyTorch `F.rms_norm`（CUDA 后端） | CUDA | block 级归约（`acc_type`=fp32） | 否 | 仅 gamma | 运行时 shape 处理 | fp32 累加 | 官方（A） |
| NVIDIA SOL-ExecBench #69 参考实现 | PyTorch(CPU ref) | Python `mean(-1)` | **是**（residual+hidden） | 仅 weight（无 bias） | 逐元素无尾块问题 | fp32 | NVIDIA 基准（B） |
| SGLang `layernorm.py` (NPU 分支) | torch_npu | 调用 `npu_add_rms_norm` | **是** | gamma（框架算子） | 框架处理 | fp32（框架） | 第三方仓库（C；**非手写 Kernel**） |
| ROCm/AMD AITER `rmsnorm2d_fwd_with_add` | HIP | CK kernel（gfx942/gfx950） | **是** | 仅 weight | 2-D 专用，D≥1024 精度略降 | fp32 | AMD 官方/社区（A/B） |
| Int21 RMSNorm-B200 | CUDA/PTX(sm_100) | 手写 PTX 归约 | **是**（含 bias/prenorm） | **含 bias**（本题最接近） | 非 2 幂/宽行支持 | fp16/bf16/fp32 | 社区（C；最接近本题语义但平台不符） |

**关键对照结论**：
- GPU 上「融合 bias」的样本很少。多数 RMSNorm 仅融合 gamma；本题 `AddRmsNormBias` 要求 `...*gamma + bias`，在 GPU 实现里对应「在 epilogue 多一步 `+bias`」，仅有 Int21-B200 与 AkiRusProd 的变体显式含 bias。
- 融合 add 在 GPU 上已是成熟模式（vLLM / Liger / naklecha / SOL-ExecBench #69 均为「先 add 后 norm 单遍完成」），其**访存收益逻辑可迁移**，但**代码不可迁移**。

---

## 4. 「仅可作研究参考」清单（逐条说明为何不能直接用于本题提交）

1. **所有 CUDA / Triton / HIP / ROCm 源代码本身** —— 本题硬性要求提交 **Ascend C 手写 Kernel（`kernel.asc`）**，且禁令明确「不要建议在本题中使用 CUDA/Triton 代码」。直接搬运任何 GPU 源码既不符合提交形态，也无法在 NPU 编译运行。
2. **NVIDIA SOL-ExecBench #69 的 Python 参考实现** —— 这是数值正确性基准（`reference implementation`），仅用于对照「fused residual + RMSNorm」的数学定义与精度预期；它是 PyTorch CPU 参考，不是可被 NPU 执行的 Kernel，且未含 bias。
3. **Triton tutorial / 社区 Triton RMSNorm 的 `@triton.jit` 写法** —— 仅作「one-program-per-row + mask 尾块」的**概念参考**；Triton 编译器生成的归约与 Ascend C 的流水/归约原语完全不同，代码不可落地。
4. **vLLM / Liger / Smurfs 的 C++ CUDA 归约（`CUB BlockReduce`、`__shfl_xor_sync`、`__shared__`）** —— 证明「两阶段归约 + fp32 累加 + 向量化」有效；但其 API（CUB、shuffle、shared）在 Ascend C 不存在，只能借鉴**算法思路**。
5. **GPU 上的 speedup 基准数字（如 Triton 8.1x vs PyTorch、A100 带宽 2TB/s）** —— 这些数字仅描述 NVIDIA 硬件表现，与 Ascend NPU 的性能无任何可比性，不能作为本题优化目标或方案选型依据。
6. **occupancy / L2 命中率 / warp divergence 调优结论** —— 这些是 NVIDIA 架构专属的调优杠杆，在 Ascend 上没有对应概念（见第 5 节），据此做决策**无依据**。
7. **SGLang `forward_npu` 调用的 `torch_npu.npu_add_rms_norm`** —— 仅证明「NPU 框架层已有融合算子」，但（a）它不是手写 Ascend C Kernel；（b）本题要求核心计算在 Kernel 内完成、禁止 Host 代算，用框架算子即违反题意。故仅作事实参考。
8. **ROCm/AMD AITER 的 `rmsnorm2d_fwd_with_add`** —— HIP 代码，平台不符；且 AITER 文档注明「D≥1024 时精度略降」「仅 2-D」，与本题多 rank / 宽 D 需求不完全吻合，仅作「融合 add 在 AMD 也成立」的旁证。

---

## 5. Ascend 上「没有对应物」的 GPU 概念清单

以下概念**仅存在于 NVIDIA/AMD GPU 架构模型**，在 Ascend NPU 的编程模型（Ascend C）中**没有同名/同语义的对应物**，把针对它们的优化直觉直接套用到本题是**无依据**的：

1. **warp / warp shuffle（`__shfl_xor_sync`、`__shfl_down_sync`）** —— Ascend 没有 warp 与 lane 编号；归约走不同的核内归约原语。
2. **occupancy（活跃线程束占 SM 理论容量的比例）** —— Ascend 调优关注的是核内流水（PIPE）与 UB 容量，没有 occupancy 这一指标。
3. **L2 cache 命中率 / L2 作为可编程缓存层次** —— Ascend 的存储层次（L1/UB 与片外 HBM）与 GPU 的 L1/L2/Global 不同，L2 命中率概念不成立。
4. **warp divergence（同一 warp 内分支导致串行化）** —— 没有 warp，故该代价模型不适用；应改用 Ascend 的向量/SIMD 分支语义思考。
5. **CUB / Thrust 等归约库** —— Ascend C 不提供等价库（受禁令未确认具体替代 API 名，见第 6 节）。
6. **thread block / grid 的精确语义（`blockDim`、`gridDim`、`__syncthreads`）** —— Ascend 用「核 / block / 流水阶段 + 标志同步」，语义不同。
7. **cooperative groups、persistent kernel、grid-stride loop 的精确语义** —— 仅「多核并行处理行」的高层思想可借鉴。
8. **`sharedMemPerBlock` / `maxThreadsPerBlock` 等设备属性查询** —— Ascend 的片上 buffer 预算查询方式不同（未确认，禁查官方文档）。

> 推论：在本题优化时，**真正可跨平台成立的杠杆**只有少数几条——（a）fp32 累加；（b）算子融合（减少 HBM 往返）；（c）合并/向量化访存；（d）按行并行 + 尾块 mask。其余 GPU 专属优化项应视为「无效类比」。

---

## 6. 未确认事项

1. **Ascend C 的具体归约 API 名称与签名**（如核内/block 级 reduce 原语）——受禁令未查官方文档与官方仓库，本报告仅给方向性对应，具体 API 待其他代理/官方资料确认。
2. **CANN 9.0.0 上 vector core 的 UB 实际容量与每核并行度**——未确认；影响 tiling 与第 2 表第 10 行「并行度调节」的实际调参。
3. **bf16 在 Ascend 上 `rsqrt`/归约的数值表现与 GPU 是否一致**——未确认；保守做法是沿用 fp32 累加（跨平台稳妥）。
4. **本题 `AddRmsNormBias`「融合 bias」在 GPU 上的成熟参考较少**——已确认题目含 bias，GPU 仅 Int21-B200、AkiRusProd 变体含 bias；Ascend 侧如何高效融合需结合具体 API。
5. **多 rank（2D/3D/4D）的 shape 合法性处理**——GPU 实现统一 flatten 成 `(rows, D)`；Ascend 同样可 flatten，但越界/对齐校验需自行实现，未在本报告细化（属实现层，非本研究主题边界）。
6. **是否需要对 `D` 非 32 倍数做 128-bit 对齐的特殊尾部路径**——GPU 用「整除向量化 + 余数标量回退」；Ascend 的等价处理（process 参数 + mask）未确认 API 细节。

---

## 7. 来源表

| 编号 | 名称 | URL | 版本 / commit | 访问日期 | 用途 | 等级 |
|------|------|-----|--------------|---------|------|------|
| S1 | Triton Fused RMSNorm（Datawhale 社区教程） | https://datawhalechina.github.io/llm-algo-leetcode/03_CUDA_and_Triton_Kernels/03_Triton_Fused_RMSNorm.html | - | 2026-09-11 | Triton 融合范式、memory-bound 动机 | C |
| S2 | LLM Stack Triton RMSNorm kernel（one program per row） | https://prakashkagitha.github.io/llm-stack-book/04-kernels-efficiency/04-triton-kernels.html | - | 2026-09-11 | Triton 行并行 + mask + fp32 累加 | C |
| S3 | Accelerating Inference in Llama V2（Pomme de Terre，Triton 融合 add） | https://pommedeterresautee.github.io/posts/2023/accelerating-inference-in-llama-v2/ | - | 2026-09-11 | 融合 add 的访存收益 | C |
| S4 | naklecha/simple-llm Normalization Kernels（`kernels/norm.py`） | https://deepwiki.com/naklecha/simple-llm/4.1-normalization-kernels | - | 2026-09-11 | `_fused_add_rms_norm_kernel`、bf16+fp32、mask 尾块 | C |
| S5 | yyyycccccc/triton-kernels（RMSNorm 8.1x, fused 6.0x 基准） | http://onlybits.org/yyyycccccc/triton-kernels | - | 2026-09-11 | speedup 基准（仅 GPU 参考，不可比） | C |
| S6 | AkiRusProd/numpy-nn-model `rmsnorm.cu`（`warp_reduce_sum`/`block_reduce_sum`） | https://deepwiki.com/AkiRusProd/numpy-nn-model/7.3-cuda-rmsnorm | - | 2026-09-11 | warp shuffle + shared 两阶段归约、含 bias 变体 | C |
| S7 | Qwen3 `kernel_src/rmsnorm.cu`（H100 优化） | https://huggingface.co/datasets/burtenshaw/kernel-skill-source/blob/main/qwen3_8b/kernel_src/rmsnorm.cu | - | 2026-09-11 | 向量化 + shuffle + `__syncthreads` | C |
| S8 | xlite-dev/LeetCUDA `rms-norm/rms_norm.cu`（多 vec 变体、f16_f32 对比） | https://deepwiki.com/xlite-dev/LeetCUDA/4.3-merge-attention-states | - | 2026-09-11 | 向量化策略、fp32 累加必要性 | C |
| S9 | veitner「Making RMSNorm really fast」（warp/shared/vectorize 演进） | https://veitner.bearblog.dev/making-rmsnorm-really-fast | - | 2026-09-11 | 归约方式演进、float4 向量化 | C |
| S10 | Liger-Kernel `fused_add_rms_norm.py` | https://github.com/linkedin/Liger-Kernel/blob/124fb8a2/src/liger_kernel/ops/fused_add_rms_norm.py | 124fb8a2 | 2026-09-11 | fused add+rmsnorm Triton 实现（仅 gamma） | A（社区开源，LinkedIn） |
| S11 | open-lm-engine fused_residual_add_rmsnorm（Triton 前向/后向） | https://deepwiki.com/open-lm-engine/accelerated-model-architectures/3.3-normalization-operations | - | 2026-09-11 | 融合 add rmsnorm、fp32 中间、atomic/deterministic | C |
| S12 | sparkproof fused_add_rmsnorm Triton（D=1003 非 2 幂自测） | https://huggingface.co/datasets/ssnowman/sparkproof-magicrails-hopper-yunwu-v8/blob/main/proof/trajectories_raw.jsonl | - | 2026-09-11 | 非对齐尾块 mask 处理示例 | C |
| S13 | PyTorch 官方博客「SOTA Normalization Performance with torch.compile」 | https://pytorch.org/?p=62291 | 2026-04-08 | 2026-09-11 | Inductor 归约（inner reduction、persistent）、fp32 | A（官方博客） |
| S14 | PyTorch PR #153666 Fused RMSNorm（AaronWang04） | https://github.com/pytorch/pytorch/pull/153666 | - | 2026-09-11 | 融合 RMSNorm vs inductor 基准（9x） | A（官方 PR，已关闭） |
| S15 | kiki632/Smurfs `csrc/layernorm_kernels.cu`（`rms_norm_kernel`，vLLM 衍生） | https://deepwiki.com/kiki632/Smurfs/5.1-rms-norm-implementation | - | 2026-09-11 | block 归约 + fp32 累加 + TODO 向量化/warp | C（vLLM 衍生） |
| S16 | NVIDIA SOL-ExecBench #69（fused residual + RMSNorm 基准+排行榜） | https://research.nvidia.com/benchmarks/sol-execbench/kernel/69 | - | 2026-09-11 | 本题语义最接近基准（仅 weight 无 bias）、参考实现 | B（NVIDIA 官方基准） |
| S17 | FlashInfer Bench `rmsnorm_h7168` / `rmsnorm_h512` | https://bench.flashinfer.ai/kernels/rmsnorm_h7168 | - | 2026-09-11 | Triton/CUDA 解法对照（B200） | B |
| S18 | Int21-AI/RMSNorm-B200（PTX，含 bias/prenorm/fused residual） | https://github.com/Int21-AI/RMSNorm-B200 | 2026-06-09 | 2026-09-11 | 最接近本题语义（含 bias）但平台为 B200 | C |
| S19 | SGLang `layernorm.py` NPU 分支（`npu_add_rms_norm`） | https://deepwiki.com/openanolis/sglang/4.2-normalization-layers | - | 2026-09-11 | NPU 框架层已有融合算子（**非手写 Kernel**） | C（第三方仓库） |
| S20 | ROCm AITER `rmsnorm2d_fwd_with_add` / FlashInfer ROCm 端口 | https://rocm.docs.amd.com/projects/atom/en/main/model_ops_guide.html ; https://www.1o1.men/demandal25/flashinfer | ROCm 7.x | 2026-09-11 | AMD HIP 融合 add rmsnorm（gfx942/950） | A/B |
| S21 | aicassindra「LayerNorm vs RMSNorm architecture / numerics」 | https://aicassindra.com/blogs/transformer_math/tm_layernorm.html | - | 2026-09-11 | fp32 累加必要性、memory-bound 论证 | C |
| S22 | vLLM fused_add_rms_norm_kernel 中文解析（CUB BlockReduce、width=8、16B 对齐） | （知乎/CSDN 解析，对应 vLLM 源码） | - | 2026-09-11 | 向量化加载 + CUB 归约 + in-place residual | C |
| S23 | 社区 RMSNorm 手写 CUDA 解析（bf16 8 元素/线程, VEC_SIZE） | （搜索摘要，未给 URL） | - | 2026-09-11 | 线程组织、向量化宽度 | D（仅摘要） |

---

### 总结（回复 team-lead 用，≤200 字）

- **文件路径**：`/Users/sunyiyang/Desktop/Project/cann/调研/调研2/agents/agent04-gpu-migration.md`
- **迁移差异表行数**：12 行
- **3 条最关键结论**：① GPU RMSNorm 瓶颈是访存带宽，融合 add+norm+bias 的本质是减少 HBM 往返（可迁移思想）；② 半精度必须用 FP32 累加（跨平台可直接迁移）；③ warp shuffle / occupancy / L2 命中率 / warp divergence 等 GPU 概念在 Ascend 上无对应物，相关调优直觉无依据。
- **明确判定「不可用于本题」的结论**：所有 CUDA/Triton/HIP 源码、SOL-ExecBench 的 Python 参考、GPU speedup 基准、occupancy/L2 调优结论、以及 torch_npu 框架算子（非手写 Kernel，违反题意）。
