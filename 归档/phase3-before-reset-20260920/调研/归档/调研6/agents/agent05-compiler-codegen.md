# Agent 5 调研报告：编译器 / IR / 算子自动生成路线评估

> 角色：CANN 挑战赛调研团队 · Agent 5（编译器 / IR / 算子自动生成）
> 题目：2026 CANN 挑战赛·西南赛区初赛 · AddRmsNormBias
> 本轮性质：纯调研，不写提交代码；对上一轮「没有任何生成类工具能产出可提交 `kernel.asc`」结论做独立复核。
> 平台约束：本机 macOS，无 CANN 工具链 / 无 NPU，仅做资料核对。
> 编号段：来源清单使用 **S121–S150**。

---

## 1. 执行摘要

### 1.1 题目对「产物形态」的硬约束（复核基线）

判题编译的是单文件直调 `kernel.asc`，被 `#include` 进 `main.asc`，**不得有 `main()`**，其骨架为：

```cpp
#include <cmath>
#include "kernel_operator.h"
extern "C" void run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, GM_ADDR residual, ...,
                           int64_t availableCoreNum, aclrtStream stream, float epsilon) {
    add_rms_norm_bias_custom<<<blockNum, nullptr, stream>>>(...);
}
```

- shape / rank / dtype **只能运行时**从 `TensorGroupInfo` 读取（`TensorInfo{ const int64_t* shape; int64_t numDims; int32_t dtype; }`，dtype 枚举 0=fp32/1=fp16/2=bf16）。
- **没有 tiling 结构体、没有 Host 侧常量**，意味着分块参数必须由 kernel 在运行时自行推导。
- CANN 9.0.0，默认 `SOC_ARCH = "dav-2201"`（对应 Atlas 800T A2 / 910B 系），kernel 有 120 秒超时。

### 1.2 独立复核裁定

**裁定：上一轮结论成立——没有任何自动生成类工具能产出符合判题形态的单文件直调 `kernel.asc`。本轮逐项证据支持该结论，无一例外。**

复核逻辑（对每一类工具的「产物形态」逐一证伪）：

1. **图编译器 / 端到端 ML 编译器**（TVM、IREE、OpenXLA/XLA、TorchInductor）——它们的产物是**算子运行时 + 调度逻辑**（TVM 的 `.so`/运行时、IREE 的 `.vmfb` + HAL 运行时、XLA 的 PJRT 插件、Inductor 经 `torch.compile` 启动的 Triton/C++）。它们都需要各自的 **runtime**，判题的纯 C++ 直调场景不提供这些 runtime，因此无法落地（见 §3 中 Q3 列）。
2. **DSL / 编译器**（Triton-Ascend、TileLang、PyPTO）——它们启动内核的方式是 **Python JIT / 运行时**（triton-ascend 经 `torch_npu`；TileLang 经 `tilelang.JITKernel`；PyPTO 经 MPMD 调度运行时）。其生成物是**编译后的内核对象或 Ascend C 源码片段**，不是带 `extern "C" run_kernel` + `<<<>>>` 启动、无 `main()`、可被 `#include` 的单文件 C++。
3. **CANN 官方生成工具 msopgen**——它生成的是**完整算子工程**（`op_host/` + `op_kernel/` + `framework/` + tiling 头文件 + CMake），核函数签名是 `extern "C" __global__ __aicore__ void add_custom(...)` 形态 + Host 侧算子注册 + aclnn 调用链，与判题要求的「单文件 `run_kernel` + `<<<>>>` 直调」不一致，且依赖框架/插件与 aclnn 运行时。
4. **MLIR linalg/vector dialect**——它是**编译器基础设施（IR + pass）**，本身不产出「可提交的 `.asc`」；要落地仍需下游 codegen（LLVM/SPIR-V/Ascend 后端）与完整运行时，目前没有现成的「直调单文件」出口。

因此 **B 类（可用于本题提交）为空**，全部 10 个被评估对象归为 **A 类（可作研究参考）**。

### 1.3 关键共识（供手写实现借鉴）

虽然不能自动生成，但上述工具的归约 lowering 思路、memory planning、尾块策略、FP32 累加约定，对手写 AddRmsNormBias kernel 有高价值启发（见 §3 七个技术点）。

