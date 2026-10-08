# W4-R15 SAFE-MULTIROW-DMA-X：跨行传输结构与安全范围

日期：2026-10-08。事件：W4-R15-SAFE-CROSSROW-STRUCTURE-20261008。

## 结论与状态

在本轮允许的跨行 DMA 范围内，没有找到不同于 flat / strided 形式且可直接实施的结构，
结论为 ROUTE_REVIEW_REQUIRED。本轮不创建性能版本，不编辑 Candidate，不运行设备实验。

本文件原位接续 4213e6f1、d13e51e9 的研究；文件名中的 V001 只标识旧研究入口。
旧记录中的 padding 和 8192 B 推论需要收窄，具体依据见下文。
两次历史提交、失败证据、Parent 源码均保留，不把研究 V001 登记为真实性能版。

~~~text
AGENT_ID = 01a119da-1e54-7720-a4dc-5db09873fff7
ROUTE = W4-R15 SAFE-MULTIROW-DMA-X
SLOT = SLOT-1
BRANCH = w4/r15-safe-multirow-dma-x
WORKTREE = /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R15-safe-multirow-dma-x
HEAD_AT_START = d13e51e90ab81451ab86e46034db3ddfde4b05d2
DIRTY_AT_START = NONE
RULES_REF = 9f91895506023d917637f707bb3f61cd9d9f8765
DIRECT_PARENT = R31B V011（研究参照）
LAST_KNOWN_REVISION = V001 RESEARCH_ONLY
REAL_EXECUTED_REVISION = NONE
NEW_PERFORMANCE_REVISIONS = 0
VALID_NUMERIC_LOCAL_RESULTS = 0
CONSECUTIVE_VALID_NO_IMPROVEMENT = 0
STAGNATION_3 = NO
CURRENT_LOCAL_BEST = NONE
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
OFFICIAL = NONE
ONLINE = PAUSED
PUSH = NO
~~~

已完整读取本工作树 AGENTS、Route Skill，以及 RULES_REF 中的 AGENTS、Route Skill、
W4 控制文件、实验总则、执行约定、服务器实验规范、本地性能测试规范、Git 工作流程、
资源脚本，并发布带上述 agent_id 的 RULE_REFRESH_RECEIPT。
AscendC 资料 Skill 在本工作树中缺失，使用已安装 ops-direct-invoke 插件的
ascendc-api-best-practices、ascendc-docs-search，两份 SKILL.md 均完整读取。
参数判断以目标工具链实际头文件为依据。

## 1. Parent 的实际存储与消费者

源码对象：d13e51e9:归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc。
下列行号属于该对象。

| 位置 | 已确认事实 | 约束 |
|---|---|---|
| 46–54 | x、residual 为两个独立 GM 输入 | 每个输入分别证明范围；不合并跨输入地址 |
| 75–76、116–117 | wide FP32 各有一个 tile；非 wide 的 x/res 各有 4096 个 T 元素 | 不能用相邻 TBuf 的物理地址扩展视图 |
| 81–111 | wide FP16/BF16 的 x/res 各有 2*tileWidth 个 T 元素 | 两槽已承担当前/下一单元 |
| 171–239 | 每核拥有连续行区间；对齐 D<=2048 且 localRows>1 进入 CONTIG | 不改 ownership、dispatch 或核数 |
| 284–348、499–619 | generic/NarrowMid 逐行消费；输入复用有 release 及输出同步 | 新行不能只载入而不调整对应的消费者 |
| 1281–1330 | tileWidth 与完整 y 行数由原 UB 计算决定 | 不改 tile、y 驻留或预算 |
| 1576–1803 | CONTIG 一次载入 batchRows*D，batchRows=min(8,4096/D,剩余行数) | 对齐小 D 已有整行 flat 批搬 |
| 2082–2136 | wide FP32 逐行逐 tile；x/res 随后用于平方及归约工作区 | 原输入槽没有独立的下一行保存期 |
| 3083–3221 | wide LP 的 u 按 row-major 展开，slot=u%2；u+1 在当前计算前发往另一槽 | 同列下一行不等于下一消费者 |
| 3255–3297 | pass-2 复用 x/res 双槽保存 gamma/bias | pass-1 槽的全部消费者须先结束 |
| 3367–3383 | Load/Store 为 Ext DataCopyPad，blockCount=1，精确有效字节数，零 stride | 现有接口没有按块变长或第二源地址参数 |

