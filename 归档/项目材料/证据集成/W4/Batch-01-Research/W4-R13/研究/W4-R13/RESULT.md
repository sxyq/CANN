# W4-R13：参数暂存区复用的单点诊断

## 本次结果：DIAG-PARAM-REUSE-01

2026-10-08，本次 fresh context 已完成真实源码修改、server3 构建和五次精度诊断。
唯一变化为：在 `ProcessWideFp32FullCacheRows` 中，仅当 `tile>0`，在参数
`Load` 前调用现有 `SyncVToMTE2()`。两个失败输入在同一设备上的 Parent 均失败，
诊断版均通过；未变化的 `1x8192` 控制项也通过，且与此前保存的 Parent 输出逐位相同。
这支持参数消费与下一次 MTE2 覆盖之间缺少顺序是本次故障的原因。

本次属于 `RESEARCH_EXECUTED_VARIANT`，确实编辑并执行了 kernel；没有建立性能版，
没有 Local 成绩，也没有正式提交。该依赖在早期 R31B 已经存在，本次不把它登记为新的
性能机制。性能版本下一可用编号仍为 V001，未使用。

身份：`AGENT_ID=01a119e0-8718-7fe3-921a-3edab7743bcb`，SLOT-2；
分支 `w4/r13-wide-fp32-cache-tail-x`；工作树
`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R13-wide-fp32-cache-tail-x`；
接手提交 `91e891a2a266ecb2b870afead3c3aa0fd94b67ac`，接手时无未提交改动。
当前结果的提交号以本文件所属提交及最终回执为准。

## producer → consumer → reuse

以下行号指未改动的 `线上结果/R31B/V011/submission.asc`。

| 阶段 | 对象、位置与顺序 |
|---|---|
| 实际分派 | D>8192、FP32：Init L69–80 分配缓存；Process L163–169、L1847–1853 调用 FullCacheRows。1x8192 走窄行控制路径。 |
| pass1 / 倒数 | xBuf_/residualBuf_ 先存输入，再作平方与归约工作区；L2135、L2157 的 V→MTE2 同步覆盖这些阶段。reduction 和行倒数计算原样保留。 |
| 参数生产 | L2179–2180 的 gammaLocal/biasLocal 分别取自 xBuf_/residualBuf_；L2191–2192 的 Load 以 MTE2 写这两个区。 |
| 参数就绪 | L2193 的 MTE2_V 保证参数写入先于本 tile 的 Vector 使用。 |
| 参数消费 | L2203 的 Mul 读取 gamma，L2205 的 Add 读取 bias；同一参数 tile 供本批全部行使用。 |
| 下一次覆盖 | 下一次 tile 迭代仍向同一个 xBuf_/residualBuf_ 起点加载 gamma/bias；需要前述 Vector 读取先完成。 |
| 现有输出等待 | L2184/L2188 为 MTE3_V；L2224–2226 的 Store 读取 valueLocal，不读取参数暂存区。这个等待没有表达 MTE2 必须等待参数消费的关系。 |
| 本次干预 | 在原 L2191 前增加四行，仅对 tile>0 调用 SyncVToMTE2；第一 tile 仍使用原 L2157 的顺序。输出等待与两层行循环全部保留。 |

```mermaid
flowchart LR
    P["MTE2 写入参数 tile t"] -->|MTE2_V| V["Vector 对全部行执行 Mul/Add"]
    V -->|V_MTE3| S["MTE3 从 valueLocal 写出"]
    V -->|本次补入 V_MTE2| N["MTE2 覆盖为参数 tile t+1"]
```

`SyncVToMTE2` 在 L3415–3419 已定义为同一 V_MTE2 事件的 Set/Wait。
本循环内没有仍待消费的 V_MTE2 分配事件；输出队列使用 V_MTE3/MTE3_V，保持原样。
server3 SDK `impl/kernel_tpipe_impl.h` L665–673 的 `ReadSpmBuffer` 也在 MTE2
覆盖 UB 前使用 V_MTE2，再用 MTE2_V 连接随后的计算。对应只读节选保存在
`parent-tail-probe/evidence/param-reuse-01/api-evidence.txt`。

源码中的旧注释把输出称作仍引用参数暂存区，与 Store 的实际 valueLocal 参数不符。
本次以具体读写位置和事件方向作判断，没有按注释扩大改动，也没有改用通用全流水等待。

## DUPLICATE_AUDIT：已有依赖，新因果证据

`MECHANISM=parameter staging consume-before-overwrite`。

| 已读历史 | 本轴结论 |
|---|---|
| 本 Route / 91e891a2 | 只有 Parent 探测；没有性能版本或本次诊断源码。 |
| W3 R2 / 6321ad44，V001–V040 | 40 份目标函数的代码与 Parent 相同；未发现本次位置的依赖变化。 |
| W3 R4 / ce6c6dc5，V001–V031 | 31 份目标函数代码相同；旧表 V013 不作为末版。 |
| W3 R5 / 1efa0863，V001–V028 | 28 份目标函数代码相同。 |
| R31B V002–V005 | 同一 FullCacheRows 的每个参数 tile 结束处已有 SyncVToMTE2。 |
| R31B V006–V015 | V006 输出队列变化删除了旧的每 tile SyncVToMTE2；V011 沿用这一顺序。V009/V012 有额外输入或参数双槽结构，未作为本次单点版本复用。 |
| R31B V016/V017 | 本工作树保存的 FullCacheRows 代码与 V011 相同。 |
| R031、R31A、MIX | 已读旧源码没有当前 FullCacheRows 实现；R31A V026 的 CachedRows 为 gamma/bias 分别释放后预取下一 tile，机制已有，不在本次重做。 |
| STORE V001–V003 | 已有整行或分块写出；没有本次 V011 参数加载前的单点恢复。 |
| EPILOGUE-ARITH V001/V002 | 已有整行缩放提前执行、gamma-first Axpy；本次没有改变这些算术因素。EPILOGUE-FUSE 只取得旧机制记录，未宣称完整覆盖其源码。 |
| W4 R01/R02/R08/R09/R10/R11 | 已提交 Candidate 的 FullCacheRows 代码相同；R09 96044629 的真实变化位于另一 FP16 路径 L3346。 |
| R05 / 61e0aa28 | 已读 PARAM-OUTPUT-LAYOUT-STUDY.md；确认同一对缓冲的真实参数消费者，没有把它当成因果实测。 |

上述跨分支源码均通过本工作树的 Git 对象读取；旧 R031/R31A/R31B、MIX、STORE
材料来自本工作树的归档。V005→V006 的函数差异明确显示依赖被删除。
因此 `MATCH_FOUND=YES_HISTORICAL_DEPENDENCY`；`WHY_NEW_OR_DUPLICATE` 是依赖本身
已有历史，本次增加当前 V011 的四行隔离干预及同设备 reference 对照。
`NEW_PERFORMANCE_REVISION=NO`。

## 构建、独立 reference 与实际输出

复用 `parent-tail-probe` 的现有 CMake、runner_ref.inc、local_types.h 和构建脚本，
没有改变输入生成、reference、容差、tile、ownership、分配、reduction 或输出形式。
新增目标 `w4r13_ref_reuse_probe` 使用保存的完整 `param_reuse_source.asc`。
对 Parent 的源码差异仅为上述四行。CMake 入口此次同时重建了 Parent 目标；旧输出、
旧构建失败和旧成功记录全部保留，未删除构建目录。

Compile：2026-10-08 05:19:22–05:19:47 UTC，退出码 0，两个目标均完成链接。
有原 runner 的 GM_ADDR 属性及 printf 宽度格式警告，没有本次构建失败。
日志为 `parent-tail-probe/evidence/param-reuse-01/compile-01.log`。

Correctness：2026-10-08 05:20:31–05:20:58 UTC。顺序为 Parent 9216 → 诊断 9216
→ Parent 10240 → 诊断 10240 → 诊断 8192。增加两次当前 Parent 运行，用于同设备
因果对照；此前三组输出和分析直接复用，没有扩展输入矩阵。

每个输出均独立比较原 runner 的 FP32 reference 与复用分析函数的 FP64 reference，
逐元素要求 `abs_error <= 1e-4 + 1e-4*abs(reference)`。所有输出的非有限值数量为 0。

| FP32 shape | 本次 Parent 超差：前8192 / 尾部 | 诊断超差：前8192 / 尾部 | 诊断最大绝对误差，FP64 reference | 原 runner 返回码：Parent / 诊断 |
|---|---:|---:|---:|---|
| 1x8192 | 复用旧结果：0 / 无尾部 | 0 / 无尾部 | 5.01749850e-7 | 未重跑 / 0 |
| 1x9216 | 5119 / 256 | 0 / 0 | 4.62491319e-7 | 3 / 0 |
| 1x10240 | 7166 / 1024 | 0 / 0 | 5.51814769e-7 | 3 / 0 |

原 FP32 runner 的诊断最大绝对误差依次为 1.66893e-6、3.33786e-6、4.76837e-6，
三项也全部通过。另一份先将 x+residual 舍入到 FP32 的参考计算仍为小误差。
`1x8192` 的完整诊断输出与旧 Parent 输出逐位相同。

本次 Parent 的尾部也出现错误，旧运行的尾部超差数为 0；旧数字没有被覆盖。
这说明错误位置随执行条件变化，不能把“只错前8192”当成固定性质，也不能把
末 tile 参数误用模型当成所有错误元素的唯一解释。四行干预在本次三组输入上消除了
超差；未据此宣称其他宽度、其他 dtype、完整多行或多批范围都已正确。

