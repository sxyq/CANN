# ROW-OCCUPANCY-CHAMPION-X Track-B Handoff

日期：2026-09-30

## 状态

- BOOTSTRAP_STATUS: COMPLETE
- TRACK: B，已完成本轮只读研究
- MAIN_SELECTED: NONE
- HYPOTHESIS_POOL_STATUS: ROUTE_HYPOTHESIS_POOL_EXHAUSTED
- QUALIFYING_INDEPENDENT_HYPOTHESES: 0
- 本轮没有建立 Revision，没有编译、正确性、测时、profiling、server3 或 Online 操作。
- 初始工作树：/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy；分支 w2/m2/row-occupancy；初始 HEAD 与 origin/main 均为 ed860e392d7604694ac6664da60aff1fc1f4c04f；初始工作区干净。

## Direct Parent

- DIRECT_PARENT: R31B V011
- PARENT_SCORE: Official 45.16
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- 依据：线上结果/R31B/V011/source-meta.json 的 submission_sha256 与 route_source_sha256、线上结果/R31B/V011/submission.sha256、提交源码文件本身的 SHA256 计算，以及技术路线/全版本记录.tsv 第30行相符。技术路线/路线成绩表.tsv 第2行将其列为 Overall Champion。
- source-meta 记录的 Judge 原生源文件摘要缺失；此处确认的是仓库提交文件、sidecar 与版本账本中的源 SHA 一致，不扩大为 Judge 原生摘要已验证。
- R31B V017、R31A V028 只作为历史机制证据；它们的直接父版本分别为 R31B V016、R31A V026，不作为本路线父源。

## Route Boundary

- ROUTE_BOUNDARY: 只研究整行到 block/core 的归属、block 数量与整行任务分配。
- ALLOWED_CHANGES: 本轮只新增本路线目录内的 Track-B 交接文档。以后如获 Main 明确批准，单个 Revision 也只能修改行任务划分或 block 分配中的一个机制。
- FORBIDDEN_CHANGES: Kernel 内每行计算流程、流水重叠、UB 生命周期、gamma/bias 常驻；Candidate、.asc、.cpp、.h、.hpp、CMake、runner、wrapper；Revision、构建、正确性、测时、profiling、server3、Online；共享账本及路线生命周期决定。
- 重复证据可引用禁止范围内的既有机制，不据此提出修改。

## Duplicate Audit

