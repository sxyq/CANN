# N04 V015 结果

## 假设与单一变化

- 父版本：V014，源码提交 `c103e43a`，结果提交 `357f1c23`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V014 的 slot C 输入和下一批首 tile 消费，仅把预取提交点从当前批最后输出 Store 后前移到参数 slot C 的最终 `prel2` 释放点；后续 A/B 参数 tile 和下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> slot C 输入 MTE2 提交 -> 当前批剩余 A/B 参数 tile -> 下一批首 tile 从 C 消费。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`；最终使用 NPU7。运行前后 HBM Usage Rate 均为 5%，精确进程名查询无 `paired_runner_v016` 残留。
- Compile：PASS。复用当前路线 `paired_runner.cpp`、既有 paired wrapper/CMake 和 CANN 8.5.0.alpha002；`CONFIGURE_RC=0`、`BUILD_RC=0`。日志：[server3-compile-v015-20261017.log](server3-compile-v015-20261017.log)。
- Correctness：PASS，四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`。日志：[server3-v015-correctness-device7-20261017.txt](server3-v015-correctness-device7-20261017.txt)。
- Local：PASS，四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`；完整原始样本：[server3-v015-paired-device7-20261017.txt](server3-v015-paired-device7-20261017.txt)。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 27.440 | 17.940 | -1.340 | -12.660–+5.680 |
| rows=40, D=32768 / FP16 | 56.840 | 44.520 | -9.700 | -20.580–-7.040 |
| rows=40, D=12288 / BF16 | 29.060 | 26.980 | -2.760 | -11.020–+8.200 |
| rows=40, D=32768 / BF16 | 72.960 | 63.580 | -9.480 | -19.720–-4.120 |

- 目标 FP16 D32768 的 P10–P90 全为负，目标稳定改善；BF16 D32768 也全为负；两个 D12288 区间跨零。按当前路线约定保留 `LOCAL_SCORE=NONE`。
- 运行前快照：[server3-before-v015-paired-device7-20261017.txt](server3-before-v015-paired-device7-20261017.txt)；运行后快照：[server3-after-v015-paired-device7-20261017.txt](server3-after-v015-paired-device7-20261017.txt)。
- Official：不执行。
- 设备：沿用已有 paired Runner；使用 NPU1/3/7 等可用入口，不使用 NPU4，不调用设备 Reset。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE
