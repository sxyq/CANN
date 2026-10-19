# N01 ROW-CORE-GEOMETRY-X V007

## HYPOTHESIS

在 V005 宽行 FP16 路径中，`x + residual` 已得到 half tile，随后使用
`Muls(yTile, xLocal, 1.0f, valid)` 完成恒等搬运。D=32768 时 tile 为
4096 个 half 元素，源、目的和字节数均为 32B 对齐。用 UB 内 `DataCopy`
替换这一处恒等 Vector 运算，可减少一条无效算术指令和对应的临时值活跃时间；
RMS、输出舍入、行到核映射、Tile 数量、同步和 Runner 均不变。

## ONE CHANGE

- Parent：`本地实验/N01-ROW-CORE-GEOMETRY-X/V005/candidate/N01-ROW-CORE-GEOMETRY-H4_kernel.asc`。
- Candidate：`本地实验/N01-ROW-CORE-GEOMETRY-X/V007/candidate/N01-ROW-CORE-GEOMETRY-H6_kernel.asc`。
- 唯一源码变化：宽行 FP16 第一遍将 `Muls(yTile, xLocal, 1.0f, valid)` 替换为 `DataCopy(yTile, xLocal, valid)`。
- V001–V003 的 block 几何、V004 的尾行方向、V005 的 FP32 临时区复用、V006 的两阶段 partial/output 路径均不使用。
- 不新增 CMake、Runner、脚本、ABI 参数或执行链。

## COMPILE

- `COMPILE=PASS`，`COMPILE_RC=0`（Configure=`0`，Build=`0`）。
- 工具链：Ascend Toolkit `8.5.0.alpha002`，SoC `Ascend910B3`，NPU arch `dav-2201`。
- 复用远端 V006 的 CMake、Runner、ABI 和 Parent；V007 构建产物：
  `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007/paired/build/paired_runner_n01`。
- 原始日志：`V007/compile-npu3-20261017.log`。首次路径链接错误也原样保留，随后使用正确的既有对象链接完成构建。

## CORRECTNESS

- `CORRECTNESS=PASS`，`CORRECTNESS_RC=0`。资格模式 18/18 Parent Correctness=PASS；配对模式 18/18 Parent、18/18 Candidate Correctness=PASS，Candidate failures 总数为 `0`。
- 配对 Runner 返回码 `0`，结尾为 `RUNNER_COMPLETE mode=PAIRED correctness=PASS cases=18`。
- 资格 Runner 返回码 `0`，结尾为 `RUNNER_COMPLETE mode=QUALIFY_V011 correctness=PASS cases=18`。
- 原始日志：`V007/qualify-npu3-20261017.log`、`V007/paired-npu3-20261017.log`。

## LOCAL

- `LOCAL_SCORE=NONE`。18 组均完成，每组 `21` 对交错设备事件样本；正值表示 Candidate 较慢。

| 用例 | Parent median us | Candidate median us | 配对差值 median us |
|---|---:|---:|---:|
| fp16-tail-d12288-r2 | 7.960 | 7.940 | +0.860 |
| fp16-tail-d12288-r8 | 7.640 | 7.580 | +0.000 |
| fp16-wide-d32768-r1 | 10.300 | 10.320 | -0.020 |
| fp16-wide-d32768-r2 | 11.040 | 11.180 | +0.000 |
| fp16-wide-d32768-r3 | 16.300 | 20.420 | -0.080 |
| fp16-wide-d32768-r4 | 11.340 | 11.300 | -0.060 |
| fp16-wide-d32768-r6 | 11.560 | 11.420 | -0.320 |
| fp16-wide-d32768-r8 | 25.840 | 25.660 | +0.000 |
| fp16-wide-d32768-r12 | 17.780 | 17.580 | -0.140 |
| bf16-tail-d12288-r2 | 7.640 | 14.100 | +0.340 |
| bf16-tail-d12288-r8 | 19.500 | 8.160 | -0.280 |
| bf16-wide-d32768-r1 | 14.860 | 14.820 | +0.300 |
| bf16-wide-d32768-r2 | 15.840 | 15.940 | +0.000 |
| bf16-wide-d32768-r3 | 21.780 | 15.100 | -0.340 |
| bf16-wide-d32768-r4 | 14.720 | 15.080 | +0.040 |
| bf16-wide-d32768-r6 | 15.100 | 15.920 | +0.600 |
| bf16-wide-d32768-r8 | 15.260 | 15.760 | +0.320 |
| bf16-wide-d32768-r12 | 25.800 | 26.140 | +0.680 |

- 18 组配对差值中，正/负/零分别为 `7/7/4`，总平均 `+0.106 us`；FP16 九组平均 `+0.027 us`，BF16 九组平均 `+0.184 us`。结果方向混合，未形成整体收益证据。
- 设备事件为主要计时，wall 时间仅作诊断；原始样本全部保留在 `V007/paired-npu3-20261017.log`。

### FOUR-GROUP STATISTICS

下表直接由 18 个 `PAIR_BLOCK` 原始记录汇总；Parent/Candidate/配对差值为组内各用例中位数的算术平均，`+/−/0` 为该组用例配对差值的正、负、零计数。

| 分组 | 用例数 | Parent median 平均 us | Candidate median 平均 us | 配对差值 median 平均 us | +/−/0 |
|---|---:|---:|---:|---:|---:|
| FP16 tail | 2 | 7.800 | 7.760 | +0.430 | 1/0/1 |
| FP16 wide | 7 | 14.880 | 15.411 | -0.089 | 0/5/2 |
| BF16 tail | 2 | 13.570 | 11.130 | +0.030 | 1/1/0 |
| BF16 wide | 7 | 17.623 | 16.966 | +0.229 | 5/1/1 |

`LOCAL_RC=0`（配对 Runner 返回码）；四组结果方向混合，保留 `LOCAL_SCORE=NONE`。

## SERVER TRANSACTION

- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N01/V007`。
- CMake、Runner、ABI 和 Parent 通过 V006 既有对象复用；Candidate 以 V007 源码提供，并保留 wrapper 所需的候选文件名链接。
- NPU3 前后状态：Health=`OK`、HBM=`3429/65536 MB`、AICore=`0%`、无运行进程；原始快照为 `V007/preflight-local-npu3-20261017.log` 和 `V007/postflight-npu3-20261017.log`。
- `Official=NO`；不执行线上提交。

## COMMIT

- `GIT_COMMIT=NONE`；本轮不执行本地提交。

## FINAL STATUS

- V007 已完成 Compile、Correctness 和 Local。
- V006 的 `fp16-wide-d32768-r1 failures=28841` 仍按失败版本保留；本结果不引用 V006 的失败实现作为收益依据。
- 不启动 V008。
