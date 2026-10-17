# TINY-FIXED-OVERHEAD-CHAMPION-X — 固定成本证据审阅

DATE: 2026-10-03
TRACK: `TRACK-B research; V001 implementation handoff`
REVISION: `V001 (H2 selected by Main)`
TINY_MAIN_SELECTED: `YES (TINY-H2-ROW-OWNERSHIP-FASTFORM / V001)`
OFFICIAL_CHAMPION: `R31B-V011`, score `45.16`, 15/15
DIRECT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

本文件整理 R016/SCHED 历史覆盖、V011 固定成本候选及 case 1/3/5 的证据边界。研究层保留 ownership 算术直达、V011 `Process` 分派短路、单 tile 循环控制、窄中多行未使用的 `paramReady` ID 四个机制；Main 已从中选择 H2 建立 V001。所有 proxy 均非 Official 输入，本记录不作路线生命周期决定。

## 结论摘要

- V011 与 case 1、3、5 的公开结果可证实共享同一份提交源码、`run_kernel` Host wrapper、dtype 分派语句和 Kernel 模板入口。它们是否使用相同 dtype specialization、`Process` 分支、wide 子路径、实际 block 数和每 block 行数，现有证据不能确认。
- 三个 testcase ID 在 V011 与 SCHED-ROWGROUP-X V001 结果中一致，但两份 Official JSON 均不带 workload shape、dtype、launch metadata 或 dispatch trace。case 1/3/5 的耗时不能用来推断这些字段。
- `ACTIVE_CORE_COUNT` 改变 Host launch block 数及 device ownership，是 R016/FULL-R016、SCHED-ROWGROUP 与 CASE47-H2 已覆盖的调度轴。它保留为 `DUPLICATE`；当前不提出把该方向恢复成实现项。
- C2C update：SELECTIVE-FASTPATH H3 用精确 FP32 proxy-shape allowlist 选择 STORE V003 donor，未命中则回退 V011。TINY H2 仅在 V011 执行时起作用；Official case1/3/5 和本地 `[M,256]` proxy 与 allowlist 的关系均未确认。
- `ACTIVE_CORE_COUNT` 与 R016/SCHED 的调度轴重复，保持 `DUPLICATE`。
- 除已批准的 H2/V001 外，下文机制仍需更多路径或目标编译证据；它们不属于当前 Candidate。

## Official 路径映射审计

目标 testcase：

| Official case | Testcase ID | V011 timeUs | 结果中 shape/dtype/path |
|---|---|---:|---|
| 1 | `6a9a9a99bf41025d6013eb8a` | 5.44 | 未提供 |
| 3 | `6a9a9a99bf41025d6013eb92` | 5.16 | 未提供 |
| 5 | `6a9a9a99bf41025d6013eb9a` | 9.93 | 未提供 |

可确认的共同层次：

1. `run_kernel` 对三个 case 使用同一份 V011 Host 入口实现。Host 按输入 dtype 选择 `float`、`half` 或 `bfloat16_t` Kernel 实例，再以计算所得 `blockCount` 发射 grid。
2. 每个 Kernel block 都进入同一模板入口 `add_rms_norm_bias_custom<T>`，创建 `AddRmsNormBiasKernel<T>`，调用 `Init` 后调用 `Process`。
3. `Process` 首先读取 `widePath_`；非 wide 分支再按模板 dtype、rowWidth、对齐条件和 `localRows` 选择多条专用函数或通用路径。
4. 因而“共用入口”成立；“共用 specialization/dispatch/ownership”没有成立证据。即使模板类型相同，`Process` 仍可能因 width、alignment、wide 与每 block ownership 条件走不同路径。

需要取得的官方逐例 manifest 至少应给出 testcase ID、输入完整 shape、dtype、epsilon、布局及设备可用核数。要确认 dispatch，还需以这些输入重放 Host 计算并记录 `rowCount`、`rowWidth`、`availableCoreNum`、`blockCount`、每个代表性 `blockIdx` 的 `localRows`，以及实际命中的函数名。Official 计时起止点也须单独确认；没有该定义，不把 Host wrapper 的前置验证、Kernel entry 或 launch 开销说成 `timeUs` 的组成部分。

## 固定成本分类

下表区分源码中可见的工作与需要目标编译产物/计时边界才能确认的成本。源码出现某个 API 不等同于该 API 在 Official 计时内产生同等成本。

