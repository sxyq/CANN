# N04 V014 结果

## 假设与单一变化

- 父版本：V013，源码与结果提交 `6794c03e`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V013 的跨批输入 DMA 时序，但把下一批首个 x/residual tile 写入第三输入 slot C；下一批 Pass1 首 tile 从 C 消费，后续 tile 恢复 A/B 交替。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前输出 MTE3 已排队 -> slot C 输入 MTE2 提交 -> 下一批首 tile 从 C 消费 -> 后续输入恢复 A/B。

## 服务器事务

- Compile：待执行。
- Correctness：待执行。
- Local：待执行。
- Official：不执行。
- 设备：沿用已有 paired Runner；使用 NPU1/3/7 等安全入口，不使用 NPU4，不调用设备 Reset。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE
