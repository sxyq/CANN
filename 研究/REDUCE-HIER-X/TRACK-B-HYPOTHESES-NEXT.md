# TRACK-B HYPOTHESES NEXT — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
MODE=TRACK-B research only — 不改 Kernel、不建 V004、不跑正式 performance、不提交 Online
PARENT_LINEAGE=V001 / V002 / V003 直接父版均为 FROZEN_R31B_V011（SOURCE_SHA a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3，Official 45.16）
OFFICIAL_ANCHOR=45.16
ROUTE_OFFICIAL_BEST=V003 44.24（submission 6abab2bb694b590c3c442da4）

---

## 0. 本文范围与门槛

本轮 Planning 授权继续在 RMS reduction topology 轴上找剩余空间，但新假设必须携带
**新瓶颈证据**或**新结构信息**，且不得重跑已证伪的三种形态：

| 已证伪形态 | 对应 Revision | 证据 | 结论 |
|---|---|---|---|
| fold：tile partial eager fold（V001 H1，partial-sum lifetime） | V001 | 8x8192 -3.8%（n=2 clean），其余形状 same-binary FAIL | 近中性，collapse 不是主要成本 |
| tree：手写 pairwise Add 树替代 per-tile ReduceSum（V002 H2） | V002 | 1x32768 +6.6% favP 5/0，8x8192 +0.9% | LOCAL_REJECTED；根因：per-level PipeBarrier 成本 > 省下的 ReduceSum V/S |
| short-span：barrier-free 折短到 kReduceSpan=128 再 ReduceSum（V003 H3） | V003 | 1x32768 -0.5% 中性，8x8192 -3.3% 近噪声，1x16384 -1.5%（父版 same-binary FAIL） | 中性；短 span 不带来预期大胜 |

允许的单变量轴（本路线）：reduction structure / span / fold strategy 三者之一。
禁止同时动：dtype、store、DMA、tiling、scheduling、mode selection、rows-per-block、
sync-removal（GetValue 次数）、invRms 数学序列（归 VECTOR-MATH-X）。

本轮所有假设挂在同一个合格起点上：**DIRECT_PARENT 建议 = FROZEN_R31B_V011**。
理由：本路线尚无 LOCAL_ACCEPTED；V003 本地 NEEDS_ONE_MORE_LOCAL 且 Official 44.24
低于 anchor 45.16，不构成可累积 Local Best，按 `执行约定.md` B 节应回到该路线已确认的
合格起点。是否采纳由 Main-2 定。

---

## 1. V003 闭环判断

**结论：V003 已闭环。**

| 项 | 记录 |
|---|---|
| 本地结论 | `NEEDS_ONE_MORE_LOCAL`（合法枚举，不是 NOT_COMPLETE） |
| 编译 / 链接 / 正确性 | PASS / PASS / PASS（FP32 父版预存失败形状已标注 PARENT_PREEXISTING） |
| 源码身份 | local == remote == sidecar（79f910e4…），formalResultEligible=true |
| 线上终局 | submission 6abab2bb694b590c3c442da4，15/15 Pass，Official 44.24，SUBMITTED_PASS |
| 路线处置 | registry 记 ROUTE_OFFICIAL_BEST + 「KEEP as small-win accumulator；no more reduction-topology revisions until new bottleneck evidence」；pivot 记录在案 |
| 校准 | 已写（direction_match=PARTIAL） |

V003 的 local-result 里曾建议「one more local round（re-qualify 1x16384）」，那是
online_worthy 字段里的备选，不是未完义务；线上已提交且 H3 已定性为中性，该建议已被
终局处置取代。**无待收口的 V003 工作。**

残留事实（不是待办）：1x16384 FP32 的父版 same-binary FAIL（drift 0.392），该形状只有
方向数据（-1.5%，4/0），不能作为强结论。

---

## 2. 新瓶颈证据与新结构信息（本轮新增的判断依据）

