# V003 精度门 — OUTHASH 逐位对照结果（STOP）

状态：**按 Main-2 指令停下，报数值差异。不测时、不自行换方案。**

## 对照结果

| shape | dtype | V003 hash | parent hash | 逐位一致? | vs golden |
|---|---|---|---|---|---|
| 64×2048 | fp32 | 68667a294de39908 | 68667a294de39908 | **是** | PASS |
| 64×2048 | fp16 | 100b419edd74fcb0 | 100b419edd74fcb0 | **是** | 容差 2 元素（双方相同） |
| 64×2048 | bf16 | 52f7fcc0048eeee7 | 52f7fcc0048eeee7 | **是** | PASS |
| **32×4096** | **fp32** | **9ba09b85c4c954f5** | **fe50b2a35f2769cf** | **否** | PASS（max_abs 3.58e-07） |
| 128×512 | fp32 | b9790b09a714c53e | b9790b09a714c53e | **是** | PASS |
| 8×8192 | fp16 | 04b38d055725c9b1 | 04b38d055725c9b1 | **是** | 容差 1 元素（双方相同） |
| 33×100 | fp32 | 9fe600dd1475d372 | 9fe600dd1475d372 | **是** | PASS |

## 差异定性

- 只有 **32×4096 fp32** 一处 OUTHASH 不一致。
- 两侧 golden 都 PASS（bad=0, max_abs=3.5762787e-07），即差异**落在容差内**，
  但不是 bit-identical。
- 原因与声明中的精度风险预判一致：`std::sqrt` 从向量 `Sqrt` 改到标量 sqrt，
  在某些 meanSquare 取值上舍入不同，经过 `1.0f /` 与后续 Muls 被放大到
  最后一个 bit。
- 其余 6 形状（含同 dtype 的 64×2048、128×512）逐位一致，说明差异只在
  特定 meanSquare 数值上触发，不是系统性公式错误。

## 已做 / 未做

```text
已做   V003 声明、OFAT 源码、编译、OUTHASH 逐位对照、证据固化
未做   same-binary、P/C、任何测时（按指令：差异即停）
未做   不自行改成向量侧方案、不自行放宽容差判定
```

## 请 Main-2 决定

1. **接受容差内差异**（32×4096 max_abs 3.58e-07 ≪ FP32 容差 3e-5），
   进入测时？还是
2. **要求 bit-identical**，本实现作废，改试「向量侧合并」方案
   （保留向量 Sqrt，把 1/sqrt 也放到向量侧，减少 GetValue 但 sqrt 仍在 Vector）？
3. 或 **W4 记 INFEASIBLE_under_bit_identity**，换 W1？

实现与证据保留：`本地实验/ASYNC-OVERLAP-CHAMPION-X/V003/`。
source SHA `d23593ce88fd7011a604f50cfba3948904f4f97f66641346626143dd16af3548`。
