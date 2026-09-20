# MIX-A D004

## Hypothesis

D003 的 Pass 2 在 `CopyOut` 发起异步 UB 到 GM 搬运后立即释放输出和 FP32 缓冲；当下一 tile 或下一行复用同一队列时，MTE3 尚未完成可能造成运行不稳定。D004 只在每个输出 tile 的 `CopyOut` 后加入 `PipeBarrier<PIPE_MTE3>()`，等待写回完成再释放缓冲，保持 R006 分块归约、R017 FP32 中间、R001 两遍扫描和按行调度不变。

## Scope

- 唯一局部改动：Pass 2 输出搬运后的 MTE3 同步。
- 不改变 tile 大小、尾块 padding、归约工作区、输入复读次数或行分配。
- 本候选只要求 server3 CANN compile PASS；不执行本地 NPU correctness、profiling、benchmark 或线上提交。
