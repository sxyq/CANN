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

---

## LIFETIME MAP VERIFICATION (2026-09-25)

对象：V003 `phase4/local/UB-LIVENESS-X/V003/submission.asc`（SHA256 `2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd`，本轮开头与结尾各核验一次，与 worktree `phase4/workspaces/UB-LIVENESS-X/submission.asc` 一致）。逐行重算全部预留、峰值与模型缺口；三列配置均为 alias=1（`UB_LIVENESS_ALIAS=1` 默认）。`bufTile = max(tile_,512)`（:244），`slot = Align32(bufTile·e)`（:245），`tileFBytes = Align32(bufTile·4)`（:246）。

### 表 A — InitBuffer 预留逐行清单（:253–:261）

| # | Buffer | InitBuffer 行 | 尺寸表达式 | fp32 d4096 t2048（计划 full） | fp16 d4096 t2048（计划 full） | fp32 d32768 t2048（计划 chunk） | 活性判定 |
|---|---|---|---|---:|---:|---:|---|
| 1 | bufParam_ | :254 | `Align32(pBytes)`；pBytes :253 = full 时 `2·Align32(4d)`，chunk 时 `2·tileFBytes` | 32768 | 32768 | 16384 | 部分活跃：`Run()` :181 强制 `fullParam_=false` → 运行期只按 chunk 触碰 16384（d4096 两列）；d32768 计划即 chunk，全部触碰 |
| 2 | bufTmp_ | :255 | `kReduceWorkBytes = 8*1024`（:44） | 8192 | 8192 | 8192 | **死**：`reduceWork_` 仅 :267 赋值、零读取；`ReduceSum` 仅 import :28、零调用；pass1 归约为标量 acc :446–449 |
| 3 | bufSum_ | :256 | 固定 64 | 64 | 64 | 64 | **死**：`reduceDst_` 仅 :266 赋值、零读取（:415 仅为注释） |
| 4 | bufX_ | :257 | `nX·slot`，nX = depth2?2:1（:249） | 16384 | 8192 | 16384 | 活跃（INGEST；alias=1 时 emit 借用 :281–284）；任一瞬时只 1 个 slot 有值（fp32 8192 / fp16 4096） |
| 5 | bufRes_ | :258 | `nR·slot`（:250） | 16384 | 8192 | 16384 | 活跃（pass1/pass2 fuse 读；narrow pass2 以 res0_ 作参数 staging :324） |
| 6 | bufOut_ | :259 | `nX·slot`（与 bufX_ 同尺寸） | 16384 | 8192 | 16384 | alias=1 **死**（句柄 :281–284 重绑到 x-slot，:493 `alias_ ? xs : out0_` 取 xs；Get :272–273 后零触碰）；alias=0 才活跃 |
| 7 | bufScratch_ | :261 | `Align32(2·max(tileF,2048))` = `2·tileFBytes`（:260） | 16384 | 16384 | 16384 | 活跃（formF_ :275 + mulF_ :277；fp32 pass2 emit :490 两段都读写） |
| — | bufAnchor_, bufPool_ | 仅声明 :502，无 InitBuffer | — | 0 | 0 | 0 | 无预留（可随 H1 清理声明，零字节影响） |
| — | **合计** | | | **106560** | **81984** | **90176** | |

计划配置来源：三列均为 `EstBytes` 计划循环 :210–239 首档命中（fp32 d4096 `EstBytes(true,true,alias)=90112 ≤ 188416` → t=2048/full/d2；fp16 d4096 = 73728 → 同；fp32 d32768 full 档 319488 落选 → chunk 档 73728 → t=2048/chunk/d2）。

### 表 B — 峰值 / 死预留 / 模型缺口（修正后）

| 配置（alias=1） | 实际预留 | 真实同时活跃峰值 | 可证死预留 | 相对 184K(188416) 余量 | EstBytes 模型值 | 模型缺口 |
|---|---:|---:|---:|---:|---:|---:|
| fp32 d4096 t2048 计划 full | 106560 ✓ | **49152**（原图 49216 → 修正） | **41024** ✓ | 81856 ✓ | 90112 ✓ | **+16448** ✓ = bufOut_ 16384 + bufSum_ 64 |
| fp16 d4096 t2048 计划 full | **81984**（原图 90112 → 修正） | **36864**（原图 41216 → 修正） | 32832 ✓ | **106432**（原图 98304 → 修正） | **73728**（原图「同构」→ 给出确值） | **+8256** = bufOut_ 8192 + bufSum_ 64 |
| fp32 d32768 t2048 计划 chunk | 90176 ✓ | **49152**（原图 ~49216 → 修正） | 24640 ✓ | 98240 ✓ | 73728 ✓ | +16448 = bufOut_ 16384 + bufSum_ 64 |

**41024 死预留分解（逐项引证，d4096 fp32）**：bufTmp_ 8192（:255，死因见表 A#2）+ bufSum_ 64（:256，表 A#3）+ bufOut_ 16384（:259，表 A#6）+ bufParam_ 运行期闲置 16384 = **41024**。参数闲置成因：计划期 `fullParam_=true` → `gElems_ = Align32(4d)/4 = 4096`（:247）→ gammaF_=[0,16384)、biasF_=[16384,32768)（:264–265）；运行期 :181 强制 chunk → `LoadParamChunk` :486 每 tile 只写 gamma [0,8192) 与 bias [16384,24576)（n≤2048 float），故 [8192,16384) 与 [24576,32768) 共 16384 B 运行期零触碰。**计划 full / 运行 chunk 的配置错配就是这 16384 B 的唯一成因**（d32768 列计划即 chunk，无此洞 → 死预留仅 24640）。

**峰值 49152 构成（d4096 fp32，pass2 fuse 时刻最宽）**：xs 8192 + rs 8192 + formF_ 8192 + mulF_ 8192 + gamma 块 8192 + bias 块 8192 = 49152（≈预算 26.1%；xs/rs 各只计 1 个瞬时活跃 slot）。fp16 列 slot 减半（4096×2）→ 36864。

**模型缺口机理**：`EstBytes` alias 分支 :194–197 = `Align32(param+reduce) + Align32(pool)`，其中 param/reduce/tileF/4·slot 各对应 bufParam_/bufTmp_/bufScratch_/bufX_+bufRes_（**模型 4 slot vs 实际 6 slot**——:257/:258/:259 三个 `nX·slot` 全部分配），模型漏 bufSum_ 64 与 alias 下的 bufOut_ → 缺口恒等于 `16384(或8192) + 64`。三列全部对上，缺口构成一致。

**逐项裁定**（对既有 UB_LIFETIME_MAP 的核验结论）：
- 41024 死预留分解 ✓ 确认（数值与成因全对）。
- 90112 / 106560 / +16448 / 81856 / 98240（fp32 两列）✓ 确认。
- 「alias1 与 alias0 预留差 = 0（bufOut_ 无条件分配 :259）」✓ 确认——这是 V003 本地对照无方向差的结构性解释。
- 「fp32 t=4096 模型 139264 vs 实际 172096」✓ 复算确认（d4096 计划 full 与 d32768 计划 chunk 两形状同值）。
- 「模型放行更大 tile 时实际可能越过 192KB(196608)」✓ 用具体配置坐实：d=8192 fp32 若直接加 4096 档而不修模型 → `EstBytes(4096, full, d2t)=172032 ≤ 188416` 放行，实际 = 65536(param)+8192(tmp)+64(sum)+4×32768(x/res/out/scratch)=**204864 > 196608** → 越过硬件 UB。**模型对齐必须先于加档**（H1 (c) 先于 (d) 的硬证据）。
- **修正 E1**：峰值 49216 → **49152**（原值把 bufSum_ 64 同时计入「死」与「活跃」；reduceDst_ 零读取）。26–27% 占比结论不变（26.1%）。
- **修正 E2**：fp16 d4096 行三处——预留 90112 → **81984**（90112 系 fp32 EstBytes 误植；逐项 32768+8192+64+8192+8192+8192+16384）；峰值 41216 → **36864**；余量 98304 → **106432**；模型值给出确数 73728、缺口 8256。死预留 32832 ✓ 原值正确。
- **修正 E3**：H1 EXPECTED_SHAPES「106560 → 约 65520（-41040）」→ **65536（-41024）**（106560-41024 精确值；65520/41040 系算错 16 B）。且注意：该 65536 是「t 保持 2048 时」的回收后预留；H1 实际放行后 d4096 fp32 会选 t=4096 → 预留 131072（见 H1 计划表）。
- 备注（非错误）：表 A#1 的「洞」按运行期描述正确；按计划期几何 gamma/bias 分区本身无洞（[0,16384)∪[16384,32768) 铺满），闲置来自 chunk 触碰范围 < 分区尺寸——既有文字「计划/运行配置错配产生洞」已准确表达此义。

---

## H1 MINIMAL OFAT DIFF PLAN (2026-09-25) — PLAN ONLY, DO NOT APPLY UNTIL JUDGE RETURNS

相对 V003（SHA `2eb9b5d…`）。单一概念：**预算按实际分配清单与实际活跃集计费 → 解封更大 tile**。四组编辑共同实现这一概念（与本文件 H1 条目的 (a)(b)(c)(d) 一致）；不得夹带同步、参数驻留、u 驻留、标量→向量改写。`kUbBudgetBytes = 184*1024`（:45）不变，数学路径不变。

### 编辑 1 — 删除两个死 InitBuffer（无条件）

- 删 `:255 pipe_.InitBuffer(bufTmp_, kReduceWorkBytes);`
- 删 `:256 pipe_.InitBuffer(bufSum_, 64);`
- 配套删 `:266 reduceDst_ = bufSum_.Get<float>();` 与 `:267 reduceWork_ = bufTmp_.Get<float>();`（Get 未分配 TBuf 不可用，必须同删）；`:503` TBuf 声明去掉 `bufTmp_, bufSum_`；`:506` 成员去掉 `reduceDst_, reduceWork_`；`:415` 注释同步改写。`:28` 的 `using AscendC::ReduceSum` 可留（纯 import，零字节影响）。
- 字节效果：全 dtype −8256（8192+64）。

### 编辑 2 — bufOut_ 按 alias 模式条件分配

```text
:259  pipe_.InitBuffer(bufOut_, nX * slotBytes_);        →  if (!alias_) { InitBuffer(bufOut_, nX*slotBytes_); }
:272–273  out0_ = bufOut_.Get<T>(); out1_ = …            →  移入同一 if (!alias_) 分支
:281–284  if (alias_) { out0_ = x0_; out1_ = x1_; }      →  保持不变
:493   outStage = alias_ ? xs : (flip ? out1_ : out0_)   →  不变
```

- alias=1（默认构建）：省 nX·slot（d4096 fp32 = 16384；fp16 = 8192）。
- alias=0（对照构建）：bufOut_ 仍分配，路径不变——条件分支必须在两种构建下都编译并跑通（battery 见第 3 节）。
- 这是 V003 :281–284 重绑从未兑现的字节承诺首次进入预留。

### 编辑 3 — 参数预留对齐运行期（消 16384 洞）+ EstBytes 改为实际清单镜像

运行期事实链：`Run()` :181 无条件 `fullParam_=false` → `LoadParamChunk` :486 每 tile chunk 写入、`EmitAt` 分支 :397–398 取基址；`LoadFullParams` :287 零调用。故计划期 full 配置是纯浪费：

