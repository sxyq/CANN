# N04 V013 结果

## 假设与单一变化

- 父版本：V012，源码结果提交 `5e0fe22c`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V009 的下一批首个 x/residual tile 预取，但把 DMA 提交点从当前最终 tile 的 Vector 计算前移除，改为当前 tile 的 MTE3 `Store` 已提交之后；slot A 仍由 `prel0` 保护，下一批 Pass1 仍等待 `prefetchedInputReady`。V012 的首参数跨阶段预取、V010 的输出双槽和 V011 的参数三槽保持不变。
- 新时序：当前输出 MTE3 已排队 -> 下一批输入 MTE2 提交 -> 下一批 Pass1 消费，测试两个搬运引擎在批次边界的重叠。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`。最终使用 NPU1；运行前后 HBM Usage Rate=5%，运行后 `No process in device.`。
- Compile：PASS。沿用 CANN 8.5.0.alpha002、既有 paired Runner 和 C++ 头文件路径；`CONFIGURE_RC=0`、`BUILD_RC=0`。日志：[server3-compile-v013-20261017.log](server3-compile-v013-20261017.log)。
- Correctness：PASS，四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`。日志：[server3-v013-correctness-device1-20261017.txt](server3-v013-correctness-device1-20261017.txt)。
- Local：PASS，四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`；配对 device-event 中位差（Candidate−V011）如下。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 28.360 | 27.200 | -0.960 | -2.460–0.740 |
| rows=40, D=32768 / FP16 | 43.700 | 38.620 | -4.400 | -21.700–6.000 |
| rows=40, D=12288 / BF16 | 26.140 | 24.060 | -1.820 | -10.180–5.340 |
| rows=40, D=32768 / BF16 | 68.300 | 58.720 | -9.360 | -11.260–-7.660 |

- 完整 Local 原始样本：[server3-v013-paired-device1-20261017.txt](server3-v013-paired-device1-20261017.txt)。目标 FP16 D32768 区间跨零，按路线保留 `LOCAL_SCORE=NONE`。
- Official：不执行；不修改共享记录，不使用 NPU4，不调用设备 Reset。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE
