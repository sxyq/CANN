# W4-R06 x/residual 搬运结构核对

日期：2026-10-08。事件：`W4-R06-TRANSACTION-STRUCTURE-20261008`。

续研索引：本文件末尾的“共享视图与 GM 跨度续研”记录本次 fresh 接管结果，
事件为 `W4-R06-SHARED-VIEW-GM-SPAN-20261008`。原研究正文及来源保留。
本次已证明共享 TBuf 的容量与相对布局可表达；对于两个独立、不相交的原 GM
输入视图，单源双块调用仍缺少合法的共同源视图依据，因此没有编辑 Candidate。

## 结论与状态

本轮完成了 Parent 输入布局、DMA 描述符、可达路径和跨输入双块方案的地址推导。
所选结构与 R15 的跨行搬运不同；在保持 Parent 两个独立 TBuf 视图的前提下，
直接把 x/residual 的两个 Load 合成一次双块 DataCopyPad 会超出目标 LocalTensor
容量。即使 GM 条件满足，也不能据此编辑 Candidate。

未创建性能版本，未改 Kernel，未运行 Compile、Correctness、Local 或 Profile。
`CURRENT_LOCAL_BEST=NONE`，`LOCAL_SCORE=NONE`，`LOCAL_DELTA=NONE`。
这份结论不代表 R06 已耗尽，也不改变 Route 生命周期。

```text
AGENT_ID = 01a119c6-56a0-77f1-b209-400567d25293
ROUTE = W4-R06 MTE2-XRES-ISSUE-X
SLOT = SLOT-1
BRANCH = w4/r06-mte2-xres-issue-x
WORKTREE = /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R06-mte2-xres-issue-x
HEAD_AT_START = b09e00eb351bf376a459fff0e690ea0613221b40
DIRTY_AT_START = NONE
RULES_REF = 9f91895506023d917637f707bb3f61cd9d9f8765
DIRECT_PARENT = R31B V011
LAST_KNOWN_REVISION = V001, RESEARCH_ONLY
REAL_EXECUTED_REVISION = NONE
NEW_PERFORMANCE_REVISIONS = 0
VALID_NUMERIC_LOCAL_RESULTS = 0
CONSECUTIVE_VALID_NO_IMPROVEMENT = 0
STAGNATION_3 = NO
ONLINE = PAUSED
OFFICIAL = NONE
PUSH = NO
```

已完整读取本工作树 AGENTS、Route Skill，以及上述规则提交中的 AGENTS、Route
Skill、W4 控制文件、实验总则、执行约定、服务器实验规范、本地性能测试规范、
Git 工作流程和资源脚本。初始 `RULE_REFRESH_RECEIPT` 已发送。
工作树没有 AscendC 资料 Skill 副本，使用已安装 ops-direct-invoke 插件中的
`ascendc-api-best-practices`、`ascendc-docs-search`，两份 SKILL.md 均完整读取；
相关资料为 `api-datacopy.md` 与 API 索引。最终参数依据采用 server3 的实际头文件。

## Parent 的实际布局与可达路径

源码对象：`b09e00eb:归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc`。
下列行号均属于该对象。没有从其他实际工作树读取文件。

| 位置 | 已确认事实 | 对 R06 的含义 |
|---|---|---|
| 46–54 | x、residual 分别绑定两个调用者提供的 GM 指针 | 没有相邻地址、固定地址差或前后顺序的承诺 |
| 3479–3551 | 元数据只有 shape、dtype；前导维度合并为 rowCount，最后一维为 rowWidth；核数受 availableCoreNum 与 rowCount 限制 | 源码按每个输入内部连续布局寻址；不能把两个输入当作一个连续张量 |
| 60–113、163–169 | rowWidth > 8192 才进入 wide；float 走 ProcessWideFp32，half/bfloat16_t 走 ProcessWideLowPrecision | R06 所读 pass-1 对 FP16/BF16 wide 可达；不把 FP32 历史函数当作本路径 |
| 81–111 | LP 的 xBuf_、residualBuf_ 各有 2 * tileWidth 个 T 元素，分别 InitBuffer | 两个独立 LocalTensor，各自容纳 A/B 两槽 |
| 1281–1329 | tileWidth 初始 4096，必要时以 512 递减至 2048；每批行数由既有完整 y 驻留计算决定 | 不改 tile、批行数、UB 预算或行归属 |
| 3083–3093、3125–3137 | 每核拥有连续行区间；u 按 row/tile 展开；输入地址为 base + ((batchBegin + batchRow) * rowWidth + col) * sizeof(T) | 同一输入内部的 tile 连续，跨行仍须按实际 rowWidth 计算 |
| 3140–3145、3146–3174 | 初始单元与下一单元均先 Load(x) 再 Load(residual)，之后发 ready 事件；两槽复用受 release 事件约束 | 合并形式必须同时满足两个目标槽的范围与既有事件语义 |
| 3176–3204 | FP16 原生加法后保留 y，BF16 转 FP32 后相加；随后按原顺序平方和归约 | 本轮不改数值运算或归约顺序 |
| 3367–3383 | Load/Store 均为 DataCopyPad，参数 (1, count * sizeof(T), 0, 0, 0) | blockCount=1，GM 长度为精确有效字节数，无跨输入或跨行 stride |

`DataCopyPadExtParams<T>` 的默认值已经核对：isPad=false、左右 padding=0、
paddingValue=0。GM 读取只包含有效字节；目标 UB 每块仍按 32 B 对齐占用。
尾块的有效长度与目标占用长度必须分开计算。

## 所选双块结构及容量证明

研究对象是同一 `(row, tile)` 的 x/residual 双块描述符，保持行遍历、tile、
双槽事件和算术不变。设：

```text
e = sizeof(T)
w = tileWidth
v = valid
L = v * e
P = ceil(L / 32) * 32
dg = address(residual[offset]) - address(x[offset])
du = address(residualLocal) - address(xLocal)

blockCount = 2
blockLen = L
srcStride = dg - L              # GM 侧按字节
dstStride = (du - P) / 32       # UB 侧按 32 B 块
```

至少需要：dg >= L，dg-L 可由目标平台的 uint32_t 字段表达；du >= P，
du-P 为 32 的倍数；目标 LocalTensor 实际容量不少于 du+P。
这些是表达式必要条件，单凭字段位宽不能宣称任意跨分配地址均受设备支持。

Parent 未承诺 dg 的值，也未承诺两块 TBuf 的地址差。即使采用最有利的
“xBuf_ 后面恰好紧接 residualBuf_”假设，仍有 du=2*w*e。
CANN 的目标容量公式给出：

```text
required_span = 2 * P + dstStride * 32
              = du + P
              = 2 * w * e + P
              > 2 * w * e
```

右侧最后一项已是整个 xBuf_ 的容量上限；从 B 槽开始的可用范围也不会更大。
因此，以原 xLocal 为目的张量的双块调用无法覆盖 residualLocal。
不能靠相邻物理地址或只增加 GM 地址条件绕过 LocalTensor 容量要求。

4096 元素 FP16 满 tile 的具体数值：L=P=8192 B，xBuf_=16384 B，
假设 du=16384 B，则 dstStride=256，required_span=24576 B。
这是地址模型计算，未作为设备实际地址或运行结果使用。

另一种“把两次 Load 合成长度 2*L 的单块”也缺乏依据。即使两个完整 GM
输入顺序相邻，以已有历史代理 16x16384 FP16 为模型，两个输入基址差为
524288 B，当前 tile 仅 8192 B；x 当前 tile 后面仍是 x 的后续数据。
应到达 residual tile 的 srcStride 是 516096 B，无法用零间隔长块表达。

### 目标工具链证据

本轮通过 `cann-server3` 只读访问 CANN `8.5.0.alpha002`，读取以下实际文件。
共同前缀为 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/tikcpp/tikcfw/`。

| 文件与行号 | 证据 |
|---|---|
| interface/kernel_struct_data_copy.h:356 | 当前通用分支的 DataCopyExtParams 使用 uint16_t blockCount、uint32_t blockLen/srcStride/dstStride；3101/5102 的有符号 stride 分支不能用于本推导 |
| interface/kernel_struct_data_copy.h:439 | DataCopyPadExtParams 默认字段值 |
| impl/dav_c220/kernel_operator_data_copy_impl.h:24 | blockCount 上限 4095，blockLen 的 CPU 调试范围上限 2097151，GM→UB 长度需整除元素字节数 |
| impl/dav_c220/kernel_operator_data_copy_impl.h:457 | Ext 调用按元素宽度进入 copy_gm_to_ubuf_align_b16 等指令；一个 src、一个 dst，附带块数、块长与两个 stride；没有独立的第二源地址参数 |
| impl/kernel_operator_data_copy_intf_impl.h:1344 | GM→UB Ext 接口在 CPU 调试路径调用目标张量容量验证，再向 dav_c220 实现传物理地址 |
| impl/utils/kernel_check_data_copy_overflow.h:458 | 目标跨度 = blockCount * AlignUp(blockLen + paddingSize, 32) + (blockCount-1) * dstStride * 32，并与 dst.GetSize() * sizeof(T) 比较 |

上述容量条件属于 API 的边界依据。本轮未运行 CPU 仿真或 NPU，也没有把
调试分支中的公式当作已发生的设备异常。

## 历史搜索与机制区分

搜索使用本工作树中的 Git 对象。提取下列范围内非 support/adapter 的 `.asc`
源码中 DataCopyParams/DataCopyExtParams 构造式，并核对相关差分、记录和声明。
源码份数包含 Parent 与 Candidate，不代表性能版数量。

| 搜索对象 | 固定来源 | 实际范围与结果 |
|---|---|---|
| W3 R2 | 6321ad44 | ADAPTIVE-CORE-OWNERSHIP 的 V001–V040，共 80 份源码；DMA 构造式仍为单块 |
| W3 R4 | ce6c6dc5 | MULTIROW-PANEL-RMS 的 V001–V031，共 31 份 submission；DMA 构造式仍为单块；另读 V031 summary 与 V007 源码差分 |
| W3 R5 | 1efa0863 | CROSSROW-FULL-PIPELINE 的 V001–V028，共 56 份 Parent/Candidate；DMA 构造式仍为单块；另读 V014 的实际 Parent/Candidate 差分 |
| R031 | b09e00eb | R031-V001-FULL-MULTIMODE-REWRITE 与 MULTIMODE-R031-RECONSTRUCTION D001–D004，共 5 份源码 |
| R31A / R31B | 09a9c5ce / aef6e728 | 本地实验与历史工作区的 30 / 22 份源码；R31B 另有单块临时标量/部分和搬运，未见跨输入双块描述符 |
| MIX | b09e00eb | MIX-A 历史工作区及本地实验，共 10 份源码；单块构造式；读取版本表中的 MTE2 相关变化 |
| STORE / EPILOGUE | 8261c094 / e9056590 | STORE-EPILOGUE-X 19 份源码，EPI-X-FRESH 3 份源码；另读版本表的 STORE/EPILOGUE 相关行 |
| MULTIROW-DMA | 0932fbd5 | V001/V002 源码、V002 diff.patch、API-PROBE-RESULT、DMA-INVENTORY 与 LANE-FACT-PACK |
| MODE-X-R015C | b09e00eb | 历史工作区与本地实验的 5 份源码；可见分段单块与旧多行 DataCopy 形式 |
| W4 R01 / R08 / R09 | 93f15d9b / fa9b19bb / 96044629 | 分别读取 2 / 1 / 2 份路线源码；DMA 构造式仍为单块 |
| W4 R14 / R15 | fcbd1814 / d13e51e9 | PARAM-MTE2-FINDINGS、构建入口；R15 DUPLICATE-AUDIT-V001 全文 |
| W4 R06 | b09e00eb | 既有 本地实验/W4-R06/V001/DUPLICATE-AUDIT.md 全文，仅研究 |

首次查询旧 MIX 分支未找到迁移后的路径，随后改用 b09e00eb 中的已提交归档
与本地实验，实际读取了上述 10 份源码。未将空搜索当作无重复证据。
构造式提取用于缩小搜索范围；未声称已穷尽全部仓库、全部平台或未提交工作。

```text
DUPLICATE_AUDIT
MECHANISM = same-tile cross-input two-block DataCopyPad
SEARCHED_HISTORY = 上表所列 Git 对象、源码与记录
MATCH_FOUND = NO_EXACT_MATCH_IN_READ_SOURCES
WHY_NEW_OR_DUPLICATE = 第二块指向 residual，行数与 tile 不变；不同于跨行 DMA。
  当前 Parent 的目标视图容量不满足该形式，不能直接进入性能编辑。
