# Agent 05 调研报告：编译器、IR 与算子生成（AddRmsNormBias）

> 子代理：Agent 5（编译器、IR 和算子生成）｜访问日期：2026-09-12（北京时间）
> 方法：网络检索（TVM Discourse / Triton 官方教程 / MLIR 官方文档 / GitHub·GitCode 仓库 / 昇腾官方文档与技术文章）+ 网页抓取。本机 macOS 无 CANN/NPU，全部结论均为资料调研，未做任何编译或运行验证。
> 任务边界：不写算子实现代码；不做 GPU kernel 实现细节（归 Agent 4）；不做 Ascend 官方仓库与 API 签名核对（归 Agent 2/3）；不调用 CANNJudge、不上传、不提交。
> 证据分级：A=官方文档/官方仓库；B=官方样例/源码/测试；C=社区文章/论坛/个人仓库；D=仅搜索摘要未核验。

---

## 1. 调研范围与平台覆盖情况

| 平台/入口 | 覆盖结果 | 状态 |
| --- | --- | --- |
| Apache TVM 官方 Discourse（discuss.tvm.apache.org） | 检索到归约初始化 lowering、矩阵/向量单元共享片上 buffer 调度两个直接相关帖子 | verified |
| TVM 官方教程（TE 入门、VTA）+ tvm-rfcs MetaSchedule RFC | 已读摘要与关键代码 | verified/partial |
| Triton 官方教程（triton-lang.org，softmax/flash attention） | softmax 行归约教程完整读取（正对"最后一维归约+epilogue"模式） | verified |
| TorchInductor（pytorch.org 博客、GitHub PR、社区深度剖析文章） | 融合规则表、reduction codegen（persistent/looped）、epilogue fusion 资料 | partial（核心规则经多来源互证） |
| MLIR 官方文档（linalg dialect、Transform 教程 Ch0）+ LLVM Dev Meeting 幻灯 | 归约/收缩的结构化 lowering、渐进式向量化、padding/peeling | verified |
| OpenXLA / IREE | 通过 Hexagon-MLIR 论文间接覆盖（NPU 编译栈的 Triton→linalg 行 softmax lowering 例子） | partial |
| TileLang 主仓库 + tilelang-ascend（ascendc_pto / npuir 两条路线） | README 完整读取（内存原语、pass 配置、示例清单） | verified |
| 昇腾官方文档（hiascend.com：msopgen/msopst 实践、bisheng 快速上手、毕昇编译器指南 PDF） | 关键命令与工程骨架摘录 | verified |
| 昇腾官方技术文章（TileLang AscendNPU IR 直播总结） | 读取全文 | partial（宣传性质） |
| CANN 版本公告页（hiascend.com/productbulletins） | CANN 9.0.0 发布时间与特性确认 | verified |
| GitHub/GitCode 仓库（Ascend/msopgen、cann/asc-devkit、tile-ai/tilelang*、apache/tvm） | 仓库元信息与 README | verified |

**覆盖结论**：已覆盖任务要求的 8 个主题中除"XLA/IREE 专向"外的全部主项；XLA/IREE 仅通过 Hexagon-MLIR 论文间接覆盖（该论文展示了 Triton 行 softmax 到 linalg 的完整 lowering，对本研究主题更有针对性）。

---

## 2. 各框架/工具条目卡片

每张卡片包含：对"最后一维归约 + 逐元素 epilogue"（即本题 `y=x+residual → mean(y²,dim=-1) → y/rms*gamma+bias` 的形状）的处理方式摘录，以及**对 CANN 9.0.0 直调提交（手写单文件 kernel.asc）的适用性判定**。

### 2.1 Apache TVM（TE/TIR + Ansor/MetaSchedule + BYOC）

- **归约表示与 lowering**：TE 用 `te.reduce_axis` + `te.sum` 声明归约；TIR 中归约块带 `T.axis.reduce` 与 `T.init()`（初始化写入）。社区帖（discuss.tvm.apache.org/t/18613，C 级）展示了生成 CUDA 代码中归约初始化被内联为 `if (迭代计数==0)` 条置零，`decompose_reduction` 可把初始化拆成独立 kernel 但引入双 kernel 启动开销——**对 Ascend C 的直接启示**：RMSNorm 的方差累加器应在 UB 内先写零或用 `ReduceSum` 的 init 语义，避免循环内条件初始化。
- **片上 buffer 调度**：TIR 通过 `T.alloc_buffer(..., scope="shared"/"local")` + `cache_read/cache_write` 做显式 memory planning。另一社区帖（discuss.tvm.apache.org/t/18797，C 级）描述自定义加速器（矩阵单元+向量单元+共享片上 buffer）上 TVM 默认插入"local→global→local"往返——**与昇腾 Cube/Vector 核经 GM 交换数据的结构同源**，说明通用编译器对"计算结果留在片上供 epilogue 消费"（即本题 y 求完均方后留在 UB 供归一化 epilogue 复用）需要手工调度干预。
- **autotune**：Ansor = 草图生成（sketch，无参数骨架）+ 注释（随机采样填充 tile/展开/向量化参数）+ 演化搜索 + 代价模型；MetaSchedule（RFC 0005，B 级）统一了 AutoTVM/AutoScheduler API，提供 Post-DFS 顺序的复合调度规则。
- **Ascend 支持现状**：TVM 主线**没有** Ascend 后端；2026-04 合入的 example NPU BYOC backend（apache/tvm#19425）是 CPU 模拟的教学桩，不做真实计算。VTA（tvm-vta 仓库）是 FPGA 加速器后端，仅作概念对照。
- **适用性判定**：**纯研究参考**。TVM 生态与昇腾无官方对接；其"归约 init 拆分""cache_read/write scope 规划"思想可迁移到 Ascend C 的 TQue/TBuf 与双 buffer 规划，但无任何可直接提交的产物。

