# W4-R09 同步依赖与历史核对

日期：2026-10-08。仅负责 W4-R09；Online=PAUSED，PUSH=NO。

## 起点

工作树为 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R09-event-barrier-min-x`，分支为 `w4/r09-event-barrier-min-x`，初始 HEAD 为 `de70b634813dea80783fc57716d6e95c158edeec`，开始时无未提交内容。该分支及指定规则提交均没有 R09 实验文件。此前 Revision、Local Best 保持 UNKNOWN。

本轮选择 `线上结果/R31B/V011/submission.asc` 作为首版 Direct Parent，理由是本工作树保存了该源码及其正式结果。`线上结果/R31B/V011/result.json` 记录 15/15、45.16。该数值仅说明历史 Parent；本轮没有 Official 结果。Parent 的实际副本位于 `本地实验/W4-R09/V001/Parent.asc`。

## 具体依赖

以下行号属于上述 Parent 源码。

| 字段 | H1：FP16 最后一个参数 tile 的 release |
|---|---|
| EVENT | `HardEvent::V_MTE2`，pass-2 的 `prel`，按 `useA` 选择 `prel0` / `prel1` |
| PRODUCER | `ProcessWideLowPrecision` 中对 `gammaLocal` / `biasLocal` 的 Vector 读取；最后读取为 FP16 affine 的 `Add(outputLocal, outputLocal, biasLocal, valid)` |
| CONSUMER | 非末次使用时，同一参数槽的下一次 MTE2 `Load`；最后一个 tile 之后只有尾部事件消费和下一批 pass-1 的输入加载 |
| SET_LOCATION | L3346，当前参数 tile 的全部行和输出等待之后 |
| WAIT_LOCATION | L3282/L3287，在复用槽前；L3354/L3357，在 pass-2 结束时 |
| RESOURCE_REUSE | `gBase=xBuf_`、`bBase=residualBuf_` 两组槽交替；下一批 pass-1 继续使用这些输入缓冲 |
| DEPENDENCY_REQUIRED | 在 Vector 最后读取参数与后续 MTE2 覆盖之间必须维持 V→MTE2 顺序；所有非末次 release、复用等待、每行同步和 MTE3 同步均保留 |
| REDUNDANCY_PROVEN | 每行 L3339 的 `SyncVToMTE2()` 已完成该行参数读取的 V→MTE2 顺序。最后 tile 没有同阶段后继加载；其 release 只在尾部被消费。该 slot 的旧事件已在上一 tile 的预取处消费并将布尔值设为 false。因此最后 tile 同时不发送新事件、不设布尔值，尾部就不会等待未发送的事件 |
| CORRECTNESS_RISK | 若将来删除每行 `SyncVToMTE2`、改变参数缓冲用途或改变布尔状态规则，需要重新证明。此次不改上述条件；不删除输出等待，不改 reduction barrier |

状态推导：首次使用槽时布尔值为 false；此后每次把该槽作为下一 tile 预取目标时，先消费旧 release 并置 false。当前 tile 结束时才置 true。最后 tile 省去该发送与置位，尾部仍消费另一个槽的有效事件。输出数据仍经过原有 `SyncVToMTE3 → Store → SyncMTE3ToV`。

在 server3 的 CANN `8.5.0.alpha002` 中读到：`aarch64-linux/tikcpp/tikcfw/impl/kernel_tpipe_impl.h` L436 的 `AllocEventID` 设置占用位；L450 的 `ReleaseEventID` 只清除占用位，没有隐式硬件等待。H1 不改变分配与释放数量，且不留下未消费的发送。

H0：删除或延后 pass-2 `MTE3_V` 输出等待。`outputBuf_` 是所有行共用的一份缓冲，下一行 `FromFloat` 会写它；直接删除缺少完成保障。延后到下次写入前已经由 R31B V017 测试，因此不建立该性能版本。

H2：删除 FP16 pass-1/reduction 的 `PIPE_V`。W3 R5 V005/V027 已测 retained-y copy 与 Cast 之间的 barrier；V026 已测 Add 后 barrier；V028 已测平方与 ReduceSum 之间的 barrier。本轮不重复这些改动。

## DUPLICATE_AUDIT

MECHANISM：仅 FP16 宽行 pass-2 的最后一个 tile，不发送其参数 `V_MTE2` release，不设置对应 pending 布尔值；由保留的每行同步完成数据依赖。

SEARCHED_HISTORY：

- 本分支 `本地实验/W4-R09`、`研究/W4-R09` 的文件及 Git 历史：开始时为空。
- `w3/m1/adaptive-core-ownership` / `6321ad44`：V001–V040 Parent/Candidate 同步行差异；没有同步点变化。
- `w3/m1/multirow-panel-rms` / `ce6c6dc5`：V001–V031 提交和源码，V003–V031 的单变化记录；相关 V005 将每行 `SyncVToMTE2` 移到参数 panel 结束，未省去末次 release。
- `w3/m1/crossrow-full-pipeline` / `1efa0863`：V001–V028 Parent/Candidate 差异；重点 V002/V003、V004、V005、V015、V026–V028。V015 提前发送 release，保留所有发送与等待。
- R31/R31A/R31B、MIX-A、STORE-EPILOGUE、EPILOGUE-FUSE 的本工作树历史源码；`m1/r31a-exploit`、`m1/r31b-exploit` 的本地 Git 对象。R31B V017 为输出等待位置；V018 为 retained-y 写入；V019 删除每行及两阶段之间的 `SyncVToMTE2`，保留 `prel`。R31A V028 删除 affine 后、SetFlag 前的 `PIPE_V`。
- `w2/m1/sync-topology` 的 V001 diff：将当前参数 `MTE2_V` readiness wait 移到下一 tile 预取之后；没有删除末次 release。
- W4 R01、R08 的本地 Git 源码对象中，`prel` 仍按 tile 无条件发送；R07/R14/R15 的已确认本地分支和相关研究内容未提供这个改动。
- 在本工作树的研究、实验和归档文本以及本地 Git 提交主题中检索 `SYNC-BARRIER-ELISION`，未找到能绑定到具体源码的同名条目。名称覆盖存在这个限制；相关 barrier 改动已按上列真实源码核对，未把名称缺失当作新机制证据。

MATCH_FOUND：H0、H2 为已有机制；H1 未找到相同末次参数事件处理。

WHY_NEW_OR_DUPLICATE：H1 改动的是每批结束前的一个末次 release 发送/尾部等待，保留所有每行同步、所有后续还会复用槽的发送/等待。V015 改位置，R31B V019 改每行同步，均没有 H1 的生命周期边界条件。

## 验证范围

测试入口复用 W4 R01 的已编译 runner 结构和构建方式，通过本地 Git 对象读取后放入 R09 专用目录。新增独立 FP64 CPU reference：解码 FP16 输入，计算完整 RMS 与 affine，最后转 FP16；Parent 与 Candidate 分别比较。每个元素要求 `abs_error <= 2^-9 + 2^-9 * abs(reference)`，并要求最大绝对误差不超过 0.1；另外要求 Parent/Candidate 逐位相同。没有修改 reference 来配合 Candidate。

计划覆盖显式 FP16 proxy：`1x12288/b1`、`16x12288/b8`、`16x16352/b8`、`16x16384/b8`、`16x20480/b8`、`16x24576/b8`、`16x32768/b8`、`128x12288/b40`。涵盖奇偶 tile 数、尾部长度、单行、余数行和多批缓冲复用。它们不代表 hidden testcase。未改变的 FP32/BF16 路径不宣称经过本轮运行验证。

Local 采用原有 device-event 与 wall-time 边界、45 warmups、31 对交错 PC/CP 样本。先保存两个 Parent same-binary block，再保存四个 P/C block；显式性能形状为 `16x16384/b8` 和 `128x12288/b40`。保留全部 raw samples，不删异常值；噪声影响解释时报告 `MEASUREMENT_BLOCKED`，不以负载为由跳过已通过精度的 Local。

初次连接时间为 `2026-10-08T03:30:11Z`；主机 `hwnput3`，用户 `lelinfeng`，SSH 别名 `cann-server3`，设备 3 为 Ascend 910B3。初次 HBM 容量 65536 MB、使用率 11%、可用量按既有公式计算为 58327 MB，AICore 7%、AIVector 4%。运行前后继续保留设备和负载原始输出。

远端专用位置：`/home/data4t2/lelinfeng/cann/server_runs/W4-R09/V001`。首次只读查询时该目录不存在，也没有 R09 的已有设备任务；本轮只在该版本目录内构建和运行。不操作其他路线目录或进程。

## V001 结果

状态：`MEASUREMENT_BLOCKED`。单因素源码、Compile、Correctness、Local 与结果证据均已完成；本次占槽不继续 V002。路线生命周期不变，整条路线本轮最多 10 个新性能版本的要求不变。

- Compile：首轮失败于编译器生成的 host 注册代码找不到 `vector`；在同一版本给运行脚本补充 GCC 11 标准库路径后，Parent、Candidate、配对程序均编译通过。两次日志分别保存在 `compile.log`、`compile-attempt02.log`。Kernel 性能变化始终只有 H1。
- Correctness：预定八个 FP16 用例全部通过。Parent/Candidate 逐位差异均为 0；双方对独立 FP64 CPU reference 的逐元素失败数、非有限值数量均为 0，最大绝对误差 0.00390625。结论仅覆盖这些本地用例，没有正式 Judge 提交。
- Local：两个形状各取得 62 对 Parent same-binary 样本和 124 对 P/C 样本，共 12 张 raw TSV、372 对样本；每次均为 45 warmups、31 对交错 PC/CP。全部原始数据保留，包含 11549.420357 us 的 Candidate 大值。

| 显式 FP16 proxy | Parent 中位数 us | Candidate 中位数 us | 延迟变化 | Local score | 配对差值中位数 us |
|---|---:|---:|---:|---:|---:|
| 16x16384，blocks=8 | 20.340000 | 19.6899995 | -3.195676% | 103.301171 | -0.6100005 |
| 128x12288，blocks=40 | 27.600000 | 28.820001 | +4.420293% | 95.766825 | +2.6399995 |

延迟变化定义为 `100 * (Candidate median / Parent median - 1)`，正数表示更慢；Local score 定义为 `100 * Parent median / Candidate median`。它们是描述性测量统计，不是有效性能提升或 Official 分数。

Parent same-binary 两侧的 MAD/median 分别为：16x16384 的 23.13% / 22.51%，128x12288 的 14.54% / 12.46%。P/C 四组的配对中位数方向均有反转；128x12288 合并样本的 PC 配对差值中位数为 +4.7099995 us，CP 为 -2.710001 us。没有足够证据把变化归因于 H1。`CURRENT_LOCAL_BEST=UNKNOWN`，本版不计入 STAGNATION_3 的有效无改善次数。

Local 期间设备 3 的可用 HBM 始终为 60948 MB（按既有百分比公式计算），AICore 快照范围 0–18%，AIVector 0–13%，另有 Python 进程驻留。负载只作为背景记录，未取消 Local。

`2026-10-08T03:54:12.933249+00:00` 的进程查询没有发现 R09 运行进程；`RUNNING_DEVICE_OPERATION=NONE`。八次 Correctness 和十二次 Local/same-binary 运行均返回 0。详见版本目录的 `process-end.log`、`correctness.log`、`local.log` 和 `local-result.json`。

## 研究事件与后续动作

`ROUTE_RESEARCH_EVENT`：H0 输出等待位置属于 R31B V017 已测机制；H2 pass-1/reduction barrier 属于 W3 R5 V005/V026/V027/V028 已测机制。两项均为 `DUPLICATE`，没有建立新性能版本。

已完成性能版本事件的事实来源是 `本地实验/W4-R09/V001/local-result.json`，Git commit 由该文件所属提交及本轮最终 `VERSION_RECORD_EVENT` 提供。`PUSH=NO`，`OFFICIAL=NONE`，`Online=PAUSED`。

首次交接的下一动作是只读分析已生成的 Parent/Candidate 设备代码，不改源码、不重复八个精度用例。本次 fresh 接手已完成下述设备对象比较；精确指令位置仍未取得，当前下一动作见末节。

剩余独立同步点目前为 `UNKNOWN`。可研究 `ProcessNarrowMidOverlap` 的末次 `inputRelease` 生命周期（Parent L499–619），先对照 MIX-A V007 与 ASYNC-OVERLAP 历史，证明资源复用与事件消费均安全且机制未测，再决定能否声明下一版。这里仅给出研究方向，不宣称它已获安全或独立性证明。

## V001 设备代码研究：2026-10-08 fresh 接手

V001 的源码变化已体现在 FP16 设备函数内容中：同名函数从 13792 B 变为 13728 B，两个版本的代码字节不同。FP32、BF16 同名函数内容及其函数内相对重定位记录一致。BF16 的入口地址前移 64 B，不能把地址移动当作 BF16 代码变化。

本次没有取得可读设备指令，无法把某个地址绑定到末次 `SetFlag`、尾部 `WaitFlag` 或条件跳转，也无法给出指令数、执行拍数或这项改动的实际耗时。原 Local 继续为 `MEASUREMENT_BLOCKED`，`CURRENT_LOCAL_BEST=UNKNOWN`。

### 接手范围与复用

- 接手 HEAD：`96044629c9783ab540daed927be37697424365dd`，dirty 为 NONE；分支与工作树沿用本文起点中的 R09 对象。
- 已读本树 AGENTS/Route Skill，并通过本树 Git 读取 `9f91895506023d917637f707bb3f61cd9d9f8765` 的九项指定规则。接手时共享来源为 `main@4959725ea0bbf8f1e3c5f939e176db6db69bd9fe`，其中 V001、H0、H2 已登记。本次研究事件等待 Record 同步。
- 本树未包含通用 AscendC/性能 Skill；改读已安装的 `ops-direct-invoke` 同名 Skill 和 `api-pipeline.md`。这些文件未修改。
- 复用原完整依赖证明、双方各八个独立 FP64 reference 结果和全部 372 对原始样本。双方 reference 失败数及逐位差异均为 0，最大绝对误差 0.00390625。本次不增加这些验证的覆盖范围。
- 复用原历史核对中 W3 R2 至 V040、R4 至 V031、R5 至 V028，以及 R31/R31A/R31B、MIX、STORE/EPILOGUE 和相关 W4 证据；没有重新扫描同一性能想法。
- 额外读取 `e697ddf35d4c42e735236e597338a148bfb437f2:研究/W4-R12/TINY-ENTRY-CODEGEN-STUDY.md` 及其 ELF 读取脚本；读取 `1a31a3b527d81d4db0fce20dda091b85889330b6:本地实验/W4-R05/gamma-view-20261008/RESULT.md`、`results/emit-parent-ir.log`、`results/parent-device-identity.log`。均通过本树 Git 对象读取，没有访问其他 Route 工作树。

```text
DUPLICATE_AUDIT
MECHANISM=既有 V001 末参数 release 的设备对象取证
SEARCHED_HISTORY=复用上述完整历史核对；新增读取 R12/R05 已提交工具诊断
MATCH_FOUND=EXISTING_V001
WHY_NEW_OR_DUPLICATE=性能机制已经执行，本次只补设备代码证据
NEW_PERFORMANCE_REVISIONS=0
```

### 源位置到实际对象

本地与远端 Parent/Candidate 的差异均只有 L3346 开始的一处条件包裹：

```cpp
if (!std::is_same<T, half>::value || tile + 1 < tileCount) {
    AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(prel);
    if (useA) {
        pRelA = true;
    } else {
        pRelB = true;
    }
}
```

源端路径可以确认如下。下表行号属于未修改的 Parent；Candidate 在改动之后增加两行。

| 源位置 | 路径或操作 | 可证范围 |
|---|---|---|
| L3546–3548 | `dtype==1` 调用 `add_rms_norm_bias_custom<half>` | FP16 入口 |
| L3474–3475、L60–83 | `Init`；`rowWidth>kCacheElems`，其中 `kCacheElems=8192` | FP16 宽行配置 |
| L159–168、L3076 | `Process` 转入 `ProcessWideLowPrecision` | 与 FP32 宽行函数分开 |
| L3267–3346 | pass-2 参数 tile 循环；末 tile 为 `tile+1==tileCount` | 仅末 tile 省发送与 pending 置位 |
| L3282/L3287、L3354/L3357 | 参数槽复用等待与尾部等待 | 仍按已证明的布尔状态消费事件 |
| L3339–3344 | 每行 V→MTE2、V→MTE3、Store、MTE3→V | 原顺序保留 |

原构建日志和保存的 CMake recipe 说明：`support/parent_adapter.asc` 包含 `../Parent.asc`，`candidate_adapter.asc` 包含 `../Candidate.asc`；编译使用 CANN `8.5.0.alpha002` 的 `bisheng -c -x asc`、`--npu-arch=dav-2201`、设备 `-O3`，分别生成下列对象，再用原 `-Wl,-Bsymbolic -shared` 链接动态库。两个 wrapper 的 host 导出名分别为 `run_kernel_parent`、`run_kernel_candidate`，设备入口的符号名相同。

远端根目录仍为 `/home/data4t2/lelinfeng/cann/server_runs/W4-R09/V001/build/`：

| 实际只读对象 | 大小 B | 内嵌已链接设备 ELF 的文件偏移 |
|---|---:|---:|
| `CMakeFiles/w4r09_v001_parent.dir/parent_adapter.asc.o` | 136288 | `0xa00` |
| `CMakeFiles/w4r09_v001_candidate.dir/candidate_adapter.asc.o` | 136176 | `0xa00` |
| `libw4r09_v001_parent.so` | 530176 | `0x53500` |
| `libw4r09_v001_candidate.so` | 530112 | `0x53500` |

四个文件均保留 `.aicore_binary` 和 `__aicore_rel_binary`。前者为已链接设备 ELF（类型 2），后者为可重定位设备 ELF（类型 1），机器字段均为 4137。每侧编译对象与动态库中的这两个 ELF 分别逐字节一致。此结论只覆盖内嵌设备 ELF；未声称不同外层对象或 Parent/Candidate 整库一致。

FP16 实际符号为 `_Z24add_rms_norm_bias_customIDhEvPhS0_S0_S0_S0_mmjff`。两个设备 ELF 均只有 FP32、FP16、BF16 三个 `FUNC` 符号，没有独立的 `ProcessWideLowPrecision` 函数符号，也没有 `.debug_*` 源码行号段。因此源端分派可以定位，机器代码中的具体基本块仍未定位；不能声称其他 FP16 分支的生成代码完全未变。

### 字节、符号与地址分别比较

下列地址属于设备 `.text`，不属于 host 地址。符号边界另由 GNU `readelf --wide --symbols` 读取匿名内存 fd，结果与解析脚本一致。

| 设备函数 | Parent 范围 | Candidate 范围 | 函数内容 | 函数内相对重定位记录 |
|---|---|---|---|---|
| FP32：`...IfE...` | `[0x0,0x3228)`，12840 B | 相同 | 逐字节一致 | 一致，8 项 |
| FP16：`...IDhE...` | `[0x3228,0x6808)`，13792 B | `[0x3228,0x67c8)`，13728 B | 不同，长度少 64 B | 7 项，部分位置不同 |
| BF16：`...Iu6__bf16E...` | `[0x6808,0xa064)`，14428 B | `[0x67c8,0xa024)`，14428 B | 逐字节一致 | 一致，7 项 |

已链接设备 `.text` 从 41060 B 变为 40996 B；`.aicore_binary` 从 44248 B 变为 44184 B。已链接设备 ELF 中，内容不同的段只有 `.text`、`.rela.text` 和 `.symtab`；`.rodata`、`.ascend.meta`、各入口的元数据、栈记录及 `.comment` 内容一致。可重定位设备 ELF 还存在 `.strtab` 内容变化，尺寸为 1064→1070 B。字符串与符号表变化单列，不作为指令变化或耗时依据。

FP16 的七项重定位均指向 `g_vecTPipePtr`，类型值为 291，addend 为 0。按函数起点归一后，位置为：

```text
Parent:    0x00e0 0x0624 0x134c 0x18e0 0x1eb4 0x26f8 0x2e84
Candidate: 0x00e0 0x0624 0x1310 0x18a0 0x1e78 0x26b8 0x2e44
```

FP16 按四字节单元进行精确序列对齐时，两种设备 ELF 都得到 11184 B 相同片段、277 处不相同片段；这些数字属于指定对齐算法，不是指令数。不同片段包含 Parent 2608 B、Candidate 2544 B，覆盖函数内 Parent `[0x5b0,0x3254)`、Candidate `[0x5b0,0x3214)`。它们并非仅集中在一个尾部字节区间。

第一处差异在两侧函数内 `0x5b0`（设备 `.text` 地址 `0x37d8`），原始八字节分别为 `8038a6028ce89d04` 与 `8038a00284e89d04`。可重定位设备对象中已有同样差异；外层库的符号名或装载地址无法单独解释这些代码内容差异。没有尝试从原始编码解释寄存器、跳转、事件指令或周期。

### 可以解释的源码工作量与不能解释的耗时

根据既有依赖证明，源码每批省去一个末参数 tile 的发送及其尾部消费，同时保留每行同步。将原两个 Local 输入代入 `ChooseWideFullYRows`（L1293）及 L3083–3100 的行分配，得到：

| 原 FP16 输入 | tile 宽度 / tile 数 | batchLimit | 每个逻辑 block 的行数 / 批数 | 源码省去的事件对总数 |
|---|---|---:|---|---:|
| `16x16384/b8` | 4096 / 4 | 2 | 2 行 / 1 批 | 8 |
| `128x12288/b40` | 4096 / 3 | 3 | 前 8 个为 4 行 / 2 批，其余 32 个为 3 行 / 1 批 | 48 |

此表是源码循环计数，未转换成实际设备指令数。新增条件 `tile+1<tileCount` 已在同轮预取处使用；编译器是否复用判断、如何安排 pending 状态以及最终执行多少次比较/跳转，均未取得指令证据。64 B 的函数尺寸差不能回答这些问题，也不能据此推导执行拍数。

原 Local 的两个描述性延迟变化仍为 -3.195676% 与 +4.420293%。Parent 同二进制 MAD/median 达 12.46%–23.13%，且 `128x12288` 的 PC/CP 配对差值中位数为 +4.7099995/-2.710001 us。代码不同仅排除了“两侧 FP16 函数完全同码”这一解释，没有解决次序与波动来源，没有建立代码变化到收益的因果关系。没有新的 Candidate 实跑，故本研究 `LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`。

### 工具能力与停止位置

沿用 R12 已提交的结果：默认、`dav-c220`、AICore、`dav-c220-vec` 解码均只有 `<not available>`；native plugin 不支持既有 `-S` / `-###` 请求，保存中间文件曾导致 frontend 139，机器打印请求被拒绝。R05 的同工具 IR 路径也未取得输出。本次没有重试这些命令。

