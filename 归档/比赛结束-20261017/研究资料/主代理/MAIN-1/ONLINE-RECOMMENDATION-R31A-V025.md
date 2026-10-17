# ONLINE_RECOMMENDATION — R31A V025

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31A
- Revision: V025
- Direct Parent: V024（LOCAL_BEST）
- Source SHA: `1475116735f390b426fc694d45fcdcbc676ed0758ef326d54d8c43d35e16b4fe`

## Recommendation: WORTHY

## Trigger

Campaign conditional decision trigger **B** met：

| 条件 | 状态 | 证据 |
|---|---|---|
| 同一 Local Best chain 连续 2–3 次 LOCAL_ACCEPTED | YES | V024（−3.66%）→ V025（−4.12% 增量），合计 vs V016 约 −6.8% |

`ONLINE_DECISION = SELECTED_ON_TRIGGER`。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS ×4（max_abs 与父一致，无 507035） |
| EXECUTABLE_IDENTITY | PASS |
| SINGLE_CHANGE_AUDIT | PASS（仅 pass-1 staging liveness 一处） |
| LOCAL_VERDICT | LOCAL_ACCEPTED（V024 vs V025，4/4 双设备） |
| Rsqrt 红线 | 未使用 |
| parent module | 已重建为 DIRECT_PARENT V024（方法论修正） |

## 与 V024 的关系

- V025 包含 V024 的机制（invRms Muls 提取）**加上** H4（pass-1 staging liveness）。
- 若只能提交一个：**优先 V025**（合计 −6.8% vs V016）。
- 若串行提交两个：V024 先（分离 H2 贡献），V025 后（叠加 H4）。由 Judge Owner 决定顺序。

## Caveats

1. 收益集中在 D=32768 CachedRows 路径；Official 多形状加权后幅度不确定。
2. 同码噪声带含候选槽位偏差 +1.5~2%；信号为噪声带约 2 倍。
3. 宽 FP32 invRms 非确定为共享基线问题。

## Online package

- Source: `线上结果/R31A/V025/submission.asc`
- SHA256: `1475116735f390b426fc694d45fcdcbc676ed0758ef326d54d8c43d35e16b4fe`
