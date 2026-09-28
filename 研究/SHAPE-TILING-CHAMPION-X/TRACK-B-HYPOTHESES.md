# SHAPE-TILING-CHAMPION-X Track-B 假设池（V001 前）

日期：2026-09-28
前置：`DEDUP-MATRIX.md`（独立假设空间已确认）。
规则：每个假设 SINGLE_HYPOTHESIS、MINIMAL_OFAT_DIFF、WHY_NOT_DUPLICATE、EXPECTED_SHAPES、正确性风险优先。
排序：按「正确性风险 × 信息增量」从低到高。V001 取 H1。

---

## H1 — FP32 中宽 D 的 tile-UB 档位（tile 优先于驻留行）

- **SINGLE_HYPOTHESIS**：对 FP32 宽行且 8192 < D ≤ 16384，把 full-y 路径的 seed tile 从 4096 提到 8192，让 `ChooseWideFullYRows` 的 UB 预算把字节花在更少的 tile 往返上，而不是更多的驻留 y 行；其余 (dtype, D) 格子保持原 policy。
- **MECHANISM / BOTTLENECK**：D=16384 现状 N=2 / tile=4096（4 个 tile 往返/行，共 2 行）；seed=8192 后预算给出 N=1 / tile=8192（2 个往返/行）。每 tile 固定开销（Load×2 + Add/Mul/ReduceSum + 2 次 store 往返 + PipeBarrier）随往返数减半；代价是丢掉多行 y 驻留带来的参数面板摊销。
- **EXPECTED_SHAPES**：FP32 D=12288（N=3/tile=4096 → N=2/tile=8192）、FP32 D=16384（N=2 → N=1）。回归对照：FP32 D=8192（非宽路径，零差）、FP32 D=32768（本假设不动该格子，预期零差）、FP16/BF16 全 D（不动）。
- **WHY_IT_MAY_HELP**：tile 往返的固定开销与同步次数近似减半；更大 `n` 也提高向量段效率。
- **WHY_IT_MAY_FAIL**：驻留行数下降会把 gamma/bias 面板与 invRms 尾部按行重付；若官方该区间是大 N 形状，行摊销损失可能吃掉往返收益。
- **ASCEND_FEASIBILITY**：高。只改 Init 里 seed tile 的取值来源，`ChooseWideFullYRows`、buffer 分配、循环结构全部不变。`kWideFullYReduceStride=16` 下 tile 数上限仍满足（更大 tile ⇒ 更少 tile 数）。
- **UB 预算**：D=16384 tile=8192：y=64KB×1 + io=2×8192×4B=64KB + reduce 64B ≈ 128KB ≤ 176KB 预算。D=12288 tile=8192：y=48KB×2 + 64KB = 160KB ≤ 176KB。均在预算内，不触碰 192KB 硬上限。
- **正确性风险**：低。数据流、数值顺序、归约语义不变；只改 tile 切分与驻留行数。历史五模式失败点（SPLIT_D 两段归约、非对齐 D、BF16）都不在本假设的改动面上。
- **MINIMAL_OFAT_DIFF**：Init 中 FP32 宽行分支按 `rowWidth` 选 seed tile（≤16384 取 8192，否则 4096）。不改 `ChooseWideFullYRows` 算法、不改任何 Process* 函数、不改 reduction/store/dtype/scheduling。
- **DUPLICATE_CHECK**：vs WIDE（弱父全局 2048→4096）——不同格子、不同起点、shape 分档；vs V016（FP16/BF16 seed 4096→8192）——不同 dtype 分支；vs R31A（7680 CachedRows）——不同函数路径；vs 五模式——不引入多模式分派。
- **EXPECTED_LOCAL_PROBES**：FP32 D=12288/16384 为主探针（预期负 delta）；FP32 D=8192/32768 为空白对照（预期落在噪声带内）。

## H2 — FP32 D=32768 的 tile 上限档（6112/6144）

- **SINGLE_HYPOTHESIS**：对 FP32 宽行且 D=32768，把 seed tile 从 4096 提到 6144（预算边界：128KB + 2×6144×4B = 176KB），其余不动。
- **WHY_NOT_DUPLICATE**：同 H1 的去重逻辑，但作用格子是 D=32768（WIDE 测过弱父 4096，V016 未动 FP32；Champion 上 FP32 D=32768 tile=4096 未被任何路线加大）。
- **EXPECTED_SHAPES**：FP32 D=32768（T14 疑似同级形状）；对照 D=16384。
- **正确性风险**：低（同 H1）。注意 6112 已是 `kWideFp32BatchTileElems` 的既有取值，6144 与 6112 只差 32 元素，尾块用 `valid` 截断，无对齐新要求。
- **信息增量**：官方 case 14 缺口 4.4x，是全项目最大缺口；若它对应 D=32768/超宽形状，本假设的杠杆比 H1 更大。风险是 6144 只比 4096 大 1.5 倍，往返从 8 降到 6，收益可能小于 H1 的 2 倍。
- **状态**：READY_FOR_MAIN_REVIEW（排在 H1 之后）。

