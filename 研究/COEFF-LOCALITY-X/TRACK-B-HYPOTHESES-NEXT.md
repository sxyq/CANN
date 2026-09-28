# TRACK-B HYPOTHESES NEXT — COEFF-LOCALITY-X

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-coeff
BRANCH=m2/coeff-locality
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16（R31B-V011）；本路线 OFFICIAL_BEST=44.16（V001，ROUTE_OFFICIAL_BEST）
DATE=2026-09-29
MODE=TRACK-B 只读研究 — 不改 Kernel、不建 V003、不跑正式 performance、不提交 Online

---

## 0. V001 / V002 闭环判断

| 版本 | 结论 | 是否闭环 | 依据 |
|---|---|---|---|
| V001 | LOCAL_REJECTED（1x32768 +19.2% favP 5/0）；Official 44.16 保留 | **闭环（结论侧）** | 编译/正确性/测时/审计/证据齐全；LOCAL_REJECTED 为最终 verdict；Official 结果与本地结论的差异已记入 `调度/本地线上校准.tsv`（`LOCAL_REJECTED_BUT_OFFICIAL_DECENT` / CONTRADICT） |
| V002 | NO_UB_BUDGET，未建源码 | **闭环** | `本地实验/COEFF-LOCALITY-X/V002/NO_UB_BUDGET.md` 完整记录 UB 算术与 host clamp 事实；按审批 stop rule 停止，未产生 V002 SOURCE_SHA，无遗留待办 |

**V001 唯一残留科学缺口（非流程缺口）**：V001 的回归混杂了 staging 副作用（tileElems 4096→2560，tileCount 8→13）。`local-result.json` 自己写明：param prefetch 机制在常量 tileElems 下从未被干净证伪。1x16384（tileElems 保持 4096）只有 +1.8% 噪声，样本不足以判定。本文件的 NH-2 就是补上这块常量 tileElems 证据，不是重跑 H1。

---

## 1. 只读 UB budget / load-map 复查（FROZEN R31B-V011）

复核对象：`线上结果/R31B/V011/submission.asc`（父源码，行号以该文件为准）。
预算常量：`kWideFullYBudgetBytes = 176 KiB = 180224 B`（`:1281`，注释：192 KiB UB − 8 KiB system reservation）；`kWideFullYTileElems = 4096`（`:1283`）；`kWideFullYReduceStride = 16`（`:1284`）。

### 1.1 全部 gamma/bias 访问点

| # | 位置 | 触发条件 | 加载粒度 | 复查结论 |
|---|---|---|---|---|
| 1 | `Process()` generic，`cacheParams=true`（`:245` 判定，`:249-281` 加载） | D≤8192 ∧ localRows>1 | 每核一次，整行进 `gammaBuf_`/`biasBuf_`（`:121-122`，kCacheElems=8192） | 已最优。缓冲区按 tile 循环 Load（`:253-259`）；可合并为整行单次 Load，属微优化 |
| 2 | `Process()` generic，`cacheParams=false`（`:390-391`） | localRows==1 且 tileCount>1 | 每 tile 一次，落在输出通道内 | **仍有合法空间**。`gammaBuf_`/`biasBuf_` 在 Init 已按 8192 分配，与 `cacheParams` 无关；generic 路径 tileElems 固定 `kTileElems=4096`，不走 `ChooseWideFullYRows`，预加载整行**不增加 UB、不改 tileElems、不增加 MTE2** |
| 3 | `ProcessNarrowMidOverlap`（`:515-517` resident，`:539-541` 非 resident） | 128<D≤4096 | localRows>1 每核一次；否则每行一次 | host clamp 下 localRows=1，每行一次 = 每核一次，字节无冗余。仅 MTE2 发射顺序（param 排在 x/residual 之后）有微小重排空间 |
| 4 | `ProcessFp32/16/Bf16FullRowOutputPipelined`（`:1127-1131` / `:1008-1012` / `:887-892`） | D==8192 ∧ localRows>1 | 每核一次 | 已最优 |
| 5 | `ProcessSmallFp32*Batched` / `ProcessSmallLowPrecision*`（`:1382` `:1467` `:1582` `:1679`） | 小 D，多行 | 每核一次 | 已最优 |
| 6 | `ProcessBf16/Fp16FullTileBatchedOutputPipelined`（`:633-634` / `:761-762`） | D==4096 ∧ localRows>1 | 每核一次 | 已最优 |
| 7 | **`ProcessWideFp32FullCacheRows` pass 2（`:2179-2193`）** | D>8192 FP32 | 每 tile 每 batch 一次；`xBuf_`/`residualBuf_` 被别名为 gamma/bias staging | **字节层无冗余**（host clamp 下 localRows=1，batch 循环只跑一次；每 tile 参数只被用一次）。剩余空间只在**发射时序**（见 NH-2），不在 cache/驻留 |
| 8 | `ProcessWideLowPrecision`（2-deep prefetch，约 `:3251-3296`） | D>8192 FP16/BF16 | 每 tile，已双缓冲 | 已是 in-kernel 最优形态；再动就是同步/多行边界 |

死代码（不构成机会）：`ProcessWideFp32CachedRows`（`:1856`）、`ProcessWideFp32PanelResident`（`:2248`）、`ProcessWideFp32Batched`（`:2383`）在 `ProcessWideFp32`（`:1851-1853`）里没有被分发。切换分发属于 wide 架构/模式选择，越界。

### 1.2 UB budget 算术（tileElems=4096 钉死，FP32 wide）

`ChooseWideFullYRows(rowWidth, 4, ioTiles=2, 4, workTiles=0, tileElems)`（`:69-72`）。ioTiles=2 = `xBuf_`+`residualBuf_` 各一个 tile。

| D | rows | y bytes | io | reduce | need | leftover | K=1 配对 stripe 32768 B | 单 gamma tile 16384 B |
|---|---|---|---|---|---|---|---|---|
| 32768 | 1 | 131072 | 32768 | 64 | 163904 | **16320** | 放不下 | 差 64 B |
| 16384 | 2 | 131072 | 32768 | 128 | 163968 | **16256** | 放不下 | 差 128 B |
| 12288 | 2 | 98304 | 32768 | 128 | 131200 | 49024 | 放得下 | 放得下 |
| 10240 | 3 | 122880 | 32768 | 192 | 155840 | 24384 | 放不下 | 放得下 |
| 9216 | 3 | 110592 | 32768 | 192 | 143552 | 36672 | 放得下 | 放得下 |

结论（与 V002 一致，本轮复核确认）：

1. 在目标大 D 形状（32768 / 16384）上，tileElems=4096 时**不存在**任何 K≥1 的 gamma/bias 驻留空间。差额 64 B / 128 B 正好被 `reduceFp32Buf_`（rows×16×4 B）吃掉。把 reduce partials 挪出 UB 理论上能凑出 16384 B，但那要改归约侧结构并引入 S 模式同步 —— 跨 REDUCE-HIER-X 边界且叠加第二机制，OFAT 不允许。
2. D=12288/9216 能放下 K=1，但 host 启动把 `blockCount` 夹到 `rowCount`，测量形状 localRows=1，batch 循环一次，stripe 加载后永远不会被再次命中，收益恒为 0。
3. 结论：**wide FP32（site 7）在 cache/驻留维度已经没有合法空间**。H2 的失败不是实现问题，是预算与 host 约束的联合结果。

### 1.3 还有合法空间的位点小结（不抢 tile 预算）

| 位点 | 空间类型 | UB 增量 | tileElems 影响 | MTE2 变化 |
|---|---|---|---|---|
| Site 2（generic，localRows==1，tileCount>1） | 整行预加载 / cache 复用 | **0**（缓冲区已存在） | **0**（固定 kTileElems） | **减少**（2×tileCount → 2） |
| Site 7（wide FP32 pass 2） | 发射时序（split-phase 复用现有两个 staging slot） | **0** | **0**（钉在 4096） | 次数不变（每 tile 仍是 2） |
| Site 3（NarrowMid 非 resident） | MTE2 发射顺序 | 0 | 0 | 次数不变 |
| Site 1 预加载块 | 描述符合并（整行单次 Load） | 0 | 0 | 减少 |

---

## 2. 明确排除（本轮不作为假设）

