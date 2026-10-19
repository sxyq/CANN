# Revision 声明 — SHAPE-TILING-CHAMPION-X V001

（改代码之前记录；本文件在代码修改前写入。）

```text
ROUTE=SHAPE-TILING-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=Official 45.16
SINGLE_HYPOTHESIS=FP32 宽行且 8192 < D <= 16384 时，把 full-y 路径 seed tile 从 4096 提到 8192，使 UB 预算把字节花在更少的 tile 往返而非更多驻留 y 行；其余 (dtype, D) 格子保持原 policy。
CONTEXT_CLASS=CHAMPION_SEEDED_TILING
WHY_NOT_DUPLICATE=对照 DEDUP-MATRIX：WIDE-X-FRESH4 是弱父全局 2048→4096；R31B V016 是 Champion 上 FP16/BF16 seed 4096→8192；R31A 是 7680 CachedRows 另一函数路径；FULL-R029/MODE-X 是五模式 taxonomy 且正确性全败。本改动是 FP32 中宽 D 区间的 tile-UB 档位（shape 分档 + 驻留行数联动），四者均未覆盖该格子，也不引入多模式分派。
EXPECTED_SHAPES=主探针 FP32 D=12288, FP32 D=16384；空白对照 FP32 D=8192（非宽路径）, FP32 D=32768（本格子不动）, FP16/BF16 全 D（不动）
EXPECTED_RISK=正确性风险低（数值顺序与归约语义不变，只改 tile 切分与驻留行数）；性能方向不确定（驻留行数下降的参数面板摊销损失 vs tile 往返减半收益）
```

## MINIMAL_OFAT_DIFF

只改 `Init` 中 FP32 宽行分支的 seed tile 取值：`rowWidth <= 16384` 时 seed=8192，否则 seed=4096。
不改 `ChooseWideFullYRows` 算法、不改任何 `Process*` 函数、不改 reduction / store 事务 / dtype 数值路径 / 行调度。

## UB 预算核对（kWideFullYBudgetBytes = 176 KiB，硬上限 192 KiB）

| 形状 | 改后 (N, tile) | 预算占用 | 结论 |
|---|---|---|---|
| FP32 D=12288 | N=2, tile=8192 | 48KB×2 + 2×8192×4B = 160KB | 在预算内 |
| FP32 D=16384 | N=1, tile=8192 | 64KB×1 + 64KB = 128KB | 在预算内 |
| FP32 D=32768 | N=1, tile=4096（不变） | 128KB + 32KB = 160KB | 不受本改动影响 |

## 正确性门槛

历史五模式全部止步于正确性，本路线正确性优先于性能：
编译 → NPU 正确性（全 dtype × 主探针与对照 D）→ same-binary → 交错 P/C。
正确性未 PASS 不测性能、不进线上候选。