| 成本 | V011 可见行为 | 证据边界 |
|---|---|---|
| Host wrapper | 验证指针、rank、dtype/shape 一致性和乘法溢出；展平 leading dimensions；计算 `blockCount`；选择 dtype Kernel 并发射。 | 是否纳入 Official `timeUs` 未知。case 输入未知，不能估计各 Host 分支成本。 |
| Kernel entry / dispatch | Host 按 dtype 选择模板实例；设备端 `if constexpr` 是编译期选择。`Process` 另有 wide、宽度、对齐与 `localRows` 的运行时路径判断。 | Official path map、目标 specialization 指令与 Official 计时边界均未知；仅将设备运行时分派短路列为待证候选。 |
| `InitBuffer` / Pipe 初始化 | `Init` 设置 GM tensor、wide 标志，并按 `rowWidth`、dtype 调用 `pipe_.InitBuffer`；wide 与 non-wide 使用不同布局。 | 当前没有逐例资源报告或生成物证据证明调用序列形成每次 launch 的固定指令成本；源码调用数不等于运行时成本。 |
| Event ID 管理 | 窄中函数申请/释放 `inputReady`、`paramReady`、`inputRelease`。当 `localRows>1` 时 `residentParams=true`，`paramReady` 不做 Set/Wait，但仍申请和释放。 | V011 源码支持单个未使用 ID 的候选；目标 CANN 8.5 生命周期规则与生成物尚未确认。其他 event 均有对应 Set/Wait。 |
| `SetFlag` / `WaitFlag` 与 PipeBarrier | V011 在 DMA、向量与标量消费之间有同步；通用与窄中函数有多处 `PipeBarrier<PIPE_V>`。 | V011 候选点尚无依赖图证明可删。R31A V028 有另一函数内 barrier 的非承重发现，不可直接转用。 |
| 每 block ownership | non-wide `Process` 计算商、余数、`beginRow`、`localRows`；wide helper 中也可见类似公式。 | 公式是否被目标编译器简化、各 Official case 是否走 non-wide/同一 helper 均未知。 |
| single-tile loop | 通用 pass-1/pass-2 均以 `col += kTileElems` 遍历，且计算 `tileCount`；窄中专用函数则是一行一组固定宽度操作。 | 只有命中通用分支且 D 符合 tile 条件，循环才可视为单次；编译器是否展开需看目标产物。 |
| `valueFp32Buf_` 容量 | non-wide 默认申请 8192 个 float；若落入 small-batch helper，同一 buffer 会存多行。 | 简单按 D 缩容可能破坏批量路径；没有逐例最大索引或资源报告，暂不保留为固定成本候选。 |
| V-S handoff | 窄中和通用路径按行执行 scalar 读取；small FP32 batch helper 已按行组批量处理 scalar tail。 | VECTOR-MATH-X V003 记录 handoff 约 0.049 µs/row，纯 V 替代更慢约 0.012 µs；不能假设删除 handoff 有收益。 |
| Small copy | V011 已有 small FP32 contiguous batch 和 aligned low-precision batch 路径；SCHED V001 明确未改多行 DMA。 | FULL-R015 有 Runtime Error；MULTIROW-DMA V001/V002 分别因流水损失退化、指令形式信号在噪声内。它们不是 TINY 路径的逐例成本拆分。 |
| DMA / 数值工作 | 各路径执行其对应输入、参数与输出搬运、归约及 affine 运算。 | 这是固定控制成本研究的对照项；各 case 的真实元素量与数据类型仍未知，不能跨 case 估算。 |

这些项目可能处在不同计时层级：Host 前置验证在 launch 前；设备 dispatch、初始化和设备同步是否进入 Official `timeUs` 尚未确认。当前四项只讨论 V011 设备侧机制，不声称能节省 Host 时间或 kernel launch latency。

## ACTIVE_CORE_COUNT 与去重

V011 Host 取正的 `availableCoreNum`（否则用 1），将其限制到 `rowCount` 和 `UINT32_MAX`，把结果同时用于 launch `blockCount` 与 Kernel ownership 参数。降低 active count 会改变 grid、每个 block 的行范围及 `localRows`，继而可能改变参数驻留和被选专用路径条件；它不是仅改一个无关 dispatch 标签。

已有材料表明：

- R016/FULL-R016 按 D 档选择 rows-per-task，并以 `min(availableCoreNum, taskCount)` 设定 block 数；未对齐 rowBytes 还会强制 `requestedBlocks=1`。Official 17.14，FULL 阶段没有本地配对结果。
- SCHED-ROWGROUP-X V001 直接以 R016 为父版，只加入 32B row-group ownership 并取消未对齐单核规则。其本地 `33×100 FP32` 为 4/4 倾向 V001、median -51.38%，`17×256 FP32` 为中性 control；Official 22.27。此覆盖的是调度/row ownership 轴，不是 TINY 的函数 dispatch、event 分配或 buffer 配置。
- SCHED-ROWGROUP-X V001 与 V011 的 Official JSON 都含相同 case 1/3/5 testcase ID，但无 shape、dtype、launch 或 dispatch 字段。SCHED 的本地 proxy 与 Official case 不可互相映射，也不能据此断言 V011 路径。
- SCHED-CHAMPION-X V002 加入 active-core 保留条件，进一步说明同一调度轴已有直接探索。
- CASE47 H2 记录将 active-core / rows-per-task 列为该路线调度假设。
- SELECTIVE-FASTPATH H3 在完整 STORE V003 donor 与 V011 fallback 间按精确 FP32 proxy allowlist 选择；它不以改变 active block 数为目标。官方输入是否命中任一分支未知。

判定：`TINY-H1-ACTIVE-CORE-COUNT = DUPLICATE`。保留本节作去重依据，不产生新的 core-count 变体，也不改变 Main 或 Planning 对路线的决定权。

## 当前四个独立机制

共同资格前提：Official manifest 和 V011 Host/Kernel replay 必须证明至少一个 case 1/3/5 命中每条假设所需路径；case 不能按耗时分桶挑选。满足路径后，先做静态源码/编译产物探针；只有目标机制仍存在、且构建与 correctness 获得授权并通过，才进入项目既有 same-binary 及交错 Parent/Candidate 流程。时序验证须有对应路径、dtype、精确 shape 的 Parent 基线资格。所有本地样本只作本地证据，不改写 Official 结果。

### TINY-C1 — 等行数 ownership 算术直达

- **机制及与其余假设的边界：** 保持 block 数、函数 dispatch、缓冲、DMA、同步不变；仅在 `rowCount == blockCount` 时，把每 block 的商余数公式改为 `beginRow=blockIdx`、`localRows=1`。不调整 core 数。
- **可证伪预期：** 目标编译产物若已经把商/余数化成等价直接映射，或替代式没有减少设备端整数指令，则“ownership 标量算术可省”不成立。
- **最小本地验证（仅 Official path map 满足后）：** 先用相同工具链生成 Parent 的该 dtype Kernel 目标产物，确认除法/取余/边界选择指令；以静态映射覆盖全部 blockIdx 并比较 Parent 与快速式行归属。授权后仅构建此单项变更，做目标 shape correctness；加一个 `rowCount != blockCount` 控制验证 fallback。若产物指令未减少，停止，不做计时。
- **重复边界：** 不改变 `blockCount` 或 rows-per-task，机制不同于 R016/SCHED；但只要 full path 或专用 helper 另有 ownership 公式，需先证明该输入命中此处 `Process` 算式。SELECTIVE H3 命中 donor 时不执行 V011，本项仅适用于 fallback；不推断 Official case 与 allowlist 的关系。
- **状态：** `MAIN_SELECTED_FOR_V001`；Official 输入命中情况仍未知。

### TINY-C2 — V011 Process 设备侧分派短路

- **机制及边界：** 仅对经 V011 fallback 路径重放确认的一个 helper，跳过位于 helper 前且目标二进制仍保留的运行时宽度/对齐分支；helper 内的计算、ownership、buffer、同步不变。Host dtype 分派与设备端 `if constexpr` 都是已有选择，不作为本项。
- **可证伪预期：** 编译产物若未保留这些运行时分支，或 Official 计时不覆盖设备侧分派，则没有可主张的时延收益。
- **最小 proxy：** `1×100 FP32`，仅标作合法 proxy；根据 V011 条件可落到 non-wide 通用路径，`tileCount=1`。仅在 SELECTIVE 选择 V011 fallback 时有效，且需编译产物确认分支保留。
- **重复边界：** SELECTIVE H3 在 STORE V003 donor 与 V011 fallback 间分派；本项只改 fallback 内部设备分支链。donor 命中时本项不适用，不据此推断 Official case 覆盖。
- **状态：** `NEEDS_MORE_EVIDENCE`；路径映射、目标指令与 Official 计时边界未明。

### PipeBarrier 候选筛除

