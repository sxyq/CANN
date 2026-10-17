# W4-R05：参数与输出的实际暂存区

本文前半保留 `61e0aa28` 的研究结果。后续 V002 已完成，当前结论及新增证据见末节和
[V002 结果](../../本地实验/W4-R05/gamma-view-20261008/RESULT.md)。

## 61e0aa28 研究结论

建议 `NARROW`：保留窄行 FP32 参数子视图的相对位置研究，排除重复的宽行扩容试验。
源码中可以界定一个与 R04 输入布局、W3 R1 padding 不同的轴：保持全部
`InitBuffer` 不变，只移动 `ProcessNarrowMidOverlap` 的 FP32 gamma 子视图起点。
现有 gamma 容量足以容纳对齐偏移，Load 与 Mul 使用同一个视图；x/residual、
输出、DMA 数量、事件和算术顺序均可保持原样。

这只证明源码范围与容量关系，尚未证明 910B3 的 bank 冲突会改变。
本轮没有收到 Main 转交的 R04 硬件结果，未重复检索或采集共同硬件资料，
没有选择性能偏移量、编辑 Candidate 或创建性能版本。
是否继续实验交 Main/Planning 复核，Route 生命周期保持原状。

## 身份与来源

- Agent：`01a119cf-3c06-7ea3-afed-1990f75245ee`，SLOT-2，唯一 Route 为 W4-R05。
- 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R05-ub-bank-param-out-layout-x`。
- 分支：`w4/r05-ub-bank-param-out-layout-x`；接手提交：`de70b634813dea80783fc57716d6e95c158edeec`；接手时无未提交改动。
- Parent：接手提交中的 `线上结果/R31B/V011/submission.asc`。下文未另标来源的行号均指此文件。
- 规则：已读取本树 AGENTS 与 Route Skill，并从本树完整读取 `9f91895506023d917637f707bb3f61cd9d9f8765` 中用户指定的九个入口，已发送 `RULE_REFRESH_RECEIPT`。
- 本树没有 AscendC API Skill 副本；已读取安装版 `ops-direct-invoke:ascendc-api-best-practices`、其 `api-buffer.md` 与 `api-datacopy.md` 的起址对齐说明。它们只支持 API 语义，不作为 bank 映射证据。
- Main 新转交的目标 API 来源：`bd825c5fd59c74c283478026e8b32702ba331eb5:研究/W4-R06/TRANSACTION-STRUCTURE-20261008.md`，已完整读取。它核对了 server3 CANN 8.5.0.alpha002 的 dav_c220 字段和 LocalTensor 目标跨度；本轮不重复读取远端头文件。
- 旧 Record 来源：`w3/m1/record-owner@cecec26b:技术路线/全版本记录.tsv` 的 R05 长 ID 行。该行写明未创建 Candidate，V001 只作旧研究标签。
- 指定规则提交的三份共享表未找到 R05 短 ID 行，保留 `STATE_SYNC_GAP`；本 Route 不写共享表。

`CURRENT_LOCAL_BEST=NONE` 表示没有 R05 自身的有效 Local 改善；
`INHERITED_REFERENCE=R31B-V011` 保留旧记录的继承参考。
没有把旧记录里的参考版本改写成新测量。

## 可达分支与物理复用

`Init` 在 D>8192 时进入宽行分配并于 113 行返回；`Process` 在 163–169 行
直接进入宽行实现。FP32 宽行经 1847–1853 行固定调用
`ProcessWideFp32FullCacheRows`。同文件中的旧 PanelResident、Batched 等函数
不能仅凭函数仍在文件中就当作当前实际消费者。

表中的“同一区域”由相同 TBuf 的 Get 与索引直接确定，表示分阶段覆盖使用；
不同名称的局部变量不代表不同内存。独立 InitBuffer 表示源码中的独立分配对象，
本轮没有测量它们的数值 UB 地址，也不据分配顺序推断 bank 编号。

| 实际路径 | gamma / bias 来源 | 最终输出来源 | 与 x/residual 的关系及寿命 | 关键行号 |
|---|---|---|---|---|
| FP32，D>8192 | `gammaLocal=xBuf_.Get<float>()`；`biasLocal=residualBuf_.Get<float>()` | `valueFp32Buf_` 中保留的整批行 | x/residual 在 pass1 先存输入，再作平方/归约工作区；行倒数得到后，同一区域在 pass2 存参数。没有分配 gammaBuf_/biasBuf_/outputBuf_ | 69–80、2121–2157、2179–2205、2224–2226 |
| FP16，D>8192 | `gBase=xBuf_.Get<T>()`；`bBase=residualBuf_.Get<T>()`，各有两段交替使用 | 独立 `outputBuf_`，单 tile | pass1 的 xBase/rBase 与 pass2 的 gBase/bBase 是同一对 TBuf；`gammaBuf_` 在整个 batch 保存 half 的 x+residual，完全不保存 gamma；biasBuf_ 未分配 | 81–97、3117–3137、3177–3190、3256–3299、3315–3344 |
| BF16，D>8192 | 与 FP16 相同的原类型 staging；再转入 xFp32Buf_/residualFp32Buf_ | 独立 `outputBuf_`，单 tile | 原类型参数复用 x/residual；FP32 参数又复用 pass1 的转换/归约工作区；完整 x+residual 保存在 valueFp32Buf_；gammaBuf_/biasBuf_ 未分配 | 98–111、3192–3203、3256–3305、3328–3344 |
| FP32，D<=8192 | 独立 gammaBuf_/biasBuf_，各 8192 float；Load 后一直供乘加使用 | valueFp32Buf_ 的原位乘加结果 | 参数没有与 xBuf_/residualBuf_ 别名；值缓存承担 x+residual 与最终输出两种用途 | 116–122、374–375、429–437、463–468、511–512、581–587、1127–1131 |
| FP16，D<=8192 | 独立 gammaBuf_/biasBuf_，各 8192 half | 独立 outputBuf_；D=4096/8192 可容纳 8192 half，其余容纳 4096 half | 参数、输入、输出在源码中均有独立分配；参数随局部行数选择整段驻留或逐行读取 | 123–132、511–517、538–541、589–597、759–762、842–854 |
| BF16，NarrowMid | 原类型 gammaBuf_/biasBuf_，参数转换后使用 xFp32Buf_/residualFp32Buf_ | xBuf_ | 原类型参数与 x/residual 分开；输出覆盖已经消费完的 x；转换工作区在 pass1 与 pass2 间复用 | 133–155、511–512、552–560、599–610 |
| BF16，D=4096/8192 且多行 | gammaBuf_/biasBuf_ 先作 staging；gammaFp32Buf_/biasFp32Buf_ 驻留参数 | gammaBuf_/biasBuf_ 交替输出 | 参数转换完成后，原 staging 才被输出覆盖；MTE3 完成事件保护再次写入 | 629–639、705–733、883–895、956–983 |
| BF16，generic / 小行连续批处理 | 多行时使用 FP32 参数缓存；单行时从 staging 转入 FP32 工作区 | generic 多行用 gammaBuf_，单行用 xBuf_；小行批处理用 gammaBuf_ | 输出别名取决于实际分支；不能把 gammaBuf_ 永久视作只读参数区 | 245–279、379–423、451–490、1674–1688、1775–1801 |

宽行低精度 pass1 在 3212–3221 行收完槽位释放事件，行倒数计算后于 3243 行
执行 V→MTE2 同步，再开始参数读取。pass2 的参数槽位在 3346–3362 行释放。
FP16 的 half 行缓存必须保存到相应输出 tile 消费结束，outputBuf_ 在 3344 行
等 MTE3 完成后才能再次写入。布局研究不能顺便改变这些寿命或事件。

R13 的已提交结果 `91e891a2:研究/W4-R13/RESULT.md` 记录 FP32 宽行
1x9216、1x10240 对 reference 失败，且部分值可由后一个参数 tile 解释。
这是复用时序的待验证线索，不是 bank 证据。本轮没有复测或实现该方向。

## W3 R1 V002 / V003 / V009 逐项对应

来源为 `w3/m1/ub-bank-layout@64e32f53`。
以 R31B V011 对比 V001–V010 的完整 submission，十版均只有各自的一项源码变化。
V001 为宽 FP16 输入分配顺序，V004 为归约步长，V005–V008 为工作区扩容，
V010 为 xBuf_ 扩容；这里重点对应三项与参数/输出有关的记录。

| 版本 / 提交 | 唯一源码变化 | 实际影响对象 | 不能据此声称的结论 |
|---|---|---|---|
| V002 / `41689004` | 88 行 outputBuf_ 由 tileElems 改为 2*tileElems | 仅宽 FP16 的输出分配容量；访问仍从 Get<half>() 起点开始，每次只用 valid 个元素 | 没有新增输出双槽、没有改变参数消费者地址；不能称为新参数布局 |
| V003 / `d84cae1a` | 92 行 gammaBuf_ 分配长度增加 32 B | 宽 FP16 的整批 half 行缓存；yStore/yTile 的逻辑索引未改变 | gammaBuf_ 此处保存 x+residual；不能把这版称为 gamma 参数偏移试验 |
| V009 / `34109316` | 88 行 outputBuf_ 分配长度增加 64 B | 宽 FP16 输出分配容量；outputLocal 的起点表达式未改变 | 没有直接把 outputLocal 移动 64 B；没有确定物理 bank 编号 |

三个变化都只位于 D>8192 且 T=half 的 Init 分支。
旧 runner 的两个输入来自各版 `support/paired_runner.cpp`：2x8192 FP16
是未触发该分支的对照，2x32768 FP16 才触发。
分配长度可能影响其他对象的实际放置，当前没有地址证据，不能把旧计时归因到单个 bank。

保留旧日志中的数字，只作历史解释，不成为 R05 Local：

| 版本 | 输入 | Parent / Candidate median，us | 配对差值 median，us |
|---|---|---:|---:|
| V002 | 2x8192 FP16 | 24.680 / 28.280 | -0.180 |
| V002 | 2x32768 FP16 | 17.700 / 23.940 | +1.720 |
| V003 | 2x8192 FP16 | 33.320 / 44.780 | +8.340 |
| V003 | 2x32768 FP16 | 58.580 / 38.440 | -20.660 |
| V009 | 2x8192 FP16 | 24.140 / 29.020 | -0.800 |
| V009 | 2x32768 FP16 | 45.280 / 22.260 | -3.860 |

每组 21 对、warmup_each=45、device=7；宽行两侧 CV 分别为
V002 0.62054/0.58143、V003 0.77501/1.01706、V009 0.46451/0.63990。
差值定义为 Candidate-Parent。配对差值中位数与两侧中位数之差是两种统计量，
不能混用。旧数据波动明显，不能据负号宣布稳定改善。
日志对双方分别报告 reference PASS；已核对 runner 的 CPU expected 计算与双方
CheckOutput 调用，FP16 容差为 0.004+0.004*abs(expected)。这不替代本轮设备验证。

日志路径均在 `本地实验/UB-BANK-LAYOUT-CHAMPION-X/<版本>/support/logs/`：
V002 `local-20261005T055123Z-runtime-fixed.log`，
V003 `local-20261005T061046Z-runtime-fixed.log`，V009 `local-20261005T090701Z.log`。
原始样本继续保存在对应 Git 对象中，没有复制或删除。

## DUPLICATE_AUDIT 与剩余独立轴

| 已读来源 | 本轮实际对比范围 | 对窄行参数子视图的结论 |
|---|---|---|
| W3 R1，`64e32f53` | V001–V010 对 R31B V011 的完整差异；V002/V003/V009 runner 与 Local 原始日志 | 已有宽行容量/顺序变化；没有本节窄行子视图变化 |
| W3 R2，`6321ad44` | V001–V040 的 Init 与 ProcessNarrowMidOverlap 完整函数 | 40 版两函数均与 Parent 相同 |
| W3 R4，`ce6c6dc5` | V001–V031 同两函数 | 31 版 NarrowMid 相同；只有 V001 的 Init 给宽 FP16 增加缓存行数上限，与窄行无关 |
| W3 R5，`1efa0863` | V001–V028 同两函数，读取 Candidate.asc | 28 版两函数均与 Parent 相同 |
| R031/R31A/R31B、MIX、COEFF、STORE、UB-LIVENESS、EPILOGUE-ARITH | 接手提交中 58 份相关源码的参数赋值与 NarrowMid；另读线上目录 10 份源码，包含 R31A V024/V025/V026/V028、R31B V016/V017、STORE V003 | 没有找到窄行 gamma 的固定非零子视图起点；旧 R031 工作区按 3*tile/4*tile 分段，UB-LIVENESS 改阶段复用，均是其他结构 |
| W4 R01/R02/R09/R10/R11 | `93f15d9b`、`60ca277d`、`96044629`、`5b7b9215`、`4243f4e9` 中已提交 Candidate 的 Init/NarrowMid | 两函数均与 Parent 相同 |
| W4 R08，`fa9b19bb` | V001 的 Init 与 NarrowMid 完整差异 | 增加跨行 x 预取；参数 Get 起点不变 |
| W4 R06/R14/R15 | `b09e00eb` 的输入 issue-order 研究；`fcbd1814` 的参数 DMA 数量研究；`d13e51e9` 的多行 DMA 研究 | 本节不改 issue-order、驻留次数或事务形态；这些记录不提供 bank 映射 |

R31B V013 的 NarrowMid 曾调整参数预载时序，gamma/bias Get 起点仍为零。
部分旧 EPILOGUE-FUSE 源码不在已读提交中，仅有机制记录；本轮不宣称覆盖全部历史源码。
上述函数相同只回答本轴是否重复，不代表其余函数或整份 Candidate 相同。

```text
DUPLICATE_AUDIT
MECHANISM=宽 FP16 outputBuf_ 扩容或 gammaBuf_ 行缓存 padding
SEARCHED_HISTORY=W3 R1 V001-V010；旧 R05 研究行；上述跨路线来源
MATCH_FOUND=YES
WHY_NEW_OR_DUPLICATE=V002/V003/V009 已有同一对象、同一 Init 分支与相同容量机制
NEW_PERFORMANCE_REVISION=NO

