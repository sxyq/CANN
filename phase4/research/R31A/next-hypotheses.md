# R31A Track B 后续假设

记录日期：2026-09-26。V021 保持原样并等待 Main 决定；本文件是只读研究，不建立新 revision，不改任何 kernel。

当前路线证据：V016 Parent SHA-256 为 `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`；V021 SHA-256 为 `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`。目标路径是 V021 `ProcessWideFp32CachedRows`，输入为 FP32、rows=2、blocks=1，D=32768 目标和 D=24576 控制形状。server3 构建使用 CANN 8.5.0.alpha002、`dav-2201`、`Ascend910B3`。

路线内重复筛选依据：V019 把 FP32 CachedRows tile 从 7680 改为 8192，在 D=32768 correctness 失败；V020 对同一路径 RMS 首轮输入尝试双槽 MTE2/V 预取，在目标形状同步失败；V021 只移动逐 tile 输出后的 MTE3/V 等待，Correctness 记录 PASS。以下三个机制均与这三项不同。其他 Route 的 Candidate 尚未比对，重复判断需 Main 复核。

证据索引：当前实现为 `phase4/local/R31A/V021/submission.asc::ProcessWideFp32CachedRows` 及其中的 `Load`/`Store`；历史声明和结果为 `phase4/local/R31A/V019/source-meta.json`、`phase4/local/R31A/V019/local-result.json`、`phase4/local/R31A/V020/local-result.json`、`phase4/local/R31A/V021/diff.patch`。API 依据为 server3 CANN 8.5.0.alpha002 的 `kernel_operator_vec_unary_intf.h`、`kernel_operator_vec_binary_intf.h`，以及本机 Ascend C 技能参考 `api-datacopy.md`。

## 1. FP32 RMS 标量倒平方根

**MECHANISM**：在 FP32 wide CachedRows 路径中，以单元素 `AscendC::Rsqrt` 替换 `Sqrt` 后再做标量 `1.0f / sqrtValue` 的序列。保持平方和归约、epsilon、输出 `Muls`、其他 dtype 路径不动。

**BOTTLENECK**：每行最后一个归约完成后，归一化标量必须串行生成才能开始第二遍输出。当前路径包含单元素 Sqrt、V/S 取值和标量倒数运算。

**EXPECTED_SHAPES**：FP32 rows=2、blocks=1，D=32768 与 D=24576；两种宽度覆盖相同 dispatch 分支，目标形状多一个 7680-element 输出 tile。

**WHY_IT_MAY_HELP**：将向量 Sqrt 加标量倒数合为一个倒平方根向量操作，可能缩短每行输出循环前的标量依赖链。

**WHY_IT_MAY_FAIL**：该路径仍需把单元素结果取回标量供 `Muls` 使用，V/S 往返并未消失；标量倒数也可能不是主耗时。`Rsqrt` 数值精度若低于现有 Sqrt 加倒数，会改变整行输出。

**ASCEND_FEASIBILITY**：server3 CANN 8.5.0.alpha002 的 `kernel_operator_vec_unary_intf.h` 声明 Level 2 `Rsqrt(dst, src, count)`，说明为 `1/sqrt(src)`。构建目标为 DAV_2201；新调用尚未在该 Route 编译或执行验证。

**EVIDENCE**：V021 的 `ProcessWideFp32CachedRows` 当前执行 Sqrt、单元素取值和标量倒数；server3 头文件公开了 `Rsqrt` Level 2 接口。

**UB/CORE/DMA_IMPACT**：原位复用 `xBuf_` 的一个 FP32 元素，不增加 UB，不改变 core 数、输入/输出搬运或 tile 数。

**SYNC_IMPACT**：保留归约后的 V/S 与输出前的 S/V 依赖；不删事件同步。预期改变仅是标量数学路径。

**PRECISION_RISK**：中。倒平方根实现可能近似；需按 V016 精确输入对 D=32768、24576 分别做 targeted correctness，并确认既有 FP32 容差仍满足。

