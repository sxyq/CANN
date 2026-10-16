# N04 V016 结果

## 假设与单一变化

- 父版本：V015，源码提交 `b90d6e0e`，结果提交 `44c7154f`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V015 的 slot C 触发点、下一批首 tile 消费和单一 ready 事件，仅把 slot C 的两次输入 DMA 提交顺序从 `x -> residual` 改为 `residual -> x`；最后 A/B 参数搬运与下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> residual 写入 slot C -> x 写入 slot C -> 最后 A/B 参数 tile -> 下一批首 tile 从 C 消费。

## 服务器事务

- Compile：待执行。
- Correctness：待执行。
- Local：待执行。
- Official：不执行。
- 设备：沿用已有 paired Runner；使用 NPU1/3/7 等可用入口，不使用 NPU4，不调用设备 Reset。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE
