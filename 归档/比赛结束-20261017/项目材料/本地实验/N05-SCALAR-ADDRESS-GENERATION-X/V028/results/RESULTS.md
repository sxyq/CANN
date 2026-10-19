# N05 SCALAR-ADDRESS-GENERATION-X V028 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X

REVISION: V028

PARENT: V027 / source commit 56b2940aa688637c0d1260caa8f1ac953ae7f1d4

SOURCE_COMMIT: d6f9606dbc500e2fb0e1d17dd4603a729f3bec38

CHANGE: In ProcessWideLowPrecision pass one, derive the retained-y UB tile address as `batchRow * rowWidth + col` instead of advancing `valueTileOffset`. Arithmetic, tile order, DMA/event order, Core ownership, and Vector emission remain unchanged. This is distinct from V022's pass-two retained-y address calculation.

SOURCE_IDENTITY: Parent V027 local and remote source SHA256 are both `adb2c54017dc1ee120ddb8037939e44512eb63c02d4dd939932d3be2e17e7229`. Candidate V028 local and remote source SHA256 are both `0e0f0a7bbf6417404175818800d07b892702c563acde9710de9a9d82549949d6`. CMakeCache binds separate Parent and Candidate directories. The built Runner SHA256 is `9d19b61a51a0b2c68731aeae57ce0416fa511d4afef8da9b5c5b355fb08aecad`, matching the saved local binary.

COMPILE: PASS, CONFIGURE_RC=0 and BUILD_RC=0. Existing paired CMake target `paired_runner_v001` built with CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The configure output includes toolkit permission notices for shared OPP paths and the existing CMake runtime search-path warning; configuration and build both completed successfully.

CORRECTNESS: PASS, all four cases for Parent and Candidate on d2; eight binary/case checks passed; CORRECTNESS_RC=0; `RUNNER_COMPLETE mode=CORRECTNESS cases=4 rows=16 device_reset=NO`. Runtime used `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6` and the established N05 CANN 8.5 `LD_LIBRARY_PATH` recorded in `build-identity.txt`.

LOCAL_SCORE: NONE

LOCAL_DELTA: NONE

LOCAL_METHOD: One run of the existing paired Runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The raw log contains 84 SAMPLE rows and `RUNNER_COMPLETE mode=PAIRED cases=4 rows=16 device_reset=NO`. No samples were selected or removed.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Paired delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 18.280 | 12.780 | -1.860 | -19.900..16.220 | 21 |
| FP16 D=32768 | 19.280 | 20.260 | +0.900 | -20.680..17.480 | 21 |
| BF16 D=12288 | 13.380 | 12.880 | +0.100 | -21.820..18.900 | 21 |
| BF16 D=32768 | 30.200 | 30.200 | +0.640 | -52.200..6.020 | 21 |

LOCAL_VALIDITY: NONE. Paired-delta medians have mixed signs and every range spans zero. Variation is broad; BF16 D=32768 includes a -52.200 us paired difference. The measurements do not establish a stable gain or a dependable case-specific signal.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. At 2026-10-12 02:24:59 UTC before Correctness, HBM was 62855/65536 MB and AICore was 13%. Before Local at 02:28:19 UTC, HBM was 62855/65536 MB and AICore was 11%. After Local at 02:31:07 UTC, HBM was 62856/65536 MB and AICore was 14%. Python PID 816979 using 59487 MB on d2 remained present and was not modified; other listed devices also had active processes. Device snapshots are retained. `device_reset=NO`.

REMOTE_RUN_DIR: `/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V028/0e0f0a7b-v027-parent`

EVIDENCE: `configure.log`, `CMakeCache.txt`, `compile.log`, `correctness.raw.log`, `local.raw.log`, `paired_runner_v028-linux-aarch64`, `build-identity.txt`, and pre-Correctness/pre-Local/post-Local device snapshots are stored in this results directory.

NEXT_ACTION: Window-end stop. Do not start V029 in this window.
