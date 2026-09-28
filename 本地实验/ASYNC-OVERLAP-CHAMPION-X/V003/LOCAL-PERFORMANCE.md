# V003 Local Performance — W4 GetValue handoff 收敛

协议：`runner_ref.inc`（device-event 主），warmup=45，samples=21，d4/d5，2026-09-28。
精度：按 `PRECISION-DECISION.md` 接受容差内 OUTHASH 差异（32×4096 max_abs 3.58e-07）。

## Same-binary

| shape | device | P | C | 资格 |
|---|---|---|---|---|
| 512×1024 fp32 | d4 | FAIL (drift 1.411) | FAIL (1.389) | 不合格 |
| 512×2048 fp32 | d4 | FAIL (1.321) | FAIL (1.163) | 不合格 |
| **512×1024 fp32** | **d5** | **PASS** (0.016/0.079, drift 1.042) | FAIL (drift 1.130) | parent 合格 |
| 512×2048 fp32 | d5 | FAIL (1.121) | FAIL (1.126) | 不合格 |
| 128×2048 fp32 | d4 | FAIL | PASS | 不合格 |
| control 33×100 | d4 | PASS | FAIL | 不合格 |

B2 系统性漂移 1.12–1.41，跨形状跨设备。仅 512×1024 d5 的 parent 双块合格。

## P/C ×6 交错（d5）

### 512×1024 fp32（kernel ~11 µs）

| pair | Δmed% | Δp10% | 方向 |
|---|---|---|---|
| 1 | **+9.31** | **+9.14** | parent |
| 2 | （C 脏） | +1.96 | — |
| 3 | **−10.21** | **−6.93** | V003 |
| 4 | **−10.00** | **−8.44** | V003 |
| 5 | **+10.09** | **+8.43** | parent |
| 6 | （P 脏） | −4.55 | — |

干净对：**2 偏 V003 / 2 偏 parent** —— 混杂（±10% 摆动）。

### 512×2048 fp32（kernel ~13 µs）

| pair | Δmed% | Δp10% | 方向 |
|---|---|---|---|
| 1 | **−2.59** | **−1.19** | V003 |
| 2 | （P 脏） | +2.58 | — |
| 3 | +1.15 | −0.44 | parent |
| 4 | −0.15 | −0.46 | 中性 |
| 5 | **−8.27** | **−7.51** | V003 |
| 6 | **+6.22** | **+5.23** | parent |

干净对：**2 偏 V003 / 2 偏 parent / 1 中性** —— 混杂。

## 本地结论

```text
LOCAL_VERDICT     NEEDS_ONE_MORE_LOCAL
REASON            精度门已接受（PRECISION-DECISION）。两形状 6 组交错干净对
                  方向均为混杂：512×1024 2/2（±10% 摆动），512×2048 2/2/1。
                  无稳定改善。W4 砍掉每行一轮 V-S 往返，但标量 sqrt 变更
                  可能抵消了同步收益；或收益低于 11–14 µs kernel 的噪声。
                  不推进 LOCAL_BEST；不开 V004。
```

## 观察

- W2（行间发射重排）与 W4（handoff 收敛）在 NarrowMid 上都未能给出稳定胜，
  印证该路径 per-row 固定成本的真正瓶颈可能在 **GetValue 往返的绝对次数**
  （每行至少要读一次 squareSum），而不是往返之间的间隔或发射时机。
- 512×1024 的 ±10% 摆动幅度接近或超过机制预期收益，说明当前窗口对
  ~10 µs kernel 的分辨率仍不足。

原始样本：`本地实验/ASYNC-OVERLAP-CHAMPION-X/V003/results/`。