### 2.2 Triton（triton-lang）

- **"最后一维归约+epilogue"的标准模式**（官方 Fused Softmax 教程，B 级）：每 program 处理若干行；`BLOCK_SIZE` 必须为 2 的幂且 `≥ n_cols`（host 端取 `next_power_of_2(n_cols)`）；尾块用 `mask = col_offsets < n_cols`，load 时 `other=-float('inf')`（softmax 需要），store 时同样带 mask——**即"整行驻留 + 掩码"策略，一次性完成 max/sum 归约与逐元素 epilogue**。
- **精度提升**：社区与厂商资料一致确认：Triton **不会自动**把 fp16/bf16 归约提升到 fp32（NVIDIA TensorRT-LLM 官方 SKILL.md 明确"Always use tl.float32 accumulators"，C 级但与官方教程一致）；超越函数（exp 等）必须 fp32 进、原 dtype 出。官方教程对 bf16 softmax 自动转 fp32 计算以保证精度。
- **autotune**：`@triton.autotune` 是"手工模板 + 穷举"（CMU 15-779 讲义对比表：Triton=Manual template + Exhaustive），key 参数（如 M/N/K）变化时重测全部配置。
- **适用性判定**：**纯研究参考（方法论层面最有价值）**。Triton 不支持昇腾目标（社区无官方 Ascend 后端）；其行驻留+掩码尾块+fp32 累加器三件套，正是 AddRmsNormBias 手写 Ascend C 时"行分块驻留 UB、FP32 累加、DataCopyPad 尾块"的直接对照。

### 2.3 TorchInductor（PyTorch 2 编译器）

- **融合规则**（社区深度剖析，多来源互证，C 级但与 PyTorch 源码 PR 一致）：
  - Pointwise → Pointwise：可融合（residual add 属于此类）；
  - Pointwise → Reduction：**可融合（prologue）**——原文举例 `(x.float() ** 2).mean()` 即 **RMSNorm 的 prologue**，与本题 `mean(y²)` 完全同构；
  - Reduction → Pointwise：**可融合（epilogue）**——layer_norm 后接逐元素激活即单 kernel，对应本题 `y/rms*gamma+bias`；
  - mm → Pointwise：默认不融合（需 max-autotune 模板）——本题无 GEMM，不受此限制。
  - **结论**：AddRmsNormBias 整条链（add→square-mean→mul-add）在 Inductor 分类法里是"单次可融合组"，这佐证了题面把它设计成单一融合算子的合理性。
- **reduction codegen**：persistent（每 program 常驻累计多行）与 looped（每 program 扫描分块累加）两种模式；GEMM epilogue fusion 走 vendored 模板把 pointwise 函数物化为 epilogue 回调（pytorch PR #190808，B 级）。
- **适用性判定**：**纯研究参考**。Inductor 不支持昇腾（Triton 后端仅 CUDA 等）；其"reduction+epilogue 是一个融合单元"的判定和 persistent/looped 两种行归约实现策略，可作为 Ascend C 多行归一化 kernel 的双 buffer/循环组织参照。

### 2.4 MLIR（linalg / vector dialect；含 Hexagon-MLIR 对照）

- **归约的结构化表示**：linalg 提供 `linalg.reduce`、`linalg.softmax` 等 named op；归约语义由 `indexing_maps`（仿射映射，指明哪个维度是 reduction iterator）+ payload region 表达（官方 Linalg Dialect 文档，A 级）。**axis analysis** 即体现在 indexing map：最后一维归约就是 `(d0, d1) -> (d0, d1), (d0, d0)` 型映射。
- **渐进式 lowering**（Transform 教程 Ch0 + LLVM Dev Meeting 2023 幻灯，A 级）：linalg → vector 级四步：① 按向量大小 tile；② 循环体向量化（`vector.transfer_read` + `vector.reduction <add>`）；③ 高层到低层 vector lowering；④ 向量维度合法化（展开到硬件支持的向量宽度）。
- **尾块**：两种策略——padding（`vector.create_mask` + masked `transfer_read`，越界补零）或 peeling（remainder loop）；MLIR 官方文档明确"Apply padding or peeling (remainder loop), if necessary"。
- **Hexagon-MLIR 论文**（arXiv 2602.19762，2026-02，B 级）：高通 NPU 编译栈把 Triton 行 softmax kernel lower 到 linalg-generic 再到 Hexagon HVX，含算子融合（元素链融合进归约）、tiling 到内存层级、多线程、double buffering——证明"归约+epilogue 经 MLIR 面向 NPU"是业界成熟路线。
- **昇腾对应物**：华为官方技术文章（hiascend.com:6066/developer/techArticles/20260506-1，A/B 级）明确 **AscendNPU IR 是基于 MLIR 构建的昇腾专用 IR 与编译器框架**，与 TileLang 前端配合。
- **适用性判定**：**研究参考（概念架构层面）**。MLIR 本身不产出 Ascend C；但其 indexing-map axis analysis、padding/peeling 尾块二选一、渐进式向量化的分解思路，对设计 D 非 32 倍数的尾块搬运与 128-lane 向量化循环结构有直接参考价值。