- V011 窄中路径中的 vector 依赖 barrier 尚未找到可删的具体点。R31A V028 记录了另一函数内两个 barrier 的非承重结果及轻微本地改善；它不证明 V011 对应 barrier 可删。
- 当前不列入四个机制，除非某个确切 fallback 函数能指出一个生成物仍保留且无生产者/消费者依赖的单独 barrier。

### TINY-C3 — 已证明 `tileCount == 1` 的通用路径移除循环控制

- **机制及与其余假设的边界：** 仅当精确 Official 输入命中通用两遍路径，且数学条件保证 tileCount 为 1 时，把 pass-1 与 pass-2 各自的 loop control 改成单次 body；保留其中所有 Load、向量操作、ReduceSum、barrier、V/S handoff、Store 及计算顺序。不改变 InitBuffer、ownership 或 dispatch。
- **可证伪预期：** 若路径不是 generic、tileCount 不为 1，或目标编译器已完全展开两段循环，假设不成立。若静态指令差异仅转成等量分支/代码开销，也不继续。
- **最小 proxy：** `1×100 FP32`，仅标作合法 proxy；V011 条件可落到 non-wide 通用路径，D 小于 `kTileElems=4096`。需以目标编译产物确认 pass-1/pass-2 的 loop control 仍存在。
- **重复边界：** SELECTIVE H3 donor 分支不执行 V011 通用 helper；只有回退分支可评估本项。proxy 是否命中 allowlist 未知，使用前须先读取选择条件。
- **状态：** `NEEDS_MORE_EVIDENCE`。

### TINY-C4 — 窄中多行未使用 paramReady ID

- **机制及边界：** 只在命中 `ProcessNarrowMidOverlap` 且 `localRows>1` 时评估是否省去 `paramReady` 的 ID 申请/释放；不改 `inputReady`、`inputRelease`、任何 Set/Wait、标量尾部、PipeBarrier 或输出。源码显示 resident 参数在循环前加载并完成 MTE2→V 同步，循环内不再 Set/Wait `paramReady`。
- **单行排除：** `localRows==1` 时三个 ID 均参与 Set/Wait 生命周期，不把原草案的单行方向列为候选。
- **可证伪预期：** 目标 CANN API 若不允许该分支生命周期、分配/释放在目标产物没有可省资源或指令、或路径未被 Official 输入命中，则不成立。
- **最小 proxy：** `M=2×Ncore, D=258, FP32`，其中 `Ncore=max(availableCoreNum,1)`；仅标作合法 proxy。按 V011 Host 公式 `blockCount=Ncore`、`localRows=2`；D 不满足 small-batch 的 8 元素宽度条件，进入窄中函数。它不是 Official case。
- **重复边界：** SELECTIVE H3 的 donor/fallback 选择不改窄中 event ID；本项只适用于 V011 fallback。donor 命中时不适用，不能据此判断 case 是否相交。
- **状态：** `NEEDS_MORE_EVIDENCE`；本机没有 `ASC_DEVKIT_DIR`，在线搜索仅找到 9.2 beta2 文档且内容抽取未给出 API 约束，CANN 8.5 生命周期仍未知。

H3、H5、H6 的状态仅反映研究成熟度，不属于当前 Candidate。H2 已由 Main 选为 V001；`ACTIVE_CORE_COUNT` 维持 `DUPLICATE`；value buffer、PipeBarrier、V-S handoff 与 small-copy 不计入当前候选集。

## SELECTIVE-FASTPATH 去重规则

C2C update：SELECTIVE-FASTPATH H3 使用精确 FP32 proxy allowlist 选择完整 STORE V003 donor，未命中则回退 V011。本记录只依此说明机制关系，不推断 Official case 1/3/5 是否命中 allowlist。

1. TINY-C1 至 C4 都研究 V011 内部机制，仅在 SELECTIVE 分支回退 V011 时适用。donor 分支使用 STORE V003，不把 V011 的源码、分支、event 或 loop 证据移用到 donor。
2. 任何未来 proxy 都要先应用 SELECTIVE 的精确选择条件，并将实际分支标作 `V011_FALLBACK` 或 `STORE_V003_DONOR`。命中 donor 的样本不能用于支持 V011 内部假设。
3. SELECTIVE donor 的 allowlist 与 Official testcase 的对应关系在本 worktree 中未知；“存在 FP32 proxy allowlist”不证明与 case 1/3/5 重合，也不证明互斥。
4. 机制审阅按 donor 选择、V011 ownership、设备分派、PipeBarrier、loop control、event ID 生命周期分别记录；只凭都存在条件判断，不认定重复或不重复。