### 2.1 新结构信息（相对 V001–V003 立项时）

1. **无中间 barrier 的 V-pipe Add 链是安全的**（V003 实测：FoldToReduceSpan 的 pairwise
   Add 无 per-level PipeBarrier，正确性全过，无对齐异常）。V002 的失败根因是 barrier 链，
   不是 Add 树本身。→ 任何依赖「多次 Add、零 barrier」的新结构不再继承 V002 的失败模式。
2. **被加数是平方项（全部 ≥ 0）**，结合顺序变化不引入相消误差。归约关联顺序的精度风险
   显著低于一般求和重排。
3. **V/S 记账（V002 研究记录）**：multi-tile 路径每行归约侧 V/S ≈ tileCount + 2
   （tileCount = 2/4/8 对应 D=8192/16384/32768）。per-tile ReduceSum 是随 tileCount
   增长的项；collapse 只占 1 次。V001 只去掉了 collapse（弱信号），真正未被攻击的
   是 per-tile 调用次数。
4. **整行单段归约的可行替代**：REDUCE-INVSCALE-X 的 OPT-4（whole-row single-stage）
   在 FP32 被 UB 算术判死（需 D×4 B 放整行平方）。本路线的 binned accumulator
   （假设 N1）只需 W×4 B（W=128/256），是同一野心下的 UB 可行形态。
5. **kTileElems=4096 是加载宽与归约宽共用常量**；reduceFp32Buf_ = 16 KB 而活跃 partial
   槽只有 tileCount ≤ 8 个。R31B 的 UB 结构与 FULL-R006 谱系不同，REDUCE-INVSCALE 的
   6144-tile 算术不能直接搬用，但逐元素成本模型的方法可用。

### 2.2 新瓶颈证据（来自已有书面分析，非新测量）

`研究/OFFICIAL-WEAK-CASE-STRATEGY.md` 对 R31B-V011 弱 case 的结构估计（未上机验证）：

| case 类 | 代表 case | 归约尾部 + handoff 估计占比 | 对本路线的含义 |
|---|---|---|---|
| 短行多行（D ≤ 256，1 tile） | 7（3.74x）、6、4 | 非 DMA 时间的 35–45% | single-tile 路径的 per-row ReduceSum 调用/handoff 是本路线可动部分 |
| 中等行 | 8 | ~30% | 混合 |
| 宽行大算力 | 14（4.40x） | ~20% | large-D 归约份额有限，与三次 large-D 失败一致 |

关键点：V001–V003 的主探针都在 large-D multi-tile（1x32768 / 8x8192 / 1x16384），
**短行多行带（case 7 类）从未做过主探针**。large-D 三次证伪不能自动外推到该带。
其中 invRms/GetValue 尾部归 VECTOR-MATH-X；本路线只动归约调用的组织方式。

### 2.3 证据缺口（见第 4 节：缺什么、如何只读补齐）

---

## 3. 假设清单（4 条）

### N1 — Binned streaming single-stage square-sum（W 元向量累加器 + 单次收尾 ReduceSum）

- **MECHANISM**
  重构归约结构：multi-tile 路径上取消「per-tile ReduceSum → partial bank → collapse
  ReduceSum」两级结构，改为单级。每个 tile 的平方向量按固定宽度 W（建议 128 或 256，
  32B 对齐）分块 `Add` 进 W 元 FP32 累加器（bin j 收所有 i≡j (mod W) 的平方），Add 链
  零中间 PipeBarrier（V003 已证安全）；行尾一次 `ReduceSum(acc, work, W)` + 原有
  GetValue 链。per-tile 的 Mul 平方、mean/epsilon 尾部、第二遍输出全不动。
- **BOTTLENECK**
  per-tile `ReduceSum` 的调用次数（随 tileCount 线性增长的 V/S 项）与 partial bank 的
  两级写读。single-stage 后每行归约侧 V/S 从 tileCount+2 降到 3（1 次收尾 ReduceSum +
  2 次 GetValue），与 D 无关。
