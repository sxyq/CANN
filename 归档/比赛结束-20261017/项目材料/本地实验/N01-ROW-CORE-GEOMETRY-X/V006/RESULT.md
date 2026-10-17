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

## SERVER TRANSACTION 2026-10-17

本节追加真实服务器事务结果，覆盖并更新上文“未执行”的 Compile、Correctness 和 Local 状态。

### COMPILE

- `COMPILE=PASS`。远端 Configure 返回码 `0`，Build 返回码 `0`。
- 复用远端既有入口：
  - 工作目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V006`。
  - CMake：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V006/paired/CMakeLists.txt`。
  - Runner：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V006/paired/paired_runner_n01.asc`、`paired_runner.cpp`、`paired_runner_abi.h`。
  - 构建产物：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V006/paired/build/paired_runner_n01`。
- Candidate SHA-256：`26b787baea9f54dd6e44d8676604d43d5280b349b8865bbcc57f5a0f4242aa8`。
- Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。
- 最终 Runner SHA-256：`9774d29a584ba2c60063558d4fd7b0255355749da4d431a59ea4839208bbebf6`。
- 远端 V006 目录未找到名为 `compile.log` 的文件；本轮 Compile 原始输出保存于本地 `server-tx-20261017/compile-npu3-20261017.log`，此前失败与重跑输出仍保留在 `build-raw.log`、`build-rerun-raw.log`。

### CORRECTNESS / LOCAL

- 设备：NPU3；前置状态为 Health=`OK`、HBM 使用率 `5%`、AICore `0%`、Aivector `0%`、无进程。运行结束后仍为 Health=`OK`、HBM 使用率 `5%`、无进程。未触碰其他设备进程。
- 既有 Runner 的资格模式 `--qualify-v011` 返回码 `0`，18/18 Parent Correctness 通过；原始结果仍保留，部分高 Rows 形状出现已有采样波动记录。
- 既有 Runner 的配对模式 `--paired` 返回码 `3`，在首个宽行用例停止：
  - `fp16-tail-d12288-r2`：Parent/Candidate Correctness 均通过；21 对 Local，Parent 中位 `7.540 us`，Candidate 中位 `7.100 us`，配对差值中位 `-0.080 us`。
  - `fp16-tail-d12288-r8`：Parent/Candidate Correctness 均通过；21 对 Local，Parent 中位 `8.500 us`，Candidate 中位 `7.740 us`，配对差值中位 `-0.220 us`。
  - `fp16-wide-d32768-r1`：Parent Correctness 通过；Candidate Correctness 失败，`failures=28841`、`max_abs=6.2617188`、`max_rel=16699.234`。Runner 随后返回 `RC=3`，后续宽行和 BF16 用例未执行。
- `LOCAL=INCOMPLETE`。已生成的两组尾行 Local 仅作为原始观测保存，不形成 V006 总体 Local 结论，也不据此认定相对 R31B V011 有效改善。
- 服务器事务原始日志：
  - `server-tx-20261017/compile-npu3-20261017.log`
  - `server-tx-20261017/qualify-npu3-20261017.log`
  - `server-tx-20261017/paired-npu3-20261017.log`
  - 对应远端运行对象：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V006/paired/build/paired_runner_n01`。

### COMMIT

- `GIT_COMMIT=NONE`；本轮没有新增候选提交。
- `Official=NO`；未执行线上提交。
- 由于 Candidate Correctness 在 `fp16-wide-d32768-r1` 失败且 Local 未完成，本版本不返回线上候选路径。
