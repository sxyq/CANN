# W4-R04：DAV_2201 UB 布局资料与旧实验核对

## 结论与本次状态

已取得明确适用于 Atlas A2/A3、NPU 架构 2201 的官方 UB 资料：192 KiB、48 个 bank、16 个 bank group，每组 3 个 bank；每个 bank 为 4 KiB，128 行，每行 32 B。服务器安装的 8.5.0.alpha002 配置却含 `ubbank_num=64`。该字段与同文件容量不一致，原因仍为 UNKNOWN，不能据此把 64 写成设备实测 bank 数。

R1 V001–V010 的实际源码差异及旧样本已核对。交换 x/residual 分配顺序、固定 padding、初始 A/B 槽位相位均有历史。当前未证明新的独立 x/residual 布局轴，不建立 OFAT，不重跑旧性能实验。本次为 `ROUTE_RESEARCH_EVENT`，新增性能版本 0，未改变 Route 生命周期。

agent_id：`01a119cc-fa30-7740-8fd2-9cdfab6e6c46`，SLOT-4。
工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R04-ub-bank-xr-layout-x`。
分支：`w4/r04-ub-bank-xr-layout-x`。接手提交：`de70b634813dea80783fc57716d6e95c158edeec`，接手时无未提交内容。
Parent：同提交的 `线上结果/R31B/V011/submission.asc`；保留该继承参考，R04 自有 `CURRENT_LOCAL_BEST=NONE`。

指定规则 `9f91895506023d917637f707bb3f61cd9d9f8765` 已按用户要求完整读取。该提交的三份共享表未检出 R04；`w3/m1/record-owner@cecec26bd852540c52b1e932cba7b9cd041331cc` 有旧长 ID 的 V001 条目，明确未编辑 Candidate、未运行实验。故 `LAST_KNOWN_REVISION=V001 (old research only)`，真实性能版本仍为 0，保留 `STATE_SYNC_GAP`。本 Route 不改共享表。

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
| 相关 W4 | R02 `traversal-coverage.md`、R06 `DUPLICATE-AUDIT.md` 及 `bd825c5f` 的 `TRANSACTION-STRUCTURE-20261008.md`、R09 `event-dependency.md` 和旧 R04 Record 条目；资料均通过本工作树的 Git 对象读取。 |

`MECHANISM=allocation order / fixed padding / initial double-buffer slot phase`；`MATCH_FOUND=YES`。当前没有独立性和精度均已得到证明的新表示法，`NEW_PERFORMANCE_REVISION=NO`。没有宣称全部可能布局已经用尽。

## 验证、交接与精确下一动作

复现本次离线验证：在本工作树执行 `python3 -B 研究/W4-R04/ub_bank_audit.py`。它只读固定 Git 对象并向 stdout 输出，完成 10 个资料地址、11 份分配模型、10 份源码差异、10 份旧 reference 函数和 840 个旧单侧样本的核对。原执行返回 0，完整输出为 `source-audit.txt`。

共同硬件资料已经可交给 R05：复用本文件、两张官方图和 `sdk-buffer-evidence.txt`，无需重新取同一硬件资料。R05 仍需针对自己的参数/输出消费者作源码归因。

下一项有判别力的 R04 研究是 Parent-only 指令归因：复用 R31B V011，在有来源的 `2×32768 FP16` 输入下，一次性取得实际 x/r 两个槽位的 UB 指针、已生成的 Add 指令与 ResourceConflictRatio；对照 `2×8192 FP16`。只回答 pass-1 输入读取是否存在可定位的冲突，以及它与 pass-2 参数访问、原地 Add 读写是否可区分。若做不到这种区分，不从整核计时继续选择 padding。具体测量入口由下一次接手时在本 Route 目录建立，先用现成二进制和结果；本次未执行该探针，也不创建后台任务。

仍未确认：配置项 64 的成因、已编译 Parent 的实际 UB 地址与指令级冲突占比、独立于固定 padding/顺序交换/槽位相位的新 x/residual 表示法、其精度与 Local 收益。新的表示法只有在来源核对、字节覆盖和数值语义均明确后才能成为 OFAT。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=R04-UB2201-EVIDENCE-20261008
AGENT_ID=01a119cc-fa30-7740-8fd2-9cdfab6e6c46
ROUTE=W4-R04 UB-BANK-XR-LAYOUT-X
EVENT_CLASS=HARDWARE_DOC_AND_SOURCE_RESEARCH
STATUS=ROUTE_REVIEW_REQUIRED
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
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=Main/Planning 复核；R05 复用共同资料；后续 R04 先做 Parent-only 指令归因，不重复旧 padding
```

本次只新增本目录中的研究脚本、输出、资料图与本说明，未改 Candidate、规则、共享 TSV、Dashboard、其他工作树或服务器文件。所有前台查询与离线验证已结束。具体提交及最终工作树状态由完成回执给出。

网页读取限制：内置浏览器的打开请求及一次状态查询均超时，临时标签页的创建/关闭状态未获确认。官方正文由网页检索服务读取，原图直接下载后核对；未启用其他浏览器。
