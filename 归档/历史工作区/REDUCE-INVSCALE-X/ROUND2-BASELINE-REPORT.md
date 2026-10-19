# ROUND-2 PARENT reference measurement — REDUCE-INVSCALE-X

独立数据集，不与 L005 合并。Main-2 ROUND-2 于 2026-09-24T05:26:43Z 下发，
自动租约 `R2-REDUCE` 起于 d4（全新窗口，无其他 cann 探针）。候选源码未改动。

## 0. 时序测量前的 SHA256 核验

```text
PARENT 源码   (FULL-R006-V001)  parent.asc
  本地 workspace  94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266  MATCH
  server3 远端    94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266  MATCH

CANDIDATE 源码 (V002)           submission.asc
  本地 workspace  bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26  MATCH
  server3 远端    bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26  MATCH

PARENT != CANDIDATE  ->  YES   (94ab0ef9... != bef271b6...)

可执行文件（server3，二者不同，符合预期）
  reduce_invscale_parent_probe    346d24cf60ab8fe892650a226d65afd678ed2c2fa3005e8a663c39f329eb5047
  reduce_invscale_candidate_probe bf9eef7e18b9a5384a57ec2d1611bed359fb8a837e285b890b310c5ae7e66742

git HEAD = ae46d7c，本路线无 commit / add / push
未改源码、无 V003、无 CANNJudge。
```

## 1. d4 起租前状态（AUTO LEASE R2-REDUCE）

```text
2026-09-24T05:27:13Z  DEVICE_ID=4  AICORE=0%  HBM=59188/65536  FREE_HBM=6348MB
d4 上的进程        : PID 2999855 VLLMEngineCor 55664MB（残余，按既定口径接受）
next6 探针进程     : none
本路线进程         : none
中止条件           : 未触发
```

每台卡的起租/收租快照分别存放在各自 `baseline-d{4,5,6}/{preflight,postlease}.npu-smi.txt`。

## 2. PARENT 参考测量（仅 PARENT，不含 CANDIDATE）

Main 指令原文为 "PARENT BASELINE ×8-10"，本文按该指令执行并记录。

形状：FP32 rows=1 D=6144（两变体在该形状 correctness 均 PASS，见归档的 16 形状矩阵）。
每台卡上单进程顺序执行，每次执行内部 warmups 3 / repeats 11 并回报一个中位数。
全部 26 次执行 `bad=0`、`max_abs=5.44672e-07`，故方差纯粹来自计时，与功能无关。

**稳定性判定（Main 给定）：CV > 0.25 或 range/median > 0.5 即判 UNSTABLE**
（`CV = 样本标准差 / 均值`）。

负载分级沿用本路线已声明的阈值（按 PARENT 抖动）：
`CLEAN ≤10%`、`MODERATE 10–30%`、`LOAD_CONTAMINATED >30%`（相对中位数）。

| device | n | 各次耗时 (µs) | mean | sd | median | min | max | CV | range | range/med | 判定 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| **d4** | 10 | 228.539 112.300 135.879 190.290 206.619 63.130 164.140 85.650 236.299 159.870 | 158.272 | 59.035 | 162.005 | 63.130 | 236.299 | **0.3730** | 173.169 | **1.0689** | **UNSTABLE** |
| **d5** | 8 | 80.623 86.585 231.788 138.462 136.342 172.748 147.554 107.017 | 137.640 | 49.269 | 137.402 | 80.623 | 231.788 | **0.3580** | 151.165 | **1.1002** | **UNSTABLE** |
| **d6** | 8 | 161.943 144.000 179.975 144.320 88.542 117.876 119.896 81.790 | 129.793 | 34.248 | 131.948 | 81.790 | 179.975 | **0.2639** | 98.185 | **0.7441** | **UNSTABLE** |

逐台明细：