---

## 2. 工具二分表（≥8 个工具）

判定口径：

- **问题1（单文件 `kernel.asc`）**：能否产出 `extern "C" run_kernel` + `<<<>>>` 启动、无 `main()`、无额外 Host 注册、可被 `#include` 的单文件 C++？
- **问题2（FP32 累加 + 非对齐尾块 + 累加顺序）**：能否保证 FP32 中间累加、正确处理非对齐尾块；归约 lowering 是否改变累加顺序（影响精度）？
- **问题3（CANN 9.0.0 落地）**：能否在该判题直调环境运行（是否需要额外 runtime / 算子注册 / 图引擎）？
- **A类 / B类**：B 类须三个问题全为「是」且有证据；否则为 A 类。

| 工具 | 问题1（单文件 kernel.asc） | 问题2（FP32 累加 + 尾块 + 顺序） | 问题3（CANN 9.0.0 落地） | A类/B类 | 依据 | 证据等级 |
|---|---|---|---|---|---|---|
| **Triton-Ascend**（triton-lang/triton-ascend） | 否。产出经 `torch_npu` 运行时启动的编译内核对象 / Python 侧 JIT 函数，非单文件 C++ `kernel.asc` | 可（默认）。`tl.sum` 将浮点提升到 **至少 fp32** 累加；尾块用 `mask` + `other=0.0` / `boundary_check`；但 `tl.sum` 为**树形/硬件归约**，改变累加顺序 | 否。需 `triton-ascend` + `torch_npu` + CANN 运行时；判题纯 C++ 直调场景无此 runtime（即便支持 CANN 9.0.0 + A2 硬件） | **A类** | 官方仓与安装指南明确为 pip/Python 包 + torch_npu；3.2.1 配 CANN 9.0.0 | B |
| **TileLang**（tile-ai/tilelang） | 否。经 `tilelang.JITKernel` 生成 torch 函数，底层 TVM/FTG 生成 CUDA/HIP/AscendC；AscendC 后端为 **preview/实验**（分支 `ascendc_pto`/`npuir`），启动仍依赖 Python 运行时 | 部分。提供 `accum_dtype`（如 `"float"`）可指定 FP32 累加；尾块需 mask；tree/并行归约改变顺序 | 否。需 tilelang 运行时 + Python；Ascend 后端尚实验性，无 CANN 直调产物 | **A类** | GitHub + ICLR2026 论文说明其在 TVM 之上、目标 CUDA/HIP，Ascend 为 2025-09 新增 preview | B/C |
| **PyPTO**（hw-native-sys/pypto，PTO 框架） | 否。JIT 生成 Ascend C 代码 + PTO 虚拟指令，需 PyPTO **MPMD 调度运行时**加载到设备 | 不确定（社区资料）。声称自动内存管理、动态 shape；但 FP32 累加/尾块策略无官方定论 | 否。需 PyPTO 运行时 + JIT；非单文件直调 | **A类** | 社区仓库 + CSDN 解析文；CANN Open Software License，但非华为官方主维护 | C |
| **msopgen**（CANN 官方算子工程生成） | 否。生成**完整算子工程**（`op_host`+`op_kernel`+`framework`+tiling），核函数为 `extern "C" __global__ __aicore__ void` 形态 + Host 注册 + aclnn 调用，与判题单文件 `run_kernel`+`<<<>>>` 不一致 | 用户自控。模板不保证 FP32 累加/尾块，需手写实现 | 否。需完整算子工程 + 框架插件 + aclnn 运行时；非纯直调 | **A类** | 华为官方文档：msopgen 生成 AddCustom 工程框架，核函数 `add_custom` 经 `<<<>>>` 由 aclnn 启动 | A/B |
| **Apache TVM**（含 Discuss） | 否。生成 `.so` / CUDA 源码经 **TVM 运行时**启动；非单文件 `kernel.asc` | 用户指定累加 dtype（compute dtype）；`rfactor` 做**树形/跨线程归约**改变顺序；尾块用 `mask` / `set_store_predicate` | 否。需 TVM runtime；且 TVM 原生目标为 CPU/CUDA/ROCm/OpenCL，无 Ascend 直调 | **A类** | 官方 Discuss rfactor 示例 + TensorIR rfactor commit | B/C |
| **IREE**（iree-org/iree） | 否。AOT 生成 **`.vmfb` + IREE runtime（HAL/VM）**，经 C API `iree-run-module` 调用；无单文件 C++ `kernel.asc` | 取决于输入方言（StableHLO/TOSA/linalg）；归约顺序由 codegen 决定 | 否。需 IREE runtime/HAL；支持后端为 CUDA/ROCm/Vulkan/Metal/VMVX，**无 Ascend** | **A类** | 官方文档：`.vmfb` 工件 + 运行时；支持矩阵无 Ascend | B |
| **OpenXLA / XLA**（openxla.org） | 否。生成 XLA 可执行 + **PJRT 插件**；无单文件 `kernel.asc` | 取决于 HLO  lowering；归约顺序由后端决定 | 否。目标后端列表为 NVIDIA/AMD/Intel GPU、Apple GPU、Google TPU、AWS Trainium/Inferentia、Cerebras、Graphcore IPU、x86/ARM CPU——**无 Ascend NPU** | **A类** | 官网 + 维基：明确支持设备列表，无华为 Ascend | A/C |
| **TorchInductor**（PyTorch `_inductor`） | 否。生成 Triton / C++ 源码，经 `torch.compile` + Inductor 运行时启动；非 `kernel.asc` | 由代码生成决定：累加 dtype = `triton_acc_type(src_dtype)`；**persistent vs looped reduction** 两种策略改变累加顺序；尾块用 `rmask`/`mask` | 否。需 PyTorch + 目标后端（默认 CUDA）；Ascend 需 `torch_npu` 且需改写 codegen，无直调产物 | **A类** | 源码 `torch/_inductor/codegen/triton.py` + persistent reduction commit + 博客 | B/C |
| **MLIR linalg/vector dialect**（reduction lowering） | 否（基础设施）。产出 MLIR IR，需下游 codegen（LLVM/SPIR-V/Ascend）；无现成「直调单文件」出口 | `vector.reduction` 可 lowering 成 **顺序 loop** 或硬件/树形归约；累加 dtype 由 IR 决定；尾块用 `vector` masking | 否。需完整后端 + runtime；无 Ascend 直调 | **A类** | 官方 Dialect 文档 + Ch0 教程 + scalable vectorization PR #97788 | B |
| **LLVM/MLIR Discourse（论坛讨论）** | N/A（非代码生成工具，仅讨论源） | N/A（提供 rfactor / scalable reduction / masking 等佐证） | N/A | **A类**（信息源） | 论坛帖佐证上述 lowering 行为 | C |

