# N05 SCALAR-ADDRESS-GENERATION-X V023 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V023
PARENT: V022 / source commit a05054d177c849c4132e0ffd6671298e26453298
SOURCE_COMMIT: d5e070374e9f21890326916c5b07cf687a5959ce

CHANGE: In ProcessWideLowPrecision pass one, remove the advancing input GM tile cursor. The initial load uses batchBaseOffset; each prefetch derives its input offset from the current batch row, next row/tile coordinates, rowWidth, and nCol. Tile sequence, arithmetic, Load order, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V022 local and remote source SHA256 are both 24fdf8e689783a23dd2d68cc26edf35185e5419ed5f85b353036351ddec74ef0. Candidate V023 local and remote source SHA256 are both d548d7e4719f983f6c66c844912c1ada03bad15bfe38ce5101f189be6213b62b. CMakeCache binds independent Parent and Candidate directories. The paired binary SHA256 is 4d61edb5e32697e11545a9e8246aeb54f572cd0e70da87a970e6b933a178e1b4.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. Attempt 1 lacked ASCEND_HOME_PATH and the bisheng plugin crashed; attempt 2 set ASCEND_HOME_PATH but its generated helper could not find <vector>. Both logs are retained. Attempt 3 used ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 and CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11, matching the successful N05 host-header setup, and built the same Candidate successfully. Candidate source SHA did not change. See compile.log, compile-envfixed.log, and compile-cpp-path.log.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. The initial launch omitted the CANN library path and failed to load libgraph.so; it is retained as correctness-loader-failed.raw.log and is not counted as a result. The same binary then passed all four cases using the existing N05 CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The complete log has 84 SAMPLE rows and RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; no samples were removed or selected.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 19.180 | 14.940 | -4.060 | -58.380..21.500 | 21 |
| FP16 D=32768 | 22.800 | 24.420 | +0.620 | -16.420..38.660 | 21 |
| BF16 D=12288 | 12.040 | 13.540 | +0.360 | -17.180..11.460 | 21 |
| BF16 D=32768 | 29.620 | 32.980 | +3.360 | -42.920..53.160 | 21 |

LOCAL_VALIDITY: NONE. Every paired-delta range spans zero. Medians change direction across cases and are small relative to broad per-pair variation. d2 was under shared load; the FP16 D=12288 Parent reached 77.840 us and Candidate reached 43.800 us, and other cases also include broad outliers. The four medians do not establish stable improvement or an isolated case signal.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Correctness, HBM was 62855/65536 MB and AICore was 12%; before Local, HBM was 62855/65536 MB and AICore was 5%. PID 816979 (python, 59487 MB on d2) remained present in all snapshots and was not modified. Pre/post snapshots are retained. device_reset=NO.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V023/d548d7e4-v022-parent

EVIDENCE: configure.log, CMakeCache.txt, compile.log, compile-envfixed.log, compile-cpp-path.log, correctness-loader-failed.raw.log, correctness-envfixed.raw.log, local.raw.log, paired_runner_v023-linux-aarch64, build-identity.txt, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue the Scalar address-generation study with a distinct single-factor change. Preserve arithmetic, tile order, DMA/event order, Core ownership, and Vector scheduling.
