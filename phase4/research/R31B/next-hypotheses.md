# R31B 后续方案研究

范围：仅基于 R31B-V011、V016 源码与现有本地记录做只读分析。当前不改 kernel、不建立新版本；所有方案都需 Main 审阅后再决定是否进入实现。

## H1：BF16 完整 y 缓存压缩

- MECHANISM：将宽路径中保留的 BF16 `y=x+residual` 从 FP32 缓存改为 BF16 缓存，减少每行驻留字节数。
- BOTTLENECK：V016 在 BF16 宽行中为完整 y 保留 FP32；高 D 会压低可容纳的行批量，并促使 `ChooseWideFullYRows` 缩小 tile。
- EXPECTED_SHAPES：BF16 D=12288、32768；重点观察更大 rowCount 和 D=32768。
- WHY_IT_MAY_HELP：较小 y 缓存可降低 UB 压力，可能容纳更大的 tile 或更多行，减少多轮 pass 与同步。
- WHY_IT_MAY_FAIL：缓存转换会改变输出舍入；UB 余量也可能不足以增加当前每核行批量。V016 paired runner 使用 rows=2，若 blockCount=2，每个 block 只有一行，此形状无法体现多行批量收益。
- ASCEND_FEASIBILITY：V016 的 `ToFloat`/`FromFloat` 已使用 `AscendC::Cast` 处理 BF16↔FP32；V016 在目标 CANN 8.5 的编译/链接及 BF16 定向正确性记录已覆盖该转换。仍需为压缩后的整行缓存确认 `LocalTensor` 类型、容量、尾 tile 与 row stride。
- UB/CORE/DMA_IMPACT：y 缓存预计减半；core 映射不变；DMA 流量不变。
- SYNC_IMPACT：现有事件顺序可保留；批量或 tile 改变后需复核缓冲区复用等待。
- PRECISION_RISK：高；尤其关注近零 RMS、极端输入和 BF16 舍入边界。
- DUPLICATE_CHECK：V016 只把低精度宽行 tile 初始值改为 8192，未改变 BF16 y 的 FP32 驻留。该方案改的是 y 的存储精度，与 WIDE-X-FRESH4 的原始输入队列容量、UB-LIVENESS-X 的别名/生命周期方向不同。
- MINIMAL_OFAT_DIFF：仅改 BF16 完整 y 缓存类型及对应转换，不同时改 tile 常量或输出流水。
- EXPECTED_LOCAL_PROBES：先对 BF16 D=12288、32768 做定向正确性；其后每个形状单独完成同一可执行文件资格测试，通过后才做成对测量。
- CLASSIFICATION：NEEDS_MORE_EVIDENCE。

## H2：宽行输出写回双缓冲

- MECHANISM：给低精度宽行输出增加第二个 UB tile，使用 MTE3/V 事件交替缓冲，让当前 tile 的 global store 与下一 tile 的向量处理重叠。
- BOTTLENECK：当前 `ProcessWideLowPrecision` 在每个输出 tile 后等待 MTE3 完成，再继续下一 tile；参数 MTE2 已采用双缓冲，输出写回仍逐 tile 串行。
- EXPECTED_SHAPES：FP16/BF16 D=32768，尤其是每行 tile 数较多的分支。
- WHY_IT_MAY_HELP：输出 tile 写回等待若占据关键路径，双缓冲可把部分 MTE3 延迟藏在后续 V 运算之后。
- WHY_IT_MAY_FAIL：短 kernel 中事件管理成本可能超过收益；当前 UB 余量可能不足以增加输出区。
- ASCEND_FEASIBILITY：现有代码已使用 MTE3/V event 类型和 `outputBuf_`；需验证目标编译器允许的队列关系及事件复用规则。
- UB/CORE/DMA_IMPACT：每个 active core 需增加约一个输出 tile 的 UB；core 数和总 DMA 字节数不变。
- SYNC_IMPACT：增加 V→MTE3 与 MTE3→V 的双槽状态，必须证明每个槽在重用前已写完。
- PRECISION_RISK：低；算术顺序保持不变。
- DUPLICATE_CHECK：V011/V016 的当前 pass 2 均有逐 tile 写回等待；但 ASYNC-TRIPLE-X 的 MTE2/V/MTE3 重叠已覆盖同一输出写回机制，WIDE-X-FRESH4 H4 也研究 output-only TQue double buffer。
- MINIMAL_OFAT_DIFF：只改宽路径输出缓冲数量、store 调度和对应 UB 预算。
- EXPECTED_LOCAL_PROBES：FP16/BF16 D=32768 定向正确性；之后按形状资格结果决定是否做成对测量。
- CLASSIFICATION：DUPLICATE。

## H3：超宽单行跨 core 部分归约

