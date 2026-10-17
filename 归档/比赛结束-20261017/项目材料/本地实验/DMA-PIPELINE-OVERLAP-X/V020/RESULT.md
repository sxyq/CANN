# N04 V020 结果

## 假设与单一变化

- 有效父版本：V018，源码为本路线 `本地实验/R31B/V016/V018/Candidate.asc`。V019 只有两次 Correctness `RC=143` 失败证据，不作为父版本。
- Candidate：本目录 `Candidate.asc`。
- 单一机制：保持 V018 的 x-first/residual-second 输入 DMA、双完成事件、slot C 触发、下一批首 tile 消费和 A/B 参数搬运；仅在宽行 FP16 双输出槽路径中，把共享的 `V_MTE3` ready event 拆成 output slot A/B 各自的 ready event。没有增加 buffer 或 queue depth，也没有移动输入 DMA 到 V019 的 `SyncVToMTE2()` 与 Store 之间。
- 研究依据：现有 N04 记录显示继续增加 buffer/depth 或只增加 MTE3 stage 已有平坦结果；per-slot store event 是未验证的独立同步原语方向。

## 服务器事务

- 主机：`hwnput3`；登录入口：`cann-server3`；远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V020/`。
- 设备：Correctness/Local 使用 NPU1；Compile 不占用 NPU。未使用 NPU4，也未调用会 Reset NPU4 的 Runner。
- 入口：复用 V018 的 `paired_runner_v016`、`paired_runner.cpp`、ABI、wrapper、CMake 和 `--correctness` / `--paired --device 1` 入口。Runner 输出中的 `binary=V011`、`binary=V016` 是既有标签；实际 Parent 为 V018，Candidate 为 V020。
- Staging：[server3-staging-v020-20261017.txt](server3-staging-v020-20261017.txt)。

## Compile

- Configure：`CONFIGURE_RC=0`；记录：[server3-configure-v020-20261017.log](server3-configure-v020-20261017.log)。
- Build：`BUILD_RC=0`，目标为 `[100%] Built target paired_runner_v016`；记录：[server3-compile-v020-20261017.log](server3-compile-v020-20261017.log)。

## Correctness

- 命令：`build-v020/paired_runner_v016 --correctness --device 1`。
- 四组 Parent/Candidate 均 `PASS`，`failures=0`，`CORRECTNESS_RC=0`；原始输出：[server3-v020-correctness-device1-20261017.txt](server3-v020-correctness-device1-20261017.txt)。
- 运行前快照：[server3-before-v020-correctness-device1-20261017.txt](server3-before-v020-correctness-device1-20261017.txt)；运行后快照：[server3-after-v020-correctness-device1-20261017.txt](server3-after-v020-correctness-device1-20261017.txt)。前后 HBM Usage Rate 均为 5%，Aicore/Aivector Usage Rate 均为 0%。

## Local

- 命令：`build-v020/paired_runner_v016 --paired --device 1`；四组各 45 warmup、21 对；计时指标为 `DEVICE_EVENT_PRIMARY`，wall-clock 仅作诊断。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10-P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 21.280 | 20.660 | +0.200 | -10.140–+11.340 |
| rows=40, D=32768 / FP16 | 31.420 | 32.020 | +0.180 | -3.620–+2.560 |
| rows=40, D=12288 / BF16 | 30.600 | 25.820 | +0.000 | -9.220–+1.260 |
| rows=40, D=32768 / BF16 | 59.840 | 59.380 | -0.360 | -10.560–+0.920 |

- Local 原始样本：[server3-v020-paired-device1-20261017.txt](server3-v020-paired-device1-20261017.txt)。运行前快照：[server3-before-v020-paired-device1-20261017.txt](server3-before-v020-paired-device1-20261017.txt)；运行后快照：[server3-after-v020-paired-device1-20261017.txt](server3-after-v020-paired-device1-20261017.txt)。`PAIRED_RC=0`。
- 四组 Delta P10-P90 均跨零，按路线记录保留 `LOCAL_SCORE=NONE`。BF16 D12288 的 wall-clock 有单个诊断异常值，但 device-event 主指标和四组 Delta 事实不受其替代。

## Official

- Official：`Pass 15/15`；SID=`6ad2b167193e507d8eefbbb7`；Official score=`41.21`。
- Official 回执：[result.json](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V020/result.json)；提交日志：[submit.log](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V020/submit.log)；源码元数据：[source-meta.json](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V020/source-meta.json)。
- source identity：`LOCAL_SHA_EQ_REMOTE_SHA`。
- Official 比较：V003=`41.79`；Champion=`45.16`；V018 Official=`41.40`；V020 Official=`41.21`。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_PASS
LOCAL_SCORE: NONE
OFFICIAL_SCORE: 41.21

## 下一步

- V020 的 per-slot `V_MTE3` ready event 具备 Correctness 通过，但四组 Local 区间均跨零；下一轮应优先转向已记录的 inter-pass prologue issue-order 方向，或结束本条 MTE3 ready event 假设，不进行线上提交。
