# R31B / V016

本目录把同一版本的源码入口、说明和成绩放在一起；原始材料仍保存在比赛归档。

## 源码

- [源码-9f5c353e65a1.asc](./源码-9f5c353e65a1.asc)

## 成绩摘要

| 记录行 | Build | Correctness | Local Parent → Candidate | Local Delta | Local Verdict | Server Metric | Official | Submission |
|---:|---|---|---|---|---|---|---|---|
| 81 | `Candidate clean build/link PASS; paired runner repaired after failed attempt` | `PASS on change-domain shapes; common FP32 D16384 failure on Parent and Candidate` | `FP16 D12288=69.290us;D32768=235.411us;BF16 D12288=122.900us;D32768=151.020us` → `FP16 D12288=226.191us;D32768=207.590us;BF16 D12288=162.820us;D32768=206.721us` | `four pair deltas +156.901,-27.821,+39.920,+55.701 us; LEGACY_LOAD_CONTAMINATED` | `MEASUREMENT_BLOCKED` | `NONE` | `NONE` | `NONE` |

原始逐版本记录：[全版本记录.tsv](../../../../全版本记录.tsv)。
## 版本说明

上表保留已记录的构建、正确性、本地、设备与 Official 结果；无结果的字段按原始记录显示。
