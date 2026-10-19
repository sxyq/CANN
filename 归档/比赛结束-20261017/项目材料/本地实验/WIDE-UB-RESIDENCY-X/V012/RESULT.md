# WIDE-UB-RESIDENCY-X V012

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V012
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
SOURCE_COMMIT: `8b5aec67249e45dffa07793d5279039f32ccfb8d`

## HYPOTHESIS

V012 attempted to reuse the retained FP16 `y` tile as the Pass2 output tile and
remove the separate wide-path output allocation. This was intended to lower UB
use and raise the FP16 tile width for D=32768.

## SERVER TRANSACTION

SERVER: `cann-server3` (`hwnput3`)
SOC_VERSION: Ascend910B3
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V012/submission.asc`
CONFIGURE: existing `paired_runner_v016` CMake project; return code 0.
BUILD: target `paired_runner_v016`; return code 0.
RUNTIME_LIBRARY: default `ldd` selected toolkit `libstdc++.so.6`; the recorded
system-GCC11-first `LD_LIBRARY_PATH` resolved `libstdc++.so.6` from
`/usr/lib/aarch64-linux-gnu`.
COMPILE: PASS.
COMPILE_LOG: `server-transaction-20261017/compile.log`.

## EXECUTION STATUS

After compile, N07's read-only route audit identified retained-y/output-tile
reuse as overlapping another active route. The candidate was therefore kept as
historical source and no device Runner was launched.

CORRECTNESS: NOT_RUN.
LOCAL: NOT_RUN.
LOCAL_SCORE: NONE.
DEVICE_ID: NOT_RUN.
DEVICE_RESET: NO.
PUSH: NO.

## NEXT

V013 uses a BF16 single-stage input/parameter staging path without changing
the FP16 retained-y output target.
