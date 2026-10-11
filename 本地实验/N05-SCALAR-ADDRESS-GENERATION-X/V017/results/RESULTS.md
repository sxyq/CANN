# N05 SCALAR-ADDRESS-GENERATION-X V017 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V017
PARENT: V016 / source commit 855e0afc263f25f18fb4ae8acd7fb061056124a3
SOURCE_COMMIT: 97222450df4d33b6920f46f42128306f82907f65

CHANGE: In ProcessWideLowPrecision pass two, initialize an output GM batch-base Scalar cursor from beginRow and advance it by the actual batchRows after each retained batch. The per-tile output cursor, Store order and count, arithmetic, DMA/event order, and Core ownership remain unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and successful N05 compiler environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pair samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 15.260 | 12.200 | -1.000 | -20.420..18.460 | 21 |
| FP16 D=32768 | 18.880 | 19.200 | 0.480 | -50.700..17.200 | 21 |
| BF16 D=12288 | 12.960 | 15.840 | -0.200 | -11.280..39.160 | 21 |
| BF16 D=32768 | 24.940 | 24.940 | 0.260 | -15.780..10.820 | 21 |

LOCAL_VALIDITY: NONE. Every paired-delta range spans zero, with large excursions in FP16 D=32768 and BF16 D=12288. Median signs are mixed and the measurements do not establish a stable gain. No sample is selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%). Python PID 816979 used 59487 MB and PID 816980 used 60246 MB; both remained present. Neither process was modified. Device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V017/97222450

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v017-linux-aarch64 are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
