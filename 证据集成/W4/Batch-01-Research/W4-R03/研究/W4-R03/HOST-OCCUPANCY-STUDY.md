# W4-R03：host 行数与分派条件的来源绑定

本文前半保留 `712e4723` 的 host 研究。2026-10-08 本次接手的 Init／InitBuffer
消费者结果见末节；当前没有新增性能版本，原 occupancy trace 不重跑。

## 结论

设备 3 的 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 实测为 40。以该值执行从
R31B V011 提取的 host 和首级分派代码，9 个有来源的输入均通过行覆盖验证。
`64x8192 FP32` 的 40 个 blocks 中，24 个各处理两行并选中
`ProcessFp32FullRowOutputPipelined`，16 个各处理一行并进入普通后续路径。
`12x8192 FP32` 的 12 个 blocks 全部处理一行。

因此，`rowWidth == 8192 && localRows > 1` 确实能区分执行路径，但 Parent
已经包含该条件。R11 已有余行场景的 source/run 证据，本轮不把它作为新想法。
结论为 `ROUTE_REVIEW_REQUIRED`，新增性能版本 0；没有新 Candidate、Local
成绩或 Official 结果，Online 保持 PAUSED。

## 范围与状态

- 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R03-owner-occupancy-guard-x`。
- 分支：`w4/r03-owner-occupancy-guard-x`；接手提交：`de70b634813dea80783fc57716d6e95c158edeec`。
- 规则来源：`9f91895506023d917637f707bb3f61cd9d9f8765`。已完整读取 AGENTS、Route Skill、W4 控制规则、实验总则、执行约定、服务器实验规范、本地性能测试规范、Git 工作流程及资源脚本，并发送新规则回执。
- Parent：本工作树已提交的 `线上结果/R31B/V011/submission.asc`。其内容与 W3 R2 V001 的 `parent.asc` 相同。
- 旧 Record 分支中存在长 ID `W4-R03-OWNER-OCCUPANCY-GUARD-X` 的 V001 研究行；该行明确没有编辑或 Candidate，不能计作真实性能版。保留 `R31B-V011` 这一继承参考，不据此宣称 R03 曾取得新 Local 成绩。
- `LAST_KNOWN_REVISION=V001 (research only; not created)`；本轮 `NEW_PERFORMANCE_REVISIONS=0`、`VALID_LOCAL_RESULTS=0`、`CONSECUTIVE_NO_GAIN=0`。
- 指定规则提交的共享记录没有 R03 短 ID 条目，旧 Record 行又采用长 ID，保留 `STATE_SYNC_GAP`；本 Route 不修改共享表。

全部本地命令在上述工作树执行，其他分支仅通过本工作树中的 Git 对象读取。
未进入其他实际工作树，未创建分支、工作树、子 Agent、服务或定时任务，`PUSH=NO`。

## 重复性范围

| 来源 | 实际读取范围 | 对 R03 的结论 |
|---|---|---|
| W3 R2，`w3/m1/adaptive-core-ownership@6321ad44` | V001–V040 各自 Parent/Candidate 完整差异；40 份 runner 的形状与核数来源；V040 correctness 日志 | V001 是 FP32 D≤2048 的两行缩核；V002–V040 的 host 不变。owner 反序、步进、余行归属、单 owner、XOR、Gray code、位反转与旋转均有历史，不做新排列搜索。 |
| W3 R4，`w3/m1/multirow-panel-rms@ce6c6dc5` | V001–V031 host；V031 runner；R02 已提交的相关历史研究 | 31 个 host 均与 Parent 相同；runner 有 128 行、40 cores 和三种浮点 dtype 的来源。 |
| W3 R5，`w3/m1/crossrow-full-pipeline@1efa0863` | V001–V028 Candidate host；R02/R10 已提交研究 | 28 个 host 均与 Parent 相同；跨行流水和 tile-major 已有实验，不能包装成新 ownership 变化。 |
| R031 reconstruction D001–D004 | 本工作树历史 `kernel.txt` 中的 launch、行分配及 D004 分派 | 使用 min(core count, M)；已有 `avgRows<=1`、`avgRows>=2/4` 的模式选择。 |
| R31A/R31B | 全版本记录的单变化列；本工作树保存的 R31A V016/V024/V025/V026/V028、R31B V011/V016/V017 源码相关条件 | 已有单行、多行、宽行分派，D-slice、tile、流水与 store 变化。 |
| MIX、STORE/EPILOGUE | MIX-A V001–V007 与 STORE V002 源码中的行数条件；EPILOGUE/STORE 版本机制记录 | `localRows==1`、`localRows>1` 等已有使用；EPILOGUE 部分旧源码仅有记录，不能宣称完整源码覆盖。 |
| 相关 W4 | 已提交 R01/R02/R08/R09/R10/R11 Candidate host；R02 研究、R10 研究及 R11 V002 结果 | 仅 R10 V001 的 host 不同；R10 多批次 BF16 缩核属于 R10，R11 已实测 extraRows=24，本轮不重复。 |
| R03 旧研究 | `w3/m1/record-owner` 中的 R03 V001 行 | 旧研究要求取得参数来源。本轮已补设备 3 与本地代理输入的绑定；Official 输入分布仍未知，不能推测。 |

R2 runner 的实际输入如下。部分源码注释仍写旧的 8-block 代理，形状以 runner 为准。

| 版本 | FP32 输入 | 核数来源 |
|---|---|---|
| V001–V008 | 2x2048、2x2056 | `aclrtGetDeviceInfo(..., ACL_DEV_ATTR_VECTOR_CORE_NUM, ...)` |
| V009–V010 | 8x2048、8x2056 | 同上 |
| V011–V040 | 16x2048、16x2056 | 同上 |

V040 既有日志记录设备 7、40 vector cores，两个输入的 Parent/Candidate 都对
FP32 reference 通过。除 V001 缩核及 V008 单 owner 等明确改变分配的版本外，
不能从这些小 M 代理推导出多行或余行条件已经覆盖。R11 的 64 行输入提供了现成的余行证据。

```text
DUPLICATE_AUDIT
MECHANISM=以每核实际行数选择单行或多行分派，不改变 blockCount 或 owner 映射
SEARCHED_HISTORY=上表列出的已提交源码、runner 和结果记录
MATCH_FOUND=YES
WHY_NEW_OR_DUPLICATE=Parent 已有 localRows>1 条件；R031/MIX 也有行数分派。余行归属与 active-core 变化分别已有 R11/R10 范围。
NEW_PERFORMANCE_REVISION=NO
```

## 探针方法与结果边界

`source_bound_probe.py` 从接手提交读取 Parent，在内存中生成 C++：

1. 保留 `run_kernel` 的 metadata 验证、M 展开、requestedBlocks 与 dtype 选择，将三处设备 launch 换成记录函数。
2. 提取 `Process` 的首级分派条件及其使用的常量；计算函数调用换成分支标签记录。`widePath_` 按 Parent 的 D>8192 条件设置。
3. 提取 Parent 的 beginRow/localRows 算式，逐 block 记录行范围；验证无空块、行范围连续、总行数等于 M。
4. 在 server3 上编译并执行这个 host 程序。它现场调用 ACL 取得设备 3 核数，再将同一个值传给提取的 `run_kernel`。

这是实际运行的 host/source trace，未执行 NPU kernel、DMA、归约或输出算术。
`GenericRow` 是没有提前返回时使用的探针标签；宽行只观测首级分派。
本探针不能代替 device reference Correctness、实际 NPU 分支采样或 Local 测量。

日志中的 `rowsPerBlock_floor` 对应源码 `baseRows=M/blockCount`；
`rowsPerBlock_ceil` 是由 M 与 blockCount 得出的最大行数。二者均明确记录，
不把后者当成所有核的 `localRows`。`occupancy` 字段仅表示 blockCount/availableCoreNum，
不表示实测硬件利用率或性能。

| 有来源的输入 | dtype | availableCoreNum | blockCount | floor / ceil | extraRows | 首级分派 |
|---|---|---:|---:|---:|---:|---|
| 2x2048、2x2056 | FP32 | 40 | 2 | 1 / 1 | 0 | 各 2 个 NarrowMidOverlap |
| 8x2048、8x2056 | FP32 | 40 | 8 | 1 / 1 | 0 | 各 8 个 NarrowMidOverlap |
| 16x2048、16x2056 | FP32 | 40 | 16 | 1 / 1 | 0 | 各 16 个 NarrowMidOverlap |
| 12x8192 | FP32 | 40 | 12 | 1 / 1 | 0 | 12 个 GenericRow |
| 64x8192 | FP32 | 40 | 40 | 1 / 2 | 24 | 24 个 FullRowOutputPipelined，16 个 GenericRow |
| 128x12288 | BF16 | 40 | 40 | 3 / 4 | 8 | 40 个 WideLowPrecision |

前六个输入来自 R2 runner，12/64 行输入来自 R11 V002 runner。
128x12288 BF16 来自 R10 已提交 Parent 域探针及 R4 的几何来源，未把它当作隐藏测试。
9 个输入合计 144 条 block 记录、256 行，行覆盖全部通过。

既有 device reference 证据直接复用：R11 V002 在设备 0 的 64x8192 与 12x8192
FP32 上双方 reference PASS；R10 提交 `5b7b92150136513c30eb4af47c6d3d395377d94a`
记录设备 2 的 128x12288 BF16 Parent reference PASS。这些均保留原设备身份，
不写成设备 3 新 Correctness。

## 执行与原始证据

远端唯一研究目录：
`cann-server3:/home/data4t2/lelinfeng/cann/server_runs/W4-R03/research/host-occupancy-20261008/`。
其中只生成本研究的 `host_probe`，没有部署或安装操作。

- 编译器：server3 的 g++ 11.4.0，C++17、O0。
- SDK：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- ACL 头文件：`include/acl/acl_rt.h:418`，`ACL_DEV_ATTR_VECTOR_CORE_NUM=201`。
- 首次编译在链接阶段缺少驱动库查找路径，保留 `host-probe-build.log`。
- 仅补充 driver/common 的链接与运行路径后，编译于 2026-10-08 04:24:53 UTC 返回 0，见 `host-probe-build-link.log`；探针源码没有为此改变。
- 执行：2026-10-08 04:25:56–04:25:58 UTC，`./host_probe 3`，主机 `hwnput3`、用户 `lelinfeng`，见 `device3-host-probe.log`。
- `aclInit`、`aclrtGetDeviceInfo`、`aclFinalize` 返回值均为 0，`availableCoreNum=40`，`HOST_SOURCE_TRACE=PASS CASE_COUNT=9`。
- HBM 前后均为 65536 MB、使用率 13%，按资源脚本算法估算 FREE_HBM=57016 MB。
- AICore 1%→0%，AIVector 1%→0%；host load 从 43.69/45.57/45.47 变为 42.99/45.40/45.41，仅作为环境记录。
- 执行命令返回 0，前台任务已结束；未启动设备 kernel，`RUNNING_DEVICE_OPERATION=NONE`。

可复用的构建方式是将生成器 stdout 传给 server3 的 g++ 标准输入。有效链接参数为：

```text
-std=c++17 -O0 -x c++ -
-I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/include
-L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64
-L/usr/local/Ascend/driver/lib64/driver
-Wl,-rpath,/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common
-Wl,-rpath-link,/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common
-lascendcl -lascend_hal -o <本研究目录>/host_probe
```

现有二进制和日志保留，不因规则更新重复构建或运行。

## 剩余独立方向与交接

本轮没有确认可直接实施的未测 occupancy/guard 变化。以下范围不再作为新版本：
owner permutation、把已有 localRows 条件再包一层、R11 的余行重新分配、R10 的
多批次 BF16 active-core 策略。核数和输入来源问题在本地代理范围已得到回答；
Official 的输入分布仍未知，但不以此替代或暂停已授权的 Local 工作。

仍可具体研究的独立问题是：在 blockCount、行归属和数值路径均不变时，
单行核能否少做确实无消费者的参数驻留初始化。下一动作限定为读取 Parent
`Init` 中 gamma/bias 的 `InitBuffer`，与 `Process` 普通路径的实际 Load 范围逐项对应，
再核对 R05/R12 已提交记录。只有找到可省的具体初始化/指令、证明分支可达且未重复，
才形成一个性能想法。当前尚无该开销的测量证据，不宣称一定有收益，也不立即编辑 Candidate。

本次首个主要研究闭环到此结束，交 Main/Planning 复核 R03 范围并安排后续。
不自行关闭、合并或替换 Route。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=R03-HOST-OCCUPANCY-20261008
ROUTE=W4-R03
EVENT_CLASS=PARENT_ONLY_RESEARCH
STATUS=ROUTE_REVIEW_REQUIRED
DIRECT_PARENT=R31B-V011
LAST_KNOWN_REVISION=V001 (old research entry; no performance Candidate)
MECHANISM=绑定 live availableCoreNum、M、blockCount、每核行数与既有分派条件
DEVICE_ID=3
AVAILABLE_CORE_NUM=40
HOST_PROBE_COMPILE=PASS
HOST_SOURCE_TRACE=PASS; 9 cases; 144 blocks; 256 rows
COMPILE=NOT_RUN (performance Candidate)
CORRECTNESS=NOT_RUN (device reference in this turn)
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=R31B-V011 (inherited reference; no new R03 Local result)
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
VERSION_RECORD_EVENT=NONE (no performance revision)
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BLOCKER=NONE
EVIDENCE=研究/W4-R03/source_bound_probe.py; host-probe-build.log; host-probe-build-link.log; device3-host-probe.log; HOST-OCCUPANCY-STUDY.md
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=Main/Planning 复核；后续 R03 先研究单行核参数初始化的具体消费者和开销，不重做本次探针
```

