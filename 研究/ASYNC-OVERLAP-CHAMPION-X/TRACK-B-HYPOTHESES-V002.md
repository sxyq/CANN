# TRACK-B-HYPOTHESES-V002 — ASYNC-OVERLAP-CHAMPION-X

状态：研究交付，**不开 V002 实现**。等 Main-2 / Planning 选一条后再声明 Revision。
上一 Revision：V001（H1 inter-pass prologue param prefetch）Local 三形状同向但
Official 44.17 < anchor 45.16，Δ=−0.99，**REJECT**。校准：`FALSE_POSITIVE_small_local_win`。

---

# 1. 归因：为何 wide LP 本地 1–3% 未转化为 Official

## 1.1 变更面与 Official case 的交集

V001 唯一改动面是 `ProcessWideLowPrecision`（FP16/BF16 且 `rowWidth > kCacheElems(8192)`）。
其余路径（FullCache FP32、NarrowMid、small/batched、generic）**逐字未动**，
correctness 的 OUTHASH 也确认非 LP 形状输出与 parent 逐位一致。

Official 15 case 里真正进入 LP 路径的只能是 **wide FP16/BF16** 子集。
按 `研究/OFFICIAL-CASE-ANALYSIS.md` 的量级分类：

| class | cases | 是否可能进 LP | V001 能否影响 |
|---|---|---|---|
| Tiny <10µs | 1,2,3,5 | 否（宽度小） | **否** |
| Small 10–70µs | 4,6,7,8 | 否（宽度小 / mid 或 small 路径） | **否** |
| Medium 70–170µs | 9–12 | 看宽度 | 仅当 wide FP16/BF16 |
| Large ~563µs | 13 | 看宽度 | 仅当 wide FP16/BF16 |
| Wide >1ms | **14,15** | **最可能** | 仅当 FP16/BF16（若 FP32 则走 FullCache，**否**） |

**关键推论**：V001 最多只能影响 14/15（若为 LP dtype）以及少数 medium 形状。
要让 15-case 均值掉 0.99 分，约需 case 分合计掉 ~15 分。
若只有 2–4 个 case 受影响，意味着**受影响 case 本身变慢了**，
而不是「没变快所以没涨」。

## 1.2 可能的变慢机制（在 H1 范围内）

1. **prologue 提前发的 HBM 竞争**：参数 Load 提前到 invRms 之前后，若该形状
   batchRows 小、invRms 极短，Load 与 pass-1 尾部 DMA 尾流仍可能重叠，
   在 HBM 紧张时反添竞争。本地探针 96/128 行 ×16384 是中等 batch，
   Official 14/15 若行数或 D 更极端，竞争形态不同。
2. **删掉 `SyncVToMTE2()` 的副作用**：该栅栏原本把参数 MTE2 排到 invRms V 之后。
   H1 让 Load 早发后删掉它。在 batchRows 大时 invRms 本身是长串标量往返，
   提前 Load 有覆盖窗口；在某些形状上可能让 MTE2 与 invRms 的 V/S 混跑
   带来不可预期的流水线停顿。
3. **测量侧假阳性**：本地全部在 d4 带 VLLM 负载的窗口完成；p10 快簇 12/12 偏
   V001，但干净对 median 只有 8/11，3 对近中性或微反向。p10 可能反映的是
   「candidate 对宿主干扰更不敏感」，而不是 judge 干净机上的真实加速。
   这与 `FALSE_POSITIVE_small_local_win` 校准一致。
4. **Local 形状与 Official 形状不对齐**：本地是 96/128 行 × 16384（FP16）与
   128×16384（BF16）。Official 14（16.5ms）更可能是更大 rows 或更宽 D
   （如 512×16384 或 64×32768）。tileCount / batchRows 组合不同，
   prologue 占比与竞争形态都不同。

## 1.3 结论（归因）

```text
1. V001 改不动 cases 1/2/3/5/4/6/7/8 —— 它们不在 LP 路径。
2. 能动的只有 wide FP16/BF16 子集（大概率含 14/15 的一部分）。
3. Official −0.99 说明该子集上净效应为负或本地信号为假阳性。
4. 若 case 14 是 FP32 wide（走 FullCache），V001 完全没碰到最大缺口 case。
5. 单纯「H1 帮助不够」解释不了负 Δ；必须有变慢或假阳性。
```

**对 V002 的直接含义**：要抬 Official，必须把机制覆盖到
**case 14/7/6/4/8**，其中 7/6/4/8 在 mid/small 路径，14 可能在 FullCache
或 LP。只做 LP 的 inter-pass 微调天花板太低。

---

# 2. 下一假设（V002 候选，3–5 条）

范围仍限定：pipeline scheduling / sync placement / issue order / loop structure。
禁止：加 buffer、加深 queue、纯 MTE3 stage、DTYPE / SCHED-CHAMPION / UB-LIVENESS 实现。

---

## W1 — FullCache inter-pass prologue param prefetch（H1 的路径扩展）

```text
EXTENSION_LABEL   H1_EXTENSION（同一机制，换到 FP32 wide 路径）
MECHANISM
  在 ProcessWideFp32FullCacheRows（L2082–2246）把 pass-2 tile-0 的 gamma/bias
  Load 从 invRms 标量回路之后（现 L2157 SyncVToMTE2 → L2191 Load）提前到
  回路之前发出。与 V001 在 LP 路径上的改动同构。
BOTTLENECK
  FP32 wide 的 inter-pass 串行边界：invRms 逐 batchRow 标量往返结束后才发参数 DMA。
  若 case 14 是 FP32 wide，这是直接对准最大缺口 case 的同一类机制。
EXPECTED_SHAPES
  FP32 rowWidth > 8192（FullCache / PanelResident 等 FP32 wide 一组路径）。
  对准 case 14（16.5ms，最可能 FP32 wide）与 case 15。
WHY_IT_MAY_HELP
  与 H1 相同的重叠逻辑；FullCache 的 invRms 回路结构（L2139–2156）与 LP 同形。
WHY_IT_MAY_FAIL
  H1 在 LP 上 Official 已 REJECT，同一机制迁移到 FP32 可能同样假阳性或变慢。
  必须先解决 §1.3 的测量可信度问题（干净窗口 + 形状对齐）再动手。
ASCEND_FEASIBILITY   高（纯 issue 位置移动）。
UB/CORE/DMA_IMPACT   不变 / 不变 / 字节不变，仅发出时间提前。
SYNC_IMPACT          与 H1 同：删/收窄 SyncVToMTE2，顺序改由 pass-1 释放保证。
PRECISION_RISK       无。
DUPLICATE_CHECK      vs V001：同机制不同路径，属 H1 延伸，不是重复。
                      vs FullCache 已有的 per-slot store 解耦：不同段。
MINIMAL_OFAT_DIFF    移动一处 Load+SetFlag；不动其他。
EXPECTED_LOCAL_PROBES 干净窗口（无 VLLM）+ 与 Official 14 量级对齐的形状
                      （大 rows 或 D=32768），先 same-binary 双块合格再 P/C。
CLASS                NEEDS_MORE_EVIDENCE（先做形状映射与干净窗口）
```

---

## W2 — NarrowMid 逐行参数发射重排（issue order / sync placement）

```text
EXTENSION_LABEL   NEW_MECHANISM（mid 路径，V001 未触碰）
MECHANISM
  ProcessNarrowMidOverlap（L499–620）现状：每行
  Load x/res → (可选 Load 参数) → V 计算 → SyncVToS/GetValue invRms →
  输出计算 → SyncVToMTE3 → Store。
  重排为：把下一行的 x/res Load（或当前行参数 Load）提前到 invRms 标量
  回路之前/期间发出，使 MTE2 与 V/S 标量尾重叠；行尾的 SyncMTE3ToV
  只在真正复用 outputLocal 前等待（延迟化）。
BOTTLENECK
  case 7/6/4/8 一类 16–69µs 形状，per-row 固定成本（V/S handoff、
  PipeBarrier、逐行冷启动 Load）占主导。NarrowMid 是 D∈(128,4096] 的
  T07 桶，直接对准 case 6/7 的量级。
EXPECTED_SHAPES
  rowWidth ∈ (128, 4096]，FP16/BF16/FP32，多行每核（localRows>1）。
  对准 case 6 (28µs)、7 (52µs)，也可能覆盖 4 (16µs)、8 (69µs)。
WHY_IT_MAY_HELP
  每行消掉「标量 invRms 往返 → 再等输入/参数 DMA」的串行段；
  行数越多收益越大。
WHY_IT_MAY_FAIL
  NarrowMid 已有 param overlap（L538–542 非 resident 时并行发参数），
  剩余空间可能是 V/S handoff 本身而非 DMA 时序。若 GetValue 往返是
  关键路径，调度重排无效。
ASCEND_FEASIBILITY   中高（行间结构重排 + 事件放置）。
UB/CORE/DMA_IMPACT   不变 / 不变 / 字节不变。
SYNC_IMPACT          中：行间 inputRelease / inputReady 顺序要重推，
                      须防 xBuf_ 别名（SCHED 路径曾出过此 race）。
PRECISION_RISK       无（同步正确时）。
DUPLICATE_CHECK      vs V001：不同路径不同机制段。
                      vs MIX-A sync 增删：这里是 overlap 重排，不是加/删 release。
                      vs EPILOGUE-FUSE / STORE-EPILOGUE：他们动 epilogue 数据流
                      与 store 形态，这里动行间发射顺序。
MINIMAL_OFAT_DIFF    只动 NarrowMid 行循环内的发射顺序 + 一处 sync 下沉。
EXPECTED_LOCAL_PROBES 用 case 6/7 量级形状（如 64×2048 / 32×4096），
                      干净窗口 + same-binary 合格后 P/C。
CLASS                READY_FOR_MAIN_REVIEW（覆盖缺口 case 的新轴）
```

