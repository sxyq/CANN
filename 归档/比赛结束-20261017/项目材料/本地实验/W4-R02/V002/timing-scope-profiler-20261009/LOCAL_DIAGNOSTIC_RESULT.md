# W4-R02 V002 profiler 开/关对照

## 判定

`LOCAL_DIAGNOSTIC_RESULT=COMPLETED_DIAGNOSTIC_ONLY`；`LOCAL_SCORE=NONE`；`VALID_LOCAL=NO`。按预先固定的样本与统计口径，profiler-off 的 event 中位数在 P/P、P/C 均低于 profiler-on；这个方向与 profiler 改变 event 观测的假设相符。但各次运行处在不同时间段且 stream handle 不同，event 稳定性也没有形成可用 Local，因此不能将差值单独归因于 profiler 开销。

## 固定身份和采样

- Route/Revision：W4-R02 / V002；任务授权：`W4-PLANNING-DIAGNOSTIC-01`；直接 Parent：R31B-V011。
- Candidate 来源 commit：`60ca277d2afc3d985965e16ca09d1ff9b03f8c10`。Parent SHA256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`；Candidate SHA256：`b512579409b6614002f4bd7463d9a243b13d2bec0723094e52cc8c2b5c35bf38`。本次未改 Candidate、runner 或统计定义。
- 远端复用二进制 SHA256：`r02_runner=5952a7e97db1f28b2b06e6050a1fe879cfb2f4ea1c3ed6bf2fb1337bfe2cd8fc`；`libr02_parent.so=b4947bfd9ef5477ee66729c05f5231b78239718e354d3b3ff48881b0e62c469a`；`libr02_candidate.so=d13382aa3b6d425ea983a8a7282fa2368c9fce9fba71003358e8cf8a9756bdca`。
- shape/dtype/input：128x20480 FP16；四条件均用同一确定性输入生成器 `W4_R02_PROXY_128XWIDTH_DETERMINISTIC_V1`、40 cores、epsilon `1e-5`、现有 runner 与 event/wall 计时边界。
- 设备：device 1。四条件顺序严格按计划执行：profiler-on P/P、profiler-off P/P、profiler-off P/C、profiler-on P/C。每个逻辑槽预热 45 次；4 组×21 对；每条件 84 对、168 个 event/wall 行；PC/CP 交替各 42 对。所有原始行保留，`samples_omitted=0`。
- P/P 两个逻辑槽均执行 Parent；P/C 对照 Parent 与 V002 Candidate。同一条件内 P/C 共用输入、stream 和输出槽。四次新进程记录的 device 输入/输出地址相同（输出 `0x12c041e00000`），但 runner 每次启动新建 stream，实际 stream handle 各不相同；不把地址相等解释为 runner 保证跨进程复用。
- profiler-on 使用已有 msprof 参数 `--ai-core=off --task-time=on --ascendcl=on --runtime-api=on --aicpu=off`。profiler-off 直接运行同一 runner。

## 统计观察

既定诊断界值为 MAD/median ≤10%、四组中位数极差/总体中位数 ≤10%；不剔除异常值。数值均为微秒，P/P 的 P/C 表示 Parent 算法在两个逻辑槽的位置。

| 条件 | Event pooled median；MAD/median；组间范围比 | Event P / C median | Wall median | Kernel-task pooled median；MAD/median；组间范围比 |
|---|---:|---:|---:|---:|
| profiler-on P/P | 49.870；13.27%；6.42% | 50.280 / 49.770 | 108.121 | 21.000；0.381%；0.905% |
| profiler-off P/P | 32.310；22.16%；13.40% | 31.710 / 32.950 | 87.441 | 不可用：runner 未采 task/API |
| profiler-off P/C | 26.590；10.30%；18.69% | 30.950 / 25.160 | 76.981 | 不可用：runner 未采 task/API |
| profiler-on P/C | 34.860；21.86%；23.41% | 34.720 / 35.100 | 89.025 | 21.480；1.862%；21.17% |

- P/P event 开关差：off 比 on 低 17.560 µs（off/on=0.648）；但 off 侧 MAD 与组间范围均超限。on 侧 event MAD 超限；其 kernel-task 两逻辑槽均满足既定稳定性界值。
- P/C event 开关差：off pooled 比 on 低 8.270 µs（off/on=0.763）；两侧 event 至少一项稳定性指标超限。off 下 P/C 各槽 median 差为 -5.790 µs、配对差中位数 -0.500 µs；on 下 P/C 各槽差 +0.380 µs、配对差中位数 -0.160 µs。不同进程和时间段的差异不可作为 Candidate 性能结论。
- 先后位置：P/C event 配对差中位数（PC / CP）分别为 on `-0.360 / -0.030 µs`、off `-0.480 / -0.510 µs`；second-minus-first 中位数分别为 on `-0.230 µs`、off `-0.150 µs`。P/P 同算法两逻辑槽的 second-minus-first 为 on `-0.880 µs`、off `-1.210 µs`。这些是描述性诊断量，不改变资格判断。
- profiler-on 的 168 个计时调用均映射到 task-time 数据；全部 258 条 op-summary 与 task-time key 对应，168 条计时调用均被前后 EVENT_RECORD 包围。P/P task-time pooled median 21.000 µs 且通过既定稳定性界值；P/C task-time pooled median 21.480 µs，但四组范围比 21.17%，不通过。profiler-off 没有 task/API 轨迹，未补算。

## 假设与证伪

- `HYPOTHESIS`：msprof 开关会改变 ACL event/wall 观测的额外区间或离散程度，而 kernel-task 时间可能更稳定。
- `OBSERVATION`：P/P 和 P/C 的 profiler-off event pooled median 均较低；profiler-on P/P 的 task-time 稳定，而同条件 event dispersion 超过界值。P/C on/off event 统计都不满足完整稳定性条件。
- `FALSIFICATION_RESULT`：未被这批数据证伪；观察方向与假设一致。因 profiler 状态与运行时间/stream 同时改变，证据不足以独立证明因果关系。
- `MEASUREMENT_SCOPE`：同设备、shape、dtype、确定性输入、runner、计时边界、预热/对数及同进程内交错顺序的 profiler 开关诊断；不是资格重测，也不是 Local 比较。
- `CONFIDENCE=LOW`：event 中位数差明显，但条件未通过稳定性界值，且存在进程 stream 与运行时负载时序混杂。

## 不支持项与限制

1. runner 不接受外部 stream 或设备指针，无法让 profiler-on/off 复用同一个 `aclrtStream` 句柄。观察到四进程的物理输入/输出地址恰好一致，但只有进程内共享由实现保证。
2. profiler-off 只输出 event/wall；不会生成 task-time、AscendCL/runtime API 轨迹，故无法做开关两侧 task/API 时间配对。
3. profiler-on 保留原始 host/API 数据库、profiler 日志、op-summary/task-time CSV 和完整调用映射；实现未提供稳定的逐调用 API-ID 与 runner 样本 ID 对照字段。可复核的是按采样序号的 168 个 event→kernel-task 映射，不宣称独立 API-ID 完整一一对应。
4. 负载随时间变化（各阶段 host load、HBM 和 AICore 见对应 raw 快照）；规则要求这些只作上下文。没有看进程列表、没有干预其他任务。
5. 首次远端包装调用 RC=1，确认 runner 未启动且远端采集目录未创建；随后正式四条件各运行一次且 RC=0。启动尝试的可见终端输出和事实记于 `startup-attempt.txt`，不计样本、不重跑。

## Raw 与可复核分析

- 全量原始文件（event、wall、profiler host/API/device 原始内容、SQLite、op/task CSV、命令、环境、返回码、时钟、HBM/usages、主机负载）：`raw/`，SHA-256 列表为 `raw-sha256.manifest`。
- profiler-on 派生统计与 task 映射：`on-pp-analysis.json`、`on-pc-analysis.json`、`on-pp-task-map.tsv`、`on-pc-task-map.tsv`。
- 四条件统一统计：`profiler-on-off-analysis.json`。
- 计划先于设备运行保存于 `analysis-plan.md`。

`PUSH=NO`；不提交 Online。该诊断不改变 V002 的既有 `MEASUREMENT_BLOCKED`，不授权新增 V003。