- `:247` → `gElems_ = tileFBytes_ / 4;`（去 full 三目）
- `:253` → `int32_t pBytes = 2 * tileFBytes_;`（去 full 三目；chunk 分支原样）
- 计划循环 :210–239：删两个 `full=true` 分支尝试（:215–226），保留原 chunk 顺序的两个分支（:227 d2=true → :233 d2=false），去掉 `fullParam_` 维度；`:206 fullParam_ = false` 与 `:181` 保留（运行期保证，注释不动）；`:308/:397/:398` 分支与 `LoadFullParams`/`EmitAt` 死代码**不动**（后者是 H2/BATCH 的原材料）。
- `EstBytes`（:186–200）重写为与 :244–:261 实际分配逐项相等：

```text
EstBytes(tile, elemBytes, d2, alias):            // 去 dim、paramFull 两形参
    bufTile = tile < 512 ? 512 : tile            // 镜像 :244（原式用裸 tile，t<512 低估）
    slot   = Align32(bufTile * elemBytes)        // 镜像 :245
    tileF  = Align32(bufTile * 4)                // 镜像 :246
    n      = d2 ? 2 : 1                          // 镜像 :249–250
    b = 2*tileF              // bufParam_（chunk，镜像 :253）
      + 2*n*slot             // bufX_ + bufRes_（:257–258）
      + 2*tileF              // bufScratch_（:260，tileF≥2048 恒等）
      + (alias ? 0 : n*slot) // bufOut_ 仅 !alias（:259）
    return Align32(b)        // bufTmp_/bufSum_ 已除；各 32B 对齐项，Align32 为恒等
```

- 镜像性质：模型 == 实际分配清单 → 模型 ≤ 188416 ⟺ 实际 ≤ 188416 < 196608（192KB），**新 tile 档下 192KB 由构造保证**（原模型在 t=4096 d8192 fp32 full 档会放行 172032 而实际 204864，见核验节——该反例是 (c) 先于 (d) 的硬性顺序依据）。

### 编辑 4 — tileChoices 加档

`:205` → `const int32_t tileChoices[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64};`（大值在前；循环对每档先试 d2=true 再 d2=false，保持原顺序语义）。`c > dim_` 时 `tt = dim_`（:211）——d=4096 在 8192 档即得 tt=4096。

### 新档预留核算（post-H1，alias=1，模型==实际）

预算：内部 188416（184K）/ 硬件参照 196608（192KB）。分解 = param(2·tileF) + x(n·slot) + res(n·slot) + scratch(2·tileF)。

| 形状 | 循环命中 | n(d2) | param | x | res | scratch | 合计 | ≤188416 | ≤196608 | 每行 tile 往返/pass（前→后） |
|---|---|---|---:|---:|---:|---:|---:|---|---|---|
| **fp32 d=4096** | c=8192→tt=4096，d2=true 命中 | 2 | 32768 | 32768 | 32768 | 32768 | **131072** | ✓ (slack 57344) | ✓ (slack 65536) | 2 → **1** |
| **fp16 d=4096** | c=8192→tt=4096，d2=true 命中 | 2 | 32768 | 16384 | 16384 | 32768 | **98304** | ✓ (90112) | ✓ (98304) | 2 → **1** |
| **fp32 d=8192** | c=8192 d2t=262144✗ / d2f=196608✗ → c=4096 d2t 命中 | 2 | 32768 | 32768 | 32768 | 32768 | **131072** | ✓ | ✓ | 4 → **2** |
| **fp16 d=8192** | c=8192 d2t=196608✗ → d2f 命中 | 1 | 65536 | 16384 | 16384 | 65536 | **163840** | ✓ (24576) | ✓ (32768) | 4 → **1** |

要点：
- 四个必证配置全部 **≤ 188416 < 196608**；最紧的 fp16 d8192（163840）距内部预算仍余 24576。
- fp32 d≥8192 永不落 t=8192：d2=true 262144、d2=false 196608 都越过 188416（后者恰等于 192KB 硬件值，内部 8KB 余量把它拦在预算内——这是 :45 留余量的直接用途）→ 落 t=4096。
- fp16/bf16（e=2）d≥8192 落 t=8192 / d2=false（单 slot，串行发射下无深度损失）。
- 补充行（探针/用例形状）：fp32 d=16384 → t=4096 d2t 131072（8→4）；fp32 d=32768 → t=4096 d2t 131072（16→8）；fp16 d=32768 → t=8192 d2f 163840（16→4）；d≤2048 全部 t 不变（空白对照）。
- alias=0 对照：加 n·slot——fp32 d4096 t4096 d2t = 147456 ✓；fp16 d8192 t8192 d2f = 180224 ✓（两对照在新档下仍过预算，可编可跑）。
- 参数：t=dim 形状上 2·tileF ≡ 2·dimF，与原计划 full 同字节；t<dim 形状参数预留随 chunk 缩小（如 d4096 t2048：32768→16384）。

### 明确不做（OFAT 边界）

不动 :93/:116/:129 等任何 PipeBarrier 与同步原语；不动 FuseU/SumSq/emit 数学；不启用 LoadFullParams（参数驻留归 BATCH）；不启用 ReduceSum（归约归 REDUCE）；不动 u 跨 pass（H4）；不改行并行分块与 core 映射；不改 `kUbBudgetBytes`。

---

## H1 CORRECTNESS BATTERY (2026-09-25) — SPEC FOR V004, RUNS ONLY AFTER JUDGE RETURN + MAIN AUTHORISATION

前提：本轮无设备运行。以下为 H1 实现后必须一次通过的用例集与不变量（沿用 V003 的 8 形状 + alias0 对照骨架，V003 全部 9 项均 bad=0 可直接对照）。golden 与容差按 `ops-precision-standard`。

### A. 回归组（V003 原 8 形状 + alias0 对照；改后配置与改前相同者即空白对照）

