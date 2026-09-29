# MAIN-2 Online Gap Audit

日期：2026-09-29
状态：wave-1 整改闭环用，无新 Online 提交

## 结果表

| Route | Rev | Local verdict | 分类 | Online 动作 | REASON |
|---|---|---|---|---|---|
| ASYNC-OVERLAP-CHAMPION-X | V001 | LOCAL_ACCEPTED | 已闭环 | **ONLINE_CLOSED** | Official 44.17 Pass 15/15 REJECT；五处一致 |
| ASYNC-OVERLAP-CHAMPION-X | V002–V004 | NEEDS_ONE_MORE_LOCAL | C/D | **ONLINE_NOT_REQUIRED** | 无 LOCAL_ACCEPTED；NarrowMid/FullCache 轴混杂或证伪 |
| REDUCE-HIER-X | V001–V005 | mixed / REJECTED | **D** | **ONLINE_NOT_REQUIRED** | 五变体全伪；无 LOCAL_ACCEPTED |
| VECTOR-MATH-X | V001–V003 + S3 | V003 mixed；轴关闭 | **D** | **ONLINE_NOT_REQUIRED** | SEQ-FUSE-2 被 S3 probe 证伪；历史 V001 已有 Official 44.22 |
| COEFF-LOCALITY-X | V001–V004 | REJECTED / N1M | **D** | **ONLINE_NOT_REQUIRED** | 时序/驻留/预加载证伪或噪声；历史 V001 已有 Official 44.16 |
| MULTIROW-DMA-CHAMPION-X | V001–V002 | REJECTED / N1M terminal | **D + C** | **ONLINE_NOT_REQUIRED** | 无收益；V002 信号在对照噪声带内 |
| DTYPE-SPECIAL-X | V001 | NEEDS_ONE_MORE_LOCAL | evidence | **ONLINE_NOT_REQUIRED** | 已有 Official 44.04；非本轮 Active |
| SCHED-CHAMPION-X | V002 | LOCAL_ACCEPTED | evidence | **ONLINE_NOT_REQUIRED** | 已有 Official 41.94；非本轮 Active |
| UB-LIVENESS-X | V003 | correctness PASS | evidence | **ONLINE_NOT_REQUIRED** | 已 Online WA 8/15；禁止建 V004 |

## 汇总

- ONLINE_REQUIRED = **0**
- ONLINE_CLOSED = **1**（ASYNC V001）
- ONLINE_NOT_REQUIRED = **全部其余**
- UNKNOWN = **0**
- CALIBRATION_SUBMISSION_RECOMMENDATION = **NONE**