**DUPLICATE_CHECK**：未与 R31A V019 tile 宽度、V020 输入预取、V021 输出完成等待重复；跨 Route 的重复情况待 Main 核对。

**MINIMAL_OFAT_DIFF**：仅改 `ProcessWideFp32CachedRows` 中生成 `invRms` 的单元素算术；不改 reduction、epsilon、输出乘法、同步、tile、dispatch 或其他 dtype。

**EXPECTED_LOCAL_PROBES**：Main 明确允许下一 revision 后，先 server3 编译/链接，再用同一精确形状做 Parent/Candidate targeted correctness；取得 MAIN-1 独占 lease 后，对 V016 Direct Parent 分别建立 D=32768 与 D=24576 same-binary 噪声记录。仅当该形状通过规程且负载可比时，才做交错 P/C。无 lease 时不启动 ACL 或 NPU。

**READINESS**：`NEEDS_MORE_EVIDENCE`。接口声明已确认；目标编译、精度和收益均未确认。

## 2. FP32 affine FusedMulAdd

**MECHANISM**：仅在 FP32 wide CachedRows 输出 tile 中，将 `Mul(valueLocal, valueLocal, gammaLocal)` 后接 `Add(valueLocal, valueLocal, biasLocal)` 替换为 `FusedMulAdd(valueLocal, gammaLocal, biasLocal, valid)`。公开接口定义 `dst = src0 * dst + src1`，对应当前 `normalized * gamma + bias`。

**BOTTLENECK**：输出第二遍每 tile 已有 normalization scalar multiply，随后还有 gamma 乘法、bias 加法和对应的 Vector 顺序屏障；D=32768 每行五个 tile，D=24576 每行四个 tile。

**EXPECTED_SHAPES**：FP32 rows=2、blocks=1，D=32768 作为目标、D=24576 作为控制；只覆盖 `ProcessWideFp32CachedRows`。

**WHY_IT_MAY_HELP**：融合 gamma 乘法与 bias 加法可少一次向量算术操作，并有机会少一次 Vector 屏障。

**WHY_IT_MAY_FAIL**：融合后只舍入一次，和原先 Mul、Add 两次舍入不同；若该路径受 DMA 限制，算术减少的收益可能不明显。实现还需确认 DAV_2201 对 FP32 的实际支持。

**ASCEND_FEASIBILITY**：server3 CANN 8.5.0.alpha002 的 `kernel_operator_vec_binary_intf.h` 声明 Level 2 `FusedMulAdd(dst, src0, src1, count)`，注释给出 `dst = src0 * dst + src1`。DAV_2201 的实际实例化及运行未验证。

**EVIDENCE**：V021 的 `ProcessWideFp32CachedRows` 当前按 gamma Mul 后 bias Add；server3 头文件公开了匹配操作数次序的 `FusedMulAdd` 接口。

**UB/CORE/DMA_IMPACT**：不增加 UB、不增加核心或 DMA；复用现有 `valueLocal`、`gammaLocal`、`biasLocal`。

**SYNC_IMPACT**：可移除 Mul 与 Add 之间的一个 `PIPE_V` 屏障；融合操作后仍需保留结果到 MTE3 store 的顺序要求，以及 V/MTE2、V/MTE3 依赖。

**PRECISION_RISK**：中。FMA 的单次舍入会改变尾数；须覆盖目标和控制宽度的 full-output correctness，并确认绝对误差满足既有比较容差。

**DUPLICATE_CHECK**：R31A 既有 V019、V020、V021 均未融合该 affine 对；V021 只调整输出 MTE3/V 等待。跨 Route 的重复情况待 Main 核对。

**MINIMAL_OFAT_DIFF**：只替换上述 FP32 输出循环中的 Mul+Add 对；保留前置 normalization `Muls`、屏障边界、数据搬运和全部非 FP32 分支。