### 2.5 TileLang-Ascend（昇腾 DSL，两条后端路线）

- **仓库与路线**（tile-ai/tilelang-ascend，B 级）：2025-09-29 开源；两条技术路线——`ascendc_pto` 分支（**Ascend C & PTO**：生成/对接 Ascend C 代码路径）与 `npuir` 分支（**AscendNPU IR**：MLIR 路线）。tilelang 主仓 README 的后端表将其列为 "Ecosystem" 级支持（Ascend A2/A3）。
- **对"最后一维归约+epilogue"的支持**：examples 目录直接包含 **softmax、normalization、reduce、elementwise、activation** 示例，覆盖本题形状。
- **内存原语**：显式 `T.alloc_L1 / T.alloc_ub / T.alloc_L0A/B/C`，与 GPU 的 global/shared/register 三级对应（README 明确映射：shared ↔ L1+UB、register ↔ L0A/B/C）——**这是所有调研对象中与 Ascend UB 规划最直接可比的 memory planning 模型**。
- **自动化 pass**：`TL_ASCEND_AUTO_SYNC`（核内同步自动插入）、`TL_ASCEND_MEMORY_PLANNING`（**自动 buffer 复用**，节省 UB 以支持更大 tiling）、`T.Parallel`（自动向量化）、`T.Pipelined`（软件流水）。
- **环境约束**：要求 CANN ≥ **8.3.RC1**、torch-npu ≥ 2.6.0.RC1；实测设备 A2/A3（与本题 dav-2201/Atlas A2 同系）。
- **适用性判定**：**研究参考（最接近昇腾的 DSL，但不可用于提交）**。判题只收手写 kernel.asc（在线编辑器上传 .asc/.h），平台无 Python/torch-npu 运行时，DSL 产物无法进入提交通道；其"UB 复用 pass + 同步自动插入 + 行归约示例"可作为手写 kernel 的结构设计参照。另注意华为官方直播文章属宣传材料，性能声称未经独立复核。

### 2.6 msopgen / msopst（CANN 官方算子工程生成与 ST 工具）

- **msopgen**（官方文档 A 级 + Ascend/msopgen 仓库 B 级，2025-12-31 全面开源、2026-02-10 适配新工程）：输入算子原型 JSON（op/input_desc/output_desc/attr），`msopgen gen -i xxx.json -f tf -c ai_core-Ascendxxxyy -lan cpp -out xxx` 生成标准算子工程：`build.sh`、`CMakeLists.txt`、`CMakePresets.json`、`op_host/`（tiling 定义 + 原型注册）、`op_kernel/`（kernel 实现）、`scripts/`。**生成的是"框架接入型"多文件算子工程，不是直调单文件 kernel.asc**。
- **msopst**：`msopst create -i op_host/xxx.cpp -out ./st` 生成 ST 测试用例并 `msopst run -i xxx.json -soc Ascendxxxyy` 执行单算子测试；需要 `DDK_PATH`/`NPU_HOST_LIB` 环境变量与真实 CANN+NPU。
- **MindSpore 封装**：MindSpore 2.3+ 提供 custom_compiler 离线编译（setup.py --op_host_path --op_kernel_path --ascend_cann_package_path），本质是对 msopgen 工具链的封装（A 级官方教程）。
- **适用性判定**：**研究参考（工程骨架）**。msopgen 工程的 tiling 结构（tiling.h 与 kernel 分离、host 侧计算切分参数）与 build 流程可作为"工程化最佳实践"对照；但判题提交物是单文件 kernel.asc，op_host/op_kernel 分离结构**不能直接用于提交**。msopst 可在获得真机后用于本地 ST 验证。

### 2.7 bisheng / ccec（昇腾异构编译器）

- **定位与安装**（官方毕昇编译器用户指南 8.5.0 PDF，A 级）：毕昇（bisheng）随 CANN 包发布，位于 `${INSTALL_DIR}/compiler/ccec_compiler/bin`（底层 ccec 工具链）；支持 Host & Device 混合编译单文件 `.cce/.asc` 直接产出可执行文件。
- **编译选项**：
  - 新式：`bisheng -O2 --npu-arch=dav-2201 xxx.asc -o demo`（dav-2201 即 910B/Atlas A2 系的 AI Core 架构版本）。
  - 旧式（8.0.RC3 文档）：`--cce-soc-version=AscendXXXYY --cce-soc-core-type=VecCore`（Vector 核）。
  - **版本约束（关键案例，A 级官方博文）**：CANN **8.2.RC1 不支持 `--npu-arch`**，报 `unsupported option '--npu-arch=dav-2201'`；**8.3.RC1 起才支持**。本题目标 CANN 9.0.0 > 8.3.RC1，按版本序列推断支持该选项（推断项，见第 6 节风险）。
  - CMake 集成（CANN 8.5+）：`find_package(ASC REQUIRED)` + `project(xxx LANGUAGES ASC CXX)` + `target_compile_options(... $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)`，由 ASCConfig.cmake 提供 ASC 语言完整 CMake 支持，`ld.lld` 链接。
- **与判题的关系**：CANNJudge 在平台侧用 CANN 9.0.0 编译上传的 kernel.asc，**本地 bisheng 仅用于复现与调试**，编译选项不影响提交物本身。
- **适用性判定**：**本地复现工具，非提交内容**。在拿到真机环境后，`bisheng --npu-arch=dav-2201 kernel.asc`（加 AscendCL 头文件/库路径）是本地复现判题编译行为的最短路径。

### 2.8 TVM BYOC example NPU / VTA / Hexagon-MLIR（横向对照项）

- example NPU BYOC（apache/tvm#19425）：教学桩，CPU 模拟，验证 BYOC 四步（注册 pattern → partition → codegen → runtime 派发）；说明"框架外挂 NPU 后端"的通用形态。
- VTA：TVM 最早的片上 buffer 显式规划实践（inp/wgt/acc 三段 SRAM scope），是"归约累加器驻留片上"思想的源头之一。
- Hexagon-MLIR：见 2.4，证明 Triton→linalg→NPU 的"行 softmax"整链在工业界落地。
- **适用性判定**：均为**概念参考**，与提交无关。

---

## 3. memory planning 与尾块生成对比

### 3.1 memory planning（片上内存规划）对比

| 框架/工具 | 片上内存模型 | 归约中间量的驻留方式 | 与 Ascend UB（TPipe/TQue/TBuf）可比性 |
| --- | --- | --- | --- |
| Triton | 编译器自动管 shared memory 分配与同步，寄存器经 LLVM | 整行驻留寄存器（BLOCK_SIZE ≥ 行长）或分块累加 | 概念可比：行驻留 ↔ y 整行入 UB 后原地做 add/平方/归约 |
| TVM TIR | `T.alloc_buffer(scope="shared"/"local")` + cache_read/write | local 累加器 + shared 暂存；默认可能插入 local→global→local 往返（社区帖实证） | 高：scope 划分思想同 TBuf/TQue；"避免中间量回落 GM"即 UB 内完成 epilogue 的核心理由 |
| TorchInductor | 融合组内中间量留在寄存器（不落显存） | reduction 结果广播回 epilogue 同一 kernel 内消费 | 高：AddRmsNormBias 全链单融合组 = 中间 y、rms 均不出片上 |
| MLIR/linalg | memref 层显式 buffer 管理，tile 到内存层级 | padding/peeling 决定尾块；double buffering（Hexagon-MLIR） | 中：tile+双缓冲对应 UB 双 buffer（TQue BUFFER_NUM=2） |
| TileLang-Ascend | `alloc_L1/alloc_ub/alloc_L0C` 显式原语 + TL_ASCEND_MEMORY_PLANNING 自动复用 | UB 内归约 + epilogue，可自动 buffer 复用扩 tiling | **最高**：其 pass 文档直指"节省 UB 空间以开出更大 tiling" |
| （对照）Ascend C 手写 | TQue/TBuf/TPipe 显式声明 | DataCopy GM↔UB，ReduceSum 在 UB | ——（本题目标形态） |

**要点**：通用编译器的经验一致指向——本题性能关键在于 (a) y=x+residual 与平方累加共用同一 UB buffer（避免二次搬运）；(b) 归一化 epilogue 原地消费 y（避免 y 回落 GM 再读回）；(c) 双 buffer 搬运与计算重叠。这三点分别对应 TVM 帖子的"local→global→local 反例"、Inductor 的"融合组内不出显存"、TileLang 的"memory planning pass"。

### 3.2 尾块生成（tail masking / padding codegen）对比

| 框架 | 尾块策略 | 摘录要点 |
| --- | --- | --- |
| Triton | **mask 谓词**：`mask = offs < n`，load 带 `other=` 填充值（softmax 用 `-inf`，求和场景用 0），store 同 mask；BLOCK_SIZE 必须 2 的幂 | "每个块必须具有 2 的幂元素数……需在内部填充每一行，并适当保护内存操作"（官方教程原文） |
| MLIR vector | **padding 或 peeling 二选一**：`vector.create_mask` + masked `transfer_read`（补零），或 remainder loop | "Apply padding or peeling (remainder loop), if necessary"（Transform 教程） |
| TVM TE | `tvm_if_then_else` 边界条件零填充（d2l 教程卷积 padding 例子）；tail 循环由 split 自然生成 | 边界外索引填 0 参与计算 |
| TorchInductor | codegen 层生成 masked load/store（Triton 后端复用上述机制） | 同 Triton |
| TileLang-Ascend | 语言层 `T.copy` 边界 + 示例中显式处理 | 未在本次抓取中见到独立 mask 原语（partial） |
| （对照）Ascend C | **DataCopyPad**：GM→UB 搬运时按 32B 对齐自动补齐，需保证不覆盖相邻行 | 判题约束：D 非 32 倍数必须验证尾块与输出边界 |

**要点**：所有框架殊途同归为"mask 谓词"或"显式补零"两条路；Ascend C 的 `DataCopyPad` 属后者（搬运层补齐），而逐元素计算段的尾块仍需注意越界与相邻行覆盖问题——这正是题面强调的验证点。

### 3.3 FP16/BF16 lowering（累加精度提升）对比

- **Triton**：不自动提升——归约/点积累加器必须显式 `tl.float32`；超越函数强制 fp32 进出；官方 softmax 教程对 bf16 输入自动 cast fp32 计算。
- **TorchInductor**：lowers 时为低精度输入选择更高精度 acc dtype，epilogue 再降回目标 dtype（社区剖析 + PyTorch 博客对 TensorCore 累加器精度的研究，C 级）。
- **MLIR/linalg**：payload region 内显式 arith.extf/truncf 控制精度边界。
- **TVM**：TE 层在 lambda 内显式 `.astype("float32")`（d2l 教程模式）。
- **（对照）本题约束**：题面/项目约定"归约及中间累加优先 FP32；FP16/BF16 仅在搬运与最终输出保留目标类型"——与上述所有主流框架的默认选择一致，属于无争议的行业标准做法。

---

## 4. CANN 算子生成工具链现状（msopgen / msopst / bisheng / ccec）

**总体判断**：昇腾的"算子生成"工具链是**工程脚手架 + 编译器**组合，而非端到端代码生成器——msopgen 生成的是空模板工程（官方明示"generates only an empty operator project template. You need to add operators"），核心计算代码仍需手写 Ascend C；真正接近"算子自动生成"的只有社区 TileLang-Ascend（生成 Ascend C 路径）与华为 AscendNPU IR（MLIR 路线）。

| 工具 | 角色 | 版本/环境约束 | 与本题（CANN 9.0.0 / dav-2201 / 直调 kernel.asc）关系 |
| --- | --- | --- | --- |
| msopgen | 算子工程骨架生成（op_host/op_kernel/build.sh/CMakePresets） | 随 CANN 包发布（`${INSTALL_DIR}/python/site-packages/bin/msopgen`）；2025-12 开源，2026-02 适配新工程 | 产物为多文件工程，**不能**作为提交物；骨架中 tiling 与 kernel 分离的模式可作参考 |
| msopst | ST 测试用例生成与执行 | 需 DDK_PATH/NPU_HOST_LIB + 真机 | 真机到位后可用于本地单算子验证（对照判题 15 测试点） |
| bisheng（ccec） | 异构编译器，编译 .asc/.cce 单文件 | 随 CANN 包；`--npu-arch=dav-2201` 需 **CANN ≥ 8.3.RC1**（8.2.RC1 实证不支持）；CMake ASC 语言集成为 CANN ≥ 8.5 | **本地复现用**；判题平台代编译，提交物不含编译选项 |
| CANN 9.0.0 | 目标运行环境 | 2026-04-30 发布（官方公告）；asc-devkit（gitcode.com/cann/asc-devkit）有 v9.0.0 源码 tag | 判题指定版本；其工具链（msopgen/msopst/bisheng）均随包提供 |
| TileLang-Ascend | DSL 生成 Ascend C / AscendNPU IR | CANN ≥ 8.3.RC1、torch-npu ≥ 2.6.0.RC1；A2/A3 验证 | 研究参考；平台无 Python 运行时，不可用于提交 |
| AscendNPU IR | MLIR 基的昇腾 IR/编译框架 | 华为官方（TileLang 直播文章，2026-05） | 研究参考（架构理解）；非提交通道 |

**工具链版本链推论**：CANN 9.0.0（2026-04）> 8.5（CMake ASC）> 8.3.RC1（--npu-arch）> 8.2.RC1，故 CANN 9.0.0 环境下 `--npu-arch=dav-2201` 与 CMake ASC 集成在版本序列上均应可用——此为版本序列推断，未在本机验证（本机无 CANN）。

---

## 5. "研究参考 vs 不可直接提交"判定表

判题事实（前序 Agent 已确认，此处仅引用）：CANNJudge 只接受直调单文件 kernel.asc（在线编辑器 files 数组上传 .asc/.h），无 DSL/工具链提交通道。