## 设备与计时边界

SSH=`cann-server3`，host=`hwnput3`，user=`lelinfeng`，device=1；构建 SoC 为
Ascend910B3 / dav-2201，CANN 8.5.0.alpha002。远端仍使用
`/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/`。
本进程沿用 `/usr/lib/aarch64-linux-gnu/libstdc++.so.6` 的 LD_PRELOAD；没有改服务器库或服务。
运行前后 FREE_HBM 为 52428–56360 MB，AICore 为 0–15%，AIVector 为 0–11%，
load1 为 54.87–73.35。负载只作运行背景，没有等待独占设备。

参数均为 rows=1、dtype=0、warmup=0、samples=1、blocks=1、gap_sec=0、batch_n=1。
这里 blocks 是 runner 采样块数；实际 kernel blockCount 由现有 run_kernel 按一行输入得到 1。
单次启动的 device_us / host_wall_us 原样保留：

| 运行 | device_us | host_wall_us |
|---|---:|---:|
| Parent 1x9216 | 109399 | 109688 |
| 诊断 1x9216 | 138885 | 139279 |
| Parent 1x10240 | 134818 | 135110 |
| 诊断 1x10240 | 115326 | 115613 |
| 诊断 1x8192 | 113752 | 114063 |

这些数值含首次启动条件，每项只有一个样本。未执行 Parent same-binary 稳定性或正式
交错性能采样；两个宽行 Parent 对 reference 失败，控制项不经过变化位置。因此
`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`CURRENT_LOCAL_BEST=NONE`，不接受性能提升。
本次新增性能版 0、有效 Local 0、连续有效无改善贡献 0。

## 本次文件、事件与停止位置

实际修改均限于本工作树 `研究/W4-R13/`：本 RESULT.md、parent-tail-probe/CMakeLists.txt；
新增 param_reuse_source.asc、runner_ref_reuse.asc、run_param_reuse_server3.sh、
analyze_param_reuse.py，以及 evidence/param-reuse-01 下完整输出、前后设备状态、
命令与返回码、原始计时、编译日志、SDK 节选和 reference-analysis.json。
五个二进制输出包含 Parent 失败样本；没有删除或覆盖旧证据。

事件：`W4-R13-PARAM-REUSE-20261008`；实际研究变体 `DIAG-PARAM-REUSE-01`。
发布 ROUTE_RESEARCH_EVENT、ROUTE_EVENT 与明确标注研究类型的 VERSION_RECORD_EVENT，
不登记为 V001 性能版。`OFFICIAL_SCORE=NONE`、`ONLINE_STATE=PAUSED`、`PUSH=NO`。
接手时读取的 main@285e7b4c 已登记本 Route 及上次研究；下方旧 STATE_SYNC_GAP
只保留为旧回执，本次事件等待 Record 同步。共享记录、规则和 Dashboard 未修改。

05:21:59 UTC 的 final-process-state.txt 记录两个目标均已生成，pgrep 返回 1，
`RUNNING_DEVICE_OPERATION=NONE`。所有五次运行和构建命令都已结束；没有定时或后台循环。

本次最小因果诊断已完成。剩余同轴动作是先证明最后参数 tile 到下一 batch pass1 的
xBuf_/residualBuf_ 再使用顺序（Parent L2235–2245 → L2125–2126），再由后续同 Route
任务选择一个有来源的最小多批输入。本次只有一行，不能验证该边界；不继续扩展矩阵。
D40000 partial 容量仍是未处理的另一条轴，reduction、tile 和 ownership 均未改动。
交还槽位由 Main 处理；没有关闭、合并或替换 Route。

---

## 之前的 Parent 尾部探测（91e891a2，原记录）

本次占槽完成研究结论，未创建性能 Candidate 或性能版本。`1×9216`、`1×10240` FP32 的 Parent 均未通过 reference；两个尾部区都通过，错误出现在前面的整 tile。当前证据支持继续研究 gamma/bias 暂存区的复用顺序，不能宣布性能提升，也不能据此认定所有宽行输入均有问题。

## 实测结果

Parent 为本树 `线上结果/R31B/V011/submission.asc`，内容未修改；与 `w4/r10-active-core-d-aware-x:本地实验/W4-R10/V001/parent.asc` 的文本对比无差异。实际执行位置为 server3 `/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/`。

| FP32 shape | 整 tile 超差数 | 尾部超差数 | 整行最大绝对误差（双精度 reference） | 结果 |
|---|---:|---:|---:|---|
| 1×8192 | 0 / 8192 | 无尾部 | 5.01750e-7 | PASS，控制项 |
| 1×9216 | 5631 / 8192 | 0 / 1024 | 1.20398729 | FAIL |
| 1×10240 | 7165 / 8192 | 0 / 2048 | 1.10208545 | FAIL |

容差为现有 runner 的 `atol=1e-4, rtol=1e-4`。原 runner 的 FP32 reference 与独立双精度 reference 得到相同超差数量；另一份先把 `x+residual` 舍入为 FP32 的参考计算仍有相同量级误差。两个尾部区最大绝对误差分别为 `3.36605e-7`、`5.51815e-7`，未发现非有限值。

这些是本轮新实测的研究尺寸，不是 Official shape。执行参数均为 `device=4, rows=1, dtype=0, warmup=0, samples=1, blocks=1, gap_sec=0, batch_n=1`。计时处于首次启动条件，原始值保留作诊断，不能作为稳定性结论或 Local 成绩。

硬件与环境：`hwnput3`、`lelinfeng`、Ascend910B3 / dav-2201、CANN `8.5.0.alpha002`。三次执行前 `FREE_HBM_MB=6553`，HBM 使用率 90%，AICore/AIVector 快照均为 0%；系统 load1 约 59.75–64.23。完整前后快照、命令、退出码和原始输出在 `parent-tail-probe/evidence/`。

## 新的可证伪线索

输出数据可部分由后一个参数 tile 解释：

| shape / 元素索引 | 实际值 | 正确 reference | 改用末 tile 对应 gamma/bias 后的值 | 与实际值的差 |
|---|---:|---:|---:|---:|
| 1×9216 / 700 | 2.78834248 | 1.58435519 | 2.78834255，参数列 8892 | 7.25087e-8 |
| 1×10240 / 1400 | 2.52721858 | 1.42513313 | 2.52721837，参数列 9592 | 2.05688e-7 |

这支持参数暂存区提前被下一次 MTE2 写入的假设，尚未通过单变化实验确认因果。

精确源码位置均在 Parent：

- `:2179`、`:2180`：gamma/bias 使用 `xBuf_`、`residualBuf_`。
- `:2183`、`:2187`：现有等待为 `MTE3_V`。
- `:2191`、`:2192`：同一暂存区加载下一 tile 的 gamma/bias。
- `:2203`、`:2205`：Vector 仍以这些暂存区作为乘加输入。
- `:2128`、`:2224`：FP32 直接把值写入保留区，并从保留区输出；该路径没有可直接删除的独立尾部拷贝。

`D=40000` 仅作源码推算：`:1293` 的容量选择给出 tile=2048、缓存行数=1，从而需要 20 个 partial；`:79` 只分配每行 16 个位置，`:2133` 以 tile 编号写入。该输入的目标支持范围未确认，本轮未运行。`D=36736` 超出现有原样 runner 的宽度上限 32768，本轮未运行。未取得设备端字段快照；tile 和缓存行数属于源码推导值。

## DUPLICATE_AUDIT

`MECHANISM=unchanged Parent tail-path/reference probe`。

`SEARCHED_HISTORY`：本分支无已提交 R13 实验；读取 `w3/m1/record-owner:调度/当前任务.tsv` 的 R13 行；W3 R2 至 V040（`6321ad44`）、R4 至 V031（`ce6c6dc5`）、R5 至 V028（`1efa0863`）的版本历史及相关结果；R031 集成记录、R31A/R31B、MIX、STORE/EPILOGUE 的相关记录；`m2/reduce-hier` 的 V001–V005；相关 W4 入口只通过 Git 对象读取。

`MATCH_FOUND`：partial-sum lifetime 已有 REDUCE-HIER-X V001，其他归约变体见 V002–V005；整行归一化提前执行已有 EPILOGUE-ARITH V001，合并行内写出已有 STORE-H2B。未把这些机制重新包装为本轮性能版本。旧记录对 staging initialization 的笼统描述没有在本轮转化为新 Candidate。

`WHY_NEW_OR_DUPLICATE`：旧 R13 行明确列出两个新尾部尺寸尚未实测；本轮增加了实际运行输出、整 tile/尾部分区误差及末 tile 参数误用模型。没有重测既有 `1×16384/1×32768` 失败项，没有重新展开归约结构。

## 构建与证据入口

当前唯一构建目标为 `w4r13_ref_parent_probe`，使用 `CMakeLists.txt`、`runner_ref_parent.asc`、`runner_ref.inc`、`local_types.h` 和远端未改动的 `parent_source.asc`。来源为 R10 V001 的完整 ASC executable 链路。对 runner 仅增加末次输出的二进制保存，没有改输入、reference、容差或 Parent 算法。

`compile-01.log` 至 `compile-05.log` 保留失败输出，`compile-06.log` 为成功构建。首次启动因 HCC 自带 libstdc++ 缺少 `GLIBCXX_3.4.29` 而未进入 NPU；失败日志保留。成功运行仅为本进程指定系统 `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`，未修改服务器库或服务。

`abi.h`、`parent_kernel.asc`、`path_kernel.asc`、`runner.cpp` 是失败包装的保留材料，不在当前 CMake 目标中，也没有作为设备探测结果使用。

`analyze_outputs.py` 复用已保存输出生成 `evidence/reference-analysis.json`。三个 `*-output.bin` 保留完整输出，`*-raw.tsv` 保留原 runner 的全部诊断计时。原仓库忽略 `.bin`，因此只对这三个明确文件作显式暂存，不改忽略规则。

## 事件与交接

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R13-PARENT-TAIL-20261008
ROUTE=W4-R13
REVISION=NONE
DIRECT_PARENT=R31B V011
PERFORMANCE_REVISIONS_ADDED=0
COMPILE=PASS (compile-06)
CORRECTNESS=FAIL (1x9216, 1x10240); CONTROL=PASS (1x8192)
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
STAGNATION_COUNTER=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BRANCH=w4/r13-wide-fp32-cache-tail-x
STATUS=RESEARCH_RESULT_READY
RUNNING_DEVICE_OPERATION=NONE
VERSION_RECORD_EVENT=NONE (no performance revision)
```

`STATE_SYNC_GAP`：旧调度行使用 V001 只读分析标签；本轮接管分支没有对应性能提交。本次继续按研究事件记录，不补造 V001 性能事实。最终 commit 与提交后状态以同轮回执为准。

下一动作：先与 W4-R09 最新事实核对 `:2191` 前对 gamma/bias 暂存区复用的 `V_MTE2` 依赖是否已实测；若未覆盖，在同一 ASC 探测链路内只做这一处正确性诊断，分别报告 Parent 和诊断版本对 reference 的结果。该动作不更改归约、tile 宽度、所有权或输出形式。当前证据尚不足以认定该处同步就是唯一根因；精度可用前不采 Candidate 性能。

本轮只修改 `研究/W4-R13/`；Parent、共享 TSV、规则、Dashboard、其他 Route 工作树及服务均未修改。远端所有本轮设备命令已结束，未创建持久后台任务。

---

## 2026-10-08 本次续接：BATCH-REUSE-01

本次已完成 batch 边界的单变化精度诊断。现有 `DIAG-PARAM-REUSE-01` 的
`tile>0` 等待覆盖同一 batch 的参数 tile 复用，最后参数 tile 到下一 batch
首笔输入 MTE2 的顺序仍有缺口。在该边界增加一次 `SyncVToMTE2()` 后，
FP32 `121x9216` 对原 FP32 reference 与独立 FP64 reference 均通过。
本次继续沿用原研究 Revision，新增性能版 0，Local 为 NONE。

