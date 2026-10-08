# W4-R04 V002 结果与交接

## 结论

本版完成 Compile、独立 reference Correctness、Parent same-binary 和 P/C Local。
精度通过，Local 状态为 `MEASUREMENT_BLOCKED`：同二进制和 P/C 的波动均超过
事前声明范围。未取得可确认的性能改善，`CURRENT_LOCAL_BEST=NONE`。

本轮新增真实性能版本 1，有效 numeric Local 0，连续无改善计数 0，
不构成 `STAGNATION_3`。所有原始样本及负向观察均保留，不另开版本、不扫偏移。

## 身份、来源与唯一变化

- Agent：`01a119cc-fa30-7740-8fd2-9cdfab6e6c46`，SLOT-4，仅负责 W4-R04。
- 分支：`w4/r04-ub-bank-xr-layout-x`。
- 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R04-ub-bank-xr-layout-x`。
- Direct Parent：`de70b634813dea80783fc57716d6e95c158edeec:线上结果/R31B/V011/submission.asc`。
- 研究事件：`R04-UB2201-EVIDENCE-20261008`，来源提交 `b6890ad19bd60560fdd9febce98760078982e51f` 与 `8c3b6659014e7904c1b14371e24045e68aaa8bee`。
- V001 保持旧研究标签；V002 是本轮首个真实性能版，没有跳过失败版。
- 远端唯一版本目录：`cann-server3:/home/data4t2/lelinfeng/cann/server_runs/W4-R04/V002`。

Candidate 仅增加以下五行，位于 NarrowMid 的 residualLocal 获取之后：

```cpp
if constexpr (AscendC::IsSameType<T, float>::value) {
    if (rowWidth <= static_cast<uint64_t>(kTileElems - 64)) {
        residualLocal = residualLocal[64];
    }
}
```

移除这一个新增片段后，Candidate 与 Parent 完全一致。因此全部 InitBuffer、
参数位置、tile、ownership、其他视图、DMA 数量、事件和算术顺序均未变。
九个已跨机传输的构建/源码文件与远端逐字节一致，见 `logs/source-transfer-verification.txt`。
没有对规则或未传输资料计算摘要。

## Compile 与独立 reference

server3 CANN 8.5.0.alpha002，Ascend910B3，dav-2201；设备 3 的运行时核数为 40。
首次 Compile 返回 0。用户增加 guard-OFF 要求后，只补 runner 的一项输入，
同版再次 Compile 返回 0，Candidate 没有改变。两次构建日志均保留。

正式 Correctness 于 2026-10-08 05:30:33–05:30:36 UTC 完成，返回 0。
Parent/Candidate 分别对 CPU FP64 reference 比较：`atol=1e-5`、`rtol=1e-4`，
要求所有元素通过且 max_abs<=1e-2。九项输入、每侧 355340 个输出元素全部通过，
双方最大绝对误差均为 `7.15255737305e-7`，逐元素字节差异为 0。

九项 FP32 输入为：16×2048、16×2056、12×8192、2×128、2×129、2×4032、
2×4033、2×4096、81×2056。后六项是容量边界与多行复用测试，仅用于精度。
本结果不扩展为其他 dtype、所有输入或 Official 的精度结论。

guard-OFF 的 12×8192 来自 R03 `712e4723` 对 R11 V002 的输入与分派绑定；
它进入 GenericRow，不进入 NarrowMid。本对照与目标路径不同，不能独自承担
指令级归因。16×2048 和 16×2056 均为 guard-ON，来源为 W3 R2 V040 runner。

## Local：全部样本与数字

每项每侧 45 warmups、4 块×11 对，按 block+pair 奇偶交错 P-C/C-P；仅 event
区间计时，分配、输入准备与 warmup 在区间外。每个阶段 132 对、264 个单侧样本，
两个阶段合计 264 对、528 个单侧样本。没有剔除、替换或选择极端值。

Parent same-binary 于 05:31:22–05:31:26 UTC 完成；下表 A/B 都调用同一 Parent
动态库。原 TSV 的 candidate 列在此阶段表示 Parent B，runner 日志明确记录该身份。

| FP32 输入 | A/B median，µs | median 比值差 | A/B MAD/median |
|---|---:|---:|---:|
| 16×2048 | 15.860 / 16.420 | +3.530893% | 30.58% / 36.72% |
| 16×2056 | 17.100 / 14.950 | −12.573100% | 34.56% / 48.09% |
| 12×8192，guard-OFF | 17.700 / 18.390 | +3.898306% | 26.10% / 20.61% |

P/C Local 于 05:32:13–05:32:16 UTC 完成：

| FP32 输入 | Parent/Candidate median，µs | `(C/P-1)*100` | 配对百分差 median | P/C MAD/median |
|---|---:|---:|---:|---:|
| 16×2048 | 15.470 / 17.070 | +10.342599% | +1.322792% | 54.04% / 23.84% |
| 16×2056 | 18.520 / 17.050 | −7.937367% | −10.752162% | 12.74% / 25.98% |
| 12×8192，guard-OFF | 16.190 / 17.960 | +10.932674% | −1.205542% | 34.22% / 21.77% |

两种 delta 统计量分别计算，不混用。完整 min/max、mean、CV、MAD、每块变化
与精度结果见 `local-analysis.json`；由 `support/analyze.py` 读取全部 raw 复算，
44 对/项的覆盖、交错顺序、配对百分差和精度行均通过断言，命令返回 0。

只对两项受影响输入汇总，观察到的 Local score=`0.992171392044`，
汇总 delta=`+0.789037863747%`。它们是无效测量中的实际数字，不能用于更新 Best。
同二进制 median 差超过 2%，所有 MAD/median 超过 5%，已不满足事前声明条件；
guard-OFF 也出现 +10.93% 的 median 差。故保留 `MEASUREMENT_BLOCKED`，
不把 2056 的负差值解释为已确认的布局收益。

原始数据：

- `logs/same-binary-20261008T053122Z-3760492-raw.tsv`
- `logs/local-20261008T053213Z-3769615-raw.tsv`

## 资源、命令结束与浏览器

各阶段目标设备 FREE_HBM 均为估算 57016 MB（65536 MB、使用率 13%）。
正式精度期间 AICore 17%→13%、AIVector 7%→3%；same-binary 为 9%→1%、
17%→15%；P/C 为 10%→17%、14%→8%。host load 的一分钟值分别为
66.21→63.71、90.57→86.20、60.57→69.50。各阶段都记录了其他 python 进程
占用 5758 MB；未杀停、暂停、迁移或修改其他用户任务与 lease。

两次 Compile、Correctness、same-binary、Local 均返回 0。源码传输比对、
离线统计命令已经结束。`diff` 返回 1 仅表示上述五行源码差异。
`logs/closure-state.txt` 确认本版 r04_probe 与构建进程均不存在；阶段成功标记只
表示命令返回 0，Local 有效性另按实际波动记录。`RUNNING_DEVICE_OPERATION=NONE`。

临时浏览器页面：早先创建请求与状态读取曾超时。用户要求收束时，通过内置浏览器
只读列出一个 2201 资料页，browserId=2、tabId=1，providerTabId 为
`browser-use:542575d3-1bb3-43b7-9e03-26eb599a5e4a`。创建回执缺失，无法可靠确认
归属，因此没有关闭任何标签页。没有对用户原有 cannjudge ranking 页面执行导航、
关闭或其他操作。此状态为 `TEMP_TAB_OWNERSHIP=UNCONFIRMED`，不声称已清理完成。

## 下一动作与边界

本次单版闭环已完成，交还 SLOT-4，由 Main 关闭本 Agent。Route 生命周期不变。
当前未证明第二个独立输入布局轴；不通过换偏移值继续造版本。

后续 R04 先复用本版二进制和这三项有来源的输入，研究现有 ACL event 计时波动：
核对 event 区间、host 发射间隔与设备任务时长能否区分，先取得可信 same-binary
依据再评价本版；保留全部旧样本，不等待独占设备。未在本轮运行该后续研究。
Parent 实际 UB 指针、Add 指令和 ResourceConflictRatio 的额外探测继续留待下一次
明确安排，不与当前交接混在一起。

官方 2201 文档的 48 banks/16 groups/32 B 模型和 SDK `ubbank_num=64` 的差异
仍按研究报告保留；V002 没有实测地址译码或指令冲突占比。R05 可继续复用已提交
共同资料，本 Agent 没有修改 R05 或共享记录。

```text
ROUTE_EVENT + VERSION_RECORD_EVENT
EVENT_ID=W4-R04-V002-20261008
AGENT_ID=01a119cc-fa30-7740-8fd2-9cdfab6e6c46
ROUTE=W4-R04
REVISION=V002
DIRECT_PARENT=R31B-V011
SINGLE_CHANGE=NarrowMid FP32 residual subview +64 float when D<=4032
COMPILE=PASS
CORRECTNESS=PASS; 9 FP32 cases against CPU FP64 reference; bitwise P/C equality
LOCAL_SCORE=0.992171392044 (observation only)
LOCAL_DELTA=+0.789037863747% (observation only)
LOCAL_BEST=NONE
STATUS=MEASUREMENT_BLOCKED
NEW_PERFORMANCE_REVISIONS=1
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BLOCKER=NONE
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=安全交接；后续复用二进制研究计时波动，不扫偏移或新增宽行探测
```

提交与最终 HEAD/dirty 由完成回执给出。本版实际文件均在本目录：

```text
CMakeLists.txt
DECLARATION.md
RESULT.md
parent.asc
submission.asc
local-analysis.json
support/
  analyze.py
  build_on_server3.sh
  run_stage.sh
  probe.cpp
  judge_types.hpp
  parent_entry.asc
  candidate_entry.asc
logs/
  compile-20261008T052403Z-3677186.log
  compile-20261008T052819Z-3724652.log
  server3-compile-20261008T052405Z-$.log
  server3-compile-20261008T052821Z-$.log
  transfer-compile-20261008.log
  runner-control-compile-20261008.log
  correctness-20261008T053033Z-3749912.log
  correctness-dispatch-20261008.log
  same-binary-20261008T053122Z-3760492.log
  same-binary-20261008T053122Z-3760492-raw.tsv
  same-binary-dispatch-20261008.log
  local-20261008T053213Z-3769615.log
  local-20261008T053213Z-3769615-raw.tsv
  local-dispatch-20261008.log
  source-transfer-verification.txt
  closure-state.txt
```
