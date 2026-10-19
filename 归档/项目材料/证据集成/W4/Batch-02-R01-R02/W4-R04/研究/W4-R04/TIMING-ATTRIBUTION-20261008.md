# W4-R04 V002 同版计时范围归因

## 结论

本次同版研究完成；没有修改 Candidate，也没有建立 V003。一次有限采集得到 768 条 P/P 与 P/C 单调用样本。Parent/Candidate 在三项实际输入上，于采集前后分别通过独立 CPU FP64 reference，输出逐位相同。768 条计时调用全部对应到 profiler kernel task、起止设备 event 和 host launch。

task 与 event 的时间尺度和离散度明显不同。P/P task 的 MAD/median 为 16.3%–28.4%；P/C 两侧为 17.6%–36.9%。分层结果有方向分歧，个别块出现长 task 样本。此次为单进程、四个小组的一次采集，无法支持 V002 的稳定性能收益或回退结论。因此沿用 V002 原有 `MEASUREMENT_BLOCKED`，`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`CURRENT_LOCAL_BEST=NONE`；本文数字仅作归因观察，不构成 Local 成绩。

本次有限结果说明，逐对交替 P/C 会把第一/第二位置与前序库切换绑定；task 时间应显式按前序库和计时位置分层。它没有识别 event/task 差的单一成因，也没有证明某个 P/C 分层的数值可复现。

## DUPLICATE_AUDIT

```text
MECHANISM=同进程计时研究，分开记录前序库、当前库、计时位置、device event、kernel task 与 host 范围
SEARCHED_HISTORY=R04 V002 原 runner/raw；w3/m1/adaptive-core-ownership@6321ad4 至 V040；w3/m1/multirow-panel-rms@ce6c6dc 至 V031；w3/m1/crossrow-full-pipeline@1efa086 至 V028；R31/R31A/R31B；MIX；STORE/EPILOGUE；已提交 W4 R01、R07、R08 的 task/event 研究
MATCH_FOUND=部分相邻：R01 有逐项 task/event/host 对应但未运行本 Candidate；R07 有 PP/PC 对照但没有本次的块内平衡；R08 有同输出地址 Parent 双槽漂移
WHY_NEW_OR_DUPLICATE=复用 V002 二进制，仅补一次共用输出地址的三输入 P/P 与 P/C 归因采集，不新增性能版本
```

R07 已研究 PP/PC 的 task/event 范围，但其分块序列不同。本次 P/C 每个小循环固定为 `PPPCCPCC`，每块四轮，循环前同步运行 Candidate、末尾也为 Candidate。八种“前序库→当前库×计时位置”组合在每个 block-cycle 各出现一次，四个 block 下每个输入每格共 16 条。各块内 P/P 和 P/C 的先后顺序交错。这让顺序格子平衡，但不消除时间推移或设备负载变化。

## 采集前声明与方法

日期为 2026-10-08。沿用 Direct Parent `R31B-V011` 的 Parent 与 V002 Candidate 动态库，无 kernel 源码修改或 Ascend kernel 重编译。设备为 server3 `hwnput3` device1，`available_vector_cores=40`；CANN 8.5.0.alpha002，Ascend910B3 / DAV-2201。

三项输入沿用 V002：16×2048 FP32、16×2056 FP32（guard-ON），12×8192 FP32（guard-OFF GenericRow 对照）。第三项不代表相同指令路径。每项每库预热 45 次；P/P 128 次、P/C 128 次，共 768 个计时样本。输入、库、core 参数、stream、输入地址及单一输出地址在每项全过程固定。

每个计时调用沿用 `RecordEvent(start) → 原 wrapper 单次 launch → RecordEvent(stop) → SynchronizeEvent → ElapsedTime`。raw 同时记录 wall 时间、block、mode、cycle、position、前序库与当前库。P/P 使用同一个 Parent 函数入口。没有按结果方向停采、去首样本、删大值或补采。

主观察量是 profiler task 时间的中位数、MAD/median 和每块中位数；另列同组配对 `(C-P)` 与百分比中位数。event、wall、host API 各自单独保留，不互相代替或相加解释。长时间戳以 Decimal 精确计算。

计划样本数固定为 768。如 reference 失败、raw 数不符、call/task/event/host 对应失败或格子覆盖不全，则保留结果并只报告未完成与 `MEASUREMENT_BLOCKED`。即使全数完成，本次也只用于归因，不单凭一轮结果认定 Local 改善。

## 正确性与逐项对应

reference 为 CPU FP64 计算后转 FP32。逐元素条件为 `1e-5 + 1e-4*abs(reference)`，并要求 max abs error ≤ `1e-2`；另核对 Parent/Candidate 输出逐位一致。

| 输入 | reference 前测 Parent/Candidate max abs | reference 后测 Parent/Candidate max abs | P/C 位差 |
|---|---:|---:|---:|
| 16×2048 FP32 | 7.1525574e-7 / 7.1525574e-7 | 7.1525574e-7 / 7.1525574e-7 | 0 |
| 16×2056 FP32 | 7.1525574e-7 / 7.1525574e-7 | 7.1525574e-7 / 7.1525574e-7 | 0 |
| 12×8192 FP32 | 4.7683716e-7 / 4.7683716e-7 | 4.7683716e-7 / 4.7683716e-7 | 0 |

768 个计时调用全部完成 host event connection ID → 设备 `EVENT_RECORD` task → 起止时间戳 → 同区间 `Node@launch` → kernel task 的逐条对应。task_time 共 4298 行（含 event task），op_summary 有 1074 个 kernel；计时调用 768/768 对应成功。`task_time` 与 `op_summary` 的 kernel 起止和时长逐条相同。event elapsed 与起止 event task 时间差的最大绝对差为 0.032011044 µs。

## 观察数字

以下为本次 task 中位数，单位 µs。P/P 每库 128 条，P/C 每库 64 条。

| 输入 | P/P Parent | P/C Parent | P/C Candidate | P/P MAD/median | P/C Parent MAD/median | P/C Candidate MAD/median |
|---|---:|---:|---:|---:|---:|---:|
| 16×2048 | 6.17 | 7.17 | 5.74 | 27.88% | 31.24% | 20.91% |
| 16×2056 | 5.67 | 6.64 | 6.65 | 28.40% | 36.90% | 27.52% |
| 12×8192 | 8.67 | 8.91 | 8.79 | 16.26% | 23.23% | 17.63% |

按前序库和位置分层，`C/P-1` 的中位数比变化及同 block-cycle 配对百分比中位数如下：

| 输入 | 前序库 / 位置 | P 中位 | C 中位 | 中位比变化 | 配对变化中位 |
|---|---|---:|---:|---:|---:|
| 16×2048 | C / 1 | 7.86 | 8.57 | +9.03% | −1.22% |
| 16×2048 | C / 2 | 7.29 | 5.06 | −30.59% | −35.24% |
| 16×2048 | P / 1 | 6.53 | 5.66 | −13.32% | −9.58% |
| 16×2048 | P / 2 | 6.83 | 5.65 | −17.28% | −8.87% |
| 16×2056 | C / 1 | 6.33 | 6.66 | +5.21% | −17.81% |
| 16×2056 | C / 2 | 6.23 | 5.95 | −4.49% | −6.23% |
| 16×2056 | P / 1 | 8.45 | 5.73 | −32.19% | −26.28% |
| 16×2056 | P / 2 | 26.41 | 15.44 | −41.54% | +8.25% |
| 12×8192 | C / 1 | 9.49 | 7.91 | −16.65% | −16.62% |
| 12×8192 | C / 2 | 17.36 | 9.24 | −46.77% | −55.47% |
| 12×8192 | P / 1 | 10.03 | 7.74 | −22.83% | −22.50% |
| 12×8192 | P / 2 | 7.89 | 9.742 | +23.47% | +15.76% |

方向在各格间不一致。16×2048 的整体 P/C task 中位比变化为 −19.94%，但格内从 +9.03% 到 −30.59%；16×2056 整体中位比变化为 +0.15%，分层仍跨正负。全部 raw、尾样本、event 与 wall 统计、host API 分布和每格的完整描述统计均保存在相邻 `timing-summary.json` 与 `task-event-host-map.tsv`。

## 设备背景与范围限制

采集前 device1 HBM 容量 65536 MB、使用率 36%，估算 FREE_HBM=41943 MB；AICore 18%、AIVector 17%。采集后 HBM 使用率仍为 36%，AICore 2%、AIVector 7%。host load1 前后为 48.93/56.49/60.65 与 43.32/52.12/57.71。另有两个 Python 进程分别占约 5748 MB 与 14957 MB，均未被改动。资源快照是离散值，不表示全过程负载恒定。

此次是单进程和单次 profiler 窗口；没有同次对照 profiler 开/关时的差异。task 的长尾可能来自多种运行时与设备条件，本证据不作唯一归因。

## 文件、命令状态与事件

server3 仅在 R04/V002 目录增加 `support/timing_probe.cpp`、`support/run_timing_study.sh`、独立 host `build-server3-v002/r04_timing_host` 和本次 `results/timing-attribution-20261008/`。既有 Parent/Candidate 库、kernel source 和旧 runner 未覆盖。GCC host build 与动态依赖解析通过。采集于 2026-10-08 11:56:02–11:56:13 UTC，msprof task-time、ascendcl、runtime-api 开启，ai-core/aicpu 指标关闭，返回码 0。所有本路线运行命令已结束，runner 进程不存在。

本地证据在 `本地实验/W4-R04/V002/logs/timing-attribution-20261008T1155Z/`：raw、reference 前后结果、完整 profiler 导出与数据库、task/event/host 映射、统计摘要和资源快照均保留。两次未启动设备采集的派发日志也单独留存。

本事件为 V002 同版研究补充：新增性能版本 0、有效 Local 0、连续无改善次数贡献 0；Local Best 仍为 NONE；Official=NONE、Online=PAUSED、PUSH=NO。V002 的原始 528 条 Local 样本与 `RESULT.md` 未修改。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R04-V002-TIMING-ATTRIBUTION-20261008
EVENT_KIND=TIMING_ATTRIBUTION_SUPPLEMENT
ROUTE=W4-R04
REVISION=V002
DEVICE_ID=1
DIRECT_PARENT=R31B-V011
COMPILE=HOST_COMPILE_PASS; EXISTING_KERNEL_LIBRARIES_REUSED
CORRECTNESS=PASS_CPU_FP64_REFERENCE_BEFORE_AND_AFTER; 3_INPUTS; P_C_BITWISE_EQUAL
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
MEASUREMENT_STATUS=MEASUREMENT_BLOCKED
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
STAGNATION_INCREMENT=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
EVIDENCE=研究/W4-R04/TIMING-ATTRIBUTION-20261008.md; 本地实验/W4-R04/V002/logs/timing-attribution-20261008T1155Z/
ROUTE_LIFECYCLE_CHANGED=NO
```

## 下一步

不按同一协议追加样本。已确认旧逐对 P-C/C-P 把计时位置与前序库切换关联；后续性能判断应报告 task 的前序与位置分层。仍待确认 profiler 与非 profiler 时长关系、分层方向能否在其他窗口复现。若之后由 Main/Planning 安排，可另行设计独立的非 profiler 对照；本次不启动后续采集。