接手为 SLOT-4、设备 3，分支及工作树沿用本文件上方的 R13 身份；接手 HEAD 为
`a10cf2a1e5c8f25aa256ce9dbfb79cd3347d12b4`，接手时无未提交内容。
当前 agent_id 未由本次调用上下文提供，交接使用 Main 持有的既有 agent_id，
不借用此前已关闭 Agent 的编号。
规则从本工作树及 `9f918955` 的九个指定入口读取；同版编辑前再次完整读取
该提交的 AGENTS、Route Skill、执行约定并发布回执。精度、精度标准和资料检索
Skill 使用已安装的 `ops-direct-invoke` 版本；保留缓存、失败证据和旧二进制。
共享状态读取 `main@07662d7b` 的三份表，已确认原研究与实际诊断均已登记。
本节新事件等待 Record，同步前不改变原行的事实。

### 最小双 batch 几何

宽度 9216 来自本路线已验证的单行诊断，行数由当前源码推导。
设备 3 的 `aclrtGetDeviceInfo(3, ACL_DEV_ATTR_VECTOR_CORE_NUM, ...)` 实测为 40，
与未改动 runner 使用同一属性。没有人为减少核心数，没有改变 launch 或 Tiling。

`ChooseWideFullYRows` 的 FP32 条件为 `4*D*N + 8*T + 64*N <= 176*1024`。
`D=9216,T=4096,N=3` 需要 143552 字节；N=4 需要 180480 字节，超过 180224
字节预算。因此缓存 3 行，最少 `M=3*40+1=121` 即可使一个 block 出现第二批。

| 参数 | 本次值及来源 |
|---|---|
| M / D / dtype | 121 / 9216 / FP32，研究输入，未引用 Official testcase |
| availableCoreNum / blockCount | 40 / 40；前者来自 ACL 查询，后者为 `min(availableCoreNum,M)` |
| baseRows / extraRows | 3 / 1 |
| block 0 | beginRow=0，localRows=4；batchBegin=0、3，batchRows=3、1 |
| block 1 至 39 | beginRow=`3*blockIdx+1`，localRows=3；仅一批，提供同次运行内的对照 |
| batchLimit / tileWidth / tileCount | 3 / 4096 / 3 |
| 每行有效 tile | 4096、4096、1024；末 tile 从列 8192 开始 |
| runner 参数 | device=3，rows=121，width=9216，dtype=0，warmup=0，samples=1，blocks=1，gap_sec=0，batch_n=1 |