| # | 形状 | H1 前 (t, d2, 计划 full) | H1 后 (t, d2) | 该形状在 H1 下变与不变 / 覆盖什么 |
|---|---|---|---|---|
| A1 | FP32 1×64 | 64, d2t, full | 64, d2t | tile 不变（空白对照）；**param 布局变**：pBytes 512→4096、gElems 64→512（bufTile=512 下限效应）→ 验证 chunk 分区 |
| A2 | FP32 4×256 | 256, d2t, full | 256, d2t | tile 不变；param pBytes 2048→4096、gElems 256→512 → chunk 分区 + 多行 |
| A3 | FP16 8×1000 | 1000, d2t, full | 1000, d2t | **tile/d2/param 布局全不变**（t=dim 时 2·dimF≡2·tileF）→ tile 决策空白对照；预留仍降 12288（8192+64+out4032，死字节回收本身）；非 2 次幂 tile |
| A4 | BF16 3×777 | 777, d2t, full | 777, d2t | tile/d2/param 布局全不变（空白对照）；预留仍降 11392；narrow 量化路径（:354–363） |
| A5 | FP16 5×64 | 64, d2t, full | 64, d2t | tile 不变；param 512→4096、gElems 64→512；narrow + 小 tile |
| A6 | FP32 2×4096 | 2048, d2t, full | **4096, d2t** | **tile 2048→4096**：fp32 新最大 tile、单 tile/行、param 洞路径消失 |
| A7 | BF16 4×128 | 128, d2t, full | 128, d2t | tile 不变；param 1024→4096、gElems 128→512 |
| A8 | FP16 3×32768 | 2048, **d2t**, chunk | **8192, d2=false** | **tile 与深度双变**：史上首次执行 d2=false（x1_≡x0_ :269）+ t=8192 narrow emit |
| A9 | alias0 对照 FP32 4×256 | 256, d2t（bufOut_ 恒分配） | 256, d2t（bufOut_ **条件**分配） | 验证编辑 2 的 !alias_ 分支在旧 tile 下原样可用 |

### B. 新路径组（H1 加档 / 条件分配 / 新布局专门覆盖）

| # | 形状 | H1 后配置 | 覆盖的新增路径 |
|---|---|---|---|
| B1 | FP32 2×8192 | t=4096, d2t | c=8192 档两深度均落选（d2t 262144 / d2f 196608）→ c=4096 命中；fp32 双 tile/pass（t<dim 多 tile 重写参数基址） |
| B2 | FP16 2×8192 | t=8192, **d2=false** | fp16 达到 t=8192 的唯一通路；单 slot 全行单 tile |
| B3 | BF16 2×8192 | t=8192, d2=false | n=8192 的 narrow 逐元素量化环（:354–363）与 emit 标量环 |
| B4 | FP32 1×4097 | t=4097（=dim）, d2t | 新表下 dim≤8192 时首档 `tt=dim`（:211）→ 单 tile/行 n=4097：非 2 次幂 tile，slot/tileF=Align32(16388)=**16416** 上取整（原表此形状为 t=2048 三 tile） |
| B4b | FP32 1×10001 | t=4096, d2t | t=4096 的**真尾块**：dim>8192 且 fp32 8192 档落选 → 3 tiles、末 tile n=1809（byteLen 7236 非 32B 对齐）→ DataCopyPad pad 在新 tile 尾部重验 |
| B5 | FP16 1×9000 | t=8192, d2=false | t=8192 尾块 n=808（byteLen 1616，非 32B 对齐）→ pad 路径在新 tile 下重验 |
| B6 | FP32 1×8193 | t=4096, d2t | 多 tile + 末 tile n=1 组合（3 tiles） |
| B7 | alias0 编译 FP32 2×4096 | t=4096, d2t, **alias=0** | 新档下 bufOut_ 真实分配路径（预留 147456 ≤ 188416）：out0_/out1_ 独立写出 |
| B8 | FP32 2×16384 / 2×32768 / FP16 2×16384 | 4096/d2t、4096/d2t、8192/d2f | 计时探针主/对照形状的正确性前置（协议要求 correctness first） |

### C. 不变量（每形状逐条成立）

1. **输出正确**：bad=0（golden 对比，按 ops-precision-standard 容差），FP32/FP16/BF16 三 dtype 全覆盖。
2. **模型==实际（静态即可核验，无需设备）**：对每个命中配置，`EstBytes(选定 t, d2, alias)` == `Σ InitBuffer 实际尺寸`（编辑 1–3 后镜像相等）；且 Σ ≤ 188416 与 Σ ≤ 196608 同时成立。t<512 形状（A1/A2/A5/A7）专门核对 bufTile=512 下限两侧相等。
3. **参数分区**：`gElems_ = tileFBytes/4`；gamma 区 [0,tileF)、bias 区 [tileF,2·tileF)；LoadParamChunk 每 tile 写 n ≤ tile ≤ tileF/4 floats 不越 2·tileF；emit :490 读 biasF_ 基址（fullParam_ 恒 false 分支）。t=dim 形状（A1–A7、B4）验证单 tile 铺满，t<dim 多 tile 形状（A8、B1、B4b、B5、B6、B8）验证同基址反复重写与读取一致。
4. **d2=false 数据流**：x1_≡x0_（:269）后，flip 交替的覆写只发生在 fuse 消费完当前 tile 之后（CopyIn :466 → fuse :471 → 下一 tile 再 CopyIn）；A8/B2/B3 验证。
5. **alias emit 安全**：outStage=xs（:493）只在 fuse 读完 xs 后被 FromFloatSeq 覆写（:494）；单 tile/行（A6/B2）时整行缓冲一次覆写——顺序未改，但形状未测过，必须实跑。
6. **与 V003 输出一致**：未变配置形状（A1–A5、A7）上 V004 输出与 V003 完全一致（数学路径零改动；任何差异=实现泄漏信号）。
7. **无越界**：dav-2201 编译/链接 PASS，运行无 `ub address out of bounds`（B 组最紧配置 fp16 t=8192 d2f 163840、alias0 B7 147456）。
8. **别名双分支**：UB_LIVENESS_ALIAS=1 与 =0 两个构建各自编译并全量跑 A+B（=0 至少 A9+B7）；alias1 预留须严格小于 alias0（差 = n·slot，首次兑现的字节差）。

### D. 通过判据

A+B 全部 bad=0、C1–C8 全部成立 → 才进入本地计时（同协议：device events、warmup≥10、in-process、交错 P/C）；任何一条失败按 correctness 修复处理，不得顺手改同步或数学（执行契约 §E 边界）。

---

## DUPLICATE SEARCH — DEAD-RESERVATION / LIVE-SET-BUDGET DONORS (2026-09-25)

检索范围与命中：`phase4/control/idea-pool-29-routes.md`（29 行全表）、`phase4/control/architecture-evidence-map.md`（全部主题行）、`cann/phase4/archive/**`（retired-routes、historical-branches）与 `cann-next6/UB-LIVENESS-X/phase4/archive/**` 全文（关键词 dead reserv / live set / reclaim / unused InitBuffer / EstBytes）、`phase4/research/{REDUCE,ASYNC,ALIGN,BATCH}/next-hypotheses.md`、本路线 workspace 自档。

**检索结论：无任何先前路线/假设实现过「预算模型镜像实际 InitBuffer 清单 + 死预留回收 + 按活跃集选 tile」这一组合机制。四条近邻记录如下；idea-pool 与 evidence-map 为显式阴性结果。**

### 记录 D1 — R030（最近的历史近邻，档案实读）

- **SOURCE_ROUTE**: FULL-R030-WIDE-PARAM-REUSE（phase3 实验，档案 `phase4/archive/historical-branches-20260924/exp__full-r030-wide-param-reuse-v001/…/结果.md`）
- **MECHANISM**: 缩减专用角色静态工作区——R030 分支 tile 6912→4096 元素、retained-y 改 half，D=16384 FP16 静态估算 236 KiB→160 KiB，目标是清除既有 `ub address out of bounds`（507035）
- **OLD_CONTEXT**: 多模宽参数谱系（R029 宽缓存分支共存），事后缩配以通过容量上限
- **CURRENT_CONTEXT**: UB-LIVENESS-X fresh 两遍内核：EstBytes↔实际分配清单镜像、删死预留（tmp/sum/out/参数洞）、再放行 4096/8192 档
- **WHY_ORTHOGONAL**: R030 砍的是**活跃**专用角色的尺寸（缩脚）；H1 删的是**从未被触碰**的预留并修正估算器。R030 不含任何估算器对齐或死字节判定
- **WHY_NOT_DUPLICATE**: 机制不同（活跃角色减配 vs 死预留回收 + 模型对齐 + tile 解封的因果链）；R030 无 tile 选择预算模型；异谱系且早已归档，无活跃占位

### 记录 D2 — R005（同主题不同机制）

- **SOURCE_ROUTE**: FULL-R005-LARGE-TILE（idea-pool R005 行，owners MID-X / WIDE-X 均 PARKED；档案 `independent__full-r005-large-tile-i001`）
- **MECHANISM**: 直接放大 tile 字节（large-tile 独立实现），表内 `still_unexplored` 仅剩「tile autotune per D bucket」
- **OLD_CONTEXT**: 独立大 tile 谱系
- **CURRENT_CONTEXT**: H1 只在模型与实际逐项相等之后加 4096/8192 档；tile 选择由对齐后的预算判定决定
- **WHY_ORTHOGONAL**: R005 不释放峰值活跃集、不修正估算器；本内核原估算器在 d8192 fp32 t=4096 计划 full 档会放行 172032 而实际 204864（>196608）——直接移植 R005 会越界，这正是 H1 的 (c) 必须先于 (d)
- **WHY_NOT_DUPLICATE**: 主题相邻（更大 tile）但性能杠杆的来源不同（死预留+模型对齐解锁 vs 直接加档）；无活跃 owner，不构成占位

### 记录 D3 — 本路线自档（直系续篇，非重复）

- **SOURCE_ROUTE**: UB-LIVENESS-X V001 `external-idea-record.md` + `WHY_NOT_DUPLICATE.md`（worktree 自档）
- **MECHANISM**: phase-role UB 池复用 INGEST→EMIT（V001 起实现 alias，:281–284 重绑）
- **OLD_CONTEXT**: V001 以「峰值活跃集收缩」立论，但 V001–V003 的 bufOut_ 始终无条件分配 → alias 实际节省 0 B，从未进入 tile 决策
- **CURRENT_CONTEXT**: H1 让该字节承诺首次兑现（条件分配 + 模型镜像 + tile 档）
- **WHY_ORTHOGONAL**: V001 改的是字节的角色归属；H1 改的是**计费口径**（预留/估算按实际活跃）
- **WHY_NOT_DUPLICATE**: 同路线直接父系机制的收尾步骤（V003 架构的下一步），非外来重复

### 记录 D4 — 活跃路线邻近主张（显式排除）

