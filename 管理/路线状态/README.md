# 路线状态

每个 `Rxxx.json` 是一条路线的机器可读状态。`official_score` 只有在 CANNJudge 返回有效分数后才能填写；本地编译、NPU 运行和 profiler 数据只能放在 `local_reference`，不能代替官方分数。

状态含义：

- `PLANNED`：只有研究材料，尚未形成可验证版本。
- `EXPLORING`：已有版本或实验，但尚未完成官方分数驱动的性能收敛。
- `BLOCKED`：有明确门禁或环境阻塞，保留反例并等待修复/资源。
- `CONVERGED`：满足统一流程中的官方分数收敛判据。
- `REOPENED`：收敛后因新证据重新打开。
