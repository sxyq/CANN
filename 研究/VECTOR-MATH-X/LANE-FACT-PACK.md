# LANE-FACT-PACK — VECTOR-MATH-X / SEQ-FUSE-2 轴关闭

DATE=2026-09-29
ROUTE=VECTOR-MATH-X (LANE M2-2)
SUBJECT=SEQ-FUSE-2 (VMX-N1) denominator-tail placement — 机制级判定与关闭建议
EVIDENCE=本地实验/VECTOR-MATH-X/S3-TAIL-PROBE/、本地实验/VECTOR-MATH-X/V003/
PROBE_DATE=2026-09-29, device 5, iters=10000, 5 reps, device-event median

## 1. 结论

**每行节省为负（机制净亏），按 STOP_CONDITION 关轴。** 不满足 Form B / V004 的触发条件
（其前提「broadcast Mul 慢于标量 Muls」被 probe 证伪——broadcast 反而更快）。

## 2. S3 tail-cost micro-probe：µs/row 分解

探针在单 kernel 内循环 10000 次跑分母 tail + apply，device-event 计时，
除以行数得 µs/row。五变体共用同一 seed/accumulate 步（抵消）。

### valid=256（中带 apply 宽度）

| 变体 | 组成 | µs/row | Δ vs PARENT |
|---|---|---:|---:|
| FLOOR | 仅标量 Muls apply | 0.0613 | −0.069 |
| RT-ONLY | +2 次 V/S round-trip + 3 标量算子 + Duplicate | 0.1108 | −0.020 |
| **PARENT** | RT-ONLY + Sqrt(1)（原分母 tail） | **0.1305** | 0 |
| **FORM_A** | 纯 V 链 + broadcast apply（=V003 实现） | **0.1421** | **+0.0116** |
| FORM_B | 纯 V 链 + 1 pull + 标量 Muls | 0.1499 | +0.0194 |

### valid=64（小形状 apply 宽度）— 符号一致

| 变体 | µs/row | Δ vs PARENT |
|---|---:|---:|
| FLOOR | 0.0594 | — |
| RT-ONLY | 0.1088 | — |
| PARENT | 0.1288 | 0 |
| FORM_A | 0.1394 | +0.0106 |
| FORM_B | 0.1484 | +0.0196 |

重复性：5 rep 内中位数稳定到 ±0.001 µs/row（个别 outlier rep 如 0.164 由宿主干扰，
median 跳过）。Δ=+0.011 远超噪声。

### 分解读数

1. **2 次 V/S round-trip + 3 标量算子的全部成本 ≈ 0.049 µs/row**（RT-ONLY − FLOOR）。
   这是本假设瞄准的开销上界。
2. **纯 V 链（Brcb+Muls+Adds+Sqrt+Div，count=8）+ broadcast apply 比原 tail 更贵 +0.012 µs/row**。
   即：省下的 handoff 被链本身的 V 指令开销吃掉还倒贴。
3. **broadcast apply 比 1-pull+Muls 快 0.008 µs/row**（FORM_A < FORM_B）
   ——Main 判定规则 2 的前提（broadcast Mul 慢于标量 Muls）**不成立**。
   问题不在 apply，在链本身不划算。
4. 上界核算：若 tail 完美归零，每核行数 ~19（768 行/40 核）→ kernel 上限收益 ~13%。
   实测净亏 → 该轴的实测方向与假设相反。

## 3. 与 V003 full-kernel 数据交叉验证

V003 S1 带（same-binary 全 PASS，6 形状 × 6 pairs）：2 形状 favor C（−0.2%/−0.7%）、
4 形状 favor P（+1.0%～+7.6%），无一致性。probe 预示每行 +0.012 µs 成本 →
按每核 ~19–51 行折算 kernel +2%～+4%，与实测 +1.0%/+2.0% 同号同量级。
**两条证据链一致：机制在当前实现下是净亏，full-kernel 无胜势不是测量噪声。**

## 4. 残余不确定度（诚实记录）

- micro-probe 的 tail 在空闲标量单元下串行测得；真实 kernel 中 V/S handoff 的停顿
  可能比隔离值更高（V/S 管线另有任务时）。但 full-kernel S1 数据同样偏负/混合，
  两条独立证据同向，该不确定度不改变结论。
- 未测其他 count（如 count=1 链）；纯 V 链若改用 count=1 形式可能省一点 V 开销，
  但 Brcb+Div 广播步仍在，理论剩余空间 <0.01 µs/row，不足以翻盘。

## 5. 建议

1. **关闭 SEQ-FUSE-2 轴**（STOP_CONDITION 3：tail handoff 不是当前实现下的净收益来源）。
2. **不提 Form B / V004**——触发前提不成立（broadcast apply 已是两种 apply 中较快者）。
3. VECTOR-MATH-X 轴上剩余可试方向（若 Planning 要保留 lane）：
   - VMX-N2 BATCHED-DENOM-CHAIN（批量摊 handoff，减少链指令数/行）——需重新做机制级 probe
   - 或接受该轴在 FROZEN_R31B_V011 上已到天花板，转向其他 lane。
4. 本包不涉及 Online、不推进 LOCAL_BEST、未改共享总账。
