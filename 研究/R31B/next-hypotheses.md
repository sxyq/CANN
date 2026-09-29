# R31B Track-B 下一轮假设（V011/V016 Champion 宽行 LP 路径）

日期：2026-09-29
ROUTE=R31B
CONTEXT_CLASS=HISTORICAL_EXPLOIT
起点：R31B-V011（SOURCE_SHA `a8c19a19…b15e3`，Official 45.16）；V016（`9f5c353e…`，LP 宽行 tile 8192）为其上唯一变更。
范围：只研究，不改 kernel；实现前需 Main 选定 1 个假设。

## 0. 对齐 case 14 归属（MAIN-1 通报，纳入方法论）

case 14（Official 最大缺口 4.4x）主缺口 = **同步把 MTE2/V/MTE3 排成串行，管线重叠不足**（msprof 管线合计 1.23/4；Pass1 63–73%，Pass2 26–38%；每行 ~48 PipeBarrier + 16 次 V/MTE2 全同步；完全重叠上限 ~2.16x）。tile 往返 8→6 仅省 0.16µs，tile 轴已两档证伪。

因此本批假设**只打流水重叠 / barrier 与同步削减 / 队列加深**，不做 tile、reduction、dtype、路径阈值、算术位置类改动。

信号判定两关（每轮都要过）：
1. 超出当轮空白对照噪声带（空白对照逐轮现测）；
2. 机制幅度核对——实测与预测同量级。幅度标定：单行 D=32768 FP32 ≈ 8.28µs/行，D 翻倍 +3.9~4.2µs/行（成本随元素线性）。探出带但幅度不匹配不算信号。

### 0.1 单位成本标定（2026-09-29 R31A V028 跨路线发现，幅度外推前必读）

| 项 | 实测单位成本 | 含义 |
|---|---|---|
| 链尾 `V_op → PipeBarrier → SetFlag` 的 barrier | **~0.01µs/个** | 非承重，可删（R31A V028 五次正确性全过、max_abs 逐位一致） |
| V 链**中部**的 PipeBarrier | 显著更贵 | R31A V024 删 5 个中部 barrier 得 −3.66%——5×0.01µs 解释不了，说明成本在**位置**不在数量 |
| 全同步（`SyncVToMTE2/MTE3` 等 SetFlag+WaitFlag 对） | ≠ barrier 成本 | 不可用 0.01µs 外推；H3 的量级须单独看待 |

推论：**删 barrier 类假设的幅度 = 个数 × 0.01µs（尾部）或按中部位置加权；先把单位成本代入再报预期区间**。H2/H4 的幅度区间已按此下调/标注（见各节）；H5（队列加深）不受影响。V017 实测 0.12–0.15µs/tile 是**排空延迟化**（同步/重叠）收益，不是 barrier 个数收益，两者不可互相外推。

已证伪/已占轴（不得重复）：
- R31B 历史：跨核 D-slice（V008）、deep-batch UB（V004/V005）、tile 收缩（V007）、MTE2 队列深度 on FP32 full-y（V009，-0.10）、LP 宽行管线本身（V011 核心）、FP16/BF16 tile 4096→8192（V016）。
- STORE lane：H1 分块合并写回（store 粒度合并，V003 进行中）。
- TILING lane：tile 轴两档证伪；已定位 case 14 主缺口。
- R31A：H3 路径阈值（+4.33% 退化）、H1 Rsqrt（精度失守）、H1 纯 V/S 往返节省（噪声内）、H2 invRms Muls 提出 tile 循环（算术位置轴，进行中）。
- EPILOGUE-ARITH：NORM-HOIST（V001 进行中）。
- 跨路线红线：`AscendC::Rsqrt`/`Reciprocal` 为 ~2^-10 近似，凡分母/归一化改动禁止使用（本批假设均不触碰 invRms 计算）；宽 FP32 invRms 非确定 ~0.8%（`kWideFullYReduceStride=16` 尾槽问题）仍在——本批不修该问题（reduction 轴，不归本 lane），宽 FP32 测量时 same-binary 资格从严。

## 1. 假设总表

| ID | 一句话 | 轴 | 预期量级（D=32768 FP16/BF16 行·批） | 优先级 |
|---|---|---|---|---|
| H1 | pass-2 store 排空延迟化（2-deep MTE3 事件队列） | store 排空 / 流水重叠 | **已落地 V017**：实测 −0.64~−0.82µs(fp16-wide) / −2.0µs(bf16-wide)，两关 PASS | 完成 |
| H2 | pass-1 retained-y 直写，删除 Muls(1.0) 拷贝 | Pass1 算子+中部 barrier 削减 | 0.3~1.0µs（§0.1 上修） | 1 |
| H3 | 删除冗余 V→MTE2 全同步（store 前 / inter-pass） | 全同步削减 | 0.2~0.8µs（全同步成本待标定） | 2 |
| H4 | pass-2 BF16 重复 widen 块删除（2 Cast + 1 PB/tile） | 死算子削减（barrier 贡献 ~0.01µs 可忽略） | 0.1~0.4µs | 3 |
| H5 | pass-1 MTE2 staging 2-deep→3-deep | 队列加深 | 0.2~0.6µs（有 UB 代价） | 4 |
| H6 | pass-2 store 前 `V_op→PB→SetFlag` 冗余 barrier 删除 | barrier 清账/标定 | 0.06~0.13µs（标定用途） | 5 |

共同 EXPECTED_SHAPES（变更域 = LP 宽行，触发条件 `rowWidth > kCacheElems(8192)`）：

- 主测：`rows=2 FP16 D=12288/16384/32768`、`rows=2 BF16 D=12288/16384/32768`
- 空白对照（预期 |delta| 落在当轮噪声带内）：`rows=2 FP16/BF16 D=8192`（非宽路径，代码不变）、`rows=2 FP32 D=8192/16384`（本批不动 FP32 宽路径）

---

## H1 — pass-2 store 排空延迟化（2-deep MTE3 事件队列）

**MECHANISM**：`ProcessWideLowPrecision` pass-2 输出循环对每个 (tile, batchRow) 执行 `PipeBarrier<PIPE_V>; SyncVToMTE2(); SyncVToMTE3(); Store(); SyncMTE3ToV();`——每笔 store 后立刻全排空 MTE3，使 store 与下一 (tile,row) 的向量计算完全串行。改为 V006 在 `ProcessWideFp32FullCacheRows` 已验证的 2-deep 事件队列（`V_MTE3` ready / `MTE3_V` release 两对 event ID，store 只在 staging 被下一次写入前才 WaitFlag），store of unit i 与 compute of unit i+1 重叠。

**BOTTLENECK**：Pass2 26–38% 时间内的 store 串行段；msprof mte3 管线分仅 0.17，store 指令与 V 计算不重叠。每行 8 笔 store（D=32768、tile=8192、rows=2 → 4 tile × 2 row）逐笔排空。

**WHY_IT_MAY_HELP**：消除每笔 store 的 MTE3_V 排空等待，mte3 与 V 重叠窗打开；Pass2 内 store 段（估 0.7–1.5µs/行批）可被下一行/下 tile 计算掩盖。完全重叠上限 2.16x 里属于「加双缓冲 / 加深流水」类手段（MAIN-1 明确优先）。

**WHY_IT_MAY_FAIL**：outputLocal（outputBuf_）每 (tile,row) 复用——若 release 事件覆盖不足会读写冲突；store 流水若已被硬件隐式重叠，收益小于预测幅度；单行 batch（batchRows=1）时行间无计算可掩盖 store，只剩 tile 间重叠，收益减半。

**EXPECTED_SHAPES**：主测 FP16/BF16 rows=2 D=12288/16384/32768；D=32768 幅度最大（8 store/批）；rows=1 或非宽 D 无变化。

**WHY_NOT_DUPLICATE**：
- V006 的 MTE3 队列只落在 `ProcessWideFp32FullCacheRows`（FP32 宽 full-y）与 generic store；LP 宽行 pass-2（FP16/BF16）的 store 循环从未做过（本 kernel 内两处代码并存，可直接对照）。
- STORE lane H1 是**合并 store 粒度**（分块合并写回，笔数变少）；本假设**笔数与 payload 不变**，只把排空点从每笔推迟到 staging 复用前。机制不同（store 流水 vs store 粒度），轴不撞。
- 非 V009（那是 FP32 full-y 的 MTE2 输入队列，且失败）；非 V011（那是 pass-1/pass-2 的 MTE2 双缓冲，输入侧）；非 V016（tile 宽度）。
- R31A V021 类 store 排空推迟在其 R31A CachedRows 路径做过（本地混杂）；本假设是 R31B LP 路径 + 事件队列形式，不共享代码与判据。

**MINIMAL_OFAT_DIFF**：仅改 `ProcessWideLowPrecision` pass-2 输出段：把 `SyncVToMTE2(); SyncVToMTE3(); Store(); SyncMTE3ToV();` 替换为 `SetFlag<V_MTE3>(ready); WaitFlag<V_MTE3>(ready); Store(); SetFlag<MTE3_V>(rel);` 两对 event ID 轮转（照抄 V006 模式），并在 outputBuf_ 被下一次 FromFloat 写入前 `WaitFlag<MTE3_V>(rel)`。不改任何算子、tile、store 地址与 payload。

**ASCEND_FEASIBILITY**：高——同一 kernel 的 `ProcessWideFp32FullCacheRows` 已有可编译可运行的同型实现（storeReady0/1, storeRelease0/1）。

**UB/DMA/SYNC_IMPACT**：UB 零增量（仅事件 ID）；每 (tile,row) 减少 1 次 MTE3_V 全排空 + 1 次 V_MTE2 全同步；mte3 管线分 0.17 → 目标 >0.5；DMA 笔数不变。

**PRECISION_RISK**：无（不触碰任何算术）。

**幅度核对基准**：预测 Pass2 内 store 串行段下降，行·批 0.7–1.5µs（D=32768）。实测若 <0.3µs 视为幅度不匹配；若与 H4 类死算子收益同量级需警惕混淆。

---

## H2 — pass-1 retained-y 直写，删除 Muls(1.0) 拷贝

**MECHANISM**：FP16 pass-1 每 unit 当前为 `Add(xLocal, xLocal, residualLocal)` → PB → `Muls(yTile, xLocal, half(1.0f))`（把 y 拷入 retained 存储）→ PB → `ToFloat(valueFp32, xLocal)` → …。改为 `Add(yTile, xLocal, residualLocal)` 把加法结果**直接写入 retained y**，后续 `ToFloat(valueFp32, yTile)`——删掉整条 Muls 拷贝与紧随的 1 个 PipeBarrier。

**BOTTLENECK**：Pass1（63–73%）每 unit 多 1 个整 tile 向量算子 + 1 个 PipeBarrier；每行约 48 PB 里占 1/5 量级（D=32768、tile=4096 时 8 unit/行 → 8 op + 8 PB）。

**WHY_IT_MAY_HELP**：Pass1 是时间主体，unit 流水里少一段 V 指令与一次 barrier，MTE2 双缓冲（V011）的重叠窗相对变宽；barrier 计数 48→40/行量级，直接服务「同步串行化管线」主缺口。

**WHY_IT_MAY_FAIL**：Pass1 若被 MTE2 延迟而非 V 吞吐卡住，删 V 指令的收益被掩盖（幅度核对会暴露这一点）；yTile 位于 gammaBuf_（retained 区），Add 目标换址后与 ToFloat 源同址，RAW 链不变长但需确认 Cast 源读时序。

**EXPECTED_SHAPES**：FP16 rows=2 D=12288/16384/32768（仅 FP16 pass-1 有该拷贝；BF16 pass-1 已是直写 valueTile）。BF16/FP32/非宽路径预期 |delta|≈0，作对照。

**WHY_NOT_DUPLICATE**：
- V002 是 full-y 缓存**架构**（保留 y 的决定）；本假设不改变保留什么，只把「先算进 xLocal 再拷贝」改为「直接算进 retained 槽」，删 1 op+1 PB。
- V011 是 pass-1/pass-2 的 MTE2 输入双缓冲（DMA 侧）；本假设动 V 侧算子序列，不碰 DMA。
- V016 是 tile 宽度；REDUCE-HIER-X 三档已证 reduction 不是大头且本假设不碰 ReduceSum。
- R31A H2（invRms Muls 提出 tile 循环）是 pass-2 算术位置；本假设是 pass-1 数据流布局，不同函数不同 pass。
- 不在已试清单（V001–V016 无 op 拷贝消除类改动）。

**MINIMAL_OFAT_DIFF**：`ProcessWideLowPrecision` FP16 pass-1 三行：`Add(xLocal,…)`→`Add(yTile, xLocal, residualLocal)`；删除 `Muls(yTile, xLocal, half(1.0f))` 及其后 PB；`ToFloat(valueFp32, xLocal)`→`ToFloat(valueFp32, yTile)`。其余（Mul 方、ReduceSum、事件）逐字不动。