| 方案 | 证据等级 | 对"最后一维归约+epilogue"的参考价值 | 提交路线判定 | 理由 |
| --- | --- | --- | --- | --- |
| TVM（TE/TIR/Ansor/BYOC） | A/B（文档/仓库）+C（论坛帖） | 中：归约 init 拆分、scope 规划、autotune 搜索空间设计 | **不可提交，仅研究参考** | TVM 主线无 Ascend 后端；产物非 Ascend C 单文件 |
| Triton | B（官方教程）+C（SKILL/讲义） | **高**：行驻留+mask 尾块+fp32 累加器三件套正对题形 | **不可提交，仅研究参考** | 不支持昇腾目标；方法论迁移价值最高 |
| TorchInductor | B（PyTorch PR）+C（剖析文章） | **高**：证明 add→square/mean→mul/add 是单一可融合组；persistent/looped 两种行归约组织 | **不可提交，仅研究参考** | 不支持昇腾；融合判定佐证单 kernel 设计合理性 |
| MLIR linalg/vector | A（官方文档/教程） | 中高：axis 用 indexing map 显式表达；padding/peeling 尾块范式 | **不可提交，仅研究参考** | 非昇腾产物路径；是 AscendNPU IR 的技术底座（理解价值） |
| TileLang-Ascend（Ascend C & PTO） | B（开源仓库+官方文章） | **最高**：同硬件（A2）、有 softmax/normalization/reduce 示例、UB 显式规划与自动复用 pass | **不可提交，仅研究参考** | 判题只收手写 kernel.asc；平台无 Python/torch-npu；但其分块与 UB 复用思路可直接借鉴到手写代码 |
| TileLang-Ascend（AscendNPU IR/MLIR） | B+C | 中：MLIR 路线现状理解 | **不可提交，仅研究参考** | 同上；且成熟度声明来自官方宣传材料 |
| msopgen 生成工程 | A（官方文档） | 中：工程骨架、tiling/host-kernel 分离范式 | **不可提交**（多文件工程 ≠ 单文件 kernel.asc） | 骨架可参考，代码须手写 |
| msopst | A | 低（测试工具，非生成） | **不可提交**；真机后可用于本地 ST 验证 | 测试工具 |
| bisheng/ccec 编译选项 | A | 无（编译工具） | **与提交无关**；本地复现判题编译用 | 平台代编译 |
| TVM BYOC example NPU / VTA / Hexagon-MLIR | B（论文/仓库） | 低-中（概念对照） | **不可提交，仅概念参考** | 教学或异构对照物 |

**核心判定**：所有 DSL/自动生成方案在本题中一律只能作为研究参考；唯一合法提交物是手写 Ascend C 单文件 kernel.asc。其中 TileLang-Ascend 因硬件同源（A2/dav-2201）且示例覆盖 softmax/normalization/reduce，是"手写前看一眼别人怎么分块"的最佳参照；Triton/Inductor 提供"行归约+epilogue"的通用方法论。

---

## 6. 风险与未验证事项

1. **全部结论未做真机验证**：本机 macOS 无 CANN/Ascend 编译器/NPU，工具链行为（msopgen 工程生成、bisheng 编译、TileLang-Ascend 运行）均来自公开资料，无任何本地执行记录。
2. **CANN 9.0.0 对 `--npu-arch=dav-2201` 的支持为版本序列推断**：实证仅到"8.3.RC1 支持、8.2.RC1 不支持"（官方博文案例）；9.0.0 未单独检索到该选项的直接文档条目。真机到位后需以 `bisheng --help` 或编译试验确认。
3. **TileLang-Ascend 产物形态未实测**：其 Ascend C & PTO 路线"生成/对接 Ascend C 代码"的具体输出（能否导出等价 .asc 文本）未验证；官方技术文章为宣传材料，性能与成熟度声称未经独立复核。
4. **TorchInductor 融合规则表来自社区剖析**（C 级，多篇互证），未逐条比对 PyTorch 源码 `torch/_inductor/scheduler.py`；方向性结论（reduction 与 pointwise prologue/epilogue 可融合）与官方教程例子一致，可信度较高。
5. **搜索结果经缓存代理**：部分抓取内容图片经 aka.doubaocdn.com CDN，正文文本与官方域名（hiascend.com、gitcode.com/Ascend、github.com、triton-lang.org、mlir.llvm.org）核对一致，但不排除快照时点差异；关键命令（msopgen/bisheng）均以官方域名文档为准。
6. **OpenXLA/IREE 未做专向检索**：仅通过 Hexagon-MLIR 论文间接覆盖"归约+epilogue 面向 NPU 的 MLIR lowering"；如需 XLA 的 fusion pass 细节（如 fusion-instrumenter 的产生-消费窗口），需后续补充。
7. **AscendNPU IR 的开放接口细节未深入**：华为文章称"向开源社区开放接口"，但其方言定义、pass 管线文档未在本次范围内逐页核对。

---

## 7. 来源登记表

