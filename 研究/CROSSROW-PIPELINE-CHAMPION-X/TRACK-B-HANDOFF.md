# CROSSROW-PIPELINE-CHAMPION-X · Track-B 研究交接

## 状态

- ROUTE: `CROSSROW-PIPELINE-CHAMPION-X`
- MAIN_SELECTED: `NONE`
- 研究结论: `ROUTE_HYPOTHESIS_POOL_EXHAUSTED`
- 有效且未重复的方向: `0`
- DIRECT_PARENT: `R31B V011 Official 45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Bootstrap: 项目规则、路线资料、调度与校准、父版本及已提交相关证据已读；工作树为 `w2/m2/crossrow`，起点 `ed860e392d7604694ac6664da60aff1fc1f4c04f`。
- 本轮只读调研；没有构建、正确性、计时、剖析或远端设备操作。

## ROUTE_BOUNDARY

只研究固定到同一 AI Core 的相邻行 N 与 N+1 之间，是否能让两个不同处理阶段并行。行到核的分配保持父版本原样；不得将列切分、行所有权或 block 映射变化纳入假设。候选方向必须比父版本已有的 `(row, tile)` MTE2/V 流水多出独立的跨阶段作用点。

## ALLOWED_CHANGES

本 Route Agent 只写 `研究/CROSSROW-PIPELINE-CHAMPION-X/` 下的研究与交接文档。后续实现需 Main-2 选定方向并另行授权；当前 `MAIN_SELECTED=NONE`。

## FORBIDDEN_CHANGES

不得改 Kernel/Candidate 或 `.asc`、`.cpp`、`.h`、`.hpp`、CMake、runner、wrapper；不得创建 Revision；不得构建、运行正确性、计时或剖析；不得访问 server3、提交 Online、改共享记录、改生命周期决定或路线池；不得读取其他 Route Agent 的私有上下文或工作树。

## 父版本流水事实

父版本 `ProcessWideLowPrecision` 的 Pass-1 已把 batch 内工作展平成 `(row, tile)` 单元，并以两个输入槽交替发出 MTE2。当前单元的 V 计算期间会加载下一个单元；当下一个单元属于 row N+1 时，这已经覆盖了“row N 的 Pass-1 V 与 row N+1 的 Pass-1 MTE2”这一组合。Pass-1 结束后逐行完成 RMS 标量处理，再进入 Pass-2。Pass-2 对 tile 做双槽 gamma/bias MTE2，并在每次输出 Store 后同步 MTE3 到 V。现有 Parent 因而留下的候选空间主要在 Pass-1/Pass-2 边界，但该边界的相邻行流水已有旧研究和 Wave-2 尝试。

## 假设审计

以下三项分别覆盖不同阶段组合；三项均为实际重复，不能作为本 Route 的新方向。每项仍记录完整字段，供 Main-2 对照。

### CX-H01 — row N epilogue 与 row N+1 prologue

- HYPOTHESIS_ID: `CX-H01`
- MECHANISM: 在 row N 的归一化、gamma/bias 与输出阶段期间，启动同一 core 上 row N+1 的首块输入 MTE2；保持每行算术次序不变。
- BOTTLENECK: 当前逐行串行结构让输出阶段和下一行输入准备不能重叠。
- DIRECT_PARENT: `R31B V011 Official 45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: 多行且每核至少两行；先前记录提及 `64×256 FP32`、`33×100 FP32`、`8×8192 FP32`。
- TARGET_DTYPES: `FP32`；迁移到父版本低精度宽行路径时只考虑 `FP16/BF16` 且 `localRows>=2`。
- WHY_IT_MAY_HELP: 输入 MTE2 与 row N 的 V 运算属不同流水单元；可覆盖逐行边界上的空档。
- WHY_IT_MAY_FAIL: 宽低精度路径保留整批 y，并复用临时区；输入、参数、输出的存储生命周期可能冲突。同步等待和数据搬运争用也可能吞掉收益。
- UB_IMPACT: 通用路径可能沿用分离的输入/输出槽；宽低精度路径需先证明 `gammaBuf_`、`valueFp32Buf_`、`xBuf_`、`residualBuf_` 在两行并行时无重叠写读。若需额外行缓存，容量和可行性未知。
- DMA_IMPACT: 总字节数不变；MTE2 提前发出，可能与当前行输出 Store 竞争 HBM。
- SYNC_IMPACT: 高；需分别保证下一行输入完成后才读取，并保证当前行输出被 MTE3 取走前不复用来源槽。
- PRECISION_RISK: 算术次序不变时数值风险低；缓冲复用错误会造成跨行数据错配，属于正确性风险。
- DUPLICATE_CHECK: `DUPLICATE_REJECTED`。`研究/NEXT-TRACK-B-OVERNIGHT.md` 的 `EPI-PIPE-3` 已明确提出当前行 epilogue 与下一行 pass-1 prologue 重叠，且给出相同多行形状。把实现点迁到 `ProcessWideLowPrecision` 不改变阶段组合；独立价值不足。
- RELATED_OLD_ROUTES: `R013/FULL-R013` 行内双缓冲；`R015/FULL-R015` 多行 DMA；`EPI-PIPE-3`；`ASYNC-TRIPLE-X H4`；`ASYNC-OVERLAP-CHAMPION-X V002`。
- MINIMAL_EXPERIMENT: 不运行。既有 `EPI-PIPE-3` 已是同机制候选；若 Main-2 要求重新评估，先在文档层证明低精度路径的槽位生命周期与既有方案不同，再决定是否需要单形状正确性和配对计时。

### CX-H02 — row N invRms 尾段与 row N+1 输入预取

- HYPOTHESIS_ID: `CX-H02`
- MECHANISM: 在 row N 的 invRms V/S 尾段期间，提前发出 row N+1 的 x/residual 首块 MTE2，并将等待放到首次读取该块之前。
- BOTTLENECK: 每行 scalar handoff 与下一行冷启动输入串行。
- DIRECT_PARENT: `R31B V011 Official 45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: 已有 Wave-2 记录的 `128×2048 FP16` 与 `128×4096 FP32`。
- TARGET_DTYPES: `FP16`、`FP32`；同机制也可能适用于 `BF16`，但未见对应记录。
- WHY_IT_MAY_HELP: 输入搬运可与标量/V 尾段重叠，减少行间冷启动空档。
- WHY_IT_MAY_FAIL: invRms 尾段很短，窗口可能小于计时噪声；x/residual 槽还可能与输出参数暂存重用。
- UB_IMPACT: 理论上不增加槽位；必须确认预取目标槽在首次消费前不会被当前行输出阶段覆盖。
- DMA_IMPACT: 总流量不变；改变下一行输入 MTE2 的发出时点，可能形成短时带宽争用。
- SYNC_IMPACT: 中高；MTE2→V 等待需绑定正确 row，不能只依赖原先的行内事件状态。
- PRECISION_RISK: 算术不变时数值风险低；事件错配会读到错误行数据。
- DUPLICATE_CHECK: `DUPLICATE_REJECTED`。`技术路线/全版本记录.tsv` 的 `ASYNC-OVERLAP-CHAMPION-X V002` 已记录 W2 NarrowMid 跨行 issue reorder：在 invRms 尾段前提早发出下一行 x/residual，并延后 MTE3→V 等待；其目标形状与本假设相同，记录结论为 `NEEDS_ONE_MORE_LOCAL` 且配对方向混合。路线机制已覆盖。
- RELATED_OLD_ROUTES: `ASYNC-OVERLAP-CHAMPION-X V002/V003`；`R31B V017` 的 MTE3 等待延后；`MIX-A V003/V007` 的本地槽复用同步。
- MINIMAL_EXPERIMENT: 不运行。Wave-2 已有此 OFAT 方向和形状记录；重复计时不能产生新机制价值。

### CX-H03 — row N MTE3 Store 与 row N+1 输出 V 前缀

- HYPOTHESIS_ID: `CX-H03`
- MECHANISM: row N 的输出 Store 发出后，先执行 row N+1 输出块中不依赖 Store 完成的 V 前缀，等到复用同一个输出槽前再等待 MTE3。
- BOTTLENECK: 每次 Store 后的 MTE3→V 同步把异步写回延迟暴露在后续输出计算前。
- DIRECT_PARENT: `R31B V011 Official 45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: `rowWidth>8192` 且每核至少两行；优先看 `128×16384` 的低精度宽行批次。
- TARGET_DTYPES: `FP16/BF16`。
- WHY_IT_MAY_HELP: 让当前行 Store 与下一行 V 前缀并行，隐藏 MTE3 延迟；不改变输出算术。
- WHY_IT_MAY_FAIL: V 前缀工作可能太短；输出暂存区只有一个 tile，过早复用会覆盖在途 Store 的源数据。旧正式结果也显示该时序变化未带来 Official 提升。
- UB_IMPACT: 保持现有单输出槽时不增加 UB；需要证明 Store 完成之前该槽不被下一行重写。
- DMA_IMPACT: Store 字节数和输入流量不变；MTE3 与后续 V 更重叠，HBM 吞吐未必改变。
- SYNC_IMPACT: 高；MTE3→V 等待必须准确落在输出槽下一次写入之前，最终 Store 仍需完成。
- PRECISION_RISK: 算术次序不变时数值风险低；槽位过早复用会造成正确性错误。
- DUPLICATE_CHECK: `DUPLICATE_REJECTED`。`线上结果/R31B/V017/diff.patch` 已把 Store 完成等待延后到输出槽重写前，恰好让在途 Store 覆盖后续 V 前缀；`R31B V017` 的 Official 结果为 `44.68`，低于父版 `45.16`。`R31A V021` 只把等待移至同一行 tile 循环末尾，未跨行；它是相邻证据，不能单独算作此跨行机制。`研究/ASYNC-TRIPLE-X/next-hypotheses.md` 的输出-ring 讨论也已覆盖同一等待位置问题。
- RELATED_OLD_ROUTES: `R31B V017`；`R31A V021`（同一行等待延后）；`ASYNC-TRIPLE-X H2`；`MIX-A V003/V007`（V_MTE2 槽位释放，不同事件方向）。
- MINIMAL_EXPERIMENT: 不运行。V017 已完成该同步时序方向的正式评估并低于父版；本 Route 不复测。

## DUPLICATE_CHECK 汇总

| 范围 | 已提交机制 / 证据 | 与本 Route 的关系 | 实质差异与独立价值 |
|---|---|---|---|
| R001–R029 idea pool | `归档/历史控制文件/idea-pool-29-routes.md` 汇总 R001–R029；R013 是双缓冲，R015 是多行 stride DMA，R016 是行到核分配，R028 是标量同步削减 | R013 是相邻的流水证据；R015 改 DMA 形状；R016 改行所有权，违反边界；R028 改归约/标量同步 | R013 的已试部分是行内 MTE2/V 双缓冲，不是新的跨 Pass 行间机制。R015 的 FULL 记录 1/15 Runtime Error，且不是本 Route 的阶段交叠。R016 即使可行也不得改映射。其余 idea pool 项目没有本 Route 所需的相邻行阶段交叠机制。 |
| FULL | `FULL-R013` 15/15、18.76；`FULL-R015` 多行 DMA 为 1/15 Runtime Error；`FULL-R016` 是行/核调度并由 SCHED-ROWGROUP-X 续接，见 `技术路线/技术路线总表.md` | 分别是行内流水、多行数据搬运、所有权调度 | `FULL-R013` 的 input double-buffer 与父版现有 `(row,tile)` Pass-1 流不同点已被父版吸收；`FULL-R015` 不是阶段流水；`FULL-R016` 属禁止变更。没有剩余独立方向。 |
| R31A/R31B | 父版 Pass-1 已跨 `(row,tile)` 双槽 MTE2/V；V017 延后输出槽 Store wait，Official 44.68；V021 把 MTE3 wait 移到本行循环末；V006/V009 的队列深度改动在 T14 平 | CX-H03 与 V017 重合；V021 是同一行内变化；队列深度不是候选方向 | V017 已直接测试 MTE3 写回等待位置，且 Official 低于父版。V021 到行末仍先完成当前行 Store 才进入下一行，不能证明跨行提升。加深队列无独立价值。 |
| MIX | `线上结果/MIX-A/V003/diff.patch` 为输入槽复用加 V→MTE2 同步，并限制 `localRows==1`；`本地实验/MIX-A/V007/diff.patch` 在单行条件下移除首个 V→MTE2 等待 | 与跨行输入/输出可能共享槽位的同步安全相关 | V003 修复的是多行 buffer reuse race，V007 只测单行首载入等待；二者没有提出新行间阶段流水。它们支持 CX-H01/H02 的槽位风险判断，独立性能方向为零。 |
| Wave-1 | `研究/NEXT-TRACK-B-OVERNIGHT.md` 的 EPI-PIPE-3；`研究/ASYNC-TRIPLE-X/next-hypotheses.md` 的 H4 inter-row continuity | EPI-PIPE-3 与 CX-H01 同机制；H4 将 Pass-2 N 与 Pass-1 N+1 连续交叠 | EPI-PIPE-3 已点名同样的行阶段和形状。H4 已记录共享深度 2 输入队列与行状态风险，且被搁置；扩展实现位置不构成新假设。 |
| Wave-2 | `技术路线/全版本记录.tsv` 的 ASYNC-OVERLAP V002–V004；V002 是跨行预取，V003 是 GetValue handoff，V004 是同一行 FullCache prologue 参数预取 | V002 与 CX-H02 重复；V003/V004 是相邻同步或同一行重叠 | V002 记录为混合结果、`NEEDS_ONE_MORE_LOCAL`；V003 调整标量交接，V004 调整同一行参数载入时点，都不形成新的 row N/row N+1 阶段对。HEAD 中 Wave-2 V002–V004 完整包未出现；本审计只引用已提交总版本记录，没有读取任何其他 Route 的私有目录。 |