> 结论：**B 类 = 0**。全部归 A 类。没有任何工具满足「问题1=是」，更不可能三问全为「是」。

---

## 3. 逐技术点分析（含「对手写 kernel 的启发」）

### 3.1 vector kernel 生成与 SIMD/向量化

**编译器视角**：TVM/Inductor/Triton 都先把逐元素运算向量化（`arith.addf` 可直接作用在 `vector<>` 上，MLIR 允许任意 rank 向量均匀扩展）；TileLang/IREE 通过 tile 抽象映射到硬件向量/矩阵指令。向量化本质是「一次加载 N 个元素、批量运算」，编译器会自动选 block/vector 长度。

**对手写 kernel 的启发**：
- AddRmsNormBias 中 `y = x + residual`、`output = y/rms*gamma + bias` 均为逐元素，应直接用 Ascend C 的 **Vector 指令**批量处理（如 `Add`/`Mul`/`Div`），配合 `DataCopy` 以 256B（32×fp16 或 16×fp32）为粒度搬运，使一次搬运对齐。
- 采用 **double buffer / pipeline**（搬入-计算-搬出三级流水）掩盖 GM↔UB 访存延迟，这是所有编译器都在做的「生产者-消费者流水」思想在 Ascend C 上的等价实现。

### 3.2 reduction lowering（树形归约 vs 顺序归约的误差差异）

