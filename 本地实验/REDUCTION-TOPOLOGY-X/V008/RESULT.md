# N03 REDUCTION-TOPOLOGY-X V008

日期：2026-10-16  
父版本：V007（Candidate 提交 `988c4f28`）  
状态：Compile PASS；NPU2 首组 Candidate Correctness 在同步时返回 507035（Runner RC=2），未得到 Correctness 结果；Local=NONE；V007/V008 的 507035 硬件触发点仍为 UNKNOWN。

## 已有运行证据

- V007 原始记录：`本地实验/REDUCTION-TOPOLOGY-X/V007/device3-correctness-local-20261016.log`。首个 `fp16-tail-d12288` Candidate 在 `aclrtSynchronizeStream` 返回 `507035 (null)`，Runner RC=2；未得到 Correctness 行或 Local 样本。
- V005 原始记录：`本地实验/REDUCTION-TOPOLOGY-X/V005/device3-paired-20261016T0800Z.log`。同一 shape 在 Candidate warmup 同步处返回 507035。V006 的同 shape Correctness PASS，但它没有 V007 新增的块内多次归约与标量树。
- 507035 在运行时指引中对应 `ACL_ERROR_RT_VECTOR_CORE_EXCEPTION`，由 stream 同步向 Host 报告。允许范围内的 V005/V007 日志没有设备侧指令位置、独立 stderr 文件或 plog；保存的 Runner 输出只留下 `(null)`。
- V007 远端构建目录中可见 `.asc.o` 与 `paired_runner_v016`，未见 plog。V007 Runner 为 AArch64 ELF，586208 bytes；编译日志记录了前端插件一次退出 139，随后同一目标构建成功。该构建异常不提供设备异常位置。

## V007 路径核对

- 首个样例为 FP16、rows=2、D=12288。该分支 tileWidth=4096，tileCount=3，每块 valid=4096。
- V006 与 V007 共用 `Load`。实际参数为 `DataCopyExtParams(1, count * sizeof(T), 0, 0, 0)`，随后传 `DataCopyPadExtParams<T>` 默认对象。DAV_2201 工具链定义把默认 pad 字段初始化为 `isPad=false`、左右 padding=0、paddingValue=0。此样例每次 blockLen=8192 B；行偏移 24576 B、tile 偏移 8192 B，均为 32 B 倍数；没有尾块。
- FP16 双槽 `xBuf_` / `residualBuf_` 的第二槽偏移为 4096 half，即 8192 B。保留 y 的 `gammaBuf_` 行、tile 子视图也以 32 B 对齐步距起址；所见访问不越出各自分配。
- 归约 partial buffer 按 `wideFullYRows_ * 16` 个 FP32 分配。FP16 D12288 的 `wideFullYRows_=3`，共 192 B；每行部分结果槽距为 2 个 FP32（8 B）。本 shape 的写入索引为 0、2，D32768 BF16 最多 7 个 partial，最大起始索引 12；均在分配范围内且满足 ReduceSum dst 的 8 B 起址要求。
- `ReduceSum<float>` 的 Level 2 声明为 `(dst, src, sharedTmpBuffer, count)`，传入的 tmp 类型为 float。实际 dav-2201 实现分支调用不带 tmp 指针的 `ReduceSumImpl(dst, src, count)`；该实现以 `vcadd` 和标量累加完成单次归约。因此本执行目标没有证据表明 `residualFp32Buf_` 的 scratch 容量或别名引发故障。源码仍分配 tileWidth 个 FP32（4096 个、16 KiB），与 V006 相同。
- 对本次 FP16 路径，`xFp32Buf_` 与 `residualFp32Buf_` 是独立 TBuf，分别承载 pair 累加值和第二块平方值；二者的 Add 后有 `PipeBarrier<PIPE_V>`，之后才调用 ReduceSum；归约输出在独立的 `reduceFp32Buf_`。未见输出与输入或 scratch 的地址别名。

## 源码问题与本版改动

V007 在 tile 循环内新增 pair-level `ReduceSum`，把各 pair 的结果写入 0、2、… 槽位，但循环结束后保留了 V006 的整行 `ReduceSum(reduceLocal[batchRow * reduceStride], xFp32, ...)`。对 FP16 D12288，循环先把 tile 0+1 写到槽 0、tile 2 写到槽 2；残留调用随后用最后一个 tile 的向量覆盖槽 0。平衡合并因而把 tile 2 的平方和计入两次，并丢掉 tile 0+1 的部分和。这是已执行路径里的明确数值错误，但不能单独证明它导致 507035。

