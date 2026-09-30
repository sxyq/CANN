# UB-LIFETIME-SAFE-CHAMPION-X — Track-B Handoff

ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
ROUTE_BOUNDARY: 仅研究可静态证明安全的 UB buffer 生命周期与字节复用。排除 row mapping、参数驻留、跨 pass/跨 row 调度、store policy、算术路径变更。
DIRECT_PARENT: `R31B V011` (`Official=45.16`)
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
MAIN_SELECTED: `NONE`

父版来源由 `线上结果/R31B/V011/submission.asc` 实算 SHA-256，并与 `线上结果/R31B/V011/source-meta.json`、`线上结果/R31B/V011/result.json`、`线上结果/R31B/V011/submission.sha256` 一致。Official 结果为 15/15、45.16。R31B V017、ASYNC V001、R31A V028 仅作历史证据，不作默认父版。

## Route Scope

ALLOWED_CHANGES: 仅在获准实现后调整宽行路径的 buffer 分配、LocalTensor 视图和同一 buffer 内部的 scratch pitch；保持父版 row ownership、batchRows、tileWidth、GM 访问次数、事件顺序和数学表达式。

FORBIDDEN_CHANGES: `Candidate`、kernel `.asc`、`.cpp`、`.h`、`.hpp`、CMake、runner、wrapper；任何 Revision；shared ledger、调度、Online、服务器实验；row mapping、参数驻留、跨 pass/跨 row 调度、store policy、归约/算术顺序。当前文档只供 Main/Planning 评估，不选择实现。

## Hypotheses

### UBX-H1 — FP16 Wide Retained-Y In-Place Output

HYPOTHESIS_ID: `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`
MECHANISM: `ProcessWideLowPrecision` 已将 half `y` 保存在 `gammaBuf_`。pass 2 的每个 `yTile` 完成 `ToFloat` 后不再被读取；可将 `FromFloat`、half `Mul`、half `Add` 和现有 `Store` 的目的视图改为同一个 `yTile`，从而不单独分配宽行路径的 `outputBuf_`。
BOTTLENECK: 宽 FP16 分支额外保留一个 `tileElems * sizeof(half)` UB output tile；`ChooseWideFullYRows` 的 `need` 计算没有独立计入该 T-sized output allocation。
DIRECT_PARENT: `R31B V011`, Official `45.16`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: 宽行 `D > kCacheElems`，首查 `D=16384, 32768`；建议分别覆盖单 row 与多 row batch。
TARGET_DTYPES: `FP16`
WHY_IT_MAY_HELP: 若读写不相交证明成立，可少分配 8192 B/tile；y 的读取、量化点、half `Mul`/`Add` 顺序和 GM store 数保持原样。当前路径已有 MTE3 完成等待，可证明 store 不再读取 source 后再推进 tile。
WHY_IT_MAY_FAIL: 某个分支可能再次读取已覆写的 `yTile`；`Store` 的源视图与后续 tile 生命周期必须逐支证明。减少分配若不改变父版 row batching，耗时也可能不变。
UB_IMPACT: 宽 FP16 路径少 `4096 * 2 = 8192 B`；保留 `wideFullYRows_` 选择不变。
DMA_IMPACT: GM 读写次数与字节数不变。
SYNC_IMPACT: 不增加或移动 event/barrier；沿用现有 `SyncVToMTE3` / `SyncMTE3ToV`。
PRECISION_RISK: 低；保持 `ToFloat -> Muls -> FromFloat -> Mul -> Add` 次序，只改写回地址。要求后续逐形状与父版输出比较。
DUPLICATE_CHECK: 与 R002/full-y 相似处是 y 在 UB 中保留；实际差异是 V011 已有 full-y，假设不增加驻留、不省 GM 重读，只在该 tile 的最后一次 y 读取后将同一字节区改作输出。旧 UB-LIVENESS-X 的 x-slot alias 是另一个输入 buffer 在 pass 2 承担输出 staging。值得单独验证的是宽 FP16 专有的 8 KiB output allocation 是否可移除。
RELATED_OLD_ROUTES: `R002`, `FULL-R002`, `MIX-R014-R002-R019`, `R31A V016`, `R31B V011`, `UB-LIVENESS-X`
MINIMAL_EXPERIMENT: 仅针对 wide FP16 将输出视图改为当前 `yTile` 并跳过该分支的 `outputBuf_` 分配；不改 row/tile 选择、计算顺序、数据搬运和同步。先覆盖 `D=16384/32768`、batchRows 为 1 与大于 1 的情况。

