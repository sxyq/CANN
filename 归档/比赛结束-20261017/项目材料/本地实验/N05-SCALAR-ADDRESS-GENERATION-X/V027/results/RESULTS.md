# N05 SCALAR-ADDRESS-GENERATION-X V027 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X

REVISION: V027

PARENT: V026 / source commit 1fbb6af47078e92057c0686ec96892e6db903472

SOURCE_COMMIT: 56b2940aa688637c0d1260caa8f1ac953ae7f1d4

CHANGE: In the wide low-precision reduction loop, derive each scratch row base as `batchRow * reduceStride` instead of advancing `reduceRowOffset`. The batch and tile sequence, reduction arithmetic, DMA/event sequence, Core ownership, and Vector emission are unchanged.

SOURCE_IDENTITY: Parent V026 local and remote source SHA256 are both `b10e3160e682d2e24a264a3ec842d14f213babdd75f192418587ab2192c1d094`. Candidate V027 local and remote source SHA256 are both `adb2c54017dc1ee120ddb8037939e44512eb63c02d4dd939932d3be2e17e7229`. CMakeCache binds the separate remote Parent and Candidate source directories. The built Runner SHA256 is `f85a67a643da207b8beb39d89d8d3cef50da8a1a12bf91cbcf77bcac2770fe50`, matching the saved local binary.

COMPILE: PASS, BUILD_RC=0. Existing CANN 8.5.0.alpha002 paired target on hwnput3 / Ascend910B3. `compile.log` ends with `Built target paired_runner_v001`.

CORRECTNESS: PASS, all four cases for Parent and Candidate on d2; eight binary/case checks passed; env-fixed CORRECTNESS_RC=0; `RUNNER_COMPLETE mode=CORRECTNESS cases=4 rows=16 device_reset=NO`. The first launch returned RC=1 before any Runner output. Its stdout capture is retained as the empty `correctness-loader-failed.raw.log`; that launch did not explicitly use the established N05 CANN `LD_LIBRARY_PATH`, and the capture did not merge stderr. The same binary then passed with the recorded CANN 8.5 runtime library path and `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`. No source edit or rebuild was made for this environment retry.

LOCAL_SCORE: NONE

LOCAL_DELTA: NONE

LOCAL_METHOD: One run of the existing paired Runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The raw log contains 84 SAMPLE rows and `RUNNER_COMPLETE mode=PAIRED cases=4 rows=16 device_reset=NO`. All samples are retained.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Paired delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 13.160 | 12.420 | -0.220 | -22.140..16.580 | 21 |
| FP16 D=32768 | 19.480 | 20.300 | +0.820 | -10.300..19.560 | 21 |
| BF16 D=12288 | 12.260 | 11.640 | -0.260 | -13.780..13.700 | 21 |
| BF16 D=32768 | 27.640 | 26.780 | -0.620 | -32.180..11.500 | 21 |

LOCAL_VALIDITY: NONE. Paired-delta medians have mixed signs, every range spans zero, and each case shows substantial spread and outliers. The measurements do not establish a stable gain or a reliable case-specific signal; no samples were selected or removed.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. HBM remained 62855/65536 MB in the pre-Correctness, pre-Local, and post-Local snapshots. AICore readings were 7% before Correctness, 0% before Local, and 16% after Local. Existing Python PID 816979 using 59487 MB on d2 remained present; it was not modified. Other listed devices also had active processes. Snapshots are retained. `device_reset=NO`.

REMOTE_RUN_DIR: `/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V027/b10e3160-v026-parent`

EVIDENCE: `configure.log`, `CMakeCache.txt`, `compile.log`, `correctness-loader-failed.raw.log`, `correctness.raw.log`, `local.raw.log`, `paired_runner_v027-linux-aarch64`, and pre-Correctness/pre-Local/post-Local device snapshots are stored in this results directory. `build-identity.txt` records source identities, execution environments, and return codes.

NEXT_ACTION: Continue with a different Scalar address-generation factor. Preserve arithmetic, tile order, DMA/event order, Core ownership, and Vector scheduling.
