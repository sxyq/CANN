ROUTE=STORE-EPILOGUE-X
REVISION=V003
DIRECT_PARENT=V002
PARENT_SOURCE_SHA=59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839
PARENT_SCORE=LOCAL -5.62% (1x32768 FP32, 6/0 clean pairs) / Official 45.07
SINGLE_HYPOTHESIS=分块合并写回（H1 STORE-H3）：mergeRowRuns 分支把行内 run 拆成 K=2 块（块边界取 tile 整数倍），每块算术完成后立刻发块级合并 Store，使 MTE3 与下一块算术重叠；ring 仍 2-deep，每 tile 算术次数与顺序不动，不跨行。
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_OUTPUT_STORE
WHY_NOT_DUPLICATE=V001=不拆分合并+多站点铺开（t=2 中带回退）；V002=整行合并、算术全结束后才发 Store（无重叠）。本版变量只有写回块粒度与发出点。R31A V021=按 tile 写回不变、等待点后移（同步点轴，不动）；ASYNC-TRIPLE-X=ring 深度/旗帜/in-flight 数量（不动）；R015=跨行合并（块严格行内）；ALIGN-TAIL=DataCopy/DataCopyPad 选型（不动，仍 Pad）；EPILOGUE-ARITH=算术链（Muls/Mul/Add 次数顺序不动）。与 H4 的区别：H4 改 gate-OFF 父版块的假等待；本版只保证 merge 分支保持 V002 已有的「循环内无 MTE3_V 等待」行为（merged store 源是 valueLocal，从不引用 staging 对），使提早发出的块能真正与后续算术并行。
MINIMAL_OFAT_DIFF=仅 ProcessWideFp32FullCacheRows 输出遍 merge 分支：(1) 判定后加 mergeChunkSplit=tileCount/2；(2) tile 循环内块边界处发块级合并 Store（2 块）；(3) 删循环后整行 Store 块；(4) 循环头 staging drain 加 !mergeRowRuns 条件（merge 分支 V002 起该 drain 从不触发，语义不变，仅让提早发出的 store 不被下一块开头误等）。
K=2（tileCount=8 → 2x4 tiles=2x64KB FP32；tileCount=4 → 2x2 tiles=2x32KB）
MAIN_SELECTION=MAIN-1 2026-09-29 选定 H1（研究/STORE-EPILOGUE-X/next-hypotheses.md H1 节）
MEASUREMENT_PLAN=主探针 1x32768 FP32 / 1x16384 FP32 / 2x16384 / 8x16384 / 2x32768；对照 t<4 形状（应与父版逐字节同码）；H2 断点补测可同窗口（不混入 kernel 改动）
