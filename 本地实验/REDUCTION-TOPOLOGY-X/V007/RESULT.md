# N03 REDUCTION-TOPOLOGY-X V007

日期：2026-10-16  
父版本：V006  
状态：Compile PASS；未运行设备，等待 Main 分配实验窗口。

## HYPOTHESIS

V006 将各输入 tile 的 FP32 平方逐 tile 加到同一个 tile-width lane 向量，末尾只做一次 `ReduceSum`。V007 将相邻两个 tile 作为一个局部块：块内按 lane 将两份 FP32 平方相加，再对该块做一次 `ReduceSum`；未配对的尾 tile 单独归约。各块的标量部分和写入现有 2-float 对齐槽，最后用平衡 `Add` 树合并。

主测 shape 为 rows=2、FP16 D32768；按现有 `ChooseWideFullYRows` 参数，tileWidth=4096、tileCount=8。每行从 V006 的 1 次 `ReduceSum` / 8 次 lane-wise `Add`，变为 V007 的 4 次 `ReduceSum` / 4 次块内 `Add` / 3 次末端标量 `Add`，标量合并深度为 2。控制 shape D12288 使用 4096-element tile，tileCount=3；V007 形成一组双 tile 和一个尾 tile，即 2 次 `ReduceSum`、1 次块内 `Add` 和 1 次末端 `Add`。

BF16 D32768 的自适应 tileWidth 按现有预算计算为 2560，tileCount=13；V007 形成 6 组双 tile 和一个尾 tile，即 7 次 `ReduceSum`、6 次块内 `Add`、6 次末端 `Add`，标量合并深度为 3。BF16 D12288 使用 4096-element tile，与控制 shape 同为 3 个 tile。

性能假设：V007 把 D32768 的同 lane 串行累加从 8 步（FP16）或 13 步（BF16）缩为每个块内至多 1 次 `Add`，并以浅层标量树汇合。代价是比 V006 多做 3 次（FP16）或 6 次（BF16）`ReduceSum`，且增加末端标量合并；若水平归约调用成本占主导，V007 可能更慢。设备测量用于判断该交换是否有利。

## CHANGE

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V007/Candidate.asc`，从 V006 延续，仅改 `ProcessWideLowPrecision` 的平方累加与部分和合并。
- 复用 `xFp32Buf_` 作为当前双 tile 块的 lane 累加向量，`residualFp32Buf_` 暂存第二 tile 平方并作为 `ReduceSum` scratch；两者仍各为一个当前 tile 的 FP32 容量。
- 部分和写入现有 `reduceFp32Buf_` 行内 2-float 槽；每行占 16 个 FP32 槽，四组目标 shape 最多 7 个部分和，不新增 UB 分配。
- 输入 tile、`Load` / `DataCopyPad`、y 的存储精度、末端 RMS/输出路径、paired CMake、Runner 与 ABI 保持不变。
- 数值代价：FP32 平方和从 V006 的逐 tile lane 链变为双 tile lane 加和、块间平衡标量树；末端低精度转换保持原实现。Correctness 待后续设备窗口验证。

## COMPILE

- 主机：`hwnput3`（SSH 入口 `cann-server3`）；SoC：Ascend910B3；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；目标：`paired_runner_v016`。
- 复用 `本地实验/R31B/V016/support/paired/CMakeLists.txt` 与既有 Runner，`R31B_V016_SOURCE_DIR` 指向 V007；Parent 使用本 Clone 的 `线上结果/R31B/V011/submission.asc`。
- 构建暂存目录：`/tmp/cann-n03-reduction-topology-v007-20261016`；生成 Runner：`/tmp/cann-n03-reduction-topology-v007-20261016/build/paired_runner_v016`。
- CMake Configure 返回码 0。首次 build shell 未加载 CANN 环境，插件提示缺少 `ASCEND_HOME_PATH` 并返回 2；source 既有 `set_env.sh`、沿用现有 `CPLUS_INCLUDE_PATH` 后重跑同一 build 目标，返回码 0，输出 `[100%] Built target paired_runner_v016`。
- 完整构建输出：`build_out/n03-reduction-topology/compile-v007-server3.log`。
- 未启动 Runner，未执行 Correctness 或设备 Local。

## DEVICE EXPERIMENT

未执行。设备窗口需由 Main 后续分配。

## RESULT

Candidate 编译成功。四组设备 Correctness 与 paired Local 均待后续分配窗口后验证；V007 的数值精度与性能结果目前未知。
