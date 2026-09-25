# UB-LIVENESS-X — Next Hypotheses (TRACK-B research)

> **MAIN-2 APPROVALS 2026-09-25** — APPROVED NEXT backlog: H1 TRUE_LIVE_SET_BUDGET (dead-reservation reclaim + live-set model fix + unlock larger effective tile); V004 implementation FORBIDDEN until Judge returns and Main issues NEXT_HYPOTHESIS. Scope: this route keeps only buffer lifetime / aliasing / peak UB footprint / live-set budgeting — H2→BATCH, H3→ASYNC, H5→REDUCE (all marked in-file). JUDGE_OWNER=MAIN-1.

ROUTE: UB-LIVENESS-X · WORKTREE: cann-next6/UB-LIVENESS-X · OWNER: UB-LIVENESS-X Route Agent (MAIN-2)
APPEND-ONLY file. Sources: V003 `phase4/local/UB-LIVENESS-X/V003/submission.asc` (SHA 2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd, byte-identical to worktree `phase4/workspaces/UB-LIVENESS-X/submission.asc`, verified by SHA256).

## CURRENT_CANDIDATE

- V003, ONLINE_CANDIDATE, JUDGE_READY=YES, JUDGE_OWNER_REQUIRED (control 中尚无具名 Judge Owner)。
- 内容：correctness-only 修订；phase-role UB alias 架构保持不变（`UB_LIVENESS_ALIAS=1`：pass2 emit 借用 x-slot 字节）。
- 本地探针：alias1 vs alias0 中位差 −1.96%，4 对中仅 1 对 alias1 更快，方向不一致 → alias 杠杆本身未测出性能差。
- 8/8 battery + alias0 control 正确性 PASS bad=0；SINGLE_CHANGE_AUDIT=PASS。

## CURRENT_BLOCKER

- 等待统一 Judge Owner 对 V003 的正式在线结果（MAIN-2 不自提交）。
- **在 Judge 返回之前，不开始任何 V004 实现：无内核修改、无新修订、无设备运行。** TRACK-B 仅做只读研究。
- 设备侧：五条 timing 路线 WINDOW UNQUALIFIED 2/2，当日窗口预算用尽（协议文档），本路线不排计时。

## BOTTLENECK_MODEL

对 V003 源码的瓶颈排序（按证据类型标注）：

1. **标量软件循环（源码事实，疑为首要）**：FuseU FP32 路径逐元素 `SetValue/GetValue` 加法（submission.asc:341）、pass1 SumSq 逐元素标量平方累加（:446-449）、pass2 emit 逐元素标量公式（:489-491）、narrow-dtype 逐元素量化循环（:354-364）。每行每 tile 至少 3 轮 O(tile) 标量工作，且 pass1+pass2 各一遍 → 约 3×dim 级标量操作/行。Vector 管线只承担 Cast/Add。
2. **同步串行化（源码事实）**：无 TQue；每个 CopyInPad 后强制 `PipeBarrier<PIPE_MTE2>`（:93，注释称 FetchEventID(MTE2_V) 在此工具链错配）；narrow fuse 每 tile 6 次 `PipeBarrier<PIPE_V>`；另有 SyncVToS / SyncSToV / MTE2_V / MTE3 每 tile 约 11–12 次同步操作（pass2）。MTE2/V/MTE3 完全顺序发射，重叠为零。dim=32768、tile=2048 时每行约 32 个 tile 往返 ×(pass1≈6 + pass2≈12) ≈ 数百次同步。
3. **参数 GM 流量（源码事实）**：`Run()` 强制 `fullParam_=false`（:181），gamma/bias 在 pass2 每 (行×tile) 重新 DMA（:486 LoadParamChunk）。dim=4096、tile=2048 时每行额外 2 tile × 2 参数 × 8KB = 32KB 参数读；乘以行数线性放大。`LoadFullParams`（:287）与 `EmitAt`（:393）已定义但从未被调用（源码 grep 证实）。
4. **输入重复读（源码事实）**：pass1 与 pass2 各自重新 CopyIn x 和 res（:425-426 与 :466-467）→ 每行 2×(x+res) GM 读。
5. **tile 上限 2048（源码事实）**：`tileChoices[]={2048,1024,...}`（:205）——宽 D 时往返数由选择表封顶，而非 UB 容量（见下表：dim=32768 fp32 实际预留仅 90176B，预算余 98240B）。
6. **UB 预算模型与实际不一致（源码事实，本研究核心发现）**：`EstBytes` alias 路径按 4 slot 建模（:195），但 `PlanAndAlloc` 无条件 `InitBuffer(bufOut_)`（:259）→ 实际 6 slot。alias=1 运行时 `out0_/out1_` 被重绑到 x-slot（:281-284），bufOut_ 字节全部闲置但仍然占位 → **alias 杠杆当前实际节省 0 字节**，alias1 与 alias0 的 UB 占用完全相同。这直接解释了本地探针 alias1 vs alias0 无方向性差异。

硬件参照：DAV_2201 UB = 192KB（`ascendc-tiling-design` 技能资料）；内核自设 `kUbBudgetBytes = 184KB`（:45），留 8KB 余量。以下所有尺寸按 `Align32`（32B 对齐）计算。

---

## UB_LIFETIME_MAP

对象：V003 submission.asc（alias=1，depth2=true 是 `EstBytes` 首选配置下的实际取值）。示例列取 fp32 / dim=4096 / tile=2048 / 计划期 fullParam=true（`EstBytes(true,true,alias)=90112 ≤ 184K` 命中）/ 运行期 `Run()` 强制 fullParam=false。

| # | Buffer (TBuf) | Views (LocalTensor) | Size 公式 | 示例尺寸 (fp32 d4096 t2048) | Birth（指令/阶段） | Last use（指令/阶段） | 与其他 buffer 的重叠 / alias 边 | Alias 可能性 |
|---|---|---|---|---|---|---|---|---|
| 1 | bufParam_ | gammaF_=[0,gElems)，biasF_=[gElems,…)（gElems 按计划期 fullParam 取 dim 或 tile） | full 时 2·Align32(4d)；chunk 时 2·tileF | 预留 32768；实际只用 [0,8192) gamma 块 + [16384,24576) bias 块 = 16384（[8192,16384) 洞 8192 + [24576,32768) 尾 8192 闲置） | 分配：Init/PlanAndAlloc :254；首次填充：pass2 首 tile LoadParamChunk :486（pass1 全程零活跃） | pass2 末 tile 标量 emit 读取 :490 | 与其他 TBuf 无重叠；内部 gamma/bias 按 gElems 分区但计划/运行配置错配产生洞；潜在边 E6：死函数 LoadFullParams 借 x1_ 做 staging（:291），与 bufX_ 冲突——这正是 `Run()` :181 强制 fullParam=false 的注释原因 | 可升级为全量参数驻留（H2）；错配洞可消除 |
| 2 | bufTmp_ | reduceWork_ | 固定 8192（kReduceWorkBytes，EstBytes 亦按此预留 :193） | 8192 | 分配 :255 | **从未使用**：ReduceSum 仅 import（:28）未调用；pass1 归约是标量 acc（:446-449） | 无 | 死预留 → 回收（H1）或改为向量归约工作区（H5） |
| 3 | bufSum_ | reduceDst_（注释称 [0]=running sum… :266） | 固定 64 | 64 | 分配 :256 | **从未使用**（同上） | 无 | 死预留 → 回收（H1） |
| 4 | bufX_ | x0_, x1_（x1_ = x0_[slot 元素步长]，depth2 时独立半区） | nX·Align32(bufTile·e)，nX=depth2?2:1；bufTile=max(tile,512) :244 | 16384（2×8192） | pass1 首 tile CopyIn `xs` :425 | pass2：fuse 读 :471/:474 →（alias=1）作为 outStage 被 FromFloatSeq 覆写 :494 → CopyOut :495（末行末 tile） | **已实现 alias 边 E1**：`out0_/out1_ := x0_/x1_`（:281-284），INGEST→EMIT 跨阶段共用；同 tile 内 x 在 fuse 后即死，覆写安全；死边 E6：LoadFullParams 借 x1_（未调用） | 已实现：emit 借 ingest 字节（但见 #6——当前未省任何预留） |
| 5 | bufRes_ | res0_, res1_ | nR·Align32(bufTile·e) | 16384（2×8192） | pass1 首 tile CopyIn `rs` :426 | pass2 fuse 读取（:471/:478）；narrow-dtype 参数 staging 复用 res0_（:324，仅 pass2 fuse 之后调用） | 潜在边 E5：LoadParamChunk 借 res0_ 作 T-typed DMA staging——安全性依赖调用顺序（fuse 先消费 rs），源码注释 :311-313 承认该脆弱性 | 可做参数 staging 常驻或队列深度槽 |
| 6 | bufOut_ | out0_, out1_ | nX·Align32(bufTile·e)（与 bufX_ 同尺寸） | 16384 预留；alias=1 时 **字节完全未触碰**（句柄在 :281-284 被重绑到 x-slot，但 InitBuffer :259 无条件占位） | 分配 :259 | alias=1：**从未使用**；alias=0 对照：pass2 :494 填充、:495 写出 | 与 bufX_ 是"逻辑或"关系（alias=1 只用其一），物理上两者都预留 → alias 的节省从未兑现 | 回收即兑现 alias 承诺（H1 核心） |
| 7 | bufScratch_ | formF_=[0,formCap)，mulF_=[formCap,…)；formCap=max(tileF,2048) | Align32(2·max(tileF,2048)) | 16384（2×8192） | pass1 fuse 首次写 formF_（fp32 Add :430 / narrow Cast :433）；mulF_ 在 narrow fuse（:435-）与 emit 中启用 | pass2 emit 标量读 formF_ :490；FromFloatSeq 源 mulF_ :494 | 内部 formF_/mulF_ 分区无重叠；**同一物理区跨 pass、跨 tile 反复复用——已是现状中唯一的真正 phase-role 复用**；u（formF_ 内容）pass1 用完即弃，不跨 pass 保留 | u 跨 pass 驻留 = H4；向量归约工作区 = H5 |
| 8 | bufAnchor_, bufPool_ | （仅声明 :502） | 0 | — | 从未 InitBuffer | — | 无 | 遗留声明，可清理（随 H1） |

### 活跃区间（lifetime interval）示意

```text
Init/PlanAndAlloc ──┬─ bufParam_ ───────────(pass1 死区)──────────┬─ pass2 每 tile 填充/读取 ─ 末 tile
                    ├─ bufTmp_ / bufSum_ ──────────── 全程死（无 Birth 语义）─────────── ×
                    ├─ bufX_  ── pass1 每 tile 读写 ── pass2 每 tile 读 → emit 覆写(行末) ─┘
                    ├─ bufRes_ ─ pass1 每 tile 读写 ── pass2 fuse 读 → (narrow) staging 复用 ─┘
                    ├─ bufOut_ ─────────────── alias=1 全程死 ×
                    └─ bufScratch_ ── pass1 fuse/SumSq 每 tile 复用 ── pass2 fuse/emit 每 tile 复用 ─┘
```

