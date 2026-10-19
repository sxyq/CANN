# WIDE-UB-RESIDENCY-X V014

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V014
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
PARENT_SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
SOURCE_COMMIT: `ee2dacd8`
PUSH: NO

## HYPOTHESIS

V014 tests a GM transaction change independent of V012/N05 retained-y or
outputLocal reuse and independent of V013 single-slot staging. The V011
two-slot wide-row planner, full-row residency, arithmetic, event flow, and
output store remain unchanged.

The existing `Load()` helper used `DataCopyPad` for every GM-to-UB read. V014
uses the existing `DataCopy(LocalTensor, GlobalTensor, count)` overload only
for BF16 wide-path reads whose element offset and count are both multiples of
16, which makes both GM address and byte count 32-byte aligned. Non-aligned
rows, tails, all FP16/FP32 reads, narrow paths, and every store keep the
existing `DataCopyPad` path.

## SOURCE CHANGE

- Source: `本地实验/WIDE-UB-RESIDENCY-X/V014/submission.asc:3378`
- New transaction branch: `widePath_ && BF16 && offset/count % 16 == 0`.
- No `InitBuffer` size, tile width, row batch, retained-y layout, output
  buffer ownership, block assignment, or Runner change.
- The V014 source was transferred to the server with SHA-256
  `8ec99660ab5a8185532e3fc5869e634eacbcdda6e53789cc91ef58a9f2c97358`.

## SERVER COMPILE

SERVER: `cann-server3` (`hwnput3`)
UTC: `2026-10-16T21:23:00Z`
SOC_VERSION: `Ascend910B3`
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V014/submission.asc`
SOURCE_LINES: `3567`
CANN_ENV: `/usr/local/Ascend/ascend-toolkit/set_env.sh`
CPLUS_INCLUDE_PATH: recorded in `server-transaction-20261017/compile.log`
RUNTIME_LIBSTDCPP: `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`

CONFIGURE_COMMAND:

```text
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/R31B/V016/support/paired -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V014/build -DCMAKE_BUILD_TYPE=Release -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/线上结果/R31B/V011 -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V014
```

CONFIGURE_RETURN_CODE: `0`
BUILD_COMMAND: `cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V014/build --target paired_runner_v016 --parallel 1 -- -B`
BUILD_RETURN_CODE: `0`
COMPILE: PASS
COMPILE_LOG: `server-transaction-20261017/compile.log`

The existing paired Runner binding was changed in the server transaction copy
to parent V011 and candidate V014. No new Runner or build script was created.

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
| FP16 D=12288 | 13.320 | 7.160 | -0.540 | -17.940..16.080 |
| FP16 D=32768 | 10.700 | 10.400 | -0.420 | -0.820..0.460 |
| BF16 D=12288 | 8.020 | 8.240 | +0.020 | -1.340..14.100 |
| BF16 D=32768 | 14.660 | 13.180 | -1.420 | -1.920..-1.180 |

CORRECTNESS: Parent and Candidate PASS for all four cases. The reported
maximum errors are unchanged from the paired V011 reference:

| Case | Max absolute error | Max relative error |
|---|---:|---:|
| FP16 D=12288 | 0.0009765625 | 0.0015923567 |
| FP16 D=32768 | 0.001953125 | 0.010989011 |
| BF16 D=12288 | 0.0078125 | 0.0073529412 |
| BF16 D=32768 | 0.0078125 | 0.01754386 |

The target BF16 D=32768 case has a negative paired interval in this run.
The other three intervals include zero, so this is recorded as a target-case
Local signal rather than a suite-wide score.

LOCAL_TARGET: BF16 D=32768, candidate median `13.180 us`, delta median
`-1.420 us`, P10-P90 `-1.920..-1.180 us`
LOCAL_SCORE: `NONE` for the full four-case suite
DEVICE_BEFORE: HBM usage `5%`, NPU2 process `none`
DEVICE_AFTER: HBM usage `5%`, NPU2 process `none`
DEVICE_RESET: NO

RAW_RUN_LOG: `server-transaction-20261017/paired-run.log`
DEVICE_BEFORE_SNAPSHOT: `server-transaction-20261017/device-before-run.snapshot`
DEVICE_AFTER_SNAPSHOT: `server-transaction-20261017/device-after-run.snapshot`
PUSH: NO
SHARED_RECORD: UNCHANGED

## NEXT EXPERIMENT

Do not repeat V014 or run another non-aligned tail variation. The next N02
experiment, if continued, should first search the owned parameter-load and UB
allocation code again and select a mechanism that changes parameter residency
or a multi-block GM transaction without reusing V012/N05 retained-y or
outputLocal behavior, V013 single-slot staging, or the V014 aligned
`DataCopy` branch. It should use the same paired Runner and stop if no
independent code entry exists.

STATUS: SERVER_VALIDATED_CORRECTNESS_LOCAL_TARGET_SIGNAL
