# ONLINE_RECOMMENDATION — R31A V024

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31A
- Revision: V024
- Direct Parent: V016 (Official 45.00)
- Source SHA: `49315432d02b238b81eb7190df6463aa2dc235d077f28b91284aab69f17fbfde`

## Recommendation: WORTHY

## Trigger

Campaign conditional approval trigger **A** is met:

| 条件 | 状态 | 证据 |
|---|---|---|
| 明显超过 noise floor | YES（3.5×） | −3.66% vs ±1% 同码噪声带 |
| 多 pair | YES | 8/8 clean blocks, MAD≤0.027 |
| 相关 shape 方向一致 | YES | d6 4/4 + d4 4/4 全负，两台独立设备 |

因此按 Campaign 规则：`ONLINE_DECISION = APPROVED_ON_TRIGGER`（无需单独等待 Planning 对该 Candidate 批准）。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS（max_abs 与 V016 逐位一致，位等价算术） |
| EXECUTABLE_IDENTITY | PASS（local SHA == remote SHA） |
| SINGLE_CHANGE_AUDIT | PASS（仅 pass-2 Muls 位置移动一处） |
| LOCAL_VERDICT | LOCAL_ACCEPTED |
| Rsqrt 红线 | 未使用 Rsqrt/Reciprocal |
| 去重 | 不与 STORE / EPILOGUE-ARITH / VECTOR-MATH / V020-V023 重叠 |

## Caveats for Judge / Planning

1. 收益形状覆盖：生效形状为 rows=2 D=32768 CachedRows 路径。Official 总分是多形状加权，单形状 −3.66% 映射到 overall 的幅度不确定。
2. 历史校准：R31A V017 曾 FALSE_POSITIVE（本地 −4.33% / Official −0.55）。本次证据质量远强于 V017（8/8 双设备 vs 单形状无噪声底），但不保证 Official 提升。
3. 宽 FP32 invRms 非确定（~0.8%）是共享基线问题，不影响本改动的位等价性（max_abs 与父一致）。

## Online package

- Source: `本地实验/R31A/V024/submission.asc`
- SHA256: `49315432d02b238b81eb7190df6463aa2dc235d077f28b91284aab69f17fbfde`
- 路径副本：`线上结果/R31A/V024/submission.asc`（exact source copy，不进 Main 主工作树）

## 下一步

交统一 Judge Owner 串行提交：

```bash
npm run cannjudge:submit -- --yes --source <exact-file>
```

提交前记录 SOURCE_PATH / 行数 / 字节数 / LOCAL_SHA；提交后核对 LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA。
