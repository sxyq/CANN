# R31B 后续方案研究

范围：仅基于 R31B-V011、V016 源码与现有本地记录做只读分析。当前不改 kernel、不建立新版本；所有方案都需 Main 审阅后再决定是否进入实现。

## H1：BF16 完整 y 缓存压缩

- MECHANISM：将宽路径中保留的 BF16 `y=x+residual` 从 FP32 缓存改为 BF16 缓存，减少每行驻留字节数。
- BOTTLENECK：V016 在 BF16 宽行中为完整 y 保留 FP32；高 D 会压低可容纳的行批量，并促使 `ChooseWideFullYRows` 缩小 tile。
- EXPECTED_SHAPES：BF16 D=12288、16384、32768；重点观察 D=32768，源码预算推导预期该宽度的 tile 数减少最多。
- WHY_IT_MAY_HELP：较小 y 缓存可降低 UB 压力，可能减少宽行 tile 数。按 V016 当前 176 KiB 预算公式估算，BF16 D=12288/16384/32768 的 tile 宽度约从 7680/6656/2560 增至 8192/8192/6656，对应 tile 数从 2/3/13 降至 2/2/5；这是源码算式推导，尚未由编译结果验证。
- WHY_IT_MAY_FAIL：缓存转换会改变输出舍入；在上述三种形状中，预算公式仍只容纳每个 block 一行，故 rows=2、blockCount=2 的 paired runner 不会获得多行批处理收益。D=12288 的 tile 增幅也很小，转换成本可能抵消节省。
- ASCEND_FEASIBILITY：V016 的 `ToFloat`/`FromFloat` 已使用 `AscendC::Cast` 处理 BF16↔FP32；V016 在目标 CANN 8.5 的编译/链接及 BF16 定向正确性记录已覆盖该转换。仍需为压缩后的整行缓存确认 `LocalTensor` 类型、容量、尾 tile 与 row stride。
- UB/CORE/DMA_IMPACT：y 缓存预计减半；core 映射不变；DMA 流量不变。
- SYNC_IMPACT：现有事件顺序可保留；批量或 tile 改变后需复核缓冲区复用等待。
- PRECISION_RISK：高；尤其关注近零 RMS、极端输入和 BF16 舍入边界。
- DUPLICATE_CHECK：V016 只把低精度宽行 tile 初始值改为 8192，未改变 BF16 y 的 FP32 驻留。该方案改的是 y 的存储精度，与 WIDE-X-FRESH4 的原始输入队列容量、UB-LIVENESS-X 的别名/生命周期方向不同；与 V016 共用 UB 预算目标，但机制不同，且须等 V016 决定后再考虑。
- MINIMAL_OFAT_DIFF：仅改 BF16 完整 y 缓存类型及对应转换，不同时改 tile 常量或输出流水。
- EXPECTED_LOCAL_PROBES：先验证 UB 算式是否给出预期 tile 宽度并覆盖 D=12288/16384/32768；若仍只是一行且 tile 不增加，或 D=32768 没有明显减少 tile 数，则证伪收益前提。若 Main 日后授权独立版本，再做 BF16 定向正确性；每个形状分别通过同一可执行文件资格后才可考虑成对测量。
- EXPECTED_INFORMATION_GAIN：高；一次源码预算推导加目标编译/正确性结果即可判定 UB 节省是否转为 tile 数减少，以及中间舍入是否可接受。
- LIKELY_GLOBAL_UPSIDE：中低；最可能集中于 BF16 D=32768，FP16、FP32 和较窄形状不受益。
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

## 2026-09-26 Track-B second screen

### H5：BF16 参数扩宽去重

- MECHANISM：BF16 pass 2 连续两次执行相同的 gamma/bias `ToFloat`。保留第二组转换及其后的 `PipeBarrier<PIPE_V>`，移除第一组重复转换；V011 行号为 3303–3312，V016 为 3306–3315。
- BOTTLENECK：参数 tile 被加载后，同一组 gamma/bias 被重复扩宽到相同的 FP32 暂存区；两组调用之间没有读取或修改这些暂存值的代码。
- EXPECTED_SHAPES：BF16 宽行 D=12288、16384、32768；实际宽路径条件为 D>8192，且更长行含有更多参数 tile。paired 形状每个 block 只处理一行时，重复工作占比更高。
- WHY_IT_MAY_HELP：每个参数 tile 少两次向量类型转换和一个同步点；数值公式、输出顺序和 DMA 流量不变。
- WHY_IT_MAY_FAIL：编译器可能已经合并重复写入；即使保留了两组转换，它们相对整行搬运和输出计算也可能很小。
- ASCEND_FEASIBILITY：两组 `ToFloat` 的输入、输出暂存区和 valid 长度相同。保留末组转换后的同步可继续约束其消费者。目标为 CANN 8.5.0.alpha002，V016 的 BF16 转换已有目标编译与定向正确性记录。
- UB/CORE/DMA-IMPACT：UB 分配、core 映射和 DMA 字节数不变；少两条向量转换。
- SYNC-IMPACT：保留转换完成到后续计算之间的 `PIPE_V` 同步，参数及输出事件配对不变；重复块移除后需确认之前的向量指令仍由保留的同步覆盖。
- PRECISION_RISK：低；保留的转换使用相同输入、舍入路径和有效长度。
- DUPLICATE_CHECK：与 H1 的 y 缓存压缩、已筛选的 affine FMA 输出算术不同；作用对象是 gamma/bias 参数预处理。此前 R31B 研究记录未列出此处的重复扩宽。shared scheduler 对 DTYPE-SPECIAL-X 只给出宽泛的 dtype/conversion 描述；此方向与其主题相邻，但没有证据显示具体重复参数扩宽机制相同。未读取该路线源码，跨路线去重状态标为待 Main 复核。
- MINIMAL_OFAT_DIFF：只移除 `ProcessWideLowPrecision` pass 2 中第一组完全相同的 BF16 gamma/bias 转换及其同步；不改其他向量操作。
- EXPECTED_LOCAL_PROBES：先取得目标编译器的向量指令清单，确认两组转换是否都保留；若产物只含一组，或两组写入没有形成额外向量指令，则证伪。若重复指令确实存在且 Main 日后授权独立版本，再做 BF16 D=12288/16384/32768 定向正确性；设备租用与对应形状资格均获准前不测时。
- EXPECTED_INFORMATION_GAIN：高；指令清单可直接回答编译器是否已消除此重复，避免为无效源码变化付出实现成本。
- LIKELY_GLOBAL_UPSIDE：低；改动仅影响 BF16 宽路径的参数预处理，且单 tile 转换可能只占总延迟一小部分。
- CLASSIFICATION：NEEDS_MORE_EVIDENCE。

### H6：pass 1 行列索引递增化