| 已提交证据 | 相似点 | 实际差异 | 对本路线的独立价值 |
|---|---|---|---|
| R001-R029：技术路线/技术路线总表.md 第30、34行 | R012 是 32B 行组归属；R016 是按行分配 AI Core。两者直接覆盖对齐行组和行任务调度。 | R012 讨论边界对齐，R016 讨论行到核的任务调度；本路线 parent 是强源 R31B V011。 | 组合或移植到 V011 本身不能形成新机制；FULL 与 Champion 派生记录已继续验证这两个方向。 |
| FULL：技术路线/技术路线总表.md 第68、72行 | FULL-R012 与 FULL-R016 分别落地行组对齐和调度架构。 | FULL-R016 Official 17.14，FULL-R012 有源 SHA 绑定差异记录；与 V011 的具体表现不能直接类推。 | 可作历史证据，不提供新的行任务分配机制。 |
| R31B/R31A：线上结果/R31B/V011/submission.asc 第171-176、3530-3540行；技术路线/全版本记录.tsv 第97-98行 | V011 的窄路径按 blockIdx 做连续商余数分配；host 以 min(availableCoreNum,rowCount) 限定 block 数。V017/V028 都在冠军近缘版本上。 | V017 改第二遍 MTE3 等待时点；V028 删除 batch-affine SetFlag。二者都没有改变整行到 block 的映射。 | V011 是唯一指定 parent。V017/V028 仅为历史证据，对行占用没有独立佐证。 |
| MIX-A：技术路线/技术路线总表.md 第五节（第114、118行）；技术路线/全版本记录.tsv 第12-18行 | MIX-A 的 localRows 条件会影响已有计算分支。 | V003 加同步并限制 localRows==1；V004-V006 调整模式分派；V007 移除同步。它们改变同步或路径选择，没有独立验证行任务映射。 | 不能把路径选择、计算流程或同步变化带入本路线；不构成新的占用方向。 |
| Wave-1，SCHED-ROWGROUP-X：研究/SCHED-ROWGROUP-X/next-hypotheses.md 第13、28、43、58、73、173、202、237行 | 已逐项记录 core-fill、even-split、dynamic task-pull、最小行组粒度、宽行 block 上限、cyclic/contiguous、放宽行组下限和 core 数量扫点。 | 该路线以 FULL-R016 为直接父源；其中部分只是研究提议，未在 V011 上实施。 | 这些都是既有行调度方案；移到指定 parent 不产生新机制。 |
| Wave-1，SCHED-CHAMPION-X：研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md 第69、165、238、298行；TRACK-B-HYPOTHESES-V003.md 第26行；技术路线/全版本记录.tsv 第65-67行 | 该路线与本路线同用 R31B V011 SHA，覆盖 group-aligned ownership、group-quantized launch width、cyclic group、tail folding。V003 已实测尾组折叠。 | V001/V002/V003 的归属变化在强 parent 上直接做过；V002 有 33x100 FP32 -4.4% 的本地结果，V003 在 33x100 为 +0.9%、17x257 为 -5.2%，记录为 LOCAL_REJECTED。 | 本路线能够复述和重命名这些变化，但没有留下独立假设。技术路线/路线成绩表.tsv 第53行记录该 ownership lane 已有 Main 处置。 |
| Wave-2，MULTIROW-DMA-CHAMPION-X：技术路线/全版本记录.tsv 第83、86行 | 同为 R31B V011 派生，工作跨多行。 | V001 改 stride multi-row DataCopy，48x16384 FP16 中位退化 8.4%；V002 改 Pad 到 non-Pad DataCopy 形式，结果在约 ±7% 范围内。两者不改 row-to-core 归属。该路线原始目录不在本 worktree；此处只引用已提交账本。 | 可作边界证据，不能作为本路线的 row occupancy 试验。 |
| Wave-2，ASYNC-OVERLAP-CHAMPION-X：技术路线/全版本记录.tsv 第87-90行 | 包含跨行预取及每行工作顺序。 | V002-V004 修改 MTE2/V/MTE3 时序、GetValue 交接或 prologue；均在本路线禁止范围内。 | 只证明邻近计算方向已有研究，不支持改变 block 分配。 |
| R31B V017、R31A V028：调度/本地线上校准.tsv 对应行及技术路线/全版本记录.tsv 第97-98行 | 均有局部性能收益记录。 | V017 Official 44.68、V028 Official 44.07，均低于各自父链中的 Official anchor；机制分别为 store drain 与 SetFlag，不涉及 row occupancy。 | 不得作为新 parent 或 row mapping 的正向证据。 |

审计结论：R012/R016、FULL-R012/R016、Wave-1 的 SCHED 两条路线已经覆盖行组、block 数和任务边界；同一 R31B V011 源上已有 SCHED-CHAMPION 的实现与测量。Wave-2 的 Multirow 与 Async 改变数据搬运或流水，超出本路线边界。没有三条可独立保留的行到核分配方向。

## Screened Hypotheses

以下五项是为复核假设空间而列出的候选；每项均因已有机制重合标记 DUPLICATE_REJECTED，不计入独立假设池，也不构成实现建议。

### H1: Core-Count Row-Group Fill

