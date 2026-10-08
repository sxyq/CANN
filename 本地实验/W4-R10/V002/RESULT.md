# W4-R10 V002 结果

## 当前需求与状态

本次单因素闭环已完成。V002 的 BF16 多完整驻留批 blockCount 策略编译、reference
验证和本卡计时均已执行；性能结论为 `MEASUREMENT_BLOCKED`，未确认提升。
`CURRENT_LOCAL_BEST=R31B V011`。本结果只交还当前任务，不改变 Route 生命周期。

```text
AGENT_ID=01a119de-51ab-7810-9515-c9c388f70ac3
ROUTE=W4-R10 ACTIVE-CORE-D-AWARE-X
SLOT=5
REVISION=V002
DIRECT_PARENT=R31B V011
BRANCH=w4/r10-active-core-d-aware-x
HEAD_AT_START=5b7b92150136513c30eb4af47c6d3d395377d94a
NEW_PERFORMANCE_REVISIONS=1
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_IMPROVEMENT=0
STAGNATION_3=NO
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
```

旧 V001 为 2026-10-07 结果，本轮不计数、不重跑、不作为 Parent。
来源研究事件为 `R10-RESEARCH-MULTIBATCH-CAP-20261008`，来源提交 `5b7b9215`；
本版是该想法 B 的首次真实实现。规则来源及重复范围见 `REVISION-DECLARATION.md`。

## 本轮实际完成

唯一性能变化为：BF16、D>8192、R>1、Q>R、Q%R=0、M%Q=0 时将 blockCount 设为 M/Q。
R 镜像 Parent 的驻留行数，Q 为 Parent 每核最大行数。与 Parent 的差异只有 host
helper 和选择语句，共 24 行；device 函数、owner 公式、tile、DMA、算术、流水均未改。

server3 编译在 2026-10-08 05:04:48–05:05:28 UTC 完成，返回 0。随后使用同一份
runner 和两份动态库完成全部 reference 与计时，期间未重新构建或更换 Candidate。
本地与实际传输到远端的 Candidate 字节比较一致；Parent 引用既有 R31B V011 源码。

最小 task-time 采集只做两次：目标 05:07:59–05:08:11 UTC、BF16 对照
05:08:12–05:08:26 UTC。每次先 P/P，再 P/C，未按结果方向删除样本或重复采集。

## 修改或操作对象

本地新增内容全部位于：

```text
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R10-active-core-d-aware-x/本地实验/W4-R10/V002/
```

- `submission.asc`：单一 blockCount 变化。
- `CMakeLists.txt`、`support/parent_entry.asc`、`support/candidate_entry.asc`、`support/probe.cpp`、`remote.sh`：复用 R11 构建方式及 R12 配对计时方法，加入 R10 的固定输入与 reference。
- `support/analyze.py`：全样本统计、逐调用 task/event 对应、次序和分组分析。
- `REVISION-DECLARATION.md`、本文件及 `results/`：声明、原始输出、日志、完整 profiler 导出、统计和失败诊断。

远端唯一新增目录：

```text
/home/data4t2/lelinfeng/cann/w4/R10-active-core-d-aware-x/V002/
```

其中 `build/` 为本版实际产物；`results/profile-*-capture/` 保留完整原始采集。
已有 V001 和研究证据原样保留。未改主工作树、共享 TSV、规则、Dashboard 或其他 Route。
没有新分支、工作树、子代理、线程、后台调度、正式提交或外部 push；未删除任何文件。

## 验证结果

### 独立 reference

设备 4 实测 availableCoreNum=40。8 个输入的 Parent/Candidate 均逐元素通过，
非有限输出数量均为 0；输出预填 NaN 后实际 launch、同步并取回。
FP32/BF16 使用 CPU FP64 参考，FP16 在参考内沿用原 runner 的逐步 half 舍入语义。
容差及完整公式见声明与 `support/probe.cpp`。下表最大误差的两侧数值相同。

