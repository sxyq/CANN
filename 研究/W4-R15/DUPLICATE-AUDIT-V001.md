# W4-R15 SAFE-MULTIROW-DMA-X — V001 DUPLICATE_AUDIT

ROUTE: W4-R15 SAFE-MULTIROW-DMA-X
REVISION: V001（未实施，仅审计）
DIRECT_PARENT: R31B V011
BRANCH: w4/r15-safe-multirow-dma-x
日期: 2026-10-08
结论: ROUTE_DUPLICATE = YES；NEED_FINGERPRINT = YES；不实施 V001

## 1. 父版本字节复核

`归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` 与 `线上结果/R31B/V011/submission.asc`
的 SHA-256 同为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。
无 PARENT_CONFLICT。

## 2. 本 Route V001 允许的改动范围

只允许改「一种 two-row / contiguous-row 传输模式，带显式 correctness-safe 边界」。
明确不做：行调度改动、tile 改动、ownership 改动。父版本与 MULTIROW-DMA-CHAMPION-X
均以 `ChooseWideFullYRows` 的 y 驻留语义与行归属为不可动项。

## 3. 父版本 DMA 形态事实（源码核对）

父版本全文件只有两处 DMA 构造点：`Load` 与 `Store`，均为
`DataCopyPad` + `DataCopyExtParams(nBursts=1, blockLen=count*sizeof(T), 0, 0, 0)`
（`归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` L3367-3383）。
全文件没有任何 `nBursts>1`、没有非零 srcStride/dstStride。

已有的多行搬运全部是「连续整行 flat 长 burst」：

| 路径 | 触发条件 | 形态 |
|---|---|---|
| ProcessSmallFp32ContiguousBatched | FP32，D<=2048，D%8==0，localRows>1 | 一条 flat burst 覆盖 batchRows*rowWidth，batchRows<=8 |
| ProcessSmallLowPrecisionContiguousBatched | FP16/BF16，D<=2048，D%16==0，localRows>1 | 同上，batchRows<=8 |
| ProcessSmallFp32Batched | FP32，D<=4096，D%8==0 | 输入仍逐行单 burst，输出 flat 批写 |
| ProcessNarrowMidOverlap | 128<D<=4096 未命中上述 | 逐行整行单 burst |

来源：`研究/MULTIROW-DMA-CHAMPION-X/DMA-INVENTORY-R31B-V011.md`（commit cacb157f），
并已与本 worktree 中父版本源码逐条核对一致。

## 4. 历史 multi-row DMA 失败与运行时风险证据

### 4.1 MODE-X-R015C（本机制族的直接失败证据）

路线声明的机制就是「Strided 2-D DataCopy for adjacent FP32 rows」，
即「相邻两行一次 2-D 传输」，与本 Route V001 的 contiguous two-row 读法同机制。
来源：`归档/历史工作区/MODE-X-R015C/LOAD_QUALITY.md`、`技术路线/全版本记录.tsv` 第 7-11 行。

| 版本 | 机制 | NPU 结果 |
|---|---|---|
| CURRENT-UNNUMBERED / r1 | 两行一块，2-D DataCopy（DataCopyParams，blockCount/blockLen） | (2,256) PASS；(5,4096) 与 (3,8192) 在 FP32 元素 2048 处首次不一致（恰好第 257 个 32B 块）。另一变体直接 `ACL_ERROR_RT_VECTOR_CORE_EXCEPTION (507035)` + `The write address of the MTE instruction is out of range`，blocks 0-2 |
| r2 | 单行分段，每段 <=256 个 32B 块 | 三个形状全部在 index 0 不一致 |
| r3 | 单行分段 <=256 块 + Ext DataCopyPad（字节长度 blockLen，isPad=false）+ PIPE_ALL | 三个形状全部 PASS（exact） |
| r4 | 一行一块，沿用 r3 分段纪律 | 终局 CORRECTNESS_FAILED |

