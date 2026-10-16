# N01 ROW-CORE-GEOMETRY-X V006

## HYPOTHESIS

对 D=32768 的 FP16/BF16 行，现有按行分核使 Rows=1/2 时分别只有 1/2 个 block 参与宽行归约，长行内的 tile 并行度没有展开。将每行切成 8 个 4096 元素 tile，让阶段一并行计算 8 个 FP32 平方和 partial，再由阶段二每行一个 block 合并，有机会提高少行形状的核心利用率。V006 改变行到 block 的映射与归约分片；没有改变算术语义或 `run_kernel` ABI。

该判断来自当前源码映射和形状，不是性能结论。V003/V004 的行分组实验和 V005 的 FP16 临时值复用没有为 D=32768 Rows=1/2 建立稳定 Local 收益，V006 的实际效果尚未测量。

## SOURCE BASIS

- 从已完成的 V005 H4 Candidate 接续；V005 汇总 revision 为 `c355c966`，候选源码提交为 `33494f43`。
- `run_kernel` 原路径把 block 数限制为 `min(availableCoreNum, rowCount)`；因此 Rows=1/2 的普通宽行路径只启动 1/2 个 block，每个 block 按 tile 顺序处理自己的行。
- V005 的 `ProcessWideLowPrecision` 保留逐行 tile 循环。V006 只为 `rowWidth == 32768`、`rowCount <= 2`、FP16/BF16 选择两阶段路径；其他形状继续原实现。
- 使用已有 `run_kernel` 参数及同一个 `aclrtStream` 连续 launch partial 和 output kernel；未添加 ABI 参数、workspace 接口、Runner 或脚本。

## ONE CHANGE

- Candidate：`V006/candidate/N01-ROW-CORE-GEOMETRY-H5_kernel.asc`。
- 阶段一把 `(row,tile)` 作为独立任务，在最多 8 个 block 间均衡分配。每个任务按原 FP16/BF16 算术次序计算本 tile 的 FP32 平方和，只写本行对应的一个 FP32 partial。
- D=32768 按 4096 元素切成 8 个 tile。新路径不保留完整 y 行，初始化单 tile 的输入、输出和 FP32 临时区，因此 FP16/BF16 均使用相同的 8-tile 几何；旧单阶段分支的 UB 分配不变。
- 阶段二启动 `rowCount` 个 block，每个 block 只负责一行。它先用 `DataCopyPad` 读入本行全部 8 个 partial 并完成归并和 RMS 标量计算，随后才逐 tile 写回该行输出。

## SCRATCH / ALIAS

- `PartialOffset` 以 `row * (rowWidth * sizeof(T) / sizeof(float)) + tile` 定位。D=32768、FP16/BF16 下每行跨度为 16384 个 FP32 槽（65536 字节）；partial 只占每行前 8 槽（32 字节）。每个 `(row,tile)` 对应唯一 4 字节地址区间，阶段一不读 scratch；阶段二在同一 stream 上排在阶段一之后，每行一个 block 先读全本行 partial，再覆盖本行输出。不同 tile 和不同 row 的写入区间不重叠，阶段二各 block 的输出行也不重叠。
- 现有 paired Runner 为输入和 output 分别分配内存，因此只确认该 Runner 输入输出不别名；其他调用者的 alias contract 未确认。若调用者让 output 与 x、residual、gamma 或 bias 共用内存，当前 scratch 用法尚无依据证明安全。
- 每个 partial GM 地址相对 output 起点按 4 字节递增；搬运采用 `DataCopyPad`，UB 端使用 TBuf 起始地址。其他调用者提供的 output 指针对齐情况未确认。

## COMPILE

- 未执行：本轮明确限定为项目内本地工作，不连接 server3/NPU，也不编译。
- V005 的 Build PASS 不代表 V006 已编译；V006 Candidate 的构建状态待后续使用本路线既有 paired CMake/Runner 入口确认。

## CORRECTNESS / LOCAL

- 未执行 Correctness 或 Local；没有设备运行和性能数据。
- V005 的 Parent/Candidate 18/18 Correctness 与 18 组各 21 对的 Local 结果仅属于 V005 H4，不用于证明 V006。

## NEXT

后续在用户允许访问设备后，沿用 N01 已有 paired CMake/Runner 入口接入 V006 Candidate，先 Compile，再对 FP16/BF16 的 D=32768、Rows=1/2 做 Correctness 和配对 Local。其余 D、Rows>2、FP32，以及非 paired Runner 调用者的 alias/对齐约束仍不在本版本已确认范围内。