### UBX-H2 — BF16 Param Tile Reused After Promotion

HYPOTHESIS_ID: `UBX-H2-BF16-PARAM-TILE-OUTPUT-REUSE`
MECHANISM: wide BF16 pass 2 先把 `gammaLocal` / `biasLocal` 转入 `xFp32Buf_` / `residualFp32Buf_`。完成转换后，当前 tile 的 `gammaLocal` 不再作为参数读取；可将它改作 `FromFloat` 的 BF16 output view，再按现有次序执行 `Store`。保留两个参数 staging slot 与父版事件次序。
BOTTLENECK: wide BF16 分支同时分配两个参数 staging banks 与一个 `tileElems * sizeof(bfloat16)` output tile；参数升为 FP32 后，当前 gamma staging slot 在 tile store 前有可证明的最后读取点。
DIRECT_PARENT: `R31B V011`, Official `45.16`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: 宽行 `D > kCacheElems`，首查 `D=16384, 32768`；包含 `batchRows > 1` 以覆盖同一 gamma tile 被多个 row 使用的情况。
TARGET_DTYPES: `BF16`
WHY_IT_MAY_HELP: 参数转换后的 staging 字节可承担最终输出，宽 BF16 分支少分配 8192 B/tile；避免额外 output slot，同时保留 gamma/bias FP32 副本供 batch 内后续 row 使用。
WHY_IT_MAY_FAIL: `gammaLocal` 必须在所有 `batchRows` 的 gamma 用途完成后才能作为输出覆写；每 row store 必须完成后再重用同一 gamma view。若编译器或 API 对 alias view 的限制不满足，假设不可行。
UB_IMPACT: 宽 BF16 路径少 `4096 * 2 = 8192 B`；不改 `ChooseWideFullYRows` 及其实际 batchRows。
DMA_IMPACT: GM 参数读与输出写保持不变；不增加 copy。
SYNC_IMPACT: 不改变 event 顺序；现有 MTE3 drain 完成后才允许释放当前 gamma staging slot。
PRECISION_RISK: 低；参数仍先转 FP32，`Muls -> Mul -> Add -> FromFloat` 次序不变。与父版做逐元素输出比对。
DUPLICATE_CHECK: 与 R31A V026 的 pass-2 staging liveness、EPILOGUE STORE-H4 的 BF16 store-source 生命周期相邻。V026 的差异是 release/prefetch 时机；STORE-H4 在通用分支增设专用 output staging 以改变 store/load 可并行性。本假设只在 V011 wide BF16 分支复用已完成参数转换的 gamma slot，不改预取、store policy 或 event 顺序；新增实验可回答现有 2-deep parameter pipeline 中该 slot 是否能安全承载输出。
RELATED_OLD_ROUTES: `R31A V026`, `R31B V011`, `EPILOGUE-FUSE-X STORE-H4`, `ASYNC-TRIPLE-X`
MINIMAL_EXPERIMENT: 只在 `ProcessWideLowPrecision` 的 BF16 编译分支以当前 `gammaLocal` 作为输出 view，并跳过该 wide BF16 `outputBuf_` 分配；保持两参数 buffers、tile loop、row loop、math、Store 与全部 event 原样。覆盖 `D=16384/32768`、batchRows 为 1 与大于 1。

### UBX-H3 — Compact Wide Reduction Partial Pitch

