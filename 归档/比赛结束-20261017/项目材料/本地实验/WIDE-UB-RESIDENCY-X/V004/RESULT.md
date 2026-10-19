# WIDE-UB-RESIDENCY-X V004

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V004
PARENT: 线上结果/R31B/V011/submission.asc
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

The V011 FP16 Pass1 path stores the complete half `y` row in UB, then uses
`Muls(yTile, xLocal, 1)` to copy each completed tile. For full 4096-element
tiles, replacing that identity multiply with the LocalTensor `DataCopy` already
used by the V003 path may remove vector arithmetic without changing GM reads,
UB capacity, reduction order, or output math. Partial final tiles retain the
existing `Muls` operation.

## ONE CHANGE

- In the FP16 branch of `ProcessWideLowPrecision`, use LocalTensor `DataCopy`
  for full tiles when writing `yTile`; retain the existing identity `Muls` for
  a partial tile.
- Full-row residency, tile size, GM load order, reduction, synchronization,
  parameter staging, and output path are unchanged.

## SOURCE ANALYSIS

Logical GM payload per row for FP16:

- V011 Pass1 reads `x` and `residual`: `4 * D` bytes.
- V011 Pass2 reads `gamma` and `bias`: `4 * D` bytes; it does not reload
  `x` or `residual`.
- V003 adds Pass2 `x`/`residual` reads for all but one cached tile per block.
  With one row per block, this is 112 KiB at D=32768 and 32 KiB at D=12288.

V011 FP16 UB allocation from `Init`:

- D=32768 selects one retained row: 64 KiB for `y`, 32 KiB input staging,
  8 KiB output staging, 48 KiB FP32 work buffers, and 64 bytes reduction
  storage; total 152 KiB plus 64 bytes.
- D=12288 selects three retained rows: 72 KiB for `y`, 32 KiB input staging,
  8 KiB output staging, 48 KiB FP32 work buffers, and 192 bytes reduction
  storage; total 160 KiB plus 192 bytes.
- Both totals fit the source's 176 KiB budget. The row choice includes the
  reduction partial storage.

These are logical source-level byte counts, not measured physical GM
transactions.

## COMPILE

COMMAND: configure the existing paired-runner CMake project, then build only
`paired_runner_v016` on `cann-server3`.
ENVIRONMENT: CANN 8.5.0.alpha002; sourced
`/usr/local/Ascend/ascend-toolkit/set_env.sh`; set the recorded system/toolkit
`CPATH` and `CPLUS_INCLUDE_PATH` values; system library directory precedes
toolkit runtime libraries in `LD_LIBRARY_PATH`.
RETURN_CODE: 0 for configure and final target build. The first target build
without `CPLUS_INCLUDE_PATH` failed because `<vector>` was not found; it did
not start the Runner. The subsequent build with the existing recorded include
paths passed.
RESULT: PASS; `[100%] Built target paired_runner_v016`.
RUNTIME_LINK: `ldd` resolved `libgraph.so` and all listed shared libraries;
`libgraph.so` resolved under `/usr/local/Ascend/ascend-toolkit/latest/lib64`.
EVIDENCE: `configure-rerun.log`, `compile-rerun-envfix.log`, and
`ldd-rerun.log`. Earlier attempts remain in `configure.log`, `compile.log`,
`compile-rerun.log`, `compile-final.log`, `configure-run.log`, and
`compile-run.log`.

## PAIRED RUNNER

- Added the optional `--device N` argument; the default remains device 4.
- Removed `aclrtResetDevice`; exit destroys events and stream, then calls
  `aclFinalize` without resetting any device.
- Reused the existing Runner and ran
  `paired_runner_v016 --paired --device 7` once, with CANN environment loaded.
- Runner return code: 0; all four configured cases completed with
  `RUNNER_COMPLETE ... correctness=PASS`.
- The original `paired.log` remains the earlier exit-127 `libgraph.so` loading
  failure; that invocation did not start the Runner. This run sourced the
  recorded CANN environment, and `ldd` resolved `libgraph.so`.

## CORRECTNESS / LOCAL

All cases use rows=2. Parent is R31B V011; Candidate is V004. Each case has
45 warmups per binary and 21 interleaved timing pairs (21 device-event samples
per binary). Paired delta is Candidate minus Parent; times are microseconds.

| Case | Parent correctness | V004 correctness | Parent median | V004 median | Paired delta median | Paired delta p10..p90 |
|---|---|---|---:|---:|---:|---:|
| fp16-tail-d12288 | PASS | PASS | 9.720 | 16.620 | -0.140 | -4.200..9.820 |
| fp16-wide-d32768 | PASS | PASS | 18.000 | 17.460 | -0.280 | -9.520..4.360 |
| bf16-tail-d12288 | PASS | PASS | 17.760 | 16.820 | -0.460 | -11.040..1.700 |
| bf16-wide-d32768 | PASS | PASS | 16.680 | 16.080 | 0.160 | -4.940..2.700 |

Correctness had zero failures for both binaries in all cases. Maximum absolute
error was 0.0009765625 (FP16 D=12288), 0.001953125 (FP16 D=32768), and
0.0078125 (both BF16 cases); maximum relative errors were respectively
0.0015923567, 0.010989011, 0.0073529412, and 0.01754386.

The device-event distributions were noisy. For FP16 D=12288, V004's separate
median is 71.0% slower than V011, while the paired delta median is -0.140 us
and its p10..p90 spans -4.200..9.820 us. This disagreement and spread provide
no stable speedup evidence. The other three paired intervals also span zero;
their paired delta medians are within 0.460 us of zero. The observed Local
results therefore do not show a repeatable performance benefit from replacing
the full-tile identity multiply with LocalTensor `DataCopy`. They remain
single-run observations, not an Official result.

RAW_SAMPLES: `paired-run.log` (complete Runner output and per-pair samples).
DEVICE_SNAPSHOTS: `device-before-run.*`, `device-after-run.*`,
`paired-run-start.time`, and `paired-run-end.time`.

DEVICE: NPU7 on `hwnput3` (Ascend910B3); Main reallocation began
2026-10-16T15:05:14+08:00. Before the single Runner invocation at
2026-10-16T15:55:03+08:00: health OK, HBM 5%, AICore 0%, AIVector 0%,
no running process. After completion at 2026-10-16T15:55:10+08:00: health OK,
HBM 5%, AICore 0%, no running process. NPU7 was released at
2026-10-16T15:55:10+08:00. DEVICE_RESET=NO; OTHER_PROCESS_ACTION=NONE.
The device's `npu-smi info proc` query is unsupported; the full device listing
reported no running process on NPU7 before and after the run. Main's shared
device event was marked RELEASED at 2026-10-16T15:55:10+08:00.

## RESULT

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NO_STABLE_GAIN
OFFICIAL: NOT_RUN
PUSH: NO
