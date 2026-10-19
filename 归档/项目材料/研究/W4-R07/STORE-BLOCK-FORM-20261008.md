# W4-R07：单 tile 输出块粒度

日期：2026-10-08。Agent：`01a119d4-5351-7f41-a83e-3fe6ca27d181`。
本研究只讨论一次 MTE3 输出调用内部的块数与块长，不调整发起位置、队列深度或跨行传输。

结果：已完成一个真实性能版 V002，Compile 与双侧 CPU reference 均通过。
目标中位数 Parent/Candidate 为 21.5300005/20.64 us，观察差值 -4.133769%；
同二进制波动及 PC/CP 反向差异使结果记为 `MEASUREMENT_BLOCKED`，不认定提升。
全部 620 条计时样本保留；R07 的 Local Best 仍为 NONE。

## 当前范围与版本

工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R07-mte3-store-queue-x`。
分支：`w4/r07-mte3-store-queue-x`；接手 HEAD：`de70b634813dea80783fc57716d6e95c158edeec`，初始无未提交内容。
已完整读取工作树的 AGENTS、Route Skill，以及 `9f918955` 中指定的九项规则与资源入口；
在 V002 编辑前再次读取并发送 `RULE_REFRESH_RECEIPT`。
Online=PAUSED，PUSH=NO。未进入其他实际工作树，不写共享 TSV、规则或 Dashboard。

旧记录 `7093376d` 的 R07 V001 明确写着无 Candidate、无 Compile/Correctness/Local，
按本轮规则保留为研究记录，不计真实性能版。本次选用尚无源码或执行记录的 V002，
与旧研究编号区分。`285e7b4c` 任务表仍写 QUEUED，最新用户接手指令优先；
此处保留 `STATE_SYNC_GAP`，不等待共享表更新。R07 当前没有已接受的 Local Best。

## Parent 的实际输出语义

Parent 为本工作树 `线上结果/R31B/V011/submission.asc`，来自 `de70b634` 对象。
下列行号对应该文件。

| 位置 | 源码事实 | 本次边界 |
|---|---|---|
| 60–113、159–170 | D>8192 进入 wide；FP32 和低精度分开 | 只选择 wide FP16 |
| 81–95 | FP16 outputBuf_ 只有 tileWidth 个 half；完整 y 另存 gammaBuf_ | 不扩大缓冲，不把多行输出拼接 |
| 2161–2239 | wide FP32 已有两路输出事件；实际 Store 源为 valueLocal 的行内 tile | 注释中的 staging 描述不能代替实参；本次不改该路径 |
| 3313–3344 | FP16 完成归一化、转换、Mul、Add 后，经 V_MTE3 同步，Store(outputLocal, valid)，紧接 MTE3_V 等待 | 保持整个计算与事件顺序 |
| 3377–3383 | Store 用 DataCopyExtParams(1, count*sizeof(T), 0, 0, 0)，再调用 DataCopyPad | 一次调用、一块、精确有效字节数 |

FP16 输出值已经在原 outputLocal 中完成。所选变化不改算术、舍入、地址或有效元素数。
这里只改变 `ProcessWideLowPrecision` 的输出描述符，保留其他 dtype 与非 wide 路径。

## 所选单一变化及地址证明

当 T=half 且 valid=4096 时，原描述符为 `(1,8192,0,0,0)`，候选为 `(2,4096,0,0,0)`。
前一个数字为块数，第二个为每块字节数；两个 stride 均为零。
其他 valid 保留单块形式，尾部不拆分。一次 Store 对应的 API 调用仍为一次。

设 UB 源为 U、GM 目的为 G。候选覆盖：

```text
块 0：UB [U,U+4096)      → GM [G,G+4096)
块 1：UB [U+4096,U+8192) → GM [G+4096,G+8192)
总有效字节：8192
UB 所需范围：2 * AlignUp(4096,32) + (2-1) * 0 * 32 = 8192
```

两个 UB 块首地址均为 32 B 对齐，所需范围恰为 outputBuf_ 的 4096 个 half。
这与跨独立缓冲或跨行双块不同；没有第二个张量地址，也不越过当前 tile。
该推导证明地址覆盖相同，不证明两种描述符的硬件耗时相同。

2026-10-08T04:54Z 起通过 cann-server3 只读读取目标 CANN 8.5.0.alpha002 头文件。
共同前缀为 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/tikcpp/tikcfw/`：

| 文件位置 | 依据 |
|---|---|
| impl/dav_c220/kernel_operator_data_copy_impl.h:535 | Ext UB→GM 实现；half 分支向 copy_ubuf_to_gm_align_b16 传块数、块长和两个 stride |
| impl/utils/kernel_check_data_copy_overflow.h:482 | UB 源范围为 blockCount*AlignUp(blockLen,32)+(blockCount-1)*srcStride*32，并与 LocalTensor 容量比较 |
| interface/kernel_struct_data_copy.h:356 | 参数字段位宽，来源为已提交 R06 研究的目标工具链取证 |

补充来源：`bd825c5fd59c74c283478026e8b32702ba331eb5:研究/W4-R06/TRANSACTION-STRUCTURE-20261008.md`，全文已读。
R06 已对输入端的等分零 stride 块做过地址模型研究，没有输出侧性能实验；本次不把其模型当成 MTE3 测量。

## DUPLICATE_AUDIT

```text
MECHANISM=wide-FP16 single completed tile, one DataCopyPad with two contiguous equal blocks
SEARCHED_HISTORY=下表列出的已提交源码、差分及结果；其他分支均经本工作树 git show 读取
MATCH_FOUND=NO_EXECUTED_MATCH_IN_READ_OUTPUT_SOURCES
WHY_NEW_OR_DUPLICATE=输出单调用内 1x8192→2x4096；不改变 API 次数、发出点、事件、输入或跨行布局
```

| 来源与实际范围 | 本次已确认内容 | 与选定机制的关系 |
|---|---|---|
| R31B aef6e728：V001–V019 的 22 份主要源码；重点 V006/V009/V017 | Store 构造仍为单块。V006 增加输出并行及后移等待；V009 改输入双缓冲；V017 从 V016 后移输出完成等待 | 均未改单调用内部块粒度；不重复上述调度变化 |
| W3 R2 6321ad44：V001–V040，80 份 Parent/Candidate | Store helper 均为单块 Ext Pad | 已读范围内无同形式 |
| W3 R4 ce6c6dc5：V001–V031，31 份 submission | Store helper 均为单块；另读 V018 summary，实际变化为 pass-1 retained-y 写法 | 不把版本标题里的 writeback 当作 MTE3 变化 |
| W3 R5 1efa0863：V001–V028，56 份 Parent/Candidate | Store helper 均为单块；提交序列包含输出次序与事件研究 | 不重复次序或事件轴 |
| R31A 09a9c5ce：本地 V019–V028，10 份源码；MIX-A de70b634：本地 V006/V007，2 份源码 | Store helper 均为单块 | 更早源码范围复用 R06 已提交构造式取证，不把本次窄搜索称作全部历史 |
| R031 及较早 R31A/MIX | 复用 bd825c5f 中的 5/30/10 份源码取证与具体来源；本次另确认旧路径名称 | 仅复用已提交结果，不重读另一实际工作树 |
| STORE-EPILOGUE-X 8261c094：V001/V002/V003 diff、V002/V003 源码和两份交接；STORE-W2 53d9f04b：V001 diff、Run-004 | V001 整行合并；V002 增加 tileCount 条件；V003 K=2 行内大块并提前发出；W2 只移动整行 Store 位置 | 这些变化跨已计算 tile 合并或移动调用；本次在原时刻处理一个 tile，调用数不变 |
| EPILOGUE-FUSE 989fbe1c：V001–V003；EPILOGUE-ARITH e9056590：V001/V002 | Store helper 仍为单块 | 本次不改输出算术 |
| MULTIROW-DMA 0932fbd5：V002 diff、local-result、API-PROBE-RESULT | 同时将 Load 和 Store 的对齐传输改为非 Pad DataCopy；16 对记录中位 -1.84%，对照 -1.48%，旧结论 NEEDS_ONE_MORE_LOCAL | 原语选择已有覆盖；两端同时变化的结果不能单独说明 MTE3 收益。本次保留 Pad 原语 |
| ALIGN-TAIL-X ce4abd84：V001 diff | 对齐前缀 DataCopy + 尾部 DataCopyPad，两次调用；Parent 更早 | 本次保持满 tile 与一次调用，不重做 bulk/tail |
| W4 R07 7093376d；R06 b09e00eb/bd825c5f；R15 d13e51e9；R14 fcbd1814 | 旧 R07 研究只覆盖发起位置/深度；R06 为输入描述符；R15 为跨行；R14 为参数输入 | 本次唯一作用对象是当前 tile 的输出描述符 |

