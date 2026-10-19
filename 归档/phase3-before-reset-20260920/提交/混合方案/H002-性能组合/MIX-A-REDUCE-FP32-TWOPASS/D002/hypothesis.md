# MIX-A D002

## Hypothesis

D001 的 2048 元素 tile 可能在 testcase 2 的实际分块边界触发 ReduceSum 或尾块搬运的运行稳定性问题。D002 只把固定 tile 降为 1024 元素，并将对应的最大对齐容量设为 1040 元素，降低单次归约长度和 UB 搬运跨度；R006 分块归约、R017 FP32 中间、R001 两遍扫描与按行调度保持不变。

## Scope

- 唯一局部改动：`kTileElements=1024`、`kPaddedTileElements=1040`。
- 不引入全局 y、跨核 partial reduction、gamma/bias 跨行驻留或其它 Track。
- 本候选只要求 server3 CANN compile PASS；不执行本地 NPU correctness、profiling 或性能比较。
