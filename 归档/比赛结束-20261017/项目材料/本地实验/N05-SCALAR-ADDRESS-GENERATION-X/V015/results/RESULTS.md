# N05 SCALAR-ADDRESS-GENERATION-X V015 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V015
PARENT: V014 / source commit e550af21b8c3bd08aee25702e3fa98f815dcdb95
SOURCE_COMMIT: 37cb7ebfac4f61a6387cecd93cf832874a971c7b

CHANGE: In ProcessWideLowPrecision pass two, replace per-tile output GM start-address calculation with a Scalar cursor advanced by each tile's valid element count. Per-row store advancement, Store order and count, arithmetic, other DMA/event order, tile order, and Core ownership are unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and successful N05 compiler environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pair samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 13.700 | 14.660 | -0.320 | -15.240..19.500 | 21 |
| FP16 D=32768 | 29.780 | 22.440 | -7.560 | -138.660..18.980 | 21 |
| BF16 D=12288 | 14.260 | 14.780 | 0.320 | -8.620..8.360 | 21 |
| BF16 D=32768 | 30.220 | 28.340 | -1.660 | -22.680..15.240 | 21 |

LOCAL_VALIDITY: NONE. Every paired-delta range spans zero. The FP16 D=32768 case has a large negative excursion and broad spread; the medians do not establish a stable gain across cases. No sample is selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%). Python PID 816979 used 59487 MB and PID 816980 used 60246 MB; both remained present. Neither process was modified. Device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V015/37cb7ebf

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v015-linux-aarch64 are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