## 2. GM 有效范围、UB 容量与 padding

记输入为连续 M 行、每行 D 个 T 元素，e=sizeof(T)。当前核行区间为 [ownedBegin, ownedEnd)。
待搬 B 行，从 r 开始，各行取列 [c,c+v)。必须满足：

~~~text
0 <= ownedBegin <= r
1 <= B
r + B <= ownedEnd <= M
0 <= c < D
0 < v <= D - c

L = v * e
srcStride = (D - v) * e
GM block i = [base + ((r+i)*D+c)*e,
              base + ((r+i)*D+c+v)*e),  0 <= i < B
~~~

所有块都位于对应行的有效范围及整个输入 [base,base+M*D*e) 内。
末核、末 batch 用实际剩余行数限制 B，不能为了两行搬运读到下一核或张量末尾之外。
这里 x/residual 各自套用公式；不依赖它们相邻。

对默认左右 padding=0，定义：

~~~text
P = ceil(L / 32) * 32
H = P + dstStride * 32
UB occupied block i = [ubBase+i*H, ubBase+i*H+P)
UB valid block i = [ubBase+i*H, ubBase+i*H+L)
required_span = B*P + (B-1)*dstStride*32
~~~

ubBase 必须 32 B 对齐；dstStride 非负；required_span 不超过当前 LocalTensor 的可用字节数。
若显式设置左右 padding，P 改为 AlignUp(L+(leftPadding+rightPadding)*e,32)，
有效数据起点还要加 leftPadding*e。消费者只处理 v 个有效元素，不使用 dummy 数值。

因此，padding 不必然覆盖下一行。使用 H 而非 L 作为 UB 行距，可以让各块互不相交。
旧文把非对齐情况全部判成重叠，缺少这个条件。本轮补充安全地址形式，
同时保留用户对 strided multi-row 的排除要求。

### 2.1 非 wide：两行整行

xBuf/residualBuf 容量各为 4096*e。取 B=2、c=0、v=D、H=P：

~~~text
2*AlignUp(D*e,32) <= 4096*e
~~~

对 FP16/BF16/FP32，最大 D 为 2048。D=2049 时两种元素字节数均超出容量。

FP32 D=129 的纯地址例子：L=516 B，P=544 B，UB 总跨度=1088 B，
第二行从第 136 个 float 开始；两个有效区间和 padding 区间互不相交。
这是地址模型输入，不宣称它属于任何隐藏测试。

该形式仍是 B=2、srcStride=0、dstStride=0 的 padded 多块传输。
现有 NarrowMid 消费者每行从 xBuf/residualBuf 起点取数并发自己的 Load；
要使用第二行，需同步修改读取偏移、跳过重复 Load，并把 release 对应到最后一次消费。
这些变化没有产生另一种传输原语。对齐 D<=2048 已由 CONTIG 覆盖；
非对齐的 padded 多块形式仍在本轮已排除范围。

### 2.2 wide LP：同列两行

令 W=tileWidth，e=2，保持原槽距 H=W*e：

~~~text
P = AlignUp(v*e,32)
dstStride = (W*e-P)/32
required_span = W*e + P
A/base view capacity = 2*W*e
B/offset view capacity = W*e
~~~

只要 0<v<=W，A 视图可容纳两块；B 视图对任何正长度 v 都不足。
满 tile 时 W=4096，L=P=8192 B，双块需 16384 B，恰好占满 A/B。
尾 tile 必须按 P 计算 dstStride；直接使用零 dstStride 会把第二块放在 P，
只有 P=W*e 时才与原 B 槽相符。

目标头文件明确：TBuf::Get() 返回 bufLen 对应视图；
LocalTensor::operator[](offset) 将 dataLen 减去 offset*sizeof(T)。
从 B 开始不能借用 residualBuf 或其它相邻分配。

### 2.3 生命周期反例

以 W=4096、D=16384、两行为例，Parent 先后消费：

~~~text
u=0 (row0,tile0) A
u=1 (row0,tile1) B
u=2 (row0,tile2) A
u=3 (row0,tile3) B
u=4 (row1,tile0) A
~~~

