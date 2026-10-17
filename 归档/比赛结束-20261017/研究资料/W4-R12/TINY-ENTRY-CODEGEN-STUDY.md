# W4-R12：tiny 入口与生成代码

本轮没有创建 Candidate 或性能版本。已完成现有入口、host 指令及相关历史的对照；
尚未取得设备函数的可读指令，不能声称某项 Init 或 dispatch 工作已被消去，
也不能确认可安全省去的设备指令。当前结果为 `CODEGEN_EVIDENCE_UNAVAILABLE`。
原 Parent、旧精度结果与全部采样保持原样，未运行新的 Correctness 或 Local。

## 对象与范围

- 唯一 Route：W4-R12 TINY-DISPATCH-MINIMAL-X，SLOT-5。
- 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R12-tiny-dispatch-minimal-x`。
- 分支：`w4/r12-tiny-dispatch-minimal-x`；接手 HEAD：`a018f702f3e6beb91569299fe903f59826961840`；接手 dirty：NONE。
- Parent：本树 `线上结果/R31B/V011/submission.asc`；已有 runner 为 `本地实验/W4-R12/parent-tiny-20261008/`。
- 已读本树 AGENTS 与 Route Skill，并从本树读取 `9f91895506023d917637f707bb3f61cd9d9f8765` 的九项指定规则；已发 `RULE_REFRESH_RECEIPT`。
- 本树仅含角色 Skill；已完整读取安装版 ops-profiling、ascendc-env-check、ascendc-api-best-practices 及其 api-buffer 说明。未修改这些文件。
- 最新共享来源仍为 `main@285e7b4c2a6810774ca46c053bb7cac0f0b52223`。R12 已登记，但表中仍是上一位代理接手 Parent 研究的阶段；`a018f702` 结果、本次接手及本事件待 Record 同步，保留 `STATE_SYNC_GAP`。

复用的输入为已有 `1x64 FP32`、availableCoreNum=8、实际 blockCount=1、
generic cached-row。原 CPU FP64 reference 64/64 和原 task 中位 1.48 us
继续引用 `parent-tiny-20261008/RESULT.md`；原窗口 0.10 us 噪声幅度不用于本轮资格。
没有重做输入来源、Parent reference 或原 P/P 采集。

## 已排除的位置与历史

下列读取均发生在本工作树。其他分支使用 Git 对象，没有读取其他实际工作树。

| 来源 | 实际对照内容 | 对本轮的结论 |
|---|---|---|
| W3 R3，`584c590c` | V001–V010 各自完整 Parent/Candidate 差异；V010 结果 | V001 为单 tile 归约特例；V002 阈值；V003 参数缓存；V004 tile 容量；V005 NarrowMid 事件；V006–V010 为 tileCount、rowOffset、endRow、nextRow、lastTileStart 表达式。均不作为新的同项变化。 |
| R31A V008/V009 | 本树归档源码；V009 的 246–250、1120–1210 行 | 已有单行 D<=128 专用计算路径，改变搬运/同步与标量尾部；与本轮保持原计算流程的范围不符。 |
| MIX V004–V006 | 本树归档源码、V006 `diff.patch`；V006 的 199–212 行 | 已有 tiny Fast 路径、dtype 与单行限定。不能重新包装为新 tiny 分派机制。 |
| R31A/R31B/MIX | 43 份归档源码中的 `add_rms_norm_bias_custom`：20/15/8 份 | 这些入口都把动态 rowCount/blockCount 原样传给 Init/Process。这里统计源码份数，包含历史副本，不等同版本数。 |
| R31B tiny 历史 | V014 深批处理、V015 FP32 mid 分派及对应机制记录 | 深批处理和 mid 分派已有记录；不改 activeCore、tile 或计算路径来复用这些机制。 |
| R031 D001–D004 | 四份 `kernel.txt` 的设备入口；D004 的 plan 选择 | 动态参数传给 MultiModeKernel；已有 small/few 分派，D004 使用 dim 与 avgRows。没有据此移植另一套内核。 |
| W3 R2，`6321ad44` | V001–V040 的 Init、设备入口、host；首末 Process 前段 | Init 与设备入口均同 Parent；host 只有 V001 缩核不同。owner 相关变化不在 R12 范围。 |
| W3 R4，`ce6c6dc5` | V001–V031 同三函数及首末 Process 前段 | 只有 V001 宽 FP16 Init 行数上限不同；设备入口与 host 相同。 |
| W3 R5，`1efa0863` | V001–V028 同三函数及首末 Process 前段 | 三函数均同 Parent；不把跨行流水作为新的 tiny 入口变化。 |
| W4 R01/R02/R08/R09/R10/R11 | `93f15d9b`、`60ca277d`、`fa9b19bb`、`96044629`、`1441fc72`、`4243f4e9` 中已提交 Candidate 的 Init/入口/host | Init/入口相同；R10 V001/V002 host 不同，属于 active-core 范围。 |
| STORE / EPILOGUE | STORE V002/V003、EPILOGUE-ARITH V001/V002 的设备入口及机制记录 | 仍为动态实参；已有写出和算术改动。缺失的旧 EPILOGUE-FUSE 源码仅有记录，不声称完整源码覆盖。 |
| R03 / R05 | `712e4723:研究/W4-R03/HOST-OCCUPANCY-STUDY.md`；`61e0aa28:研究/W4-R05/PARAM-OUTPUT-LAYOUT-STUDY.md` | 单行参数初始化消费者及参数子视图位置分别属于 R03/R05。本轮不改这些对象。 |

```text
DUPLICATE_AUDIT
MECHANISM=tiny generic 改为专用精简计算路径
SEARCHED_HISTORY=W3 R3；R31A V008/V009；MIX V004-V006；R31B tiny 历史
MATCH_FOUND=YES
WHY_NEW_OR_DUPLICATE=已有路径替换，包含本轮禁止改变的搬运、同步或归约。

