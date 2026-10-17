# W4-R02：五 tile 的选择性跨行遍历

## 本轮范围

仅操作 `w4/r02-selective-tile-traversal-x`。初始提交为 `de70b634813dea80783fc57716d6e95c158edeec`，初始工作树无未提交内容。Online=PAUSED，PUSH=NO。

最新历史登记来自 `w3/m1/record-owner@cecec26b`。其 R02 V001 行只有编辑前研究，Candidate、Compile、Correctness、Local、Git commit 均不存在。为保留该行并避免编号重用，本轮首个真实性能实验使用 V002。旧控制状态不作为停止理由。

## DUPLICATE_AUDIT

MECHANISM：只在 FP16、tileWidth=4096、tileCount=5、当前 batchRows=2 时，将 ProcessWideLowPrecision 的 pass-1 从 row-major 改为 tile-major。当前单元与下一预取单元使用同一映射。其他路径保留 Parent 映射。

SEARCHED_HISTORY：

| 来源 | 已读证据 | 对当前想法的约束 |
|---|---|---|
| 本 Route | 本地分支及所有本地 Git 对象中的 R02 路径；Record Owner 的 R02 V001 行 | 无真实性能版；旧研究仅否定 12288 列 |
| W3 R4，ref w3/m1/multirow-panel-rms，至 V031 | V003、V020、V031 完整 summary；V003–V031 的单变化和路径字段；Record Owner V014–V031 行 | V003：4096/3/3；V020：6144/2/2。其后宽度、预算及归约间距实验仍为 128×12288 |
| W3 R5，ref w3/m1/crossrow-full-pipeline，至 V028 | V002–V016 声明；V006 Parent/Candidate diff 与 Local 日志；V017–V028 登记；V001–V028 的实际 Local 命令字段 | V006 已测 16×16384 FP16、8 blocks、4096/4/2 的 tile-major；不能重做 |
| W3 R2，ref w3/m1/adaptive-core-ownership，至 V040 | V001–V016 Parent/Candidate diff；V017–V040 登记与提交序列 | 核数量、行归属及其排列；不提供五 tile 的遍历结果 |
| R031、R31A、R31B | 技术路线总表 R031 项、R031 原始源码、R31A/R31B 全版本机制登记；R31B V011 Init、ChooseWideFullYRows、pass-1、Host dispatch | R31B V016 改 tile 宽度，后续改 store 或输入队列深度；没有当前选择性五 tile 条件的 Local 证据 |
| MIX、STORE、EPILOGUE | 本工作树和 Record Owner 的机制登记 | 分派、算术与写出机制；未覆盖当前输入遍历组合 |
| 相关 W4 | Record Owner 的 R01/R02/R03/R06/R07/R08/R10/R13/R14/R15 行 | 参数相位、输入次序、跨行预取、活跃核数等，未记录当前五 tile 的选择条件 |

MATCH_FOUND：4-tile 想法 YES（R5 V006）；5-tile 选择性想法 NO。

WHY_NEW_OR_DUPLICATE：R4 V003/V020 的已测 tileCount 分别为 3/2；R5 V006 为 4。本轮五 tile 的两行输入流具有不同的奇偶交替相位，且明确排除以上几何。复用历史 tile-major 映射，新增内容是未测的几何启用条件，不声称 tile-major 本身首次提出。

HISTORICALLY_UNCOVERED=YES，范围限定为上述可取得的源码与 Local 证据。隐藏测试点的 shape 未知，OFFICIAL_COVERAGE=UNKNOWN。

## 几何推导

Parent 常量为 176×1024 字节预算、4096 元素 tile、16 个 FP32 归约槽。FP16 两行路径的选择式为：

`need = rows × (2 × D + 64) + 4 × 4096 × 2 + 3 × 4096 × 4`

| D | tileWidth | tileCount | batchLimit | 最后 tile 元素数 | V002 条件 |
|---:|---:|---:|---:|---:|---|
| 12288 | 4096 | 3 | 3 | 4096 | false，已有历史，不重测 |
| 16384 | 4096 | 4 | 2 | 4096 | false，已有 R5 V006，不重测 |
| 18432 | 4096 | 5 | 2 | 2048 | 两行 batch 为 true |
| 20480 | 4096 | 5 | 2 | 4096 | 两行 batch 为 true |
| 22528 | 4096 | 6 | 2 | 2048 | false，负对照 |
| 24576 | 4096 | 6 | 1 | 4096 | false，单行对照 |

