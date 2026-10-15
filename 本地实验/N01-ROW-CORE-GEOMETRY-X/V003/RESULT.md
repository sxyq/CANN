# N01 ROW-CORE-GEOMETRY-X V003

## HYPOTHESIS

V001 测试了宽行目标两行/核，V002 测试了四行/核；两者在 D=32768 的 Rows=2/4/8 均未显示收益。V003 测试第三个几何点：在 `D >= 16384 && Rows > 1` 时使用 `min(availableCoreNum, ceil(Rows / 3))`，即目标三行/核。新增 Rows=3/6/12 形状用于覆盖完整三行组和尾行组。

## INPUT AND ONE CHANGE

- Parent：`本地实验/N01-ROW-CORE-GEOMETRY-X/V003/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`，沿用 V002 父源码；其公开来源为线上结果/R31B/V011/submission.asc。
- Candidate：`本地实验/N01-ROW-CORE-GEOMETRY-X/V003/candidate/N01-ROW-CORE-GEOMETRY-H3_kernel.asc`。
- Kernel 唯一候选变化：`run_kernel` 在宽行且 `rowCount > 1` 时加入 `kTargetRowsPerCore = 3` 的 `blockCount` 上限。
- Runner 仅扩展已有入口的形状集合：FP16/BF16 各新增 D=32768、Rows=3/6/12；原有 D=12288 Rows=2/8 和 D=32768 Rows=1/2/4/8 保留。
- 算术、Tile、同步、参数加载和设备侧连续行归属公式未改动。

## COMPILE

- 设备：server3，Ascend910B3，device 0；Toolkit `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- Configure + Build：
  `export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake -S /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V003/paired -B /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V003/paired/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 && cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V003/paired/build --parallel 2`
- 返回码：0。
- 产物：`/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V003/paired/build/paired_runner_n01`。
- 运行时首次失败证据：缺少 `libgraph.so`；第二次失败证据：HCC 旧版 `libstdc++.so.6` 缺少 `GLIBCXX_3.4.29`。两份原始日志分别为 `qualify-v011.runtime-loader-failure.log` 和 `qualify-v011.glibcxx-failure.log`。使用系统 GCC 11 与 Toolkit runtime 的 `LD_LIBRARY_PATH` 后，资格流程正常运行。

## CORRECTNESS

- 资格命令返回码：0；18/18 Parent Correctness=PASS。
- 配对命令返回码：0；18/18 Parent Correctness=PASS，18/18 Candidate Correctness=PASS。
- 新增 Rows=3/6/12 的 FP16/BF16 共 6 组均 Correctness=PASS。
- 同二进制资格中部分形状出现 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` 或 `NEEDS_VALIDATION`；原始状态保留，不用其解释配对性能。
- 原始日志：`qualify-v011.log`、`paired.log`。

## LOCAL

设备事件中位数及配对差值（H3 - V011，单位 us）：

| 用例 | Parent | H3 | 配对差值 |
|---|---:|---:|---:|
| fp16-tail-d12288-r2 | 6.220 | 6.120 | +0.040 |
| fp16-tail-d12288-r8 | 6.440 | 6.340 | -0.100 |
| fp16-wide-d32768-r1 | 9.700 | 9.840 | +0.020 |
| fp16-wide-d32768-r2 | 9.940 | 16.900 | +6.920 |
| fp16-wide-d32768-r3 | 9.980 | 24.000 | +13.840 |
| fp16-wide-d32768-r4 | 14.720 | 17.580 | +7.560 |
| fp16-wide-d32768-r6 | 10.700 | 24.400 | +13.920 |
| fp16-wide-d32768-r8 | 27.100 | 28.240 | +11.100 |
| fp16-wide-d32768-r12 | 17.860 | 24.340 | +6.640 |
| bf16-tail-d12288-r2 | 18.980 | 7.980 | -10.600 |
| bf16-tail-d12288-r8 | 32.320 | 44.500 | +0.100 |
| bf16-wide-d32768-r1 | 13.780 | 13.860 | -0.020 |
| bf16-wide-d32768-r2 | 14.060 | 24.680 | +10.520 |
| bf16-wide-d32768-r3 | 13.880 | 35.800 | +21.760 |
| bf16-wide-d32768-r4 | 15.100 | 24.840 | +10.080 |
| bf16-wide-d32768-r6 | 17.820 | 36.260 | +21.340 |
| bf16-wide-d32768-r8 | 32.900 | 73.080 | +25.660 |
| bf16-wide-d32768-r12 | 45.220 | 66.980 | +19.920 |

负载上下文：开测和结束均为 `npu-smi info`；device 0 为 910B3，HBM 为 39044/65536 MB，AICore 为 0%，且 device 0 存在 VLLMEngineCor 进程。原始快照为 `load-before-device0.txt` 和 `load-after-device0.txt`。

## RESULT

H3 Correctness 通过，但 Local 不支持三行/核收益。新增宽行 Rows=3/6/12 在 FP16、BF16 中配对差值全部为正；原有 Rows=2/4/8 也全部为正。控制组方向混合，保留原始结果，不据此归因。

`LOCAL_SCORE=NONE`。本轮没有真实性能支持，因此不创建本地 revision，不提交 V003 候选。

## COMMIT

PUSH=NO；Official=NO；COMMIT SHA=NONE（结果不支持提交）。

## NEXT

NONE。V001/V002/V003 已覆盖两行、三行、四行/核，当前没有新的证据支持继续重复行组粒度实验。
