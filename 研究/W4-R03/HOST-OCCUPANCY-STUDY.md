# W4-R03：host 行数与分派条件的来源绑定

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
