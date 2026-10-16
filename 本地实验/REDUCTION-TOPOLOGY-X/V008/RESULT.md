# N03 REDUCTION-TOPOLOGY-X V008

日期：2026-10-16  
父版本：V007（Candidate 提交 `988c4f28`）  
状态：Compile PASS；V008 设备 Correctness 未运行；V007 的 507035 硬件触发点仍为 UNKNOWN。

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

设备启动前 SSH banner 连续两次连接失败，原始记录保存在 `本地实验/REDUCTION-TOPOLOGY-X/V008/device3-correctness-20261016.log`。未取得本轮实时设备快照或 postflight，远端 NPU3 状态为 `UNKNOWN`；Runner 未启动。因此本轮没有 V008 Candidate Correctness 结果，也没有 Local 结果。保留 Compile PASS 记录，不据此推断设备验证结果。

待 Main 确认 server3 可达并分配新窗口后，再执行首组 FP16 D12288 Parent/Candidate Correctness-only 验证。V007 的 `RUNTIME_CAUSE=UNKNOWN` 仍未变化；V008 本轮没有运行时输出。
