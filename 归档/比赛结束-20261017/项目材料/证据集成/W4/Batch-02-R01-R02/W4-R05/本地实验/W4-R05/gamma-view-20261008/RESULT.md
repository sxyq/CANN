# W4-R05 V002：NarrowMid FP32 gamma 子视图 +256 B

V002 已完成 Compile、独立 reference Correctness 和交错 Local。
结果为 `MEASUREMENT_BLOCKED`：两个受影响输入的 task 中位数下降，但单侧波动超出要求，
P/C 先后次序也会令配对差反向。没有确认性能收益，`CURRENT_LOCAL_BEST=NONE`。
本轮新增性能版 1、有效 Local 0、连续无改善次数贡献 0；不触发 `STAGNATION_3`。

## 身份、版本与范围

```text
AGENT_ID=01a119ef-e539-7793-8f84-608d6ac528d0
ROUTE=W4-R05 UB-BANK-PARAM-OUT-LAYOUT-X
SLOT=1
REVISION=V002
DIRECT_PARENT=R31B-V011
HEAD_AT_START=61e0aa28d0bdf439a879a171abece8477c806936
BRANCH=w4/r05-ub-bank-param-out-layout-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R05-ub-bank-param-out-layout-x
REMOTE=/home/data4t2/lelinfeng/server_runs/W4-R05/gamma-view-20261008
RULE_REF=9f91895506023d917637f707bb3f61cd9d9f8765
LAST_OLD_LABEL=V001; research only; no performance Candidate
INHERITED_REFERENCE=R31B-V011
CURRENT_LOCAL_BEST=NONE
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
```

开始和 V002 编辑前，均完整读取用户指定的规则及资源脚本并发出 receipt。
本树缺少相关技能副本，使用已安装的 API、性能与精度 Skill，读取所需 Buffer、
DataCopy、msprof、浮点精度与 CPU reference 说明。
共享事实另从 `main@285e7b4c` 的三份 TSV 读取：R05 已登记，阶段仍为旧 QUEUED，
没有本次结果。`STATE_SYNC_GAP=RESEARCH_AND_VERSION_EVENTS_PENDING_RECORD`；本 Route 不写共享记录。

Parent 直接引用本树 `线上结果/R31B/V011/submission.asc`，没有新增本地 Parent 副本。
远端首次盘点的两个 R05 已知位置均不存在，也没有 R05 在途进程，故使用上面的唯一实验目录。
构建与运行均使用 `cann-server3`、用户 lelinfeng、主机 hwnput3、CANN 8.5.0.alpha002、
Ascend910B3 / dav-2201，设备始终为 2。

## 唯一变化与非重复性

Candidate 只在 `ProcessNarrowMidOverlap` 获取 gamma 后增加：

```cpp
if constexpr (AscendC::IsSameType<T, float>::value) {
    gammaLocal = gammaLocal[64];
}
```

删除这三行后，完整 Candidate 与 Parent 源码逐字符一致。
所有 `InitBuffer`、x/residual、输出地址、DMA 数量和形态、事件、算术顺序、tile、ownership 均不变。
gamma 的两个 Load 和 FP32 Mul 继续使用同一局部视图；其他类型与分支不改。

```text
DUPLICATE_AUDIT
MECHANISM=NarrowMid FP32 gamma consumer subview +256 B with unchanged allocations
SEARCHED_HISTORY=61e0aa28:研究/W4-R05/PARAM-OUTPUT-LAYOUT-STUDY.md 的完整专项结果
MATCH_FOUND=NO_IN_READ_SOURCES
WHY_NEW_OR_DUPLICATE=旧宽行容量、输入布局和参数预载未改变本窄行消费者视图
```

复用原报告的 W3 R1 V001–V010、R2 至 V040、R4 至 V031、R5 至 V028、
R31/R31A/R31B、MIX、STORE/EPILOGUE 与相关 W4 核对，不重复搜索。
R2/R4/R5 的 ref 仍分别为 `6321ad44`、`ce6c6dc5`、`1efa0863`。
原报告关于部分 EPILOGUE 仅有机制记录的限制继续保留。

## 地址、模型与容量

复用 R04 `b6890ad1` 的报告、SDK 摘录和两张官方 8.5 原图，没有重新访问网页或取 SDK 资料。
官方 2201 模型为 16 个 group、每行 32 B，每个完整 256 B repeat 包含 8 个 DataBlock。
SDK 的 `ubbank_num=64` 与 196608 B / 4096 B 不一致，原因继续为 UNKNOWN。

`address_probe.asc` 在设备上调用未改的 Parent `Init`，通过 `LocalTensor::GetPhyAddr()`
回读地址。D=129、2048、2056、4096 的四次结果完全一致：

| 对象 | 实际回读 UB 字节起点 | 分配长度 B |
|---|---:|---:|
| x | 0x00000 | 16384 |
| residual | 0x04000 | 16384 |
| gamma | 0x08000 | 32768 |
| bias | 0x10000 | 32768 |
| xFp32 | 0x18000 | 16384 |
| residualFp32 | 0x1C000 | 16384 |
| valueFp32 / FP32 output | 0x20000 | 32768 |
| reduceFp32 | 0x28000 | 16384 |
| gamma[64] | 0x08100 | 原 gamma 分配中的视图 |

该探针只观测原 Init 和 Get 的地址，不是性能 Candidate，不参与计时。
原函数中的 TBuf 身份、Get、Load 与 Mul 数据流将这些地址对应到两个读操作数。
安装版反汇编器对实际设备 ELF 返回 `<not available>`，设备 LLVM IR 导出也未成功；
故本次没有取得 VMUL 指令级操作数或冲突计数，不能把地址探针写成完整的指令归因。

可证伪预测为：`group(A)=floor(A/32) mod16` 下，两读源的 group 相位差为 `(s/8) mod16`。
选定 `s=64 float`，对应 256 B，完整 repeat 的读集合重合数由 8 变为 0。
这只预测地址集合关系，不预测执行拍数或整核加速比；原地 Mul 的读写、参数 DMA 与 bias Add
也可能影响总耗时。没有实测 bank 译码。

128<D<=4096；64 是 8 的倍数，起点仍满足 API 的 32 B 对齐。
`64+round_up(D,8)<=4160<8192`，最大结束字节为 0xC100，小于 gamma 分配末尾 0x10000。
原单块 DataCopyPad 的目标跨度不变，全部落在原 gamma 分配中；没有跨越另一 TBuf。
32 B 对齐仅用于 API 合法性，选值依据是 256 B 读集合的相位关系。

R04 后续来源 `8c3b6659:研究/W4-R04/UB-BANK-EVIDENCE.md` 在本版 Local 已结束后读取。
其 `s mod128=64` 与本次已选值及回读地址相符，未用于另选偏移或增加采样。

## Compile 与独立 Correctness

Candidate 首次编译于 05:43:34–05:43:53 UTC 完成，返回 0。
Parent 的先行支持构建有两次失败，分别涉及默认 `-save-temps` 目录解释和 kernel 名称宏替换；
只改支持层后通过，未修改 Parent。失败日志与后续成功日志全部保留。
设备 IR 的失败尝试仅为编译期观察，未形成可执行替代 Parent，也未用于性能数据。

Correctness 在 05:44:37–05:44:45 UTC 完成，12 次真实 launch，返回 0。
输出先填 NaN，分别运行 Parent/Candidate、同步并取回结果；二者各自与 CPU FP64 reference 比较：

```text
y = double(x) + double(residual)
reference = y / sqrt(sum(y*y)/D + float32(1e-5)) * double(gamma) + double(bias)
abs(actual-reference) <= 2^-16 + 2^-10*abs(reference)
abs(actual-reference) <= 0.01
全部元素通过，非有限输出失败
```

| 输入 FP32 | 来源与覆盖 | 每侧元素数 | Parent/Candidate 最大绝对误差 | 失败数 P/C |
|---|---|---:|---:|---:|
| 16x2048 | W3 R2 V040；NarrowMid、每核一行 | 32768 | 6.468383553e-7 | 0 / 0 |
| 16x2056 | W3 R2 V040；同为受影响路径 | 32896 | 6.829999979e-7 | 0 / 0 |
| 1x64 | R12/R3 既有输入；guard OFF、generic | 64 | 1.924902224e-7 | 0 / 0 |
| 16x129 | 原 D>128 条件的下边界及尾部 DMA | 2064 | 3.745213446e-7 | 0 / 0 |
| 16x4096 | 原 D<=4096 条件的上边界 | 65536 | 7.067245029e-7 | 0 / 0 |
| 80x2056 | M=2x实测40核；每核两行、驻留参数 Load | 164480 | 7.291146074e-7 | 0 / 0 |

R2 输入沿用 `mt19937(314159+D)` 和原分布；1x64 沿用 R12 的 sin/cos 输入。
额外三项为明确的源码覆盖用例，没有映射或猜测 Official shape。
共验证 595616 个输出；另有编辑前 Parent 的 297808 个输出，完整数据保留为压缩 TSV。
传回后还原原始 FP32 输入并重新计算 FP64 reference，893424 条记录全部通过，
reference 与记录值的最大差为 0，详见 `results/reference-validation.log`。
未覆盖其他 dtype 或宽行，此处的 PASS 只限上述 FP32 用例。

## Local 方法与全部样本

复用 R12 `a018f702` 的内存缓存 raw、交替标签和 task/event 对应方法；同时采用 R11 `4243f4e9`
对同进程次序偏差的警示。没有沿用 R12 的 1.48 us 或稳定性结论。

每个输入先分配、H2D；60 次总预热在计时外。每进程复用 stream、两枚 event 和同一个输出地址，
两个 block 各 31 对，交替 P1/P2、P2/P1 或 P/C、C/P。每侧 62 个样本，每输入 124 个样本。
raw 和调用标签预留内存后收集，采样结束统一写文件；没有逐样本打印、写盘、主动暂停或删样本。
Parent 和 Candidate 由独立动态库载入，使用 RTLD_LOCAL 与 SYMBOLIC 链接。

两次有限采集都使用：

```text
msprof --ai-core=off --aic-mode=task-based --task-time=on
       --ascendcl=on --runtime-api=on --aicpu=off
```

Parent session 为 05:26:50–05:27:02 UTC，包含 6 次 reference、180 次预热、372 次计时。
P/C session 为 05:45:45–05:45:56 UTC，包含 180 次预热、372 次计时。
两次 op_summary 与 task_time 共 1110 个 kernel 在 device、stream、task ID、开始时间和时长上逐条相符。
744 次 timed call 都位于同 stream 相邻的 EVENT_RECORD 之间；完整调用顺序保存在 calls TSV。
ACL elapsed 与 event 时间戳间隔的最大差约 0.024003 us，逐条保留，没有为此丢弃样本。

### Parent same-binary

| FP32 输入 | P1 / P2 task median us | 合并 median us | P1 / P2 MAD比 | P1 / P2 组间漂移 | 配对绝对差 p90 us |
|---|---:|---:|---:|---:|---:|
| 16x2048 | 3.00 / 3.04 | 3.02 | 0.05333 / 0.05263 | 0.02000 / 0.05921 | 1.404 |
| 16x2056 | 3.10 / 3.16 | 3.13 | 0.05806 / 0.07911 | 0.02581 / 0.00633 | 1.214 |
| 1x64 | 2.37 / 2.36 | 2.36 | 0.03797 / 0.03390 | 0.02532 / 0.01695 | 0.878 |

这些 task 数据的 MAD/median 与组间漂移均不超过既有 0.10 要求。
但长尾令配对差 p90 较大，不能用稳定的中位数保证能区分小收益。原 event 三项均未通过稳定性要求。

### V002 Parent/Candidate

| FP32 输入 | Parent task median us | Candidate task median us | (C/P-1) x100% | P / C MAD比 | task 结论 |
|---|---:|---:|---:|---:|---|
| 16x2048，受影响 | 4.82 | 4.54 | -5.809129% | 0.13485 / 0.14758 | MEASUREMENT_BLOCKED |
| 16x2056，受影响 | 4.84 | 4.41 | -8.884298% | 0.08471 / 0.12925 | MEASUREMENT_BLOCKED |
| 1x64，guard OFF | 1.50 | 1.52 | +1.333333% | 0.02667 / 0.03947 | 当前 task 样本稳定；无收益结论 |

```text
LOCAL_SCORE=4.54 us observed; primary input 16x2048 FP32
LOCAL_DELTA=-5.809128630705396% observed
STATUS=MEASUREMENT_BLOCKED
LOCAL_BEST=NONE
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
```

| FP32 输入 | 全部配对 C-P median us | P先、C后时 C-P median us | C先、P后时 C-P median us |
|---|---:|---:|---:|
| 16x2048 | -0.328 | -1.680 | +1.220 |
| 16x2056 | -0.370 | -1.500 | +0.980 |
| 1x64 | +0.010 | 0.000 | +0.020 |

两个受影响输入的次序分组方向相反，波动又超出要求；0.28/0.43 us 的两侧中位差小于先前 P/P 的噪声幅度。
因此不采用负号作为优化成立依据。两个输入都受本变化影响，2056 不充当未改路径对照。

P/C event 的 P/C 中位数分别为 27.68/28.91、24.97/26.94、26.78/27.45 us，全部不稳定。
同次 event-task 差的中位数分别为 23.508、21.740、25.410 us。
这些间隔没有全部归因到 host API、排队或某一种资源，task 次序偏差也未定位到具体原因。
不因 event 单独不稳停止实验；本次已取得完整 P/C task 和 event 数据后形成上述结论。

完整 raw：`results/parent.raw.tsv`、`results/local.raw.tsv`；逐调用 task 数据：
`results/parent.task-map.tsv`、`results/local.task-map.tsv`。相邻 summary JSON 保留所有
均值、标准差、CV、MAD、p10/p90、范围、每 block 和每次序统计。未选择最快一次或排除极端值。

## 二进制与工具限制

Parent 首次成功构建的动态库为 530432 B。为了观察设备代码，05:27:56 UTC 使用
`llvm-objcopy --dump-section` 时未传独立输出文件，工具重写了输入 ELF，变为 530392 B。
此动作发生在 P/P 之后、P/C 之前；没有保留整库字节不变，报告不作该保证。
`results/parent-device-identity.log` 已确认：当前设备段、提取的原设备段与原编译 object 中的
44248 B 设备代码逐字节一致；字符表尺寸有变化。没有把这一结果扩大为整库字节一致。
Parent 没有重新编译，host runner 也未重建；后续 Candidate 使用同一支持程序。
该封装差异作为额外限制保留，不用于解释或消除已经观测到的时长波动。

反汇编输出、IR 导出失败和 Parent 支持构建失败均保留在 `results/`。
它们不构成 bank 冲突反证，也不计入性能版本数。地址证据最终来自实际编译运行的地址探针。

## 资源、操作对象与结束位置

所有阶段 npu-smi 均报告容量 65536 MB、使用率 9%，按项目公式估算 FREE_HBM=59637 MB。
P/P 进程的 ACL 可用量约 59045.12–59045.68 MiB，P/C 约 59044.51–59044.57 MiB。
P/P host load1 为 49.84→48.76，P/C 为 51.71→48.77；相邻 AICore 为 0–1%、AIVector 为 2–4%。
这些快照不代表整个计时区间始终空闲。已有 VLLM 等任务照常运行，没有改动它们或任何 lease。
未发生资源不足、OOM 或连接失败，`BLOCKER=NONE`。

本地新增对象均在本目录：Candidate、两个入口、类型头、CMake、runner、地址探针、
单次远端入口、统计脚本、本说明以及 results 下的完整时序、精度、profile 导出和失败日志。
仅更新 `研究/W4-R05/PARAM-OUTPUT-LAYOUT-STUDY.md` 的后续说明。
远端只操作本 Route 的上述目录，保留 source、build、原始 reference 与 profile 数据。
未删除任何证据或用户数据，未改 Parent 源码、规则、共享 TSV、Dashboard、其他工作树。

所有 NPU 命令已结束；05:49:25 UTC 的最终进程记录为 `R05_ACTIVE_PROCESSES=[]`。
最终提交与工作树状态见交接回执；本轮不 push、不自行改变 Route 生命周期。

本次新事件为 `W4-R05-GAMMA-ADDRESS-PARENT-20261008` 与 `W4-R05/V002`。
研究事件说明地址及 Parent 方法证据；版本事件单独记录唯一性能变化及无效测量。
Main/Record 尚需同步这两项及本次 commit。

下一动作：Main/Planning 复核 V002。若继续同版测量，复用现有库，先将 P/P 与 P/C 放入
相同的双库驻留、同一进程和相同输出地址条件，保留两种先后次序，针对本次 task-map 中的次序差作有限比较。
不重跑当前相同命令、不先开 V003、不扫描其他偏移。指令级冲突计数仍可作为后续归因来源，
本轮未采集。尚未确认第二个独立布局变化，参数/输出的其他消费者需要新的专项来源后才可声明版本。