旧 STORE V002/V003 记录中有 Parent/Candidate 同时未通过 CPU expected 值比较的形状；
W2 Run-004 四次调用均 RC=3 且两侧重复输出变化。它们不能被写成全部 reference 正确，
也不能证明当前输出机制必然有问题。本轮使用当前 Parent 与独立 CPU reference 重新验证选定输入。
没有声称穷尽全仓、全部机制或未提交研究。

## V002 测试声明

复用 `ce6c6dc5:本地实验/MULTIROW-PANEL-RMS-CHAMPION-X/V031/support/` 的 runner、ABI 和构建形式。
保留输入生成、CPU reference、计时边界及 PC/CP 次序；只调整文件位置、目标名和来源标签，
去掉与测量无关的源码摘要输入要求。CPU reference 计算 FP32 加法与输出链、FP64 平方和，
最后按输出 dtype 舍入；FP16 沿用绝对容差 0.0025、rtol=0，所有元素及非有限值均参与判定。
它是 Local reference，不代表 Official 验证。

| 输入 | 来源与作用 | 预期路径 |
|---|---|---|
| 128x12288 FP16 | R4 既有真实代理；性能目标 | 40 个 block，8 个处理 4 行、32 个处理 3 行；tile=4096，3 tiles/行，batchLimit=3 |
| 128x8192 FP16 | 源码 D=8192 分派边界构造的对照 | 非 wide；候选变化不生效 |
| 128x12304 FP16 | 合成尾块输入：12288+16；仅验证精度 | 3 个满 tile 使用双块，末尾 16 half 保留单块 |

全部输入都在本轮明确给定；未从 hidden testcase 推测 shape。
目标每次 launch 仍有 384 次输出调用，总有效数据 3145728 B；描述符块数由 384 变为 768。
非 wide 对照应保留原代码。行归属、tile、UB 预算、参数预取、输出先后及事件不变。

优先设备 0。04:54Z HBM 容量 65536 MB、使用率 18%，估算空闲 53739 MB；
后续每个阶段以实际采样为准。没有修改其他用户进程、服务或 lease。
首次查找远端旧 R07 目录不存在；本版仅使用必要的新目录
`/home/data4t2/lelinfeng/w4-r07/V002`，不复用其他 Route 的运行目录。

Compile 通过后先执行 3 个输入的 Parent/Candidate reference 比较。
全部通过才采 Local：目标与对照各先做 Parent same-binary，45 warmups、31 samples、2 blocks；
随后各做 4 blocks 的交错 Parent/Candidate。保留全部 device-event 与 wall-time 原始数据。
波动不能支持判断时记 MEASUREMENT_BLOCKED，不提升 Local Best，不连续扫块长参数。

## V002 执行结果

性能差分只有 `Candidate.asc` 中 wide-FP16 输出处的一块变化。原 `Parent.asc`
保留 R31B-V011 源码。编辑后直接进入 server3 Compile，中间未加入文档或第二个性能变化。
2026-10-08T05:08:35Z–05:08:54Z 完成构建，`COMPILE_RC=0`，Parent/Candidate
两个库及 w4r07_runner 均已产生。构建证据为 `V002/logs/compile-a1.log`。

05:09:57Z–05:10:44Z 完成三种输入的双侧 reference 比较，六项均返回 0。
所有结果 max_abs_error=0.001953125、mismatches=0、nonfinite=0。
结果只覆盖声明的三个 FP16 输入；没有声称逐位相同、全输入正确或 Official 正确。
精度日志和六份逐项 TSV 位于 `V002/logs/correctness-a1*`。

05:11:45Z–05:12:15Z 完成 Local，返回 0。每个输入 62 个 Parent same-binary 样本，
以及 Parent/Candidate 各 124 个配对样本；两种输入合计 620 条，未删任何样本。