- **BATCH-RESIDENT-X**（`research/BATCH-RESIDENT-X/next-hypotheses.md`）：自建 `kUbBudgetBytes=96KB` 行批量选择器、参数条带驻留**花** UB 换驻留——方向相反（增加活跃区）；参数驻留主题已由 MAIN-2 仲裁归 BATCH（本文件 H2 → DUPLICATE·DEFER_TO_BATCH）。H1 的参数改动仅把预留缩到运行期实际触碰范围，**运行期读写行为逐字节不变**，不触碰驻留语义 → 与 BATCH 正交。
- **ALIGN-TAIL-X**（:54）：为其自身 padded 布局「重算 192KiB 预算」——是自身 buffer 数量变化后的重算，无死预留判定、无估算器镜像 → 阴性。
- **ASYNC-TRIPLE-X**（:554）：静态盘点自身 InitBuffer 常量（90208 B / 69728 B）——现状盘点，非计费机制 → 阴性。
- **REDUCE-INVSCALE-X**（:312）：tile 由自身 UB 预算常量选取——无死预留概念 → 阴性。
- **idea-pool-29-routes.md**：29 行无 dead-reservation / live-set-budget 条目（R013 双缓冲反而**增加**峰值占用，见本路线 WHY_NOT_DUPLICATE 表）→ **显式阴性**。
- **architecture-evidence-map.md**：最近主题为「UB layout = MIXED（pad 已证、layout sweep thin）」与「queue depth = MIXED」，无死预留/活跃集计费主题 → **显式阴性**。
- **phase4 archives 全文 grep**（cann 主仓 + UB worktree 两套 archive）：`EstBytes` / dead-reserv / live-set 字样零命中（EstBytes 仅存在于本路线内核系）→ **显式阴性**。

仲裁依据复核：`phase4/control/consolidation-20260924.md` :175–176 已记录 UB 只保留 buffer lifetime / aliasing / peak UB footprint / live-set budgeting，且 UB H1 TRUE_LIVE_SET_BUDGET 为已批准 backlog、Judge 返回前不得开工——本节结论与该仲裁一致，无需提交新的归属争议。

---

## CYCLE 2026-09-25（续）— TRACK-A 复核 + H1 OFAT 定稿 + 全量 tile 预算矩阵 + 逐编辑否证判据

### A. TRACK-A 状态复核（只读，本轮零内核字节）

| 项 | 结果 |
|---|---|
| `phase4/local/UB-LIVENESS-X/V003/submission.asc` SHA256 | `2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd` ✓ 与既定值一致 |
| `submission.sha256` sidecar | 同值 ✓ |
| worktree `phase4/workspaces/UB-LIVENESS-X/submission.asc` | 同值 ✓（字节同一性三处一致） |
| `source-meta.json` | DECISION=ONLINE_CANDIDATE、SINGLE_CHANGE_AUDIT=PASS、CORRECTNESS=PASS bad=0 ✓ |
| `local-result.json` | decision=ONLINE_CANDIDATE ✓ |
| `phase4/online/UB-LIVENESS-X/` | 仍不存在（从未提交） |
| `phase4/control/judge-handoff-ub-v003.md` | SUBMISSION_ID=PENDING、REMOTE_SHA=PENDING、DECISION=PENDING（MAIN-1 尚未提交） |
| `phase4/control/online-candidate-pool.tsv` 第 4 行 | JUDGE_READY=YES，SHA 同上 ✓ |

本轮无设备计时；MAIN-2 不自提交；V004 不开工（等 Judge 返回 + Main 发 NEXT_HYPOTHESIS）。

### B. H1 OFAT 编辑顺序定稿与逐编辑核验点（PLAN ONLY，不实施）

四组编辑（编辑 1–4，见上文「H1 MINIMAL OFAT DIFF PLAN」）实现同一概念变更：**预算按实际分配清单与真实活跃集计费**。顺序不可换；G1–G4 全部静态可核，不需设备：

| 序 | 编辑 | 为什么必须在此位 | 核验点（进入下一编辑前） |
|---|---|---|---|
| 1 | 删 bufTmp_/bufSum_ 死 InitBuffer | 纯删除、最小审阅面，先确立「死字节」口径；编辑 3 的模型镜像以删除后的清单为基准 | **G1**：删后 `reduceDst_`/`reduceWork_` 零残余引用（编译 PASS 即证）；ΣInitBuffer 恰降 8256B（两 alias 构建同值，静态加总） |
| 2 | bufOut_ 按 `!alias_` 条件分配 | 建立在编辑 1 之后的干净清单上；必须先于编辑 3——模型镜像的是最终分配清单 | **G2**：静态枚举 21 配置，alias1 预留 − alias0 预留 == −n·slot 恒等；alias0 构建编译 PASS |
| 3 | EstBytes 改实际清单镜像 + 参数预留对齐运行期 | 必须先于编辑 4 的硬证据见下「越界反例」：模型未对齐就加档 = 按错误账本扩容 | **G3**：网格静态枚举 model == ΣInitBuffer 全等；FP32 D=16384 新实际 == 131072；A1–A5/A7 输出与 V003 逐位一致（运行期本就强制 chunk，输出必须不变，任何差异 = 实现泄漏） |
| 4 | tileChoices 加 8192/4096 档 | 唯一性能杠杆，只能建在对齐后的模型上 | **G4**：计划循环静态重放，21 格命中配置与 C 节矩阵逐一相等；全部 ≤188416 且 ≤196608 |

- 若 Main 的 SINGLE_CHANGE 审阅要求更窄的修订粒度，自然拆分点在编辑 3 之后：**V004 = 编辑 1–3（预算真实性，tile 决策不变档）/ V005 = 编辑 4（加档）**，两段各自独立可核、各自过 battery。默认仍按单修订 V004 四编辑执行，拆分与否由 Main 定。
- 每编辑的字节效果与「明确不做」边界沿用计划节，不重复。

**越界反例（编辑 3 先于编辑 4 的硬证据，本轮新算出）**：旧 `EstBytes` alias 分支在 FP32 D=16384 / t=2048 / plan-full / d2t 恰返回 188416（== 内部预算，`≤` 放行）→ V003 实际分配 = param 131072 + 8192 + 64 + 3×16384 + scratch 16384 = **204864 > 196608（192KB 硬件）**。推广：FP32 D∈(15352,16384] 全区间旧模型放行、实际越硬件；FP16/BF16 有 D∈(18424,18432] 同型窄窗（本矩阵 D 集不含）。V003 battery 8 形状 + alias0 对照均不含 FP32 D=16384（本地 results 目录逐文件核对：64/4096/32768-fp16/777-bf16/128/256/64/1000-fp16 + a0-256）→ 这是 V003 的潜在越界形状（静态推断，未上设备验证；在线形状未知，若 Judge 侧含 FP32 D≈16K 需留意）。H1 编辑 3+4 后该形状预算 = 131072，越界消除。