| 排除项 | 理由 |
|---|---|
| H1 重跑（2-deep staging / 2-slot gamma+bias） | V001 已 LOCAL_REJECTED：staging 抢 UB 致 tileElems 4096→2560、tileCount 8→13，+19.2%。任何形式的**新增 staging slot** 都复现同一预算问题 |
| H2 无预算路径（K-tile stripe 驻留） | V002 NO_UB_BUDGET：D=32768/16384 在 tileElems=4096 下 leftover 16 KiB < 32 KiB；host clamp 使 localRows=1，无跨 batch 命中窗口 |
| multi-row mode / rows-per-block / blockCount 策略 | MAIN-1（MULTIROW-DMA-CHAMPION-X）边界，越界即 STOP |
| wide 分发切换（改走 PanelResident / CachedRows / Batched） | wide 架构/模式选择，越界且多机制 |
| 把 1x8192 放宽进 `ProcessFp32FullRowOutputPipelined`（`:178` 的 `localRows>1` 门槛） | 分发/模式改动，带入 x/residual prefetch 与输出流水等第二机制 |
| 同步消除作为性能变量 | 路线 brief 明文禁止；所有假设必须保留既有 Sync/Flag 语义 |
| store / epilogue / row scheduling / reduction 拓扑 | 各有归属路线（EPILOGUE / SCHED / REDUCE-HIER-X），不重叠 |

---

## 3. 下一轮假设（均通过 UB budget 与 tile-shrink 审查）

### NH-1 — Generic 单行多 tile 整行参数预加载（= H3 精化）

- **MECHANISM**：`Process()` 中 `cacheParams = cacheRow && localRows > 1`（`:245`）把单行核排除在外，导致 localRows==1 且 tileCount>1 时在输出通道里逐 tile `Load(gamma/bias)`（`:390-391`）。把条件改为 `cacheRow && (localRows > 1 || tileCount > 1)`，复用**已存在**的预加载块（`:249-281`，含 FP32 直载、FP16 直载、BF16 `ToFloat` 到 `gammaFp32Buf_`/`biasFp32Buf_` 三条既有分支）。预加载块内部把逐 tile 循环改成整行单次 `Load(..., 0, rowWidth)`（缓冲区 8192 覆盖 rowWidth≤8192）。
- **EXPECTED_BOTTLENECK**：generic 输出通道里 2×tileCount 个暴露的 param MTE2 描述符 + 每 tile 一次 `SyncMTE2ToV`，处在关键路径上。
- **FILES/FUNCTIONS**：`Process()` 的 `cacheParams` 判定（`:245`）与预加载块（`:249-281`）；`cacheParams` 消费点（`:387` `:393` `:417` `:430` `:442` `:464`）语义不变。不碰 host / CMake / runner / 其他 process 函数。
- **UB budget 审查**：**PASS，增量 0**。`gammaBuf_`/`biasBuf_` 在 generic Init（`:121-122` FP32、`:126-127` FP16、`:137-138`+`:154-155` BF16）无条件按 kCacheElems/kTileElems 分配，与 `cacheParams` 取值无关。
- **tile-shrink 审查**：**PASS，增量 0**。generic 路径不调用 `ChooseWideFullYRows`，tileElems 恒为 `kTileElems=4096`。
- **额外 MTE2 审查**：**PASS，交易数减少**（2×tileCount → 2），无新增事务。
- **WHY_NOT_DUPLICATE**：不是 REDUCE-HIER（不动归约）、不是 EPILOGUE/SCHED、不是 BATCH-RESIDENT（不引入跨行 batch DMA）、不是 V001（不加 staging）。是 site 2 的条件修正 + 描述符合并。
- **EXPECTED_WIN_SHAPES**：1x6144、1x8192、2x8192（FP32/FP16/BF16，localRows=1 且 tileCount=2）。**不覆盖** large-D（D>8192 走 wide），不覆盖 1x32768/1x16384/8x32768。
- **EXPECTED_RISK_SHAPES**：D≤4096（tileCount=1，无变化）、localRows>1（原本已缓存）。
- **CORRECTNESS_RISK**：很低。预加载块已在多行分支验证；唯一要对齐的是 `cacheParams=true` 时输出通道按 `gammaLocal[col]` 索引（`:430-453` 已有该分支），以及 BF16 的 `cacheParamFp32` 分支。
- **MINIMAL_OFAT_DIFF**：一个条件 + 预加载块内循环合并为整行 Load。
- **EXPECTED_LOCAL_PROBES**：1x8192 FP32 主探针，1x6144 FP32 副探针，1x4096 FP32 控制（期望 ≈0）。协议按 `本地性能测试规范.md`（warmup≥45、样本≥21、交错 P/C ≥4 组）。
- **成熟度**：`READY_FOR_MAIN_REVIEW`（但天花板小，见第 4 节）

