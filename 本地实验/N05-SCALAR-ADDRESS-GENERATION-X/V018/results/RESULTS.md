# N05 SCALAR-ADDRESS-GENERATION-X V018 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V018
PARENT: V017 / source commit 97222450df4d33b6920f46f42128306f82907f65
SOURCE_COMMIT: 6921ccbf87d45fe8e6ebea4faaf1b403405dc53d

CHANGE: In ProcessWideLowPrecision pass one, derive the next input GM tile offset once and use it for both x and residual prefetches, then advance the current offset to that value. Tile order, arithmetic, DMA/event order, and Core ownership remain unchanged.

SOURCE_IDENTITY: Parent V017 local and remote file SHA256 are both 920aa94c3c847fa38d96ab81a62c897d8bbfbd7efb4ece7d1857feee6ffda01c. Candidate V018 local and remote file SHA256 are both 9cfa3d8367be47e467a3afde1f8aebb9dda6c28341d452e6f63717b451246703. CMakeCache names separate source/parent and source/candidate directories. The existing Runner includes those two files independently; output labels V011 and V001 map to actual sources V017 and V018 respectively.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. Per case, order P,C occurred 11 times and C,P 10 times. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pairs and the per-binary samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 14.620 | 12.820 | -0.760 | -54.160..10.580 | 21 |
| FP16 D=32768 | 17.680 | 17.740 | -0.180 | -7.000..13.680 | 21 |
| BF16 D=12288 | 17.140 | 16.900 | 0.220 | -477.360..226.200 | 21 |
| BF16 D=32768 | 33.100 | 32.340 | -0.240 | -33.780..27.260 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero. BF16 D=12288 has extreme samples in both binaries (Parent max 492.040 us; Candidate max 510.840 us) and paired deltas from -477.360 to 226.200 us. The remaining cases also contain both positive and negative paired deltas. No samples were removed or selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%); Python PID 816979 used 59487 MB and PID 816980 used 60246 MB. Both remained present and were not modified. Pre/post device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V018/6921ccbf

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v018-linux-aarch64 are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
