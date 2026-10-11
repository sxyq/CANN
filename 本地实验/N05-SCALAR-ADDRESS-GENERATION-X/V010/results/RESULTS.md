# N05 SCALAR-ADDRESS-GENERATION-X V010 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V010
PARENT: V009 / 768dd8b88ec5993db984f01e95a9472d98ce5980
SOURCE_COMMIT: 531158beb674dcc6e28cea2e0de13854b70c22eb

CHANGE: In ProcessWideLowPrecision pass two, replace the retained-y UB row-offset table with a tile-local offset cursor that advances by rowWidth after each row. V009's output GM offset path is unchanged. Row/tile traversal, arithmetic, DMA calls, event sequence, and Core ownership are unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. Existing V001 paired CMake target used the V009 parent and V010 candidate files.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate. The runner used device 2 and returned `device_reset=NO`.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. LOCAL_RC=0. All 84 paired rows for both binaries remain in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Pairs |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 28.220 | 24.100 | -8.100 | 21 |
| FP16 D=32768 | 30.540 | 26.600 | -5.100 | 21 |
| BF16 D=12288 | 26.340 | 27.800 | 1.760 | 21 |
| BF16 D=32768 | 27.820 | 33.600 | 2.160 | 21 |

LOCAL_VALIDITY: NONE. Parent/Candidate CV ranged from 0.257 to 0.420; paired deltas had wide ranges, including -34.820 to 32.720 us for BF16 D=32768. No favorable sample was selected as a score.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2
PRE_LOCAL_LOAD: HBM usage 95%, AICore 17%, AIVector 15%; existing Python process PID 816979 used 59485 MB.
POST_LOCAL_LOAD: HBM usage 95%, AICore 17%, AIVector 8%; the same Python process remained present and unchanged.
REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V010/531158beb674dcc6e28cea2e0de13854b70c22eb

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v010-linux-aarch64 are stored here. The runner labels its Parent binary V011 and Candidate binary V001; build identity binds those labels to V009 and V010 source files.

NEXT_ACTION: Continue N05 with a different Scalar GM input-address generation change in ProcessWideLowPrecision. Preserve traversal, arithmetic, DMA, event, and ownership behavior.