runner 保留历史 R4 的 128 rows、40 available cores。Host 实际 blockCount=40；8 个核各 4 行，32 个核各 3 行。前者是两个两行 batch，后者是一个两行 batch 加单行收尾。单行收尾保留 Parent 映射。均为明确声明的 Local 探针，不代表 Official 测试点。

两行五 tile 的 row-major 顺序为 (r0,t0)…(r0,t4),(r1,t0)…(r1,t4)；tile-major 为 (r0,t0),(r1,t0)…(r0,t4),(r1,t4)。每行归约槽的 tile 下标和最终 ReduceSum 输入次序不变。

## 验证约定

Parent=本工作树的 `线上结果/R31B/V011/submission.asc`；原样保存到实验目录 parent.asc。runner 与构建结构复用 W3 R4 V031 的已用实现，移除来源摘要计算，保留 device-event 计时边界、同步方式和输入模式。

Correctness：Parent 与 Candidate 分别对独立 CPU 公式进行比较。输入先按 dtype 量化，CPU 用 double 累积 RMS、按公式求输出并量化回输出 dtype。FP16 使用 atol=rtol=0.001，每个元素均须通过，非有限输出计失败。该标准取自题面/模板的本地历史记录，属于 Local reference 验证，不能写成 Official 通过。没有把 Parent 的输出充当 reference。

范围：18432、20480、22528、24576 列 FP16。未改变的 FP32/BF16 不在本次运行矩阵内。原 R4 的固定绝对容差 0.0025 不作为本轮通过依据。

Local：每个 shape 先取 Parent same-binary，45 warmups、21 samples × 4 blocks；再按 PC/CP 交错取 Parent/Candidate，各 84 个样本。计时循环外分配与搬运。主 shape=20480，尾 tile=18432，条件为 false 的对照=22528。保存全部 device_event_us、wall_us、顺序和设备负载。

数值解释预先固定：Local score=Candidate device-event 中位数；delta=(Candidate median / Parent median - 1)×100；同时报告配对差中位数。Parent same-binary 或任一侧 MAD/median>0.10、block 中位数相对范围>0.10 时，记 MEASUREMENT_BLOCKED，不宣布改善。仍照常完成已经允许的测量并保留所有样本。只有资源规则中的真实失败才停止对应服务器动作。

## 运行位置

本地：`本地实验/W4-R02/V002/`。

远端：`cann-server3:/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/`。已确认规范父目录存在，现有目录中未发现 R02 专用对象。本轮仅在此位置创建必要源码、build 与日志，不安装服务，不改变系统配置，不操作其他 Route。

