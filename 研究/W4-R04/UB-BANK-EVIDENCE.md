# W4-R04：DAV_2201 UB 布局资料与旧实验核对

## 结论与本次状态

已取得明确适用于 Atlas A2/A3、NPU 架构 2201 的官方 UB 资料：192 KiB、48 个 bank、16 个 bank group，每组 3 个 bank；每个 bank 为 4 KiB，128 行，每行 32 B。服务器安装的 8.5.0.alpha002 配置却含 `ubbank_num=64`。该字段与同文件容量不一致，原因仍为 UNKNOWN，不能据此把 64 写成设备实测 bank 数。

R1 V001–V010 的实际源码差异及旧样本已核对。交换 x/residual 分配顺序、宽行分配 padding、初始 A/B 槽位相位均有历史，不重跑旧性能实验。随后根据新到的 R05 子视图证据，完成窄行输入消费者的专项核对：NarrowMid FP32 可在既有分配中只移动 residual 视图，不改变后续分配或参数位置；174 份已读源码未发现该变化。该新轴已具备源码范围与容量依据，设备精度和收益仍待完整 OFAT。本文件的研究事件完成时新增性能版本为 0，未改变 Route 生命周期。

agent_id：`01a119cc-fa30-7740-8fd2-9cdfab6e6c46`，SLOT-4。
工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R04-ub-bank-xr-layout-x`。
分支：`w4/r04-ub-bank-xr-layout-x`。接手提交：`de70b634813dea80783fc57716d6e95c158edeec`，接手时无未提交内容。
Parent：同提交的 `线上结果/R31B/V011/submission.asc`；保留该继承参考，R04 自有 `CURRENT_LOCAL_BEST=NONE`。

指定规则 `9f91895506023d917637f707bb3f61cd9d9f8765` 已按用户要求完整读取。开始时在该规则提交的三份共享表中未检出 R04；这是旧对象的查询快照，不代表当前登记状态。`w3/m1/record-owner@cecec26bd852540c52b1e932cba7b9cd041331cc` 有旧长 ID 的 V001 条目，明确未编辑 Candidate、未运行实验。故 `LAST_KNOWN_REVISION=V001 (old research only)`，真实性能版本仍为 0。

2026-10-08 状态补充：从本工作树读取的 `main@285e7b4c2a6810774ca46c053bb7cac0f0b52223` 已在任务表、成绩表登记 R04/R05，均标为 `REGISTERED_ROUTE`；阶段仍为 QUEUED，结果为 UNKNOWN，版本表未出现两者的真实性能版本。当前 `STATE_SYNC_GAP=RESEARCH_EVENT_PENDING`，只待新研究事件同步。本 Route 不改共享表，后续状态以 main 及最新来源回执为准。

## 官方资料事实

| 来源 | 适用范围与可确认内容 |
|---|---|
| [CANN 8.5：220x 架构](https://www.hiascend.com/document/detail/en/canncommercial/850/opdevg/Ascendcopdevg/atlas_ascendc_10_0011.html) | A2/A3；AIV 的 UB 数据访问与对齐粒度为 32 B。L0 的 512 B 要求不能移用于 UB。 |
| [CANN 8.5：UB bank 冲突](https://www.hiascend.com/document/detail/en/canncommercial/850/opdevg/Ascendcopdevg/atlas_ascendc_best_practices_10_0025.html) | A2/A3；给出 48 bank、16 group、4 KiB/bank、32 B/行，以及同 bank 读写、同 group 多读或多写的限制和地址示例。 |
| [CANN 9.1：明确标注 2201 的说明](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/910/programug/Ascendcopdevg/docs/en/guide/operator_practice/simd_operator_optimization/memory_access/avoid_ub_bank_conflict/avoid_bank_conflict_npu_arch_2201.md) | 与 8.5 的 2201 结构和地址示例一致；未借用该版本中 3510 的参数。 |
| [8.5 原始结构图](https://www.hiascend.com/doc_center/source/en/canncommercial/850/opdevg/Ascendcopdevg/figure/en-us_image_0000002534425193.png) | 已下载并目视核对，见 `official-850-bank-structure.png`。 |
| [8.5 原始地址图](https://www.hiascend.com/doc_center/source/en/canncommercial/850/opdevg/Ascendcopdevg/figure/en-us_image_0000002534425197.png) | 已下载并目视核对，见 `official-850-address-example.png`。 |

对图示中的 UB 字节地址 A（0 ≤ A < 196608），可以将其布局写成以下等价算式。算式是对官方图示和示例的整理，没有从延迟反推地址规律：

```text
group(A) = floor(A / 32) mod 16
bank(A)  = 16 * floor(A / 65536) + group(A)
row(A)   = floor((A mod 65536) / 512)
byte(A)  = A mod 32
```

例如 `0x10000→bank16`、`0x10020→bank17`、`0x10E20→bank17`、`0x20020→bank33`。本次程序对文档例子和图中边界共 10 个地址完成一致性验证。它验证的是资料模型，未实测硬件地址译码。

文档描述每个 group 每拍只能服务一行的读或写；一个常见 Vector repeat 有 8 个 32 B DataBlock。由此可分析同一 repeat 的地址组重合。实际指令调度、跨引擎竞争及其总耗时贡献仍需设备证据，不能用这个模型直接算 Local 加速比。

## server3 安装内容与限制

`sdk-buffer-evidence.txt` 保存 2026-10-08T04:50:42.998430Z 的只读输出。唯一读取的 SDK 为 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`，未修改服务器文件。

