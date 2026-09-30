# SMALLMID-DATAFLOW-CHAMPION-X Track-B 交接

状态：Track-B 完成；未选实现；等待 `MAIN_SELECTED=YES`。

## 父版与边界

- 直接父版：`R31B V011`；Official anchor：`45.16`。
- 父版源码 SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。已对照 `线上结果/R31B/V011/source-meta.json` 的 `submission_sha256`、`route_source_sha256`、`judge_payload_sha256`，`线上结果/R31B/V011/submission.sha256` 及已提交 `submission.asc` 实际值，三处一致。V011 自身的 `parent_revision=V010`，本路线仍以 V011 为直接父版。
- 仅限 `D<=4096`、每核既有行段内的批处理固定成本、UB 数据复用、初始化和 dispatch。不得改变 `blockCount`、行到核分配、`beginRow/localRows` 归属，也不碰大 D 分块。
- 本轮仅写本文件；未建 Revision、未改 Candidate/kernel 或共享台账；未构建、测正确性、计时或访问 server3。

## 证据与重复项

- `线上结果/R31B/V011/submission.asc`、`diff.patch`：现有小行批处理与 `(128,4096]` `ProcessNarrowMidOverlap`；父源码内 BF16 mid 分支已在归约后用 `xBuf_` 写回，FP16 分支另用 `outputBuf_`。
- `线上结果/R31B/V011/source-meta.json`、`submission.sha256`：父版来源身份。`技术路线/全版本记录.tsv`：V010 mid overlap；V014 只 build、批大小 8→32，无性能证据；V015 单行 mid dispatch 本地配置失败。`调度/本地线上校准.tsv`：V010 无有效配对本地数据。
- `归档/历史阶段/retired-routes/H001/HANDOFF.md`、`线上结果/H001/V008/submission.asc`、`source-meta.json`：H001 `D<=1024` 高行批处理与非对齐 `CopyRows` 先例；V008 Official 15/15、29.04，主要变化是 wide path 归约，没有小 D 单因素配对数据。
- `归档/历史工作区/MID-X/architecture-metadata.md`、`V001/kernel.asc`、`V002/kernel.asc`、`V003/kernel.asc`：覆盖 mid batch、参数驻留和 `D<=128` 路径；V001 部分中宽 case 有耗时记录但线上仅 2/15，V002/V003 为 Runtime Error。V002 记录了 ReduceSum 槽位对齐及 padding 单位风险；其整体行分配不纳入本路线。
- V011 官方 case 形状映射缺失；以下收益均未实测，需先以本地探针确认实际 `localRows`、批次数和路径。

## 假设

### SMD-H2：FP16 mid 输出复用已读完的 x UB

- `HYPOTHESIS_ID`: `SMD-H2-FP16-MID-OUTPUT-ALIAS`
- `MECHANISM`: 对 `2048<D<4096` 的 FP16 mid path，以归约后已读完的 `xBuf_` 代替 `outputBuf_` 完成转换、affine 与 Store；对应 shape 不预留 `outputBuf_`。
- `BOTTLENECK`: mid path 为最终 FP16 输出单独预留 tile-sized UB。
- `DIRECT_PARENT`: `R31B V011`; source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Official `45.16`。
- `TARGET_SHAPES`: FP16，`R={1,8,32,128}`，`D={2049,3072,3073,4095}`。
- `TARGET_DTYPES`: FP16。
- `WHY_IT_MAY_HELP`: 少预留最多 8192 bytes/core；同函数 BF16 分支已有 `xBuf_` 写回用法。
- `WHY_IT_MAY_FAIL`: 数据访问与算术数量不变，UB 地址变化未必带来时延变化；错误的复用时序会覆盖尚未完成的 Store。
- `UB_IMPACT`: 对目标 shape 少预留 `4096*sizeof(half)`；不增加容量。
- `DMA_IMPACT`: GM 读写字节及 Store 数量不变。
- `SYNC_IMPACT`: 不变；保留输入释放、下一行前的 MTE3 完成等待及最终写回等待。
- `PRECISION_RISK`: 低；数值顺序不变，主要风险是 buffer 生命周期。
- `DUPLICATE_CHECK`: 对照 `线上结果/R31B/V011/submission.asc` 的 BF16 mid 写回与 FP16 `outputBuf_` 用法；未发现 FP16 同路径省去该预留的历史记录。
- `RELATED_OLD_ROUTES`: R31B V010/V011；H001 V008；MID-X V001-V003。
- `MINIMAL_EXPERIMENT`: 只改目标 shape 的输出 tensor 来源与 `outputBuf_` 初始化条件；正确性跑 `R={1,8,32,128}`、`D={2049,3072,3073,4095}`，并确认下一行 MTE2 仅在前一行 Store 完成后复用 `xBuf_`；通过后对同形状做父版/候选配对计时。
- `UNCERTAINTY`: 中；有同函数 BF16 先例，FP16 时序及收益未测。

### SMD-H3：非对齐 tiny-D 使用每核局部批处理