```

已明确排除的相近变化：

- x/residual 顺序交换：R5 V010/V021 已覆盖，沿用 R06 原有结论，不重复实施。
- 跨两行 tile 合并：MULTIROW-DMA V001 与 R15 已覆盖；它改变逐行输入暂存方式。
- 对齐 DataCopyPad 改为 DataCopy：MULTIROW-DMA V002 已覆盖，实际差分同时涉及 Load/Store。
- 提前发下一 tile：R5 V014 移动 ready wait；属于发射/事件位置，不构成新的搬运描述符。
- gamma/bias 的同张量预载片段合并：R14 的研究范围，不由 R06 实施。
- 把单 tile 的单块改为两个等长零 stride 块：对半块均为 32 B 整数倍时，
  字节覆盖可以相同，API 调用数和传输字节数也相同；已读资料没有给出当前
  dav_c220 对这种划分的独立 transaction 时间证据。本轮不据此排列块长参数，
  也不宣称它与单块的硬件执行完全相同。

## 本轮验证与限制

本地只读 Python 地址模型覆盖 tileWidth={2048,2560,3072,3584,4096}，
valid={1,15,16,17,tileWidth-1,tileWidth} 和 A/B 两槽，共 60 项。
60 项均满足 required_span=du+P，且超出原 x 目标视图容量。
另验证 4096/5120/6144/7168/8192 B 的零 stride 等分双块与单块具有相同
字节覆盖。模型不包含硬件耗时，也不替代与 reference 的 Correctness。

本轮没有 Parent/Candidate raw latency，没有新的设备空闲显存或负载测量。
device 2 仅为获分配的优先设备，未使用。server3 的只读连接及头文件读取成功；
远端未创建、修改或删除文件，未启动 NPU 任务，未操作其他用户进程。

## 剩余独立方向与交接

精确下一研究动作：围绕已读的 LocalTensor 容量公式，核对目标工具链中
TPipe::InitBuffer 与 TBuf::Get 的实际分配/视图规则，证明单个 4*tileWidth
输入存储及两个 2*tileWidth 视图能否保持原 A/B 槽、pass-2 复用、总 UB 用量
和 ChooseWideFullYRows 不变。同时明确跨输入 GM 地址差的合法区间与未满足
条件时的原调用路径。只有这份完整地址与生命周期证明成立，才考虑跨输入双块 OFAT。
该方向尚未实现，也未被称为性能提升。

另一项可证伪方向是单 tile 零 stride 双块的底层行为：以当前
`copy_gm_to_ubuf_align_b16` 为入口，先取得与单块可区分的 transaction 或耗时
证据；保持两个输入独立、单 tile 数据量和运算顺序不变。没有这项证据时不扫参数。

```text
ROUTE_RESEARCH_EVENT = W4-R06-TRANSACTION-STRUCTURE-20261008
STATUS = RESEARCH_COMPLETED_NO_CANDIDATE
VERSION_RECORD_EVENT = NONE
COMPILE = NOT_RUN
CORRECTNESS = NOT_RUN
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
CURRENT_LOCAL_BEST = NONE
RESOURCE_BLOCKER = NONE
RUNNING_DEVICE_OPERATION = NONE
NEXT_ACTION = 将本事件交 Main；后续按上述共享输入视图的地址证明继续研究
```

本轮写入对象只有本文件。旧 V001、Parent、共享 TSV、规则、Dashboard、
其他工作树与远端运行数据均保持原样。所有本轮命令已返回；由 Main 处理槽位释放。

## 共享视图与 GM 跨度续研

日期：2026-10-08。事件：`W4-R06-SHARED-VIEW-GM-SPAN-20261008`。
以下是本次 fresh 接管的增量证据。上文的下一研究动作已在本节处理。

### 本次结论

UB 侧可使用一个等总量 TBuf，并从它取得原 x/residual 的两个子视图。
在目标 8.5.0.alpha002 的分配实现中，此形式保持原相对地址、后续 UB 起点、
A/B 两槽、pass-2 的 gamma/bias 复用及现有手动事件位置。这个结论来自源码与
地址模型，尚未编译成 Candidate，也没有宽 FP16 地址实测。

GM 侧只确认了描述符的数值条件。DataCopyPad 接收一个 GlobalTensor；
Parent 的输入为两个独立指针，各自具有自己的数据范围，没有共同存储区间元数据。
在“保持原被授权视图”的要求下，不能依据两个地址差可由 uint32_t 表达，就把
x 的源视图扩展到 residual。对不相交输入，下面给出了数值条件成立而原源视图
范围不成立的反例。共享 UB 本身不能解决这一项。

局部重叠的输入视图存在数值正例，本节也保留了它；当前历史代理与新转交来源
没有提供该输入关系的工作负载依据。没有为这个特例制造输入性能版。
本结论限定于当前 API、Parent 和原视图要求，不声称硬件必然读取 stride 间隙，
也不声称所有 GM 布局都无法用双块描述符表达。

```text
ROUTE = W4-R06
AGENT_ID = UNKNOWN_IN_THIS_CONTEXT
SLOT = 4
DEVICE_ASSIGNED = 3
DEVICE_USED = NONE
HEAD_AT_START = bd825c5fd59c74c283478026e8b32702ba331eb5
DIRTY_AT_START = NONE
BRANCH = w4/r06-mte2-xres-issue-x
WORKTREE = /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R06-mte2-xres-issue-x
RULE_REF = 9f91895506023d917637f707bb3f61cd9d9f8765
SHARED_STATE_READ = 07662d7b96e9beaaa0f56d97c9cf346b86081eb3
DIRECT_PARENT = R31B-V011
REVISION = NONE
LAST_KNOWN_REVISION = NONE_REAL; SOURCE_LABEL=V001_RESEARCH_ONLY
CURRENT_LOCAL_BEST = NONE
NEW_PERFORMANCE_REVISIONS_THIS_TURN = 0
VALID_NUMERIC_LOCAL_RESULTS = 0
CONSECUTIVE_VALID_NO_IMPROVEMENT = 0
STAGNATION_3 = NO
OFFICIAL_SCORE = NONE
ONLINE_STATE = PAUSED
PUSH = NO
```

已完整读取本树 AGENTS 与 Route Skill，以及指定规则提交中的九个入口；已发送
`RULE_REFRESH_RECEIPT`。AscendC 资料 Skill 沿用已安装 ops-direct-invoke 插件，
完整读取 API 最佳实践与资料检索两份 SKILL，使用 Buffer、DataCopy 和 API 索引。
本树没有这些资料 Skill 的副本，没有从其他实际工作树取得文件。

共享状态读取的是 main 当时的实际提交。R06 已有研究登记，表中的排队阶段早于
本次接管；本次 SLOT=4、DEVICE=3、唯一 writer 按用户新消息执行。
本事件尚待 Main/Record 同步，不等待同步开展本次研究。

### 新来源与适用范围

通过本工作树的 `git show` 读取 `1a31a3b527d81d4db0fce20dda091b85889330b6`
中的 R05 `gamma-view-20261008/RESULT.md`、`address_probe.asc` 和
`results/address-probe.log`，没有进入 R05 工作树。

R05 调用未改的 Parent Init/GetPhyAddr，在 FP32、D=129/2048/2056/4096 的
四次结果中观测到 x=0、residual=16384 B，两块各为 16384 B。它只证明这些
声明输入与分派下的地址。该探针没有观测宽 FP16，也不授权从 x 的 LocalTensor
越过自己的容量；本次没有使用 R05 Candidate 或其测量数字。

新 API 来源为 server3 的只读文档和实际安装头文件：

- 文档根：`/home/data4t2/lelinfeng/asc-devkit/docs/zh/api/SIMD-API/basic_api/`。
- 目标头文件根：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/tikcpp/tikcfw/`。

asc-devkit 文档同时包含更新芯片的内容；本次只引用其 910B 对应参数与下列
视图说明，目标实现依据为实际安装的 8.5.0.alpha002 头文件。950 的有符号
stride、Compact 与多层循环接口均未用于本推导。文档对 VECCALC 的逻辑位置
说明与 Parent 用法有差异；本次以目标头文件按物理 UB 分流的源码事实为限，
没有改变 Parent 的 TPosition，也未声称完成新的设备验证。

| 来源与行号 | 本次采用的事实 |
|---|---|
| `resource_management/TBuf/GetWithOffset.md:44,59,64` | size 单位为元素、offset 单位为 B；offset 需 32 B 对齐；size*sizeof(T)+offset 不超过原 TBuf 长度 |
| `resource_management/TBuf/Get.md:72,92` | Get 长度受原 TBuf 限制；连续 Get 返回相同首地址 |
| `resource_management/TPipe/InitBuffer.md:104,108` | 分配长度向上取 32 B 整数倍；存储期由 TPipe 管理 |
| `impl/kernel_tpipe_impl.h:289,303,317,329,337` | TBuf 从对应物理池当前末尾分配；末尾只前进对齐后的 len |
| `impl/kernel_tbuf_impl.h:82` | GetWithOffset 使用 ptr->address+bufOffset，记录本次视图的 dataLen |
| `impl/kernel_tensor_impl.h:795` | LocalTensor 下标递增地址，并扣除剩余 dataLen |
| `impl/kernel_tensor_impl.h:821,834` | SetAddrWithOffset 已弃用；SetSize 改元数据，不新分配原 TBuf 的存储 |
| `data_structures/GlobalTensor/SetGlobalBuffer.md:40,56` | 无 size 参数时 GetSize 为 0；显式 size 需保证不超过实际数据长度 |
| `interface/kernel_operator_data_copy_intf.h:412` | 目标 Ext 原型仅有一个 GlobalTensor 源；没有第二源地址参数 |
| `memory_vector_compute/data_move/DataCopyPad_GMToUB.md:162` | 910B blockCount 上限 4095、blockLen 上限 2^21-1 B；srcStride 为 uint32_t、单位 B，dstStride 单位 32 B |
| `impl/dav_c220/kernel_operator_data_copy_impl.h:457,478` | 16 位元素直接向 copy_gm_to_ubuf_align_b16 传一个源、一个目的与块/stride 参数 |
| `impl/utils/kernel_check_data_copy_overflow.h:458` | CPU 调试的目标跨度公式继续成立；该函数不核对 GM 源长度 |
| `impl/kernel_utils.h:329,370,458` | GM 辅助逻辑使用跨度与一个登记范围比较；Ext 辅助函数中的 stride 局部量为 uint16_t |

