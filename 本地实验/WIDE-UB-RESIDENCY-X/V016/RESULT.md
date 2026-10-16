# WIDE-UB-RESIDENCY-X V016

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V016
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
PARENT_SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
SOURCE_COMMIT: `4a35ca9987044b944bf2fef676db410232cbe40e`
PUSH: NO
SHARED_RECORD: UNCHANGED

## SEARCH AND HYPOTHESIS

The owned V011 source already contained an unused `ProcessWideFp32PanelResident()`
entry. Its schedule keeps one channel panel in UB, performs a first reduction
sweep, then loads one gamma/bias panel while materializing the rows. No V010 to
V015 result used this entry for BF16.

V016 applies that existing schedule shape to BF16 D=32768 only:

- First sweep keeps a 6144-element FP32 panel and one reduction partial per row.
- Second sweep rereads x/residual for the current panel, normalizes it, and
  keeps one converted gamma/bias panel resident while applying it to the rows.
- A separate BF16 output buffer is used after the FP32 affine operation.
- Input staging remains two-slot. This does not reuse the V013 single-slot
  staging path or the V012 retained-y/output-buffer path.

The D=32768 allocation is 159,808 bytes:
`x=24,576`, `residual=24,576`, `output=12,288`, `value=49,152`,
`xFp32=24,576`, `residualFp32=24,576`, `reduce=64`.
There are six panels per row; the final panel has 2,048 BF16 elements and
4,096 bytes.

## SERVER COMPILE

SERVER: `cann-server3` (`hwnput3`)
SOC_VERSION: `Ascend910B3`
CANN_ENV: `/usr/local/Ascend/ascend-toolkit/set_env.sh`
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V016/submission.asc`
RUNNER_BINDING: existing `paired_runner_v016`; the server transaction copy was
updated to include V016, recorded in `server-transaction-20261017/runner-binding.log`.

The first compile attempt returned 0 while the existing server runner copy still
included V015. It is retained as
`server-transaction-20261017/compile-v015-binding-attempt.log` and is not used
as V016 compile evidence. The final transaction used the V016 binding.

CONFIGURE_COMMAND:

```text
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/R31B/V016/support/paired -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V016/build -DCMAKE_BUILD_TYPE=Release -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/线上结果/R31B/V011 -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V016
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V016/build --target paired_runner_v016 --parallel 1 -- -B
```

CONFIGURE_RETURN_CODE: `0`
BUILD_RETURN_CODE: `0`
COMPILE: PASS
COMPILE_LOG: `server-transaction-20261017/compile.log`
RUNTIME_LIBSTDCPP: `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`

## CORRECTNESS AND LOCAL

RUNNER: existing `paired_runner_v016`
RUNNER_COMMAND: `paired_runner_v016 --paired --device 2`
WARMUP: `45` per binary
PAIRS: `21` interleaved Parent/Candidate pairs per case
RUNNER_RETURN_CODE: `0`
RUNNER_COMPLETE: PASS

Delta is Candidate minus Parent. Times are device-event microseconds.

| Case | Parent median | Candidate median | Delta median | Delta P10-P90 |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 22.160 | 21.920 | -0.280 | -4.440..1.700 |
| FP16 D=32768 | 27.820 | 27.760 | -0.880 | -14.240..7.280 |
| BF16 D=12288 | 7.900 | 7.540 | -0.160 | -13.940..13.460 |
| BF16 D=32768 | 26.520 | 22.780 | +0.100 | -11.460..13.080 |

CORRECTNESS: Parent and V016 passed all four cases.

| Case | Max absolute error | Max relative error |
|---|---:|---:|
| FP16 D=12288 | 0.0009765625 | 0.0015923567 |
| FP16 D=32768 | 0.001953125 | 0.010989011 |
| BF16 D=12288 | 0.0078125 | 0.0073529412 |
| BF16 D=32768 | 0.0078125 | 0.01754386 |

All four paired P10-P90 intervals cross zero. The target BF16 D=32768 delta
median is `+0.100 us`, so this mechanism has no stable Local gain.

LOCAL_TARGET: BF16 D=32768, candidate median `22.780 us`, delta median
`+0.100 us`, P10-P90 `-11.460..13.080 us`
LOCAL_SCORE: `NONE`
RAW_RUN_LOG: `server-transaction-20261017/paired-run.log`
DEVICE_BEFORE: `server-transaction-20261017/device-before-run.snapshot`
DEVICE_AFTER: `server-transaction-20261017/device-after-run.snapshot`
DEVICE_BEFORE_FACTS: HBM `5%`, AICore `0%`, no process on NPU2, Health `OK`
DEVICE_AFTER_FACTS: HBM `5%`, AICore `0%`, no process on NPU2, Health `OK`
DEVICE_RESET: NO

## NEXT EXPERIMENT

Stop V016. Do not repeat the panel schedule or combine it with the V012,
V013, V014, or V015 mechanisms. A later N02 round needs a new search of the
owned parameter and UB entries before any further version is started.

STATUS: SERVER_VALIDATED_CORRECTNESS_LOCAL_NO_STABLE_GAIN