| # | URL | 标题 | 仓库/版本 | 访问日期 | 用途 | 证据等级 | 证据状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | https://discuss.tvm.apache.org/t/optimizing-reduction-initialization-in-generated-cuda-code/18613 | Optimizing Reduction Initialization in Generated CUDA Code | TVM Discourse 帖 18613 | 2026-09-12 | 归约 init 的 lowering 形态与 decompose_reduction 取舍 | C | verified |
| 2 | https://discuss.tvm.apache.org/t/how-to-keep-data-in-local-buffer-between-matrix-and-vector-ops-avoid-extra-global-memory-copies/18797 | How to keep data in local buffer between matrix and vector ops | TVM Discourse 帖 18797 | 2026-09-12 | 片上 buffer 避免回落 GM 的调度问题（与 UB 复用同源） | C | verified |
| 3 | https://tvm.d2l.ai/chapter_gpu_schedules/conv.html | d2l-tvm: Convolution（GPU 调度） | d2l-tvm 教程 | 2026-09-12 | shared/local tiling 与 tvm_if_then_else 边界零填充 | C | verified |
| 4 | https://github.com/apache/tvm-rfcs/blob/main/rfcs/0005-meta-schedule-autotensorir.md | RFC 0005: Meta Schedule | apache/tvm-rfcs | 2026-09-12 | MetaSchedule 搜索空间设计与统一 API | B | verified |
| 5 | https://arxiv.org/html/2406.20037v2/ | Explore as a Storm, Exploit as a Raindrop（Ansor 调优改进） | arXiv 2406.20037v2 | 2026-09-12 | Ansor 草图生成+注释+演化搜索机制 | B | partial（摘要与正文首节） |
| 6 | https://www.cs.cmu.edu/~zhihaoj2/15-779/slides/09-ML-compilers-part-2.pdf | CMU 15-779 Lecture 9: Kernel Autotuning | CMU 课程幻灯 2025-09 | 2026-09-12 | Triton/AutoTVM/Ansor 搜索空间对比表 | C | verified |
| 7 | https://github.com/apache/tvm/pull/19425 | [Backend][Relax] Add NPU BYOC backend example | apache/tvm PR#19425（2026-04 合入） | 2026-09-12 | TVM 主线无真实 Ascend 后端的佐证 + BYOC 四步流程 | B | verified |
| 8 | https://github.com/apache/tvm-vta | VTA Hardware Design Stack | apache/tvm-vta | 2026-09-12 | 片上 buffer 显式 scope 规划的早期实践对照 | B | verified |
| 9 | https://triton-lang.org/main/getting-started/tutorials/06-fused-attention.html | Triton 官方教程：Fused Attention | triton-lang.org | 2026-09-12 | Triton 归约+融合 kernel 组织（旁证） | B | verified |
| 10 | https://blog.csdn.net/ouliten/article/details/160897982 | Triton 笔记 3：融合 Softmax | CSDN（2026-05） | 2026-09-12 | 行归约+epilogue 模式中文详解（BLOCK_SIZE 2 的幂、mask、精度提升） | C | verified |
| 11 | https://github.com/NVIDIA/TensorRT-LLM/blob/main/.claude/skills/kernel-triton-writing/SKILL.md | Triton Kernel Writing（SKILL） | NVIDIA/TensorRT-LLM | 2026-09-12 | Triton 不自动提升 fp16/bf16 累加精度、超越函数 fp32 规则 | B | verified |
| 12 | https://jvoltci.github.io/mosaic/ml-execution/roofline-profiling/torch-compile-fusion/ | Inductor Fusion Heuristics | jvoltci.github.io | 2026-09-12 | Inductor 融合规则表（Pointwise↔Reduction prologue/epilogue；RMSNorm 例子） | C | verified |
| 13 | https://calwoo.github.io/notes/concepts/pytorch-internals/torch-compile/inductor/ | TorchInductor: Deep Dive | calwoo.github.io | 2026-09-12 | ir.Pointwise/ir.Reduction、persistent vs looped reduction codegen | C | verified |
| 14 | https://github.com/pytorch/pytorch/pull/190808 | [inductor][NVGEMM] Fuse pointwise epilogues into scaled GEMM | pytorch PR#190808（2026-07） | 2026-09-12 | epilogue fusion 的模板物化机制 | B | verified |
| 15 | https://mlir.llvm.org/docs/Dialects/Linalg/ | MLIR 'linalg' Dialect（含 linalg.reduce/softmax） | mlir.llvm.org | 2026-09-12 | 归约 named op 与 indexing map 表达 | A | verified |
| 16 | https://mlir.llvm.org/docs/Tutorials/transform/Ch0/ | Transform 教程 Ch0：结构化 Linalg 操作 | mlir.llvm.org | 2026-09-12 | vector.reduction/contract、padding-peeling 范式 | A | verified |
| 17 | https://www.llvm.org/devmtg/2023-10/slides/techtalks/Warzynski-Caballero-VectorizationinMLIR.pdf | Vectorization in MLIR（LLVM Dev 2023） | llvm.org | 2026-09-12 | 渐进式向量化四步与 masked transfer_read | B | verified |
| 18 | https://arxiv.org/html/2602.19762 | Hexagon-MLIR: An AI Compilation Stack for Qualcomm NPUs | arXiv 2602.19762（2026-02） | 2026-09-12 | Triton 行 softmax→linalg→NPU 全链 lowering 实例 | B | verified |
| 19 | https://github.com/tile-ai/tilelang | Tile Language（主仓后端支持表） | tile-ai/tilelang | 2026-09-12 | Ascend 后端列为 Ecosystem 级（A2/A3） | B | verified |
| 20 | https://github.com/tile-ai/tilelang-ascend | TileLang-Ascend（ascendc_pto/npuir 双路线） | tile-ai/tilelang-ascend（2025-09 开源） | 2026-09-12 | 显式 UB/L1/L0C 原语、TL_ASCEND_MEMORY_PLANNING/AUTO_SYNC、softmax/normalization/reduce 示例、CANN≥8.3.RC1 约束 | B | verified |
| 21 | https://www.hiascend.com:6066/developer/techArticles/20260506-1 | TileLang AscendNPU IR 从入门到专家算子开发知识点 | 昇腾官方技术文章（2026-05-08） | 2026-09-12 | AscendNPU IR=MLIR 底座；TileLang 编译依赖（bisheng/g++/TVM） | A | verified（宣传材料，性能声称未复核） |
| 22 | https://www.hiascend.com/document/detail/zh/canncommercial/80RC2/devaids/auxiliarydevtool/atlasopdev_16_0027.html | Ascend C 自定义算子开发实践（msOpGen/msOpST） | CANN 商用版 8.0.RC2 文档 | 2026-09-12 | msopgen 工程骨架（op_host/op_kernel/build.sh）与 msopst ST 流程 | A | verified |
| 23 | https://github.com/Ascend/msopgen | MindStudio Ops Generator | Ascend/msopgen（2025-12 开源） | 2026-09-12 | msopgen 开源状态与功能表 | B | verified |
| 24 | https://www.hiascend.cn/document/detail/zh/canncommercial/800/devaids/opdev/optool/atlasopdev_16_0021.html | 创建算子工程（json 原型说明） | CANN 8.0.0 文档 | 2026-09-12 | 算子原型 JSON 字段（op/input_desc/output_desc/attr） | A | verified |
| 25 | https://www.mindspore.cn/tutorials/experts/zh-CN/r2.3.0/operation/op_custom_ascendc.html | Ascend C 自定义算子开发与使用指南 | MindSpore 2.3.0 教程 | 2026-09-12 | custom_compiler 离线编译（msopgen 封装） | A | verified |
| 26 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/850/opdevg/BishengCompiler/CANN%E7%A4%BE%E5%8C%BA%E7%89%88%208.5.0%20%E6%AF%95%E6%98%87%E7%BC%96%E8%AF%91%E5%99%A8%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97%2001.pdf | 毕昇编译器用户指南 8.5.0 | CANN 社区版 8.5.0 | 2026-09-12 | `bisheng -O2 --npu-arch=dav-2201` 官方命令样例 | A | verified（PDF 摘录） |
| 27 | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/devguide/opdevg/BishengCompiler/atlas_bisheng_10_0002.html | 毕昇编译器快速上手 | CANN 8.0.RC3 文档 | 2026-09-12 | 旧式选项 --cce-soc-version/--cce-soc-core-type=VecCore、.cce 混合编译 | A | verified |
| 28 | https://www.hiascend.com/developer/blog/details/0289201670005562056 | 案例：CANN 版本问题导致 bisheng 报 unsupported option '--npu-arch' | 昇腾官方博文（2025-12-21） | 2026-09-12 | --npu-arch 需 CANN≥8.3.RC1 的实证（8.2.RC1 失败） | A | verified |
| 29 | https://bbs.huaweicloud.com/blogs/92700f7e560b41e79a12b83c96bf0af8 | CANN 学习资源开源仓：算子开发二 Tiling 和 CMake | 华为云社区（2026-03-26） | 2026-09-12 | find_package(ASC)+LANGUAGES ASC、--npu-arch=dav-2201 CMake 集成、两级并行 tiling 说明 | C | verified |
| 30 | https://www.hiascend.com/productbulletins/detail/803 | 产品公告：昇腾 CANN 9.0.0 版本发布 | hiascend.com（2026-04-30） | 2026-09-12 | CANN 9.0.0 发布时间与特性（判题目标版本） | A | verified |
| 31 | https://github.com/hicann/asc-devkit | Ascend C（asc-devkit） | hicann/asc-devkit（v9.x） | 2026-09-12 | Ascend C 开发工具包开源仓（版本对应） | B | verified |
| 32 | https://blog.csdn.net/xyz3120/article/details/161866077 | 昇腾 950 cv 融合算子体验 | CSDN（2026-06） | 2026-09-12 | CANN 9.0.0 环境 + asc-devkit v9.0.0 源码版本对照、__NPU_ARCH__ 映射旁证 | C | verified |
| 33 | https://blog.csdn.net/gitblog_00820/article/details/151949080 | CANN/asc-devkit AI CPU 算子编译指南 | CSDN（2026-05） | 2026-09-12 | bisheng 编译 .asc/.aicpu 的选项表（--npu-arch 等） | C | verified |
| 34 | https://github.com/endaiHW/tilelang-mlir-ascend | tilelang-mlir-ascend（fork） | endaiHW fork of tile-ai | 2026-09-12 | AscendNPU IR 分支活跃度旁证 | C | verified |

> 检索主查询词：`TVM reduce lowering axis memory planning`、`Triton reduction tl.sum accumulation dtype autotune`、`msopgen msopst CANN operator generation`、`TileLang NPU target Ascend`、`bisheng 编译器 ccec --npu-arch`、`TVM BYOC ascend backend`、`TorchInductor fusion decision reduction epilogue`、`TVM Ansor MetaSchedule search space`、`MLIR linalg reduction lowering vector`、`CANN 9.0.0 版本发布`。

---

**报告收尾说明**：本报告所有"适用性判定"均以"判题只收手写直调单文件 kernel.asc"为前提；报告本身不构成任何实现建议，手写 Ascend C 实现路线以 Agent 2/3 的官方 API 与样例调研为准。
