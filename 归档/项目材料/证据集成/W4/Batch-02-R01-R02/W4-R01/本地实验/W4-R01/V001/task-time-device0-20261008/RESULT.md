# R01 V001：同进程 P/P 与 task-time 结果

## 当前需求与状态

本次限定研究已完成。1052 次 kernel 与 profiler 逐项对应，其中 504 次 timed call
均有完整事件、输出地址、位置和 host API 记录。kernel-task 的中心稳定性达标，
原 event 范围仍不稳定；目标 P/P 配对噪声为 0.14 us，高于事先声明要辨认的旧观察差
0.06 us。因此 V001 保持 `MEASUREMENT_BLOCKED`，没有确认性能收益。

本次 Candidate 为 `NOT_EXECUTED`，`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`。
未创建 V002，新增性能版 0、有效 Local 0、连续无改善贡献 0。
沿用 Parent R31B/V011；W4-R01 没有已接受的 Local Best。Official=NONE，
Online=PAUSED，PUSH=NO。本次交接不改变 Route 生命周期。

## 本轮实际完成

复用初始 HEAD `93f15d9bab643fa6e8dbf6ed8833a144672d9fb3` 的 V001、原两库、
全部旧样本和四组 reference。规则来源为 `9f91895506023d917637f707bb3f61cd9d9f8765`；
共享状态读取来源为 `07662d7b96e9beaaa0f56d97c9cf346b86081eb3`。
初始工作树无未提交内容；所有本地命令均在本 Route 工作树执行。

采样前声明位于 `研究/W4-R01/TASK-TIME-20261008.md`。
只为原 runner 增加固定 task-study 模式、计时外地址记录，以及失败时保存部分样本。
原 `InputValue`、FP16 转换、`CpuReference`、`CompareReference`、`RunCorrectness`、
`Measure` 和全部数值常量均逐字符保持一致。Parent、Candidate、ASC 入口、ABI 与
kernel CMake 文件均未修改，验证来源为 `validation.log`。

06:38:00–06:38:02 UTC 仅编译 host，参数保持 `-O2 -std=c++17`，直接链接原两库。
06:38:28–06:38:42 UTC 完成唯一一次 msprof 应用采集与导出，两个阶段返回码均为 0。
没有源码性能变化、kernel 重建或追加设备实验。

两个输入在同一 PID 262386 中依次执行：16x16384 FP16、16x32768 FP16；blocks=8。
每输入 4 次原精度调用，然后三组，每组 45 对预热、42 对计时，PC/CP 各 21 对。
每个事件区间一次 launch，输入、分配、reference 与地址打印均在计时循环外。
每组结束后按原位置写 raw 和统计，没有逐样本打印、主动暂停或样本删除。

## 修改或操作对象

本地工作树和分支：

```text
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R01-selective-param-pipeline-x
BRANCH=w4/r01-selective-param-pipeline-x
```

- `本地实验/W4-R01/V001/support/runner_main.cpp`：原 host 的有限 P/P 模式。
- `本地实验/W4-R01/V001/support/analyze_requalification.py`：扩展原分析入口，
  对应 task/event、真实输出指针、位置与 host API；旧分析模式保留。
- `本地实验/W4-R01/V001/support/collect_task_time.sh`：本 Route 的 host 构建与单次采集入口。
- `研究/W4-R01/TASK-TIME-20261008.md`：事先声明、来源与后到线索处理。
- 本目录：完整六份 P/P raw、全部 op_summary/task_time、完整 timeline、PID/设备元数据、
  编译与采集日志、资源记录、`all-calls.tsv`、`result.json` 和验证结果。

远端只操作原有目录：

```text
cann-server3:/home/data4t2/lelinfeng/cann/w4/W4-R01/V001/
```

更新其中两个 support 文件及原 `support/build/w4r01_v001_paired_runner`，
新增 `task-time-device0-20261008/`。完整 profiler 原始数据保留于：

```text
task-time-device0-20261008/pp-profile/PROF_000001_20261008063830878_FMGNGBGCINIDCBMA/
```

本地取回原始导出、完整 raw 和必要元数据；profiler 的原始采集块和中间数据库仍在
上述远端目录。没有建立第二套 runner 或 kernel 实现，没有删除文件。
规则、共享 TSV、Dashboard、主工作树和其他 Route 工作树均未写入。

## 验证结果

### 输入、输出地址与同协议范围

库映射与函数地址来自本次进程自身；两库全程驻留，两槽实际函数地址相同，均为
`run_kernel_parent`。每个输入的三个组复用同一份输入、两个输出指针、stream 与 event。
第二个输入按原分配方式建立另一组对象；host stream 指针被运行时复用，profiler 的
stream ID 分别为 6 和 7，分析没有混同它们。

| FP16 输入 | P 槽设备指针 | C 槽设备指针 | profiler stream | 每槽样本 |
|---|---|---|---:|---:|
| 16x16384 | 0x12c0c0127000 | 0x12c0c0200000 | 6 | 126 |
| 16x32768 | 0x12c041600000 | 0x12c041800000 | 7 | 126 |

P/P 使用原 P/C 的 `RunLocal` 与 `Measure`，只将第二函数指向 Parent，保持槽位和
交错次序。旧实验各组使用独立进程，本次将三组合并到同一进程；不能称两个端到端
流程完全相同。本次没有执行 P/C，也没有形成已经实测的同进程 P/P→P/C 比较。

旧 P/P 与 P/C 间隔 529 秒，旧地址与 kernel-task 未采集；不能把本次 task 时间分配
给旧 event 样本。旧 24.08→24.0199995 us、-0.249169% 仍为观察，原文件原样保留。

### Reference

两槽在采样前与采样结束时都与原 CPU FP64 公式、最终 FP16 reference 比较。
atol=rtol=0.001，允许失配比例=0.001，所有输出有限。两个输入每侧分别有 1/3 个
严格超差元素，max_abs 都为 0.00390625，模板判定均为 PASS；两个 Parent 输出逐位相同。
采样结束时只读回已有输出，未增加 launch。

原四组 Candidate/reference 结果仍引用 `93f15d9b` 的
`requal-device2-20261008/result.json` 及四份 `correctness-template-*.log`。
按 16x16384、8x16384、16x32768、16x16352 顺序，每侧严格超差为 1/0/3/0；
没有改变舍入、容差或允许比例，也没有写成严格零误差。没有 Official 精度结果。

### 全样本时间分布

以下 P/C 是两个逻辑输出槽位，两侧均执行 Parent。每行每侧 126 次；所有样本均保留。
组间漂移按原要求取各组两侧中位数对双侧合并中位数的最大相对偏移。

| 输入 / 范围 | P / C 中位 us | P / C MAD比例 | 组间漂移 | 配对绝对差 p90 us | 原中心要求 |
|---|---:|---:|---:|---:|---|
| 16x16384 / event | 25.400 / 24.580 | 13.0315% / 17.5346% | 7.4370% | 11.5900 | 未通过 |
| 16x16384 / task | 8.800 / 8.780 | 0.6818% / 0.6834% | 0.2273% | 0.1400 | 通过 |
| 16x32768 / event | 26.800 / 27.650 | 14.8134% / 11.5009% | 10.4762% | 12.5600 | 未通过 |
| 16x32768 / task | 15.620 / 15.620 | 0.2689% / 0.3841% | 0.1280% | 0.1980 | 通过 |

两个输入的 task 三组合并中位数分别为 8.78/8.80/8.80 us 与 15.64/15.62/15.62 us。
每组双侧 MAD 比例均不超过原 10% 要求。全部 task 范围为 8.624–9.040 us、
15.420–15.940 us；本次没有 R11 那种幅度的 kernel 长尾，不能移用别处的现象。

`MEASUREMENT_FLOOR=max(p90(abs(P2-P1)), range(group medians))` 分别为
0.14/0.198 us。目标为旧观察差 0.06 us 的约 2.33 倍；这条事先声明的小信号要求未满足。
它是本次有限设计的判定，不是任何统计估计永远无法分辨小差异的证明。

| task 输入 | 两槽中位数比差 | 逐对百分比中位 | PC 次序的 C-P 中位 us | CP 次序的 C-P 中位 us |
|---|---:|---:|---:|---:|
| 16x16384 | -0.227273% | -0.229621% | -0.020 | -0.040 |
| 16x32768 | 0.000000% | -0.254615% | 0.000 | -0.040 |

目标同一个 Parent 已出现与旧小幅观察相近的百分比槽位差；这不能证明旧 Candidate
没有收益，也不支持把旧负号直接归给 V001。按槽位和第一/第二位置划分的完整统计
保存在 result.json；均值、stdev、CV、p10/p90、极值和每组数据也全部保留。

### 逐调用对应与计时范围

op_summary 的全部 1052 次 kernel 与 task_time 的 device、stream、task ID、开始时间、
时长及名称一致：8 次精度、540 次预热、504 次计时。task_time 全部 2313 行与 timeline
全部 19109 项均保留。实际 kernel 名称为：

```text
_Z24add_rms_norm_bias_customIDhEvPhS0_S0_S0_S0_mmjff
```

所有 timed call 都位于同 stream 相邻的 EVENT_RECORD 之间。ACL elapsed 与两事件
时间戳差的最大绝对差为 0.024001 us。时间戳用 Decimal 精确相减，再转为小范围数值，
没有先对巨大时间戳做浮点相减。每次调用的完整对应位于 all-calls.tsv。

同次调用的 event-task 差中位数为目标 16.21 us、对照 11.6999995 us。
start-event 到 kernel 的间隔中位为 5.61/5.62 us；kernel 到 stop-event 为 8.22/2.27 us。
各项中位数不能相加；它们没有全部归到某个 host API、调度或 profiler。

