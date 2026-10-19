# N05 SCALAR-ADDRESS-GENERATION-X V022 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V022
PARENT: V021 / source commit bca94254e8cbfe639c593366a4637addfcd5cb3e
SOURCE_COMMIT: a05054d177c849c4132e0ffd6671298e26453298

CHANGE: In ProcessWideLowPrecision pass two, derive the retained-y UB row/tile address as batchRow * rowWidth + col for each output row. The per-tile valueOffset cursor and rowWidth increment are removed. Math, tile order, DMA/event order, Core ownership, and Vector scheduling remain unchanged.

SOURCE_IDENTITY: Parent V021 local and remote source SHA256 are both 035c5e689e0d0a91865b726eb3be7dda747064f471f577b3605dfea95fdd95b4. Candidate V022 local and remote source SHA256 are both 24fdf8e689783a23dd2d68cc26edf35185e5419ed5f85b353036351ddec74ef0. CMakeCache and the paired runner bind distinct V021 parent and V022 candidate directories.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and successful CANN plugin environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 and the established CANN library path.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. Each case used P,C order 11 times and C,P order 10 times. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; all 84 SAMPLE rows and per-binary summaries are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 13.300 | 11.620 | -0.540 | -10.320..11.520 | 21 |
| FP16 D=32768 | 19.280 | 19.460 | +0.060 | -4.080..22.620 | 21 |
| BF16 D=12288 | 12.640 | 13.460 | +0.720 | -13.160..15.660 | 21 |
| BF16 D=32768 | 27.160 | 27.320 | +0.300 | -30.860..8.540 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero and the medians are small with mixed direction. The data do not support a stable gain or an isolated case result. No sample was removed or selected.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%) and AICore usage was 0%. The full snapshot lists Python PID 816979 using 59487 MB on d2 and PID 816980 using 60246 MB on d3. Both remained present and were not modified. Pre/post snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V022/a05054d1-v021-parent

EVIDENCE: configure.log, compile.log, correctness.raw.log, local.raw.log, build-identity.txt, CMakeCache.txt, paired_runner_v022-linux-aarch64, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Preserve math, tile order, DMA/event order, and Vector scheduling.