| 位置/字段 | 读到的内容 | 解释 |
|---|---|---|
| `aarch64-linux/data/platform_config/Ascend910B3.ini` | `SoC_version=Ascend910B3`、`AIC_version=AIC-C-220`、`CCEC_AIV_version=dav-c220-vec` | 安装内容与当前目标相符。 |
| `AICoreSpec.ub_size` | 196608 | 与官方 192 KiB 一致。 |
| `ubblock_size / ubbank_size / ubbank_group_num` | 32 / 4096 / 16 | 配置值，与文档对应。 |
| `ubbank_num` | 64 | 64×4096=262144，与同文件 `ub_size` 不同；具体字段成因 UNKNOWN。 |
| `ubburst_in_one_block` | 32 | 只保留字段，不将其重新解释为周期数或端口数。 |
| `kernel_tpipe_impl.h:289` | TBuf 长度按块对齐；将当前 pool 的 `maxAddr` 赋给地址，随后增加长度 | 支持源码层的顺序分配推导。 |
| 同文件 `ResetPool:920` | pool 的 `maxAddr` 从 0 开始 | 用于下面的源码模型；没有替代已编译对象的实际地址观察。 |

已完整读取获准的已安装 `npu-arch`、`ascendc-docs-search`、API 使用及直调模板 Skill；其中架构映射将 Ascend910B 系列对应到 DAV_2201。Skill 索引只用作导航；具体访问粒度以上述官方说明和实际 SDK 为依据。

R03 提交 `712e47230287182bc65ab433a3ce714e6fdafd9f` 中设备 3 的 `VECTOR_CORE_NUM=40` 直接复用，未重新运行其核数探针。该值不提供 bank 映射。此次只读资源查询得到 HBM 容量 65536 MB、使用率 13%，按项目方法估算可用 57016 MB；AICore 0%、AIVector 3%。这些是查询时的上下文，本次未启动 NPU kernel、Profile 或性能计时。

用户转交的 `bd825c5fd59c74c283478026e8b32702ba331eb5:研究/W4-R06/TRANSACTION-STRUCTURE-20261008.md` 已完整读取并复用。其证据范围是 server3 CANN 8.5 dav_c220 的 DataCopyExt 字段、32 B 目标占用和 LocalTensor 容量公式：原有两个独立 x/residual 视图不能直接承载跨输入双块目的范围。该结论不提供 bank 映射，也不证明任何跨输入 GM 地址关系；本次没有重复读取这些 DMA 头文件。R04 的地址组分析不能用来绕过这项视图容量要求，跨输入双块描述符仍属于 R06 的研究范围。

## W3 R1 V001–V010：实际差异与阴性证据

