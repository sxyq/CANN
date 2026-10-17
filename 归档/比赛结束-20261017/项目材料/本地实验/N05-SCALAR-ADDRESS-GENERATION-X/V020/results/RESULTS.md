# N05 SCALAR-ADDRESS-GENERATION-X V020 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V020
PARENT: V019 / source commit 1521893105b92c02a47e07cea99fdafc3340a066
SOURCE_COMMIT: bf514ec41ef0f29cbc91df43025abfa8b64306ec

CHANGE: In ProcessWideLowPrecision pass two, derive the next gamma/bias GM parameter offset from the current tile column plus tileWidth. The independent next-parameter cursor and its increment are removed. Parameter tile sequence, arithmetic, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V019 local and remote source SHA256 are both 57a8b98de7bceae8785e126f62eacc69dc23db91d4b30a56c38631fbef55ab4c. Candidate V020 local and remote source SHA256 are both 17b5586fdc1b38925a643b569d3bfad821fbeaec05d1df20bc3c9c97ba366641. CMakeCache and the existing runner bind independent V019 parent and V020 candidate directories.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target and successful plugin environment.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 and the existing CANN library path.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. Each case used P,C order 11 times and C,P order 10 times. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; all 84 SAMPLE rows and per-binary summaries are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21.200 | 13.940 | +1.000 | -111.600..42.060 | 21 |
| FP16 D=32768 | 19.040 | 19.240 | +0.100 | -26.720..9.300 | 21 |
| BF16 D=12288 | 12.000 | 14.840 | +1.280 | -11.220..11.380 | 21 |
| BF16 D=32768 | 26.720 | 26.740 | -0.200 | -35.320..14.300 | 21 |

LOCAL_VALIDITY: NONE. Every paired-delta range spans zero; the medians are small and mixed in sign. FP16 D=12288 has a large Parent outlier (device 139.880 us), and d2 HBM usage remained about 95.9%. No sample was removed or selected to claim a gain; no independent case signal is supported.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%) and AICore usage was 5%. The full snapshot lists Python PID 816979 using 59487 MB on d2 and PID 816980 using 60246 MB on d3. Both remained present and were not modified. Pre/post snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V020/bf514ec4-v019-parent

EVIDENCE: configure.log, compile.log, correctness.raw.log, local.raw.log, build-identity.txt, CMakeCache.txt, paired_runner_v020-linux-aarch64, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
