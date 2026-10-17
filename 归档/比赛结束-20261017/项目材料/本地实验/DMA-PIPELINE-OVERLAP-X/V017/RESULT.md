# N04 V017 结果

## 假设与单一变化

- 父版本：V016，源码提交 `c3284183`，结果提交 `d8ad775d`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行跨批边界。
- 单一变化：沿用 V016 的 residual-first 提交、V015 的 slot C 触发点和下一批首 tile 消费，仅把 slot C 的 residual/x 完成通知拆成两个 `MTE2_V` 事件；下一批首 tile 等待第二次 DMA 对应的 x 事件，旧 residual 事件在后续可复用点或函数尾部消费。最后 A/B 参数搬运与下一批 Pass1 的消费方式保持不变。V010 的输出双槽、V011 的参数三槽、V012 的首参数跨阶段预取保持不变。
- 新时序：当前批 slot C 最后参数 tile 完成 -> `prel2` 释放 -> residual 写入并发出 residual-ready -> x 写入并发出 x-ready -> 最后 A/B 参数 tile -> 下一批首 tile 等待 x-ready 后从 C 消费。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`；Compile、Correctness、Local 均使用 NPU1。运行前后 HBM Usage Rate 均为 5%，AICore Usage Rate 均为 0%。精确收尾查询无 `paired_runner_v016` 残留；收尾快照中的 `npu-smi info -i 1` 缺少 type 参数并返回帮助文本，`common/usages` 查询成功，原始输出已保留。
- 入口：复用既有 `paired_runner_v016`、`paired_runner.cpp`、`paired_runner_abi.h`、wrapper 和 CMake；远端独立目录为 `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V017/`。Runner 输出中的 `binary=V016` 是既有入口标签，实际 Candidate 源码为本目录 V017 `Candidate.asc`。
- Compile：首次构建 `BUILD_RC=2`，原因是独立 V017 staging 缺少父版本 `submission.asc`；沿用 V011 已有父入口补入后重编 `BUILD_RC=0`，目标 `[100%] Built target paired_runner_v016`。Configure `CONFIGURE_RC=0`。
- Compile 原始记录：[server3-configure-v017-20261017.log](server3-configure-v017-20261017.log)、[server3-compile-v017-20261017.log](server3-compile-v017-20261017.log)、[server3-compile-v017-rerun-20261017.log](server3-compile-v017-rerun-20261017.log)。
- Correctness：四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`；命令为 `build-v017/paired_runner_v016 --correctness --device 1`。原始记录：[server3-v017-correctness-device1-20261017.txt](server3-v017-correctness-device1-20261017.txt)。
- Local：四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`，`PAIRED_RC=0`；命令为 `build-v017/paired_runner_v016 --paired --device 1`。完整原始样本：[server3-v017-paired-device1-20261017.txt](server3-v017-paired-device1-20261017.txt)。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 17.920 | 16.480 | -1.040 | -10.300–+0.760 |
| rows=40, D=32768 / FP16 | 39.120 | 31.060 | -8.120 | -14.140–-7.220 |
| rows=40, D=12288 / BF16 | 21.780 | 22.000 | -1.100 | -19.140–+7.700 |
| rows=40, D=32768 / BF16 | 58.280 | 48.560 | -9.660 | -29.040–+13.300 |

- FP16 D32768 的 P10–P90 全为负，目标形状稳定改善；BF16 D32768 和两个 D12288 区间跨零。按当前路线约定保留 `LOCAL_SCORE=NONE`。
- Correctness 运行前快照：[server3-before-v017-correctness-device1-20261017.txt](server3-before-v017-correctness-device1-20261017.txt)；运行后快照：[server3-after-v017-correctness-device1-20261017.txt](server3-after-v017-correctness-device1-20261017.txt)。
- Local 运行前快照：[server3-before-v017-paired-device1-20261017.txt](server3-before-v017-paired-device1-20261017.txt)；运行后快照：[server3-after-v017-paired-device1-20261017.txt](server3-after-v017-paired-device1-20261017.txt)；收尾快照：[server3-final-v017-device1-20261017.txt](server3-final-v017-device1-20261017.txt)。
- Official：`Pass 15/15`；score `41.19`；Submission ID `6ad2a20a193e507d8ee4e83c`。
- Official 回执：`线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V017/result.json`；提交日志：`线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V017/submit.log`；源码元数据：`线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V017/source-meta.json`。
- source identity：`LOCAL_SHA_EQ_REMOTE_SHA`。
- Official 比较：V003=`41.79`；Champion=`45.16`；V017 Official=`41.19`。
- 设备范围：沿用 NPU1/3/7 等安全入口，本轮使用 NPU1；不使用会 Reset NPU4 的 Runner。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_PASS
LOCAL_SCORE: NONE
OFFICIAL_SCORE: 41.19
