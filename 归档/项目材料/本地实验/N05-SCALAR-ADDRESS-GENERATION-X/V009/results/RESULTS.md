# N05 SCALAR-ADDRESS-GENERATION-X V009 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V009
PARENT: V008 / 8f2f031e3f2884b663793f91a1782c5daabe53da
SOURCE_COMMIT: 768dd8b88ec5993db984f01e95a9472d98ce5980

CHANGE: In ProcessWideLowPrecision pass two, replace the per-row output GM offset table with a per-tile starting offset and a rowWidth increment after each Store. The row and tile traversal, arithmetic, DMA calls, event order, and output values are unchanged. No other kernel address path changed.

RUNNER_CONFIGURATION: Reused the V001 paired runner. Its existing device constant is set to the allocated d2 (runtime device 2); no separate runner was added.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3, Ascend910B3. Existing V001 paired CMake target was built from the V008 Parent source and V009 Candidate source.
COMPILE_COMMAND_1: cmake -S /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980/support/paired -B /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980/build -DN05_PARENT_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980/source/parent -DN05_CANDIDATE_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980/source/candidate
COMPILE_COMMAND_2: cmake --build /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980/build --parallel 2

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate. The first launch failed before device initialization because the CANN toolchain libstdc++ was selected without GLIBCXX_3.4.29. That raw error is retained. The same binary then passed with the V008 library order and again with LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6. Both successful runs used device 2 and ended with device_reset=NO.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner, d2, 4 cases, 21 interleaved pairs per case, warmup_each=45, device event timing with wall-clock diagnostics, LOCAL_RC=0.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Pairs |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 12.200 | 12.740 | 0.320 | 21 |
| FP16 D=32768 | 20.060 | 19.760 | -0.220 | 21 |
| BF16 D=12288 | 52.100 | 37.620 | 6.760 | 21 |
| BF16 D=32768 | 110.900 | 94.900 | -3.700 | 21 |

LOAD_NOTE: Before Local, d2 reported HBM usage 95%, AICore 24%, AIVector 42%; an existing Python process (PID 816979) used 59485 MB. After Local, HBM usage remained 95%, AICore was 11%, and the same process remained present. No process was changed. Parent/Candidate device-latency CV ranged from 0.238 to 0.663, with large paired outliers; the sample set does not support a score.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2
REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V009/768dd8b88ec5993db984f01e95a9472d98ce5980

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, build-identity.txt, CMakeCache.txt, and paired_runner_v009-linux-aarch64 are stored in this directory. The V008 Direct-Parent raw logs, device snapshots, and build identity files were saved with the V009 source commit 768dd8b8.

NEXT_ACTION: Continue N05 with a different Scalar address-generation factor in ProcessWideLowPrecision. Preserve traversal, arithmetic, DMA, event, and ownership semantics.