### 共享 UB 的构造、容量和生命周期

令 `e=2`，`w=tileWidth`，`S=w*e`，分配前 UB 末尾为 `a`。
Parent 两次分配的长度均为 `2S`。w 取原模型的五个值时，S 均为 32 B 的
整数倍，因此两次对齐后的长度和等于一次 `4S` 分配的长度。

```text
原分配：x      [a,      a+2S)
        residual[a+2S,  a+4S)
共享分配：       [a,     a+4S)

仅作API构造说明，未写入Candidate：
InitBuffer(shared, 4*w*sizeof(T))
xBase = shared.GetWithOffset<T>(2*w, 0)
rBase = shared.GetWithOffset<T>(2*w, 2*w*sizeof(T))
dmaBaseForSlotQ = shared.Get<T>()[q*w], q∈{0,1}
```

两种分配使所有后续 TBuf 的分配起点一致。此推导保留 `a` 为符号量；没有把
R05 窄行 FP32 的绝对地址套用给宽行。共享存储会少一个 TBuf 管理项，但
目标 InitBuffer 不为 TBuf 分配 ready/release 事件；原手动事件不依赖这个数量。

| 视图/用途 | 起点相对 a | 容量 B | 使用期 |
|---|---:|---:|---|
| x A | 0 | S | pass-1 当前或预取单元；pass-2 gamma A |
| x B | S | S | pass-1 另一单元；pass-2 gamma B |
| residual A | 2S | S | pass-1 当前或预取单元；pass-2 bias A |
| residual B | 3S | S | pass-1 另一单元；pass-2 bias B |
| DMA 共用视图 A | 0 | 4S | 仅访问 x A 与 residual A 的两个块 |
| DMA 共用视图 B | S | 3S | 仅访问 x B 与 residual B 的两个块 |

令 `L=valid*e`、`P=align32(L)`，则 `0<P<=S`。
保持原 x→residual 的空间顺序，`dstStride=(2S-P)/32`，每次双块调用的目标
跨度为 `2S+P`。从 A 槽起点所需容量不超过 4S，从 B 槽起点不超过 3S。
实际写区间为 `[a+qS,a+qS+P)` 和 `[a+(q+2)S,a+(q+2)S+P)`。
两个区间均位于该槽原有范围内，之间的 stride 区间不写入；另一槽仍可供 Vector 使用。

原来的 x 子视图无法直接作为这个 DMA 共用视图：A 剩余容量 2S，B 剩余容量 S，
两者都小于 `2S+P`。对旧 xBuf 调用更大的 Get/GetWithOffset，或者仅修改
SetSize，均没有新建合法的 `4S` 分配。API 本体没有执行某个边界判断，也不能
扩大该 API 文档定义的视图长度。

Parent 的两次输入 Get 位于 3117/3118，pass-2 两次复用 Get 位于 3256/3257；
这些位置可取相同子视图。原 ready/release 的位置为 3143/3145、3161–3174、
3205–3221；pass-1 最后排空后还有 3243 的同步。这个次序允许两个原槽继续
复用，未要求改变生命周期。ChooseWideFullYRows、完整 y 驻留、输出和工作区
的长度计算均不因共用视图而改变。本次只做证明，未实施上述替换。