**ASCEND_FEASIBILITY**：高——Add/ToFloat 支持任意 LocalTensor 槽位；数值逐位相同（half 加法结果原样保留，Muls×1.0 为精确拷贝）。

**UB/DMA/SYNC_IMPACT**：UB/DMA 零变化；每 unit 减 1 V 指令 + 1 PipeBarrier（**中部** barrier，位置贵，见 §0.1）。

**PRECISION_RISK**：无——half 加法结果逐位一致，retained y 位型不变。

**幅度核对基准**：按 §0.1 重估——收益主体是**整 tile Muls 拷贝算子**（6–13 个/行批 × 约 0.05–0.1µs）+ 1 个中部 barrier/unit（比 0.01µs 贵，参照 V024 五删 −3.66% 折算约 0.07µs 量级），合计预测 **0.3–1.0µs/行批**（较原 0.2–0.7µs 上修，因删的是中部 barrier 且含整 tile 算子）。小于 0.1µs 判幅度不匹配。

---

## H3 — 删除冗余 V→MTE2 全同步

**MECHANISM**：LP 宽行路径存在两处**冗余** `SyncVToMTE2()`（SetFlag/WaitFlag 全管道 round）：
(a) pass-2 每 (tile,row) store 前的 `SyncVToMTE2()`——store 走 MTE3 读 outputBuf_，与 MTE2 无数据依赖；param staging（xBuf_/residualBuf_）的复用冲突已由 V011 的 `V_MTE2` 释放事件（prel0/prel1）覆盖；
(b) pass-1 与 pass-2 之间的 `SyncVToMTE2()`——pass-1 的 xBuf_ 释放已由 rel0/rel1 drain 覆盖，invRms 尾段只写 xFp32Buf_/residualFp32Buf_/reduceFp32Buf_，不写 T 型 staging。
删除后由既有事件承担正确性顺序。

**BOTTLENECK**：每行 16 次 V/MTE2 全同步之一大半来自 (a)；每次全同步把 V 与 MTE2 拍平，直接贡献「同步串行化管线」。

**WHY_IT_MAY_HELP**：每 (tile,row) 少一次全管道 round；D=32768、rows=2、tile=8192 时 8 次/批；给 V011 的 MTE2 预取让出重叠窗。

**WHY_IT_MAY_FAIL**：(a) 若在某些编译序里还兼作 V 写 outputBuf_ 的顺序保证（V_MTE3 sync 紧随，应无此职责）；删除后若 same-binary 偶发读写冲突则说明事件覆盖不完整，回退。

**EXPECTED_SHAPES**：FP16/BF16 rows=2 D=12288/16384/32768；(b) 批内生效（batch 均存在）；非宽/FP32 对照 ≈0。

**WHY_NOT_DUPLICATE**：
- 与 H1 同文件同 pass 但机制不同：H1 改的是 **MTE3 排空点**（store 流水），本假设删的是 **V→MTE2 冗余全同步**（同步计数）。一次只做一个。
- R31A「纯 V/S 往返节省」已证伪——那是 V↔S 标量往返；本假设是 V↔MTE2 全同步，不同管道对，不属同一轴。
- V009 是加 MTE2 队列深度（反方向且 FP32 站点）；本假设不动队列。
- 不在 V001–V016 已试清单。

**MINIMAL_OFAT_DIFF**：删除 pass-2 内 `SyncVToMTE2();`（store 前，每 (tile,row) 一处）与 pass-1→pass-2 交界 `SyncVToMTE2();`（一处）；保留 `SyncVToMTE3();` 与所有 event ID 逻辑不动。

**ASCEND_FEASIBILITY**：高——纯删除同步调用；编译期即可验证 API 用法。

**UB/DMA/SYNC_IMPACT**：UB/DMA 零变化；每 (tile,row) 减 1 次全同步（批内 8 次 + 交界 1 次）。**注意**：`SyncVToMTE2()` 是 SetFlag+WaitFlag 全同步对，成本不适用 §0.1 的 0.01µs/barrier 标定，须按全同步单独标定后外推（V028 只标定了 barrier）。

**PRECISION_RISK**：无。

**幅度核对基准**：预测 0.2–0.8µs/行批（全同步成本待标定，区间保留）。若 >1.5µs 反而可疑（说明删掉的不只是冗余）。

---

