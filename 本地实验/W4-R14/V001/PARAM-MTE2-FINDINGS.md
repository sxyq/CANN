# W4-R14 Parent 参数 MTE2 指纹

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
