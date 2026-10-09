# LOCAL_MEASUREMENT_ROOT_CAUSE_REPORT

## 范围与结论

角色：SUPPORT_RESEARCH。工作树 `/Users/sunyiyang/Desktop/Project/cann/worktrees/support/arch-hardware`，分支 `support/arch-hardware`，分析起点 `503b98cb22ae88798bb6bfca83a46676933f3600`。所有项目规则和路线证据均从本工作树的 Git 对象读取；没有更新工作树，没有进入或改动 Route 工作树、Candidate、共享 TSV、Dashboard，也没有运行设备实验。

未找到既有统一的测量根因报告。主线中有一份与本主题无关的 `线上结果/R31A/V016/root-cause-note.md`。未找到共用测量解析器；仓库里已有 R01、R02、R04、R05、R11 的路线内统计程序。本报告复用其已提交汇总与原始记录，不维护新工具。

当前主线 `技术路线/路线成绩表.tsv` 中 W4-R01 至 W4-R15 的已接受 Local 最佳均为 `NONE`，Official 最佳也均为 `NONE`；主线 `线上结果/` 没有 W4 正式提交目录。依据当前可见记录，暂无 W4 Route 满足线上资格。

**直接答复：NO。** 对这批证据，kernel-task P/P 稳定、device-event P/P 不稳定时，不能把 task-time 单独换作有效 Local 主指标。当前 Local 规范首选 device event，并未授权 task-time 在主要时域不稳时接替它。task-time 可作为独立诊断量；它不能让不稳定的主要时域变成有效 Local。若要改变计时口径，应由 Planning 评估，本报告不改规范或执行条件。

## 要求字段