设备：1。工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`，编译目标 dav-2201。开始时 HBM 65536 MB、使用率 14%，据此可用 56360 MB；各运行阶段另保留快照。服务器已有用户负载，未作任何停止或迁移动作。

## V002 实际结果

V002 已完成 Compile、独立 reference 比较和 Local。结果为 `MEASUREMENT_BLOCKED`；实测数值全部保留，不宣布提升，不更新 Local Best，`STAGNATION_INCREMENT=0`。本轮只实现这一个性能版本，不启动 V003。

Compile：首次在 2026-10-08T03:44:48Z 因工具链目录没有 `set_env.sh` 而退出，编译器尚未启动。留在 V002，只把构建环境设置改为历史使用的 PATH、库路径和 include 路径；Candidate 未再改动。第二次构建在 03:46:12Z 开始、03:46:32Z 结束，`COMPILE_RC=0`。两个日志均保留。

Correctness：FP16，128 rows，D=18432、20480、22528、24576。每个宽度分别运行 Parent 与 Candidate，共 8 次，均对独立 CPU reference PASS；每次 mismatches=0、nonfinite=0、max_abs_error=0.001953125。混合容差为 `error <= 0.001 + 0.001 × abs(reference)`，没有仅凭 Parent/Candidate 一致认定正确。实际输入为 runner 的固定确定性模式；未执行 FP32/BF16、其他输入分布或正式 Judge 验证。

Local：每个 shape 有 84 条 Parent same-binary、84 条配对 Parent、84 条配对 Candidate，共 756 条。无样本被删除。下表的 delta 使用两侧中位数之比；配对差是另一项统计量，二者不混用。

| FP16 shape | Parent same-binary 中位数 µs | 配对 Parent 中位数 µs | Candidate 中位数 µs | delta % | 配对差中位数 µs |
|---|---:|---:|---:|---:|---:|
| 128×20480 | 26.200001 | 26.230000 | 25.770001 | -1.753715 | -0.420001 |
| 128×18432 | 24.650000 | 32.410000 | 28.990001 | -10.552296 | -2.150000 |
| 128×22528 | 39.920001 | 35.629999 | 35.660000 | +0.084203 | +0.640000 |

主 shape 的配对 Parent/Candidate MAD/median 分别为 0.110941632、0.109817654，均超过预定 0.10。PC 次序的配对差中位数为 +0.200000 µs，CP 为 -2.540000 µs；四个 block 的配对差中位数依次为 +0.020001、-1.560001、-1.060000、+0.219999 µs，方向不一致。

18432 的 Parent same-binary block 中位数相对范围为 0.199594320，配对两侧 MAD/median 为 0.213514335、0.187650928。22528 的 same-binary MAD/median 为 0.212424885，配对两侧为 0.209093452、0.197980942。该条件未启用的对照也出现明显波动，当前数据不足以归因五 tile 遍历的速度变化。

Local 前后 FREE_HBM 均为 55705 MB；AICore 17%→6%，AIVector 21%→11%。host load 三个值由 60.24/58.47/54.20 变为 48.54/55.83/53.46。负载未被当作拒绝运行的理由；`BLOCKER=NONE`。这里的 `MEASUREMENT_BLOCKED` 只表示当前数值不能用于提升判断。

统计脚本使用所有原始观测；CV 使用标准差除以均值绝对值，配对差保留正负号。完整统计位于 `本地实验/W4-R02/V002/result.json`，可仅在本机运行 `python3 本地实验/W4-R02/V002/support/summarize.py` 复算，不触发任何设备操作。

## 实际入口与证据

服务器主机 `hwnput3`，用户 `lelinfeng`，SSH 入口 `cann-server3`。设备操作仅涉及前述 R02 V002 专用目录。

| 内容 | 入口或文件 |
|---|---|
| Compile 入口 | 远端 V002/support/build_server3.sh；CMake Release、dav-2201、parallel=4 |
| Correctness/Local 入口 | 远端 V002/support/run_server3.sh |
| Correctness 参数 | `build/r02_runner 1 D fp16 correctness-only SIDE 0 0 1 OUTPUT.tsv`；D 与 SIDE 见上述 8 个组合 |
| Parent same-binary 参数 | `build/r02_runner 1 D fp16 same parent 45 21 4 OUTPUT.tsv`；D=20480、18432、22528 |
| 配对参数 | `build/r02_runner 1 D fp16 paired - 45 21 4 OUTPUT.tsv`；相同三个 D，逐样本交替 PC/CP |
| 构建日志 | `本地实验/W4-R02/V002/logs/compile-01.log`、`compile-02.log` |
| 精度结果 | `本地实验/W4-R02/V002/logs/correctness-D-SIDE.tsv` 及各自 `.log` |
| 原始样本 | `本地实验/W4-R02/V002/local/same-parent-D.tsv`、`paired-D.tsv` 及各自 `.log` |
| 资源和结束记录 | `本地实验/W4-R02/V002/logs/validation-01.log`、`stages.txt`、各阶段 `.usages.txt`、`.load.txt`、`.npu.txt` |

Correctness/Local 命令于 2026-10-08T03:47:35Z 开始，03:48:54Z 结束，`RUN_COMPLETE RC=0`。全部前台命令已退出；结束后的本 Route runner/build/run 进程查询无匹配。`RUNNING_DEVICE_OPERATION=NONE`。已构建二进制留在远端 V002/build，供需要时复用。本轮没有清理文件、安装服务、改变系统配置、操作其他用户任务或写入共享记录。

本地 Parent 与 `线上结果/R31B/V011/submission.asc` 逐字节一致。Parent/Candidate 源码差异仅为 pass-1 的五 tile 两行启用条件，以及当前/下一单元的行列映射；没有修改 tile 宽度、行归属、归约顺序、事件或输出流程。

## 交付状态与后续动作

`DIRECT_PARENT=R31B-V011`；`CURRENT_LOCAL_BEST=NONE`（W4-R02 本轮没有新增可接受 Best），保留 R31B-V011。`OFFICIAL_SCORE=NONE`、`ONLINE_STATE=PAUSED`、`PUSH=NO`。结果、源码、支持文件及全部失败/原始证据随 V002 单一实验提交保存；实际 commit 与最终工作树状态由提交后的版本事件提供。

本次达到一个真实性能版本的完整闭环后返回 Main，不改变路线生命周期。首要后续动作是复用现有 V002 二进制研究主 shape 的 PC/CP 次序偏差：先对照 runner 的单次 event 起止位置与已存 PC/CP 样本；若 Main 再次安排设备时间，用同二进制 Parent 对照配合有限 kernel-duration 采集，区分 host launch 间隙与 kernel 时长。新采集必须使用独立输出路径，保留本次所有观测；本次不执行该采集，不创建后台任务。

仍值得审计的独立轴是分段遍历：每行连续处理两个 tile 后，再切换到另一行，对五 tile 的末段单独处理。该想法的历史覆盖状态仍为 UNKNOWN，尚不具备开版依据。精确研究入口是本版 ProcessWideLowPrecision 的单元映射、R5 V006/V028 与 R4 V021–V031 的 Candidate diff；先确定是否已有同等分段映射及对应几何，再决定是否构成独立 OFAT 实验。隐藏测试点覆盖和 Official 相关性均未确认。

## 2026-10-08：V002 计时范围补充，采样前声明

本次接手从 `60ca277d2afc3d985965e16ca09d1ff9b03f8c10` 开始；工作树干净，
唯一分支仍为 `w4/r02-selective-tile-traversal-x`。本节为同版研究，新增性能版 0。
Parent、Candidate、两份 kernel 库、旧 result.json 和 756 条旧样本全部保留。
不创建 V003，不重做 128x12288、R4 V003/V020，不推进分段遍历。

规则已从自己的工作树完整读取指定 `9f918955` 的九项入口并发送
`RULE_REFRESH_RECEIPT`；本地 AGENTS 和 Route Skill 也已读取。
共享事实另从 `main@a7d3f04c` 的三份 TSV 读取：V002 已登记，Local Best 为 NONE；
表内排队阶段尚未反映本次接手，按用户最新分配使用 SLOT-2、device 1。
工作树无性能 Skill 副本，已读安装版 ops-profiling 及其 msprof-guide 的适用部分。

### DUPLICATE_AUDIT

- MECHANISM：原 P/C 框架内双方调用同一 Parent，区分 task、自身 event 间隔与位置。
- SEARCHED_HISTORY：复用上文已完成的 R02、W3 R2/R4/R5、R31/R31A/R31B、MIX、STORE/EPILOGUE 和 W4 历史研究；本次没有新性能概念，不重做已完成的几何审计。另读 R11 `1b528176` 的 RESULT、probe.cpp、host_diagnostic.sh、统计映射；R10 `1441fc72` 和 R12 `a018f702` 的 task/event 方法。W3 当前端点分别为 `6321ad44` V040、`ce6c6dc5` V031、`1efa0863` V028。
- MATCH_FOUND：方法已有来源；R02 当前尚无同框架 P/P 的逐调用 task/event 数据。
- WHY_NEW_OR_DUPLICATE：新增本 Route 的时间范围证据；不重新实施旧 tile-major 变化，不把跨 Route 测量结果当作 R02 资格。

### 已有输入路径与次序

已直接读取 Init、ChooseWideFullYRows、ProcessWideLowPrecision、host launch 和
Parent/Candidate diff。128 rows、availableCoreNum=40 对应 host blockCount=40；
8 blocks 各 4 行，32 blocks 各 3 行。18432/20480 的 tileCount=5、batchLimit=2，
两行 batch 启用 V002；单行尾批不启用。22528 的 tileCount=6、batchLimit=2，
V002 条件为 false。三者都进入 wide low-precision 路径。

原配对数据每输入 84 对，PC/CP 各 42 对；没有遗漏样本。

| D | P 第一/第二位置中位 us | C 第一/第二位置中位 us | PC/CP 配对 C-P 中位 us | 第二减第一次配对中位 us |
|---:|---|---|---|---:|
| 20480 | 25.210000 / 29.640001 | 25.450001 / 25.960000 | +0.200000 / -2.539999 | +1.509999 |
| 18432 | 30.210000 / 37.209999 | 27.370000 / 30.510001 | +0.280002 / -5.730002 | +2.149999 |
| 22528 | 31.930000 / 42.460000 | 35.550000 / 36.920000 | +3.159999 / -1.199999 | +2.309999 |

三种输入的次序差均可反向，包含未启用条件的 22528。旧数据只能说明位置相关现象，
不能判定其全部来自 host 或 kernel。旧 20480 的 -1.753715% 仍为观察。

### 固定采样与评价

本次只新增主输入 `128x20480 FP16`，使用原确定性输入 ID 和 epsilon。
18432/22528 只做旧样本分析，不新增上板运行。原 reference 的 8 次 PASS 保留；
本轮以未修改的 Parent/reference 作为前置证据，P/P 采样后再读回现有输出做
同一独立 CPU 公式的逐元素比较，不增加额外 kernel 调用。

原 R02 runner 的两侧共用一个 `deviceOutput`。本次保持共享地址关系，并在计时前
记录两槽地址、输入地址、stream/event 的 host handle；实际 blockCount 从 profiler
的 Block Dim 读取。旧采样实际指针未记录，填 UNKNOWN，不能把本次地址追认给旧采样。
R11 的两地址条件不适用于 R02，不另分配输出。

只给原 runner 添加 `paired-parent`：P/C 作为逻辑槽位标签，两个函数均为
`run_kernel_parent`，每条 raw 的 source_id 均为 R31B-V011。只原位构建
`build/r02_runner` 的 host 对象与链接；使用原生成的 build.make、flags.make 和
link.txt，不构建 kernel，不新增 host 优化选项，不创建第二套 runner 源码。

固定顺序：P 槽 45 次预热并逐次 stream 同步，再 C 槽 45 次；4 blocks×21 对，
逐对 PC/CP 交替，各槽 84 样本，共 168 个计时调用。总 kernel 调用预期 258，
前 90 个只为预热。start event → 单次 launch → stop event → synchronize stop
的范围不变；wall clock 仍在第一次 record 之前开始，在同步完成后结束。
每个计时调用后仍用原 WriteSample 写一行，每 block 末尾 fflush；不改打印或写盘节奏。

仅一次有限 `msprof --ai-core=off --task-time=on --ascendcl=on --runtime-api=on
--aicpu=off` 采集 P/P。每个计时调用对应 op_summary、task_time 和相邻 EVENT_RECORD；
记录 device/stream/task ID、Block Dim、kernel 时间、event 时间、wall 时间、
start-event 到 kernel、kernel 到 stop-event、第一/第二位置、PC/CP 和分组。
无法直接观测的字段填 UNKNOWN；event-task 差不全部指定给某个 host API。

原两项阈值不变：MAD/median<=0.10、组中位数相对范围<=0.10。event 与 kernel-task
分别对所有 168 样本、P 槽 84 样本、C 槽 84 样本报告这两项。只有二种时间范围的
完整集合及两槽均通过，才用相同参数和框架对既有 V002 追加一次有限 P/C；否则
结束设备采样，本轮 Candidate score/delta=NONE。所有首样本、长尾和分组进入统计；
不挑子集，不重跑到通过，不改变接受阈值。P/P 槽位差只作诊断。

证据位置为 `本地实验/W4-R02/V002/timing-scope-20261008/`；远端位于原 V002
目录下同名子目录。本次 SSH 确认既有源码与本地一致、两库存在；磁盘可用 352G，
无本 Route 运行进程。首次只读查询的 `npu-smi info -i 1` 返回用法错误，已使用
其支持的 `npu-smi info` 取得设备/进程信息；该查询没有启动实验。
device 1 usages 为 65536 MB、22%，按既有公式空闲 51118 MB；采样前后另存快照。
已有用户任务继续运行，无独占要求，不创建后台采集或定时任务。

## 2026-10-08：同框架 P/P 实际结果与交接

### 当前需求与状态

限定的计时归因已完成，结果为 `MEASUREMENT_BLOCKED`。按采样前声明停止设备采集，
没有执行本轮 P/C，没有新增性能版本；V002 原完整结果继续保留。本轮
`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`CURRENT_LOCAL_BEST=NONE`。
原 V002 的 -1.753715% 仍仅为旧观察，不提升为有效结果。

