# N04 V018 结果

## 假设与单一变化

- 父版本：V017，源码 `本地实验/R31B/V016/V017/Candidate.asc`，SHA256 `1502a3fdda99e3aa54d9dfcf4a8598b855c42338a56b81fb1cbe596d388ea893`。
- Candidate：本目录 `Candidate.asc`，SHA256 `487e8302a19db9ec2e055342d395c96708ef9db288deca3f0e073674535624a8`。
- 单一变化：保持 V017 的 slot C 触发点、分离的 x/residual 完成事件、下一批首 tile 消费方式和最后 A/B 参数搬运；仅把 slot C 的两次 DMA 提交顺序改为 x-first、residual-second。下一批首 tile 等待第二次 DMA 的 residual-ready 事件。
- 目的：比较 x-first 与 V017 residual-first 对下一批首 tile 消费及尾部 A/B 搬运重叠的影响。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`；远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V018/`。
- 设备：Compile 不占用 NPU；Correctness/Local 使用 NPU1。运行前后 HBM Usage Rate 均为 5%，AICore Usage Rate 均为 0%。未使用 NPU4，也未调用会 Reset NPU4 的 Runner。
- 入口：复用 V017/V016 的 `paired_runner_v016`、`paired_runner.cpp`、`paired_runner_abi.h`、CMake 和既有 `--correctness` / `--paired --device 1` 入口。Runner 输出中的 `binary=V011`、`binary=V016` 是既有标签；本轮实际 Parent 为 V017，Candidate 为 V018。
- Staging 与源码身份：[server3-staging-v018-20261017.txt](server3-staging-v018-20261017.txt)。

### Compile

- Configure：`CONFIGURE_RC=0`；记录：[server3-configure-v018-20261017.log](server3-configure-v018-20261017.log)。
- 首次 Build：`BUILD_RC=2`，原因是 Ascend 注册编译步骤缺少 `CPLUS_INCLUDE_PATH`，原始失败记录保留：[server3-compile-v018-20261017.log](server3-compile-v018-20261017.log)。
- 沿用既有工具链环境补充 include 路径后重编：`BUILD_RC=0`，目标 `[100%] Built target paired_runner_v016`；记录：[server3-compile-v018-rerun-20261017.log](server3-compile-v018-rerun-20261017.log)。

### Correctness

- 命令：`build-v018/paired_runner_v016 --correctness --device 1`。
- 四组 Parent/Candidate 均 `PASS`，`failures=0`，`CORRECTNESS_RC=0`。
- 原始输出：[server3-v018-correctness-device1-20261017.txt](server3-v018-correctness-device1-20261017.txt)；运行前快照：[server3-before-v018-correctness-device1-20261017.txt](server3-before-v018-correctness-device1-20261017.txt)；运行后快照：[server3-after-v018-correctness-device1-20261017.txt](server3-after-v018-correctness-device1-20261017.txt)。

### Local

- 命令：`build-v018/paired_runner_v016 --paired --device 1`；四组各 45 warmup、21 对；计时为 `DEVICE_EVENT_PRIMARY`，wall-clock 仅作诊断；`PAIRED_RC=0`。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 17.000 | 16.440 | -0.840 | -9.220–+8.700 |
| rows=40, D=32768 / FP16 | 31.840 | 31.860 | -0.100 | -1.660–+1.840 |
| rows=40, D=12288 / BF16 | 20.260 | 23.440 | +0.220 | -7.460–+13.480 |
| rows=40, D=32768 / BF16 | 48.320 | 48.480 | +0.140 | -20.340–+1.900 |

- 原始样本：[server3-v018-paired-device1-20261017.txt](server3-v018-paired-device1-20261017.txt)；运行前快照：[server3-before-v018-paired-device1-20261017.txt](server3-before-v018-paired-device1-20261017.txt)；运行后快照：[server3-after-v018-paired-device1-20261017.txt](server3-after-v018-paired-device1-20261017.txt)。
- 四组 Delta P10–P90 均跨零或接近零，按当前路线记录保留 `LOCAL_SCORE=NONE`。

## 线上状态

- Official：`Pass 15/15`；SID=`6ad2a90d193e507d8ee9a04f`；Official score=`41.40`。
- Official 回执：[result.json](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V018/result.json)；提交日志：[submit.log](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V018/submit.log)；源码元数据：[source-meta.json](线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V018/source-meta.json)。
- source identity：`LOCAL_SHA_EQ_REMOTE_SHA`。
- Official 比较：V003=`41.79`；Champion=`45.16`；V018 Official=`41.40`。
- 共享 TSV、V009/V010 未提交 `RESULT.md` 和其他版本未修改。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_PASS
LOCAL_SCORE: NONE
OFFICIAL_SCORE: 41.40