若先把 (row0,tile0)、(row1,tile0) 放进 A/B，Parent 在 u=0 计算前向 B 发出 u=1。
B 内的下一行数据会被覆盖；即使不覆盖，u=1 的消费者也期待另一块数据。
rd/rel 事件仅对应现有单元，不能自动提供下一行的额外保存期。

旧 MULTIROW-DMA V001 已用 tile-outer/row-inner 循环及每组结束同步处理此矛盾，
并停用命中分支原来的 2-deep 预取。保持原遍历而额外保留下一行，需要另一块
独立存储或不同的存储生命周期；旧 C2 已讨论 3-slot 与 UB 计入变化。
本轮不把这一旧 strided 方案改名后实施，也不借用完整 y 区或 pass-2 参数区。

### 2.4 一行尾部与下一行首部

设上一行尾长 v、下一行首块长 W。GM 上两段相邻：

~~~text
tail_start + v*e = next_row_start
flat_payload = (v+W)*e
~~~

单块覆盖这两段属于 flat；在 UB 中下一行从 v*e 开始。
保持原 B 槽 W*e 偏移时，只有 v=W 可直接对应。
若使用两个等长 W*e 块从 tail_start 和 next_row_start 发出，
需要 srcStride=(v-W)*e；v<W 时为负，dav_c220 当前 Ext 字段为无符号。
更换起点、额外读前缀或重新整理消费者仍落在 flat/strided 或布局变化范围，
没有得到独立的跨行原语。最后一行还必须排除没有下一行的情况。

## 3. 其它接口能否给出独立形式

本轮通过 cann-server3 只读读取 CANN 8.5.0.alpha002 实际头文件。
共同前缀：/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/tikcpp/tikcfw/。

| 文件与行号 | 实际实现 | 对本任务的结论 |
|---|---|---|
| interface/kernel_struct_data_copy.h:356、439 | Ext 使用 uint16_t blockCount 与 uint32_t blockLen/srcStride/dstStride；默认无显式 padding | 单源、等长块、固定间隔；没有每块地址/长度列表 |
| impl/utils/kernel_check_data_copy_overflow.h:458 | 容量公式为 B*AlignUp(L+paddingSize,32)+(B-1)*dstStride*32 | 支持第 2 节容量推导；该验证在 CPU 调试路径使用 |
| impl/kernel_tbuf_impl.h:66、impl/kernel_tensor_impl.h:795 | Get() 使用 bufLen；offset 视图缩小 dataLen | 两块 TBuf 不可仅因物理相邻就跨视图写入 |
| impl/dav_c220/kernel_operator_data_copy_impl.h:24、457 | Ext 调试长度范围 0..2097151，块数 0..4095；执行按元素宽度调用 copy_gm_to_ubuf_align_b16/b32 等 | 字段与单位不能沿用旧非 Pad 的 32 B 长度单位 |
| impl/kernel_operator_data_copy_intf_impl.h:532 | SliceInfo 生成 offset 列表，逐项循环调用 DataCopyGM2UBImpl 或 DataCopySliceGm2UBImpl | 一次高层调用可以包含多条已有搬运；不构成原生不规则多块指令 |
| impl/dav_c220/kernel_operator_data_copy_impl.h:408 | Slice 的非对齐间隔分支转成 Ext DataCopyPad | 仍由固定长度/间隔描述符表达 |
| impl/kernel_operator_data_copy_intf_impl.h:148；impl/dav_c220/kernel_operator_data_copy_impl.h:728、762、842、883 | GM→UB Nd2Nz 按 C0 片段循环，片段内构造 Ext 多块调用 | 典型 NZ 布局需要相应消费者；底层仍是小块 strided 搬运 |
| impl/kernel_operator_data_copy_intf_impl.h:1500 | MultiCopyParams/NDDMA 仅编译进 3101/5102 分支 | 当前 2201 不能使用该入口 |

以 half 的 Nd2Nz 实现为例，C0=32/2=16 个元素，D=4096、nValue=2、
ndNum=1 时源代码展开为 256 个列片段，每片段发一次两行小块 Ext 调用。
这与“新的单条跨行传输”不符；未把命令数推导当作硬件耗时测量。

