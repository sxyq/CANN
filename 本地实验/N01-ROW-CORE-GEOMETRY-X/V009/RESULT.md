# N01 ROW-CORE-GEOMETRY-X V009

## HYPOTHESIS

V005 已经改变过 FP16 宽行中 FP32 scratch 的复用方式，V006 改变过宽行阶段结构，V007 改变过恒等搬运，V008 改变过行访问顺序。V009 只收缩 `ProcessWideLowPrecision` 第一遍 partial-reduction tensor handle 的源码作用域：`reduceLocal` 在第一遍块内创建，第一遍结束后不再跨过 RMS 标量计算和第二遍输出。

候选机制是宽行 Vector 临时值的生命周期：编译器可能据此减少第一遍 reduction 地址句柄或相关临时值在标量 RMS 和第二遍 Vector 操作之间的活跃范围。数据流、Buffer 大小、行/核数量、行访问顺序、tile 顺序、算术、DMA 和事件同步均保持不变。

## SOURCE BASIS

- Parent comparison：R31B V011，沿用 N01 既有 paired Runner 的 Parent。
- Parent source：沿用 V008 远端 Parent。
- Candidate：`V009/candidate/N01-ROW-CORE-GEOMETRY-H8_kernel.asc`。
- 基底：V008 H7 Candidate。
- 唯一功能变化：`ProcessWideLowPrecision` 中 `reduceLocal` 从 batch 外层移入第一遍局部块。
- 未新增或修改 Runner、CMake、ABI、参数、Buffer 配置、行/核映射、tile 宽度、归约顺序和同步入口。

## COMPILE

- `COMPILE=PASS`。Configure 返回码 `0`，Build 返回码 `0`。
- 工具链：Ascend Toolkit `8.5.0.alpha002`，SoC `Ascend910B3`，NPU arch `dav-2201`。
- 远端构建对象：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V009/paired/build/paired_runner_n01`。
- H8 Candidate SHA-256：`5fa9e751afe90e59e6202ad1580319ba8c5514e5acd8fb10757661453292abe8`。
- Runner SHA-256：`363790f871dceb710b63b15e4faecfdcaa5d219d8e22c1e74e973cd430073186`。
- Compile 未输出 Vector 寄存器占用、spill 或 stack 资源统计；源码作用域收缩是否改变寄存器分配无法从当前 Build 日志直接确认。

## CORRECTNESS / LOCAL

- `CORRECTNESS=PASS`。资格命令返回码 `0`，18/18 Parent Correctness 通过；配对命令返回码 `0`，18/18 Parent 和 18/18 Candidate Correctness 通过，Candidate failures 总数为 `0`。
- NPU3：开测前后 Health=`OK`、HBM=`3429/65536 MB`、AICore=`0%`、无进程。
- 18 组均有 21 对交错设备事件样本；设备事件为主要指标，正值表示 Candidate 较慢。

### LOCAL SUMMARY

| 用例 | Parent median us | Candidate median us | 配对差值 median us |
|---|---:|---:|---:|
| fp16-tail-d12288-r2 | 8.480 | 8.580 | -0.240 |
| fp16-tail-d12288-r8 | 7.160 | 7.160 | -0.080 |
| fp16-wide-d32768-r1 | 11.080 | 10.800 | -0.440 |
| fp16-wide-d32768-r2 | 10.360 | 10.620 | -0.120 |
| fp16-wide-d32768-r3 | 10.740 | 10.720 | -0.120 |
| fp16-wide-d32768-r4 | 10.580 | 10.300 | -0.100 |
| fp16-wide-d32768-r6 | 12.380 | 10.800 | -0.040 |
| fp16-wide-d32768-r8 | 10.720 | 10.740 | -0.100 |
| fp16-wide-d32768-r12 | 20.580 | 19.960 | -0.660 |
| bf16-tail-d12288-r2 | 7.060 | 7.220 | +0.140 |
| bf16-tail-d12288-r8 | 7.540 | 7.620 | +0.000 |
| bf16-wide-d32768-r1 | 13.980 | 14.320 | +0.380 |
| bf16-wide-d32768-r2 | 14.120 | 14.300 | +0.260 |
| bf16-wide-d32768-r3 | 14.360 | 14.680 | +0.440 |
| bf16-wide-d32768-r4 | 18.580 | 18.060 | -0.480 |
| bf16-wide-d32768-r6 | 14.680 | 14.980 | +0.520 |
| bf16-wide-d32768-r8 | 24.560 | 24.980 | +0.060 |
| bf16-wide-d32768-r12 | 39.360 | 39.980 | +0.380 |

- 18 组配对差值中位数正/负/零为 `7/10/1`，整体平均 `-0.011 us`。FP16 宽行 7 组全部为负，平均 `-0.226 us`；BF16 宽行 7 组为 `5 正/2 负`，平均 `+0.223 us`。
- Candidate/Parent Correctness 均通过，但 Local 未显示跨 dtype 一致方向。`LOCAL_SCORE=NONE`。该作用域变化没有形成可重复、可解释的硬件收益证据。
- 原始日志：`server-tx-20261017/configure-npu3-20261017.log`、`build-npu3-20261017.log`、`qualify-npu3-20261017.log`、`paired-npu3-20261017.log`、`preflight-npu3-20261017.log`、`postflight-npu3-20261017.log`。

## SERVER TRANSACTION

- 主机：`hwnput3`；入口：`cann-server3`；远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V009`。
- CMake、paired Runner、ABI、wrapper 和 R31B V011 Parent 从 V008 既有对象复制；H8 通过 Runner 所需的 H3 文件名链接进入，不改 V008。
- Configure：`cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V009/paired -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V009/paired/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`，返回码 `0`。
- Build 使用远端 `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002` 和 `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11`，返回码 `0`。
- Correctness / Local 使用既有 `./paired_runner_n01 --qualify-v011` 和 `./paired_runner_n01 --paired`，返回码均为 `0`。NPU3 前后快照均为 Health=`OK`、HBM=`3429/65536 MB`、无运行进程；其他设备上的进程未触碰。

## COMMIT

- `GIT_COMMIT=b2be1e3a`；用户要求无线上提交。
- `PUSH=NO`，`Official=NO`。

## NEXT

V009 的 Compile、Correctness 和 Local 均已完成。句柄作用域收缩未形成跨 dtype 一致收益，停止同类生命周期变体；不创建 V010。
