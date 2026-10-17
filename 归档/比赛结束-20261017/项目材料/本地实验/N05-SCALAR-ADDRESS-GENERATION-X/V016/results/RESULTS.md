# N05 SCALAR-ADDRESS-GENERATION-X V016 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V016
PARENT: V015 / source commit 37cb7ebfac4f61a6387cecd93cf832874a971c7b
SOURCE_COMMIT: 855e0afc263f25f18fb4ae8acd7fb061056124a3

CHANGE: In ProcessWideLowPrecision pass one, initialize an input GM batch-base Scalar cursor from beginRow and advance it by the actual batchRows after each retained batch. The existing in-batch tile cursor, tile order, arithmetic, DMA/event order, and Core ownership remain unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and successful N05 compiler environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pair samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 14.380 | 12.060 | -0.720 | -22.120..37.840 | 21 |
| FP16 D=32768 | 30.520 | 30.320 | 1.200 | -38.180..42.860 | 21 |
| BF16 D=12288 | 22.760 | 17.680 | 0.720 | -137.780..135.120 | 21 |
| BF16 D=32768 | 26.560 | 27.100 | 0.240 | -14.560..14.580 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero, with especially broad spread for FP16 D=32768 and BF16 D=12288. The medians do not establish a consistent gain. No individual sample is selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%). Python PID 816979 used 59487 MB and PID 816980 used 60246 MB; both remained present. Neither process was modified. Device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V016/855e0afc

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v016-linux-aarch64 are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
