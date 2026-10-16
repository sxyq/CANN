# N04 V013 结果

## 假设与单一变化

- 父版本：V012，源码结果提交 `5e0fe22c`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V009 的下一批首个 x/residual tile 预取，但把 DMA 提交点从当前最终 tile 的 Vector 计算前移除，改为当前 tile 的 MTE3 `Store` 已提交之后；slot A 仍由 `prel0` 保护，下一批 Pass1 仍等待 `prefetchedInputReady`。V012 的首参数跨阶段预取、V010 的输出双槽和 V011 的参数三槽保持不变。
- 新时序：当前输出 MTE3 已排队 -> 下一批输入 MTE2 提交 -> 下一批 Pass1 消费，测试两个搬运引擎在批次边界的重叠。

## 服务器事务

- Compile：待执行。
- Correctness：待执行。
- Local：待执行。
- Official：不执行。
- 设备：沿用已有 paired Runner；不调用设备 Reset，不使用 NPU4。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE
