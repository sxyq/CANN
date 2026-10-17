# N01 ROW-CORE-GEOMETRY-X V008

## HYPOTHESIS

V001、V002、V003 已覆盖连续行归属下的两行、四行、三行每核；这些版本均让同一 block 处理相邻 GM 行。V008 保持 block 数和每个 block 的行数不变，只把宽行 FP16/BF16 的行访问改为 block 轮转：block `b` 处理逻辑行 `b, b + blockCount, b + 2 * blockCount, ...`。

这个映射把相邻 GM 行的起始地址分散到不同 Vector Core，测试行间地址间距是否影响 GM 访问服务或 Vector Core 间的访存竞争。单个 block 的工作量、tile 顺序、归约顺序、同步和输出归属保持不变，因此结果可直接观察行/核访问几何的作用。

## SOURCE BASIS

- Parent comparison：R31B V011，沿用 N01 既有 paired Runner 的 Parent。
- Candidate source：`V008/candidate/N01-ROW-CORE-GEOMETRY-H7_kernel.asc`。
- Candidate 基于 V007 H6，保留 V007 的宽行 FP16 `DataCopy` 路径和既有算术实现。
- 唯一行为变化位于 `ProcessWideLowPrecision`：连续 `beginRow + batchRow` 改为 `blockIdx + (batchOrdinal + batchRow) * blockCount`。
- 未新增或修改 Runner、CMake、ABI、参数、Buffer、tile 宽度、归约和同步入口。

## GEOMETRY

以 `blockCount=8` 为例：

| Rows | 连续归属 | V008 轮转归属 |
|---:|---|---|
| 1/2/4/6/8 | 每 block 一行 | 等价，每 block 一行 |
| 12 | block 0..3 取得连续两行，其余取得一行 | block 0..3 取得 `[0,8]`、`[1,9]`、`[2,10]`、`[3,11]`，其余取得 `[4]..[7]` |

## COMPILE

- `COMPILE=PASS`。远端 Configure 返回码 `0`，最终 Build 返回码 `0`。
- 工具链：Ascend Toolkit `8.5.0.alpha002`，SoC `Ascend910B3`，NPU arch `dav-2201`。
- 远端构建对象：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/build/paired_runner_n01`。
- H7 Candidate SHA-256：`a0d767aa44438467277ed119f09f87c51895d173427f16a9786cf7a085d88b42`。
- 最终 Runner SHA-256：`363790f871dceb710b63b15e4faecfdcaa5d219d8e22c1e74e973cd430073186`。
- 首次 Build 返回码 `2`，原始错误为远端编译进程未收到 `ASCEND_HOME_PATH`；保留于 `server-tx-20261017/build-npu3-20261017.log`。
- 环境变量传入远端后首次重跑返回码 `2`，错误为输出循环使用了第一遍局部变量 `row`；保留于 `build-rerun-npu3-20261017.log`。补齐同一轮转公式后第二次重跑返回码 `0`，保留于 `build-rerun2-npu3-20261017.log`。

## CORRECTNESS / LOCAL

- `CORRECTNESS=PASS`。资格模式返回码 `0`，18/18 Parent Correctness 通过；配对模式返回码 `0`，18/18 Parent 和 18/18 Candidate Correctness 通过，Candidate failures 总数为 `0`。
- 运行设备：NPU3，Ascend910B3。开测前后 Health=`OK`、HBM=`3429/65536 MB`、AICore=`0%`，NPU3 均无运行进程。
- 资格原始输出：`server-tx-20261017/qualify-npu3-20261017.log`。
- 配对原始输出：`server-tx-20261017/paired-npu3-20261017.log`。

### LOCAL

- 计时：设备事件为主要指标，wall 时间仅作诊断；每组 21 对交错样本，正值表示 Candidate 较慢。

| 用例 | Parent median us | Candidate median us | 配对差值 median us |
|---|---:|---:|---:|
| fp16-tail-d12288-r2 | 7.080 | 7.260 | +0.380 |
| fp16-tail-d12288-r8 | 6.900 | 6.760 | -0.740 |
| fp16-wide-d32768-r1 | 21.440 | 21.280 | -0.160 |
| fp16-wide-d32768-r2 | 10.840 | 10.580 | -0.040 |
| fp16-wide-d32768-r3 | 10.320 | 10.180 | -0.240 |
| fp16-wide-d32768-r4 | 10.960 | 10.940 | -0.020 |
| fp16-wide-d32768-r6 | 18.540 | 18.580 | -0.040 |
| fp16-wide-d32768-r8 | 23.860 | 21.900 | -0.100 |
| fp16-wide-d32768-r12 | 24.020 | 24.080 | -0.060 |
| bf16-tail-d12288-r2 | 7.640 | 7.640 | +0.120 |
| bf16-tail-d12288-r8 | 8.620 | 8.320 | -0.120 |
| bf16-wide-d32768-r1 | 14.920 | 15.000 | +0.420 |
| bf16-wide-d32768-r2 | 14.400 | 14.840 | +0.360 |
| bf16-wide-d32768-r3 | 20.080 | 21.680 | +0.620 |
| bf16-wide-d32768-r4 | 18.700 | 20.420 | +1.440 |
| bf16-wide-d32768-r6 | 17.060 | 20.480 | +0.380 |
| bf16-wide-d32768-r8 | 28.020 | 21.860 | -0.140 |
| bf16-wide-d32768-r12 | 25.080 | 25.740 | +0.580 |

- 18 组配对中位差值中，正/负/零为 `8/10/0`，整体平均为 `+0.147 us`；FP16 九组平均 `-0.113 us`，BF16 九组平均 `+0.407 us`。
- 实际改变轮转映射的只有 D=32768、Rows=12 的 FP16/BF16 两组：FP16 `-0.060 us`，BF16 `+0.580 us`。其余 16 组映射与 Parent 相同，用作同一 Runner 下的形状参照。
- `LOCAL_SCORE=NONE`。轮转行归属未形成跨 dtype 的稳定收益证据。

## SERVER TRANSACTION

- 主机：`hwnput3`；登录入口：`cann-server3`；远端工作目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008`。
- 目录初始化与对象复用：