DUPLICATE_AUDIT
MECHANISM=仅调整 NarrowMid FP32 gamma 子视图起点，全部分配保持原样
SEARCHED_HISTORY=上表限定的已提交源码与机制记录
MATCH_FOUND=NO_IN_READ_SOURCES
WHY_NEW_OR_DUPLICATE=现有分配中的消费者视图位置变化，区别于宽行扩容、x/residual 布局、预取和输出队列变化
NEW_PERFORMANCE_REVISION=NO
```

具体容量证明：FP32 gammaBuf_ 在 121 行申请 8192 float；NarrowMid 的 valid=D，
入口限定 128<D<=4096，参数只经 gammaLocal 读写。若偏移 s 个 float，
要求 s 为 8 的倍数且 `s+round_up(D,8)<=8192`。例如 s=8 仅用来说明
32 B 对齐与容量可同时满足：最大终点为 4104 float，小于 8192。
这不是选定的性能值；没有把 32 B 对齐单位当作 bank 粒度。

R06 新来源中的目标容量公式为
`blockCount*AlignUp(blockLen+paddingSize,32)+(blockCount-1)*dstStride*32`，
来源是该提交记录的 `impl/utils/kernel_check_data_copy_overflow.h:458`。
本轴保留 blockCount=1、默认左右 padding=0，因而每次目标跨度仍为
`AlignUp(D*4,32)`，仅需落在 gamma 子视图剩余容量内。没有跨越另一 TBuf，
没有把 gamma/bias 合为双块搬运。该公式来自 CPU 调试路径的 API 边界依据，
不提供 bank 编号或硬件耗时。R06 的跨输入双块结论不直接替代本轴的容量证明。

数据流为：gamma GM[0:D] → 偏移后的 gammaLocal → 原来的 Mul。
两个 Load 位置分别为 515 行（多行驻留）和 539 行（单行），Mul 在 582 行；
它们引用同一个局部变量。保留数据类型、有效长度、事件和所有算术调用即可保持
逐元素语义；仍须实际 Compile 与独立 reference Correctness 才能取得实验结果。
其他 dtype、Init、x/residual/输出视图均不应随此想法改变。

有来源的未来输入为 R2 V040 `support/probe.cpp:368`、`:369` 中的
16x2048 与 16x2056 FP32；`712e4723:研究/W4-R03/HOST-OCCUPANCY-STUDY.md`
已绑定这两项与 40 个可用核、16 blocks、每核一行、NarrowMid 分派。
两项都会受 gamma 视图变化影响，不能把原 runner 名为 control 的 2056 输入
当作本轴的未变化对照。该 host 证据没有在设备 1 重新采集，也不提供 bank 事实。

R03 提出的“单行核无消费者初始化”属于它的后续范围。本轮只提供消费者对应，
不删除初始化、不调整 blockCount、不实施 R03 策略。

## 停止位置与事件

尚缺的具体依据是 R04 经 Main 转交的 DAV_2201/910B3 bank 映射及访问粒度来源，
以及该来源能否说明本节消费者地址差值会影响 Mul 的实际访问。
若资料不足，保持本研究结果，向 Planning 提交范围复核；不扫偏移参数，
也不重复 R04 探测。若资料充分，再选一个有机制依据的偏移值，重读规则、
声明真实 Revision，在设备 1 执行完整 Compile→Correctness→Local 循环。
未来 Local 必须先保留同二进制 Parent 数据，再交错采样；本轮没有新计时。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R05-PARAM-OUTPUT-CONSUMERS-20261008
AGENT_ID=01a119cf-3c06-7ea3-afed-1990f75245ee
ROUTE=W4-R05
EVENT_CLASS=SOURCE_RESEARCH
REVISION=NONE
LAST_KNOWN_REVISION=V001 (old research label; no performance Candidate)
DIRECT_PARENT=R31B-V011
STATUS=ROUTE_REVIEW_REQUIRED
RECOMMENDATION=NARROW
SOURCE_AXIS=NarrowMid FP32 gamma subview placement with unchanged allocations
HARDWARE_SUPPORT=UNCONFIRMED; R04 result not received
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
INHERITED_REFERENCE=R31B-V011
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
VERSION_RECORD_EVENT=NONE
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BLOCKER=NONE
STATE_SYNC_GAP=old long-ID research entry; short-ID state absent at supplied rules ref
EVIDENCE=研究/W4-R05/PARAM-OUTPUT-LAYOUT-STUDY.md
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=Main 转交 R04 已提交硬件来源后复核窄行 gamma 子视图轴；当前交还槽位，不改变 Route 生命周期
```

