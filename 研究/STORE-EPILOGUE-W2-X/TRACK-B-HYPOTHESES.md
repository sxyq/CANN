# STORE-EPILOGUE-W2-X — Track-B handoff

- 日期：2026-09-30
- 直接父版本：`STORE-EPILOGUE-X V002`
- 父源码 SHA：`59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`
- Official anchor：45.07
- 分支：`w2/m1/store-epilogue`
- 状态：`MAIN_SELECTED=NO`

本轮仅写路线研究。V003 是 Wave-1 供体证据，不是父版本。没有创建 Revision、修改 Candidate/kernel、构建、测时或访问 server3。

## 路线边界与证据

仅讨论 store issue、最终 drain、写回依赖、行内 chunk 边界和写回启用条件。排除 reduction、dtype 分路、多行 DMA 算法、tiling 和 row scheduling。

- V002 在 `ProcessWideFp32FullCacheRows` 中使用 `tileCount>=4 && rowWidth%8==0`；命中时每行发出一次整行 Store。`1x32768 FP32` 本地中位数 -5.62%，6/6 配对方向有利；Official 45.07。父源码文件 SHA 与任务给定值一致。
- V003 使用 K=2 分块；本地 `8x16384 FP32` 为 -5.64%（6/0），`1x32768 FP32` 为 -3.15%。Official 44.38，较 V002 低 0.69。仅作 donor evidence。
- V002 的函数尾 drain 后立即释放 event ID，没有后续工作可隐藏等待；V002 的逐 tile 分支已有 Store 前同 slot wait。
- 既有 gap 在 commit `acc92000` 的 `研究/STORE-EPILOGUE-X/next-hypotheses.md`；handoff 在 commit `76cbb94d` 的 `研究/STORE-EPILOGUE-X/handoff-2026-09-29.md`。其中旧 H4 与本轮依赖等待种子的差异相同，列为重复。

## H1 — 整行 Store 发出点

- `HYPOTHESIS_ID=STORE-W2-H1-ISSUE-POINT`
- `STATUS=READY_FOR_MAIN_REVIEW`
- `MECHANISM`：合并条件、整行 Store 次数及所有 event 操作不变；把唯一整行 Store 从 tile 循环之后移到最后一个 tile 的算术和 `PIPE_V` barrier 之后、循环退出之前。
- `BOTTLENECK`：最后一次 Store issue 前的循环退出指令可能推迟 MTE3 启动。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`。
- `TARGET_SHAPES`：`1x32768 FP32` 主探针；`8x16384 FP32` 复验。`1x16384 FP32` 暂不测，V002 记录为父子共同 golden mismatch。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：循环退出指令可与同一笔异步 Store 重叠，减少 issue 空隙。
- `WHY_IT_MAY_FAIL`：最后 tile 后没有后续算术可供重叠，省下的指令可能低于测量噪声。
- `UB_IMPACT`：无。
- `DMA_IMPACT`：每行一次、相同长度的 Store；字节数与调用数不变。
- `SYNC_IMPACT`：wait、flag、event ID 和 drain 顺序不变，仅移动 Store issue 点。
- `PRECISION_RISK`：数值路径不变；须保证所有行最后 tile 的算术完成后才 Store。
- `DUPLICATE_CHECK`：与 V003 不同；V003 将一行改为两次 Store，本项保留 V002 的单次整行 Store。与旧 gap 的 chunk 机制也不同。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V002/V003；ASYNC-OVERLAP-CHAMPION-X 仅作流水时序相邻路线。
- `MINIMAL_OFAT_DIFF`：只移动 merge 分支中的整行 Store 代码块；不改条件、循环顺序、调用数或 event 操作。
- `MINIMAL_EXPERIMENT`：基于 V002 只做上述代码移动；先跑 `1x32768 FP32` correctness 与 same-binary，再按现行协议做至少 4 组相邻交错 P/C。
- `EXPECTED_LOCAL_PROBES`：`1x32768 FP32` 主测，`8x16384 FP32` 复验；每个形状独立通过 same-binary 后才配对。
- `UNCERTAINTY`：高；没有 Store issue-gap 的独立测量，理论收益偏小。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2179-2289`、`本地实验/STORE-EPILOGUE-X/V002/local-result.json`、`线上结果/STORE-EPILOGUE-X/V002/result.json`。
- `RECOMMENDATION_TO_MAIN`：保留为低优先级对照候选；只有 Main 认为小幅 issue 点移动值得测时再选。

## H3 — 两段写回的末段长度

