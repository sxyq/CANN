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
