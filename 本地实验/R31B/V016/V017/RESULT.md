# N04 V017 结果

## 假设与单一变化

- 父版本：V016，源码提交 `c3284183`，结果提交 `d8ad775d`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V016 的 residual-first 提交、V015 的 slot C 触发点和下一批首 tile 消费，仅把 slot C 的 residual/x 完成通知拆成两个 `MTE2_V` 事件；下一批首 tile 等待第二次 DMA 对应的 x 事件，旧 residual 事件在后续可复用点或函数尾部消费。最后 A/B 参数搬运与下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> residual 写入并发出 residual-ready -> x 写入并发出 x-ready -> 最后 A/B 参数 tile -> 下一批首 tile 等待 x-ready 后从 C 消费。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE

## 服务器事务

- 当前尚未执行 Compile、Correctness 或 Local；复用现有 `paired_runner_v016`、`paired_runner.cpp` 和既有 CMake 入口。
- Official：不执行。
- 设备入口：仅使用 NPU1/3/7 等安全设备，不使用会 Reset NPU4 的 Runner。