### C. 全量 tile 预算矩阵（2026-09-25 重算）— FP16/BF16/FP32 × D{64,256,1024,4096,8192,16384,32768}

口径：alias=1；预算 = `kUbBudgetBytes` 188416（184KiB 实用上限，已含 runtime/temp/queue 余量），硬件参照 196608（192KB）。OLD = V003 旧 `EstBytes`（裸 tile 无 512 下限、4-slot pool、含 8192 reduce、plan-full 维度）及其选中配置与实际预留；NEW = 编辑 3 后镜像模型 == 实际分配（编辑 1–2 生效后清单），选档循环 = 编辑 4 新表（8192 起降序、每档先 d2t 后 d2f、无 full 维度）。峰值 = 真实同时活跃集（2·slot + 2·formCap + 2·tileF，pass2 fuse 时刻最宽，含 gamma/bias 两 chunk）。证据脚本与输出：worktree `phase4/local/UB-LIVENESS-X/track-b/tile-budget-matrix-20260925.{py,txt}`。

**FP32（e=4）**

| D | OLD EstBytes | OLD 命中(t,full,d2) | V003 实际预留 | OLD 缺口 | NEW 模型==预留 | NEW 命中(t,d2) | 活跃峰值 | 余量 vs 184K | tile 旧→新 | tiles/pass 旧→新 |
|---:|---:|---|---:|---:|---:|---|---:|---:|---|---|
| 64 | 10240 | (64,T,T) | 25152 | +14912 | 16384 | (64,T) | 12288 | 172032 | 64→64 | 1→1 |
| 256 | 16384 | (256,T,T) | 26688 | +10304 | 16384 | (256,T) | 12288 | 172032 | 256→256 | 1→1 |
| 1024 | 40960 | (1024,T,T) | 49216 | +8256 | 32768 | (1024,T) | 24576 | 155648 | 1024→1024 | 1→1 |
| 4096 | 90112 | (2048,T,T) | 106560 | +16448 | 131072 | (4096,T) | 98304 | 57344 | 2048→**4096** | 2→1 |
| 8192 | 122880 | (2048,T,T) | 139328 | +16448 | 131072 | (4096,T) | 98304 | 57344 | 2048→**4096** | 4→2 |
| 16384 | 188416 | (2048,T,T) | **204864 >HW** | +16448 | 131072 | (4096,T) | 98304 | 57344 | 2048→**4096** | 8→4 |
| 32768 | 73728 | (2048,F,T) | 90176 | +16448 | 131072 | (4096,T) | 98304 | 57344 | 2048→**4096** | 16→8 |

**FP16（e=2）**

| D | OLD EstBytes | OLD 命中(t,full,d2) | V003 实际预留 | OLD 缺口 | NEW 模型==预留 | NEW 命中(t,d2) | 活跃峰值 | 余量 vs 184K | tile 旧→新 | tiles/pass 旧→新 |
|---:|---:|---|---:|---:|---:|---|---:|---:|---|---|
| 64 | 9728 | (64,T,T) | 19008 | +9280 | 12288 | (64,T) | 10240 | 176128 | 64→64 | 1→1 |
| 256 | 14336 | (256,T,T) | 20544 | +6208 | 12288 | (256,T) | 10240 | 176128 | 256→256 | 1→1 |
| 1024 | 32768 | (1024,T,T) | 36928 | +4160 | 24576 | (1024,T) | 20480 | 163840 | 1024→1024 | 1→1 |
| 4096 | 73728 | (2048,T,T) | 81984 | +8256 | 98304 | (4096,T) | 81920 | 90112 | 2048→**4096** | 2→1 |
| 8192 | 106496 | (2048,T,T) | 114752 | +8256 | 163840 | (**8192,F**) | 163840 | 24576 | 2048→**8192** | 4→1 |
| 16384 | 172032 | (2048,T,T) | 180288 | +8256 | 163840 | (**8192,F**) | 163840 | 24576 | 2048→**8192** | 8→2 |
| 32768 | 57344 | (2048,F,T) | 65600 | +8256 | 163840 | (**8192,F**) | 163840 | 24576 | 2048→**8192** | 16→4 |

**BF16（e=2）**：与 FP16 逐格全等（公式只依赖 elemBytes；脚本单独重算 7 格，输出与 FP16 表逐一相同）。