本轮仅新增本文件。未修改 Candidate、Parent、规则、共享记录、Dashboard 或其他
工作树；未访问 server3、未运行设备命令、未创建分支/工作树/子 Agent/定时任务。
本地 18 项源码位置断言与窄行子视图容量算式均通过；文档用词核对无命中。
本地只读分析命令均已结束，提交与最终工作树状态随交接回执给出。

## 2026-10-08 后续：V002 已完成，收益未确认

接手 Agent 为 `01a119ef-e539-7793-8f84-608d6ac528d0`，SLOT-1，继续使用原 R05 分支和工作树。
复用本报告的消费者与非重复性研究，并收到 R04 `b6890ad1` 的 2201 官方模型与 SDK 证据。
本次未重复检索官方资料或扫描偏移，选择唯一的 `s=64 float`。

设备 2 的地址探针调用原 Parent Init，D=129/2048/2056/4096 均回读 gamma=`0x8000`、
value=`0x20000`、gamma[64]=`0x8100`；其他分配起点与顺序模型相符。
官方 16-group 模型下，该偏移使两个完整 repeat 的读集合不重合。
容量上界为 `64+round_up(D,8)<=4160<8192`，两个 Load 与 Mul 共用偏移后的视图。
没有取得 VMUL 反汇编或冲突计数，没有实测 bank 译码；SDK 的 `ubbank_num=64` 差异仍为 UNKNOWN。