这些判断只覆盖已读版本的公开接口及对应实现，不声称其它工具链或平台也没有其它形式。
R06 的 same-tile cross-input 两块结构和 R07 的单 tile 输出方向不属于本任务。

## 4. 历史去重与原始结果

继续复用 bd825c5fd59c74c283478026e8b32702ba331eb5:
研究/W4-R06/TRANSACTION-STRUCTURE-20261008.md 的 DMA 构造式搜索。
本轮另补查替代 API 符号，范围如下；所有读取均通过自己的工作树和 Git 对象完成。

| 来源 | 固定 ref | 本轮替代 API 搜索源码数 |
|---|---|---:|
| W3 R2 V001–V040 | 6321ad44 | 80 |
| W3 R4 V001–V031 | ce6c6dc5 | 31 |
| W3 R5 V001–V028 | 1efa0863 | 56 |
| R031 V001 与重建 D001–D004 | d13e51e9 | 5 |
| R31A / R31B | 09a9c5ce / aef6e728 | 30 / 22 |
| MIX-A | d13e51e9 | 12 |
| STORE / EPI-X-FRESH | 8261c094 / e9056590 | 19 / 2 |
| MULTIROW-DMA V001/V002 | 0932fbd5 | 2 |
| MODE-X-R015C | d13e51e9 | 4 |
| W4 R01 / R08 / R09 | 93f15d9b / fa9b19bb / 96044629 | 2 / 1 / 2 |

共 268 份 .asc，包含 Parent/Candidate，不等于性能版数。排除 support、adapter、
op_host、workspace；R4 只读各版 submission。检索符号为 SliceInfo、Nd2NzParams、
MultiCopyParams、DataCopyEnhancedParams、DataCopyWithNDDMAImpl、
copy_gm_to_ubuf_align_v2，均无命中。首次 R031 路径为空，随后定位到
归档/phase3-before-reset-20260920 下的原源码与四份重建源码，实际读取五份后再计数。

名称未命中不能证明机制独立；第 3 节的实现展开说明这些高层写法仍复用旧搬运。
另直接读取 R4 V031 summary、R2 V040 Parent 记录和 R5 V028 提交说明，
核对到 V040/V031/V028；不采用旧共享表中的 V016/V013/V016 作为末版。
R14 的输入参数搬运和 R15 旧研究沿用 R06 的已提交证据；R15 本身无 Kernel Candidate。

| 形式 | 已有来源 | 本轮判断 |
|---|---|---|
| 对齐整行 flat | Parent:1576–1803；原 R15 研究 | Parent 已覆盖；遵守用户禁止重做的要求 |
| 非对齐整行 padded 多块 | MODE-X-R015C 历史及第 2 节表达式 | 有可证明的安全地址布局；仍属已排除的跨行多块形式 |
| 同 tile 列窗 stride | 0932fbd5:本地实验/MULTIROW-DMA-CHAMPION-X/V001/diff.patch | 与原 kStrideChunkRows=2 机制相同 |
| 对齐 Pad→非 Pad | 同目录 V002/diff.patch | 已实施，未取得可分离的稳定收益 |
| C2：stride + 额外槽；C3：首对部分合并 | 0932fbd5:研究/MULTIROW-DMA-CHAMPION-X/C2-OPTIONS-FACT-PACKAGE.md、LANE-FACT-PACK-FOR-PLANNING.md | 已在案，仍属 strided；不以新名称实施 |
| SliceInfo / Nd2Nz | 第 3 节实际工具链实现 | 高层写法未见于所查源码；底层形式重复 |
| NDDMA | 目标接口的编译条件 | 目标架构不提供 |

原始负结果仍使用原 Route/Revision，不计入 R15：

- MODE-X-R015C 的 LOAD_QUALITY.md:35–45 记录 r1 在 FP32 D=4096/8192
  首次不一致位于 index 2048；r2 三个代理均失败，r3 分段 Ext 形式三项 exact 通过。
  该 8192 B 现象来自特定实现和输入，不能推导为所有 DataCopyPad 的硬件上限。
  Parent 对齐 CONTIG 源码本身包含 FP32 D=2048、两行共 16384 B 的单块请求。
  源码请求与头文件范围也不替代任何新的设备精度验证。
