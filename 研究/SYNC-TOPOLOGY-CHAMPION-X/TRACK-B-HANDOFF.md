# SYNC-TOPOLOGY-CHAMPION-X Track-B 交接

状态：研究完成，等待 Main 选择；本轮不创建 Revision，也不改 kernel 或共用记录。

共同父版本：`R31B-V011`，Official `45.16`，源码 SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，来源 `线上结果/R31B/V011/submission.asc`。

下列三项分别调整 MTE3 完成等待、MTE2 参数就绪等待、以及独立 V 工作与 MTE2 等待的相对次序。每项均以 V011 为直接父版本，单项验证，不合并实施。形状名中的 `rows` 指输入总行数，探针使用 `blockCount=1`，代码不改行分配。

## H1：FP32 全缓存路径按 MTE3 槽位等待

- **机制**：在 `ProcessWideFp32FullCacheRows` 删除 tile 循环入口无条件等待 `storeRelease0/1` 的两段；保留已有的逐槽等待、事件分配与尾部排空。这样前一 tile 的 Store 可与下一 tile 的 gamma/bias MTE2 搬运及 V 计算并行。Store 调用、地址、次数、次序均不变。
- **瓶颈**：当前两个 MTE3_V 等待位于 gamma/bias Load 之前；当上一 tile Store 尚未结束时，MTE3 排空先挡住了下一 tile 的 MTE2/V 工作。
- **直接父版本**：R31B-V011，SHA 如上。
- **形状/dtype**：主探针 `rows=2, D=12288, blockCount=1, FP32`；该宽路径保留两行且有三个 tile，两个 Store 事件槽都会使用。`D=8192` 可作未改路径对照。
- **为什么可能有效/失败**：逐槽等待只在同一事件槽要再次发 Store 前触发，可把上一 tile 的 MTE3 完成时间藏在下一 tile 的参数搬运和向量工作下。若 Store 已在这段时间内完成，或 MTE3 与后续工作争用造成反向影响，收益会很小或为负。
- **UB/DMA/同步影响**：UB 不变；Load/Store 数量和传输字节不变；事件数不变，只把两处整 tile 入口等待改为现有的逐槽等待时点，尾部等待保留。
- **精度风险**：算术与舍入顺序不变，预期逐位一致。主要风险是事件槽过早复用或 Store 源切片在完成前被覆写；逐槽等待和尾部等待必须保留。
- **重复路线对照**：R31B V017 已延后低精度 `outputBuf_` 的 MTE3_V 等待，Official 为 `44.68`，低于 V011；本项改在 FP32 全行缓存路径，利用其已有双槽事件和逐槽等待，属于相邻的排空等待方向，重复风险中等。R31A V021 也延后过 MTE3_V 等待，结果为 `NEEDS_ONE_MORE_LOCAL`。证据：`线上结果/R31B/V017/diff.patch`、`线上结果/R31B/V017/source-meta.json`、`本地实验/R31A/V021/local-result.json`。
- **旧路线与证据路径**：父源码等待位点 `线上结果/R31B/V011/submission.asc:2181-2193`，逐槽等待与尾部排空 `:2214-2244`；V017 的不同输出缓冲做法见上列路径。
- **最小单因子探针**：只移除 `:2183-2190` 两处入口等待，保留 `:2214-2220` 与 `:2235-2244`。先对父子版本做同形状正确性与同码稳定性，再以交错设备事件测 `rows=2, D=12288, FP32`；`D=8192` 只作未改路径对照。

## H2：低精度参数双缓冲先发下一块 DMA