- **EXPECTED_SHAPES**
  multi-tile FP32：D=8192（tileCount=2）→ 1x16384（4）→ 1x32768（8），收益随 tileCount
  增长；宽路径 ProcessWideFp32FullCacheRows 同理。短行单 tile 带（D≤4096）无收益
  （本来只有 1 次 ReduceSum），只作负对照。
- **WHY_IT_MAY_HELP**
  (1) 直接去掉随 tileCount 增长的调用项，是 V001（只去 collapse）没碰的那根杠杆；
  (2) 零 barrier 的 Add 链避开 V002 的失败根因；(3) 平方非负，关联顺序风险低；
  (4) UB 只多 W 个 float（0.5–1 KB），可直接用 reduceFp32Buf_（16 KB，活跃 partial 槽
  本来只有 ≤8 个）承载，不挤占 tile 预算（吸取 COEFF-LOCALITY-X V001 的教训）。
- **WHY_IT_MAY_FAIL**
  (1) 大 D 带三次证伪后归约份额可能确实只剩 ~20%，改动落在噪声内；
  (2) 累加器每个 bin 有跨 tile 的串行依赖链，Add 总数约 D 次，与 V002 的树同量级，
  若 V-pipe 吞吐是限速项则不会更快；(3) 8x8192 的 V001 弱信号（-3.8%）可能本就是
  collapse 贡献，而非 per-tile 调用；(4) 1x32768 主探针历史上最不出信号。
- **ASCEND_FEASIBILITY**
  高。只用 Add / ReduceSum / PipeBarrier / SyncVToS，全部现网在用；无新 API；
  W 需 8 的倍数保证 32B 对齐（与 V003 kReduceSpan 同约束）。
- **UB/CORE/DMA_IMPACT**
  UB：W floats（W=128 → 512 B），可从 reduceFp32Buf_ 划出，**不新建大 buffer**；
  CORE：不动行归属/核数；DMA：加载与存储完全不动。
- **SYNC_IMPACT**
  每行 SyncVToS/GetValue 次数不变（2 次）；去掉 per-tile ReduceSum 与 collapse 的调用
  本身（若其内部含 V/S，则随调用消失）。行尾保留 1 次收尾 ReduceSum 前的
  PipeBarrier。**不改 GetValue 数量**（sync-removal 不在本路线）。
- **PRECISION_RISK**
  低。平方非负；binned 结果只差 ULP 级；golden 为 double 参考 + 既有 dtype 容差。
  仍须过完整正确性矩阵（含非对齐 D=100/65 尾块）。
- **DUPLICATE_CHECK**
  - ≠ V001 fold：V001 保留 per-tile ReduceSum、折叠的是 1 元 partial；本条取消
    per-tile ReduceSum、折叠的是原始平方向量到 W 元累加器，结构上是「两级→单级」。
  - ≠ V002 tree：V002 是 per-tile 二叉树 + per-level barrier；本条无树、无 barrier、
    跨 tile 分箱累加。V002 失败根因不适用。
  - ≠ V003 short-span：V003 保留 per-tile ReduceSum 并缩短 span；本条不缩短 span，
    而是取消 per-tile 调用。方向相反（结构，不是 span 微调）。
  - ≠ REDUCE-INVSCALE-X OPT-4 whole-row：那是「整行平方一次进 ReduceSum」（FP32 需
    D×4 B，已判不可行）；本条是「W 元累加器代替整行缓冲」的可行形态，span 只有 W。
  - ≠ UB-LIVENESS-X 的向量归约想法：那是另一 kernel 的 pass1 死字节处置，父系与
    实现归属不同；本条只在 R31B-V011 谱系的归约拓扑内。
  - 不碰 REDUCE-INVSCALE-X H1/H2（invRms 尾部）与 H3（加宽 tile）。