| 输入 | same-binary 中位 us | same-binary MAD/中位 | 配对 Parent us | 配对 Candidate us | 两侧中位数之比 delta |
|---|---:|---:|---:|---:|---:|
| 128x12288 FP16 | 21.6499995 | 19.0300% | 21.5300005 | 20.6400000 | -4.133769% |
| 128x8192 FP16 对照 | 14.4700000 | 20.2488% | 19.9100005 | 20.0300005 | +0.602712% |

目标的逐对差值中位为 -0.4599995 us，逐对百分比中位为 -2.614212%，
与两侧中位数之比是不同统计量。目标 68/124 对偏向 Candidate、56/124 对偏向 Parent。
目标配对 Parent/Candidate 的 MAD/中位分别为 21.3191%/17.4419%。

| 输入与次序 | Parent 中位 us | Candidate 中位 us | delta |
|---|---:|---:|---:|
| 目标，PC | 18.5600010 | 22.0700000 | +18.911632% |
| 目标，CP | 25.1099995 | 18.8600000 | -24.890480% |
| 对照，PC | 15.6600000 | 20.4499995 | +30.587481% |
| 对照，CP | 21.1200000 | 19.6199995 | -7.102275% |

次序相反时方向相反，未改输出代码的对照也出现同类现象。
这些数据无法分离单 tile 输出块粒度的影响；不把负 delta 当成有效提升。
设备负载仅作为解释信息：空闲 HBM 估算始终 53739 MB；AICore 1%→0%，
AIVector 4%→0%，host load1 87.19→68.13，device0 已有 python 进程 PID 3167028。
没有等待独占，没有停止、迁移或修改该进程。

结构化结果、各 block/次序统计和原始文件路径见 `本地实验/W4-R07/V002/result.json`。
四份完整 raw 文件为 `local-a1-12288-same.tsv`、`local-a1-12288-paired.tsv`、
`local-a1-8192-same.tsv`、`local-a1-8192-paired.tsv`，均在本版 logs 目录。

```text
ROUTE_RESEARCH_EVENT=W4-R07-STORE-BLOCK-FORM-20261008
ROUTE_EVENT=W4-R07/V002
VERSION_RECORD_EVENT=W4-R07/V002
STATUS=MEASUREMENT_BLOCKED
COMPILE=PASS
CORRECTNESS=PASS_BOTH_CPU_REFERENCE
LOCAL_SCORE=20.64 us, observed only
LOCAL_DELTA=-4.133769%, observed only
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=1
VALID_NUMERIC_LOCAL_RESULTS=0
CONSECUTIVE_VALID_NO_IMPROVEMENT=0
STAGNATION_3=NO
OFFICIAL=NONE
ONLINE=PAUSED
PUSH=NO
RESOURCE_BLOCKER=NONE
RUNNING_DEVICE_OPERATION=NONE
```

## 剩余问题与交接

本次已证明双块输出在三个声明输入上通过 reference；尚不能判断硬件耗时收益。
所选单 tile 块粒度仍是待分离的独立变量，不能据此宣称 R07 已穷尽。
精确下一动作：保持 V002，复用这两个已构建库、runner 和 raw；围绕相同
128x12288 FP16、40 block 输入取得 kernel-task 时间，与当前 device-event 区间对照，
并区分同一对中的第一/第二次执行位置。已算出的 PC/CP 分组可直接复用。
这一动作不需要新的 Candidate，也不按 2/4/8 块枚举新性能版本。

不重做原语替换、整行合并、发起位置、队列深度或 R15 的跨行 DMA；
未创建 R16，未实现 PASS2-REDUNDANT-VECTOR-X。只交还本次任务槽位，Route 生命周期由 Planning 决定。
本轮所有原子命令均已返回；提交编号和最终工作树状态随交接事件发送。

提交前已从四份 raw 逐项复算样本数、双侧及分组统计、逐对差值和百分比，
全部与 result.json 一致；六条精度记录也与各 TSV 一致。Parent 源码与
本工作树的 R31B-V011 逐字节相同。最后通过 cann-server3 查询专用进程名
`w4r07_runner`，返回 `RUNNER_PROCESS=NONE`；本次查询已结束。