HYPOTHESIS_ID: `UBX-H3-WIDE-REDUCE-PARTIAL-PITCH`
MECHANISM: wide low-precision 以 `ceil(D / 4096)` 个 FP32 partials 写入每 row 的 reduction bank；V011 固定 `kWideFullYReduceStride=16`。对 `8192 < D <= 32768`，tileCount 为 3–8，可将 scratch row pitch 改为 `AlignUp(tileCount, 8)` floats，并按新 pitch 分配与寻址。`ChooseWideFullYRows` 仍用旧 stride 预算，避免改变 row batch。
BOTTLENECK: 静态 UB reservation 为每 retained row 留 16 个 float partial，而目标宽度最多写 8 个；未使用 pitch 空间占着 UB。
DIRECT_PARENT: `R31B V011`, Official `45.16`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
TARGET_SHAPES: `8192 < D <= 32768`；优先 `D=8193` (3 partials), `16384` (4), `16385` (5), `32768` (8)。
TARGET_DTYPES: `FP16`, `BF16`
WHY_IT_MAY_HELP: pitch 从 16 降为 8 floats 时，每 retained row 少占 32 B，最多释放 256 B；每行仍 32-byte 对齐，`ReduceSum` 输入元素、顺序与次数不变。可获得可核算的 UB 余量。
WHY_IT_MAY_FAIL: 节省量很小且仅改变地址 pitch，父版 row batching 固定后可能没有计时收益；若宽度超过 32768，应使用足够 pitch，不能沿用 8-float 配置。
UB_IMPACT: 对目标 D 每 row `-32 B`，row 数不变；大于目标区间时按实际 tileCount 扩大 pitch。
DMA_IMPACT: 无变化。
SYNC_IMPACT: 无变化。
PRECISION_RISK: 低；只改 partial row pitch，不动 partial 生成、ReduceSum 次序或归一化。
DUPLICATE_CHECK: 与 REDUCE-HIER-X 的 partial-sum lifetime 有相似词面；其假设改变 partial 合并时机/归约步骤。本假设只移除每 row 未触碰的 padding slots，所有向量指令与浮点累加顺序不变。与旧 UB-LIVENESS-X 的 live-set/tile budget 研究相邻，但这里固定 `wideFullYRows_`，仅针对 V011 的 16-slot reduction bank。
RELATED_OLD_ROUTES: `R005`, `FULL-R005`, `REDUCE-HIER-X`, `REDUCE-INVSCALE-X`, `UB-LIVENESS-X`, `R31B V011`
MINIMAL_EXPERIMENT: 静态确认 `tileCount<=8` 与各 row 起始地址 32-byte 对齐；只将 wide low-precision reduction allocation/index pitch 改为 8，并保持 chooser、batchRows、partial ReduceSum 与 finish ReduceSum 调用完全不变。再对上列 tileCount 3/4/5/8 形状做父子输出比较。

## Duplicate Audit

