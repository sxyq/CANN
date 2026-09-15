# Agent 9：竞赛与社区失败案例调研

> 日期：2026-09-11
> 题目：AddRmsNormBias（CANN 9.0.0 / vector kernel / 直调模板）
> 职责：扩大搜索失败模式，归纳为风险清单供主线使用。
> 本机无 CANN、无 Ascend C 编译器、无昇腾 NPU。所有结论仅基于公开 Issue/PR 正文。

---

## 一、失败模式总表

| 模式 | 真实案例 | 根因 | 本题对应检查项 | 证据等级 |
| --- | --- | --- | --- | --- |
| **CE：保留标识符冲突** | InfiniCore #1519：`block_idx` 作局部变量名，被 Ascend C 编译器预处理器展开为内建函数，编译失败 | Ascend C 保留 `block_idx`、`pipe_*`、`GetBlockNum` 等标识符；用户代码中同名变量会被宏替换 | 全文搜索 `block_idx`、`pipe_`、`tid`、`ubuf` 等保留词；本题 V001 已踩 `pipe_` 坑，V003 需再确认无残留 | B |
| **CE：BF16 标量类型转换** | tilelang-ascend #1762 / PR #1758：BiSheng dav-2201 对标量 BF16→FP32 直接 cast 报 `not support bf16 type cast` | BiSheng 后端对 BF16 标量转换支持不完整；向量路径正常，标量路径缺失 | 本题若 BF16 输入需要读取标量 gamma/bias 元素做类型转换，必须走 `AscendC::ToFloat` 而非直接 `(float)` 强转；检查所有 BF16 标量转换路径 | B |
| **CE：UB 容量溢出** | triton-ascend #1638：64×64 `tl.flip` lowering 导致 UB overflow；triton-ascend #1640：4096 元素 reshape+broadcast 链导致 UB overflow；mcore-bridge #188：默认 tile 尺寸导致 `ub overflow, requires 1918976 bits while 1572864 bits available` | tile 过大或中间 buffer 过多，UB（约 192KB）不够用 | 本题需静态估算 UB 占用：x/residual/y/gamma/bias 各 D 元素 + FP32 中间累加；D=64 时 UB 压力小，D=4096 时需分块；多 buffer 策略会翻倍占用 | B |
| **CE：高维 GM/UB 拷贝 lowering 失败** | tilelang-ascend PR #1452：高维 rank>2 的 GM↔UB 拷贝若不正确展平会导致静默数据损坏或编译失败 | Ascend DMA 是 2D 模型（blockCount × blockLen），高维必须展平为 2D | 本题支持 2D/3D/4D，最后一维以外展平为 outer 行；若用 DataCopyPad 按行拷贝，blockCount=outer、blockLen=D×sizeof(T)，无需额外展平 | B |
| **WA：DataCopyPad 非 32B 对齐数据损坏** | tilelang-ascend #1682：`copy_ub_to_gm` 缺少 `DataCopyPadExtParams` padding，blockLen=4B（非 32B 倍数）时数据损坏（max_diff=896）；tilelang-ascend PR #1777：`(M,1)` f32 buffer 的 2D DataCopyPad 产生损坏物理布局 | MTE2/MTE3 DMA 引擎要求 32B 粒度 burst；子 32B 的 2D 块搬运会以损坏布局写入/读取，与 `isPad` 设置无关 | **本题核心风险**：D 非 32 字节倍数时（如 D=48 → 48×4=192B 对齐，但 D=50 → 200B 不对齐；BF16 D=50 → 100B 不对齐），DataCopyPad 尾块必须正确设置 `DataCopyPadExtParams{isPad=true, rightPadding=...}`；否则静默 WA | B |
| **WA：非 32B 对齐 tile 静默错误** | tilelang-ascend #1717：`T.tile.bitwise_not` 在非 32B 对齐 tile 上编译运行成功但 ~10.7% 元素不匹配 | 硬件向量指令按 32B 对齐设计；非对齐尾部未正确 mask | 本题 FP32 D 非 8 倍数、FP16/BF16 D 非 16 倍数时，向量操作尾部需显式 mask；不能依赖默认 mask 覆盖 | B |
| **WA：小 shape tiling 精度问题** | vllm-ascend PR #12424：`npu_dequant_swiglu_quant` 在 `x.shape=[2,192]` 时精度异常，根因是小 shape tiling bug | tiling 逻辑对小 shape 的边界处理不当 | 本题 outer 行数很少时（如 2D shape=[1,D]），需验证单核/少核路径；不能只测大 shape | B |
| **WA：BF16 间歇性不匹配** | tilelang-ascend PR #1777 描述：`MTE2写→MTE3读→V读` 序列中跨管线 RAW 同步被丢弃，导致 online_softmax 示例间歇性 bf16 不匹配 | 同步 pass bug：V 读只与最新访问（MTE3 读）对比，丢失 MTE2_V 同步 | 本题若使用多级 buffer（GM→UB 中转→计算），需确保每级搬运后有正确 pipe 同步；BF16 对同步更敏感 | B |
| **WA/RE：尾块越界读写** | flash-linear-attention-npu PR #324：varlen partial last chunk 时 B-matrix `colCount` 无条件设为 `BT_`，导致 GM→L1 越界读，报 `507015` / MTE DDR out-of-range | 尾块有效长度 < tile 长度时，拷贝长度未用 `min(tile, valid)` 截断 | **本题核心风险**：D 非 tile 整数倍时，最后一块的 DataCopyPad 长度必须是实际剩余元素数，不能用满 tile 长度；写回时也不能覆盖相邻行 | B |
| **WA：32 位偏移溢出** | FlagGems PR #4156：大 tensor 的 index/index_put 因 int32 offset 溢出导致越界；Liger-Kernel #1335：`tl.program_id(0)` 默认 int32 可溢出导致 OOB memory access | 元素索引 × stride 超过 2^31 | 本题 V002 已识别此风险；需确认所有 GM 偏移计算使用 int64_t；`outer * D + col` 在 D 很大、outer 很大时可能溢出 | B |
| **RE：tiling 缓存溢出后异步 MTE 错误** | sgl-kernel-npu #769：tiling 缓存超过 512 条后，临时 tensor 被释放但异步 kernel 仍在使用，报 `DDR address of the MTE instruction is out of range` (507057) | 异步 kernel 生命周期与临时 buffer 生命周期不同步 | 本题直调模板单次编译单次执行，无 tiling 缓存问题；但若 kernel 内使用异步拷贝，需确保同步后再释放 | C |
| **RE：误导性对齐错误信息** | sgl-kernel-npu #424：报 `dim must be multiple of 16 for fp16/bf16 alignment, but got 2560`，但 2560 实际是 16 的倍数；实际要求已变为更大值 | 错误信息与实际硬件要求不同步 | 本题遇到对齐错误时不能只看错误信息字面意思，需查阅 CANN 9.0.0 实际文档确认对齐要求 | C |

---

## 二、与本题直接相关的高风险模式详析

### 2.1 DataCopyPad 尾块（最高优先级）

**问题**：本题 D 可非 32 倍数。当 `D × sizeof(T)` 不是 32B 倍数时：
- FP32：D 不是 8 的倍数 → 不对齐
- FP16：D 不是 16 的倍数 → 不对齐
- BF16：D 不是 16 的倍数 → 不对齐

**tilelang-ascend #1682 实测**：`(M,1)` f32 buffer，blockLen=4B，DataCopyPad 不带 ExtParams 时数据损坏（max_diff=896，元素完全错位）。

**tilelang-ascend PR #1777 根因分析**（更权威）：
> MTE2/MTE3 DMA 引擎要求 32B 粒度的 burst：子 32B 的 2D 块搬运会以损坏的布局写入/读取（元素丢失/移位/槽位复制），与 `isPad` 设置及正确的 `SetFlag/WaitFlag<MTE2_V>` 同步无关。

**修复方向**（来自 PR #1777）：
- 连续窄行（整块不 32B 对齐）：将 2D 搬运折叠为单个 1D burst
- 带行距窄行：编译期拒绝，避免静默损坏

**本题检查项**：
1. 尾块 DataCopyPad 必须设置 `isPad=true` + 正确的 `rightPadding`
2. 输出写回时尾块不能覆盖相邻行的有效数据
3. 验证矩阵必须包含：D=32（对齐）、D=48（FP32 对齐/FP16 不对齐）、D=50（全不对齐）、D=63（接近对齐）

### 2.2 保留标识符（已踩坑，需持续警惕）

**InfiniCore #1519**：`block_idx` 被 Ascend C 定义为编译器内建变量/宏。用户代码中声明 `int block_idx = ...` 会被预处理器展开。

**已知保留词清单**（从社区 Issue 汇总，非官方完整列表）：
- `block_idx`、`block_num`
- `pipe_mte1`、`pipe_mte2`、`pipe_mte3`、`pipe_v`、`pipe_s`、`pipe_m`（及 `PIPE_*` 宏）
- `GetBlockIdx()`、`GetBlockNum()`
- `ubuf`（某些版本）
- `tid`（某些上下文）

**本题 V001 已因 `pipe_` 冲突 CE，V003 需全文扫描确认无残留。**

### 2.3 尾块越界（RE/WA 双重风险）

**flash-linear-attention-npu PR #324**：
> B-matrix `colCount` 无条件设为 `BT_`，当 `rowStart + BT > T` 时 GM→L1 越界读。
> 修复：`colCount = min(BT_, meta.valid)`。

**本题映射**：
- 输入读取：每行拷贝长度 = min(D, 剩余有效元素)
- 输出写回：每行写回长度 = min(D, 剩余有效元素)，且不能写到下一行的起始位置
- gamma/bias 读取：长度 = D（完整读取，无尾块问题）

### 2.4 32 位偏移溢出（V002 已识别）

**Liger-Kernel #1335**：
> Triton 的默认 32 位 `tl.program_id(0)` 可溢出，导致越界内存访问。

**本题映射**：
- GM 偏移 = `outer_idx * D + col_idx`
- 当 outer 很大（如 4D shape=[64,64,64,4096] → outer=64×64×64=262144）且 D 很大时，`262144 × 4096 = 1,073,741,824`（约 1G），未超过 int32 上限
- 但若 shape=[256,256,256,4096] → outer=16M，`16M × 4096 = 68G`，远超 int32
- **所有偏移计算必须用 int64_t**

### 2.5 BF16 特殊风险

**tilelang-ascend #1762**：BiSheng 后端对标量 BF16→FP32 转换报错。
**tilelang-ascend PR #1777**：BF16 对同步 bug 更敏感，间歇性不匹配。

**本题映射**：
- BF16 输入加载后必须转 FP32 再做累加（题目 golden 已要求）
- 标量转换路径需确认 CANN 9.0.0 是否支持直接 cast
- BF16 精度验证需比 FP16 更严格（BF16 尾数位更少）

---

## 三、来源清单

| # | 来源 | URL | 证据等级 | 用途 |
| --- | --- | --- | --- | --- |
| 1 | InfiniCore #1519：Ascend C compilation fails because paged attention uses reserved block_idx | https://github.com/InfiniTensor/InfiniCore/issues/1519 | B | 保留标识符 CE 模式 |
| 2 | tilelang-ascend #1682：copy_ub_to_gm 缺少 padding 配置导致非对齐 blockLen 数据损坏 | https://github.com/tile-ai/tilelang-ascend/issues/1682 | B | DataCopyPad 非对齐 WA 模式 |
| 3 | tilelang-ascend #1717：bitwise_not 非 32B 对齐 tile 静默错误 | https://github.com/tile-ai/tilelang-ascend/issues/1717 | B | 非对齐 tile WA 模式 |
| 4 | tilelang-ascend #1762 / PR #1758：BiSheng BF16 标量转换失败 | https://github.com/tile-ai/tilelang-ascend/issues/1762 | B | BF16 CE 模式 |
| 5 | tilelang-ascend PR #1777：窄行 DataCopyPad 静默错误结果（含根因分析） | https://github.com/tile-ai/tilelang-ascend/pull/1777 | B | DataCopyPad 根因（最详细） |
| 6 | tilelang-ascend PR #1452：高维 GM/UB 拷贝展平修复 | https://github.com/tile-ai/tilelang-ascend/pull/1452 | B | 高维拷贝模式 |
| 7 | triton-ascend #1638：64×64 tl.flip UB overflow | https://github.com/triton-lang/triton-ascend/issues/1638 | B | UB 溢出 CE 模式 |
| 8 | triton-ascend #1640：4096 元素 reshape UB overflow | https://github.com/triton-lang/triton-ascend/issues/1640 | B | UB 溢出 CE 模式 |
| 9 | triton-ascend #1633：tl.flip after tl.cumsum 返回错误值 | https://github.com/triton-lang/triton-ascend/issues/1633 | B | 静默 WA 模式 |
| 10 | mcore-bridge #188：QSA Triton kernel UB/Cc overflow | https://github.com/modelscope/mcore-bridge/issues/188 | B | UB 溢出（含具体数值） |
| 11 | flash-linear-attention-npu PR #324：varlen tail OOB | https://github.com/flashserve/flash-linear-attention-npu/pull/324 | B | 尾块越界 RE 模式 |
| 12 | vllm-ascend PR #12424：小 shape tiling 精度问题 | https://github.com/vllm-project/vllm-ascend/pull/12424 | B | 小 shape WA 模式 |
| 13 | sgl-kernel-npu #424：误导性对齐错误 | https://github.com/sgl-project/sgl-kernel-npu/issues/424 | C | 对齐要求不一致 |
| 14 | sgl-kernel-npu #769：tiling 缓存溢出后 MTE 错误 | https://github.com/sgl-project/sgl-kernel-npu/issues/769 | C | 异步生命周期 RE |
| 15 | Liger-Kernel #1335：int32 program_id 溢出 | https://github.com/linkedin/Liger-Kernel/issues/1335 | B | 32 位偏移溢出 |
| 16 | FlagGems PR #4156：int32 offset overflow in index | https://github.com/flagos-ai/FlagGems/pull/4156 | B | 32 位偏移溢出 |

---

## 四、已确认 / 未找到 / 无法确认

### 已确认

1. Ascend C 保留标识符（`block_idx` 等）会导致编译失败，有 InfiniCore #1519 实证。
2. DataCopyPad 非 32B 对齐搬运会静默损坏数据，有 tilelang-ascend #1682 实测（max_diff=896）和 PR #1777 根因分析。
3. BiSheng 后端对 BF16 标量转换有已知限制，有 tilelang-ascend #1762 实证。
4. 尾块越界拷贝会触发 MTE DDR out-of-range 错误，有 flash-linear-attention-npu PR #324 实证。
5. int32 偏移溢出是跨平台共性问题（Triton/CUDA/Ascend），有 Liger-Kernel #1335 和 FlagGems PR #4156 实证。
6. 小 shape tiling 可能有独立精度 bug，有 vllm-ascend PR #12424 实证。

### 未找到

1. CANN 官方算子竞赛（含 CANNJudge）的公开失败案例集合——搜索 GitHub/GitCode/Gitee 均未找到专门的竞赛失败复盘仓库或 Issue 集合。
2. 昇腾论坛关于算子竞赛提交失败的结构化帖子——论坛为 JS 渲染，无法直接抓取内容。
3. GPU MODE kernel 竞赛的精度失败案例——与 Ascend 无直接关联，仅作类比参考价值有限。
4. CANN 9.0.0 官方关于 DataCopyPad 32B 对齐约束的明确文档说明——未在公开 API 文档中找到。

### 无法确认

1. 本题 15 个测试点的具体 D 值分布——题面未公开，无法确认哪些 D 会触发非 32B 对齐。
2. CANNJudge 平台的编译器版本是否与 BiSheng dav-2201 完全一致——tilelang-ascend 报告的 BF16 标量转换问题是否影响本题取决于此。
3. CANNJudge 是否有提交次数硬限制——题面描述与本项目文档均未找到明确数字。

---

## 五、对本题 V003 的具体建议

基于以上失败模式，V003 在提交前应完成以下检查：

| 检查项 | 对应失败模式 | 具体动作 |
| --- | --- | --- |
| 保留标识符全文扫描 | CE：保留标识符冲突 | `grep -n 'block_idx\|pipe_\|ubuf\|tid' kernel.asc`，确认无用户变量使用保留词 |
| DataCopyPad 尾块参数 | WA：非 32B 对齐 | 尾块 `isPad=true`，`rightPadding = 32 - (D%32)*sizeof(T)%32`（按实际公式） |
| 尾块不越界 | WA/RE：尾块越界 | 输出写回 blockCount×blockLen 不超过实际剩余元素 |
| int64 偏移 | WA：32 位溢出 | 所有 GM 偏移变量声明为 `int64_t` |
| BF16 标量转换 | CE：BF16 转换 | 确认 CANN 9.0.0 支持 BF16→FP32 标量 cast；不支持则用 `AscendC::ToFloat` |
| UB 占用估算 | CE：UB 溢出 | 静态计算所有 UB buffer 总字节数，确认 < 192KB |
| 精度验证矩阵 | WA：小 shape / 非对齐 | 至少覆盖：FP32 D=64（对齐）、FP32 D=50（不对齐）、FP16 D=64、BF16 D=50、3D/4D shape |
| 输出边界 | WA：相邻行覆盖 | 验证 D 非对齐时输出不写到下一行 |