上述文件属于同一个研究结果提交，具体 commit、最终 HEAD/dirty 随交接回执给出。

## 2026-10-08 续：Init 与 InitBuffer 的消费者

本次已完成限定范围的源码和 SDK 研究，没有确认可在设备上省去的独立初始化开销，
因此没有创建 Candidate 或 V001。源码中有五处可省默认赋值；部分路径也有不使用的
数据区，但直接省掉相应 InitBuffer 会改变分配容量、后续地址或对象管理状态。
`NEW_PERFORMANCE_REVISIONS=0`，`CURRENT_LOCAL_BEST=NONE`；R31B-V011 仅为继承参考。

### 本次范围与证据等级

唯一工作树和分支沿用本文前半的 R03 对象，接手 HEAD 为
`712e47230287182bc65ab433a3ce714e6fdafd9f`，接手 dirty 为 NONE。
本次为 SLOT-2，分配设备 1，实际未运行 NPU 命令；原设备 3 的核数、
9 inputs／144 blocks／256 rows 和 64x8192 的 24 个双行块、16 个单行块全部复用。
没有把这些旧结果写成设备 1 的新观测。

已完整读取本树 AGENTS／Route Skill，以及 `9f918955` 的九项指定规则，先发出
RULE_REFRESH_RECEIPT。通用 AscendC Skill 不在本分支；本次读取安装版的 API
最佳实践、文档检索 Skill、Buffer 说明和 API 索引，不读取主工作树中的技能文件。

共享事实另从本树 `git show main:<path>` 读取三份 TSV；当时 main 为
`07662d7b96e9beaaa0f56d97c9cf346b86081eb3`。R03 已登记、真实性能版数为 0；
表内 QUEUED 阶段早于本次用户接手指令。本次事件等待 Record，前半旧文中的
“规则对象内无短 ID”不代表当前共享状态。

下文 L 行号均指本树未修改的 `线上结果/R31B/V011/submission.asc`。
完整语法索引、SDK 摘录及历史搜索输出在 `init-consumer-evidence.log`；
实际命令、返回码和失败原输出在 `init-consumer-commands.log`。

| 等级 | 本次结论 | 限制 |
|---|---|---|
| 源码可省 | L57、L70 的无读取字段赋值；L58、L59、L68 的使用前必被覆盖默认值 | 只证明删除这些赋值的源码语义；没有实施删除 |
| 已确认编译器消去 | NONE | 没有可读的目标设备指令或最终优化 IR，不能用常见优化经验替代实际证据 |
| 实际设备执行未知 | 上述五处赋值、InitBuffer 的描述符写入及相关管理循环 | SDK 源码和函数尺寸都不能直接说明设备执行次数或耗时 |

`if constexpr` 排除的 dtype 分支属于语言层面的路径选择，单列处理；
不把它当作已取得动态默认赋值消除的设备证据。

### 可达路径与缓冲用途

只读文本调用索引识别 45 个本类方法，从 Init／Process 可连到 29 个；其余 16 个
没有从当前入口到达的调用链，包括旧 WideFp32CachedRows、PanelResident、Batched
等路径。该索引保留条件分支的全部可能边，具体 dtype／行数判断以源码为准，
没有模拟编译器或设备。Init 中共有 34 处 InitBuffer 调用位置。

| 当前路径 | 本路径 InitBuffer 调用数，源码计数 | 数据消费者与生命周期 |
|---|---:|---|
| 窄 FP32，NarrowMid／generic／8192 多行 | 8 | x/residual 搬入；xFp32 平方与标量尾部；residualFp32 归约工作区；valueFp32 保存 y 并原位输出；reduceFp32 保存部分和；gamma/bias 用于最终 Mul/Add |
| 窄 FP32，小行批处理 | 8 | 实际数据使用 x/residual、gamma/bias、valueFp32、reduceFp32 六块；xFp32/residualFp32 的数据区在这三条小行批处理路径中不使用，原分配仍存在 |
| 窄 FP16 | 9 | gamma/bias 为 half 参数；outputBuf 为独立 half 输出；两个 FP32 工作区承担平方、归约和标量尾部；多行输出事件保护下一次覆盖 |
| 窄 BF16 | 10 | gamma/bias 为原类型 staging；FP32 参数缓存仅在部分多行路径消费；generic 单行和 NarrowMid 用 xFp32/residualFp32 转换参数，输出使用已消费完的 xBuf |
| 宽 FP32 | 4 | 仅 xBuf、residualBuf、valueFp32、reduceFp32；前两块在输入、平方／归约、参数阶段复用；valueFp32 的整批 y 保留到写出完成 |
| 宽 FP16 | 8 | gammaBuf 保存整批 half y；参数复用 xBuf/residualBuf 的双槽；独立 outputBuf 每次写出后等待；valueFp32、两个 FP32 工作区及 reduceFp32 均有实际算术消费者 |
| 宽 BF16 | 7 | valueFp32 保存整批 FP32 y；参数原类型复用 x/residual，参数转换复用 xFp32/residualFp32；outputBuf 和 reduceFp32 均有消费者 |

这几条路径共用 L3474–3475 的 Init → Process 入口。宽 FP32 在
L1847–1853 固定转入 FullCacheRows；旧函数内出现某个 Get 不能让它变成当前消费者。
本次没有改变 availableCoreNum、blockCount、beginRow/localRows 算式或任何数值路径。

### 逐项删减判断

| 拟省项 | 可达条件与消费者证据 | 生命周期与决定 |
|---|---|---|
| `wideFp32FullYPath_` 两次赋值，L57/L70 | 全类只有两次赋值和 L3463 声明，没有读取；窄行走 L57，宽 FP32 还走 L70 | 普通 bool 无独立析构；仅删赋值可以保持成员、ABI 与全部分配原样。源码可省，设备开销未知，不建性能版 |
| `wideFullYRows_=1`，L58 | 窄行无读取；每个宽 dtype 在 L71/L82/L99 调用 ChooseWideFullYRows 后，才用于分配和 L2096/L3090 的批大小 | 后续真正计算值必须保留；仅默认值可省，设备开销未知 |
| `wideFullYTileElems_=kWideFullYTileElems`，L59 | 窄行无读取；宽行 L73/L84/L101 先赋 helper 输出，消费者为 L2095/L3089 | 字段和后续赋值保留，源码默认值可省；设备开销未知 |
| 局部 `tileElems=kWideFullYTileElems`，L68 | 宽行三条 dtype 路径都先调用 helper；helper 的 L1297 在任何读取前写入同一常量 | 传引用时不读取旧 int 值；可以只保留声明，尚无设备生成代码依据 |
| 单行 gammaBuf／biasBuf 的 InitBuffer | generic L374–392 获取并 Load，L429–448 的 FP32／FP16 affine 消费；NarrowMid L511–512、L539–540、L582–602 同样消费 | `cacheParams=false` 只改变何时加载，不取消参数存储。不可删除 |
| 窄 BF16 的 gammaFp32Buf／biasFp32Buf，L154–155 | generic 单行不走 L269–270／L452–453；NarrowMid 完整函数没有这两个 Get；BF16 特定多行路径在 L631–632、L885–886、L1682–1683 消费 | 在无数据消费者路径删去尾部分配，会让总预留从 176 KiB 变为 112 KiB，并少两个管理项，违反本次容量不变要求；不采用 |
| 小行 FP32 的 xFp32Buf／residualFp32Buf，L141–142 | L1372、L1458、L1576 三个小行批处理函数用 xBuf/residualBuf 自身作平方／归约；它们及调用的算术 helper 不获取这两个 FP32 TBuf | 原分配位于 valueFp32/reduceFp32 之前；删去会减少 32 KiB，并改变后两块位置。与本次范围不符 |
| 宽 FP16 gammaBuf，L91 | L3177–3183 写入 half y，L3315–3318 在 pass2 读取；真实 gamma 参数来自 L3256 的 xBuf | 名字不表示当前保存 gamma。整批 y 有跨阶段消费者，不可删除 |
| 五个 SetGlobalBuffer，L50–54 | x/residual 供 Load；gamma/bias 供参数 Load；output 供 Store；宽行参数复用 UB 不影响这五个 GM 来源 | SDK 对应重载设置 GM 地址，没有在这里搬入参数；删除会使后续 Load/Store 缺少正确地址 |
| TPipe 整体初始化或析构 | AllocEventID 使用 eventPool；InitBuffer/Get 使用分配和地址状态；析构读取管理项并执行最终 PIPE_ALL | 不能用本类没有显式 Destroy 调用，推导自动析构无作用；保持原状 |