来源固定为 `64e32f535348cdde1828cd8a8fd89fad100aaa33`，路径前缀 `本地实验/UB-BANK-LAYOUT-CHAMPION-X/`。每份 Candidate 均与该对象中的 R31B V011 比较，全部差异保存在 `source-audit.txt`；没有依据旧共享表推测变化。

下表全部为旧设备 7、FP16 `2×32768` 的数据。每侧 45 warmups、21 对交错采样；P/C 为两侧时长中位数，配对差为每对 C−P 的中位数。它们是两种不同统计量。

| R1 版本 | 唯一源码变化 | P/C 中位数（µs） | 配对差中位数（µs） |
|---|---|---:|---:|
| V001 | residualBuf_ 在 xBuf_ 前分配 | 21.600 / 23.400 | +1.300 |
| V002 | outputBuf_ 容量由 1 tile 改为 2 tiles | 17.700 / 23.940 | +1.720 |
| V003 | gammaBuf_ 容量 +32 B | 58.580 / 38.440 | −20.660 |
| V004 | wide reduction stride 16→32 | 23.520 / 34.120 | −0.140 |
| V005 | valueFp32Buf_ 容量 +32 B | 34.620 / 37.960 | +1.740 |
| V006 | xFp32Buf_ 容量 +32 B | 42.780 / 41.780 | −1.540 |
| V007 | residualFp32Buf_ 容量 +32 B | 14.200 / 18.520 | −0.020 |
| V008 | reduceFp32Buf_ 容量 +32 B | 13.160 / 18.100 | −0.300 |
| V009 | outputBuf_ 容量 +64 B | 45.280 / 22.260 | −3.860 |
| V010 | xBuf_ 容量 +64 B | 15.060 / 23.540 | +1.380 |

所有原始样本仍在上述提交的各版本 `support/logs/local-*.log` 中。程序复算了 20 个旧 case、420 对、840 个单侧样本，中位数与旧日志一致；同时保留另一输入 `2×8192 FP16` 的统计。没有新采样、丢弃极端值或选择最好一次。

旧宽行数据的单侧 CV 为 0.37783–2.54152，且未改变宽行分配的 `2×8192` 对照也出现大幅时长变化。因此，表内若干负差值不能提升为稳定 Local Best。V008 只增加最后一个 TBuf 的尾部容量，所用 buffer 起始地址和访问下标均不变；其数值变化尤其不能被解释成 bank 摆放收益。

十版旧 `CheckOutput` 函数一致。它们把 FP16 x+residual 及 affine 中间阶段按 FP16 舍入，再与 CPU reference 比较，容差为 `0.004 + 0.004 × abs(reference)`。旧日志中两种输入的双方均 PASS；这是该 runner、该输入和该 reference 的结果，未提升为当前完整精度验证或 Official 通过。本轮未运行新的 Correctness。

## 源码推导：x/residual 与参数暂存共用地址

Parent 的 `Init` 宽行 FP16 分支见 L81–97，行批量选择见 L1293，pass-1 见 L3076–3221，pass-2 从 L3246 附近开始。以旧 R1 输入宽度 32768 代入，得到 tile=4096、分配的完整 y 行数=1。按所读 TPipe 分配方式，得到以下地址；这些都是 SOURCE_MODEL，未读取实际设备指针。

| Buffer | 推导起始字节地址 | 长度（B） |
|---|---:|---:|
| xBuf_ | 0x00000 | 16384 |
| residualBuf_ | 0x04000 | 16384 |
| outputBuf_ | 0x08000 | 8192 |
| gammaBuf_（pass-1 的完整 y） | 0x0A000 | 65536 |
| valueFp32Buf_ | 0x1A000 | 16384 |
| xFp32Buf_ | 0x1E000 | 16384 |
| residualFp32Buf_ | 0x22000 | 16384 |
| reduceFp32Buf_ | 0x26000 | 64 |

总分配 155712 B。pass-1 的 x/residual 各有两个连续 8192 B 槽位。两者起点相差 0x4000，在图示模型中同一 256 B repeat 的 8 个 group 全部重合；V001 交换两个等长区间后仍然如此。V010 的 +64 B 使 residual 及所有后续分配整体移动，两个输入每个 repeat 仍重合 6 个 group。这个数值只表示地址集合，未预测执行拍数。

`Add(xLocal, xLocal, residualLocal, valid)` 使用原地输出；读写关系也需要纳入归因。pass-2 又将 `gBase=xBuf_`、`bBase=residualBuf_`，同一地址变成 gamma/bias 暂存。因而扩大 xBuf_ 会同时改变输入、参数、输出和工作区的位置。整核时长不足以单独说明 x/residual 的贡献。

官方 8.5 示例中的 +256 B 可以提示如何把两个 256 B 读区间分开，但在本 Route 上仍属于固定 padding 方向；不把换一个数值包装成新的独立性能概念。

## R05 新来源与既有硬件模型的对应

用户转交的 `61e0aa28d0bdf439a879a171abece8477c806936:研究/W4-R05/PARAM-OUTPUT-LAYOUT-STUDY.md` 已完整读取。该来源只提出 NarrowMid FP32 gamma 子视图位置变化，保持全部 `InitBuffer` 和其他视图不变；没有选定性能偏移值、建立 Candidate 或取得设备结果。此项归 R05，不能计入 R04 的新 x/residual 轴。

以本报告已取得的 TPipe 分配代码，对同一 Parent L116–149 的 FP32 分支作整数地址推导：`gammaBuf_` 起点为 `0x8000`，`valueFp32Buf_` 起点为 `0x20000`。NarrowMid 的 `Mul(valueLocal, valueLocal, gammaLocal, valid)` 位于 L582，两个 Load 位于 L515/L539，均引用同一个 gammaLocal。原地址在官方图示模型中的 group 起点都为 0，bank 分别为 0 和 32；这是 SOURCE_MODEL，未观察设备指针。

设 gamma 子视图偏移 s 个 float。R05 的范围与容量条件为 `128 < D <= 4096`、`s >= 0`、`s % 8 == 0`、`s + round_up(D,8) <= 8192`。结合上面的官方模型，两个读取源的 group 相位差满足：

```text
gamma_start(s) = 0x8000 + 4*s
value_start   = 0x20000
group_phase_difference = (s/8) mod 16
```

对于同一连续 256 B repeat 的两个 8-DataBlock 读取集合，原集合重合 8 个 group；相位差为 8 时集合不重合，对应关系式 `s ≡ 64 (mod 128)`。本次只用整数集合枚举 16 种 group 相位验证该关系，没有扫性能参数，也没有替 R05 选定具体 s。实际输出为：

```text
SOURCE_MODEL_ONLY=PASS
FP32_GAMMA_START=0x8000; FP32_VALUE_START=0x20000
SAME_REPEAT_PARENT_GROUP_INTERSECTION=8
DISJOINT_GROUP_PHASES_MOD_16=[8]; FLOAT_OFFSET_RELATION=s mod 128 = 64
CAPACITY_CONDITION=s mod 8 = 0; s + round_up(D,8) <= 8192
WIDE_FP16_INPUT_ALLOCATION=16384; TWO_FULL_SLOTS=16384; FORWARD_SPARE=0
PERFORMANCE_OFFSET_SELECTED=NONE; DEVICE_OPERATIONS=0
```

上述关系提供可转交的资料依据，不能直接预测 Mul 周期数或整核收益。实际 UB 起点、生成指令的访问方式、原地 Mul 的读写关系及随后 bias Add 的冲突贡献仍未实测；若实际地址或访问方式不符合模型，对应推论即失效。SDK 的 `ubbank_num=64` 与官方 48 的差异也继续保留为 UNKNOWN。

R04 已审阅的宽行 FP16 x/residual 分配各为 16384 B，两个完整槽位各占 8192 B，没有类似 gamma 分配的正向空余子视图空间。这项宽行限制没有排除窄行分配的剩余容量，后续专项核对见下一节。此补充复用既有硬件资料，没有访问 server3 或增加硬件采集。

## R04 窄行输入子视图：新增源码依据

`narrow_input_audit.py` 与输出 `narrow-input-audit.txt` 专门回答新的输入子视图问题，不重算旧性能结果。它从固定 Git 对象读取 174 份源码：Parent 所在对象的 59 份相关历史文件、W3 R1/R2/R4/R5 的 10/40/31/28 份 Candidate，以及 6 份相关 W4 Candidate。含 NarrowMid 的历史函数均与 Parent 相同，只有 W4 R08 的跨行预取函数不同；R08 的 residual 视图起点仍为零。较早版本中没有该函数的文件明确标记为 `NARROW_MID_PRESENT=NO`，不将其当作函数相同。

Parent L116–117 给 x/residual 各分配 4096 个 T，NarrowMid 在 L528–529 获取起点为零的视图，L536 将 residual 数据加载到同一视图，FP32 在 L546 用该视图参与 `Add(valueLocal, xLocal, residualLocal, valid)`。Add 输出是独立的 valueLocal；本路径的参数使用 gammaBuf_/biasBuf_，与宽行复用输入区域的做法不同。所有分配、事件、DMA 数量、数据类型和算术顺序均可保持原样。

因此，一个容量受限的独立输入轴是：仅在 NarrowMid FP32 中，将 residual 的 Load/Add 视图向后移动 64 float（256 B）；条件为 `D <= 4096-64`。D>4032 时保留原视图，其他 dtype 和路径均不改。D 已由原分派限定为大于 128；因为 4032 是 8 的倍数，对每个受影响 D 都有 `64+round_up(D,8)<=4096`。尾部 DataCopyPad 的 32 B 目标范围也完整落在原分配中，沿用 R06 已核对的单块跨度依据。

该 256 B 来自 2201 官方图示的 group 相位关系，没有从旧时长选值。源码模型中 x 起点为 0，residual 起点由 `0x4000` 变为 `0x4100`；两个完整 repeat 的读取 group 集合由重合 8 个变为不重合。其余 buffer 起点不变。地址模型及 SDK 字段差异的限制继续适用，不宣称已测得 bank 冲突下降。

数据语义可逐项对应：原 residual GM 的同一 D 个 float 写入偏移后的视图，原 Add 从该视图读取，余下归约和输出不变；两行之间仍保留原 inputRelease 依赖。此为源码可行性证明，实际 Correctness 必须分别对 Parent/Candidate 与独立 reference 比较。

首组有来源的 Local 输入可复用 `16×2048`、`16×2056 FP32`：来自 W3 R2 V040 runner，R03 `712e4723` 的 host 证据已将两者绑定到设备 3 的 40 个可用核、16 blocks、NarrowMid 每核一行。两者都会受此视图变化影响，不能称为未变化对照。新的正式版本须先保留 Parent same-binary 数据，再交错采样；本研究没有新计时，也没有编辑 Candidate。

## DUPLICATE_AUDIT 范围

| 来源 | 本次实际读取范围与结论 |
|---|---|
| 本 R04 | 当前分支无性能文件；旧 Record 的 V001 是研究记录。 |
| W3 R1 | V001–V010 完整 P/C 源码差异、旧日志、reference 函数；分配顺序和固定 padding 已有实验。 |
| W3 R2，`6321ad4427b1819d172c719ebde190eb447ce9ae` | V001–V040 各自 P/C 差异，筛出布局、stride 与 buffer 相关变化；owner 变换不构成本 Route 的新布局。 |
| W3 R4，`ce6c6dc568256ac8b80b674096bd7ca081b897cb` | V001–V031 Candidate 相对 R31B V011 的累积差异；归约步长、tile 宽度、panel 遍历已有覆盖，V010 还交换了 pass-2 gamma/bias 暂存位置。未把累积差异冒充单版直接差异。 |
| W3 R5，`1efa0863611f1a9b76ab13dd361eba161b32b46d` | V001–V028 各自 Parent.asc/Candidate.asc 差异；V012 初始参数槽相位、V016/V019 参数 issue-order、V020 输入初始槽相位已有覆盖。 |
| R031 / R31A / R31B | R031 D001–D004 分配代码；R31A V016/V024/V025/V026/V028、R31B V016/V017 的分配代码及本工作树版本机制记录；多槽、驻留、生命周期和 tile 变化不重新作为布局发现。 |
| MIX、STORE/EPILOGUE | MIX-A V007、STORE V002 的分配代码及版本机制记录；EPILOGUE 仅核对已有机制记录，未宣称全源码覆盖。 |
| 相关 W4 | R02 `traversal-coverage.md`、R06 `DUPLICATE-AUDIT.md` 及 `bd825c5f` 的 `TRANSACTION-STRUCTURE-20261008.md`、R09 `event-dependency.md`、R05 `61e0aa28` 的 `PARAM-OUTPUT-LAYOUT-STUDY.md` 和旧 R04 Record 条目；资料均通过本工作树的 Git 对象读取。 |

`MECHANISM=allocation order / wide allocation padding / initial double-buffer slot phase`；`MATCH_FOUND=YES`。这些旧机制不创建新版本。

专项补充：`MECHANISM=NarrowMid FP32 residual consumer subview with unchanged allocations`；`MATCH_FOUND=NO_IN_READ_SOURCES`。它有独立的消费者范围和容量证明，区别于移动全部后续分配的宽行扩容；设备精度与 Local 尚未验证。本研究事件仍为 `NEW_PERFORMANCE_REVISION=NO`，后续真实性能版本单独声明和记录。

## 验证、交接与精确下一动作

复现原离线验证：在本工作树执行 `python3 -B 研究/W4-R04/ub_bank_audit.py`。它只读固定 Git 对象并向 stdout 输出，完成 10 个资料地址、11 份分配模型、10 份源码差异、10 份旧 reference 函数和 840 个旧单侧样本的核对。原执行返回 0，完整输出为 `source-audit.txt`。该脚本与原输出均未改变，未重跑旧实验或原脚本。新专项命令 `python3 -B 研究/W4-R04/narrow_input_audit.py` 返回 0，174 份源码结果保存在 `narrow-input-audit.txt`。

共同硬件资料及 R05 消费者对应已可由 Main 转交：复用本文件、两张官方图和 `sdk-buffer-evidence.txt`，无需重新取同一硬件资料。R05 已安全交接；其后续 fresh Agent 可结合自身研究声明参数子视图实验，仍须实际 Compile、独立 reference Correctness 和 Local，不能把本报告的地址集合关系写成性能结果。

R04 的精确下一动作是重读规则、声明单一 residual 子视图变化，完成一版 Compile→Correctness→Local；不新增 bank 探测、不扫偏移值。原宽行的 Parent-only 指令归因保留为后续研究建议：复用 R31B V011 的 `2×32768 FP16`，对照 `2×8192 FP16`，取得实际 x/r 槽位地址、已生成 Add 指令及 ResourceConflictRatio，以区分输入与 pass-2 参数/原地读写贡献。本轮不执行这项额外采集；若后续需要，由 Main/Planning 安排。

仍未确认：配置项 64 的成因、已编译 Parent 的实际 UB 地址与指令级冲突占比、窄行输入子视图的设备精度与 Local 收益。不得将源码可行性或 group 集合关系写成实测结果。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=R04-UB2201-EVIDENCE-20261008
AGENT_ID=01a119cc-fa30-7740-8fd2-9cdfab6e6c46
ROUTE=W4-R04 UB-BANK-XR-LAYOUT-X
EVENT_CLASS=HARDWARE_DOC_AND_SOURCE_RESEARCH
REVISION=NONE
STATUS=SOURCE_AXIS_READY
DIRECT_PARENT=R31B-V011
LAST_KNOWN_REVISION=V001 (old research only; no performance Candidate)
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN (no new device reference run)
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE (R31B-V011 retained as inherited reference)
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
VERSION_RECORD_EVENT=NONE (no performance revision)
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BLOCKER=NONE
STATE_SYNC_GAP=RESEARCH_EVENT_PENDING; R04/R05 registered at main@285e7b4c
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=Main 转交共同资料与 R05 相位关系；R04 声明并完成一版 NarrowMid FP32 residual 子视图 OFAT
```

本次只新增本目录中的研究脚本、输出、资料图与本说明，未改 Candidate、规则、共享 TSV、Dashboard、其他工作树或服务器文件。所有前台查询与离线验证已结束。具体提交及最终工作树状态由完成回执给出。

网页读取限制：内置浏览器的打开请求及一次状态查询均超时，临时标签页的创建/关闭状态未获确认。官方正文由网页检索服务读取，原图直接下载后核对；未启用其他浏览器。