### 峰值与足迹汇总（本地图的量化输出）

| 配置 | 实际 InitBuffer 预留合计 | 真实同时活跃峰值 | 可证死预留 | 相对 184KB 预算余量 | EstBytes(alias) 模型值 | 模型缺口 |
|---|---:|---:|---:|---:|---:|---:|
| fp32 d=4096 t=2048（计划 full） | 106560 (104.1K) | ~49216 (xs+rs+form+mul+gamma+bias) | 41024（tmp 8192 + sum 64 + out 16384 + param 洞/尾 16384） | 81856 | 90112 | +16448（out+sum 未建模） |
| fp16 d=4096 t=2048 | 90112 (88.0K) | ~41216（slot 减半，param/scratch/reduce 不变） | 32832 | 98304 | 同构 | 同类缺口 |
| fp32 d=32768 t=2048（计划 chunk） | 90176 (88.1K) | ~49216 | 24640（tmp+sum+out） | 98240 | 73728 量级 | 同类缺口 |

要点：
- **峰值活跃仅约 26–27% 预算；预留 57%；死预留 40KB 级（fp32 计划 full 配置）**。
- alias=1 相对 alias=0 的预留差 = 0（bufOut_ 无条件分配）→ V003 本地 alias 对照无差异是结构性的，不是测量噪声。
- **模型缺口方向**：EstBytes 系统性低估（漏掉 alias 模式下的 bufOut_ 与 bufSum_）。tile=2048 时缺口 16448B；若直接在现有模型上放行更大 tile（如 fp32 t=4096：模型 139264 但实际将达 ~172096），接近 184K 模型上限的配置实际可能越过 192KB 硬件 UB → 放大 tile 前必须先对齐模型。
- dim≥2048 的宽形状下，**tile=2048 是选择表封顶而非 UB 封顶**（d=32768 预留仅 88KB）。
- 输入生命周期：x/res 每行被 GM 读 2 遍（pass1+pass2）；gamma/bias 每 (行×tile) 读一遍 pass2（本应一次驻留）；输出仅 pass2 写 1 遍。

（假设待设备侧确认项：DAV_2201 UB=192KB 取自本地 tiling 技能资料，未在 dav-2201 编译产物上实测边界。）

---

## HYPOTHESIS-1 — TRUE_LIVE_SET_BUDGET（死预留回收 + 模型对齐 + 解封更大 tile）

Classification: **READY_FOR_MAIN_REVIEW**（作为 V004 首选候选；仍须等 Judge 返回 V003 后由 Main 决定是否立项。当前不实现。）

- **MECHANISM**：一个概念变更——"tile 选择只按真实活跃集计费"。具体联动：(a) 删除 bufTmp_(8192)+bufSum_(64) 的死 `InitBuffer`（ReduceSum 未调用，源码证实）；(b) alias=1 时不再分配 bufOut_（兑现 :281-284 重绑从未兑现的占位节省，或干脆去掉 alias0/1 分叉、emit 恒借 x-slot）；(c) `EstBytes` 改为精确复刻 `PlanAndAlloc` 的实际分配清单（含 bufSum、alias 下不含 out、param 按计划期配置计）；(d) 在对齐后的模型上把 `tileChoices` 上限从 2048 提到能通过预算的最大档（4096/8192）。(a)(b)(c) 是 (d) 的前置，四步共同实现同一性能机制：真实活跃集计费 → 更大 tile → 每行 tile 往返与同步次数减半。
- **BOTTLENECK**：瓶颈模型第 2、5 条——每 tile 约 11–12 次同步 + 每 tile 固定开销的标量/向量段；dim=32768 时 2×16=32 个往返/行被 tile 表封顶，而 UB 实际只用了 88KB/184KB。
- **EXPECTED_SHAPES**：宽 D（≥8192）收益最大：往返数 ∝ dim/tile，tile 翻倍 → 往返与每 tile 固定同步减半；fp32 收益 > fp16（fp16 每 tile 6 次 V barrier 也随 tile 数减半，但单 tile 标量工作量不变，摊薄有限）；窄 D（dim≤2048，tile 已=dim）无变化；正确性全 dtype 不变。足迹形态：fp32 d4096 预留 106560 → 约 65520（-41040），模型缺口 16448 → 0。
- **WHY_IT_MAY_HELP**：死预留 40KB 级 + 模型缺口 16KB 级是源码可证的纯浪费；释放后宽 D 的 tile 从 2048 提到 4096（d32768 fp32 实际预留 ~172096 ≤ 192KB 可行），每行同步次数近似减半；大 tile 还提升向量段效率（FuseU/Add 的 n 更大、per-tile 固定开销摊薄）。
- **WHY_IT_MAY_FAIL**：(1) V003 alias 对照已证明"占位优化本身不加速"——若 Judge 后发现主瓶颈是标量循环（BOTTLENECK 第 1 条），tile 翻倍只是把慢循环次数减半而每次变长，收益被 O(dim) 标量工作吃掉（标量总量不变，仅固定开销减半）；(2) 大 tile 下 DataCopyPad 单次 byteLen 翻倍，非对齐尾块 pad 行为需重验；(3) plan-full 参数洞在 tile 变化后形态变化，gElems 分区需同步修正；(4) 本地探针噪声（V003 对照 median −1.96%、worst −43%）可能淹没 <5% 的真实差异。
- **ASCEND_FEASIBILITY**：高。全部是 TPipe InitBuffer 与常量选择改动，无新 API、无 ISA 假设；tileChoices 加档是普通整数逻辑；192KB 边界在估算中显式按实际分配清单核算（DAV_2201 UB=192KB，本地技能资料）。风险点仅在接近 184K 的配置要按实际值而非旧模型复核。
- **UB/CORE/DMA_IMPACT**：UB：预留↓ 40KB 级、模型缺口归零、宽 D tile↑；CORE：不变（行并行分块不动）；DMA：单次 DataCopyPad 字节数↑（tile 翻倍），总 GM 流量不变（仍 2 遍读 x/res + 参数重读——参数流量不归本假设解决），拷贝次数减半。
- **SYNC_IMPACT**：每行同步操作数近似减半（11–12/tile × tile 数）；不引入新同步原语；同步语义不变（仍是 PipeBarrier 系）。
- **PRECISION_RISK**：无。数学路径、dtype 转换、量化点、inv 计算全部不动；尾块 pad 语义需在正确性 battery（8 形状 + alias0 control）上重验，属验证而非风险源。
- **DUPLICATE_CHECK**：
  - vs idea-pool R005（大 tile）：R005 是"直接放大 tile 字节"，从未处理峰值活跃集；本假设的杠杆是"死预留+模型缺口让大 tile 在同一物理 UB 内成立"，且带模型修正这一前置。R005 当前 owner MID-X/WIDE-X 均已 PARKED。
  - vs R013/R030：R013 加深度增加峰值占用；R030 只缩减专用角色工作集以躲 `ub address out of bounds`——都不以"分配清单与活跃集对齐"为机制。
  - vs 本路线 V003：V003 的 alias 是"借字节"但未省预留；本假设是首次让 alias 的字节承诺兑现并进入 tile 决策，属 V003 架构的直接下一步而非重复。
  - 活跃路线：无一做预算模型/tile 封顶（SCHED=行组调度、ALIGN=尾块、REDUCE=归约、BATCH=参数驻留+批量 DMA、ASYNC=MTE3 重叠）。WHY_NOT_DUPLICATE 成立。
