# Agent 05 调研报告：编译器 / IR / DSL 层面的算子生成（AddRmsNormBias）

- **负责主题**：vector kernel 生成、reduction lowering、memory planning、尾块生成、FP16/BF16 lowering、kernel fusion、自动调优，及其与 CANN 9.0.0 的兼容性
- **判题提交约束（关键前提）**：CANNJudge 直调工程，提交物是**单个手写 `kernel.asc` 文件**，内含 `extern "C" void run_kernel(...)`，内部用 `<<<blocks, nullptr, stream>>>` 启动 **`__global__ __vector__`** 核函数。自动生成工具产出的工程形态与这一单文件直调形态**不一致**。
- **本机环境**：macOS，无 CANN 工具链、无昇腾 NPU，无法编译/运行任何生成产物（所有结论均基于公开文档与代码，未做任何本地编译验证）。
- **访问日期**：2026-09-11

---

## 1. 结论摘要（≤10 条）

1. **没有任何工具能直接产出可提交的 `kernel.asc`**。所有调研到的自动生成/DSL 工具，产物形态都是「多文件算子工程 + aclnn/ATC 部署」「JIT 编译出的 `.so` 动态库」或「Bisheng 编译出的二进制」，均不是「单文件 + `run_kernel` + `__vector__` 直调」形态。**明确结论：没有证据支持存在可直接产出本题 `kernel.asc` 的工具。**
2. **PyPTO 0.2.0 是已知唯一明确对齐 CANN 9.0.0 的 DSL**（2026-04 发布），但它编译产物是 MPMD 二进制/算子图，不是 `kernel.asc`，故只能作为「研究参考」。
3. **TileLang-Ascend 有真实 Ascend 后端**，能生成 Ascend C 代码，但其生成的是 `__global__ __aicore__` 核函数并通过 JIT→`.so` 执行，**既没有 `__vector__` 直调形态，也不含 `run_kernel` 入口**，且 CANN 9.0.0 支持未确认。
4. **msopgen 只能生成标准多文件 aicore 算子工程**（含 `op_host`/`op_kernel`/`op_proto` + CMakeLists + aclnn/ATC 部署），核函数签名为 `__global__ __aicore__ void add_custom(GM_ADDR..., GM_ADDR workspace, GM_ADDR tiling)`，**与 `kernel.asc` 直调形态不符，且是 aicore 而非 vector**。
5. **IREE、上游 TVM 均无 Ascend 后端**，不能生成任何昇腾产物；其价值仅在于 reduction lowering / 自动调优的「算法思想」可借鉴。
6. **TorchInductor / LLVM Loop Vectorizer 是 CPU/GPU 通用编译器**，无 Ascend 后端，但其 reduction 向量化、BF16 legalization、双循环尾处理等思路对本题手工 Ascend C 实现极具借鉴价值。
7. **处理「D 非 32 倍数」最成熟的技术是「双循环尾处理」**（主循环按对齐块、尾部带 mask 的剩余循环），源自 TorchInductor，可直接映射到 Ascend 的 `WholeReduce` + 带 mask 的尾块。
8. **BF16 累加的最佳实践是「以 FP32 累加、末端转回 BF16」**（BF16 legalization），即 sum-of-squares / mean 用 FP32 累加器，避免 BF16 累加精度坍塌——直接适用本题。
9. **last-dim reduction 在 MLIR/LLVM 已有「可扩展向量化 + 向量累加器 + 单次水平归约」策略**，与 Ascend vector 核「每核处理整行、向量累加、末尾水平归约」的天然匹配。
10. **自动生成工具全部落入「仅作研究参考 / 不可用」两类，无一类可「直接用于本题提交」**；本调研的核心产出是为手写 Ascend C `kernel.asc` 提供可借鉴的算法/IR 模式清单。

---

## 2. 生成方案对照表