```bash
ssh cann-server3 'mkdir -p /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/candidate /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/parent /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired'
ssh cann-server3 'cp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/paired/CMakeLists.txt /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/CMakeLists.txt'
ssh cann-server3 'cp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/paired/paired_runner.cpp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/paired_runner.cpp'
ssh cann-server3 'cp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/paired/paired_runner_abi.h /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/paired_runner_abi.h'
ssh cann-server3 'cp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/paired/paired_runner_n01.asc /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/paired_runner_n01.asc'
ssh cann-server3 'cp /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc'
scp /Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V008/candidate/N01-ROW-CORE-GEOMETRY-H7_kernel.asc cann-server3:/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/candidate/
ssh cann-server3 'ln -s N01-ROW-CORE-GEOMETRY-H7_kernel.asc /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/candidate/N01-ROW-CORE-GEOMETRY-H3_kernel.asc'
```

- Configure：

```bash
ssh cann-server3 'cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002'
ssh cann-server3 'env ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11 cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/build --parallel 2 --verbose'
```

返回码 `0`。Build 使用远端环境 `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`、`CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11`，最终返回码 `0`。

- Correctness / Local：

```bash
ssh cann-server3 "bash -lc 'source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:\$LD_LIBRARY_PATH; cd /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/build; ./paired_runner_n01 --qualify-v011'"
ssh cann-server3 "bash -lc 'source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:\$LD_LIBRARY_PATH; cd /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V008/paired/build; ./paired_runner_n01 --paired'"
```

资格与配对命令返回码均为 `0`。开测前状态保存于 `preflight-npu3-20261017.log`，结束状态保存于 `postflight-npu3-20261017.log`；NPU3 前后无运行进程，未触碰其他设备进程。

## COMMIT

- `GIT_COMMIT=f76b8005`（随后 amend 结果记录，最终 SHA 以 Git 输出为准）；只纳入 V008 路径。
- `PUSH=NO`，`Official=NO`。

## NEXT

V008 已完成 Compile、18 组 Correctness 和配对 Local。轮转映射结果为 `LOCAL_SCORE=NONE`；下一实验停止继续叠加行归属变体，转回 N01 尚未覆盖的硬件机制。
