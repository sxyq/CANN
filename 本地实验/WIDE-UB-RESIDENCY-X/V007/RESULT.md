# WIDE-UB-RESIDENCY-X V007

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V007
PARENT: `本地实验/WIDE-UB-RESIDENCY-X/V006/submission.asc`
PARENT_SOURCE_COMMIT: `3c91563a869f18b26027e1eee0d649392959f5b9`
PARENT_RESULT_COMMIT: `5bdf8fc1`
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

For BF16 wide rows, Pass2 converts each loaded `gamma`/`bias` tile into the
FP32 work buffers before applying it. The original `gammaLocal` staging slot
is then dead for this tile: all later math reads widened parameters from
`xFp32` and `residualFp32`. Writing the BF16 output conversion into that slot
removes the separate `outputBuf_` allocation without changing arithmetic,
logical GM bytes, or synchronization.

On BF16, rows=2, D=32768, the existing 176 KiB planner selects 2560 elements
per tile: 13 tiles per row, with a 2048-element final tile. Reusing the output
slot permits a 177 KiB BF16 planner budget. The resulting tile is 3072
elements: 11 tiles per row, still with a 2048-element final tile. Both tile
sizes and final lengths are multiples of 16 BF16 elements (32 bytes).

## ONE CHANGE

- Reuse the converted-dead BF16 `gammaLocal` slot as the Pass2 output tile.
- Do not allocate `outputBuf_` in the wide BF16 path.
- Use the 177 KiB BF16 budget; FP16 and FP32 budgets stay unchanged.
- V006's Pass2 synchronization change is retained unchanged.

## ACCESS AND SPACE EVIDENCE

- Pass1 reads each `x` and `residual` element once, retains the complete
  FP32 `y` row in UB, and reduces tile partials. Pass2 does not reread `x` or
  `residual` from GM.
- At D=32768, `wideFullYRows_` is one. With two rows and the paired Runner's
  eight requested cores, the launch is clamped to two blocks, one row per
  block. Pass2 therefore reads each `gamma` and `bias` vector once per block.
- For BF16, the V006 planner chooses tile=2560 and 13 tiles. The BF16-specific
  177 KiB budget chooses tile=3072 and 11 tiles. Pass1 GM-to-UB loads fall
  from 52 to 44 `DataCopyPad` calls across two rows; Pass2 parameter loads
  fall from 52 to 44 calls across two blocks; output stores fall from 26 to
  22 calls. Total logical payload remains 655,360 bytes for the two-row
  launch; this count describes GM↔UB copy calls, not physical HBM bytes.
- Planned BF16 UB allocation at tile=2560 with `outputBuf_` is 177,216 bytes.
  At tile=3072 without `outputBuf_` it is 180,288 bytes, within the 177 KiB
  planning budget (181,248 bytes) and the 910B3 usable 184 KiB (188,416
  bytes). All buffers are multiples of 32 bytes.
- The 2048-element final tile is unchanged. No tail-specific branch is added.

## COMPILE / DEVICE

SERVER: `cann-server3` (`hwnput3`)
CANN: 8.5.0.alpha002.
RUNNER: Reuse `paired_runner_v016`; its existing candidate include now points
to V007. No new Runner or script is added.
CONFIGURE: RC=0, existing paired CMake project with
`R31B_V016_SOURCE_DIR` set to the remote V007 source directory.
BUILD_ATTEMPT_1: RC=2; compiler-generated registration code could not find
`<vector>`. No device process was started. The raw failure is `compile.log`.
BUILD: RC=0 after using the GCC 11 and CANN include directories from the
route's existing successful build record in `CPLUS_INCLUDE_PATH`;
`[100%] Built target paired_runner_v016` is in `compile-envfix.log`.
RUNTIME_LINK: PASS. The new executable resolves
`libstdc++.so.6` to `/usr/lib/aarch64-linux-gnu/libstdc++.so.6.0.30` and
`libgraph.so` to the CANN toolkit; see `ldd.log`.
REMOTE_BUILD_DIR: `/tmp/wide-ub-residency-x-v006-build.bh1s3Ux7` (existing
build directory reused for V007).
DEVICE: NPU2, Ascend910B3; lease `N02-V007-D2-CORR-LOCAL-20261016T103707Z`.
PREFLIGHT_UTC: `2026-10-16T10:42:23.431129309Z`; Health=OK,
HBM=3428/65536 MiB, AICore=0%, and no running process on NPU2. The earlier
10:38:26Z sample remains in `device-before-run-npu2.snapshot`; 10:42:23Z is
the immediate pre-run reading. Current preflight readings are in
`device-before-run-npu2-current.snapshot`,
`device-before-run-npu2-process.snapshot`, and
`device-before-run-npu2-list.snapshot`. The installed `npu-smi` rejected
`info -t process` and reported that per-device `info proc` is unsupported;
the supported all-device `npu-smi info` output showed NPU2 had no process.
ENVIRONMENT: sourced `/usr/local/Ascend/ascend-toolkit/set_env.sh` and put
`/usr/lib/aarch64-linux-gnu` before toolkit libraries in `LD_LIBRARY_PATH`,
using the route's V006 runtime resolution.
COMMAND: `paired_runner_v016 --paired --device 2` from the existing build
directory; one invocation, no retry.
START_UTC: `2026-10-16T10:43:00.483959436Z`.
RETURN_CODE: 0; the Runner reports four cases, 45 warmups per binary, 21
pairs per case, and Parent/Candidate Correctness results all PASS.
| Case | Correctness | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10-P90 (us) |
|---|---|---:|---:|---:|---:|
| FP16 D=12288 | PASS/PASS | 14.180 | 14.900 | +0.140 | -1.520 to +7.480 |
| FP16 D=32768 | PASS/PASS | 13.080 | 12.880 | -0.160 | -2.500 to +3.600 |
| BF16 D=12288 | PASS/PASS | 16.020 | 15.700 | -0.480 | -10.040 to +2.220 |
| BF16 D=32768 | PASS/PASS | 22.100 | 18.640 | -1.500 | -7.160 to +2.280 |

Delta is Candidate minus Parent. All four P10-P90 intervals cross zero;
`LOCAL_SCORE=NONE`.
END_UTC: `2026-10-16T10:43:07.216700643Z`.
POSTFLIGHT_UTC: `2026-10-16T10:43:07.218872809Z`; NPU2 Health=OK,
HBM=3430/65536 MiB, AICore=0%, and no running process. Full raw output,
including preflight, every sample, Correctness, and postflight, is preserved in
`paired-run-npu2.log`.
DEVICE_RESET: NO.
LEASE_RELEASE: report postflight to Main for shared lease release; the shared
scheduling table was not changed by this route.
PUSH: NO.