| 记录 / 状态 | 相似机制 | 实际差异 | 提供的新价值 / 证据 |
|---|---|---|---|
| `R002` / `FULL-R002` / `MIX-R014-R002-R019`；H1 为相关但不重复 | 保留 y 减少输入重读 | H1 不新增 resident-y，也不省任何 x/res GM 读取；仅将父版已有 `gammaBuf_` 中刚用完的 half tile 改作本 tile 最终输出。R002 正式实现 14/15 Wrong Answer。 | 可单独量出 8 KiB output allocation 的生命周期复用；`归档/历史控制文件/idea-pool-29-routes.md`，`技术路线/技术路线总表.md`，`技术路线/路线成绩表.tsv` |
| `R31A V025` pass-1 staging release/prefetch；`DUPLICATE_REJECTED` | staging lifetime + prefetch overlap | 本路线不提前 load、不改 MTE2/V 重叠；该 donor 的双设备信号和 D=32768 CachedRows 路径不同，也不作为父版。 | 明确排除跨阶段排程；`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V025.md`，`技术路线/全版本记录.tsv` |
| `R31A V026` pass-2 param staging release/prefetch；H2 为相邻但可独立验证 | pass-2 parameter buffer 的生命周期 | H2 不挪动 load/release 点；先完成原有 gamma/bias FP32 转换，再借 gamma staging 存输出。 | 在 R31B V011 的 2-deep pipeline 中隔离“stage 字节作 output”是否安全；`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V026.md`，`线上结果/R31A/V026/submission.asc` |
| `R31A V028` barrier removal；`DUPLICATE_REJECTED` | 删除同一流水线附近的同步 | V028 只删 SetFlag 前的 PipeBarrier；本路线不改任何同步指令。 | 将同步/调度从 UB 生命周期池排除；`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V028.md`，`线上结果/R31A/V028/diff.patch` |
| `R31B V017` / `ASYNC V001`；`DUPLICATE_REJECTED` | 延后 MTE3 drain 或 prologue prefetch | 分别改变 store drain 与 inter-pass MTE2 issue；本路线维持 V011 完整事件顺序，且二者 Official 均低于父版 45.16。 | 只取作历史排程证据，不借其源作父版；`线上结果/R31B/V017/result.json`，`本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/` |
| `FULL-R013` / `ASYNC-TRIPLE-X` / overnight EPI-PIPE-3；`DUPLICATE_REJECTED` | 双缓冲或跨 row/pass 重叠 | 都改变 issue order 或 row 间调度，违反本路线边界。 | 不把 event pipeline 伪装为 buffer-only 方案；`归档/历史控制文件/idea-pool-29-routes.md`，`研究/ASYNC-TRIPLE-X/next-hypotheses.md`，`研究/NEXT-TRACK-B-OVERNIGHT.md` |
| `UB-LIVENESS-X` H1 / EPILOGUE-FUSE-X V003-2；`DUPLICATE_REJECTED` scratch-to-output variant | 用死 scratch 承接后续用途 | 旧 UB-LIVENESS H1 是删死 reservation、对齐预算并放大 tile；EPILOGUE V003-2 用 reduction scratch 改 epilogue accumulator/writeback。此 handoff 不改 tile 选择或算术，仅提出 H1/H2 的特定宽低精度 buffer 生命周期。 | 保持对比范围在 V011 wide path；`研究/UB-LIVENESS-X/next-hypotheses.md`，`研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-V003.md` |
| `EPILOGUE-FUSE-X STORE-H4`；H2 为相邻研究 | BF16 output source 与 MTE2 staging 的 alias 风险 | STORE-H4 在通用路径增加专用 output staging，目的是拆开 store/load source；H2 在 V011 wide path尝试用现有双槽中的 gammaLocal作 output，保留现有 drain/release次序。两者对性能的方向可能相反，Main 需先审阅风险。 | 对照不同 path 的 slot 深度和释放事件，避免把泛化结论直接套入 V011；`研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-STORE.md` |
| `REDUCE-HIER-X` / `REDUCE-INVSCALE-X`；H3 为相关但不重复 | reduction scratch / partial storage | 这些路线改变归约组织或倒数路径；H3 只缩小已写 partial bank 的每-row pitch，保留所有 ReduceSum 操作与浮点次序。 | 可独立验证 reservation 的静态减少，不接管归约算法；`研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md`，`研究/REDUCE-INVSCALE-X/next-hypotheses.md` |
| Wave-2 `BATCH-RESIDENT-X` / `COEFF-LOCALITY-X`；`DUPLICATE_REJECTED` residency candidates | gamma/bias 在 UB 中多次复用 | 其机制是参数驻留或跨 batch 条带缓存；本路线不延长参数驻留，也不减少 parameter DMA。 | 确认参数驻留仍归其他路线；`研究/BATCH-RESIDENT-X/next-hypotheses.md`，`研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md` |
| `MIX-A V007`；`DUPLICATE_REJECTED` barrier-removal candidate | 去掉 MTE2 前同步 | V007 的可执行差异只有一处 `SyncVToMTE2()` 移除；不改变 buffer 分配或 alias。 | 证明 MIX-A 该 revision 与本路线机制不同；`本地实验/MIX-A/V007/handoff.md`，`技术路线/全版本记录.tsv` |

### Architecture evidence map cross-check

`归档/历史控制文件/architecture-evidence-map.md` 未单列 UB lifetime、alias 或 cross-stage reuse 条目。以下是与本路线最接近的机制级证据；它们用于界定相似范围，不等同于同一实现或直接性能结论。