DUPLICATE_AUDIT
MECHANISM=仅 tiny entry 固定语义不变的单行实参，Init/Process 原样
SEARCHED_HISTORY=上表入口源码及相关记录
MATCH_FOUND=NO_IN_READ_SOURCES
WHY_NEW_OR_DUPLICATE=已读入口均使用动态实参；但尚无设备生成指令证明可省工作。
PERFORMANCE_CANDIDATE=NOT_CREATED
```

## 已有二进制与指令依据

唯一远端对象为
`cann-server3:/home/data4t2/lelinfeng/server_runs/W4-R12/parent-tiny-20261008/build/r12_parent_probe`。
文件为 478048 字节，mtime 为 `2026-10-08T04:40:20.501076+00:00`。
所有诊断均未改写此对象。没有重新链接 Parent。

`parent-host-entry.disasm.txt` 来自这个原可执行文件的 GNU objdump：

| 位置 | 可确认的实际指令 | 不能据此推出的结论 |
|---|---|---|
| `0x3c114 / 0x3c39c / 0x3c494` | 三个 `bl 0x3b5b0 <run_kernel>` 调用点 | 不能据调用点数推测 host 开销比例。 |
| `0x3b5b0–0x3b788` | 指针、shape/dtype、行数展开及乘法溢出处理 | 不移除输入有效性保护，不把 host 指令当作 NPU task 指令。 |
| `0x3b798–0x3b7b4` | 请求 block 数量的比较/选择，以及 `fdiv` 求 invRowWidth | 不改变核数和参数值。 |
| `0x3b7b8 / 0x3b7c0 / 0x3b7c8 / 0x3b7e0` | dtype 比较和 FP32 stub 尾跳转 | 这部分位于 host，原 task 资格不能证明它的优化收益。 |

原 ELF 的 `.aicore_binary` 位于 `0x46200`，大小 44248 字节。
FP32 符号 `_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff` 的
`.text` 范围为 `[0, 0x3228)`，共 12840 字节。尺寸来自符号表，不能换算为有效
指令数或执行耗时。设备 ELF 通过匿名内存 fd 读取，没有生成第二份 Parent 可执行文件。

四份 `parent-fp32*.disasm.txt` 保存默认解码、dav-c220、AICore 模式以及
从 native 输出确认的 dav-c220-vec 模式。工具均返回 0，但助记符全部为
`<not available>`，因此不视为成功取得设备指令。

`parent-native-plugin.json` 是本 Route 编译诊断生成的原始文件。
其中 DeviceVecExtraCompileOptions 指定 `dav-c220-vec`；DeviceStub 继续把动态
实参传入原函数。HostStub 含 overflow 状态的分配/释放调用，但本轮未测其内部
实现和耗时，不把调用名称直接解释为每次真实 HBM 分配。

## 编译诊断的限制与失败证据

仅为取得可读生成代码，诊断脚本从原 `flags.make`、`build.make` 复用编译参数，
一直使用未修改的 `parent_probe.asc`。未编辑任何性能源码。

| 本地原日志 | 结果 |
|---|---|
| `parent-assembly-compile.log` | `-S` 配合单一 `-o` 被拒绝：存在多个输出；返回 1。 |
| `parent-assembly-compile-no-o.log` | 省去单一输出后，原生入口报告不支持 `-S`；返回 1，无汇编文件。 |
| `parent-intermediates-compile.log` | `-save-temps` 的 native plugin frontend 退出 139；外层返回 1，未生成目标对象。 |
| `parent-driver-plan.log` | 顶层 `-###` 仅给出 native plugin 两段入口，无 device cc1；日志保留。一次 shlex 解析遇到混排文字而失败，没有改变日志。 |
| `parent-device-plan.log` | 读取已有 plugin 输出后，入口仍报告不支持 `###`。外层返回 0 不代表计划有效。 |
| `parent-machine-output.log` | 已确认 LLVM 存在 print-after/filter-print-funcs 选项；通过原 Xaicore 路径请求机器指令时，native 入口报告 `unsupported option 'unsupport-options'`；返回 1。 |

