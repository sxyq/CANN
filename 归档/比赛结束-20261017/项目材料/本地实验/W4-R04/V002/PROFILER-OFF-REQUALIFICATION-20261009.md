# W4-R04 V002 无 profiler 重资格结果

## 结论

本次只执行了预先固定的 Parent/Parent（P/P）同二进制阶段。三项输入的重复性均未达到既定范围，故不启动 Parent/Candidate（P/C），也不接受 Local 成绩。`CURRENT_LOCAL_BEST=NONE` 保持不变，原 V002 Local 记录未改写。

本次 profiler 已关闭，Candidate 与 Parent 源文件及输入 shape 均沿用 V002。需要注明：上一份 profiler 归因采集使用 device 1，而现有 `run_stage.sh` 固定使用 device 3；本次沿用该 runner，因此设备也不同，不能把两次结果的差异单独归于 profiler 开关。遵照本任务的一次运行限制，不再换设备补跑。最终状态为 `MEASUREMENT_BLOCKED`。

## 版本与源码

```text
ROUTE=W4-R04
REVISION=V002
DIRECT_PARENT=R31B-V011
RUNNER=support/run_stage.sh same-binary -> r04_probe --mode same-binary
PROFILER=OFF
CANDIDATE_EDIT=NONE
NEW_PERFORMANCE_REVISION=NO
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=2d2e7da242ab9796d3ab6eeb6e86bd69521e1d40537a15f65e9cfb946cd41475
REMOTE_SOURCE_MATCH=YES
COMPILE=PASS (V002 existing build; no rebuild)
CORRECTNESS=PASS (this run's three inputs; existing V002 nine-input result also PASS)
```

Parent 源文件为 `parent.asc`，Candidate 源文件为 `submission.asc`。本机与 server3 两个源文件摘要一致。当前 runner 每个 shape 先执行 CPU FP64 reference 比较，随后每个计时调用按 `aclrtRecordEvent(start) → single kernel launch → aclrtRecordEvent(stop) → aclrtSynchronizeEvent → aclrtEventElapsedTime` 取 device event 时间；命令未启动 msprof。

## 事前固定的样本方案

每项输入分别预热 45 次/库，随后运行 4 块 × 11 对，即每项 44 对、88 个单次 event 样本。三项共 132 对、264 个单次 event 样本。runner 以 `(block + pair)` 奇偶交错 `P-C` 与 `C-P`，每种次序 22 对/shape。P/P 阶段的两次调用都解析到 Parent 同一动态库。资格条件固定为：三项均须满足 P/P 两列中位数比值差绝对值 ≤2%，且两列各自 MAD/median ≤5%。结果出来后没有改变样本数、顺序或条件。

| Shape / dtype | P/P 样本/侧 | Parent 中位数 µs | 同库第二列中位数 µs | 中位数比值差 `(B/A-1)` | 配对差中位数 | Parent MAD/median | 第二列 MAD/median |
|---|---:|---:|---:|---:|---:|---:|---:|
| 16×2048 FP32 | 44 | 13.320000 | 14.340000 | +7.657660% | +3.360574% | 46.9970% | 26.1506% |
| 16×2056 FP32 | 44 | 13.380000 | 15.910001 | +18.908824% | +4.139940% | 41.1061% | 21.2445% |
| 12×8192 FP32（guard-OFF） | 44 | 9.750000 | 8.520000 | −12.615385% | −1.546114% | 35.6923% | 28.6385% |

P/P 两列中位数差绝对值及 MAD/median 均超出既定范围，三项均未通过。表中两种差值只描述同一 Parent 库在两列/配对次序下的观察，不表示 Candidate 性能变化。故本次 `LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`，不更新 Local Best。

| Shape | P/P 第一位置中位数 µs | P/P 第二位置中位数 µs | 第二位置相对第一位置 |
|---|---:|---:|---:|
| 16×2048 FP32 | 14.230000 | 13.330000 | −6.324665% |
| 16×2056 FP32 | 14.760000 | 15.240000 | +3.252032% |
| 12×8192 FP32（guard-OFF） | 8.520000 | 9.750000 | +14.436620% |