| 方案 | 平台 | 能生成什么 | 是否支持 Ascend | 产物形态 | 与判题直调形态的差距 | 本题可用性判定 | 理由 | 证据 |
|---|---|---|---|---|---|---|---|---|
| **msopgen** | CANN 工具链（华为官方） | Ascend C 算子工程骨架（空实现需补） | 是（aicore） | 多文件工程：`op_host/ op_kernel/ op_proto/ CMakeLists.txt` + aclnn/ATC 部署；核函数为 `__global__ __aicore__` | ① 多文件非单文件；② aicore 非 `__vector__`；③ 无 `run_kernel` 直调入口；④ 需 build/deploy 流程 | **不可用（作提交）**；结构可作研究参考 | 默认产物是标准算子工程，需 CMake 编译为 `.run`/`.so` 后由 aclnn 调用，与 CANNJudge 单文件直调不符 | S2(A) |
| **PyPTO 0.2.0** | CANN 生态（华为，Python DSL） | PTO 算子（Tensor→Tile→Block→Execution 四层 IR），Bisheng 编译为二进制 | 是（CANN 9.0.0 对齐） | 编译后二进制 / 算子图（MPMD 执行）；前端为 `.py` + Tile 编程 | ① 产物是编译二进制/算子图，非 `kernel.asc`；② 无 `run_kernel` + `__vector__` 直调入口 | **不可用（作提交）**；算法/PTO 指令可作研究参考 | 0.2.0 明确支持 CANN 9.0.0，但运行时是 NPU MPMD 调度，非单文件直调；且需 CANN 9.0.0 + 目标硬件（950PR/A2/A3） | S7(B) |
| **TileLang-Ascend** | TileLang 社区分叉（基于 TVM） | Ascend C / PTO 源文件 | 是（AscendC 后端 + PTO 后端，A2/A3/A5） | JIT 编译出 **`.so` 动态库**；核函数为 `__global__ __aicore__` | ① `.so` 非 `kernel.asc`；② `__aicore__` 非 `__vector__`；③ 无 `run_kernel` 直调 | **不可用（作提交）**；reduction/last-dim/BF16 思路可作研究参考 | 明确生成 aicore 核函数并经 JIT→`.so`，与直调形态不符；CANN 9.0.0 支持未确认 | S3(C)/S4(C)/S5(C) |
| **hivm（AscendNPU IR）** | 昇腾内部/社区 IR（gitcode） | hivm dialect IR（含 `vreduce`/`vrsqrt`/`vrelu`） | 是（面向 Ascend NPU） | 编译器中间表示，非直接生成可读 `kernel.asc` | ① 是 IR 而非完整提交文件；② 未提供「直出 kernel.asc」证据 | **不可用（作提交）**；算子分解（`vreduce`+`vrsqrt`）可作研究参考 | 文档仅描述 dialect 与 pass，未说明能产出单文件直调核；属内部 IR 研究原型 | S6(C) |
| **IREE** | LLVM/MLIR 系（LF AI 沙盒） | SPIR-V / 原生码 / Vulkan/ROCm/CUDA/Metal/AMD AIE | **否（无 Ascend 后端）** | 编译后二进制 + runtime 调度 | 根本无法生成 Ascend 产物 | **不可用** | 官方文档列出的目标仅 CPU/GPU/Apple/Vulkan/ROCm/CUDA/AMD AIE，无 Ascend；仅 lowering-config 思路可借鉴 | S8(A) |
| **上游 TVM** | Apache（社区） | Relay IR → TIR → 各后端 codegen | **否（无上游 Ascend 后端）** | 各后端 kernel（CPU/GPU 等） | 无 Ascend codegen；TBE 是华为内部分叉，非上游 | **不可用** | 上游 TVM 不含 Ascend 后端；CANN 基于 TVM 设计思想但为华为内部实现（TBE/DSL 已属旧路径） | S9(C)/S10(C) |
| **TorchInductor** | PyTorch（Meta/社区） | C++/OpenMP/Triton kernel | **否（CPU/GPU）** | 生成 C++/Triton 源码 | 无 Ascend codegen | **不可用（作提交）**；reduction/BF16/尾处理思路强借鉴 | CPU 后端 BF16 legalization、双循环尾处理、reduction 向量化均有公开设计，可直接转译到 Ascend C | S11(A)/S12(B)/S13(B) |
| **LLVM Loop Vectorizer** | LLVM（官方） | 主机端 SIMD 向量化（x86/ARM 等） | **否（通用 CPU）** | 主机汇编/目标码 | 与 Ascend 无关 | **不可用（作提交）**；reduction lowering + 标量尾循环思想可作参考 | 官方文档描述 reduction 向量化与 epilogue（标量/向量/尾折叠）尾处理，思想可迁移 | S14(A)/S15(C) |

