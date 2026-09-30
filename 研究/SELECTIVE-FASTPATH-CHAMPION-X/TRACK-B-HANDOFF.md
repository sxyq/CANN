# SELECTIVE-FASTPATH-CHAMPION-X — Track-B Handoff

STATUS: RESEARCH_COMPLETE; MAIN_SELECTED=WAITING
DIRECT_PARENT: R31B V011
OFFICIAL_ANCHOR: 45.16
V011_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
ROUTE_BOUNDARY: 每个候选只分派一个完整 donor；未命中输入精确回到 V011。不得组合 donor。本轮未选实现。

## 共用证据

Judge 结果有 15 个 testcase ID、分数和通过状态，没有 shape/dtype 字段；`研究/OFFICIAL-CASE-ANALYSIS.md` 也说明精确映射未知。不能声称本地 shape 对应特定 Official case。

V011 fallback 证据：`线上结果/R31B/V011/submission.asc`、`submission.sha256`、`result.json`。

下列局部数据均来自 donor 自己的直接父版，不是 donor 对 V011 的同条件结果。若 Main 选择一项，最小性能探针须直接比较完整 donor 与 V011。所有 donor Official 均 15/15 PASS，但总分低于 45.16。

## H1 — BF16 wide store wait

HYPOTHESIS_ID: `H1-BF16-WIDE-V017`
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
DUPLICATE_CHECK: 种子 H2 的 FP16 项与本项是同一 V017 donor、同一机制，因此不另列为独立假设。其旧信号为 -6.7% 至 -8.5% vs V016；V016 FP16 D32768 为 -6.5% 至 -7.0% vs V011。不得把两段增量相加。证据：`线上结果/R31B/V017/diff.patch`、`线上结果/R31B/V017/source-meta.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31B-V017.md`。
RELATED_OLD_ROUTES: R31B V016/V017、STORE-EPILOGUE-X V003。
EVIDENCE_PATHS: `线上结果/R31B/V017/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31B-V017.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅新增上述 BF16 D32768 分派，命中时运行完整 V017；其余输入完整使用 V011。不抽取 event 片段，不添加别的 donor。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后先确认旧探针 M；V011 与 V017 对确切 BF16 M×32768 做交错 P/C，记录 noise floor、M、blockCount、每核 batchRows、tile 与正确性。用 FP16 D32768 和一个非目标 shape 验证分派边界。
UNCERTAINTY: V017 原始局部样本与 M 未在本工作树证据中；Official shape/dtype 映射未知。

## H3 — FP32 chunked writeback

HYPOTHESIS_ID: `H3-FP32-WIDE-STORE-V003`
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
DUPLICATE_CHECK: V003 继承 V002 的单行写回合并，属于 STORE-H2B 后续。与 V017 的延迟 Store wait、EPI V002 的 FP32 `1x32768` 均有相邻或重叠范围；探针必须各自单 donor。证据：`研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md`、`研究/STORE-EPILOGUE-X/MAIN-APPROVAL-V002.md`、`线上结果/STORE-EPILOGUE-X/V003/diff.patch`。
RELATED_OLD_ROUTES: STORE-EPILOGUE-X V002/V003、STORE-H2B、ASYNC-TRIPLE-X、R31B V017、EPILOGUE-ARITH-CHAMPION-X V002。
EVIDENCE_PATHS: `线上结果/STORE-EPILOGUE-X/V003/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-STORE-V003.md`、`研究/STORE-EPILOGUE-X/STORE-H2B-SPEC.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅对三个精确 shape 分派到完整 V003，其余回到 V011；不拆取 V002、V017 或 EPI 代码。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后对三项 shape 做 V011/V003 交错 P/C，以 `8x16384` 为主；另测 tileCount<4 及一个不匹配 shape 验证 V011 回退，并执行正确性与 chunk 边界验证。
UNCERTAINTY: 当前可见摘要无原始样本；三项 shape 的 Official case 映射未知。

## H4 — FP32 affine arithmetic

HYPOTHESIS_ID: `H4-FP32-AFFINE-EPI-V002`
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
DUPLICATE_CHECK: V002 继承 V001 NORM-HOIST；`1x32768 FP32` 与 STORE V003 H3 重叠，必须分别测量、不得同时分派。证据：`线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/diff.patch`、`source-meta.json`；source-meta 只列出另一工作树中的局部资料引用，本轮未访问该处。
RELATED_OLD_ROUTES: EPILOGUE-ARITH-CHAMPION-X V001/V002、STORE-EPILOGUE-X V003。
EVIDENCE_PATHS: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/submission.asc`、`diff.patch`、`source-meta.json`、`result.json`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-EPI-V002.md`。
PROPOSED_ONE_FACTOR_DIFF: 仅对两项 shape 且 `rowCount<=blockCount` 分派到完整 EPI V002；其余精确回到 V011。不叠加 STORE 或 V017。
MINIMAL_EXPERIMENT: MAIN_SELECTED=YES 后先完成目标 shape 的 V011/V002 正确性对照并标明已知差异，再做跨窗交错 P/C；加 `batchRows>1` 控制验证分派未触发。记录 blockCount 和实际 Axpy 分支。
UNCERTAINTY: 原始局部样本在另一工作树，本轮未访问；可见摘要缺 blockCount 与逐例 26 项资料，Official shape 映射未知。

## 给 Main

种子 H1/H2 共用 V017 donor 和等待机制，故合并为 BF16 H1；FP16 只留在重复项证据中。H3、H4 分别代表写回和算术机制。R31A V028 暂不列入：它要求同时移植 batch Init/缓冲路径，且本地 M 未记录。H3 的 shape 记录最完整，但 Official 44.38；这只说明探针较易定义，不构成实现选择。请 Main 确认 workload 对照和缺失的 M/blockCount，再发出 `MAIN_SELECTED=YES`。当前未建 Revision、未改 Candidate/kernel、未构建或测时、未用 server3、未改共享台账。
