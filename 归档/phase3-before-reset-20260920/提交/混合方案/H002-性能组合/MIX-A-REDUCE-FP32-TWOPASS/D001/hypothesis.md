# MIX-A D001

## Hypothesis

固定 2048 元素 tile、按行分配 AI Core，并在每个 tile 内先转换到 FP32、平方后使用 `ReduceSum`；Pass 1 只累计每行平方和，Pass 2 重读输入完成归一化、gamma、bias 和输出。这样可以把大 D 的归约工作区限制在 UB 内，同时保留两遍扫描的低峰值内存形态。

## Scope

- 只组合 R006 Reduction Architecture、R017 FP32 Middle、R001 Two-Pass Scan。
- 不保留全局 FP32 y，不建立跨核 partial reduction，不做 gamma/bias 跨行驻留。
- 本候选只要求 server3 CANN compile PASS；不执行本地 NPU correctness、profiling 或性能比较。
