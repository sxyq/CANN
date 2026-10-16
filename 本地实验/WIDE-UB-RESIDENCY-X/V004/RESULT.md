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

COMMAND: existing `paired_runner_v016` CMake configure and build on `cann-server3`.
ENVIRONMENT: CANN 8.5.0.alpha002; `ASCEND_HOME_PATH`, `CPATH`, and
`CPLUS_INCLUDE_PATH` set to the recorded system/toolkit paths.
RETURN_CODE: 0
RESULT: PASS; `[100%] Built target paired_runner_v016`.
EVIDENCE: `configure.log`, `compile-final.log`. The initial missing
`ASCEND_HOME_PATH` failure and the following missing `<vector>` failure remain
in `compile.log` and `compile-rerun.log`.

## CORRECTNESS / LOCAL

STATUS: NOT_RUN; NPU execution waits for Main's device arrangement.

## RESULT

STATUS: COMPILE_PASS_DEVICE_DATA_PENDING
OFFICIAL: NOT_RUN
PUSH: NO
