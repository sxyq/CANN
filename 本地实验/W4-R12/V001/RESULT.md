# W4-R12 V001：单行编译期分派

```text
ROUTE = W4-R12
REVISION = V001
DIRECT_PARENT = R31B-V011
PARENT_SOURCE_COMMIT = 43a1049a1e08e518c88e354a754fdebb85a96f99
PARENT_SOURCE_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE = 本地实验/W4-R12/V001/submission.asc
CANDIDATE_SOURCE_SHA256 = 04c158a5a617f88738351014154fa7645a5fcb47568c5830e6833d2806e83994
CHANGE = rowCount=1 且 blockCount=1 时实例化 Process<true>，将 beginRow/localRows 固定为 0/1，省去设备侧 block-row 划分计算；其余输入保持原路径。
COMPILE = PASS
CORRECTNESS = PASS（本记录所列 1x64 FP32 代理输入）
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
LOCAL_VERDICT = MEASUREMENT_BLOCKED
CURRENT_LOCAL_BEST = NONE
OFFICIAL = NOT_SUBMITTED
PUSH = NO
```

## 实现与来源

候选源码由 `线上结果/R31B/V011/submission.asc` 派生。单一性能变化是给 `Process` 增加布尔模板参数，并在 `run_kernel` 对单行、单 block 情况调用该实例；FP32/FP16/BF16 均保留，其他行数继续使用原通用实例。数值运算、内存布局及单行以外路径没有改动。

该方向来自本 Route 的 `TINY-ENTRY-CODEGEN-STUDY.md`：Parent 的 host 入口把动态 `rowCount`、`rowWidth`、`blockCount` 传入设备入口；可读设备指令尚未取得。V001 用实际构建和测量检验编译期单行分派是否带来收益，不声称已从设备指令确认优化效果。

## Compile 与 Correctness

server3 `hwnput3`，CANN `8.5.0.alpha002`，Ascend910B3 / `dav-2201`，设备 4。首次环境载入在 shell `nounset` 下退出；第二次缺少编译器插件所需的 `<vector>` 头文件；第三次沿用本 Route `remote.sh` 的环境加载和 C++ include 配置后构建成功。完整输出保存在 `server3/compile-attempt-02.log`、`server3/compile-attempt-03.log`。

既有 runner 使用 `1x64 FP32`，epsilon `1e-5`，`availableCoreNum=8`，实际 `blockCount=1`，60 次预热、124 次计时。64 个输出逐元素与 CPU FP64 reference 比较，容差为 `2^-16 + 2^-10 * abs(reference)`，并要求绝对误差不超过 `0.01`。Candidate 未加 profiler 运行一次并在 C1/C2 两次采集时各运行一次；每次均为 64/64 通过，最大绝对误差 `1.9249022242817659e-7`。Parent 的 P1/P2 运行也通过相同 reference。

## Local 观察

Parent 与 Candidate 共用本 Route 已有 `parent_probe.asc`、相同输入和运行参数。profiler 顺序为 P1 → C1 → C2 → P2，每轮 124 个计时调用并导出 task/event 对应关系。各轮为独立进程，stream ID 记录在对应 task map 中；输出地址只在每个进程内部复用，因此跨进程的 stream 和地址没有固定为同一值。未删长尾样本。

四轮合并观察如下。百分比按 `(Candidate median / Parent median - 1) × 100%` 计算，只作为诊断观察值，不作为有效 Local 分数。

| 指标 | Parent，n=248 | Candidate，n=248 | 观察差异 | 稳定性 |
|---|---:|---:|---:|---|
| Device event median | 14.7000 µs | 14.3100 µs | -2.65% | `MEASUREMENT_BLOCKED`；MAD/median 分别 62.72%、59.89% |
| Kernel task median | 3.2600 µs | 3.4400 µs | +5.52% | `MEASUREMENT_BLOCKED`；MAD/median 分别 20.86%、25.58% |
| Host wall median | 136.1705 µs | 144.6555 µs | +6.23% | `MEASUREMENT_BLOCKED`；MAD/median 分别 25.96%、28.43% |

P1/P2 task 中位数为 `3.45 / 2.99 µs`；C1/C2 为 `3.43 / 3.44 µs`。Candidate 两轮中心接近，但 Candidate 池的 task MAD/median 仍为 25.58%；Parent 两轮中心差为 14.11%，Parent 池也未稳定。设备 event 结果同样长尾显著。task-time 保留为诊断列，不替代现行 Local 主指标。

无 profiler 的 Candidate 124 个 event 样本中位数为 `12.0600 µs`，MAD/median `50.00%`，两 block 中位数漂移 `4.64%`。它没有同窗口 Parent 对照，不能单独形成 Local 比较。

采集时设备 4 的 HBM 为 65536 MB、使用率 90%，按现有计算约 `6553 MB FREE_HBM`；ACL 进程内读数约 `6285–6292 MiB`。AICore 约 55–67%、AIVector 约 29–36%，HBM bandwidth 约 63–66%。负载仅作背景记录。所有 runner 均结束，`RUNNING_DEVICE_OPERATION=NONE`。

结论：当前样本支持 `LOCAL_VERDICT=MEASUREMENT_BLOCKED`。虽然 task-time 汇总的 Candidate 中位数高于 Parent 5.52%，两侧都未达到本次同二进制稳定表现；device event 分布更宽，不能据此判为 Candidate 负收益或 Local Best。没有更换窗口或继续采样寻找有利数值。

## 原始证据

- `profiles/P1.raw.tsv`、`profiles/C1.raw.tsv`、`profiles/C2.raw.tsv`、`profiles/P2.raw.tsv`：四轮逐调用 event/wall 原始值。
- `profiles/*-msprof/`：对应 Profiler 原始数据、task-time、op-summary、API 统计与数据库。
- `profiles/*.analysis.json`、`profiles/*.analysis.task-map.tsv`：复用既有 `parent-tiny-20261008/analyze.py` 生成的逐调用映射及统计。
- `profiles/candidate-unprofiled.raw.tsv`、`profiles/candidate-unprofiled.reference.tsv`、`profiles/candidate-unprofiled.analysis.json`：无 profiler Candidate 运行原始数据和 CPU reference。
- `server3/correctness-local-session.log`、`server3/profile-pc-cp.log`：命令输出、资源快照、Correctness 与运行结束信息。
- `server3/compile-attempt-02.log`、`server3/compile-attempt-03.log`：编译失败与最终成功输出。
- `server3/r12_candidate_probe`：对应本 Candidate 的编译产物。

本次不创建新测量 runner，不改共享记录，不提交线上评测。原远端对象保留在 `/home/data4t2/lelinfeng/server_runs/W4-R12/candidate-v001-20261009/`。
