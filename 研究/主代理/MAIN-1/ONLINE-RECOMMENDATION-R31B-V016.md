# ONLINE_RECOMMENDATION — R31B V016

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31B（Champion lineage）
- Revision: V016
- Direct Parent: V011（Overall Champion, Official 45.16）
- Source SHA: `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`

## Recommendation: WORTHY

## Trigger

Campaign conditional approval trigger **A** met:

| 条件 | 状态 | 证据 |
|---|---|---|
| 明显超过 noise floor | YES | fp16-wide-d32768 −6.5~−7.0%，双 attempt |
| 多 pair | YES | 15/16 与 16/18 对为负 |
| 相关 shape 方向一致 | YES | 变更域 fp16 宽行一致；其余形状持平无回退 |

`ONLINE_DECISION = APPROVED_ON_TRIGGER`。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS（FP32 D16384 父/子共性失败已排除出计时候选，非候选回退） |
| EXECUTABLE_IDENTITY | PASS |
| SINGLE_CHANGE_AUDIT | PASS（FP16/BF16 宽行 seed tile 4096→8192，单常量） |
| LOCAL_VERDICT | LOCAL_ACCEPTED |
| 去重 | 不与 STORE / TILING / R31A / EPILOGUE-ARITH 重叠 |

## Caveats

1. 配对布局为 21 对 × 2 attempt（非 4-block×11）；协议核心（same-binary、交错、device event、噪声底）齐备。
2. bf16-wide 收益未建立（平）。收益主要在 fp16-wide-d32768。
3. 相对 V011（45.16）的 Official 映射不确定；但这是 Champion 血统上首个本地可复现收益。
4. 宽 FP32 invRms 非确定性为共享基线问题，不影响 fp16 变更域。

## Online package

- Source: `本地实验/R31B/V016/R31B-V016-WIDE-TILE-SEED_kernel.asc`
- SHA256: `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`
- 证据副本：`线上结果/R31B/V016/`

## 含义

若 Official 确认，将在 Champion 45.16 之上再进一步，是本战役最高价值的 Online 候选之一。