**编译器视角**：
- Triton 的 `tl.sum` 是**树形/硬件级归约**（warp 内 butterfly reduction），累加顺序与「从左到右顺序累加」不同（S142、S143）。
- TVM 的 `rfactor` 把归约轴因式化出 `B_rf` 中间缓冲，先做跨线程（warp/block）部分归约、再写回汇总，**也是树形/并行归约**（S130、S131）。
- MLIR `vector.reduction` 可 lowering 成 `scf.for` 顺序 loop，也可保留为硬件归约（S140）。

**误差结论（关键）**：顺序累加（从左到右）误差约 `~eps·N`；树形归约误差约 `~eps·log2(N)`（N 为归约长度）。对 RMSNorm，N=D∈[64,32768]，`eps(fp32)≈1.2e-7`。D=32768 时顺序累加理论误差上界约 `3.9e-3`，树形约 `4.2e-4`——**树形通常更稳**，但二者都与「参考实现」不同；判题通常以 PyTorch 参考（FP32 累加）为准，故**核心是 FP32 累加器**，而非树/顺序之争。

**对手写 kernel 的启发**：
- 必须在 **FP32 累加器**中做 `sum(y*y)`（无论输入 fp16/bf16），再开方。这是与 Triton 默认 `tl.sum` 提升 fp32 一致的约定。
- 块内归约可用「两两相加」树形；跨块（多 AI Core / 多 UB tile）汇总时，因已 FP32，顺序或树形差异极小，可直接用顺序累加降低实现复杂度。
- 注意：判题参考若用 FP32 累加，手写也必须 FP32 累加，否则尾差超限。

### 3.3 memory planning / scratchpad（UB）分配与复用

**编译器视角**：
- MLIR Linalg 的「Promotion to Temporary Buffer in Fast Memory」「Progressive Buffer Allocation」即把中间结果提升（promote）到高速内存（UB 类比 shared mem）（S139、S145）。
- IREE/TVM 的 bufferization 做全局内存规划，决定哪些中间张量物化、哪些仅在寄存器/UB 中存在。
- TileLang 的 `T.alloc_shared` / `T.alloc_fragment` 显式区分 shared/register 级缓冲。

**对手写 kernel 的启发**：
- AddRmsNormBias 的中间量：`y`（=x+residual）、`y*y`、部分归约和 `sum`、最终 `output`。UB 容量有限，**不应同时物化全部**。
- 推荐 UB 复用规划：`tile_y`（当前块 y）→ 就地算 `y*y` 累加进 FP32 累加器 `acc` → 用 `acc` 算 `rms` → 用 `tile_y` 与 `rms`/`gamma`/`bias` 算 `output` 并搬出。**`y` 必须保留到除法阶段**（因为 `output = y/rms*gamma+bias`），所以 `tile_y` 在 UB 中生命周期覆盖「累加」与「归一化」两段，这正是 fusion 带来的 UB 压力，需要据此反推 tile 大小。

### 3.4 尾块（非对齐）代码生成策略（mask / 多版本特化 / padding）

**编译器视角**：
- Triton：`mask` + `other=0.0` / `boundary_check`，对所有 `tl.load`/`tl.store` 施加掩码，天然处理 `D % BLOCK != 0`（S142、S144）。
- TVM：`set_store_predicate`（如 `if (threadIdx.x==0)`）防止越界写；归约体内对超界元素用中性元（0）参与（S130）。
- MLIR：`vector` masking / scalable vectorization 仅支持 trailing-dim 归约（S141）。

**对手写 kernel 的启发**：
- 运行时读 `D` 后，按 `D` 与向量长度（32 for fp16 / 16 for fp32）取模：
  - **对齐快路径**：`D % vecLen == 0` 时整块搬运、无 mask，性能最佳。
  - **非对齐尾块**：用 Ascend C 的 **Mask 参数 / Tail mask** 处理最后一个不满块；或 load 时补零（padding 到对齐长度）以避免越界读 GM。
- **多版本特化**可选：若 `D` 为常见对齐值（如 4096/8192）走无 mask 路径，否则走带 mask 路径；但判题强调运行时读 shape，建议**统一走带 mask 路径**以覆盖 `D∈[64,32768]` 任意值（包括非 32 倍数），牺牲少量性能换正确性。
- 关键陷阱：归约的尾块也必须在 mask 内用**中性元 0** 参与 `sum`，否则 FP32 累加器会累加垃圾值。

