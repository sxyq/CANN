# N03 REDUCTION-TOPOLOGY-X V005

日期：2026-10-16
状态：Compile PASS；NPU3 配对运行在首个 Candidate warmup 同步处返回 507035，四组配对未完成；未 Reset。

## HYPOTHESIS

V004 将每个 4096-element tile 的 FP32 部分和交给一次最长 8 项的 `ReduceSum`。V005 改为两条交错的 FP32 累加链：偶数 tile 进入一条链，奇数 tile 进入另一条链，最后只做一次链间 Add。假设两条彼此独立的依赖链能让 Vector 指令调度更好地隐藏累加延迟。

与已做版本的区别：V001 对全部 tile 部分和做平衡 Add 树；V002/V003 采用连续 4-tile ReduceSum 分组；V004 使用 8-tile 单次 ReduceSum。V005 的单一实验变量是 tile 部分和的最终合并图，采用偶/奇交错的双链累加。

## CHANGE

- 父版本：V004。
- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V005/Candidate.asc`。
- 现有 paired CMake/Runner：`本地实验/R31B/V016/support/paired/`。
- `ReduceSum<float>` 的目的地址按 8 字节对齐；每个 tile 输出使用 2-float 槽。该槽距是 API 对齐所需的数据布局，性能假设仍只针对最终双链合并图。
- tile 宽度、FP32 平方和、RMS 运算、输出和搬运路径不变。

## COMPILE

- 主机：`hwnput3`；SoC：Ascend910B3；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- 目标：`paired_runner_v016`。
- 执行命令：

  `cmake -S /tmp/cann-n03-reduction-topology-v005-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v005-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V016_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v005-20261016/本地实验/REDUCTION-TOPOLOGY-X/V005`

  `cmake --build /tmp/cann-n03-reduction-topology-v005-20261016/build --target paired_runner_v016 --parallel 1`

- 返回码：0；输出 `[100%] Built target paired_runner_v016`。
- 构建目录与 Runner：`/tmp/cann-n03-reduction-topology-v005-20261016/build`、`/tmp/cann-n03-reduction-topology-v005-20261016/build/paired_runner_v016`。
- CMake 提示运行时库搜索路径存在 `libstdc++.so.6` 次序警告；构建与链接均成功。
- 日志：`build_out/n03-reduction-topology/compile-v005-server3.log`。

## DEVICE EXPERIMENT

用户分配 NPU3。沿用现有 Runner `--paired --device 3`；目标为 `rows=2` 的 FP16/BF16 × D12288/D32768 四个 shape，每个 shape warmup 45 次、21 组 Parent/Candidate 配对。Runner 在首个 Candidate warmup 阶段退出，后续 shape 和配对采样均未运行。全过程未 Reset。

- 主机：`hwnput3`；SoC：Ascend910B3；设备：NPU3。
- 运行窗口：开始前采样 `2026-10-16T08:00:19Z`；Runner 启动 `2026-10-16T08:00:52Z`；退出及 post 快照 `2026-10-16T08:01:00Z`；精确进程名确认无 Runner、NPU 无进程 `2026-10-16T08:01:42Z`。本次设备窗口结束时间：`2026-10-16T08:01:42Z`。
- Pre：Health=OK；HBM usage rate=5%；AICore=0%；AIVector=0%；`proc-mem` 显示 `No process in device`。
- 命令：`./paired_runner_v016 --paired --device 3`；CANN 环境 source 返回 0。
- Runner 返回码：2。stderr：`candidate warmup synchronize failed: 507035 (null)`。stdout 仅进入 `fp16-tail-d12288` 的 `CASE_BEGIN`，没有 Correctness 行、配对样本或后续 shape。
- Post：Health=OK；HBM usage rate=5%；AICore=0%；AIVector=0%；`proc-mem` 显示 `No process in device`；`pgrep -x paired_runner_v016` 确认为 ABSENT。
- 原始 stdout/stderr、命令、时间和前后快照：`本地实验/REDUCTION-TOPOLOGY-X/V005/device3-paired-20261016T0800Z.log`。
- 未重跑、未 Reset、未触碰其他进程。NPU2 未使用。

## CORRECTNESS

状态：未完成。首个 FP16 D12288 Candidate warmup 同步失败，Correctness 未产生结果；其他三组未执行。

## LOCAL

状态：未产生配对样本；`LOCAL_SCORE=NONE`。

## RESULT

Candidate 编译成功。设备运行在 FP16 D12288 首个 Candidate warmup 同步处返回 507035，未取得 Correctness 或 Local 结果。进程已退出，NPU3 于 `2026-10-16T08:01:42Z` 释放；未 Reset。

## V005 507035 源码诊断

- 触发位置：现有 Runner 先完成 Parent warmup，再启动 Candidate 并调用 `aclrtSynchronizeStream`；原始日志在 `candidate warmup synchronize failed: 507035 (null)` 处结束。没有 kernel 指令位置或设备侧错误详情，不能从此调用点确定 Candidate 内部哪条操作失败。
- 目标样例为 FP16、rows=2、D=12288。宽行路径 tileWidth=4096、tileCount=3，每个输入 tile 的 `valid=4096`。
- `Load` 实际构造 `DataCopyExtParams(1, count * sizeof(T), 0, 0, 0)`，再传入默认构造的 `DataCopyPadExtParams<T>`。FP16 全 tile 的 blockLen 为 8192 字节，src/dst stride 均为 0；此样例没有尾 tile。pad 参数字段在 `Load` 中没有显式赋值。该 helper 与 V004 相同；缺少本 Clone 内 API 定义，不能进一步确认默认字段值。
- 按 V005 的 `ChooseWideFullYRows` 计算，FP16 D12288 取 3 行，tileWidth=4096。`reduceFp32Buf_` 容量为 `3 * 16` 个 FP32，即 192 字节；每行 stride 为 16 个 FP32。V005 的三项 tile 部分和写入当前行的 offset 0、2、4，最大写入 index 为 4；若处理到第三行，最大 index 为 36，小于 48 个 FP32 的总容量。
- FP16 的 `xBuf_`、`residualBuf_` 各有 2 个 4096-half 子块，第二块偏移为 4096 个 half；`xFp32Buf_`、`residualFp32Buf_`、`valueFp32Buf_` 各为 4096 个 float。完整 y 的 `gammaBuf_` 为 `12288 * 3` 个 half。所见 tile 与 batchRow 子张量的最大访问都落在对应分配范围内。
- V005 的每个 `ReduceSum<float>` 临时区是独立的 4096-float scratch。dst offset 为 `batchRow * 16 + tile * 2`，均为 2-float 倍数，满足 8 字节对齐；源和临时区类型均为 FP32。V004 对应调用使用 `batchRow * 16 + tile`，奇数 tile 的 dst 只有 4 字节偏移，存在对齐风险；V005 已改成 2-float 槽，这项不能说明 V005 的同步错误。
- 汇总阶段 `reduceFp32Buf_` 与 `xFp32Buf_` 是分别初始化的 TBuf；前者保留 tile 部分和，后者只用 index 0 和 2 作双链累加。输入循环结束前等待挂起的 V_MTE2 事件，汇总阶段在 Duplicate、每对 Add、尾项 Add 和最终 Add 之后均有 `PipeBarrier<PIPE_V>()`。从源码可见的槽位与 scratch 生命周期没有交叉覆盖。
- V005 相对 V004 的代码差异集中在 tile 部分和 dst 间距及最终双链 Add 图。后者是唯一新的算术路径，但当前源码没有显示越界、未对齐 dst 或 scratch 重叠的直接证据。
- 本 Clone 的 `.agents/skills` 中没有 `ascendc-runtime-debug/SKILL.md` 与 `references/debug_workflow.md`；本次未读取其他 Clone。故无法依据该工作流进一步映射 507035 的设备侧语义。

诊断结论：`RUNTIME_CAUSE=UNKNOWN`。DataCopyPad 默认 pad 字段的具体初始化语义及双链 Add 对应的设备执行位置仍未确认。未找到支持 Candidate 源码修改的明确问题；Candidate 未改、未重编、未重跑设备。