V008 从 V007 复制 Candidate，只删除该残留 `ReduceSum` 及其紧邻的 barrier；pair-level 归约、平衡合并、搬运和 buffer 分配未改变。V007 源码、V007 提交中的 Runner 与原始日志保持原样。

为下一轮只运行首组 Correctness，在 `本地实验/R31B/V016/support/paired/paired_runner.cpp` 增加 `--correctness-first` 模式。它复用原有 `RunCase(..., kCorrectnessOnly)`，只选现有 case 列表首项 `fp16-tail-d12288`，跳过其他 shape 与全部配对计时；原 `--paired` 和 `--qualify-v011` 行为保持不变。没有新增 Runner 或脚本。

## Compile

- 主机：`hwnput3`；SoC：Ascend910B3；CANN：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；目标：`paired_runner_v016`。
- 复用 `本地实验/R31B/V016/support/paired/CMakeLists.txt`、既有 `paired_runner_v016.asc` 与本 Clone 的 R31B V011 Parent 源文件。V008 Candidate 经 SCP 放入本任务的远端暂存目录；没有复制其他路线内容。
- Configure 命令：`cmake -S /tmp/cann-n03-reduction-topology-v008-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v008-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V016_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v008-20261016/本地实验/REDUCTION-TOPOLOGY-X/V008`
- Build 命令：`cmake --build /tmp/cann-n03-reduction-topology-v008-20261016/build --target paired_runner_v016 --parallel 1`；已 source 同一 Toolkit 的 `set_env.sh`。
- 两条命令完成，返回码 0；输出 `[100%] Built target paired_runner_v016`。产物 Runner 为 AArch64 ELF、584720 bytes；Ascend object 为 274992 bytes。
- 完整构建输出：`build_out/n03-reduction-topology/compile-v008-server3.log`。
- `paired_runner.cpp` 加入 `--correctness-first` 后，复用同一构建目录重新运行 `cmake --build /tmp/cann-n03-reduction-topology-v008-20261016/build --target paired_runner_v016 --parallel 1`，返回码 0。更新后的 Runner 为 AArch64 ELF、584976 bytes；Ascend object 为 275304 bytes。日志：`build_out/n03-reduction-topology/compile-v008-targeted-server3.log`。
- Main 分配设备后所用命令：`./paired_runner_v016 --correctness-first --device 3`。该模式仅对 Parent 与 V008 Candidate 执行一次首组 Correctness，不进行 warmup 或 Local。

## 设备状态

Main 分配 NPU3 lease `N03-V008-D3-CORRECTNESS-20261016T105012Z`，起始时间为 `2026-10-16T10:50:12Z`。Main 随后将 lease 记为 `RELEASED`，结束时间 `2026-10-16T10:54:55Z`（UTC）。

该次 NPU3 lease 启动前 SSH banner 连续两次连接失败，原始记录保存在 `本地实验/REDUCTION-TOPOLOGY-X/V008/device3-correctness-20261016.log`。未取得该次实时设备快照或 postflight，远端 NPU3 状态为 `UNKNOWN`；Runner 未启动，因此该次尝试没有 Candidate Correctness 或 Local 结果。此记录不代表后续 NPU2 测试状态。

Main 于 `2026-10-16T11:45:19Z` 成功 SSH 取得新的只读快照：NPU3 `Health=OK`，HBM 使用 `5297/65536 MiB`，设备上有既存 Python PID `1496374`。该时点 NPU3 状态更新为 `occupied`；未干预该进程。Main 未再分配 V008 Correctness lease，本轮不得运行 V008。此前因连接失败而缺失的 postflight 仍未补齐。

Main 于 `2026-10-16T12:18:39Z` 再次取得 server3 快照：NPU3 `Health=OK`，HBM 使用 `5298/65536 MiB`，`AICore=29%`，存在 Python PID `1714635`（1921 MiB）。快照时没有 Route lease 或 Runner；该进程未被干预。NPU3 在该时点仍为 `occupied`，不适合 V008 Correctness，因此未运行 V008。状态记录时间顺序为：首次 SSH 失败后 `UNKNOWN`，`11:45:19Z` 更新为 `occupied`，`12:18:39Z` 仍为 `occupied`。