| 输入 | guard | Parent/Candidate 最大绝对误差 | 结论 |
|---|---|---:|---|
| 128x12288 BF16 | ON | 0.007812081001827842 | PASS |
| 48x12288 BF16 | OFF，Q=R | 0.007812028585504116 | PASS |
| 127x12288 BF16 | OFF，M%Q!=0 | 0.007812081001827842 | PASS |
| 120x12288 BF16 | OFF，Q%R!=0 | 0.007812081001827842 | PASS |
| 128x8192 BF16 | OFF，D=8192 | 0.007812379349095622 | PASS |
| 1x32768 BF16 | OFF，R=1 | 0.0078090248221673875 | PASS |
| 12x8192 FP32 | OFF | 0.0000005457309528722476 | PASS |
| 2x12288 FP16 | OFF | 0 | PASS |

证据为 `results/correctness.log`、8 份 `correctness-*.reference.tsv` 和 16 份
`correctness-*.parent.bin` / `correctness-*.candidate.bin`。每行 TSV 保留元素数、
失败数、最大误差和对应位置；二进制保留全部实际输出。两侧输出逐字节一致，
该事实作为补充，reference 通过结论来自独立公式逐元素比较。
本版没有解决或重验 V001 记录的宽 FP32、较大 FP16 输入问题，不声称全域正确。

### Parent P/P 与交错 P/C

每个输入为单进程、同输入与同输出地址。Parent 预热 60 次后 P/P 两组各 31 对，
先落盘；Parent/Candidate 各预热 60 次后 P/C 两组各 31 对。次序逐对交替，
每个 event 内只有一次 launch，raw 在每阶段结束后统一写出，无采样内分配或写盘。
每个输入 P/P 124 条、P/C 124 条；两输入合计 496 条计时样本，全部保留。

| 输入 / 计时范围 | P/P 中位数 us | MAD/中位数 | 两组中位数 us | 组间相对差 | 中心指标结论 |
|---|---:|---:|---|---:|---|
| 128x12288 BF16 kernel task | 32.17 | 13.2111% | 33.99 / 30.33 | 11.3771% | MEASUREMENT_BLOCKED |
| 128x12288 BF16 event | 49.110001 | 37.2226% | 60.520001 / 38.140001 | 45.5712% | MEASUREMENT_BLOCKED |
| 48x12288 BF16 kernel task | 13.95 | 2.6523% | 14.172 / 13.84 | 2.3799% | 两项中心指标通过 |
| 48x12288 BF16 event | 24.759999 | 42.3667% | 57.530001 / 16.800000 | 164.4992% | MEASUREMENT_BLOCKED |

对照的 kernel-task P/P 最大值为 2205.544 us，P/C 的 Parent 最大值为 10012.24 us；
全部长尾保留。两项中心指标通过不等于整次采集没有波动。

以下 P/C 数字全部为观察值。Local score 定义为逐对 `(C/P-1)*100` 的中位数；
正数为更慢。总体中位比单列，二者不混用。

| 输入 / 范围 | Parent 中位数 us | Candidate 中位数 us | 配对 delta % | 总体中位比 delta % |
|---|---:|---:|---:|---:|
| 128x12288 BF16 kernel task | 34.25 | 33.73 | -3.7819107924 | -1.5182481752 |
| 128x12288 BF16 event | 54.970000 | 50.969999 | +5.5463434408 | -7.2766975804 |
| 48x12288 BF16 kernel task | 14.05 | 13.98 | +0.0590618192 | -0.4982206406 |
| 48x12288 BF16 event | 19.950001 | 19.090001 | -0.5441367493 | -4.3107769672 |

目标 kernel-task P/C 的 MAD/中位数为 Parent 11.4978%、Candidate 17.0886%。
P-C 次序的 C-P 差中位数为 +1.620 us，C-P 次序为 -1.564 us；
两组差中位数为 -1.564 / +0.560 us，方向随次序与组变化。
目标 P/P 逐对绝对差 p90 为 29.066 us；当前数据不能支持该小幅差异为提升。

