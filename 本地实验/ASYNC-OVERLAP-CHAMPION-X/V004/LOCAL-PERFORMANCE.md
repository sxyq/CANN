# V004 Local Performance — W1 FullCache inter-pass prologue

协议：`runner_ref.inc`，warmup=45，samples=21，device 4，2026-09-28。
正确性：控制组逐位一致；FullCache 为 parent 非确定路径（UB-GAP-CLUE，已标注）。
bugfix：首版在 GetValue 前 Load 覆盖 xBuf_（max_abs 1.85），已修复。

## Same-binary

| shape | P | C | 资格 |
|---|---|---|---|
| **128×16384 fp32** | PASS (0.019/0.034, drift 1.007) | PASS (0.012/0.010, drift 1.013) | **合格** |
| 64×16384 fp32 | PASS | FAIL (drift 1.579) | 不合格 |
| 64×32768 fp32 | PASS | FAIL (drift 1.678) | 不合格 |

128×16384 fp32 kernel ~23 µs，是唯一双方合格形状。

## P/C ×6 交错（128×16384 fp32）

| pair | P med / MAD | C med / MAD | Δmed% | Δp10% | 方向 |
|---|---|---|---|---|---|
| 1 | 23.90 / 0.24 | 23.60 / 0.28 | **−1.26** | **−1.02** | V004 |
| 2 | 23.50 / 0.34 | 22.90 / 0.18 | **−2.55** | **−2.88** | V004 |
| 3 | 22.90 / 0.52 | 23.78 / 0.26 | **+3.84** | **+1.72** | parent |
| 4 | 23.30 / 0.20 | 23.72 / 0.48 | **+1.80** | **+2.03** | parent |
| 5 | 22.70 / 0.56 | 23.02 / 0.46 | **+1.41** | **+2.45** | parent |
| 6 | 24.22 / 0.32 | 23.04 / 0.42 | **−4.87** | **−3.59** | V004 |

干净对：**3 偏 V004 / 3 偏 parent** —— 完全对半开。p10 同样 3/3。
幅度 1–5%，落在噪声带内。

## 本地结论

```text
LOCAL_VERDICT     NEEDS_ONE_MORE_LOCAL
REASON            128×16384 fp32 双方 SB 合格，6 组干净对方向完全对半开。
                  W1 在 FullCache 上的重叠窗口比 H1 在 LP 上小：
                  invRms 把 xBuf_ 当 scratch 用到最后一次 GetValue，
                  提前 Load 只能盖住 Duplicate/Sqrt + 最后标量尾。
                  无稳定改善。不推进 LOCAL_BEST。
```

## 结构观察（供下一假设）

FullCache 的 invRms 把 xBuf_/residualBuf_ 当 scratch，导致 pass-2 参数 Load
无法像 LP 那样在整个 invRms 回路期间提前。要扩大重叠窗口，需要把 invRms
scratch 挪到专用小缓冲（valueFp32Buf_ 尾部或 reduceFp32Buf_ 空闲槽），
使 xBuf_ 在 invRms 一开始就空闲——那是 buffer 用法变化，须单独声明。

原始样本：`本地实验/ASYNC-OVERLAP-CHAMPION-X/V004/results/`。
