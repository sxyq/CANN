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

## 2026-10-08：同版计时归因与交叉诊断

### 当前需求与状态

本次 V002 计时归因任务完成，接受结果仍为 `MEASUREMENT_BLOCKED`：
`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、R10 已接受 Local Best=NONE，
上游 `CURRENT_LOCAL_BEST=R31B V011`。没有改 kernel、重建两份 kernel 库或创建 V003。
新增性能版 0、有效 Local 0、连续无改善贡献 0；不是 STAGNATION_3。
原 V002 的 34.25/33.73 us、配对 -3.7819107924% 继续保留为旧观察值。

### 本轮实际完成

先对旧 496 个计时调用做 connection_id、API 区间和前序调用分析；随后只执行一次
有区别的交叉诊断，每个原有 BF16 输入启动一个进程，进程内先 P/P 再 P/C。
两个阶段保持同输入、双库驻留、两个输出 buffer、stream、event 和同步节奏。
每个观测前指定 Parent/Candidate 前序调用，独立改变其同/异输出 buffer、
逻辑槽位、buffer 对应与调用先后。32 条件、两组，实际安排与全部前序调用均保留。
完整采集前声明在 `研究/W4-R10/TIMING-ATTRIBUTION-20261008.md`。

旧 trace 已证明三项具体限制：

- 两份设备二进制分别在 launch 1 和 186 前注册；旧 P/P 为 61–184，尚未包含
  Candidate 设备注册后的运行状态。
- P/C 第一位置有 61/62 次紧接同一 kernel，第二位置 0/62 次紧接同一 kernel。
  原调用位置与前序 kernel 混杂；原 P/P 的两位置则都只调用 Parent。
- target 的 248 个 timed call 中有 46 个设备开始早于对应 host Node@launch，
  control 有 25 个；对应 Runtime@KernelLaunch 为 49/32。连接关系逐条一致，
  跨时钟绝对时刻不能直接用作因果先后或真实提交等待时长。

旧两输入共 496/496 个 timed call 的两次 host RecordEvent 之间均有
Runtime@DevMalloc 和 AscendCL@aclrtFree。原外围分配在计时外的描述，只适用于
runner 明写的五个输入/输出 buffer，不涵盖生成式 launch 包装内部的调用。
旧 task 1169 的 10012.24 us 和 task 180 的 2205.544 us 主要处在 kernel-task
区间内，因此仅移除 event 外围区间不足以取得可靠测量。

### 修改或操作对象

本轮本地修改/新增限于指定 R10 工作树内：

- `本地实验/W4-R10/V002/support/probe.cpp`：新增 diagnose 模式；抽取原 reference
  计算供新旧模式共用；输入、epsilon、容差与原八个 reference 公式保持不变。
- `support/host_diagnostic.sh`：只编译 host 的入口和一次有限采集入口。
- `support/timing_attribution.py`：旧 trace 与新 trace 逐调用对应、全部样本统计、
  独立因素对照及原八个 reference 结果复用核对。
- 本文件、前述研究声明、`results/timing-attribution-20261008/`：原始数据、
  导出、日志、统计和本次事实。原 V001/V002 数据保持原样。

远端仍只操作既有
`/home/data4t2/lelinfeng/cann/w4/R10-active-core-d-aware-x/V002/`。
原 `build/r10_probe`、`libr10_parent.so`、`libr10_candidate.so` 保留；仅新增必要的
`build/r10_probe_timing`（101928 B）。其动态依赖明确指向原两库。未使用 objcopy，
没有改写已测 ELF，也没有把局部段比较当作整个文件相同的证明。

host 构建采用原 `/usr/bin/c++ -std=gnu++17` 和原 include/link 参数，无新增优化
参数；2026-10-08 06:56:05–06:56:08 UTC 返回 0。采集命令为：

```text
bash support/host_diagnostic.sh capture
build/r10_probe_timing diagnose 4 128 12288 2 <new-results>/target
build/r10_probe_timing diagnose 4 48 12288 2 <new-results>/control
msprof: task-time=on, aic-mode=task-based, ai-core=off, ascendcl=on, runtime-api=on, aicpu=off
```

### 验证结果

两个原有 BF16 输入的 Parent/Candidate 分别在 A/B 两个实际输出 buffer 对独立
CPU FP64 reference 通过；每输入共四次真实 launch，所有 failures/nonfinite=0。
采样后再次读回 A/B，两者仍通过。target 最大绝对误差为
0.0078120810018278419，control 为 0.0078120285855041161。
原八输入、16 份双方 reference 结果继续复用；其余六个输入没有重复运行。
V001 原宽 FP32/较大 FP16 失败域没有被重验或宣布解决。

两次采集实际各 756 个 kernel：4 次 reference、240 次预热、512 个 timed call。
timed call 中，P/P 和 P/C 各 128 个观测、128 个指定前序调用。两输入全部 1024 条
计时值均保留；下表只用观测调用，前序调用没有混入 P/P 资格。
target 的 Parent 为 40 blocks，Candidate 为 32；control 双方均 40。
两份设备二进制均已在 launch 1/3 前注册，首个 timed call 为 125。

| 输入 / P/P 范围 | 整体中位 us | MAD/中位 | 两组中位 us | 组间相对差 | 结论 |
|---|---:|---:|---|---:|---|
| target kernel task | 31.500 | 17.4286% | 32.042 / 31.190 | 2.7048% | 未取得资格 |
| control kernel task | 18.462 | 10.2481% | 18.590 / 18.382 | 1.1266% | 未取得资格 |
| target event | 39.550001 | 27.3325% | 41.530000 / 38.070001 | 8.7484% | 未取得资格 |
| control event | 25.150000 | 20.0795% | 26.020000 / 24.290000 | 6.8787% | 未取得资格 |

target P1/P2 task 中位为 31.310/31.572 us，MAD 为 16.3526%/17.6422%；
control 为 18.300/18.570 us，MAD 为 10.9180%/9.3700%。此外，target 两个真实
输出 buffer A/B 的 MAD 为 18.8100%/16.2290%；control A 为 11.4116%。
没有通过选择单个槽位或单个较稳的分组来接受全次结果。

| 输入 / P/C 范围 | Parent 中位 us | Candidate 中位 us | 配对 delta % | 总体中位比 delta % |
|---|---:|---:|---:|---:|
| target kernel task | 31.930 | 28.180 | -1.4061318749 | -11.7444409646 |
| control kernel task | 17.850 | 17.860 | -0.3834903657 | +0.0560224090 |
| target event | 39.020000 | 40.200001 | -8.9564315979 | +3.0240917522 |
| control event | 24.770000 | 41.449999 | +0.2690109684 | +67.3395212174 |

这些 P/C 数字全部为观察值。target P/C 两侧 task MAD 为 19.2922%/22.5053%，
Candidate 的两组中位为 24.840/33.680 us，相对差 31.3698%。control Candidate
两组为 16.690/21.730 us，相对差 28.2195%。P/P 配对绝对差 p90 为 target
19.1872 us、control 183.2040 us，远大于 P/C 配对差中位 -0.230/-0.070 us。

独立因素对照在相同 block、同一观测标签且其余四个因素相同时逐项相减；每项
每标签 32 对，全部 1280 个对照及原始操作序号保留。下表为 target P/P task
的对照差中位，两个标签都真正调用同一 Parent：

| 改变的因素（后者减前者） | P1 us | P2 us |
|---|---:|---:|
| 前序 Parent → Candidate | +2.730 | +1.800 |
| 前序同 buffer → 另一个 buffer | -0.650 | +4.518 |
| 观测输出 A → B | -7.118 | +5.918 |
| 观测第一 → 第二位置 | +4.082 | -3.118 |
| 逻辑槽位 0 → 1 | -0.460 | -0.022 |

前序 kernel 可保留为线索，但这些对照的 p10–p90 均横跨零，P/C 中方向也随分组
变化。例如 target P/C 在 Candidate 写 A/B 时配对差中位为 +0.780/-5.650 us，
两组总体为 -1.342/+0.492 us。现有数据没有支持一个固定的地址或位置偏移作为
全部差异来源。新设计已经解开原前序/位置对应，却仍没有取得可靠 Local；不能把
该结果反推成旧采集的无干扰收益，也不能将全部波动指定给某个因素。

新 target 观测 task 1110 达 304.768 us，event 为 310.180008411 us，此时 host
分配/释放仅 3.42/4.27 us；control task 744 为 341.788 us，event 405.140012503 us。
另一方面，target task 288 的 kernel 32.460 us、event 205.259993672 us，外围区间
172.824 us，分配/释放 3.84/5.14 us。两类长尾同时存在，短 API 时长不能独自解释
它们，也不能据此排除分配可能带来的间接影响。所有样本都参与统计。

每次新采集的 756 个 op_summary 与 task_time 全部一致，512 个 timed call 的
device/stream/task、前后 EVENT_RECORD、host connection_id 与前序调用对应均通过。
ACL elapsed 与设备事件时间差最大偏差为 target 0.024006328 us、control
0.028012579 us。新 control 仍有 6 个设备开始早于 host Node、13 个早于 Runtime
launch；不把这些跨时钟绝对差作为真实队列等待。两份新 trace 都有 512/512 次
逐 launch 分配/释放，target 观测 P/P 对应 API 中位为 3.390/4.010 us。

只读原两库的 BF16 host 包装取得了额外结构证据：Parent 入口 0x4b0ac、Candidate
入口 0x4b31c；分别在 0x4b0fc/0x4b36c 调用 AllocAscendMemDevice，size 参数为
8 字节，返回指针保存在栈上偏移 72 字节处；以栈首地址和 80 字节大小传给 launch
helper，正常返回路径读取同一指针并调用 FreeAscendMemDevice。
这是原已测库的结构事实；尚未确认该附加参数在设备侧的完整用途，不擅自绕过它。

全部数据入口为 `results/timing-attribution-20261008/`：

- `target.pp.tsv`、`target.pc.tsv`、`control.pp.tsv`、`control.pc.tsv`：1024 条原始
  event/wall、槽位、地址、前序与执行序号。
- `target-task-map.tsv`、`control-task-map.tsv`：1024 条逐调用设备 task/API 对应。
- `target-pairs.tsv`、`control-pairs.tsv`：256 对观测的全部数值；
  `target-contrasts.tsv`、`control-contrasts.tsv`：1280 个独立因素对照。
- `diagnostic-summary.json`：全样本统计、各组/标签/因素数值和八输入 reference 复用。
- `old-*-attribution.tsv`、`old-attribution-summary.json`：旧 496 条原始计时的扩展归因。
- 两份 `*-capture/PROF*/mindstudio_profiler_output/`：完整 CSV、runtime timeline 和
  辅助导出；更底层采集继续保留在远端同名 PROF 目录。
- `host-compile.log`、`capture-session.log`、`transfer-*.log`、`analysis.log`、
  `wrapper-final-state.log`：构建、执行、传输、离线统计、只读对象分析与结束状态。

### 剩余工作与风险

本轮没有新增已证实且已排除历史重复的性能轴。现有 V002 的有效 Local 仍缺；
完整批之外的核数取整范围也没有新增研究结论。两个 buffer 是设备地址身份，
物理 HBM 通道/分配位置仍未知。当前采集不能分开设备内部负载、主机调度、
profiler 开销与内部申请/释放的间接影响；没有逐核计时或无 profiler 对照。

不同的精确下一研究动作：从上述两份原库的 BF16 host 包装，沿 Parent 的
0x49c24、Candidate 的 0x49cb4 launch helper 及 80 字节参数布局，追踪尾部
8 字节设备指针的接收方、生命周期和必要同步。先做只读 ABI/生成对象分析，
确认用途后再提出单一 host 测量干预；不重跑本次 32 条件矩阵、不先改 kernel
或创建 V003。该动作不要求使用设备，也不预设移除这次申请是安全的。

采集于 06:56:48–06:57:18 UTC 完成。各阶段 HBM 使用率 90%，项目公式可用
6553 MB；ACL 在 target 采样前后均为 6270.550781 MiB，control 为
6278.175781→6277.437500 MiB。AICore 59–62%、AIVector 37–40%，host load1
68.85–73.98；快照不代表整个窗口负载恒定。没有停止或更改其他用户进程。

07:06:38 UTC 结束审计确认 `R10_ACTIVE_PROCESSES=[]`；原 runner 与两库的大小、
mtime、inode 仍与构建前记录一致。所有本轮构建、reference、计时、导出、传输、
离线统计与远端只读命令已结束，`RUNNING_DEVICE_OPERATION=NONE`。上述构建、采集、
传输和统计均返回 0。
无文件删除、外部 push、新分支、工作树、子代理或定时任务。证据为真实研究产物，
继续保留供复核；没有临时备份需要清理。当前槽位可交回 Main，Route 生命周期不变。

研究事件 ID 为 `W4-R10-TIMING-ATTRIBUTION-20261008`，V002 同版事件类型为
`TIMING_ATTRIBUTION_SUPPLEMENT`。提交后回执提供实际提交号与最终 dirty 状态，
等待 Record 异步同步。

## 2026-10-09：V002 设备 0 两轮计时资格

### 当前结论

在 Main 分配的 DEVICE_ID=0 上，复用现有 `r10_probe` 与 `r10_probe_timing`，对
`128x12288 BF16` 完成两种不同条件的同进程 Parent/Parent 与 Parent/Candidate 采集。
两轮的 P/C kernel-task 稳定性均未通过；第二轮的 P/P 总体通过，但 P1 侧及部分
槽位/位置分层未通过。性能方向随首侧与分层变化，不能形成可接受 Local。

```text
ROUTE=W4-R10
REVISION=V002
SHAPE=128x12288 BF16
DEVICE_ID=0
PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=baad7b0588f44f0d7942ea115319b3bb1f04f77a6e0a5a28af28ace0819e4540
COMPILE=PASS_EXISTING
CORRECTNESS=PASS_EXISTING_AND_DEVICE0_REFERENCE
LOCAL_VERDICT=MEASUREMENT_BLOCKED
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
MEASUREMENT_CONFIDENCE=LOW
ROUTE_LOCAL_BEST=NONE
CURRENT_LOCAL_BEST=R31B V011
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
PUSH=NO
```

Candidate 与 Parent 源码未改，未重建 kernel 库。`COMPILE=PASS_EXISTING` 沿用
V002 的 `results/compile.log`；Correctness 沿用 V002 八个输入的通过记录。第二轮
另在设备 0 对 P/C 两库、A/B 两个输出地址做了 CPU FP64 reference 比较，768 行
全部 `failures=0`、`nonfinite=0`，最大绝对误差 `0.007812081001827842`。

### 两轮结果

| 轮次 / UTC | runner / 条件 | Parent/Parent kernel-task | Parent/Candidate kernel-task | 结论 |
|---|---|---|---|---|
| 1 / `04:34:11–04:34:22` | `r10_probe measure`；单输出地址；每阶段 124 条计时样本 | P1/P2 中位 `16.78/16.77 us`；MAD/中位 `1.48%`；两段差 `0.24%` | Parent/Candidate 中位 `22.20/20.45 us`；配对观察 `-1.5116%`；MAD/中位 `17.15%`；两段差 `9.32%` | P/P 通过；P/C 未通过。首侧分组为 `+6.37%/-9.12%`，方向不同 |
| 2 / `04:41:54–04:42:06` | `r10_probe_timing diagnose`；显式前序调用、A/B 输出地址和逻辑槽交叉；每阶段 128 probes + 128 conditioners | P1/P2 中位 `19.892/17.20 us`；合并中位 `17.84 us`、MAD/中位 `6.95%`、两段差 `1.73%`；P1 侧 MAD/中位 `16.49%`，逻辑槽 0 未通过 | Parent/Candidate 中位 `19.82/17.78 us`；配对观察 `-4.9806%`；MAD/中位 `10.67%`；两段差 `17.73%` | P/P 分层未通过；P/C 未通过。首侧分组为 `+0.75%/-12.83%`，方向不同 |

第一轮 device-event 的 Parent/Candidate 中位为 `43.11/44.85 us`，配对观察
`-1.9813%`；P/P device-event MAD/中位 `11.36%`。第二轮 Parent/Candidate
device-event 中位为 `31.54/31.46 us`，配对观察 `-7.3656%`；P/P device-event
MAD/中位 `15.66%`、两段差 `14.86%`。这些 event 数字与 kernel-task 统计均保留，
不作为已通过的 Local 分数。全部样本参与统计，长尾值未删除。

第二轮实际输出 A/B 地址分别为 `0x12c041a00000`、`0x12c041e00000`；两份库在
launch 1、3 前完成设备注册。Profiler 中 756 个 kernel 调用与 task-time 记录对应；
512 个计时调用的 host connection、device task 与前后 EVENT_RECORD 均完成逐项关联。
父版实际 block dim 为 40，Candidate 为 32。配对中位数只作观察，受 P/P 波动、首侧
方向和分层离散影响，不能确立提升。

### 资源与运行信息

两轮均使用 hwnput3 / DEVICE_ID=0，CANN 8.5.0.alpha002、Ascend910B3，NPU
`availableCoreNum=40`。各轮开始时资源脚本
报告 `FREE_HBM=51118 MB`，`npu-smi` HBM Usage Rate 为 22%（容量 65536 MB）。
第一轮 ACL free HBM 在分配前/计时前/结束时为 `50646.246/50634.246/50634.246 MiB`；
主机 load average 开始 `34.51/44.38/54.73`，结束 `31.96/43.33/54.22`，AICore/
AIVector 从 `10%/7%` 变化到 `2%/5%`。第二轮 ACL free HBM 在分配前/诊断计时前/
结束时为 `50646.000/50629.938/50629.938 MiB`；主机 load average 开始
`30.69/35.29/46.48`，结束 `56.03/40.80/48.06`，AICore/AIVector 结束为 `9%/16%`。
没有读取或更改其他进程。

### 原始数据与停止位置

第一轮原始输出、task-time 映射及完整 profiler 导出均在
`本地实验/W4-R10/V002/results/qualification-device0-round1/`，其中包括
`profile-target.pp.tsv`、`profile-target.pc.tsv`、`profile-target.task-map.tsv`、
`profile-target-capture/`、`profile-session.log` 和 `resource-snapshot.log`。

第二轮全部数据在 `本地实验/W4-R10/V002/results/qualification-device0-round2/`，其中
`target.pp.tsv`、`target.pc.tsv` 含 conditioner 与 probe 的全部记录；
`target-task-map.tsv`、`target-pairs.tsv`、`target-contrasts.tsv`、
`diagnostic-summary.json`、`target.reference.tsv`、`target-capture/`、
`profile-session.log` 和 `resource-snapshot.log` 保留逐调用映射、统计、reference、
设备事件与负载事实。

该 shape 已完成本次允许的两轮；不再换设备或采样窗口重试，不创建 V003。Local
分数与增益均为 NONE，V002 不列为 Route Local Best。下一步交 Main 决定 R10 是否转入
新研究方向；本 Route Agent 不扩展当前 V002 任务范围。
