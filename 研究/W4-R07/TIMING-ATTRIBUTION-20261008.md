# W4-R07 V002 计时范围研究

本次有限研究已完成，V002 仍为 `MEASUREMENT_BLOCKED`，Local Best=NONE。
新增性能版 0；一次成功采集新增 128 条 raw，均已对应到 task、event 和 host 调用。
P/C task 中位数均为 14.84 us；event 中位数为 32.9899995/33.090001 us。
没有认定描述符改善，也没有据此认定两种描述符性能等价。

## 采集前声明

日期 2026-10-08；接手提交 addff7d657da4f3fee46e3b882fb80556d916da8。
本次只研究 V002 的计时，不修改 Parent.asc、Candidate.asc 或两份 kernel 包装源码，
不建 V003，不改变 blockCount、MTE3 块长或输出事件。
现有六侧 reference PASS 和全部 620 条 raw 保留，以原 result.json 为来源。

原 runner 的 PC/CP 是逐对交替，每次 launch 各记录一对 event、同步 stop event、写一条 raw。
同一次运行中 P/C 共用 stream、两个 event 和输出地址。每块结束 fflush，块内不重做预热。
Parent-only 在独立进程中预热 45 次，P/C 则先预热 45 P、再预热 45 C。
两份动态库均由 executable 链接，但原 same 模式没有第二个逻辑槽位。
旧 Parent-only 与 P/C 的进程、时段、预热总数及调用节奏不完全可比。

旧目标第一/第二位置的 event 中位数为 18.70/23.81 us；
Parent 为 18.560001/25.1099995 us，Candidate 为 18.86/22.07 us。
同位置的 P/C 也处在不同的相邻调用上下文，不能由该分组单独认定代码收益。
原数据只有 event 与 wall，没有 kernel-task 时长，不能替旧样本补出 task 时间。

## DUPLICATE_AUDIT

```text
MECHANISM=现有V002的task/event范围及逐对第一/第二位置研究；无新性能概念
SEARCHED_HISTORY=addff7d6中的R07 result、研究、runner与全部raw；沿用该研究所列R31/R31A/R31B、W3 R2至V040/R4至V031/R5至V028、MIX、STORE/EPILOGUE及W4输出机制证据；新增读取1bc84959中的R08 RETEST全文
MATCH_FOUND=已有R08方法参考；R07尚无task/event逐次对应证据
WHY_NEW_OR_DUPLICATE=复用V002，不重复源码性能试验；R07为双动态库及逐对交错，不能套用R08单executable、每侧连续31次的数值或原因
```

## 一次有限 host 研究

明确区分三个量：kernel task 时间、event 内 task 前后间隔、host API 时间。
具体待否证假设：本次 event 变化全部来自 kernel task；本次位置差只能由 P/C 代码不同产生。
另观察 R07 的 RecordEvent 范围是否也出现运行时申请/释放，不预先假定与 R08 原因相同。

固定 device2、128x12288 FP16、availableCoreNum=40、epsilon=1e-5，
输入生成器和 CPU reference 复用原 runner。两份既有动态库常驻同一进程，
所有调用共用一次分配的输出地址、stream 和两个 event；地址、函数入口、库路径写入 raw 头部。
不设置 CPU affinity，延续原 runner 的调度方式；每条 raw 记录完成后的 CPU 编号。

采前实际发起 Parent、Candidate 并分别比较独立 CPU reference；失败则不采计时。
只预热一次：45 Parent、45 Candidate，每次 stream 同步，与原 paired 模式一致。
随后 8 个小组，各 8 对：PP、PC、PC、PP、PC、PP、PP、PC。
PP 的两个逻辑槽位使用完全相同的 Parent 函数入口；PC 使用原两库。
每对按原 `(block+sample)%2` 交换第一/第二位置，每次调用仍执行
RecordEvent(start) → 原 kernel wrapper → RecordEvent(stop) → SynchronizeEvent → ElapsedTime → raw。
每组后 fflush。两种比较共享相同输入、地址、预热历史、事件节奏及 host 函数。
PP/PC 无法同时发生，时间进程与上一调用仍是解释限制；组次序交错只提供观察上的平衡。