要点：
1. **21 格新配置全部 ≤188416 < 196608**（模型==实际，192KB 由构造保证）；最紧 FP16/BF16 D≥8192 = 163840，余量 24576——若日后 H3 摄入队列要从预算里取字节，此余量是唯一来源。
2. **旧模型缺口两型**：(i) 中大 D 恒等缺口 +16448（fp32）/+8256（fp16/bf16）= bufOut_ + bufSum_ 未建模；(ii) D<512 小 tile 地板缺口（+14912/+10304/+9280/+6208/+4160）= 旧式用裸 tile、实际 `bufTile=512` 地板 + 死预留叠加。两型在编辑 1–3 后同时归零。
3. **旧模型放行硬件越界**：FP32 D=16384 实际 204864 > 196608（见 B 节）。
4. **tile 解封形态**：FP32 宽 D 统一 4096/d2t（往返减半）；FP16/BF16 宽 D 统一 8192/**d2=false**（往返 ×1/4，D=32768 从 16 tiles/pass 降到 4；d2 丢失在串行发射下无深度代价——现状双缓冲本就零重叠）。D≤1024 全部 tile 不变 = 空白对照。
5. **峰值始终 ≤ 预留**（FP32 宽 D 峰值 98304 = 52% 预算），按预留计费是保守口径；回收死预留不解封峰值，解封来自「计费模型对齐 + 加档」这条因果链。

### D. 逐编辑否证判据 + battery 映射（V004 spec；battery 用例明细见上文「H1 CORRECTNESS BATTERY」A/B/C/D 节，此处不重复用例，只做映射与否证）

| 编辑 | 静态否证条件（不需设备，任一成立即该编辑方案被推翻） | battery 映射（设备侧） | 设备侧否证条件 |
|---|---|---|---|
| 1 删死预留 | (i) 删后仍存在 `reduceDst_`/`reduceWork_` 引用（编译报错即证）；(ii) Σ降幅 ≠ 8256B；(iii) 源码出现任何 `ReduceSum` 实调用（则「死」判定错） | 全组 A 回归（任何形状依赖被删字节即会 bad>0 或链接失败）+ C8 alias1 预留 < alias0 | A 组任一 bad>0 且 diff 归因于本编辑 |
| 2 条件分配 bufOut_ | (i) 21 格中 alias1−alias0 差 ≠ −n·slot；(ii) alias0 构建编不过 | A9（alias0 旧 tile 原样）+ B7（alias0 新档真分配）+ A6/A8/B2（alias1 emit 借 xs）+ C8 | A9/B7 bad>0 → `!alias_` 分支坏；A6/A8/B2 bad>0 → emit 借 xs 安全性破坏 |
| 3 模型镜像 + 参数对齐 | (i) 任一格 model ≠ ΣInitBuffer；(ii) FP32 D=16384 新实际 ≠ 131072（越界未消除）；(iii) 网格任一格 >188416 或 >196608 | A1–A5/A7（param 布局变、tile 不变的空白对照）+ C2/C3/C6 | A1–A5/A7 输出与 V003 非逐位一致 → 实现泄漏（运行期本就 chunk，输出必须不变）；C3 参数分区失败 |
| 4 tileChoices 加档 | (i) 21 格任一命中 (t,d2) 与 C 节矩阵不等（循环序错）；(ii) 本应变 tile 的格子未变（加档未生效） | A6（fp32 2048→4096）+ A8（fp16 2048→8192 且首次 d2f）+ B1–B6/B8（新档尾块 pad、多 tile、单 tile 覆写、非 2 次幂 dim）+ C4/C5/C7 | B 组任一 bad>0（尾块 pad B4b/B5、d2=false A8/B2/B3、整行覆写 B4/B2）；运行无 ub OOB（最紧 163840 / alias0 147456） |

**H1 整体（性能主张）否证条件**——前置 G1–G4 与 battery 全绿之后：
- 宽 D 主探针（FP32 D=16384/32768、FP16 D=32768，同协议交错 P/C、warmup≥10、robust 统计）中位 Δ 落在 [-3%, +3%] 且 CI 跨 0，而往返数已按矩阵减半/×1/4 → 标量循环主导成立、tile 杠杆不兑换时间 → **H1 降级为预算真实性修复，不作性能修订主张**。
- 窄 D 对照（D≤1024，tile 无变化）出现与宽 D 同幅差异 → 差异系噪声而非 tile 杠杆 → 性能主张否证。
- 预算主张与性能主张分开裁定：前者由静态证据（G1–G4 + C 节矩阵）独立成立，不受性能否证影响。

### E. 新增 UB-pure 假设筛查（2026-09-25，阴性记录）

本轮按「可选新 UB-pure 假设」筛查两个候选，均**不立项**：

- **SCRATCH_HALF_MERGE（formF_/mulF_ 合并，scratch 减半省 tileF 字节）→ 静态否证**：两半同时活跃——narrow fuse `Add(formF_, formF_, mulF_, n)`（:437）与 pass2 emit `mulF_[i] = formF_[i]·…`（:490）都同时读写两半；formCap 分区（:276–277）是必要布局而非浪费。生命周期核对直接排除，无需设备。
- **RES_BYTES_POST_FUSE_RECLAIM（emit 期回收 res 字节）→ 无预算收益**：峰值由 pass2 fuse 时刻决定（x+res+scratch+param 同时活跃），emit 期 rs 已死但只影响瞬时占用、不抬升峰值口径；alias 已提供 out 字节 → 对计费与 tile 解封零贡献。

现有假设池保持 ≥3 条已筛假设，无需新增。

### F. 假设池状态确认 + 更新后的 RECOMMENDED_NEXT

假设池状态（不变，本轮复核）：**H1 TRUE_LIVE_SET_BUDGET = READY_FOR_MAIN_REVIEW**（计划、矩阵、否证判据、battery 四件套本轮齐全）；H2 → DUPLICATE · **DEFER_TO_BATCH**；H3 → **DEFER_TO_ASYNC**（EnQue/DeQue 工具链前置证据未取）；H4 U_RETENTION = NEEDS_MORE_EVIDENCE + 重复预警（对照 R31A/R002 待 Main）；H5 → **DEFER_TO_REDUCE**。活跃可推进者仅 H1。

RECOMMENDED_NEXT（本轮更新）：
1. 等 Judge 返回 V003（handoff 仍 PENDING；MAIN-2 不自提交、不排设备计时）。
2. Main 审阅 H1 四件套：OFAT 顺序与 G1–G4（B 节）+ 全量矩阵（C 节）+ 否证判据/battery 映射（D 节）+ 计划/battery 原文。若批准，V004 = 编辑 1–4 单修订（或按 B 节拆 V004/V005）。
3. 向 Main 上报的新事实：**V003 在 FP32 D=16384 存在静态可证的硬件越界（实际 204864 > 196608），battery 未覆盖该形状**——是否影响 Judge 判断由 Main 决定，Route Agent 不动作。
4. H3 的 EnQue/DeQue 工具链证据补齐仍是可并行的只读工作（属 ASYNC 边界确认前置，不升级 H3）。
