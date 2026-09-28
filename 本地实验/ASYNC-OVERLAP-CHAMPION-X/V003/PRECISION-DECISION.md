# PRECISION-DECISION — V003 W4

```text
DECISION          ACCEPT_TOLERANCE_WITHIN_DIFF（Main-2 2026-09-28 裁定）
REVISION          V003
HYPOTHESIS        W4 GetValue handoff 收敛
SOURCE_SHA        d23593ce88fd7011a604f50cfba3948904f4f97f66641346626143dd16af3548
```

## 接受的事实

| 项 | 值 |
|---|---|
| OUTHASH 不一致形状 | 仅 32×4096 fp32 |
| V003 hash / parent hash | `9ba09b85c4c954f5` / `fe50b2a35f2769cf` |
| 两侧 golden | 均 PASS（bad=0） |
| max_abs_error | 3.5762787e-07 |
| FP32 容差（harness） | 3e-5 |
| 比值 | max_abs ≈ 容差的 1/84 |
| 其余 6 形状 | OUTHASH 逐位一致 |

## 接受理由

1. 差异量级 3.58e-07，远在 FP32 容差内，两侧 golden 都过。
2. W4 验证的是 handoff 次数，不是 bit-exact sqrt；要求 bit-identical 会使该轴无法验证。
3. 父版本身已有 golden 容差差异先例（FP16 宽行 err=0.00293）。
4. 差异来源明确：`std::sqrt`（标量）与 `AscendC::Sqrt`（向量）在特定 meanSquare 上舍入不同。

## 风险记录

- Official judge 的容差若严于本地 harness，32×4096 类形状可能出现 WA。
- 差异只在部分 meanSquare 上触发（同 dtype 的 64×2048、128×512 逐位一致），
  不能保证所有 15 case 都 bit 一致。
- 若 Online 正确性失败，归因到本 Revision 的 sqrt 执行侧变更。

## 回滚点

```text
DIRECT_PARENT   R31B-V011
PARENT_SHA      a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
回滚动作        下一 Revision 直接以 R31B-V011 为 seed；V003 不进 LOCAL_BEST
                （除非 LOCAL_ACCEPTED 且 Main 再审精度后放行）。
```

## 规则边界

- **不放宽容差判定规则**；只是本 Revision 允许非 bit-identical 的 OUTHASH。
- 任何后续 Revision 恢复默认 bit-identical 要求，除非 Main 再次明示。
- LOCAL_ACCEPTED 之后、Online 之前，Main-2 会再审精度风险。

## 继续项

同一 revision 继续 same-binary + 交错 P/C（512×2048 / 512×1024 fp32 + 对照）。