- **MINIMAL_OFAT_DIFF**
  只改「平方如何变成行 squareSum」这一件事：删 per-tile ReduceSum 与 collapse
  ReduceSum，加分块 Add 进 W 累加器 + 1 次收尾 ReduceSum。先只落 S1/S2/S3 三个
  multi-tile 站点（Process / ProcessFp32FullRowOutputPipelined /
  ProcessWideFp32FullCacheRows），S4/S5 留作对照。Mul、mean/epsilon、invRms、
  输出遍、调度、dtype、DMA 不动。
- **EXPECTED_LOCAL_PROBES**
  主探针：1x32768 FP32（tileCount=8，调用项最大）、1x16384 FP32（4）、8x8192 FP32
  （2）；负对照：1x4096 FP32（单 tile，预期 Δ≈0）。先补 1x16384 的父版 same-binary
  资格（V003 在该形状父版 FAIL）。判据预注册：若主探针配对中位数未稳定超过该形状
  噪声带，则 NEEDS_ONE_MORE_LOCAL / 归约份额确认过小，不再续归约拓扑。
- **CLASSIFICATION**: **READY_FOR_MAIN_REVIEW**（唯一推荐立项；先验偏保守，见 handoff）

---

### N2 — Group-span ReduceSum（G 个 tile 的平方拼接后一次 ReduceSum）

- **MECHANISM**
  只改 span 与 fold 分组：保持两级结构和 `ReduceSum` API 不变，把连续 G 个 tile 的
  平方拼进同一块连续 UB 区域（G=2 → span 8192），一次 `ReduceSum` 产 G 个 tile 的
  共同 partial，行尾 collapse 的源变成 tileCount/G 个 partial。per-tile Mul、调用拓扑
  层级、GetValue 不动。
- **BOTTLENECK**
  per-tile `ReduceSum` 调用次数（与 N1 同一根杠杆），但只按因子 G 削减，不消除。
- **EXPECTED_SHAPES**
  G=2：D=8192（2→1）、16384（4→2）、32768（8→4）；V003 在 8x8192（-3.3%）与
  1x16384（-1.5%）有弱方向信号，恰是 G=2 让调用数减半的带。
- **WHY_IT_MAY_HELP**
  改动比 N1 小（不动拓扑层级，只动 span 与分组）；与 short-span（V003）方向相反，
  是未试过的 span 端点。
- **WHY_IT_MAY_FAIL**
  (1) 需要 G*tileElems 连续平方区（G=2 → 32 KB）：xFp32 与 residualFp32 是两块不连续
  buffer，要么新建大 buffer 要么借 valueFp32Buf_（8192 float），后者在 cacheRow 路径
  被 y 缓存占用 —— 有挤占 tile 预算风险（COEFF V001 教训）；(2) large-D 归约份额已
  三次证伪，G 倍削减未必出噪声；(3) 本质仍是「少几次 ReduceSum」，上限低于 N1。
- **ASCEND_FEASIBILITY**
  中高。API 现成；难点在 UB 拼接区的来源与别名生命周期。
- **UB/CORE/DMA_IMPACT**
  UB：最坏需额外 32 KB（G=2）或复用 valueFp32（有别名风险）；CORE/DMA 不动。
  禁止用缩小 tileElems 的方式腾 UB（会变成第二个变量）。
- **SYNC_IMPACT**
  调用次数减少；每行 GetValue/Sync 数量不变。
- **PRECISION_RISK**
  低（平方非负，只换求和关联与分组）。
- **DUPLICATE_CHECK**
  - ≠ V003 short-span：span 加宽 vs 折短，方向相反。
  - ≠ REDUCE-INVSCALE-X H3：那是 kReduceTileElems 6144→6656 的 tile 加宽，信号带只在
    D∈(30720, 32768]；本条不动 tileElems、按 G 分组拼接，在 R31B 的 4096-tile 结构上
    每个 multi-tile D 都改变调用数。效应带与 UB 算术都不同。
  - ≠ N1：N1 取消 per-tile 调用；本条保留 ReduceSum 只减少调用数。