两组无数据消费者缓冲的 KiB 数字来自原 Init 的长度表达式与 SDK 分配算法，
属于源码容量推导，没有新的地址探针或 NPU 测量。不能用占位分配、参数重排或
手工构造 TBuf 来规避“总容量、地址和布局不变”的要求。

`Init` 的 rowCount／blockCount 参数在函数体中未读取；本次仍保留签名和调用。
这不构成删除传参、改核数或重排参数的授权。

### SDK 中的真实管理消费者

本次只读访问 `cann-server3`，远端命令工作目录为原 R03
`/home/data4t2/lelinfeng/cann/server_runs/W4-R03/research/host-occupancy-20261008/`。
未新增远端文件、未编译或重链接、未查询设备 HBM，也未运行旧 host_probe。
读取对象来自 CANN `8.5.0.alpha002/aarch64-linux/tikcpp/tikcfw`：

- `impl/kernel_tpipe_impl.h:289` 的 TBuf 重载先对齐长度，设置 bufStart/bufLen/offset，
  再登记 FREE、无效事件 ID、address、dataLen、usertag，最后推进 maxAddr 和 curBufSize。
  此重载没有对数据区执行清零或参数 DMA；CPU 调试专用调用不能当作设备初始化搬运。
- `impl/kernel_tbuf_impl.h:22`／`:66` 的 Get 用 bufLen 求长度，读取 bufStart/address，
  更新 dataLen 并构造 LocalTensor。即使忽略数值容量，随意跳过 InitBuffer 也无法保证
  Get 获取正确地址。