设备观测到的边界是「每行每次传输 <=256 个 32B 块」，即 8192 B。
该边界来自设备实测，不是头文件里的规格上限；CANN 8.5.0.alpha002 dav_c220 头文件
只声明 blockLen 1..65535、blockCount 1..4095。

### 4.2 MULTIROW-DMA-CHAMPION-X（直接前例，同父版本）

| 版本 | 机制 | 结果 |
|---|---|---|
| V001 | H1 stride multi-row：LP wide pass-1 输入按 nBursts=B、srcStride=(rowWidth-tile) 发多行；满 32B 对齐 tile 走 stride 事务，尾 tile 回退单 burst；实现里 `kStrideChunkRows = 2`，即按两行一组发 | Correctness 在触及路径与父版本逐位一致，R015 的运行时故障未复现；但 48x16384 FP16 主形状 4/4 退步 +3.2%/+9.6%/+13.4%/+7.2%（中位 +8.4%），48x12288 BF16 落在噪声带，8x16384 对照 +/-4.7% 噪声底约 5pp。LOCAL_REJECTED |
| V002 | C1 对齐非 Pad DataCopy 指令形态（blockLen 以 32B 为单位），2-deep 不动 | 16 对中位 -1.84%，同代码对照中位 -1.48%、散布 -7.1%…+5.4%，不可分。NEEDS_ONE_MORE_LOCAL 终局 |

机制结论（LANE-FACT-PACK-FOR-PLANNING.md，commit 0932fbd5）：
主导成本是 MTE2 流水重叠与 V 端指令/事件，不是命令计数，也不是单条搬运指令形态；
零 UB 增长下无法同时保留 2-deep 与 B 行暂存。
轴内唯一剩余的结构性选项 C2（stride 合并 + 保 2-deep）必须改 UB 计入
（ioTiles 4->6 或等价压缩），会动 `ChooseWideFullYRows` 的 y 驻留语义，
且其 (a)/(b)/(c) 取舍仍等 Planning 裁定，本 agent 不选。

### 4.3 其余三项历史机制（均不属于 DMA 形态轴）

- R2 V004 rowStride：round-robin 行归属（rowStride=blockCount），且所有快路径加
  `rowStride==1` 限制。属于行调度/ownership 范畴，本 Route 明确禁止。
  证据：`技术路线/技术路线图.md` 第 416 行；
  `本地实验/ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X/V004/{parent,submission}.asc` 差分。
- ASYNC-OVERLAP-CHAMPION-X V002：NarrowMid 跨行提前发射（把下一行 x/res 载入提到
  invRms 尾部之前，推迟 SyncMTE3ToV），128x2048 fp16 / 128x4096 fp32 正确性 7/7
  OUTHASH 一致，NEEDS_ONE_MORE_LOCAL。属发射顺序，不改 DMA 形态。
- ASYNC-TRIPLE-X H3：pass-2 内 CopyOut(N-1) 与 CopyInOutputData(N+1) 的发射顺序对调。
  属发射顺序/仲裁，不改 DMA 形态。

## 5. 机制比对与判定

「two-row / contiguous-row 传输模式」在父版本上只有两种可落地读法，两种都被已有事实覆盖：

| 读法 | 落地位置 | 机制归属 | 判定 |
|---|---|---|---|
| A. 相邻两行在 GM 连续，合并为一条 flat 多行 burst | ProcessNarrowMidOverlap、ProcessSmallFp32Batched 的逐行输入 | 与 MODE-X-R015C r1/r2 同机制（相邻行一次 2-D/多 burst 传输） | 旧失败机制相同。r1 在每行超过 256 个 32B 块后输出错误，一个变体还触发 MTE 写地址越界 507035；r2 三个形状全错。不得实施 |
| B. 同一 tile 列窗跨两行，用 nBursts=2 + 非零 srcStride | LP wide pass-1 输入（ProcessWideLowPrecision） | 与 MULTIROW-DMA-CHAMPION-X V001 同机制，V001 实现本身就是 `kStrideChunkRows=2` 按两行一组发 | 旧失败机制相同。V001 已在同一父版本上 LOCAL_REJECTED，主形状 4/4 退步、中位 +8.4% |

读法 A 若限定在「相邻行一次事务」，需要同时满足三条约束，而它们的交集已被父版本覆盖：

1. UB 约束：NarrowMid 与 generic 路径的输入缓冲是 `xBuf_`，容量 `kTileElems = 4096` 元素。
   两行必须落进同一个缓冲（`dstStride=0`），即 `2 * rowWidth <= 4096`，`rowWidth <= 2048`。
2. DMA 字节约束：MODE-X-R015C r1 的实测边界是「每个 burst <= 256 个 32B 块」= 8192 B。
   对应 `blockLen = rowWidth * sizeof(T) <= 8192`，FP32 与 FP16/BF16 都给出 `rowWidth <= 2048`
   （FP16/BF16 元素更小，字节约束在 `rowWidth <= 4096` 处才生效，被 UB 约束更紧地压住）。
3. 对齐约束：非 Pad `DataCopy` 要求 `blockLen` 为 32B 整数倍。
   `rowWidth <= 2048` 且对齐的形状，正好落在父版本两条 CONTIG 分支的门槛上
   （FP32 `D % 8 == 0`；FP16/BF16 `D % 16 == 0`），两条分支一次 flat burst 最多搬 8 行，
   命令数严格少于两行一次的形式。

不满足对齐门槛的形状（FP32 `D % 8 != 0`、FP16/BF16 `D % 16 != 0`）才进 NarrowMid 或 generic，
它们的 `rowWidth * sizeof(T)` 不是 32B 整数倍。此时只有两种 API 形式可用：
非 Pad 形式被对齐门槛挡住；Pad 形式会在同一缓冲里写入补齐元素，
与相邻行数据重叠，无法得到 correctness-safe 边界。因此该窗口内不存在可辩护的两行事务。

`rowWidth > 2048` 的部分：generic 多 tile 路径（4096 < D <= 8192）与 wide full-y 路径
的输入不是「相邻两行整行连续」，只能按 tile 列窗跨行，走的就是读法 B。

读法 B 的两个已知形态（命令数减半、指令 Pad/非 Pad 形态）都已实测为非主导成本；
剩余结构选项 C2 需要改 UB 计入并越出本 V001 的允许范围，且等 Planning 裁定。

## 6. Local 矩阵

MEASUREMENT_DESIGN_BLOCKED = YES。
不是缺少 shape 记录，而是可辩护的 shape 集合为空：
满足 UB 容量与 32B 对齐的形状全部已由父版本 CONTIG 路径以更宽形态（<=8 行一次）承接；
不满足对齐的形状无法用非 Pad 形式，Pad 形式会与相邻行数据重叠；
超出 `rowWidth <= 2048` 的形状只能按 tile 列窗跨行，即读法 B，已 LOCAL_REJECTED。
若要在观测到的 256 块边界之上继续，缺少可信的每传输行块数上限指纹。

## 7. 判定

- ROUTE_DUPLICATE = YES（读法 A 与 MODE-X-R015C 旧失败同机制；读法 B 与
  MULTIROW-DMA-CHAMPION-X V001 旧失败同机制）
- NEED_FINGERPRINT = YES（若后续要在观测边界之上继续，需要可信的每传输行块数上限指纹）
- 不实施 V001，不做任何 Candidate edit，不进入 Compile / Correctness / Local
- 未创建 Revision，未产生 RESULT，因此不发 VERSION_RECORD_EVENT
- STATE = HOLD_FOR_PLANNING；不自行 PARK / CLOSE / REPLACE / MERGE

## 8. 本轮未做的事

- 未修改任何 Kernel 源码、CMakeLists、gen_data.py、run.sh
- 未在 server3 上执行 Compile / Correctness / Local / Profile
- 未写入其他 Route 的 worktree（只读取历史分支与归档证据）
- 未写 official-result.json，未正式提交 Online