---

### NH-2 — Wide FP32 pass 2 split-phase 参数发射（常量 tileElems 的 param 时序检验）

- **MECHANISM**：`ProcessWideFp32FullCacheRows` pass 2 里 `xBuf_`/`residualBuf_` 被别名为 gamma/bias staging（`:2179-2180`）。当前序是每 tile `Load(g,b) → SyncMTE2ToV → Muls → Mul(g) → Add(b) → Store`。`Mul(g)` 结束后 `xBuf_` 即死，`Add(b)` 结束后 `residualBuf_` 即死。把下一 tile 的 gamma 发射点提前到本 tile 的 Mul 之后、bias 发射点提前到本 tile 的 Add 之后，**只用现有两个 slot 轮转**，不新增 staging。Sync/Flag 语义保留（每次 apply 前仍 `SyncMTE2ToV` / WaitFlag），MTE3 store-drain 等待保留。
- **EXPECTED_BOTTLENECK**：V001 指向的暴露 param MTE2 延迟。V001 因 staging 抢 UB 失败，从未在常量 tileElems 下干净检验过该假设。本假设就是那个缺失的检验。
- **FILES/FUNCTIONS**：`ProcessWideFp32FullCacheRows` pass 2 循环（`:2174-2234`）内部发射点重排。不改 Init、不改 `ChooseWideFullYRows`、不改 ioTiles、不改 tileElems、不改 host/runner。
- **UB budget 审查**：**PASS，增量 0**。零新缓冲区；两个 slot 本来就在。
- **tile-shrink 审查**：**PASS，增量 0**。不进 `ChooseWideFullYRows` 的预算项，tileElems 钉在 4096（D=32768 → tileCount=8）。
- **额外 MTE2 审查**：**PASS，次数不变**（每 tile 仍是 gamma+bias 两次），只是提前发射。
- **WHY_THIS_IS_NOT_H1_RERUN**：H1 = 新增 2-slot staging（ioTiles 2→4），直接导致 tileElems 下降。本假设**不增加任何 staging 字节**，只改现有两个 slot 上的发射时点。它测的是「param MTE2 时序是否重要」，补的是 V001 明文记录的常量 tileElems 证据缺口，不是重做 H1 的实现。若 Main 判定这仍算 H1 范畴，按排除表处理即可，不进 Revision。
- **EXPECTED_WIN_SHAPES**：1x32768 FP32（tileCount=8，暴露往返最多）、1x16384 FP32（tileElems 不变的对照，V001 此处只测到 +1.8% 噪声）、8x32768 FP32。
- **EXPECTED_RISK_SHAPES**：tileCount=1 的形状无收益；若 apply 本身已遮蔽 MTE2，收益 ≈0。
- **CORRECTNESS_RISK**：中低，需实现前逐项核：(a) `Mul` 后 `xBuf_` 确实不再被读；(b) 现有 MTE3_V drain（`:2183-2190`）保护的是 store 侧，store 源是 `valueLocal`（`:2224-2226`）而非 staging pair，drain 等待保持原位不动；(c) 每 tile apply 前的 `SyncMTE2ToV` 保留，不删任何 Flag。
- **MINIMAL_OFAT_DIFF**：pass 2 循环内两个 Load 的发射位置，无其他改动。
- **EXPECTED_LOCAL_PROBES**：1x32768 FP32 主探针（同 V001 判定协议）、1x16384 FP32、8x32768 FP32、1x4096 FP32 控制。判据：若 1x32768 在常量 tileElems 下 |Δ|≤噪声，param MTE2 时序正式出局，COEFF 轴 large-D 维度可以收口。
- **成熟度**：`READY_FOR_MAIN_REVIEW`

---

### NH-3 — NarrowMid 参数 MTE2 发射顺序（与 x/residual 并行）

