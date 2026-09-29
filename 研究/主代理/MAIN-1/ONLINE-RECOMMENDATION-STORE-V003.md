# ONLINE_RECOMMENDATION — STORE-EPILOGUE-X V003

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: STORE-EPILOGUE-X
- Revision: V003
- Direct Parent: V002
- Source SHA: `0cdef265459d4683a1813593a881a5cf25ae75246aab49279d121896b71184ca`

## Recommendation: WORTHY

## Trigger

| 形状 | 信号 | 方向 | same-binary |
|---|---|---|---|
| 8x16384 | **−5.64%** | 6/0 | PASS |
| 1x32768 | −3.15% | 6/0 | PASS |
| 1x16384 | −2.90% | 5/0 | PASS |

同码噪声带 ±3.5%；merge-ON 形状以 6/6、5/5 一致性破出。机制幅度核对通过（收益随 batch 数增长）。

`ONLINE_DECISION = APPROVED_ON_TRIGGER`。

## Main Review checklist

| 项 | 状态 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | 24 形状逐字节一致；5 wide-FP32 COMMON_MODE_WITH_PARENT |
| SINGLE_CHANGE_AUDIT | PASS（H1 分块合并写回 K=2） |
| LOCAL_VERDICT | LOCAL_ACCEPTED |
| same-binary | PASS 10/10（最干净窗口） |

## 累计收益

1x32768 vs FROZEN_R31B_V011 约 **−8.6%**（V002 −5.62% + V003 −3.15%）。

## Online 优先级（最终）

| # | Candidate | 信号 | 强项 |
|---|---|---|---|
| **1** | **R31B V017** | bf16 −14.3% + fp16 −6.7~8.5% | Champion 血统 |
| **2** | **R31A V028** | 双域 −8.3% / −1.09% | 形状覆盖广 |
| **3** | **EPILOGUE-ARITH V002** | 跨窗 −3.1~−6.6% | 算术链 |
| **4** | **STORE-EPILOGUE-X V003**（本推荐） | 多行 wide −5.64% | **case 14 类主战场** |

## Online package

- Source: `线上结果/STORE-EPILOGUE-X/V003/submission.asc`
- SHA256: `0cdef265459d4683a1813593a881a5cf25ae75246aab49279d121896b71184ca`