---

## W3 — LP pass-2 store 延迟等待（旧 H2，重新对准 case 14）

```text
EXTENSION_LABEL   NEW_MECHANISM（相对 H1；Track-B 第一轮的 H2 未做）
MECHANISM
  ProcessWideLowPrecision pass-2 内层（V001 后约 L3338–3350）每 batchRow
  仍是 SyncVToMTE2 / SyncVToMTE3 / Store / SyncMTE3ToV 全耦合。
  改成 FullCache 已有的 per-slot MTE3_V 延迟等待形态，store 源改用
  不重叠的 y 行区域，不加 UB。
BOTTLENECK
  LP pass-2 store 完成被拉回标量 issue，batchRows 越大暴露越多。
  若 case 14/15 是 LP 且 batchRows 大，这是主暴露点。
EXPECTED_SHAPES
  FP16/BF16 wide，batchRows>1（大 rows / 小 blocks）。
WHY_IT_MAY_HELP
  把已暴露的 store 变成被 V 盖住的 store；FullCache 有先例。
WHY_IT_MAY_FAIL
  y 行区域生命周期复杂；事件方向写错会正确性失败。
  若 case 14 是 FP32 则完全打不中。
ASCEND_FEASIBILITY   中（依赖审计先行）。
UB/CORE/DMA_IMPACT   不变（复用已有 y 区域）/ 不变 / 字节不变。
SYNC_IMPACT          最高的一档（事件方向）。
PRECISION_RISK       无（同步正确时）。
DUPLICATE_CHECK      vs queue depth（V006/V009 proven-flat）：不同轴。
                      vs UB-LIVENESS：不改别名架构，只改 store 可见性。
MINIMAL_OFAT_DIFF    只改 LP pass-2 内层 store 段。
EXPECTED_LOCAL_PROBES 大 batchRows 的 LP 形状；先 correctness 字节对照再 P/C。
CLASS                NEEDS_MORE_EVIDENCE
```

---

## W4 — Small/Mid 路径的 V/S handoff 收敛（loop structure）

```text
EXTENSION_LABEL   NEW_MECHANISM
MECHANISM
  case 7/6/4/8 一类 16–69µs 形状里，每行多次
  SyncVToS / GetValue / SyncSToV 往返（NarrowMid L565–574 就有两轮）。
  在保持算术顺序不变的前提下，把这些标量 handoff 合并/上提：
  例如把 sum-of-squares 的 GetValue 与 invRms 计算合并到一次 V→S→V 周期，
  或把相邻行的标量段与下一行的 MTE2 重排到同一段。
BOTTLENECK
  短 kernel 的 per-row 标量往返与 PipeBarrier 串行链。
EXPECTED_SHAPES
  D∈(128,4096]，多行；对准 case 6/7/4/8。
WHY_IT_MAY_HELP
  减少每行固定往返次数，直接砍 per-row overhead。
WHY_IT_MAY_FAIL
  GetValue 语义要求 V 完成；合并可能改变浮点结合顺序 → 有精度风险。
  若 handoff 不是关键路径则无效。
ASCEND_FEASIBILITY   中（需小心不改算术顺序）。
UB/CORE/DMA_IMPACT   不变。
SYNC_IMPACT          减少同步点；风险是掩盖依赖。
PRECISION_RISK       **有**：若合并改变了求和/归一化顺序，会动数值。
                      必须保持与 parent 逐位一致或明示可接受容差。
DUPLICATE_CHECK      vs VECTOR-MATH-X（invRms 数学序列）：那是数学路径，
                      这里只动同步次数与位置，不动公式。
                      vs REDUCE-HIER（归约拓扑）：不动 ReduceSum 结构。
MINIMAL_OFAT_DIFF    一版只改一条路径的标量 handoff。
EXPECTED_LOCAL_PROBES 32×2048 / 64×1024 一类；先 correctness 字节对照。
CLASS                NEEDS_MORE_EVIDENCE（精度风险需先评估）
```