- **MECHANISM**：`ProcessNarrowMidOverlap` 非 resident 分支（`:536-541`）先发 x/residual 再发 gamma/bias，param 的 MTE2 排在输入之后。把 param 对与输入对作为同一次 MTE2 突发发射（或 param 提前），用既有 `paramReady` 事件，让参数延迟与输入延迟重叠而不是串行。
- **EXPECTED_BOTTLENECK**：单 tile 路径上 param MTE2 串行在输入 MTE2 之后的暴露延迟（一次往返）。
- **FILES/FUNCTIONS**：`ProcessNarrowMidOverlap` 发射顺序（`:536-541`）。零新缓冲区、零 Flag 删除。
- **UB budget 审查**：PASS，增量 0。
- **tile-shrink 审查**：PASS，增量 0。
- **额外 MTE2 审查**：PASS，次数不变。
- **WHY_NOT_DUPLICATE**：不是 NH-1（那是 cache 语义）、不是 NH-2（那是 wide 的 split-phase）；只动 site 3 的发射顺序。
- **EXPECTED_WIN_SHAPES**：D∈(128, 4096] 且 localRows=1 的形状。天花板极小（一次往返）。
- **CORRECTNESS_RISK**：低；事件配对必须保持（`inputReady`/`paramReady` 语义不变）。
- **MINIMAL_OFAT_DIFF**：Load 语句顺序。
- **成熟度**：`NEEDS_MORE_EVIDENCE`（收益可能落在噪声内，建议排在 NH-1/NH-2 之后或不单独开 Revision）

---

### NH-4 — 预加载块描述符合并（site 1 微优化，可并入 NH-1 实现）

- **MECHANISM**：`cacheParams=true` 预加载块用 `for col += kTileElems` 逐 tile `Load`（`:253-259`）。rowWidth≤8192=kCacheElems 且对齐时可整行单次 `Load(gammaLocal, gammaGm_, 0, rowWidth)`，描述符 2×tileCount → 2。
- **UB budget / tile-shrink / 额外 MTE2**：全部 PASS（0 / 0 / 减少）。
- **EXPECTED_WIN_SHAPES**：localRows>1 的 D≤8192 形状；预期落在噪声内或极小。
- **成熟度**：`DUPLICATE`（建议作为 NH-1 实现细节带上，不单独占一个 Revision 的单变量名额）

---

## 4. H3 专项判断

H3 = NH-1。registry 记 “ceiling small”，本轮复查支持这个判断，但**不建议因此放弃**，而是调整定位：

**结论：H3 技术上站得住、天花板确实小。不建议作为主攻假设；建议作为收口探针或 NH-2 之后的顺手项。**

理由：

1. **能过审**：site 2 是 load-map 上唯一还有免费 cache/reuse 空间的位点（缓冲区已存在、tileElems 不受预算函数支配、MTE2 变少）。H2 被预算钉死、H1 被 staging 钉死之后，H3 是仅存的不碰 tile 预算的 cache 项。
2. **天花板小的原因是形状覆盖**：它只作用于 D∈(4096, 8192] 且 localRows=1 的 generic 路径（典型 1x8192 / 1x6144 / 2x8192）。Official case 14（与最优差距约 4.4× 的那条）是 large-D 形状，走 wide FP32（site 7），H3 完全够不着。所以 H3 解释不了主差距。
3. **价值定位**：(a) 便宜且零预算风险，一次干净的小胜探针；(b) 若 NH-2 把 site 7 也证伪，H3 是 COEFF 轴唯一还能留下本地正收益的机制；(c) 不做 H3，轴内在 generic 维度就没做过实验，闭环不完整。

**替代主攻（推荐顺序）**：**NH-2 优先**（补常量 tileElems 证据，直接打 large-D），**NH-1 次之**（收口 generic），NH-3/NH-4 不单独开 Revision。

---

## 5. 推荐与请求

推荐 Main 下一批只批一个：

**首选 NH-2**（wide FP32 split-phase 参数发射，常量 tileElems 的 param 时序检验）。
**备选 NH-1**（generic 单行多 tile 整行预加载，= H3 精化）。

- ROUTE: COEFF-LOCALITY-X
- REVISION: V003（由 Main 批准后才创建）
- DIRECT_PARENT: FROZEN_R31B_V011
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- PARENT_SCORE: 45.16（OFFICIAL_ANCHOR）
- CONTEXT_CLASS: FROZEN_STRONG_BASELINE_COEFF_LOCALITY

在 Main 发出明确批准之前：不改 Kernel、不建 V003、不跑正式 performance、不提交 Online。
