# REVISION DECLARATION — EPILOGUE-ARITH-CHAMPION-X V001

（本文件在改代码之前写入。）

```text
ROUTE=EPILOGUE-ARITH-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=Official 45.16
SINGLE_HYPOTHESIS=EA-H1 NORM-HOIST：invRms 的应用时机与粒度——在 ProcessWideFp32FullCacheRows 第二遍，把逐 tile 逐行的 Muls(valueRow[col], invRms) 改为 invRms 就绪后对驻留 y 整行每行一次 Muls，gamma/bias 循环与 Store 不动（表达式仍为 (y·s)·g + b，逐元素按位一致）
CONTEXT_CLASS=CHAMPION_SEEDED_EPILOGUE_ARITH
WHY_NOT_DUPLICATE=STORE-EPILOGUE-X 只改输出回写形态（本假设 Store/事件逐字不动）；VECTOR-MATH-X 改 invRms 求值序列 Duplicate+Sqrt+GetValue/倒数（本假设一行不碰，只改标量乘的应用位置）；EPILOGUE-FUSE-X SCALE-FOLD 把 s 折进 gamma 刻度瓦片、MulAddDst/VMLA 融合仿射、ROW-WIDE-AFFINE 把 s·g·b 三步整行发射（本假设表达式保持 (y·s)·g+b、无融合指令、只动宽路径的 normalize 一步，且 gamma/bias 仍按 tile 流入）；R31A 的 affine/Rsqrt 属其自身 Track-B 想法且 invRms 求值侧与 VECTOR-MATH 重合（本假设不涉 Rsqrt）
EXPECTED_SHAPES=宽 FP32（rowWidth>8192）：1×16384、1×32768、2×16384、2×32768、4×12288 等；零差控制 2×8192_fp32（不走该函数）
EXPECTED_RISK=零浮点结合序风险（Muls 逐元素乘同一标量，整行一次与分 tile 多次按位一致）；残余风险仅在大 calCount 发射的工具链接受度
```

## 变更点（单变量）

仅 `ProcessWideFp32FullCacheRows` 第二遍（种子 L2174–2207 区域）：

- 删除：tile 循环内逐 batchRow 的 `Muls(valueRow[col], invRmsValues[batchRow], valid)` 及其 `PipeBarrier`（L2194–2199）。
- 新增：invRmsValues 就绪后、参数 tile 循环之前，逐 batchRow 一次
  `Muls(valueLocal[batchRow * rowWidth], …, invRmsValues[batchRow], rowWidth)`，其后一道 `PipeBarrier<PIPE_V>`。
- 其余（gamma/bias Load、Mul、Add、Store、事件、其他函数）字节不动。

## 身份

- 本地种子核验：`线上结果/R31B/V011/submission.asc` SHA256 = PARENT_SOURCE_SHA（开工前 shasum 复核一致）。
- Candidate 源码：本目录 `submission.asc`；`submission.sha256` 记其 SHA；`diff.patch` 记与父版差异。
