# W4-R09 V001 测量资格复测

## 结果

```text
RESULT_CLASS=MEASUREMENT_BLOCKED
ROUTE=W4-R09
REVISION=V001
DEVICE_ID=7
SHAPE=16x16384
DTYPE=fp16
BLOCKS=8
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=UNKNOWN
NEW_PERFORMANCE_REVISION=0
PUSH=NO
```

只复用已提交的 V001 Candidate 与 runner，没有改写 Candidate，也没有新增 V002。Parent 为 `R31B-V011`。Parent 与 Candidate 均取自 V001 性能提交 `96044629c9783ab540daed927be37697424365dd`：`Parent.asc`、`Candidate.asc`。两份源码以该提交和路径定位；本次没有计算源码 SHA256。

V001 Compile 最终为 PASS（第二次尝试，GCC 11 include/library 路径直接处理首次 host `vector` 头文件缺失）。已有本地代理精度结果为 8/8 PASS。四次重测中 runner 自带的 Parent/Candidate 对比与 FP64 CPU reference 复核也全部 PASS；这些结果仍是本地代理覆盖。

## 两轮结果

两轮均为同一进程内 Parent/Candidate 交错；Parent/Parent 使用同一 Parent kernel 指针，分别写入两个输出缓冲。第一轮参数为 `warmups=45, repeats=31`；第二轮为 `warmups=60, repeats=47`。msprof task-based 仅用于保留 kernel task 轨迹，计时结论以 runner 的 device-event raw 为依据。

| 轮次 / 方式 | Parent 中位数 us | Candidate / 第二侧中位数 us | 配对差中位数 us | PC 差中位数 us | CP 差中位数 us | P/C MAD us | P/C p05–p95 spread us |
|---|---:|---:|---:|---:|---:|---:|---:|
| R1 Parent/Parent，31 对 | 21.32 | 22.60 | +0.40 | +0.90 | -0.82 | 6.70 / 6.42 | 52.44 / 41.86 |
| R1 Parent/Candidate，31 对 | 21.68 | 22.36 | +0.30 | +0.49 | -0.12 | 8.40 / 7.10 | 62.84 / 54.74 |
| R2 Parent/Parent，47 对 | 46.54 | 38.10 | -13.04 | -8.70 | -23.34 | 22.36 / 14.74 | 147.06 / 91.38 |
| R2 Parent/Candidate，47 对 | 24.22 | 24.16 | -0.16 | +0.74 | -0.42 | 3.46 / 4.28 | 17.78 / 23.72 |

R1 的 P/C 百分比差为 +2.3184%（更慢）；R2 为 -0.7992%（更快）。两轮方向不同，且两轮的 PC/CP 子组方向都反转。Parent/Parent 本身也出现大幅波动，R2 的同一 Parent 两侧中位数相差 8.44 us，配对差中位数为 -13.04 us，Parent 单样本最大值达到 230.16 us。结果不足以区分 Candidate 效应与次序/负载变化，故不形成 Local score，也不提升 Local Best。

## 设备与采集

设备 7 每个测量阶段开始前的 FREE_HBM 均为 35389 MB（准入线 100 MB）。运行期间观测到其他进程；AICore/AIVector 活动随阶段变化，范围分别为 7–28% 和 0–83%。这些值只作为负载记录。阶段快照和采集输出完整保留在本目录。

Runner 原始 event / wall 样本：

- `r1-parent-same.tsv`：31 对 Parent/Parent
- `r1-parent-candidate.tsv`：31 对 Parent/Candidate
- `r2-parent-same.tsv`：47 对 Parent/Parent
- `r2-parent-candidate.tsv`：47 对 Parent/Candidate

四个 `*-prof/PROF_*/` 目录均保留原始 device/host 数据、SQLite 文件及导出文件。每份 `task_time_*.csv` 含 156 或 218 行 `AI_VECTOR_CORE` kernel task，并带 device、kernel、stream、task ID、task 时间和起止时间；另有 EVENT_RECORD 行。两个入口在 task CSV 中使用同一设备符号，Parent/Candidate 对应关系由相同进程中的 runner 顺序字段确定。

## 设备快照路径

- `r1-parent-same-load-before.txt`
- `r1-parent-candidate-load-before.txt`
- `r2-parent-same-load-before.txt`
- `r2-parent-candidate-load-before.txt`
- `r2-parent-candidate-load-after.txt`

## 下一步

此 shape 在本轮最多两轮内未取得测量资格，保持 `MEASUREMENT_BLOCKED / LOCAL_SCORE=NONE` 并停止本 shape 的本轮测量。保留 V001 Candidate、原提交和全部失败/波动数据；不创建性能版本、不提交 Official、不 push。后续若重新安排，应先由 Main/Planning 提供新的独立测量条件或测量方法依据。