- **MINIMAL_OFAT_DIFF**：相对 V003（单一概念"按真实活跃集计费"）：删 2 个死 InitBuffer；alias 时删 bufOut_ InitBuffer；EstBytes 重写为实际分配清单的镜像；tileChoices 增加 4096/8192 档。不得夹带同步改动、参数驻留、u 驻留或标量→向量改写。
- **EXPECTED_LOCAL_PROBES**：同协议（device events, warmup≥10, in-process, 交错 P/C）：d=32768 / d=16384 fp32 为主探针（往返减半的形状），d=4096 对照（预期≈中性），fp16 d=16384 次探针；预期方向：宽 D 负 delta（更快），幅度取决于固定开销占比——若标量循环主导则仅 3–8%，若同步主导则 10%+。d≤2048 形状预期零差（tile 无变化），可作为空白对照。分类前提：V004 立项须在 Judge 返回且 Main 发出 NEXT_HYPOTHESIS 之后。

---

## HYPOTHESIS-2 — PARAM_RESIDENCE_VIA_COLD_X_STAGING（冷 x-slot staging 实现参数驻留）

Classification: **DUPLICATE · DEFER_TO_BATCH（MAIN-2 仲裁 2026-09-25：parameter staging / gamma-bias residency / row-batch parameter reuse 固定归 BATCH-RESIDENT-X；UB 不继续实现该方向，本条保留为分析记录）**——与活跃路线 BATCH-RESIDENT-X（"Parameter residency + multi-row batch DMA"）主题重叠。