- **MINIMAL_OFAT_DIFF**
  只改 span/分组：平方拼接 + 一次 ReduceSum(G*valid)，partial 槽从 tileCount 变
  tileCount/G。若必须动 buffer 尺寸才能拼接，把 buffer 改动并入同一变量
  （reduction workspace）并在声明里写明，不动 wideFullYRows_ 的选择输入。
- **EXPECTED_LOCAL_PROBES**
  1x8192 FP32（G=2 调用 2→1）、1x16384、1x32768；负对照 1x4096。
- **CLASSIFICATION**: **NEEDS_MORE_EVIDENCE**（先出逐路径 UB 算术，确认拼接区来源
  且不动 tile 预算，再谈立项）

---

### N3 — 内建向量输出归约原语替换（换 primitive，不改 span、不手写树）

- **MECHANISM**
  在 per-tile 站点把 `AscendC::ReduceSum` 换成 Ascend C 内建的块级/整段归约变体
  （`BlockReduceSum` / `WholeReduceSum` 一类，具体以 CANN 8.5.0 头文件为准），产出
  向量侧 partial、不做内部 V/S 交接；span 仍等于 valid，两级拓扑、partial bank、
  collapse、GetValue 全不动。变体若只能产出多元素结果，再用既有 collapse 结构收
  （同一假设内的实现细节，不引入第二个性能变量）。
- **BOTTLENECK**
  `ReduceSum` 调用内部的 V/S 交接（V002 研究记录把它记为每次调用固定成本）。
  目标带是短行多行：case 7 类里「归约尾部 + handoff」占非 DMA 时间 35–45%。
- **EXPECTED_SHAPES**
  短行多行带优先：rows>1 × D≤4096（S4 batched / S5 mid）；multi-tile 宽行的收益
  与调用数成正比（弱）。
- **WHY_IT_MAY_HELP**
  这是本路线唯一同时对准「短行 handoff」与「large-D 调用数」的 primitive 级变量；
  large-D 三次证伪改的是拓扑/折叠/span，没换过内建 primitive。
- **WHY_IT_MAY_FAIL**
  (1) 「ReduceSum 内部有 V/S」目前是源码注释与研究记录里的假设，未对着 CANN 8.5.0
  实现确认；若内建变体同样有交接或没有向量输出形式，此条直接落空；
  (2) 若 handoff 主要来自 GetValue/SyncVToS（归 VECTOR-MATH 轴），换 primitive 无效；
  (3) BlockReduceSum 的 repeat/stride 契约弄错会先打正确性。
- **ASCEND_FEASIBILITY**
  待确认——本地无 ASC_DEVKIT，API 变体契约必须先只读核对（见第 4 节 E3）。
  确认后可行性高（纯 API 替换）。
- **UB/CORE/DMA_IMPACT**
  UB：可能需要该 API 指定的 work 区（现用 xFp32/residualFp32 顶替，需核对契约）；
  CORE/DMA 不动。
- **SYNC_IMPACT**
  目标是减少调用内部交接；GetValue/SyncVToS 次数不动。
- **PRECISION_RISK**
  低–中：内建归约的内部结合顺序可能与 ReduceSum 不同，平方非负故容差应可过；
  必须按 dtype 跑完整正确性矩阵。
- **DUPLICATE_CHECK**
  - ≠ V002 tree：V002 是手写 Add 树（barrier 链失败）；本条是内建指令/内建 API，
    不手写、不引入 barrier 链。
  - ≠ V001/V003：不改 fold 时机、不改 span。
  - ≠ UB-LIVENESS-X 的「把死 work 区改给 ReduceSum 用」：那是该路线的字节处置；
    本条只换 primitive。
  - ≠ VECTOR-MATH-X：不动 invRms/GetValue 序列。
