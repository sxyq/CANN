# WIDE-UB-RESIDENCY-X V005

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V005
PARENT: 本地实验/WIDE-UB-RESIDENCY-X/V004/submission.asc
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

For BF16 in `ProcessWideLowPrecision` Pass2, each loaded `gamma/bias` UB tile
is converted to FP32 twice by two identical consecutive `ToFloat` pairs. The
second pair rewrites the same FP32 work tiles with the same values immediately
before their use. `ToFloat` instantiates `AscendC::Cast` for BF16. Removing the
second pair should reduce redundant UB traffic and vector conversion work
without changing GM reads, output math, allocation size, row tiling, or
synchronization between loads and output.

## ONE CHANGE

- Remove the repeated BF16 `gamma/bias` `ToFloat` pair in the Pass2 tile loop.
- The compile-time FP16 path remains unchanged; no partial-tile condition is
  involved.

## ACCESS EVIDENCE

Target case: BF16, rows=2, D=32768; each row is eight full 4096-element tiles.
The existing paired Runner passes `availableCoreNum=8`; `run_kernel` clamps the
launch to two blocks for two rows, so each block handles one row.

Pass2 loads one gamma tile and one bias tile from GM into the local staging
buffers, then converts those same BF16 local tiles before applying them to the
retained row. The duplicate pair performs, per tile, two extra 4096-element
BF16 source reads from UB (16,384 bytes) and two FP32 destination writes
(32,768 bytes), totaling 49,152 bytes of source-level UB payload. Across 16
row/tile iterations this is 786,432 bytes (768 KiB) removed, along with 32
source-level `AscendC::Cast` calls per candidate launch.

GM payload is unchanged; no additional UB buffer is allocated. These byte
counts describe source-level local-tensor traffic, not measured physical UB
transactions or generated machine instructions.

## ROUTE SEPARATION

N04 concerns GM/L2 access policy; this change leaves GM load order and cache
policy untouched, and removes repeated conversion after data is already in
UB. N05 concerns Scalar address generation; no address expression, offset, or
index calculation changes here.

## COMPILE / DEVICE

SERVER: `cann-server3` (`hwnput3`)
CANN: 8.5.0.alpha002; sourced the existing
`/usr/local/Ascend/ascend-toolkit/set_env.sh` and used the paired Runner's
recorded system/toolkit C++ include paths.
COMMAND: configure the existing paired CMake project and build only
`paired_runner_v016`.
RETURN_CODE: 0; `[100%] Built target paired_runner_v016`.
RUNTIME_LINK: `ldd` resolved all listed libraries, including `libgraph.so`
from `/usr/local/Ascend/ascend-toolkit/latest/lib64`.
EVIDENCE: `configure.log`, `compile.log`, and `ldd.log`.
REMOTE_BUILD_DIR: `/tmp/wide-ub-residency-x-v005-build.9NYOdo` (retained for the
allocated device run).
DEVICE: not run; no device process was launched.
ALLOCATION_REQUEST: not sent; this runtime exposed no Main recipient or
multi-agent messaging tool after Candidate Compile.
DEVICE_RESET: NO.
PUSH: NO.