本次 P/C 未运行，因此 profiler-off 的 `P-C`/`C-P` Candidate 次序差为 `NOT_MEASURED`。两种次序在 P/P 中各占一半；该 P/P 位置观察不能代替 P/C 的顺序效应结果。

## 正确性与每次调用关联

本次运行对三项实际输入的 Parent 与 Candidate 都通过 CPU FP64 reference：`atol=1e-5`、`rtol=1e-4`、`max_abs_limit=1e-2`；逐元素差异为 0。最大绝对误差依次为 `7.15255737305e-7`、`7.15255737305e-7`、`4.76837158203e-7`。P/P 的计时调用虽只用 Parent 库，runner 仍按原逻辑完成每项 reference 比较。

`same-binary-20261009T052944Z-1880917-calls.tsv` 将原始 132 对展开为有序的 264 次调用，含全局 `call_sequence`、shape、block、pair、P-C/C-P 次序、对内位置和单次 device event 时间。`task-event` 任务编号未启用 profiler 后不可取得；每行的 event 数值仍按 runner 内一次 start/launch/stop/synchronize 调用对应。未推造硬件 task ID。

## 原始证据与设备状态

原始文件完整保留，无样本剔除或替换：

- `logs/profiler-off-qualification-20261009/same-binary-20261009T052944Z-1880917-raw.tsv` — 132 对原始记录；
- `logs/profiler-off-qualification-20261009/same-binary-20261009T052944Z-1880917-calls.tsv` — 264 次调用顺序展开；
- `logs/profiler-off-qualification-20261009/same-binary-20261009T052944Z-1880917.log` — 完整运行输出与运行前后资源快照。

```text
HOST=hwnput3
DEVICE_ID=3
AVAILABLE_VECTOR_CORES=40
TOOLKIT=CANN 8.5.0.alpha002
SOC=Ascend910B3 / DAV-2201
HBM_CAPACITY_MB=65536
HBM_USAGE_RATE_BEFORE_AFTER=5% / 5%
FREE_HBM_MB_BEFORE_AFTER=62259 / 62259
AICORE_USAGE_BEFORE_AFTER=0% / 0%
AIVECTOR_USAGE_BEFORE_AFTER=0% / 0%
DEVICE_PROCESS_SNAPSHOT=No process in device (before and after)
HOST_LOAD_AVERAGE_BEFORE=63.67, 55.84, 51.14
HOST_LOAD_AVERAGE_AFTER=60.09, 55.23, 50.96
RUN_UTC=2026-10-09T05:29:44Z–2026-10-09T05:29:47Z
RUN_EXIT_CODE=0
RUNNING_DEVICE_OPERATION=NONE
```

## 事件与状态

```text
ROUTE_EVENT
ROUTE=W4-R04
REVISION=V002
LAST_ACTION=Profiler-off P/P 同二进制重资格运行
NEXT_ACTION=结束本次 R04 任务
CHANGE=测量时关闭 profiler；源码、二进制和输入保持 V002；设备与前次 profiler 归因采集不同
COMPILE=PASS (existing V002)
CORRECTNESS=PASS
FREE_HBM_MB=62259
DEVICE_ID=3
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
PUSH=NO
BLOCKER=NONE
RUNNING_DEVICE_OPERATION=NONE
```

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R04-V002-PROFILER-OFF-20261009
EVENT_KIND=PROFILER_OFF_PP_REQUALIFICATION
ROUTE=W4-R04
REVISION=V002
DIRECT_PARENT=R31B-V011
COMPILE=PASS (existing build)
CORRECTNESS=PASS
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
MEASUREMENT_STATUS=MEASUREMENT_BLOCKED
P_P_PAIRS=132
P_P_SINGLE_CALL_SAMPLES=264
P_C=NOT_RUN (P/P did not meet the predeclared qualification conditions)
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=UNCHANGED
PUSH=NO
EVIDENCE=本地实验/W4-R04/V002/PROFILER-OFF-REQUALIFICATION-20261009.md; 本地实验/W4-R04/V002/logs/profiler-off-qualification-20261009/
ROUTE_LIFECYCLE_CHANGED=NO
```
