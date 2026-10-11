# N05 SCALAR-ADDRESS-GENERATION-X V021 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V021
PARENT: V020 / source commit bf514ec41ef0f29cbc91df43025abfa8b64306ec
SOURCE_COMMIT: bca94254e8cbfe639c593366a4637addfcd5cb3e

CHANGE: In ProcessWideLowPrecision pass two, derive each output GM tile start as batchBaseOffset + col. The per-tile storeTileOffset cursor and valid-length increment are removed. Per-row Store stride, tile order, arithmetic, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V020 local and remote source SHA256 are both 17b5586fdc1b38925a643b569d3bfad821fbeaec05d1df20bc3c9c97ba366641. Candidate V021 local and remote source SHA256 are both 035c5e689e0d0a91865b726eb3be7dda747064f471f577b3605dfea95fdd95b4. CMakeCache and the paired runner bind separate V020 parent and V021 candidate directories.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3, using the existing paired CMake target with ASCEND_HOME_PATH and CPLUS_INCLUDE_PATH set as in the successful V020 build.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 and the established CANN library path.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. Each case used P,C order 11 times and C,P order 10 times. RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; 84 SAMPLE rows and all per-binary summaries are retained in local.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 11.040 | 13.080 | +0.780 | -16.080..25.280 | 21 |
| FP16 D=32768 | 20.020 | 21.940 | +1.460 | -24.440..73.780 | 21 |
| BF16 D=12288 | 18.020 | 17.860 | +2.820 | -32.240..72.000 | 21 |
| BF16 D=32768 | 26.820 | 27.240 | +0.060 | -12.320..38.320 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero. Medians are small relative to spread and do not establish a consistent gain; FP16 D=32768 and BF16 D=12288 have broad ranges and outliers. No samples were removed or selected to claim a gain, and no single-case signal is supported.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Local, HBM was 62855/65536 MB (about 95.9%) and AICore usage was 0%. The full snapshot lists Python PID 816979 using 59487 MB on d2 and PID 816980 using 60246 MB on d3. Both remained present and were not modified. Pre/post snapshots are retained.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V021/bca94254-v020-parent

EVIDENCE: configure.log, compile.log, correctness.raw.log, local.raw.log, build-identity.txt, CMakeCache.txt, paired_runner_v021-linux-aarch64, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with one distinct Scalar address-generation factor in ProcessWideLowPrecision. Do not change Vector instruction emission.