| 地图条目 / 相似机制 | 实际差异 | 独立实验价值与证据路径 |
|---|---|---|
| `row batching` / `PROVEN_WIN`；`R31 LP/row pipelines in V011`、`H001 multi-row tiles`（map L12）；与 H1/H2 的 row reuse 情境相近 | H1/H2 固定 V011 的 row ownership 与 batchRows，不改变 row pipeline；H2 仍须证明 gamma 的所有 row 消费完成前不覆写 staging。 | 分离“保持已知 batching”与“tile 字节别名”的影响；审查 H2 的静态最后读取点。证据：`归档/历史控制文件/architecture-evidence-map.md`、`线上结果/R31B/V011/submission.asc`。 |
| `parameter residency (gamma/bias)` / `PROVEN_WIN`；A001 FastKernel param queue、后续 R31 param cache（map L13）；与 H2 的参数 buffer 相似 | H2 不新增驻留、不减少参数 DMA，也不延长参数有效期；仅尝试在当前 gamma tile 完成 promotion 后复用其原 staging 字节。 | 可独立验证宽 BF16 路径的 gamma staging 与 output view 是否能安全别名，避免把参数驻留收益归给本假设。证据：`归档/历史控制文件/architecture-evidence-map.md`、`研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-STORE.md`。 |
| `cached-y / full-y / retained-u` / `MIXED`；R31B V002 full-y win、R31A V016/V017 D-boundary regressions（map L14）；与 H1 的 retained-y 相似 | H1 不改变 y residency 策略、不省 GM 输入读取；只在当前 y tile 最后一次读取之后，把同一 tile 用作输出目的区。 | 单独回答 V011 wide FP16 是否可移除 8 KiB/tile output allocation；结果不外推为 full-y 策略收益。证据：`归档/历史控制文件/architecture-evidence-map.md`、`研究/UB-LIVENESS-X/next-hypotheses.md`、`线上结果/R31B/V011/submission.asc`。 |
| `pipeline overlap (double-buffer)` / `MIXED`；A001 V017 x/res 2-slot small win、R31 MTE depth flat on T14（map L19）；与 H2 的 multi-slot 参数 staging 同处流水线 | H1/H2 不改变 issue order、prefetch、event 或 barrier；H2 的问题是已转换 gamma 字节能否充当 output，不是 overlap 深度。 | 若获批，能把 alias 安全性与流水线调度收益分开评估；不借用 A001/R31 overlap 结果作为预期收益。证据：`归档/历史控制文件/architecture-evidence-map.md`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V026.md`。 |
| `UB layout` / `MIXED`；I001 DataCopyPad tail proven、WIDE layout exploratory（map L23）；与 H3 的 scratch pitch 相似 | H3 只缩短 reduction partial 的未写 pitch 空间，保留 ReduceSum 调用与次序；它关注 scratch allocation pitch，不与 I001 tail handling 重复。 | 地图指出 WIDE layout 证据仍薄；H3 的独立价值仅是验证 V011 该 scratch bank 的静态尺寸/地址计算，不据此声称时延收益。证据：`归档/历史控制文件/architecture-evidence-map.md`、`研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md`、`线上结果/R31B/V011/submission.asc`。 |

该地图交叉核对没有发现与 H1/H2/H3 完全相同的已记录实现，因此未新增 `DUPLICATE_REJECTED` 假设；相似机制、实现差异及独立验证价值如上。地图对 UB layout 的结论是 `MIXED`，H3 的容量收益很小，不能据此推断性能提升。

R001–R029 总表已逐项阅读，实际相关项为 R002（resident-y）、R005（tile size）、R013（double buffer）、R019（inverse RMS）；其他项目未发现同类 buffer lifetime 机制。FULL 对应实现与结果按 `技术路线/技术路线总表.md` 的 FULL-R002/R005/R013/R029/R030 行复核。Wave-1 对照包括 R31A V025/V026/V028、R31B V017 与 ASYNC V001；Wave-2 对照包括 EPILOGUE-FUSE-X、STORE-EPILOGUE-X、COEFF-LOCALITY-X、REDUCE-HIER-X、ASYNC-TRIPLE-X 与 BATCH-RESIDENT-X。相关差异见上表及 `技术路线/全版本记录.tsv`。

## Route Handoff

ALLOWED_CHANGES: 仅本路线 buffer 分配、视图和 scratch pitch 的静态生命周期研究；本文件不授权实现。
FORBIDDEN_CHANGES: 任何 Candidate/Revision 或 kernel、build、runner、wrapper、共享记录、Online、server3、计时或正确性运行；不改变 row ownership。
EVIDENCE_PATHS:
- `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`
- `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`
- `调度/当前任务.tsv`; `调度/线上候选.tsv`; `调度/本地线上校准.tsv`
- `线上结果/R31B/V011/{submission.asc,submission.sha256,source-meta.json,result.json,diff.patch}`
- `研究/UB-LIVENESS-X/{ROUTE-BRIEF.md,next-hypotheses.md}`; `本地实验/UB-LIVENESS-X/V003/MAIN-REVIEW.md`; `线上结果/UB-LIVENESS-X/V003/result.json`
- `归档/历史控制文件/{idea-pool-29-routes.md,architecture-evidence-map.md}`; `技术路线/技术路线总表.md` FULL/MIX entries; `归档/phase3-before-reset-20260920/管理/路线状态/{R002,R005,R013,R019}.json`
- `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V025.md`; `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V026.md`; `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V028.md`; `线上结果/R31A/{V025,V026,V028}/submission.asc`
- `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-V003.md`; `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-STORE.md`; `研究/EPILOGUE-FUSE-X/STORE-H2B-SPEC.md`; `研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md`; `研究/REDUCE-INVSCALE-X/next-hypotheses.md`
- `研究/ASYNC-TRIPLE-X/next-hypotheses.md`; `研究/ASYNC-OVERLAP-CHAMPION-X/` evidence through `技术路线/全版本记录.tsv` and `调度/本地线上校准.tsv`; `研究/BATCH-RESIDENT-X/next-hypotheses.md`; `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md`; `研究/NEXT-TRACK-B-OVERNIGHT.md`; `本地实验/MIX-A/V007/handoff.md`

PROPOSED_ONE_FACTOR_DIFF: Main 若批准，优先单独评审 UBX-H1；唯一变量为 wide FP16 输出 destination 从独立 output tile 改到已消费完的当前 `yTile`，并省去该 arm 的 output allocation。H2/H3 是独立备选，不应和 H1 合入同一修订。

EXPECTED_LOCAL_PROBES: 尚未执行。H1: wide FP16 `D=16384/32768`，batchRows=1 与 >1；H2: wide BF16 同组形状，重点确保 gamma 在组内多 row 使用期间未被提前覆盖；H3: FP16/BF16 `D=8193/16384/16385/32768`，覆盖 tileCount=3/4/5/8 与 32-byte row pitch。先做父子正确性和逐输出比较，再按本地性能规范进行交错配对；宽度/类型需先与 Judge case mapping 对齐。不得据此文档直接运行。

CHILD_RECOMMENDED_HYPOTHESIS: `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`，理由是最后读取点清晰、数据搬运/同步/数学顺序不动，并可静态省出 8192 B。此建议不代表 MAIN_SELECTED，也不代表实现授权。

OPEN_QUESTIONS:
- `architecture-evidence-map.md` 只有机制级相关条目，没有静态 UB lifetime/alias/cross-stage reuse 的直接证据；是否存在需要补充的已提交路线证据？
- `result.json` 没有保留 Official case 的 shape/dtype 对照；Main 是否有 V011 case mapping 可用于选择本地探针？
- V011 target 上 `Store` 的 MTE3 源读取完成保证是否可由现有 `SyncMTE3ToV` 在两个输出 alias 路径中直接适用？
- Main 是否接受 H2 与 STORE-H4 的 wide/generic path 区分，或要求先由 Main 指定唯一范围？
- H3 的 32-byte row pitch 是否满足 V011 所用 reduction API 对各个 tileCount 的全部要求？

执行边界：本轮只完成只读资料审阅与本 handoff；未创建 Revision，未改 Candidate/kernel/build/runner/shared files；未构建、运行正确性、计时、profiling、访问 server3 或 Online。
