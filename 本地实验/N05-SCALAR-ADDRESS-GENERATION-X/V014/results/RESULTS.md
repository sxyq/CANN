# N05 SCALAR-ADDRESS-GENERATION-X V014 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V014
PARENT: V013 / source commit 5baa441743d3cfe307223ab0db987b275c60e81f
SOURCE_COMMIT: e550af21b8c3bd08aee25702e3fa98f815dcdb95

CHANGE: In ProcessWideLowPrecision pass one, replace retained-y Row+Tile address construction with a linear Scalar cursor. Advance it by the processed tile's valid element count. Reduction scratch addressing, tile order, arithmetic, DMA/event order, and Core ownership remain unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and the successful N05 compiler environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pair samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 20.820 | 19.480 | -3.320 | -44.160..69.580 | 21 |
| FP16 D=32768 | 23.820 | 24.360 | 0.180 | -17.160..25.320 | 21 |
| BF16 D=12288 | 14.220 | 16.360 | -0.820 | -9.880..18.480 | 21 |
| BF16 D=32768 | 27.280 | 36.480 | 6.060 | -11.920..54.920 | 21 |

LOCAL_VALIDITY: NONE. Paired-delta medians have mixed signs and every case's delta range spans zero. Candidate spread is elevated in FP16 D=12288 and both D=32768 cases. No sample is selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%). Python PID 816979 used 59487 MB and PID 816980 used 60246 MB; both remained present. Neither process was modified. Device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V014/e550af21

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v014-linux-aarch64 are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
