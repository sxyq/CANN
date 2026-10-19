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
DEVICE_RESET: NO.

## PAIRED RUN

The existing Runner was executed once on NPU7 after sourcing
`/usr/local/Ascend/ascend-toolkit/set_env.sh`:

```text
paired_runner_v016 --paired --device 7
```

RETURN_CODE: 0. The run covered rows=2 for FP16/BF16 at D=12288 and D=32768;
each case used 45 warmups per binary and 21 interleaved timing pairs. Device
event timing was primary. Parent was V011 and Candidate was V016. The complete
Runner output is `paired-run.log`; `paired-run.rc` and the start/end time files
preserve invocation status and timing.

## CORRECTNESS / LOCAL

All eight Parent/Candidate correctness checks passed with zero failures. Times
and paired deltas are microseconds; paired delta is Candidate minus Parent.

| Case | Parent correctness | V016 correctness | Parent median | V016 median | Paired delta median | Paired delta P10–P90 |
|---|---|---|---:|---:|---:|---:|
| fp16-tail-d12288 | PASS | PASS | 7.640 | 6.980 | -0.380 | -15.800..9.460 |
| fp16-wide-d32768 | PASS | PASS | 12.720 | 12.140 | -0.320 | -7.640..10.920 |
| bf16-tail-d12288 | PASS | PASS | 16.020 | 17.220 | -0.260 | -11.960..11.680 |
| bf16-wide-d32768 | PASS | PASS | 14.540 | 13.900 | -0.700 | -10.300..8.560 |

Maximum absolute/relative errors were 0.0009765625/0.0015923567 for FP16
D=12288, 0.001953125/0.010989011 for FP16 D=32768, 0.0078125/0.0073529412
for BF16 D=12288, and 0.0078125/0.01754386 for BF16 D=32768. Every paired
P10–P90 interval spans zero. The BF16 medians are small relative to the
observed spread, and the FP16 kernel path is unchanged by V005. This single
run shows no stable Local gain.

LOCAL_SCORE: NONE
LOCAL_DELTA: NO_STABLE_GAIN

## DEVICE

DEVICE: NPU7 on `hwnput3` (Ascend910B3). The captured preflight at
2026-10-16T08:48:03Z reported Health=OK, HBM 3449/65536 MiB, AICore 0%, and no
running process. The Runner invocation began at 08:48:07Z and ended at
08:48:13Z with RC=0. The captured post-run state at 08:48:17Z reported
Health=OK, HBM 3430/65536 MiB, AICore 0%, and no running process. Main later
observed NPU7 idle and released it at 2026-10-16T08:51:31Z.
DEVICE_RESET: NO. OTHER_PROCESS_ACTION: NONE.
EVIDENCE: `paired-run.log`, `paired-run.rc`, `paired-run-start.time`,
`paired-run-end.time`, and the `device-before-run.*` / `device-after-run.*`
snapshots. `paired-run.log` retains the raw per-sample output.

## RESULT

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NO_STABLE_GAIN
OFFICIAL: NOT_RUN
SOURCE_COMMIT: `42650fc80a08bdbc61dda7ae280c903c615644c2`
PUSH: NO