---

## 3. 三分类判定

### 3.1 可用于本题（直接产出可提交 `kernel.asc`）
**无。** 经全面检索，没有任何工具/DSL 能够直接产出「单个 `kernel.asc` + `run_kernel` + `__global__ __vector__` 直调核」这一判题要求形态。最接近的单文件/单核输出（msopgen 的 aicore、TileLang 的 aicore、PyPTO 的二进制）在「单文件 / `__vector__` 关键字 / `run_kernel` 入口 / 直调启动」四个维度上全部不满足。

### 3.2 仅作研究参考（算法/IR 思想可借鉴，但不能作为提交物）
- **TileLang-Ascend**：真实的 Ascend 后端，提供 `reduce_sum/reduce_max`、last-dim reduction、`vid reduction`、`workspace reduction`、BF16 类型映射、自动 CV ratio 等完整实现；其 reduction lowering 与 UB 分配策略对本题手工实现极有参考价值。但产物是 `__aicore__` + JIT `.so`，非 `kernel.asc`。
- **hivm（AscendNPU IR）**：其内部 `hivm.hir.vreduce`、`hivm.hir.vrsqrt` 直接对应 RMSNorm 的「求和 + 倒数平方根」分解，且带 `VectorCoreTypeTrait (PIPE_V)`，是 vector 核语义的现成参考；但它是 IR/研究原型，非生成器。
- **PyPTO 0.2.0**：唯一明确对齐 CANN 9.0.0 的 DSL，提供 `TROWSUM/TCOLSUM/TROWMAX`（轴归约）、`TCVT`（类型转换）等 PTO 指令；其「Tensor→Tile→Block」分块与 SRAM 自动规划思路可直接借鉴到手写 Tiling。但产物是 Bisheng 二进制/算子图，非 `kernel.asc`。
- **TorchInductor**：BF16 legalization（以 FP32 计算再转回 BF16）、reduction 的「主循环 + 带 mask 尾部循环」双循环尾处理、reduction 在归约维/并行维的向量化——三条思路可直接转译到 Ascend C。
- **LLVM Loop Vectorizer**：reduction 变量展开为向量、末尾水平归约；trip count 非 VF 整数倍时的 scalar/vectorized epilogue 与 tail folding——对应 Ascend 中「D 非对齐时的尾部掩码处理」通用范式。
- **IREE lowering-config**：`partial_reduction` / `thread` / `workgroup` tile sizes、`expand_dims`（拆分归约维做更细粒度累加）的 reduction 策略，是「沿最后一维归约的分块与部分累加」的权威设计参考（虽无 Ascend 后端）。
- **TVM / Ansor**：compute/schedule 分离、基于代价模型的自动调度搜索（autotuning）——搜索空间与调优方法可借鉴到手写 Tiling 的调参。

### 3.3 不可用（无 Ascend 后端 / 旧路径 / 形式完全不符，无法用于本题）
- **IREE（整工具）**：官方明确无 Ascend 后端，不能生成任何昇腾产物；只能借鉴其 published 的 reduction lowering 思路（已在 3.2 引用）。
- **上游 TVM（整工具）**：上游不含 Ascend 后端；CANN 基于其设计思想的 TBE/DSL 是华为内部实现且属**已淘汰路径**（当前推荐 Ascend C），与本题 CANN 9.0.0 的 `__vector__` 直调形态不符。
- **msopgen 产物（作提交）**：生成的是多文件 aicore 标准算子工程 + aclnn/ATC 部署流程，核函数为 `__global__ __aicore__` 并带 `GM_ADDR workspace/tiling` 参数，**不是单文件 `kernel.asc` + `run_kernel` + `__vector__`**。注意：msopgen 工具本身可用于「搭框架/看 Tiling 模板」作参考，但其默认产物不可直接提交。
- **TBE DSL / TIK（CANN 旧算子开发路径）**：已被 Ascend C 取代，且同样是多文件 op 工程形态，与本题 vector kernel 直调形态不符，不在 CANN 9.0.0 推荐路径内。