## H4 — pass-2 BF16 重复 widen 块删除

**MECHANISM**：`ProcessWideLowPrecision` pass-2 内连续两个**完全相同**的 `if constexpr (!std::is_same<T, half>::value) { ToFloat(xFp32, gammaLocal, valid); ToFloat(residualFp32, biasLocal, valid); PipeBarrier<PIPE_V>(); }` 块（V016 submission.asc 约 3302–3315 行；V011 同构）。第二个块是死工作：重复 Cast + 重复 PB。删除第二个块。

**BOTTLENECK**：BF16 宽行 pass-2 每 tile 多 2 个整 tile Cast + 1 个 PipeBarrier；D=32768、tile=8192 → 4 tile/行 → 8 Cast + 4 PB/行。

**WHY_IT_MAY_HELP**：直接削 barrier 计数与 V 发射量，零风险；与 H1/H3 同向叠加（但一次只测一个）。

**WHY_IT_MAY_FAIL**：幅度小（0.1–0.4µs），可能落在当轮噪声带内——这正是要用两关判定的情形；若幅度 <0.1µs 记为「方向一致但不构成信号」。

**EXPECTED_SHAPES**：仅 BF16 rows=2 D=12288/16384/32768（`!is_same<T,half>` 且非 float 的分支）；FP16/FP32 预期逐位零变化，作对照。

**WHY_NOT_DUPLICATE**：死算子消除，V001–V016 与全部活跃 lane（STORE/TILING/R31A/EPILOGUE-ARITH）均无此站点改动；不涉 tile/reduction/store 粒度/算术位置/队列深度任一轴。

**MINIMAL_OFAT_DIFF**：删除 pass-2 中第二个重复 `if constexpr` widen 块（3 行 + 1 PB），保留第一个。

**ASCEND_FEASIBILITY**：高——删除后与删除前输出位型一致（同参数同轮次 Cast）。

**UB/DMA/SYNC_IMPACT**：零 UB/DMA 变化；每 tile 减 2 Cast + 1 PB。按 §0.1，PB 部分仅 ~0.01µs，收益主体是 **2 个整 tile Cast 算子**（重复 widen 的真实算力浪费）。

**PRECISION_RISK**：无（重复计算删除）。

**幅度核对基准**：0.1–0.4µs/行批（Cast 主导，barrier 贡献可忽略）；低于 0.05µs 判为噪声内。

---

## H5 — pass-1 MTE2 staging 2-deep → 3-deep

**MECHANISM**：V011 的 pass-1 (row,tile) 流是 2-deep MTE2 双缓冲（rd0/rd1, rel0/rel1）。计算一个 unit 时下一 unit 在载入；若单 unit 计算时间 < 载入延迟，MTE2 会断流（mte2 管线分 0.44、V 0.46，二者接近）。把 staging 加深到 3-deep（xBuf_/residualBuf_ 各 3 slot，事件 3 组），载入窗口覆盖 u+1 与 u+2，掩长延迟。

**BOTTLENECK**：Pass1 的 MTE2/V 重叠窗只有 1 unit 深；宽行 tile 数多（D=32768 8 tile）时断流反复出现。

**WHY_IT_MAY_HELP**：MAIN-1 明确「扩大 MTE2/MTE3 queue depth」是主战场手段之一；V011 的 2-deep 已在**本路径本站点**证明有效（+1.25 Official），加深是同轴同站的下一步。

**WHY_IT_MAY_FAIL**：UB 代价——xBuf_/residualBuf_ 各 +1 tile slot（FP16 tile=4096 时 +16KB），`ChooseWideFullYRows` 的 ioTiles 计入后可能把 wideFullYRows_ 从 2 压到 1，批内行数变少反而净亏（COEFF-LOCALITY-X H2 死于 UB 挤占，教训在前）。必须先做 UB 预算核对再写码；若预算不成立则本假设降级为 INFEASIBLE。

**EXPECTED_SHAPES**：rows=1 且 D=16384/32768（UB 有余量的宽行）；rows=2 场景取决于预算核对结论。非宽/FP32 对照 ≈0。

**WHY_NOT_DUPLICATE**：
- V009 是 MTE2 队列深度 on **FP32 full-y** 站点（不同函数、不同 UB 预算结构）且失败；V011 在 LP 站点做 2-deep 成功——本假设是 LP 站点 2→3，与 V009 的站点/深度都不同，属未试组合。
- 不碰 tile 宽度（V016）、不碰 store（H1）、不碰算子（H2/H4）。

