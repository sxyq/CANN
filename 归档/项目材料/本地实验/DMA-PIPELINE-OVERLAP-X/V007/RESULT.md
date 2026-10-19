# N04 V007 结果

## 变更与编译

- 父版本：V006；Candidate 源码提交：`3204fc84`。
- 单一变化：FP16、rows=40、D=32768 时，Pass 1 输入释放后、逐行 RMS 收尾前，提前提交 Pass 2 首块 gamma/bias 的 GM→UB 搬运；其余 shape 沿用 V006 顺序。
- Compile：PASS，`COMPILE_RC=0`；现有 paired Runner 构建于 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V007/build/paired_runner_v016`。

## 设备运行

- NPU3，Ascend910B3；Main 分配快照为 HBM 3428/65536 MiB。
- 路线启动前采样：2026-10-16 09:33:52Z，Health=OK、HBM 使用率 5%、AICore=0%、无进程。
- 启动 CANN 环境：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`。
- 命令：`./paired_runner_v016 --paired --device 3`；每组 warmup 45、21 对。
- 开始：09:34:29Z；结束：09:34:37Z；Runner RC=0。
- Correctness：Parent 与 Candidate 均 4/4 PASS，failures=0。

## Local 结果

paired delta 为 Candidate−Parent，负值表示 Candidate 耗时较低。

| Shape / dtype | Parent median (μs) | Candidate median (μs) | Delta median (μs) | Delta P10–P90 (μs) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 18.440 | 17.220 | -1.580 | -2.840–13.060 |
| rows=40, D=32768 / FP16（目标） | 39.380 | 33.220 | -6.220 | -7.060–9.000 |
| rows=40, D=12288 / BF16 | 22.020 | 26.600 | -2.440 | -4.540–6.840 |
| rows=40, D=32768 / BF16 | 67.380 | 58.780 | -8.880 | -10.660–-8.100 |

`LOCAL_SCORE=NONE`。目标 FP16 D=32768 的 paired median 为 -6.220 μs，但 P10–P90 跨零；其他 shape 也不足以支持稳定的机制归因。Official 未运行。

## 设备收尾与原始证据

- Postflight：2026-10-16 09:34:37Z，Health=OK、HBM 使用率 5%、AICore=0%、无进程；Main 记录该时刻释放 NPU3。
- 启动前快照：[server3-before-paired-device3-20261016-window.txt](server3-before-paired-device3-20261016-window.txt)
- 完整 Runner 输出、逐样本与统计及 postflight：[server3-v007-device3-paired-20261016.txt](server3-v007-device3-paired-20261016.txt)