**EXPECTED_LOCAL_PROBES**：获 Main 授权后，先确认 DAV_2201 编译与链接；做 D=32768、24576 targeted correctness；静态比较生成的 Vector 操作数和屏障数。取得独占 lease 后分别完成 Parent same-binary 形状资格，再按统一规程执行交错 P/C。当前不执行这些探测。

**READINESS**：`NEEDS_MORE_EVIDENCE`。接口语义已核对；硬件实例化、精度影响和性能信号未知。

## 3. 对齐 FP32 tile 的 DataCopy 快路径

**MECHANISM**：只在 FP32 wide CachedRows 分支，为已经满足 32-byte 地址和长度对齐的 x、residual、gamma、bias 读入及 output 写回评估 `DataCopy`，替代共享 `Load`/`Store` 使用的 `DataCopyPad`。不得改动其他 dtype 或非对齐路径。

**BOTTLENECK**：每行每 tile 有多次 GM/UB 搬运；若对齐复制可以减轻每次搬运的参数或边界处理开销，多个 tile 的累计效果可能有小幅收益。

**EXPECTED_SHAPES**：FP32 rows=2、blocks=1，D=32768 与 D=24576。当前 tile 为 7680 个 FP32，即 30720 bytes；目标尾 tile 为 2048 个元素，控制尾 tile 为 1536 个元素。tile offset 和这些长度均为 8 个 FP32 的倍数。

**WHY_IT_MAY_HELP**：当前通用 `Load`/`Store` 一律建立 `DataCopyExtParams` 并调用 `DataCopyPad`；目标尺寸从静态计算看满足 32-byte 长度对齐，存在移除边界处理路径的可能。

**WHY_IT_MAY_FAIL**：本地 DataCopy 指引说明对齐场景下 `DataCopy` 与 `DataCopyPad` 的性能差异可忽略。底层可能落到相同 MTE 指令，收益为零；GM 基址及 UB tile 起始地址还必须在目标上确认对齐。

**ASCEND_FEASIBILITY**：Ascend C 指引允许严格 32-byte 对齐的 GM↔UB `DataCopy`，非对齐必须保留 `DataCopyPad`。所有运行地址的对齐属性仍需由编译后实现和 targeted correctness 确认。

**EVIDENCE**：V021 的 `ProcessWideFp32CachedRows` 调用通用 `Load`/`Store`，二者使用 `DataCopyPad`；Ascend C 技能参考 `api-datacopy.md` 列出 32-byte 对齐条件，并指出对齐场景的性能差异可忽略。

**UB/CORE/DMA_IMPACT**：不增加 UB、不改变核心分工、复制字节数或 DMA 次数；只改变对齐搬运 API 路径。

**SYNC_IMPACT**：保留现有 MTE2/V、V/MTE2、V/MTE3、MTE3/V 同步顺序，不用 API 替换顺带删除同步。

**PRECISION_RISK**：低至中。算术不变，但错误的基址、尾 tile 或有效长度对齐判断会造成越界或搬运错误；必须覆盖两种形状及每行尾 tile。

**DUPLICATE_CHECK**：不同于 V019 的 tile 宽度变更和 V020 的输入双槽预取；只变更搬运 API。跨 Route 的重复情况待 Main 核对。

**MINIMAL_OFAT_DIFF**：新增仅供 `ProcessWideFp32CachedRows` 调用的对齐 load/store helper，并只替换该函数中的五类搬运调用；保留通用 helper 和其他路径不变。

**EXPECTED_LOCAL_PROBES**：Main 授权后先用静态算术列举每个 tile 的 GM/UB offset 与 byte length，再编译并比较 DAV_2201 生成的搬运指令；若产物指令相同则不进入设备测试。若不同，先跑两形状 targeted correctness，再在独占 lease 下做 Parent same-binary 形状资格和交错 P/C。

**READINESS**：`NEEDS_MORE_EVIDENCE`，优先级低。对齐成立的依据明确，但 API 文档预期收益很小，须先由生成代码证明确有差异。
