# N03 REDUCTION-TOPOLOGY-X V006

日期：2026-10-16  
父版本：V005  
状态：Compile PASS；Correctness 与 Local 未运行。NPU3 当前分配给 N05。

## HYPOTHESIS

V001 在 tile 级 `ReduceSum` 后用二叉 `Add` 树合并；V002/V003 采用每 4 个 tile 的局部归约；V004 将最多 8 个 tile partial 放进一次 `ReduceSum`；V005 仍逐 tile 归约，再把标量 partial 送入偶/奇双链。

V006 改变归约发生的维度顺序：每个 tile 得到 FP32 平方向量后，先与同 lane 的跨 tile 累加值相加；一行的所有 tile 完成后，再对 4096-lane 累加向量调用一次 `ReduceSum`。没有逐 tile 标量 partial，也没有标量链合并。目标是减少水平归约调用与其串行开销。

主测 shape：rows=2、D=32768、FP16/BF16，tileCount=8。V005 每行调用 8 次 tile `ReduceSum`；V006 每行调用 1 次 4096 元素 `ReduceSum`，并执行 8 次 4096-lane 向量 `Add`。V004 在此 shape 上为 8 次 tile `ReduceSum` 加 1 次 partial 合并 `ReduceSum`。控制 shape：rows=2、D=12288、FP16/BF16；tileCount=3，V006 每行从 3 次 tile `ReduceSum` 改为 1 次。

数值代价：FP32 平方值的结合顺序改变，变成先沿 tile 方向逐 lane 累加、再横向求和；末端 FP16/BF16 转换与 y 的生成路径保持不变。性能风险：D32768 有 8 步串行 lane 累加，若完整向量 `Add` 链成本高于被省去的 `ReduceSum` 调用，可能回退。共享单向量累加器需要逐行处理 Pass 1；tile 内仍保留双缓冲 MTE2，输出及参数阶段不变。

## CHANGE

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V006/Candidate.asc`，从 V005 延续，只改 `ProcessWideLowPrecision` 的宽行归约数据流。
- 复用 `xFp32Buf_` 保存 4096 个 lane 的累加值，`residualFp32Buf_` 保存当前 tile 平方并作为 `ReduceSum` scratch。两者为独立 TBuf。
- 行结果写入 `reduceFp32Buf_` 的 `batchRow * 16` FP32 槽距，即 64 字节偏移；目的地址满足 8B 对齐。归约源和 scratch 长度均为 4096 个 FP32。
- tile 宽度、`Load`/`DataCopyPad` 参数、y 的存储精度、输出和参数处理保持 V005 原样。未改 paired CMake、Runner 或 ABI；构建时通过现有 `R31B_V016_SOURCE_DIR` 参数指向 V006。

## COMPILE

- 主机：`hwnput3`；SoC：Ascend910B3；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；目标：`paired_runner_v016`。
- 构建暂存目录：`/tmp/cann-n03-reduction-topology-v006-20261016`；生成 Runner：`/tmp/cann-n03-reduction-topology-v006-20261016/build/paired_runner_v016`。
- 首次构建已完成 CMake Configure，但编译插件生成阶段因环境未提供 C++ 标准库头文件路径而以返回码 2 结束，报错 `fatal error: 'vector' file not found`。日志：`build_out/n03-reduction-topology/compile-v006-server3-attempt1.log`。
- 沿用 V003 的构建环境设置 `CPLUS_INCLUDE_PATH` 后，仅重跑现有 build 目标，返回码 0，日志末行为 `[100%] Built target paired_runner_v016`。日志：`build_out/n03-reduction-topology/compile-v006-server3.log`。
- 已确认 Runner 二进制存在；本轮没有执行 Runner 或调用设备。

## DEVICE EXPERIMENT

- Correctness：未运行。
- Local：未运行，`LOCAL_SCORE=NONE`。
- `DEVICE_ID=NONE`；未查询或使用 NPU3，等待 Main 后续分配设备窗口。
- V005 的 507035 仍为 UNKNOWN；本轮只完成 V006 编译，未试图复现该运行时问题。