```text
d4: CV 0.3730 > 0.25 TRUE ; range/med 1.0689 > 0.5 TRUE  -> UNSTABLE  LOAD=LOAD_CONTAMINATED
d5: CV 0.3580 > 0.25 TRUE ; range/med 1.1002 > 0.5 TRUE  -> UNSTABLE  LOAD=LOAD_CONTAMINATED
d6: CV 0.2639 > 0.25 TRUE ; range/med 0.7441 > 0.5 TRUE  -> UNSTABLE  LOAD=LOAD_CONTAMINATED
```

**d4 / d5 / d6 三台全部 UNSTABLE**，无一达到 CLEAN 或 MODERATE。
d6 是三者中最好的，但两条判定线仍未过。

## 3. 依 Main 序列得出的后果

Main 序列第 3 条：*"All UNSTABLE → NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED;
no candidate."* 该条生效。

- **ROUND-2 未执行任何 CANDIDATE（V002）计时。**
  `reduce_invscale_candidate_probe` 调用次数 = 0。
- 无交错配对，故本轮不产生 candidate 与抖动的比较，也不据此作 ONLINE / REJECT 判定。
- 本数据集**独立**保存，**不与 L005 数据集（`results_L005/`）求平均或合并**；
  两者并列留存。

## 4. 设备已交还

```text
2026-09-24T05:34:39Z  next6 探针进程: none
                       REDUCE-INVSCALE-X 进程: none
                       远端 submission.asc sha256 = bef271b6...ad26（未变）
```

参考测量为顺序单进程执行，d4/d5/d6 上无常驻进程。远端 `*.o/*.d` 中间产物已删除。

## 5. Handoff

**NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED**

- `MEASUREMENT_BLOCKED`：在 Main 自定的稳定性判定下，d4/d5/d6 这组设备无法承载
  可采信的 PARENT-vs-CANDIDATE 比较。PARENT 单边抖动为 CV 26–37%、
  range/median 74–110%，而机器当时 AICore 空闲、FREE_HBM 5.6–6.3GB、
  仅余残余 VLLM。
- `NEEDS_ONE_MORE_LOCAL`：被卡住的是**测量**，不是候选。V002 的 correctness
  在归档的 16 形状矩阵上仍为全 PASS；V002 保持 `bef271b6…ad26` 不变。
- 无 ONLINE_CANDIDATE、无 LOCAL_REJECTED：本轮未采集任何 CANDIDATE 数据。
- 无 V003、无 CANNJudge、未改源码。

**本轮新增的事实**

1. 不稳定与选卡无关：d4、d5、d6 三张卡都撞同一条判定线，"换一张更安静的卡"
   在当前主机上不是一个可用解法。
2. 判定本身的代价：26 次 PARENT 执行（10+8+8），correctness 全干净，
   说明波动完全来自计时环境。
3. ROUND-2 **独立地**复现了 L005 的结论（PARENT 抖动远超可用区间），
   两套数据互不掺混、结论一致。

**下一步建议（本轮未执行）**：约束变量在 harness 或主机，不在选卡。
可供建议 Main 的方向：每个被测块之前对两个二进制做丢弃式预热；
把测量钉到带 CPU 亲和性的独立进程；或在租约窗口内暂停/排空残余的
`VLLMEngineCor`（55.6 GB）。上述任一项未改动之前，继续扫设备仍会撞同一条线。

## 6. 文件

```text
phase4/local/REDUCE-INVSCALE-X/V002/support/
  ROUND2-BASELINE-REPORT.md      本文
  run_baseline_r2.sh             参考测量 harness
  results-round2/
    baseline-d4/  baseline.tsv lease.tsv preflight/postlease.npu-smi.txt run-{1..10}.*
    baseline-d5/  baseline.tsv lease.tsv preflight/postlease.npu-smi.txt run-{1..8}.*
    baseline-d6/  baseline.tsv lease.tsv preflight/postlease.npu-smi.txt run-{1..8}.*
    ROUND2-METRICS.json          机器可读的判定结果
```

`results/`（L005 device-6）、`results_dev4/`（修复代价复测）、
`results_L005/`（L005 六对）均未改动。