- **机制**：在 `ProcessWideLowPrecision` 第二遍，把当前 tile 的 `WaitFlag<MTE2_V>(prd)` 从下一 tile 预取之前移到下一 tile 的 gamma/bias `Load` 与 `SetFlag<MTE2_V>` 之后、当前 gamma/bias 首次被 V 使用之前。下一槽现有的 `WaitFlag<V_MTE2>(prel)` 仍在覆写该槽之前；双缓冲事件和深度不变。
- **瓶颈**：当前 tile 的参数就绪等待先挡住下一 tile 的 MTE2 命令发射；当当前参数搬运尚未完成时，MTE2 可能没有及时收到下一块工作。
- **直接父版本**：R31B-V011，SHA 如上。
- **形状/dtype**：主探针 `rows=2, D=12288, blockCount=1, FP16`；相同形状 `BF16` 可作一次迁移确认。`D=8192` 不走此宽路径，可作未改路径对照。
- **为什么可能有效/失败**：先发下一槽搬运可让 MTE2 在当前就绪等待期间接续工作；若 MTE2 已连续满载、当前等待已被其他工作隐藏，或事件发射顺序本身不能排队，收益会落在噪声内。当前 tile 参数仍须在消费前等待就绪。
- **UB/DMA/同步影响**：UB、DMA 数量和传输内容不变；事件数与双缓冲深度不变；只调整当前 MTE2_V 等待相对下一槽 Load 的位置。
- **精度风险**：不改算子及运算次序，预期逐位一致。风险集中在错等事件或消费未完成的 gamma/bias；正确性先于测时。
- **重复路线对照**：V011 已有低精度参数双缓冲；V009 改的是 FP32 full-y 的 MTE2 队列深度，Official `43.81`；MIX-A V007 删除的是窄中单行路径的反向 `V_MTE2` 释放同步，且没有形成稳定收益；R31B V019 删除 `SyncVToMTE2` 全同步，不触及此处 `MTE2_V` 参数就绪事件。证据：`技术路线/全版本记录.tsv`、`本地实验/MIX-A/V007/{diff.patch,handoff.md,local-result.json}`、提交 `ebee3ded:研究/R31B/handoff-v019.md`。
- **旧路线与证据路径**：当前等待和下一槽预取见 `线上结果/R31B/V011/submission.asc:3267-3297`；V011 双缓冲说明见同文件 `:3245-3255`。
- **最小单因子探针**：只移动当前 `WaitFlag<MTE2_V>(prd)`，不改任何 `V_MTE2` 释放等待、Load、事件 ID 或运算。先做 `rows=2, D=12288, FP16` 父子正确性与同码稳定性，再交错测时；如有稳定方向，再加同形状 BF16 确认。

## H3：FP32 参数搬运期间执行已有归一化向量段

- **机制**：在 `ProcessWideFp32FullCacheRows` 第二遍，gamma/bias `Load` 后，把现有的 `Muls(valueRow, valueRow, invRmsValues[batchRow])` 循环及其 V 屏障移到 `SyncMTE2ToV()` 之前；保留等待，并确保它仍在首次读取 gamma/bias 的 `Mul` 之前。每元素的 `Muls → Mul(gamma) → Add(bias)` 数值顺序不变。
- **瓶颈**：当前参数搬运发出后立即等待 MTE2_V；缓存 y 和 invRms 已可用，但此时 V 管线没有处理该 tile 的归一化工作。
- **直接父版本**：R31B-V011，SHA 如上。
- **形状/dtype**：主探针 `rows=2, D=12288, blockCount=1, FP32`；`D=8192` 可作未改路径对照。先确认该父形状的输出可与现有参考稳定对齐。
- **为什么可能有效/失败**：Muls 与 gamma/bias MTE2 目标缓冲互不相同，可在参数搬运进行时处理缓存 y。若 MTE2 搬运在 Muls 开始前已完成，或 Muls 太短而无法覆盖等待，端到端变化会很小。
- **UB/DMA/同步影响**：UB 不变；DMA 次数、内容不变；同步调用数不变，只把现有 V 工作放到 MTE2_V 完成等待之前。`valueFp32Buf_` 与参数所在的 `xBuf_/residualBuf_` 分开使用。
- **精度风险**：保留每元素运算先后与舍入点，预期逐位一致。需留意同一 `valueRow` 在 MTE3 尚未完成时被改写的风险；此候选不移动 MTE3_V 等待，且仍按 V011 时序执行 Store。
- **重复路线对照**：ASYNC-OVERLAP-CHAMPION-X V004 曾将首个参数块搬运提前到 invRms 尾段，属于相邻的 MTE2/V 重叠思路；本项不改首块搬运时间，而只把现有 Muls 放入逐 tile 参数等待窗口，重复风险中等。R31B V018 删除低精度 pass-1 的 Muls 拷贝，改的是运算量；本项保留运算和数值顺序。证据：`技术路线/全版本记录.tsv`、提交 `b9d7bc09:本地实验/R31B/V018/diff.patch`。
- **旧路线与证据路径**：父源码 Load/等待/现有 Muls 的顺序见 `线上结果/R31B/V011/submission.asc:2179-2206`；ASYNC V004 与 R31B V018 分别见上述证据。
- **最小单因子探针**：只把现有 Muls 循环和紧随其后的 V 屏障移到 `SyncMTE2ToV()` 之前；`Mul`、`Add`、Store 及全部事件不动。先以 `rows=2, D=12288, FP32` 做父子正确性、同码稳定性及交错测时，`D=8192` 作未改路径对照。

## 历史项处置

- R31A V028 只证明其 batch affine 函数中 `V_op → PipeBarrier → SetFlag` 的尾部 barrier 可删，单位成本约 `0.01 us`；它是 H1/H2/H3 的重复对照材料，不把 barrier 删除另列为第四条。证据：`线上结果/R31A/V028/diff.patch`、`研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V028.md`。
- R31B V018 的 pass-1 V 算子/中部 barrier 删除、V019 的两处 `SyncVToMTE2` 删除均为正确性逐位一致、测时落入噪声；其父版本是 V017。证据：`7adf582c:研究/R31B/handoff-v018.md`、`ebee3ded:研究/R31B/handoff-v019.md`。
- MIX-A V003 加入 `SyncVToMTE2` 以保护多行复用；V007 在 `localRows==1` 前提下删除窄中预读释放同步，当前协议结果没有建立收益。证据：`本地实验/MIX-A/V007/{handoff.md,MAIN-REVIEW.md,local-result.json}`。
- R028 改的是跨行归约与标量 `GetValue` 汇总，不纳入本路线；它触及归约，超出本次范围。证据：`归档/phase3-before-reset-20260920/提交/单方案/R028-标量同步削减/README.md`、`归档/历史阶段/historical-branches-20260924/independent__full-r028-scalar-sync-reduction-i001/files/提交/单方案/FULL-R028-SCALAR-SYNC-REDUCTION/I001/kernel.txt`。

**停止点**：三条假设已交 Main 评估。本 child 不选方向、不创建 Revision、不改实现；等待 Main 指定后续。

## Track-B 补充字段

本节补齐路线边界、执行约束和跨路线去重信息；上方 H1–H3 的候选描述保持原样。所有探针都只是 Main 选定之后的建议，本 child 不据此创建 Revision。

### 路线级约束