## ROUTE_HANDOFF

- PROPOSED_ONE_FACTOR_DIFF: `NONE`。CX-H01、CX-H02、CX-H03 都已在上表所列证据中出现；父版本也已实现 Pass-1 相邻行输入/计算重叠。没有能同时满足独立性与 ROUTE_BOUNDARY 的新单因素差异。
- EXPECTED_LOCAL_PROBES: 本轮 `NONE / NOT_RUN`。不构建、不跑正确性、不计时、不剖析，也不访问 server3。现有记录中的多行形状只用于去重，不作为本 Route 的新测量建议。
- CHILD_RECOMMENDED_HYPOTHESIS: `NONE`
- OPEN_QUESTIONS: 若 Main-2 希望继续此 Route，需要新的已提交证据指出一个尚未覆盖、且不改行到核分配的跨行阶段对；在此之前保持 `MAIN_SELECTED=NONE`，不创建 Revision。

## EVIDENCE_PATHS

- 启动规则：`AGENTS.md`、`.agents/skills/cann-mainline/SKILL.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`、`项目规则/本地性能测试规范.md`、`项目规则/服务器实验规范.md`、`项目规则/Git工作流程.md`、`项目规则/线上提交规范.md`。
- 路线与调度：`技术路线/技术路线总表.md`、`技术路线/技术路线图.md`、`技术路线/路线成绩表.tsv`、`技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`调度/本地线上校准.tsv`。
- 父版本身份与实现：`线上结果/R31B/V011/submission.asc`、`线上结果/R31B/V011/source-meta.json`、`线上结果/R31B/V011/submission.sha256`、`线上结果/R31B/V011/result.json`。
- 旧路线：`归档/历史控制文件/idea-pool-29-routes.md`、`归档/phase3-before-reset-20260920/管理/路线状态/R013.json`、`归档/phase3-before-reset-20260920/提交/外部轨道-ChatGPT编译并修复/源码/R013-V001/结果.md`、`归档/历史阶段/historical-branches-20260924/independent__full-r015-multi-row-dma-i001/files/提交/独立实现/FULL-R015-MULTI-ROW-DMA/I001/README.md`。
- R31 与 MIX：`线上结果/R31B/V017/diff.patch`、`线上结果/R31B/V017/result.json`、`线上结果/R31B/V017/source-meta.json`、`本地实验/R31A/V021/diff.patch`、`本地实验/R31A/V021/handoff.md`、`线上结果/MIX-A/V003/diff.patch`、`本地实验/MIX-A/V007/diff.patch`。
- Wave-1/Wave-2：`研究/NEXT-TRACK-B-OVERNIGHT.md`、`研究/ASYNC-TRIPLE-X/next-hypotheses.md`、`研究/主代理/MAIN-2/初始化报告.md`、`研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`、`技术路线/全版本记录.tsv` 中 `ASYNC-OVERLAP-CHAMPION-X V001–V004` 记录。

## 交接结论

`ROUTE_HYPOTHESIS_POOL_EXHAUSTED`。目前三个可描述的跨行阶段组合均已重复；父版本已有 Pass-1 row/tile 双槽流水。没有推荐假设、候选 Kernel 或 Revision；等待 Main-2 或 Planning 提供新的已提交证据或新的路线边界。
