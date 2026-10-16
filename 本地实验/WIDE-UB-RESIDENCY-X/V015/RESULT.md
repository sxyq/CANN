# WIDE-UB-RESIDENCY-X V015

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V015
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
PARENT_SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
SOURCE_COMMIT: `4b6af99c`
PUSH: NO
SHARED_RECORD: UNCHANGED

## SEARCH AND HYPOTHESIS

The owned N02 sources have one shared parameter-load entry: V011
`ProcessWideLowPrecision()` calls `Load()` for the alternating gamma/bias
tiles in Pass2. Existing calls used one GM block per staging slot. No existing
N02 multi-block parameter call was present, so V015 extends this existing
entry with one mechanism only.

For BF16 D=32768, two adjacent parameter tiles fit the existing two staging
slots. V015 uses `DataCopyPad` with
`DataCopyExtParams{blockCount=2, blockLen=tileBytes, srcStride=0,
dstStride=0}` for gamma and bias, then consumes the two slots in sequence.
The candidate starts from V011, so V014's single-block aligned `DataCopy`
branch is not included. UB sizes, y residency, output ownership, input loads,
stores, arithmetic, and all other widths/dtypes remain V011.

SOURCE_ENTRY:

- Parameter calls: `submission.asc:3272-3306`
- Existing load helper: `submission.asc:3374-3381`
- New pair helper: `submission.asc:3403-3412`
- Target condition: BF16, `rowWidth == 32768`, even tile count.

## SERVER COMPILE

SERVER: `cann-server3` (`hwnput3`)
HOST: `hwnput3`
UTC: `2026-10-16T21:33:43Z`
SOC_VERSION: `Ascend910B3`
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V015/submission.asc`
SOURCE_LINES: `3592`
SOURCE_SHA256: `41f2dde4c2bb0eb70e1e8df7a99fc8bec53273ce84a1f1835e2fe530478dd4fc`
CANN_ENV: `/usr/local/Ascend/ascend-toolkit/set_env.sh`
CPLUS_INCLUDE_PATH: recorded in `server-transaction-20261017/compile.log`
RUNTIME_LIBSTDCPP: `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`

CONFIGURE_COMMAND:

```text
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/R31B/V016/support/paired -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V015/build -DCMAKE_BUILD_TYPE=Release -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/线上结果/R31B/V011 -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V015
```

CONFIGURE_RETURN_CODE: `0`
BUILD_COMMAND: `cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V015/build --target paired_runner_v016 --parallel 1 -- -B`
BUILD_RETURN_CODE: `0`
COMPILE: PASS
COMPILE_LOG: `server-transaction-20261017/compile.log`

The existing `paired_runner_v016` binding was changed only in the server
transaction copy to parent V011 and candidate V015.

## CORRECTNESS AND LOCAL

RUNNER: existing `paired_runner_v016`
RUNNER_COMMAND: `paired_runner_v016 --paired --device 2`
WARMUP: `45` per binary
PAIRS: `21` interleaved Parent/Candidate pairs per case
DEVICE_ID: `2`
RUNNER_RETURN_CODE: `0`
RUNNER_COMPLETE: `PASS`

| Case | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10-P90 (us) |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 7.740 | 7.600 | -0.200 | -15.920..6.540 |
| FP16 D=32768 | 10.960 | 11.240 | -0.340 | -1.480..1.200 |
| BF16 D=12288 | 8.800 | 8.100 | -0.760 | -4.560..1.520 |
| BF16 D=32768 | 18.660 | 13.120 | -1.140 | -10.060..9.080 |

CORRECTNESS: Parent and Candidate PASS for all four cases.

| Case | Max absolute error | Max relative error |
|---|---:|---:|
| FP16 D=12288 | 0.0009765625 | 0.0015923567 |
| FP16 D=32768 | 0.001953125 | 0.010989011 |
| BF16 D=12288 | 0.0078125 | 0.0073529412 |
| BF16 D=32768 | 0.0078125 | 0.01754386 |

The target BF16 D=32768 median was lower by `1.140 us`, but its P10-P90
interval crosses zero. No stable Local gain is recorded.

LOCAL_TARGET: BF16 D=32768, delta median `-1.140 us`, P10-P90
`-10.060..9.080 us`
LOCAL_SCORE: `NONE`
DEVICE_BEFORE: HBM usage `5%`, NPU2 process `none`
DEVICE_AFTER: HBM usage `5%`, NPU2 process `none`
DEVICE_RESET: NO

RAW_RUN_LOG: `server-transaction-20261017/paired-run.log`
DEVICE_BEFORE_SNAPSHOT: `server-transaction-20261017/device-before-run.snapshot`
DEVICE_AFTER_SNAPSHOT: `server-transaction-20261017/device-after-run.snapshot`

## NEXT

Do not repeat V015 or vary the non-aligned tail path. The multi-block
parameter transaction is Correctness-safe in this run but has no stable Local
signal, so this mechanism stops here. A later N02 round must search the owned
parameter/UB code again and select a different entry; if no clear independent
mechanism exists, record the conclusion and stop.

STATUS: SERVER_VALIDATED_CORRECTNESS_LOCAL_NO_STABLE_GAIN