**MINIMAL_OFAT_DIFF**：`Init()` 内 FP16/BF16 宽分支 xBuf_/residualBuf_ 尺寸 ×3/2，`ChooseWideFullYRows` 调用处 ioTiles 4→6，pass-1 事件组 rd/rel 2→3 轮转。不改任何算子与 tile 逻辑。

**ASCEND_FEASIBILITY**：中——事件轮转模式与 2-deep 同构，编译可行；风险在 UB 预算（见上）。

**UB/DMA/SYNC_IMPACT**：UB +2 tile slot（FP16 tile=4096 时 +16KB / BF16 同）；DMA 笔数不变、单笔不变；sync 计数不变。

**PRECISION_RISK**：无。

**幅度核对基准**：0.2–0.6µs/行批（仅在断流被掩盖的部分）；若 rows 2→1 则预期为**负**收益，幅度核对直接判负。

---

## H6 — pass-2 store 前 `V_op → PipeBarrier → SetFlag` 冗余 barrier 删除（R31A V028 模式）

**MECHANISM**：pass-2 每 (tile,row) 的 store 前奏是 `Add(outputLocal…) → PipeBarrier<PIPE_V> → SyncVToMTE2() → SyncVToMTE3() → Store`。`SyncVToMTE2/SyncVToMTE3` 自身以 SetFlag/WaitFlag 承担 V→MTE 顺序（R31A V028 实测：SetFlag 前的 barrier 非承重，删后正确性 5 次全过、max_abs 逐位一致）。删除该 PipeBarrier 一个，不动同步对。V017 落地后该前奏只余这一处 `V_op→PB→SetFlag` 模式（outputLocal 写入是最后 V op）。

**BOTTLENECK**：barrier 计数每行 ~48 个之一；按 §0.1 标定单个仅 ~0.01µs，总量 = (tileCount×batchRows) × 0.01µs ≈ 0.06–0.13µs/行批。

**WHY_IT_MAY_HELP**：零风险（V028 已五次正确性验证同模式）；逐位一致的算子删减。

**WHY_IT_MAY_FAIL**：幅度极小，很可能落在噪声带内——本轮定义为**标定/清账 Revision**：若实测也≈0.01µs/个则完成本 lane 的 barrier 单位成本标定（信息量价值），不以收益论成败。

**EXPECTED_SHAPES**：FP16/BF16 wide 全部；对照非宽（无该模式）。

**WHY_NOT_DUPLICATE**：R31A V028 是其自身函数的验证；本假设在 R31B pass-2 站点独立落刀（不同函数不同 lane）。与 H3 不同：H3 删整个 SyncVToMTE2 全同步（V→MTE2 顺序归属存疑），H6 只删同步对**前面的** barrier（V028 已证非承重）。与 H1 已落的排空延迟化正交（H1 动 MTE3_V 等待点，H6 动 V→MTE 同步对之前的 barrier）。

**MINIMAL_OFAT_DIFF**：`ProcessWideLowPrecision` pass-2 store 前奏删除一个 `AscendC::PipeBarrier<PIPE_V>();`（紧邻 `SyncVToMTE2()` 之前）。其余逐字不动。

**ASCEND_FEASIBILITY**：高——V028 同模式已验证。

**UB/DMA/SYNC_IMPACT**：零 UB/DMA；每 (tile,row) 减 1 barrier（~0.01µs/个）。

**PRECISION_RISK**：无（V028 max_abs 逐位一致）。

**幅度核对基准**：0.06–0.13µs/行批；用途为标定而非收益。

---

## 2. 实现顺序建议（待 Main 选定）

1. **H2**（pass-1 直写，中部 barrier + 整 tile 算子，V017 后幅度最大的干净项）
2. H3（同步削减；与 H6 二选一或先后独立——H6 幅度小但零风险，可作标定 Revision）
3. H4（BF16 死 widen 块，零风险中等幅度）
4. H5（MTE2 3-deep，先 UB 预算核对）
5. H6（barrier 标定/清账，幅度 ~0.1µs 级）

实施纪律：一个 Revision 只落一个假设（H1 与 H3 互斥已过；H3 与 H6 亦不得合并）；每个 Revision 先写硬性声明；空白对照每轮现测；两关判定写入 local-result.json；幅度外推先用 §0.1 单位成本。