- MECHANISM：将 pass 1 扁平单位索引的 `u / tileCount` 与余数解码改成显式 row/tile 递增状态；当前单位可复用上一轮已算出的下一单位位置。V011 行号为 3125–3154，V016 为 3128–3157。
- BOTTLENECK：每轮先解码当前 `(row,tile)`，随后再解码 `(row,tile)` 的下一项；下一轮又会重新计算同一项。`tileCount` 由运行时 rowWidth 和 tileWidth 得出，编译器是否消除这些重复商余数运算尚未确认。
- EXPECTED_SHAPES：FP16/BF16 D=12288、16384、32768 且 `tileCount >= 2`；若每个 block 有多行，重复索引工作更多。当前 paired rows=2、blockCount=2 对应每 block 一行，预计收益受限。
- WHY_IT_MAY_HELP：降低启动下一组 MTE2 load 前的标量索引运算，并缩短地址生成依赖；数据搬运、算术和 tile 次序不变。
- WHY_IT_MAY_FAIL：编译器可能已用循环强度折减或公共子式合并消除此工作；每行 tile 数不多时，向量计算和 MTE 延迟可能完全掩盖标量开销。
- ASCEND_FEASIBILITY：只涉及整数循环状态，不需新增 Ascend C API。必须保持 `u` 奇偶缓冲选择、row 尾部、tile 尾长及现有 event ID 的 wait/set 顺序。
- UB/CORE/DMA-IMPACT：UB、core 映射和 DMA 字节数不变。
- SYNC-IMPACT：不改同步事件；需证明行切换处的 A/B 缓冲奇偶次序与原扁平序列一致。
- PRECISION_RISK：无算术变化；地址或尾部错误会导致错误读写。
- DUPLICATE_CHECK：V016 只改变低精度宽行 tile 初始值；现有 R31B 历史记录了缓存、参数复用和 MTE/V 流水方向，未记录把当前/下一项索引解码改成递增状态。与 H1 的存储格式、H5 的 BF16 参数转换去重均互不依赖；已读的 shared scheduler 摘要未出现相同索引递推机制，未读取其他路线源码。
- MINIMAL_OFAT_DIFF：只改 pass 1 `(row,tile)` 的当前/下一项索引生成，保留扁平单位奇偶值和全部 event 操作。
- EXPECTED_LOCAL_PROBES：先取目标编译器生成的标量指令；若没有重复除法/余数解码，或编译器已将其改成递增状态，则证伪。若指令仍重复且 Main 日后授权独立版本，再做 D=12288/16384/32768 定向正确性，覆盖完整 tile、尾 tile 和 row 切换；后续测时需先满足设备租用与对应形状资格。
- EXPECTED_INFORMATION_GAIN：高；可以从已构建的目标指令判断商余数是否真的重复，直接决定这条窄优化是否值得实现。
- LIKELY_GLOBAL_UPSIDE：低；只减少标量地址生成，DMA 与向量运算量不变，预期仅在索引指令落入关键路径时有收益。
- CLASSIFICATION：NEEDS_MORE_EVIDENCE。

### 本轮去重与 API 依据

- 不提出通用删除 `PipeBarrier<PIPE_V>` 的方案。CANN 8.5 官方 `PipeBarrier(ISASI)` 文档说明它阻塞同一流水，并要求有数据依赖的同流水指令之间插入同步；V016 pass 1 存在 Add→Mul→ReduceSum 等实际依赖。V016 正式构建的本地 CMake 配置含 `--cce-auto-sync`，但现有产物未提供可读的 AICore 指令清单，因此不能据此认定具体手工 barrier 可删。
- 再调宽行 tile、输出 MTE3/V 重叠、跨 core 部分归约和向量 inverse-RMS 分别与 V016 当前 tile 方案或本文件 H2/H3/H4 已记录的活跃路线研究重合；重新命名不会增加独立方向。
- 改写 `y * invRms * gamma + bias` 的向量合并/重排，已在既有 DTYPE-SPECIAL-X 与 MIX-A 研究覆盖，且会改变舍入次序；本轮不另列。
- 路线编译记录确认目标为 Ascend910B3 / DAV_2201，工具链为 CANN 8.5.0.alpha002。CANN 8.5 官方 `PipeBarrier` 页面：<https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/API/ascendcopapi/atlasascendc_api_07_0271.html>；DAV_2201 架构资料：<https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/920beta2/programug/Ascendcopdevg/docs/zh/guide/programming_guide/advanced_programming/hardware_implementation/architecture_spec/npu_arch_2201.md>。9.2 页面记载该架构 UB 总容量为 192KB，并说明默认预留 256B 与 8KB；使用 `--cce-disable-asc-reserved-ubuf` 时不再预留 8KB。这个页面比目标工具链新，仅作为 UB 资源背景；具体构建选项和可用容量仍以本路线证据为准。资料结论不构成任何 timing 许可。
- 结果：在 H1 之外找到 H5、H6 两个源码机制上独立的方向；两者均需先核实编译器产物，暂不列 READY_FOR_MAIN_REVIEW。本轮未改变 Candidate，V016 仍待 Main 的测量决定。

