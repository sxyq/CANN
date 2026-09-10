# 调研报告：AddRmsNormBias 的 Ascend C 实现路线

> 调研日期：2026-09-10。每条结论尽量附来源链接与证据等级（A=官方文档原文 / B=官方仓库源码或官方培训权威文章 / C=开源社区二手资料）。逐条标注“是否在真实 NPU 验证”。完整来源清单见 `sources.md`。

## 1. 官方 Ascend C 编程模型（A 级，未在 NPU 验证）

- 昇腾 AI Core 计算单元：标量单元 / 向量单元（Vector，如 AIV）/ 矩阵单元（Cube）；Vector 指令基于 256B 数据块、每次 repeat 处理元素数 = 数据类型决定（fp16=128、fp32=64）。
- 内存层级：GM（Global Memory，板载大容量）→ L2/HBM → UB/Local（片上，容量小、访问快）。**数据必须在 Global 与 Local 间显式搬运**。
- Ascend C 的 LocalTensor（UB）与 GlobalTensor（GM），配套 `DataCopy`/`DataCopyPad` 搬运与 `TQue`（TPosition Queue）双缓冲流水。
- 核函数由 `__global__ __aicore__` 入口启动，`GetBlockIdx()` 获得核号，`GetBlockNum()` 获得核数；多核并行是默认性能模型。
- 参考（证据 A）：华为官方文档“Ascend C 向量化编程”、“无DataCopyPad的处理方式”（hiascend 文档）、昇腾开发者社区训练营（B 级）。

## 2. GlobalTensor / LocalTensor / TQue 相关机制（A/B 级）

- `GlobalTensor<T>`：指向 GM 的句柄，通过 `SetGlobalBuffer(ptr, len)` 绑定；用于 CopyIn（GM→UB）/ CopyOut（UB→GM）的源/目的。
- `LocalTensor<T>`：UB 内存句柄，由 `TPipe::InitBuffer(TPosition::VECIN/VECOUT/VECCALC, bytes)` 分配；TQue 通过 AllocTensor/EnQue/DeQue/FreeTensor 管理双缓冲。
- TQue（TBufferQueue）双缓冲：CopyIn 与 Compute 可在不同 buffer 上重叠，提升吞吐。
- `DataCopy`（基础搬移）要求 GM/Local 地址与长度满足 32B 对齐约束；**非对齐场景必须用 DataCopyPad**（见第 4 节）。

## 3. UB 分块与最后一维归约方案（重点）

### 3.1 总体策略：按行（样本）切分，多核均分行

- 视后续维度产物：`outer = batch*seq*heads`，每行长度 D。每核处理 `outer / coreNum` 行（余数分摊到前几核）。
- 每行内部流程（两遍）：
  1. **Pass1（归约）**：逐块搬入 x、residual → Cast FP32 → Add（y=x+residual）→ Mul（y²）→ `ReduceSum` 得块内平方和 → 跨块标量累加 → 行末 `rms = sqrt(sum/D + eps)`。
  2. **Pass2（归一化+偏置）**：重读同块 x、residual、gamma、bias → Cast FP32 → Add → 乘 `1/rms` → 乘 gamma → 加 bias → Cast 回原 dtype → 搬出。
- 若 D ≤ 单块预算（如 D≤4096），一行一次搬入即可（训练营 Case1）；D 大时（32768）分段（Case2），段内 ReduceSum + 段间标量/向量累加。

### 3.2 ReduceSum 使用要点（A 级证据：CANN API 手册）

- 原型：`ReduceSum(dstLocal, srcLocal, workLocal, count)`（tensor 前 n 个数据）—— 结果放在 `dst[0]`；
  或 mask 连续模式版本：`ReduceSum(dst, src, work, mask, repeatTimes, srcRepStride)`。
- 数据类型：Atlas A2 训练/推理支持 half、float → **FP32 归约可行**。
- 对齐：`srcLocal` 起始地址需 **32B 对齐**；`dstLocal` 按数据类型（fp32=4B）对齐。
- mask 连续模式每次 repeat 元素数：fp16 ∈[1,128]、fp32 ∈[1,64]；尾块有效元素可通过 mask 限定。
- **workLocal 空间**（手册给出公式）：对 count 版本 `firstMaxRepeat = count / elementsPerRepeat(min 1)`，`workLocal` 需 ≥ `RoundUp(firstMaxRepeat, 32/typeSize) * (32/typeSize)` 个元素。实现按此预留。
- 二叉树累加顺序（A2 高维切分/前n个）：两两相加；FP32 累加可显著降低 f16/bf16 平方和的误差累积。

### 3.3 为什么要 FP32 累加（B 级，社区权威训练营结论 + 官方 ops-transformer 精度分析）

- 官方 CANN 训练营第二十期“LLaMA 核心算子 RMSNorm”：**黄金法则——做 ReduceSum 前必须转 FP32**，否则 fp16 平方极易溢出（65504 上限）或精度损失。
- ops-transformer RMSNorm 数值分析文章同样强调溢出/下溢与 Kahan 求和必要性（Kahan 用于高精度需求，本题 1e-3/1e-4 预算下 FP32 累加通常已够，可作优化项）。
- 实现顺序：`Cast(x, fp32) → Mul(平方) → ReduceSum(fp32) → GetValue(0) → 标量 rsqrt`。（训练营给出的标准三步）

## 4. FP16 / BF16 输入输出处理（A/B 级）

- 输入可能是 half/bfloat16_t/float：统一 `Cast` 到 float 参与计算；最终输出 `Cast` 回原类型。
- Cast round 模式：整数/无损转换用 `CastMode/CAST_NONE`（fp16→fp32、bf16→fp32 精确）；输出 fp32→fp16 如需舍入用 `CAST_RINT`（保守做法：不对称优化时按默认模式）。真机验证两种模式的精度差异。
- bfloat16：Ascend C 提供 `bfloat16_t` 类型；DataCopyPad 在 A2 支持 `bfloat16_t`（8.x 手册数据列表含 bfloat16_t，9.0.0 应在 A2 继续支持，真机确认）。
- 概率风险：bf16 位宽小，若用 fp32 cast 中间 + 末尾单次 cast，相对误差 ~2^-8≈3.9e-3？→ 注意！**bf16 本身精度 8 位尾数，任意实现也不可能把 bf16 输出相对误差压到远小于 2^-8**。官方阈值 1e-3 相对 bf16 精度（约 4e-3 的 one-ulp）是合理的——判题基准同为 bf16 计算，误差按“与 PyTorch bf16 组合结果对比”而非与真值对比。故：输出对齐 PyTorch 同 dtype 舍入即可达标；绝对误差也 <1e-3 可能对量级大的值有要求（如 |v| 大时相对满足则绝对自然，需在真机逐点统计）。

## 5. 尾块和对齐处理（A/B 级，DataCopyPad 为本实现地基）

- 关键约束：DMA 传输对基础 `DataCopy` 有 32B 对齐要求；**DataCopyPad 专为非对齐设计**。
- 官方 DataCopyPad 功能（A 级手册）：
  - 通路：`GM→VECIN/VECOUT`（可指定填充值，自动补 0/指定值）、`VECIN/VECOUT→GM`（搬出）。
  - **Global 地址无对齐约束**（可任意字节地址）；Local 起始需 32B 对齐；`blockLen` 单位为 Byte 且支持非对齐长度。
  - 产品支持：**Atlas A2 训练系列 / Atlas 800I A2 推理产品支持**（无 mode 模板参数版本）；Atlas 推理系列（910B 等）不支持（需 GatherMask/UnPad/atomic 降级方案，见官方“无DataCopyPad的处理方式”）。
- 本实现的尾块策略：
  - 搬入尾块（CopyIn）：`DataCopyPad(dstLocal, srcGlobal, params, padParams)` 长度=实际字节，自动补 0 → 归约不受污染（0²=0）。
  - 搬出尾块（CopyOut）：`DataCopyPad(dstGlobal, srcLocal, params)` 非对齐搬出，避免覆盖相邻行。
- 备选（若判题机无 DataCopyPad）：GatherMask 借位搬运 / 自 kernel 清零 + atomic 累加 / Duplicate 掩码 —— 记录在案，仅当真机发现 A2 不支持时启用。

## 6. 官方样例与 API 来源（A/B 级）

- CANN 官方 Ascend C 文档（见 sources.md）：DataCopyPad、ReduceSum、Cast、GetValue、TPipe/InitBuffer、无 DataCopyPad 处理方式、Aclnn 算子工程化开发快速入门。
- 昇腾 CANN 训练营第二十期（RMSNorm 完整实现，B 级）：`Cast → Mul → ReduceSum → GetValue(0) → rsqrt → Muls → Mul(gamma)`；该文为最接近本题官方教学实现。

## 7. 开源项目相关（B 级为主，均未在本次工作区本地运行）

- **cann-learning-hub**（昇腾学习资源开源仓）：noc 提及中级算子开发一二、AddCustom 工程级样例；msopgen 生成工程结构与其一致。gitcode.com/cann/cann-learning-hub（未实际 clone，B 级来源为华为云博客转述 + 目录展示）。
- **Ascend/ops-transformer**（gitcode.com/cann/ops-transformer，GitHub 无官方镜像，有 ai2open fork）：
  - 含 `add_rms_norm` 系列（如 `moe_distribute_combine_add_rms_norm`、`inplace_matmul_all_reduce_add_rms_norm`、`aclnnAddRmsNormQuantV2`）——**与本题 AddRmsNormBias 语义最接近的官方实现系列**，公式 `y = (1/Rms(x)) * x * gamma + beta` 与本题完全同构（beta 即本题 bias）。
  - 仓库结构（B 级）：mc2/ 下每算子一个目录，含 op_host/、op_kernel/、examples/、docs；支持 Atlas A2（官方页面列出 A2 支持矩阵）。
  - 迁移关系：本题可以直接借鉴其 kernel 归约/尾块/FP32 累加模式，但不是"抄同一算子"（官方 AddRmsNorm 多为 MoE/MM 组合上下文，本题是独立 T 算子 + 属性 epsilon）；差异点：本题仅 4 输入 1 输出小算子、无通信/量化、需要额外支持 fp32 输入与 D 非齐尾块。
- **Ascend/cann-samples**：AscendC 快速入门样例（AddCustom 等），用于生成工程与编译流程模板（未在本地验证编译）。
- **ops-nn / asc-devkit**：Acclnn 量化 RMSNorm（AddRmsNormQuantV2）与 Reg 级非对齐搬出（StoreUnAlign，A3/950 系，A2 不支持）——记录为潜在参考，当前版本不依赖。

本轮实际读取的公开仓库快照：`/tmp/cann-samples.vc2a1d`，commit
`23c981c0918e3183958e94e58ef6989d44983230`；`/tmp/ops-transformer.eKIcVc`，commit
`e7019c299cfc02293b184dcf0ca270b08740cdc1`。前者读取
`Samples/2_Performance/rms_norm_quant_story/src/0_naive.asc`、`2_multi_core.asc`、
`6_binary_sum.asc`；后者读取 `mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm.h`、
`mc2/3rd/rms_norm/op_kernel/rms_norm_base.h` 及对应 Host tiling 源码。以上源码用于 API 和算法迁移判断，未在本机编译。

## 8. OpenAI 公开 Agent 研究方式与本题迁移边界

### 8.1 已核对的公开事实

- OpenAI News RSS 的 `On the Navier-Stokes Millennium Prize Problem` 条目明确写明：分享 AI 生成的解答、写作说明和 Lean 形式化证明。
- OpenAI News RSS 的离散几何条目明确写明：模型处理了有 80 年历史的 Erdős unit distance problem，并声称推翻其中一个主要猜想。准确名称是“单位距离问题/猜想”，不是“单位圆距离猜想”。
- OpenAI 预印本 `Finite Time Blowup for Navier-Stokes` 的摘要和定理 1.1 给出三维不可压 Navier-Stokes 的有限时间速度无界、动能有界构造。论文第 1 页、目录、第 6-7 页的证明纲要显示，最终产物按背景流、振荡脉冲、应力调整、局部化和全空间构造组织。
- OpenAI 的 `NavierStokesAndEuler` 官方 GitHub 仓库 README 说明包含 Lean 4 形式化，并给出 `lake exe cache get`、`lake build` 和 Comparator 独立核验入口；本机没有 Lean 环境，未执行构建。
- Nature 独立报道披露：简化问题阶段约使用 1,000 个 Agent、持续约 50 小时；完整 Navier-Stokes 阶段扩大到约 10,000 个 Agent，并指出结果仍需要数学界独立审阅。该信息是 C 级独立报道，不是 OpenAI 官方技术规格。
- CNBC 独立报道转述：约 10,000 个协作 Agent 可以读取互联网缓存、运行代码，并按组协作和在组内通信；报道给出约 88 小时的整体时间。这里能确认的是高层能力描述，不能由此推导出具体调度器、提示词或任务路由。

OpenAI 官方公开材料没有给出完整的 Agent 调度、提示词、模型组合、通信格式或算力分配细节。不能从论文的章节结构倒推出内部 Agent 分工；不能把报道中的数量写成比赛方案的硬指标。Clay Mathematics Institute 的 Navier-Stokes 页面在 2026-09-11 仍显示 `Active`，因此不能写成数学界已经正式接受。完整的资料核对和迁移说明见 `openai-agent-research.md`。

### 8.2 可迁移的研究组织方式

从上述公开材料可以提炼一套适合本题的工程组织方式：

```text
定义可验收问题
→ 拆分题面、API、Kernel、数值和性能子问题
→ 并行探索多个实现路线
→ 共享中间结果、反例和来源
→ 用编译器、运行时、数值参考或形式化工具验证
→ 独立复核关键结论
→ 汇总差异、风险、证据和下一步实验
```

迁移到 AddRmsNormBias 时，Agent 的验收对象应是：题面接口是否确认、CANN 9.0.0 API 是否与模板相容、FP32 归约是否正确、D 尾块是否安全、FP16/BF16 舍入是否满足判题、以及真实 NPU 上的编译、精度和性能记录。多 Agent 只能提高探索覆盖，不能替代 NPU 运行证据。

### 8.3 不可直接迁移的内容

- Navier-Stokes 是连续介质方程的数学构造问题；本题是固定张量形状下的 Ascend C 算子工程问题。
- Lean 形式化证明可以验证数学命题，但不能验证 Ascend C 的 DMA 对齐、UB 容量、SoC 指令支持或 Kernel 性能。
- 论文中的证明纲要不能直接转化为 Kernel 代码；当前实现仍需逐 API、逐 dtype、逐尾块在真实环境核对。

证据状态：官方 RSS 和预印本为 A 级，Nature 为 C 级；OpenAI 相关正文页面在当前网络环境返回 Cloudflare 403，因此正文细节只保留可直接读取的 PDF、RSS 和独立报道内容。以上 OpenAI 研究与本题 Kernel 的关系属于组织方式参考，未在 NPU 上验证。

## 9. Issue / Release / 版本兼容信息（C 级，需真机复核）

- CANN 9.0.0 相对 8.x 的 DataCopyPad 支持矩阵未见公开变更公告；按“A2 系列支持 DataCopyPad 无 mode 版本”的 8.0 手册推断 9.0.0 兼容，**未验证**。
- ops-transformer master 分支持续活跃（2026-09 仍有合并），其 AddRmsNorm 系列接口在较新 CANN 上可用；但其代码针对 server 版 CANN（含 libascend），与判题“vector 核函数工程”环境的适配度待真机确认。
- msopst（单算子 ST 测试）工具随 CANN 包提供，可用作真机精度/性能验证入口。

## 10. 尚未在真实 NPU 验证的内容（明确清单）

1. DataCopyPad 在 CANN 9.0.0 + 判题机 SoC 上的实际可用性（A2 系概率高）。
2. `bfloat16_t` 的 Cast/搬运在同一环境的行为。
3. GetValue(0) 标量同步的耗时量级与对 15 点性能的影响。
4. msopgen（CANN 9.0.0）生成工程模板与本工作区 `源码/` 骨架的 API 兼容性（op_proto 宏、tiling 宏、CMake 预设）。
5. 判题机 SoC 型号 → `-c ai_core-<soc>` 参数取值。
6. 判题误差判定对 bf16 输出与 PyTorch 基准对比语义的确认。
7. 平台 15 测试点与 50 次/天的明文出处。


## 11. Host 层数值验证记录

见 `validation-host-notes.md`（无 NPU 辅助验证：fp32 全通过；fp16/bf16 达 dtype 固有精度上限，判题容差语义需真机确认）。