- MECHANISM：将单行的平方和分片到多个 vector core，写出部分和，再以单独归约步骤合并并完成归一化与输出。
- BOTTLENECK：当前 host wrapper 把 blockCount 限制为 `min(availableCoreNum,rowCount)`；rowCount 小时，即使 D 很大也只有少数 core 参与整行工作。
- EXPECTED_SHAPES：rowCount 小于可用 core 数且 D=32768 的 FP16/BF16；优先验证 rowCount=1、2。
- WHY_IT_MAY_HELP：超宽行可使用更多 core 并行搬运和平方和计算。
- WHY_IT_MAY_FAIL：额外 kernel launch、workspace 写读和归约等待可能超过单行计算收益；小 D 尤其不合适。
- ASCEND_FEASIBILITY：需要修改 host launch 流程并新增 partial-sum 存储/合并步骤；现有单 kernel ABI 不提供跨 block 同步，不能假定有安全的全局屏障。
- UB/CORE/DMA-IMPACT：每个 core 的 UB 需求可下降；活跃 core 增加；新增 partial-sum workspace 流量。
- SYNC_IMPACT：跨 kernel stream 顺序承担阶段同步，新增一次或多次 launch 边界。
- PRECISION_RISK：中；分片归约顺序改变 FP32 累加次序，需按现有 dtype 容差验证。
- DUPLICATE_CHECK：V011/V016 都按行划分 core；但 WIDE-X-FRESH4 H3 同样提出把超宽行分片到多个 vector core、先写部分和再做输出阶段，属于同一跨核归约架构。
- MINIMAL_OFAT_DIFF：仅为超宽低 rowCount 形状引入分片平方和与合并路径，保留现有路径作为其余形状的回退。
- EXPECTED_LOCAL_PROBES：先做 rowCount=1、2 的定向正确性与 workspace 边界覆盖；性能对比须计入整段多 kernel 延迟。
- CLASSIFICATION：DUPLICATE。

## H4：降低行归约的 V/S 往返

- MECHANISM：研究使用目标 toolkit 支持的 vector reciprocal-square-root/标量广播路径，减少当前平方和读取、sqrt 结果读取中的 V/S 往返。
- BOTTLENECK：宽路径每行先把 partial sum 归约到标量，再经过 V/S handoff 得到平方根和倒数；大 rowCount 时该串行段会重复。
- EXPECTED_SHAPES：FP16/BF16 D>=12288，rowCount 较大且单行计算并非由 DMA 完全主导的形状。
- WHY_IT_MAY_HELP：若归约结果和 invRms 能留在 vector 数据通路，可减少标量同步停顿。
- WHY_IT_MAY_FAIL：目标指令/API 未确认；当前生成代码也可能已经合并部分操作；近似倒数平方根可能引入额外误差。
- ASCEND_FEASIBILITY：先核对 Ascend 910B3 对应 toolkit 头文件、API 与编译产物；未确认前不写 kernel 代码。
- UB/CORE/DMA-IMPACT：预计 UB、core 数和 DMA 字节数不变。
- SYNC_IMPACT：目标是减少 V/S 事件；MTE2/MTE3 时序保持不变。
- PRECISION_RISK：中高；关注 invRms 舍入误差在宽行归一化后的累计影响。
- DUPLICATE_CHECK：该 invRms 向量路径与 REDUCE-INVSCALE-X 的 R019 归一化方向、MIX-A H04 的向量 inverse-RMS 方向重合；不因目标宽度不同视为独立机制。
- MINIMAL_OFAT_DIFF：仅替换 invRms 的计算与广播路径，不改分片算法或 tile 配置。
- EXPECTED_LOCAL_PROBES：先对 FP16/BF16 D=12288、32768 定向正确性；取得编译证据后再评估形状资格测试。
- CLASSIFICATION：DUPLICATE。

## 2026-09-25 Track-B screening

- H1 是四项中唯一未发现同机制活跃路线的方案，暂列 NEEDS_MORE_EVIDENCE。只有在目标宽行形状能让每个 block 处理多行、且压缩后确实增加 `wideFullYRows_` 时，UB 节省才可能转为可见收益；V016 rows=2 的 paired 形状本身不满足这一前提。
- H2 与 ASYNC-TRIPLE-X、WIDE-X-FRESH4 H4 重复；H3 与 WIDE-X-FRESH4 H3 重复；H4 与 REDUCE-INVSCALE-X、MIX-A H04 重复。均保留原记录并改列 DUPLICATE，不计入独立候选数。
- 未补入新候选：输出 affine FMA 已在 DTYPE-SPECIAL-X 与 MIX-A 研究；参数 tile 跨 batch 行复用已由 V016 pass 2 实现；整行重读输入的路线形态存在于 R31B V001，之后 V002 full-y 方案记录为胜出。重新包装这些机制不能构成新的独立方向。
- 本轮审阅了四项，独立且仍可研究的方案只有 H1；3–5 项独立候选批次尚未形成。等待 Main 对 V016 的下一决定期间，不创建新版本，也不改 Candidate。

## 2026-09-26 Track-B continuation

- Ascend C `Cast` 文档列出 Atlas A2/A3 支持 float↔bfloat16_t；V016 自身的转换 helper 和 CANN 8.5 编译/正确性记录进一步证明当前目标路径可用。H1 的主要未知已收敛为改用 BF16 整行缓存后的 UB 预算、地址/尾部覆盖和输出舍入误差，不再是 Cast API 是否存在。
- `ChooseWideFullYRows` 当前以 FP32 字节数估算 BF16 的 y 缓存。若 Main 后续审阅 H1，需按实际缓存类型重新估算每行容量，并确认只有 rowCount 大于 blockCount 的形状才可能增加每 block 的批行数；V016 paired runner 固定 rows=2，不能体现该批行收益。
- 本轮只研究 R31B 文件、历史记录和 API 资料；未新增假定已完成跨路线去重的候选。现有独立候选仍只有 H1，研究批次尚未达到 3–5 项。