Main 约于 `2026-10-16T12:39Z` 的只读快照仍显示 NPU3 有 Python PID `1714635`（约 1921 MiB），故该时点仍为 `occupied`。截至该次快照，V008 Correctness 与 Local 均为 `NONE`；该时点未连接设备或启动 Runner。

### NPU2 首组 Correctness-only

- Main 分配 lease `N03-V008-D2-CORRECTNESS-20261016T125019Z`。预检快照 `2026-10-16T12:52:55Z`：NPU2 Health=OK、HBM 使用率 5%（总量 65536 MiB）、AICore=0%、无进程；SSH 返回码 0。
- 使用 Candidate 提交 `d82d8fc4` 与 Runner 入口提交 `c9d2a88f`。source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh` 返回码 0。复用构建目录 `/tmp/cann-n03-reduction-topology-v008-20261016/build` 中的 `paired_runner_v016`，命令：`./paired_runner_v016 --correctness-first --device 2`。
- Runner 开始于 `2026-10-16T12:53:54Z`，结束于 `2026-10-16T12:54:01Z`。首个 `fp16-tail-d12288` 的 Candidate 同步返回 `507035 (null)`；Runner RC=2，SSH 返回码 2。未到输出比较步骤，Candidate Correctness 结果为 `NONE`；未运行 Local，也未重试或 Reset。
- Postflight `2026-10-16T12:54:01Z`：NPU2 Health=OK、HBM 使用率 5%、AICore=0%、无进程。Lease 由 Main 负责释放。
- 预检、Runner 原始输出及 postflight：`本地实验/REDUCTION-TOPOLOGY-X/V008/device2-correctness-20261016.log`。

## V007/V008 507035 共同路径判断

- V007 NPU3 与 V008 NPU2 的原始日志都记录首个 `fp16-tail-d12288` 在 `candidate correctness synchronize` 返回 `507035 (null)`、Runner RC=2。Runner 的 `RunCase(..., kCorrectnessOnly)` 先启动并同步 Parent，再启动并同步 Candidate，之后才拷回和比较输出；两次错误都定位到 Candidate kernel 的 stream 同步阶段，未到输出比较。
- V007 的 `--paired` 在首个 case 先走 `kCorrectnessOnly`；V008 的 `--correctness-first` 直接选择同一 case。两次都进入 Candidate 的 FP16 `ProcessWideLowPrecision` 路径：D=12288、tileWidth=4096、tileCount=3，共用 `Load`/DataCopyPad 和 tile 循环内 pair-level ReduceSum（部分结果槽 0、2）。
- V007 另有循环后的 ReduceSum 写回槽 0；V008 只删除了这次调用及其紧邻 barrier，保留上述共同路径。V008 仍在 Candidate 同步处报错，说明这个末尾调用不是本次 507035 必需的触发条件。它覆盖了已确认的 partial 数值覆写，但没有证据表明它覆盖了向量核异常根因。
- 构建记录中 V007 与 V008 最终均出现 `[100%] Built target paired_runner_v016`；V007 编译日志先记录一次前端退出 139，随后目标构建成功。构建记录未提供设备异常指令位置。文件：`build_out/n03-reduction-topology/compile-v007-server3.log`、`build_out/n03-reduction-topology/compile-v008-server3.log`、`build_out/n03-reduction-topology/compile-v008-targeted-server3.log`。
- **根因仍为 UNKNOWN。唯一缺失证据**是 Vector Core 异常报告中的故障 PC/指令位置，能够映射到本次 Candidate kernel；两次运行日志只有 `(null)`，现有 V008 记录没有对应 plog/异常位置。
- **最小后续诊断动作**：等待 Main 分配设备窗口后，用现有 `paired_runner_v016 --correctness-first --device <分配设备号>` 单跑首组，同时启用现有 SLOG/PLOG 输出并保存故障 PC/异常记录，再映射到已编译 Candidate。当前不执行此动作。

V008 Candidate Correctness 与 Local 结果均为 `NONE`。V007/V008 的 `RUNTIME_CAUSE=UNKNOWN` 仍未变化；V008 本次 Candidate 同步异常没有设备侧指令位置或 plog 可用于定位。
