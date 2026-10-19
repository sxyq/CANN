# ONLINE_RECOMMENDATION — R31B V017（Champion 血统主推）

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31B（Champion lineage）
- Revision: V017
- Direct Parent: V016
- Source SHA: `7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4`

## Recommendation: WORTHY（Champion 血统主推）

## Trigger

Campaign trigger **B** met：V016 + V017 连续 2 次 LOCAL_ACCEPTED。

| Revision | 机制 | 信号 |
|---|---|---|
| V016 | FP16/BF16 wide tile 4096→8192 | fp16-wide −6.5~−7.0% |
| **V017** | pass-2 store drain defer | **bf16-wide −14.3% (20/20)**；fp16-wide −6.7~−8.5% |

`ONLINE_DECISION = SELECTED_ON_TRIGGER`。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS 12/12 |
| SAME_BINARY | PASS 6/6 |
| SINGLE_CHANGE_AUDIT | PASS（Store 后全排空 → MTE3_V 事件延迟等待） |
| LOCAL_VERDICT | LOCAL_ACCEPTED |
| 机制幅度核对 | PASS（delta ≈ (tileCount−1)×0.12–0.15µs） |

## Online 优先级（更新）

| # | Candidate | 信号 |
|---|---|---|
| **1** | **R31B V017**（本推荐，含 V016） | bf16 −14.3% + fp16 −6.7~8.5% + V016 fp16 −6.5~7% |
| 2 | R31A V026 | chain −8.3% |
| 3 | EPILOGUE-ARITH V001 | −2.55% |
| 4 | R31B V016（若需分离） | fp16 −6.5~7% |

## Online package

- Source: `线上结果/R31B/V017/submission.asc`
- SHA256: `7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4`

## 含义

Champion V011 (45.16) 之上，V016+V017 累计本地收益显著（fp16+bf16 wide）。这是本战役最高价值 Online 候选。