### 3.5 FP16/BF16 的 lowering 与精度保持

**编译器视角**：
- Triton 默认把浮点归约提升到 **至少 fp32**（S142）；TVM/Inductor 累加 dtype 由 IR 决定（通常 fp32）。
- bf16 指数位宽同 fp32、尾数仅 8 位；fp16 尾数 11 位。两者做大规模归约时，**窄尾数 + 大动态范围**下误差主要来自尾数，故必须提升累加位宽。

**对手写 kernel 的启发**：
- 输入 fp16/bf16 时，`y*y` 可先转 fp32 再累加（`sum` 用 fp32 累加器），避免「窄尾数平方 + 长序列累加」的灾难性误差。
- `rms = sqrt(mean + eps)`、`y/rms` 均在 fp32 完成，最后 `output = ... * gamma + bias` 根据输出 dtype 转回 fp16/bf32 再写 GM。
- `gamma`/`bias` 若为 fp16 入参，读取后亦建议转 fp32 参与计算（与编译器「计算用宽类型、存储用窄类型」一致）。

### 3.6 kernel fusion（elementwise + reduction 的融合边界）

**编译器视角**：
- TVM/Inductor 的 producer-consumer fusion：把 `Add → RmsNorm → *gamma + bias` 融为单 kernel，避免物化中间 `y`、`rms`（S130、S138）。
- Inductor 的 `should_use_persistent_reduction` 启发：**inner/连续归约维**适合持久化（单 program 一次算完），这正是 RMSNorm「沿最后一维 D 归约」的理想形态。

**对手写 kernel 的启发**：
- AddRmsNormBias 本就是「elementwise + reduction + elementwise」三阶段融合，**应在一个 kernel 内完成**，不要拆成多个 pass（否则 `y` 需写回 GM 再读，带宽翻倍）。
- 融合边界决策：归约（`sum(y*y)`）必须跨整个 D 完成才能算 `rms`，因此**块内先局部累加、跨块/跨核再汇总**是融合下的唯一正确结构；汇总后的 `rms` 再回到每个 tile 做归一化。
- 注意融合带来的 UB 压力（见 §3.3）：融合虽省带宽，但 UB 需同时容纳 `tile_y`、累加器、输出 tile，tile 大小要据此收敛。

### 3.7 autotuning（tile size 搜索）

**编译器视角**：
- Triton/`@triton.autotune`、TVM MetaSchedule、TileLang 都通过**搜索 tile/block/warp/stage 组合**找最优（S128、S129）。
- Inductor 用启发式（`size_hints`、`reduction_hint`）而非暴力搜索（S138）。

**对手写 kernel 的启发**：
- 判题环境**无外部 autotuning runtime**，且 120s 超时，无法在提交 kernel 内做在线搜索。
- 可行做法：**静态经验分块**——按 UB 容量上限反推 `tileLength`（满足 `tile_y(fp32 or fp16) + 输出 tile + 累加器 + 双缓冲 ≤ UB 容量`），并对 `blockNum = ceil(total/D / perCore)` 做核间划分；D 维单核内再分若干 UB tile。
- 可保留**少量编译期 constexpr 档位**（如针对 D≤1024 / D≤8192 / 更大）做轻量分支，但本质是静态设计，不是在线 autotune。这与 Inductor 用启发式而非搜索的思路一致。

---

## 4. 明确结论

**本轮不采用自动生成路线作为提交方案。**

证据链（逐工具见 §2）：
- 唯一能在 CANN 9.0.0 + A2 硬件上**编译运行**的 Ascend 友好生成器是 **Triton-Ascend**（3.2.1 配 CANN 9.0.0），但它经 `torch_npu` + Python 运行时启动，**不产出单文件 `kernel.asc`**，且判题直调场景无该 runtime。
- 其余生成器（TileLang/PyPTO/msopgen/TVM/IREE/OpenXLA/Inductor/MLIR）要么无 Ascend 后端、要么需各自 runtime、要么产物形态为完整工程/`.vmfb`/`.so`/JIT 函数，**均不满足问题1**。
- **没有任何工具满足「问题1=是」**，故 B 类为空，结论与上一轮一致且独立得到证实。

