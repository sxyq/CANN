# SELECTIVE-FASTPATH-CHAMPION-X — Track-B Handoff

STATUS: RESEARCH_COMPLETE; MAIN_SELECTED=WAITING
H1_QUALIFICATION: QUALIFICATION_INCOMPLETE
DIRECT_PARENT: R31B V011
OFFICIAL_ANCHOR: 45.16
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
V011_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
ROUTE_BOUNDARY: 每个候选只分派一个完整 donor；未命中输入精确回到 V011。不得组合 donor。本轮未选实现。
ALLOWED_CHANGES: 仅本文件内的 Track-B 研究与交接文字；MAIN_SELECTED=YES 前不实施任何候选。
FORBIDDEN_CHANGES: Kernel/Candidate、Revision、共享台账、技术路线记录、构建、正确性运行、测时、server3、Online；不访问其他 Agent 私有上下文或 worktree。

## 对照口径

证据只取本 worktree 可见的正式项目记录、已提交 source-meta/diff/result 与路线研究文件。Local 记录属于各 donor 自己的直接父版；它们不能替代候选 donor 对 R31B V011 的 P/C，也不能预测所有 Official case。15 个 Judge case 没有公开 shape/dtype 对照，所有 TARGET_SHAPES 都只是待验证的探针范围。

## 历史重复对照

| 范围 | 与本路线最接近的机制及结论 | 证据路径 |
|---|---|---|
| R001-R029：归约与标量 | R001 两遍扫描、R006 分块归约、R007 ReduceSum、R011 手工向量树、R019 invRms、R020 sqrt、R028 标量同步削减均改归约或 V/S 标量链。H1 改的是 MTE3 完成事件的等待位置；H3 改输出写回分块；H4 改逐元素 affine 算术。R028 的 GetValue/V-S 往返与 MTE3_V 等待不是同一个同步点。 | `技术路线/全项目成绩与技术路线盘点.md` §6、§6.1；`技术路线/技术路线总表.md` §一 |
| R001-R029：数据驻留、精度、tile、copy、调度 | R002 单遍 y 驻留、R004 低精度中间、R005 大 Tile、R009 DataCopyPad 尾块、R010 手工尾块、R012 32B 行组、R014 参数驻留、R015 多行 DMA、R016 行到核、R017 FP32 中间、R018 CAST_RINT、R029 wide cached-row 与目标形状或输出缓冲相邻；各自变化的是输入/参数复用、精度、tile/copy/所有权/整行缓存，不等同于 H1/H3/H4 的单个 donor 变化。R003、R021-R027 是入口、构建、生成、工程、验证、提交通道或迁移参考，不是可复用的算子优化 donor。R008 跨核 D-slice 也不在候选范围。 | `技术路线/全项目成绩与技术路线盘点.md` §6、§6.1；`技术路线/技术路线总表.md` §一 |
| FULL-R001-R029、FULL-R030 | 29 个 FULL 路线只有 Official 终态，无可与本假设同条件比较的 Champion Local P/C；FULL-R030 参数复用为额外路线且 4/15 Runtime Error。历史分数和失败状态不说明本候选在局部或全 workload 上的收益。 | `归档/phase3-before-reset-20260920/文档/当前状态.md`；`归档/phase3-before-reset-20260920/实验/online/`；`归档/phase3-before-reset-20260920/实验/online-independent/`；`技术路线/全项目成绩与技术路线盘点.md` §7 |
| R031、H00N、MIX | R031 是多模式集成参考，不归因单一机制。H001/H002 是多机制组合且没有可比的 Champion P/C；H003/H004 未实现。MIX-A V003 的 V_MTE2 释放及 localRows 限制、V007 的等待位置变化与本候选不构成同一 donor；V007 未通过 shape 资格，V004/V005 Official 低于 V003。组合或单点结果不外推至全 workload。 | `技术路线/全项目成绩与技术路线盘点.md` §7-§8；`技术路线/技术路线总表.md` §三-§六；`线上结果/MIX-A/V003/diff.patch`、`线上结果/MIX-A/V004/result.json`、`线上结果/MIX-A/V005/result.json`、`本地实验/MIX-A/V007/` |
| R31B lineage | V017 的局部结果以 V016 为父；本 handoff 只提议把完整 V017 donor 与 V011 直接比较。不得把 V016→V017 的百分比当作 V011→donor，也不得把 V016 与 V017 两段数字相加。 | `线上结果/R31B/V017/source-meta.json`、`diff.patch`、`result.json`；`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31B-V017.md` |
| R31A V028 | 不纳入候选。它删除 R31A V026 `ProcessWideFp32Batched` 的两个 PipeBarrier，目标为 D24576 batch；直接父不是 V011。已有本地信号相对 V026，Official 44.07 低于其 45.00 参照；移植到 V011 的收益、shape 覆盖和与当前 SYNC 项的边界均未证实。排除表示缺乏可比的 V011 证据，不表示机制已被证明无效。若 Main 要研究，需另行明确单一分派条件并先补 V011 对照。 | `线上结果/R31A/V028/source-meta.json`、`diff.patch`、`result.json`；`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V028.md`；`调度/本地线上校准.tsv` |
| Wave-1 Local-positive / Official-negative | ASYNC-OVERLAP V001、R31B V017、R31A V028、STORE V003、EPI V002 均被校准记录标为 Local 正向而 Official 低于提交参照。它们覆盖异质机制和形状，已直接说明 Local 强信号不足以推出整体 Official 提升；本 handoff 只规划精确 donor-vs-V011 探针，不把 Local 数字当预测值。 | `研究/主代理/MAIN-2/本地评分器校准.md` §1、§2、§8；`调度/本地线上校准.tsv`；各行对应的 `线上结果/<ROUTE>/<REV>/result.json` 与 `diff.patch` |

