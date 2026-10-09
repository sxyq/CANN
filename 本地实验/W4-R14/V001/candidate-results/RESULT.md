# W4-R14 V001 结果

```text
ROUTE = W4-R14-PARAM-DMA-GRANULARITY-X
REVISION = V001
DIRECT_PARENT = R31B V011
WORKTREE = /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R14-param-dma-granularity-x
BRANCH = w4/r14-param-dma-granularity-x
PARENT_SOURCE = 线上结果/R31B/V011/submission.asc
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE = 本地实验/W4-R14/V001/submission.asc
CANDIDATE_SHA256 = df7ddefea478c6e8da7d83ddb8df9788522794ef145ae8c5f68d4fa88cc634de
OFFICIAL = NOT_SUBMITTED
PUSH = NO
```

## 单一变化与可证伪预期

Candidate 只改 `ProcessFp32FullRowOutputPipelined` 的 gamma、bias 预载：同一 GM 张量中相邻的两段 4096 元素拷贝，分别合成一段 8192 元素拷贝。每核参数命令数预期由 4 降至 2；总传输字节数、UB 缓冲容量、参数驻留时间、分派和算术顺序保持不变。

此路径要求 `rowWidth == kCacheElems` 且每核拥有多行。测试的 `80×8192 FP32` 按 40 个核分配，每核两行，满足源码分派条件。可证伪预期是：若参数 MTE2 命令发射成本对端到端时间有可测贡献，则同一输入、同一评测时点下该路径的整体时间应下降；没有可复现下降则不支持此变化有已测收益。单次 Local 不能判定这一点。

父版已记录每核 4 条参数命令、每次 launch 共 160 条；设备 0 的参数时间估算为 `1.223342600 us`，由字节占比推导的乐观节省估算为 `0.611671300 us`。参数专属直接耗时仍为 `UNKNOWN`，该节省值不是实测结果，也不是严格上界。设备 3 的父版 kernel-task 中位数为 `8.320000 us`，同进程噪声幅度为 `8.028400 us`；设备事件噪声幅度为 `15.020000 us`。这些数量说明该估算远小于现有波动，不能当作 Candidate 收益证据。

DMA 语义仍由已有 `Load` helper 执行 `DataCopyPad`。本变化只放大单次 `DataCopyExtParams.blockLen`：8192 个 FP32 为 32768 字节，低于现有 API 资料记载的 `2^21-1` 字节上限；块数为 1，连续 GM 源和容量为 8192 元素的 UB 目标均满足现有实现约束。Parent 与 Candidate 的唯一区别是该处两行 `Load` 变为每个参数张量一行 `Load`。

这与既有参数预取时序、槽位相位或驻留变化不同，也没有沿用输入 x/residual 的多行 DMA 变化；变更目标是参数张量本身的单次搬运粒度。此前的 Parent-only profile 仍保留为独立研究证据，本版额外产生了 Candidate 源码、编译、精度和 Local 结果。

## Compile 与 Correctness

```text
COMPILE = PASS
CORRECTNESS = PASS
DEVICE = hwnput3 / NPU 3 / IT21HMDC_Bin6
TOOLKIT = CANN 8.5.0.alpha002
NPU_ARCH = DAV-2201
```

最终 Compile 将 `candidate-support-v001` 内的候选实现编入 `r14_parent_probe` 并成功链接，产物大小 485224 字节。原始完整输出见 `compile-attempt-separate-build.log`；此前未通过的尝试也保留在同目录 `compile-attempt-*.log` 与 `compile.log` 中，没有删去。

Parent 与 Candidate 分别以 `80×8192 FP32`、device 3、40 个可用核执行 CPU FP64 参考比较。两者均为 0 个超差元素，最大绝对误差 `7.6549910899e-7`；容差为 `2e-5 + 1e-4 × abs(reference)`，输入种子 `322351`。完整输出分别见 `parent-round1.log` 和 `candidate-round1.log`。

## Local