一共 128 条计时：PP 64 条、PC 64 条；每模式每逻辑槽位 32 条。
采后重新实际发起 Parent、Candidate，分别取回输出比较 reference。
预期 kernel 总数 222：采前 2、预热 90、计时 128、采后 2。
reference 范围仅为本次显式目标；另两个宽度沿用历史六侧结果，不重复执行。

用 msprof 的 task-time、AscendCL、runtime-api 取得逐次数据，关闭硬件指标与 aicpu。
按 device/stream/task ID、时间戳和 connection_id 对应 event 与 kernel；十进制时间戳精确相减。
保存全部原始导出、raw、失败日志和逐次对应表。不能把带 profiler 的时长回用于旧采集。
这次不测 profiler 自身开销，不剔除任何已取得的计时样本，不按结果再采一轮。
固定样本数用于范围归因，不作为有效 Local 的充分证据；Local Best 保持 NONE，
只有真实可支持的范围才写结论。

## 执行位置

本地唯一工作树 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R07-mte3-store-queue-x`，
分支 `w4/r07-mte3-store-queue-x`。支持代码和结果仅在本 Route 的本地实验及研究目录中。
远端复用 `/home/data4t2/lelinfeng/w4-r07/V002/`；新 host 链接原
`build/libw4r07_parent.so` 与 `build/libw4r07_candidate.so`，不重新编译 kernel，
不覆盖原 `build/w4r07_runner`。新 host 可执行文件是此次研究的必要运行对象。
不运行会原位改写 ELF 的命令。Online=PAUSED，PUSH=NO。

开始时 device2 HBM 容量65536 MB、使用率9%，估算空闲59637 MB，
AICore 0%、AIVector 4%，host load1=72.46；采前后另保存实际负载快照。
其他用户任务不操作；不等待独占，不创建后台或定时任务。

## 执行结果

06:51:37–06:51:39 UTC 完成纯 host 构建，GCC 11.4.0，返回0。
06:52:10–06:52:25 UTC 完成一次采集与导出，返回0；没有失败的设备运行或追加采样。
原 runner 和两份动态库的尺寸、mtime 在 host 构建和采集前后均一致。
构建命令只写新 host executable；没有使用 objcopy，没有重新编译或写入原 kernel 库。
这些事实不表示新旧整个可执行文件逐字节相同。

运行进程为 PID404720；输出地址 `0x12c041a00000`，所有计时共用该地址。
Parent 与 parent_peer 函数地址同为 `0xffffa7839f40`，来自原 Parent 库；
Candidate 为 `0xffffa7799f90`，来自原 Candidate 库。两库路径都由 dladdr 写入 raw 头部。
没有设置 CPU affinity，128 次调用结束时记录的 CPU 均为48；该观测不能排除区间内迁移。
raw 写入增加了研究字段和一次 sched_getcpu，均在计时外，PP/PC 使用同一实现。

采前、采后分别真实 launch Parent/Candidate，再与原独立 CPU reference 比较。
四项均 PASS，max_abs_error=0.001953125、mismatches=0、nonfinite=0；
FP16 atol=0.0025、rtol=0。两个其他宽度仅复用 addff7d6 的既有六侧结果，未再次运行。
本次 Candidate 为 EXECUTED，不以两个 Parent 的一致性代替 reference。

首次本地读取发生在 SCP 尚未完成时，返回文件不存在；原错误保留在
`V002/logs/timing-analysis-read-before-transfer-20261008.log`。
传输完成后离线分析通过；该错误没有引发设备重跑或样本删除。

## 旧数据中可以直接确认的顺序关系

全部620条原始数据未改动。离线新增结果为
`V002/results/timing-attribution-20261008/old-order-analysis.json`。

对每个旧 paired 输入，逐对 PC/CP 使实际调用形成 `P,C,C,P,P,C…`。
124次第二位置全部切换库，124次第一位置中123次沿用上一调用的库；
唯一例外是首样本，它接在 Candidate 预热之后。因此库切换与执行位置高度相关。
原 runner 的 same 模式一直调用 Parent，无法单独复现这种库切换关系。

| 旧输入 | 第一位置 event 中位 us | 第二位置 event 中位 us | 逐对第二减第一的中位 us |
|---|---:|---:|---:|
| 128x12288 FP16 | 18.7000000 | 23.8100000 | 1.2799995 |
| 128x8192 FP16 | 19.0000005 | 20.6999990 | 1.0699995 |

两侧中位数之差与逐对差值中位数是不同统计量。不能把“第二位置较长”直接归给
描述符、库切换或某一个 host API。新数据也不能为旧样本补出 task 时间。

## 新数据的精确对应与完整统计

op_summary 有222个 kernel：采前2、预热90、计时128、采后2。
所有222个 kernel 在 device=2、stream、task ID、开始时间和时长上与 task_time 逐条一致；
全部 Block Dim=40。task_time 共526行，timeline共4301项，原始数据全部保存。

128条 raw 各对应一个 task 和两次显式 RecordEvent，并用 connection_id 对应到 host。
起止 event 通过连接编号定位，没有以相邻的任意 EVENT_RECORD 代替。
每个显式 event 区间内恰有一个 kernel task。ACL elapsed 与精确十进制事件时间戳差的
最大绝对差为0.024 us。完整对应见 `task-map.tsv`，没有先把大时间戳转为浮点数相减。
两库 profiler kernel 名相同；P/C 身份来自已知调用序列及函数入口，不能单凭名称区分库。

下表每槽位/侧各32条，全部样本参与。delta 定义为 `100*(右侧中位数/左侧中位数-1)`。

| 对比 | 范围 | 左侧中位 us | 右侧中位 us | delta |
|---|---|---:|---:|---:|
| Parent / 同函数 Parent peer | event | 32.8200015 | 33.8899995 | +3.260201% |
| Parent / 同函数 Parent peer | task | 14.7300000 | 14.6800000 | -0.339443% |
| Parent / Candidate | event | 32.9899995 | 33.0900010 | +0.303127% |
| Parent / Candidate | task | 14.8400000 | 14.8400000 | 0.000000% |

PP 的数值仅为同函数两个槽位差，不是 Candidate Local 分数。
PC 的逐对 delta 中位另为：event +1.450001 us / +4.970628%；
task -0.02 us / -0.133875%。没有用这些统计量替换中位数之比。

| 对象 | event MAD/中位 | task MAD/中位 | task四组中位 us |
|---|---:|---:|---|
| PP Parent | 10.6642% | 0.7604% | 14.72 / 14.79 / 14.56 / 14.80 |
| PP peer | 6.9932% | 0.6812% | 14.68 / 14.74 / 14.54 / 14.71 |
| PC Parent | 10.4577% | 1.3477% | 14.90 / 14.87 / 14.91 / 14.64 |
| PC Candidate | 12.0882% | 1.2129% | 14.77 / 14.84 / 16.00 / 14.67 |

PC 四组的 event delta 为 -8.228042%、+3.006380%、-9.936265%、+8.248082%；
task delta 为 -0.872483%、-0.201748%、+7.310530%、+0.204918%。
第五小组 Candidate task 中位升至16.00 us；全样本中位相同不证明没有局部或尾部差异。

按位置合并，PP第一/第二位置 event 为34.10/32.66 us，task均14.70 us；
PC第一/第二位置 event 为31.330001/33.6399995 us，task为14.812/14.85 us。
这些位置现象仍与时间进程、前序库及 host 路径相关，不认定固定位置的因果效应。

## 归因范围

所有128次计时调用，在 host 两次 RecordEvent 之间均有一次 Runtime@DevMalloc、
一次 AscendCL@aclrtFree；API 时长中位分别为3.23/4.035 us，Node@launch为8.855 us。
event 内、task 外的间隔中位为18.30 us；start-event到task为15.67 us，
task结束到stop-event为3.13 us。不同中位数不可相加还原另一中位数；host和设备区间可重叠，
嵌套API也不可重复相加。本次没有证明内部申请占据了全部外围间隔。

Parent库的只读反汇编进一步定位了申请：FP16 host stub 的 `0x4afe4` 设置请求大小8，
`0x4b018` 调用 AllocAscendMemDevice；helper 的 `0x4ce28` 将该大小传给 rtMalloc。
`0x4b068` 调用 FreeAscendMemDevice，其 `0x4ced0` 调用 aclrtFree。
这是请求8B的静态证据，未确定物理分配粒度、用途或 Candidate 的请求大小。
原源码顶层输入/输出分配在计时外，仍不能排除编译生成的 launch 包装内部申请。
没有移动、删除或改变这些申请与释放。

| 新样本序号 / task ID | 模式 | event us | task us | event内的其他部分 |
|---|---|---:|---:|---|
| 49 / 285 | PP，沿用Parent库 | 67.860000 | 14.58 | task前53.244 us，task后0.020 us |
| 58 / 312 | PP，沿用Parent库 | 59.900001 | 27.02 | task前32.880 us，task后0.020 us |
| 117 / 489 | PC，连续Candidate库 | 53.520001 | 14.56 | task前18.820 us，task后20.120 us |

第49条说明长event可以主要出现在task之前；第58条说明同Parent、不切库时task本身也有长样本。
本次证据排除了“所有event变化都来自kernel执行”和“所有变化都来自Candidate代码”的完整归因。
未确定剩余task长尾由设备负载、调度、缓存或其他哪个因素造成；未采硬件流水线指标。

## 资源、文件和结束位置

采前后 HBM 容量65536 MB、使用率均9%，估算空闲均59637 MB；
AICore均1%、AIVector均3%，host load1=40.78→39.59。
device2已有python进程，其他用户任务与lease均未操作。快照不代表全过程负载恒定。

本地变更只有本 Route 下：`V002/support/runner_main.cpp`、`V002/result.json`，
新增 `V002/support/timing_server3.sh`、`V002/support/analyze_timing.py`、本研究，
`V002/logs/timing-*` 与 `V002/results/timing-attribution-20261008/`。
全部620条旧raw与旧六侧reference保持原样；新raw128条及四项reference、原始profiler数据、
完整导出、逐次对应与统计均保留。未改Candidate、Parent、kernel包装、共享TSV、规则或Dashboard。

远端只操作 `/home/data4t2/lelinfeng/w4-r07/V002/`，
支持源码在support，新host为build/w4r07_timing_host，结果在results/timing-attribution-20261008。
两个原kernel库和旧runner继续保留。没有删除文件，没有安装服务或建立未来任务。

当前结论只覆盖这一次显式目标的有限研究；旧device0与新device2、host支持程序及profiler条件不同。
没有成对测量profiler开销，不能把新旧event差当作任何单因素效果。
新增性能版0、有效Local0、连续无改善次数贡献0；V001仍只作旧研究标签。
Online=PAUSED、Official=NONE、PUSH=NO，Route生命周期未改变。

剩余独立问题为描述符本身的task效应，以及前序库切换与计时位置的关系。
精确下一动作：Main再次安排本Route时，先用现有task-map及旧序列统计设计一个固定的有限host序列，
使 P→P、P→C、C→P、C→C 转移分别覆盖两个计时位置；保持本次输出地址共用方式、两库、
显式输入及event节奏一致，再比较同前序条件下P/C的task时间。
这将回答目前位置与库切换相关造成的歧义；不继续原协议加样本，不开V003或扫MTE3参数。
本轮不执行该后续采集，交回Main安排。

```text
ROUTE_RESEARCH_EVENT=W4-R07-TIMING-ATTRIBUTION-20261008
ROUTE_EVENT=W4-R07/V002
VERSION_RECORD_EVENT=W4-R07/V002
EVENT_KIND=TIMING_ATTRIBUTION_SUPPLEMENT
DIRECT_PARENT=R31B-V011
COMPILE=REUSED_KERNEL_PASS; HOST_PASS
CORRECTNESS=PASS_BOTH_CPU_REFERENCE_BEFORE_AFTER; PRIOR_THREE_INPUTS_RETAINED
OBSERVED_EVENT_SCORE=33.090001 us
OBSERVED_EVENT_DELTA=+0.303126710%
OBSERVED_TASK_SCORE=14.840000 us
OBSERVED_TASK_DELTA=0.000000%
ACCEPTED_LOCAL_SCORE=NONE
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL=0
STAGNATION_INCREMENT=0
STATUS=MEASUREMENT_BLOCKED
OFFICIAL=NONE
ONLINE=PAUSED
PUSH=NO
RECORD_SYNC=WAITING_RECORD
ROUTE_LIFECYCLE_CHANGED=NO
```

提交号与最终工作树状态以提交后的回执为准；运行对象结束情况保留于
`V002/logs/timing-final-state-20261008.log`。本Agent只交还SLOT-1，不自行切换Route。
