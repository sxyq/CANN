# W4-R06 x/residual 搬运结构核对

日期：2026-10-08。事件：`W4-R06-TRANSACTION-STRUCTURE-20261008`。

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