- `ROUTE_BOUNDARY`：只研究 R31B-V011 宽行路径里已有 MTE3_V / MTE2_V 等待的位置与先后关系，以及不改变每元素运算顺序的既有 V 工作相对等待的位置。只讨论 `ProcessWideFp32FullCacheRows` 与 `ProcessWideLowPrecision`。
- `ALLOWED_CHANGES`：候选获 Main 选择并创建 Revision 后，最多改动已有等待点的相对位置；H3 可移动既有 Muls 循环及紧随其后的 V 屏障。事件、缓冲区、运算、搬运和 Store 均沿用 V011。
- `FORBIDDEN_CHANGES`：不改 tile 宽度/数量、Store 次数/范围、归约、dtype 分支、UB 大小/别名、行到核分配、blockCount 或 dispatch；不增加/删除事件、DMA、算术或缓冲区；未获 Main 选择前不碰 Kernel/Candidate、不创建 Revision。
- `PARENT_SOURCE_SHA`：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，R31B-V011，Official `45.16`。
- `EVIDENCE_PATHS`：
  - 父版身份及等待位置：`线上结果/R31B/V011/source-meta.json`、`submission.asc:2181-2244`、`submission.asc:3245-3297`。
  - R001–R029 / FULL-R 关系：`技术路线/技术路线总表.md:19-47,57-85`、`技术路线/技术路线图.md:273-280`；特别相关的是 R013/FULL-R013，R028/FULL-R028 是标量归约同步，R015 是多行 DMA，R005 是 tile，R016 是行调度。
  - R31A/R31B/MIX：`本地实验/R31A/V021/handoff.md`、`线上结果/R31A/V028/diff.patch`、`线上结果/R31B/V017/{diff.patch,source-meta.json,result.json}`、`研究/R31B/handoff-v018.md`（commit `7adf582c`）、`研究/R31B/handoff-v019.md`（commit `ebee3ded`）、`本地实验/MIX-A/V007/{handoff.md,MAIN-REVIEW.md,local-result.json}`、`线上结果/MIX-A/V003/diff.patch`。
  - 前一波相邻证据：`技术路线/全版本记录.tsv:82`、`线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/{diff.patch,result.json,ONLINE-HANDOFF.md}`；EPI 的既有算术 donor 见 `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/{diff.patch,result.json}`；写回 donor 见 `线上结果/STORE-EPILOGUE-X/V003/{diff.patch,result.json}`。
  - Wave-2 已提交 handoff：STORE `80676bc73334fc0b7c3a912353fc02abdfec16b1:研究/STORE-EPILOGUE-W2-X/TRACK-B-HYPOTHESES.md`；EPI `5031a5fa253407ab6f65f16b00e59e21312dd1e2:研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md`；SELECTIVE-FASTPATH `9e5a573112a1dbed7b2450963b5e9961415f53b5:研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md`；SMALLMID `47dacfa21f7da5158cb855567b81d8163576ff6a:研究/SMALLMID-DATAFLOW-CHAMPION-X/track-b-handoff.md`。只引用这些已提交文件，未读取其工作树内容。

### 每项单因子差异

- `H1 / PROPOSED_ONE_FACTOR_DIFF`：仅移除 V011 `ProcessWideFp32FullCacheRows` tile 循环入口的两处 `WaitFlag<MTE3_V>`（父源码 2183–2190）；保留已有同槽等待、事件分配、Store 语句和尾部 drain。不得改变 Store 数量、地址、tile 或行分配。
- `H1 / EXPECTED_LOCAL_PROBES`：Main 选定后先比较父子正确性和父版同码稳定性；主形状 `rows=2,D=12288,FP32,blockCount=1`，`D=8192` 是不命中宽路径的控制。形状与实际分支命中须记录；通过后再做同设备交错 P/C。
- `H2 / PROPOSED_ONE_FACTOR_DIFF`：仅把 `WaitFlag<MTE2_V>(prd)` 从当前 tile 参数消费前移到下一槽 gamma/bias Load 与 SetFlag 之后、当前 tile 首次读 gamma/bias 之前；保留原 `V_MTE2` 槽位释放等待、事件 ID、DMA 和双缓冲深度。
- `H2 / EXPECTED_LOCAL_PROBES`：先用 `rows=2,D=12288,FP16,blockCount=1` 做父子正确性与同码稳定性；`BF16` 仅在 FP16 探针有稳定方向后确认，`D=8192` 作未改路径控制。之后才做同设备交错 P/C。
- `H3 / PROPOSED_ONE_FACTOR_DIFF`：仅把 gamma/bias Load 后现有的 Muls 循环及其 V 屏障，移到 `SyncMTE2ToV()` 之前；保留等待，并保证 gamma/bias 首次被 Mul 读取前已完成等待。Mul、Add、Store、事件和每元素运算顺序不变。
- `H3 / EXPECTED_LOCAL_PROBES`：先用 `rows=2,D=12288,FP32,blockCount=1` 做父子正确性与同码稳定性，`D=8192` 作不改路径控制；之后才做同设备交错 P/C。记录实际 `batchRows`，不改行分配。