## H3 — SPLIT_D 单模式的安全形态（内核内 D-tile 两段归约）

- **SINGLE_HYPOTHESIS**：只在 Champion 的 FP32 超宽路径上实现 SPLIT_D 一个模式：一行按 D 切 tile，第一段在 UB 内累加 partial 平方和，第二段用 partial 求 invRms 后回写 y；不做其余四模式。
- **WHY_NOT_DUPLICATE**：五模式 taxonomy 从未在 Champion 源码上实现；R31B 现有 wide 路径是「整行 y 驻留 + 分 tile 读 x/res」，不是「y 不驻留的 D 切段两遍」。机制不同。
- **EXPECTED_SHAPES**：FP32 D=32768 且 N 大到无法 full-y 驻留多行时（N=1 时 Champion 已能驻留整行 y，本假设只在 y 放不进或驻留行数=1 且 tile 往返仍多时有区别）。
- **ASCEND_FEASIBILITY / 正确性风险**：**高风险**。MODE-X 的失败点正是 SPLIT_D（partial 累加精度、非对齐 D 尾块、两段之间的同步）。历史五模式全部止步于正确性。必须单独 Revision、正确性全 dtype 全 D 先过，再谈性能。
- **信息增量**：高——回答「显式 SPLIT_D 在 Champion 上是否仍有独立空间」这一 open question。
- **状态**：NEEDS_MORE_EVIDENCE（先出设计稿与正确性用例矩阵，不急于开 Revision）。

## H4 — MERGE_N 受限形态（多行联合归约，D≤2000）

- **SINGLE_HYPOTHESIS**：只对窄 D 多行（D≤2000、localRows≥2）做一次联合归约：多行 x/res 连续搬运，逐行 RMS 但共享 invRms 尾部与参数面板；不做 BATCH 级参数驻留、不做跨核归约。
- **WHY_NOT_DUPLICATE**：BATCH-RESIDENT-X 是参数驻留 + 批量 DMA（DMA 形态）；MIX-A 是 localRows==1 约束下的混合分派。MERGE_N 是归约形状的受限联合，作用面是归约尾部摊销。
- **边界警告**：与 BATCH-RESIDENT-X / MIX-A 领地相邻，立项前须再做一次领地确认；一旦发现改动面滑向批量 DMA 或分派切换，记 DUPLICATE 并停止。
- **正确性风险**：中高。多行联合归约的行间隔离（不能串行污染 partial）是历史 WA 高发点。
- **状态**：NEEDS_MORE_EVIDENCE。

## H5 — tile-UB 预算矩阵（多档一次性标定）

- **SINGLE_HYPOTHESIS**：不改任何单一档位，而是先在 Champion 上建立 (D × dtype × tile) 的 UB 预算与往返数矩阵，用同 binary 测出各档实际耗时，找出「最优档随 (D, dtype) 怎么变」。
- **性质**：这是测量/标定型假设，不产生性能 Revision；产出是 H1/H2 之外的新档位候选。
- **WHY_NOT_DUPLICATE**：UB-LIVENESS-X 的预算矩阵是针对 UB-LIVENESS V003 那套 TBuf/alias 源码的足迹分析，不是 Champion 的 full-y 路径。
- **正确性风险**：无（不改 kernel）。
- **状态**：READY_FOR_MAIN_REVIEW（可与 H1 的测量共用设备窗口）。

---

## 排序建议（给 Planning / Main）

1. **H1**（V001）：风险最低、OFAT 最干净、直接检验本路线主题「不同 D 区间的 tile policy」。
2. **H2**：若 H1 方向成立，D=32768 档是下一个单变量；若 H1 无效，H2 仍可独立做（不同格子）。
3. **H5**：与 H1/H2 的测量窗口合并做，补全档位地图。
4. **H3**：等 H1/H2 的正确性与测量链路跑通后再开；必须单独正确性预算。
5. **H4**：领地确认后再定。

## 预算

SOFT 6 个闭环性能 Revision；有收益可到 10；连续 3 个无收益 → `LANE_NEEDS_PLANNING_REVIEW`（由 Main 转呈，本路线不自行 PARK）。