## 合法 proxy

以下仅是满足 V011 wrapper 形状约束的研究 proxy，不代表 Official case 输入、耗时档或 dtype。它们只用于解释机制触发条件；本轮未运行。

| 标记 | 形状与 dtype | V011 fallback 条件 | 对应机制 |
|---|---|---|---|
| `PROXY-TINY-C1` | `M=1, D=256, FP32` | Host `blockCount=1`，故 `rowCount==blockCount`；单行落入窄中函数 | 等行数 ownership 算术 |
| `PROXY-TINY-C2-C3` | `M=1, D=100, FP32` | 不满足 small FP32 的 8 元素宽度条件，且 `D<=128`，落入通用路径；`tileCount=1` | 设备分派短路、单 tile loop |
| `PROXY-TINY-C4` | `M=2×Ncore, D=258, FP32`，`Ncore=max(availableCoreNum,1)` | Host `blockCount=Ncore`，每 block 两行；D 不满足 small-batch 的 8 元素宽度条件，落入窄中函数 | 未使用 `paramReady` ID |

每个 proxy 都受 SELECTIVE H3 选择条件约束。若对应尺寸被 allowlist 选为 STORE V003 donor，则它不能用来验证 TINY 假设；只有实际回退 V011 才可用于对应机制的后续评估。具体 allowlist 值未提供，当前不声称任一 proxy 一定回退。

## 允许继续的最小证据顺序

1. 只读取得 Official workload manifest 和 Official 计时边界说明；以 testcase ID 关联，不从延迟推测 shape/dtype。
2. 用 manifest 输入重放未改的 V011 Host/Kernel 路径，产出 case 1/3/5 的 specialization、dispatch、blockCount/localRows 与函数名映射；同表列出是否满足 SELECTIVE donor 条件。
3. 每个目标机制单独核对原始 V011 目标编译产物。若指令/资源层没有该开销，否证对应假设并停止该项。
4. 其他研究机制仍需 Main / Planning 单独选择；不扩展 V001 的 H2 范围。
5. V001 按其声明在 Main 预检后做 exact-source Build 与 PROXY Correctness；same-binary 和 Parent/Candidate 测时仍需 Main timing lease。proxy 结果不代表 Official case 覆盖。

## 本轮未确认事项

- 三个 testcase 的完整 shape、dtype、epsilon、布局与 `availableCoreNum`。
- 每个 case 的 dtype specialization、实际 dispatch、`blockCount`、`localRows` 及路径函数。
- Official `timeUs` 是否包括 Host wrapper、launch、Kernel entry，或只包括设备执行。
- V011 对应目标二进制/汇编中 ownership 除法、分支、循环、barrier 和 event 管理的保留情况。
- SELECTIVE donor 是否会覆盖 case 1/3/5 中任一 testcase。

## C2C 补记：V001 与相邻机制

- Main 已选 `TINY-H2-ROW-OWNERSHIP-FASTFORM` 为 V001，Direct Parent 为 R31B V011，父源码 SHA256 为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。Candidate 范围只有等式 ownership 快速式。
- 代理组为 FP32 `[M,256]`：读取实时 Vector Core 数 `A`，取 `M=max(2,floor(A/2))`，显式控制上限 `B=M-1`。A/M/B 必须在设备运行时记录；不预填设备核数，也不映射到 Official case。
- 等式组传 A，V011 Host 计算得 `blockCount=M`，H2 条件为真；控制组传 B，得 `blockCount=B<M`，候选走与 V011 完全相同的 ownership 公式。两组复用同一组 x/residual/gamma/bias allocations。改变 core 上限仅用于 Correctness 分支覆盖，不比较两组时延。
- CASE47 H1 的 C2C 说明将其机制描述为 non-aligned narrow-mid scalar-handoff grouping proxy。TINY H2 保持 block 数与数学 ownership 结果，仅在相等条件下改算式；不改标量 handoff 次序或 grouping。机制轴不同，可能出现路径邻接，尚无证据确认 proxy 或 Official case 重叠。
- SELECTIVE-FASTPATH H3 使用精确 FP32 proxy-shape allowlist 决定 donor / V011 fallback。TINY V001 以 standalone R31B V011 为父源码做局部验证；未核验 `[M,256]` allowlist，也不据此推断 Official case1/3/5 的分支。
- 当前阶段：源码和声明已按 V001 准备；服务器准入前不 Build/Correctness；不做测时、Online 或共享记录编辑。