### 本轮实际完成与操作对象

原位扩展 `本地实验/W4-R02/V002/support/runner_main.cpp` 的 P/P 模式、计时前地址
输出和采样后 reference 比较；`LaunchAndMeasure`、输入生成、WriteSample、预热同步、
PC/CP 排列与 block 末尾 fflush 均未改变。扩展原 `support/summarize.py` 的
`--timing-scope pp` 离线入口；新增 `support/timing_scope_server3.sh` 作为有限命令入口。
全部新样本、设备快照、导出和分析位于同版 `timing-scope-20261008/`，研究结果追加于本文。

远端始终为 `cann-server3:/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/`。
06:09:06–06:09:08 UTC 只构建原 host runner，返回 0；原生成的 flags.make 只有
`-std=gnu++17`，本次原样使用。原 `build/r02_runner` 更新为 54640 字节，时间
06:09:07；两 kernel 库分别仍为 530176/530216 字节、03:46:30 原时间。
Parent、Candidate、两个 entry 文件和构建配置未改，未重建 kernel，未建立另一个 runner。
实际传输的 host 源码和脚本与远端逐字节一致，见 `transfer-source.log`。

唯一一次 P/P profiler 于 06:10:34–06:10:47 UTC 完成，返回 0，导出成功。
168 个计时调用全部执行同一个 Parent 函数，P/C 仅表示逻辑槽位。
实际输出地址均为 `0x12c041e00000`，输入指针与 stream/event handle 已保存在
`pp-event.tsv` 头部。实际 Block Dim=40，device=1，stream ID=45。

