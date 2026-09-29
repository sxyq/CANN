# HANDOFF-TO-MAIN — VECTOR-MATH-X V003 本地结论

DATE=2026-09-29
FROM=Route Agent (LANE M2-2, worktree cann-m2-vector, branch m2/vector-math)
TO=Main-2
REVISION=V003 SEQ-FUSE-2 (VMX-N1), DIRECT_PARENT=FROZEN_R31B_V011

## 1. 流程完成情况

| 阶段 | 结果 | 证据 |
|---|---|---|
| Revision 声明 | 已写（含 Div 安全形约束） | `本地实验/VECTOR-MATH-X/V003/REVISION-DECLARATION.md` |
| OFAT 实现 | PASS（单变量=denominator-tail placement） | 同上 SINGLE_CHANGE_AUDIT |
| compile / link | RC=0 / RC=0 | link SHA `89d69448…` |
| correctness | **53/54** | `correctness-v003.log` |
| same-binary | **PASS** 全部 6 个 S1 形状 | `support/results-s1-v003/sb.csv` |
| 交错 P/C | 6 形状 × 6 pairs 完成 | `support/results-s1-v003/pc.csv` |
| 本地结论 | **NEEDS_ONE_MORE_LOCAL** | `local-result.json` |

SOURCE_SHA `f019d099973b8ab4df07741b494376d9807a6d6a0b1686cfc0aca63ee4245bc8`（本地=server3 一致）。

## 2. 实现摘要（SEQ-FUSE-2 Form A）

12 个 live tail 站点的标量 handoff 链替换为纯 V 链 + broadcast apply：

```
ReduceSum → Brcb(meanSq, squareSumSlot) → Muls(invD) → Adds(ε) → Sqrt
→ Div(invRms8, ones8, meanSq, 8)     ← probe P1 形（三 slot 互异 32B 对齐）
→ SeqFuseApply: Level-0 Mul src1 stride-0 广播 8 元素块
```

- 每行消除 2 次 V/S round-trip、2 次 GetValue pull、1 次 Duplicate、3 个标量算子
- Div 精度：probe 已证 0 ulp（精确 IEEE）；dst 与两源互异、全部 32B 对齐
- 未触碰 reduction / store / DMA / scheduling / tiling / dtype

## 3. correctness 53/54 说明

唯一 FAIL = `resident_wide_colsplit` FP32 D=16384（max_abs 0.44175）。
**PRE_EXISTING_PARENT_FAILURE**：V002 同 case 同 max_abs 0.44175 FAIL；registry 记载 frozen 父版在 wide FP32 D=16384/32768 本就与 local-runner golden 不符。非 V003 回归。

## 4. 测量结果（S1 加长带，same-binary 全 PASS）

| 形状 | kernel 长度 | MAD/med | 配对 med Δ% | favor C |
|---|---:|---:|---:|---:|
| 768x256 | 9.8µs | 0.026 | **+0.99** | 1/6 |
| 1024x256 | 12.2µs | 0.020 | **+7.56** | 2/6 |
| 512x512 | 10.2µs | 0.022 | **+1.30** | 2/6 |
| 1024x128 | 11.5µs | 0.044 | **-0.68** | 4/6 |
| 768x128 | 9.7µs | 0.023 | **-0.20** | 4/6 |
| 2048x64 | 14.5µs | 0.013 | **+1.99** | 2/6 |

Δ = 100×(C−P)/P，负值 favor Candidate。**方向混合，无形状类别一致性，全部中位数 |Δ|<8%。**

DIR 带（5–6µs，same-binary 不合格，仅方向证据）：16x2048 med −4.03%（6/6 favor C）、8x1024 −1.02%（5/6）、4x2048 −5.12%（4/6）——方向偏正但按测量边界不可作 LOCAL_BEST。

控制形状 1x32768：same-binary PASS，med +3.3%——无回归，tail 摊薄符合预期。

## 5. 本地结论：NEEDS_ONE_MORE_LOCAL

- 不是 LOCAL_ACCEPTED：S1 带无一致胜势（2 favor C / 4 favor P）
- 不是 LOCAL_REJECTED：非一致退化（2 形状 favor C，pair 级方差大）
- 不是 MEASUREMENT_BLOCKED：S1 形状 same-binary 全部 PASS（S1 策略有效——长度分层问题已在形状侧解决）
- LOCAL_BEST 未推进

## 6. 建议与待决

1. **建议下一步跑 S3 tail-cost micro-probe**（合成 kernel 只跑分母 tail 链，长循环 device-event 计时）——回答机制是否真的省时间。若每行节省 ≈0，按 STOP_CONDITION 3 关闭该线。
2. 若 micro-probe 显示真实节省但 full-kernel 无胜势：**Form A 的 Level-0 broadcast Mul 可能比标量 Muls 慢**。可在同一 fused tail 内做 Form B（1 pull + Muls）A/B，隔离 apply 成本（仍属同一假设的 apply 步，由 probe 裁定的 A/B 选择，不构成第二机制）。
3. 1024x256 的 +7.56% 需注意：pair 级 4/6 偏 P。若下轮复测仍现，应记录为 apply 步可疑点。
4. Online：不建议（ONLINE_WORTHY=NO）。
5. 未改共享总账；V002 保持 frozen；LOCAL_BEST 未推进。

**STOP，等待 Main-2 裁定。**