## 2026-09-26 Track-B screened batch

- 本批计入三项独立的 R31B 源码机制：H1 改 BF16 完整 y 的存储格式；H5 去掉 pass 2 重复 gamma/bias 扩宽；H6 将 pass 1 当前/下一 `(row,tile)` 商余数解码改为递增状态。三项分别作用于缓存字节数、向量转换、标量地址生成，彼此不依赖。
- H1 与 V016 共用 UB 预算目标，但不重复其 tile 初值变化；H2/H3/H4 分别与已记录的输出双缓冲、跨 core 归约、inverse-RMS 方向重合，均不计入本批。输出 affine 算术与参数跨行复用也已由旧记录覆盖。
- H5 与 DTYPE-SPECIAL-X 的共享摘要存在 dtype/conversion 主题邻近；摘要没有给出相同的 BF16 参数重复扩宽机制，故当前记为“未见精确重复，待 Main 复核”，不访问该路线源码。三项均为 NEEDS_MORE_EVIDENCE，尚无一项可直接进入实现。
- 后续证伪次序：H1 先复算 UB 预算并确认目标编译的 tile/行配置；H5 先看向量指令清单是否保留重复转换；H6 先看标量指令是否仍重复商余数解码。只有机制通过各自证伪点、Main 明确处理 V016 后，才讨论后续实现；任何测时另需新 Main-1 租用和精确形状资格。

## 2026-09-26 Track-B evidence closure

- 身份未变：V016 SHA-256=`9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`；直接 Parent V011 SHA-256=`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。本轮只读，没有启动 NPU 或 timing。
- H1：V016 `ChooseWideFullYRows` 预算公式估算 y 缓存、4 个低精度 I/O tile、FP32 work tile 和 reduction partial；`outputBuf_` 是另一项 TBuf，公式没有单列。按该函数返回值，BF16 y 每元素字节数从 4 改为 2 后，D=12288：2→2 tiles，D=16384：3→2，D=32768：13→5；三种形状均仍是每 block 一行。这些是预算函数的源码推导，不是总 UB 占用测量或分配成功证明。最大潜在变化在 D=32768；当前材料不能验证精度变化，也不能证明实际设备耗时改善。分类仍为 `NEEDS_MORE_EVIDENCE`。
- H5：V011/V016 源码均存在连续两组相同的 BF16 gamma/bias `ToFloat` 调用。现有正式构建 CMake 指定 `-O3` 和 `--cce-auto-sync`，但远端构建目录仅留 `.o`/`.alink` 等产物，没有 `.s`、`.asm`、`.ll`、`.bc` 或 `.ir`；因此无法从现存产物判断优化后是否仍执行两组转换。分类仍为 `NEEDS_MORE_EVIDENCE`。
- H6：V011/V016 源码均分别计算当前 `u / tileCount` 和下一项 `(u + 1) / tileCount` 及其余数；`tileCount` 来自运行时 rowWidth 与选定 tileWidth。源代码足以确认有重复解码表达式，但不能判断 CCE 优化是否将其合并或转换为递增状态。现有编译产物没有可读 AICore 指令清单，分类仍为 `NEEDS_MORE_EVIDENCE`。
- 最低后续成本与顺序：先做一次不启动设备的精确源码指令输出，复用 V011/V016 当前编译选项，同时筛查 H5 的转换指令数与 H6 的索引解码；`bisheng --help` 已确认支持 `-S` 和 `-save-temps`，但需先确认输出确实包含 AICore specialization，只有 host 汇编不构成证据。本轮未执行该编译。H1 的预算计算无需再构建；如 Main 后续授权新方向，再分别验证 BF16 精度，性能结论仍需独立租用和形状资格。
- 去重依据仅限 R31B 历史与 canonical scheduler 摘要：H1 与 V016 共用 UB 目标但机制不同；H5 与 DTYPE-SPECIAL-X 的摘要主题相邻，是否重复仍待 Main 判断；H6 在已读摘要中未发现同机制方向。未读取其他 Route Candidate 或源码。