> 若未来出现「能直接 emit 出 `extern "C" run_kernel(...)` + `<<<>>>` 启动、无 `main()`、无额外 Host 注册、可被 `#include` 的 Ascend C 单文件」的官方工具，需以该工具的官方样例为证据重新评估；截至本轮检索（2026-09-12），不存在此类证据。

---

## 5. 从编译器视角给手写实现的建议

1. **分块（Tiling）**：运行时读 `D` 与 `availableCoreNum`，按 UB 容量反推 `tileLength`（兼顾双缓冲 + `tile_y` 生命周期 + 输出 tile + FP32 累加器），`blockNum` 按 `(totalElements/D)` 跨核切分；D 维单核内再细分为 UB tile。
2. **归约顺序**：块内用「两两相加」树形归约到 FP32 累加器；跨块/跨核用顺序或树形汇总均可（已 FP32，差异极小）。**严禁用 fp16/bf16 直接累加 `sum(y*y)`**。
3. **尾块**：统一走带 **Mask / Tail mask** 路径覆盖 `D` 非 32 倍数；归约尾块以中性元 0 参与累加；必要时 load 端 padding 到对齐长度避免越界读 GM。
4. **UB 复用**：`tile_y` 生命周期覆盖「累加」与「归一化」两阶段；`gamma`/`bias`/`rms` 用寄存器/小规模 UB 持有，避免中间张量落 GM；参考 MLIR/TVM 的「promote to fast memory」思路。
5. **精度保持**：fp16/bf16 输入在归约前转 fp32；`rms`、`y/rms`、`*gamma+bias` 全程 fp32，末态按输出 dtype 转回；与 Triton `tl.sum` 默认提升 fp32 的约定对齐，以匹配 PyTorch 参考。
6. **融合**：Add+Rms+γ+bias 单 kernel 融合，省 GM 带宽；代价是 UB 压力上升，tile 大小需据此收敛而非盲目取大。
7. **静态调优**：无在线 autotune，采用经验 constexpr 档位 + 启发式分块；优先保证正确性与 120s 内完成，再谈性能。

---

## 6. 未找到 / 访问受限清单

- **Ascend C 官方「自动生成单文件 `kernel.asc`」工具**：未找到。msopgen 仅生成完整算子工程，无单文件直调形态。
- **IREE / OpenXLA 的 Ascend 后端**：未找到。IREE 支持矩阵无 Ascend；OpenXLA 目标设备列表无华为 Ascend。
- **PyPTO FP32 累加 / 尾块策略的官方定论**：受限（仅社区文章，无权威规范），故 §2 中标记为「不确定」。
- **TileLang AscendC 后端细节**：仅 preview 级别信息（分支 `ascendc_pto`/`npuir`），无稳定文档；标记为 preview/实验。
- **本机验证**：macOS 无 CANN 工具链 / 无 NPU，**所有结论均为资料核对，未做编译/运行实证**（符合本轮「只调研」约束）。

---

## 7. 来源清单（S121–S150）