### GM 数值条件与原视图边界

设每个输入有效字节数为 `N=M*D*e`，当前 tile 字节偏移为 o，两个输入起点
分别为 X、R。原 Load 已授权的范围分别是 `[X,X+N)` 与 `[R,R+N)`；两次
实际读取为 `[X+o,X+o+L)` 与 `[R+o,R+o+L)`。

若使用一个 x 源的双块描述符，必须计算运行时字节差 `d=R-X`，并满足：

```text
0 <= o <= N-L
L <= d
0 <= d-L <= 2^32-1
srcStride = d-L
sourceSpan = 2L+srcStride = d+L
```

数值运算应在已确认顺序后用足够宽的无符号整数完成；不能对两个无关 C++ 数组
直接做指针相减，也不能先缩窄再判断。所有模型输入及地址均小于 2^64，模型
没有生成设备指针或访问所列地址。Parent 的 host 元数据只含 shape/dtype，
没有共同 GM 分配的起点、容量或归属信息；本次没有改它的元数据处理。

如果严格保留原 x 源视图，另需 `o+d+L<=N`。对 R 在 X 之后且两个原视图
不相交的情况，`d>=N`；任何有效 tile 的 `o+d+L` 都大于 N。
R 在 X 之前则无法用当前无符号 srcStride 按 x→residual 空间顺序表达。
调换源顺序还需要反向目的跨度，会改变原布局或引入第二个动作，本次不采用。

| 模型输入 | 描述符数值 | 原视图结论 |
|---|---|---|
| D=8193、M=1、FP16，N=16386 B，L=8192 B，d=32768 B | srcStride=24576 B，字段合法；sourceSpan=40960 B | 原 x 只有 16386 B；两个独立原 Load 分别在范围内，共同源视图无依据 |
| 历史 16x16384 FP16 的大小，假设两完整输入顺序相邻，N=d=524288 B | srcStride=516096 B；sourceSpan=532480 B | 相邻完整输入仍不能作为 x 原视图的一次双块读取；不是一个 16384 B 连续 tile |
| d<0、d=0 或 0<d<L | srcStride 为负 | 2201 的 uint32_t 不能表达；950 的相关能力不适用 |
| d-L=2^32-1 / 2^32 | 前者处于字段上限，后者超出 | 字段上限与实际数据范围须分别满足；缩窄会改变第二块地址 |
| D=8193、M=1 的合成重叠视图，d=8192 B | 第一个 8192 B tile 的 span=16384 B，在 x 原视图内 | 是局部正例；第二 tile 和尾部超出原 x 视图，不能整体套用 |

上述地址关系全部是反例或边界模型，没有被描述成当前调用者的真实布局。
重叠正例也没有历史工作负载来源。保留原双 Load 可正确处理无法合并的情况，
但对于现有两个独立、不相交输入，严格原视图条件不产生可合并路径。

无长度 SetGlobalBuffer 得到的 GetSize=0 表示未提供长度，不能据此声明任意跨度
都属于同一个源视图。为它设置更大的 size 同样不提供共同存储事实。只有每个
块分别落在 x/residual 内这一点，不能独立证明目标 API 允许把它们视为一个源。

### GM 辅助逻辑的额外限制

实际头文件 `impl/kernel_utils.h:329` 的 Ext 辅助函数包含：

```cpp
uint16_t stride = intriParams.dstStride;
if (isSrc) {
    stride = intriParams.srcStride;
}
if (isMovAlignIntri) {
    burstLenUnit = 1;
    strideUnit = 1;
}
uint64_t gmLen = static_cast<uint64_t>(intriParams.blockCount) * intriParams.blockLen * burstLenUnit
                 + (intriParams.blockCount - 1) * stride * strideUnit;
```

该代码片段中的 stride 只有 16 位。原 N=524288 B 的模型中，516096 截短成
57344，辅助跨度为 73728 B；完整数值跨度应为 532480 B。辅助跨度仍落在 x
范围内，不能用于证明完整描述符合法。小间距反例的 stride=24576 没有截短，
按 `OOMCheckAddrIsOverflow` 对起点所在单个登记范围的比较，会得到范围超出。

这是读取辅助源码并在本地建模的结果。`ASCENDC_OOM`、登记范围非空等条件
控制设备端辅助逻辑是否执行，本轮没有运行它，没有观察到设备异常。
实际 MTE2 调用仍传递原 uint32_t 字段，不能据此把硬件 stride 上限改写成
16 位。CPU 的目标容量辅助函数还将 GM 源跨度置为 0，不能补足这个证明。
本次没有修改工具链、Parent 或任何辅助实现。

