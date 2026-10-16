# N04 V016 结果

## 假设与单一变化

- 父版本：V015，源码提交 `b90d6e0e`，结果提交 `44c7154f`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V015 的 slot C 触发点、下一批首 tile 消费和单一 ready 事件，仅把 slot C 的两次输入 DMA 提交顺序从 `x -> residual` 改为 `residual -> x`；最后 A/B 参数搬运与下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> residual 写入 slot C -> x 写入 slot C -> 最后 A/B 参数 tile -> 下一批首 tile 从 C 消费。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`；最终使用 NPU1。运行前后 HBM Usage Rate 均为 5%，精确进程名查询无 `paired_runner_v016` 残留。
- Compile：PASS。复用当前路线 `paired_runner.cpp`、既有 paired wrapper/CMake 和 CANN 8.5.0.alpha002；`CONFIGURE_RC=0`、`BUILD_RC=0`。日志：[server3-compile-v016-20261017.log](server3-compile-v016-20261017.log)。
- Correctness：PASS，四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`。日志：[server3-v016-correctness-device1-20261017.txt](server3-v016-correctness-device1-20261017.txt)。
- Local：PASS，四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`；完整原始样本：[server3-v016-paired-device1-20261017.txt](server3-v016-paired-device1-20261017.txt)。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 18.260 | 17.980 | -1.700 | -8.540–+3.840 |
| rows=40, D=32768 / FP16 | 38.980 | 30.920 | -8.020 | -13.100–-7.520 |
| rows=40, D=12288 / BF16 | 20.580 | 18.440 | -1.960 | -7.100–+1.860 |
| rows=40, D=32768 / BF16 | 58.020 | 48.340 | -9.680 | -10.420–-8.860 |

- 目标 FP16 D32768 的 P10–P90 全为负，目标稳定改善；BF16 D32768 也全为负；两个 D12288 区间跨零。按当前路线约定保留 `LOCAL_SCORE=NONE`。
- 运行前快照：[server3-before-v016-paired-device1-20261017.txt](server3-before-v016-paired-device1-20261017.txt)；运行后快照：[server3-after-v016-paired-device1-20261017.txt](server3-after-v016-paired-device1-20261017.txt)。
- Official：不执行。
- 设备：沿用已有 paired Runner；使用 NPU1/3/7 等可用入口，不使用 NPU4，不调用设备 Reset。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE
