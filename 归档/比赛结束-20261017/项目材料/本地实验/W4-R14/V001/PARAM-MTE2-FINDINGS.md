# W4-R14 Parent 参数 MTE2 指纹

## 2026-10-08 续接：kernel-task 与设备事件逐调用对应

本次限定研究已完成。`SIGNAL_ABOVE_NOISE=NO_PROXY_BELOW_FLOOR`，
`STATUS=MEASUREMENT_BLOCKED`；未编辑 Candidate，未创建 V002。
当前参数 DMA 节省估计仍低于本路线实测噪声，不能声明性能提升。

复用 `fcbd1814764337b89bc3b9fb6950d37241feab5a` 的 Parent runner、现有二进制、
504 个 P/P 原始样本与两组参数 profile。内核和 host runner 均未改动、未重建。
新增 shell 仅在同一次应用进程外启用 task-based profiler；新增 Python 只读取采集结果。
原始命令、stdout/stderr、设备状态、全部 profiler 原始数据及导出表均保留。

### 指标与直接结论

以下时间单位均为 us。参数计数及时间代理沿用前次 device 0 证据；
kernel-task、设备事件与噪声来自本次 device 3，不能把旧代理写成本次参数实测值。

| 字段 | 80x8192 FP32 | 120x6144 FP32 |
|---|---:|---:|
| PARAM_MTE2_COMMAND_COUNT，源码归因，每核 / launch | 4 / 160 | 4 / 160 |
| PARAM_MTE2_TIME_OR_PROXY，旧字节占比代理 | 1.223342600 | 0.961718744 |
| 参数专属实测时间 | UNKNOWN | UNKNOWN |
| TOTAL_KERNEL_TIME，本次 84 次 timed 中位数 | 8.320000 | 9.430000 |
| P1 / P2 kernel-task 中位数 | 8.320000 / 8.320000 | 8.940000 / 9.940000 |
| EXPECTED_SAVING，旧乐观代理 | 0.611671300 | 0.480859372 |
| 实测 Candidate 节省 | NONE | NONE |
| MEASUREMENT_FLOOR，kernel-task | 8.028400 | 17.442000 |
| MEASUREMENT_FLOOR，设备事件 | 15.020000 | 13.298000 |
| 设备事件中位数 | 27.890000 | 27.050001 |
| 逐调用 event 减 kernel 中位数 | 17.310000 | 15.869999 |
| SIGNAL_ABOVE_NOISE | NO_PROXY_BELOW_FLOOR | NO_PROXY_BELOW_FLOOR |

旧节省代理仅为本次 kernel-task 噪声幅度的 7.62% / 2.76%。它按参数字节占全部
GM-to-UB 字节的比例分配旧 MTE2 活跃时间，再乘命令数减半的比例；实际字节量不变，
参数 issue/wait 与流水重叠尚未分离。该代理既非已测收益，也非严格上界。

80x8192 的 kernel-task MAD/median 为 1.3221%，block 中位数漂移 0.7212%，
通过既有的这两项中心稳定性条件；但完整配对分布仍给出 8.0284 us 的绝对噪声。
120x6144 的 MAD/median 为 8.3563%，block 漂移为 12.3012%，后一项超过 10%。
两组设备事件的 MAD/median 分别为 14.2345% / 10.6839%。所有尾部样本均参与统计。

长尾确实包含在 kernel-task 内。例如 80x8192 的 block 1、pair 13、P1 为
`stream=6, task=387`，kernel 为 170.064 us，设备事件为 174.740001559 us。
120x6144 的 block 1、pair 20、P1 为 `stream=9, task=463`，分别为
43.400 us 与 55.619999766 us。现有证据没有说明这些长尾的内核内部原因，
不能全部归因于 host 发射间隔，也不能将其删除后重新宣布低噪声。

### 有限采集声明与对应验证

采集前已在对话中声明：device 3，先 80x8192，再 120x6144；每组只运行一次进程，
主指标 kernel-task，保留设备事件、wall 时间、event 减 kernel 和前后事件区间。
每组固定 1 correctness、45 warmup、2 个 block、每 block 21 对 P/P，共 130 次调用。
P1/P2 为同一二进制、同一份输入、同一 stream，次序沿用 `(block + pair) % 2`。
没有按结果方向重跑，也没有增加或删除测量点。

```text
MEASUREMENT_FLOOR = max(p90(abs(P2-P1)), range(block medians))
CORRECTNESS_CALL = ordinal 1
WARMUP_CALLS = ordinal 2..46
TIMED_CALLS = ordinal 47..130
```

两次应用 PID 分别为 4055608 / 4057074，来源为各自 profiler 的 `host/info.json`。
`op_summary` 和 `task_time` 按 device、stream、task 编号逐项对应；全部 260 次
kernel 的开始时间和持续时间一致。单 stream 时间顺序结合原 runner 的固定次序，
把每次调用标为 correctness、warmup 或 timed。

全部 168 个 timed 调用均找到紧邻的两个 `EVENT_RECORD`，并满足
`start event <= kernel start <= kernel stop <= stop event`。
两个事件时间戳之差与 ACL elapsed time 的最大差异分别为
0.024000875 / 0.024000857 us；差值原样保留，未要求两个计时接口逐位相等。
对应程序首次执行通过，全部原始样本保留在 `profile.log`，原表提取为 `event.raw.tsv`。
每组 `all-calls.tsv` 保存全部 130 次调用和 84 次事件对应关系。

方法来源为 R11 `4243f4e9` 的调用序列对应，以及 R12 `a018f702`、R10 `1441fc72`
的 task 编号与前后事件对应方法。只复用方法，未移用这些 Route 的测量资格。
本次使用 `--task-time=on --aic-mode=task-based --ai-core=off --ascendcl=on
--runtime-api=on --aicpu=off`，没有重采前次 MTE2 计数或地址资料。

### 精度与资源

80x8192 / 120x6144 的原 Parent 对 CPU FP64 reference 均为 0 个超差元素，
最大绝对误差分别为 `7.6549910899e-7` / `6.98246055642e-7`。
输入种子仍为 322351 / 320303；容差仍为 `2e-5 + 1e-4 * abs(reference)`。
没有修改输入生成、reference、epsilon、分派、核数或计算次序。
两组各自在本次进程中先通过 reference，再预热和计时。

前次 1x32768 FP32 的 26234 个超差元素及最大绝对误差 1.87199266423 继续保留；
本次未执行该失败输入。R13 `a10cf2a1` 只证明其 1x9216 / 1x10240 的 tile>0
参数复用时序诊断结果，未覆盖本 Route 的 1x32768 或多批；未将其变化叠加到 Parent。
本次两个尺寸为 Local 代理，没有推断 Official 输入或分数。

实际设备为 `hwnput3` 的 device 3，物理与可用 Vector core 数均为 40，blockCount=40。
两组前后 FREE_HBM 均为 58327 MB；开测 load1 为 68.01 / 63.09，结束为
63.09 / 59.31。开测 AICore 使用率为 1% / 7%，AIVector 均为 4%。
已有 PID 3836347 的 python 任务仍驻留，设备内存为 3906 MB；未对其执行任何操作。
完整前后状态在本次结果目录内，负载只用于解释采样。

### 范围、历史核对与实际文件

本次开始 HEAD 为 `fcbd1814764337b89bc3b9fb6950d37241feab5a`，分支与工作树保持
`w4/r14-param-dma-granularity-x`、
`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R14-param-dma-granularity-x`。
最新共享提交 `4959725e` 已登记本次 SLOT-4、device 3 接手，
`agent_id=01a11a12-4527-71a3-97d4-69dc9720e841`；本次新结果尚待 Record 同步。
工作树 AGENTS 与 Route Skill 已读；另从自己的 Git 对象完整读取 `9f918955` 的
AGENTS、Route Skill、W4 控制文件、实验总则、执行约定、服务器规范、本地性能规范、
Git 工作流程和资源脚本，并在任何写入前发送 `RULE_REFRESH_RECEIPT`。
缺少工作树性能 Skill 时，完整读取已安装的同名 Skill 与 msprof 使用说明。

```text
DUPLICATE_AUDIT
MECHANISM=unchanged Parent per-call kernel-task/event timing attribution
SEARCHED_HISTORY=复用 fcbd1814 对 R14、R31/R31A/R31B、MIX、STORE/EPILOGUE、
  W4 相关历史的既有审计；本轮核对 W3 R2 V040/6321ad44、R4 V031/ce6c6dc5、
  R5 V028/1efa0863 端点，追加 R11/4243f4e9、R12/a018f702、R10/1441fc72、R13/a10cf2a1
MATCH_FOUND=跨 Route 已有 task/event 对应方法；本 Route 原先只有分离采集的统计
WHY_NEW_OR_DUPLICATE=首次为这两个 R14 输入取得同次进程的逐调用对应；不重复参数计数，
  不重新实施已有性能机制，不增加性能版本
```

本轮实际修改本文件；新增 `parent-probe/collect_task_time.sh`、
`parent-probe/analyze_task_time.py` 与 `parent-probe/task-time-20261008/`。
后者保留分析 JSON、全部调用表、原始事件表、原始 profiler 数据、导出表与负载记录。
原 7 个未跟踪 support 文件、Parent、旧 504 个样本和两个旧 profile 均保持原样，
旧 support 不暂存。共享 TSV、规则、Dashboard、其他工作树和服务均未修改。

远端只新增 R14 原专用目录下的 `parent-probe/collect_task_time.sh` 与
`parent-probe/task-time-20261008/`，未使用 R04 目录：

```text
/home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008/parent-probe/
```

采集、传输和分析均已结束。远端 `final-state.txt` 在 06:00:36 UTC 记录
runner 查询返回 1、`RUNNING_DEVICE_OPERATION=NONE`。没有创建定时或后台循环。

### 研究事件与下一动作

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R14-KERNEL-TASK-20261008
PARENT_EVENT_ID=W4-R14-PARAM-MTE2-20261008
ROUTE=W4-R14
REVISION=NONE
SOURCE_DIRECTORY_LABEL=V001_RESEARCH
DIRECT_PARENT=R31B V011
AGENT_ID=01a11a12-4527-71a3-97d4-69dc9720e841
SLOT=4
SINGLE_CHANGE=NONE; Parent-only observation
COMPILE=REUSED_PASS; fcbd1814 existing Parent executable
CORRECTNESS=PASS_2_OF_2_THIS_CAPTURE; PREVIOUS_1x32768_FAILURE_RETAINED
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL=0
STAGNATION_3_CONTRIBUTION=0
VERSION_RECORD_EVENT=NONE; no performance revision
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
STATUS=MEASUREMENT_BLOCKED
SIGNAL_ABOVE_NOISE=NO_PROXY_BELOW_FLOOR
RUNNING_DEVICE_OPERATION=NONE
GIT_COMMIT=see final receipt for this document's commit
BRANCH=w4/r14-param-dma-granularity-x
```

下一研究动作：先核实当前 DAV-2201 SDK 的 kernel cycle 计时能力，能否对原 Parent
`ProcessFp32FullRowOutputPipelined` 的参数预载段（原 1130–1131 行）及 generic
预载段（原 254–255 行）记录 issue 到参数就绪的时间；须明确计时插入本身的影响，
并与输入 MTE2 分开。来源为现有 Parent、对应 `SyncMTE2ToV` 调用与 SDK 计时 API。
现有就绪等待还覆盖随后发出的输入搬运，不能把该等待区间直接归为参数专属时间。
该能力尚未验证，本轮没有实施插入式诊断。没有新的分段计时依据时，继续保留参数
专属时间 UNKNOWN，不无变化重跑这两组 P/P，也不据旧代理开性能版。

同张量连续片段 4→2 仍是未完成的独立性能轴；FP16 预载粒度与参数搬运指令形式
仍未测。它们没有因本次研究被判为耗尽。后续安排和槽位释放交 Main/Planning，
本 Agent 不接手第二条 Route。本次新事件待 Record 同步。

---

## 前次记录：fcbd1814，原文保留

2026-10-08。完成本次占槽的主要研究结论；未创建 Candidate，未创建 V002。

## 结论

`SIGNAL_ABOVE_NOISE=UNPROVEN`，本轮不进入性能编辑。默认 40 核下，
`80x8192 FP32` 与 `120x6144 FP32` 确实进入整行参数预载路径。
每核参数搬运为 4 次，整次 launch 为 160 次；同张量两段合并可在源码上减少
2 次/核，gamma 与 bias 仍须分开搬运，传输字节量不变。

本轮测出的参数时间代理值约为 1.223 / 0.962 us。把参数命令数减半对应到
该代理值，得到 0.612 / 0.481 us 的乐观节省估算。该估算低于本轮同进程 P/P
噪声幅度 11.354 / 16.424 us，尚不支持可辨认的收益。

这份证据支持 `MEASUREMENT_BLOCKED`；它没有证明该机制永远无效，也没有改变
Route 生命周期。`STAGNATION_3=NO`，本轮新增性能版本数为 0。

## 身份、读取范围与复用

```text
ROUTE=W4-R14
BRANCH=w4/r14-param-dma-granularity-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R14-param-dma-granularity-x
HEAD_AT_START=db11b221635f630ead503fabce87b4c6cf9f92a2
DIRECT_PARENT=R31B V011
CURRENT_LOCAL_BEST=NONE
LAST_KNOWN_REVISION=V001，既有研究目录
NEXT_PERFORMANCE_REVISION=V002，尚未创建
ONLINE=PAUSED
OFFICIAL=NONE
PUSH=NO
RESOURCE_BLOCKER=NONE
```

工作树的 `AGENTS.md` 与 Route Skill 已完整读取；随后从本工作树执行
`git show 8f9338f8d01230635e617ff037e5951bb77bf67f:<path>`，完整读取以下入口：

- `AGENTS.md`
- `.agents/skills/cann-route-executor/SKILL.md`
- `项目规则/W4持续探索控制契约.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/服务器实验规范.md`
- `项目规则/本地性能测试规范.md`
- `项目规则/Git工作流程.md`
- `工具/server3-resource-policy.sh`

已发送包含 branch、worktree、HEAD、dirty、Parent 和下一动作的
`RULE_REFRESH_RECEIPT`。采用用户最新的单 Route 分配与本次占槽收口要求。
工作树缺少性能 Skill 副本，已完整读取已安装的 `ops-profiling` 及其
`msprof-op-guide.md`、CSV 字段参考；精度验证采用 `ops-precision-standard`
的浮点与 CPU 参考说明。

已有 `support/param-mte2-fingerprint/` 的 7 个未跟踪文件均保留原内容，未暂存。
其中 `kernel.asc` 与本工作树的 R31B V011 源码比较无差异。
已有远端 `build-attempt8/r14_param_mte2_fingerprint` 被直接复用于新 profile。
新增 Parent P/P runner 直接 include 既有 `main.asc`，复用内核、ABI 和资源释放代码；
仅补 CPU reference、设备事件计时与 raw 输出。

## DUPLICATE_AUDIT

```text
MECHANISM=同一参数张量的连续预载片段由两条搬运变为一条；本轮仅量 Parent
SEARCHED_HISTORY=R14 V001；W3 R2 到 V040、R4 到 V031、R5 到 V028 的 Git 记录；
  R31/R31A/R31B、MIX、COEFF-LOCALITY、MULTIROW-DMA、STORE/EPILOGUE；
  W4 R01、R06、R07、R15 的记录与现有 R14 审计
MATCH_FOUND=存在参数预取次序、驻留、槽位相位、输入 DMA 与输出合并的相近历史；
  在本次读取材料中未找到参数预载 4→2 的已完成性能版本
WHY_NEW_OR_DUPLICATE=新增证据是默认 40 核实际预载路径的计数、时间代理和 P/P 分布；
  没有重复实施旧性能变化，没有把研究包装成性能版本
```

最新 W3 端点分别为 `6321ad44`（R2 V040）、`ce6c6dc5`（R4 V031）、
`1efa0863`（R5 V028）。旧调度行通过
`git show w3/m1/record-owner:调度/当前任务.tsv` 读取。
COEFF-LOCALITY-X V004 与 MULTIROW-DMA V001/V002 的旧结论仅用于定位，
没有作为停止本轮测量的理由。

旧审计的两项数量解释需要区分：

- `2x8192`、`3x6144` 配默认可用核数 40 时，每核只有一行，整行预载不生效。
- 预载生效时，gamma 与 bias 各两条，共四条；合并每个张量的两段共减少两条，
  每核参数字节量仍为 65536 B（D=8192 FP32）或 49152 B（D=6144 FP32）。

`80=2x40` 与 `120=3x40` 来自已读取的 Parent 分派公式和设备实测核数，
用于自然激活两行/三行路径；没有对 hidden testcase 做形状推断。

## 本轮新 profile

设备为 server3 `hwnput3` 的 device 0，用户 `lelinfeng`，CANN
`8.5.0.alpha002`，DAV-2201。`msprof op` 参数为
`--aic-metrics=Memory,PipeUtilization --warm-up=10 --launch-count=1`。
实际命令与输出见 `parent-probe/results/profile-natural-permissions-fixed.log`。
两次采集均成功输出非空 `Memory.csv`、`PipeUtilization.csv`、`OpBasicInfo.csv`，
每份流水与内存表各有 40 个核；Current/Rated Freq 均为 1800。

| 字段 | 80x8192 FP32 | 120x6144 FP32 |
|---|---:|---:|
| blockCount / localRows | 40 / 2 | 40 / 3 |
| PARAM_MTE2_COMMAND_COUNT，源码归因，每核 | 4 | 4 |
| 参数命令数，整次 launch | 160 | 160 |
| 全部 MTE2 指令数，硬件计数，每核 | 13 | 17 |
| 输入搬运数，源码归因，每核 | 8 | 12 |
| 未归因的额外指令，每核 | 1 | 1 |
| MTE2 活跃时间，40 核均值，us | 3.670027800 | 3.846874975 |
| MTE2 活跃时间，逐核最小/最大，us | 2.570555 / 5.750000 | 2.948333 / 4.870555 |
| PARAM_MTE2_TIME_OR_PROXY，us | 1.223342600 | 0.961718744 |
| TOTAL_KERNEL_TIME，profiler task，us | 9.440188 | 10.060202 |
| EXPECTED_SAVING，乐观命令占比代理，us | 0.611671300 | 0.480859372 |
| MEASUREMENT_FLOOR，本轮 P/P，us | 11.354000867 | 16.423999332 |
| SIGNAL_ABOVE_NOISE | UNPROVEN | UNPROVEN |

归因依据是 Parent `ProcessFp32FullRowOutputPipelined` 的参数 Load（1130–1131 行）
与 generic cacheParams Load（254–255 行），以及 `Load` 的单块 DataCopyPad
实现（3367–3375 行）。参数命令数属于源码路径计数；硬件给出的总 MTE2 指令数
与输入、参数计数之和始终相差 1。该额外指令未被强行归入参数。

参数时间代理按每核 `MTE2 active time * parameter bytes / GM_to_UB bytes` 计算，
再对 40 核取均值。节省代理再乘 1/2；字节量不变，真实 command issue 时间、
等待和重叠尚未单独测出，因此该值无法充当已测收益或严格上界。
`EXPECTED_SAVING_MEASURED=NONE`，`LOCAL_SCORE=NONE`，`LOCAL_DELTA=NONE`。

## Parent 同进程 P/P

每个用例只创建一次 ACL context、stream、输入输出和事件；CPU reference 通过后，
预热 45 次，再采 2 个 block，每 block 21 对。P1/P2 调用同一个 Parent，按
P1-P2/P2-P1 交错，每次事件范围包含一次 launch。分配、输入搬运和 CPU reference
在计时外；wall 时间另列。6 个有效用例共保留 504 个 device-event raw samples。

噪声统计事先写入分析程序：

```text
MEASUREMENT_FLOOR = max(p90(abs(P2_us - P1_us)), range(block medians))
```

全部样本参与中位数、MAD、均值、标准差、CV、p10、p90 与范围统计；没有删尾部样本。
以下均为 us。完整结果见 `parent-probe/results/analysis.json` 与 `same_*.tsv`。

| 形状 / availableCoreNum | P1 中位 | P2 中位 | 全部中位 | MAD | P/P 噪声幅度 |
|---|---:|---:|---:|---:|---:|
| 2x8192 / 40 | 15.000000 | 15.150000 | 15.090000 | 1.020000 | 6.750000 |
| 3x6144 / 40 | 19.910000 | 18.000000 | 18.890000 | 8.420001 | 19.224000 |
| 2x8192 / 1 | 17.820000 | 18.580000 | 18.110000 | 6.510000 | 30.964000 |
| 3x6144 / 1 | 16.600001 | 17.729999 | 16.650000 | 3.310001 | 15.656000 |
| 80x8192 / 40 | 21.540000 | 20.370000 | 20.770000 | 2.740000 | 11.354001 |
| 120x6144 / 40 | 19.090000 | 18.600000 | 18.639999 | 3.110000 | 16.423999 |

80x8192 与 120x6144 的 MAD/median 为 13.19% 与 16.68%。
计时区间与 profiler task 的边界不同，不能把两者的中位差当作内核改进。
`2x8192 / 40` 的重复性较好，但参数整行预载在该分派下不生效。

本轮 profile 开测 FREE_HBM 为 62259 MB；P/P 开测为 61603 或 60948 MB，
始终高于 100 MB。P/P 各用例开测 host load1 为 71.68–97.63；已有 VLLM 等
其他用户任务继续运行。负载与利用率仅用于说明采样上下文，未被用作拒绝运行的理由。

## 精度结果与限制

FP32 runner 与 CPU FP64 参考直接比较，公式为
`(x + residual) / sqrt(mean((x + residual)^2) + 1e-5) * gamma + bias`，
逐元素要求 `abs_error <= 2e-5 + 1e-4 * abs(reference)`。
随机输入种子、范围和参考实现保存在 `parent_probe.asc`。

前 6 个用例 mismatches 均为 0，最大绝对误差不超过 `7.65499109e-7`。
`1x32768 FP32` Parent 在非零随机输入上失败：32768 个元素中有 26234 个
不满足容差，最大绝对误差 `1.87199266423`。该用例返回 3，未预热、未采性能；
原始 stderr 与空的 timing 文件均保留。失败元素的逐元素输出未落盘，保留的是
失败计数、最大误差、种子及可重现参考代码；本轮没有无变化重复执行该失败。

旧零输入 profile 的成功仅说明对应零输入结果为零，不能替代这次 CPU reference。
已有 STORE/EPILOGUE 记录也提及 wide FP32 Parent reference 风险；本轮没有据此
推断具体原因，也没有修改 Parent 或把 Parent 一致性称为全部正确。

## 已有证据与失败保留

`parent-probe/prior-profile/` 是从 R14 已有远端目录取回的旧证据，未计为本轮新采集。
包括默认分派的 2x8192、3x6144 FP32，1x32768 FP32，16x16384 FP16，以及
单核探针的已有采集。其参数源码计数分别为 4、4、16、8 次/核。
旧 TimelineDetail 记录明确出现 kernel args dump 失败；本轮没有复用其时间线数据。

本轮两项工具适配问题已处理并保留证据：

- 初次 Parent runner 构建的辅助注册阶段找不到 `<vector>`；给编译器子进程补上
  `CPLUS_INCLUDE_PATH` 后，同一个 build 目录构建成功。见 `compile.log`、
  `compile-failure.stderr.txt`、`compile-with-includes.log`。没有新建第二个 build 副本。
- 首次新 profile 被目录权限要求拒绝，未产生有效采集。只将本轮创建、属主为
  `lelinfeng` 的 `parent-probe` 及其 `results` 目录收紧为 750，未改变共享父目录
  或旧 support 权限。原错误日志与随后成功的日志均保留。

## 操作对象、停止位置与后续

本地新增内容仅位于本工作树的 `本地实验/W4-R14/V001/parent-probe/` 与本文件。
旧 `REVISION-NOT-READY.md`、旧 support、Parent 源码、共享 TSV、规则、Dashboard
与其他 Route 工作树均未修改。

远端新增内容仅位于：

```text
/home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008/parent-probe/
```

profile、构建与 P/P 命令均已返回；最终进程查询没有匹配到本轮 runner 或 profile。
`final-device-state.log` 的退出码 1 来自无匹配的结果过滤，NPU 查询正常。

```text
RUNNING_DEVICE_OPERATION=NONE
NEW_PERFORMANCE_REVISIONS=0
VERSION_RECORD_EVENT=NONE（本轮无性能版本）
ROUTE_RESEARCH_EVENT=W4-R14-PARAM-MTE2-20261008
STATUS=MEASUREMENT_BLOCKED
ONLINE=PAUSED
OFFICIAL=NONE
PUSH=NO
```

精确后续动作：Main 若再次分配 R14，复用本 runner 与现有 504 个 P/P 样本，
先对 80x8192 / 120x6144 采用同进程的 kernel-task 时间戳记录，区分当前
device-event 区间与实际 kernel task 的差异；保留同一预热和交错次序。只有形成
低于约 0.5 us 的可靠噪声口径，并能分离参数 issue/wait 贡献后，再考虑预载
同张量 4→2 的单因素版本。旧 TimelineDetail 的 args dump 失败日志为相关工具
适配的直接输入，不能以空时间线作依据。

尚未完成的独立范围包括 FP16 整行预载的参数粒度证据，以及参数搬运指令形式；
本轮均未实施。任何新想法仍须重新做对应历史核对。wide FP32 reference 失败
由 Main 决定后续归属，不在 R14 擅自扩展 Parent 修复。无 Online 操作，无定时任务。
