# N05 SCALAR-ADDRESS-GENERATION-X V013 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V013
PARENT: V012 / source commit 5ce3904a554eb9fc2c2db90a8b72198b3739597a
SOURCE_COMMIT: 5baa441743d3cfe307223ab0db987b275c60e81f

CHANGE: In ProcessWideLowPrecision pass one, use an advancing Scalar cursor for the reduction scratch tile offset. Increment once per processed tile and reset at each row boundary. Tile order, reduction order, arithmetic, DMA/event order, and Core ownership are unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The first three build attempts are preserved: missing ASCEND_HOME_PATH, plugin missing <vector> with a direct toolkit variable, and plugin missing <vector> after the toolkit environment script. The final retry used the successful N05 environment, including CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11. See compile-cpp-path.log.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. The first launch failed to load libgraph.so and is preserved in correctness.raw.log. The same binary passed with the recorded CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 in correctness-envfixed.raw.log.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; LOCAL_RC=0. All 84 pair samples are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 13.280 | 11.180 | -1.960 | -28.060..10.640 | 21 |
| FP16 D=32768 | 18.820 | 19.660 | 0.940 | -20.780..47.300 | 21 |
| BF16 D=12288 | 19.800 | 17.680 | 1.060 | -98.540..918.960 | 21 |
| BF16 D=32768 | 27.560 | 27.400 | -0.040 | -36.820..38.220 | 21 |

LOCAL_VALIDITY: NONE. Paired-delta medians change sign across cases and every case has a range spanning zero. BF16 D=12288 has especially large spread, reaching 918.960 us. No individual sample is selected to claim a gain.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%); Python PID 816979 used 59487 MB and remained present. The process was not modified. Device snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V013/5baa4417

EVIDENCE: compile.log, compile-envfixed.log, compile-cann-env.log, compile-cpp-path.log, correctness.raw.log, correctness-envfixed.raw.log, local.raw.log, device snapshots, build-identity.txt, CMakeCache.txt, and paired_runner_v013-linux-aarch64 are stored here.

NEXT_ACTION: Continue with a distinct Row-Tile Scalar address-generation factor in ProcessWideLowPrecision. Keep vector instruction emission, arithmetic, DMA/event order, tile order, and Core ownership unchanged.
