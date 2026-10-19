# SHAPE-TILING-CHAMPION-X 去重矩阵

日期：2026-09-28
范围：创建任何 Revision 之前的强制去重前置（Track-B 第 1 步）。
对照对象：WIDE-X-FRESH4 V001、FULL-R029 / MODE-X 五模式、R31B 当前宽行路径、R31A shape specialization。

## 1. 四者实际做过什么（证据源）

| 对象 | 机制 | 起点源码 | 已有结果 | 证据 |
|---|---|---|---|---|
| WIDE-X-FRESH4 V001 | 宽路径 tile 常量 2048→4096，并放大该实例化 `tmp_`；fallback 分配保留 | 弱父 BUILD-FIX-001（无 Official） | 正确性 18/18；D32768 mean -17.1%、D16384 -16.8%（4/4）`LOCAL_ACCEPTED`；Official 19.72 | `本地实验/WIDE-X-FRESH4/V001/source-meta.json` |
| FULL-R029 / MODE-X 五模式 | 运行时分档 taxonomy：NORMAL / SPLIT_D / MERGE_N / SINGLE_N / MULTI_N，按 (D, outer, UB, 核数) 切换 | 独立实现（非 Champion 源码） | FULL-R029 线上 4/15 RE；MODE-X 正确性在 SPLIT_D、非对齐 D、BF16 失败；后续 FRESH/R015B 无源码 | `归档/历史工作区/MODE-X/HANDOFF_V001.md`；`技术路线/路线成绩表.tsv` MODE-X 行 |
| R31B 当前宽行路径 | `ProcessWideFp32FullCacheRows` + LP 管线 + full-y 自适应驻留（`ChooseWideFullYRows`）；V016 把 FP16/BF16 宽行 seed tile 4096→8192 | R31B V011（Official 45.16，Champion） | V011 45.16；V016 正确性 change-domain PASS，测量 `MEASUREMENT_BLOCKED` | `线上结果/R31B/V011/source-meta.json`；`本地实验/R31B/V016/{source-meta.json,compile-evidence.txt}` |
| R31A shape specialization | `kWideFp32CachedTileElems=7680` 的 FP32 CachedRows；V016 加 D=32768 full-y 分派；V021 推迟 store 等待 | R31A lineage（Official 45.00） | V016 45.00；V021 本地方向混杂 | `技术路线/路线成绩表.tsv` R31A 行；`本地实验/R31A/V021/` |

Champion 源码内的 tiling 常量现状（`线上结果/R31B/V011/submission.asc:1241-1288`）：
`kTileElems=4096`、`kWideTileElems=8192`、`kWideFp32TileElems=12288`、`kWideFp32BatchTileElems=6112`、
`kWideFp32CachedTileElems=7680`、`kWideFp16CachedTileElems=7680`、`kWideFullYTileElems=4096`、`kCacheElems=8192`。
宽路径入口 `rowWidth > kCacheElems`（即 D≥12288 的整数档）；FP32 宽行统一进 `ProcessWideFp32FullCacheRows`，
tile 与驻留行数由 `ChooseWideFullYRows` 按 176KB UB 预算决定（优先最大化驻留行数，再缩 tile）。

## 2. 相同点

1. 都在处理「一行放不进 UB / tile 粒度怎么切」问题；都是 shape-conditioned。
2. tile 与 UB 预算的权衡是共用底层；WIDE 与 R31B V016 已覆盖「单纯把某个 tile 常量加大」这条思路。
3. R31A 与 R31B 宽路径同源（相似度 0.947），不是两套独立架构。
4. 五模式的 SPLIT_D / SINGLE_N 与 R31B 的 tile 循环、LP 管线在「按 D 切块 + 内核内两段」这一层有概念交叠。

## 3. 不同点

| 维度 | WIDE-X-FRESH4 V001 | R31B V016 | R31A specialization | FULL-R029 / MODE-X | 本路线要做的事 |
|---|---|---|---|---|---|
| 改动粒度 | 单常量 + tmp 尺寸 | 单常量（仅 FP16/BF16 seed） | 固定 7680 + 分派阈值 | 整套运行时分档 + 多条 kernel 路径 | 单条 shape/tile policy（每次一个 D 区间或一个模式） |
| dtype / D 覆盖 | 三 dtype 同一 tile，弱父 | 只动 FP16/BF16，FP32 未动 | FP32 CachedRows 7680 | 五模式含 FP16 对齐 / MERGE_N 上限 | FP32 中宽 D 的 tile-UB 档位；后续单模式 |
| rows 策略 | 未按 rows 分档 | 未改 rows 选择 | full-y 单行为主 | SINGLE_N/MERGE_N/MULTI_N 显式按 outer | 只通过 UB 预算间接影响 rows，不新增 rows 分档 |
| 与 UB 预算关系 | 只放大单 tile | 只放大 seed tile | 固定常量 | 宿主算 rowFactor/ubFactor | 显式做 tile ↔ 驻留行数的预算权衡 |
| 起点强度 | 弱父，Official 19.72 | Champion 45.16 | 45.00 | 弱/失败（正确性从未全过） | Champion 45.16 exact seed |
| 验证深度 | 本地合格，Official 弱 | 测量未完成 | Official 45.00 | 正确性失败 | 正确性优先，再 same-binary + 交错 P/C |

## 4. 去重结论

**判定：独立假设空间成立，可创建 Revision。** 依据：

1. 「单纯 tile 加宽」（全局加大一个 tile 常量）已被 WIDE-X-FRESH4（弱父 2048→4096）与 R31B V016（Champion 上 FP16/BF16 4096→8192）覆盖。本路线不再做全局常量加大。
2. **tile-UB 权衡的中间档未覆盖**：Champion 上 FP32 中宽 D（8192 < D ≤ 16384）当前是 seed tile=4096 + 多行 full-y 驻留（D=16384 → N=2/tile=4096；D=12288 → N=3/tile=4096）。tile=6144/8192 这一档、以及「少驻留行、更大 tile」的取舍方向，四者都没做过。
3. **五模式显式 taxonomy 在 Champion 上未验证**：SPLIT_D 内核内 D-tile 两段归约、MERGE_N 多行联合归约、SINGLE_N 单核事件流水，都从未落在 Champion 源码上。历史实现全部止步于正确性。这是一个独立但高风险的空间，必须单模式、正确性优先。
4. R31A 的 7680 CachedRows 是另一条函数路径（`ProcessWideFp32CachedRows`，Champion 上 `ProcessWideFp32` 已不再调用它，统一走 `ProcessWideFp32FullCacheRows`），与 full-y 路径的 tile seed 不是同一改动点。

**重叠红线（本路线不得触碰）**

- 不做全局 tile 常量单调加大（WIDE / V016 已做）。
- 不改 reduction 形态、store 事务、dtype 专用数值路径、行调度（属其他 Lane）。
- 不做 BATCH-RESIDENT-X 的批量 DMA / 参数驻留，不做 MIX-A 的 localRows 路径。
- SPLIT_D / MERGE_N 只做单模式受限形态，不重建完整五模式分档。