- `impl/kernel_tpipe_impl.h:55` 自动调用 Destroy；`:491` 遍历 curBufSize，读取
  freeBufEvtID/state 决定是否等待，随后在正常独立 kernel 路径执行 PIPE_ALL。
  本类不调用 TBuf 的 EnQue/DeQue/FreeTensor；无效事件 ID 的初值仍是源码上
  避免误等待的依据，编译器能否折叠这段读取尚未确认。
- `impl/kernel_tpipe_impl.h:920` 重设分配游标、事件占用和共享池初值；
  `:126` 将 TPipe 指针登记到当前架构的全局指针，`:137` 设置 isDestroy=false。
  不能只观察本类直接字段使用，忽略库函数和自动析构。
- `impl/kernel_tensor_impl.h:1121` 是本 kernel 使用的单参数 SetGlobalBuffer；
  2201 分支设置 address/oriAddress。它不等同于初始化 UB 参数内容。
- `impl/kernel_utils.h:81` 的 InitSocStateImpl 在 2201 Vector 分支调用
  set_atomic_none、set_mask_norm 和 set_vector_mask。这些是有名称的设备状态操作，
  本次未证明首个算术消费者前存在等价覆盖，不能删除。

首次 SDK 查询因本地命令引号错误失败，未改文件；改用标准输入后只读查询成功。
远端没有 rg，保留返回 127 的原输出后用 Python 读取相同已知文件。
没有重试 R12/R09/R05 已失败的设备解码或编译导出参数，没有调用 objcopy。

### 历史对应与事实边界

| 来源 | 本次使用方式与结论 |
|---|---|
| W3 R2 `6321ad44` 至 V040、R4 `ce6c6dc5` 至 V031、R5 `1efa0863` 至 V028 | 本轮核对 ref，复用 R12 `e697ddf3` 和 R05 `1a31a3b5` 的完整 Init 对照：R2/R5 相同，R4 只有 V001 宽 FP16 行数上限变化；不重扫已经完成的 99 版对照 |
| R31/R31A/R31B、MIX、STORE／EPILOGUE | 本树指定历史目录共 64 份包含 kernel 类的源码，25 份仍有该无读取字段；R31B V003 已如此。另读两个历史分支的机制列，没有发现同项删赋值实验；源码份数不当作版本数 |
| 相关 W4 | 复用 R03 `712e4723`、R05 `1a31a3b5`、R12 `e697ddf3`、R09 `ec24f3ac` 的已提交研究及它们明确引用的 R01/R02/R08/R10/R11 Init 对照；不读取其他实际工作树 |
| R05 地址结果 | 只引用已测的窄 FP32 x=0、residual=0x4000、gamma=0x8000、value=0x20000 及其原输入范围；不外推宽 FP16。本次不调整子视图起点 |
| R12／R09 设备代码能力 | 复用无法读出目标助记符和既有编译导出失败的事实；没有新支持来源，不重复旧参数。符号大小或内嵌设备段相同都不能证明默认赋值已消失，更不能推出整库相同 |

部分旧 EPILOGUE-FUSE 只有机制记录，缺少本次可读的完整源码；历史搜索结论仅覆盖上表。
无读取字段的“源码已经存在”与“删除它的性能实验已经执行”分开记录。
本次没有发现可直接实施的非重复设备开销，不宣称整个 Route 已耗尽。

另有一项与消费者判断有关的来源细节：R05 runner 的 `resident-80x2056` 使用实测核数，
通过 run_kernel 传入原 Parent。按其记录的 40 核，localRows=2、2056%8=0，
L197–203 会先进入 ProcessSmallFp32Batched，无法到达 NarrowMid 的 residentParams。
因此该用例的既有 reference PASS 保留，但不能用其“resident”标签证明 NarrowMid
多行参数 Load 已被验证。这里是源码条件推导，未添加新的 occupancy trace 或计时；
没有改 R05 文件或共享成绩。