旧 V001 继续作为研究标签；新 V002 的唯一变化为 FP32 NarrowMid 的 gamma 子视图 +256 B。
Candidate 首次编译通过，双方六项独立 CPU FP64 reference 均通过。
16x2048 的 task 中位数为 4.82/4.54 us，16x2056 为 4.84/4.41 us；
两项均有超出要求的波动及先后次序反向，故 `MEASUREMENT_BLOCKED`，Local Best 仍为 NONE。
1x64 的 guard-OFF 输入为 1.50/1.52 us。所有 raw、失败与工具限制都在 V002 结果目录中。

R04 后续 `8c3b6659` 在本版 Local 完成后读取，给出的 `(s/8) mod16` 和 `s mod128=64`
关系与本次选择相符；没有为此增加版本或采样。main@285e7b4c 已登记 R05，仍待 Record 同步本次事件，
不再沿用旧规则对象中的“未登记”解释。

本轮新增性能版 1、有效 Local 0、连续无改善次数贡献 0，Online=PAUSED、PUSH=NO。
Parent-only 研究事件为 `W4-R05-GAMMA-ADDRESS-PARENT-20261008`；真实性能事件引用 `W4-R05/V002`。
当前没有确认第二个独立布局变化；后续优先复用现有库与 task-map 对齐双库驻留和先后次序条件，
不扫描新偏移，不自行改变 Route 生命周期。