## 当前 Wave-2 名称对照

共享的 `调度/当前任务.tsv`、`调度/主代理分工.md`、版本/路线记录和 `研究/` 中没有带 `Wave-2 SYNC / STORE / EPI / SMALLMID` 的正式机制说明。本段只比较已公开的相邻路线与本任务列出的名称，不读取其他 Agent 私有上下文，也不替这些名称补写未见的实现细节。

| Wave-2 名称 | 相似处 | 差异与仍可独立验证的部分 | 未决边界 |
|---|---|---|---|
| SYNC | 与 R31B V017 延后 MTE3_V wait、R31A V028 删除 flag 前 PipeBarrier、MIX-A V007 调整 SyncVToMTE2 及 R013/ASYNC 的流水同步相邻。 | H1 仅针对完整 V017 donor 的 BF16 wide pass-2 输出事件等待；只有 Wave-2 SYNC 落在不同 pipe/调用点/shape 且不改写该 donor 时，才是独立单变量实验，可分离其作用。 | Wave-2 具体事件方向、函数、shape 条件和 donor 未出现在共享记录，不能判定是否实质重复。 |
| STORE | 与 STORE V002/V003 的整行合并、两段 chunk 写回直接相邻；H1 也涉及 store 完成时序。 | H3 是完整 STORE V003 donor 的两段写回布局/发出边界；V017 是 store 数量不变的完成等待移动。对不同函数或 shape 条件分别做完整 donor-vs-V011 P/C，可区分写回分块与等待重排，不组合两个 donor。 | 若 Wave-2 STORE 仍改同一 `ProcessWideFp32FullCacheRows` 条件及 K=2 分块，则重复；其实际范围待公开说明。 |
| EPI | 与 EPILOGUE-ARITH V001/V002 的整行 norm hoist、Mul+Axpy 顺序，以及 EPILOGUE-FUSE 的 affine 变换同属输出算术。 | H4 仅运行完整 EPI V002 的 FP32、`batchRows==1` affine 路径。不同算术位置或不同可命中的 workload 域可独立测量；不得将 Wave-2 EPI 机制拼入 H4。 | Wave-2 EPI 的运算次序、精度约束、函数和目标 shape 未在共享记录出现。 |
| SMALLMID | 与 MIX-A、BATCH-RESIDENT、ASYNC-OVERLAP V002 的多行/窄中宽路径及 R31A batch 路径有形状邻接。 | H1/H3/H4 当前列出的 donor 探针集中在 D=16384/32768，SMALLMID 若定义在窄/中宽且由独立机制覆盖，可填补这些 donor 未探的 shape 域；价值是取得该域上的独立 P/C 事实，不是从宽形状推断收益。 | 共享记录未给 SMALLMID 的具体边界、dtype、机制和候选源码；Official case 映射也缺失，不能主张它覆盖某一 Official case。 |

因此，当前三项候选彼此仍是不同的完整 donor 实验：V017 等待位置、STORE V003 写回分块、EPI V002 affine 算术。Wave-2 名称与其是否重复，须以 Main 提供的公开函数/条件/shape 边界审阅；本 handoff 不选其一，也不假设四类 Wave-2 都已证明独立。

## 共用证据

Judge 结果有 15 个 testcase ID、分数和通过状态，没有 shape/dtype 字段；`研究/OFFICIAL-CASE-ANALYSIS.md` 也说明精确映射未知。不能声称本地 shape 对应特定 Official case。

V011 fallback 证据：`线上结果/R31B/V011/submission.asc`、`submission.sha256`、`result.json`。

下列局部数据均来自 donor 自己的直接父版，不是 donor 对 V011 的同条件结果。若 Main 选择一项，最小性能探针须直接比较完整 donor 与 V011。所有 donor Official 均 15/15 PASS，但总分低于 45.16。

## H1 — BF16 wide store wait

HYPOTHESIS_ID: `H1-BF16-WIDE-V017`
CHILD_RECOMMENDED_HYPOTHESIS: `NONE`; child 不排序、不选择，等待 Main 明确 `MAIN_SELECTED=YES`。
MECHANISM: 在 BF16 D32768 上运行完整 V017 来源；将 pass-2 的 MTE3_V 等待延后到 staging tile 复用前。完整 donor 也继承 V016 tile 策略。
BOTTLENECK: 宽行输出中，逐 tile 排空 Store 限制写回与后续向量前缀重叠。
DIRECT_PARENT: R31B V011.
DONOR_SOURCE_SHA: `7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4` (R31B V017)
DONOR_LINEAGE_PARENT: R31B V016.
V011_FALLBACK_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: BF16 `M x 32768`; 局部记录未给 M。拟议条件为 `dtype==BF16 && rowWidth==32768`，M 范围尚未获局部证据支持。
TARGET_DTYPES: BF16.
WORKLOAD_COVERAGE: V017 局部 BF16 D32768 为 -14.3%、20/20，相对 V016；V017 Official 44.68（-0.48 vs 45.16）。
WHY_IT_MAY_HELP: 延迟等待可让 MTE3 Store 覆盖下一 tile 的向量工作。
WHY_IT_MAY_FAIL: 局部对比对象是 V016，M 未知；若 Store 不是关键路径或事件操作抵消重叠，收益可能消失。Official 总分低于锚点。
UB_IMPACT: V017 将低精度宽行 tile 初值从 4096 提到 8192，再按 176 KiB 预算收缩；新探针需记录实际 tile 和每核行数。
DMA_IMPACT: 字节数与 Store 数不变，变化仅在 MTE3/V 重叠。
SYNC_IMPACT: 以 MTE3_V event 延迟等待替代逐 tile 排空；需验证 staging 复用前及循环结束时均已等待。
PRECISION_RISK: 算术次序不变；风险在 event 生命周期、输出 staging 与数据竞争。
DUPLICATE_CHECK: 种子 FP16 项与本项是同一 V017 donor 和等待机制，不另列独立方向。V017 的 BF16/FP16 局部结果均相对 V016；V016 FP16 D32768 结果相对 V011，两个区间不可相加。R013/ASYNC 与之同属流水大类，但不改同一等待点；R028 是 V/S 标量交接，非 MTE3_V。完整 R001-R029、FULL、R031、MIX 与 Wave-1 对照见“历史重复对照”。证据：`线上结果/R31B/V017/diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31B-V017.md`、`研究/主代理/MAIN-2/本地评分器校准.md`。
RELATED_OLD_ROUTES: R31B V016/V017、STORE-EPILOGUE-X V003。
EVIDENCE_PATHS: `线上结果/R31B/V017/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31B-V017.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅新增上述 BF16 D32768 分派，命中时运行完整 V017；其余输入完整使用 V011。不抽取 event 片段，不添加别的 donor。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后先确认旧探针 M；V011 与 V017 对确切 BF16 M×32768 做交错 P/C，记录 noise floor、M、blockCount、每核 batchRows、tile 与正确性。用 FP16 D32768 和一个非目标 shape 验证分派边界。
EXPECTED_LOCAL_PROBES: 先从共享 workload 清单恢复 V017 BF16 D32768 的确切 M 集合；每个目标形状分别核对 V011 与完整 V017 的 correctness、same-binary 和设备事件噪声底，再做同设备交错 P/C。控制只取 FP16 D32768 与一个 BF16 非目标宽度，确认分派不外溢；按每个 shape 报告原始样本、paired delta 与方向一致性，不合并成 Official 预测。
OPEN_QUESTIONS: V017 原始 BF16 局部样本的 M 未在此处来源中出现；工作负载是否含 BF16 D32768 尚需公开清单支持；实际 tile、blockCount 和每核 batchRows 是否与旧记录一致；无 Official case shape/dtype 映射，无法估算该分支对 15 case 的覆盖。
UNCERTAINTY: V017 原始局部样本与 M 未在本工作树证据中；Official shape/dtype 映射未知。

### H1 资格确认（只读）

RESULT: `QUALIFICATION_INCOMPLETE`。本节只整理当前分支已提交材料；未运行构建、正确性或测时。

**版本与来源**

- V011 fallback：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，见 `线上结果/R31B/V011/source-meta.json` 与 `submission.sha256`。
- 完整 V017 donor：`7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4`；direct parent 是 V016，source SHA 为 `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`。V017 包含 V016 的低精度宽行 tile 调整及 V017 的等待调整；拟议对照必须使用完整 V017，不能把它当作仅含 V017 单点变化的 V011 子版本。

**已确认形状与可达分派**

- V017 的公开局部摘要只写 `BF16 wide D32768`，没有 M、完整输入 shape 或 runner 参数。已提交 V016 parent-control runner 明确使用 `M=2, D=32768, BF16`，`kCoreCount=8`；这是 V011/V016 的同形状证据，不能证明 V017 的 `20/20` 使用同一个 M。
- 在 V011 与 V017 源码中，`D=32768 > kCacheElems(8192)` 会令 `widePath_` 生效；BF16 (`dtype=2`) 进入 `ProcessWideLowPrecision`，其中包含 pass-2 store 等待代码。因此该 donor 路径在目标 dtype/width 上可达，M 不参与该入口判断。
- 对已记录的 V016 runner 配置 `M=2, availableCoreNum=8`，源码规则推得 `blockCount=min(8,2)=2`，每个 block `localRows=1`。V017 的 BF16 UB 预算选择推得 `wideFullYRows/batchLimit=1`、`tileWidth=2560`、`tileCount=ceil(32768/2560)=13`，故该配置每轮 `batchRows=1`。这些数值是依据提交源码与 runner 参数的静态推导；日志没有逐 block 运行时遥测，且未证明它们对应 V017 的历史 `20/20` 执行。

**已有稳定性与配对记录**

- V016 的统一本地记录在 d4、`M=2`、`D=32768`、BF16、45 次 warmup 下运行 V011 parent-only same-binary：第一次为 `NEEDS_VALIDATION`（MAD/median=0.069、block drift=0.130），第二次为 `PASS`（0.021、0.025）。这是 V011 的历史形状证据，不替代本轮新窗口资格。
- 同一 V011/V016 配对记录的 BF16 D32768 delta 分别为 `+0.14 us (+1.01%)` 与 `-0.02 us (-0.14%)`，未显示有意义的 V016 改善；这不是 V011/V017 对照。
- V017 公开摘要报告相对 V016 的 BF16-wide D32768 局部结果为 `-2.0 us / -14.3%`、`20/20`，并记载 same-binary `PASS 6/6`；摘要未给 M、blockCount、localRows、batchRows、runner/executable 身份或原始样本。V017 Official 为 15/15 PASS、44.68，相对 V011 的 45.16 为 -0.48；该 Official 结果不用于推断局部对照或整体收益。

**仍缺材料与最小后续探针**

- 当前提交树没有 `本地实验/R31B/V017/`；缺 V017 局部 runner/shape manifest、原始 same-binary 与配对样本、执行时 `availableCoreNum/blockCount`、逐 block `localRows/batchRows` 记录，以及实际 tile width/count 和 executable 身份。也没有 V011/V017 的直接配对 delta。
- 因 V017 历史 M 无法从本分支已提交记录恢复，不能把 M=2 的 V016 记录代填为 V017 的 exact M；资格状态保持 `QUALIFICATION_INCOMPLETE`，本阶段停止于只读报告。
- 后续仅在 Main 提供独立 job、device 与时间窗口后，先用恢复出的 exact M/shape 对 V011 按项目统一协议重做 same-binary；通过后再以相同 runner、device、dtype、shape 交错比较 V011 与完整 V017。记录实际 `blockCount/localRows/batchRows/tileWidth/tileCount` 及原始样本。未恢复 exact M 前不启动探针、不形成局部收益结论，也不选择实现方向。

## H3 — FP32 chunked writeback

HYPOTHESIS_ID: `H3-FP32-WIDE-STORE-V003`
CHILD_RECOMMENDED_HYPOTHESIS: `NONE`; child 不排序、不选择，等待 Main 明确 `MAIN_SELECTED=YES`。
MECHANISM: 对三个已测 shape 分派到完整 STORE V003；满足 `tileCount>=4 && rowWidth%8==0` 时，把一行合并写回切成两个 tile 对齐 chunk。
BOTTLENECK: 多 tile FP32 行的 Store 描述成本，以及整行延后写回失去 chunk 间重叠。
DIRECT_PARENT: R31B V011.
DONOR_SOURCE_SHA: `0cdef265459d4683a1813593a881a5cf25ae75246aab49279d121896b71184ca` (STORE-EPILOGUE-X V003)
DONOR_LINEAGE_PARENT: STORE-EPILOGUE-X V002.
V011_FALLBACK_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: 精确集合 `8x16384`、`1x32768`、`1x16384`，FP32。
TARGET_DTYPES: FP32.
WORKLOAD_COVERAGE: 对 V002 局部结果依次为 -5.64% (6/0)、-3.15% (6/0)、-2.90% (5/0)。Official 44.38，低于 V002 45.07 和 V011 45.16。
WHY_IT_MAY_HELP: 两次 chunk Store 可减少相对逐 tile 写回的描述数，并让前一段写回覆盖后一段计算。
WHY_IT_MAY_FAIL: 旧局部结果只对比 V002；Store 成本可能不是主项，chunk 等待也可能吃掉并行收益。Official 结果未提升。
UB_IMPACT: 复用 V002 的完整 valueLocal，不增加 UB；需确认 chunk 起点、长度和各行偏移。
DMA_IMPACT: 总字节不变；合并行每行两次 Store，非合并路径保留 donor 内逐 tile 写回。
SYNC_IMPACT: 复用两深度 event ring；槽位复用前等待，循环末排空 outstanding Store。
PRECISION_RISK: 算术不变，预期逐位一致；风险为 chunk 边界、尾段、行对齐和 event 复用。
DUPLICATE_CHECK: V003 是 V002 单行合并写回的 K=2 分块后续；这与 V017 的单 in-flight store 等待移动不同，H4 的 `1x32768` 仅 shape 重叠、机制不同。R009/R010/R012 涉及 copy/tail/alignment 约束，R015 涉及输入多行 DMA；都不等于把输出整行写回分为两段。完整 R001-R029、FULL、R031、MIX 与 Wave-1 对照见“历史重复对照”。证据：`研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md`、`研究/STORE-EPILOGUE-X/MAIN-APPROVAL-V002.md`、`线上结果/STORE-EPILOGUE-X/V003/source-meta.json`、`线上结果/STORE-EPILOGUE-X/V003/diff.patch`、`线上结果/STORE-EPILOGUE-X/V003/result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-STORE-V003.md`。
RELATED_OLD_ROUTES: STORE-EPILOGUE-X V002/V003、STORE-H2B、ASYNC-TRIPLE-X、R31B V017、EPILOGUE-ARITH-CHAMPION-X V002。
EVIDENCE_PATHS: `线上结果/STORE-EPILOGUE-X/V003/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-STORE-V003.md`、`研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅对三个精确 shape 分派到完整 V003，其余回到 V011；不拆取 V002、V017 或 EPI 代码。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后对三项 shape 做 V011/V003 交错 P/C，以 `8x16384` 为主；另测 tileCount<4 及一个不匹配 shape 验证 V011 回退，并执行正确性与 chunk 边界验证。
EXPECTED_LOCAL_PROBES: 对精确集合 `8x16384`、`1x32768`、`1x16384` 分别核验 V011 与完整 V003 的 correctness、same-binary 与噪声底；再逐形状交错 P/C，以 `8x16384` 作为覆盖多行的主探针。只加 `tileCount<4` 和一个 shape/dtype 不匹配控制确认回退。记录 tileCount、chunk 边界、每行起止地址和 paired 结果；不与 H1/EPI donor 组合。
OPEN_QUESTIONS: 当前公开的三项局部摘要没有原始样本；各 shape tileCount 与执行到 V003 merge 分支的精确条件须从 donor 源码确认；`8x16384` 的收益不能外推到其他 M 或 D；Official case 对应未知。
UNCERTAINTY: 当前可见摘要无原始样本；三项 shape 的 Official case 映射未知。

