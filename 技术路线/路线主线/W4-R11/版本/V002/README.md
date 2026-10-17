# W4-R11 / V002

本目录把同一版本的源码入口、说明和成绩放在一起；原始材料仍保存在比赛归档。

## 源码

- [源码-21835f22db43.asc](./源码-21835f22db43.asc)
- [源码-a6e3ff4c3e5b.cpp](./源码-a6e3ff4c3e5b.cpp)

## 成绩摘要

| 记录行 | Build | Correctness | Local Parent → Candidate | Local Delta | Local Verdict | Server Metric | Official | Submission |
|---:|---|---|---|---|---|---|---|---|
| 28 | `PASS_REUSED` | `PASS_LOCAL_REFERENCE_BOTH_SHAPES` | `OBSERVATION_ONLY:64x8192 FP32 event=18.16us;12x8192 FP32 event=14.92us` → `OBSERVATION_ONLY:64x8192 FP32 event=15.69us;12x8192 FP32 event=15.25us` | `OBSERVATION_ONLY:target paired_median=-9.690739%,ratio_of_medians=-13.601322%;control paired_median=-9.246666%,ratio_of_medians=+2.211796%` | `MEASUREMENT_BLOCKED` | `NONE` | `42.32` | `6ac8cf21694b590c3ca24ba5` |
| 44 | `PASS_HOST_ONLY;KERNEL_LIBRARIES_REUSED` | `PASS_PARENT_BOTH_OUTPUT_ADDRESSES_TWO_SHAPES_BEFORE_AND_AFTER` | `PARENT_ONLY_TASK_MAD_RATIO target=0.33391304; control=0.13587786` → `NONE` | `NONE` | `RESEARCH_ONLY;MEASUREMENT_BLOCKED` | `NONE` | `NONE` | `NONE` |

原始逐版本记录：[全版本记录.tsv](../../../../全版本记录.tsv)。
## 版本说明

上表保留已记录的构建、正确性、本地、设备与 Official 结果；无结果的字段按原始记录显示。
