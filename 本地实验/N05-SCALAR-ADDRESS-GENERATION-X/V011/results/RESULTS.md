# N05 SCALAR-ADDRESS-GENERATION-X V011 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V011
PARENT: V010 / 531158beb674dcc6e28cea2e0de13854b70c22eb
SOURCE_COMMIT: ec078b52b0ab94b8b80bfd22a7793ce470aab6d3

CHANGE: In ProcessWideLowPrecision pass one, stream the x/residual GM input offset by each consumed tile's valid length. Only the input address calculation changed. Row/tile traversal, arithmetic, DMA calls/order, event sequence, and Core ownership are unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. Existing V001 paired target used V010 as Parent and V011 as Candidate.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2. CORRECTNESS_RC=0; `device_reset=NO`.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. LOCAL_RC=0. All raw pair rows remain in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Pairs |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 12.420 | 11.640 | -0.380 | 21 |
| FP16 D=32768 | 19.620 | 19.240 | -0.160 | 21 |
| BF16 D=12288 | 18.500 | 18.380 | -0.940 | 21 |
| BF16 D=32768 | 26.400 | 26.880 | 0.180 | 21 |

LOCAL_VALIDITY: NONE. Parent/Candidate device CV ranged from 0.20294 to 0.38606; paired delta ranges were wide, from -18.300 to 15.800 us. The result does not support a performance score.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2
PRE_LOCAL_LOAD: HBM usage 95%, AICore 7%, AIVector 8%; Python PID 816979 used 59487 MB.
POST_LOCAL_LOAD: HBM usage 95%, AICore 0%, AIVector 0%; the same Python process used 59487 MB.
REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V011/ec078b52b0ab94b8b80bfd22a7793ce470aab6d3

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v011-linux-aarch64 are stored here. Runner output labels Parent as V011 and Candidate as V001; identity maps these to V010 and V011 source files.

NEXT_ACTION: Continue with a distinct Scalar address-generation change for pass-two gamma/bias prefetch. Preserve traversal, arithmetic, DMA, event, and ownership behavior.
