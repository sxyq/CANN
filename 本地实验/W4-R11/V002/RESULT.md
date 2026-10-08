# W4-R11 V002 RESULT

Parent: R31B V011. The sole performance change assigns `extraRows` to the highest-index blocks; block count, row arithmetic, tile choices, and active-core count remain unchanged.

## Build and correctness

- server3 build: PASS. The measurement runner was rebuilt after fixing its `same`-mode raw stream argument; Candidate performance source was not changed in this turn.
- Device: Ascend 910B3, device 0; runtime reported 40 vector cores.
- Target `64x8192 FP32`: Parent and Candidate correctness both PASS; maximum absolute error `4.76837e-06` for each.
- Control `12x8192 FP32`: Parent and Candidate correctness both PASS; maximum absolute error `3.09944e-06` for each.
- The target exercises `blockCount=40`, `extraRows=24`; the control has `blockCount=12`, `extraRows=0`. These are local proxy shapes, not an asserted Official testcase mapping.

## Parent same-binary qualification

Each shape has 82 Parent samples across two blocks. Qualification did not pass the existing stability limits:

| Shape | Median (us) | MAD/median | Block medians (us) | Block drift | Result |
|---|---:|---:|---:|---:|---|
| 64x8192 FP32 | 21.19 | 0.158565 | 21.62 / 20.72 | 0.0424729 | MEASUREMENT_BLOCKED |
| 12x8192 FP32 | 19.92 | 0.146084 | 19.04 / 22.34 | 0.165663 | MEASUREMENT_BLOCKED |

The earlier same-mode attempts are retained. Their first sample exposed a null raw-stream argument in the runner; the direct runner fix now passes the opened stream to same mode. This is a measurement-tool fix and does not alter Candidate performance logic.

## Interleaved Parent/Candidate Local observations

The runner collected 44 Parent/Candidate pairs per shape, alternating order. Numeric paired median deltas are retained as observations; because Parent same-binary qualification failed, they are not accepted as a Local Best.

| Shape | Parent median (us) | Candidate median (us) | Median paired delta | Parent MAD/median | Candidate MAD/median |
|---|---:|---:|---:|---:|---:|
| 64x8192 FP32 | 9.85 | 9.72 | -2.76053% | 0.0517766 | 0.0462964 |
| 12x8192 FP32 | 8.32 | 7.07 | -1.83724% | 0.241587 | 0.154173 |

The control has substantial dispersion. Keep the per-shape numeric deltas and all raw samples, but do not promote V002. `CURRENT_LOCAL_BEST=NONE`; `OFFICIAL_SCORE=NONE`; Online remains PAUSED.

## Device context and evidence

- Admission: device 0 had at least 62,108 MB free by the full NPU snapshot; the usages readout reported 5% HBM use on a 65,536 MB device. The required 100 MB threshold was met.
- Device 0 AICore use was 0% in the pre/post snapshots. Other NPU devices had active processes; none were stopped or changed.
- Host load averages were about 44.65 before and 38.96 after the Local run. Record as measurement context only.
- Same-binary stdout/raw: `logs/same-binary-device0-debug7.stdout.txt`, `results/same-binary-device0-debug7.raw.tsv`.
- Interleaved Local stdout/raw: `logs/local-device0-pc-after-debug7.stdout.txt`, `results/local-device0-pc-after-debug7.raw.tsv`.
- Admission snapshots: `logs/local-device0-preflight.txt`, `logs/local-device0-postflight.txt`.
- Runner-fix compile transcript: `logs/server3-compile-runner-fix.log`.
- The same-mode crash raw header and the earlier failed-run evidence remain alongside these files.

## 2026-10-08：device 1 测量资格复核

本次占槽完成 V002 旧事件恢复、原二进制复测、有限 task-time 采集和结果分析。
结论仍为 `MEASUREMENT_BLOCKED`。未创建 V001/V003，未改变 Candidate、Parent、
runner 或二进制；未重新构建。新增性能版 0、有效 Local 0、连续无改善贡献 0，
`STAGNATION_3=NO`，`CURRENT_LOCAL_BEST=NONE`，`OFFICIAL_SCORE=NONE`，
`ONLINE_STATE=PAUSED`，`PUSH=NO`。本次交接不改变 Route 生命周期。

### 来源、工作树与规则

- Route：W4-R11，SLOT-2；branch：`w4/r11-row-remainder-balance-x`。
- 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R11-row-remainder-balance-x`。
- 初始 HEAD：`d48af49d1ad3d9dd47b68375886fb93c5c36f5f6`，初始无未提交内容。
- 旧 V002 结果来源：`e6b7ca35`；后续余行几何来源：`d48af49d`。
- 先完整读取本工作树 AGENTS 与 Route Skill，再从本工作树读取
  `8f9338f8d01230635e617ff037e5951bb77bf67f` 的九项指定入口并发回执。
  收到规则更新后，完整读取 `9f91895506023d917637f707bb3f61cd9d9f8765`
  的同九项适用入口并再次发回执：AGENTS、Route Skill、W4 控制规则、实验总则、
  执行约定、服务器规范、本地性能规范、Git 工作流程、资源脚本。
- 工作树未带性能 Skill；已完整读取已安装的 ops-profiling 与 msprof-guide。
- 跨 Route 只读取 Git 对象：R14 `fcbd1814`、R02 `60ca277d` 提供计时研究线索。
  W3 ref 端点确认到 R2 V040、R4 V031、R5 V028；未把旧共享表当成最新版本。
  本轮没有新性能想法或新性能编辑，不把本次测量工具研究登记成新性能版。

`parent.asc` 与本工作树 `线上结果/R31B/V011/submission.asc` 比较无差异，
Direct Parent 保持 R31B V011。`parent-source.json` 留有 W3 R2 V002 的旧字段，
属于来源说明不一致；该文件原样保留，不作为本次 Parent 依据。旧
`correctness-current.log` 也留有其他输入名称；本次精度事实使用对应 same/local
运行日志，不能从那个旧文件代取。远端 runner 源码与本地 `support/probe.cpp`
比较无差异；实际复用 2026-10-07 构建的 `build-server3-v002/adaptive_probe`
和两份 kernel 动态库。

### 余行与 reference

device 1 实际报告 40 vector cores。64x8192 FP32 使用 40 blocks、24 extra rows；
Parent 的 blocks 0–23 各两行，V002 的 blocks 16–39 各两行。12x8192 FP32
使用 12 blocks、0 extra rows。两种输入的所有本次运行均先与 CPU reference 比较，
Parent/Candidate 的 failures 均为 0，最大绝对误差分别为 4.76837e-06、3.09944e-06。
沿用原 runner 的随机种子、输入范围和逐元素容差 `2e-5 + 1e-4 * abs(reference)`。
这些是 Local reference 结果，没有正式 Judge 结果。

128x16384 FP16 的既有几何仅说明 40 blocks、8 extra rows。源码显示该输入进入
`widePath_`，`ProcessWideLowPrecision` 在 V002 改动之前被调用并返回；内部仍使用
低编号余行归属。已在原探针文档补充这一范围，未对该输入运行新实验。

### 原二进制的 device-event 复测

保持原协议：每 shape 预热 45 次；same 采 2x41 个 Parent 样本，组间暂停 2 秒；
local 预热 45 对 P/C 后采 4x11 对，逐对交替 P-C/C-P。一次事件区间只含一次
launch；分配、输入搬运与 reference 在计时外。原 runner 不提供 wall_us。
所有样本保留，包括首样本和大尾值；没有按结果方向筛选。

| FP32 shape | Parent same median us | MAD/median | 组中位数 us | 组间相对差 | 结论 |
|---|---:|---:|---|---:|---|
| 64x8192 | 27.22 | 0.288024 | 19.72 / 36.20 | 0.605437 | MEASUREMENT_BLOCKED |
| 12x8192 | 22.20 | 0.453604 | 18.16 / 38.12 | 0.899099 | MEASUREMENT_BLOCKED |

下表全部是观察值。中位比 delta 为 `(Candidate median / Parent median - 1)*100`；
配对 delta 为逐对百分比的中位数，两个量分开保留。

| 计时方式 | FP32 shape | Parent median us | Candidate median us | 中位比 delta % | 配对 delta % | P-C 差中位 us | C-P 差中位 us |
|---|---|---:|---:|---:|---:|---:|---:|
| 原 event | 64x8192 | 18.16 | 15.69 | -13.601322 | -9.690739 | +1.17 | -4.70 |
| 原 event | 12x8192 | 14.92 | 15.25 | +2.211796 | -9.246666 | -3.19 | +2.10 |
| 采集期间 event | 64x8192 | 31.82 | 30.99 | -2.608422 | -2.484051 | -2.74 | -0.09 |
| 采集期间 event | 12x8192 | 30.02 | 27.82 | -7.328448 | -2.947989 | +0.99 | -5.16 |
| kernel task | 64x8192 | 14.91 | 11.20 | -24.882629 | -19.508585 | +0.52 | -5.03 |
| kernel task | 12x8192 | 4.76 | 4.95 | +3.991597 | +0.970099 | -0.022 | +0.23 |

原 event 的目标 Parent/Candidate MAD/median 为 0.301211 / 0.251753，
对照为 0.421582 / 0.418361。目标配对差随次序反向，四组差中位数为
-3.92 / -16.38 / +6.70 / -4.78 us；不能认定 V002 提升。

### 有限 task-time 采集与对应关系

直接给既有二进制附加 `msprof --ai-core=off --task-time=on --ascendcl=on
--runtime-api=on --aicpu=off`，依次运行 same 和 local。没有采集多组硬件指标，
没有改写 runner。两次采集与解析成功，分别在 04:14:24Z、04:14:37Z 完成。

same 的 op_summary 共 258 次 kernel 调用，每 shape 为 2 次精度调用、45 次预热、
82 次计时。local 共 360 次，每 shape 为 2 次精度调用、90 次预热、88 次计时。
按实际开始时间与 runner 顺序对应，shape 对应 Block Dim 40 / 12。全部
op_summary 与 task_time 的 stream、task ID、开始时间和时长一致。
总共保留 680 个新 event 样本、618 个完整 kernel 调用记录；其中 340 个 kernel
调用与计时样本逐一对应。精度与预热调用保留在原始 CSV 中，不混入计时统计。

| FP32 shape | Parent task same median us | MAD/median | 组中位数 us | 组间相对差 | 结论 |
|---|---:|---:|---|---:|---|
| 64x8192 | 9.51 | 0.197687 | 8.10 / 11.74 | 0.382755 | MEASUREMENT_BLOCKED |
| 12x8192 | 7.41 | 0.256410 | 5.86 / 10.22 | 0.588394 | MEASUREMENT_BLOCKED |

两种计时范围都未达到既有 MAD/median <= 0.10、组间相对差 <= 0.10 的要求。
task-time 的目标 P/C MAD/median 为 0.119383 / 0.096429，次序差仍反向。
同次采集、同次调用的 `event - task` 差中位数为目标 17.596 us、对照 23.830 us；
它说明 event 包含内核之外的时间，不能将该差全部归入某一种 host API。
kernel 自身的波动仍在，不能只去掉打印或写盘就宣称测量已经可靠。

源码还表明 same 每样本 flush、组间暂停，local 的节奏不同；首样本保留的调试打印
位于事件区间内。这些事实解释了需要对齐计时范围的理由，未被当作已验证的单一根因。

### 设备、命令和证据

server3：`hwnput3`，用户 `lelinfeng`，SSH 入口 `cann-server3`，device 1，
Ascend 910B3，已有 CANN 8.5.0.alpha002 / dav-2201 构建。
每阶段 usages 均为 65536 MB、使用率 20%，按项目方法推算 FREE_HBM=52428 MB。
原复测的 host load1 为 48.79–64.43，采集前后为 39.64–43.53；
对应 AICore 快照为 0–16%。其他用户任务继续运行；无资源停止条件，`BLOCKER=NONE`。

本地命令入口为原 `run_server3.sh same 1` 与 `run_server3.sh local 1`，
输出参数分别为 `results/requal-device1-20261008-original-same.raw.tsv`、
`results/requal-device1-20261008-original-pc.raw.tsv`。远端同一 V002 目录运行
`build-server3-v002/adaptive_probe --mode same|local --device 1 --output <新路径>`。
采集使用上节列出的 msprof 选项，两个 `--output` 分别为远端采集目录下的
`same-profile`、`local-profile`；event 输出为 `same-event.raw.tsv`、
`local-event.raw.tsv`。执行输出保存在
`logs/requal-device1-20261008-task-session-env.log`。

首次 profile 启动命令在打印设备信息前返回 1，未创建采集目录。取消环境加载前的
`nounset` 后成功；首次错误正文被原命令重定向，不能进一步归因。该次空日志
`logs/requal-device1-20261008-task-session.log` 保留，没有性能样本或构建动作。

远端仅新增本 Route V002/results 下的原 event TSV，以及：

```text
/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x/V002/results/requal-device1-20261008-task/
```

本地新增证据位于 V002/logs 与 V002/results，必要的导出 CSV、timeline、原始 TSV
已取回；原 profiler 数据继续保存在上述远端专用目录，未清理任何旧数据。
本地离线统计脚本是 `support/summarize_requalification.py`，结果为
`results/requal-device1-20261008-summary.json`。该 JSON 保留每个 timed sample
与 task ID 的对应、均值、标准差、CV、MAD、p10/p90、范围、分组及次序统计。

### 事件与交接

旧 V002 的 `ROUTE_EVENT + VERSION_RECORD_EVENT` 已按 `e6b7ca35` 恢复；
本次提交后的事件只补充同一 V002 的测量与研究，不新增性能版本。
`ROUTE_RESEARCH_EVENT` 的事实为：R11 的 event 与 task 计时范围差异已有本路线
直接证据；task 时长自身及目标 P-C/C-P 仍不稳定；128x16384 FP16 未经过 V002
高编号余行改动。没有把 R14/R02 的输入结果移作 R11 结果。

后续精确动作：若再次安排 R11，先在原 `RunLocal` 结构加入 Parent/Parent
配对模式，保持同一预热、两个输出地址和 P-C/C-P 次序，复用现有 kernel 动态库，
只对齐 host 测量逻辑。与本次 task ID、kernel 时长和次序数据比较，分离第一/第二次
调用及输出地址因素；不无变化重跑当前四组命令，不先开 V003。

尚未解决的独立性能范围是宽行函数内部的余行归属。它需要先从 W3 R2 到 V040、
R4 到 V031、R5 到 V028 及 R31/R31A/R31B、MIX、STORE/EPILOGUE 与相关 W4 的
已提交源码做重复核对；本次没有完成该新机制的审计，也没有创建 Candidate。
128x16384 FP16 可供该研究使用，仍须先完成独立 reference 和实际路径验证。

原二进制复测、两次 profiler、导出和传输命令均已结束；最终远端进程记录见
`logs/requal-device1-20261008-final-state.log`。没有修改共享 TSV、规则、Dashboard、
其他工作树或主目录，没有创建子代理、线程或持续定时任务。