- HYPOTHESIS_ID: ROWOCC-H1-CORE-COUNT-FILL
- STATUS: DUPLICATE_REJECTED
- MECHANISM: 按可用 core 数、rowCount 和 32B rowGroup 数推导 block 数，使每个 rowGroup 有一个 owner。
- BOTTLENECK: rowGroup 数少于已启动 block 数时的空 block 与启动开销。
- DIRECT_PARENT: R31B V011，Official 45.16。
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- TARGET_SHAPES: 33x100 FP32、17x257 FP16；17x256 FP32 作为 rowGroup=1 对照；7x65 FP16 作为 rowCount 小于 rowGroup 的风险形状。
- TARGET_DTYPES: FP32、FP16。
- WHY_IT_MAY_HELP: 可能减少没有完整 rowGroup 的 block，并让参与的 block 取得更多连续行。
- WHY_IT_MAY_FAIL: SCHED-CHAMPION-X 的 H2 已提出 group-quantized launch width；V011 当前 block 数已是 min(availableCoreNum,rowCount)。改变映射还会改变 localRows，既有分支可能随之选择不同路径。
- UB_IMPACT: 不改 UB 容量或存活范围。
- DMA_IMPACT: 每行搬运量不变；不同 block 边界可能改变并发及连续访问局部性。
- SYNC_IMPACT: 不新增跨 block 同步。
- PRECISION_RISK: 行内数学不变；需保证每行只归属一个 block。
- DUPLICATE_CHECK: DUPLICATE_REJECTED。与 SCHED-ROWGROUP-X H1 core-fill、SCHED-CHAMPION-X H2 group-quantized launch width 相同；parent 不同不会形成新机制。
- RELATED_OLD_ROUTES: R012、R016、FULL-R012、FULL-R016、SCHED-ROWGROUP-X、SCHED-CHAMPION-X。
- MINIMAL_EXPERIMENT: 单独把 host blockCount 上限改为 min(availableCoreNum,rowCount,totalGroups)。该差异已由 SCHED-CHAMPION-X H2 覆盖，本轮不实施。

### H2: Even-Split Contiguous Row Extents

- HYPOTHESIS_ID: ROWOCC-H2-EVEN-SPLIT-EXTENTS
- STATUS: DUPLICATE_REJECTED
- MECHANISM: block 数固定，只把连续行区间从固定 rowsPerTask 改为按 taskCount 商余数均分。
- BOTTLENECK: 尾部短任务导致的 block 间工作量差异。
- DIRECT_PARENT: R31B V011，Official 45.16。
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- TARGET_SHAPES: 33x100 FP32、17x256 FP32；128x16384 FP16 作为 rowCount 高于 core 数的探针候选。
- TARGET_DTYPES: FP32、FP16。
- WHY_IT_MAY_HELP: 在固定 block 数下把长短任务差距压小，理论上减少最长任务的尾部等待。
- WHY_IT_MAY_FAIL: V011 已按 rowCount/blockCount 做连续商余数分配，所有行的算量相同，任务行数差最多为一；R31B 同形状上没有已证实的长短任务瓶颈。
- UB_IMPACT: 不变。
- DMA_IMPACT: 总字节数不变；连续行仍由单个 block 顺序处理。
- SYNC_IMPACT: 不变，无新增跨 block 同步。
- PRECISION_RISK: 不改变行内算术；要求区间无重叠、无遗漏。
- DUPLICATE_CHECK: DUPLICATE_REJECTED。SCHED-ROWGROUP-X H2 EVEN-SPLIT TASK EXTENTS 已记录同一边界变更；该旧路线的 16/16/1 分区来自另一 parent 的 band table，不能证明 V011 有该失衡。
- RELATED_OLD_ROUTES: R016、SCHED-ROWGROUP-X H2、SCHED-CHAMPION-X H4 tail-group folding。
- MINIMAL_EXPERIMENT: 仅改 host 的 task range 公式，blockCount 不变。机制与已有 even-split 假设相同，且 V011 的商余分配已接近均衡；不运行。

### H3: Cyclic Row-Group Ownership

