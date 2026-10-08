# W4-R09 同步依赖与历史核对

日期：2026-10-08。仅负责 W4-R09；Online=PAUSED，PUSH=NO。

## 起点

工作树为 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R09-event-barrier-min-x`，分支为 `w4/r09-event-barrier-min-x`，初始 HEAD 为 `de70b634813dea80783fc57716d6e95c158edeec`，开始时无未提交内容。该分支及指定规则提交均没有 R09 实验文件。此前 Revision、Local Best 保持 UNKNOWN。

本轮选择 `线上结果/R31B/V011/submission.asc` 作为首版 Direct Parent，理由是本工作树保存了该源码及其正式结果。`线上结果/R31B/V011/result.json` 记录 15/15、45.16。该数值仅说明历史 Parent；本轮没有 Official 结果。Parent 的实际副本位于 `本地实验/W4-R09/V001/Parent.asc`。

## 具体依赖

以下行号属于上述 Parent 源码。

| 字段 | H1：FP16 最后一个参数 tile 的 release |
|---|---|
| EVENT | `HardEvent::V_MTE2`，pass-2 的 `prel`，按 `useA` 选择 `prel0` / `prel1` |
| PRODUCER | `ProcessWideLowPrecision` 中对 `gammaLocal` / `biasLocal` 的 Vector 读取；最后读取为 FP16 affine 的 `Add(outputLocal, outputLocal, biasLocal, valid)` |
| CONSUMER | 非末次使用时，同一参数槽的下一次 MTE2 `Load`；最后一个 tile 之后只有尾部事件消费和下一批 pass-1 的输入加载 |
| SET_LOCATION | L3346，当前参数 tile 的全部行和输出等待之后 |
| WAIT_LOCATION | L3282/L3287，在复用槽前；L3354/L3357，在 pass-2 结束时 |
| RESOURCE_REUSE | `gBase=xBuf_`、`bBase=residualBuf_` 两组槽交替；下一批 pass-1 继续使用这些输入缓冲 |
| DEPENDENCY_REQUIRED | 在 Vector 最后读取参数与后续 MTE2 覆盖之间必须维持 V→MTE2 顺序；所有非末次 release、复用等待、每行同步和 MTE3 同步均保留 |
| REDUNDANCY_PROVEN | 每行 L3339 的 `SyncVToMTE2()` 已完成该行参数读取的 V→MTE2 顺序。最后 tile 没有同阶段后继加载；其 release 只在尾部被消费。该 slot 的旧事件已在上一 tile 的预取处消费并将布尔值设为 false。因此最后 tile 同时不发送新事件、不设布尔值，尾部就不会等待未发送的事件 |
| CORRECTNESS_RISK | 若将来删除每行 `SyncVToMTE2`、改变参数缓冲用途或改变布尔状态规则，需要重新证明。此次不改上述条件；不删除输出等待，不改 reduction barrier |

状态推导：首次使用槽时布尔值为 false；此后每次把该槽作为下一 tile 预取目标时，先消费旧 release 并置 false。当前 tile 结束时才置 true。最后 tile 省去该发送与置位，尾部仍消费另一个槽的有效事件。输出数据仍经过原有 `SyncVToMTE3 → Store → SyncMTE3ToV`。

在 server3 的 CANN `8.5.0.alpha002` 中读到：`aarch64-linux/tikcpp/tikcfw/impl/kernel_tpipe_impl.h` L436 的 `AllocEventID` 设置占用位；L450 的 `ReleaseEventID` 只清除占用位，没有隐式硬件等待。H1 不改变分配与释放数量，且不留下未消费的发送。

H0：删除或延后 pass-2 `MTE3_V` 输出等待。`outputBuf_` 是所有行共用的一份缓冲，下一行 `FromFloat` 会写它；直接删除缺少完成保障。延后到下次写入前已经由 R31B V017 测试，因此不建立该性能版本。

H2：删除 FP16 pass-1/reduction 的 `PIPE_V`。W3 R5 V005/V027 已测 retained-y copy 与 Cast 之间的 barrier；V026 已测 Add 后 barrier；V028 已测平方与 ReduceSum 之间的 barrier。本轮不重复这些改动。

## DUPLICATE_AUDIT

MECHANISM：仅 FP16 宽行 pass-2 的最后一个 tile，不发送其参数 `V_MTE2` release，不设置对应 pending 布尔值；由保留的每行同步完成数据依赖。

SEARCHED_HISTORY：

- 本分支 `本地实验/W4-R09`、`研究/W4-R09` 的文件及 Git 历史：开始时为空。
- `w3/m1/adaptive-core-ownership` / `6321ad44`：V001–V040 Parent/Candidate 同步行差异；没有同步点变化。
- `w3/m1/multirow-panel-rms` / `ce6c6dc5`：V001–V031 提交和源码，V003–V031 的单变化记录；相关 V005 将每行 `SyncVToMTE2` 移到参数 panel 结束，未省去末次 release。
- `w3/m1/crossrow-full-pipeline` / `1efa0863`：V001–V028 Parent/Candidate 差异；重点 V002/V003、V004、V005、V015、V026–V028。V015 提前发送 release，保留所有发送与等待。
- R31/R31A/R31B、MIX-A、STORE-EPILOGUE、EPILOGUE-FUSE 的本工作树历史源码；`m1/r31a-exploit`、`m1/r31b-exploit` 的本地 Git 对象。R31B V017 为输出等待位置；V018 为 retained-y 写入；V019 删除每行及两阶段之间的 `SyncVToMTE2`，保留 `prel`。R31A V028 删除 affine 后、SetFlag 前的 `PIPE_V`。
- `w2/m1/sync-topology` 的 V001 diff：将当前参数 `MTE2_V` readiness wait 移到下一 tile 预取之后；没有删除末次 release。
- W4 R01、R08 的本地 Git 源码对象中，`prel` 仍按 tile 无条件发送；R07/R14/R15 的已确认本地分支和相关研究内容未提供这个改动。
- 在本工作树的研究、实验和归档文本以及本地 Git 提交主题中检索 `SYNC-BARRIER-ELISION`，未找到能绑定到具体源码的同名条目。名称覆盖存在这个限制；相关 barrier 改动已按上列真实源码核对，未把名称缺失当作新机制证据。

MATCH_FOUND：H0、H2 为已有机制；H1 未找到相同末次参数事件处理。

WHY_NEW_OR_DUPLICATE：H1 改动的是每批结束前的一个末次 release 发送/尾部等待，保留所有每行同步、所有后续还会复用槽的发送/等待。V015 改位置，R31B V019 改每行同步，均没有 H1 的生命周期边界条件。

## 验证范围

测试入口复用 W4 R01 的已编译 runner 结构和构建方式，通过本地 Git 对象读取后放入 R09 专用目录。新增独立 FP64 CPU reference：解码 FP16 输入，计算完整 RMS 与 affine，最后转 FP16；Parent 与 Candidate 分别比较。每个元素要求 `abs_error <= 2^-9 + 2^-9 * abs(reference)`，并要求最大绝对误差不超过 0.1；另外要求 Parent/Candidate 逐位相同。没有修改 reference 来配合 Candidate。

计划覆盖显式 FP16 proxy：`1x12288/b1`、`16x12288/b8`、`16x16352/b8`、`16x16384/b8`、`16x20480/b8`、`16x24576/b8`、`16x32768/b8`、`128x12288/b40`。涵盖奇偶 tile 数、尾部长度、单行、余数行和多批缓冲复用。它们不代表 hidden testcase。未改变的 FP32/BF16 路径不宣称经过本轮运行验证。

Local 采用原有 device-event 与 wall-time 边界、45 warmups、31 对交错 PC/CP 样本。先保存两个 Parent same-binary block，再保存四个 P/C block；显式性能形状为 `16x16384/b8` 和 `128x12288/b40`。保留全部 raw samples，不删异常值；噪声影响解释时报告 `MEASUREMENT_BLOCKED`，不以负载为由跳过已通过精度的 Local。

初次连接时间为 `2026-10-08T03:30:11Z`；主机 `hwnput3`，用户 `lelinfeng`，SSH 别名 `cann-server3`，设备 3 为 Ascend 910B3。初次 HBM 容量 65536 MB、使用率 11%、可用量按既有公式计算为 58327 MB，AICore 7%、AIVector 4%。运行前后继续保留设备和负载原始输出。

远端专用位置：`/home/data4t2/lelinfeng/cann/server_runs/W4-R09/V001`。首次只读查询时该目录不存在，也没有 R09 的已有设备任务；本轮只在该版本目录内构建和运行。不操作其他路线目录或进程。

## V001 结果

状态：`MEASUREMENT_BLOCKED`。单因素源码、Compile、Correctness、Local 与结果证据均已完成；本次占槽不继续 V002。路线生命周期不变，整条路线本轮最多 10 个新性能版本的要求不变。

- Compile：首轮失败于编译器生成的 host 注册代码找不到 `vector`；在同一版本给运行脚本补充 GCC 11 标准库路径后，Parent、Candidate、配对程序均编译通过。两次日志分别保存在 `compile.log`、`compile-attempt02.log`。Kernel 性能变化始终只有 H1。
- Correctness：预定八个 FP16 用例全部通过。Parent/Candidate 逐位差异均为 0；双方对独立 FP64 CPU reference 的逐元素失败数、非有限值数量均为 0，最大绝对误差 0.00390625。结论仅覆盖这些本地用例，没有正式 Judge 提交。
- Local：两个形状各取得 62 对 Parent same-binary 样本和 124 对 P/C 样本，共 12 张 raw TSV、372 对样本；每次均为 45 warmups、31 对交错 PC/CP。全部原始数据保留，包含 11549.420357 us 的 Candidate 大值。

| 显式 FP16 proxy | Parent 中位数 us | Candidate 中位数 us | 延迟变化 | Local score | 配对差值中位数 us |
|---|---:|---:|---:|---:|---:|
| 16x16384，blocks=8 | 20.340000 | 19.6899995 | -3.195676% | 103.301171 | -0.6100005 |
| 128x12288，blocks=40 | 27.600000 | 28.820001 | +4.420293% | 95.766825 | +2.6399995 |

延迟变化定义为 `100 * (Candidate median / Parent median - 1)`，正数表示更慢；Local score 定义为 `100 * Parent median / Candidate median`。它们是描述性测量统计，不是有效性能提升或 Official 分数。

Parent same-binary 两侧的 MAD/median 分别为：16x16384 的 23.13% / 22.51%，128x12288 的 14.54% / 12.46%。P/C 四组的配对中位数方向均有反转；128x12288 合并样本的 PC 配对差值中位数为 +4.7099995 us，CP 为 -2.710001 us。没有足够证据把变化归因于 H1。`CURRENT_LOCAL_BEST=UNKNOWN`，本版不计入 STAGNATION_3 的有效无改善次数。

Local 期间设备 3 的可用 HBM 始终为 60948 MB（按既有百分比公式计算），AICore 快照范围 0–18%，AIVector 0–13%，另有 Python 进程驻留。负载只作为背景记录，未取消 Local。

`2026-10-08T03:54:12.933249+00:00` 的进程查询没有发现 R09 运行进程；`RUNNING_DEVICE_OPERATION=NONE`。八次 Correctness 和十二次 Local/same-binary 运行均返回 0。详见版本目录的 `process-end.log`、`correctness.log`、`local.log` 和 `local-result.json`。

## 研究事件与后续动作

`ROUTE_RESEARCH_EVENT`：H0 输出等待位置属于 R31B V017 已测机制；H2 pass-1/reduction barrier 属于 W3 R5 V005/V026/V027/V028 已测机制。两项均为 `DUPLICATE`，没有建立新性能版本。

已完成性能版本事件的事实来源是 `本地实验/W4-R09/V001/local-result.json`，Git commit 由该文件所属提交及本轮最终 `VERSION_RECORD_EVENT` 提供。`PUSH=NO`，`OFFICIAL=NONE`，`Online=PAUSED`。

精确下一动作：由 Main 安排 R09 再次执行时，先只读分析本次已生成的 Parent/Candidate 设备代码，定位该 `V_MTE2` 发送、尾部等待与新条件分支，确认生成指令的实际差异；不改源码，不重复已完成的八个精度用例。随后再决定是否需要新的测量信息。

剩余独立同步点目前为 `UNKNOWN`。可研究 `ProcessNarrowMidOverlap` 的末次 `inputRelease` 生命周期（Parent L499–619），先对照 MIX-A V007 与 ASYNC-OVERLAP 历史，证明资源复用与事件消费均安全且机制未测，再决定能否声明下一版。这里仅给出研究方向，不宣称它已获安全或独立性证明。
