# ONLINE_RECOMMENDATION — R31A V028（R31A 最终主推）

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: R31A
- Revision: V028
- Direct Parent: V026
- Source SHA: `ee52831c435f07f51ea3917af0e7faed9a012f852ab19641882c401fb78d10f1`

## Recommendation: WORTHY（R31A 最终主推，取代 V026）

## Trigger

Campaign trigger **B** met 且持续：V024→V025→V026→V028 连续 LOCAL_ACCEPTED（V027 为 NEEDS_ONE_MORE，不打断 ACCEPTED 链的实质）。

`ONLINE_DECISION = SELECTED_ON_TRIGGER`。

## 为什么 V028 取代 V026

| 项 | V026 | **V028** |
|---|---|---|
| CachedRows D32768 | −8.3% | −8.3%（继承） |
| batch D24576 | 无 | **−1.09%**（新增） |
| 覆盖 | 单域 | **双域** |
| 父版 | V025 | V026（严格超集） |

V028 是本地增益的严格超集，单一候选覆盖两类形状域。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | PASS ×5（max_abs 逐位一致） |
| SINGLE_CHANGE_AUDIT | PASS（仅删 2 个 PipeBarrier） |
| LOCAL_VERDICT | LOCAL_ACCEPTED（8/8 双设备） |
| ARCHITECTURE_FINDING | SetFlag 前 barrier 非承重（跨路线可复用） |

## Online 优先级（最终）

| # | Candidate | 信号 |
|---|---|---|
| **1** | **R31B V017** | Champion 血统，bf16 −14.3% + fp16 −6.7~8.5% |
| **2** | **R31A V028**（本推荐） | 双域：CachedRows −8.3% + batch −1.09% |
| 3 | EPILOGUE-ARITH V001 | NORM-HOIST −2.55% |
| 4 | 其他 | 分离贡献用 |

## Online package

- Source: `线上结果/R31A/V028/submission.asc`
- SHA256: `ee52831c435f07f51ea3917af0e7faed9a012f852ab19641882c401fb78d10f1`