## H4 — FP32 affine arithmetic

HYPOTHESIS_ID: `H4-FP32-AFFINE-EPI-V002`
CHILD_RECOMMENDED_HYPOTHESIS: `NONE`; child 不排序、不选择，等待 Main 明确 `MAIN_SELECTED=YES`。
MECHANISM: 对精确 FP32 shape 分派到完整 EPI V002；block-local `batchRows==1` 时，以 Mul+Axpy 执行 `(y*gamma)*invRms+bias`，替代 V001 的整行 Muls 加逐 tile Mul/Add。
BOTTLENECK: 单行完整 y 驻留后的 affine 向量链。
DIRECT_PARENT: R31B V011.
DONOR_SOURCE_SHA: `00a5c8186d66640c159cf1834e394257754e7cf032a439e6274bc3b12a52b108` (EPILOGUE-ARITH-CHAMPION-X V002)
DONOR_LINEAGE_PARENT: EPILOGUE-ARITH-CHAMPION-X V001.
V011_FALLBACK_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: `1x32768`、`2x16384` FP32；要求 `rowCount<=blockCount` 以使实际 `batchRows==1`，并记录分支命中。
TARGET_DTYPES: FP32.
WORKLOAD_COVERAGE: d4/d6 局部：`1x32768 -6.64% (6/6), -3.13% (4/6)`；`2x16384 -1.43% (5/6), -3.62% (5/6)`。正确性 24/26，两个宽 FP32 差异记为 parent/golden 共通问题。Official 44.96（-0.20 vs V011）。
WHY_IT_MAY_HELP: d4/d6 方向一致；单行分支省去整行 Muls，并以 Axpy 合并乘 invRms 与加 bias。
WHY_IT_MAY_FAIL: 运算次序与 Axpy 舍入可能改变 FP32 结果；2x16384 的收益依赖 blockCount，且 Official 低于锚点。
UB_IMPACT: 不增 buffer；复用 biasLocal 暂存结果，需保证 Store 前不覆盖仍需使用的数据。
DMA_IMPACT: DMA 次数、数据量和 Store 分块不变。
SYNC_IMPACT: event 与 DMA 不变；单行分支略过 V001 整行 Muls 后的 barrier，每 tile 仍有 PipeBarrier。
PRECISION_RISK: 高；`(y*gamma)*invRms+bias` 改变舍入次序，且旧正确性为 24/26。
DUPLICATE_CHECK: V002 是 V001 NORM-HOIST 的后继，在 `batchRows==1` 时使用 Mul+Axpy；H3 的 `1x32768` 有 shape 重叠但只改写回分块。两者不得同时分派。R019/R020 是归一化表达式，R017/R018 是中间精度/输出舍入；机制边界见“历史重复对照”。R31A V028 被排除，因其为 V026 父链上的 batch flag 前 barrier 删除，非 V011 affine donor。证据：`线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/diff.patch`、`线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/source-meta.json`、`线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/result.json`、`线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/local-result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V002.md`、`线上结果/R31A/V028/diff.patch`、`调度/本地线上校准.tsv`。
RELATED_OLD_ROUTES: EPILOGUE-ARITH-CHAMPION-X V001/V002、STORE-EPILOGUE-X V003。
EVIDENCE_PATHS: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V002.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅对两项 shape 且 `rowCount<=blockCount` 分派到完整 EPI V002；其余精确回到 V011。不叠加 STORE 或 V017。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后先完成目标 shape 的 V011/V002 正确性对照并标明已知差异，再做跨窗交错 P/C；加 `batchRows>1` 控制验证分派未触发。记录 blockCount 和实际 Axpy 分支。
EXPECTED_LOCAL_PROBES: 先对 V011 与完整 V002 的 `1x32768`、`2x16384` FP32 分别做目标正确性复核，明确记录历史 24/26 中 parent/golden 共通的两项；只有差异来源可判定后，才逐 shape 做 same-binary、噪声底及跨窗交错 P/C。加一个实际 `batchRows>1` 控制确认 Axpy 分支未命中；记录 blockCount、batchRows、分支命中和所有 precision diff。
OPEN_QUESTIONS: FP32 已知 golden 差异能否在当前 V011 runner 上稳定复现及如何分类；目标 shape 的 rowCount/blockCount 是否保证实际 `batchRows==1`；Mul+Axpy 顺序是否满足官方容差；Local 信号来自不同窗口且不得外推到未探 shape，Official case 映射仍未知。
UNCERTAINTY: 原始局部样本在另一工作树，本轮未访问；可见摘要缺 blockCount 与逐例 26 项资料，Official shape 映射未知。

## 给 Main

种子 H1/H2 共用 V017 donor 和等待机制，故合并为 BF16 H1；FP16 只留在重复项证据中。H3、H4 分别代表写回和算术机制。R31A V028 不纳入：它属于 V026 父链上的 FP32 batch barrier 变化，和 V011 没有直接 P/C 证据；Official 低于其参照，且本地收益不能外推到 V011 或整体 workload。此处只说明缺少可比依据，不判定该机制无效。H3 的 shape 记录较具体，但 Official 44.38；这仅说明其探针较易定义，不构成实现选择。请 Main 确认 workload 对照和缺失的 M/blockCount，再发出 `MAIN_SELECTED=YES`。当前未建 Revision、未改 Candidate/kernel、未构建或测时、未用 server3、未改共享台账。