### 重复范围与本次验证

```text
DUPLICATE_AUDIT
MECHANISM = shared-UB same-tile cross-input two-block MTE2
SEARCHED_HISTORY = bd825c5f 中的完整专项范围；本次补读 R05 1a31a3b5
MATCH_FOUND = NO_EXACT_MATCH_IN_READ_SOURCES
WHY_NEW_OR_DUPLICATE = 两个独立输入的描述符合并；不改变issue-order，
  不重复Load/Store组合、跨行flat/strided或R05 gamma消费者偏移。
  缺少原GM视图条件下的完整合法表达，未进入性能版本。
```

复用上文 W3 R2/R4/R5、R31/R31A/R31B、MIX、STORE/EPILOGUE 和相关 W4 的
搜索证据；本次读取的 W3 分支仍分别为 `6321ad44`、`ce6c6dc5`、`1efa0863`，
末版分别为 V040、V031、V028，没有重新搜索已经充分覆盖的版本。
原构造式扫描的覆盖限制继续保留，不声称包含未提交工作。

将原 60 项地址模型保存在同目录 `transaction_view_model.py`，增加共用根容量、
原槽写区间与另一活动槽不重合的断言，并增加 10 项 GM 边界/反例。
命令 `python3 -B 研究/W4-R06/transaction_view_model.py` 返回 0；
`transaction-view-model.log` 保留全部 60 项 UB 数据及 10 项 GM 数据。
最终输出 `UB_CASES=60 GM_CASES=10 RESULT=PASS`。

这里的 PASS 只代表地址模型中的预期等式和反例成立。没有运行 Kernel Compile、
独立 reference Correctness、P/P、P/C、Local 或 Profile；没有新的 latency、
free HBM、load 或 Official 数据。R05 的地址与性能结果也没有计成本 Route 的测量。

### 事件与安全交接

```text
ROUTE_RESEARCH_EVENT = W4-R06-SHARED-VIEW-GM-SPAN-20261008
ROUTE = W4-R06
REVISION = NONE
DIRECT_PARENT = R31B-V011
STATUS = RESEARCH_COMPLETED_NO_CANDIDATE
LAST_ACTION = SHARED_UB_AND_GM_VIEW_PROOF
VERSION_RECORD_EVENT = NONE
COMPILE = NOT_RUN
CORRECTNESS = NOT_RUN
PARENT_SAME_BINARY = NOT_RUN
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
CURRENT_LOCAL_BEST = NONE
NEW_PERFORMANCE_REVISIONS_THIS_TURN = 0
VALID_NUMERIC_LOCAL_RESULTS = 0
CONSECUTIVE_VALID_NO_IMPROVEMENT = 0
OFFICIAL_SCORE = NONE
ONLINE_STATE = PAUSED
PUSH = NO
RESOURCE_BLOCKER = NONE
RUNNING_DEVICE_OPERATION = NONE
STATE_SYNC_GAP = THIS_RESEARCH_EVENT_PENDING_RECORD
EVIDENCE = 研究/W4-R06/TRANSACTION-STRUCTURE-20261008.md;
  研究/W4-R06/transaction_view_model.py;
  研究/W4-R06/transaction-view-model.log
```

本次实际写入仅为上面三个研究文件：在原报告追加本节，新增可复用模型和它的
完整输出。旧 V001、Parent、Candidate、共享 TSV、规则、Dashboard 及其他
工作树均未修改。server3 只读取得文档与头文件，没有创建实验目录、编辑远端
文件或启动 NPU 操作。未触碰已测二进制，没有使用 objcopy，没有删除任何证据。

提交完成后的 commit、最终 HEAD/dirty 与命令结束状态由交接回执给出。
Main/Record 可按本研究事件异步同步；本 Agent 不改变 Route 生命周期。

剩余独立轴仍为单一 GM 视图内、单 tile 零 stride 双块的底层事务区别；当前没有
来源证明它具有独立的事务或时延表现，本次未实施。
精确下一动作：后续安排时，从目标 `impl/dav_c220/kernel_operator_data_copy_impl.h:477`
对应的 `copy_gm_to_ubuf_align_b16` 指令资料取得 blockCount=1 与等总长度
blockCount=2 的事务划分定义；得到可区分的证据后才设计新实验，不扫描块长。
