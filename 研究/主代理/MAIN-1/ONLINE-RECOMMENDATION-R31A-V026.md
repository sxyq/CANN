# ONLINE_RECOMMENDATION — R31A V026（主推）

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31A
- Revision: V026
- Direct Parent: V025
- Source SHA: `7f2029e478d7675e02c2e017b0db3eaf33ae5dc0fa478479b340f441ab46bb18`

## Recommendation: WORTHY（R31A 主推候选）

## Trigger

Campaign trigger **B** fully met：同一 Local Best chain 连续 **3 次** LOCAL_ACCEPTED。

| Revision | 机制 | 增量 | 累计 vs V016 |
|---|---|---|---|
| V024 | invRms Muls 提出 tile 循环 | −3.66% | −3.66% |
| V025 | pass-1 staging liveness | −4.12% | ≈ −6.8% |
| **V026** | pass-2 param staging liveness | **−1.69%** | **≈ −8.3%** |

`ONLINE_DECISION = APPROVED_ON_TRIGGER`。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS ×4（max_abs 与父一致，无 507035） |
| EXECUTABLE_IDENTITY | PASS |
| SINGLE_CHANGE_AUDIT | PASS（仅 pass-2 参数 staging 一处） |
| LOCAL_VERDICT | LOCAL_ACCEPTED（V025 vs V026，7/7 双设备） |
| parent module | = DIRECT_PARENT V025（方法论合规） |

## Online 优先级

**V026 是 R31A 最强候选**（包含全部三个机制的累计收益）。推荐：

1. **R31B V016**（Champion 血统，fp16-wide −6.5~−7%）
2. **R31A V026**（chain −8.3%）← 本推荐
3. R31A V025 / V024（若需分离贡献）

## Caveats

1. 收益集中在 D=32768 CachedRows；Official 多形状加权后幅度不确定。
2. V026 增量 −1.69% 约 2× 噪声带（±0.8%），比 V024/V025 的信号弱，但方向 7/7 一致。
3. 宽 FP32 invRms 非确定为共享基线问题。

## Online package

- Source: `线上结果/R31A/V026/submission.asc`
- SHA256: `7f2029e478d7675e02c2e017b0db3eaf33ae5dc0fa478479b340f441ab46bb18`
