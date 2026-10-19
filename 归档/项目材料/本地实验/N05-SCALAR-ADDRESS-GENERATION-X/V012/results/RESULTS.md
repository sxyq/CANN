# N05 SCALAR-ADDRESS-GENERATION-X V012 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V012
PARENT: V011 / ec078b52b0ab94b8b80bfd22a7793ce470aab6d3
SOURCE_COMMIT: 5ce3904a554eb9fc2c2db90a8b72198b3739597a

CHANGE: In ProcessWideLowPrecision pass two, use an advancing Scalar cursor for gamma/bias next-tile GM prefetch offsets. The prefetched tile order, arithmetic, DMA call order, events, and ownership remain unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. Existing V001 paired CMake target used V011 as Parent and V012 as Candidate.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2. CORRECTNESS_RC=0; device_reset=NO. Runtime used LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. The 84 pair rows and per-binary samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 15.000 | 13.620 | 0.140 | -57.180..58.240 | 21 |
| FP16 D=32768 | 19.740 | 19.760 | 0.100 | -20.440..17.420 | 21 |
| BF16 D=12288 | 19.080 | 19.940 | 0.500 | -47.280..19.200 | 21 |
| BF16 D=32768 | 27.100 | 26.840 | -0.200 | -8.960..10.180 | 21 |

LOCAL_VALIDITY: NONE. Parent/Candidate CV ranges from 0.109 to 0.857. FP16 D=12288 and BF16 D=12288 have large spread and paired-delta ranges; the other cases also show deltas on both sides of zero. No selected sample is used to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2
PRE_LOCAL_LOAD: HBM usage 95%, AICore 4%, AIVector 11%; Python PID 816979 used 59487 MB.
POST_LOCAL_LOAD: HBM usage 95%, AICore 9%, AIVector 5%; Python PID 816979 used 59487 MB.
REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V012/5ce3904a6f4d948129c4cf08757e4a90e76b07d0

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v012-linux-aarch64 are stored here. Runner output labels Parent as V011 and Candidate as V001; identity maps those labels to V011 and V012 source files.

NEXT_ACTION: Continue N05 with a distinct Scalar address-generation factor in ProcessWideLowPrecision. Keep tile order, arithmetic, DMA/event sequence, and Core ownership unchanged.