- HYPOTHESIS_ID: ROWOCC-H3-CYCLIC-GROUP-OWNERSHIP
- STATUS: DUPLICATE_REJECTED
- MECHANISM: 保持 rowGroup 与 block 数不变，令 block i 处理 group i、i+blockCount 等循环序列。
- BOTTLENECK: 只有任务成本不均时，连续分组才可能形成 block 间长尾。
- DIRECT_PARENT: R31B V011，Official 45.16。
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- TARGET_SHAPES: 128x16384 FP16 或其他 rowCount 显著高于 blockCount 的形状；须先对 V011 做逐形状资格确认。
- TARGET_DTYPES: FP16、FP32。
- WHY_IT_MAY_HELP: 若行成本不均，循环分组可能把较重的 group 分散到不同 block。
- WHY_IT_MAY_FAIL: 同一 shape 内各行执行相同算术和搬运；V011 商余分配已将行数差限制为一。循环访问还可能降低同一 block 的 GM 连续性。
- UB_IMPACT: 不变。
- DMA_IMPACT: 总字节数不变；跨 group 循环可能降低地址连续性。
- SYNC_IMPACT: 不新增同步。
- PRECISION_RISK: 行间分配不改变数值；必须保证 group 被处理恰好一次。
- DUPLICATE_CHECK: DUPLICATE_REJECTED。SCHED-CHAMPION-X H3 CYCLIC GROUP ASSIGNMENT 与 SCHED-ROWGROUP-X H6 scheduleMode uniformity 已明确提出 cyclic/contiguous 比较。
- RELATED_OLD_ROUTES: R016、SCHED-ROWGROUP-X H6、SCHED-CHAMPION-X H3。
- MINIMAL_EXPERIMENT: 只替换 group 到 block 的映射循环，在 rowCount>blockCount 的已合格形状上与连续映射配对。相同假设已有记录，不运行。

### H4: Fold Partial Tail Group

- HYPOTHESIS_ID: ROWOCC-H4-FOLD-TAIL-GROUP
- STATUS: DUPLICATE_REJECTED
- MECHANISM: 把末尾不足一个 rowGroup 的余数行并入前一 group owner，避免独立短 group。
- BOTTLENECK: rowCount 非 rowGroup 整数倍时的短尾任务。
- DIRECT_PARENT: R31B V011，Official 45.16。
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- TARGET_SHAPES: 33x100 FP32、17x257 FP16。
- TARGET_DTYPES: FP32、FP16。
- WHY_IT_MAY_HELP: 当末尾 group 占总任务的大比例时，省去一个很短的独立 block。
- WHY_IT_MAY_FAIL: 33x100 的 rowGroup=2、尾数为 1；并入前一 owner 会令其处理 3 行而其他 owner 处理 2 行。已提交结果在该形状 +0.9%；17x257 结果 -5.2%，均未支持此方向。
- UB_IMPACT: 不变。
- DMA_IMPACT: 总字节数不变；owner 覆盖范围改变。
- SYNC_IMPACT: 不新增同步。
- PRECISION_RISK: 不改变行内数学；尾段边界仍须完整覆盖。
- DUPLICATE_CHECK: DUPLICATE_REJECTED。SCHED-CHAMPION-X V003 已批准并测过 tail-group folding，结果为 LOCAL_REJECTED。
- RELATED_OLD_ROUTES: R012、SCHED-CHAMPION-X H4/V003、SCHED-ROWGROUP-X H2。
- MINIMAL_EXPERIMENT: 对末尾 partial group 执行一次 owner 合并。该单变量已实测，无剩余独立价值；不复跑。

### H5: Relax the 32B Row-Group Floor

