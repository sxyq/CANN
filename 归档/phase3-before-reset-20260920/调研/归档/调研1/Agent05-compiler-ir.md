# Agent 5：编译器 / IR / 算子生成工具调研报告

> 调研对象：AddRmsNormBias（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，Ascend C vector Kernel）。
> 语义：`y = x + residual`；`rms = sqrt(mean(y², dim=-1) + epsilon)`；`output = y/rms*gamma + bias`。
> 约束：FP16/BF16/FP32，2D/3D/4D，沿最后一维 D 归约，D 可非 32 倍数。
> 本机：macOS，无 CANN、无 NPU。本轮只做能力评估，**所有"能生成/能编译"表述均为对公开资料的判断，一律未在真实 NPU 验证**。
> 判题入口：CANNJudge 直调模板，只能提交平台指定文件（如 `kernel.asc`），不是开放工程构建。

调研日期：2026-09-11。证据等级：A=官方文档/官方仓库源码；B=官方样例/教程；C=社区。

---

## 0. 核心结论（先行）

1. **没有任何自动生成器可以直接产出 CANNJudge 可提交的 `kernel.asc`**——本轮未找到反例证据，该结论成立。唯一官方"生成"工具 msopgen 生成的是**工程脚手架**（`op_host/op_kernel/CMake` + 算子注册 + Tiling 框架），**核心 kernel 算法仍须手工编写**；其余编译器（Triton-Ascend、AKG、TileLang-Ascend、XLA-NPU、PyPTO、TorchInductor 昇腾链路）要么输出**编译后二进制**，要么依赖 CANN/BiSheng/torch_npu 完整工具链运行，产物形态、函数签名与直调模板（单文件 `kernel.asc`，`GM_ADDR` 参数签名）均不匹配。
2. 因此 **kernel.asc 必须由人工（或脚本辅助人工）按直调模板手写 Ascend C**；自动生成器对本轮的正确用法是"**参考其生成逻辑 / 降级产物**"，不是直接产出提交物。
3. **可借鉴的编译层思路充分**：归约树/分段归约、FP32 累加提升、DMA-vector 双缓冲流水线、尾块掩码/填充、elementwise 链融合、liveness 内存复用、tiling 自动调优——详见第 2 节，均可迁移到本题手写实现。
4. **事实修正**：任务背景中"PyPTO（github.com/pypto/pypto，AMD 的）"有误。真实的 PyPTO 是**华为昇腾**的高性能编程框架（上游仓库 `hw-native-sys/pypto`，现由 `hicann/pypto` 承载），面向昇腾虚拟指令集 PTO-ISA，与 AMD 无关。

---

## 1. 工具 / 框架能力表