本次只做符号表读取和一次已安装 `bisheng --help` 查询。帮助中出现的 `-emit-llvm`、`--cce-aicore-only`、`-save-temps`、`-mllvm` 和 plugin 选项属于已知入口；通用帮助不证明 native ASC 路径接受某个输出组合。compiler 目录本身仅有 bin/include/lib，查询的 doc/docs/share/doc 子目录不存在。此有限范围内未取得新的正式支持入口，不推断整套工具链永远无法输出设备指令。没有新增编译诊断、安装或工具链变更。

取证使用只读文件输入和内存 ELF 解析；GNU `readelf` 仅访问匿名内存 fd。本次没有调用 `objcopy`，没有重新链接或复制 Parent/Candidate 库。四个原对象读取前后 size、inode、mtime、ctime 均一致，原编译时间仍为 03:45 UTC。本次未运行 NPU 程序；分配设备为 4，但实际使用设备为 NONE，也未查询新的 HBM 数值。原 V001 的测量设备 3 保持原记载。

必要的新证据只有：`研究/W4-R09/device-code-audit.py`、`本地实验/W4-R09/V001/code-study/device-code-audit.json`、`本地实验/W4-R09/V001/code-study/toolchain-readonly.log`，以及本文的研究补充。JSON 保留全部比较片段、原对象状态和实际构建 recipe；日志保存独立符号读取与能力查询。没有修改 Parent、Candidate、原 runner、旧结果、规则、共享 TSV 或 Dashboard；远端没有新增持久文件。