- `HYPOTHESIS_ID=STORE-W2-H3-TAIL-CHUNK-POLICY`
- `STATUS=READY_FOR_MAIN_REVIEW`
- `MECHANISM`：仅对 V002 merge-on 行改用 K=2；令首段结束于 `tileCount-2`，t=8 时为 6+2。
- `BOTTLENECK`：V002 要等整行完成才发 Store；末段长度可能决定最后暴露的写回量。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`；V003 只供机制和性能对照。
- `TARGET_SHAPES`：`1x32768 FP32`（t=8）是区分 6+2 与 V003 4+4 的主探针；`8x16384 FP32`（t=4）仅复验 2+2，不区分末段策略。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：若 6-tile 首段 Store 可在末两 tile 算术期间完成，最终待完成的 Store 仅覆盖两 tile。
- `WHY_IT_MAY_FAIL`：6-tile 首段比 V003 的 4-tile Store 更长，发出后也只剩两个 tile 可重叠；首段未完成时，第二笔 Store 会排在其后。V003 的 Official 结果低于 V002，风险明确。
- `UB_IMPACT`：整行驻留 buffer 大小与布局不变。
- `DMA_IMPACT`：每行 Store 从一次变两次，总字节数与 copy primitive 不变；不跨行。
- `SYNC_IMPACT`：沿用 V002 的 2-deep event ring；两次 Store 各自遵循现有依赖，批尾 drain 保持不变。
- `PRECISION_RISK`：算术顺序及 dtype 不变；需保证 chunk 边界按 tile 对齐、范围不越行，且 Store 完成前不覆盖 UB 源区。
- `DUPLICATE_CHECK`：与 V003 同属 chunked writeback，但唯一新变量是末段边界；V003 的 split 为 `tileCount/2`。V003 不作本轮父项。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V002/V003；commit `acc92000` 的旧 H1 为较宽泛的分块提早写回想法，本项只检验 t=8 的末段长度变化。
- `MINIMAL_OFAT_DIFF`：只在 V002 merge-on 分支增加两次 chunk Store，边界设为 `splitTile=tileCount-2`；不改启用条件、batch 计算、ring 深度或循环调度。
- `MINIMAL_EXPERIMENT`：仅先测 `1x32768 FP32`；完成 correctness、same-binary，再按协议做至少 4 组交错 P/C。保留原始配对样本，不加 kernel 内计时插桩。
- `EXPECTED_LOCAL_PROBES`：`1x32768 FP32` 为判别形状；`8x16384 FP32` 只作 V003 2+2 复验，不用于确认 6+2。
- `UNCERTAINTY`：高；chunk 的真实传输时长和最后 drain 暴露量未知。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2174-2289`、`线上结果/STORE-EPILOGUE-X/V003/diff.patch`、`线上结果/STORE-EPILOGUE-X/V003/result.json`、commit `64cf431a` 的 `本地实验/STORE-EPILOGUE-X/V003/local-result.json`。
- `RECOMMENDATION_TO_MAIN`：保留为 V003 的窄化后续；不要从 V003 本地优势推断线上收益，先评估首段 Store 未完成时的尾部代价。

## H4 — 多行 t=3 写回启用条件

- `HYPOTHESIS_ID=STORE-W2-H4-MULTIROW-ELIGIBILITY`
- `STATUS=NEEDS_MORE_EVIDENCE`
- `MECHANISM`：仅改 V002 predicate：`tileCount >= (batchRows > 1 ? 3 : 4) && rowWidth % 8 == 0`。t=3 只对已有每核多行 batch 开启整行 Store。
- `BOTTLENECK`：多行 t=3 仍逐 tile Store，描述符数量高于 V002 已合并的 t>=4 行。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`。
- `TARGET_SHAPES`：候选 `128x12288 FP32`，仅在现有 `availableCoreNum`、runner 与 tiling 得出 `batchRows>1` 时适用；`1x12288 FP32` 为单行控制，`128x16384 FP32` 为 t=4 控制。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：可把可达的多行 t=3 从每行三次 Store 改为一次，减少描述符，同时保留单行 t=3 的 V002 行为。
- `WHY_IT_MAY_FAIL`：V001 曾显示短行延后写回代价可超过描述符节省；目标测试集也可能没有 `batchRows>1` 的 t=3 wide FP32 输入。
- `UB_IMPACT`：无；使用 V002 已分配的整行缓存。
- `DMA_IMPACT`：命中行从每行三次 Store 变一次，字节数不变，不跨行。
- `SYNC_IMPACT`：命中行沿用 V002 整行 Store 与现有 event ring；其他形状逐字节维持原分支。
- `PRECISION_RISK`：数值路径不变；新增命中行仍须满足 V002 对整行起点及完整 UB 范围的要求。
- `DUPLICATE_CHECK`：与 commit `acc92000` 旧 H2 的全局 tileCount 阈值不同；本项只对 `batchRows>1` 开 t=3，单行仍需 t>=4。与多行 DMA 不同，每行仍独立写回。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V001/V002/V003；旧 gap H2；R015 仅作“不跨行”的边界参照。
- `MINIMAL_OFAT_DIFF`：只替换 merge predicate；不动 tile width、batch limit、block count、row ownership 或 Store body。
- `MINIMAL_EXPERIMENT`：先按现有 host blockCount 与 tiling 静态确认 `128x12288` 的 `batchRows>1`；若可达，只改 predicate 并以该形状做 correctness、same-binary、至少 4 组交错 P/C。
- `EXPECTED_LOCAL_PROBES`：主测 `128x12288 FP32`；控制 `1x12288 FP32`、`128x16384 FP32`。若主形状不受现有 runner 支持或 `batchRows<=1`，记为无可用探针，不改 row scheduling。
- `UNCERTAINTY`：中高；谓词差异明确，形状可达性与净性能收益未实测。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2088-2105`、`:2174-2289`、`:3577-3585`、`本地实验/STORE-EPILOGUE-X/V002/local-result.json`、`调度/当前任务.tsv:39`。
- `RECOMMENDATION_TO_MAIN`：先确认现有 workload 中有可达的多行 t=3 形状；确认后再与 H1/H3 比较，不触碰 row scheduling。

## 筛除与 Main 建议

- H2（最终 drain timing）：V002 最终等待后紧接 event ID 释放；没有合法的 drain 后工作，当前无可写成 OFAT 的代码差异。
- H5（store dependency）：与 commit `acc92000` 旧 H4 为同一差异；旧 H4 的 evidence path 已列于上方，不另建候选。
- Main 可讨论的 distinct 候选仅为 H1、H3、H4。建议先确认 H4 的现有形状可达性；H1 预期上限较小；H3 有 V003 Official 反向风险。`MAIN_SELECTED=NO`，收到 Main 选择前不实现。