- **MECHANISM**：在首行首 tile 之前调用现有但从未执行的 `LoadFullParams`（:287）：此时 x1_ 尚未被任何 CopyIn 触碰（冷），借其做 T-typed staging 把全量 gamma/bias DMA+Cast 进 bufParam_（计划期 full 配置已能按 184K 通过，如 fp32 dim≤17408），随后整个 kernel 生命周期零参数 DMA；同时删除 `Run()` :181 的强制 `fullParam_=false`。pass2 emit 改读 `gammaF_[off]` 分段（:397 已有该分支）。窄于预算的 D 用分段驻留（一次驻留覆盖多行），超预算 D 退回现状。
- **BOTTLENECK**：瓶颈模型第 3 条——gamma/bias 每 (行×tile) 重读。dim=4096、1000 行、1 核（示意）：现状每核 1000×2 tile×2×8KB = 32MB 参数重读；驻留后 32KB 一次。
- **EXPECTED_SHAPES**：行数越多、D 越大参数流量占比越高 → 大 batch/宽 D 收益大；fp32 与 fp16 的参数流量相同（param 恒为 float 区）→ 两 dtype 同向；dim≤17408（fp32 计划 full 上限）单核驻留一次；超限形状退回 per-tile，零收益零损失。DMA 时间占比高的形状（小计算量/大参数）收益上限最高。
- **WHY_IT_MAY_HELP**：架构证据图为"parameter residency = PROVEN_WIN"（A001 FastKernel param queue、R31 param cache；R014 族历史正面）；本内核的参数重读是 correctness 修订（V003）留下的结构性遗留，不是刻意设计——`LoadFullParams` 已写好、注释已解释冲突原因，修复路径短。
- **WHY_IT_MAY_FAIL**：(1) pass2 的 LoadParamChunk 每 tile 还伴随 2 次 MTE2 barrier + MTE2_V（:319-322），驻留省的不只是字节还有同步——但也意味着省下后若标量 emit 仍主导则时间不敏感；(2) plan-full 参数洞（gamma/bias 间隔 dim 而非 tile）需要 emit 分段读正确性重验；(3) 冷 x1_ staging 依赖"首 CopyIn 前调用"这一顺序不变式，后续任何插入都可能悄悄破坏它（V002/V003 两次 correctness 事故都源于此类时序假设）；(4) 超预算 D 形状（在线测试 D 未知！）完全无收益。
- **ASCEND_FEASIBILITY**：高。全部已有代码路径（LoadFullParams/EmitAt 分段分支已存在）；无新 API；唯一改动是调用时机与去掉一个强制赋值 + emit 读法切换。
- **UB/CORE/DMA_IMPACT**：UB：bufParam_ 维持 plan-full 尺寸（无新增峰值，反而 pass2 期间 param 洞仍在——驻留不省 UB，是纯 DMA 收益）；CORE：不变；DMA：参数 GM 流量 O(rows×tiles) → O(1/core)。
- **SYNC_IMPACT**：每 tile 减 2×MTE2 barrier + MTE2_V + （narrow 时）两次 SyncMte2ToV；初始化期加一次性的全量加载同步。净同步数下降。
- **PRECISION_RISK**：无路径变化（同一 Cast CAST_NONE/CAST_RINT 序列，只是提前执行）；但 plan-full 的 gElems 分区使 bias 偏移从 tile 尺度变为 dim 尺度——读错分区是 correctness 风险而非精度风险，须过 8-shape battery。
- **DUPLICATE_CHECK**：
  - SOURCE_ROUTE: BATCH-RESIDENT-X（ACTIVE_EXPLORE, NEXT6, parent A001-V017）· MECHANISM: 参数驻留 + 多行批量 DMA · OLD_CONTEXT: A001/FastKernel 谱系上的参数队列 · CURRENT_CONTEXT: UB-LIVENESS-X 两遍扫描 fresh 内核，冷 x-slot staging · WHY_ORTHOGONAL: BATCH 走 A001 谱系且叠加多行 DMA；本条仅参数生命周期、不动 DMA 形状 · WHY_NOT_DUPLICATE: **弱**——"让 gamma/bias 驻留、消参数重读"的性能机制同一，实现载体不同不足以构成机制差异 → 按 DUPLICATE 标记。
  - vs idea-pool R014（covered, A001/H001/R31）：同一主题，历史已证 PROVEN_WIN。
  - vs WIDE-X（PARKED）R014 stripe-resident for D>UB：分段驻留方向一致，路线已停。
  - 结论：主题已被活跃路线占位；若 BATCH-RESIDENT-X 最终 PARK，本条可作为其在 fresh 内核上的再实现候选。
- **MINIMAL_OFAT_DIFF**（若 Main 立项）：去掉 :181 强制 fullParam=false + 首行前调用 LoadFullParams + emit 读法切到 fullParam 分支 + 恢复 LoadParamChunk 的 early-return 语义。不得同时改 tile、同步或 u 驻留。
- **EXPECTED_LOCAL_PROBES**：d=4096/8192 fp32 与 fp16（param 重读占比高的形状）、行数取 battery 中大行数形状；同协议交错 P/C；预期全 dtype 同向负 delta；若 d=32768（超预算）出现差异即为实现泄漏信号。

---

## HYPOTHESIS-3 — MTE2_INGEST_QUEUE（用腾出的 UB 建 MTE2 摄入队列，兑换搬运/计算重叠）

Classification: **DEFER_TO_ASYNC（MAIN-2 仲裁 2026-09-25：MTE2 ingress queue / prefetch scheduling / copy-compute-store pipeline 固定归 ASYNC-TRIPLE-X；UB 不实现本条，EnQue/DeQue 工具链证据可作为 donor 移交 ASYNC）**（源码 :92 注释记录过 FetchEventID(MTE2_V) 错配问题，移交时须一并注明。）