## 本次研究事件与交接

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R09-V001-DEVICE-CODE-DIFF-20261008
ROUTE=W4-R09
REVISION=V001
EVENT_CLASS=EXISTING_DEVICE_OBJECT_RESEARCH
DIRECT_PARENT=R31B-V011
SOURCE_COMMIT=96044629c9783ab540daed927be37697424365dd
STATUS=DEVICE_CODE_DIFFERENCE_CONFIRMED_INSTRUCTION_MAPPING_UNAVAILABLE
SOURCE_BRANCH=FP16; rowWidth>8192; pass-2 final parameter tile
DEVICE_SYMBOL=_Z24add_rms_norm_bias_customIDhEvPhS0_S0_S0_S0_mmjff
DEVICE_BASIC_BLOCK=UNRESOLVED
PARENT_FP16_BYTES=13792
CANDIDATE_FP16_BYTES=13728
FP32_BF16_CODE_BYTES_EQUAL=YES
OBJECT_LIBRARY_DEVICE_ELFS_EQUAL=YES_WITHIN_EACH_SIDE
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
EXISTING_COMPILE=PASS_REUSED
EXISTING_REFERENCE=BOTH_SIDES_8_OF_8_FP64_CPU_REUSED
DIAGNOSTIC_COMPILE=NOT_RUN
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
EXISTING_LOCAL_VERDICT=MEASUREMENT_BLOCKED
CURRENT_LOCAL_BEST=UNKNOWN
NEW_PERFORMANCE_REVISIONS=0
VALID_LOCAL_RESULTS=0
STAGNATION_3_CONTRIBUTION=0
VERSION_RECORD_EVENT=NONE
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BRANCH=w4/r09-event-barrier-min-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R09-event-barrier-min-x
DEVICE_ASSIGNED=4
DEVICE_USED_THIS_EVENT=NONE
RUNNING_DEVICE_OPERATION=NONE
STATE_SYNC_GAP=本研究事件与 fresh 接手等待 Record 同步
ROUTE_LIFECYCLE_CHANGED=NO
```

精确下一动作：取得当前 CANN `8.5.0.alpha002` 正式支持的 DAV_2201 设备指令输出方法，或带源码映射的最终生成代码；以本文记录的 FP16 符号和两侧设备 ELF 为输入，先定位 L3346 的末 tile 条件、发送与尾部消费对应的基本块，再解释实际新增和省去的执行工作。没有这一新来源前，不重复旧解码命令、原 P/C 或八个精度用例，不建立 V002。当前对象本身没有行号信息，后续如需编译诊断，仅使用未改 Parent 与独立研究输出位置。

`ProcessNarrowMidOverlap` 的 `inputRelease` 仍只是前文记载的独立研究线索，本次未扩展该方向，也未涉及 R13 的 FP32 同步诊断。所有本次命令已返回；远端取证记录中 R09 进程列表为空。最终本地 commit、HEAD 和 dirty 由交接回执给出，槽位交还 Main 处理，不自行改变 Route 生命周期。
