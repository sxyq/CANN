# ONLINE_RECOMMENDATION — EPILOGUE-ARITH-CHAMPION-X V002（主推，取代 V001）

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: EPILOGUE-ARITH-CHAMPION-X
- Revision: V002
- Direct Parent: V001
- Source SHA: `00a5c8186d66640c159cf1834e394257754e7cf032a439e6274bc3b12a52b108`

## Recommendation: WORTHY（取代 V001 为 EPILOGUE-ARITH 主推）

## Trigger

跨窗一致 + same-binary PASS + 机制幅度同量级。

| 形状 | d4 | d6 | 方向 |
|---|---|---|---|
| 1x32768 | −6.64% (6/6) | −3.13% (4/6) | 两窗均 favor C |
| 2x16384 | −1.43% (5/6) | −3.62% (5/6) | 两窗均 favor C |

2x16384 effect/MAD = 4.7×。`ONLINE_DECISION = SELECTED_ON_TRIGGER`。

## 为什么 V002 取代 V001

| 项 | V001 | **V002** |
|---|---|---|
| 机制 | NORM-HOIST（整行 Muls） | GAMMA-FIRST-AXPY（Mul+Axpy 链） |
| 1x32768 | −2.55% | **−3.1~−6.6%** |
| 2x16384 | −2.65% | **−3.62%** |
| same-binary | PASS（窗口 3） | PASS（d6） |

V002 是 LOCAL_BEST，信号更强。

## Online 优先级（最终）

| # | Candidate | 信号 |
|---|---|---|
| **1** | **R31B V017** | Champion 血统 bf16 −14.3% + fp16 −6.7~8.5% |
| **2** | **R31A V028** | 双域 CachedRows −8.3% + batch −1.09% |
| **3** | **EPILOGUE-ARITH V002**（本推荐） | 跨窗 −3.1~−6.6% |

## Online package

- Source: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/submission.asc`
- SHA256: `00a5c8186d66640c159cf1834e394257754e7cf032a439e6274bc3b12a52b108`
