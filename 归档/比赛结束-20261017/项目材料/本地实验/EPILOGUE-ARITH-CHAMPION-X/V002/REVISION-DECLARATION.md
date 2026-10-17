# REVISION DECLARATION — EPILOGUE-ARITH-CHAMPION-X V002

（本文件在改代码之前写入。MAIN-1 已选定 EA-H2，2026-09-29。）

```text
ROUTE=EPILOGUE-ARITH-CHAMPION-X
REVISION=V002
DIRECT_PARENT=V001
PARENT_SOURCE_SHA=eda335d16e7e311b279e8918c6405c622bb7022e040459143f19a6240a546dbf
PARENT_SCORE=LOCAL_ACCEPTED -2.55% (1x32768_fp32) / 三窗 pooled -2.32%
SINGLE_HYPOTHESIS=EA-H2 GAMMA-FIRST-AXPY：把仿射表达式 (y·s)·g + b 重排为 (y·g)·s + b，并以 Mul(value,y,g) + Axpy(biasAcc,value,s) 实现（Axpy 语义 dst=src·scalar+dst，即 biasAcc = (y·g)·s + b），把 invRms 标量乘与 bias 加合成一步——依赖链 3 深变 2 深、每 tile V 趟 3→2（V001 的 hoisted normalize 由 Axpy 吸收，不再单独发射）
CONTEXT_CLASS=CHAMPION_SEEDED_EPILOGUE_ARITH
WHY_NOT_DUPLICATE=与 V001 NORM-HOIST 不同轴（V001 动的是标量乘的应用位置/粒度，表达式不变；本假设改的是表达式结合序与融合指令形态）；与 EPILOGUE-FUSE-X V002/V003 的 MulAddDst/VMLA 不同（他们融合 gamma 乘 + bias 加、保留独立 normalize，表达式 (y·s)·g+b；本假设融合 invRms 标量乘 + bias 加、指令 Axpy=标量乘加，表达式 (y·g)·s+b）；与 SCALE-FOLD 不同（不建 s·g 刻度瓦片）；与 VECTOR-MATH-X 不同（invRms 求值链不动，无 Rsqrt/Reciprocal）；与 STORE-EPILOGUE-X 不同（Store 调用/事件/写回形态逐字不动）
EXPECTED_SHAPES=宽 FP32 且 batchRows==1（bias 瓦片单次消费）：1x32768、2x16384、8x16384、2x32768；同码对照 2x8192（不进本函数）
EXPECTED_RISK=浮点结合序（(y·s)·g vs (y·g)·s，1 ulp 级）+ Axpy 融合乘加舍入；中等精度风险，须全形状正确性对照父版 max_abs
```

## 变更点（单变量：仿射表达式形态 (y·g)·s+b 用 Mul+Axpy 实现）

仅 `ProcessWideFp32FullCacheRows` 第二遍（与 V001 同一函数同一段）：

1. 合法性闸门：仅当 `batchRows == 1`（该 tile 的 bias 瓦片被恰好一行消费、可作 Axpy 累加底）时启用新形态；
   闸门关闭分支逐字保留 V001 链（hoisted normalize + Mul + Add，store 源不变）。
2. 闸门开启时：
   - 跳过 V001 的整行 hoisted `Muls`（标量乘移入 Axpy，同一机制的必然对应面）。
   - 每 tile：`Mul(valueRow[col], valueRow[col], gammaLocal)` → `Axpy(biasLocal, valueRow[col], invRms)`。
   - `Store` 的 UB 源张量改为 `biasLocal`（算术链落点；Store 调用/事件/循环/写回形态不变）。
     既有 MTE3 落盘-再装载纪律注释（"drain any store still referencing the previous
     tile's staging pair"）本就覆盖 store 读 staging pair 的情形。

## 身份

- PARENT_SOURCE_SHA = V001 submission SHA `eda335d1…`（MAIN-1 parent-module 规则：parent module
  重建为 V001，不复用 harness 默认父版）。
- Candidate 源码：本目录 `submission.asc`；`diff.patch` 记与 V001 的差异。