- **MECHANISM**：把 pass 内的裸 `DataCopyPad + PipeBarrier<PIPE_MTE2>` 摄入路径改为 TQue 双缓冲：x/res 各一个 depth-2 队列（`InitBuffer` num=2），tile k 的 V 计算与 tile k+1 的 MTE2 搬运用 EnQue/DeQue 配对重叠；配套把手写 `SyncMte2ToV` HardEvent 换成 DeQue 等待。深度所需的额外 slot 字节来自 H1 回收（或现有 slack：d=32768 预留 88KB，本就有 96KB 余量——即使不依赖 H1，2-slot 摄入已是现状 depth2_ 槽位，**缺的从来不是字节而是重叠调度**）。范围限定：只做 MTE2 摄入侧；MTE3 写出与 triple 不在本假设内。
- **BOTTLENECK**：瓶颈模型第 2 条——当前 MTE2 与 V 完全串行：CopyIn 两连发后 `PipeBarrier<PIPE_MTE2>` 阻塞，V 算完才发下一次 CopyIn；宽 D 每行 32 个串行往返，DMA 空转与 V 空等互相可见。
- **EXPECTED_SHAPES**：搬运占比高的形状（大 dim、fp32 大 slot、行数多）收益大；纯标量循环主导的形状收益小（重叠救不了 S 管线）；fp16 摄入字节减半 → 重叠收益上限减半；窄 D 无往返可重叠。方向性：宽 D 负 delta，窄 D ≈0。
- **WHY_IT_MAY_HELP**：(1) 架构证据图 pipeline overlap=MIXED 但 A001 V017 x/res 2-slot 有小胜、R31 MTE 深度 flat 是"T14 上单侧加深"，非本内核的"零重叠起点"——从 0 到 1 的增益与从 1 到 2 不同量级；(2) 本地技能资料明确 EnQue/DeQue 为推荐同步（自动硬件同步点，替代手写 PipeBarrier），且当前每 tile 11–12 次同步里多数可被队列语义吸收；(3) UB 侧 H1 已证有 40KB 级死字节 + 96KB slack，深度不需要牺牲 tile 或参数驻留。
- **WHY_IT_MAY_FAIL**：(1) 源码 :92 注释记录该工具链 `FetchEventID(MTE2_V)` back-to-back 错配——若 EnQue/DeQue 底层同样受累，同步错误会以 correctness 形式暴露（V002/V003 两次 correctness 事故均为同步次序类）；(2) 标量循环主导时重叠上限被 S 管线封顶（Amdahl）；(3) `PipeBarrier<PIPE_MTE2>` 是本内核目前唯一被证明安全的 MTE2 等待方式，换成队列属于"未验证假设"；(4) R31 双向证据：MTE depth flat on T14。
- **ASCEND_FEASIBILITY**：中。TQue/EnQue/DeDeque 是 Ascend C 标准 API（本地技能 api-pipeline.md 有完整范式），DAV_2201 可用；不确定性集中在本工具链的事件配对实现，需要一次编译级+正确性级的最小验证（非计时）。
- **UB/CORE/DMA_IMPACT**：UB：x/res 维持 2-slot（现状已分配），或 param 也入队（+tileF 字节，来自 H1 回收）；CORE：不变；DMA：MTE2 与 V 并行，GM 总字节不变，有效带宽利用率↑。
- **SYNC_IMPACT**：结构性减少——每 tile 的手动 barrier 组（MTE2 barrier、SyncMte2ToV、V_S 部分）被 DeQue/EnQue 吸收；发射序列改变，属于本假设的核心变更面（也是风险面）。
- **PRECISION_RISK**：无（纯调度；数据内容与顺序不变）。
- **DUPLICATE_CHECK**：
  - EXTERNAL_IDEA: TQue 双缓冲 EnQue/DeQue 流水线范式 · SOURCE: Ascend C 官方高性能模板与本地技能 api-pipeline 资料（00_introduction/01_add basic_api_memory_allocator_add 双缓冲+流水线标准实现）· PROVENANCE_CLASS: PUBLIC_OFFICIAL_API_PATTERN（概念与 API 用法参考，不复制任何代码）· WHY_DIFFERENT_FROM_EXISTING_29: R013 讲"加深度"，本假设讲"在零重叠起点上用队列语义替换 barrier 语义"，且范围裁剪为 MTE2 摄入单侧。
  - vs ASYNC-TRIPLE-X（ACTIVE_EXPLORE, MTE2/V/MTE3 triple overlap, 当前 IDLE/MEASUREMENT_BLOCKED）：其记录声明 SINGLE_CHANGE=only MTE3 store overlap、"no double-buffer-only"。本假设只做 MTE2 摄入 → 机制相邻但字节面不同（其 MTE3 写出 vs 本条 MTE2 读入）。**边界须由 Main 确认**，在确认前按部分重叠对待，不立项。
  - vs R013 owner WIDE-X/MID-X：均已 PARKED；A001 V017 小胜为 donor 记录：SOURCE_ROUTE: A001 · MECHANISM: x/res 2-slot double buffer 小胜 · OLD_CONTEXT: A001 谱系 · CURRENT_CONTEXT: fresh 两遍内核零重叠起点 · WHY_ORTHOGONAL: A001 已有部分流水，本内核是 0→1 · WHY_NOT_DUPLICATE: donor 是深度增量，本假设是同步范式替换。
  - vs REDUCE-INVSCALE-X / ALIGN-TAIL-X / SCHED-ROWGROUP-X：机制无关。
- **MINIMAL_OFAT_DIFF**（若证据补齐且 Main 立项）：仅 MTE2 摄入侧改 TQue depth-2 + EnQue/DeQue 配对，删对应手写 barrier；不动 MTE3、不动 tile、不动参数/u 驻留、不动数学。
- **EXPECTED_LOCAL_PROBES**：先 correctness battery（同步类改动必须先证对）；计时探针 d=16384/32768 fp32 为主，d=4096 对照；判定需同协议交错 P/C；前置实验：单点 EnQue/DeQue 配对在编译与 8-shape battery 上的通过性（不计时）。

---

## OPTIONAL-HYPOTHESIS-4 — U_RETENTION_CROSS_PASS（u 跨 pass 驻留，消 pass2 输入重读）

Classification: **NEEDS_MORE_EVIDENCE + 重复预警**——性能机制与 idea-pool R002/resident-u 及 champion R31A V016"input reread reduction PROVEN_WIN"同源；是否算作本路线可用候选须 Main 对照 R31A exploit 谱系裁定。当前不实现。