- `H1 / CROSS_ROUTE_DUPLICATE_AUDIT`：R013/FULL-R013 已有双缓冲流水，但 H1 不加缓冲或深度；R31A V021 是最接近的等待移动，最新记录没有合格 P/C timing；R31B V017 是最强相邻项，改低精度路径的 MTE3 等待且 Official `44.68`，但 donor 同时含不同 tile 策略。MIX-A V007 改的是窄中单行反向 `V_MTE2` 释放点；前一波 STORE V003 改每行 Store 分块，EPI V002 改算术。H1 的 FP32 V011 单等待探针可单独量化迁移性，但与 V017 重复风险高。
- `H2 / CROSS_ROUTE_DUPLICATE_AUDIT`：R013/FULL-R013 是已有双缓冲的广义先例；前一波 ASYNC-OVERLAP V001 与 H2 都想让参数搬运覆盖计算，V001 提前首 tile 到 invRms 循环期间且 Official `44.17`，H2 只移动后续 tile 的 `MTE2_V` wait，首 tile 不动。R31B V019 移除的是反向 `SyncVToMTE2`，MIX-A V007 移除的是 `V_MTE2` 释放；两者不等同于参数就绪事件。STORE/EPI 前一波 donor 分别改 MTE3 写回和算术，可用 FP16 `D=12288` 单变量探针区分。
- `H3 / CROSS_ROUTE_DUPLICATE_AUDIT`：R028/FULL-R028 是跨行标量归约，不属本项；R31A V028 删除 affine 尾部 barrier，而 H3 保留 barrier 数量；R31B V018 删除 Muls 拷贝、改变工作量，H3 保留 Muls/Mul/Add，只移动等待关系。前一波及 Wave-2 EPI 都涉及 affine 运算，但 EPI V002 改算术表达，Wave-2 EPI H3 改跨行循环分组/屏障数；H3 仅将既有 Muls 放入参数 DMA 等待窗口。STORE 改写回粒度，MIX-A 改反向同步，均可与本项分开测量。

### 跨路线重复审计

下表逐项对照当前 Wave-2 四条已提交 handoff。相似性用于标出重叠风险；独立验证只说明最小差异可单独测量，不代表候选已获选。

| 假设 | STORE | EPI | SELECTIVE-FASTPATH | SMALLMID |
|---|---|---|---|---|
| H1 | 同属 MTE3 写回时序。STORE-W2 改 V002 合并写回的整行 Store 发出点/分块；H1 留在 V011 FP32 分 tile 路径，只后移循环入口等待，Store 调用不变。可用 D12288 单独隔离。 | EPI-W2 H3 调整 Mul/Add 跨行分组及屏障数；H1 不改算术或屏障。性能差异可归于 MTE3 等待点。 | SELECTIVE H1 运行完整 V017 BF16 donor，包含既有低精度 tile 策略；H1 只动 V011 FP32 等待点。二者是直接相邻的 MTE3 等待做法，重复风险高；FP32 单独探针可确认 V017 结果是否能迁移，不能把 donor 成绩归因给单个等待。 | SMALLMID H5 针对 D≤2048 的 FP32 小行双槽 value buffer；H1 是 D=12288 的宽行 full-cache，既不切分 buffer 也不增槽。D 区间和函数路径不同，可分开测。 |
| H2 | STORE-W2 调 MTE3 Store issue/分块，H2 调 MTE2 参数就绪等待，搬运/写回引擎与改动点不同。 | EPI-W2 改 affine 算术/屏障组织；H2 保持算术，仅让下一 tile 参数 DMA 更早发出。 | SELECTIVE H1 变化是 MTE3 等待，且执行完整 BF16 V017 donor；H2 只移动 V011 宽低精度的 MTE2_V 等待，事件槽和搬运量不变。可在 FP16 目标上独立测。 | SMALLMID H3 也涉及 MTE2→V 等待，但针对 D≤128 非对齐行的每核批处理与 padding；H2 是 D=12288 低精度 gamma/bias 双缓冲预取，不批量化、不改变 DMA 形状。引擎相邻、工作路径可分。 |
| H3 | STORE-W2 移 Store 发出点或改变写回 chunk；H3 不动 Store，只把既有 Muls 放入 MTE2 等待窗口。 | EPI-W2 H3 同样保留每元素 Muls→Mul→Add，但跨行分组以减少屏障；本项不改循环分组或屏障数，只改 Muls 相对 MTE2 等待的先后。EPI 探针需满足其 `batchRows>=2` 形状条件，本项固定两行探针，分别对 V011 测量。 | SELECTIVE H4 使用完整 EPI V002 算术 donor并改变算术表达/Store 源；H3 保留每元素顺序和 Store 源，仅重排已有工作与等待。形状、精度风险和单因子差异均可分离。 | SMALLMID 主要改变小 D buffer 复用、批处理或等待；H3 不改 buffer 生命周期、DMA 或小 D 路径，只重排宽 FP32 的 Muls 与参数就绪等待。D12288 可独立确认等待是否被有效覆盖。 |

