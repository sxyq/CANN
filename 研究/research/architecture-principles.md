# Phase4 架构原则

本文件只记录公开硬件和 Ascend C 资料提炼出的通用原则，不指定候选实现。

1. GM 基础流量至少包括两路输入读取和一路输出写回；参数流量由参数驻留生命周期决定。
2. `D` 过大时，完整 row residency 会受 Vector 工作区、队列深度和临时 reduction 区共同限制。
3. 非 32B 对齐数据必须有明确的 DataCopyPad、mask 或等价尾块方案。
4. FP16/BF16 的 RMS 统计通常需要 FP32 累加，再转换回输入 dtype。
5. TQue 双缓冲只有在搬运和 Vector 计算能够重叠时才有意义，并且所有 slots 都要计入 UB 预算。
6. 跨核 reduction 的性能收益必须与 workspace 流量和同步次数一起评估。
7. `blockDim`、实际参与 Core 数和同步 API 的参与范围必须完全一致。
8. 高阶 API 的 dtype、shape、对齐和临时空间限制不能从其它平台经验推断，必须以目标 toolkit 头文件和编译结果为准。
9. generic fallback 的职责是完整正确，不应成为性能调优分支。