每个源码身份独立启动一次同结构 runner，使用相同输入生成方式与设备；各自先做 45 次 warmup，再采 2 个 block、每 block 21 对同源码 P/P，共 84 个 device-event raw samples。P1/P2 在各自进程内交错。Candidate 与 Parent 在不同进程、不同时间运行，没有组成同进程的跨源码配对；采样先后为 Candidate `2026-10-09T12:28:16Z`、Parent `2026-10-09T12:29:53Z`。wall time 另行保留作诊断。

| 数值 | Parent | Candidate |
|---|---:|---:|
| raw samples | 84，`parent-round1.log` | 84，`candidate-round1.log` |
| median latency | 22.1299994735 us | 25.2100005745 us |
| MAD | 4.0999995545 us | 4.8800008370 us |
| MAD / median | 18.5269% | 19.3574% |
| p10–p90 | 16.0999998453–35.3240005674 us | 15.1959997600–35.7159990813 us |
| within-run P/P absolute-difference p90 | 23.7839994955 us | 20.4699994993 us |
| two-block median range | 3.2799998305 us | 3.8400003685 us |
| within-run repeatability floor | 23.7839994955 us | 20.4699994993 us |

```text
SHAPE / DTYPE = 80×8192 / FP32
DEVICE = NPU 3; physical / available vector cores = 40 / 40
FREE_HBM_MB = 27525
LOCAL_SCORE = 25.2100005745 us (Candidate median latency)
LOCAL_DELTA = +13.9177640049% (Candidate slower by 3.0800011010 us)
MAX_WITHIN_RUN_FLOOR = 23.7839994955 us
CURRENT_LOCAL_BEST = NONE
INTERPRETATION = observed Candidate median is slower; the 3.0800011010 us difference is below the largest within-run P/P floor, and separate-process timing prevents a causal P/C conclusion
```

两次运行时设备负载都较高且不同。Candidate 启动时 load average 为 `95.52 / 72.26 / 58.56`，AICore/AIVector 使用率为 `43% / 15%`；Parent 启动时为 `131.65 / 92.31 / 67.27` 与 `24% / 11%`。两个阶段都有 VLLMEngineCor 进程驻留。原始设备和进程快照在两份 Local 日志内。负载只作为结果背景，不改变 Official 处理。

## Judge 时点背景

以下为用户提供的 14 个不同 W4 源码身份相对已保存 R31B V011 的汇总，只保留 Case ID，不据此推断输入形状：

| Case ID | 14 个身份中的结果方向 |
|---|---|
| 03 / 08 / 15 | 各 14/14 较慢 |
| 11 | 13/14 较慢 |
| 01 | 14/14 较快 |
| 14 | 11 个较慢、3 个较快 |

该分布显示 Official 比分会随 Case 与评测时点呈现方向差异；不能把跨源码身份的分数变化单独归因于本 Candidate。Local 也没有覆盖未知 Case，且本次独立进程测量自身已显示较大波动。上述信息只用于解释 Judge 参考结果和时间变化，不替代 Judge 返回值。

## 证据位置与状态

- 父版路径与 SHA256：`线上结果/R31B/V011/submission.asc`；字节一致副本为 `归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc`。
- Candidate：`本地实验/W4-R14/V001/submission.asc`。
- Parent 参数命令、设备 profile、kernel-task 与测量噪声：`本地实验/W4-R14/V001/PARAM-MTE2-FINDINGS.md`、`parent-probe/task-time-20261008/analysis.json` 与 `parent-probe/task-time-20261008/80x8192_fp32/`。
- Compile：`本地实验/W4-R14/V001/candidate-results/compile-attempt-separate-build.log`。
- Correctness 与完整 Parent/Candidate raw samples、设备状态：`parent-round1.log`、`candidate-round1.log`。

```text
STATUS = REAL_CANDIDATE_COMPLETE
COMPILE = PASS
CORRECTNESS = PASS
LOCAL_SCORE = 25.2100005745 us
LOCAL_DELTA = +13.9177640049%
LOCAL_BEST = NONE
OFFICIAL_SCORE = NONE
ONLINE = NOT_SUBMITTED
PUSH = NO
```
