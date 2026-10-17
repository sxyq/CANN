# W4-R12：Parent-only tiny 测量

本次任务已完成。`1x64 FP32` 的 kernel-task 同二进制数据在本次采集窗口内通过
既有稳定性要求；device-event 数据仍为 `MEASUREMENT_BLOCKED`。
没有创建 Candidate、性能版本或 Local Best。Parent 时间不作为 Candidate 成绩。

## 来源与执行范围

```text
AGENT_ID=01a119c3-61de-7a42-9bed-c6b142253710
ROUTE=W4-R12 TINY-DISPATCH-MINIMAL-X
SLOT=5
BRANCH=w4/r12-tiny-dispatch-minimal-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R12-tiny-dispatch-minimal-x
HEAD_AT_START=de70b634813dea80783fc57716d6e95c158edeec
DIRTY_AT_START=NO
DIRECT_PARENT=R31B V011
CURRENT_LOCAL_BEST=NONE
HISTORICAL_ENTRY=V001 NOT_READY，按用户说明保留为历史研究登记
REVISION=NONE
ONLINE=PAUSED
OFFICIAL=NONE
PUSH=NO
```

完整读取工作树中的 AGENTS 与 Route Skill，再从自己的工作树用 `git show`
读取 `9f91895506023d917637f707bb3f61cd9d9f8765` 的 AGENTS、Route Skill、
W4 控制文件、实验总则、执行约定、服务器实验规范、本地性能测试规范、Git 工作流程
及资源脚本。已发送 `RULE_REFRESH_RECEIPT`。本工作树未带性能/精度 Skill，
使用已安装的 ops-profiling、ascendc-env-check、ops-precision-standard，
并读取 msprof、浮点精度和 CPU 参考的相关说明。

实际输入来自 `584c590c` 的
`本地实验/TINY-MINIMAL-KERNEL-CHAMPION-X/V001/support/paired_runner.cpp`：
`target-r1-d64-fp32`、MakeInput 的 sin/cos 公式、epsilon=1e-5、availableCoreNum=8。
R3 V010 继续使用该输入。没有从 Official case ID 或耗时猜测 shape。

Parent 直接 include 本工作树已提交的 `线上结果/R31B/V011/submission.asc`。
没有另存本地 Parent 副本，没有编辑 Parent。传输到本 Route 远端目录后，
用 `cmp` 对实际传输内容逐字节核对，返回 0。未计算额外内容摘要。

## 重复性核对

```text
DUPLICATE_AUDIT
MECHANISM=Parent-only 测量方法验证；不改变内核
SEARCHED_HISTORY=R3 V001–V010；W2 tiny H1–H5；W3 R2 到 V040、R4 到 V031、R5 到 V028；R31A/B、MIX、STORE/EPILOGUE 相关记录；R08、R14、R11 测量证据
MATCH_FOUND=旧 tiny 固定开销、循环、参数缓存、tile 容量和 event 复用已有研究或版本
WHY_NEW_OR_DUPLICATE=新增独立 CPU reference、实际 launch、P/P 与 task/event 对应证据；不重复实施旧性能变化
```

W3 最新读取端点为 R2 `6321ad44`、R4 `ce6c6dc5`、R5 `1efa0863`；
没有将旧共享表的 V016/V013 当作末版。R3 `584c590c` 保留至 V010，
其 V010 Parent event 中位 35.75 us、MAD/median 54.238%、组间漂移 56.671%，
仅作历史定位，未重复运行旧版本。

方法来源包括：

- `fa9b19bb`：R08 的 `RETEST-20261008.md`，event 与实际 task 尚需分离。
- `fcbd1814`：R14 的 `PARAM-MTE2-FINDINGS.md`，Parent P/P 与时间范围差异。
- `4243f4e9`：R11 V002 `RESULT.md`，直接包已有二进制的有限 task-time 方法。
- `990a6a37`：W2 tiny `TRACK-B-HYPOTHESES.md`，旧固定开销想法的来源和限制。

这些路线的 shape、精度、时长与得分均未替代本次 tiny 结果。
本次阅读也不代表已排除所有未来性能想法的重复；下一想法仍须针对实际改动核对。

## 构建、精度与真实 launch

远端唯一目录：

```text
/home/data4t2/lelinfeng/server_runs/W4-R12/parent-tiny-20261008/
```

server3 为 `hwnput3`，用户 `lelinfeng`，SSH 入口 `cann-server3`，设备 4。
沿用 R3 成功的 CANN 8.5.0.alpha002 / Ascend910B3 / dav-2201 构建方式。
构建在 2026-10-08 04:39:58–04:40:20 UTC 完成，返回 0；仅生成 Parent 测量 runner。
同一个 `build/r12_parent_probe` 随后用于 event 和 profile，期间未重建。

独立 CPU FP64 参考为：

```text
y = double(x) + double(residual)
reference = y / sqrt(mean(y*y) + float32(1e-5)) * double(gamma) + double(bias)
```

逐元素容差为 `2^-16 + 2^-10 * abs(reference)`，同时要求绝对误差不超过 0.01，
全部 64 个元素通过，非有限输出失败。设备输出先填 NaN，再真正调用 Parent、
同步 stream、取回数据；两次进程的 failures 均为 0，最大绝对误差均为
`1.9249022242817659e-7`。全部输入、输出、参考值和误差保存在两份 reference TSV。
没有将 R3 的 Parent/Candidate 逐字节一致性称为独立 reference 验证。
离线再按 TSV 中的原 FP32 输入重算 CPU 参考，两次均通过；重算参考与记录值的
最大差为 `6.661338147750939e-16`。支持脚本语法与 Python 语法解析均通过。

该输入得到 `rowCount=1`、`rowWidth=64`、`blockCount=1`、`localRows=1`，
不命中多行分支或 D>128 的 NarrowMidOverlap，进入 generic cached-row 路径，
`cacheRow=true`、`cacheParams=false`。8 是 runner 的可用核数实参，未声称设备仅有 8 核。

profiler 实际保留 185 次 `_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff`
调用，全部为 `AI_VECTOR_CORE`、Block Dim 1、device 4、stream 495。
按 runner 顺序分别为 1 次精度、60 次预热、124 次计时；没有仅设置函数指针后读输出。

## 同二进制数据

每个进程只创建一次输入输出、stream 和两枚计时 event。分配、H2D、reference、
预热均在计时外；每次 event 范围只含一次 launch。两个 block，每 block 31 对，
P1/P2 与 P2/P1 交替；两侧使用同一函数与同一输出地址。P1/P2 是调用标签，
没有第二个内核。raw 数据先留在内存，采样结束后统一写入。

沿用 R3 的判据：全样本 MAD/median <= 0.10，两个 block 中位数之差除以全样本
中位数 <= 0.10。所有计时样本参与统计，未删极端值或选择最快一组。
完整均值、标准差、CV、MAD、p10/p90 和范围在相邻 JSON。

| 口径 | n | 中位数 us | MAD us | MAD/median | 两组中位数 us | 组间漂移 | 结论 |
|---|---:|---:|---:|---:|---|---:|---|
| 原 event | 124 | 6.260000 | 1.560000 | 24.9201% | 6.07 / 6.31 | 3.8339% | MEASUREMENT_BLOCKED |
| 采集期间 event | 124 | 5.780000 | 1.620000 | 28.0277% | 10.63 / 5.67 | 85.8131% | MEASUREMENT_BLOCKED |
| 同次 kernel task | 124 | 1.480000 | 0.060000 | 4.0541% | 1.48 / 1.47 | 0.6757% | PASS，仅限本次采集窗口 |

kernel-task 的 P1/P2 各 62 个样本，中位数均为 1.48 us；P2/P1 中位比差为 0%，
逐对差中位数为 0 us。两组各自 MAD/median 为 2.7027% / 4.7619%。
全部 task 的 min/max 为 1.38 / 1.76 us，p10/p90 为 1.40 / 1.56 us，CV 为 4.5275%。

本次 P/P 差值绝对值的 p90 为 0.10 us，组中位差为 0.01 us，取两者较大值的
测量幅度为 0.10 us。它用于描述这次数据的分辨能力，未证明跨时段重复性，
不能预先保证更小变化可被可靠识别。

原 event 的 P1/P2 中位数为 6.37 / 6.17 us；采集期间为 6.43 / 5.43 us。
对应中位比差 -3.1397% / -15.5521% 均来自零变化二进制，全部保留为测量诊断，
不是 Candidate 提升。`LOCAL_SCORE=NONE`，`LOCAL_DELTA=NONE`。

## task / event 对应关系

唯一一次 profiler 命令使用：

```text
msprof --ai-core=off --aic-mode=task-based --task-time=on
       --ascendcl=on --runtime-api=on --aicpu=off
       --application="<R12_ROOT>/build/r12_parent_probe 4 <R12_ROOT>/results/profile"
       --output=<R12_ROOT>/results/kernel-task
```

程序与解析在 04:42:05–04:42:16 UTC 内完成，没有增加硬件指标组或后台任务。
op_summary 与 task_time 的 185 个 kernel 在 device/stream/task ID、开始时间和时长上
逐条相同。124 个计时调用各自位于同 stream 的前后 EVENT_RECORD 之间，
`profile-summary.task-map.tsv` 保存 ordinal、标签、task ID、event ID、各段时间和差值。