PRIMARY_TIMING_METRIC=device-event elapsed，按同一 stream 上的起止 Event 包住单次 kernel wrapper/launch，并在读取前完成同步；wall 和 kernel-task 单独报告作诊断，不合并为一个时长。现行 Local 规范写明优先使用 device event，并将 wall-clock 留作诊断。`task_time` 不是纯设备计算时间；CANN 8.5 文档将其定义为包括调度到加速器、设备执行和响应结束的任务时长。`aclrtEventElapsedTime` 文档定义为两 Event 间隔，并列出同 stream 记录、同步后读取的调用顺序。参见[ CANN 8.5 task_time 文档](https://www.hiascend.com/document/detail/en/canncommercial/850/devaids/profiling/atlasprofiling_16_0062.html)及[ AscendCL EventElapsedTime 文档](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/910/API/runtimeapi/aclcppdevg_03_0090.html)。

SAME_BINARY_QUALIFICATION=当前规范要求报告每次 raw latency、统计量、repeatability、执行顺序和异常；W4 控制文件规定稳定性不成立时数值只作观察，不提升 Local Best。规范没有规定一个全项目统一的 P/P 样本数或通过数值。若用 P/P 解释某一具体 shape 的波动，应与候选测量使用同设备、同 runner、同计时范围、同输入及相同地址/槽位安排，并保留完整样本；这是证据解释建议，不增加 Local 资格流程。不要把 task P/P 的稳定结论挪用为 event P/P 的通过结论。

PARENT_PARENT_STABILITY=只在同一 shape、dtype、device、runner、计时范围内评价同一 Parent 的重复分布，包括两槽位、各组中心、P/P 次序及长尾。中心接近不能抵消明显组间漂移；删尾、挑最佳组或事后改统计定义都不能证明稳定。R01/R02/R05/R10/R11 的报告中出现过 MAD/median 0.10、组间差 0.10 等预先声明的路线内数值；R04 采用过不同的预声明数值。这些是各次研究自己的判读条件，不是本报告新增或推广的全局规则。

ORDER_EFFECT_CONTROL=Parent/Candidate 交错，按 block 与 pair 平衡 P→C、C→P 及第一/第二位置；每组都报告次序、前序函数、位置和时间段。R05、R07、R09、R10 的分层对照均出现方向变化，故只报合并中位数不够。P/P 时间漂移应与 P/C 顺序效应分开呈现。

OUTPUT_ADDRESS_CONTROL=记录每次 launch 实际写入的 device pointer、逻辑槽位、调用位置及前序函数。可行时让 P/C 共用同一输出指针；若需双输出，则在 P/P 和 P/C 中对称交换 A/B 映射，且报告各映射结果。记录里的 `0x...` 是设备指针值，不能据此声称已知物理 HBM 页地址；不同进程内相同指针数值也不证明物理分配相同。R02、R07、R08 在共用一个输出指针时仍有波动；R10 的 A/B 交叉对照没有显示一个跨组一致的地址效应。因此地址是要控制和记录的变量，现有证据不支持把它认作唯一原因。

MEASUREMENT_NOISE_FLOOR=按 `Route × shape × dtype × device × runner × timing scope` 分开估计。可用该条件下 P/P 逐对绝对差分布、组中位数范围及位置/次序分层作为经验噪声描述，并与 P/C 的不确定度并列报告。当前规范没有统一数值底线；不能混合不同 shape 或不同计时范围，也不能把一个 Route 的 10% 设定推广到全项目。

VALID_LOCAL_ACCEPTANCE_CRITERIA=依现行规范完成 Compile、Correctness 后，Local 必须保留 Parent/Candidate 全部 raw samples 与 medians、numeric score/delta、shape/dtype、runner 与计时边界、device、free HBM、负载说明、jitter、顺序及 repeatability。主要 device-event 结果须有足够稳定证据支撑；稳定性不成立则保留原数字并标为观察，不计有效 Local、不更新 Local Best。全项目当前没有统一的数值稳定阈值，不能由本研究代定。Official 还需满足线上规范列出的 Compile、Correctness、有效 Local、同 Route 可比最佳、源码对应及额度/当期规则等全部条件。

## 重点路线数据

下表中的比例沿用各提交报告定义；仅在同一行列出的同 shape、同范围数据之间比较。`PP` 为 Parent/Parent，`PC` 为 Parent/Candidate。

| 优先级与证据 | P/P 主要观察 | task-time 与 P/C 观察 | 当前结论 |
|---|---|---|---|
| R10 V002，BF16 128×12288，commit `2903d8c326b534197417307b05e0f0338d15d652` | 第一轮 task PP 中位 32.17 us、MAD/median 13.21%、组差 11.38%；event PP 中位 49.11 us、37.22%、45.57%。第二次诊断 task PP 为 31.50 us、17.43%、2.70%；event PP 为 39.55 us、27.33%、8.75%。目标 shape 的两种范围都未稳定。 | 候选 task P/C 观察受大尾样本、组别与顺序影响；报告将全部 P/C 数值列作观察。输出 A/B、位置和前序对照的差值区间横跨零，没有支持一个固定地址或位置原因。 | `MEASUREMENT_BLOCKED`，有效 Local 0。 |
| R05 V002，FP32 16×2056，commit `da7654a1d80c2399ff44f050b4547db2d1121812` | 第一轮 task PP 两槽中位 3.39/4.04 us，MAD/median 12.7%/21.0%，组差 8.3%/26.7%；第二轮 4.37/4.38 us，MAD/median 13.7%/24.0%，组差 13.3%/28.8%。两轮同二进制控制均未达各自预先声明范围。 | 第二轮 P/C task 单独看来较稳，但 PP 控制失败；event 两侧不稳定。第一轮 16×2056 event 中位数方向与 task 中位数不同。R05 同进程第二轮保持同一输出指针，仍未取得有效 Local。 | `MEASUREMENT_BLOCKED`，有效 Local 0。 |
| R02 V002，FP16 128×20480，commit `f68fb36c53accda216eb175dc187f9d0fb367e56` | 同一 Parent、同一输出指针的 event PP 中位 33.58 us，MAD/median 17.004%，四组范围 26.295%；task PP 中位 21.744 us，MAD/median 0.368%，四组范围 0.230%。报告按本路线预声明的范围判为 event 失败、task 通过。 | PC event 的 P/C 中位数为 27.62/32.43 us；task 为 21.480/21.272 us。task 配对差 -0.218 us 仅是观察，不能代替失败的主要时域 P/P。event 内 task 前间隔中位 11.66 us、最大 24.78 us；event-task 差中位 11.81 us，不能把它们全称为某个 API 的耗时。 | `MEASUREMENT_BLOCKED`，直接回答本报告核心问题的例子。 |

其余证据：

- R01 V001，FP16 16×16384，commit `4b9dcade629bd7e0fdbe3194c740bd1c7824d107`：task PP 两槽中位 8.82/8.80 us；event PP 为 32.35/31.86 us，配对差 95% 区间 [-2.78, 3.52] us。P/C task 中位 9.50/8.91 us 只是观察，配对区间跨零，Parent/Candidate 组差 10.63%/16.05%。所有 timed call 有 task/event/host 对应；1024/1024 次 host Event 区间内记录到 DevMalloc 与 aclrtFree。
- R04 V002，commit `766bb53acb85db70a4c2648feae72511ac20a10e` 与 `66e952958662b8437546ff949ab0156ac79801a2`：768 条 profiler 样本均完成逐项 task/event/host 关联；task PP MAD/median 16.3%–28.4%，P/C 两侧 17.6%–36.9%。后续 profiler-off PP 仍失败：三种 FP32 shape 的两槽 event 中位差为 +7.66%、+18.91%、-12.62%，MAD/median 为 26.15%–47.00%。两次采集设备不同，不能据此单独量化 profiler 的影响；profiler 也不能解释全部不稳定，因为 profiler-off 仍出现波动。
- R07 V002，commit `bca492a32476d579f44b60516f8aeaac6e36b6d5`：同 Parent PP task 两槽中位 14.73/14.68 us，MAD/median 0.76%/0.68%；event 为 32.82/33.89 us，MAD/median 10.66%/6.99%。PC task 两侧中位均 14.84 us，但 event 两侧 MAD/median 10.46%/12.09%，task 分组也有一组 Candidate 中位升至 16.00 us。task PP 稳定未带来有效 Local。
- R08 V001，commit `fa9b19bbcd05240c0398d9a7f5747b651ffe686e`、`1bc84959fbc5dea190ad06a3dcef8b8b3005cae1`：372 条同版证据的两次 PP 中位分别为 21.32/22.60 us 与 46.54/38.10 us，PC/CP 子组方向也反转。后续 248 条 Parent-only 采样完成 task/event/host 对应；event 中位 53.24 us、MAD/median 18.41%、四组范围 20.78%；task 中位 3.412 us、MAD/median 18.76%、四组范围 36.63%。每次 event 内有 DevMalloc 与 aclrtFree；API 用时与设备区间可能交叠，不能直接相加。
- R09 V001，commit `64697a80732bf306071d058861f43a9cba93417f`：两轮 event PP 两槽中位数为 21.32/22.60 us 与 46.54/38.10 us；P/C 两轮百分比方向不同，PC 与 CP 子组都反转。Profiler 文件保留 task_time 及 Event 记录；已提交摘要以 event raw 和次序判断测量无效，本报告不从未呈现逐调用对应的 task 统计推导额外结论。
- R11 V002，commit `1b528176920d57936a9a889c13c3af8293b2a85b`：最新补充是 Parent-only，未运行 Candidate。目标 FP32 64×8192 event MAD/median 15.70%、四组范围 12.95%；task 为 33.39%、30.35%。对照 12×8192 event 为 14.76%、19.06%；task 为 13.59%、4.27%。地址与位置分层不支持用单一槽位解释整组波动。

## 根因判断与可证伪点

**有直接证据的事实**

1. 两种工具时域并不等价。msprof `task_time` 对同一任务给出任务调度/执行相关时长；device event 给出两个 stream Event 之间的经过时间。R02、R07、R08 的逐调用映射完整，但 event 与 task 中间仍出现大小可变的间隔。时间戳、task ID 和 host connection 对应能证明样本配对，不会把两个时域变成同一个量。
2. 部分 task 自身也不稳。R10 目标、R04、R08、R11 有高 task MAD 或显著组差。故只删去 event 外围区间并不能保证 kernel-task 稳定。
3. Host launch wrapper 的运行时内存 API 确实落在不少 event 包围的 host 调用中。R01 1024/1024、R07 128/128、R08 248/248 次记录均出现 DevMalloc 与 aclrtFree。R07 只读 Parent stub 还能看到 8-byte 请求，但实际分配粒度、用途及 Candidate 侧等价请求未确认。API 与设备时间有重叠；现有记录没有证明这些 API 单独造成全部 event-task 差。
4. 次序效应有直接记录。R07 旧 PC/CP 流程让第二位置全部切库；R05、R09 的 PC 与 CP 差会反向；R01/R04/R10 分层后仍有跨组变化。不能把合并差值都归到 Candidate。
5. 输出指针不是充分解释。单指针条件下 R02、R07、R08 仍波动；双槽与交叉地址条件下 R01、R10、R11 也未出现能一致解释全组数据的单一地址效应。R10 的对照分位区间跨零，仍不足以排除地址与位置交互。

**可能成因及否证观测**

| 可能成因 | 当前支持 | 可推翻或显著减弱它的观测 |
|---|---|---|
| Event 之前的 host 下发/设备排队及 Event 之后的完成间隔 | 多份逐调用映射显示 event-task 差与 task 前间隔会变化；R02 的 task 前间隔最大 24.78 us，R08 的 event 外 task 间隔中位 50.162 us。 | 在固定设备、runner、输入和先后序列的多组样本中，event-task 前后区间不再随 event 长尾变化，而 event 长尾仍重复出现。 |
| Wrapper 内 DevMalloc/aclrtFree 造成额外提交或同步等待 | 每次调用都观测到 API，部分 API 时间与长间隔同时出现；R07 静态代码能定位 Parent 的请求入口。 | 在同一计时边界的受控比较中，API 出现与否、时长变化均不伴随 event 前后间隔变化。仅有 API 同时出现不足以确认因果。 |
| P/C 位置、前序库、输出地址映射 | 有分组方向反转；R10 做了交叉地址和前序对照。 | 同进程、均衡的 PC/CP 与 A/B 交叉记录在多组内呈相同方向，且 P/P 对应分层没有位置或地址差。 |
| Host 或设备负载与调度波动 | 多份离散快照有较高 load、AICore/AIVector 活动或其他进程；这些只说明运行背景，不能说明整个计时区间的状态。 | 负载快照和进程背景变化时，按同 shape 分组的 event/task 分布仍无可重复关系。R02 低 AICore/AIVector、FREE_HBM 62259 MB 时 event PP 仍失败；R04 profiler-off、设备快照 0% 利用率且无设备进程时仍失败，所以负载不是唯一解释。 |
| Profiler 本身扩大时间间隔 | 目前只有 profiler-on/off 采样条件不同的对照；R04 两次使用不同 device，不能隔离该因素。 | 同一设备与 runner 下，预先均衡的 profiler-on/off P/P 分布没有稳定区别。现有数据尚未完成这个对照。 |
| 跨进程分配或初始化状态 | R05 首轮 P/P、P/C 分进程；即便两进程日志出现相同指针数值，也不能证明物理内存、缓存或运行状态相同。 | 同进程、双库常驻、共用输入/输出和相同先后条件下，事件与 task 分布仍同样不稳；R01、R07、R08 已提供同进程波动例子，故跨进程不是唯一原因。 |

## 有限诊断建议

已有程序已覆盖单进程/分进程、PC/CP、task/event/host 映射、地址交叉、Parent-only 和 profiler-off；照抄旧采集不会补上新的区分能力。若 Planning 同意一个有限诊断例外，建议只在 R02 V002 的 128×20480 FP16 上复用现有 runner 与二进制，做一组同设备对照：相同输入、相同 stream 与输出指针、45 次预热、四组各 21 对；P/P 与 P/C 各自均衡 PC/CP。分别以 profiler off、profiler on 完成一次固定采集，保存全部 event、wall、task、host API 和调用映射，不按负载挑时段、不删除样本、不改 Candidate。该设计与旧 R02 的区别是同一硬件和同一 runner 上成对记录 profiler 开关条件；R04 现有 profiler-off PP 使用了不同设备，不能完成这个区分。

这是一项诊断建议，不运行实验、不替代 Local 规范。若 event P/P 仍不稳，结果仍为观察，不改用 task-time 取得成绩；若两种条件间差别不能在完整样本和分层顺序中复现，则不把 profiler 认定为主要原因。Planning 可据结果决定是否需要调整公开计时口径及资格定义。

在当前 W4 共享表无任何 Local/Official 最佳的情况下，建议 Main 向 Planning 提交 `OFFICIAL_GATE_REVIEW_REQUEST`，请求的范围仅为上述一次诊断性采集的有限例外，不请求绕过当前 Local 有效性条件，也不请求提交任何 Candidate。是否授权以及之后是否调整规范由 Planning 决定；本 Support Agent 不提交、不更改规范。

## 来源索引

- 当前规则：`main:AGENTS.md`、`main:.agents/skills/cann-support-research/SKILL.md`、`main:.agents/skills/cann-mainline/SKILL.md`、`main:项目规则/本地性能测试规范.md`、`main:项目规则/服务器实验规范.md`、`main:项目规则/线上提交规范.md`、`main:项目规则/W4持续探索控制契约.md`、`main:项目规则/实验总则.md`、`main:项目规则/执行约定.md`。
- 共享状态：`main:技术路线/路线成绩表.tsv`、`main:技术路线/全版本记录.tsv`、`main:调度/当前任务.tsv`；状态基于本机 `main` ref，未更新分支。
- Route 提交：R01 `4b9dcade629bd7e0fdbe3194c740bd1c7824d107`；R02 `f68fb36c53accda216eb175dc187f9d0fb367e56`；R04 `766bb53acb85db70a4c2648feae72511ac20a10e`、`66e952958662b8437546ff949ab0156ac79801a2`；R05 `da7654a1d80c2399ff44f050b4547db2d1121812`；R07 `bca492a32476d579f44b60516f8aeaac6e36b6d5`；R08 `fa9b19bbcd05240c0398d9a7f5747b651ffe686e`、`1bc84959fbc5dea190ad06a3dcef8b8b3005cae1`；R09 `64697a80732bf306071d058861f43a9cba93417f`；R10 `2903d8c326b534197417307b05e0f0338d15d652`；R11 `1b528176920d57936a9a889c13c3af8293b2a85b`。
- 当前主线搜索未找到 `LOCAL_MEASUREMENT_ROOT_CAUSE_REPORT` 或同名字段。已找到的 analyzer 均随各 Route 提交；未发现共用实现。