```text
COMPILE=PASS
CORRECTNESS=PASS
CORRECTNESS_SCOPE=8_LOCAL_INPUTS_VS_INDEPENDENT_REFERENCE
LOCAL_SCORE=-3.7819107924181745
LOCAL_DELTA=-3.7819107924181745%
LOCAL_SCORE_KIND=OBSERVATION_ONLY_PAIRED_KERNEL_TASK_DELTA
STATUS=MEASUREMENT_BLOCKED
CURRENT_LOCAL_BEST=R31B V011
NEW_PERFORMANCE_REVISIONS=1
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_IMPROVEMENT=0
RESOURCE_BLOCKER=NONE
```

### task/event 与资源

两次 `op_summary` 各 428 个 kernel 调用，均为 device 4 的 AI_VECTOR_CORE。
目标的计时 Parent 为 40 blocks、Candidate 为 32 blocks；对照两侧均为 40 blocks。
所有 856 个 kernel 与 task_time 的 device/stream/task ID、开始时间、时长逐条一致；
496 个计时调用均位于对应 stream 的前后 EVENT_RECORD 之间。
每次 180 个预热调用也保留在原始 CSV，不混入计时统计。

event 与对应 kernel-task 的差中位数为目标 7.340000787 us、对照 3.020000306 us。
ACL elapsed 与两个 EVENT_RECORD 开始时间差的最大偏差分别为 0.024006957 us、
0.180251160 us；实际差值逐条保留，没有由该间隔指定某个未经证实的单一原因。

完整样本与逐调用对应：`results/profile-target.pp.tsv`、`profile-target.pc.tsv`、
`profile-control.pp.tsv`、`profile-control.pc.tsv`、`profile-target.task-map.tsv`、
`profile-control.task-map.tsv`。全部统计见 `results/local-summary.json`。
原始 op_summary、task_time、timeline 和辅助导出位于相邻两份 `profile-*-capture/`。

server3 为 hwnput3，用户 lelinfeng，连接入口 cann-server3；CANN 8.5.0.alpha002、
Ascend910B3/dav-2201。各阶段 npu-smi 均给出 HBM 容量 65536 MB、使用率 90%，
按项目口径 FREE_HBM=6553 MB。目标采样前 ACL 空闲 HBM 为 6274.753906 MiB；
对照为 6280.195312 MiB，结束为 6279.640625 MiB。两种口径分开保留。
Local 前后 AICore 为 61–62%、AIVector 为 33–41%，host load1 为 50.85–55.39。
其他 VLLM 等任务持续运行，未被停止、修改或等待其退出。

首次离线统计在传输结束前启动，因 raw 文件尚未到达返回 1；
`results/analysis-transfer-order.log` 保留失败事实。原传输随后返回 0，
对同一批完整数据的统计返回 0，没有重跑 NPU、重建或修改 Candidate。
支持脚本语法、生成文件与实际原始数据对应均已核对。

## 剩余工作与风险

当前没有第二个已证实独立且可直接实施的性能轴。V002 的正确性及实际缩核已确认，
收益仍需可靠的本路线时间支持；不从本次负载或不稳定数据宣布 Route 无效。
完整批条件之外的核数取整只保留为未研究范围，不能直接登记成非重复新想法。

精确下一动作：若再次安排 R10，先复用本次 `profile-target.task-map.tsv` 和相邻
task_time / runtime timeline，按调用开始时间、P/P 与 P/C 次序定位长尾及方向反转，
据此提出有区别的有限测量设计。kernel task 自身也有波动，单纯去掉 event 间隔
不足以证明可测性。不无变化重采本次两组，不先建立 V003，不叠加未接受的 V002。

所有构建、reference、采样、profiler 导出和传输命令均已结束。
`results/final-state.log` 于 05:14:43 UTC 确认 `R10_ACTIVE_PROCESSES=[]`，
runner 与两份动态库时间仍为本次 05:05 构建时刻。
`RUNNING_DEVICE_OPERATION=NONE`。最终提交号与 Git 状态由提交后回执提供。