---

## 4. 值得手工借鉴的具体技术手段（≥5 条）

> 以下技术均可在手写 Ascend C `kernel.asc` 的 `run_kernel` 内实现，来源为研究参考类工具/编译器。

1. **双循环尾处理（主循环对齐块 + 带 mask 尾循环）**
   - 出处：TorchInductor（pytorch #148402，2023）。当归约维 `rnumel` 非块对齐（如 BF16 的 stride padding 到 64 倍数），将 reduction 循环拆为：主循环处理 `[0, rnumel_rounded)`，剩余 `[rnumel_rounded, rnumel)` 用带 `mask` 的循环处理。
   - 用到本题：AddRmsNormBias 的 `D` 可能非 32 倍数。在 Ascend 上用 `WholeReduce` 处理完整 32/256 位块，剩余尾部用带 mask 的标量/短向量累加，避免越界读写。
   - 风险：mask 边界（当 `D` 小于一个完整 vector 宽度）需特殊处理；尾部不能用 `WholeReduce` 全宽加载。

2. **BF16/FP16 legalization（以 FP32 累加，末端转回）**
   - 出处：TorchInductor CPU BF16 推理路径（dev-discuss.pytorch.org，PyTorch 2.1）。对所有非 GEMM/Conv 的逐元素与归约运算，先把 BF16 载入为 FP32 计算，结果再转回 BF16，复用向量化支持并保证精度。
   - 用到本题：RMSNorm 的 `sum(y^2)` 与 `mean` 一律用 **FP32 累加器**（即便输入是 BF16/FP16），`rms=sqrt(mean+eps)` 也用 FP32，最后乘 `gamma+bias` 时再 cast 回原 dtype。这直接避免 BF16 累加的精度坍塌，与「BF16 累加」问题强相关。
   - 风险：每次加载/存储多一次 cast，有带宽/指令开销；需确认 Ascend C 的 `Cast` 指令支持 fp16/fp32 互转且性能可接受。

3. **向量累加器 + 单次水平归约（last-dim reduction 策略）**
   - 出处：MLIR Discourse「New Linalg Code Generation Strategy for Innermost Reductions」（#64596，2022）；LLVM PR #97788（2024-07）允许对**最内（trailing）维**做可扩展向量化归约。
   - 用到本题：`D` 恰为最后一维，天然是 trailing-dim reduction。可维护一个 **vector 部分累加器**（如 256 位向量跨多个 `D` 块累加），在归约循环结束后做**一次水平归约**得到标量 rms。这比「每元素标量累加 + 反复水平归约」更高效，与 Ascend vector 核「每行一个核、向量累加」模型一致。
   - 风险：归约顺序改变 → 浮点结果相对严格顺序略有差异；RMSNorm 对顺序不敏感，可接受，但需在精度验证时确认。

4. **partial_reduction 分块 + expand_dims（归约维切分）**
   - 出处：IREE lowering-config 文档（iree.dev/developers/lowering-config）。`partial_reduction` tile size 把归约维 `r → r_outer, r_partial`，线程维持 `r_partial` 部分累加器，循环结束后合并；`expand_dims` 可进一步拆分归约维做更细粒度累加。
   - 用到本题：当 `D`（如 32768）超过片上 UB 单次可容纳宽度时，将 `D` 切成多个 partial 块（例如每块 1024~8192），每块内向量累加，块间串行合并；`expand_dims` 思想对应「先按核分行、再按 UB 分块」的两级 Tiling。
   - 风险：UB 容量预算需精确（gamma/bias/rms/中间 y 缓冲叠加），须做 static memory plan 防止溢出。

