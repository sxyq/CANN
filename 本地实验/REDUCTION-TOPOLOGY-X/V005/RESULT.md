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