runner 的 `blocks=1` 是采样分组，`batch_n=1` 是一次计时中的 launch 数；它们均不代表
kernel 的 blockCount 或行批数。几何来自源码和 ACL 属性，未插入设备字段输出。
原始属性查询、计算式及 block 0/1/39 的结果保存于
`parent-tail-probe/evidence/batch-reuse-01/runtime-geometry.txt`。

### 实际视图与顺序

下表行号均指未改动的 `线上结果/R31B/V011/submission.asc`。
视图区间使用各缓冲自身起点的偏移，未宣称取得绝对 UB 地址。

| 对象 | 最后读取与下一次覆盖 |
|---|---|
| xBuf_，16384 字节 | `gammaLocal=xBuf_.Get<float>()`（2179）；最后 tile 的 1024 个参数占 `[0,4096)` 字节。最后消费者为 batchRow=2 的 `Mul`（2203）。下一 batch 的 `xLocal=xBuf_.Get<float>()`（2121）被首个 `Load`（2125）覆盖 `[0,16384)` 字节。 |
| residualBuf_，16384 字节 | `biasLocal=residualBuf_.Get<float>()`（2180）；相同 `[0,4096)` 字节由 batchRow=2 的 `Add`（2205）最后读取。下一 batch 的 residual `Load`（2126）覆盖 `[0,16384)` 字节。 |
| valueFp32Buf_，110592 字节 | 保存本批 3 行的 x+residual 与最终结果。Store（2224–2226）读取该缓存，与参数区分配独立。下一 batch 的 Vector Add（2128）才重写相应缓存。 |
| reduceFp32Buf_，192 字节 | 沿用 3 行、每行 16 个 FP32 partial；本次未改变布局、partial 数量或归约。 |

对 block 0，下一批第一行的 GM offset 为 `3*9216=27648`，首 tile 从
xGm/residualGm 的 `[27648,31744)` 元素区间写入上述两个暂存区。
这两笔 MTE2 与上一批最后的 gamma/bias Vector 读取使用重叠空间。

现有顺序分为四段：

- pass-1 每 tile 的 `SyncVToMTE2`（2135）覆盖平方/归约到下一输入 tile 的复用；
  它位于当前 tile 的 Load 之后，无法保护进入新 batch 时已经发出的第一笔 Load。
- 倒数阶段后的 `SyncVToMTE2`（2157）覆盖当前 batch 的工作区到第一个参数 tile。
- 既有诊断仅在参数 `tile>0` 的 Load 前增加等待，覆盖参数 tile 0→1、1→2；
  最后 tile 之后没有下一次参数循环，因此该条件不能覆盖 batch 0→1。
- batch 末尾的 `MTE3_V` 等待（2235–2240）连接输出 MTE3 与后续 Vector，保护
  valueLocal 的写出与重用。下一笔 MTE2 不在这个目标流水上。
  SDK `TPipe::ReleaseEventID` 只修改 `eventOccupy`，没有补入等待，见
  `batch-reuse-01/sdk-ordering.txt` 的 SDK 450–459 行。

```mermaid
flowchart LR
    V["末 tile Mul/Add：读 gamma/bias"] -->|V_MTE3| O["Store：读 valueLocal"]
    O -->|MTE3_V| N["下一批 Vector 写 valueLocal"]
    V -->|本次 V_MTE2| I["MTE2：x/residual 覆盖参数区"]
    I -->|MTE2_V| N
```

本次在 `param_reuse_source.asc:2103` 增加一个可选诊断块：仅当
`batchBegin>beginRow` 时调用 `SyncVToMTE2()`。`runner_ref_batch_reuse.asc`
定义 `W4_R13_BATCH_REUSE_SYNC` 后复用原 reference runner；旧诊断入口不定义该符号。
相对 a10cf2a1 的 kernel 源码只增加这六行，原每 tile 同步保持不动。
没有改变 reduction、D40000 容量、UB 布局、参数 DMA 粒度、tile、核心数或 FP16 路径。

### DUPLICATE_AUDIT 与诊断身份

`MECHANISM=last parameter consumer before next batch input overwrite`。
`SEARCHED_HISTORY` 复用本文件上方 a10cf2a1 的已完成审计：本 Route、W3 R2 至 V040
（6321ad44）、R4 至 V031（ce6c6dc5）、R5 至 V028（1efa0863）、R031/R31A/R31B、
MIX、STORE/EPILOGUE 和相关 W4。三个 W3 refs 本轮仍指向这些提交，未重跑或重扫全部版本。