| # | 工具/框架 | 能否生成 Ascend C vector kernel | 证据（文档/源码链接，访问 2026-09-11） | 直调模板下是否可直接使用 | 参考价值 |
| --- | --- | --- | --- | --- | --- |
| 1 | **msopgen**（CANN 官方，MindStudio Ops Generator） | 否（生成**工程骨架**，非算法；kernel 为待填充模板） | [Ascend/msopgen](https://github.com/Ascend/msopgen)（2025-12-31 全面开源；2026-02-10 适配 AscendC 新工程）；[工具参数说明](https://developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-creating-operator-project-msopgen-0000002293225826)（`msopgen gen -i x.json -c ai_core-<soc> -lan cpp -out <dir>`，生成 op_host/op_kernel/CMake）；[快速入门](https://device.harmonyos.com/cn/docs/apiref/harmonyos-guides/cannkit-operator-development) | **部分可用**：生成的 op_kernel 源码可作为 kernel.asc 的起点（本项目已按此组织 `提交/首版/kernel.asc`）；但 msopgen 不生成计算逻辑 | **高**（工程结构、注册、Tiling 框架与直调模板对齐的唯一官方途径） |
| 2 | **TVM 主线**（apache/tvm） | 否（`src/target/` 仅 llvm/source/canonicalizer，**无 ascend codegen**） | [TVM src/target 目录（GitHub API）](https://api.github.com/repos/apache/tvm/contents/src/target) | 否 | **中**（TIR 调度/归约/流水线思路是 TileLang-Ascend 的基础，间接可借鉴） |
| 3 | **TileLang-Ascend**（tile-ai/tilelang-ascend） | **是**（专为昇腾的 DSL→编译器：AscendC 后端生成 AscendC C++ 源码（LocalTensor/GlobalTensor + Catlass 模板），PTO 后端生成 PTO 代码；支持 A2/A3/A5） | [tile-ai/tilelang-ascend](https://github.com/tile-ai/tilelang-ascend)（2025-09-29 开源；ascendc_pto 与 npuir 两分支）；[Ascend NPU Code Generation（deepwiki 源码索引）](https://deepwiki.com/tile-ai/tilelang-ascend/7.2-ascend-npu-code-generation)（`src/target/codegen_ascend.cc`、`src/tl_templates/ascend/common.h`、Pass：TL_ASCEND_AUTO_CV_COMBINE / AUTO_CV_SYNC / AUTO_SYNC / MEMORY_PLANNING）；[elementwise 测试](https://raw.githubusercontent.com/tile-ai/tilelang-ascend/ascendc_pto/testing/python/language/test_tilelang_ascend_language_elementwise.py)（`T.Kernel(is_npu=True)`、`T.reduce_sum`、`reduce_ascend_lang`） | **否**：生成源码依赖 tl_templates（Catlass）与 CANN 8.3+ 编译为算子二进制，经 torch_npu 运行；产物非直调模板格式。理论上可人工提取 kernel 片段改编，工作量大、接口需重写 | **高**（最接近"自动生成 Ascend C"的工具；其归约、自动同步、内存规划 pass 直接对应本题手写点） |
| 4 | **Triton-Ascend**（triton-lang/triton-ascend，昇腾官方组织） | 部分（编译链：Triton IR → Linalg IR → AscendNPU IR → `triton_xxx_kernel.o`（BiSheng 编译），输出**二进制 kernel**，非 Ascend C 源码） | [triton-lang/triton-ascend](https://github.com/triton-lang/triton-ascend)（2025-05-20 开源；3.2.2 于 2026-07-31 发布；支持 Atlas A2/A3/950，CANN 9.1.0，TorchNPU 2.7.1.post8）；[架构设计](https://github.com/triton-lang/triton-ascend/blob/main/docs/en/architecture_design_and_core_features.md)；[README_zh](https://github.com/triton-lang/triton-ascend/blob/main/README_zh.md) | **否**（二进制 + CANN/TorchNPU 运行时驱动） | **高**（reduce/融合/autotune 的昇腾语义映射；mask、多核切分写法可对照） |
| 5 | **IREE**（iree-org/iree） | 否（官方支持矩阵：Vulkan / ROCm/HIP / CUDA / Metal / AMD AIE(实验) / WebGPU(实验)，**无 Ascend**；仓库亦无 Ascend 后端代码与 issue 讨论） | [iree.dev 支持矩阵](https://iree.dev/index.html)；[IREE 仓库](https://github.com/iree-org/iree)；[IREE RVV issue #24576](https://github.com/iree-org/iree/issues/24576)（反证：连 RISC-V 加速器都走 LLVM-CPU 而非新 HAL，无 Ascend 痕迹） | 否 | **中**（MLIR bufferization、软件流水线、数据移动显式化思路可迁移；AMD AIE 的显式内存/同步模型与昇腾 UB 管理相似） |
| 6 | **OpenXLA / XLA-NPU** | 否（OpenXLA 主线无昇腾后端；华为 fork `cann/xla-npu` 接入 OpenXLA 生态对接 CANN，面向 **JAX/图编译推理**，三种策略 AFIR（AI Fusion IR，MLIR 融合）/ GE / Aclnn，产物是图级可执行，非单 kernel） | [cann/xla-npu（gitcode）](https://gitcode.com/cann/xla-npu)（转述：[XLA-NPU 编译后端解析（CSDN）](https://blog.csdn.net/gitblog_07156/article/details/151463150)） | 否 | **低-中**（融合策略命名与"融合 vs 单算子"取舍可参考，层级太高） |
| 7 | **TorchInductor（昇腾链路）** | 否（PyTorch 上游 Inductor 无昇腾后端；昇腾经 torch_npu（PrivateUse1 机制）接入，`torch.compile` 在 NPU 走 Inductor→MLIR/Triton→CANN Runtime 链路；社区 `npu_inductor_mlir` 走 torch-mlir + BiSheng 编译器；产物为图编译结果） | [TorchNPU（Ascend/pytorch）](https://github.com/ascend/pytorch)；[PyTorch Shanghai Meetup（Ascend 多后端）](https://pytorch.org/blog/pytorch-shanghai-notes/)；[npu_inductor_mlir（gitee）](https://gitee.com/rmch/npu_inductor_mlir2)；[昇腾 for PyTorch 训练营笔记（③④⑤ 链路）](https://bbs.huaweicloud.com/blogs/482845) | 否 | **中**（fusion 的 scheduler 表达、tiling 配置如 `config.ascend.cube_tile_shape` 可参考；但纯图模式，非单算子直调） |
| 8 | **AKG**（mindspore-ai/akg，MindSpore 自动算子生成器） | 部分（基于多面体编译（Polyhedral），支持 GPU/Ascend **自动生成高性能 kernel**；现含 AKG-MLIR 子项目，支持 Atlas 800T A2/A3；产物经 TVM/MLIR 编译链为二进制） | [mindspore-ai/akg](https://github.com/mindspore-ai/akg)；[图算融合加速引擎（官方文档）](https://www.mindspore.cn/docs/zh-CN/r1.10/design/graph_fusion_engine.html)；[PLDI 2021 AKG 论文演讲材料（IMPACT 2022 keynote）](https://acohen.gitlabpages.inria.fr/impact/impact2022/slides/keynote.pdf) | 否（二进制，且绑定 MindSpore 图算融合上下文） | **中-高**（昇腾上"自动生成 kernel"的官方先例；polyhedral 调度/融合/切分思路可借鉴） |
| 9 | **PyPTO**（hicann/pypto，**华为昇腾**；任务背景误记为 AMD） | 部分（Python DSL → 多层 IR → **PTO-ISA（昇腾虚拟指令集）** → ge 图编译 → NPU 可执行指令；是**指令级**编程，不是 Ascend C 源码；v0.1.0 于 2026-01-06 发布，支持 A2/A3） | [hicann/pypto](https://github.com/hicann/pypto)；[PyPTO 上游 hw-native-sys/pypto（fork 视图）](https://github.com/tsung-li/pypto)；[pypto 上手（PTO 虚拟指令集，CSDN）](https://blog.csdn.net/2502_93572233/article/details/161360732) | 否（指令级产物 + 运行时，与直调模板无关） | **中**（tile 编程模型、显式内存布局与 Ascend C 手工管理可对照；但比 Ascend C 更低层，迁移成本高） |
| 10 | **MLIR / LLVM 生态（HIVM、AscendNPU-IR）** | 否（MLIR 主线无 Ascend 方言；华为 2012 实验室 HIVM 构建"MLIR 方言栈 → AscendNPU-IR"（AscendNPU-IR 同时被 triton-ascend 与 tilelang-ascend npuir 分支使用）） | [HIVM: MLIR Dialect Stack for Huawei Atlas NPU（LLVM 开发者会议 2026-04 教程 PDF）](https://www.llvm.org/devmtg/2026-04/slides/tutorial/tutorial_tarasov.pdf) | 否 | **低-中**（了解昇腾 IR 结构；对本题手写 Ascend C 无直接输入） |
| 11 | **FlagTree**（flagos-ai/flagtree） | 部分（Triton 兼容的统一多后端编译器；2025-06-04 加入 ascend 后端，2025-09-25 支持 `flagtree_hints` 编译指导；与 Triton-Ascend 同族，产物为二进制） | [FlagTree（gitee 镜像）](https://gitee.com/flagos-ai/flagtree) | 否 | **低**（Triton-Ascend 的再封装，无额外昇腾降级信息） |

**一句话判断**：能"生成昇腾代码"的只有 TileLang-Ascend（Ascend C 源码级）、Triton-Ascend/AKG/FlagTree（二进制级）、PyPTO（指令级）；**全部需要昇腾本机 CANN 工具链才能编译/运行，且产物都不是直调模板的 `kernel.asc`**。

---

## 2. 可借鉴的编译层通用思路（本题手写 kernel 的输入清单）

以下思路均来自上述编译器/IR 体系，全部可迁移到本题 `add + rmsnorm + affine` 的 Ascend C 手工实现。每条给"编译器怎么做的"→"本题怎么用"。

### 2.1 归约：树形/分段归约 + 归约因子（reduction factor）
- **编译器做法**：TVM/TileLang/MLIR 把一维长归约拆成"块内向量化归约（reduction factor 对齐 vector 宽）→ 部分和 → 再合并"（split-reduction / tree reduction）；Triton-Ascend 的 `tl.reduce` 在昇腾上落到逐 repeat 的部分和链。
- **本题用法**：D 大（如 4096/32768）时，把 D 归约拆成 `ceil(D/vecLen)` 段，每段用向量平方+局部累加，最后标量链式合并；比一次长链 ReduceSum 更可控，也便于与流水线结合。参考 `reduce_ascend_lang`（tilelang-ascend）与已有 `ReduceSum(workLocal)` 调研。

### 2.2 归约累加类型提升（fp16/bf16 → fp32 accum）
- **编译器做法**：Triton `tl.sum(..., acc_dtype=)`、MLIR arith 归约的 accumulation type 提升；TileLang reduce 的 accum dtype 参数；主流编译器默认 fp16 归约提升到 fp32 以控精度。
- **本题用法**：`y²` 与 `sum(y²)` 一律先在 FP32 中算（输入 Cast 到 float），FP16/BF16 仅保留在搬入/搬出边界——与 AGENTS.md 的"归约及中间累加优先使用 FP32"一致；题面 epsilon 与 rsqrt 也按 FP32。

### 2.3 内存：双缓冲 / 软件流水线（double buffering / pipelining）
- **编译器做法**：TVM MetaSchedule 的 pipeline 变换、Triton `num_stages`、IREE 的软件流水线 pass、tilelang 的 `TL_ASCEND_AUTO_CV_SYNC`（自动插入 DMA 与 vector 计算的同步/重叠）。
- **本题用法**：GM→UB 的 MTE2 与 UB 内 vector 计算重叠；对 multi-tile 的 outer 行循环用双缓冲（TQue 两槽位轮换）隐藏搬运延迟——对应 Ascend C 的 `EnQue/DeQue` 与 `SetFlag/WaitFlag` 手工编排。

### 2.4 尾块：掩码 / 谓词 / 填充（predication & padding）
- **编译器做法**：Triton `tl.load/store(..., mask=)`、TVM ramp+predicate、TileLang 对非整除 tile 的边界处理；IREE/MLIR 的 `vector.mask`。
- **本题用法**：D 非 32 倍数时，搬入用 `DataCopyPad`（补零到 32 对齐）或掩码搬运，归约/输出边界必须限定在有效 D 内，不得把填充覆盖进相邻行——直接对应题面"尾块不能覆盖相邻行"约束与已有调研（`DataCopyPad`/`StoreUnAlign` 等）。

### 2.5 融合：elementwise 链单 kernel 化（kernel fusion）
- **编译器做法**：TVM/Inductor 把 `add → square → reduce → rsqrt → mul → add` 视为同一 kernel 内的循环融合，消除中间张量落 GM；XLA-NPU 的 AFIR 也是同类融合策略。
- **本题用法**：residual add、平方、归约、rsqrt、gamma 缩放、bias 加全部在单个 vector kernel 内完成，中间量留在 UB（或 UB 内复用缓冲），全程零中间 GM 往返——这是性能题的基本盘。

### 2.6 内存规划：liveness 分析与缓冲复用
- **编译器做法**：MLIR bufferization 的 liveness-based buffer 复用、tilelang 的 `TL_ASCEND_MEMORY_PLANNING`（自动 buffer 复用）。
- **本题用法**：UB 手工规划时按生命周期复用同一块缓冲：`t = x + residual` 可原地、`y²` 复用 t 的缓冲、`sum` 结果与 rsqrt 用标量/小块缓冲，避免每步新开 TQue/TBuf——对应 Ascend C 的 TBuf/TQue 手工管理。

### 2.7 自动调优：tiling 搜索
- **编译器做法**：TVM autotvm / MetaSchedule、Triton `@triton.autotune`、TileLang tuner、FlagTree LibTuner——按 UB 容量、核数与访存带宽枚举 tile 与核数配置。
- **本题用法**：本题直调模板下 tile 由代码内决定（无自动搜索），但可借鉴其**配置空间**：按 D 与 outer 枚举 `每核行数 × 每行 UB 占用 × 核数（GetBlockNum）`，本地以 roofline/模拟预估，真机上做小规模网格扫描。

### 2.8 跨核归约（可选项）
- **编译器做法**：Triton-Ascend / TVM 对大归约支持"分两阶段：每核部分和 → 跨核（AllReduce/GM 原子或二次 kernel）归约"。
- **本题用法**：D 极大且 outer 少时（如 `outer=1, D=32768`），单核长链归约可能带宽/延迟不佳，可考虑每核算部分和再二次归约；但直调模板单 kernel 下跨核同步手段有限，**默认按每行单核归约、多行并行**设计，跨核归约仅作真机验证后的备选。

---

## 3. 对本题的结论

### 3.1 可直接使用的
- **msopgen**：用于生成与判题 SoC 匹配的算子工程骨架，再把 `提交/V001/kernel.asc` 与 op_host 接入（与 `文档/source-build.md` 一致）。它是**唯一官方**"生成"入口，且只生成骨架。

### 3.2 不可作为提交路径（产物形态不匹配）
- Triton-Ascend、AKG、FlagTree（二进制 kernel）、TileLang-Ascend（依赖 Catlass 模板与 CANN 工具链的完整算子）、XLA-NPU（图级）、TorchInductor 昇腾链路（图级）、PyPTO（指令级）、IREE（无昇腾）、TVM 主线（无昇腾）、MLIR 主线（无昇腾方言）。

### 3.3 明确建议采纳的编译层思路（≥3 条，全部满足验收）
1. **分段/树形归约**（2.1）：大 D 拆段部分和再合并；
2. **双缓冲软件流水线**（2.3）：MTE2 与 vector 重叠，multi-tile 双缓冲；
3. **尾块掩码/填充**（2.4）：`DataCopyPad` + 有效 D 边界限定；
4. 加分项：FP32 累加提升（2.2）、单 kernel 全融合（2.5）、UB liveness 复用（2.6）。

### 3.4 重要提醒
- 所有"生成器"的产物都无法**直接**成为直调模板的 `kernel.asc`；如需从 TileLang-Ascend 借用代码，只能作为**算法/边界处理参考**，再按直调模板签名（`GM_ADDR` + tiling 参数）人工重写。
- 本报告所有结论基于公开资料，**未在任何真实 CANN/NPU 上验证**；tilelang-ascend / triton-ascend 均要求 CANN ≥ 8.3/9.1 与 torch_npu，与本题目标 CANN 9.0.0 的兼容性也未经真机确认。

---

## 4. 本轮新增来源清单

> 规则同 `sources.md`。全部条目"是否在真实 NPU 验证"：**否**（本机无 CANN/NPU，仅核对公开内容）。

| # | 名称 | URL / 仓库 | 版本/日期 | 访问日期 | 用途 | 证据等级 |
| --- | --- | --- | --- | --- | --- | --- |
| A1 | Ascend/msopgen（MindStudio Ops Generator，官方开源） | https://github.com/Ascend/msopgen | master，2025-12-31 开源，2026-02-10 适配 AscendC 新工程 | 2026-09-11 | 确认 msopgen 生成工程骨架而非算法 | A |
| A2 | msopgen 算子工程创建工具参数说明 | https://developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-creating-operator-project-msopgen-0000002293225826 | 2026-01-04 更新 | 2026-09-11 | 命令/JSON 字段（`msopgen gen -i -c -lan cpp -out`） | A |
| A3 | CANN Kit 快速入门（msopgen 生成 AddCustom 工程结构） | https://device.harmonyos.com/cn/docs/apiref/harmonyos-guides/cannkit-operator-development | 2026-05-18 更新 | 2026-09-11 | 生成工程目录（op_host/op_kernel）证据 | A |
| A4 | MindStudio 全流程工具链（msOpGen 生成器说明，官方博客） | https://www.hiascend.com/developer/blog/details/02178216137870866383 | 2026-06 | 2026-09-11 | msOpGen 定位与 AddCustom 基线模板 | C |
| A5 | tile-ai/tilelang-ascend（昇腾 DSL 编译器，AscendC/PTO 后端） | https://github.com/tile-ai/tilelang-ascend | ascendc_pto 分支，2025-09-29 开源 | 2026-09-11 | 能否生成 Ascend C 的核心证据（codegen_ascend.cc、tl_templates） | A |
| A6 | TileLang-Ascend: Ascend NPU Code Generation（deepwiki 源码索引） | https://deepwiki.com/tile-ai/tilelang-ascend/7.2-ascend-npu-code-generation | fe6517 commit | 2026-09-11 | 两个后端差异、自动同步/内存规划 pass 列表 | C |
| A7 | tilelang-ascend elementwise 测试（T.reduce_sum / reduce_ascend_lang） | https://raw.githubusercontent.com/tile-ai/tilelang-ascend/ascendc_pto/testing/python/language/test_tilelang_ascend_language_elementwise.py | ascendc_pto | 2026-09-11 | 归约 DSL 与 PassConfig 证据 | A |
| A8 | triton-lang/triton-ascend（昇腾官方组织的 Triton 适配） | https://github.com/triton-lang/triton-ascend | 3.2.2（2026-07-31），3.2.1/3.2.0 | 2026-09-11 | TTIR→Linalg→AscendNPU IR→.o 编译链；支持 A2/A3/950 | A |
| A9 | Triton-Ascend 架构设计与核心特性 | https://github.com/triton-lang/triton-ascend/blob/main/docs/en/architecture_design_and_core_features.md | main | 2026-09-11 | 编译链/目录结构/驱动 | A |
| A10 | Triton-Ascend 中文 README（版本依赖矩阵） | https://github.com/triton-lang/triton-ascend/blob/main/README_zh.md | main | 2026-09-11 | CANN 9.1.0 / TorchNPU 2.7.1.post8 依赖 | A |
| A11 | Triton-Ascend Quick Start | https://github.com/triton-lang/triton-ascend/blob/main/docs/en/quick_start.md | main | 2026-09-11 | 运行方式（torch_npu 环境） | A |
| A12 | IREE 官方支持矩阵（无 Ascend） | https://iree.dev/index.html | 当前 | 2026-09-11 | 确认 IREE 后端列表不含昇腾 | A |
| A13 | IREE 仓库（Ascend 相关 issue 检索结果为空/不相关） | https://github.com/iree-org/iree ；https://github.com/search?q=repo%3Airee-org%2Firee+ascend&type=issues | main | 2026-09-11 | 反证 IREE 无昇腾后端 | A/C |
| A14 | TVM 主线 src/target 目录（GitHub API） | https://api.github.com/repos/apache/tvm/contents/src/target | main | 2026-09-11 | 确认 TVM 主线无 ascend codegen | A |
| A15 | cann/xla-npu（华为 XLA 昇腾后端） | https://gitcode.com/cann/xla-npu （转述：https://blog.csdn.net/gitblog_07156/article/details/151463150 ） | 当前 | 2026-09-11 | AFIR/GE/Aclnn 三策略；OpenXLA 生态接入 | B（转述 C） |
| A16 | TorchNPU（Ascend/pytorch，PrivateUse1） | https://github.com/ascend/pytorch | master（TorchNPU 26.0.0，2026-04-30） | 2026-09-11 | torch.compile 昇腾链路入口 | A |
| A17 | PyTorch Shanghai Meetup（Ascend 多后端案例） | https://pytorch.org/blog/pytorch-shanghai-notes/ | 2024-09-08 | 2026-09-11 | 昇腾接入 PyTorch 机制背景 | A |
| A18 | npu_inductor_mlir（社区：Inductor→torch-mlir→BiSheng） | https://gitee.com/rmch/npu_inductor_mlir2 | master | 2026-09-11 | 昇腾 Inductor 链路产物形态（二进制） | C |
| A19 | 昇腾 for PyTorch 训练营笔记（③④⑤ 编译链路） | https://bbs.huaweicloud.com/blogs/482845 | 2026-07-26 | 2026-09-11 | torch.compile→Inductor（昇腾）→Triton/MLIR→CANN Runtime | C |
| A20 | mindspore-ai/akg（Auto Kernel Generator） | https://github.com/mindspore-ai/akg | master（AKG-MLIR；支持 Atlas 800T A2/A3） | 2026-09-11 | 昇腾自动生成 kernel 的官方先例 | A |
| A21 | MindSpore 图算融合加速引擎（AKG polyhedral 说明） | https://www.mindspore.cn/docs/zh-CN/r1.10/design/graph_fusion_engine.html | r1.10 | 2026-09-11 | AKG 调度/融合/切分机制 | A |
| A22 | IMPACT 2022 keynote：MindSpore/AKG 架构（PLDI 2021 论文演讲） | https://acohen.gitlabpages.inria.fr/impact/impact2022/slides/keynote.pdf | 2022 | 2026-09-11 | 图算融合 + AKG 三层融合（Poly/内存/并行） | C |
| A23 | hicann/pypto（**华为昇腾** PTO 框架；任务背景误记为 AMD） | https://github.com/hicann/pypto | 0.2.0（2026-04-10） | 2026-09-11 | 事实修正：PyPTO 属昇腾 PTO-ISA 生态 | A |
| A24 | PyPTO 上游视图（hw-native-sys/pypto fork） | https://github.com/tsung-li/pypto | main（PTOCodegen for PTO-ISA MLIR） | 2026-09-11 | PTO-ISA MLIR 生成证据 | C |
| A25 | pypto 上手：用 Python 直接调 PTO 虚拟指令集 | https://blog.csdn.net/2502_93572233/article/details/161360732 | 2026-05-25 | 2026-09-11 | PTO 在 CANN 五层架构中的位置 | C |
| A26 | HIVM: MLIR Dialect Stack for Huawei Atlas NPU（LLVM 开发者会议 2026-04 教程） | https://www.llvm.org/devmtg/2026-04/slides/tutorial/tutorial_tarasov.pdf | 2026-04-15 | 2026-09-11 | AscendNPU-IR 方言栈、Triton-Ascend 关联 | A |
| A27 | FlagTree（Triton 兼容统一多后端编译器，含 ascend 后端） | https://gitee.com/flagos-ai/flagtree | main（2025-06-04 ascend 接入） | 2026-09-11 | 昇腾 Triton 生态另一入口 | B/C |
| A28 | 昇腾 Triton 算子开发初体验（编译链路实操） | https://www.hiascend.com/developer/blog/details/0269202487813540173 | 2026-01-06 | 2026-09-11 | "Triton IR→AscendNPU IR" 链路社区佐证 | C |
| A29 | SITS2026：昇腾 NPU 后端算子覆盖 92%（ACL+AclGraph） | https://blog.csdn.net/PoliVein/article/details/160025359 | 2026-04-10 | 2026-09-11 | 背景：昇腾后端通过 ACL/AclGraph 而非开放编译器 | C |
