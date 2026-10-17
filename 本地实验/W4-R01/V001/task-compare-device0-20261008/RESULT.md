# W4-R01 V001 同进程 P/P 与 P/C task-time 结果

## 结论

本次使用原 V001 Parent/Candidate 两份 kernel 库，在 device0、同一 runner 进程内完成有限 P/P 与真实 P/C task-time 对照。目标 `16x16384 FP16` 的 P/P 配对中位差 95% 区间为 `[-0.06, 0.00] us`，通过采前资格；P/C 配对区间 `[-0.16, 0.036] us` 跨 0，且 Parent/Candidate 组间漂移分别为 `10.63% / 16.05%`，超过事前 `10%` 限线。P/P 调整后中位差 `-0.04 us`，95%区间 `[-0.24, 0.06] us` 也跨 0。结论为 `MEASUREMENT_BLOCKED`：当前证据无法确认 V001 有收益，也不能据此认定两实现等效或 V001 无收益。

原始 P/C 两侧中位数为 `9.50 / 8.91 us`（`-6.2105%`）；这个侧中位数比值没有通过配对、漂移和 P/P 调整判据，不能作为 Local 提升。`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`CURRENT_LOCAL_BEST=NONE`。没有创建 V002。

## 预先声明与实际运行

- 继续 V001，Direct Parent=`R31B/V011`；kernel 源码、ASC wrapper、ABI、CMake 和 Parent/Candidate 库均未修改。
- `cann-server3` / `hwnput3`，Ascend910B3，CANN `8.5.0.alpha002`，GCC `11.4.0`，device0。
- 一个 profiler 采集和 runner 进程 PID `3775510`；两库同时驻留。按声明依次处理 `16x16384` 目标和 `16x32768` OFF 对照，`blocks=8`、`epsilon=1e-5`。
- 每个输入四组；PP/PC 阶段先后分别两组。每阶段 45 对预热、8 个固定循环，每循环一个未计时 priming 与 8 个 timed calls；每种比较每输入 Parent 与第二槽各128条 timed 样本。两个输入合计1024条 timed、1440次预热 kernel、128次 priming、8次采前正确性 kernel，共2600条 kernel 调用。
- 循环按 `PPPCCPCC` / `CCCPPCPP` 交替，P/P 与 P/C 均实际覆盖两种前序函数、两个位置；输出 A/B 映射对称交换。原始 event 与 wall 样本一并保留。
- 2026-10-08 12:22:09–12:22:12 UTC 纯 host 编译成功；12:23:35–12:23:52 UTC 一次 msprof 采集成功。未追加重跑。

device0 采前与采后估算可用 HBM 均为 `57016 MB`。AICore `1%→0%`，AIVector `2%→3%`，host load1 `41.71→38.55`。NPU 快照中已有 device0 Python PID `251901` 使用 `5752 MB`，未触碰。负载只作背景，非停止条件。

## task-time 数值

每侧每种比较128个样本。`side median delta` 是两侧总体中位数之比；`paired delta` 是同设计配对的 `C_i-P_i` 中位数，统计定义不同。

| Shape / 对照 | Parent median µs | 第二侧 median µs | 两中位数差 | 配对 task 差 median [95%区间] µs | 组内 MAD / 漂移资格 |
|---|---:|---:|---:|---:|---|
| 16x16384 PP | 8.82 | 8.80 | -0.2268% | -0.020 [-0.060, 0.000] | PASS / PASS |
| 16x16384 PC | 9.50 | 8.91 | -6.2105% | -0.080 [-0.160, 0.036] | PASS / FAIL |
| 16x32768 PP | 15.70 | 15.72 | +0.1274% | +0.020 [-0.078, 0.110] | PASS / PASS |
| 16x32768 PC | 16.04 | 15.99 | -0.3117% | -0.080 [-0.200, 0.040] | PASS / PASS |

| Shape | P/P 调整后 PC−PP median µs | 95%区间 µs | 方向可确认 |
|---|---:|---:|---|
| 16x16384 | -0.040 | [-0.240, 0.060] | 否，跨0 |
| 16x32768 | -0.120 | [-0.400, 0.060] | 否，跨0 |

两种 shape 的 PC 原始配对区间也均跨0。`16x16384` 目标 PC 的 Parent 与 Candidate 组漂移比分别为 `10.63%`、`16.05%`，资格不通过；OFF 对照均低于10%，但其调整后区间跨0。各组、阶段先后、前序、位置、映射与首循环统计均在 `result.json` 中，所有值来源于固定样本，不筛选。

## event、wall 与 host API 诊断

目标 PP event 两槽中位数 `32.35 / 31.86 us`，配对差中位 `-0.370 us`，95%区间 `[-2.780, 3.520]`；目标 PC event 为 `31.41 / 32.01 us`，配对差中位 `+0.150 us`，区间 `[-1.840, 3.500]`。PC wall 中位数 `95.386 / 97.872 us`。它们只作诊断，不替代 task 计时范围。

1024/1024 timed 调用均按实际 host 事件连接号与 profiler 记录对应；每次 RecordEvent 区间内观察到一次 `Runtime@DevMalloc` 和一次 `AscendCL@aclrtFree`。请求字节数、对象、用途仍为 `UNKNOWN`。host API 时长区间会重叠，不能与 event/task 相加来推导总耗时。

## Correctness

Parent 与真实 Candidate 在采前、采后都分别对照原 CPU FP64 reference（最终 FP16），并做 P/C 位级比较。两个 shape 均 P/C 逐位相同，reference 模板判定均 PASS；严格逐元素判定仍 FAIL：`16x16384` 每侧1个超差元素，`16x32768` 每侧3个，最大绝对误差均 `0.00390625`。原 `atol=rtol=0.001`、允许失配比例 `0.001` 均未改变。不得描述为全元素严格正确。

## 证据与状态

- `comparison-profile.log`：runner 与逐阶段摘要、采前/采后 reference、函数/地址及资源信息。
- `pcstudy-r16-d*-g*-*.tsv`：16份 PP/PC 原始双侧 event/wall 样本。
- `pcstudy-r16-d*-calls.tsv`：每个输入1300条完整 host 调用台账，含前序函数/地址。
- `comparison-profile/`：完整 msprof 原始数据库、timeline、task_time 与 op_summary 导出。
- `all-calls.tsv`：2600条 kernel 与task/event逐项对应；`result.json`：统计结果；`analysis.stdout.log`：摘要；`analysis.stderr.log`：空，最终分析通过。
- Parent/Candidate 库前后尺寸与 mtime一致；该信息仅为文件元数据，不声称整库字节相同。
- `OFFICIAL_SCORE=NONE`、`ONLINE=PAUSED`、`PUSH=NO`；新性能版0、有效 Local 0、连续无改善贡献0。Route 生命周期未改变。
- 一次早期 host 编译失败和随后成功编译记录见 `build-attempts.md`；失败原因仅为诊断 host 代码引用未定义变量，修后编译通过。