- **MINIMAL_OFAT_DIFF**
  只换归约 primitive 调用（同站点、同 span、同两级拓扑）；先只落 S1 一个站点，
  其余站点后续再扩。
- **EXPECTED_LOCAL_PROBES**
  短行带：8x256 FP32、32x256 FP32（batched S4）、1x1024 / 1x4096；large-D 对照
  1x32768。预注册否证：若在 D=4096 单 tile 上 primitive 不比 ReduceSum 快，则
  handoff 假设在本工具链不成立。
- **CLASSIFICATION**: **NEEDS_MORE_EVIDENCE**（E3 的 API 契约只读核对是前置；核对
  后若确认无交接优势 → INFEASIBLE）

---

### N4 — Batched 多行同时归约（R007：多行平方一次归约出多行 partial）

- **MECHANISM**
  只改归约在行间的组织方式：S4 batched 路径（4 或 8 行已经同批驻留 UB）现在每行一次
  `ReduceSum`；改为一次多行归约（如带 repeat/stride 的块归约，或每行一个 W 元累加器
  + 一次收尾归约），每批从 rows 次调用降到 1 次。行的加载、存储、dtype、行归属不动。
- **BOTTLENECK**
  短行多行带的 per-row 归约调用与 handoff（case 7 类估计 35–45% 非 DMA 时间）。
- **EXPECTED_SHAPES**
  batched 形状：4x256 / 8x256 / 32x256 / 8x1024 一带；单行形状无收益（负对照）。
- **WHY_IT_MAY_HELP**
  idea-pool R007「multi-row simultaneous ReduceSum」仍标未覆盖（owner REDUCE-X）；
  V001–V003 主探针全部是大 D，短行多行带没有归约拓扑实验。
- **WHY_IT_MAY_FAIL**
  (1) case 7 的形状类别是从耗时量级推断的，未证实；若短行带主要是 launch/Init
  （case 1/3 估计 ~70% 固定开销），归约侧改动无效；(2) 多行 stride 归约契约易错；
  (3) 需确认 S4 路径的 row 布局是否允许跨行 stride 归约（否则退化为 N1）。
- **ASCEND_FEASIBILITY**
  中。取决于是否有支持 repeat/stride 的内建归约；若无，则需 N1 的多行分箱变体
  （结构上可做，但与 N1 的差距变小）。
- **UB/CORE/DMA_IMPACT**
  UB：每行 W 元累加器（4 行 × 128 × 4 B = 2 KB）或一次 stride 归约的工作区；
  CORE：不动；DMA：不动（禁止借机改 multi-row DMA）。
- **SYNC_IMPACT**
  每批 GetValue 次数可从 rows 次降到 rows 次（仍逐行取值）或合并同步后再取——
  后者会碰 sync-removal 边界，**本假设只减归约调用数，不减 GetValue 次数**。
- **PRECISION_RISK**
  低（同行求和关联变化；跨行互不影响）。
- **DUPLICATE_CHECK**
  - ≠ BATCH-RESIDENT-X：那是多行 DMA / 参数驻留；本条不动数据搬运。
  - ≠ SCHED-CHAMPION-X / SCHED-ROWGROUP-X：不动行归属、核数、rowGroup。
  - ≠ N1：N1 是单行内单级化；本条是行间归约调用合并。
  - ≠ idea-pool R007 的另一半「reduce/DMA overlap」：那是 DMA 时序，本路线禁止；本条
    只取「多行同时归约」这一半，且 R007 至今无实现。
- **MINIMAL_OFAT_DIFF**
  只改 batched 路径的归约调用组织（每行一次 → 每批一次）；不动单行路径。
- **EXPECTED_LOCAL_PROBES**
  32x256 FP32（batched）、8x256 FP32、8x1024；负对照 1x256。