同次调用的 `event - kernel task` 差中位数为 `4.290000007 us`，最大值为
`210.580000138 us`。record 到 kernel 开始的间隔中位数为 2.64 us，kernel 结束到
后一个 record 的间隔中位数为 1.39 us。不同样本的中位数不可直接相加；
这些间隔未全部归因到某一个 host API、排队原因或设备资源。

ACL elapsed 与两个 EVENT_RECORD 开始时间差的最大偏差为 0.024006614 us。
初次离线分析要求二者在 0.0001 us 内相等，实际在第 63 次调用遇到 -0.020000164 us
而返回 1；错误保存在 `analysis-initial-failure.txt`。随后只调整离线分析，保留每条
实际差值与前后 event 关系，未改变采集数据或重跑 NPU。时间表示的舍入来源尚未进一步确认。

## 资源、文件与结束位置

所有阶段 npu-smi 均报告 HBM 容量 65536 MB、使用率 90%，按项目公式得到
FREE_HBM=6553 MB。ACL 在原 event 进程报告 6292.519531 MiB，profile 进程报告
6286.289062 MiB，均满足 100 MB 条件；两种数值口径分别保留。
原 event 前后 host load1 为 70.60 / 67.03，profile 为 50.50 / 49.03。
AICore/AIVector 前后快照均为 0%；这不代表采样期间始终空闲。
已有 VLLM 等进程继续运行，未被停止或修改，负载没有用于拒绝实验。

新增或修改的本地对象全部位于本目录：

- `parent_probe.asc`、`CMakeLists.txt`：只构建 Parent 测量支持代码。
- `remote.sh`：单次 build/event/profile 命令入口，无轮询或持续调度。
- `analyze.py`：全样本统计和 task/event 对应。
- `results/compile.log`、`event-session.log`、`profile-session.log`：实际阶段与资源输出。
- `results/event.raw.tsv`、`profile.raw.tsv`：共 248 条 event raw 样本。
- `results/event.reference.tsv`、`profile.reference.tsv`：各 64 条完整精度证据。
- `results/event-summary.json`、`profile-summary.json`、`profile-summary.task-map.tsv`：统计与逐调用对应。
- `results/profile-export/`：完整 185 条 kernel CSV、565 条 task CSV、timeline 与辅助导出。
- `results/analysis-initial-failure.txt`：首次离线分析失败事实。
- `results/final-state.log`：结束时资源与本 Route 进程核验。

远端只新增本 Route 上述目录，包含 source、同一 build 与原始 profile 数据。
所有构建、精度、测量、profile、传输命令均已结束。没有删除历史或失败证据，
没有修改共享 TSV、规则、Dashboard、Parent 或其他工作树。
04:48:23 UTC 的最终进程查询返回 `R12_ACTIVE_PROCESSES=[]`，runner 文件时间仍为
04:40:20 UTC，大小 478048 字节。历史 R3 V010 Parent 与本工作树原源码的 `cmp` 也返回 0。

## 研究事件与后续

```text
ROUTE_RESEARCH_EVENT=W4-R12-PARENT-TINY-TASK-20261008
AGENT_ID=01a119c3-61de-7a42-9bed-c6b142253710
ROUTE=W4-R12
REVISION=NONE
DIRECT_PARENT=R31B V011
COMPILE=PASS_PARENT_PROBE
CORRECTNESS=PASS_CPU_FP64_64_OF_64
PARENT_EVENT_STATUS=MEASUREMENT_BLOCKED
PARENT_TASK_STATUS=PASS_THIS_CAPTURE
PARENT_TASK_MEDIAN_US=1.48
PARENT_TASK_MAD_RATIO=0.04054054054054058
PARENT_TASK_BLOCK_DRIFT=0.006756756756756763
PARENT_PP_ABS_DIFFERENCE_P90_US=0.10
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
STAGNATION_INCREMENT=0
VERSION_RECORD_EVENT=NONE
OFFICIAL=NONE
ONLINE=PAUSED
PUSH=NO
RESOURCE_BLOCKER=NONE
RUNNING_DEVICE_OPERATION=NONE
```

本次交接不改变 Route 生命周期。具体提交号由提交后的回执提供。

下一动作：复用已有 Parent 构建产物，定位 `1x64 FP32` generic 路径的编译后
entry/Init/dispatch 控制序列，与 R3 V002/V004/V006–V010、R31A V009 和 MIX
V004–V006 的已提交变更逐项对照，证明是否还存在独立可测变化；未完成该证据前
不重复旧循环或固定开销想法。已有 P/P 与 task 对应方法可直接复用，不无变化重跑本次命令。

尚未覆盖 FP16/BF16、其他 tiny 输入、不同时间窗或正式调用方；host 提交间隙和设备
排队的具体贡献尚未分离。内核入口与分派开销是否值得一个新 Candidate，仍须编译产物
与历史变更支持。本次不声称拥有新的性能提升或 Official 结果。