- HYPOTHESIS_ID: ROWOCC-H5-RELAX-ROWGROUP-FLOOR
- STATUS: DUPLICATE_REJECTED
- MECHANISM: 对低精度奇数宽行不再强制 rowGroup=32/gcd(rowBytes,32)，允许每个 block 取得更少的完整行。
- BOTTLENECK: rowGroup 下限可能让少行 shape 只由少数 block 承担。
- DIRECT_PARENT: R31B V011，Official 45.16。
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- TARGET_SHAPES: 17x257 FP16、7x65 FP16。
- TARGET_DTYPES: FP16。
- WHY_IT_MAY_HELP: 放宽每个 owner 的最小行数，可能增加并行 block 数。
- WHY_IT_MAY_FAIL: 32B group 用来保持整行 group 的访问边界；放宽后需依赖逐行 pad 搬运，可能增加 DMA 成本或触发边界错误。SCHED-CHAMPION-X V001 在 17x257 已出现 +145.9% 退化；V002 用 active-core 条件避免该塌缩。
- UB_IMPACT: 不变。
- DMA_IMPACT: 可能让非对齐行更多走逐行 pad 搬运；总数据量不变。
- SYNC_IMPACT: 不新增跨 block 同步。
- PRECISION_RISK: 行内数学不变；主要风险是写入边界和行归属正确性，不预期数值精度变化。
- DUPLICATE_CHECK: DUPLICATE_REJECTED。SCHED-ROWGROUP-X H7 PER-TASK OWNERSHIP WITHOUT ROW-GROUP FLOOR 已记录同一改变与风险。
- RELATED_OLD_ROUTES: R009、R012、SCHED-ROWGROUP-X H7、SCHED-CHAMPION-X V001/V002。
- MINIMAL_EXPERIMENT: 只取消 rowGroup 向上取整，在 17x257 FP16 做来源一致的 correctness 与配对性能验证。已有完全相同假设，且现有强源测量出现大幅退化；不运行。

## Required Handoff Fields

- EVIDENCE_PATHS: AGENTS.md；.agents/skills/cann-mainline/SKILL.md；项目规则/实验总则.md、执行约定.md、本地性能测试规范.md、服务器实验规范.md、Git工作流程.md、线上提交规范.md；技术路线/技术路线总表.md、技术路线/技术路线图.md、技术路线/路线成绩表.tsv、技术路线/全版本记录.tsv；调度/当前任务.tsv、调度/线上候选.tsv、调度/本地线上校准.tsv；线上结果/R31B/V011/{source-meta.json,submission.sha256,submission.asc,result.json}；研究/SCHED-ROWGROUP-X/{ROUTE-BRIEF.md,next-hypotheses.md}；研究/SCHED-CHAMPION-X/{TRACK-B-BRIEF.md,TRACK-B-HYPOTHESES.md,TRACK-B-HYPOTHESES-V003.md,MAIN-APPROVAL-V003.md}；本地实验/SCHED-ROWGROUP-X/V001/WHY_NOT_DUPLICATE.md；本地实验/SCHED-CHAMPION-X/V002/{source-meta.json,local-result.json}。
- PROPOSED_ONE_FACTOR_DIFF: NONE。五条单变量差异仅为重复审计对象，未获实现批准。
- EXPECTED_LOCAL_PROBES: 本轮 NONE。若 Main 与 Planning 日后批准新的独立方向，先对 R31B V011 的直接父源和每个具体 shape/dtype 做 same-binary；只有资格通过后，按项目统一 device-event 流程做相邻交错 P/C，warmup 45，至少 4 组 pair。历史 shape 数值不代替本轮父源资格。
- CHILD_RECOMMENDED_HYPOTHESIS: NONE。此字段只报告研究建议，不代表选择实现；当前没有可推荐的新假设。
- OPEN_QUESTIONS: (1) V011 的 Official shape-to-case 映射在提交记录中标为缺失，本文形状只用于本地探针讨论。(2) MULTIROW-DMA-CHAMPION-X 原始目录不在当前 worktree，审计只依赖正式账本和成绩表。(3) 若后续出现每行成本不同或 core 物理分组影响行任务的新证据，可据此重新开一个独立研究问题；当前提交记录没有这类证据。(4) 本 handoff 不作路线 KEEP/PARK/CLOSE、合并、替换或新建决定。

## Handoff Conclusion

ROUTE_HYPOTHESIS_POOL_EXHAUSTED。已筛选的五个方向都因与 R012/R016、Wave-1 行组路线或同一 V011 parent 上既有研究重合而被标记 DUPLICATE_REJECTED；没有三条可独立交 Main 的方向。MAIN_SELECTED 保持 NONE，不创建 Revision。路线生命周期与后续方向由 Planning / Review Layer 决定。