| timed 序号 / stream:task | 说明 | event us | kernel us | wall us |
|---|---|---:|---:|---:|
| 1 / 6:127 | 目标首样本，P 第一位置 | 41.460 | 8.700 | 178.761 |
| 34 / 6:226 | 目标最大 task，C 第二位置 | 23.380 | 9.040 | 78.471 |
| 410 / 7:760 | 对照最大 event，C 第二位置 | 68.939999 | 15.744 | 115.390 |
| 312 / 7:322 | 对照最大 wall，P 第二位置 | 18.020 | 15.640 | 132.361 |

这些样本均进入统计。目标首样本 host start RecordEvent 为 85.75 us，对照最大 wall
样本的 stop-event 同步为 94.88 us；device-event 与 wall 的大值不等同于 kernel 大值。
本次没有删除首样本，也没有按最大值挑选时间窗。

### 计时内的运行时申请与释放

采集完成后按用户转交的 R08 线索，只分析本次已有 timeline，没有追加设备运行。
方法来源为 `1bc84959fbc5dea190ad06a3dcef8b8b3005cae1` 的
`本地实验/W4-R08/V001/RETEST-20261008.md`；R01 数据全部来自本次自身采集。

504/504 个 timed call 通过 connection_id 对应到两次 host RecordEvent 和 Node@launch。
每次 start RecordEvent 返回后、stop RecordEvent 开始前，均有一次 Runtime@DevMalloc
和一次 AscendCL@aclrtFree，且都属于本次应用线程。它们发生在外层六个输入/输出分配之外。

| 输入 | DevMalloc 中位 us | aclrtFree 中位 us | Node@launch 中位 us | stop-event 同步中位 us |
|---|---:|---:|---:|---:|
| 16x16384 | 2.53 | 3.79 | 7.34 | 40.525 |
| 16x32768 | 2.56 | 3.80 | 7.41 | 41.345 |

这些是 host API 区间，存在嵌套与 host/device 重叠，不直接相加解释 device-event。
原 Parent 的 `run_kernel` 在 3479 行进入 host 分派，FP16 launch 位于 3547 行；
该源码未显式调用 aclrtMalloc/aclrtFree。运行时申请属于生成包装或更下层调用的哪一个
对象、申请字节数与用途仍为 UNKNOWN。本次没有把全部波动归结为申请/释放或 profiler。

### 资源与工具状态

cann-server3 / hwnput3，Ascend910B3，CANN 8.5.0.alpha002，GCC 11.4.0。
device0 开测与结束 FREE_HBM 为 61603/60948 MB，AICore/AIVector 快照均为 0%，
host load1 为 93.91→81.76；device0 已有 python PID 251901，其他设备有 VLLM 任务。
没有改动这些进程、服务或 lease，负载只作为背景。

两份 kernel 库仍为 532296/532360 B，mtime 与 2026-10-07 原构建记录一致；
只报告元数据和未执行 kernel 构建的事实，不以此声称整库字节相同。没有调用 objcopy。
host 源码真实跨机传输后用字节比较确认一致，没有对未传输源码另算摘要。
编译、采集、离线对应与源码验证均通过，没有本轮失败实验。

## 剩余工作与风险

V001 的真实收益仍未确认。本次未执行 Candidate，不能给出新的 Candidate 分数，
也不能由本次较稳定的 task 中心替代 P/P 与 P/C 的实际同进程对应。
原四组 reference 的严格差异继续保留；它们不代表 Official 判定。

精确下一动作：只读原 `support/build/libw4r01_v001_parent.so` 的 host 入口，
从 `run_kernel_parent` 和 Parent.asc:3547 的 FP16 launch 对应到本次
all-calls.tsv 中的 Node@launch、DevMalloc、aclrtFree，确认内部申请对象、大小及寿命。
复用现有 trace，不重采相同协议，不修改 Parent/Candidate 或创建 V002。
后续是否安排同进程 P/P→P/C，由 Main 基于已存证据另行决定；不为旧 0.06 us event
差重复当前 P/P。输出指针与逻辑槽位互换仍是未运行的测量方向，用于区分当前约
0.02 us 槽位差，尚无对应实验结论。

尚未确认第二个独立性能轴；V001 的参数起始槽位收益与测量小信号能力仍未定。
本次新研究事件为 `W4-R01-TASK-TIME-20261008`，版本引用为 `W4-R01/V001`，
类型为 `TIMING_ATTRIBUTION_SUPPLEMENT`；提交号由最终回执提供，等待 Record 异步同步。

06:49:34 UTC 的 final-state.txt 确认本 Route runner/profiler 无运行进程。
采集、导出、传输和分析均已结束，`RUNNING_DEVICE_OPERATION=NONE`。
本 Agent 交还 SLOT-3，由 Main 处理槽位释放，不接手第二条 Route。
