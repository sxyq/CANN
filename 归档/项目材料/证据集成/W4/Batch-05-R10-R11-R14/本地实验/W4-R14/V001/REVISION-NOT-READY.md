# W4-R14 PARAM-DMA-GRANULARITY-X — V001 REVISION_NOT_READY

ROUTE = W4-R14 PARAM-DMA-GRANULARITY-X
REVISION = V001
STAGE = DUPLICATE_AUDIT + REVISION_DECLARATION
VERDICT = REVISION_NOT_READY
NEED_FINGERPRINT = YES
DECISION = NO_EDIT
EDIT_PERFORMED = NO
CANDIDATE_WRITTEN = NO
SERVER3_ACTION = NONE
STATE = HOLD_FOR_PLANNING

## Parent verification

PARENT = R31B V011
PARENT_PATH = 归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA256_EXPECTED = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA256_RESULT = MATCH
SUBMISSION_COPY = 线上结果/R31B/V011/submission.asc
SUBMISSION_COPY_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BYTE_IDENTICAL = YES
PARENT_CONFLICT = NO

## Rule refresh

RULE_REFRESH_RECEIPT 在写本文件之前发出。六个入口已在本 worktree 逐项读完：

- AGENTS.md
- .agents/skills/cann-route-executor/SKILL.md
- 项目规则/实验总则.md
- 项目规则/执行约定.md
- 项目规则/服务器实验规范.md
- 项目规则/本地性能测试规范.md

规则正文仍写 ACTIVE PORTFOLIO W3。当前会话指令写 W4，按优先级取 W4。

## DUPLICATE_AUDIT

ROUTE = W4-R14
RELATED_OLD_ROUTES = COEFF-LOCALITY-X V001-V004；
  CROSSROW-FULL-PIPELINE-CHAMPION-X V012 / V015 / V016 / V019 / V020；
  MULTIROW-DMA-CHAMPION-X V001 / V002；W4-R06 MTE2-XRES-ISSUE-X V001
SAME_MECHANISM_ALREADY_TESTED = NO
WHAT_IS_DIFFERENT = 历史改的是参数搬运的时序、次序、槽位相位、驻留时长，
  以及输入搬运的事务条数与指令形式；本 Route 要改的是参数搬运自身的事务条数与
  单条长度，这一项在父版本上没有直接测量过。
NEW_INFORMATION_EXPECTED = 父版本上仅存的一处合法合并站点落在 2x8192 / 3x6144
  这类形状，而 COEFF-LOCALITY-X V004 已在同一父版本的同一批形状上记录
  “残差信号无法与设备侧测量偏置分离”，因此本次 Local 无法给出可读结论。

### 同一父版本上最接近的既有工作（参数路径）

| Revision | Commit | 实际改动 | 记录结果 |
|---|---|---|---|
| COEFF-LOCALITY-X V001 | 6e47a9a0 | FP32 wide pass-2 参数双槽预取，staging 深度 1→2 | 1x32768 FP32 偏向父版 +19.2%，LOCAL_REJECTED |
| COEFF-LOCALITY-X V003 | 5d98c691 | pass-2 参数分相发射，gamma 在 Mul 后、bias 在 Add 后发出 | 每 tile 仍是 2 次 Load，1x32768 +2.37% 偏向父版，LOCAL_REJECTED |
| COEFF-LOCALITY-X V004 | 498a3830 | cacheParams 条件放宽到单核多 tile（驻留提前） | 1x8192 被否，残差不可分离，NEEDS_ONE_MORE_LOCAL |
| CROSSROW V012 | b68fcebe | FP16 pass-2 参数双缓冲起始槽位相位 | 未提升 |
| CROSSROW V015 | 066b7212 | FP16 pass-2 参数槽位 V_MTE2 释放位置 | 未提升 |
| CROSSROW V016 | 0f020a20 | FP16 pass-2 bias 先发、gamma 后发 | 配对 -21.1246%，账本判 LOCAL_NOT_IMPROVED |
| CROSSROW V019 | 39d43f2d | FP16 pass-2 按 tile 奇偶交替参数发出次序 | +2.5048%，未提升 |
| CROSSROW V020 | 7b47e83c | FP16 pass-1 双缓冲槽位相位 | +0.1901%，未提升 |
| W4-R06 V001 | b09e00eb | pass-1 输入 x/residual 发出次序 | ROUTE_DUPLICATE_BLOCKED，停止 |

### 同一父版本上最接近的既有工作（DMA 命令条数）

MULTIROW-DMA-CHAMPION-X V001 在同一父版本上把输入 x/residual 的逐行 DataCopy
合并成 stride 多块事务，nBursts = B；48x16384 FP16 主形状 4/4 退步，
中位 +8.4%，LOCAL_REJECTED。V002 把 Pad 形式换成非 Pad 单突发形式，
合并 16 对中位 -1.84%，对照中位 -1.48%，落在噪声内，终局
NEEDS_ONE_MORE_LOCAL。账本原文：

> C1 instruction form not dominant cost; command count (V001) and instruction
> form (V002) both non-dominant

两者的搬运对象都是输入，不是参数。

### 判为不同机制的对象

- MULTIROW-DMA-CHAMPION-X：输入事务条数与指令形式，非参数。
- ALIGN-TAIL-X：尾部块对齐，非参数。
- ASYNC-TRIPLE-X / EPILOGUE-FUSE-X / STORE-EPILOGUE-X：输出侧重叠与融合。
- W4-R06：pass-1 输入发出次序，已 DUPLICATE_BLOCKED。

## 父版本参数搬运站点清点

父版本 Load 辅助函数（第 3367-3375 行）是固定形态的单块搬运：

```cpp
const AscendC::DataCopyExtParams copyParams(
    1, static_cast<uint32_t>(count * static_cast<int32_t>(sizeof(T))), 0, 0, 0);
const AscendC::DataCopyPadExtParams<T> padParams;
AscendC::DataCopyPad(dst, src[offset], copyParams, padParams);
```

blockCount 恒为 1，srcStride 与 dstStride 恒为 0。

| # | 位置 | 行号 | gamma/bias 事务数 | 可否改粒度 |
|---|---|---|---|---|
| 1 | Process generic cacheParams=true 预载，FP32/FP16 | 251-256 | D=8192 时 2 条，每条 4096 元素 | 可合并 |
| 2 | Process generic cacheParams=true 预载，BF16 | 273-274 | 2 条 staging，逐条 ToFloat | 不可，gammaBuf_ 只有 4096 元素 |
| 3 | Process generic cacheParams=false 输出遍每 tile | 390-391 | 2 x tileCount | 不可，合并等于改驻留时长 |
| 4 | ProcessNarrowMidOverlap 预载 / 每行 | 515-516, 539-540 | 1 条，width <= 4096 | 已是单条 |
| 5 | ProcessBf16FullTileBatchedOutputPipelined | 633-634 | 1 条，width = 4096 | 已是单条 |
| 6 | ProcessFp16FullTileBatchedOutputPipelined | 761-762 | 1 条，width = 4096 | 已是单条 |
| 7 | ProcessBf16FullRowOutputPipelined | 888-889 | 2 条 staging，逐条 ToFloat | 不可，同 #2 |
| 8 | ProcessFp16FullRowOutputPipelined | 1012-1013 | 2 条，每条 4096 元素 | 可合并 |
| 9 | ProcessFp32FullRowOutputPipelined | 1130-1131 | 2 条，每条 4096 元素 | 可合并 |
| 10 | ProcessSmallFp32Batched | 1382-1383 | 1 条，rowWidth <= 4096 | 已是单条 |
| 11 | ProcessSmallFp32FullTileBatched | 1467-1468 | 1 条，width = 4096 | 已是单条 |
| 12 | ProcessSmallFp32ContiguousBatched | 1582-1583 | 1 条，width <= 2048 | 已是单条 |
| 13 | ProcessSmallLowPrecisionContiguousBatched | 1679-1680 | 1 条，width <= 2048 | 已是单条 |
| 14 | ProcessWideFp32FullCacheRows pass 2 | 2191-2192 | 每 batch 每 tile 2 条，落在 xBuf_/residualBuf_ 别名槽 | 不可，合并需加宽槽位，会改 UB 预算与 tileElems |
| 15 | ProcessWideLowPrecision pass 2 | 3261-3264, 3295-3296 | 双槽预取，每 tile 2 条 | 每槽已是单条 |

