# N04 V014 结果

## 假设与单一变化

- 父版本：V013，源码与结果提交 `6794c03e`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V013 的跨批输入 DMA 时序，但把下一批首个 x/residual tile 写入第三输入 slot C；下一批 Pass1 首 tile 从 C 消费，后续 tile 恢复 A/B 交替。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前输出 MTE3 已排队 -> slot C 输入 MTE2 提交 -> 下一批首 tile 从 C 消费 -> 后续输入恢复 A/B。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`；最终使用 NPU3。运行前后 HBM Usage Rate 均为 5%，精确进程名查询无 `paired_runner_v016` 残留；NPU3 不支持 `npu-smi info proc` 查询。
- Compile：PASS。最终复用当前路线 `paired_runner.cpp`、既有 paired wrapper/CMake 和 CANN 8.5.0.alpha002；`CONFIGURE_RC=0`、`BUILD_RC=0`。最终日志：[server3-compile-v014-runner-fix-20261017.log](server3-compile-v014-runner-fix-20261017.log)。首次使用旧 Runner 的 Compile 也通过，但其模式入口不含 `--correctness`，未作为有效运行入口；失败原始证据：[server3-v014-runner-mode-mismatch-device3-20261017.txt](server3-v014-runner-mode-mismatch-device3-20261017.txt)。
- Correctness：PASS，四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`。日志：[server3-v014-correctness-device3-20261017.txt](server3-v014-correctness-device3-20261017.txt)。
- Local：PASS，四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`；完整原始样本：[server3-v014-paired-device3-20261017.txt](server3-v014-paired-device3-20261017.txt)。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 17.740 | 16.300 | -1.440 | -1.700–-0.600 |
| rows=40, D=32768 / FP16 | 39.500 | 31.400 | -8.260 | -10.720–-7.140 |
| rows=40, D=12288 / BF16 | 22.240 | 22.060 | -0.720 | -12.160–+2.140 |
| rows=40, D=32768 / BF16 | 58.580 | 49.200 | -9.340 | -11.420–-8.080 |

- 目标 FP16 D32768 的 P10–P90 全为负，目标稳定改善；BF16 D32768 也全为负；BF16 D12288 区间跨零。按当前路线约定保留 `LOCAL_SCORE=NONE`。
- 运行前快照：[server3-before-v014-paired-device3-20261017.txt](server3-before-v014-paired-device3-20261017.txt)；运行后快照：[server3-after-v014-paired-device3-20261017.txt](server3-after-v014-paired-device3-20261017.txt)；最终设备快照：[server3-final-v014-device3-20261017.txt](server3-final-v014-device3-20261017.txt)。
- 失败入口证据保留：[server3-before-v014-runner-mode-mismatch-device3-20261017.txt](server3-before-v014-runner-mode-mismatch-device3-20261017.txt)、[server3-after-v014-runner-mode-mismatch-device3-20261017.txt](server3-after-v014-runner-mode-mismatch-device3-20261017.txt)。
- Official：不执行；不修改共享记录，不使用 NPU4，不调用设备 Reset。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE
