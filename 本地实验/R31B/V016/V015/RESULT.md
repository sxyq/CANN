# N04 V015 结果

## 假设与单一变化

- 父版本：V014，源码提交 `c103e43a`，结果提交 `357f1c23`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V014 的 slot C 输入和下一批首 tile 消费，仅把预取提交点从当前批最后输出 Store 后移到参数 slot C 的最终 `prel2` 释放点；后续 A/B 参数 tile 和下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> slot C 输入 MTE2 提交 -> 当前批剩余 A/B 参数 tile -> 下一批首 tile 从 C 消费。

- Compile：待执行。
- Correctness：待执行。
- Local：待执行。
- Official：不执行。
- 设备：沿用已有 paired Runner；使用 NPU1/3/7 等可用入口，不使用 NPU4，不调用设备 Reset。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE
