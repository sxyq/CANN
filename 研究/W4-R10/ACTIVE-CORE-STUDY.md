# W4-R10 active-core 研究

本轮只持有 `w4/r10-active-core-d-aware-x` 和对应 R10 工作树。
V001 既有事件已恢复于 `本地实验/W4-R10/V001/VERSION-RECORD-EVENT-RECOVERED.md`，
恢复提交 `b1459725d35842acd616668570d345b73465c1e8`。V001 不计入本轮新增版本。

## 已读取的历史范围

| 来源 | 实际覆盖 | 与当前想法的关系 |
|---|---|---|
| `w3/m1/adaptive-core-ownership`，`6321ad44` | V001–V040 的 Parent/Candidate 差异 | V001 为 FP32 D≤2048 的两行上限；V002–V040 为 owner 排列与余行归属，均不复做 |
| `w3/m1/multirow-panel-rms`，`ce6c6dc5` | V001–V031 host launch；提交说明与 V031 runner/reference | 全部 host launch 与 R31B V011 相同；已有 128×12288 几何样例 |
| `w3/m1/crossrow-full-pipeline`，`1efa0863` | V001–V028 host launch 与单版源码差异 | 全部 host launch 不变；V009 改核内 FP16 batchLimit，不是本轮变量 |
| R031 reconstruction D001–D004 | 自有历史副本中的 launch 与多核分配 | min(core,rowCount)；按每核行数选 kernel mode，没有多批次缩核策略 |
| R31A、R31B、MIX、STORE、SCHED | 自有历史源码中的 host launch，加全版本记录中的后续变化说明 | D-slice、row-group、核内流水及 store 方向，与当前 host 缩核不同 |
| 现有 W4 本地 refs | 每条 Route 自有前缀下已提交 Candidate 路径 | R01、R08、R11 host 不变；R10 V001 为单批驻留上限；其他 ref 未找到对应 Candidate，不据此推断未提交状态 |
| W4 R06、R15 研究 | 本地 Git 对象中的重复性报告 | input issue-order 与 multirow DMA，不覆盖本轮策略 |

其他分支只通过本工作树执行 `git show` / `git ls-tree` 读取，没有进入它们的实际工作树。

## 想法 A：单批驻留行数上限

```text
DUPLICATE_AUDIT
MECHANISM=D>8192 时按 M/residentRows 限制 blockCount
SEARCHED_HISTORY=W4-R10 V001 source、diff.patch、local-result.json
MATCH_FOUND=YES
WHY_NEW_OR_DUPLICATE=V001 已实测；BF16 48x12288 为 40→24；已有负结果和不稳定样本
ROUTE_RESEARCH_EVENT=R10-DUP-RESIDENT-CAP
NEW_PERFORMANCE_REVISION=NO
NEXT_ACTION=只研究多批次场景下保持最大行数的缩核
```

## 想法 B：宽 BF16 多批次完整批缩核

```text
DUPLICATE_AUDIT
MECHANISM=保持 Parent 每核最大行数 Q，在 Q 为完整驻留批倍数且至少两批时，用 M/Q 个核消除不满批
SEARCHED_HISTORY=上表范围；重点 W3 R2 V001–V040、R4 V001–V031、R5 V001–V028、R10 V001
MATCH_FOUND=NO
WHY_NEW_OR_DUPLICATE=V001 在 M=128、D=12288 BF16 时仍为 40 核；本策略预计 40→32，Q=4 保持不变，且不作用于 V001 的单批目标 M=48
```

定义：`C=min(availableCoreNum,M)`；`R(D)` 沿用 Parent BF16 的
`ChooseWideFullYRows(D,4,4,2,2)`；`Q=ceil(M/C)`。
只在 BF16、D>8192、R>1、Q>R、Q%R=0、M%Q=0 时采用 `blockCount=M/Q`。
其余输入保持 Parent。没有新的 owner 计算，没有 tile、buffer、算术、event 或 store 改动。

源码依据：`本地实验/W4-R10/V001/parent.asc` 的 `ChooseWideFullYRows`、
`ProcessWideLowPrecision`、`run_kernel`。参数 gamma/bias 在每个驻留批的每个 tile
读取一次。在 40 个 vector cores、M=128、D=12288 BF16 下，R=2：

- Parent：8 核各 4 行、32 核各 3 行，合计 80 个驻留批。
- Candidate 预测：32 核各 4 行，合计 64 个驻留批。
- 每核最大行数同为 4；tileWidth=4096，3 个 tile；参数 DMA 命令总数预计 480→384。
- V001：min(40,128/2)=40，该点没有发生缩核。

这只是可证伪预测。是否获益以完整 reference 验证后的交错 Local 数据为准。

## 目标来源与精度范围

128×12288 几何来自 W3 R4 的 `R4_MULTIROW_PROXY_128XWIDTH_SEED0`，
同一 runner 明确支持 `fp32/fp16/bf16`。BF16 domain 来自现有算子描述
`归档/phase3-before-reset-20260920/源码/add_rms_norm_bias.json`，以及 R10 V001
48×12288 BF16 的双方 `bad=0`。128×12288 BF16 是有来源的本地组合探针，
此前 R10 没有该点精度结果；不声称它是任何隐藏测试。

在 Candidate 编辑前先使用 R10 原有 Parent 二进制验证该组合。若 reference 失败，
不以它接受或测量 Candidate。新版本只改变 BF16；旧 FP16 与宽 FP32 reference
失败不在性能结论内，也不宣称已经解决。

对照使用原有 48×12288 BF16（只有一个最大驻留批，因此策略不触发）、
12×8192 FP32（短 D）、1×32768 BF16（单行、单驻留行）。所有形状均有已有来源。

## 本次必要探测

2026-10-08 03:41:28–03:41:37 UTC，复用远端 R10 V001 已有
`build/w4r10_ref_parent_probe`，没有重新构建或修改 Parent。

```text
HOST=hwnput3
USER=lelinfeng
DEVICE=2
TOOLCHAIN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
COMMAND=V001/build/w4r10_ref_parent_probe 2 128 12288 2 <prefix> 0 1 1 0
SHAPE=128x12288
DTYPE=BF16
PARENT=R31B V011
PARENT_REFERENCE_RC=0
BAD=0
MAX_ABS=0.015625
REFERENCE=V001/runner_ref.inc CPU FP32 sum(x+residual)^2、RMS、gamma、bias，最后转 BF16
ATOL=0.02
RTOL=0.02
EPSILON=1e-5
WARMUP=0
SAMPLES=1
BLOCKS=1
BATCH_N=1
FREE_HBM_MB=60948
PRE_DEVICE_LOAD=AICore 0%, AIVector 0%
POST_DEVICE_LOAD=AICore 2%, AIVector 3%
HOST_LOAD_PRE=92.69/61.19/52.82
HOST_LOAD_POST=85.08/60.59/52.72
```

原 runner 会附带一条首次运行计时：device `142002 us`，wall `142274 us`。
这里没有 warmup，也没有同二进制稳定性样本；该值只保留为探测原始输出，
不作为 Local score、性能方向或速度依据。本次只支持一个固定输入的 Parent
reference 通过，尚未验证 Candidate，也不推出所有 BF16 输入均正确。

证据：`parent-domain-probe.log`、
`parent-domain/parent-r128-d12288-bf16-stats.txt`、
`parent-domain/parent-r128-d12288-bf16-raw.tsv`。
远端证据位置：
`/home/data4t2/lelinfeng/cann/w4/R10-active-core-d-aware-x/research/multibatch-parent-domain-20261008/`。

辅助头文件检索首次使用远端 `rg`，返回 `command not found`；该操作发生在
Parent 探测成功之后。随后用 `awk` 读取 SDK 头文件，确认
`ACL_DEV_ATTR_VECTOR_CORE_NUM=201`，再调用 SDK 实测设备 2 的
`availableCoreNum=40`；`aclInit`、`aclrtGetDeviceInfo`、`aclFinalize` 均返回 0。
证据见 `device2-core-info.log`。没有为此改服务器配置或安装工具。

## 本轮研究事件与交接

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=R10-RESEARCH-MULTIBATCH-CAP-20261008
ROUTE=W4-R10
STATUS=NEW_HYPOTHESIS_READY_FOR_SINGLE_CHANGE
DIRECT_PARENT=R31B V011
MECHANISM=BF16 宽 D、多完整驻留批，在保持每核最大行数 Q 的条件下将 blockCount 设为 M/Q
DUPLICATE_MATCH=NO_IN_READ_HISTORY
SOURCE_PREDICTION=128x12288 BF16; residentRows=2; 40→32 cores; maxRows=4; 80→64 resident batches; 480→384 parameter DMA commands
PREDICTION_KIND=STATIC_SOURCE_DERIVATION_ONLY
PARENT_REFERENCE=PASS; bad=0; max_abs=0.015625
CANDIDATE_REFERENCE=NOT_RUN
DEVICE_ID=2
AVAILABLE_CORE_NUM=40
FREE_HBM_MB=60948
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=R31B V011
LAST_KNOWN_REVISION=V001
NEW_PERFORMANCE_REVISIONS=0
V002_CREATED=NO
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BRANCH=w4/r10-active-core-d-aware-x
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=Main 后续安排 R10 时重读规则，声明 V002，以 R31B V011 为 Parent，仅实施本文件想法 B 的 host blockCount 选择；紧接 server3 Compile、reference Correctness、Parent same-binary、交错 Local、结果提交和版本事件
```

同一研究文件中的 `R10-DUP-RESIDENT-CAP` 事件确认单批上限与 V001 重复，
未创建版本。V001 恢复事件见前述原有实验目录，新增研究不另造版本事件。

用户最新滚动调度要求允许在首个带必要探测的主要研究结论后返回。本轮在此交接，
保留上述尚未测试的独立轴；不改变路线生命周期，不切换路线、不新建 Child。
下一次性能版本的稳定性和 P/C 数据仍需实际取得；本轮没有任何改善结论。