- **CLASSIFICATION**: **NEEDS_MORE_EVIDENCE**（需先只读确认 S4 行布局与 stride 归约
  可行性，并确认 case 7 形状推断）

---

## 4. 瓶颈证据缺口与只读补齐方案

历史门槛是「no more reduction-topology revisions until new bottleneck evidence」。
本轮拿到的是**书面结构估计**（weak-case 百分比是推断值），不是上机分解。缺口如下。

| 编号 | 缺什么 | 只读补齐方法 | 补齐后解锁 |
|---|---|---|---|
| E1 | 父版第一遍时间分解：V/S 交接 vs 向量算术 vs MTE2 等待 vs 第二遍，各占多少 | 读 `本地实验/REDUCE-HIER-X/V001..V003/support/` 的 raw samples 与配对统计，用 V002（+6.6%，barrier 重）与 V001（-3.8%，去 collapse）的 delta 给归约份额上下界；不新跑测量 | N1/N2 的预期收益上限 |
| E2 | 每路径每行的算子清单（ReduceSum 次数、PipeBarrier、SyncVToS/SToV、MTE2/MTE3） | 读 FROZEN_R31B_V011 与 V003 源码，按 S1–S5 路径 × 形状类列 op-count 表 | 全部四条假设的机理量级 |
| E3 | `ReduceSum` 是否真有内部 V/S；`BlockReduceSum`/`WholeReduceSum` 等变体是否存在、契约与输出形态 | 读 CANN 8.5.0 API 文档/头文件（本地无 ASC_DEVKIT，在线文档或服务器工具链头文件只读） | N3 能否立项（否则 N3 → INFEASIBLE） |
| E4 | Official case 的真实 shape/dtype 映射 | 读 `研究/OFFICIAL-CASE-ANALYSIS.md`、`线上结果/**/result.json` 比对耗时量级；仍属推断则标注未证实 | N4 的形状带是否打中 |
| E5 | 真实 profile（msprof：V/S、MTE、pipe 占用） | **只读补不了**。需 Main-2 授权一次「只跑父版、不改 Kernel」的 profiling 会话 | 历史门槛里说的 new bottleneck evidence 的本体 |

建议顺序：E2/E3 当场可做（纯读）；E1 从既有 raw 数据回算；E5 留给 Main-2 决定是否
用一次父版 profiling 换更强的立项依据。若 Main-2 接受「新结构信息」（第 2.1 节）
作为立项依据，N1 可不等 E5；若坚持「新瓶颈证据」字面含义，则先 E5 再 N1。

---

## 5. 排序建议

| 序 | 假设 | 分类 | 一句话理由 |
|---|---|---|---|
| 1 | N1 binned single-stage | READY_FOR_MAIN_REVIEW | 唯一带新结构信息、直接去掉 tileCount 项、UB 代价最小、避开 V002 失败根因 |
| 2 | N3 内建 primitive 替换 | NEEDS_MORE_EVIDENCE | 对准短行 handoff 带，但依赖 E3 契约核对 |
| 3 | N2 group-span | NEEDS_MORE_EVIDENCE | span 端点未试，但 UB 拼接与 large-D 先验都不利 |
| 4 | N4 多行同时归约 | NEEDS_MORE_EVIDENCE | 短行带结构未证实，R007 半边未覆盖 |

若 N1 也落在噪声内，则归约拓扑轴在 R31B 谱系的剩余空间应视为基本耗尽，建议向
Planning 报告该事实（是否 PARK 由 Planning 决定）。

---

## 6. 本文件遵守的边界

- 未改任何 Kernel、未建 V004、未跑 performance、未提交 Online。
- 未动共享调度文件、未动 DTYPE-SPECIAL-X / SCHED-CHAMPION-X / UB-LIVENESS-X 实现。
- 未改技术路线/路线成绩表.tsv、全版本记录.tsv（本轮只产研究文件，记录同步待 Main
  审阅后按 `实验总则.md` M 节执行）。
