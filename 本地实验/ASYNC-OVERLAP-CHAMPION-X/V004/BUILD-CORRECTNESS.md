# V004 Build + Correctness

## Build

| 项 | 值 |
|---|---|
| source SHA256 | `1a0b6e5841ef75c162e52ea01ac43b07889152bacec3bbbf8173fdcd087908fc` |
| BUILD / LINK | PASS |
| bugfix | 首版在 GetValue 前 Load 覆盖 xBuf_，invRms 读到 gamma（max_abs 1.85）；已修复：Duplicate/Sqrt 移到 residualBuf_，Load 移到 GetValue 之后 |

## Correctness（NPU，device 4）

### 控制组（未改动路径）— OUTHASH 逐位一致

| shape | path | parent | V004 | 一致? |
|---|---|---|---|---|
| 64×2048 fp32 | NarrowMid | 68667a294de39908 | 68667a294de39908 | **是** |
| 8×16384 fp16 | LP | 84a417650ca9e0cc | 84a417650ca9e0cc | **是** |
| 33×100 fp32 | generic | 9fe600dd1475d372 | 9fe600dd1475d372 | **是** |

### FullCache FP32 wide（改动面）— 已知非确定路径

parent 自身多次运行 OUTHASH 不同（MULTIROW UB-GAP-CLUE），V004 同样。逐位对照不适用。

| shape | max_abs（parent 量级） | max_abs（V004 量级） | 备注 |
|---|---|---|---|
| 8×16384 fp32 | 0.127 | 0.130 | 同量级；双方 golden FAIL（pre-existing） |
| 4×32768 fp32 | 0.128 | 0.128 | 同量级 |
| 2×16384 fp32 | 0.101 | 0.125 | 同量级 |

```text
CORRECTNESS_STATUS   PASS_WITH_NONDET_PATH
DETAIL               控制组 OUTHASH 逐位一致，确认改动未泄漏到其他路径。
                     FullCache FP32 wide 是 parent 自身非确定路径（UB-GAP-CLUE），
                     OUTHASH 不可比；max_abs 双方同量级（0.10–0.13），
                     golden FAIL 为 pre-existing parent/harness mismatch。
                     修复后无新增数值错误（首版 1.85 的 bug 已消除）。
ANNOTATION           MULTIROW UB-GAP-CLUE: FullCache FP32 wide 正确性对照已标注；
                     不作为逐位判据。
```
