# R10 V002 计时归因

## 范围与采集前声明

本次接续 `1441fc7297b37d37a4af67d0775cb49f37f5956d` 的 V002，仅改变 host
测量工具。Parent 为 R31B V011；V001 的 reference 失败原样保留，不作为 Parent。
不改 kernel、不建 V003、不改共享记录、规则或 Dashboard。设备固定为 4，
SLOT-5，Online=PAUSED，PUSH=NO。新增性能版数、有效 Local 数及连续无改善贡献
初始均为 0；R10 已接受 Local=NONE，上游 Parent 仍为 R31B V011。

工作树为 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R10-active-core-d-aware-x`，
分支为 `w4/r10-active-core-d-aware-x`。开始无未提交内容，领先远端 3 个提交。
已完整读取本工作树 AGENTS 与 Route Skill，以及 `9f918955` 指定的十项规则/脚本。
已在对话中发送 RULE_REFRESH_RECEIPT。共享事实读取自 `main@07662d7b`；V002
已经登记，当前接管阶段优先于表中的排队文字。新研究结果等待 Record 同步。
本工作树没有 ops-profiling 副本，使用已安装插件的同名 Skill 与 msprof guide。

## 已存证据与不可判定范围

离线分析复用 V002 两份完整 profiler 导出，没有重新运行旧命令。856 个 kernel
与 op_summary/task_time 对应；496 个 timed call 又由 connection_id 对应 host
Node@launch 与两次 RecordEvent。完整扩展行位于
`本地实验/W4-R10/V002/results/timing-attribution-20261008/old-*-attribution.tsv`。

每个 timed call 的两次 host RecordEvent 之间均有一次 Runtime@DevMalloc 和一次
AscendCL@aclrtFree。target P/P 的对应时长中位为 4.54/5.54 us，P/C 为
4.455/5.415 us。外围五个 buffer 的分配在计时外，不能据此宣称 launch 内没有分配。
只读反汇编在原两库中定位到 AllocAscendMemDevice→rtMalloc 与
FreeAscendMemDevice→aclrtFree；本次不改变这些调用，不从 API 名称猜申请用途。

target 的 P/P/P/C event 内、kernel 外总间隔中位为 7.38/7.26 us；control 为
4.07/2.85 us。各 API 与各区间中位数不能相加，也不能把嵌套 API 重复累加。
control task 1169 为 10012.24 us，event 为 10013.859748840 us，表明该长尾主要
处在 kernel-task 区间内。task 180 为 2205.544 us，同样不是首个计时样本。
这排除了全部长尾仅来自 event 外围区间的解释；没有证明设备内某条指令的原因。

两次采集的设备二进制注册都发生在 launch 1 与 186 之前。P/P timed 范围是
61–184，Candidate 的首次设备注册在其后。host 两库已链接不等于 P/P 开始时
两库均完成设备注册。P/C 的第一位置有 61/62 次重复前序 kernel，第二位置
0/62 次重复；原位置与前序 kernel 发生混杂。P/P 则两位置都只运行 Parent。

target 的 248 个 timed call 中，46 个设备开始时间早于对应 Node@launch，49 个
早于 Runtime@KernelLaunch；control 对应为 25/32。连接关系已逐条确认，这些
跨时钟时间倒序不满足因果先后。保留各时间域自身的时长与顺序，不据此量化
host 提交到设备开始的真实等待。现有 trace 没有逐核时间、内存物理布局、其他
进程的同时段任务、完整主机调度或 profiler 开销对照，无法把全部波动指定给它们。

## 重复范围

```text
DUPLICATE_AUDIT
MECHANISM=同进程双库提前注册；指定前序调用；逻辑槽位、输出buffer与先后次序独立交叉
SEARCHED_HISTORY=R10 V002@1441fc72；R10 ACTIVE-CORE-STUDY@5b7b9215；R02@e100b5d9；R11@1b528176；R08@1bc84959
MATCH_FOUND=NO_FOR_THIS_MEASUREMENT_DESIGN
WHY_NEW_OR_DUPLICATE=既有同地址或双地址P/P没有独立指定每个观测调用的前序kernel与输出关系；本次是测量研究，不是新的kernel概念
REVISION=V002
DIRECT_PARENT=R31B V011
NEW_PERFORMANCE_REVISIONS=0
```

R10 已存历史研究覆盖 W3 R2 V040、R4 V031、R5 V028，以及 R031/R31A/R31B、
MIX、STORE 和相关 W4；本次复用该范围，不重新提出缩核想法或声称新增性能轴。
R02/R11/R08 数字仅提供方法线索，不进入 R10 统计，不替代本 Route 的 P/P。
只从本工作树用 git show 读取这些提交，未进入其他实际工作树。

## 唯一有限诊断

使用原 `support/probe.cpp` 的固定输入生成、epsilon、dtype 与 reference 公式。
原八个输入的双方 reference PASS 继续有效；本次改变 buffer 安排，故对 target
128x12288 BF16 与 control 48x12288 BF16 的两个实际输出 buffer，分别执行
Parent/Candidate reference 比较。其余六个原输入不重新采样。两个地址在采样后
再次读回验证，不额外 launch。未宣称 V001 失败域已解决。

每个输入只启动一次 diagnose 进程；进程内先 P/P、落盘，再 P/C。两阶段使用
相同输入、两个固定输出 buffer、stream、start/stop event、双库路径与装载地址。
两份库在 reference 和预热时已实际调用，之后不卸载。两个输出指针标为 A/B；
这是设备指针与 buffer 身份，不是已知的物理 HBM 通道映射。

每阶段预热 60 次 Parent、60 次 Candidate，A/B 各分到每库 30 次。随后两组，
每组完整覆盖以下 32 个条件，各条件一对观测调用：

| cell 位 | 两个取值 |
|---|---|
| 0 | 指定前序 kernel：Parent / Candidate |
| 1 | 前序写出：与观测相同 buffer / 另一个 buffer |
| 2 | 逻辑槽位到 A/B 的对应：正常 / 交换 |
| 3 | 观测先后：槽位 0 先 / 槽位 1 先 |
| 4 | P/C 到逻辑槽位的对应：正常 / 交换 |

每个观测前都执行一次指定的前序调用。前序与观测使用同一套
RecordEvent→launch→RecordEvent→SynchronizeEvent→ElapsedTime；全部保留 raw。
P/P 的两个观测均调用 Parent；前序条件仍可调用 Candidate，以保持与 P/C 相同的
双库及前序安排。两个 block 使用预先固定的奇数步长排列，P/P 与 P/C 排列相同。
不在采样循环内新增打印、写盘、分配或暂停；原库内部的申请/释放维持原状。

每阶段 64 对、128 个观测加 128 个前序 timed call；每输入两阶段合计 512 条，
另有 240 次预热、4 次 reference launch，共 756 次 kernel 调用。两输入合计
1024 条 timed call。计数用于完整性核对，不通过追加样本获得结论。

只用 msprof task-time、AscendCL、runtime-api，关闭 ai-core 指标与 aicpu。
原 `build/r10_probe` 与两份库不重建；使用原 GCC 11 的 gnu++17、原 include/link
参数，单独生成必要的 host 诊断程序 `build/r10_probe_timing`，原已测 ELF 保留。
远端只操作既有 `/home/data4t2/lelinfeng/cann/w4/R10-active-core-d-aware-x/V002/`。

## 指标与停止位置

P/P 先保留全部样本。资格以观测调用为对象，前序调用不冒充 P/P 观测。
沿用 MAD/median≤10%、两组中位差/全样本中位≤10%；分别报告整体、两个逻辑槽、
两个输出 buffer 的值。P/C 两侧使用相同统计；同时报告每个前序 kernel、
同/异 buffer、两种次序和槽位对应下的配对差，及 P/P 配对绝对差 p90。
主观察量仍为逐对 `(C/P-1)*100` 的中位数，总体中位比另列。

不稳定或效果被 P/P 误差覆盖时，接受值保持 NONE，状态 MEASUREMENT_BLOCKED。
这次交叉设计改变了调用历史，只能解释本次条件下的结果，不能倒推旧 V002 的
无干扰收益。若独立因素仍不能分开，交付不可判定范围和不同的下一研究动作；
不增加无变化运行，不创建下一性能版。所有本轮命令结束后提交证据、发送 V002
同版事件与 ROUTE_RESEARCH_EVENT，交还槽位，由 Main 决定后续安排。

## 实际完成与研究事件

上述唯一设计已执行，目标与对照各一次，无追加采集。host 构建、reference、采集、
导出、传输和离线分析均返回 0；原两份库未重建。1024 个 timed call、1512 个
完整 kernel 调用、256 个观测对与 1280 个因素对照全部保留。旧 496 个调用也完成
host connection_id 关联。详细数值与源路径见 V002 `RESULT.md` 的本次补充节和
`results/timing-attribution-20261008/diagnostic-summary.json`。

target P/P task 中位 31.500 us、MAD 17.4286%；control 为 18.462 us、MAD 10.2481%。
新 P/C 的 target 中位 31.930/28.180 us，配对 delta -1.4061318749%；control
17.850/17.860 us，配对 -0.3834903657%。全部仅观察，接受值保持 NONE。
target P/P 配对绝对差 p90 19.1872 us，大于 P/C 配对差中位的绝对值 0.230 us。
两组及地址/次序分组仍有反向，未取得可靠 Local。

可确认的新边界是：双库提前注册与原前序/位置混杂已经按本设计分开，但仍不能
将剩余波动固定归给某个逻辑槽位、输出地址或前序因素。原两库的 host 包装还
明确包含每次 launch 的 8 字节临时设备申请，作为 80 字节参数布局的末尾指针，
正常路径释放。完整用途未确定，不应先绕过或复用它。下一动作改为沿原库
BF16 host 入口和 launch helper 做只读 ABI/生命周期分析，不重复本矩阵。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R10-TIMING-ATTRIBUTION-20261008
ROUTE=W4-R10
REVISION=V002
DIRECT_PARENT=R31B V011
KIND=MEASUREMENT_RESEARCH
STATUS=ATTRIBUTION_BOUNDARY_ESTABLISHED
LOCAL_STATUS=MEASUREMENT_BLOCKED
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=R31B V011
ROUTE_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_IMPROVEMENT_CONTRIBUTION=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=只读追踪原两库 BF16 launch 尾部8字节指针的接收方与生命周期；不重采本矩阵、不建V003
RECORD_SYNC=PENDING
```

实际提交号与最终 Git 状态由提交后同版回执提供。07:06:38 UTC 远端最后审计中
R10_ACTIVE_PROCESSES=[]；本次全部命令已结束，可由 Main 回收 SLOT-5。