本轮补读 `归档/历史工作区/R31B/R31B-V005-NARROW-DEEP-BATCH-FIXED_kernel.asc`
2245–2263 行及 `R31B-V006-MTE3-QUEUE-DEPTH_kernel.asc` 2089–2112 行。
V005 的 2261 行在每个参数 tile 末尾执行 `SyncVToMTE2`，也覆盖最后 tile 到下一批；
V006 删除该等待，改为输出事件队列。`MATCH_FOUND=YES_HISTORICAL_DEPENDENCY`。
`WHY_NEW_OR_DUPLICATE`：依赖有历史，本次补充当前 V011 派生诊断在实际双批输入上的
单点因果证据，不把它登记为新的性能机制。

`REVISION=DIAG-PARAM-REUSE-01`，本次 `VARIANT=BATCH-BOUNDARY-SYNC`；
直接来源是 a10cf2a1 的 `param_reuse_source.asc`，其 kernel 上游为 R31B-V011。
这三个执行对象为 Parent、旧 tile-sync 诊断、新 batch-sync 诊断，分别独立比较 reference。

### Compile、reference 与原始输出

先复用两个已编译对象，各执行一次 `121x9216`；随后只构建新目标：

```bash
bash build_server3.sh --target w4r13_ref_batch_reuse_probe
```

当前工具链仍为 CANN 8.5.0.alpha002、Ascend910B3 / dav-2201。
2026-10-08 07:01:05–07:01:28 UTC 构建成功，`COMPILE_EXIT_CODE=0`。
原 runner 的 GM_ADDR 属性、printf 宽度格式警告保留，未因此改动 runner。
新目标位于原 build 目录；Parent 与 tile-sync 二进制仍保持 05:19 的 mtime。
没有对已有 ELF 运行可改写输入的提取命令。

| 对象 | 运行时间 UTC | 原 FP32 reference 超差数 | FP64 reference 超差数 | FP64 最大绝对误差 | runner 返回码 |
|---|---|---:|---:|---:|---:|
| Parent，原 R31B-V011 | 06:55:33–06:55:37 | 603872 | 603870 | 5.472246700460024 | 3 |
| tile-sync，既有诊断 | 06:55:40–06:55:44 | 3072 | 3072 | 5.472246700460024 | 3 |
| batch-sync，本次单点诊断 | 07:02:07–07:02:11 | 0 | 0 | 6.780158479102738e-7 | 0 |

三份完整输出各 1115136 个 FP32 元素，非有限值均为 0；沿用逐元素
`abs_error <= 1e-4 + 1e-4*abs(reference)`。Parent 的两种 reference 超差数相差 2，
分别保留。batch-sync 的原 FP32 reference 最大误差为 4.29153e-6。

FP64 分区结果如下；每格为“前 8192 / 末 1024”超差数。

| 实际行与 batch | Parent | tile-sync | batch-sync |
|---|---:|---:|---:|
| block 0 / batch 0，行 0–2 | 15866 / 3072 | 0 / 3072 | 0 / 0 |
| block 0 / batch 1，行 3 | 5119 / 0 | 0 / 0 | 0 / 0 |
| block 1–39，各自仅一批，行 4–120 | 548719 / 31094 | 0 / 0 | 0 / 0 |

batch-sync 相对 tile-sync 恰好改变行 0–2 的全部 3072 个尾部输出；其余
1112064 个元素逐位相同。独立复算进一步发现，将下一批行 3 首 tile 的 x/residual
用作旧批尾部的 gamma/bias，可以在原容差内解释 2944/3072 项。
例如行 0 列 8192：旧输出 0.47737669944763184，覆盖模型 0.477376685854779，
新输出 -0.8777997493743896。另 128 项不符合这个完整覆盖模型，模型最大差值
4.4776149953481；不把它当作全部错误元素的唯一解释，也不追加变体拟合这些元素。

分析复用 `analyze_outputs.py` 原有 FP64 与 FP32-add 参考计算，只增加可选逐行计数及
FP64 参考值保存；
与已保存的三个单行参考结果比较完全一致，没有重复 NPU 单行实验。
`analyze_batch_reuse.py` 将同一计算按 batch 汇总，并对已保存输出进行区域差异比较。

全部新证据位于 `parent-tail-probe/evidence/batch-reuse-01/`：

- `compile-01.log`：新目标的完整构建输出。
- `parent-121x9216-*`、`tile-sync-121x9216-*`、`batch-sync-121x9216-*`：三份完整
  `output.bin`、命令和返回码、raw.tsv、stats.txt、运行前后设备与负载输出。
- `parent-reference.json`、`tile-sync-reference.json`、`batch-sync-reference.json`：
  独立 FP64 结果、原 FP32 runner 结果、全部 121 行及 batch 分区计数。
