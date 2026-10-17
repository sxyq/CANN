# WIDE-UB-RESIDENCY-X V013

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V013
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
PARENT_SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
SOURCE_COMMIT: `36504788249e45dffa07793d5279039f32ccfb8d`

## HYPOTHESIS

For BF16 D=32768, V011 retains two input tiles for Pass1 and two parameter
tiles for Pass2. Using one staging slot per input and parameter stream lowers
that staging footprint by 32,768 bytes. The existing UB planner then grows the
tile from 2048 to 4096 elements. This changes Pass1/Pass2 GM tile traffic and
staging lifetime; it does not change row-to-core assignment, arithmetic, or
output-buffer ownership.

## SOURCE CHANGE

- Enable one-slot staging only for BF16 D=32768.
- Keep two-slot Pass1 and Pass2 staging for other dtypes and widths.
- Retain V011 BF16 full-y storage and `outputLocal = gammaLocal`; no FP16
  retained-y/output reuse is present.
- BF16 D=32768 tile count changes from 16 to 8.
- Planned UB use is 180,288 bytes, within the 177 KiB (181,248 byte) planner
  budget.

## SERVER COMPILE

SERVER: `cann-server3` (`hwnput3`)
UTC: `2026-10-16T20:30:25Z`
SOC_VERSION: Ascend910B3
DEVICE_ID: 2
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V013/submission.asc`
CONFIGURE_COMMAND: `cmake -S .../本地实验/R31B/V016/support/paired -B .../V013/build -DCMAKE_BUILD_TYPE=Release -DR31B_V011_SOURCE_DIR=.../线上结果/R31B/V011 -DR31B_V016_SOURCE_DIR=.../本地实验/WIDE-UB-RESIDENCY-X/V013`
CONFIGURE_RETURN_CODE: 0
BUILD_COMMAND: `cmake --build .../V013/build --target paired_runner_v016 --parallel 1 -- -B`
BUILD_RETURN_CODE: 0
RUNTIME_LIBRARY: `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`; CANN libraries
resolved from the existing toolkit.
COMPILE: PASS.
COMPILE_LOG: `server-transaction-20261017/compile.log`.

## CORRECTNESS AND LOCAL

RUNNER: existing `paired_runner_v016 --paired --device 2`
WARMUP: 45 per binary.
SAMPLES: 21 interleaved Parent/Candidate pairs per case.
PREFLIGHT: HBM usage 5%, AICore 0%, no process on NPU2.
POSTFLIGHT: HBM usage 5%, no process on NPU2.
RUNNER_RETURN_CODE: 0.
CORRECTNESS: Parent and Candidate PASS for all four cases.

Times below use device-event microseconds. Delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Delta median | Delta P10–P90 |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 7.660 | 7.340 | +0.040 | -1.960..+1.240 |
| FP16 D=32768 | 24.760 | 24.880 | -0.360 | -4.860..+4.000 |
| BF16 D=12288 | 7.320 | 6.840 | -0.340 | -13.060..+0.360 |
| BF16 D=32768 | 14.040 | 14.000 | -0.140 | -0.340..+0.200 |

All paired P10–P90 intervals cross zero; this run shows no stable Local gain.
`LOCAL_SCORE=NONE`.

| Case | Parent correctness | Candidate correctness | Max absolute error | Max relative error |
|---|---|---|---:|---:|
| FP16 D=12288 | PASS | PASS | 0.0009765625 | 0.0015923567 |
| FP16 D=32768 | PASS | PASS | 0.001953125 | 0.010989011 |
| BF16 D=12288 | PASS | PASS | 0.0078125 | 0.0073529412 |
| BF16 D=32768 | PASS | PASS | 0.0078125 | 0.01754386 |

RAW_RUN_LOG: `server-transaction-20261017/paired-run.log`.
DEVICE_SNAPSHOT: `server-transaction-20261017/device-before-run.snapshot`.
DEVICE_RESET: NO.
PUSH: NO.

## NEXT

No further V013 runs are planned from this result. Choose the next BF16 wide-row
UB/GM mechanism only after the active-route audit confirms it does not reuse
the V013 single-stage staging path.