- MULTIROW-DMA V001 的 local-result.json 记录 48x16384 FP16 的四对
  Parent/Candidate medians 为 13.30/13.72、12.92/14.16、12.94/14.68、
  13.66/14.64 us，四对均回归，所记录 delta 中位数 +8.4%。
  该版本同时改变输入分组与消费流水；结果只支持对此实现的否定，
  不构成对所有跨行算法的通用性能结论。
- V002 的 local-result.json 记录 16 对 delta 中位数 -1.84%，
  同代码对照为 -1.48%，范围 -7.1%..+5.4%；旧记录未给出可分离的稳定提升。
- 两份历史 local-result 中 LP 与 Parent 逐位一致的描述，不能用于宣布
  全矩阵均通过 reference；V001 还记录未改 FP32 wide 路径的 reference 异常。
  本轮没有重跑这些实验，也没有把 Parent 一致性替代正式 Correctness。

## 5. 本轮验证

唯一执行的模型为同目录 cross_row_address_model.py：

~~~text
python3 研究/W4-R15/cross_row_address_model.py

status = PASS
scope = HOST_ADDRESS_AND_LIFETIME_ONLY
full_row_cases = 3840
unaligned_subset = 3480
nonwide_capacity_exclusions = 2
wide_view_cases = 30
lifetime_counterexamples = 2
tail_head_cases = 6
total_cases = 3880
reference_correctness = NOT_RUN
device_operation = NONE
~~~

模型涵盖 D=129..2048、e=2/4 的两行有效范围与 padding；
D=2049 的容量超出；五个既有 tile 尺寸、六种 valid 的 A/B 视图差异；
tileCount=3/4 的下一消费者覆盖；六种尾长的跨行首尾范围。
这些为合成地址案例，非性能参数扫描、非隐藏 shape 推测，也不替代 NPU/reference 验证。

server3 只读头文件访问成功；未创建、修改或删除远端文件，未运行 NPU。
设备 2 仅为本轮指定的优先设备，未使用；无新的 free HBM、负载或 latency 数据。
没有资源停止事件，没有等待独占设备，也没有建立定时或后台任务。

## 6. 研究事件与交接

~~~text
ROUTE_RESEARCH_EVENT = W4-R15-SAFE-CROSSROW-STRUCTURE-20261008
AGENT_ID = 01a119da-1e54-7720-a4dc-5db09873fff7
STATUS = ROUTE_REVIEW_REQUIRED
MECHANISM = padded 跨行、尾/首拼接、SliceInfo、Nd2Nz、NDDMA
MATCH_FOUND = MECHANISM_MATCH_OR_TARGET_API_UNAVAILABLE
WHY_NEW_OR_DUPLICATE = 可安全表达的形式仍为 flat/strided；其它写法的底层重复或目标不支持
VERSION_RECORD_EVENT = NONE
COMPILE = NOT_RUN
CORRECTNESS = NOT_RUN
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
CURRENT_LOCAL_BEST = NONE
OFFICIAL = NONE
ONLINE = PAUSED
PUSH = NO
RESOURCE_BLOCKER = NONE
RUNNING_DEVICE_OPERATION = NONE
REMAINING_INDEPENDENT_AXIS = NONE_IDENTIFIED_WITHIN_CURRENT_SCOPE
NEXT_ACTION = Main/Planning 根据地址证明及旧 C2/C3 资料复核 R15 的后续范围
~~~

当前可执行研究已完成。建议 Main/Planning 先确认是否继续维持对 flat/strided 的排除；
若维持，现有资料没有支持下一 Candidate 的独立形式。若另行提出新形式，
下一研究输入必须包含适用于该 dav_c220 工具链的入口与实际展开，
逐项给出 GM 范围、LocalTensor 容量、padding 和全部消费者的保存期，
并与本表比较；在证据成立前不扫参数、不创建性能版。

本 Agent 不选择旧 C2/C3、不转到 R06/R07、不改变 Route 生命周期。
交 Main 释放 SLOT-1；释放 Agent 槽不表示 Route 关闭。

本轮写入仅为本文件和 cross_row_address_model.py。
Parent、Kernel、构建脚本、共享 TSV、规则、Dashboard、其他工作树及远端数据均未修改。