```
- [S121] Triton-Ascend 官方仓库（triton-lang/triton-ascend）| https://github.com/triton-lang/triton-ascend | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 Triton-Ascend 存在、CANN 9.0.0 兼容性、需 torch_npu 运行时 | 可支持的结论：问题1/3 为否，归 A 类
- [S122] Triton-Ascend 安装指南 | https://triton-ascend.readthedocs.io/en/v3.2.2/installation_guide.html | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 pip 安装、依赖 torch_npu + CANN、Linux 环境 | 可支持的结论：产物经 Python 运行时启动，非单文件 kernel.asc
- [S123] Triton-Ascend 概览（GitCode 镜像/Ascend）| https://gitcode.com/Ascend/triton-ascend | 平台 社区镜像 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 3.2.1 配 CANN 9.0.0、Atlas A2/A3/950 支持 | 可支持的结论：硬件可达但形态不符
- [S124] msopgen 快速入门（华为开发者文档）| https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-operator-development | 平台 华为官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 msopgen 生成完整算子工程、核函数 extern "C" __global__ __aicore__ 形态 + aclnn 启动 | 可支持的结论：问题1/3 为否，归 A 类
- [S125] 创建算子工程（CANN 9.0.0 社区版）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta2/devaids/optool/atlasopdev_16_0021.html | 平台 华为官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 msopgen gen 参数、生成 op_host/op_kernel/framework | 可支持的结论：生成完整工程而非单文件直调
- [S126] PyPTO 仓库（hw-native-sys/pypto）| https://github.com/hw-native-sys/pypto | 平台 GitHub | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 PTO 范式、JIT、MPMD 调度、生成 Ascend C 代码 | 可支持的结论：需 PyPTO 运行时，非单文件 kernel.asc
- [S127] PyPTO 解析（昇腾开源生态专区）| https://ascendai.csdn.net/69dcb64072111d255bf8a440.html | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Tile 编程模型、自动内存管理、动态 shape | 可支持的结论：FP32/尾块策略无官方定论
- [S128] TileLang 官方仓库（tile-ai/tilelang）| https://github.com/tile-ai/tilelang | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认基于 TVM、目标 CUDA/HIP、AscendC 为 2025-09 preview | 可支持的结论：问题1/3 为否，归 A 类
- [S129] TileLang ICLR2026 论文笔记 | https://en.papernotes.org/ICLR2026/llm_efficiency/tilelang_bridge_programmability_and_performance_in_modern_neural_kernels | 平台 论文笔记 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 FTG、accum_dtype、layout inference | 可支持的结论：支持指定 FP32 累加，但需 Python 运行时
- [S130] TVM rfactor CUDA codegen 讨论 | https://discuss.tvm.apache.org/t/cuda-codegen-could-it-generate-warp-shuffle-instructions/14641 | 平台 Apache TVM Discuss | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 rfactor 树形/跨线程归约 + set_store_predicate | 可支持的结论：归约 lowering 改变累加顺序，需 TVM runtime
- [S131] TVM TensorIR RFactor 提交（apache/tvm）| https://github.com/apache/tvm/commit/dc7bbb63838b0780b21c68030efdb60e73a5b224 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 rfactor 产出 B_rf 中间缓冲、部分归约+写回 | 可支持的结论：树形归约改变顺序，精度受影响
- [S132] IREE 官方文档（iree.dev）| https://iree.dev/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 AOT 生成 .vmfb + 运行时、支持后端 | 可支持的结论：需 IREE runtime，无 Ascend 后端
- [S133] IREE 架构/wiki | https://hivebook.wiki/wiki/iree-mlir-based-ml-compiler-and-runtime | 平台 社区 wiki | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Flow/Stream/HAL 分层、.vmfb 工件 | 可支持的结论：产物形态非单文件 kernel.asc
- [S134] OpenXLA 官网 | http://openxla.org/ | 平台 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 XLA 编译器、目标后端列表 | 可支持的结论：目标无 Ascend NPU，问题3 为否
- [S135] XLA 支持设备（Wikiwand）| https://www.wikiwand.com/en/articles/Accelerated_Linear_Algebra | 平台 维基 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认支持 NVIDIA/AMD/Intel/Apple/TPU/Trainium/IPU/CPU，无 Huawei | 可支持的结论：无 Ascend 后端
- [S136] TorchInductor triton.py 源码 | https://github.com/pytorch/pytorch/blob/1cae60a87e5bdda8bcf55724a862eeed98a9747e/torch/_inductor/codegen/triton.py | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 triton_acc_type、reduction 代码生成、rmask/mask | 可支持的结论：需 torch.compile 运行时，问题1/3 为否
- [S137] TorchInductor Persistent reductions 提交 | https://github.com/pytorch/pytorch/commit/a8fdfb4ba8a804c67d744a763fd9fa1f72d28590 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 persistent vs looped reduction 两种策略 | 可支持的结论：归约策略改变累加顺序
- [S138] TorchInductor Reduction Kernels 博客 | https://karthick.ai/blog/2025/Learn-By-Doing-Torchinductor-Reduction | 平台 博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 should_use_persistent_reduction 启发式、inner 归约融合 | 可支持的结论：融合边界与归约顺序启发
- [S139] MLIR 'linalg' Dialect 官方文档 | https://mlir.llvm.org/docs/Dialects/Linalg | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 Tiling / Promotion / Fusion / Vectorization 变换 | 可支持的结论：基础设施，无直调单文件出口
- [S140] MLIR Ch0：Structured Linalg / vector.reduction | https://mlir.llvm.org/docs/Tutorials/transform/Ch0/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 vector.reduction 可 lowering 为 scf.for 顺序 loop | 可支持的结论：归约 lowering 可顺序可硬件，顺序 loop 保序
- [S141] MLIR [Linalg] Scalable Vectorization of Reduction PR #97788 | https://github.com/llvm/llvm-project/pull/97788 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 trailing-dim 归约 scalable vectorization + masking | 可支持的结论：尾块用 vector masking
- [S142] Triton tl.sum API 文档 | https://triton-lang.cn/main/python-api/generated/triton.language.sum.html | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认浮点默认提升到至少 fp32 累加、可指定 dtype | 可支持的结论：Triton 默认 FP32 累加，树形归约改顺序
- [S143] Triton 归约深度 vs 数值误差 gist | https://gist.github.com/EthanZhong02/fef62df5728ad5af60dd035bb91e6e05 | 平台 GitHub Gist | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：量化 BLOCK_SIZE / 累加 dtype 对误差影响 | 可支持的结论：FP32 累加显著降低误差，树形归约改顺序
- [S144] Triton Exercises（归约/边界）| https://lweitkamp.github.io/triton_exercises/print.html | 平台 教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 mask/boundary_check 处理非对齐尾块、fp32 累加约定 | 可支持的结论：尾块 mask 策略启发
- [S145] MLIR Linalg Dialect 中文解析 | https://www.lei.chat/zh/posts/mlir-linalg-dialect-and-patterns/ | 平台 博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Linalg 分层、Promotion to fast memory 思想 | 可支持的结论：UB 复用/memory planning 启发
- [S146] TVM reduction 教程（tvm-fork）| https://gitlab.engr.illinois.edu/yifanz16/tvm-fork/-/blob/b236f10908d22eef2d83dd80183fd0e9affcd67d/tutorials/language/reduction.py | 平台 GitLab | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 te.sum / rfactor 基础归约调度 | 可支持的结论：归约 lowering 基础
- [S147] PyPTO 深度解析（昇腾开源生态专区）| https://ascendai.csdn.net/69dcb64072111d255bf8a440.html | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认三层 IR、PTO-ISA、融合指令 | 可支持的结论：需 PyPTO 运行时
- [S148] TileLang-MUSA（摩尔线程，跨平台佐证）| https://ima.qq.com/wiki/?shareId=e1480c65f3c34052d1c165c9b57fe17d7af11352a365513ba43e59f1d4d3dbb0 | 平台 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：佐证 TileLang 跨平台能力、需后端运行时 | 可支持的结论：进一步说明 DSL 需各自后端运行时
- [S149] IREE Developer overview | https://iree.dev/developers/general/developer-overview/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 iree-compile / .vmfb / runtime 工作流 | 可支持的结论：产物形态非单文件 kernel.asc
- [S150] Triton-Ascend 上手指南（上海交大 xflops）| https://xflops.sjtu.edu.cn/hpc-start-guide/ascend/triton/ | 平台 高校指南 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Triton-Ascend 安装组合（CANN 9.1.0 + torch_npu）、Python 启动 | 可支持的结论：判题纯 C++ 直调场景无法使用
```

---

> 备注：本报告严格限定在「编译器 / IR / 算子自动生成」主题，未越界检索题面与提交规则（Agent 1）、Ascend C API 文档（Agent 2）、官方仓库源码（Agent 3）、GPU/Triton 手写 kernel 实现（Agent 4）、数值精度专题（Agent 6）、性能与 UB（Agent 7）、Linux 环境（Agent 8）、失败案例（Agent 9）。精度相关讨论仅用于回答任务强制要求的「lowering 是否改变累加顺序」问题，未展开为独立精度专题。