- `effect-analysis.json`：输出逐位差异、2944 项覆盖模型匹配及模型未解释部分。
- `reference-fp64-121x9216.bin`：完整独立参考，行优先、little-endian float64，
  1115136 个值、8921088 字节，三个执行对象共用同一输入和参考；
  `offline-validation.txt` 保存源码单变化、输出长度、逐行汇总及参考数据验证结果。
- `runtime-geometry.txt`、`sdk-ordering.txt`、`sdk-event-destinations.txt`、
  `existing-variants-console.log`、`batch-sync-console.log`、`final-process-state.txt`：
  几何、SDK、连接失败及恢复、执行和结束证据。

### 设备、计时与观察限制

仍使用 `cann-server3` 的 R13 原目录
`/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/`，未进入其他 Route 目录。
三个样本运行前后 HBM 使用率均为 11%，按原协议换算 FREE_HBM=58327 MB。
样本前后 AICore 为 1–12%，AIVector 为 4–7%，load1 为 55.47–70.10。
没有等待独占，没有修改其他用户进程或 lease。

一次独立 SDK SSH 查询在 banner 阶段超时；一次只读重试已连接成功。
两个在途旧二进制诊断正常结束，新目标的构建及运行也正常结束。
三次样本的进程列表附加命令 `npu-smi info -t proc -i 3` 不受当前版本支持，
错误原样保留；HBM 与负载查询有效。07:05 的结束快照改用 `-t proc-mem` 成功，
显示一个非 R13 的 python 进程占 3914 MB，未对它执行任何写操作。
运行脚本现已采用该有效参数，未重跑三次样本；结束快照不能替代样本前的进程列表。

原始单次冷启动 device_us / host_wall_us 分别为 Parent `111941/112295`、
tile-sync `115226/115548`、batch-sync `117952/118282`。
这些时间只保留为诊断原始值，不计算 Local 分数或性能改善。此次任务只研究精度；
未运行 Parent same-binary 稳定性或正式交错性能采样。

本次验证限于一个有来源的 D、一个 block 的两批（3+1）、一次固定输入。
尚未验证两批均满、多于两批、其他宽度或低精度路径。新诊断只对这一复现问题给出
源码与因果实测结论，不宣称全范围正确或任何 Official 成绩。
D40000 的 partial 容量问题仍为独立未处理方向，本次没有运行或解决。

### 实际修改、事件与交接

实际修改限于本工作树 `研究/W4-R13/`：本 RESULT.md；parent-tail-probe 下
CMakeLists.txt、build_server3.sh、param_reuse_source.asc、analyze_outputs.py；
新增 runner_ref_batch_reuse.asc、run_batch_reuse_server3.sh、analyze_batch_reuse.py
以及上述新证据目录。未复制整套工程，旧诊断入口、旧输出、失败日志和历史提交保留。
远端只更新本 Route 的同名构建/诊断文件并增加一个目标和本次证据；没有修改服务、
共享 TSV、规则、Dashboard 或主工作树。

```text
ROUTE_RESEARCH_EVENT + ROUTE_EVENT + VERSION_RECORD_EVENT
EVENT_ID=W4-R13-BATCH-REUSE-20261008
ROUTE=W4-R13
REVISION=DIAG-PARAM-REUSE-01
VARIANT=BATCH-BOUNDARY-SYNC
DIRECT_PARENT=a10cf2a1:param_reuse_source.asc; kernel upstream R31B-V011
SINGLE_CHANGE=V_MTE2 before first input Load when batchBegin>beginRow
CLASSIFICATION=RESEARCH_EXECUTED_VARIANT; SAME_RESEARCH_REVISION
COMPILE=PASS_RC0; original two binaries reused
CORRECTNESS=NEW_DIAGNOSTIC_PASS_FP32_AND_FP64; PARENT_FAIL; TILE_SYNC_FAIL
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL=0
STAGNATION_3_CONTRIBUTION=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BRANCH=w4/r13-wide-fp32-cache-tail-x
GIT_COMMIT=see post-commit receipt
EVIDENCE=研究/W4-R13/parent-tail-probe/evidence/batch-reuse-01/
STATUS=RESEARCH_RESULT_READY; RECORD_PENDING
LAST_ACTION=single batch-dependency diagnosis completed
NEXT_ACTION=Record sync and Main release SLOT-4
RUNNING_DEVICE_OPERATION=NONE
```

07:05:16 UTC 的结束快照确认三个二进制均存在、运行中的 R13 executable 列表为空。
三次设备运行和构建均已退出，没有计时器、后台轮询或未完设备命令。
本次边界证明与最小诊断已完成；交还槽位，不关闭、合并或替换 Route。
后续独立研究的精确入口：先从正式题目输入范围确认 D40000 是否需要支持，再决定
是否在新的同 Route 任务中处理 `ChooseWideFullYRows` 与每行 16 个 partial 的容量关系；
本次三份诊断对象和数据可直接复用。