### 验证结果

旧 Parent/Candidate 在四个 FP16 输入上的 8 次独立 reference PASS 继续有效。
本次 P/P 采样后，读取共享输出并按原 CPU double RMS 公式逐元素比较：
max_abs_error=0.001953125、mismatches=0、nonfinite=0，atol=rtol=0.001，PASS。
这一次新增比较只覆盖采样结束后的 Parent 输出，不声称逐调用验证或重新执行 Candidate。

全部 258 条 op_summary 与 task_time 的 device/stream/task ID、开始时间、时长一致；
90 次为原预热，168 次与 raw 样本及前后 EVENT_RECORD 一一对应。task_time 共 683 条，
所有行保留。ACL elapsed 与两事件开始时间之差的最大偏差为 0.024001 us；逐条差值
保存在 `pp-task-map.tsv`，没有把舍入差用于筛样本。

原统计脚本默认输出与旧 `result.json` 完全一致。脚本语法、完整映射和新统计均通过；
新样本无遗漏，分析日志见 `analysis.log`。原始 756 条旧样本、旧 reference、失败日志
和旧结果文件未改。

| 时间范围/样本 | n | 中位 us | MAD/中位 % | 四组中位 us | 组中位相对范围 % |
|---|---:|---:|---:|---|---:|
| event，全部 | 168 | 32.530001 | 20.565636 | 31.260001 / 30.680000 / 39.959999 / 31.640001 | 28.527510 |
| event，P 槽 | 84 | 32.970000 | 21.565059 | 30.680001 / 30.200001 / 45.940001 / 37.039999 | 47.740370 |
| event，C 槽 | 84 | 32.470001 | 20.172469 | 33.679999 / 36.320001 / 35.999998 / 28.300000 | 24.699725 |
| kernel task，全部 | 168 | 23.362000 | 10.110436 | 22.380000 / 23.470000 / 23.882000 / 24.610000 | 9.545416 |
| kernel task，P 槽 | 84 | 23.540000 | 10.790144 | 22.780000 / 21.980000 / 27.780000 / 24.840000 | 24.638912 |
| kernel task，C 槽 | 84 | 23.352000 | 9.986297 | 21.464000 / 26.040000 / 22.640000 / 23.920000 | 19.595752 |

