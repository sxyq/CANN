# LEASE Q-REDUCE — 窗口资格判定（PRIORITY 5 REDUCE-INVSCALE-X）

独立数据集 `support/results-window-qual/`，**不与 L005 的 78.4% 数据集合并**。
Main-2 于 2026-09-24T07:38:58Z 下发。今日不再继续计时。

## 0. 时序测量前的 SHA256 核验

```text
PARENT    parent.asc     94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266  本地 MATCH / 远端 MATCH
CANDIDATE submission.asc bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26  本地 MATCH / 远端 MATCH
PARENT != CANDIDATE -> YES
可执行文件: parent 346d24cf… / candidate bf9eef7e…（不同）
git HEAD = ae46d7c，本路线无 commit / add / push
未改源码、无 V003、无 CANNJudge。
```

## 1. 判定线（Main 给定）

```text
序列   : PRECHECK-A x6  ->  gap 10s  ->  PRECHECK-B x6   （仅 PARENT 可执行文件）
合格   : CV <= 0.15   AND   max/min <= 1.30
设备序 : d6 先试，再 d4，最多 2 次
两台都不合格 -> NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED；不跑 candidate；今日停止计时；不与旧数据合并
```

`CV = 样本标准差 / 均值`。判定线在**合并的 12 个样本**上评估；A、B 两块的
分块指标同时记录，用于观察块间漂移。

形状：FP32 rows=1 D=6144。全部 24 次执行 `bad=0`、`max_abs=5.44672e-07`，
波动与计算结果无关，纯属计时环境。

## 2. 第一次尝试 — d6

租约快照（AICore 全程 0%）：

```text
preflight  2026-09-24T07:40:57Z  DEVICE_ID=6 AICORE=0% HBM=59877/65536 FREE_HBM=5659MB PROCS=none
gap_start  2026-09-24T07:41:38Z  DEVICE_ID=6 AICORE=0% HBM=59878/65536 FREE_HBM=5658MB PROCS=none
gap_end    2026-09-24T07:41:49Z  DEVICE_ID=6 AICORE=0% HBM=59876/65536 FREE_HBM=5660MB PROCS=none
postlease  2026-09-24T07:42:31Z  DEVICE_ID=6 AICORE=0% HBM=59878/65536 FREE_HBM=5658MB PROCS=none
next6 探针进程: none（起租中止条件未触发）
```

样本（µs）：

```text
A: 94.182 167.043 155.932 95.491 110.942 178.412
B: 118.432 193.942 146.342 139.022 128.442 193.973
```

| 视图 | n | mean | sd | median | min | max | CV | max/min | 判定 |
|---|---|---|---|---|---|---|---|---|---|
| A | 6 | 133.667 | 37.801 | 133.437 | 94.182 | 178.412 | 0.2828 | 1.8943 | UNQUALIFIED |
| B | 6 | 153.359 | 32.839 | 142.682 | 118.432 | 193.973 | 0.2141 | 1.6378 | UNQUALIFIED |
| **合并** | 12 | 143.513 | 35.291 | 142.682 | 94.182 | 193.973 | **0.2459** | **2.0596** | **UNQUALIFIED** |

两条件在 A、B、合并三个视图上**全部为 False**。块间漂移（B 中位 − A 中位）= +9.245 µs。

**d6 判定：UNQUALIFIED。**

## 3. 第二次尝试 — d4（最多 2 次，本次为最后一次）

租约快照（AICore 全程 0%）：

```text
preflight  2026-09-24T07:43:10Z  DEVICE_ID=4 AICORE=0% HBM=59187/65536 FREE_HBM=6349MB PROCS=none
gap_start  2026-09-24T07:43:52Z  DEVICE_ID=4 AICORE=0% HBM=59188/65536 FREE_HBM=6348MB PROCS=none
gap_end    2026-09-24T07:44:03Z  DEVICE_ID=4 AICORE=0% HBM=59186/65536 FREE_HBM=6350MB PROCS=none
postlease  2026-09-24T07:44:42Z  DEVICE_ID=4 AICORE=0% HBM=59187/65536 FREE_HBM=6349MB PROCS=none
next6 探针进程: none（起租中止条件未触发）
```

样本（µs）：

```text
A: 309.394 80.151 34.290 87.541 64.240 88.801
B: 70.591 69.481 147.222 140.702 80.271 118.922
```