5. **轴归约 + 类型转换的算子分解（TROWSUM / TCVT）**
   - 出处：PyPTO 0.2.0 PTO 指令集（TROWSUM 沿轴 sum、TCVT 类型转换）；hivm `vreduce`+`vrsqrt` 同理。
   - 用到本题：把 AddRmsNormBias 显式拆成 `(1) y=x+residual` → `(2) TROWSUM(y*y)` 得 sumsq → `(3) rms=vrsqrt(sumsq/D+eps)` → `(4) out=y*rms*gamma+bias`，每步用 TCVT 管理 fp16/fp32 往返。这种「elementwise + reduction 融合」分解是 kernel fusion 的直接蓝本。
   - 风险：融合粒度需平衡 UB 占用与流水；过细会增加同步，过粗会爆缓冲。

6. **自动 memory planning / UB 分配与双缓冲**
   - 出处：hivm `-hivm-plan-memory` pass；TileLang `T.alloc_shared` + 自动 CV ratio + workspace reduction；PyPTO 据 tile shape 自动生成内存访问指令与 SRAM 分配。
   - 用到本题：手写时按 `y(临时)`、`rms(标量/向量)`、`gamma/bias(只读)`、`output` 规划 UB 队列与乒乓双缓冲（Pipe 多 buffer），参考上述工具「据 tile size 自动推算 SRAM」的思路做静态预算。
   - 风险：多 buffer 与同步标志（Set/WaitFlag）需手动对齐，否则跨核/跨流水数据错乱。

7. **自动调优搜索（tile size / 向量宽度 / 双缓冲级数）**
   - 出处：TVM/Ansor 基于代价模型的调度搜索；TileLang autotuning 系统；AscendOptimizer 的 profiling-in-the-loop 演化搜索（arXiv 2026）。
   - 用到本题：选定候选 Tiling 参数空间（每行/核处理元素数、vector 宽度、partial 块大小、双缓冲级数），在小规模样例上做穷举/贝叶斯搜索选优。**注意：本机无 NPU，无法本地跑 profiling**，只能把搜索空间设计方法借鉴到「在判题环境手动试参」。
   - 风险：无硬件反馈则调优无效；且自动调优工具本身不能产出 `kernel.asc`。

---

## 5. 与 CANN 9.0.0 的兼容性分析

| 工具 | 明确对齐 CANN 9.0.0？ | 说明 |
|---|---|---|
| **PyPTO 0.2.0** | ✅ 是 | 发布说明明确「0.2.0 — CANN 9.0.0 — 2026-04」，支持 950PR/A2/A3。但因产物是 Bisheng 二进制/算子图，**对齐版本 ≠ 可提交 `kernel.asc`**。 |
| **msopgen** | ⚠️ 随包版本 | msopgen 随 CANN 工具包发布，CANN 9.0.0 自带对应版本，可生成 Ascend C 工程；但产物形态不符合直调要求。 |
| **TileLang-Ascend** | ❓ 未确认 | 文档要求 CANN ≥ 8.3.RC1 + torch-npu，未明确列出 9.0.0；作为社区分叉，9.0.0 适配需自行验证。 |
| **hivm IR** | ❓ 未确认 | 属内部/社区 IR 研究原型，无公开版本-对照表。 |
| **IREE / 上游 TVM / TorchInductor / LLVM** | N/A | 均无 Ascend 后端，版本兼容性对「提交昇腾代码」无意义；其价值仅为思想借鉴。 |

**关键兼容性结论**：
- 唯一**版本对齐** CANN 9.0.0 的生成型工具是 **PyPTO 0.2.0**，但它解决的是「算子开发效率」，而非「判题直调提交形态」——二者正交。
- 所有生成型工具都没有「`__vector__` 直调核 + `run_kernel` + 单文件」这一形态，因此**即便版本对齐，也无法替代手写 `kernel.asc`**。
- 本题实际可行路径仍是：**手工编写 Ascend C `kernel.asc`**，上述工具的 reduction/尾块/BF16/memory-planning 模式作为实现参考。

---

## 6. 未确认事项