- `HYPOTHESIS_ID`: `SMD-H3-UNALIGNED-LOCAL-BATCH`
- `MECHANISM`: 对 `localRows>1 && D<=128` 的非对齐行，在现有每核行段内用 32-byte row stride 和逐行 `DataCopyPad` 暂存；保留父版逐行归约与 affine，不改行归属。
- `BOTTLENECK`: tiny generic 路径逐行执行 MTE2/Vector 同步和标量归约设置。
- `DIRECT_PARENT`: `R31B V011`; source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Official `45.16`。
- `TARGET_SHAPES`: `R={64,256,1024}`，`D={65,73,127}`；另测小行控制组。
- `TARGET_DTYPES`: FP32、FP16、BF16。
- `WHY_IT_MAY_HELP`: 每批共用 MTE2-to-Vector 等待；复用已分配的 `localRows`，不改核间工作。
- `WHY_IT_MAY_FAIL`: pad 和边界操作可能抵消固定成本节省；MTE3 仍按原始 D 写回。
- `UB_IMPACT`: 增加批内 padding，限制 `batchRows` 使 rowStride 总量不超过现有 tile 预算。
- `DMA_IMPACT`: 有效 GM 字节不变；非对齐行仍逐行搬运，不主张减少 DMA 数量。
- `SYNC_IMPACT`: 目标是批量搬入后共用 MTE2-to-Vector 等待；保留输出源复用所需等待。
- `PRECISION_RISK`: 低到中；逐行算术不变，但 padding 不得参与 ReduceSum 或输出。
- `DUPLICATE_CHECK`: `线上结果/H001/V008/submission.asc` 的 `CopyRows` 已有非对齐 padding 后批处理先例；MID-X V001/V002 也尝试过整体批处理且结果不可靠。此假设只补 V011 `D<=128` 非对齐边缘，不搬用其行分配；重复度仍偏高。
- `RELATED_OLD_ROUTES`: H001 V008；MID-X V001/V002/V003；R31B V011。
- `MINIMAL_EXPERIMENT`: 只新增该条件下的批处理 helper；固定父版 `blockCount` 与 `beginRow/localRows`。三 dtype、`D=65,73,127` 先跑正确性，再对 `R=64,256,1024` 配对计时并含小行控制组。
- `UNCERTAINTY`: 中高；收益依赖逐行同步占比，pad 的逐 dtype 单位和 ReduceSum 有效长度需确认。

### SMD-H5：FP32 小 D 批次延后 Store 等待

- `HYPOTHESIS_ID`: `SMD-H5-DEFER-BATCH-STORE-WAIT`
- `MECHANISM`: 只改 `ProcessSmallFp32ContiguousBatched`，将 8192-element value buffer 分成两个 4096-element slot 交替使用；只在复用对应 slot 前等待 MTE3，保留最终 drain。
- `BOTTLENECK`: 当前每批 Store 后立即执行 `SyncMTE3ToV`，串行等待输出搬运。
- `DIRECT_PARENT`: `R31B V011`; source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Official `45.16`。
- `TARGET_SHAPES`: FP32，`D={64,256,512,1024,2048}`；选能形成 2、3 个以上 local batch 的 R，另含单 batch 控制。
- `TARGET_DTYPES`: FP32。
- `WHY_IT_MAY_HELP`: 父版 `rowsByInputBuffer` 限制每批不超过 4096 elements，现有 8192-element buffer 可容纳双 slot；MTE3 读一个 slot 时，Vector 可准备另一个。
- `WHY_IT_MAY_FAIL`: 单 batch 无收益；Store 时间可能短；slot 复用时序若错会覆写未完成输出。
- `UB_IMPACT`: 总预留不变，分为两个最多 4096-element slot。
- `DMA_IMPACT`: GM 字节和 Store 数量不变，尝试与下一批计算重叠。
- `SYNC_IMPACT`: 多批次的 MTE3-to-Vector wait 移到 slot 复用点，保留最终完成等待；目标是减少可见串行等待。
- `PRECISION_RISK`: 无算术变化；风险集中在 UB 生命周期。
- `DUPLICATE_CHECK`: `技术路线/全版本记录.tsv` 的 R31B V014 只改 batch cap 且无性能结果；H001/MID-X 有队列双缓冲，但不是 V011 该函数的 value-cache 双 slot Store 等待。
- `RELATED_OLD_ROUTES`: H001 V008；MID-X V001/V002；R31B V011/V014。
- `MINIMAL_EXPERIMENT`: 只改 slot offset 与 wait 位置，不改 batch cap、算术、DMA 形状或行分配。先验证父版全目标正确性，再测 `D=64,256,512,1024,2048`，覆盖 1、2、3+ 批次，同设备配对计时。
- `UNCERTAINTY`: 中；生命周期依据明确，收益取决于 MTE3 延迟能否被下一批计算覆盖。

## 给 Main 的建议

优先评审 SMD-H5，其次 SMD-H2。SMD-H3 与 H001/MID-X 旧路相邻，仅在主控确认非对齐 tiny-D 形状仍未覆盖后考虑。静态参数预留未进入三项，因没有运行时开销证据；tiny dispatch 未进入三项，因与 MID-X V003、R31B V015 相邻且缺少本地数据。研究排序不代表实现选择，仍等待 `MAIN_SELECTED=YES`。