诊断时设备 4 的 FREE_HBM 均为 6553 MB。负载和 AICore/AIVector 使用率均保留在
日志中，没有据此等待或拒绝执行。上述过程没有调用 Parent runner 或启动 NPU kernel。
远端新增内容仅在原目录的 `code-study/`，包含失败时生成的 JSON、注册代码和空的
临时子目录；全部保留。没有创建新分支、工作树、子 Agent 或持续调度任务。

## 唯一后续动作与交接

下一动作限定为：取得原 Parent 上述 FP32 设备符号的可解码指令，或由当前工具链
正式支持的最终优化 IR 输出；沿已知 M=1、D=64、blockCount=1 的路径定位
3474–3475 行 Init/Process 实参以及 171–246 行控制序列。
只有证明动态控制仍实际执行，才能判断单行 tiny entry 的常量实参是否值得一版实验。
此处没有确认收益、没有确认新的可删指令，也不声称本 Route 已耗尽。
不重复尝试本节已失败的同参数命令，不重测旧 P/P，不进入 R03/R05 范围。

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R12-TINY-ENTRY-CODEGEN-20261008
ROUTE=W4-R12
EVENT_CLASS=SOURCE_AND_COMPILER_RESEARCH
REVISION=NONE
DIRECT_PARENT=R31B-V011
STATUS=CODEGEN_EVIDENCE_UNAVAILABLE
HOST_DISASSEMBLY=PASS
DEVICE_DISASSEMBLY=UNAVAILABLE
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
EXISTING_PARENT_REFERENCE=CPU_FP64_64_OF_64_REUSED
COMPILER_DIAGNOSTICS=NO_READABLE_DEVICE_CODE
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
CONSECUTIVE_NO_GAIN=0
VERSION_RECORD_EVENT=NONE
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
RESOURCE_BLOCKER=NONE
STATE_SYNC_GAP=本事件与此前 Parent 结果等待 Record
RUNNING_DEVICE_OPERATION=NONE
NEXT_ACTION=取得原 FP32 设备入口的可读生成代码，限定核对 tiny entry 控制工作
ROUTE_LIFECYCLE_CHANGED=NO
```

本轮实际新增文件全部位于 `研究/W4-R12/`：本报告、诊断脚本、四份设备解码原输出、
host 指令、native plugin JSON、六份诊断日志及两份最终进程记录。
首次进程查询把自身 shell 算入结果，原 `final-state.log` 保留；
`final-state-verified.log` 只排除当前查询的祖先进程，确认 R12 活动进程为空，
原 runner 大小和时间未变，`parent-inspect.o` 不存在。
所有本地/远端运行命令已结束。未改 Parent、已有实验、规则、共享记录、Dashboard
或其他工作树。提交号、最终 HEAD 与 dirty 随提交后的回执提供。