---

## W5 — H1 条件化发射（issue order 精修，H1 延伸）

```text
EXTENSION_LABEL   H1_EXTENSION（对 V001 的条件化精修）
MECHANISM
  V001 无条件把参数 Load 提到 invRms 之前。改为条件化：仅当
  batchRows>=2 或 tileCount>=4 时提前；否则保持原顺序。
  依据 §1.2：prologue 提前在小 batch / 短 invRms 时可能反而竞争。
BOTTLENECK
  H1 在 Official 上净负，可能来自小 batch 形状上的反效果。
EXPECTED_SHAPES
  同 LP 路径，但分开测小 batch（1 行/核）与大 batch（多行/核）。
WHY_IT_MAY_HELP
  保留 H1 在大 batch 的收益，去掉小 batch 的反效果。
WHY_IT_MAY_FAIL
  若 Official 14/15 都是大 batch 且 H1 本来就在上面变慢，
  条件化救不了；若变慢源自删 SyncVToMTE2，条件化也不对症。
ASCEND_FEASIBILITY   高（加一个 host 可见或 kernel 内条件）。
UB/CORE/DMA_IMPACT   不变。
SYNC_IMPACT          小。
PRECISION_RISK       无。
DUPLICATE_CHECK      vs V001：同一假设的精修，不是新的机制类别。
MINIMAL_OFAT_DIFF    在 H1 改动上加一个条件分支。
EXPECTED_LOCAL_PROBES 分别测 batchRows=1 与 batchRows>1 两档。
CLASS                NEEDS_MORE_EVIDENCE（先归因 §1.2 是哪种反效果）
```

---

# 3. 汇总与建议顺序

| ID | 机制 | 标签 | 对准 case | 分类 |
|---|---|---|---|---|
| **W2** | NarrowMid 行间发射重排 | NEW | **6/7/4/8** | READY_FOR_MAIN_REVIEW |
| W1 | FullCache inter-pass prologue | H1_EXT | **14/15（若 FP32）** | NEEDS_MORE_EVIDENCE |
| W3 | LP store 延迟等待 | NEW | 14/15（若 LP） | NEEDS_MORE_EVIDENCE |
| W4 | Small/Mid V/S handoff 收敛 | NEW | 6/7/4/8 | NEEDS_MORE_EVIDENCE（精度） |
| W5 | H1 条件化发射 | H1_EXT | LP 全体 | NEEDS_MORE_EVIDENCE |

**推荐 V002 = W2**（新机制、对准最大缺口 case 7/6/4/8、不重复 H1 的假阳性面）。
W1 作为 H1 延伸可作第三位，但须先做 Official 形状映射与干净窗口校准。

## 明确排除（本轮）

- 再加 buffer / queue depth（R31B V006/V009 T14 proven-flat）。
- 只再加 MTE3 stage（ASYNC-TRIPLE-X 已做）。
- 归约拓扑 / invRms 公式 / dtype 分裂 / row-group 调度 / UB 别名（他路线轴）。
- 纯粹重做 V001 而不做 §1 的形状对齐与干净窗口校准。

## 测量前置（写进 V002 任何实现前）

1. 确认 Official case 14/7/6/4/8 的 dtype 与宽度路径（映射未知是当前最大盲区）。
2. same-binary 必须在**无 VLLM 的干净窗口**做；d4 带载数据只作诊断。
3. 优先使用与 case 量级对齐的形状（7/6/4/8 → 16–70µs 档；14 → >1ms 档）。

---

Sources: `线上结果/R31B/V011/submission.asc`（parent）、
`本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/`（V001 证据）、
`线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/result.json`（Official 44.17）、
`研究/OFFICIAL-CASE-ANALYSIS.md`、`研究/OFFICIAL-WEAK-CASE-STRATEGY.md`、
`归档/历史控制文件/main2-r2-route-registry.md`（Official score structure）。