kernel task 的全部样本 MAD 略高于原 10% 要求，两个槽位又分别存在较大的组间变化；
不能只拿总体组间范围或 C 槽 MAD 单项宣布可靠。event 同样未通过。完整均值、标准差、
CV、MAD、p10/p90/p95、范围、各组与槽位统计见 `pp-summary.json`。

P/P 的两槽中位比差为 event -1.516527%、task -0.798641%；逐对百分比差中位数为
event -4.156538%、task +0.140865%。这些数值全部来自同一 Parent，不能当成 Candidate
成绩。两种统计量没有混用。全部 task 配对绝对差 p90=31.9632 us，未删除长尾。

### 时间范围、位置和长尾

| 范围 | P 第一/第二中位 us | C 第一/第二中位 us | PC/CP 槽位 C-P 配对差中位 us | 第二减第一次配对差中位 us |
|---|---|---|---|---:|
| event | 31.390000 / 36.179998 | 30.380000 / 34.490001 | -1.090001 / -2.650000 | +1.060001 |
| kernel task | 23.760000 / 23.180000 | 22.540000 / 24.710000 | +0.090000 / +0.010000 | +0.010000 |

只按位置汇总，第一/第二次 event 中位为 31.06/35.69 us，task 为 23.082/24.00 us；
start-event 到 kernel 的间隔中位为 3.88/7.27 us。按每对计算的差与这些总体中位差
是不同统计量。当前 P/P 未复现旧 P/C 的 task 方向反转，仍存在明显分组和槽位变化；
不同时间窗口的数据不能据此还原旧 V002 的真实收益。