以下函数在父版本中只有定义、没有调用点，参数搬运不生效：
ProcessWideFp32CachedRows（1856）、ProcessWideFp32PanelResident（2248）、
ProcessWideFp32Batched（2383）、ProcessWideFp16CachedRows（2720）、
ProcessWideBf16CachedRows（2802）、ProcessWideFp16BatchedOutputPipelined（2895）。

## 为什么推不出可辩护的单一改动

### 方向 (a) 分离传输 → 一次合并事务

API 依据，本地文档
`.cannbot/dependencies/ops-direct-invoke/asc-devkit/docs/zh/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopyPad_GMToUB.md`：
blockCount 取值 [0, 65535]；blockLen 取值 [0, 2^21-1] 字节；
dstStride 单位为 32B dataBlock；dst 需 32B 对齐；blockLen 需为 sizeof(T) 的整数倍。

跨张量合并不成立。gamma 与 bias 是两个独立 GM 张量，第 52-53 行分别
SetGlobalBuffer。#3、#4、#5、#6、#10-#15 各需要 2 条传输，来源地址分属两个
张量。把两条并成一条，要求 bias 的首地址正好等于 gamma 末地址加 1 字节，
这由 Host 侧分配决定，内核无法假定。源码中没有支持这一合并的地址证据。

同张量内合并在 API 与 UB 两侧都成立。#1、#8、#9 是同一张量内的相邻两段：
GM 偏移连续、UB 目的连续、gammaBuf_ 容量恰为 kCacheElems = 8192
（FP32 第 121-122 行、FP16 第 126-127 行），所以 8192 元素的单条搬运
在 blockLen 上限和 UB 容量上都合法。BF16 的 gammaBuf_ 只有 kTileElems = 4096
（第 137-138 行）且逐条 ToFloat，不成立。

合并后的量级。每个核在进入主循环之前少发 1 条参数命令，gamma 与 bias 仍各 1 条，
只是每条从 4096 元素变成 8192 元素。D=8192 时 FP32 预载字节数本身只有 65536B，
而同形状下输入流量是每行 32768B 乘 localRows。这个一次性的 1 条命令差
落在 `归档/历史控制文件/local-timing-protocol.md` 记录的 MAD/median <= 0.10
与 block drift <= 0.10 门限之下。

合并幅度最大的一处已经测过。#3 的“每 tile 两次”换成“整行一次”，就是
COEFF-LOCALITY-X V004 的 NH-1；该版只改 cacheParams 条件，预载块逐字节复用，
预载内部仍是两次 Load。结果为 NEEDS_ONE_MORE_LOCAL，路线记录写明
“residual signal cannot be separated from apparatus bias; no credible
positive Candidate”。

### 方向 (b) 一次大传输 → 两段式传输

#4、#5、#6、#10-#13 已经是单条。拆成两段只是多发 1 条命令，两段都落在同一个
MTE2_V 等待之前，不产生新的重叠窗口，源码中没有支持依据。

反向证据来自同一父版本的 MULTIROW-DMA-CHAMPION-X。减少命令条数（V001 合并多块）
在主形状上退步 +8.4%，指令形式（V002）落在噪声内，账本已把命令条数与指令形式
都记为非主导因素。把单条拆成两条是同一方向的相反极端。

### 激活形状与已知的测量困难

#1、#8、#9 的激活条件是 rowWidth 落在 (4096, 8192) 或等于 8192 且 localRows > 1。
分派见第 178-181、207-210、218-221、245 行，widePath_ 阈值
kCacheElems = 8192 见第 1288 行。可测形状因此是 2x8192、3x6144 这类少行宽形状。

COEFF-LOCALITY-X V004 在同一父版本上量的正是这批形状。账本记录为
1x6144 -4.25%..-10.22%，15/18 偏向候选，但对照 1x32768 -2.77%，
1x8192 被否，残差不可分离。

在同批形状上再改一次参数事务条数，得到的“落在噪声内”无法区分两件事：
参数事务条数本身不重要，还是这次改动量级太小。

## Local 矩阵

未执行。形状来源已核实，取自 COEFF-LOCALITY-X V004 的实测矩阵：

- 目标形状，tileCount = 2：2x8192 FP32、3x6144 FP32
- 对照：1x32768 FP32，V004 记录为装置偏置对照
- 边界形状：1x8192 FP32，localRows = 1 时本改动不激活

dtype 固定 FP32。BF16 预载 staging 只有 4096 元素，粒度改动不成立，不混 dtype。
矩阵本身可由现有证据支撑，阻塞点不在形状，而在上述量级与形状类别。

## 缺少的指纹

需要而当前仓库没有的事实：在目标形状上，参数搬运每核发出的 MTE2 命令条数，
以及这部分命令发射在核时间线上的占比。具体缺口：

1. msOpProf 或 msprof 内核级采集，在 2x8192 FP32、3x6144 FP32、1x32768 FP32、
   16x16384 FP16 上给出参数 MTE2 命令条数与 MTE2 发射时间占比；
2. 由此得出参数事务条数在核时间线上的占比这一个数；
3. 该占比若低于噪声门限对应的绝对时间，本轴不值得占用一次 Revision。

仓库当前没有针对 R31B V011 的内核级采集目录。`本地实验/` 与 `技术路线/`
下只有 local timing 与 official 结果，没有 msopprof 输出，所以无法在
Revision 内自行补齐。

## 可观测参数

M = UNKNOWN，未执行
D = UNKNOWN，未执行
DTYPE = UNKNOWN，未执行
DISPATCH = Process generic cacheParams 预载（FP32/FP16）、
  ProcessFp32FullRowOutputPipelined 预载、ProcessFp16FullRowOutputPipelined 预载
BLOCKCOUNT = UNKNOWN，未执行
ACTIVECORECOUNT = UNKNOWN，未执行
LOCALROWS = UNKNOWN，未执行
ROWSPERBLOCK = UNKNOWN，未执行
TILEWIDTH = kTileElems = 4096，常量，第 1242 行
TILECOUNT = UNKNOWN，未执行
OWNERSHIP_MAPPING = 行连续分片，baseRows = rowCount / blockCount，
  extraRows = rowCount % blockCount，第 171-176 行
MTE2_TRANSACTIONS = 现状 gamma 与 bias 各 1 条每段；可合并站点在 D=8192 时
  每核 4 条，2 段乘 2 张量
MTE3_TRANSACTIONS = 不涉及
EVENT_BARRIER = MTE2_V 等待在消费点，预载站点无 V_MTE2 释放
PARAM_RESIDENCY = generic cacheParams 与 FullRow 站点为每核预载后全程驻留；
  wide FP32 pass 2 别名 xBuf_/residualBuf_，逐 batch 逐 tile 重载

## 本轴之外的发现

ProcessWideLowPrecision pass 2 第 3303-3312 行有一段完全重复的非 half 分支：
ToFloat(xFp32, gammaLocal, valid) 与 ToFloat(residualFp32, biasLocal, valid)
加 PipeBarrier 连续出现两次，条件相同、内容相同。属向量侧重复计算，
不是 MTE2 事务粒度，本 Route 未改动，仅记录。

## 安全

HISTORICAL_BRANCHES = 只读。origin/w3/m1/crossrow-full-pipeline、origin/main、
  exp/main2-r2-coeff-locality 的历史提交只用 git show 与 git log 读取。
  历史 W2/W3 worktree 内没有 build、write、commit、push。
WRITES_THIS_ROUTE = 本文件，位于
  /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R14-param-dma-granularity-x
SERVER3 = 未连接。无 Compile、Correctness、Local、Profile。
SHARED_RECORDS = 未写入。全版本记录.tsv、路线成绩表.tsv、当前任务.tsv、
  技术路线图.md 与 Dashboard 均未触碰。
OTHER_ROUTES = 其他 Route 的 worktree、分支、实验目录均未读写。
ONLINE = PAUSED。没有 Judge 提交。

## 用词

本文件不含 AGENTS.md 列出的禁用表达。
