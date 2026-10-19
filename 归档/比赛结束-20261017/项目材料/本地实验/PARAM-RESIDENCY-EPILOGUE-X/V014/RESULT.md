# PARAM-RESIDENCY-EPILOGUE-X V014

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V014
PARENT=V012
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V014/R31B-V016-WIDE-TILE-SEED-V014_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=03c91580eb27c016044fb8c937c94162526c11c1
PUSH=NO
OFFICIAL=NOT_RUN

## N07 READ-ONLY AUDIT FOLLOW-UP

V013 的 FP16 retained-y/outputLocal 复用机制与另一条活动路线重叠。V013 后续服务器事务已停止；V013 源码和已生成的远端材料保留为未提交内容。本版本从 V012 源码快照重新选择独立机制，不包含 V013 的 retained-y/outputLocal 改动。

## HYPOTHESIS

`ProcessWideLowPrecision` Pass 2 已使用 `prd0`/`prd1` 参数就绪事件和 `prel0`/`prel1` 槽位释放事件。原顺序在每个 tile 入口先等待当前 Gamma/Bias MTE2 完成，再发出下一 tile 的参数 Load。V014 将当前 `WaitFlag<MTE2_V>(prd)` 移到下一槽 Gamma/Bias Load 与 `SetFlag<MTE2_V>` 之后、当前 Gamma/Bias 首次被 Vector 读取之前。这样下一参数槽的 DMA 可以更早开始，当前 epilogue 仍使用同一槽、同一数据和同一写回次序。

## CHANGE

- 只调整宽低精度 Pass 2 的参数就绪等待位置。
- 保留 `prel0`/`prel1` 槽位释放等待、参数双槽、Gamma/Bias Load 地址、FP16/BF16 运算次序、MTE3 Store 和尾部事件排空。
- 不修改 retained-y 存储布局，不修改 output buffer 选择，不引入输出双槽写回。
- 沿用已有 `paired_runner_v016`、CMake、ABI、Runner 和 CANN/GLIBCXX 环境处理；没有新增脚本。

## COMPILE

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_ID=3
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
CONFIGURE_COMMAND=`set +u; source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh; set -u; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}; cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/support -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/src`
BUILD_COMMAND=`cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/build --target paired_runner_v016 --parallel 1`
CONFIGURE_WARNING=Existing CMake runtime search-path warning involving libstdc++.so.6; configure completed.
REMOTE_BINARY=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/build/paired_runner_v016
REMOTE_BINARY_BYTES=582472

Evidence: `remote/compile-env-v014.txt`, `remote/configure-v014.log`, `remote/configure-v014.rc`, `remote/compile-v014.log`, `remote/compile-v014.rc`, `remote/source-identities-local.txt`, `remote/source-binary-identities-v014.txt`.

## CORRECTNESS AND LOCAL

RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; warmup_each=45; pairs_each=21; device-event timing primary; wall time diagnostic`
CORRECTNESS=PASS_8_OF_8
LOCAL=MEASURED
SAMPLE_COUNT=21 pairs per case
LOCAL_SCORE=BF16_WIDE_SINGLE_RUN_SIGNAL

| Case | Parent median us | Candidate median us | Paired delta median us | Paired delta P10 us | Paired delta P90 us |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 7.340 | 7.320 | +0.420 | -1.580 | 20.180 |
| fp16-wide-d32768 | 10.680 | 10.520 | -0.320 | -0.960 | 0.360 |
| bf16-tail-d12288 | 7.040 | 6.840 | -0.120 | -0.720 | 0.580 |
| bf16-wide-d32768 | 14.360 | 13.640 | -0.760 | -1.340 | -0.400 |

INTERPRETATION=All four cases passed Parent and Candidate correctness. BF16-wide is the only case whose paired P10-P90 interval stayed below zero in this run; FP16-wide is near neutral, and both tail cases include large outliers or intervals crossing zero. Treat V014 as a parameter-DMA ordering result with one BF16-wide signal, not a broad performance conclusion.

## DEVICE AND EVIDENCE

PRE_HBM_USAGE=5% of 65536 MiB
POST_HBM_USAGE=5% of 65536 MiB
PRE_AICORE=0%
POST_AICORE=0%
PRE_AIVECTOR=0%
POST_AIVECTOR=0%
DEVICE_TEMPERATURE_PRE=25 C
DEVICE_TEMPERATURE_POST=25 C
OTHER_PROCESS_PRESENT=Other user processes were visible in the saved process snapshots; no other process or service was stopped or modified.
PROCESS_QUERY_NOTE=`npu-smi info proc -i 3` is unsupported by this npu-smi version; its raw output is retained.
DISK_AVAIL_PRE=259G on /home/data4t2
DISK_AVAIL_POST=259G on /home/data4t2
RAW_REMOTE_LOGS=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/logs/
RAW_LOCAL_LOGS=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V014/remote/

Evidence: `remote/paired-v014.stdout.log`, `remote/paired-v014.stderr.log`, `remote/paired-v014.rc`, `remote/pre-run-device-v014.txt`, `remote/post-run-device-v014.txt`, `remote/pre-run-processes-v014.txt`, `remote/post-run-processes-v014.txt`, `remote/run-env-v014.txt`.

## RESULT

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_NOT_RUN
ONLINE_SUBMIT=NOT_RUN

## COMMIT

SOURCE_COMMIT=03c91580eb27c016044fb8c937c94162526c11c1
PUSH=NO

## NEXT EXPERIMENT

Repeat V014 with the same paired Runner and four cases on NPU3, keeping the parameter-wait relocation unchanged. Use BF16-wide as the primary signal and retain FP16-wide plus both D=12288 cases as controls. Do not continue V013.

## REPEAT SERVER TRANSACTION 2026-10-17

REPEAT_REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014
REPEAT_DEVICE_ID=3
REPEAT_RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/build/paired_runner_v016 --paired 3`
REPEAT_RUNNER_RC=0
REPEAT_METHOD=`existing paired_runner_v016; same V014 binary; warmup_each=45; pairs_each=21; device-event timing primary; wall time diagnostic`
REPEAT_CORRECTNESS=PASS_8_OF_8
REPEAT_LOCAL=MEASURED
REPEAT_SAMPLE_COUNT=21 pairs per case
REPEAT_LOCAL_SCORE=BF16_WIDE_REPEAT_SIGNAL_WITH_OUTLIER

| Case | Parent median us | Candidate median us | Paired delta median us | Paired delta P10 us | Paired delta P90 us |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 8.160 | 7.220 | -0.580 | -23.220 | 7.760 |
| fp16-wide-d32768 | 10.740 | 10.460 | -0.320 | -14.360 | 0.420 |
| bf16-tail-d12288 | 7.380 | 7.360 | -0.020 | -0.980 | 13.220 |
| bf16-wide-d32768 | 14.440 | 13.700 | -0.840 | -11.820 | -0.220 |

REPEAT_INTERPRETATION=All four cases again passed Parent and Candidate correctness. BF16-wide again stayed below zero at the paired median and its P10-P90 interval remained below zero, but the Parent stream contained a 119.840 us device-time outlier and the paired delta range was -106.640 to +10.220 us. FP16-wide remained near neutral; the two tail controls were noisy. The repeat supports retaining BF16-wide as the next observation target, but does not support a broad performance claim.

REPEAT_PRE_HBM_USAGE=5% of 65536 MiB
REPEAT_POST_HBM_USAGE=5% of 65536 MiB
REPEAT_PRE_AICORE=0%
REPEAT_POST_AICORE=0%
REPEAT_PRE_AIVECTOR=0%
REPEAT_POST_AIVECTOR=0%
REPEAT_PRE_TEMPERATURE=29 C
REPEAT_POST_TEMPERATURE=30 C
REPEAT_DISK_AVAIL_PRE=259G on /home/data4t2
REPEAT_DISK_AVAIL_POST=259G on /home/data4t2
REPEAT_PROCESS_QUERY_NOTE=`npu-smi info proc -i 3` is unsupported by this npu-smi version; raw output retained.

REPEAT_EVIDENCE=`remote/pre-rerun-v014.log`, `remote/pre-rerun-v014.rc`, `remote/rerun-20261017T123900Z.stdout.log`, `remote/rerun-20261017T123900Z.stderr.log`, `remote/rerun-20261017T123900Z.rc`, `remote/rerun-20261017T123900Z-pre-device.txt`, `remote/rerun-20261017T123900Z-post-device.txt`, `remote/rerun-20261017T123900Z-pre-processes.txt`, `remote/rerun-20261017T123900Z-post-processes.txt`, `remote/rerun-20261017T123900Z-env.txt`

## UPDATED RESULT

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_REPEAT_MEASURED_OFFICIAL_NOT_RUN
ONLINE_SUBMIT=NOT_RUN

## NEXT EXPERIMENT

Repeat the unchanged V014 binary once more only if NPU3 has a comparable idle window; keep BF16 D32768 primary and retain all four paired controls. Preserve both previous raw runs. Do not continue V013.