同次调用 event-task 差中位为 6.498001 us；start-event 到 kernel 的间隔中位为
6.05 us，kernel 结束到 stop-event 的间隔中位为 0.02 us。各项中位数不能相加。
以下两种长尾都存在，不能只去掉打印、flush 或某个 event 间隔就宣布解决：

| 计时序号/stream/task | 位置 | event us | kernel task us | 前间隔 us | 后间隔 us |
|---|---|---:|---:|---:|---:|
| 1 / 45 / 160 | P 第一 | 25.440000 | 21.260 | 2.720 | 1.480 |
| 33 / 45 / 256 | P 第一 | 90.700001 | 22.780 | 22.960 | 44.960 |
| 61 / 45 / 342 | P 第一 | 163.959995 | 158.040 | 5.904 | 0.020 |
| 71 / 45 / 375 | C 第一 | 69.519997 | 25.884 | 43.600 | 0.040 |
| 116 / 45 / 514 | P 第二 | 155.039996 | 140.604 | 14.420 | 0.020 |

第 33 次的主要扩张位于 kernel 之外，第 61/116 次的主要长尾位于 kernel-task
区间内；首样本并非最大值。这个区间包含设备执行期间可能存在的干扰，不等于已证明
某条 kernel 指令变慢。当前采集没有把这些间隔进一步归因到某个 host API、调度或
内存机制，也没有测到逐核时间。两槽共用地址，不能套用 R11 的双地址结论。

### 资源、证据和下一动作

server3 为 hwnput3、用户 lelinfeng，CANN 8.5.0.alpha002、Ascend 910B3 / dav-2201。
P/P 前后 usages 均为 65536 MB、使用率 22%，按项目公式推算空闲 51118 MB。
AICore 0%→1%，AIVector 2%→12%，host load1 57.05→66.27。快照只说明对应时刻，
不表示整个采样窗口空闲。其他用户任务未被改变；`BLOCKER=NONE`。

新证据位于 `本地实验/W4-R02/V002/timing-scope-20261008/`：

- `pp-event.tsv`：全部 168 个 event/wall 原始样本与真实指针。
- `pp-task-map.tsv`：全部计时调用的槽位、位置、task ID、前后事件和间隔。
- `pp-summary.json`：全样本统计、资格结论及旧三输入的次序补充。
- `pp-profile/PROF_000001_20261008061036312_MAHHPQNFROMGREBB/mindstudio_profiler_output/`：完整 CSV、runtime timeline 与辅助导出。更底层的原始采集保留在远端同名 PROF 目录。
- `compile*.log`、`pp*.log`、前后设备/负载快照、`analysis.log`、`transfer-source.log`、`final-state.log`：执行与验证证据。

06:19:12 UTC 的远端记录确认 R02 runner、构建和采集进程均为 NONE，原 kernel 库
仍保留原大小与时间。没有删除文件、创建后台任务、改变规则/共享 TSV/Dashboard，
没有进入其他工作树。全部本次命令结束后交还 SLOT-2，由 Main 处理 Agent 槽位。

本轮新增性能版 0、有效 Local 0、连续无改善贡献 0、STAGNATION_3=NO。
OFFICIAL_SCORE=NONE、ONLINE_STATE=PAUSED、PUSH=NO。本轮研究事件和 V002 同版
补充事件在提交后提供实际提交号，等待 Record 异步同步；这不改变 Route 生命周期。

下一动作：Main 复核并转交本次事件。若后续继续 R02，先只读本次 runtime timeline
与 `pp-task-map.tsv`，从 task 256/342/375/514 对应的 API 时段和相邻调用间隔取证，
形成有区别的有限计时设计；不无变化重采本次命令，不先改打印/flush，不先开 V003。
V002 的有效性能结论仍缺；分段遍历的历史覆盖仍为 UNKNOWN，本轮没有推进该方向。