| 视图 | n | mean | sd | median | min | max | CV | max/min | 判定 |
|---|---|---|---|---|---|---|---|---|---|
| A | 6 | 110.736 | 99.424 | 83.846 | 34.290 | 309.394 | 0.8978 | 9.0229 | UNQUALIFIED |
| B | 6 | 104.531 | 35.516 | 99.596 | 69.481 | 147.222 | 0.3398 | 2.1189 | UNQUALIFIED |
| **合并** | 12 | 107.634 | 71.254 | 83.906 | 34.290 | 309.394 | **0.6620** | **9.0229** | **UNQUALIFIED** |

A 块首样本 309.394 µs 与第三样本 34.290 µs 相差 9.0 倍，是本路线至今最差的
单块离散度。块间漂移 = +15.750 µs。

**d4 判定：UNQUALIFIED。**

## 4. 两次尝试用尽 -> 依 Main 序列停止

```text
d6 UNQUALIFIED   (CV 0.2459 > 0.15, max/min 2.0596 > 1.30)
d4 UNQUALIFIED   (CV 0.6620 > 0.15, max/min 9.0229 > 1.30)
attempts used = 2 / 2  ->  按序列停止
```

- **未获得合格窗口 -> 未执行任何 CANDIDATE（V002）计时。**
  `results-window-qual/` 下无任何 candidate 文件，调用次数 = 0。
- 未产生交错配对，故不作 ONLINE / REJECT 判定。
- **今日停止计时**，不再发起新的租约或预检。
- 本数据集独立，**未与 L005 的 78.4% 数据集合并**，三套旧数据
  （`results/`、`results_dev4/`、`results_L005/`、`results-round2/`）均未改动。

## 5. 设备已交还

```text
2026-09-24T07:45:44Z  next6 探针进程: none
                       REDUCE-INVSCALE-X 进程: none
                       远端 submission.asc sha256 = bef271b6...ad26（未变）
```

两台卡上均为顺序单进程执行，无常驻进程；远端 `*.o/*.d` 中间产物已删除。

## 6. Handoff

**NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED**

- `MEASUREMENT_BLOCKED`：Main 给定的更严判定线（CV ≤ 0.15 且 max/min ≤ 1.30）
  在允许的两次尝试内均未达成。d6 已是两台中较好的一台，CV 仍为 0.2459、
  max/min 仍为 2.0596，两条都差一截；d4 更差。
- `NEEDS_ONE_MORE_LOCAL`：被卡住的是窗口资格，不是候选。V002 correctness
  在归档的 16 形状矩阵上仍为全 PASS，源码保持 `bef271b6…ad26` 不变。
- 无 ONLINE_CANDIDATE、无 LOCAL_REJECTED：本轮零 CANDIDATE 样本。
- 无 V003、无 CANNJudge、未改源码；今日停止计时，转 IDLE。

**本轮新增的事实**

1. 判定线从 `CV<=0.25 / range-med<=0.5` 收紧到 `CV<=0.15 / max/min<=1.30` 后，
   即便在 AICore 全程 0%、FREE_HBM 5.6–6.3GB、无其他 next6 探针的窗口里
   仍拿不到合格样本；d6 是三轮尝试（ROUND-2 d4/5/6 + 本轮 d6/d4）里最好的，
   仍差约 0.10 的 CV 和 0.76 的 max/min。
2. d4 本轮出现 309.394 µs 与 34.290 µs 同块并存（9.0 倍），
   说明该卡上存在偶发的长尾/短尾事件，与 AICore 占用读数无关。
3. A/B 两块之间存在 +9.2 µs（d6）与 +15.8 µs（d4）的中位漂移，
   即使 10 秒间隔也未抹平，窗口在分钟尺度上并不平稳。

**下一步建议（今日不执行）**：继续换卡已无空间（d4/d5/d6 全试过）。
需要改 harness 或改主机状态才可能拿到合格窗口：丢弃式预热后再取样；
把被测块压缩到更短时间以避开分钟级漂移；或在租约窗口内排空残余
`VLLMEngineCor`。在其中任一项落地前，重复本判定序列大概率仍会 UNQUALIFIED。

## 7. 文件

```text
phase4/local/REDUCE-INVSCALE-X/V002/support/
  WINDOW-QUAL-REPORT.md          本文
  run_windowqual_q.sh            资格判定 harness
  results-window-qual/
    d6/  qual.tsv lease.tsv preflight/gap_start/gap_end/postlease.npu-smi.txt A/run-{1..6}.* B/run-{1..6}.*
    d4/  qual.tsv lease.tsv preflight/gap_start/gap_end/postlease.npu-smi.txt A/run-{1..6}.* B/run-{1..6}.*
    WINDOW-QUAL-METRICS.json     机器可读判定结果
```

旧数据集 `results/`、`results_dev4/`、`results_L005/`、`results-round2/` 全部未改动。