### 历史与前一波判定

- R013 / FULL-R013 已覆盖广义 MTE2/V/MTE3 双缓冲流水，FULL-R013-V001 Official `18.76`；本轮不新增缓冲或流水深度，只问 V011 已有事件中某个等待是否过早。R028 / FULL-R028 虽名为同步削减，实质是逐行 GetValue 与归约标量汇总，超出边界；R015/FULL-R015 改多行 DMA，R005/FULL-R005 改 tile，R016/FULL-R016 改行/核分配，也都不作为本轮候选。
- R31A V021 曾移动 FP32 CachedRows 的 MTE3 等待，但最新 handoff 记录 shape same-binary 未达标、没有 P/C timing，因此只能证明改动相邻，不能证明收益。R31A V028 删除的是 batch-affine 尾部 barrier，记录的单 barrier 成本约 `0.01 us`；H1–H3 均不删该类屏障。
- R31B V017 延后低精度输出等待，Official `44.68`；它与 H1 最接近，但还包含不同 dtype/tile donor，不能独立归因。R31B V018/V019 的 V 侧算子/barrier 与 `SyncVToMTE2` 删除没有形成超出噪声的收益；H1/H2/H3 改的是具体等待的顺序并保留必要的槽位释放和尾部 drain。MIX-A V003 加入反向同步保护多行复用，V007 在单行窄中路径删释放等待也未建立稳定收益；本轮不改该反向 V_MTE2 释放点。
- 前一波 ASYNC-OVERLAP V001 把首个 gamma/bias tile 提前到 invRms 循环期间，局部宽 FP16 有小幅收益但 Official `44.17`，低于 `45.16`。H2 不改首 tile 发出时点，只移动每轮当前 tile 等待相对下一槽预取的位置；这一历史结果提高了重复审查要求，不能当作 H2 的实测依据。前一波 STORE V003 改 chunked writeback、Official `44.38`；EPI V002 改算术、Official `44.96`，二者均未超过锚点，也都不是本轮等待点的单因子证据。

`CHILD_RECOMMENDED_HYPOTHESIS`：H2 仅建议 Main 优先审阅，不代表选定。它只移动已有 MTE2_V wait，DMA 数量、事件数、双缓冲深度和算术均不变，差异边界比 H1 的 V017 等待重叠更窄，也比 H3 的算术管线重排更易单独解释。最终选择仍由 Main 作出。

`OPEN_QUESTIONS`：

1. H2 的关键前提是当前 V 侧等待会推迟后续 MTE2 Load 的发出；现有提交记录没有该精确等待顺序的设备证据，Main 需判断是否值得作为单因子候选。
2. H1/H3 的 `rows=2,blockCount=1` 是否可由正式 runner 原样到达，以及 V011 实际 `batchRows`/tileWidth，需在 Main 选择后先确认；不得为命中形状改行分配。
3. H1 必须确认 MTE3 仍在同槽复用前完成；H3 必须确认 Muls 可在 gamma/bias DMA 期间安全访问独立的 value 缓冲。两项都需先做正确性，不可由静态推断替代。
4. Official testcase 到 shape/dtype 的映射在现有证据中未知；即使未来局部探针有收益，也不能据此推断 Official 总分变化。