- **MECHANISM**：pass1 每 tile 的 u（formF_，float）在 SumSq 后不再弃掉，而是按行段累积进回收出的 UB 池（H1 释放 + 现有 slack）；pass2 直接从 UB 读 u，跳过 x/res 二次 CopyIn 与 fuse——pass2 只剩 emit。u 行段尺寸 = dim×4B(fp32)，fp16 也可用 float 存 u（golden 在量化点已固定）。生命周期上这是把 map 中"formF_ per-tile 死亡"延长为"跨 pass 行驻留"，bufScratch 从 scratch 升格为 resident 区。
- **BOTTLENECK**：瓶颈模型第 4 条——x/res GM 读 2 遍；pass2 的 CopyIn×2 + fuse（含 narrow 6 barrier）全部可省。dim=4096 fp32 每行省 32KB 读 + pass2 约 1/2 的 tile 往返工作。
- **EXPECTED_SHAPES**：收益 ∝ 行驻留可容纳的行数：fp32 dim=4096 每行 16KB → 池内可驻留 4–5 行/核（回收后 ~100KB 级）；dim=32768 每行 128KB → 仅 1 行勉强（收益形态突变）；dim≤2048 驻留 8+ 行，读消除占比最高。fp16 u=float 仍 4B/elem——与 fp32 同足迹（dtype 无关），但 fp16 原 fuse 往返更贵（6 barrier）→ fp16 收益更大。宽 D 反而难驻留 → 形态与 H1 相反（H1 利宽、本条利窄中）。
- **WHY_IT_MAY_HELP**：pass2 的存在理由本来只是"inv 算出后重新 fuse"——inv 是行级标量，u 可完全避开重读；省一半输入带宽 + pass2 fuse 同步链；证据图"input reread reduction = PROVEN_WIN"。
- **WHY_IT_MAY_FAIL**：(1) **重复性**——R002 idea-pool 标 covered（A001 full-u、R31B V002 full-y win、R31A V016 D=32768 full-y 45.00），机制同一；(2) 宽 D 驻留不下 → 双模式分支（驻留/不驻留）违反 OFAT 单机制要求或在宽 D 无收益；(3) u=float 使足迹 2×于输入 dtype——fp16 形状上"省 2×读"换"4× 存"，窄 D 才划算；(4) 行段驻留把 per-tile 复用变成 per-row 常驻，峰值活跃集从 ~49KB 涨到 49KB+K×dim×4——与 H1 的峰值收缩方向相反，两者不可同一修订叠加。
- **ASCEND_FEASIBILITY**：高（无新 API；只是 formF_ 内容不再被覆盖而是按行偏移保存 + pass2 读 UB）。池内布局需要按行分段的地址管理，属常规下标计算。
- **UB/CORE/DMA_IMPACT**：UB：活跃峰值显著上升（驻留区 ×K 行），与 H1 互斥于同一修订；CORE：不变；DMA：x/res 读减半，无新增写流量（u 在 UB 内）。
- **SYNC_IMPACT**：减少（pass2 的 CopyIn MTE2 barriers、SyncMte2ToV、fuse V barriers 全免）；不引入新同步。
- **PRECISION_RISK**：低但非零——u 以 float 保存跨 pass，pass2 不再重复"cast-add-quantize"fuse 序列。V003 的 correctness 核心恰是"pass1 acc 与 pass2 fuse u² 一致"（V003 修订动机）；驻留后 pass2 用的就是 pass1 的 u → 数学上更一致，golden 对比应更稳，但 narrow-dtype 量化点位置变化须重验 battery。
- **DUPLICATE_CHECK**：
  - SOURCE_ROUTE: R31A (V016 full-y / input reread reduction) · MECHANISM: 宽 D 保留 y/u 消二次读 · OLD_CONTEXT: champion 谱系 multimode 内核 · CURRENT_CONTEXT: fresh 两遍内核的 UB 池生命周期延长 · WHY_ORTHOGONAL: 载体与谱系不同（fresh UB 池 vs champion cache 路径） · WHY_NOT_DUPLICATE: **弱**——性能机制（消输入重读）同一，R31A 已在线兑现 45.00 → 按 DUPLICATE 预警。
  - SOURCE_ROUTE: idea-pool R002 · 状态: covered（G001 parked、A001 full-u、R31 full-y）。
  - 活跃 next6 路线无一做 u/y 驻留。
- **MINIMAL_OFAT_DIFF**（若 Main 清除重复性后立项）：仅 u 的跨 pass 保存与 pass2 读源切换；不得同时改 tile、同步、参数驻留。
- **EXPECTED_LOCAL_PROBES**：窄中形状 d=512/4096 fp32+fp16 为主（驻留行数≥4 的配置），d=32768 作"驻留不下"的预期零差对照；同协议交错 P/C；先行 battery 8 形状 + 额外 narrow 量化一致性核对。

---

## OPTIONAL-HYPOTHESIS-5 — UB_RESIDENT_VECTOR_REDUCTION（用死 8KB 归约区承接向量归约，替代标量 acc）

Classification: **DEFER_TO_REDUCE（MAIN-2 仲裁 2026-09-25：reduction topology / partial reduction / reduction temp organization 固定归 REDUCE-INVSCALE-X；UB 只保留 buffer lifetime、aliasing、peak UB footprint、live-set budgeting。本条的"死 8KB 转归约工作区"生命周期事实保留在 UB 证据中，归约实现归 REDUCE）**（疑似最大时间项，但计算拓扑改动超出"UB liveness"单一主题的边界。当前不实现。）

