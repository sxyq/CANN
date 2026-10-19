# W4-R14 V002 结果

```text
ROUTE = W4-R14
REVISION = V002
DIRECT_PARENT = R31B V011
PARENT_PATH = 线上结果/R31B/V011/submission.asc
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE = 本地实验/W4-R14/V002/submission.asc
CANDIDATE_SHA256 = ae2c1788e005c5a2daa78b1763bd6a32fd229eabfb8a612f95c0c8b196a3dea0
OFFICIAL = NOT_SUBMITTED
PUSH = NO
```

## 单一变化

在 `ProcessFp32FullRowOutputPipelined` 中，仅将 gamma 与 bias 的两次参数预载由 `DataCopyPad` helper 调用改为直接 `DataCopy`。每个张量搬运 8192 个 FP32，即 32768 字节、1024 个 32B block；传输命令数、地址范围、UB 容量、驻留时间和计算顺序不变。该变化测试 API 形式，不重做 V001 的参数命令数量变化。

源代码唯一差异见 V001/V002 对照；API 形式参考仓内既有对齐 `DataCopy` 用法。V002 已在 DAV-2201 上编译并运行通过。

## Compile 与 Correctness

```text
COMPILE = PASS
DEVICE = NPU 0 / 40 vector cores
FREE_HBM_MB = 8519
TOOLKIT = CANN 8.5.0.alpha002
NPU_ARCH = DAV-2201
CORRECTNESS = PASS
SHAPE / DTYPE = 80x8192 / FP32
REFERENCE = CPU FP64; atol=2e-5, rtol=1e-4, seed=322351
MISMATCHES = 0
MAX_ABS_ERROR = 7.6549910899e-7
```

完整构建输出：`candidate-results/compile.log`。设备端参考比较与试跑样本：`candidate-results/correctness-pilot.log`。Parent 对照版本构建输出：`candidate-results/parent-compile.log`。编译器的 `cce_global` host-side warning 保留在日志中，目标已成功链接。

## Local

Parent 与 Candidate 各运行同一个 runner，使用 `80x8192 FP32`、40 个可用核、45 次 warmup、21 对/块、2 块；每边 84 个 device-event 样本。Parent 来源为 R31B V011，Candidate 来源为本版。两者按 Parent 后 Candidate 顺序在独立进程运行，不能视为同进程逐样本因果配对；所有 raw samples 均保留在 `candidate-results/local-paired.log`。

| 指标 | Parent | Candidate |
|---|---:|---:|
| raw samples | 84 | 84 |
| median latency | 9.6200 us | 9.8100 us |
| MAD | 0.1700 us | 0.2300 us |
| MAD / median | 1.77% | 2.34% |
| p10–p90 | 9.3060–10.0340 us | 9.5000–45.1960 us |
| min–max | 9.2000–114.5600 us | 9.3800–125.0800 us |
| same-binary pair absolute-difference p90 | 1.3380 us | 53.0740 us |

```text
LOCAL_SCORE = 9.8100 us (Candidate median latency)
LOCAL_DELTA = +1.9751% (Candidate slower by 0.1900 us)
CURRENT_LOCAL_BEST = NONE
LOCAL_INTERPRETATION = noisy observation; Candidate pair spread and tail exceed the observed Parent/Candidate median difference, so no Local Best claim
```

两边采样期间 NPU 0 的 AICore/AIVector 使用率为 0%；VLLMEngineCor 进程驻留，使用约 53962 MB HBM，FREE_HBM 为 8519 MB。系统 load average：Parent 前 `26.47 / 39.75 / 42.78`、后 `25.47 / 39.32 / 42.63`；Candidate 前 `24.47 / 38.88 / 42.47`、后 `23.95 / 38.53 / 42.34`。每次执行前未检出 R15 路线进程。上述负载只作为采样背景。

## 状态

```text
STATUS = REAL_CANDIDATE_COMPLETE
COMPILE = PASS
CORRECTNESS = PASS
LOCAL_SCORE = 9.8100 us
LOCAL_DELTA = +1.9751%
LOCAL_BEST = NONE
OFFICIAL_SCORE = NONE
ONLINE = NOT_SUBMITTED
PUSH = NO
```