```text
DUPLICATE_AUDIT
MECHANISM=省去无读取字段或必被覆盖的默认赋值；所有分配、ABI、核心数与行归属不变
SEARCHED_HISTORY=上述 R03/W3/R31/R31A/R31B/MIX/STORE/EPILOGUE/W4 已提交来源
MATCH_FOUND=NO_IDENTICAL_REMOVAL_IN_READ_SCOPE
WHY_NEW_OR_DUPLICATE=不同于 R05 子视图位置和 R12 tiny 专用入口；只有源码可省依据，设备工作量未知
NEW_PERFORMANCE_REVISIONS=0
```

### 研究事件、停止位置与下一动作

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R03-INIT-CONSUMERS-20261008
ROUTE=W4-R03
SLOT=2
REVISION=NONE
DIRECT_PARENT=R31B-V011
SOURCE_COMMIT=712e47230287182bc65ab433a3ce714e6fdafd9f
EVENT_CLASS=SOURCE_AND_SDK_CONSUMER_RESEARCH
STATUS=SOURCE_ONLY_REMOVALS_DEVICE_WORK_UNPROVEN
SOURCE_ASSERTIONS=PASS
INITBUFFER_SOURCE_SITES=34
METHOD_COUNT=45
SYNTACTICALLY_REACHABLE_METHODS=29
UNREACHABLE_METHODS_FROM_CURRENT_ENTRY=16
REMOVABLE_SOURCE_ASSIGNMENTS=5
COMPILER_ELIMINATION_CONFIRMED=NONE
PERFORMANCE_CANDIDATE=NONE
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
INHERITED_REFERENCE=R31B-V011
LAST_KNOWN_REVISION=NONE; old V001 label was research only
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
VERSION_RECORD_EVENT=NONE
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
DEVICE_ASSIGNED=1
DEVICE_USED=NONE
FREE_HBM_MB=NOT_QUERIED
RESOURCE_BLOCKER=NONE
STATE_SYNC_GAP=THIS_RESEARCH_AND_FRESH_ASSIGNMENT_PENDING_RECORD
RUNNING_COMMANDS=NONE
DEVICE_OPERATION=NONE
RUNNING_DEVICE_OPERATION=NONE
ROUTE_LIFECYCLE_CHANGED=NO
```

本次没有性能源码变化，因此不产生 VERSION_RECORD_EVENT、不计 STAGNATION_3。
源码位置断言通过；它们不替代 Compile、独立 reference Correctness 或 Local。
所有前台命令已返回，失败命令原输出保留；没有后台进程或持续调度任务。
实际文件修改仅为本文追加说明，以及两份 init-consumer 日志。Parent、已有 host trace、
研究生成器、历史失败证据、规则、共享 TSV、Dashboard 和其他工作树均保持原样。

剩余独立问题是 TPipe 的设备状态默认值是否被非 tiny 路径的首个算术 API 等价覆盖。
下一动作限定为：以 `impl/kernel_utils.h:81` 的 2201 Vector 状态设置和
Parent NarrowMid 的首个 Add 及最终 Store 为输入，逐项追踪 mask／atomic 的第一次使用与覆盖；
同时取得当前工具链正式支持的可选择初始化接口。只有接口允许保持 TPipe 分配、
自动析构、ABI 和 UB 地址／总容量不变，且证明仍有可省设备工作，才考虑一个新概念。
当前只看到 TPipe 默认构造入口，不手写替代分配器或修改公共 SDK。
此动作与本次默认字段、缓冲消费者研究分开，不重跑 occupancy、旧 P/P、原精度用例，
也不重复失败的设备代码导出参数。若这些条件不能成立，向 Main/Planning 保留研究结论。

提交号、最终 HEAD/dirty 与槽位交接随提交后的回执给出；Main 决定释放当前 Agent，
本 Agent 不切换路线，不自行关闭、合并或替换 Route。