- **MECHANISM**：pass1 的 SumSq 标量循环（:446-449，逐元素 GetValue×dim）改为向量管线：u² 入 bufScratch/mulF_，用已死的 bufTmp_(8KB) 作 ReduceSum 工作区、bufSum_(64B) 作归约目标（这正是这两个 buffer 的设计用途，:266-267 注释仍在）；inv 侧继续标量 Newton。生命周期上：bufTmp_/bufSum_ 从"死预留"变为"pass1 活跃"，与 H1 的"删除"互斥——二者是同一死字节的两种用法。
- **BOTTLENECK**：瓶颈模型第 1 条——pass1 每 tile O(tile) 标量平方累加，dim 级全标量；FuseU 与 emit 的标量段仍在（不在本假设内），但 SumSq 是最长的纯标量读-乘-加环。
- **EXPECTED_SHAPES**：所有 dtype/dim 上 pass1 标量时间 ↓；dim 越大绝对收益越大；fp16 收益含"fuse 6 barrier 不变、仅归约段向量化"→ 收益 = 标量环占比；若 fuse/emit 标量环（第 1 条其余部分）主导，则本假设只砍掉约 1/3 的标量工作 → 上限受限。
- **WHY_IT_MAY_HELP**：证据图 reduction redesign = MIXED 但 H001 V007→V008 "+score via ReduceSum" 是正面点；死 8KB 本来就是给 ReduceSum 的，向量化不占新 UB；同时 pass1 的 SyncVToS（scalar 读 formF_ 前置）可被 reduce-dst 等待替代 → 同步也减。
- **WHY_IT_MAY_FAIL**：(1) REDUCE-X 三次修订失败的谱系风险（归约主题路线级负面）；(2) ReduceSum 的 tmp 尺寸/dtype 约束需按 8.5.0 文档核实（技能资料要求列全 API 变体逐一确认——本地无 ASC_DEVKIT，只能在线兜底，证据尚未取）；(3) 与 H1 方向冲突：H1 删这 8KB，本条留用——两假设在死字节处置上分叉，须先裁定 H1 的删除项保留还是转用；(4) 若标量总量里 fuse/emit 占大头（本条不碰），省下的只是局部。
- **ASCEND_FEASIBILITY**：中高——ReduceSum 为 Ascend C 标准 API（:28 已 import，历史 H001 用过），但当前环境本地文档缺失，tmp 步长/对齐/多核降级路径需一次文档核对或最小编译验证。
- **UB/CORE/DMA_IMPACT**：UB：死区转活跃，峰值 +8256B（8192+64），远小于 184K 余量；CORE：不变；DMA：不变。
- **SYNC_IMPACT**：可减（V→S 标量同步点改为 V 内归约完成事件）；不加新同步。
- **PRECISION_RISK**：**中**——ReduceSum 累加次序 ≠ 逐元素标量顺序，浮点求和结果会有 ULP 级差异；golden 为 float 比对，须过 atol/rtol battery（ops-precision-standard）；INV 的 Newton 输入微变可能放大到输出的最后几位。battery bad=0 是硬前提。
- **DUPLICATE_CHECK**：
  - SOURCE_ROUTE: H001 V007→V008（ReduceSum +score）· MECHANISM: 向量归约替代标量 · OLD_CONTEXT: H001 small-D fresh 谱系（已 PARKED）· CURRENT_CONTEXT: UB-LIVENESS-X pass1 · WHY_ORTHOGONAL: H001 是归约形态本身，本条是"死预留转活跃"的处置方式 · WHY_NOT_DUPLICATE: 性能机制（向量归约）同一，但 owner H001 已 PARK → 无活跃占位。
  - vs REDUCE-INVSCALE-X（ACTIVE）：其 SINGLE_HYPOTHESIS=R006 归约+R019 invscale，V002 frozen sync-only，"do not start R019 perf revision until Main"——**归约主题有活跃占位**，本条与 R006/R011 族的主题边界需 Main 裁定。
  - vs idea-pool R007/R011/R028：均归 REDUCE-X（PARKED）。
- **MINIMAL_OFAT_DIFF**（若 Main 归属本路线并立项）：仅 pass1 归约段替换（SumSq 循环 → u²+ReduceSum+dst 读回），保留 bufTmp/bufSum；不动 fuse/emit 标量、不动 tile、不动同步布局（除归约点自身）。
- **EXPECTED_LOCAL_PROBES**：先 battery + 精度核对（不同累加序）；计时 d=4096/32768 fp32 与 fp16，预期 pass1 段时间大降、总时长降 10–30%（若 fuse/emit 标量占 2/3 则仅 ~10-15%）；同协议交错 P/C。

---

## RECOMMENDED_NEXT

1. **等待 Judge 返回 V003 正式结果**（JUDGE_OWNER_REQUIRED 未解除前不产生任何内核字节）。V003 结果决定 Direct Parent 基线；在此之前本文件全部为研究证据，不含实现承诺。
2. **V004 首选候选（Main 审阅）：H1 TRUE_LIVE_SET_BUDGET**——源码级证据最硬（40KB 死预留 + 16KB 模型缺口 + tile 表封顶均可逐行指认），风险面最小（无同步、无数学改动），且是 H3/H5 的共同前置。OFAT 边界已在条目内写明。
3. **H3 前置小实验**（属 TRACK-B 证据补齐，可与等待并行，不改内核）：确认 EnQue/DeQue 在 CANN 8.5.0 / dav-2201 工具链的配对行为（文档+最小编译级验证），并向 Main 确认与 ASYNC-TRIPLE-X 的 MTE2/MTE3 边界。完成前 H3 不升级。
4. **重复性事项交 Main**：H2 ↔ BATCH-RESIDENT-X（参数驻留主题）、H4 ↔ R31A/R002（消输入重读主题）、H5 ↔ REDUCE-INVSCALE-X（归约主题）三组边界需 Main 裁定路线归属，Route Agent 不自行推进被标记 DUPLICATE 的条目。
5. 本路线不安排当日设备计时（窗口预算 2/2 已用尽）；所有 EXPECTED_LOCAL_PROBES 待新会话按统一协议（device events、warmup≥10、in-process、交错 P/C、robust 统计）执行。
