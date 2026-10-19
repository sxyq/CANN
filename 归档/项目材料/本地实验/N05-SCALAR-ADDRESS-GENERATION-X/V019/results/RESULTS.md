# N05 SCALAR-ADDRESS-GENERATION-X V019 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V019
PARENT: V018 / source commit 6921ccbf87d45fe8e6ebea4faaf1b403405dc53d
SOURCE_COMMIT: 1521893105b92c02a47e07cea99fdafc3340a066

CHANGE: In ProcessWideLowPrecision, share the input and output batch-base Scalar cursor. Both previously started at beginRow * rowWidth and advanced by batchRows * rowWidth. Per-tile address calculation, traversal, arithmetic, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V018 local and remote source SHA256 are both 9cfa3d8367be47e467a3afde1f8aebb9dda6c28341d452e6f63717b451246703. Candidate V019 local and remote source SHA256 are both 57a8b98de7bceae8785e126f62eacc69dc23db91d4b30a56c38631fbef55ab4c. CMakeCache and the existing paired runner bind independent V018 parent and V019 candidate directories.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The first build attempt lacked ASCEND_HOME_PATH and is retained as compile-v017-parent.log; the environment-fixed build and the direct-parent V018/V019 build both passed. The direct-parent build log is compile-v018-parent.log.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 and the existing CANN library path.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. Each case used P,C order 11 times and C,P order 10 times. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; 84 SAMPLE rows and all per-binary summaries are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 11.040 | 13.280 | +0.960 | -37.560..18.320 | 21 |
| FP16 D=32768 | 18.260 | 18.620 | 0.000 | -81.460..4.600 | 21 |
| BF16 D=12288 | 11.800 | 13.280 | -0.240 | -18.420..29.980 | 21 |
| BF16 D=32768 | 26.640 | 27.840 | +0.260 | -25.060..25.580 | 21 |

LOCAL_VALIDITY: NONE. Every case's paired-delta range spans zero and the median deltas are small and mixed in sign. FP16 D=32768 has a large Parent outlier (device 111.460 us; wall 3806.420 us); the other cases also have broad ranges. No sample was removed or selected to claim a gain, and no single-case signal is supported.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62856/65536 MB (about 95.9%) and AICore usage was 11%. The full snapshot lists Python PID 816979 using 59487 MB on d2 and PID 816980 using 60246 MB on d3. Both remained present and were not modified. Pre/post snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V019/15218931-v018-parent

EVIDENCE: compile-v017-parent.log records the first environment failure. compile-envfixed-v017-parent.log and the correctness/local outputs tied to the initial V017-parent diagnostic attempt are retained with v017-parent in their filenames and excluded from V019 scores. The valid V018-parent inputs, build, Correctness and paired Local are identified in build-identity.txt; configure-v018-parent.log, compile-v018-parent.log, correctness.raw.log, local.raw.log, CMakeCache.txt, paired_runner_v019-linux-aarch64, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
