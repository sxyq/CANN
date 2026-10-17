# W4-R04 / V002

本目录把同一版本的源码入口、说明和成绩放在一起；原始材料仍保存在比赛归档。

## 源码

- [源码-2d2e7da242ab.asc](./源码-2d2e7da242ab.asc)

## 成绩摘要

| 记录行 | Build | Correctness | Local Parent → Candidate | Local Delta | Local Verdict | Server Metric | Official | Submission |
|---:|---|---|---|---|---|---|---|---|
| 10 | `PASS_EXISTING_V002_BUILD;NO_REBUILD` | `PASS_3_FP32_CPU_FP64_REFERENCE;P_C_BITWISE_EQUAL` | `OBSERVATION_ONLY_P_P_MEDIANS_US=13.32/14.34;13.38/15.91;9.75/8.52` → `NOT_RUN_P_C` | `NONE` | `MEASUREMENT_BLOCKED` | `NONE` | `NONE` | `NONE` |
| 11 | `PASS` | `PASS_9_FP32_INPUTS_CPU_FP64;P_C_BITWISE_EQUAL` | `OBSERVATION_ONLY:16x2048 FP32 guard-ON=15.47us;16x2056 FP32 guard-ON=18.52us;12x8192 FP32 guard-OFF GenericRow=16.19us` → `OBSERVATION_ONLY:16x2048 FP32 guard-ON=17.07us;16x2056 FP32 guard-ON=17.05us;12x8192 FP32 guard-OFF GenericRow=17.96us` | `OBSERVATION_ONLY:TWO_AFFECTED_INPUTS_SUMMARY=+0.789037863747%;16x2048 ratio=+10.342599%,paired=+1.322792%;16x2056 ratio=-7.937367%,paired=-10.752162%;12x8192 guard-OFF ratio=+10.932674%,paired=-1.205542%` | `MEASUREMENT_BLOCKED` | `NONE` | `41.94` | `6ac8bdf4694b590c3c9747c6` |

原始逐版本记录：[全版本记录.tsv](../../../../全版本记录.tsv)。
## 版本说明

上表保留已记录的构建、正确性、本地、设备与 Official 结果；无结果的字段按原始记录显示。