1. **TileLang-Ascend 对 CANN 9.0.0 的适配状态**：官方文档仅标注 CANN ≥ 8.3.RC1，未列 9.0.0；其生成的 `__aicore__` 核是否能在 9.0.0 的 vector 核路径运行需实测（本机无法验证）。
2. **hivm IR 是否对外开放为可调用生成器**：目前仅见 dialect/pass 文档，未见「输入算子描述 → 输出 kernel.asc」的完整工具链证据。
3. **CANNJudge 直调工程是否允许除手写外的任何辅助文件**：依据题面，提交物为单一 `kernel.asc`，故所有外部生成物（`.so`/工程/二进制）均不适用；具体判题加载细节超出本主题范围（由其他代理负责）。
4. **PyPTO 0.2.0 在 CANN 9.0.0 上的实际算子覆盖**：其 `TROWSUM` 等是否覆盖「elementwise + 沿最后一维归约 + 逐元素缩放」的融合模式，需查其算子清单（本机无环境，未深究）。
5. **Ascend C 在 CANN 9.0.0 下 `WholeReduce` / 向量归约 / mask 尾处理的具体 API 形态**：依题面约束，不展开 Ascend C API 细节（由其他代理负责）。

---

## 7. 来源表

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 |
|---|---|---|---|---|---|---|
| S1 | LLVM Discourse / PR #97788 可扩展向量化 trailing-dim 归约 | https://github.com/llvm/llvm-project/pull/97788 ； https://lists.llvm.org/pipermail/mlir-commits/2024-July/064289.html | LLVM/MLIR 2024-07（main） | 2026-09-11 | last-dim reduction 可扩展向量化、归约前置条件 | B |
| S2 | 鲲鹏昇腾社区：msopgen 工程标准目录 / Ascend C 单算子工程 | https://hwcomputing.csdn.net/695f741e0846ec2c4c5adb4c.html | 社区文章（无版本） | 2026-09-11 | msopgen 产物结构、aicore 核函数形态 | C |
| S3 | MindStudio 官方文档：msopgen 快速入门 | https://mindstudio-docs-master.readthedocs.io/zh-cn/latest/msot/docs/zh/quick_start/op_tool_quick_start/ | 官方文档（无明确版本） | 2026-09-11 | msopgen 命令与生成工程结构 | A |
| S4 | CANN 官方：自定义算子开发快速入门（AscendC） | https://developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-operator-development-0000002122321604 | 更新 2026-01-04 | 2026-09-11 | aicore 核函数签名、`op_kernel`/`op_host` 结构 | A |
| S5 | TileLang 官方文档 | https://www.tilelang.com/index.html | TileLang 0.1.14 | 2026-09-11 | TileLang 定位（基于 TVM 的 GPU/CPU DSL） | B |
| S6 | TileLang-Ascend：Reduction Operations（DeepWiki） | https://deepwiki.com/tile-ai/tilelang-ascend/8.5-reduction-operations | tile-ai/tilelang-ascend（社区分叉） | 2026-09-11 | Ascend 后端 reduce_sum/WholeReduce/BlockReduce、last-dim 归约 | C |
| S7 | TileLang-Ascend：Quick Start（DeepWiki） | https://deepwiki.com/tile-ai/tilelang-ascend/3-quick-start-guide | tile-ai/tilelang-ascend | 2026-09-11 | JIT→.so、Ascend C codegen、`is_npu`、自动 CV ratio | C |
| S8 | TileLang-Ascend：Code Generation（DeepWiki） | https://deepwiki.com/tile-ai/tilelang-ascend/7.2-warp-specialization | tile-ai/tilelang-ascend | 2026-09-11 | CodeGenTileLangAscend 生成 `__global__ __aicore__`、PTO 后端 | C |
| S9 | AscendNPU IR：hivm Dialect | https://ascendnpu-ir.gitcode.com/en/developer_guide/dialects/HIVMDialect.html | 社区/内部 IR（无版本） | 2026-09-11 | `vreduce`/`vrsqrt`/`vrelu` 等 vector 核算子、PIPE_V | C |
| S10 | AscendNPU IR：hivm Passes | https://ascendnpu-ir.gitcode.com/en/developer_guide/passes/HIVMPasses.html | 社区/内部 IR | 2026-09-11 | `-hivm-plan-memory`/`map-forall-to-blocks` 等 memory planning 与并行映射 | C |
| S11 | IREE 官方站 | https://iree.dev/ | IREE（LF AI 沙盒） | 2026-09-11 | 支持的 targets/后端列表（无 Ascend） | A |
| S12 | IREE Lowering Configs | https://iree.dev/developers/lowering-config/ | IREE 官方 | 2026-09-11 | `partial_reduction`/`thread`/`workgroup` tile、`expand_dims` reduction 策略 | A |
| S13 | 昇腾 CANN 基于 TVM 设计（TBE/DSL） | https://sciencedirect.publicaciones.saludcastillayleon.es/topics/computer-science/ai-tools （引自《Ascend AI Processor Architecture and Programming》, 2020） | 2020 书籍 | 2026-09-11 | CANN 复用 TVM 后端代码生成思想、TBE DSL | C |
| S14 | TVM 深度学习编译技术综述 | https://ictjournal.itri.org.tw/... （工研院） | 综述 | 2026-09-11 | TVM compute/schedule 分离、autotuning 思想 | C |
| S15 | TorchInductor Update 6：BF16 推理路径 | https://dev-discuss.pytorch.org/t/torchinductor-update-6-cpu-backend-performance-update-and-new-features-in-pytorch-2-1/1514 | PyTorch 2.1 | 2026-09-11 | BF16 legalization（载入 FP32 计算再转回） | A/B |
| S16 | TorchInductor：reduction 双循环尾处理（pytorch #148402） | https://githubissues.com/pytorch/pytorch/148402 | PyTorch issue（2023） | 2026-09-11 | 主循环对齐 + 带 mask 尾部循环处理非对齐归约维 | C |
| S17 | TorchInductor Update 9：向量化硬化 | https://dev-discuss.pytorch.org/t/torchinductor-update-9-harden-vectorization-support-and-enhance-loop-optimizations-in-torchinductor-cpp-backend/2442 | PyTorch（2023） | 2026-09-11 | reduction 在归约维/并行维向量化、任意层级向量化、尾拆分 | B |
| S18 | LLVM Auto-Vectorization 官方文档 | https://llvm.org/docs/Vectorizers.html | LLVM 官方 | 2026-09-11 | reduction 向量化、FP 不结合性、`-fassociative-math` 约束 | A |
| S19 | LLVM VPlan / Epilogue 尾处理（DeepWiki） | https://deepwiki.com/espressif/llvm-project/3-llvm-optimization-infrastructure | LLVM（社区解读） | 2026-09-11 | Scalar/Vectorized epilogue 与 tail folding 三种尾处理策略 | C |
| S20 | PyPTO 框架（cann-recipes-infer, DeepWiki） | https://deepwiki.com/chenqi123/cann-recipes-infer/7-pypto-framework | cann-recipes-infer | 2026-09-11 | PyPTO 架构、PTO 指令（TROWSUM/TCVT）、Bisheng 编译、MPMD | C |
| S21 | PyPTO 版本与硬件支持（CANN 开发者社区） | https://cann.csdn.net/6a62ad0410ee7a33f291ddcc.html | PyPTO 0.1.0/0.1.2/0.2.0 | 2026-09-11 | **0.2.0 对齐 CANN 9.0.0**；四层 IR；仿真模式 | B |
| S22 | PyPTO 实战（DeepSeek-V3.2-Exp, 腾讯云） | https://cloud.tencent.com/developer/article/2593245 | 社区长文 | 2026-09-11 | PyPTO 作为算子 DSL/编排层、tile_fwk 原子算子 | C |
| S23 | AscendOptimizer（arXiv 2026） | https://arxiv.org/html/2603.23566v1 | arXiv 2026 | 2026-09-11 | AscendC 自动优化、profiling-in-the-loop 演化搜索、两部件 artifact | C |

> **证据等级说明**：A=官方文档/官方仓库；B=官方样例/RFC/官方教程；C=社区文章/论坛/分叉文档。本主题未使用 D（仅搜索摘要）级来源作为结论依据。所有版本结论均标注，过时 RFC（如 S1 的 2024-07 main 分支、S13 的 2020 书籍）已注明适用范围，不当作当前事实。
